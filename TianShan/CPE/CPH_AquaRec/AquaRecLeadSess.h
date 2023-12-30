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

#ifndef ZQTS_CPE_AQUARECLEADSESS_H
#define ZQTS_CPE_AQUARECLEADSESS_H

#include "AquaRecLeadSessI.h"
#include "AquaRecVirtualSessI.h"
#include "NativeThreadPool.h"
#include "SystemUtils.h"
#include "TianShanIce.h"

class CdmiClientBase;

namespace ZQTianShan 
{
namespace ContentProvision
{
class AquaRecLeadSess : public AquaRecLeadSessI,
	protected ZQ::common::ThreadRequest
{
public:
	AquaRecLeadSess(ZQ::common::NativeThreadPool* pool, CdmiClientBase* pcdmiClient);

	virtual ~AquaRecLeadSess();

	virtual bool initialize();
	virtual bool execute();

	virtual void stop();
	virtual void uninitialize();

	virtual void setLog(ZQ::common::Log* pLog);

	virtual bool isIdle();

	// in mili-second
	virtual uint64 getIdleTime();

	virtual bool updateScheduledTime(AquaRecVirtualSessI* pVirtualSess);
	
	virtual bool add(AquaRecVirtualSessI* pVirtualSess);
	virtual bool remove(AquaRecVirtualSessI* pVirtualSess);

	virtual void setChannelName(const std::string& strChannelName)
	{
		_channnelName = strChannelName;
		_strLogHint = strChannelName;
	};
	virtual std::string getChannelName()
	{
		return _channnelName;
	};

	virtual bool getMediaInfo(MediaInfo& mInfo);

	virtual void setEventHandle(AquaRecLeadSessEvent* pEvent)
	{
		_pEvent = pEvent;
	}
   /*   virtual void setBitrateContainers()
	{
		readBitratesFolder(_bitrateContainers);
	}*/
	virtual bool isInitialized();

	virtual bool isExecuted();
protected:
	///< from BaseGraph
	
	void OnMediaInfoParsed(MediaInfo& mInfo);

	///< from ThreadRequest
	virtual int run(void); 

	virtual void final(int retcode =0, bool bCancelled =false);

private:
	//without lock
	bool _isIdle();
	bool updateSubscribers();

	bool readBitratesFolder(TianShanIce::StrValues& currentSourceBitrates);
	bool createBitratesFolder(TianShanIce::StrValues& currentSourceBitrates);
	bool updateBFSubscribers(TianShanIce::StrValues& bitrateContainers);
	
	bool createFolder( AquaRecVirtualSessI* pVirtualSess ,const std::string &bitrate);
	void compareBitrates(TianShanIce::StrValues& currentSourceBitrates,TianShanIce::StrValues& newSourceBitrates);
protected:	
	MediaInfo			_mediaInfo;
	bool				_bMediaInfoReady;
	ZQ::common::Mutex	_lock;

	AquaRecLeadSessEvent*		_pEvent;
	//HANDLE				_hThreadExited;
	SYS::SingleObject		_hThreadQuit;
	SYS::SingleObject		_hLeadSessWait;
	bool				_bInitialized;
	bool				_bExecuted;
	bool				_bStoped;

	typedef std::vector <AquaRecVirtualSessI *> AquaRecVirSessLists;
	AquaRecVirSessLists     _aquaRecVirSessList;
	ZQ::common::Mutex	_lockVirSesss;
	
	CdmiClientBase*     _pCdmiClient;
	std::string         _channnelName; 
	TianShanIce::StrValues   _bitrateContainers;

	uint64		_lastIdleStamp;
};

}
}

#endif
