// ===========================================================================
// Copyright (c) 2008 by
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
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================

#ifndef ZQTS_CPE_RTIPARAMSI_H
#define ZQTS_CPE_RTIPARAMSI_H

#include <string>
#include "SubjectWrite.h"
#include "RtiParams.h"
#include "BaseClass.h"
#include "SessionGroupId.h"


namespace ZQ
{
	namespace common{
		class Log;
	}

}

namespace ZQTianShan 
{
namespace ContentProvision
{

class LeadSessI;	

class LeadSessEvent
{
public:
	virtual void onDestroy(LeadSessI* pLeadSess) = 0;

};

class LeadSessI : public SubjectWrite,
	public RtiParams,
	protected BaseGraph
{
public:
	virtual ~LeadSessI(){};
	virtual bool initialize() = 0;
	virtual bool execute() = 0;
	virtual void stop() = 0;
	virtual void uninitialize() = 0;

	virtual bool isInitialized() = 0;
	virtual bool isExecuted() = 0;

	virtual void setLog(ZQ::common::Log* pLog) = 0;

	virtual bool getMediaInfo(MediaInfo& mInfo) = 0;
	
	virtual void setEventHandle(LeadSessEvent* pEvent) = 0;
};


}
}

#endif