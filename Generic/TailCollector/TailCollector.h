// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This source was copied from shcxx, shcxx's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : define a clog message collector
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/TailCollector/TailCollector.h 1     10-11-12 15:59 Admin $
// $Log: /ZQProjs/Generic/TailCollector/TailCollector.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 1     6/24/05 2:34p Hui.shao
// ============================================================================================

#ifndef __TailCollector_h__
#define __TailCollector_h__

#include "BaseCTail.h"
#include "Ini.h"
#include <boost/regex.hpp>

// -----------------------------
// class MessageHandler
// -----------------------------
class MessageHandler
{
public:

	/// Constructor
	///@param pFile  handle to the output file that records the specified log messages
	///@param syntax  a regular expression to match
	///@param output_fmt  the output format, $0 means the whole message, $1, $2 ... means the matched fields
	MessageHandler(FILE* pFile, const char* syntax, const char* output_fmt, bool echoToScreen =true);

	/// Destructor
	virtual ~MessageHandler();

	/// Entry to handle a detected message
	///@param msg  the NULL-terminated detected message
	virtual bool handleMessage(const char* msg);
	
protected:
	boost::regex _syntax;
	std::string  _output_fmt;
	FILE*		 _pFile;
	int			 _flush;
	bool		 _bEcho;

	typedef boost::match_results<std::string::const_iterator> res_t;

	virtual bool matched(const char* msg, res_t& results);
};

// -----------------------------
// class CTailCollector
// -----------------------------
class CTailCollector : public BaseCTail
{
public:

	/// Constructor
	///@param filename  name of the log file to monitor
	CTailCollector (const char* filename = NULL) : BaseCTail(filename) {}

	virtual ~CTailCollector() {}

	typedef std::vector < MessageHandler > msghandler_v;

	/// add a message handler into the tail collector
	///@param mh  a message handler
	bool addHandler(MessageHandler& mh);

protected:
	
	virtual void OnNewMessage(const char* line);

	msghandler_v _msghandlers;
		
};

#endif // __TailCollector_h__
