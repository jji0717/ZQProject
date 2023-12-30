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
// Name  : DBSAdiMan.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-11-21
// Desc  : DBSync Add-in Manager Implementation
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/DBSync/DBSAdiMan.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 4     06-02-17 11:43 Bernie.zhao
// before performance tunning
// 
// 3     05-12-28 14:51 Bernie.zhao
// 
// 2     05-12-27 15:21 Bernie.zhao
// 
// 1     05-11-24 16:53 Bernie.zhao
// added for DBSync Add-in
// ===========================================================================

#include <string>
#include "stdafx.h"
#include "sclog.h"
#include "DBSyncServ.h"
#include "DBSAdiMan.h"


std::vector<std::string> DBSAdiMan::_procNameVector;
extern ZQ::common::Log * gpDbSyncLog;

DBSAdiMan::DBSAdiMan()
:_enabled(false),_hPluginLib(NULL)
{
	// clear procedure validity list
	memset(_procValidList, 0, sizeof(_procValidList));

	// init procNameVecotr
	_procNameVector.clear();
	_procNameVector.resize(EVID_TRGGSTAT+1);
	_procNameVector[0]				="NULL";
	_procNameVector[EVID_INIT]		=DBSACBNAME_INIT;
	_procNameVector[EVID_UNINIT]	=DBSACBNAME_UNINIT;
	_procNameVector[EVID_SYNCBEGIN]	=DBSACBNAME_SYNCBEGIN;
	_procNameVector[EVID_SYNCEND]	=DBSACBNAME_SYNCEND;
	_procNameVector[EVID_TRGGMD]	=DBSACBNAME_TRGGMD;
	_procNameVector[EVID_TRGGMDD]	=DBSACBNAME_TRGGMDD;
	_procNameVector[EVID_TRGGHRCHY]	=DBSACBNAME_TRGGHRCHY;
	_procNameVector[EVID_TRGGCA]	=DBSACBNAME_TRGGCA;
	_procNameVector[EVID_TRGGSTAT]	=DBSACBNAME_TRGGSTAT;

}

void DBSAdiMan::setPluginPath(wchar_t* pluginPath)
{
	// set add-in path
	if(pluginPath)
		wcsncpy(_szPluginPath, pluginPath, MAX_PATH-1);

	// check path
	if( wcscmp(pluginPath, L"")==0 || wcscmp(pluginPath, L"\\")==0 || wcscmp(pluginPath, L"NULL")==0 )
	{
		(*gpDbSyncLog)(ZQ::common::Log::L_NOTICE, "[DBSA]  No DBSync Add-in library specified.");
		_enabled = false;
	}
	else
	{
		// Here we take some risk to change the current working directory to the DLL
		// path.  As far as I know, DBSync does not care about the working directory after
		// it starts.  
		// Heaven or Hell?                 
		//											- Bernie Zhao, 2005-12-27
		std::wstring modulepath = _szPluginPath;
		size_t posSlash = modulepath.find_last_of(L"\\");
		if(posSlash!=std::wstring::npos)
		{
			modulepath = modulepath.substr(0, posSlash);
			::SetCurrentDirectory(modulepath.c_str());
		}
		_hPluginLib = LoadLibrary(_szPluginPath);

		if(_hPluginLib==NULL)
		{
			int errcode = ::GetLastError();
			(*gpDbSyncLog)(ZQ::common::Log::L_WARNING, L"[DBSA]  Can not load DBSync Add-in library (\"%s\"), errorcode=0x%08X.", _szPluginPath, errcode);
			_enabled = false;
		}
		else
		{
			(*gpDbSyncLog)(ZQ::common::Log::L_NOTICE, L"[DBSA]  DBSync Add-in (\"%s\") Loaded successfully.", _szPluginPath);
			_enabled = true;
		}
	}
	
	// check which procedure is exported
	if(_enabled)
	{
		for(int i=EVID_INIT; i<MAX_EVID_NUM; i++)
		{
			FARPROC	pFn = NULL;
		
			pFn = ::GetProcAddress(_hPluginLib, _procNameVector[i].c_str());
			if(pFn!=NULL)
			{
				(*gpDbSyncLog)(ZQ::common::Log::L_INFO, "[DBSA]  DBSync Add-in exported function (\"%s\") - Found.", _procNameVector[i].c_str());
				_procValidList[i]=true;
			}
			else
			{
				(*gpDbSyncLog)(ZQ::common::Log::L_INFO, "[DBSA]  DBSync Add-in exported function (\"%s\") - Not Found.", _procNameVector[i].c_str());
				_procValidList[i]=false;
			}
		}
	}
}

DBSAdiMan::~DBSAdiMan()
{
	if(_hPluginLib)
	{
		::FreeLibrary(_hPluginLib);
	}
}

void DBSAdiMan::Init(DA_dbsyncInfo* pDbsInfo, DA_itvInfo* pItvInfo)
{
	if(_enabled==false || _procValidList[EVID_INIT]==false)
		return;
	DBSAProto_Init pFn = (DBSAProto_Init)::GetProcAddress(_hPluginLib, _procNameVector[EVID_INIT].c_str());
	if(pFn)
	{
		try
		{
			(pFn)(pDbsInfo, pItvInfo);
		}
		catch(...)
		{
			(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, "[DBSA]  Unknown exception caught when calling (%s).", _procNameVector[EVID_INIT].c_str());
		}
	}
}

void DBSAdiMan::Uninit()
{
	if(_enabled==false || _procValidList[EVID_UNINIT]==false)
		return;
	DBSAProto_Uninit pFn = (DBSAProto_Uninit)::GetProcAddress(_hPluginLib, _procNameVector[EVID_UNINIT].c_str());
	if(pFn)
	{
		try
		{
			(pFn)();
		}
		catch(...)
		{
			(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, "[DBSA]  Unknown exception caught when calling (%s).", _procNameVector[EVID_UNINIT].c_str());
		}
	}
}

void DBSAdiMan::SyncBegin()
{
	if(_enabled==false || _procValidList[EVID_SYNCBEGIN]==false)
		return;
	DBSAProto_SyncBein pFn = (DBSAProto_SyncBein)::GetProcAddress(_hPluginLib, _procNameVector[EVID_SYNCBEGIN].c_str());
	if(pFn)
	{
		try
		{
			(pFn)();
		}
		catch(...)
		{
			(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, "[DBSA]  Unknown exception caught when calling (%s).", _procNameVector[EVID_SYNCBEGIN].c_str());
		}
	}
}

void DBSAdiMan::SyncEnd()
{
	if(_enabled==false || _procValidList[EVID_SYNCEND]==false)
		return;
	DBSAProto_SyncEnd pFn = (DBSAProto_SyncEnd)::GetProcAddress(_hPluginLib, _procNameVector[EVID_SYNCEND].c_str());
	if(pFn)
	{
		try
		{
			(pFn)();
		}
		catch(...)
		{
			(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, "[DBSA]  Unknown exception caught when calling (%s).", _procNameVector[EVID_SYNCEND].c_str());
		}
	}
}

void DBSAdiMan::TrggMd(DA_entryDb* pEntryBlock, DWORD dwMdNumber, DA_metaDb* pFirstMdBlock)
{
	if(_enabled==false || _procValidList[EVID_TRGGMD]==false)
		return;
	DBSAProto_TrggMd pFn = (DBSAProto_TrggMd)::GetProcAddress(_hPluginLib, _procNameVector[EVID_TRGGMD].c_str());
	if(pFn)
	{
		try
		{
			(pFn)(pEntryBlock, dwMdNumber, pFirstMdBlock);
		}
		catch(...)
		{
			(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, "[DBSA]  Unknown exception caught when calling (%s).", _procNameVector[EVID_TRGGMD].c_str());
		}
	}
}

void DBSAdiMan::TrggMdd(DA_mddListDb* pMddList)
{
	if(_enabled==false || _procValidList[EVID_TRGGMDD]==false)
		return;
	DBSAProto_TrggMdd pFn = (DBSAProto_TrggMdd)::GetProcAddress(_hPluginLib, _procNameVector[EVID_TRGGMDD].c_str());
	if(pFn)
	{
		try
		{
			(pFn)(pMddList);
		}
		catch(...)
		{
			(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, "[DBSA]  Unknown exception caught when calling (%s).", _procNameVector[EVID_TRGGMDD].c_str());
		}
	}
}

void DBSAdiMan::TrggHrchy(DWORD dwOperation, DA_hierarchyDb* pHierarchyBlock)
{
	if(_enabled==false || _procValidList[EVID_TRGGHRCHY]==false)
		return;
	DBSAProto_TrggHrchy pFn = (DBSAProto_TrggHrchy)::GetProcAddress(_hPluginLib, _procNameVector[EVID_TRGGHRCHY].c_str());
	if(pFn)
	{
		try
		{
			(pFn)(dwOperation, pHierarchyBlock);
		}
		catch(...)
		{
			(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, "[DBSA]  Unknown exception caught when calling (%s).", _procNameVector[EVID_TRGGHRCHY].c_str());
		}
	}
}

void DBSAdiMan::TrggCa(DA_entryDb* pAsset)
{
	if(_enabled==false || _procValidList[EVID_TRGGCA]==false)
		return;
	DBSAProto_TrggCa pFn = (DBSAProto_TrggCa)::GetProcAddress(_hPluginLib, _procNameVector[EVID_TRGGCA].c_str());
	if(pFn)
	{
		try
		{
			(pFn)(pAsset);
		}
		catch(...)
		{
			(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, "[DBSA]  Unknown exception caught when calling (%s).", _procNameVector[EVID_TRGGCA].c_str());
		}
	}
}

void DBSAdiMan::TrggStat(DA_entryDb* pEntryBlock, DA_stateDb* pStateBlock)
{
	if(_enabled==false || _procValidList[EVID_TRGGSTAT]==false)
		return;
	DBSAProto_TrggStat pFn = (DBSAProto_TrggStat)::GetProcAddress(_hPluginLib, _procNameVector[EVID_TRGGSTAT].c_str());
	if(pFn)
	{
		try
		{
			(pFn)(pEntryBlock, pStateBlock);
		}
		catch(...)
		{
			(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, "[DBSA]  Unknown exception caught when calling (%s).", _procNameVector[EVID_TRGGSTAT].c_str());
		}
	}
}