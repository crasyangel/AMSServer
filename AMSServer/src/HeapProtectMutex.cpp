#include "stdafx.h"
#include "HeapProtectMutex.h"

HeapProtectMutex *HeapProtectMutex::mutexsingletoninstance = NULL;

HeapProtectMutex::HeapProtectMutex()
{
	//Initialize the singletoninstance
	if ( mutexsingletoninstance != NULL )
	{
		AMS_DBUG("Singleton pointer is not NULL!  There are multiple FileLog!" 
			"Leaving the singleton pointer alone...\n");
	}
	else
	{
		AMS_DBUG("Setting up singleton pointer.\n");
		mutexsingletoninstance = this;
	}

	heapprotectmutex = CreateMutex(NULL,FALSE,NULL);
}

HeapProtectMutex::~HeapProtectMutex()
{
	CloseHandle(heapprotectmutex);

	// Clear the singleton pointer.
	if ( mutexsingletoninstance == this )
	{
		AMS_DBUG("Clearing the singleton pointer.\n");
		mutexsingletoninstance = NULL;
	}
	else
	{
		AMS_DBUG("I'm not the singleton instance!  Leaving the singleton pointer alone...\n");
	}
}

//SingletonInstance Design Pattern implement
HeapProtectMutex *HeapProtectMutex::GetSingletonInstance(void)
{
	if ( mutexsingletoninstance == NULL )
	{
		AMS_DBUG("FileLog::GetSingletonInstance:  WARNING - the singleton is NULL"
			", and someone is accessing it!\n");
	}

	return mutexsingletoninstance;
}

void HeapProtectMutex::WaitHeapProtectMutex()
{
	WaitForSingleObject(heapprotectmutex,INFINITE);
}

void HeapProtectMutex::ReleaseHeapProtectMutex()
{
	ReleaseMutex(heapprotectmutex);
}