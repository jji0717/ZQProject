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
// Name  : ChODDefines.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2006-8-23
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ChannelOnDemand/ChODDefines.h $
// 
// 2     1/11/16 5:30p Dejian.fei
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 9     09-10-08 16:11 Haoyuan.lu
// 
// 8     09-04-24 17:19 Haoyuan.lu
// 
// 7     09-02-06 17:25 Haoyuan.lu
// 
// 6     06-10-23 10:04 Jie.zhang
// 
// 5     06-09-26 11:18 Jie.zhang
// 
// 4     06-09-20 14:32 Jie.zhang
// ===========================================================================

#ifndef __CHODDEFINES_H__
#define __CHODDEFINES_H__

#ifdef ZQCOMMON_DLL
#  include "ZQ_common_Conf.h"
#  include "Exception.h"
#  include "Locks.h"
#endif ZQCOMMON_DLL

#include "TianShanIce.h"
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

#ifdef _DEBUG
#define LIBSUFFIX "d"
#else
#define LIBSUFFIX ""
#endif // _DEBUG

#pragma comment(lib, "Ice" LIBSUFFIX)
#pragma comment(lib, "IceUtil" LIBSUFFIX)
#pragma comment(lib, "freeze" LIBSUFFIX)

#pragma warning (disable: 4541)

#define PD_FIELD(_CATEGORY, _FIELD)  (#_CATEGORY "." #_FIELD)

#define ADAPTER_NAME_ChOD				"ChannelOnDemandAdapter"
#define DEFAULT_ENDPOINT_ChOD			"tcp -h 10.15.10.250 -p 9832"

#define SERVICE_NAME_ChannelPublisher	"ChannelPublisher"
#define SERVICE_NAME_ChannelPublisherEx	"ChannelPublisherEx"
#define CHANNEL_ONDEMAND_APPNAME		"ChannelOnDemandApp"

#define TODAS_ENDPOINT		            "TodasEndpont"
//#define USE_OLD_NS // The switch to use the old namespace

#ifdef USE_OLD_NS
#  define  NS_PREFIX(_CLS) _CLS
#else
#  define  NS_PREFIX(_CLS) TianShanIce::Application::_CLS
#endif // USE_OLD_NS

#ifdef ZQCOMMON_DLL
#  define CONN_TRACE(_CURRENT, _CLASS, _FUNC) if (_CURRENT.con) glog(ZQ::common::Log::L_DEBUG, CLOGFMT(_CLASS, #_FUNC "() by \"%s\""), c.con->toString().c_str());
#else
#  define CONN_TRACE(_CURRENT, _CLASS, _FUNC) ;
#endif

namespace ZQChannelOnDemand {
	
	/// schema definitions
	typedef struct _ConfItem
	{
		const char*			keyname;
		::TianShanIce::ValueType			type;
		//bool				optional;
		bool				optional2;
		const char*			hintvalue;
	} ConfItem;
	
	typedef ::std::vector< Ice::Identity > IdentCollection;
	
	::TianShanIce::Variant& PDField(::TianShanIce::ValueMap& PD, const char* field);
	
	/// return the current GMT time in msec
	::Ice::Long now();

	/// return invoke signature
	std::string invokeSignature(const ::Ice::Current& c);
	
#define ASSETVARIANT(_VAL, _TYPE) if (_TYPE != _VAL.type) ::ZQ::common::_throw<::TianShanIce::InvalidParameter> ("variant type %x doesn't match %x", _VAL.type, _TYPE);
	
} // namespace

#endif