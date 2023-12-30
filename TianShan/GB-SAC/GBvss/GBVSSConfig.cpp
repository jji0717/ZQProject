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
// Ident : $Id: GBVSSConfig $
// Branch: $Name:  $
// Author: Xiaoming Li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/GB-SAC/GBvss/GBVSSConfig.cpp $
// 
// 1     6/23/11 4:15p Xiaohui.chai
// 
// 4     2/09/11 1:59p Haoyuan.lu
// 
// 3     1/28/11 5:23p Haoyuan.lu
// 
// 2     1/19/11 5:00p Haoyuan.lu
// 
// 1     1/10/11 2:40p Haoyuan.lu
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

#include "GBVSSConfig.h"

namespace ZQTianShan{

namespace GBVSS{

GBVSSConfig::GBVSSConfig(const char *filepath):
_logFile(ZQ::common::Log::L_DEBUG),
_crashDump(ZQ::common::Config::Loader< CrashDump >("")),
_iceTrace(ZQ::common::Config::Loader< IceTrace >("")),
_iceStorm(ZQ::common::Config::Loader< IceStorm >("")),
_dataBase(ZQ::common::Config::Loader< Database >("")),
_GBVSSBaseConfig(ZQ::common::Config::Loader< GBVSSBaseConfig >(""))
{
	if (filepath != NULL)
		_strFilePath = filepath;
}

GBVSSConfig::~GBVSSConfig()
{
}

void GBVSSConfig::ConfigLoader()
{
	//get default configuration
	_crashDump.setLogger(&_logFile);
    if(!_crashDump.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(GBVSSConfig,"fail parse <CrashDump>"));
    }

	_iceTrace.setLogger(&_logFile);
    if(!_iceTrace.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(GBVSSConfig,"fail parse <IceTrace>"));
    }

	_iceStorm.setLogger(&_logFile);
	if(!_iceStorm.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(GBVSSConfig,"fail parse <IceStorm>"));
	}
	
	//get GBVSS configuration
	_GBVSSBaseConfig.setLogger(&_logFile);
	if(!_GBVSSBaseConfig.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(GBVSSConfig,"fail parse <GBVSS>"));
	}
}

}//namespace GBVSS

}//namespace ZQTianShan