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
// Desc  : a clog message collector
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/TailCollector/TailCollector.cpp 1     10-11-12 15:59 Admin $
// $Log: /ZQProjs/Generic/TailCollector/TailCollector.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 1     6/24/05 2:34p Hui.shao
// ============================================================================================

#include "TailCollector.h"

// -----------------------------
// class MessageHandler
// -----------------------------
MessageHandler::MessageHandler(FILE* pFile, const char* syntax, const char* output_fmt, bool echoToScreen)
: _syntax(syntax), _output_fmt(output_fmt), _pFile(pFile), _bEcho(echoToScreen)
{
}

MessageHandler::~MessageHandler()
{
	if (_pFile)
		fflush(_pFile);
}

bool MessageHandler::handleMessage(const char* msg)
{
	res_t results;
	std::string value;
	
	if (!matched(msg, results))
		return false;
	
	std::string output = _output_fmt;
	for (int i = results.size() - 1; i >=0; --i)
	{
		value.assign(results[i].first, results[i].second);
		char strbuf[10];
		
		sprintf(strbuf, "$%d", i);
		int len = strlen(strbuf);
		
		for (int pos = output.find(strbuf, 0); pos != std::string::npos; pos = output.find(strbuf, pos))
			output.replace(pos,len,value.c_str());
	}
	
	if (_pFile)
	{
		fprintf(_pFile, "%s\n", output.c_str());
		if (0 == (++_flush % 5))
		{
			fflush(_pFile);
			_flush =0;
		}
	}

	if (_bEcho && _pFile !=stdout && _pFile !=stderr)
		printf("%s\n", output.c_str());
	
	return true;
}

bool MessageHandler::matched(const char* msg, res_t& results)
{
	if (msg == NULL || _syntax.empty())
		return false;
	
	return boost::regex_match(msg, results, _syntax);
}

// -----------------------------
// class CTailCollector
// -----------------------------
bool CTailCollector::addHandler(MessageHandler& mh)
{
	_msghandlers.push_back(mh);
	return true;
}

void CTailCollector::OnNewMessage(const char* line)
{
	for (msghandler_v::iterator it = _msghandlers.begin(); it < _msghandlers.end(); it++)
		it->handleMessage(line);
}

