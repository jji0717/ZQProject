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
// Ident : $Id: McastFwd.h,v 1.3 2004/07/22 06:17:48 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : define the main service class McastFwd
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/McastFwd.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 10    04-11-08 18:03 Hui.shao
// switched listener to sniffer
// 
// 9     04-09-09 10:33 Kaliven.lee
// make some change so that service class can inherit from mcastfwd
// 
// 8     04-08-23 8:42 Jian.shen
// 
// 6     04-08-19 10:59 Kaliven.lee
 //* 
 //* 5     04-08-18 16:03 Jinming.wang
 //* 
 //* 4     04-08-18 15:02 Jinming.wang
// Revision 1.3  2004/07/22 06:17:48  shao
// adjusted some method/paramter names, and added comments
//
// Revision 1.2  2004/07/20 13:30:19  shao
// 2nd stage impl
//
// Revision 1.1  2004/07/07 11:42:22  shao
// created
//
// ===========================================================================

#include "McastFwdConf.h"

#include "Conversation.h"
//#include "BaseSchangeServiceApplication.h"
#include "tunnel.h"


// -----------------------------
// class McastFwd
// -----------------------------

class McastFwd : protected Conversation
{
friend class MCastFwdServ;
public:

	McastFwd();
	~McastFwd();

	/// Initial the conversation with a preference
	/// @param preference      point to a preference instance which define this coversation
	/// @param checkPrefName   false if ignore the top preference name
	/// @return                true if successful
	bool initialize(ZQ::common::IPreference* preference, bool checkPrefName=true, int tunnelListenPort = DEFAULT_TUNNEL_LISTERN_PORT);

	/// Start this conversation, the conversation must be initialized and valid
	/// @return                true if successful
	virtual bool start();

	/// Stop this conversation, the conversation can not be start once stopped
	/// @return                true if successful
	virtual bool stop();

	/// Check if the server is initialized successfully
	/// @return                true if valid
	virtual bool isValid();

	/// get this server instance
	/// @return                point to self
	McastFwd* getServer() { return this; }


	int defaultListenAddressCount() { return _listenAddrs.size(); }
	ZQ::common::InetHostAddress defaultListenAddress(int index)
	{ 
		static ZQ::common::InetHostAddress dumyAddr;
		if (index>=0 && index< defaultListenAddressCount())
			return _listenAddrs[index];
		return dumyAddr;
	}

//	int mcastData(const void* message, const int len, ZQ::common::InetMcastAddress& group, const int gport);

	//// Add by Wang Jinming
	//const static int _listenBufSize = 0;
	//static int GetBufSize() { return _listenBufSize;}

	McastSniffer _theSniffer;

protected:

/*
/// Event fired when the listener interface captured a message on the conversation group/port
	/// @param message         message to send
	/// @param len             length of the message
	/// @param source          the source machine where the message was sent from
	/// @param sport           the source port that the message was sent
	/// @param group           the target multicast group
	/// @param gport           the target multicast port
	/// @return                byte of the message that has been processed
	int OnCapturedData(const void* data, const int datalen, ZQ::common::InetHostAddress& from, int sport, ZQ::common::InetMcastAddress* group, int gport);

	/// Check if the source IP/port is denied
	/// @param message         message to send
	/// @param len             length of the message
	/// @param source          the source machine where the message was sent from
	/// @param sport           the source port that the message was sent
	/// @param group           the target multicast group
	/// @param gport           the target multicast port
	/// @return                byte of the message that has been processed
	bool isDenied(ZQ::common::InetHostAddress& from, int sport);
*/
	typedef std::vector < Conversation* > conversations_t;
	conversations_t _conversations;
	ZQ::common::Mutex _conversationsLock;

	typedef std::vector < ZQ::common::InetHostAddress > addrs_t;
	addrs_t _listenAddrs;
	ZQ::common::Mutex _listenAddrsLock;

	int _tunnelListenPort;
};

