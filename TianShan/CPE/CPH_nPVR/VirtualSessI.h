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

#ifndef ZQTS_CPE_VIRTUALSESSI_H
#define ZQTS_CPE_VIRTUALSESSI_H

#include "ObserverWriteI.h"
#include "RtiParams.h"
#include "SessionGroupId.h"
#include "BaseClass.h"


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
	
class VirtualSessI : public ObserverWriteI,
	public RtiParams
{
public:

	enum SessionState
	{
		StateInit,
		StateProcessing,
		StateSuccess,
		StateFailure
	};

	VirtualSessI()
	{
		_state = StateInit;
		_llProcv = 0;
		_llTotal = 0;
		_errCode = 0;
		_bEnableMD5 = false;
	}

	virtual ~VirtualSessI(){};

	virtual bool initialize() = 0;
	virtual bool execute() = 0;
	virtual void uninitialize() = 0;

	//get the index file path & file name
	virtual std::string getIndexPathName() = 0;

	virtual void getLastError(std::string& strErr, int& errCode)
	{
		strErr = _strErr;
		errCode = _errCode;
	}

	virtual bool isError()
	{
		return (getState()==StateFailure);
	}

	virtual void getProgress(LONGLONG& procv, LONGLONG& total)
	{
		procv = _llProcv;
		total = _llTotal;
	}

	virtual LONGLONG getSupportFileSize() = 0;

	// get md5 check sum for the main file
	virtual std::string getMD5Sum() = 0;

	virtual void enableMD5sum(bool bEnable = true)
	{
		_bEnableMD5 = bEnable;
	}

	virtual SessionState getState()
	{
		return _state;
	}

	virtual void setLog(ZQ::common::Log* pLog) = 0;

	virtual bool getMediaInfo(MediaInfo& mInfo) = 0;

	virtual void setLastError(const std::string& strErr, int errCode)
	{
		_strErr = strErr;
		_errCode = errCode;

		setState(StateFailure);
	}
    virtual void setTestMode(bool btest = false)=0;
protected:
	virtual void setState(SessionState st)
	{
		_state = st;
	}


	std::string							_strErr;
	int									_errCode;

	SessionState			_state;
	LONGLONG				_llProcv;
	LONGLONG				_llTotal;

	bool					_bEnableMD5;

	ZQ::common::Log*		_log;
};



}
}

#endif