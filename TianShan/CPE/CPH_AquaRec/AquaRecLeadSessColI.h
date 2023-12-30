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

#ifndef ZQTS_CPE_AQUARECLEADSESSCOLI_H
#define ZQTS_CPE_AQUARECLEADSESSCOLI_H

#include "BaseClass.h"

namespace ZQTianShan 
{
namespace ContentProvision
{

class AquaRecLeadSessI;
class AquaRecVirtualSessI;

class AquaRecLeadSessColI
{
public:
	AquaRecLeadSessColI()
	{
		_nMaxIdleInMs = 10*1000;		//default 10 seconds
		_errCode = 0;
		_nMaxSessionCount = 60;			//max session count to 60
	}

	virtual ~AquaRecLeadSessColI(){};

	virtual void setMaxIdleTime(int nMaxIdleInMs)
	{
		_nMaxIdleInMs = nMaxIdleInMs;
	}

	virtual int getMaxIdleTime()
	{
		return _nMaxIdleInMs;
	}

	//set the max concurrent session count
	virtual void setMaxSessionCount(int nMaxSessionCount)
	{
		_nMaxSessionCount = nMaxSessionCount;
	}

	virtual int getMaxSessionCount()
	{
		return _nMaxSessionCount;
	}


	virtual bool startMonitor() = 0;
	virtual void stopMonitor() = 0;

	//add virtual session to LeadSession
	virtual bool addVirtualSession(AquaRecVirtualSessI* pVirtualSess) = 0;
	//remove virtual session from LeadSession
	virtual bool removeVirtualSession(AquaRecVirtualSessI* pVirtualSess) = 0;

	virtual bool updateScheduledTime(AquaRecVirtualSessI* pVirtualSess)= 0;

	virtual bool getMediaInfo(AquaRecVirtualSessI* pVirtualSess, MediaInfo& mInfo) = 0;

	static AquaRecLeadSessColI* instance();

	static void setInstance(AquaRecLeadSessColI* pNew);

	static void destroyInstance();

	virtual void getLastError(int& errCode, std::string& strErr)
	{
		strErr = _strErr;
		errCode = _errCode;
	}

public:

	virtual void setLastError(const std::string& strErr, int errCode)
	{
		_strErr = strErr;
		_errCode = errCode;
	}

	virtual void monitorIdleSession() = 0;

protected:	
	static AquaRecLeadSessColI*	        _instance;	

	std::string							_strErr;
	int									_errCode;

	int									_nMaxIdleInMs;
	int									_nMaxSessionCount;
};

}}

#endif
