
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
// Ident : $Id: CPEImpl.cpp $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
//
// ===========================================================================


#ifndef _ZQTianShan_ProvEventSink_H_
#define _ZQTianShan_ProvEventSink_H_

#include "Log.h"
#include "ContentStore.h"
#include "TsContentProv.h"


#define CS_PROVISION_EVENT_SINK							"CSEvtSink"

using namespace ::TianShanIce::ContentProvision;

class ProvisionEventSink : public ::TianShanIce::ContentProvision::ProvisionSessionBind 
{
public:
	typedef ::IceInternal::Handle<ProvisionEventSink> Ptr;

	ProvisionEventSink(::TianShanIce::Storage::ContentStoreExPrx csPrx, ZQ::common::Log& log);

	void init(::TianShanIce::Storage::ContentStoreExPrx	csPrx);

public:

	virtual void OnProvisionStateChanged(
		const ProvisionContentKey& contentKey, 
		::Ice::Long timeStamp, 
		::TianShanIce::ContentProvision::ProvisionState prevState, 
		::TianShanIce::ContentProvision::ProvisionState currentState, 
		const ::TianShanIce::Properties& params, 
		const ::Ice::Current& = ::Ice::Current());

	virtual void OnProvisionProgress(
		const ProvisionContentKey& contentKey, 
		::Ice::Long timeStamp, 
		::Ice::Long processed, 
		::Ice::Long total, 
		const ::TianShanIce::Properties&params, 
		const ::Ice::Current& = ::Ice::Current());

	virtual void OnProvisionStarted(const ProvisionContentKey&, 
		::Ice::Long, const ::TianShanIce::Properties&, 
		const ::Ice::Current& = ::Ice::Current());

	virtual void OnProvisionStopped(
		const ProvisionContentKey&, 
		::Ice::Long, bool, 
		const ::TianShanIce::Properties&, 
		const ::Ice::Current& = ::Ice::Current());

	virtual void OnProvisionStreamable(
		const ProvisionContentKey& contentKey, 
		::Ice::Long timeStamp, 
		bool streamable, 
		const ::TianShanIce::Properties& params, 
		const ::Ice::Current& = ::Ice::Current());

	virtual void OnProvisionDestroyed(
		const ProvisionContentKey& contentKey, 
		::Ice::Long timeStamp, 
		const ::TianShanIce::Properties& params, 
		const ::Ice::Current& = ::Ice::Current());

private:
	::TianShanIce::Storage::ContentStoreExPrx	_csPrx;
	ZQ::common::Log& _log;
};


#endif

