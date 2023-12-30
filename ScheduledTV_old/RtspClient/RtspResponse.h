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
// Desc  : rtsp response message definition
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspResponse.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 3     05-06-27 18:07 Bernie.zhao
// fixed interface error with SM when creating new STV list
// 
// 2     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// ===========================================================================
#pragma once
#include "rtspmessage.h"

/// \class RtspResponse RTSP response message class
class RtspResponse :
	public RtspMessage
{
protected:
	std::string	_commandversion;
	std::string	_commandstatus;
	std::string	_commandmessage;
public:
	RtspResponse(void);
	RtspResponse(const RtspMessage& msg);
	RtspResponse(std::string command);
	RtspResponse(std::string version, std::string status, std::string message);
	~RtspResponse(void);
	
public:
	std::string	getCommandversion() { return _commandversion; }
	void		setCommandversion( std::string param) { _commandversion = param; }
	std::string	getCommandstatus() { return _commandstatus; }
	void		setCommandstatus( std::string param) { _commandstatus = param; }
	std::string	getCommandmessage() { return _commandmessage; }
	void		setCommandmessage( std::string param) { _commandmessage = param; }

public:
	void parseCommand(std::string command);

	static RtspResponse parseResponse(const char* messagebytes);

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

	// clear this response message
	virtual void	clearMessage();
};
