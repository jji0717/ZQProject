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
// Ident : $Id: Neighborhood.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Sentry/Neighborhood.h $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:40 Admin
// Created.
// 
// 8     09-12-24 18:00 Fei.huang
// * data type changed to be backward compatible 
// 
// 7     09-11-23 15:19 Xiaohui.chai
// make the SentryListener and SentryBarker NativeThread 
// 
// 6     09-08-20 15:18 Fei.huang
// * use the same message code for both linux and windows
// 
// 5     09-07-06 14:30 Fei.huang
// * linux port
// 
// 4     08-06-10 11:31 Xiaohui.chai
// added machine type to multicast message
// 
// 3     07-11-06 18:34 Xiaohui.chai
// 
// 2     07-05-22 17:30 Hui.shao
// added exporting logger information
// ===========================================================================

#ifndef __ZQTianShan_Node_Neighborhood_H__
#define __ZQTianShan_Node_Neighborhood_H__

#include "../common/TianShanDefines.h"
#include "NativeThread.h"
#include "UDPSocket.h"

namespace ZQTianShan {
namespace Sentry {

class SentryEnv;

#define MAX_NODEMSG_LEN (4096)

// message codes
#if defined(ZQ_OS_MSWIN) || defined(ZQ_OS_LINUX)
#  define _MSGCODE(_B)			((_B & 0xff)<<8 | 0x6e)
#else
#  define _MSGCODE(_B)			((_B & 0xff) | 0x6e00)
#endif

#define MSGCODE_NEIGHBOR		_MSGCODE(1)

// -----------------------------
// class SentryListener
// -----------------------------
///
class SentryListener : virtual public ZQ::common::NativeThread, ZQ::common::UDPReceive
{
public:
	/// constructor
    SentryListener(SentryEnv& env);
	virtual ~SentryListener();

	//quit listening
	void quit();

	typedef SentryListener* Ptr;

protected:

	// impls of NativeThread
	virtual bool init(void);
	virtual int run(void);

	void enableMsgDump(bool enable=true) {_bMsgDump = enable; }

	// received message, UDPReceive
	int OnCapturedData(const void* data, const int datalen, ZQ::common::InetHostAddress source, int sport);

protected:

	SentryEnv&   _env;
	bool		  _bQuit;
	bool		  _bMsgDump;

	char	_buf[MAX_NODEMSG_LEN+2];
};

// -----------------------------
// class SentryBarker
// -----------------------------
///
class SentryBarker : virtual public ZQ::common::NativeThread, ZQ::common::UDPMulticast
{
public:
	/// constructor
    SentryBarker(SentryEnv& env);
	virtual ~SentryBarker();

	//quit barking
	void quit();
	void enableMsgDump(bool enable=true) {_bMsgDump = enable; }

	void refreshMsg();

	typedef SentryBarker* Ptr;

protected:

	// impls of NativeThread
	virtual bool init(void);
	virtual int run(void);

	void wakeup();

protected:

	friend class SentryListener;

	typedef struct _NodeMcastMsg
	{
		uint16	msgCode; // should always be MSGCODE_NEIGHBOR
		uint16	msgLen;
		char	nodeId[32];
		char	name[40];
		int64 stampLastChange;
        int64 stampOSStartup;
		uint32  memTotalPhys, memAvailPhys, memTotalVirtual, memAvailVirtual;
		char    servAddrs[80]; // served local addresses delimited by ' '
		char	sentrysvcPrx[80]; // the ice proxystring to the peer nodesvc
        char    type[16]; // the type of the node
	} NodeMcastMsg;

	SentryEnv&   _env;
	bool		  _bQuit;
	bool		  _bMsgDump;
    ZQ::common::Semaphore _wakeup;
	NodeMcastMsg  _msg; 	ZQ::common::Mutex _lockMsg;
};

}} // namespace

#endif // __ZQTianShan_Node_Neighborhood_H__

