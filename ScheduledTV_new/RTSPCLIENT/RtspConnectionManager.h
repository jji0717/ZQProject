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
// $Log: /ZQProjs/ScheduledTV_new/RTSPCLIENT/RtspConnectionManager.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// ===========================================================================
#pragma once

// local include
#include "RtspDaemon.h"
#include "Log.h"

using namespace ZQ::common;

/// \class RtspConnectionManager Manager class of RtspClientConnection
/// \see \class RtspClientConnection
class RtspConnectionManager
{

public:
	RtspConnectionManager(int maxdaemon, bool trace=true);
	~RtspConnectionManager(void);

public:

public:
	//////////////////////////////////////////////////////////////////////////
	// Manager control operations
	
	/// start the manager of connection, should be called before appending any connection
	bool start(void);

	/// terminate the manager of connection
	bool terminate(void);

public:
	//////////////////////////////////////////////////////////////////////////
	// Client control operations

	/// create a new client with host specified, and connect it to the server
	///@param[in] purchaseID	the channel id of this connection
	///@param[in] hostname		the name or ip address of the host
	///@param[in] hostport		the destination port of the host
	///@param[in] nsec			the timeout span, in msec
	///@param[in] createnew		if true and there is already an client with this purchase id, return the exited one; if false, create a new one anyway 
	///@return the pointer to the client on success, or NULL on failure
	RtspClient*		createClient(DWORD purchaseID, const char* hostname, int hostport=RTSP_DEFAULT_PORT, int nsec=RTSP_DEFAULT_NSEC, bool createnew=false);


	/// remove a client 
	///@param[in] exclient		the pointer to the client need remove
	///@return	ture on success, or false when no connection with session_id found
	bool			removeClient(RtspClient*	exclient);

	/// get client containing the purchase id pid
	///@param[in] pid		the specified purchase id
	///@return the client pointer
	RtspClient*		getClientByPurchase(DWORD pid);

	/// get client containing the socket fd
	///@param[in] fd		the specified socket description
	///@return the client pointer
	RtspClient*		getClientBySocket(SOCKET fd);

private:
	//////////////////////////////////////////////////////////////////////////
	// internal daemon control methods

	/// allocate a free daemon object
	///@return		the pointer to daemon object allocated, NULL if failed
	///@remarks		If the daemon objects have not reached _maxDaemonNum, this function will generate a new daemon and start it; \n
	///If the _maxDaemonNum has already reached, a daemon object with the least clients will be returned
	RtspDaemon*		allocateDaemon();
	
private:
	/// maximum number of deamon objects
	int							_maxDaemonNum;

	/// mutex for daemons
	ZQ::common::Mutex			_lkDaemons;	

	/// daemon pool containing RtspDaemon instances
	std::vector<RtspDaemon*>	_daemons;	

	/// manager state
	bool			_IsRunning;		

	/// trace flag
	bool			_trace;
	
};
