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
// Ident : $Id: SessionWatchDog.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/SessionWatchDog.h $
// 
// 5     3/25/15 5:01p Build
// 
// 4     3/16/15 2:34p Build
// 
// 3     9/12/12 6:11p Zonghuan.xiao
// implement snmp   HeadEnd Sessions (Weiwoo)
// 
// 2     3/07/11 5:00p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 4     07-01-05 10:59 Hongquan.zhang
// 
// 3     10/08/06 6:38p Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_Weiwoo_SessionWatchDog_H__
#define __ZQTianShan_Weiwoo_SessionWatchDog_H__

#include "../common/TianShanDefines.h"

#include "WeiwooAdmin.h"
#include "NativeThreadPool.h"
#include "SystemUtils.h"

namespace ZQTianShan {
namespace Weiwoo {

class WeiwooSvcEnv;

// -----------------------------
// class SessionWatchDog
// -----------------------------
///
class SessionWatchDog : public ZQ::common::ThreadRequest
{
public:
	/// constructor
    SessionWatchDog(WeiwooSvcEnv& env);
	virtual ~SessionWatchDog();

	///@param[in] sessIdent identity of session
	///@param[in] timeout the timeout to wake up timer to check the specified session
	void watchSession(const ::Ice::Identity& sessIdent, long timeout);
	
	int getWatchSize(void)
	{
		return (int)_expirations.size();
	}

	//quit watching
	void quit();

protected: // impls of ThreadRequest

	virtual bool init(void);
	virtual int run(void);
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false);

	void wakeup();

protected:

	typedef std::map <Ice::Identity, ::Ice::Long > ExpirationMap; // sessId to expiration map
	ZQ::common::Mutex   _lockExpirations;
	ExpirationMap		_expirations;

	bool		  _bQuit;
	WeiwooSvcEnv& _env;
	::Ice::Long			_nextWakeup;
	SYS::SingleObject		  _hWakeupEvent;
};

}} // namespace

#endif // __ZQTianShan_Weiwoo_SessionWatchDog_H__

