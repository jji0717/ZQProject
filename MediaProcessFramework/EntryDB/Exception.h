// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This was copied from enterprise domain object sys, edos's copyright is belong to Hui Shao
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
// Desc  : exceptions
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/Exception.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/Exception.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 4     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 3     4/12/05 5:39p Hui.shao
// ============================================================================================

#ifndef __Exception_h__
#define __Exception_h__

#include "EntryDB.h"
#include <exception>
#include <string>

ENTRYDB_NAMESPACE_BEGIN

class Exception : public std::exception 
{
private:
	std::string _what;

public:
	Exception(const std::string& what_arg) throw() :_what(what_arg) {}
	virtual ~Exception() throw() {}
	virtual const char *getString() const { return _what.c_str(); }
};

class IOException : public Exception
{
public:
	IOException(const std::string &what_arg) throw() : Exception(what_arg) {};
};

ENTRYDB_NAMESPACE_END

#endif // __Exception_h__

