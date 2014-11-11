// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <cstdio>
#include <cassert>
#include <tchar.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include <fstream>

#include <afxwin.h>

#include <winsock2.h>
#include <MSWSock.h>

#define CONSOLE_PRINT_OUT

#ifdef CONSOLE_PRINT_OUT

#define AMS_DBUG(format, ...)												\
	{																		\
		printf("%s,%d: " format, __FUNCTION__, __LINE__, ##__VA_ARGS__);	\
		fflush(stdout);														\
		AMS_DBUG_LOG(format, ##__VA_ARGS__);								\
	}

#else

	#define AMS_DBUG(format, ...)											\
	{																		\
		AMS_DBUG_LOG(format, ##__VA_ARGS__);								\
	}
#endif

//because we must use one temporary memory
//use func instead of macro
extern void __cdecl AMS_DBUG_LOG(const char* format, ...);