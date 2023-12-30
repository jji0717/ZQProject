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
// Ident : $Id: pho_SeaChange.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/pho/pho_SeaChange/pho_SeaChange.cpp $
// 
// 6     9/05/13 3:25p Zonghuan.xiao
// modify log from glog to mlog because glog cannot confirm which pclog
// used
// 
// 5     1/23/13 10:11a Hongquan.zhang
// change log level by configuration
// 
// 4     3/21/11 3:53p Fei.huang
// 
// 3     3/10/11 2:25p Fei.huang
// migrate to linux
// 
// 2     3/08/11 12:28p Hongquan.zhang
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 11    08-04-09 16:14 Xiaohui.chai
// new config loader
// 
// 10    08-03-18 14:49 Hongquan.zhang
// use configloader as a global member
// 
// 9     07-12-14 11:38 Hongquan.zhang
// 
// 8     07-09-18 12:55 Hongquan.zhang
// 
// 7     07-03-01 15:27 Hongquan.zhang
// 
// 6     07-01-12 12:08 Hongquan.zhang
// 
// 5     07-01-09 15:10 Hongquan.zhang
// 
// 4     07-01-05 10:59 Hongquan.zhang
// 
// 3     06-12-28 16:43 Hongquan.zhang
// 
// 2     06-09-19 11:50 Hui.shao
// ===========================================================================

#include "Raid5sqrPHO.h"
#include "IpEdgePHO.h"
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

typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::Raid5sqrPHO > Raid5sqrPHOPtr;
typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::IpEdgePHO > IpEdgePHOPtr;

Raid5sqrPHOPtr _gRaid5sqrPHOPtr = NULL;
IpEdgePHOPtr   _gIpEdgePHOPtr = NULL;

bool bInService = false;
ZQ::common::Config::Loader<PHOConfig>       cfg("");

ZQ::common::Log* pho_Seachangelog = NULL;
#define MLOG (*pho_Seachangelog)

extern "C" 
{
__EXPORT void InitPHO(ZQTianShan::AccreditedPath::IPHOManager& mgr, void* pCtx,const char* configFile,const char* logFolder)
{
	if (bInService) 
		return;	
	std::string		strLogFolder;
	std::string		strLogFileName="pho_seachange.log";
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
			logLevel = cfg.lPHOLogLevel;
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
	
	pho_Seachangelog=new ZQ::common::FileLog( (strLogFolder+strLogFileName).c_str(),
														logLevel,
														ZQLOG_DEFAULT_FILENUM,
														logFileSize,
														logBufferSize,
														logWriteTimeout);
	
	if (!_gRaid5sqrPHOPtr)
		_gRaid5sqrPHOPtr = new ZQTianShan::AccreditedPath::Raid5sqrPHO(mgr);

	if (!_gIpEdgePHOPtr)
		_gIpEdgePHOPtr = new ZQTianShan::AccreditedPath::IpEdgePHO(mgr);

	mgr.registerStorageLinkHelper(STORLINK_TYPE_RAID5SQR, *_gRaid5sqrPHOPtr, pCtx);
	mgr.registerStreamLinkHelper(STRMLINK_TYPE_IPEDGE_IP, *_gIpEdgePHOPtr, pCtx);
	mgr.registerStreamLinkHelper(STRMLINK_TYPE_IPEDGE_DVBC, *_gIpEdgePHOPtr, pCtx);
	mgr.registerStreamLinkHelper(STRMLINK_TYPE_IPEDGE_DVBCSHARELINK,*_gIpEdgePHOPtr,pCtx );
	mgr.registerStreamLinkHelper(STRMLINK_TYPE_IPEDGE_IPSHARELINK , *_gIpEdgePHOPtr , pCtx );
	bInService = (_gRaid5sqrPHOPtr && _gIpEdgePHOPtr);
	MLOG(ZQ::common::Log::L_DEBUG,"============start pho_seachange================");
}

__EXPORT void UninitPHO(void)
{
	MLOG(ZQ::common::Log::L_DEBUG,"============stop pho_seachange================");
 	if(pho_Seachangelog)
 	{
 		try
 		{
 			delete pho_Seachangelog;
 			pho_Seachangelog = NULL;
 		}
 		catch (...)
 		{
 		}
 	}
	bInService = false;
	_gRaid5sqrPHOPtr = NULL;
	_gIpEdgePHOPtr = NULL;
}
}
