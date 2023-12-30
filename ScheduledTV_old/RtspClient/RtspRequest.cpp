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
// Desc  : rtsp request message implementation
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspRequest.cpp $
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
// 3     04-09-21 11:14 Bernie.zhao
// get rid of hash_map
// 
// 2     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// ===========================================================================
//#include "StdAfx.h"
#include ".\rtsprequest.h"

RtspRequest::RtspRequest(void)
{
}

RtspRequest::~RtspRequest(void)
{
}

RtspRequest::RtspRequest(const RtspMessage& msg)
{
	_command	= const_cast<RtspMessage&>(msg).getCommand();
	_headers	= const_cast<RtspMessage&>(msg).getHeaders();
    _body		= const_cast<RtspMessage&>(msg).getBody();
	parseCommand( _command);
}

RtspRequest::RtspRequest(std::string command) : RtspMessage(command)
{
	parseCommand(command);
}

RtspRequest::RtspRequest(std::string type, std::string target, std::string version/*="RTSP/1.0"*/) : RtspMessage(type+" "+target+" "+version)
{
	_commandtype	= type;
	_commandtarget	= target;
	_commandversion	= version;
}

RtspRequest RtspRequest::parseRequest(const char* messagebytes)
{
	RtspRequest	retmsg;
	retmsg.parse(messagebytes);
	retmsg.parseCommand(retmsg._command);

	return	retmsg;

}

void RtspRequest::parseCommand(std::string command)
{
	std::string	subs = command;
	size_t	bs;

	bs = subs.find_first_of(' ');
	_commandtype	= subs.substr(0,bs);	// get type

	while( subs.at(bs)==' ')
		bs++;		// skip white space
	subs	= subs.substr(bs,subs.size()-bs);
	bs	= subs.find_first_of(' ');
	_commandtarget	= subs.substr(0,bs);	// get target

	while( subs.at(bs)==' ')
		bs++;		// skip white space
	subs	= subs.substr(bs,subs.size()-bs);
	bs	= subs.find_first_of(CH_CR);	
	if(bs == std::string::npos)
		bs = subs.size();
	_commandversion	= subs.substr(0,bs);	// get version

}

int RtspRequest::sendMessage(RtspSock* sockd,int flag/* =0 */)
{
	int ret = RtspMessage::sendMessage(sockd,flag);
	return ret;
}

int RtspRequest::sendMessage(RtspSock* sockd, char* data, int flag/* =0 */)
{
	int ret = RtspMessage::sendMessage(sockd, data,flag);
	return ret;
}

int RtspRequest::recvMessage(RtspSock* sockd)
{
	int ret = RtspMessage::recvMessage(sockd);
	parseCommand(_command);
	return ret;
}

void RtspRequest::clearMessage()
{
	RtspMessage::clearMessage();
	_commandtype="";
	_commandversion="";
	_commandtarget="";
}

