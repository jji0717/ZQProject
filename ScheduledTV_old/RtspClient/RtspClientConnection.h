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
// Ident : $Id:  $
// Branch: $Name:  $
// Author: Bernie Zhao (Tianbin Zhao)
// Desc  : rtsp client connection definition
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspClientConnection.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 10    05-03-24 14:51 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 9     05-03-08 16:53 Bernie.zhao
// upon version 0.4.0.0
// 
// 8     04-12-06 20:55 Bernie.zhao
// socket reset improved
// 
// 7     04-10-15 16:14 Bernie.zhao
// 
// 6     04-10-14 14:00 Bernie.zhao
// 
// 5     04-10-07 16:52 Bernie.zhao
// added connection query interface with purchase id
// 
// 4     04-10-07 16:01 Bernie.zhao
// fixed TEARDOWN problem
// 
// 3     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// 2     04-09-06 9:38 Bernie.zhao
// make getDynamicMac() static method
// 
// ===========================================================================
#pragma once

#include "ScLog.h"
#include "Locks.h"

// local include
#include "RtspRequest.h"
#include "RtspResponse.h"
#include "../PlaylistMod/STVPlaylist.h"

using namespace ZQ::common;
class RtspConnectionManager;

/// \class RtspClientConnection class stands for a connection
/// \see \class RtspConnectionManager
class RtspClientConnection
{
protected:
	friend class RtspConnectionManager;
	friend class RtspRecvSink;
	
public:
	/////////////////////////////////// Constructors and destructor ///////////////////////////////////////

	/// constructor, create a dummy connection
	RtspClientConnection(RtspConnectionManager* manager);

	/// destructor
	~RtspClientConnection(void);

public:
	/////////////////////////////////// initialization and termination ///////////////////////////////////////
	
	/// initialize the connection and connect to host, ready for operations
	///@param hostname		is the name or ip address of the host
	///@param hostport		is the destination port of the host
	///@param freq			is the frequence for listening server
	///@param nsec			is the timeout span
	///@return				true if initialization success, else false
	bool init(SOCKET sd, const char* hostname, const char* hostport, int nsec  = DEFAULT_NSEC);

	/// terminate the connection and reset to dummy connection
	bool cease(void);
	
	/// reset to active connection
	bool reset(void);

public:
	/////////////////////////////////// Attribute methods ///////////////////////////////////////

	/// get RtspConnectionManager object which controls this connection
	///@return				the pointer to RtspConnectionManager object
	RtspConnectionManager*	getManager() { return _CManager; }

	/// get session id of this connection
	///@return				the session id of this connection
	std::string	getSession() { return _SetupSession; }

	/// clear session id to empty string
	void clearSession() { _SetupSession = ""; }
	
	/// get purchase id of this connection
	///@return				the purchase id of this connection
	DWORD getPurchase() { return _PurchaseID; }

	/// set purchase id of this connection
	///@param pid			the purchase id of this connection
	void	setPurchase(DWORD pid) { _PurchaseID = pid; }

	/// get current playlist
	STVPlaylist*	getCurrentList() { return _pCurrrentList; }

	/// set current playlist
	void	setCurrentList(STVPlaylist* pList) { _pCurrrentList = pList; }
	
	/// get active state of this connection
	///@return				true if connection is active, false on dummy state
	bool		isActive() { return _ActiveConn; }

	/// get socket of this connection
	///@return				socket description of this connection
	SOCKET		getSocket() { return _sd; }


	/// Increase the CSeq of session 
	///@return the current CSeq number, 0 if session is first referenced
	///@remarks		after this call, the current CSeq of the session was increased by 1
	int increCSeq() { int ret = _CSeq;  _CSeq++;  return ret; }

	/// get CSeq
	///@return the current CSeq number
	int	getCSeq() { return _CSeq; }

	/// update the current state
	///@return the current state before update
	///@state			is the update aim state
	int updateState(int state) { int ret = _State; _State = state; return ret;}

	/// get the current state
	///@return the current state
	int getState() { return _State; }

	/// update the current Keep alive heartbeat rate
	///@return the current keep alive before update
	///@keepalive		is the update aim number
	int updateKeepAlive(int keepalive) { int ret=_Heartbeat; _Heartbeat = keepalive; return ret;}

	/// get the current Keep alive heartbeat rate
	//@return the current keep alive
	int getKeepAlive() { return _Heartbeat; }

public:
	/////////////////////////////////// RTSP METHOD send functions ///////////////////////////////////////
	
	/// send an formatted RTSP message such as SETUP message or PLAY message
	///@return true on success and false on failure within _NSec time limit
	///@reqmsg		is the request message that needs sent
	///@feedback	is the response of this message from server
	///#Remark!		you can ignore the "CSeq" and "Session" fields, this function will automatically fills it
	virtual bool sendMSG(RtspRequest	reqmsg, RtspResponse	&feedback);

	/// send a keepalive signal to server
	///@return true on success and false on failure
	virtual bool sendKeepAlive();

	/// get the mac addr of multicast ip
	///@return the mac address
	///@ipaddr			is the multicast ip address
	static std::string getDynamicMac(std::string ipaddr);

public:
	//////////////////////////////////// virtual functions invoked by listening thread //////////////////////////////////////
	////////////////////////////////////           that can be overrided			   //////////////////////////////////////

	/// function for handling when announce message arrived
	virtual bool OnAnnounce(RtspRequest annc);
	
	/// function for handling when response message arrived
	virtual bool OnResponse(RtspResponse resp);
	
	/// function for handling when request message should be sent
	virtual bool OnRequest();
	
	/// function for handling when network down or keep alive timeout
	virtual bool OnRecover(int errornum);

	/// function for handling when a TEARDOWN request succeeded or this connection needs reset
	virtual bool OnDestroy(void);

public:
	/// set event for getting response from server
	void OnSetGotResponse(void) {
		_eventLock.enter();
		::SetEvent(_hGotOrFail[0]);
		_eventLock.leave();
	}

protected:
	
	std::string		_HostName;	/// host name for connection

	std::string		_HostPort;	/// host port for connection

	int		_NSec;	/// time out attributes, in milli second

	int				_CSeq;		/// receive frequence, every _Frequence milli-second

	std::string		_SetupSession;	/// session id, to identify unique connection

	DWORD		_PurchaseID;	/// purchase id, to identify unique channel

	int				_State;		/// connection state

	bool		_isBlocking;	/// indicate if is waiting for someting

	int		_Heartbeat;		/// heartbeat time
	
	bool	_ActiveConn;	/// dummy connection with _ActiveConn set to FALSE

	RtspSock*	_pRtspSd;	/// rtsp socket of this connection
	SOCKET		_sd;		/// socket description of this connection

	RtspRequest		_SetupMsg;	/// session SETUP message backup, for recovery if necessary
	
	RtspResponse	_RecvMsg;	/// receive message buff

	RtspRequest		_WaitMsg;	/// send message buff

	RtspRequest		_SentMsg;	/// last sent message back, for verification of "TEARDOWN" message

	HANDLE			_hSentOrFail[2];	/// Handle for request message signal event, or send fail

	HANDLE			_hGotOrFail[2];		/// Handle for response message arrived signal event, or recv fail

	ZQ::common::Mutex _msgLock;	/// mutex for thread message synchronization
	ZQ::common::Mutex _eventLock; /// mutex for thread event synchronization

	RtspConnectionManager*	_CManager;	/// connection manager which holds control of this connection

	STVPlaylist*	_pCurrrentList;		/// current playlist related to this connection
	
};
