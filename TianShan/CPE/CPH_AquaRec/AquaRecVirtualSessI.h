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

#include "BaseClass.h"
#include "ErrorCode.h"
class  CdmiClientBase;
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
	
class AquaRecVirtualSessI 
{
public:

	enum SessionState
	{
		StateInit,
		StateProcessing,
		StateSuccess,
		StateFailure
	};

	AquaRecVirtualSessI()
	{
		_state = StateInit;
		_errCode = 0;
		_contentState = false;
	}

	virtual ~AquaRecVirtualSessI(){};

	virtual bool initialize() = 0;
	virtual bool execute() = 0;
	virtual void uninitialize() = 0;

   
	virtual void setBandWidth(int nBandWidth)
	{
		_bBandWidth = nBandWidth;
	}
	virtual void setSessionId(const std::string& sessId) 
	{
		_sessId = sessId;
	};

	virtual void setContentId(const std::string& contentId) 
	{
		_contentId = contentId;
	};
	virtual void setChannnelName(const std::string& channelName)
	{
		_channelName = channelName;
	};

	virtual std::string getContentId() 
	{
		return _contentId;
	};
	virtual std::string getChannnelName()
	{
		return _channelName;;
	};

	virtual std::string getSessId()
	{
		return _sessId;
	};
	virtual void updateScheduledTime(const std::string& startTimeUTC, const std::string& endTimeUTC) = 0;

	virtual void setScheduledTime(const std::string& startTimeUTC, const std::string& endTimeUTC)
	{
		_startTimeUTC = startTimeUTC;
		_endTimeUTC = endTimeUTC;
	};
	virtual void getScheduledTime(std::string& startTimeUTC, std::string& endTimeUTC)
	{
		startTimeUTC = _startTimeUTC;
		endTimeUTC = _endTimeUTC;
	};

	virtual void getLastError(std::string& strErr, int& errCode)
	{
		strErr = _strErr;
		errCode = _errCode;
	}

	virtual bool isError()
	{
		return (getState()==StateFailure);
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

	virtual void getProgress(int64& procv, int64& total) = 0;

	virtual void setState(SessionState st)
	{
		_state = st;
	}
	virtual bool getContentState()
	{
		return _contentState;
	}
	virtual void  setContentState(bool cst)
	{
		_contentState = cst;
	}

	virtual bool readContent(CdmiClientBase* ptrCdmiClient,std::string& bitrate) = 0;

protected:

	std::string							_strErr;
	int									_errCode;
	SessionState						_state;
	int									_bBandWidth;
	bool								_contentState;

//	ZQ::common::Log*					_log;
	std::string							_startTimeUTC;
	std::string							_endTimeUTC;

	std::string                         _channelName;
	std::string                         _contentId;
	std::string                         _sessId;
};

 

}
}
#endif

