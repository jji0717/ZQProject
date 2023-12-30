// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This was copied from enterprise domain object sys, edos's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : dll entry
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/build_win32/edosdll.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/build_win32/edosdll.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 3     4/14/05 10:13a Hui.shao
// 
// 2     4/12/05 5:46p Hui.shao
// ============================================================================================

#include "EntryDB.h"

BOOL APIENTRY DllMain(HANDLE  hModule, DWORD dwReason, LPVOID lpreserved)
{
   return TRUE ;
}

/*
extern "C" __declspec(dllexport)
void CALLBACK sysinfo(HWND hwnd, HINSTANCE hinst, LPTSTR lpCmdLine, int nCmdShow)
{
	edos::gblSysInfo.refreshMemInfo();
	edos::gblSysInfo.refreshDisplayInfo();
//	edos::gblSysInfo.refreshProcInfo();

	edos::gblSysInfo.openRoot();

	char buf[1024];

	::GetTempPath(1000, buf);
	strcat(buf, "_sysinfo.xml");

	edos::gblSysInfo.xexport(buf);
	::ShellExecute(hwnd, "open", buf, NULL, NULL, nCmdShow);
}

*/