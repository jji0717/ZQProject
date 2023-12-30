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
// Ident : $Id: CPEImpl.cpp $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
//
// ===========================================================================

#ifndef _ZQTianShan_CPCCLIENT_H_
#define _ZQTianShan_CPCCLIENT_H_

#include "Locks.h"
#include "TsContentProv.h"
#include "NativeThread.h"
#include <vector>
#include <IceUtil/IceUtil.h>

class CPCClient: public ZQ::common::NativeThread
{
public:
	CPCClient();
	
	bool initModule(Ice::CommunicatorPtr ic);
	
	void unInitModule();
protected:
	int run(void);
	
	void reportInstance();
#ifdef ZQ_OS_MSWIN
	HANDLE								_stopEvent;
#else
	sem_t								_stopSem;
#endif
	bool								_bStop;
	Ice::Long							_stampStartUp;
	
	Ice::CommunicatorPtr				_ic;
	TianShanIce::ContentProvision::ContentProvisionClusterPrx		_cpcPrx;	
	TianShanIce::ContentProvision::ContentProvisionServicePrx		_cpePrx;		
	uint32								_dwInstanceLeaseTermSecs;
};


#endif
