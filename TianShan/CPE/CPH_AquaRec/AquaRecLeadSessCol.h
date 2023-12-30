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

#ifndef ZQTS_CPE_AquaRecLEADSESSCOL_H
#define ZQTS_CPE_AquaRecLEADSESSCOL_H



#include "AquaRecLeadSessI.h"
#include "AquaRecLeadSessColI.h"
#include <map>
#include "Locks.h"
#include "NativeThread.h"
#include "SystemUtils.h"

namespace ZQ
{
	namespace common{
		class NativeThreadPool;
	}
}

class CdmiClientBase;

namespace ZQTianShan 
{
	namespace ContentProvision
	{

class AquaRecLeadSessFac;


class AquaRecLeadSessCol : public AquaRecLeadSessColI, protected AquaRecLeadSessEvent, protected ZQ::common::NativeThread
{
public:
	friend class AquaRecLeadSessI;
	
	AquaRecLeadSessCol(ZQ::common::NativeThreadPool* pool, ZQ::common::Log* pLog, CdmiClientBase* cdmiClient);

	virtual ~AquaRecLeadSessCol();

	virtual bool startMonitor();
	virtual void stopMonitor();

	virtual bool addVirtualSession(AquaRecVirtualSessI* pVirtualSess);
	virtual bool removeVirtualSession(AquaRecVirtualSessI* pVirtualSess);

	virtual bool updateScheduledTime(AquaRecVirtualSessI* pVirtualSess);

	virtual bool getMediaInfo(AquaRecVirtualSessI* pVirtualSess, MediaInfo& mInfo);

protected:

	virtual void onDestroy(AquaRecLeadSessI* pLeadSess);

	void close();
	void stopAllSession();

	// find session, create & start session if not exist
	AquaRecLeadSessI* getLeadSession(AquaRecVirtualSessI* pVirtualSess);

	virtual int run(void);

	virtual void monitorIdleSession();

	AquaRecLeadSessI* find(const std::string& chId);
	AquaRecLeadSessI* create(const std::string& chId);
	void remove(const std::string& chId);

protected:
	//map the chId to leadsess
	typedef std::map<std::string, AquaRecLeadSessI*>	LeadSessList;
	LeadSessList				_leadSessLists;
	ZQ::common::Mutex			_lockLeadsessLists;

	AquaRecLeadSessFac*			_lsFac;

	//HANDLE						_hThreadExit;
	SYS::SingleObject				_hThreadQuited;
	ZQ::common::Log*			        _log;
	ZQ::common::NativeThreadPool*		_threadPool;
};


}
}

#endif
