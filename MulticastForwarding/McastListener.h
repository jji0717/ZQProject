// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: McastListener.h,v 1.4 2004/07/27 09:40:04 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : define the multicast listener
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/McastListener.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 11    04-09-14 16:29 Hui.shao
// fixed buffer size to recv()
// 
// 10    04-09-14 12:16 Kaliven.lee
// 
// 9     04-09-09 17:46 Kaliven.lee
// 
// 8     04-08-27 9:52 Kaliven.lee
// 
// 7     04-08-26 11:04 Kaliven.lee
// add destruct function define
// 
// 6     04-08-22 14:49 Jian.shen
// Revision 1.4  2004/07/27 09:40:04  shao
// renamed paramter name
//
// Revision 1.3  2004/07/22 06:17:48  shao
// adjusted some method/paramter names, and added comments
//
// Revision 1.2  2004/07/07 11:43:19  shao
// build the relationship to McastFwd
//
// Revision 1.1  2004/06/24 12:34:05  shao
// created
//
// ===========================================================================

#ifndef	__ZQ_McastListener_H__
#define	__ZQ_McastListener_H__


#include "MCastFwdConf.h"
#include "Thread.h"
#include "UDPSocket.h"
#include "getopt.h"

extern DWORD gdwMaxUDPBufferSize;
class McastListener : public ZQ::common::Thread, ZQ::common::UDPReceive
{
	friend class Conversation;
	friend class MCastFwdServ;
public:
	McastListener(ZQ::common::InetMcastAddress group, int group_port, ZQ::common::InetHostAddress bind, Conversation* owner =NULL);
	~McastListener();
	void quit() { _bQuit = true; }

protected:

	ZQ::common::InetMcastAddress _group;
	int _gport;
	ZQ::common::InetHostAddress _bind;
	bool _bQuit;

	Conversation* _pOwner;

	virtual int OnCapturedData(const void* data, const int datalen, ZQ::common::InetHostAddress source, int sport);
	char _sEstablishTime[32];

	char *_buf;
	size_t _bufsz;

private:
	/// the initial steps can be put here after start() is called
	virtual bool init(void);

	/// should be Overrided! main steps for thread
	virtual int run();
};

#endif // __ZQ_McastListener_H__
