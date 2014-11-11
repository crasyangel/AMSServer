#include "stdafx.h"
#include "SocketHandle.h"
#include "HeapProtectMutex.h"


SocketHandle::SocketHandle(void)
{
	ssocket = INVALID_SOCKET;
	ZeroMemory(&ssocketaddr, sizeof(ssocketaddr)); 
}

SocketHandle::~SocketHandle(void)
{
	//close the socket
	if( ssocket!=INVALID_SOCKET )
	{
		closesocket(ssocket);
		ssocket = INVALID_SOCKET;
	}
}

// new iodata and add it in list
//return new io which will be used
PerIOData* SocketHandle::GetNewIOData(void)
{
	PerIOData* speriodata = new PerIOData();
	return speriodata;
}

//delete the io transfered and erase it form list
//MUST not force to release the periodata which hold overlapped struct
//let the deconstruct func do the recycle
//we need it just in the end of one io's life.
//just for case, some ugly client keep sending message to me with no interval

void SocketHandle::RemoveIOData(PerIOData* speriodata)
{
	if(NULL == speriodata)
	{
		AMS_DBUG("you just transfer a NULL pointer\n");
		return;
	}

	HeapProtectMutex::GetSingletonInstance()->WaitHeapProtectMutex();
	delete speriodata;
	speriodata = NULL;
	HeapProtectMutex::GetSingletonInstance()->ReleaseHeapProtectMutex();

	AMS_DBUG("the io has been removed\n");
}
