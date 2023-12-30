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


#include "Log.h"
#include "VstrmLoader.h"



#define MOLOG				(glog)
#define VsmLoader			"VsmLoader"

using namespace ZQ::common;


VstrmLoader::VstrmLoader()
{
	fpVstrmDeleteFile = NULL;
	fpVstrmGetLastError = NULL;
	fpVstrmFindFirstFileEx = NULL;
	fpVstrmClassOpenEx = NULL;
	fpVstrmFindClose = NULL;
	fpVstrmClassReleaseAllBandwidth = NULL;
	fpVstrmClassCloseEx = NULL;
	fpVsGetFileData = NULL;
	fpVsRead = NULL;
	fpVsWrite = NULL;
	fpVsOpen = NULL;
	fpVsOpenEx = NULL;
	fpVsClose = NULL;
	fpVsSeek = NULL;
	fpVsIoControl = NULL;
	fpVsSetEndOfFile = NULL;
	fpVstrmClassReserveBandwidth = NULL;
	fpVstrmClassGetErrorText = NULL;
	fpVstrmClassSetSessionAttributesEx = NULL;
	fpVstrmClassGetStorageData = NULL;
	fpVstrmClassReleaseBandwidth = NULL;
	fpVstrmFindNextFileEx = NULL;

//	_pLog = &ZQ::common::NullLogger;
	
	_hVsIoLib = NULL;
	_hVstrmDLL = NULL;
}

VstrmLoader::~VstrmLoader()
{
	close();
}

bool VstrmLoader::load()
{
	if (!loadVsIoLib())
		return false;

	if (!loadVstrmDll())
		return false;

	return true;
}

void VstrmLoader::close()
{
	if (_hVsIoLib)
	{
		FreeLibrary(_hVsIoLib);
		_hVsIoLib = NULL;
	}

	if (_hVstrmDLL)
	{
		FreeLibrary(_hVstrmDLL);
		_hVstrmDLL = NULL;
	}
}

bool VstrmLoader::loadVsIoLib()
{
	if ((_hVsIoLib = LoadLibrary("VsIoLib.dll")) == NULL)
	{	
		MOLOG(Log::L_ERROR, CLOGFMT(VsmLoader, "LoadLibrary() VsIoLib.dll failed, error:[%d]"),GetLastError());
		return false;
	}
	
	NameAddrs vsIoAddrs[] = { 
		{"VsGetFileData",	(FARPROC *)&fpVsGetFileData},
		{"VsRead",			(FARPROC *)&fpVsRead },
		{"VsWrite",			(FARPROC *)&fpVsWrite},
		{"VsOpen",			(FARPROC *)&fpVsOpen},
		{"VsOpenEx",		(FARPROC *)&fpVsOpenEx},
		{"VsClose",			(FARPROC *)&fpVsClose},
		{"VsSeek",			(FARPROC *)&fpVsSeek},
		{"VsIoControl",		(FARPROC *)&fpVsIoControl},
		{"VsSetEndOfFile",	(FARPROC *)&fpVsSetEndOfFile},
		{0,0} 
	}; 

	for (int i = 0; vsIoAddrs[i].pAddr != NULL; i++)
	{
		FARPROC farProc = GetProcAddress(_hVsIoLib, vsIoAddrs[i].pName);
		if (!farProc)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(VsmLoader, "Loading VsIoLib routine %s failed, error:[%d]"),vsIoAddrs[i].pName,GetLastError());
			return false;
		}

		*vsIoAddrs[i].pAddr = farProc;
	}

	MOLOG(Log::L_INFO, CLOGFMT(VsmLoader, "VsIoLib.dll loaded successful"));
	return true;
}

bool VstrmLoader::loadVstrmDll()
{
	if ((_hVstrmDLL = LoadLibrary("VstrmDLLEx.dll")) == NULL)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(VsmLoader, "LoadLibrary() VstrmDLLEx.dll failed, error:[%d]"),GetLastError());
		return false;
	}

	NameAddrs vsEntries[] = {
		{"VstrmDeleteFile",				(FARPROC *)&fpVstrmDeleteFile},
		{"VstrmGetLastError",			(FARPROC *)&fpVstrmGetLastError},
		{"VstrmFindFirstFileEx",		(FARPROC *)&fpVstrmFindFirstFileEx},
		{"VstrmFindClose",				(FARPROC *)&fpVstrmFindClose},
		{"VstrmClassOpenEx",			(FARPROC *)&fpVstrmClassOpenEx},
		{"VstrmClassReleaseAllBandwidth", (FARPROC *)&fpVstrmClassReleaseAllBandwidth},
		{"VstrmClassCloseEx",			(FARPROC *)&fpVstrmClassCloseEx},
		{"VstrmClassReserveBandwidth",	(FARPROC *)&fpVstrmClassReserveBandwidth},
		{"VstrmClassGetErrorText",		(FARPROC *)&fpVstrmClassGetErrorText},
		{"VstrmClassSetSessionAttributesEx", (FARPROC *)&fpVstrmClassSetSessionAttributesEx},
		{"VstrmClassGetStorageData",	(FARPROC *)&fpVstrmClassGetStorageData},
		{"VstrmClassReleaseBandwidth",	(FARPROC *)&fpVstrmClassReleaseBandwidth},
		{"VstrmFindNextFileEx",	(FARPROC *)&fpVstrmFindNextFileEx},
		{"VstrmMoveFile",				(FARPROC *)&fpVstrmMoveFile},
		{0,0} 
	};

	for (int i = 0; vsEntries[i].pAddr != NULL; i++)
	{
		*vsEntries[i].pAddr = GetProcAddress(_hVstrmDLL, vsEntries[i].pName);
		if (*vsEntries[i].pAddr == NULL)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(VsmLoader, "Loading VstrmDLL routine %s failed , error:[%d]"),vsEntries[i].pName,GetLastError());
			return false;
		}
	}

	MOLOG(Log::L_INFO, CLOGFMT(VsmLoader, "VstrmDLLEx.dll loaded successful"));
	return true;
}

void VstrmLoader::setLog( ZQ::common::Log* pLog )
{
	ZQ::common::setGlogger(pLog);
}