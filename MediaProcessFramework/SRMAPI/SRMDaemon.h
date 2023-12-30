
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
//
// Dev  : Microsoft Developer Studio
// Name  : SRMDaemon.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-8
// Desc  : daemon
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/SRMDaemon.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 24    05-06-24 5:11p Daniel.wang
// 
// 23    05-06-14 4:58p Daniel.wang
// ===========================================================================

#ifndef _ZQ_SRMDAEMON_H_
#define _ZQ_SRMDAEMON_H_

#include "MetaNode.h"
#include "SRMSubscriber.h"
#include "SRMInfo.h"

SRM_BEGIN


// -----------------------------
//SRMDaemon
// -----------------------------
/// daemon server
class DLL_PORT SRMDaemon : public rpc::RpcServer, public common::NativeThread
{
private:
	int							m_nPort;
	std::string					m_strIp;

	SRMInfo						m_si;
public:

	///constructor
	///@param strIp - server end IP address
	///@param nPort - server end port number
	SRMDaemon(const char* strIp, int nPort);

	///constructor
	///@param strUrl - server url, format:MPF://IP:port/
	SRMDaemon(const char* strUrl);

	///destructor
	virtual ~SRMDaemon();
	
	///run
	///do daemon work here, it's a thread main content
	///@return - return error code when thread exit
	int run();
};

SRM_END

#endif//_ZQ_SRMDAEMON_H_
