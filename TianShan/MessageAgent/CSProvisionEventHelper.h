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
// Desc  : Definition of provision StateChange & Progress event helper to 
//         turn to IceStorm message to JMS format message.
//
// Revision History: 
// ---------------------------------------------------------------------------

// ===========================================================================
#ifndef   _CONTENTSTORE_PROVISION_EVENT_HELPER_H
#define   _CONTENTSTORE_PROVISION_EVENT_HELPER_H

#include "JmsPublisher.h"
#include "TsStorage.h"

namespace TianShanIce
{
namespace MessageAgent
{

/// predefinition
class ProvStateChangeHelper;
class ProvProgressHelper;


/// ProvisionStateChangeSink's helper class to convert the received IceStorm message to JMS message
class ProvStateChangeHelper
{
public:
	ProvStateChangeHelper(JMSPublisher& publiser);
	virtual ~ProvStateChangeHelper(){};

public:
	/// event will be fired when a content's provision status is changed
	///@param[in] netId the net Id of the specified contentstore
	///@param[in] contentname content name that can be used to access Contnet thru ContentStore::openContent()
	///@return false if no more ContentStoreEventBind objects need to process this event in the sequence
	void OnStateChange(std::string netId, std::string contentname, std::string triggerTimeUTC, 
						TianShanIce::Storage::ContentProvisionStatus previousStatus, 
						TianShanIce::Storage::ContentProvisionStatus currentStatus, 
						int errorCode, std::string errorMsg);

private:
	JMSPublisher& _publiser;
};


/// ProvisionProgressSink's helper class to convert the received IceStorm message to JMS message
class ProvProgressHelper
{
public:
	ProvProgressHelper(JMSPublisher& publiser);
	virtual ~ProvProgressHelper(){};

public:
	/// event will be fired when a content provision is processing with progress
	///@param[in] netId the net Id of the specified contentstore
	///@param[in] contentname content name that can be used to access Contnet thru ContentStore::openContent()
	///@param[in] processed the processed provision units in the current step
	///@param[in] total the total provision units in the current step. The unit can percentage, or file size in KB, etc
	///@param[in] stepNo the sequence number of the current step
	///@param[in] totalSteps the total step number must be performed for this provision procedure
	///@return false if no more ContentStoreEventBind objects need to process this event in the sequence
	void OnProgress(std::string netId, std::string contentname, std::string triggerTimeUTC, __int64 processed, __int64 total, int stepNo, int totalSteps);

private:
	JMSPublisher& _publiser;

};

}
}



#endif    // _CONTENTSTORE_PROVISION_EVENT_HELPER_H