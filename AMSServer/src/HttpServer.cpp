#include "stdafx.h"

#include "HttpServer.h"

#define PAYLOAD_BUFFERSIZE 256
#define HTTP_GET_LENGTH 256

HttpServer::HttpServer(u_short useport):
	IOCPServer(useport)
{

}

HttpServer::~HttpServer()
{

}

BOOL HttpServer::DeriveAcceptEx(PerIOData *listeniodata)
{
	AMS_DBUG("do nothing in base class\n");
	return TRUE;
}

BOOL HttpServer::DeriveCheckRecv(DWORD bytestransfer, LPWSABUF recvbuf)
{
	AMS_DBUG("do nothing in base class\n");
	return TRUE;
}

BOOL HttpServer::CheckRecvBufEnd(DWORD bytestransfer, LPWSABUF recvbuf)
{
	//let derived class do its thing
	if(FALSE == DeriveCheckRecv(bytestransfer, recvbuf))
	{
		AMS_DBUG("DeriveCheckRecv failed: %d\n", WSAGetLastError());
		return FALSE;
	}

	char tempbuf[5] = {0};
	strncpy_s(tempbuf, 5, recvbuf->buf + bytestransfer - 4, 4);
	AMS_DBUG("receive message end is %s\n", tempbuf);

	if(0 != strcmp(tempbuf, "\r\n\r\n"))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL HttpServer::DoRecv(SocketHandle* connecthandle, PerIOData* connectiodata)
{
	if(NULL == connecthandle || NULL == connectiodata)
	{
		AMS_DBUG("you just transfer a NULL pointer\n");
		return FALSE;
	}

	//process the massage
	connectiodata->bytestransfer = strlen(connectiodata->databuf.buf);
	AMS_DBUG("receive message len is %d.\n", connectiodata->bytestransfer);

	if(TRUE == CheckRecvBufEnd(connectiodata->bytestransfer, &connectiodata->databuf))
	{
		AMS_DBUG("receive from %s:%d, message:\n%s\n", 
			inet_ntoa(connecthandle->ssocketaddr.sin_addr), 
			ntohs(connecthandle->ssocketaddr.sin_port),connectiodata->databuf.buf);

		//parse the "GET" string
		if(NULL == strstr(connectiodata->databuf.buf, "GET"))
		{
			AMS_DBUG("the client did not send the \"GET\" string, wrong use in http\n");
			return FALSE;
		}

		CHAR getbuf[HTTP_GET_LENGTH] = {0};
		sscanf_s(connectiodata->databuf.buf, "%*[^?]?%s", getbuf);
		AMS_DBUG("client send \"GET\" context is \n%s\n", getbuf);

		//new HttpData, transfer the getbuf
		HttpData* httpdata = new HttpData(getbuf);

		// usually we need send after recv
		connectiodata->ResetIOBuf();
		if(FALSE == PostSend(connecthandle, connectiodata, httpdata))
		{
			AMS_DBUG("PostSend failed to %s:%d\n", inet_ntoa(connecthandle->ssocketaddr.sin_addr), 
				ntohs(connecthandle->ssocketaddr.sin_port));
		}
	}
	else
	{
		AMS_DBUG("receive message incomplete\n");
		PostRecv(connecthandle, connectiodata);
	}

	// handle recv signal complete, post new one
	//MUST not force to release the periodata which hold overlapped struct
	//new one periodata
	PerIOData *newconnectiodata = connecthandle->GetNewIOData();
	PostRecv(connecthandle, newconnectiodata);

	return TRUE;
}

BOOL HttpServer::DeriveBuildSend(LPWSABUF sendbuf)
{
	AMS_DBUG("do nothing in base class\n");
	return TRUE;
}


BOOL HttpServer::BuildSend(LPWSABUF sendbuf, HttpData* httpdata)
{
	strcat_s(sendbuf->buf, sendbuf->len, "HTTP/1.1 200 OK");
	strcat_s(sendbuf->buf, sendbuf->len, "\r\n");

	strcat_s(sendbuf->buf, sendbuf->len, "Content-Type: text/html");
	strcat_s(sendbuf->buf, sendbuf->len, "\r\n");

	if(FALSE == httpdata->HandleHttpData())
	{
		AMS_DBUG("failed in httpdata->HandleHttpData\n");
		return FALSE;
	}

	LPSTR payloadbuf = httpdata->GetPayLoadBuf();

	CHAR tmpBuf[32] = {0};
	sprintf_s(tmpBuf, 32, "Content-Length: %d", strlen(payloadbuf));
	strcat_s(sendbuf->buf, sendbuf->len, tmpBuf);
	strcat_s(sendbuf->buf, sendbuf->len, "\r\n\r\n");

	strcat_s(sendbuf->buf, sendbuf->len, payloadbuf);

	//let derived class do its thing
	if(FALSE == DeriveBuildSend(sendbuf))
	{
		AMS_DBUG("failed in httpdata->HandleHttpData\n");
		return FALSE;
	}
	return TRUE;
}

//post send
BOOL HttpServer::PostSend(SocketHandle* connecthandle, PerIOData* connectiodata, HttpData* httpdata)
{
	if(NULL == connecthandle || NULL == connectiodata)
	{
		AMS_DBUG("you just transfer a NULL pointer\n");
		return FALSE;
	}

	// do some clean
	// make operationtype signal be SEND_POSTED
	DWORD flags = 0;
	DWORD sendbytes = 0;
	connectiodata->ResetOverLapped();
	connectiodata->operationtype = PerIOData::SEND_POSTED;

	//build the message
	if(FALSE == BuildSend(&connectiodata->databuf, httpdata))
	{
		AMS_DBUG("failed in BuildSend\n");
		delete httpdata;
		AMS_DBUG("httpdata has been deleted\n");
		return FALSE;
	}
	*(connectiodata->databuf.buf + strlen(connectiodata->databuf.buf)) = 0;

	//recycle the memory once finish
	delete httpdata;
	AMS_DBUG("httpdata has been deleted\n");

	//send the message
	int nbytes = WSASend(connecthandle->ssocket, &connectiodata->databuf, 1,
		&sendbytes, flags, &connectiodata->overlapped, NULL );
	if ((SOCKET_ERROR == nbytes) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		AMS_DBUG("WSASend() failed: %d.\n", WSAGetLastError());
		return FALSE;
	}

	AMS_DBUG("send to %s:%d, message: \n%s\n", 
		inet_ntoa(connecthandle->ssocketaddr.sin_addr), 
		ntohs(connecthandle->ssocketaddr.sin_port),connectiodata->databuf.buf );

	//just do it in the end of one io here
	//this is dangerous in somewhere else
	connecthandle->RemoveIOData(connectiodata);

	return TRUE;
}