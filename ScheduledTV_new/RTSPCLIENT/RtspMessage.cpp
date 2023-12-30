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
// Desc  : rtsp general message implementation
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/RTSPCLIENT/RtspMessage.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 3     06-08-21 10:59 Bernie.zhao
// added error check in recvMessage()
// 
// 1     05-08-30 18:30 Bernie.zhao
// 
// 10    04-12-16 14:56 Bernie.zhao
// 
// 9     04-10-21 15:29 Bernie.zhao
// 
// 8     04-10-19 18:15 Bernie.zhao
// 
// 7     04-10-19 17:20 Bernie.zhao
// mem leak?
// 
// 6     04-10-18 18:46 Bernie.zhao
// 
// 5     04-10-18 10:33 Bernie.zhao
// test 1 ok
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

#include "rtspmessage.h"
#include "RtspSocket.h"

RtspMessage::RtspMessage(void)
{
	_command	= "";
	_headers.clear();
	_body		= "";
}

RtspMessage::RtspMessage(const char* command)
{
	_command = command;
	_headers.clear();
	parseCommand(command);	// call overrided function if exist
	_body = "";
}

RtspMessage::RtspMessage(const RtspMessage& right)
{
	*this = right;
}

RtspMessage::~RtspMessage(void)
{
	_command="";
	_headers.clear();
	_body="";
}

void RtspMessage::getHeaders(std::map<std::string, std::string>& out)
{
	ZQ::common::Guard<ZQ::common::Mutex> tmpGd(_hdLock);
	
	out.clear();
	std::map<std::string, std::string>::const_iterator	iter;
	for( iter=_headers.begin();iter!=_headers.end(); iter++ )
		out[iter->first]=iter->second;
}

std::string RtspMessage::getHeaderField(const char* name)
{
	std::string retstr = "";
	
	ZQ::common::Guard<ZQ::common::Mutex> tmpGd(_hdLock);
	std::map<std::string, std::string>::iterator	iter = _headers.find(name);

	if( iter != _headers.end() ) {
		retstr = iter->second;	// find match
		return	retstr;
	}
	return "";	// can't find
}

void RtspMessage::setHeaderField(const char* name, const char* value)
{
	ZQ::common::Guard<ZQ::common::Mutex> tmpGd(_hdLock);
	_headers[name] = value;
}

bool RtspMessage::hasSubHeader(const char* name)
{
	std::string value = getHeaderField(name);
	if(value=="")	// can't find value
		return false;

	size_t semicolon = value.find_first_of(';');
	if(semicolon == std::string::npos)	// has no sub headers
		return false;

	return true;
}

void RtspMessage::getSubHeader(const char* name, RtspMsgHeader& subhd)
{
	subhd.clearMsgHeader();
	std::string value = getHeaderField(name);
	if(value=="")	// can't find value
		return;

	size_t semicolon = value.find_first_of(';');
	if(semicolon == std::string::npos)	// has no sub headers
		return;

	subhd.parse(value.c_str());
}

void RtspMessage::setBody(const char* data)
{
	_body = "";
	while(*data!=NULL) {
		_body = _body + *data;
		data++;
	}
}

void RtspMessage::parse(const char* messagebytes)
{
	if(NULL==messagebytes)
		return;
	
	std::string	msgstr = "";
	
	int ii=0;

	// clear former component
	_command="";
	_headers.clear();
	_body="";

	// skip blanks
	while(*messagebytes == ' ')
		messagebytes++;

	// parse for start line into _command
	while(*messagebytes && (*messagebytes+1))
	{
		if(*messagebytes == CH_CR && *(messagebytes+1) == CH_LF)
		{
			break;		// double CRLF, break
		}
		_command = _command + *messagebytes;
		messagebytes++;
	}

	
	// parse for headers into _headers until double CRLF
	while(*messagebytes && (*messagebytes+1))
	{
		messagebytes+=2; // skip CRLF

		if(*messagebytes == CH_CR && *(messagebytes+1) == CH_LF)
		{
			break;		// double CRLF, break
		}
		
		msgstr = "";
		while(*messagebytes && (*messagebytes+1))
		{	// scan for a line
			if(*messagebytes == CH_CR && *(messagebytes+1) == CH_LF)
			{
				break;
			}
			
			msgstr = msgstr + *messagebytes;
			messagebytes++;
		}

		size_t	colon = msgstr.find_first_of(':');
		std::string	name = msgstr.substr( 0,colon );
		std::string	value= "";
		if(colon<msgstr.size()-1) 
			value = msgstr.substr( colon+1, msgstr.size()-1-colon);
		RtspMessage::eatWS(name);
		RtspMessage::eatWS(value);
		setHeaderField(name.c_str(), value.c_str());
		
	}

	// parse for body into _body
	messagebytes+=2;
	std::string length = getHeaderField("Content-Length");
	if(!length.empty()) 
	{
		// has body part
		int num = atoi(length.c_str());
		for(int i =0; i<num; i++) 
		{
			if(!*messagebytes)
			{
				break;	//EOF already
			}
			_body = _body + *messagebytes;
			messagebytes++;
		}
	}

	// parse command, if derived class overrided parseCommand() function, 
	// the overrided one will be called here
	parseCommand(_command.c_str());
}

void RtspMessage::parseCommand(const char* command)
{
	if(NULL==command)
		return;

	// do nothing in base class
	return;
}

void RtspMessage::parseNewMessage(const char* messagebytes, RtspMessage& newMsg, int* bytesSoFar/* =NULL */)
{
	newMsg.clearMessage();

	if(bytesSoFar)
	{
		if(*bytesSoFar>=strlen(messagebytes))
			return;
		
		newMsg.parse(messagebytes+*bytesSoFar);
		*bytesSoFar += newMsg.toString().length();
	}
	else
	{
		newMsg.parse(messagebytes);
	}
}

std::string RtspMessage::toString(void)
{
	std::string	retstr = "";

	ZQ::common::Guard<ZQ::common::Mutex> tmpGd(_hdLock);
	
	std::map<std::string, std::string>::iterator	iter;

	retstr = retstr + _command + KEY_CRLF;
	for( iter=_headers.begin();iter!=_headers.end(); iter++ )
		retstr = retstr + iter->first + ":" + iter->second + KEY_CRLF;
    retstr = retstr + KEY_CRLF;		//double CRLF indicates the end of headers

	if(!_body.empty())
		retstr = retstr + _body;
	
	return retstr;
}

int RtspMessage::sendMessage(SOCKET fd)
{
	int sofar=0;

	std::string out = toString();

	bool ret = RtspSocket::nbWrite(fd, out, &sofar);
	
	if(!ret){
			//throw ZQ::common::IOException("RTSP socket sending data failed");
			return -1;
	}
	
	return sofar;
}

int RtspMessage::sendMessage(SOCKET fd, char* data)
{
	int sofar;
	
	std::string out = toString() + data;
	
	int ret = RtspSocket::nbWrite(fd, out, &sofar);
	if(!ret){
		//throw ZQ::common::IOException("RTSP socket sending data failed");
		return -1;
	}

	return sofar;
}

int RtspMessage::recvMessage(SOCKET fd)
{
	bool eof;
	int errcode=0;

	std::string in = "";

	int ret = RtspSocket::nbRead(fd, in, &eof, errcode);

	if(!ret){
		//throw ZQ::common::IOException("RTSP socket recving data failed");
		return -1;
	}

	parse(in.c_str());

	return ret;
}

int RtspMessage::isRequOrResp(void)
{
	if(_command.empty())
		return RTSP_UNKNOWN_MSG;

	if(RtspMessage::beginWith(_command,"RTSP/"))
		return RTSP_RESPONSE_MSG;

	for(int i=0; i<RTSP_VALID_TYPE; i++) {
		if(RtspMessage::beginWith(_command,RTSP_REQUEST_TYPE[i]))
			return RTSP_REQUEST_MSG;
	}

	return RTSP_UNKNOWN_MSG;

}

void RtspMessage::clearMessage(void)
{
	ZQ::common::Guard<ZQ::common::Mutex> tmpGd(_hdLock);

	_command="";
	_headers.clear();
	_body="";
}

bool RtspMessage::isEmpty()
{
	return _command.empty();
}

RtspMessage& RtspMessage::operator	=(const RtspMessage& right)
{
	clearMessage();
	
	ZQ::common::Guard<ZQ::common::Mutex> tmpGd(_hdLock);
	_body		= right._body;
	_command	= right._command;
	const_cast<RtspMessage&>(right).getHeaders(_headers);
	
	parseCommand(_command.c_str());	// call overrided function if exist

	return *this;
}

//////////////////////////////////////////////////////////////////////////
// string functions

int RtspMessage::eatWS(std::string& src)
{
	int len =src.length();

	if(len == 0)	// if empty string, return
		return 0;
	
	size_t pos = src.find_first_not_of(" ", 0);
	if(pos!=std::string::npos)
		src = src.substr(pos);
	else
		src = src;
	
	return (len - src.length());
}

bool RtspMessage::beginWith(std::string src, std::string head)
{
	size_t occur = src.find( head,0);
	if(occur == 0)
		return true;
	return false;
}



