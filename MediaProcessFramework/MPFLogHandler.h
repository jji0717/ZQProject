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
// Name  : MPFLogHandler.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-5-17
// Desc  : base class for log, could be derived to create other format log
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/MPFLogHandler.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 5     05-06-10 4:51p Daniel.wang
// 
// 4     05-06-02 6:28p Daniel.wang
// 
// 3     05-05-19 19:17 Bernie.zhao
// 
// 2     05-05-17 22:17 Bernie.zhao
// ===========================================================================

#ifndef _MPFLOGHANDLER_
#define _MPFLOGHANDLER_

#include "MPFCommon.h"

MPF_NAMESPACE_BEGIN

#define MPFLog (*(MPFLogHandler::getLogHandler()))

///log handler of MPF\n
///usage:\n
///1.create a class that inherited by this class\n
///2.write you log storage code in MPFLogHandler::writeMessage function\n
///3.use MPFLogHandler::setLogHandler() function to register the new log handler\n
///note:\n
///it will call default log handler to storage log information\n
class DLL_PORT MPFLogHandler
{
public:

	typedef enum  //	level range: value 0~7
	{
		L_EMERG=0, L_ALERT,  L_CRIT, L_ERROR,
		L_WARNING, L_NOTICE, L_INFO, L_DEBUG
	} loglevel_t;
	
	virtual ~MPFLogHandler();

	///get current log handler instance
	///@return - log handler instance
	static MPFLogHandler* getLogHandler();

	///set current log handler instance
	///@param lh - new log handler instance
	///@remarks the LogHandler will be set to be defaultLogHandler if the lh is NULL
	static void setLogHandler(MPFLogHandler* lh = NULL);

	///get verbosity level
	///@return - current verbosity level
	static int getVerbosity();

	///set verbosity level
	///@param v - new verbosity level
	static void setVerbosity(int v);

	///log storage process
	///usage:
	///MPFLog(<level>, <string_fromat>, ...);
	void operator()(int level, const char* fmt, ...);

	/// this function could be overrided if derived\n
	/// to provide customized log
	virtual void writeMessage(const char* msg) =0;

protected:
	static MPFLogHandler* _logHandler;
	static int _verbosity;
};

MPF_NAMESPACE_END

#endif // _MPFLOGHANDLER_