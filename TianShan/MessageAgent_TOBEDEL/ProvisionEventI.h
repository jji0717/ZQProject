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
// Ident : $Id: CSProvisionEventHelper.cpp $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : Provision StateChange and Progress event's subscribe implementation.
//
// Revision History: 
// ---------------------------------------------------------------------------

// ===========================================================================

#ifndef _PROVSIONEVENTI_H
#define _PROVSIONEVENTI_H

#include "CSProvisionEventHelper.h"

namespace TianShanIce
{
namespace MessageAgent
{


/// ProvisionStateChangeSink's IceStorm subscriber side implementation 
class ProvisionStateChangeSinkI : public TianShanIce::Storage::ProvisionStateChangeSink
{
public:
	ProvisionStateChangeSinkI(ProvStateChangeHelper& helper) : _helper(helper) {};
	virtual ~ProvisionStateChangeSinkI() {};

public:
    virtual void OnStateChange(const ::std::string&, const ::std::string&, const ::std::string&, ::TianShanIce::Storage::ContentProvisionStatus, ::TianShanIce::Storage::ContentProvisionStatus, ::Ice::Int, const ::std::string&, const ::Ice::Current& = ::Ice::Current());	

	virtual void ping(::Ice::Long, const ::Ice::Current& = ::Ice::Current()) {};

private:
	ProvStateChangeHelper& _helper;
};
typedef IceUtil::Handle<ProvisionStateChangeSinkI> ProvisionStateChangeSinkIPtr;


/// ProvisionProgressSink's IceStorm subscriber side implementation 
class ProvisionProgressSinkI : public TianShanIce::Storage::ProvisionProgressSink
{
public:
	ProvisionProgressSinkI(ProvProgressHelper& helper) : _helper(helper) {};
	virtual ~ProvisionProgressSinkI() {};

public:
    virtual void OnProgress(const ::std::string&, const ::std::string&, const ::std::string&, ::Ice::Long, ::Ice::Long, ::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current());
	virtual void ping(::Ice::Long, const ::Ice::Current& = ::Ice::Current()) {};

private:
	ProvProgressHelper& _helper;
};
typedef IceUtil::Handle<ProvisionProgressSinkI> ProvisionProgressSinkIPtr;

	
}
}


#endif  // _PROVSIONEVENTI_H