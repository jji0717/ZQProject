   // ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: MiniDump.h,v 1.20 2004/07/26 10:56:35 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : A mini dump for the application crash
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/nPVR/EventCollector/MiniDump.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// ===========================================================================
/**** usage example ****
void IllegalOp1()
{
    _asm {cli};
}

void IllegalOp2()
{
	char* p= NULL;

	p[0] = 0;
	int a = 1;
}

ZQ::common::MiniDump dumper("c:\\temp");

void main()
{
	IllegalOp1();
}
************************/

#ifndef __MINIDUMP_H__
#define __MINIDUMP_H__

#include "ZQ_common_conf.h"
extern "C"
{
#include <dbghelp.h>
}


namespace ZQ {
namespace common {

class ZQ_COMMON_API MiniDump;

// -----------------------------
// class MiniDump
// -----------------------------
class MiniDump
{
public:
	typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
									CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
									CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
									);

	MiniDump(TCHAR* DumpPath =NULL, TCHAR* ModuleName =NULL);
	
	BOOL setDumpPath(TCHAR* DumpPath);
	void enableFullMemoryDump(BOOL bEnable){ _bEnableFullMemoryDump = bEnable;}

	typedef void (WINAPI *ExceptionCallBack)(DWORD ExceptionCode, PVOID ExceptionAddress);
	
	//this callback will be called after the minidump created
	void setExceptionCB(ExceptionCallBack cb);
protected:
	
	static void trace(const TCHAR *fmt, ...);
	static LONG WINAPI _OnUnhandledException( struct _EXCEPTION_POINTERS *pExceptionInfo );
	static TCHAR _moduleName[128], _dumpPath[_MAX_PATH];
	static TCHAR _moduleFullPath[_MAX_PATH];
	static LONG _hOldFilter;
	static BOOL _bEnableFullMemoryDump;
	static ExceptionCallBack		_pExceptionCB;
};

} // namespace common
} // namespace ZQ

#endif // __MINIDUMP_H__