// ===========================================================================
// Copyright (c) 2008 by
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
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================

#ifndef ZQTS_CPE_VSTRMDLLLOADER_H
#define ZQTS_CPE_VSTRMDLLLOADER_H


#include "vstrmuser.h"


typedef BOOLEAN (*pVstrmDeleteFile)(HANDLE, LPCTSTR);
typedef DWORD	(*pVstrmGetLastError)(void);
typedef VHANDLE	(*pVstrmFindFirstFileEx)(HANDLE, LPCTSTR, LPDLL_FIND_DATA_LONG);
typedef BOOLEAN	(*pVstrmFindClose)(HANDLE , VHANDLE);
typedef VSTATUS (*pVstrmClassOpenEx)(PHANDLE);
typedef VSTATUS (*pVstrmClassReleaseAllBandwidth)(VHANDLE ,ULONG ,ULONG);
typedef void    (*pVstrmClassCloseEx)(HANDLE);
typedef VSTATUS (*pVsGetFileData)(char *, LARGE_INTEGER *, FILETIME *);
typedef BOOL (*pVsRead)(HANDLE, char *, int, DWORD *, OVERLAPPED *);
typedef BOOL (*pVsWrite)(HANDLE, int, char *, DWORD *, OVERLAPPED *);
typedef VSTATUS	(*pVsOpen)(HANDLE *, char *, DWORD, DWORD, DWORD, char *, OBJECT_ID *);
typedef VSTATUS	(*pVsOpenEx)(HANDLE *, char *, DWORD, DWORD, DWORD, DWORD, char *, OBJECT_ID *);
typedef VSTATUS	(*pVsClose)(HANDLE, OBJECT_ID);
typedef VSTATUS	(*pVsSeek)(HANDLE, OBJECT_ID, LARGE_INTEGER *, int) ;
typedef VSTATUS	(*pVsIoControl)(HANDLE, int, IOCTL_CONTROL_PARMS_LONG *, IOCTL_RESULT *);
typedef VSTATUS (*pVsSetEndOfFile)(HANDLE , OBJECT_ID , LARGE_INTEGER *);
typedef VSTATUS (*pVstrmClassReserveBandwidth)(VHANDLE, PVSTRM_BANDWIDTH_RESERVE_BLOCK, PULONG64);
typedef VSTATUS (*pVstrmClassGetErrorText)(HANDLE, VSTATUS, PCHAR, ULONG);
typedef VSTATUS (*pVstrmClassSetSessionAttributesEx)(HANDLE ,SESSION_ID ,PATTRIBUTE_ARRAY);
typedef VSTATUS (*pVstrmClassGetStorageData)(HANDLE, PLARGE_INTEGER, PLARGE_INTEGER);
typedef VSTATUS (*pVstrmClassReleaseBandwidth)(VHANDLE,ULONG64);
typedef BOOLEAN	(*pVstrmFindNextFileEx)(HANDLE, VHANDLE, LPDLL_FIND_DATA_LONG);
typedef BOOLEAN	(*pVstrmMoveFile)(HANDLE vstrmClassHandle, LPCTSTR lpszOld, LPCTSTR lpszNew);

namespace ZQ
{
	namespace common
	{
		class Log;
	}
}

class VstrmLoader
{
public:
	VstrmLoader();
	~VstrmLoader();
	
	void setLog(ZQ::common::Log* pLog);

	bool load();

	void close();

public:
	pVstrmDeleteFile  fpVstrmDeleteFile;
	pVstrmGetLastError fpVstrmGetLastError;
	pVstrmFindFirstFileEx fpVstrmFindFirstFileEx;
	pVstrmClassOpenEx fpVstrmClassOpenEx;
	pVstrmFindClose   fpVstrmFindClose;
	pVstrmClassReleaseAllBandwidth fpVstrmClassReleaseAllBandwidth;
	pVstrmClassCloseEx fpVstrmClassCloseEx;
	pVsGetFileData fpVsGetFileData;
	pVsRead fpVsRead;
	pVsWrite fpVsWrite;
	pVsOpen fpVsOpen;
	pVsOpenEx fpVsOpenEx;
	pVsClose fpVsClose;
	pVsSeek fpVsSeek;
	pVsIoControl fpVsIoControl;
	pVsSetEndOfFile fpVsSetEndOfFile;
	pVstrmClassReserveBandwidth fpVstrmClassReserveBandwidth;
	pVstrmClassGetErrorText fpVstrmClassGetErrorText;
	pVstrmClassSetSessionAttributesEx fpVstrmClassSetSessionAttributesEx;
	pVstrmClassGetStorageData fpVstrmClassGetStorageData;
	pVstrmClassReleaseBandwidth fpVstrmClassReleaseBandwidth;
	pVstrmFindNextFileEx        fpVstrmFindNextFileEx;
	pVstrmMoveFile				fpVstrmMoveFile;

protected:
	bool loadVsIoLib();
	bool loadVstrmDll();

	struct NameAddrs
	{
		char *pName;
		FARPROC *pAddr;
	};

private:
	HINSTANCE	_hVsIoLib;
	HINSTANCE	_hVstrmDLL;

//	ZQ::common::Log* _pLog;

};



#endif