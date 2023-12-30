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
#include "ProvisionEventI.h"

namespace TianShanIce
{
namespace MessageAgent
{


/// ProvisionStateChangeSink's IceStorm subscriber side implementation 
void ProvisionStateChangeSinkI::OnStateChange(const ::std::string& netId, const ::std::string& contentName, 
											  const ::std::string& utcTimestamp, 
											  ::TianShanIce::Storage::ContentProvisionStatus preStatus, 
											  ::TianShanIce::Storage::ContentProvisionStatus curStatus, 
											  ::Ice::Int errorCode, const ::std::string& errorMsg, const ::Ice::Current&)
{
	// turn to helper
	_helper.OnStateChange(netId, contentName, utcTimestamp, preStatus, curStatus, errorCode, errorMsg);
}


/// ProvisionProgressSink's IceStorm subscriber side implementation 
 void ProvisionProgressSinkI::OnProgress(const ::std::string& netId, const ::std::string& contentName, 
												const ::std::string& utcTimestamp, 
												::Ice::Long processed, ::Ice::Long total, 
												::Ice::Int stepNO, ::Ice::Int totalSteps, 
												const ::Ice::Current&)
{
	 // turn to helper
	_helper.OnProgress(netId, contentName, utcTimestamp, processed, total, stepNO, totalSteps);
}


}
}
