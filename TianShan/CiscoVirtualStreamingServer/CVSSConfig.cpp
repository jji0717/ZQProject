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
// Ident : $Id: CVSSConfig $
// Branch: $Name:  $
// Author: Xiaoming Li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CiscoVirtualStreamingServer/CVSSConfig.cpp $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 3     09-01-21 16:59 Xiaoming.li
// change config format
// 
// 2     09-01-20 9:50 Xiaoming.li
// delete timeout
// 
// 1     08-12-15 9:07 Xiaoming.li
// initial checkin
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

#include "CVSSConfig.h"

namespace ZQTianShan{

namespace CVSS{

CVSSConfig::CVSSConfig(const char *filepath):
_logFile(ZQ::common::Log::L_DEBUG),
_crashDump(ZQ::common::Config::Loader< CrashDump >("")),
_iceTrace(ZQ::common::Config::Loader< IceTrace >("")),
_iceStorm(ZQ::common::Config::Loader< IceStorm >("")),
_iceProperties(ZQ::common::Config::Loader< IceProperties >("")),
_dataBase(ZQ::common::Config::Loader< Database >("")),
_publishedLogs(ZQ::common::Config::Loader< PublishedLogs >("")),
_rtspProp(ZQ::common::Config::Loader< RTSPProp >("")),
_cLogFile(ZQ::common::Config::Loader< LogFile >("")),
_bind(ZQ::common::Config::Loader< stBind >("")),
_iceLog(ZQ::common::Config::Loader< IceLog >("")),
//_timeOut(ZQ::common::Config::Loader< TimeOut >("")),
_streamingServer(ZQ::common::Config::Loader< StreamingServer >("")),
_soapLog(ZQ::common::Config::Loader< SoapLog >("")),
_storeInfo(ZQ::common::Config::Loader< StoreInfo >(""))
{
	if (filepath != NULL)
		_strFilePath = filepath;
}

CVSSConfig::~CVSSConfig()
{
}

void CVSSConfig::ConfigLoader()
{
	//get default configuration
	_crashDump.setLogger(&_logFile);
    if(!_crashDump.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSConfig,"fail parse <CrashDump>"));
    }

	_iceTrace.setLogger(&_logFile);
    if(!_iceTrace.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSConfig,"fail parse <IceTrace>"));
    }

	_iceStorm.setLogger(&_logFile);
	if(!_iceStorm.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSConfig,"fail parse <IceStorm>"));
	}
	
	_iceProperties.setLogger(&_logFile);
    if(!_iceProperties.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSConfig,"fail parse <IceProperties>"));
    }

	_dataBase.setLogger(&_logFile);
    if(!_dataBase.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSConfig,"fail parse <DataBase>"));
    }
	
	_publishedLogs.setLogger(&_logFile);
	if(!_publishedLogs.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSConfig,"fail parse <PublishedLogs>"));
	}

	_rtspProp.setLogger(&_logFile);
	if(!_rtspProp.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSConfig,"fail parse <RTSPProp>"));
	}

	//get CVSS configuration
	if(!_cLogFile.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSConfig,"fail parse <LogFile>"));
	}

	_bind.setLogger(&_logFile);
    if(!_bind.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSConfig,"fail parse <Bind>"));
    }

	_iceLog.setLogger(&_logFile);
	if(!_iceLog.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSConfig,"fail parse <Icelog>"));
	}
	
	_streamingServer.setLogger(&_logFile);
    if(!_streamingServer.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSConfig,"fail parse <StreamingServer>"));
    }
	
	_soapLog.setLogger(&_logFile);
    if(!_soapLog.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSConfig,"fail parse <SoapLog>"));
    }

	_storeInfo.setLogger(&_logFile);
	if(!_storeInfo.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSConfig,"fail parse <StoreInfo>"));
	}
}

}//namespace CVSS

}//namespace ZQTianShan