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
// Desc  : define time stamp
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/Timestamp.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/Timestamp.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 6     05-05-26 11:38 Daniel.wang
// 
// 5     4/14/05 10:11a Hui.shao
// 
// 4     4/13/05 6:33p Hui.shao
// changed namespace
// 
// 3     4/12/05 5:16p Hui.shao
// ============================================================================================


#ifndef __Timestamp_h__
#define __Timestamp_h__

#include "EntryDB.h"
#include "EDB.h"

ENTRYDB_NAMESPACE_BEGIN

class ENTRYDB_API Timestamp;
class ENTRYDB_API eIDset;

class Timestamp
{
	friend class eIDset;

public:
	Timestamp(const FILETIME* ft=NULL, bool isLocal=false);
	Timestamp(const WORD wFatDate, const WORD wFatTime); // DOS datetime
	Timestamp(const Timestamp& ts); // copier
	Timestamp(const char* hex_str);
	
	long compare(const Timestamp& ts);
	long difference(const Timestamp &ts);
	void addDiff(const float seconds);
	bool toLocal(Timestamp& ts);
	bool toSystemtime(SYSTEMTIME& SystemTime);
	bool toDosDatetime(WORD& FatDate, WORD& FatTime);
	const char* hex_str(char *buf);
	const char* display_str(char* buf, bool withnsec=false);

#ifndef WIN32
	static FILETIME AdjustForTimezone(const FILETIME *Orig, int Direction);
	static FILETIME UnixTimeToFileTime(time_t sec, long nsec);
	static time_t	FileTimeToUnixTime(FILETIME FileTime, long *nsec);
#endif // WIN32

private:
	FILETIME mFT;
};

#define SECS_TO_100NS	10000000; // 10^7
#define MSEC_TO_100NS	10000; // 10^4

class eIDset
{
public:
	eIDset(void);
	virtual ~eIDset(void);
	const char* operator()(void);

private:
	uint32 mhid;
	uint32 mlid;

	std::string idstr;
};

extern eIDset ENTRYDB_API newID;

ENTRYDB_NAMESPACE_END

#endif // __Timestamp_h__
