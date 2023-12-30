// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This source was copied from shcxx, shcxx's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : define the soap interface of AssetGear for ZQ integration with ISA components
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/nPVR/AssetGear/AssetGear_soap.h 1     10-11-12 16:01 Admin $
// $Log: /ZQProjs/nPVR/AssetGear/AssetGear_soap.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// ============================================================================================

#ifndef __AssetGear_soap_H__
#define __AssetGear_soap_H__

#include "./asset_impl.h"
#include "./soapH.h"
#include "../PMClient.h"
#include "NativeThreadPool.h"

#define DEFAULT_THREAD_POOL_SZ (20)
#define DEFAULT_PORT (1200)
#define BACKLOG (100)
#define TIMEOUT (24*60*60) // timeout after 24hrs of inactivity

// -----------------------------
// class AssetGearService
// -----------------------------
class AssetGearService : public soap, public ZQ::common::NativeThread, protected ZQ::common::NativeThreadPool
{
	friend class SoapProcessRequest;
public:
	AssetGearService(const char* localIp, const int port = DEFAULT_PORT, const int thpoolsize =DEFAULT_THREAD_POOL_SZ);
	virtual ~AssetGearService();
	
protected:

	virtual bool init(void);
	virtual int run(void);
	virtual void final(void) {}

	std::string* _localIP;
	int			 _port;
	bool		 _bQuit;
};

#endif // __AssetGear_soap_H__