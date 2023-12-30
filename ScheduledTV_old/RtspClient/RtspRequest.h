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
// Archive : $Archive: /ZQProjs/ScheduledTV_old/RtspClient/RtspRequest.h $
// Branch: $Name:  $
// Author: Bernie Zhao (Tianbin Zhao)
// Desc  : rtsp request message definition
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspRequest.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 4     05-06-27 18:07 Bernie.zhao
// fixed interface error with SM when creating new STV list
// 
// 3     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// 2     04-08-18 14:51 Bernie.zhao
// 
// ===========================================================================
#pragma once
#include "rtspmessage.h"

/// \class RtspRequest RTSP request message class
class RtspRequest :
	public RtspMessage
{
protected:
	std::string	_commandtype;
	std::string	_commandtarget;
	std::string	_commandversion;
public:
	RtspRequest(void);
	RtspRequest(const RtspMessage& msg);
	RtspRequest(std::string command);
	RtspRequest(std::string type, std::string target, std::string version="RTSP/1.0");

	~RtspRequest(void);
	

public:
	std::string	getCommandtype() { return _commandtype; }
	void		setCommandtype( std::string param) { _commandtype = param; }
	std::string	getCommandtarget() { return _commandtarget; }
	void		setCommandtarget( std::string param) { _commandtarget = param; }
	std::string	getCommandversion() { return _commandversion; }
	void		setCommandversion( std::string param) { _commandversion = param; }

public:
	void parseCommand(std::string command);

	static RtspRequest parseRequest(const char* messagebytes);

public:
	// overwrite functions
	/// send this message via sockd
	///@return the number of bytes sent on success and -1 on failure
	///@param sockd		is the RtspSock instance of the connection
	///@param flag		is the way that socket send data
	virtual int sendMessage(RtspSock* sockd,int flag=0);

	/// send this message 
	/// and append data stream until '\0'
	///@return the number of bytes sent on success and -1 on failure
	///@param sockd		is the RtspSock instance of the connection
	///@param data		is the stream that needed to be appended after this message
	///@param flag		is the way that socket send data
	virtual int sendMessage(RtspSock* sockd, char* data, int flag=0);

	/// receive data from sock into this message and parse it
	///@return the number of bytes received on success and -1 on failure
	///@param sockd		is the RtspSock instance of the connection
	virtual int recvMessage(RtspSock* sockd);

	// clear this request message
	virtual void	clearMessage();
};
