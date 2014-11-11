#pragma once

#include "stdafx.h"

#include <list>
using namespace std;

#include "PerIOData.h"

class SocketHandle
{
public:
	SocketHandle(void);
	virtual ~SocketHandle(void);

public:
	//the socket and its addr the handle hold
	SOCKET ssocket;
	SOCKADDR_IN ssocketaddr;

public:
	PerIOData* GetNewIOData(void);
	
	// check the cpp source file for the reason
	void RemoveIOData(PerIOData* speriodata);
};