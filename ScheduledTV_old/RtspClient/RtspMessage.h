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
// Desc  : rtsp general message definition
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspMessage.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 5     04-10-21 15:29 Bernie.zhao
// 
// 4     04-10-19 17:20 Bernie.zhao
// mem leak?
// 
// 3     04-09-21 11:14 Bernie.zhao
// get rid of hash_map
// 
// 2     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// ===========================================================================
#pragma once
#pragma warning (disable: 4996)		// disable warning about hash_map "deprecated"

#include <winsock2.h>

// ZQ::common namespace
#include "Socket.h"
#include "Exception.h"

// local
#include "RtspHeaders.h"
#include "RtspMsgHeader.h"
#include "RtspSock.h"

/// \class RtspMessage	class stands for general RTSP message
class RtspMessage
{
protected:
	std::string		_command;	/// start line of a RTSP message

	std::map<std::string, std::string>		_headers;	/// hash map of the headers

	std::string		_body;	/// content body
	
public:
	RtspMessage(void);
	RtspMessage(std::string command);
	~RtspMessage(void);

public:
	// attributes methods
	std::string	getCommand() { return _command; }
	std::map<std::string, std::string>	getHeaders() { return _headers; }
	std::string	getBody()	{ return _body; }

public:
	/// Returns a header field value based on its name
	///@return	field value or "" if not found
	///@param	name	field name
	std::string	getHeaderField(std::string name);

	/// Parses an rtsp message from an input stream
	///@param	messagebytes	input message chunk
	void		parse(const char* messagebytes);

	/// Parses a new RtspMessage using parse(byte*)
	///@return	new pointer to RtspMessage Object
	static void parseNewMessage(const char* messagebytes, RtspMessage& newMsg);

	/// Sets the body of this message to be sent after the header
	///@param	data	content body chunk
	void		setBody(const char* data);

	/// Sets a header field value based on its name
	///@param	name	field name
	///@param	value	field value
	void		setHeaderField(std::string name, std::string value);

	/// Returns the entire valid RTSP Header represented by this message object
	///@return	a formed RTSP Message string
	std::string toString(void);

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

	/// clear all message info of this message
    virtual void clearMessage(void);
	
	/// check if message is empty
	virtual bool isEmpty(void);

	/// Check if this header has sub headers and returns the whole subHeader as an RtspMsgHeader object
	///@return RtspMsgHeader object or NULL if this header has no sub headers
	///@param name		is the name of this header
	RtspMsgHeader hasSubHeader(std::string name);

	/// check if this message is a request or response
	///@return RTSP_REQUEST_MSG, RTSP_RESPONSE_MSG, and RTSP_UNKNOWN_MSG
	int isRequOrResp(void);
	
	// assign operator
	RtspMessage& operator=(RtspMessage& right);
};
