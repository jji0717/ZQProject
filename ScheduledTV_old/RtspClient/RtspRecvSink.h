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
// Desc  : rtsp receive sink thread definition
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspRecvSink.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 9     05-06-09 10:16 Bernie.zhao
// 
// 8     05-03-24 14:52 Bernie.zhao
// version 0.4.3. Release to Maynard
// 
// 7     05-03-08 16:54 Bernie.zhao
// upon version 0.4.0.0
// 
// 6     04-12-06 20:55 Bernie.zhao
// socket reset improved
// 
// 5     04-11-03 19:59 Bernie.zhao
// Oct/3 Before Service Release
// 
// 4     04-10-14 13:54 Bernie.zhao
// enable listening frequency configurable
// 
// 3     04-10-07 16:01 Bernie.zhao
// fixed TEARDOWN problem
// 
// 2     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// ===========================================================================
#pragma once

// common include
#include "ZqThread.h"
#include "Locks.h"

#include <queue>

// local include
#include "RtspConnectionManager.h"

class RtspConnectionManager;

/// \class RtspRecvSink listening thread for Rtsp sink
class RtspRecvSink :
	public ZQ::comextra::Thread
{
public:
	RtspRecvSink(RtspConnectionManager* manager, int freq = MAX_SELECTTIME);
	~RtspRecvSink(void);

	//////////////////////////////////////////////////////////////////////////
	
	// override of Thread functions
	virtual int run();

	//////////////////////////////////////////////////////////////////////////
	
	// get the next part of a string until reached double CRLF
	///@return the position of next part(after double CRLF)
	///@param bytes		is the original string
	char* splitNextPart(char* bytes);

	/// check sockets in the set and receive data or recover connection
	///@return	sockets number that had been handled
	int handleResponse(fd_set* socketset);

	/// check all sockets and send heartbeat on the sockets
	///@return	sockets number that had been handled
	int handleHeartbeat();
	
	/// send request to server
	///@return	sockets number that hand been handled
	int handleRequest();

	//////////////////////////////////////////////////////////////////////////
	
	/// update socket set for recving response
	//?@return	socket number that updated
	int sockSetUpdate();
	

	/// push a socket into the queue
	///@param[1n] sd	the socket to push
	void	waitingListPush(SOCKET sd)
	{
		_WaitingListLock.enter();
		_WaitingList.push(sd);
		_WaitingListLock.leave();
	}

	/// pop a socket out of the queue
	///@return	the next socket popped out
	SOCKET	waitingListPop()
	{
		SOCKET sd = INVALID_SOCKET;
		_WaitingListLock.enter();
		if(!_WaitingList.empty())
		{
			sd = _WaitingList.front();
			_WaitingList.pop();
		}
		_WaitingListLock.leave();
		return sd;
	}

	//////////////////////////////////////////////////////////////////////////
	
	/// signal event that data is ready to send
	void OnSetWillSend() { 
		_eventLock.enter();
		::SetEvent(_hOverOrSend[1]); 
		_eventLock.leave();
	}

	/// signal event that listening thread should terminate
	void OnSetListenOver() { 
		_eventLock.enter();
		::SetEvent(_hOverOrSend[0]); 
		_eventLock.leave();
	}

private:
	RtspConnectionManager*	_man;

	/// event when listening thread should terminate or when data is ready to send
	HANDLE	_hOverOrSend[2];

	/// listening frequency
	int _Freq;
	
	/// mutex object
	ZQ::common::Mutex	_eventLock;

	//////////////////////////////////////////////////////////////////////////
	
	/// queue for sending request
	std::queue<SOCKET>	_WaitingList;

	/// mutex for waiting list
	ZQ::common::Mutex	_WaitingListLock;

	//////////////////////////////////////////////////////////////////////////
	
	/// socket set for recving response
	fd_set				_sockSet;

};
