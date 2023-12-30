// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: pho_HttpSS.cpp $
// Branch: $Name:  $
// Author: Li.huang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/pho/pho_HttpSS/pho_HttpSS.cpp $
// 
// 1     5/08/14 3:32p Li.huang
// ===========================================================================

#include "C2StreamPHO.h"
#include "Log.h"
#include "FileLog.h"
#include "Config.h"
#include "FileSystemOp.h"

#ifdef ZQ_OS_MSWIN
HMODULE			_gCurrentModule=NULL;

BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	_gCurrentModule=(HMODULE)hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif

typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::C2StreamPHO > C2StreamPHOPtr;

C2StreamPHOPtr   _gC2StreamPHOPtr = NULL;

bool bInService = false;
ZQ::common::Log* gHttpSSLog = NULL;
ZQ::common::Config::Loader<PHOConfig>       cfg("");

extern "C" 
{
	__EXPORT void InitPHO(ZQTianShan::AccreditedPath::IPHOManager& mgr, void* pCtx,const char* configFile,const char* logFolder)
	{
		if (bInService) 
			return;	
		std::string		strLogFolder;
		std::string		strLogFileName="pho_HttpSS.log";
		long			logFileSize=50*1024*1024;
		long			logBufferSize=204800;
		long			logWriteTimeout=2;
		long			logLevel=7;
		if(configFile&&strlen(configFile)>0)
		{
			//read the config file

			if(cfg.load(configFile))
			{
				strLogFileName=cfg.szPHOLogFileName;
				logFileSize=cfg.lPHOLogFileSize;
				logBufferSize=cfg.lPHOLogBufferSize;
				logWriteTimeout=cfg.lPHOLogWriteTimteout;				
			}
		}
		if(logFolder&&strlen(logFolder)>0)
		{
			strLogFolder=logFolder;
		}
		else
		{
			strLogFolder=FS::getImagePath();
			std::string::size_type iPos=strLogFolder.rfind(FNSEPC);
			if(iPos!=std::string::npos)
			{
				strLogFolder=strLogFolder.substr(0,iPos);
			}
		}
		if(!(strLogFolder.at(strLogFolder.length()-1)==FNSEPC))
		{
			strLogFolder+=FNSEPC;
		}

		gHttpSSLog =new ZQ::common::FileLog( (strLogFolder+strLogFileName).c_str(),
			logLevel,
			ZQLOG_DEFAULT_FILENUM,
			logFileSize,
			logBufferSize,
			logWriteTimeout);
	
		if(NULL == gHttpSSLog)	
			return;

		if (!_gC2StreamPHOPtr)
			_gC2StreamPHOPtr = new ZQTianShan::AccreditedPath::C2StreamPHO(mgr);

		mgr.registerStreamLinkHelper(STRMLINK_TYPE_HTTPSS_C2STREAM, *_gC2StreamPHOPtr, pCtx);
		bInService = (_gC2StreamPHOPtr );
		(*gHttpSSLog)(ZQ::common::Log::L_DEBUG,"============start pho_HttpSS================");
	}

	__EXPORT void UninitPHO(void)
	{
		(*gHttpSSLog)(ZQ::common::Log::L_DEBUG,"============stop pho_HttpSS================");
		if(gHttpSSLog)
		{
			try
			{
				delete gHttpSSLog;
				gHttpSSLog=NULL;
			}
			catch (...)
			{
			}
		}
		bInService = false;
		_gC2StreamPHOPtr = NULL;
	}
}

