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
// Ident : $Id: ProvSchdSafeStore.ICE $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/StreamSmith/NodeContentStore/ICE/ProvSchdSafeStore.ice $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 1     08-04-28 12:26 Hongquan.zhang
// do not share with \TianShan\ContentStore\ice\
// any longer
// 
// 6     07-08-16 11:25 Fei.huang
// 
// 6     07-08-13 13:26 Fei.huang
// 
// 5     07-04-06 11:01 Fei.huang
// 
// 4     06-09-25 16:45 Ken.qian
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_ProvSchdSafeStore_ICE__
#define __ZQ_TianShanIce_ProvSchdSafeStore_ICE__

#include "TianShanIce.ICE"
#include "TsStorage.ICE"
#include <Ice/Identity.ice>

module TianShanIce
{

module Storage
{

//class ScheduleInfo
//{
//	nonmutating string getContentName();
//	void setContentName(string name);
//
//	nonmutating string getSourceURL();
//	void setSourceURL(string name);
//
//	nonmutating bool isActiveProv();
//	void setIsActiveProv(bool active);
//
//	nonmutating string getSrcType();
//	void setSrcType(string type);
//
//	nonmutating string getDesType();
//	void setDesType(string type);
//
//	nonmutating bool getOverrideExisting();
//	void setOverrideExisting(bool override);
//
//	nonmutating string getStartTime();
//	void setStartTime(string time);
//
//	nonmutating string getEndTime();
//	void setEndTime(string time);
//
//	nonmutating long getTransferBitrate();
//	void setTransferBitrate(long bandwidth);
//
//
//	string contentName;
//	string sourceURL;
//
//	bool   activeProv;
//	
//	string srcType;
//	string desType;
//	
//	bool overrideExisting;
//	
//	string startTime;
//	string endTime;
//	
//	long transferBitrate;
//};

class StatusMsg 
{
	nonmutating string getContentName();
	void setContentName(string name);

	nonmutating ContentProvisionStatus getPreviousStatus();
	void setPreviousStatus(ContentProvisionStatus stat);

	nonmutating ContentProvisionStatus getCurrentStatus();
	void setCurrentStatus(ContentProvisionStatus stat);

	nonmutating string getTriggerTime();
	void setTriggerTime(string time);

	nonmutating int getErrorCode();
	void setErrorCode(int err);

	nonmutating string getErrorMsg();
	void setErrorMsg(string msg);

	string contentName;				         // content name

	ContentProvisionStatus previousStatus;   // the status of the event
	ContentProvisionStatus currentStatus;    // the status of the event

	string triggerTime;                      // the trigger time

	int errorCode;                           // error code
	string errorMsg;				         // error message
};

};
};

#endif // __ZQ_TianShanIce_ProvSchdSafeStore_ICE__
