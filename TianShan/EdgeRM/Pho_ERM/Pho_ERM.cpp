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
// Ident : $Id: $
// Branch: $Name:  $
// Author:
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/EdgeRM/Pho_ERM/Pho_ERM.cpp $
// 
// 10    2/03/15 4:20p Li.huang
// fix adapter stop crash
// 
// 9     5/27/14 9:34a Li.huang
// fix up s6 connection with base url
// 
// 8     2/28/14 10:54a Li.huang
// modify log
// 
// 7     1/02/14 3:19p Zonghuan.xiao
// 
// 6     3/28/13 2:19p Li.huang
// 
// 5     10/26/12 9:08a Li.huang
// 
// 4     9/28/12 2:34p Li.huang
// using new rtsplcient interface
// 
// 3     3/25/11 10:24a Li.huang
// 
// 2     3/15/11 5:23p Fei.huang
// migrate to linux
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 11    10-02-01 17:15 Li.huang
// add SeaChange.S6.EdgeRM StreamLink
// 
// 10    10-01-11 13:58 Li.huang
// 
// 9     09-12-23 21:10 Li.huang
// add NSS streamLink
// 
// 8     09-12-15 18:09 Li.huang
// restore
// 
// 9     09-12-09 10:02 Li.huang
// 
// 8     09-12-08 17:54 Li.huang
// 
// 7     09-11-18 16:06 Li.huang
// 
// 6     09-10-21 17:02 Li.huang
// 
// 5     09-10-14 18:12 Li.huang
// 
// 4     09-09-28 17:12 Li.huang
// review


#include "FileLog.h"
#include "Configuration.h"

#ifdef ZQ_OS_MSWIN
#include  <io.h>
#endif
#include  "ZQResource.h"
#include "public.h"
#include "PhoEdgeRM.h"
#include "PhoNSSEdgeRM.h"
#include "S6EdgeRM.h"

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif

#define PHO_ERM_CFGFILENAME     "pho_ERM.xml"
#define PHO_ERM_LOGFILENAME     "pho_ERM.log"

bool bInService = false;



ZQ::common::Config::Loader<PHOConfig>  _cfg(PHO_ERM_CFGFILENAME);
::ZQTianShan::EdgeRM::PhoEdgeRMEnv *_gEnv = NULL;

ZQ::common::Log* phoErmLog;

typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::EdgeRMPHO > EdgeRMPHOPtr;
typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::NSSEdgeRMPHO > NSSEdgeRMPHOPtr;
typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::S6EdgeRMPHO > S6EdgeRMPHOPtr;

EdgeRMPHOPtr   _gEdgeRMPHOPtr = NULL;
NSSEdgeRMPHOPtr _gNSSEdgeRMPHOPtr = NULL;
S6EdgeRMPHOPtr  _gS6EdgeRMPHOPtr = NULL;

ZQTianShan::AccreditedPath::IPHOManager* _mgr = NULL;

bool loadConfigFile(std::string& configFilePath)
{
	std::string cfgpath;
	int npos = configFilePath.rfind(FNSEPC);
	if(npos > 0)
	{
		configFilePath = configFilePath.substr(0, npos +1);
		cfgpath = configFilePath + PHO_ERM_CFGFILENAME;
	    if(access (cfgpath.c_str(), 0 ) != 0 ) {
			configFilePath.clear();
		}
	}
	else {
		configFilePath.clear();
	}

	if(configFilePath.size() < 3)
	{
		cfgpath = ZQTianShan::getProgramRoot();
		cfgpath += "\\etc\\"PHO_ERM_CFGFILENAME;
	}

	if(!_cfg.load(cfgpath.c_str()))
		return false;

	if (_cfg.endpoint.empty())
		_cfg.endpoint = DEFAULT_ENDPOINT(EdgeRMPHO);

	if (_cfg.evictorSize < 1000)
		_cfg.evictorSize = 1000;
	if (_cfg.allocationLease < 60000)//minimum 60s
		_cfg.allocationLease = 60000;
	else if (_cfg.allocationLease > 60*1000*10)//maximum 10minute
		_cfg.allocationLease = 60*1000*10;

	return true;
}

extern "C"
{
	__EXPORT void InitPHO( ZQTianShan::AccreditedPath::IPHOManager& mgr, void* pCtx, const char* configFile, const char* logFolder)
	{
		if (bInService) 
			return;	

         ///load PHO configration file
		std::string cfgfile = configFile;
		if(!loadConfigFile(cfgfile))
			return;

        ///get logs folder
		std::string		strLogFolder;
		if( logFolder&&strlen(logFolder) > 0 )
		{
			strLogFolder = logFolder;
		}
		else
		{
			std::string strLogFolder = ZQTianShan::getProgramRoot();
			strLogFolder += "\\Logs\\";
		}
		if(  !(strLogFolder.at(strLogFolder.length()-1)=='\\' || strLogFolder.at(strLogFolder.length()-1)=='/'  )   )
		{
			strLogFolder+='\\';
		}
		///create log file
		phoErmLog = new ZQ::common::FileLog((strLogFolder + PHO_ERM_LOGFILENAME).c_str(),
			_cfg.lPHOLogLevel,
			_cfg.lPHOlogFileCount,
			_cfg.lPHOLogFileSize,
			_cfg.lPHOLogBufferSize,
			_cfg.lPHOLogWriteTimteout);

		if(NULL == phoErmLog)
			return;

		(*phoErmLog)(ZQ::common::Log::L_INFO, CLOGFMT(PHO_ERM, "Load PHO_ERM Version: %d.%d.%d.%d"), ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_PRODUCT_VER_PATCH, ZQ_PRODUCT_VER_BUILD);

		/// create PhoEdgeRMEnv object	
		if(_cfg.interval < 300)
			_cfg.interval = 300;
		_gEnv = new ::ZQTianShan::EdgeRM::PhoEdgeRMEnv(*phoErmLog, strLogFolder,_cfg.maxCandidates, _cfg.evictorSize, _cfg.allocationLease, _cfg.threadPoolsize, _cfg.interval, _cfg.databasePath.c_str());	
		if(!_gEnv)
		{
			(*phoErmLog)(ZQ::common::Log::L_ERROR, CLOGFMT(PHO_ERM, "failed to create PhoEdgeRMEnv object"));
			return;
		}

       /// initializa PhoEdgeRMEnv object
		if(!_gEnv->initialize())
		{
			(*phoErmLog)(ZQ::common::Log::L_ERROR, CLOGFMT(PHO_ERM, "failed to initialize PhoEdgeRMEnv object"));
			return;
		}

        /// create EdgeRMPHO object
		_gEdgeRMPHOPtr = new ZQTianShan::AccreditedPath::EdgeRMPHO(mgr, *_gEnv);
		if (_gEdgeRMPHOPtr )
		{
			mgr.registerStreamLinkHelper(STRMLINK_TYPE_TianShanERM, *_gEdgeRMPHOPtr, pCtx);
		}
		else
		{
			(*phoErmLog)(ZQ::common::Log::L_ERROR , CLOGFMT(PHO_ERM,"failed to create EdgeRMPHO object"));
			return;
		}

		/// create EdgeRMPHO object
		_gNSSEdgeRMPHOPtr = new ZQTianShan::AccreditedPath::NSSEdgeRMPHO(mgr, *_gEnv, *_gEdgeRMPHOPtr);
		if (_gNSSEdgeRMPHOPtr )
		{
			mgr.registerStreamLinkHelper(STRMLINK_TYPE_TianShanNSSERM, *_gNSSEdgeRMPHOPtr, pCtx);
		}
		else
		{
			(*phoErmLog)(ZQ::common::Log::L_ERROR , CLOGFMT(PHO_ERM,"failed to create NSSEdgeRMPHO object"));
			return;
		}

		/// create S6EdgeRMPHO object
		_gS6EdgeRMPHOPtr = new ZQTianShan::AccreditedPath::S6EdgeRMPHO(mgr, *_gEnv);
		if (_gS6EdgeRMPHOPtr )
		{
			mgr.registerStreamLinkHelper(STRMLINK_TYPE_TianShanS6ERM, *_gS6EdgeRMPHOPtr, pCtx);
		}
		else
		{
			(*phoErmLog)(ZQ::common::Log::L_ERROR , CLOGFMT(PHO_ERM,"failed to create S6EdgeRMPHO object"));
			return;
		}

		_mgr = &mgr;
		bInService = ( _gEdgeRMPHOPtr ? true:false);
	}

	__EXPORT  void UninitPHO(void)
	{	
		try
		{
			if(phoErmLog)
			{
				(*phoErmLog)(ZQ::common::Log::L_INFO , CLOGFMT(PHO_ERM,"UninitPHO() deactivate adapter"));
				(*phoErmLog).flush();
			}
			if(_mgr)
			{
				_mgr->unregisterStreamLinkHelper( STRMLINK_TYPE_TianShanERM );
				_mgr->unregisterStreamLinkHelper( STRMLINK_TYPE_TianShanNSSERM);
				_mgr->unregisterStreamLinkHelper( STRMLINK_TYPE_TianShanS6ERM);
			}
		}
		catch(...){}

		try
		{
		if (_gEnv != NULL)
		{
			delete _gEnv;
			_gEnv = NULL;
		}
		}
		catch(...){
		}

		if(phoErmLog)
		{
			(*phoErmLog)(ZQ::common::Log::L_INFO , CLOGFMT(PHO_ERM,"UninitPHO() destroy ice communicator"));
			(*phoErmLog).flush();
		}

		bInService = false;	
		_gEdgeRMPHOPtr = NULL;
		_gNSSEdgeRMPHOPtr = NULL;
		_gS6EdgeRMPHOPtr = NULL;

		if(phoErmLog)
		{
			(*phoErmLog)(ZQ::common::Log::L_INFO , CLOGFMT(PHO_ERM,"UninitPHO() destory log"));
			(*phoErmLog).flush();
		}

		if(phoErmLog)
		{
			try
			{
				delete phoErmLog;
				phoErmLog = NULL;
			}
			catch (...)
			{
			}
		}
	}

}
