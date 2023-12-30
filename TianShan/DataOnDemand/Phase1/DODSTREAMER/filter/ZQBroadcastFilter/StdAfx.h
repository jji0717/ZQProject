// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__3B92D03B_B483_479F_BB34_E6DDFA9453FA__INCLUDED_)
#define AFX_STDAFX_H__3B92D03B_B483_479F_BB34_E6DDFA9453FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*

Revision History:

Rev			   Date        Who			Description
----		---------    ------		----------------------------------
V1,0,1,0	2006.07.28	zhenan_ji   1.Add getlasterror into channel.cpp ,add "Receive log" in Pin:receive()
V1,0,1,2	2006.08.15	zhenan_ji   1.fix bug :DataWrapper give two flag = 0 continue;led to read and write file competition.
										Operation: add "fileChangeFlag = false" in CChannelManager::GetFileName().

V1,0,1,3	2006.09.14	zhenan_ji   1.fix bug : Xiamen occurs the below log:
										ZQBroadcastFilter.ax(tid e28)  9460045 : 09-13 22:58:45 133 rcPID=141,len=188,flag=1 
										ZQBroadcastFilter.ax(tid 14cc) 9460056 : 09-13 22:58:45 148 File Changed,file=PID141cur errorcode=32
										ZQBroadcastFilter.ax(tid e28)  9460060 : 09-13 22:58:45 148 Receive data 
										ZQBroadcastFilter.ax(tid e28)  9460060 : 09-13 22:58:45 148 rcPID=141,len=188,flag=1 this channel has error.
									Operation: 
										1,Add lock for CChannelManager:ChangeFileFlag();
										2,Modified all m_fileHandle = NULL to "INVALID_HANDLE_VALUE".



















*/
// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
//#include <winsock2.h>
#include "stdio.h"
#include "string.h"
#include <streams.h>
#include <vector>
#define PATPMTBUFFERLEN (188*3)
#define PACKET_SIZE		188
//
#define BUFTYPEBUFFERLEN (188*256)



//void LogToFile(int nErrorLevel, char* szMsg);
void LogToSystemEvent(int nErrorLevel,char* szMsg);
// 不要使用这个函数输出 log
void LogMyEvent(int errorLevel,int errorcode,char* errorStr);

#ifndef NEED_EVENTLOG
#define NEED_EVENTLOG
#endif

#ifndef _NO_FIX_LOG
#include "fltinit.h"
void glog(ISvcLog::LogLevel level, const char* fmt, ...);
#else
#define glog
#endif

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__3B92D03B_B483_479F_BB34_E6DDFA9453FA__INCLUDED_)
