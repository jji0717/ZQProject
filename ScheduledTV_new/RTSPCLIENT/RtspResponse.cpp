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
// Desc  : rtsp response message implementation
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/RTSPCLIENT/RtspResponse.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 1     05-08-30 18:30 Bernie.zhao
// 
// 4     04-10-07 9:24 Bernie.zhao
// get rid of stdafx.h
// 
// 3     04-09-21 11:14 Bernie.zhao
// get rid of hash_map
// 
// 2     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// ===========================================================================
//#include "StdAfx.h"
#include ".\rtspresponse.h"

RtspResponse::RtspResponse(void)
{
	_commandversion = "";
	_commandstatus	= "";
	_commandmessage	= "";
}

RtspResponse::~RtspResponse(void)
{
	_commandversion = "";
	_commandstatus	= "";
	_commandmessage	= "";
}

RtspResponse::RtspResponse(const RtspMessage& msg)
{
	clearMessage();
	
	ZQ::common::Guard<ZQ::common::Mutex> tmpGd(_hdLock);
	_body		= const_cast<RtspMessage&>(msg).getBody();
	_command	= const_cast<RtspMessage&>(msg).getCommand();
	const_cast<RtspMessage&>(msg).getHeaders(_headers);
	
	parseCommand(_command.c_str());	// call overrided function if exist
}

RtspResponse::RtspResponse(const char* command) : RtspMessage(command)
{
	parseCommand(command);
}

RtspResponse::RtspResponse(const char* version, const char* status, const char* message)
{
	_commandversion	= version;
	_commandstatus	= status;
	_commandmessage	= message;

	_command = _commandversion + " " + _commandstatus + " " + _commandmessage;
	_headers.clear();
	_body	= "";
}

void RtspResponse::parseCommand(const char* command)
{
	std::string	subs = command;
	size_t	bs;

	bs = subs.find_first_of(' ');
	_commandversion	= subs.substr(0,bs);	// get version

	if(bs == std::string::npos)
		return;

	while( subs.at(bs)==' ')
		bs++;		// skip white space
	subs	= subs.substr(bs,subs.size()-bs);
	bs	= subs.find_first_of(' ');
	_commandstatus	= subs.substr(0,bs);	// get status

	if(bs == std::string::npos)
		return;

	while( subs.at(bs)==' ')
		bs++;		// skip white space
	subs	= subs.substr(bs,subs.size()-bs);
	bs	= subs.find_first_of(CH_CR);	
	
	if(bs == std::string::npos)
		bs = subs.size();
	
	_commandmessage	= subs.substr(0,bs);	// get message
}

void RtspResponse::clearMessage()
{
	RtspMessage::clearMessage();
	_commandversion="";
	_commandstatus="";
	_commandmessage="";
}