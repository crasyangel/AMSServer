#pragma once

#include "stdafx.h"

// per io data for socket handle
// one socket can has more than one io
//WSAOVERLAPPED struct must be the first in memory of this class
//virtual func seems to be fine with this
class PerIOData
{
public:
	PerIOData(void);
	virtual ~PerIOData(void);

public:
	// overlapped struct, must be in the first place of class memory
	WSAOVERLAPPED overlapped;

	//just for ACCEPT_POSTED signal
	SOCKET acceptsocket;

	//the buffer of me
	WSABUF databuf;

	//the bytes has sent
	DWORD bytestransfer;

	//the signal belong to me
	DWORD operationtype;

public:
	void ResetIOBuf(void);
	void ResetOverLapped(void);

// this four signal is the public signal for other class, IOCPServer etc..
//		ACCEPT_POSTED - signal for accept
//		RECV_POSTED   - signal for recv
//		SEND_POSTED   - signal for send
//		NULL_POSTED   - terminal signal, no meaning
//
public:
	typedef enum  
	{  
		ACCEPT_POSTED,
		RECV_POSTED,
		SEND_POSTED,
		NULL_POSTED
	}OPERATION_TYPE;
};