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
// $Log: /ZQProjs/ScheduledTV_new/RTSPCLIENT/RtspMsgHeader.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 6     04-10-21 15:29 Bernie.zhao
// 
// 5     04-10-19 17:20 Bernie.zhao
// mem leak?
// 
// 4     04-10-12 9:55 Bernie.zhao
// pragma warning 4786
// 
// 3     04-09-21 11:14 Bernie.zhao
// get rid of hash_map
// 
// 2     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// ===========================================================================
#ifndef _RTSPMSGHEADER_H_
#define _RTSPMSGHEADER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Locks.h"

#include "RtspHeaders.h"
//#include "RtspMessage.h"

/// \class RtspMsgHeader  sub-header in RTSP Message Header
class RtspMsgHeader
{
	typedef struct _pair{
	std::string first;
	std::string second;
	} Pair;

public:
	RtspMsgHeader(void);
	RtspMsgHeader(const char*  name);
	RtspMsgHeader(const RtspMsgHeader& right);
	~RtspMsgHeader(void);
public:
	std::string	getName() { return _name; }
	void		setName( const char*	name) { _name = name; }
	void		getSubHeaders(std::vector<Pair>& out);

public:

	/// get sub header field inside this header
	///@return the subvalue of specified subname or NULL when this sub item is missing
	///@param subname		is the name of sub items in header
	///@remarks  SeaChange-Server-Data:node-group-id=2;smartcard-id=0000000001;device-id=00900f00ba91 \n
	///  "SeaChange-Server-Data" is this header's name \n
	///  "node-group-id" is a subname and "2" is a subvalue, so are "smartcard-id" and "0000000001", etc.
	std::string getSubHeaderField(const char* subname);

	/// set sub header field inside this header
	///@param subname		is the name of sub item in header
	///@param subvalue	is the value of sub item in header
	///@remarks		the sequence of sub items should been maintained. 
	/// SeaChange-Server-Data:node-group-id=2;smartcard-id=0000000001;device-id=00900f00ba91
	/// "node-group-id" sub item should been set before "smartcard-id" sub item
	void setSubHeaderField(const char* subname, const char* subvalue);

	/// parse header and separate into sub items
	///@return sub item numbers in this header
	///@param headerbytes		is the header that needs parsed
	int parse(const char* headerbytes);

	/// parse a new header 
	///@param headername		is the name of this header
	///@param headerbytes		is the header that needs parsed
	///@param newMsgHeader		is the new created header
	static void parseNewHeader(const char* headername, const char* headerbytes, RtspMsgHeader& newMsgHeader);

	/// Returns a valid RTSP Header represented by this message object
	///@return	a formed RTSP Message string
	///@remarks	the sequence of sub items are maintained
	std::string toString(void);

	/// clear header
	void clearMsgHeader();

	/// check if header is empty
	///@return		true if empty, false else
	bool isEmpty(void);

	// assign operator
	RtspMsgHeader& operator=(const RtspMsgHeader& right);

protected:
	std::string			_name;
	std::vector<Pair>	_subheaders;
	ZQ::common::Mutex	_subLock;
};

#endif // _RTSPMSGHEADER_H_