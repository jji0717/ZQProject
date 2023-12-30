// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#pragma warning(disable:4312)
#include <streams.h>

#include <comdef.h>

#include <Winsock2.h>

#include <TCHAR.h>

#include "interfaceDefination.h"

// DShow interfaces used in smart pointers
_COM_SMARTPTR_TYPEDEF(IMemAllocator, IID_IMemAllocator);
_COM_SMARTPTR_TYPEDEF(IAsyncReader, IID_IAsyncReader);
_COM_SMARTPTR_TYPEDEF(ICatalogRountine, IID_ICatalogRountine);
_COM_SMARTPTR_TYPEDEF(IMediaSample, IID_IMediaSample);

// 不要使用这个函数输出 log
void LogMyEvent(int errorLevel,int errorcode,char* errorStr);

#ifndef _NO_FIX_LOG
#include "fltinit.h"
void glog(ISvcLog::LogLevel level, const char* fmt, ...);
#else
#define glog
#endif

// TODO: reference additional headers your program requires here
