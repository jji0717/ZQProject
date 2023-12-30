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
// Desc  : rtsp message headers definition
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/RTSPCLIENT/RtspMsgHeader.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 1     05-08-30 18:30 Bernie.zhao
// 
// 3     04-10-21 15:29 Bernie.zhao
// 
// 2     04-10-19 17:20 Bernie.zhao
// mem leak?
// 
// ===========================================================================
//#include "StdAfx.h"
#include ".\rtspmsgheader.h"
#include "RtspMessage.h"

RtspMsgHeader::RtspMsgHeader(void)
{
	ZQ::common::Guard<ZQ::common::Mutex>	tmpGd(_subLock);
	_subheaders.clear();
}

RtspMsgHeader::RtspMsgHeader(const char* name)
{
	_name	= name;
}

RtspMsgHeader::RtspMsgHeader(const RtspMsgHeader& right)
{
	*this = right;
}

RtspMsgHeader::~RtspMsgHeader(void)
{
	ZQ::common::Guard<ZQ::common::Mutex>	tmpGd(_subLock);
	_subheaders.clear();
}

void RtspMsgHeader::getSubHeaders(std::vector<Pair>& out)
{
	ZQ::common::Guard<ZQ::common::Mutex>	tmpGd(_subLock);

	out.clear();

	for(size_t i=0; i<_subheaders.size(); i++)
	{
		Pair tmppair = _subheaders.at(i);
		out.push_back(tmppair);
	}
}

std::string RtspMsgHeader::getSubHeaderField(const char* subname)
{
	ZQ::common::Guard<ZQ::common::Mutex>	tmpGd(_subLock);
	
	for(size_t i=0; i<_subheaders.size(); i++) {
		Pair tmppair = _subheaders.at(i);

		if(tmppair.first == subname)	// match
			return	tmppair.second;
	}
	return "";	// not found
}

void RtspMsgHeader::setSubHeaderField(const char* subname, const char* subvalue)
{
	ZQ::common::Guard<ZQ::common::Mutex>	tmpGd(_subLock);

	for(size_t i=0; i<_subheaders.size(); i++) {
		Pair tmppair;
		tmppair.first=_subheaders.at(i).first;
		tmppair.second=_subheaders.at(i).second;

		if(tmppair.first == subname) {	// match
			_subheaders.at(i).second = subvalue;
			return;
		}
	}
	Pair subitem;		// no existed item, so create a new item
	subitem.first=subname;
	subitem.second= subvalue;	
	_subheaders.push_back(subitem);
}

int RtspMsgHeader::parse(const char* headerbytes)
{
	int ret =0;
	std::string	currstr = "";

	// parse header into string, cut off CRLF
	while( !(*headerbytes == CH_CR && *(headerbytes+1) == CH_LF)  && *headerbytes){
			currstr = currstr + *headerbytes;
			headerbytes++;
	}
	
	bool	quitflag = false;

	while(1) {
		size_t	semicolon = currstr.find_first_of(';');
		if( semicolon == std::string.npos ) {	// reach end, do last time and quit
			semicolon = currstr.size();
			quitflag = true;
		}

		// get item
		std::string	subitem = currstr.substr(0,semicolon);
		std::string	subname = "";
		std::string	subvalue= "";

		// separate subname and subvalue, set sub header field
		size_t	equalch = subitem.find_first_of('=');
		if( equalch != std::string.npos) {	// '=' valid
			subname		= subitem.substr(0, equalch);
			subvalue	= subitem.substr(equalch+1,subitem.size()-equalch-1);
		}
		else {	// '=' invalid
			subname		= subitem;
		}
		RtspMessage::eatWS(subname);
//		eatWS(subvalue);	-- comment by Bernie.  Some SeaChange Rtsp value requires a space at the front
		setSubHeaderField(subname.c_str(),subvalue.c_str());
		ret++;

		if(quitflag)
			break;
		currstr = currstr.substr(semicolon+1,currstr.size()-semicolon-1);	// update current string
	}

	return	ret;
}

void RtspMsgHeader::parseNewHeader(const char* headername, const char* headerbytes, RtspMsgHeader& newMsgHeader)
{
	newMsgHeader.setName(headername);
	newMsgHeader.parse(headerbytes);
}

std::string RtspMsgHeader::toString(void)
{
	ZQ::common::Guard<ZQ::common::Mutex>	tmpGd(_subLock);
	
	std::string	retstr = "";
	for(size_t i=0; i<_subheaders.size(); i++) {
		retstr = retstr + _subheaders.at(i).first;
		if(!_subheaders.at(i).second.empty())
			retstr = retstr + "=" + _subheaders.at(i).second;
		retstr = retstr + ";";
	}
	if(_subheaders.size()!=0)
		retstr.erase(retstr.size()-1,1); // cast away the last ";"
		
	return retstr;
}

RtspMsgHeader& RtspMsgHeader::operator =(const RtspMsgHeader& right)
{
	ZQ::common::Guard<ZQ::common::Mutex>	tmpGd(_subLock);

	_name = right._name;
	const_cast<RtspMsgHeader&>(right).getSubHeaders(_subheaders);

	return *this;
}

bool RtspMsgHeader::isEmpty()
{
	return _subheaders.empty();
}

void RtspMsgHeader::clearMsgHeader()
{
	ZQ::common::Guard<ZQ::common::Mutex>	tmpGd(_subLock);

	_name = "";
	_subheaders.clear();
}