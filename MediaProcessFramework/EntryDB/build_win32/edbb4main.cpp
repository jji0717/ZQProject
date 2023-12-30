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
// Author: Daniel Wang
// Desc  : entry database berkeley database 4 edition
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/build_win32/edbb4main.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/build_win32/edbb4main.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 3     4/13/05 6:51p Hui.shao
// changed namespace
// 
// 2     4/12/05 6:44p Hui.shao
// ============================================================================================


#include "..\EDBb4.h"
#include "..\EDBImpl.h"
#include <string>

BOOL APIENTRY DllMain(HANDLE  hModule, DWORD dwReason, LPVOID lpreserved)
{
   return TRUE ;
}

EDBIMPL_DECLARE_BEGIN
	EDBIMPL_DECLARE_DB("EDB-BerkeryDB 4", EBDB4URLPROT "://", EBDB4URLPROT "://<server>/<path>", "Entry Database, Berkeley Database 4 Edition")
EDBIMPL_DECLARE_END

EDBIMPL_EXPORT_BEGIN
	EDBIMPL_EXPORT_DB(EDBb4)
EDBIMPL_EXPORT_END

#define ABOUT_HEAD \
	"Entry Database, Berkeley Database 4 Edition\n" \
	"Copyright (C) 2005 Xiao Bai\n" \
	"All Rights Reserved, Xiao Bai\n" \
	"\n"

extern "C" __declspec(dllexport)
void CALLBACK about(HWND hwnd, HINSTANCE hinst, LPTSTR lpCmdLine, int nCmdShow)
{
	std::string about_str(ABOUT_HEAD);

	about_str +="Supported URL:\n";

	int i=0;
	const char* urlhlp;
	do
	{
		urlhlp = URLHelp(i++);
		if (urlhlp !=NULL)
			about_str += std::string("  ") + urlhlp +"\n";

	} while (urlhlp !=NULL);

	::MessageBox(hwnd, about_str.c_str(), "Entry Databse", MB_OK);
}

