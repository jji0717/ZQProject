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
// Archive : $Archive: /ZQProjs/ScheduledTV_new/RTSPCLIENT/RtspRequest.h $
// Branch: $Name:  $
// Author: Bernie Zhao (Tianbin Zhao)
// Desc  : rtsp request message definition
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/RTSPCLIENT/RtspRequest.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 3     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// 2     04-08-18 14:51 Bernie.zhao
// 
// ===========================================================================
#ifndef _RTSPREQUEST_H_
#define _RTSPREQUEST_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
	RtspRequest(const char* command);
	RtspRequest(const char* type, const char* target, const char* version="RTSP/1.0");

	virtual ~RtspRequest(void);
	

public:
	std::string	getCommandtype() { return _commandtype; }
	void		setCommandtype( const char* param) { _commandtype = param; }
	std::string	getCommandtarget() { return _commandtarget; }
	void		setCommandtarget( const char* param) { _commandtarget = param; }
	std::string	getCommandversion() { return _commandversion; }
	void		setCommandversion( const char* param) { _commandversion = param; }

public:
	/// overrided parsesCommand function
	///@param[in]	command			command part to parse
	///@remarks						override for REQUEST command
	virtual void		parseCommand(const char* command);

	/// overrided clear function
    virtual void clearMessage(void);
};

#endif	// _RTSPREQUEST_H_