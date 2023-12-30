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
// Desc  : Implementation of provision StateChange & Progress event helper to 
//         turn to IceStorm message to JMS format message.
//
// Revision History: 
// ---------------------------------------------------------------------------

// ===========================================================================

#include "CSProvisionEventHelper.h"

namespace TianShanIce
{
namespace MessageAgent
{
/////////////////////////////////////////////////////////////////////////////////////
// If there is any changing on the TianShanIce::Storage::ContentProvisionStatus,   //
// including adding, deleting, changing order and so on, following definition      //
// MUST be do corresponding changing.                                              //
/////////////////////////////////////////////////////////////////////////////////////

char* status_name[] =	{    
						"setup",
						"start",
						"streamable",
						"provisioned",
						"fail",
						"destroy"
						};


ProvStateChangeHelper::ProvStateChangeHelper(JMSPublisher& publiser)
: _publiser(publiser)
{
}
	
void ProvStateChangeHelper::OnStateChange(std::string netId, std::string contentname, std::string triggerTimeUTC, 
					TianShanIce::Storage::ContentProvisionStatus previousStatus, 
					TianShanIce::Storage::ContentProvisionStatus currentStatus, 
					int errorCode, std::string errorMsg)
					
{
//                        JMS Message Format                            //
//////////////////////////////////////////////////////////////////////////
//    netid|content name|status|error code|time stamp                   //
//                                                                      //
//	status - (setup, start, streamable, provisioned, fail, destroy)     //
//////////////////////////////////////////////////////////////////////////

	// compose the jms message's body
	char jmsMsg[512];
	snprintf(jmsMsg, 510, "%s|%s|%s|%d|%s", 
					netId.c_str(), 
					contentname.c_str(), 
					status_name[(int)currentStatus],
					errorCode,
					triggerTimeUTC.c_str() );

	glog(ZQ::common::Log::L_DEBUG, "Received Provision OnStateChange event, composed JMS message is %s", jmsMsg);

	// push the message to the publisher
	_publiser.SendJMSMessage(jmsMsg);
}


ProvProgressHelper::ProvProgressHelper(JMSPublisher& publiser)
: _publiser(publiser)
{
}

void ProvProgressHelper::OnProgress(std::string netId, std::string contentname, 
									std::string triggerTimeUTC, 
									__int64 processed, __int64 total, int stepNo, int totalSteps)
{
//                        JMS Message Format                            //
//////////////////////////////////////////////////////////////////////////
//    netid|content name|processed (unit is not certain)|total          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

	// compose the jms message's body
	char jmsMsg[512];
	snprintf(jmsMsg, 510, "%s|%s|%lld|%lld", 
					netId.c_str(), 
					contentname.c_str(), 
					processed,
					total);

	glog(ZQ::common::Log::L_DEBUG, "Received Provision OnProgress event, composed JMS message is %s", jmsMsg);

	// push the message to the publisher
	_publiser.SendJMSMessage(jmsMsg);
}

}
}
