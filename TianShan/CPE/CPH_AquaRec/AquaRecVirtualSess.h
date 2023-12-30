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
#ifndef ZQTS_CPE_AQUARECVIRTUALSESS_H
#define ZQTS_CPE_AQUARECVIRTUALSESS_H

#include "AquaRecVirtualSessI.h"

namespace ZQTianShan 
{
namespace ContentProvision
{

class AquaRecVirtualSess : public AquaRecVirtualSessI
{
public:	
	AquaRecVirtualSess();
	virtual ~AquaRecVirtualSess();
    
	virtual bool initialize();
	virtual bool execute();
	virtual void uninitialize();

	virtual void updateScheduledTime(const std::string& startTimeUTC, const std::string& endTimeUTC);

	virtual bool getMediaInfo(MediaInfo& mInfo);
	virtual void setLog(ZQ::common::Log* pLog);
	virtual void getProgress(int64& procv, int64& total);
	virtual bool readContent(CdmiClientBase* ptrCdmiClient,std::string& bitrate);
protected:

	virtual bool makeReservation();
	
	virtual void notifyDestroy(const std::string& strErr, int nErrorCode);
	
	std::string			_strLogHint;
	bool				_bUninitialized;

//	ZQ::common::Log*	_log;
};

}
}

#endif
