#include "stdafx.h"
#include "PerIOData.h"

// default buffer size
#define BUFFERSIZE 1500

//new buffer and Initialize
PerIOData::PerIOData(void):
	operationtype(NULL_POSTED),
	acceptsocket(INVALID_SOCKET)
{
	ZeroMemory(&overlapped, sizeof(overlapped));  
	databuf.buf = new char[BUFFERSIZE];
	databuf.len = BUFFERSIZE;
	ZeroMemory(databuf.buf, databuf.len);
	bytestransfer = 0;
}

//delete the buffer
PerIOData::~PerIOData(void)
{
	if(NULL != databuf.buf)
	{
		delete [] databuf.buf;
		databuf.buf = NULL;
	}
}

//reset buffer and overlapped of io
void PerIOData::ResetIOBuf(void)
{
	databuf.len = BUFFERSIZE;
	ZeroMemory(databuf.buf, databuf.len);
	bytestransfer = 0;
}

void PerIOData::ResetOverLapped(void)
{
	ZeroMemory(&overlapped, sizeof(WSAOVERLAPPED));
}