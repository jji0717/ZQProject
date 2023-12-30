// ===========================================================================
// Copyright (c) 2004 by
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

// Branch: $Name:BroadcastChService.h$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BroadcastChService.h $
// 
// 3     5/30/14 4:43p Li.huang
// 
// 2     5/30/14 3:55p Li.huang
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
#if !defined(AFX_BROADCASTCHSERVICE_H__95E43CAC_5C0B_407F_9C55_A3D4E6E2220C__INCLUDED_)
#define AFX_BROADCASTCHSERVICE_H__95E43CAC_5C0B_407F_9C55_A3D4E6E2220C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (disable : 4251 4275) 

#ifdef ZQ_OS_MSWIN
#include "BaseZQServiceApplication.h"
#else
#include <ZQDaemon.h>
#endif

#include <string>
#include "BroadcastChCfg.h"
#include "IceLog.h"
#include "BroadCastChannelEnv.h"

class BroadcastChService : public ZQ::common::BaseZQServiceApplication 
{
public:
	BroadcastChService(void);
	~BroadcastChService(void);
protected: 
	HRESULT OnInit();
	HRESULT OnStart();
	HRESULT OnStop();
	HRESULT OnUnInit();
protected:
	bool initMiniDump();

private:
	Ice::CommunicatorPtr							_communicator;
	ZQBroadCastChannel::BroadCastChannelEnv*		_pBcastCHSvcEnv;	
	TianShanIce::common::IceLogIPtr				_icelog;
	Ice::PropertiesPtr								_properties;
	ZQ::common::FileLog*                             icelog;
};
#endif // !defined(AFX_BROADCASTCHSERVICE_H__95E43CAC_5C0B_407F_9C55_A3D4E6E2220C__INCLUDED_)
