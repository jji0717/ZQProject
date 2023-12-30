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
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspMessage.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 12    05-07-29 16:15 Bernie.zhao
// 
// 11    05-06-27 18:07 Bernie.zhao
// fixed interface error with SM when creating new STV list
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
//#include "StdAfx.h"
#include ".\rtspmessage.h"

int eatWS(std::string& src);
bool beginWith(std::string src, std::string head);

RtspMessage::RtspMessage(void)
{
	_body = "";
}

RtspMessage::RtspMessage(std::string command)
{
	_command = command;
	_body = "";
}

RtspMessage::~RtspMessage(void)
{
	_command="";
	_headers.clear();
	_body="";
}

std::string RtspMessage::getHeaderField(std::string name)
{
	std::string retstr = "";
	std::map<std::string, std::string>::iterator	iter = _headers.find(name);

	if( iter != _headers.end() ) {
		retstr = iter->second;	// find match
		return	retstr;
	}
	return "";	// can't find
}

void RtspMessage::parse(const char* messagebytes)
{
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
		std::string	value= msgstr.substr( colon+1, msgstr.size()-1-colon);
		eatWS(name);
		eatWS(value);
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
}

void RtspMessage::parseNewMessage(const char* messagebytes, RtspMessage& newMsg)
{
	newMsg.parse(messagebytes);
}

void RtspMessage::setBody(const char* data)
{
	_body = "";
	while(*data!=NULL) {
		_body = _body + *data;
		data++;
	}
}

void RtspMessage::setHeaderField(std::string name, std::string value)
{
	std::map<std::string, std::string>::iterator	iter = _headers.find(name);
	if( iter != _headers.end() ) { // already has field, remove it
		_headers.erase(iter);
	}
	//iter = _headers.insert(_headers.begin(),std::pair<std::string, std::string>(name,value) );
	_headers.insert(std::pair<std::string, std::string>(name,value) );
}


std::string RtspMessage::toString(void)
{
	std::string	retstr = "";
	std::map<std::string, std::string>::iterator	iter;

	retstr = retstr + _command + KEY_CRLF;
	for( iter=_headers.begin();iter!=_headers.end(); iter++ )
		retstr = retstr + iter->first + ":" + iter->second + KEY_CRLF;
    retstr = retstr + KEY_CRLF;		//double CRLF indicates the end of headers

	if(!_body.empty())
		retstr = retstr + _body;
	
	return retstr;
}

int RtspMessage::sendMessage(RtspSock* sockd, int flag/*=0*/)
{
	int bytes=sockd->sendN(this->toString().c_str(),(int)this->toString().size(), flag); 
	if(bytes<=0){
			//throw ZQ::common::IOException("RTSP socket sending data failed");
			return -1;
	}
	
	return bytes;
}

int RtspMessage::sendMessage(RtspSock* sockd, char* data, int flag/*=0*/)
{
	std::string tmpstr = this->toString() + data;
	
	int bytes=sockd->sendN(tmpstr.c_str(),(int)tmpstr.size(), flag);
	if(bytes<=0){
		//throw ZQ::common::IOException("RTSP socket sending data failed");
		return -1;
	}

	return bytes;
}

int RtspMessage::recvMessage(RtspSock* sockd)
{
	char buff[MAX_BUFFER_SIZE];
	int bytes=sockd->recvN(buff,MAX_BUFFER_SIZE);
	if(bytes<=0)
		return -1;

	buff[bytes]=NULL;
	this->parse(buff);

	return bytes;
}

RtspMsgHeader RtspMessage::hasSubHeader(std::string name)
{
	RtspMsgHeader ret(name);
	std::string value = getHeaderField(name);
	if(value=="")	// can't find value
		return NULL;

	size_t semicolon = value.find_first_of(';');
	if(semicolon == std::string.npos)	// has no sub headers
		return NULL;

	ret.parse(value);

	return ret;
}

int RtspMessage::isRequOrResp(void)
{
	if(_command.empty())
		return RTSP_UNKNOWN_MSG;

	if(beginWith(_command,"RTSP/"))
		return RTSP_RESPONSE_MSG;

	for(int i=0; i<RTSP_VALID_TYPE; i++) {
		if(beginWith(_command,RTSP_REQUEST_TYPE[i]))
			return RTSP_REQUEST_MSG;
	}

	return RTSP_UNKNOWN_MSG;

}

void RtspMessage::clearMessage(void)
{
	_command="";
	std::map<std::string, std::string>::iterator iter;
	for(iter=_headers.begin(); iter!=_headers.end(); iter++) {
		iter->second="";
	}
	_headers.clear();
	_body="";
}

bool RtspMessage::isEmpty()
{
	return _command.empty();
}

RtspMessage& RtspMessage::operator =(RtspMessage& right)
{
	clearMessage();
	_body		= right._body;
	_command	= right._command;

	std::map<std::string, std::string>::iterator	iter;
	for( iter=right._headers.begin();iter!=right._headers.end(); iter++ )
		_headers[iter->first]=iter->second;

	return *this;
}

//////////////////////////////////////////////////////////////////////////
// global functions
// eat white space of a string
///@return the number of space eaten
int eatWS(std::string& src)
{
	int len =src.length();

	if(len == 0)	// if empty string, return
		return 0;
	
	size_t pos = src.find_first_not_of(" ", 0);
	if(pos!=std::string::npos)
		src = src.substr(pos);
	else
		src = "";
	
	return (len - src.length());
}

// check if the src string starts with the head string
///@return check result
bool beginWith(std::string src, std::string head)
{
	size_t occur = src.find( head,0);
	if(occur == 0)
		return true;
	return false;
}



