#pragma once

#include "stdafx.h"

#pragma comment(lib,"ws2_32.lib")

#include "PerIOData.h"
#include "SocketHandle.h"

#define MAX_THREAD_NUM 8

// this is IOCPServer which use IOCP
// has no GUI, we provide two public function to call
// two public function: start() and stop()
// you can write new interface
class IOCPServer
{
public:
	IOCPServer(u_short useport);
	virtual ~IOCPServer(void);

public:
	bool Start(void);
	void Stop(void);

// self member hold in whole life of the class object
//so we have no need to tranfer these in function parameters
protected:
	u_short serverport;
	HANDLE completeport;
	SocketHandle *listenhandle;
	HANDLE threadhandle[MAX_THREAD_NUM];
	unsigned threadnum;

private:
	//SingletonInstance Design Pattern member
	static IOCPServer* serversingletoninstance;

public:
	//SingletonInstance Design Pattern member
	static IOCPServer *GetSingletonInstance(void);

private:
	LPFN_ACCEPTEX lpfnacceptex;
	LPFN_GETACCEPTEXSOCKADDRS lpfngetacceptexsockaddrs;

//Initialize functions, check the cpp file 
//Return value for BOOL function:
//		true, if succeed;
//		false, if failed.
//
private:
	BOOL InitializeIOCP(void);
	BOOL InitializeSocket(void);
	BOOL GetAcceptExFuction(void);


private:
	// CreateThread only can use static function in class
	// so must transfer the 'this' pointer to lpParam to use non-static function member
	static unsigned WINAPI WorkerThread(LPVOID lpParam);


// the six function member below has same parameter format and return value
//
//Parameter:
//		connecthandle - the client socket and addr storage struct for the operation
//		listeniodata/connectiodata - the io data for the operation
//			this struct PerIOData MUST begin with WSAOVERLAPPED/OVERLAPPED struct,
//			To convert the PerIOData pointer to OVERLAPPED pointer.
//			actually, the first address should be the same, so they are compatible
//
//Return value:
//		true, if succeed;
//		false, if failed.
//

protected:
	// post function for AcceptEx
	// no need for do function, ACCEPT_POSTED signal corresponding to listen socket
	// listeniodata is hold in whole life of master thread
	// MUST not release in worker thread
	void PostAcceptEx(PerIOData *listeniodata);
	BOOL DoAcceptEx(PerIOData *listeniodata);

	//maybe Derive class need do somthing in DoAcceptEx, openssl need this
	virtual BOOL DeriveAcceptEx(PerIOData *listeniodata);

protected:
	// post and do function for recv/send 
	// connectiodata is hold in worker thread
	void PostRecv(SocketHandle *connecthandle, PerIOData *connectiodata);
	virtual BOOL DoRecv(SocketHandle *connecthandle, PerIOData *connectiodata);

protected:
	//PostSend
	void PostSend(SocketHandle *connecthandle, PerIOData *connectiodata);
};

