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
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspResponse.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 5     05-06-27 18:07 Bernie.zhao
// fixed interface error with SM when creating new STV list
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
}

RtspResponse::~RtspResponse(void)
{
}

RtspResponse::RtspResponse(const RtspMessage& msg)
{
	_command	= const_cast<RtspMessage&>(msg).getCommand();
	_headers	= const_cast<RtspMessage&>(msg).getHeaders();
	_body		= const_cast<RtspMessage&>(msg).getBody();
	parseCommand( _command);
}

RtspResponse::RtspResponse(std::string command) : RtspMessage(command)
{
	parseCommand(command);
}

RtspResponse::RtspResponse(std::string version, std::string status, std::string message) : RtspMessage(version+" "+status+" "+message)
{
	_commandversion	= version;
	_commandstatus	= status;
	_commandmessage	= message;
}

void RtspResponse::parseCommand(std::string command)
{
	std::string	subs = command;
	size_t	bs;

	bs = subs.find_first_of(' ');
	_commandversion	= subs.substr(0,bs);	// get version

	while( subs.at(bs)==' ')
		bs++;		// skip white space
	subs	= subs.substr(bs,subs.size()-bs);
	bs	= subs.find_first_of(' ');
	_commandstatus	= subs.substr(0,bs);	// get status

	while( subs.at(bs)==' ')
		bs++;		// skip white space
	subs	= subs.substr(bs,subs.size()-bs);
	bs	= subs.find_first_of(CH_CR);	
	if(bs == std::string::npos)
		bs = subs.size();
	_commandmessage	= subs.substr(0,bs);	// get message
}

RtspResponse RtspResponse::parseResponse(const char* messagebytes)
{
	RtspResponse	retmsg;
	retmsg.parse(messagebytes);
	retmsg.parseCommand(retmsg._command);

	return	retmsg;
}

int RtspResponse::sendMessage(RtspSock* sockd,int flag/* =0 */)
{
	int ret = RtspMessage::sendMessage(sockd,flag);
	return ret;
}

int RtspResponse::sendMessage(RtspSock* sockd, char* data, int flag/* =0 */)
{
	int ret = RtspMessage::sendMessage(sockd, data,flag);
	return ret;
}

int RtspResponse::recvMessage(RtspSock* sockd)
{
	int ret = RtspMessage::recvMessage(sockd);
	parseCommand(_command);
	return ret;
}

void RtspResponse::clearMessage()
{
	RtspMessage::clearMessage();
	_commandversion="";
	_commandstatus="";
	_commandmessage="";
}