//here is just a simple server, I don't want use Design Pattern
// if you need different implement
//you should think about Design Pattern, include IOCPServer
//first of all first, you MUST define how the interface works

#include "stdafx.h"

#include "AMSServer.h"
#include "HeapProtectMutex.h"

//the port
#define SERVERPORT 5863

#define LOGFILENAME _T("log.txt")

//because we must use one temporary memory
//use func instead of macro
#define ONELOGLEN 1000

void AMS_DBUG_LOG(const CHAR* format, ...)																
{
	va_list args;
	va_start(args, format);

	CHAR templog[ONELOGLEN] = {0};	
	size_t printbytes = 1 + _vscprintf(format, args);
	vsprintf_s(templog, printbytes, format, args);
	FileLog::GetSingletonInstance()->WirteLogFile(templog);	

	va_end(args);
}

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef CONSOLE_HIDE	
	ShowWindow( GetConsoleWindow(), SW_HIDE );
#endif

	FileLog* filelog = new FileLog();
	FileLog::GetSingletonInstance()->OpenLogFile(LOGFILENAME);
	
	HeapProtectMutex* heapmutex = new HeapProtectMutex();

	//here perhaps not use a new base class
	//this will cause multi-inherit, and we don't need it for now
	//multi-inherit may cause ambiguity and confusion of memory distribution
	//if you do not use many gui, you shouldn't use the multi-inherit
	//if you want use another gui method, please change in the macro USE_MFC_GUI, and the header
	//and wirte you login gui refer to MFCLoginGUI, and main gui refer to MFCMainGUI

#ifdef USE_MFC_GUI
	// The one and only MFCAPP object
	MFCAPP theApp(SERVERPORT);

	// initialize MFC and print and error on failure	
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		AMS_DBUG("Fatal Error: MFC initialization failed\n");
		return 1;
	}
	theApp.InitApplication();
	theApp.InitInstance();
	theApp.Run();

	AfxWinTerm();
#endif	

	delete heapmutex;
	delete filelog;
	return 0;
}
