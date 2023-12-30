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
// Ident : $Id: NSSConfig $
// Branch: $Name:  $
// Author: Xiaoming Li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ComcastNGOD/NSS/NSSConfig.cpp $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 6     08-11-14 11:48 Xiaoming.li
// add version info, change data folder setting and service name
// 
// 5     08-11-11 16:43 Xiaoming.li
// 
// 4     08-11-06 11:49 Xiaoming.li
// 
// 5     08-11-05 11:14 Xiaoming.li
// 
// 4     08-10-21 15:12 Xiaoming.li
// 
// 3     08-08-20 17:30 Xiaoming.li
// 
// 2     08-07-14 14:54 Xiaoming.li
// 
// 1     08-06-13 11:23 Xiaoming.li
// 
// 1     08-04-22 14:30 xiaoming.li
// initial checkin
// ===========================================================================

#include "NSSConfig.h"

namespace ZQTianShan{

namespace NSS{

NSSConfig::NSSConfig(const char *filepath):
_logFile(ZQ::common::Log::L_DEBUG),
_crashDump(ZQ::common::Config::Loader< CrashDump >("")),
_iceTrace(ZQ::common::Config::Loader< IceTrace >("")),
_iceStorm(ZQ::common::Config::Loader< IceStorm >("")),
_iceProperties(ZQ::common::Config::Loader< IceProperties >("")),
_dataBase(ZQ::common::Config::Loader< Database >("")),
_publishedLogs(ZQ::common::Config::Loader< PublishedLogs >("")),
_stBind(ZQ::common::Config::Loader< stBind >("")),
//_nssLog(ZQ::common::Config::Loader< nssLog >("")),
_nssIceLog(ZQ::common::Config::Loader< nssIceLog >("")),
_timeOut(ZQ::common::Config::Loader< TimeOut >("")),
_mediaCluster(ZQ::common::Config::Loader< MediaCluster >("")),
_sessionGroup(ZQ::common::Config::Loader< SessionGroup >(""))
//,_debug(ZQ::common::Config::Loader< Debug>(""))
{
	if (filepath != NULL)
		_strFilePath = filepath;
}

NSSConfig::~NSSConfig()
{
}

void NSSConfig::ConfigLoader()
{
	//get default configuration
	_crashDump.setLogger(&_logFile);
    if(!_crashDump.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <CrashDump>"));
    }

	_iceTrace.setLogger(&_logFile);
    if(!_iceTrace.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <IceTrace>"));
    }

	_iceStorm.setLogger(&_logFile);
	if(!_iceStorm.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <IceStorm>"));
	}
	
	_iceProperties.setLogger(&_logFile);
    if(!_iceProperties.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <IceProperties>"));
    }

	_dataBase.setLogger(&_logFile);
    if(!_dataBase.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <DataBase>"));
    }
	
	_publishedLogs.setLogger(&_logFile);
	if(!_publishedLogs.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <PublishedLogs>"));
	}

	//get NSS configuration
	_stBind.setLogger(&_logFile);
    if(!_stBind.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <Bind>"));
    }

	//_nssLog.setLogger(&_logFile);
 //   if(!_nssLog.load(_strFilePath.c_str()))
 //   {
 //       _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <log>"));
 //   }

	_nssIceLog.setLogger(&_logFile);
	if(!_nssIceLog.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <Icelog>"));
	}

	_timeOut.setLogger(&_logFile);
	if(!_timeOut.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <TimeOut>"));
	}
	
	_mediaCluster.setLogger(&_logFile);
    if(!_mediaCluster.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <MediaCluster>"));
    }
	
	_sessionGroup.setLogger(&_logFile);
    if(!_sessionGroup.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <MediaCluster>"));
    }

	//_debug.setLogger(&_logFile);
	//if(!_debug.load(_strFilePath.c_str()))
	//{
	//	_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <DEBUG>"));
	//}
}

}//namespace NSS

}//namespace ZQTianShan