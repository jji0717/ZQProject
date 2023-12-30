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
// Desc  : rtsp connection control class definition
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspConnectionManager.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 9     05-06-27 18:07 Bernie.zhao
// fixed interface error with SM when creating new STV list
// 
// 8     05-03-24 14:51 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 7     04-12-06 20:55 Bernie.zhao
// socket reset improved
// 
// 6     04-10-15 16:14 Bernie.zhao
// 
// 5     04-10-14 14:00 Bernie.zhao
// 
// 4     04-10-07 16:52 Bernie.zhao
// added connection query interface with purchase id
// 
// 3     04-10-07 16:01 Bernie.zhao
// fixed TEARDOWN problem
// 
// 2     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// ===========================================================================
#pragma once

// local include
#include "RtspClientConnection.h"
#include "RtspRecvSink.h"
#include "ScLog.h"
#include "Locks.h"

using namespace ZQ::common;

/// \class RtspConnectionManager Manager class of RtspClientConnection
/// \see \class RtspClientConnection
class RtspConnectionManager
{
protected:
	friend class RtspRecvSink;
	
	RtspRecvSink*	_Listener;		/// sink thread, invoke all data recv() and send() in the thread, to avoid operation to sockets in different threads

protected:
	int				_ConnNum;		/// connection number

	ZQ::common::Mutex	_ConnMutex;	/// mutex for resource

	std::vector<RtspClientConnection*>	_ConnPool;	/// connection pool containing RtspClientConnection instances

	std::vector<SOCKET>	_SockPool;	/// sockets for each connection

	int				_HeartbeatSec;	/// server keep alive second, initialized to DEFAULT_KEEPALIVESEC

	int				_Frequence;		/// listening frequence, in milli-second

	bool			_IsRunning;		/// manager state
	
public:
	RtspConnectionManager(void);
	~RtspConnectionManager(void);

public:
	/////////////////////////////////// Attribute methods ///////////////////////////////////////
	/// get listening thread
	///@return		pointer to listening thread
	RtspRecvSink* getListener() { return _Listener; }
	
	/// get connection numbers
	///@return		connection numbers in pool
	int		getConnNum() { return _ConnNum; }
	void	setConnNum( int connnum ) { _ConnNum = connnum; }

	/// get listening frequence
	///@return		listening frequence
	int		getFrequence() { return _Frequence; }

	/// set listening frequence
	///@param freq		the value of listening frequence
	void	setFrequence(int freq)
	{ 
		_Frequence = freq;
		glog(ZQ::common::Log::L_DEBUG, "SUCCESS  RtspConnectionManger::setFrequence()  Listenging frequence had been set to %d milli-second", _Frequence);
	}
	
	/// get connection heart beat keepalive time, in second
	///@return		heartbeat second
	int		getHeartbeat() { return _HeartbeatSec; }
	/// set connection heart beat keepalive time, in second
	///@param heartbeat	the heartbeat second to setup
	void	setHeartbeat( int heartbeat ) { _HeartbeatSec = heartbeat; }

	/// update old socket with new socket
	///@return	true if update successfully, false if connection not found
	///@param[in] pConn	the connection whose socket need update
	bool	updateSock(RtspClientConnection* pConn );
	/// remove specific socket from pool
	///@return	true if remove successfully, false if socket not found
	///@param sd	the socket to remove
	bool	eraseSock( SOCKET sd);
public:
	/////////////////////////////////// Manager control methods ///////////////////////////////////////
	
	/// start the manager of connection, should be called before appending any connection
	bool start(void);

	/// terminate the manager of connection
	bool terminate(void);

public:
	/////////////////////////////////// Connection control methods ///////////////////////////////////////

	/// create a new connection with host specified, and add it to Connection Pool if succeeded
	///@return the pointer to the connection on success, or NULL on failure
	///@param hostname		the name or ip address of the host
	///@param hostport		the destination port of the host
	///@param purchaseID	the channel id of this connection
	///@param freq			the frequence for listening server
	///@param nsec			the timeout span
	RtspClientConnection*	createConn(const char* hostname, const char* hostport, DWORD purchaseID,int nsec  = DEFAULT_NSEC);

	/// create and SETUP a new connection with host specified, and add it to Connection Pool if succeeded
	///@return the session-id of this connection on success, or empty string "" on failure
	///@param setupmsg		is the SETUP message
	///@param hostname		is the name or ip address of the host
	///@param hostport		is the destination port of the host
	///@param purchaseID	the channel id of this connection
	///@param freq			is the frequence for listening server
	///@param nsec			is the timeout span
	std::string	createConnEx(RtspRequest setupmsg, const char* hostname, const char* hostport, DWORD purchaseID,int nsec  = DEFAULT_NSEC);

	/// remove a connection from the pool and cease it
	///@return	ture on success, or false when no connection with session_id found
	///@param session_id		is the session id of the connection need remove
	bool		removeConn(std::string session_id);

	/// get connection containing the session-id session
	///@return the connection pointer
	///@param session_id		is the specified session string
	RtspClientConnection* getConnBySession(std::string session_id);

	/// get connection containing the purchase id pid
	///@return the connection pointer
	///@param pid		is the specified purchase id
	RtspClientConnection* getConnByPurchase(DWORD pid);

	/// get connection containing the sd Socket
	///@return	the connection pointer
	///@param sd				is the socket description of the connection searching for
	RtspClientConnection* getConnBySocket(SOCKET sd);
	
public:
	/////////////////////////////////// methods for listening thread call ///////////////////////////////////////
	
	/// function for handling when announce message arrived
	/// should be overrided for user purpose
	virtual bool OnAnnounce(SOCKET sd, RtspRequest reqmsg);
	
	/// function for handling when request message should be sent
	/// may be overrided for user purpose
	virtual bool OnRequest(SOCKET sd);

	/// function for handling when response message arrived
	/// may be overrided for user purpose
	virtual bool OnResponse(SOCKET sd, RtspResponse resmsg);
	
	/// function for handling when network down or keep alive timeout
	/// may be overrided for user purpose
	virtual bool OnRecover(SOCKET sd, int errnum);

	/// function for handling when a TEARDOWN request succeeded or any connection needs reset
	/// may be overrided for user purpose
	virtual bool OnDestroy(SOCKET sd);
	
	
};
