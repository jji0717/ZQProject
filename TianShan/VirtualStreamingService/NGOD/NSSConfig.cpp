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
// $Log: /ZQProjs/TianShan/VirtualStreamingService/NGOD/NSSConfig.cpp $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 2     09-07-21 15:03 Xiaoming.li
// merge from 1.8
// 
// 4     09-07-21 11:02 Xiaoming.li
// add netID for multiple NSS instance
// 
// 3     09-07-16 17:27 Xiaoming.li
// 
// 2     09-04-28 13:55 Xiaoming.li
// after CCUR integration
// 
// 1     09-02-20 16:14 Xiaoming.li
// 
// 6     08-12-17 14:09 Xiaoming.li
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
_iceStorm(ZQ::common::Config::Loader< IceStormConfig >("")),
_dataBase(ZQ::common::Config::Loader< Database >("")),
_NSSBaseConfig(ZQ::common::Config::Loader< NSSBaseConfig >(""))
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

	_dataBase.setLogger(&_logFile);
    if(!_dataBase.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <DataBase>"));
    }
	

	//get NSS configuration
	_NSSBaseConfig.setLogger(&_logFile);
	if(!_NSSBaseConfig.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSConfig,"fail parse <Icelog>"));
	}
}

}//namespace NSS

}//namespace ZQTianShan