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
// $Log: /ZQProjs/ScheduledTV_new/RTSPCLIENT/RtspMessage.h $
// 
// 1     10-11-12 16:01 Admin
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
#ifndef _RTSPMESSAGE_H_
#define _RTSPMESSAGE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RtspSocket.h"

// ZQ::common namespace
#include "Locks.h"

// local
#include "RtspHeaders.h"
#include "RtspMsgHeader.h"

/// \class RtspMessage	class stands for general RTSP message
class RtspMessage
{
public:
	RtspMessage(void);
	RtspMessage(const char* command);
	RtspMessage(const RtspMessage& right);
	virtual ~RtspMessage(void);

public:
	// attributes methods
	std::string	getCommand() { return _command; }
	void		getHeaders(std::map<std::string, std::string>& out);
	std::string	getBody()	{ return _body; }

public:
	/// Returns a header field value based on its name
	///@return	field value or "" if not found
	///@param[in]	name	field name
	std::string	getHeaderField (const char* name);

	/// Sets a header field value based on its name
	///@param[in]	name	field name
	///@param[in]	value	field value
	void		setHeaderField(const char* name, const char* value);

	/// Check if this header has sub headers of name
	///@return true if has sub headers, false else
	///@param[in] name		is the name of sub headers
	bool		hasSubHeader(const char* name);

	/// get sub headers of name
	///@param[in] name		is the name of sub headers
	///@param[out] subhd	is the sub header to receive
	void		getSubHeader(const char* name, RtspMsgHeader& subhd);

	/// Sets the body of this message to be sent after the header
	///@param[in]	data	content body chunk
	void		setBody(const char* data);

	/// Returns the entire valid RTSP Header represented by this message object
	///@return[in]	a formed RTSP Message string
	std::string toString(void);

	/// send this message via socket
	///@return the number of bytes sent on success and -1 on failure
	///@param[in] fd		is the socket description
	int sendMessage(SOCKET fd);

	/// send this message 
	/// and append data stream until '\0'
	///@return the number of bytes sent on success and -1 on failure
	///@param[in] fd		is the socket description
	///@param[in] data		is the stream that needed to be appended after this message
	int sendMessage(SOCKET fd, char* data);

	/// receive data from sock into this message and parse it
	///@param[in] fd		is the socket description
	///@return the number of bytes received on success and -1 on failure
	int recvMessage(SOCKET fd);

	/// check if this message is a request or response
	///@return RTSP_REQUEST_MSG, RTSP_RESPONSE_MSG, and RTSP_UNKNOWN_MSG
	int isRequOrResp(void);
	
	// assign operator
	RtspMessage& operator=(const RtspMessage& right);

public:
	//////////////////////////////////////////////////////////////////////////
	// static functions

	/// Parses a new RtspMessage using parse(byte*)
	///@param[in]		messagebytes	message to parse
	///@param[out]		newMsg			new RtspMessage Object
	///@param[in,out]	bytesSoFar 		the bytes that have been parsed
	///@remarks		if you pass a reference of a derived class of RtspMessage (such as RtspRequest, RtspResponse...),
	/// this function will call the overrided parse() and parseCommand() function of the derived class, therefore making
	/// customized parsing of the derived class.
	static void		parseNewMessage(const char* messagebytes, RtspMessage& newMsg, int* bytesSoFar=NULL);

		/// eat white space of a string
	///@param[in]	src		source string to handle
	///@return the number of space eaten
	static int		eatWS(std::string& src);

	// check if the src string starts with the head string
	///@param[in]	src		source string to handle
	///@param[in]	head	the sub-string to check with
	///@return check result
	static bool		beginWith(std::string src, std::string head);


public:
	//////////////////////////////////////////////////////////////////////////
	// virtual functions

	/// Parses an rtsp message from an input stream
	///@param[in]	messagebytes	input message parse
	///@remarks						override if you want to new parse method
	virtual void	parse(const char* messagebytes);

	/// Parses the command
	///@param[in]	command			command part to parse
	///@remarks						override for REQUEST or RESPONSE to have different command part
	virtual void	parseCommand(const char* command);


	/// clear all message info of this message
    virtual void	clearMessage(void);
	
	/// check if message is empty
	///@return		true if empty, false else
	virtual bool	isEmpty(void);

	
protected:
	/// start line of a RTSP message
	std::string								_command;	

	/// hash map of the headers
	std::map<std::string, std::string>		_headers;	
	
	/// lock for the headers
	ZQ::common::Mutex						_hdLock;

	/// content body
	std::string								_body;	

};

#endif	// _RTSPMESSAGE_H_