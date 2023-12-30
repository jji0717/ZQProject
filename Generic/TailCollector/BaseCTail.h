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
// Desc  : ctail implement for clog messages
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/TailCollector/BaseCTail.h 1     10-11-12 15:59 Admin $
// $Log: /ZQProjs/Generic/TailCollector/BaseCTail.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 1     6/24/05 2:34p Hui.shao
// ============================================================================================

#ifndef __BaseCTail_h__
#define __BaseCTail_h__

#include "NativeThread.h"
#include <string>

// -----------------------------
// class BaseCTail
// -----------------------------
class BaseCTail : public ZQ::common::NativeThread
{
public:

	/// Constructor
	///@param filename the filename to monitor
	BaseCTail(const char* filename = NULL, const bool checkIfFileExist = true);

	/// destructor
	virtual ~BaseCTail() {}
	
	/// stop the tail
	void stop(void) { _bQuit = true; }

	/// set the filename to monitor
	///@param filename   the filename to monitor
	void setFilename(const char* filename);

	/// get the monitored filename
	///@return		filename of the monitored clog file
	const char* getFilename();
	
protected:
	
	/// impl of NativeThread::init()
	virtual bool init(void);
	
	/// impl of NativeThread::run()
	virtual int run(void);
	
	/// impl of NativeThread::final()
	virtual void final(void) {}
	
protected:
	
	/// All tail execute by deriving the OnNewMessage method of BaseCTail. This method is called 
	/// once the tail engine detect a new log message has been added into the log file
	///@param line    NULL-terminated message that has been newly put into the log file
	virtual void OnNewMessage(const char* line)
	{
		printf("%s\n", line);
	}
	
public:
	
	static DWORD getLogPos(HANDLE hFile);
	
private:
	
	std::string _filename;
	long  _pos;
	bool   _bQuit;
	bool   _bCheckIfExist;
	
	int nextLogLine(char* buf, char** pline);
};

#endif // __BaseCTail_h__