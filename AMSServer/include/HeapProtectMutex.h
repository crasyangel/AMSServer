#pragma once

#include "stdafx.h"


class HeapProtectMutex
{
public:
	HeapProtectMutex(void);
	virtual ~HeapProtectMutex(void);

public:
	//SingletonInstance Design Pattern member
	static HeapProtectMutex *GetSingletonInstance(void);
	void WaitHeapProtectMutex(void);
	void ReleaseHeapProtectMutex(void);

private:
	//SingletonInstance Design Pattern member
	static HeapProtectMutex* mutexsingletoninstance;
	HANDLE heapprotectmutex;
};