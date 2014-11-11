#pragma once

#include "stdafx.h"

#include "IOCPServer.h"
#include "HttpData.h"

class HttpServer : public IOCPServer
{
public:
	HttpServer(u_short useport);
	virtual ~HttpServer(void);

protected:
	//maybe Derive class need do somthing in DoAcceptEx, openssl need this
	//you can use another protocol to protect your data
	//same to DeriveCheckRecv and DeriveBuildSend
	virtual BOOL DeriveAcceptEx(PerIOData *listeniodata);

	//maybe Derive class need do somthing in CheckRecv, openssl need this
	virtual BOOL DeriveCheckRecv(DWORD bytestransfer, LPWSABUF recvbuf);

private:
	BOOL CheckRecvBufEnd(DWORD bytestransfer, LPWSABUF recvbuf);

protected:
	BOOL DoRecv(SocketHandle* connecthandle, PerIOData* connectiodata);


private:
	//check recv message, need Derive class to do its thing
	BOOL BuildSend(LPWSABUF sendbuf, HttpData* httpdata);

protected:
	//maybe Derive class need do somthing in CheckRecv, openssl need this
	virtual BOOL DeriveBuildSend(LPWSABUF sendbuf);
	BOOL PostSend(SocketHandle* connecthandle, PerIOData* connectiodata, HttpData* httpdata);
};