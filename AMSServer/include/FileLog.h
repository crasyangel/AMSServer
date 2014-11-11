#pragma once

#include "stdafx.h"


#include <Shlwapi.h>
#pragma comment(lib,"shlwapi")


class FileLog
{
public:
	FileLog(void);
	virtual ~FileLog(void);

public:
	//SingletonInstance Design Pattern member
	static FileLog *GetSingletonInstance(void);

public:
	//open interface for log file
	BOOL OpenLogFile(WCHAR* logpath);

	//write interface for log file
	void WirteLogFile(CHAR *logstring);
	
private:
	//mutex for log file
	HANDLE logmutex;

	//file descriptor for log
	HANDLE logfd;

private:
	//SingletonInstance Design Pattern member
	static FileLog* logsingletoninstance;
};
