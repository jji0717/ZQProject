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
// Name  : ChODDefines.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2006-8-23
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ChannelOnDemand/ChODDefines.cpp $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 5     09-10-08 16:11 Haoyuan.lu
// 
// 4     07-05-23 15:14 Jie.zhang
// BaseException changed
// 
// 3     06-08-28 12:01 Bernie.zhao
// 1st draft done
// 
// 2     06-08-23 12:42 Bernie.zhao
// creation
// ===========================================================================

#include "ZQ_common_Conf.h"
#include "Exception.h"
#include "Locks.h"
#include "ChODDefines.h"
#include "TianShanDefines.h"

namespace ZQChannelOnDemand {
	
	::TianShanIce::Variant& PDField(::TianShanIce::ValueMap& PD, const char* field)
	{
		if (NULL == field || *field ==0x00)
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("COD", 0, "NULL field to access private data");
//			::ZQ::common::_throw<::TianShanIce::InvalidParameter> ("NULL field to access private data");
		
		::TianShanIce::ValueMap::iterator it = PD.find(field);
		if (PD.end() == it)
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("COD", 0, "private data field \"%s\" is not found", field);
//			::ZQ::common::_throw<::TianShanIce::InvalidParameter> ("private data field \"%s\" is not found", field);
		
		return it->second;
	}
	
	// return the current GMT time in msec
	::Ice::Long now()
	{
		FILETIME systemtimeasfiletime;
		unsigned __int64 ltime;
		
		GetSystemTimeAsFileTime(&systemtimeasfiletime);
		memcpy(&ltime,&systemtimeasfiletime,sizeof(ltime));
		ltime /= 10000;  //convert nsec to msec
		
		return ltime;
	}

	std::string invokeSignature(const ::Ice::Current& c)
	{
		char buf[64];
		return ::ZQTianShan::IceCurrentToStr(c) + " @"+ ZQTianShan::TimeToUTC(ZQTianShan::now(), buf, sizeof(buf) -2, true);
	}

}

