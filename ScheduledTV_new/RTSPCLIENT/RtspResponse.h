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
// $Log: /ZQProjs/ScheduledTV_new/RTSPCLIENT/RtspResponse.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 2     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// ===========================================================================
#ifndef _RTSPRESPONSE_H_
#define _RTSPRESPONSE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
	RtspResponse(const char* command);
	RtspResponse(const char* version, const char* status, const char* message);
	
	virtual ~RtspResponse(void);
	
public:
	std::string	getCommandversion() { return _commandversion; }
	void		setCommandversion( const char* param) { _commandversion = param; }
	std::string	getCommandstatus() { return _commandstatus; }
	void		setCommandstatus( const char* param) { _commandstatus = param; }
	std::string	getCommandmessage() { return _commandmessage; }
	void		setCommandmessage( const char* param) { _commandmessage = param; }

public:
	/// overrided parsesCommand function
	///@param[in]	command			command part to parse
	///@remarks						override for RESPONSE command
	void parseCommand(const char* command);

	/// overrided clear function
	virtual void	clearMessage();
};

#endif	// _RTSPRESPONSE_H_
