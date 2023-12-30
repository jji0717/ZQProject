
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
// Dev  : Microsoft Developer Studio
// Name  : SRMException.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-7
// Desc  : exception
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/MPFException.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 2     05-04-19 14:15 Daniel.wang
// 
// 1     05-04-19 14:10 Daniel.wang
// move exception from srm to mpf common
// 
// 3     05-04-18 15:22 Daniel.wang
// 
// 2     05-04-15 21:22 Daniel.wang
// 
// 1     05-04-08 10:40 Daniel.wang
// create
// ===========================================================================

#ifndef _ZQ_SRMEXCEPTION_H_
#define _ZQ_SRMEXCEPTION_H_

#include "MPFCommon.h"

#include <exception>

#define MAX_EXCEPTION_STRING_LEN 2048
#define UNNAMED_EXCEPTION "Un-named exception"

MPF_NAMESPACE_BEGIN

// -----------------------------
//SRMException
// -----------------------------
/// comment: exception
///
/// usage: 
///
/// note: 
///
class SRMException : public std::exception
{
private:
	char	m_pStr[MAX_EXCEPTION_STRING_LEN];

public:
	//default constructor with un-named exception
	SRMException() throw()
	{
		memset(m_pStr, 0, MAX_EXCEPTION_STRING_LEN);
		_snprintf(m_pStr, MAX_EXCEPTION_STRING_LEN - 1, UNNAMED_EXCEPTION, MAX_EXCEPTION_STRING_LEN-1);
	}

	//constructor with set an exception name
	SRMException(const char* strFmt, ...) throw()
	{
		memset(m_pStr, 0, MAX_EXCEPTION_STRING_LEN);
		va_list vl;
		va_start(vl, strFmt);
		_vsnprintf(m_pStr, MAX_EXCEPTION_STRING_LEN - 1, strFmt, vl);
		va_end(vl);
	}

	//destructor
	virtual ~SRMException() throw()
	{
	}

	//get exception name for describe of the error
	virtual const char* what() const throw()
	{
		return m_pStr;
	}
};

MPF_NAMESPACE_END

#endif//_ZQ_SRMEXCEPTION_H_
