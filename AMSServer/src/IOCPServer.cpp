#include "stdafx.h"

#include "IOCPServer.h"
#include "HeapProtectMutex.h"

// we only post accpetex after do acceptex, if there is no mistake
// so the MAX_POST_ACCEPT is the number we post accpetex first
// and it is max client number we can accpet in the same time
#define MAX_POST_ACCEPT 100

//1MB stack size enough for now
//whole process occupy about 60MB merrory, but actually use less than 10MB
#define WORKET_STACK_SIZE 1024*1024*5*8  

//backlog, see Stevens' book, the nginx set this to 511
#define BACKLOG 511

IOCPServer *IOCPServer::serversingletoninstance = NULL;



//constructor
IOCPServer::IOCPServer(u_short useport):
	completeport(NULL),
	listenhandle(NULL),
	lpfnacceptex(NULL),
	lpfngetacceptexsockaddrs(NULL)
{
	//Initialize the singletoninstance
	if ( serversingletoninstance != NULL )
	{
		AMS_DBUG("Singleton pointer is not NULL!  There are multiple FileLog!" 
			"Leaving the singleton pointer alone...\n");
	}
	else
	{
		AMS_DBUG("Setting up singleton pointer.\n");
		serversingletoninstance = this;
	}

	serverport = useport;
	memset(threadhandle, 0, MAX_THREAD_NUM);
	threadnum = 0;

	//load the win socket lib
	WSADATA wsaData;
	if(NO_ERROR != WSAStartup(MAKEWORD(2,2), &wsaData))
	{
		AMS_DBUG("WSAStartup WinSock 2.2 failed£¡\n");
		assert(0);
	}

	//establish listen socket
	listenhandle = new SocketHandle();
}

// destructor
IOCPServer::~IOCPServer(void)
{
	delete listenhandle;

	// Clear the singleton pointer.
	if ( serversingletoninstance == this )
	{
		AMS_DBUG("Clearing the singleton pointer.\n");
		serversingletoninstance = NULL;
	}
	else
	{
		AMS_DBUG("I'm not the singleton instance!  Leaving the singleton pointer alone...\n");
	}
}


//SingletonInstance Design Pattern implement
IOCPServer *IOCPServer::GetSingletonInstance(void)
{
	if ( serversingletoninstance == NULL )
	{
		AMS_DBUG("IOCPServer::GetSingletonInstance:  WARNING - the singleton is NULL"
			", and someone is accessing it!\n");
	}

	return serversingletoninstance;
}


// start the server
// we use IOCP instead of select events
// of cource, you can register other method, not implement here
bool IOCPServer::Start(void)
{
	//Initialize IOCP
	if(FALSE == InitializeIOCP())
	{
		AMS_DBUG("InitializeIOCP() failed: %d\n", GetLastError());
		return FALSE;
	}
	AMS_DBUG("InitializeIOCP() succeed\n");

	//Initialize Socket, associate it with IOCP
	if(FALSE == InitializeSocket())
	{
		AMS_DBUG("InitializeSocket() failed: %d\n", GetLastError());
		return FALSE;
	}
	AMS_DBUG("InitializeSocket() succeed\n");

	// get AcceptEx func pointer
	if(FALSE == GetAcceptExFuction())
	{
		AMS_DBUG("GetAcceptExFuction() failed: %d\n", GetLastError());
		return FALSE;
	}
	AMS_DBUG("GetAcceptExFuction() succeed\n");

	// new listeniodata only here
	// post some ACCEPT_POSTED signal, let the server start
	for(int i=0; i<MAX_POST_ACCEPT; ++i)
	{
		PerIOData *listeniodata = listenhandle->GetNewIOData();
		PostAcceptEx(listeniodata);
	}

	return TRUE;
}

void IOCPServer::Stop(void)
{
	WSAOVERLAPPED tmpoverlapped;
	ZeroMemory(&tmpoverlapped, sizeof(WSAOVERLAPPED));

	ULONG_PTR completionkey_close = 0;

	for(DWORD i=0; i < threadnum; ++i)
	{
		if(FALSE == PostQueuedCompletionStatus(completeport, NULL,
					completionkey_close, NULL))
		{
			AMS_DBUG("PostQueuedCompletionStatus failed: %d!\n", WSAGetLastError());
		}
	}

	WaitForMultipleObjects(threadnum, threadhandle, TRUE, INFINITE);

	for(DWORD i=0; i < threadnum; ++i)
	{
		CloseHandle(threadhandle[i]);
		AMS_DBUG("threadhandle %u close!\n", (uintptr_t)threadhandle[i]);
	}

	//close ListenSocket
	closesocket(listenhandle->ssocket);

	CloseHandle(completeport);

	AMS_DBUG("main thread can exit now!\n");
}

//Initialize IOCP
BOOL IOCPServer::InitializeIOCP(void)
{
	// establish completeport
	completeport = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0 );
	if ( NULL == completeport)
	{
		AMS_DBUG("CreateIoCompletionPort failed: %d!\n", WSAGetLastError());
		return FALSE;
	}

	// run worker thread
	SYSTEM_INFO systeminfo;
	GetSystemInfo(&systeminfo);
	threadnum = 
		(unsigned)systeminfo.dwNumberOfProcessors < MAX_THREAD_NUM ?
		(unsigned)systeminfo.dwNumberOfProcessors : MAX_THREAD_NUM;

	AMS_DBUG("systeminfo.dwNumberOfProcessors: %d.\n", systeminfo.dwNumberOfProcessors);
	AMS_DBUG("threadnum: %d.\n", threadnum);

	for(DWORD i=0; i < threadnum; ++i)
	{
		SIZE_T dwStackSize = WORKET_STACK_SIZE;
		threadhandle[i] = (HANDLE)_beginthreadex
			(NULL, dwStackSize, &WorkerThread, (LPVOID)this, 0, NULL);
	}

	return TRUE;
}

//Initialize Socket
BOOL IOCPServer::InitializeSocket(void)
{
	listenhandle->ssocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(INVALID_SOCKET == listenhandle->ssocket)
	{
		AMS_DBUG("WSASocket failed: %d.\n", WSAGetLastError());
		return FALSE;
	}

	//associate listen socket with IOCP, let IOCP get ACCEPT_POSTED signal
	if (NULL == CreateIoCompletionPort((HANDLE)listenhandle->ssocket, completeport, (ULONG_PTR)listenhandle, 0))
	{
		AMS_DBUG("CreateIoCompletionPort() failed: %d.\n", WSAGetLastError());
		return FALSE;
	}

	// set reuse addr option
	int reuseaddr = 1;
	if (SOCKET_ERROR == 
		setsockopt(listenhandle->ssocket, SOL_SOCKET, SO_REUSEADDR, (const char*) &reuseaddr, sizeof(int)))
	{
		AMS_DBUG("listenhandle->ssocket: setsockopt(SO_REUSEADDR) failed %d\n", WSAGetLastError());
		closesocket(listenhandle->ssocket);
		return FALSE;
	}

	//bind and listen
	SOCKADDR_IN ServerAddress;
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);                              
	ServerAddress.sin_port = htons(serverport);     

	if (SOCKET_ERROR == 
		bind(listenhandle->ssocket, (PSOCKADDR) &ServerAddress, sizeof(SOCKADDR_IN))) 
	{
		AMS_DBUG("listensocket: bind() failed, %d\n", WSAGetLastError());
		closesocket(listenhandle->ssocket);
		return FALSE;
	}

	if (SOCKET_ERROR == listen(listenhandle->ssocket, BACKLOG)) 
	{
		AMS_DBUG("listensocket: listen() failed %d\n", WSAGetLastError());
		closesocket(listenhandle->ssocket);
		return FALSE;
	}

	return TRUE;
}

//get two function pointer
//this has a lower occupancy rate than use win lib directlly
BOOL IOCPServer::GetAcceptExFuction(void)
{
	SOCKET s = listenhandle->ssocket;
	if(INVALID_SOCKET == s)
	{
		AMS_DBUG("WSASocket failed: %d.\n", WSAGetLastError());
		return FALSE;
	}

	// get the fuction pointer for AcceptEx
	DWORD dwBytes = 0; 
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	if (SOCKET_ERROR == 
		WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GUID),
		&lpfnacceptex, sizeof(LPFN_ACCEPTEX), &dwBytes, NULL, NULL))
	{
		AMS_DBUG("WSAIoctl(SIO_GET_EXTENSION_FUNCTION_POINTER, WSAID_ACCEPTEX) failed"
			": %d.\n", WSAGetLastError());
		return FALSE;
	}

	// get the fuction pointer for GetAcceptExSockAddrs
	GUID GuidAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS; 
	if (SOCKET_ERROR == 
		WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptExSockAddrs, sizeof(GUID),
		&lpfngetacceptexsockaddrs, sizeof(LPFN_GETACCEPTEXSOCKADDRS), &dwBytes, NULL, NULL))
	{
		AMS_DBUG("WSAIoctl(SIO_GET_EXTENSION_FUNCTION_POINTER, WSAID_ACCEPTEX) failed"
			": %d.\n", WSAGetLastError());
		return FALSE;
	}
	return TRUE;
}

//worker thread, usually not one,
//they are busy until exit.
//because these thread is asynchronous,
//so one socket can have more than one io.
unsigned WINAPI IOCPServer::WorkerThread(LPVOID lpParam)
{ 
	IOCPServer* temp = (IOCPServer*)lpParam;
	LPWSAOVERLAPPED lpoverlapped = NULL;
	SocketHandle *sockethandle = NULL;
	PerIOData *periodata = NULL;
	DWORD bytestransfered = 0;

	BOOL keeprunning = TRUE; 

	while (keeprunning)
	{
		// get the status of comleteport
		BOOL ret = 
			GetQueuedCompletionStatus(temp->completeport, &bytestransfered,
			(PULONG_PTR)&sockethandle, &lpoverlapped, INFINITE);

		if(FALSE == ret)  
		{  
			int sockerror = WSAGetLastError();
			AMS_DBUG("GetQueuedCompletionStatus failed: %d.\n", sockerror);

			//something wrong in GetQueuedCompletionStatus
			//but we MUST not close the handle, maybe recv or send not end
			//we don't close the handle right here
			//perhaps a good solution is heartbeat package, but need agree on both side
			//per io has been delete after postsend
			//the problem is when to close client socket which was not alive but not close obviously

			continue;  
		}

		if(0 == sockethandle)
		{
			AMS_DBUG("time to exit!!!\n");
			keeprunning = FALSE;
			continue;
		}
		
		// read the data in completeport
		periodata = CONTAINING_RECORD(lpoverlapped, PerIOData, overlapped);
		if(NULL == periodata)  
		{  
			AMS_DBUG("CONTAINING_RECORD failed: %d.\n", WSAGetLastError());
			continue;  
		}

		// free io and handle, if client close the socket
		// all normal handle and io free here, if there is no mistake
		if((0 == bytestransfered) &&
			( PerIOData::RECV_POSTED == periodata->operationtype
			|| PerIOData::SEND_POSTED == periodata->operationtype))  
		{  
			AMS_DBUG("client has close the socket, we need close the connection\n");

			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//can not delete sockethandle, it may lead to a segment fault
			//even with mutex
			//TODO: sockethandle should be deleted somewhere else
			//we should use a recycle thread with sometime delay
			// the destructor of sockethandle do recycle
			//delete sockethandle;
			//sockethandle = NULL;
			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			continue;  
		}
		
		switch(periodata->operationtype)  
		{  
			// AcceptEx  
			case PerIOData::ACCEPT_POSTED:
			{
				AMS_DBUG("recv the ACCEPT_POSTED signal\n");

				if(FALSE == temp->DoAcceptEx(periodata))
				{
					// here sockethandle is listenhandle
					// periodata is listeniodata
					// MUST NOT release them here, let master thread do the bad thing
					// JUST re post
					AMS_DBUG("DoAcceptEx failed\n");
				}
			}
			break;

			// RECV
			case PerIOData::RECV_POSTED:
			{
				AMS_DBUG("recv the RECV_POSTED signal\n");

				// re post, if failed
				// MUST not close socket here
				// because one socket may have more io in use
				if(FALSE == temp->DoRecv(sockethandle, periodata))
				{
					//MUST not force to release the periodata which hold overlapped struct
					//re post
					AMS_DBUG("DoRecv failed in %s:%d\n", 
						inet_ntoa(sockethandle->ssocketaddr.sin_addr), 
						ntohs(sockethandle->ssocketaddr.sin_port));
				}
			}
			break;

			// SEND
			case PerIOData::SEND_POSTED:
			{
				AMS_DBUG("recv the SEND_POSTED signal\n");

				//in test, found that some client may close the socket immediately after post send
				//but almost meanwhile we just do someting on the handle, so memory error
				// so actually we can not do anything about the close handle
			}
			break;

			case PerIOData::NULL_POSTED:
			default:
				AMS_DBUG("MUST exist a problem, the last error: %d.\n", WSAGetLastError());
			break;
		} //switch
	}//while
	return 0;
}

// post AcceptEx
void IOCPServer::PostAcceptEx(PerIOData* listeniodata)
{
	if(NULL == listeniodata)
	{
		AMS_DBUG("you just transfer a NULL pointer\n");
		return;
	}

	// need new socket before AcceptEx
	listeniodata->acceptsocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(INVALID_SOCKET == listeniodata->acceptsocket)
	{
		AMS_DBUG("WSASocket failed: %d.\n", WSAGetLastError());
		return;
	}

	// set the connect delay for 3 sec
	// because AcceptEx will not return, if client just connect but send no bytes
	// win7 do not support SO_CONNECT_TIME to prevent DDOS
	// instead, dwReceiveDataLength should be ((sizeof(SOCKADDR_IN)+16)*2)
	// only get the local and remote addr
	/*
	INT timeout = 3;
	if (SOCKET_ERROR == 
		setsockopt(listeniodata->acceptsocket, SOL_SOCKET, SO_CONNECT_TIME,
			(const char*)&timeout, sizeof(timeout)))
	{
		AMS_DBUG("setsockopt(SO_CONNECT_TIME) failed: %d.\n", WSAGetLastError());
		return;
	}
	*/

	// do some clean
	// make operationtype signal be ACCEPT_POSTED
	listeniodata->ResetIOBuf();
	listeniodata->ResetOverLapped();
	listeniodata->operationtype = PerIOData::ACCEPT_POSTED;  
	DWORD dwBytes = 0; 
	if (FALSE == 
		lpfnacceptex(listenhandle->ssocket, listeniodata->acceptsocket, listeniodata->databuf.buf, 
		0, sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16,
		&dwBytes, &listeniodata->overlapped))
	{
		if (WSA_IO_PENDING != WSAGetLastError()) 
		{
			AMS_DBUG("AcceptEx() falied: %d.\n", WSAGetLastError());
			closesocket(listeniodata->acceptsocket);
			listeniodata->acceptsocket = -1;
			return;
		}
	}

	return;
}

BOOL IOCPServer::DoAcceptEx(PerIOData* listeniodata)
{
	/* SO_UPDATE_ACCEPT_CONTEXT is required for shutdown() to work */
	// win7 do not support SO_UPDATE_ACCEPT_CONTEXT
	/*
	if (SOCKET_ERROR == 
		setsockopt(listeniodata->acceptsocket, SOL_SOCKET, 
			SO_UPDATE_ACCEPT_CONTEXT, (char *)&listenhandle->ssocket, sizeof(SOCKET)))
	{
		AMS_DBUG("setsockopt(SO_UPDATE_ACCEPT_CONTEXT) failed: %d.\n", WSAGetLastError());
		return FALSE;
	}
	*/

	//get the addr for client which connect me
	//for AcceptEx, Should use GetAcceptExSockAddrs
	SOCKADDR_IN *lplocalsockaddr = NULL;
	SOCKADDR_IN *lpremotesockaddr = NULL;
	int localsockaddrlen = sizeof(SOCKADDR_IN);
	int remotesockaddrlen = sizeof(SOCKADDR_IN);

	lpfngetacceptexsockaddrs(listeniodata->databuf.buf, 0,
		sizeof(SOCKADDR_IN)+16,	sizeof(SOCKADDR_IN)+16,
		(LPSOCKADDR*)&lplocalsockaddr, &localsockaddrlen,
		(LPSOCKADDR*)&lpremotesockaddr, &remotesockaddrlen);

	HeapProtectMutex::GetSingletonInstance()->WaitHeapProtectMutex();
	SocketHandle *connecthandle = new SocketHandle();
	HeapProtectMutex::GetSingletonInstance()->ReleaseHeapProtectMutex();

	connecthandle->ssocket = listeniodata->acceptsocket;
	memcpy(&connecthandle->ssocketaddr, lpremotesockaddr, sizeof(SOCKADDR_IN));
	if (NULL == CreateIoCompletionPort((HANDLE)connecthandle->ssocket, completeport, (ULONG_PTR)connecthandle, 0))
	{
		AMS_DBUG("CreateIoCompletionPort() failed: %d.\n", WSAGetLastError());
		return FALSE;
	}

	// let derive class do its thing
	if(FALSE == DeriveAcceptEx(listeniodata))
	{
		AMS_DBUG("DeriveAcceptEx failed: %d\n", WSAGetLastError());
		return FALSE;
	}

	// usually we need to recv something, so post recv 
	// we new connecthandle and connectiodata here first
	PerIOData *connectiodata = connecthandle->GetNewIOData();

	PostRecv(connecthandle, connectiodata);

	//delete old one
	listenhandle->RemoveIOData(listeniodata);

	//post new one
	PerIOData *newlisteniodata = listenhandle->GetNewIOData();
	PostAcceptEx(newlisteniodata);

	return TRUE;
}

BOOL IOCPServer::DeriveAcceptEx(PerIOData *listeniodata)
{
	AMS_DBUG("do nothing in base class\n");
	return TRUE;
}

// post recv
void IOCPServer::PostRecv(SocketHandle* connecthandle, PerIOData* connectiodata)
{
	if(NULL == connecthandle || NULL == connectiodata)
	{
		AMS_DBUG("you just transfer a NULL pointer\n");
		return;
	}

	// do some clean
	// make operationtype signal be RECV_POSTED
	DWORD flags = 0;
	DWORD recvbytes = 0;
	connectiodata->ResetOverLapped();
	connectiodata->operationtype = PerIOData::RECV_POSTED;

	//recv the message
	int nbytes = WSARecv(connecthandle->ssocket, &connectiodata->databuf + connectiodata->bytestransfer,
		1, &recvbytes, &flags, &connectiodata->overlapped, NULL );
	if ((SOCKET_ERROR == nbytes) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		AMS_DBUG("WSARecv() failed: %d.\n", WSAGetLastError());
		return;
	}

	return;
}

// do after recv
BOOL IOCPServer::DoRecv(SocketHandle* connecthandle, PerIOData* connectiodata)
{
	if(NULL == connecthandle || NULL == connectiodata)
	{
		AMS_DBUG("you just transfer a NULL pointer\n");
		return FALSE;
	}

	//process the massage
	connectiodata->bytestransfer = strlen(connectiodata->databuf.buf);
	AMS_DBUG("receive message len is %d.\n", connectiodata->bytestransfer);

	AMS_DBUG("receive from %s:%d, message:\n%s\n", 
			inet_ntoa(connecthandle->ssocketaddr.sin_addr), 
			ntohs(connecthandle->ssocketaddr.sin_port),connectiodata->databuf.buf);

	// usually we need send after recv
	connectiodata->ResetIOBuf();
	PostSend(connecthandle, connectiodata);

	// handle recv signal complete, post new one
	//MUST not force to release the periodata which hold overlapped struct
	//new one periodata
	PerIOData *newconnectiodata = connecthandle->GetNewIOData();
	PostRecv(connecthandle, newconnectiodata);

	return TRUE;
}

//post send
void IOCPServer::PostSend(SocketHandle* connecthandle, PerIOData* connectiodata)
{
	if(NULL == connecthandle || NULL == connectiodata)
	{
		AMS_DBUG("you just transfer a NULL pointer\n");
		return;
	}

	// do some clean
	// make operationtype signal be SEND_POSTED
	DWORD flags = 0;
	DWORD sendbytes = 0;
	connectiodata->ResetOverLapped();
	connectiodata->operationtype = PerIOData::SEND_POSTED;

	//build the message
	strcat_s(connectiodata->databuf.buf, connectiodata->databuf.len, "IOCP test");
	*(connectiodata->databuf.buf + strlen(connectiodata->databuf.buf)) = 0;

	//send the message
	int nbytes = WSASend(connecthandle->ssocket, &connectiodata->databuf, 1,
		&sendbytes, flags, &connectiodata->overlapped, NULL );
	if ((SOCKET_ERROR == nbytes) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		AMS_DBUG("WSASend() failed: %d.\n", WSAGetLastError());
		return;
	}

	AMS_DBUG("send to %s:%d, message: \n%s\n", 
		inet_ntoa(connecthandle->ssocketaddr.sin_addr), 
		ntohs(connecthandle->ssocketaddr.sin_port),connectiodata->databuf.buf );

	//just do it in the end of one io here
	//this is dangerous in somewhere else
	connecthandle->RemoveIOData(connectiodata);

	return;
}