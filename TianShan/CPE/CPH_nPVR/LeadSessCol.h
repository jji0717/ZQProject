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

#ifndef ZQTS_CPE_LEADSESSCOL_H
#define ZQTS_CPE_LEADSESSCOL_H



#include "LeadSessI.h"
#include "LeadSessColI.h"
#include <map>
#include "locks.h"
#include "NativeThread.h"

namespace ZQ
{
	namespace common{
		class NativeThreadPool;
	}
}


class IMemAlloc;



namespace ZQTianShan 
{
	namespace ContentProvision
	{

class FileIoFactory;
class LeadSessFac;


class LeadSessCol : public LeadSessColI, protected LeadSessEvent, protected ZQ::common::NativeThread
{
public:
	friend class LeadSessI;
	
	LeadSessCol(ZQ::common::NativeThreadPool* pool, ZQ::common::BufferPool* pAlloc, FileIoFactory* pFileIoFactory, ZQ::common::Log* pLog);

	virtual ~LeadSessCol();

	virtual bool startMonitor();
	virtual void stopMonitor();

	virtual bool reservation(VirtualSessI* pVirtualSess);

	virtual bool registerObserver(VirtualSessI* pVirtualSess);
	virtual void removeObserver(VirtualSessI* pVirtualSess);

	virtual bool getMediaInfo(VirtualSessI* pVirtualSess, MediaInfo& mInfo);

	virtual std::string getLeadSessPathName(VirtualSessI* pVirtualSess);


protected:

	virtual void onDestroy(LeadSessI* pLeadSess);


	void close();
	void stopAllSession();

	// find session, create & start session if not exist
	LeadSessI* getLeadSession(VirtualSessI* pVirtualSess);

	bool startLeadSession(LeadSessI* pLeadSess, VirtualSessI* pVirtualSess);

	virtual int run(void);

	virtual void monitorIdleSession();

	LeadSessI* find(const std::string& uniqueId);
	LeadSessI* create(const std::string& uniqueId);
	void remove(const std::string& uniqueId);

protected:
	typedef std::map<std::string, LeadSessI*>	SubjectListType;
	SubjectListType				_subjectList;
	ZQ::common::Mutex			_lock;

	LeadSessFac*				_lsFac;

	HANDLE						_hThreadExit;

	ZQ::common::Log*			_log;
	ZQ::common::NativeThreadPool*		_threadPool;
};


}
}

#endif