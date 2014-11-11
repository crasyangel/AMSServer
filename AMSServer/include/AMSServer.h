#pragma once

#include "stdafx.h"


#include "FileLog.h"

#define USE_MFC_GUI

#ifdef USE_MFC_GUI
#include "MFCAPP.h"
#endif

//this value should use most of the time when you do not debug the app
//#define CONSOLE_HIDE