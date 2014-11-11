
#include "stdafx.h"
#include "FileLog.h"

//SingletonInstance Design Pattern member Initialize
FileLog *FileLog::logsingletoninstance = NULL;

FileLog::FileLog() :
	logfd(INVALID_HANDLE_VALUE),
	logmutex(INVALID_HANDLE_VALUE)
{
	//Initialize the singletoninstance
	if ( logsingletoninstance != NULL )
	{
		printf("Singleton pointer is not NULL!  There are multiple FileLog!" 
			"Leaving the singleton pointer alone...\n");
	}
	else
	{
		printf("Setting up singleton pointer.\n");
		logsingletoninstance = this;
	}
	
	//create log file write mutex
	logmutex = CreateMutex(NULL, FALSE, NULL);
	if (NULL == logmutex) 
	{
		printf("Create/Open Mutex error: %d\n", GetLastError());
		assert(0);
	}
}

FileLog::~FileLog()
{
	//recycle my own member
	CloseHandle(logmutex);
	CloseHandle(logfd);

	// Clear the singleton pointer.
	if ( logsingletoninstance == this )
	{
		printf("Clearing the singleton pointer.\n");
		logsingletoninstance = NULL;
	}
	else
	{
		printf("I'm not the singleton instance!  Leaving the singleton pointer alone...\n");
	}
}

//SingletonInstance Design Pattern implement
FileLog *FileLog::GetSingletonInstance(void)
{
	if ( logsingletoninstance == NULL )
	{
		printf("FileLog::GetSingletonInstance:  WARNING - the singleton is NULL"
			", and someone is accessing it!\n");
	}

	return logsingletoninstance;
}

BOOL FileLog::OpenLogFile(WCHAR* logpath)
{
	WCHAR filename[MAX_PATH] = {0};
	WCHAR tmpfilename[MAX_PATH] = {0};

	//get the path of EXE file 
	if(0 == GetModuleFileName(NULL, tmpfilename, MAX_PATH))
	{
		printf("GetModuleFileName failed, error: %d\n", GetLastError());
		return FALSE;
	}

	//remove the file name in the end of path 
	if(0 == PathRemoveFileSpec(tmpfilename))
	{
		printf("PathRemoveFileSpec failed, error: %d\n", GetLastError());
		return FALSE;
	}

	//make the new path include log file name
	if(NULL == PathCombine(filename, tmpfilename, logpath))
	{
		printf("PathCombine failed, error: %d\n", GetLastError());
		return FALSE;
	}

	//create or open log file
	logfd = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);

	if (INVALID_HANDLE_VALUE == logfd) 
	{
		printf("Creat/Open file error: %d\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

void FileLog::WirteLogFile(CHAR *logstring)
{
	DWORD len = 0;

	//wait mutex for log file
	WaitForSingleObject(logmutex,INFINITE);

	//seek to the end of file
	SetFilePointer(logfd,0,NULL,FILE_END);

	//write string to the end of file
	if (FALSE == WriteFile(logfd, logstring, strlen(logstring), &len, NULL)) 
	{
		printf("Fail to write file, error: %d\n", GetLastError());
	}

	//release mutex for log file
	ReleaseMutex(logmutex);
}