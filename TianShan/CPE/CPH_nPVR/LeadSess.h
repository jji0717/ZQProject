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

#ifndef ZQTS_CPE_LEADSESS_H
#define ZQTS_CPE_LEADSESS_H


#include "LeadSessI.h"
#include "SubjectWrite.h"
#include "NativeThreadPool.h"
#include "WriteNotificationI.h"
#include "FileIo.h"

namespace ZQTianShan 
{
namespace ContentProvision
{

class FileIoFactory;

class LeadSess : public LeadSessI,
	protected ZQ::common::ThreadRequest,
	protected WriteNotificationI
{
public:
	LeadSess(ZQ::common::NativeThreadPool* pool, ZQ::common::BufferPool* pAlloc, FileIoFactory* pFileIoFactory);

	virtual ~LeadSess();

	virtual bool initialize();
	virtual bool execute();

	virtual void stop();
	virtual void uninitialize();

	virtual void setLog(ZQ::common::Log* pLog);

	virtual bool getMediaInfo(MediaInfo& mInfo);

	virtual void setEventHandle(LeadSessEvent* pEvent)
	{
		_pEvent = pEvent;
	}

	void makeReservation(ObserverWriteI* pObserver);

	//inherited from the subjectwrite
	virtual void registerObserver(ObserverWriteI* pObserver);

	virtual bool isInitialized();

	virtual bool isExecuted();

protected:
	///< from BaseGraph
	
	void OnMediaInfoParsed(MediaInfo& mInfo);

	///< from ThreadRequest
	virtual int run(void); 

	///< from WriteNotificationI
	virtual bool notifyWrite(const std::string& file, void* pBuf, int nLen);

	virtual void final(int retcode =0, bool bCancelled =false);
	

protected:
	bool updateExtionFile(const std::string& strVirtualName);

protected:	
	MediaInfo			_mediaInfo;
	bool				_bMediaInfoReady;
	ZQ::common::Mutex	_lock;

	FileIoFactory*		_pFileIoFactory;

	LeadSessEvent*		_pEvent;
	HANDLE				_hThreadExited;
	std::string         _strLocalIP;

	bool				_bInitialized;
	bool				_bExecuted;
	bool				_bStoped;
};



}
}

#endif