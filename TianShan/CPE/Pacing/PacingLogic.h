// ===========================================================================
// Copyright (c) 2010 by
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
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================

#ifndef ZQTS_CPEPLG_PACINGLOGIC_H
#define ZQTS_CPEPLG_PACINGLOGIC_H


#include "vvccommon.h"
#include <string>
#include "Log.h"

#define NEW_VVC_DEFINE	(VVC_INDEX_MAJOR_VERSION>=1 && VVC_INDEX_MINOR_VERSION>=1)

class PacingLogic
{
public:
	PacingLogic();

	void setLog(ZQ::common::Log* pLog);

	void setLogHint(const char* strHint );

	///@return, if ext is not correct, return false, else return true
	bool subFileWritten(const char* ext, int size, int64 offset);

	bool parseIndexHeadBlock(char* pHeadBlock, int nLen);

	///@return, if the data is releasable, return true
	bool indexReleasable(VVC_INDEX_RECORD_HEADER* ppRecHeader, char* pRecord, int nRecordLen);


protected:

	enum
	{
		MAX_SUBFILE_COUNT = (1 + 2 * 12)
	};

	struct SubfileInfo
	{
		int64	startingOffset;
		int64	endingOffset;
		int		direction;
		std::string	strExt;

		SubfileInfo()
		{
			startingOffset = 0x7ffffffffffff;			//set to max value
			endingOffset = 0;			//set to min value
			direction = 0;
		}
	};

#if NEW_VVC_DEFINE
	/// trick file index record releasable or not
	bool trickIndexReleasable(VVC_INDEX_RECORD_MP2_TRICK_IFRAME* pFrame);

	/// normal file index record releasable or not
	bool normalIndexReleasable(VVC_INDEX_RECORD_MP2_NORMAL_IFRAME* pFrame);
#else
	/// trick file index record releasable or not
	bool trickIndexReleasable(VVC_INDEX_RECORD_TRICK_RATE_IFRAME* pFrame);

	/// normal file index record releasable or not
	bool normalIndexReleasable(VVC_INDEX_RECORD_NORMAL_RATE_IFRAME* pFrame);
#endif


	///@return <0 if error, if success, value <MAX_SUBFILE_COUNT
	int extToSubfileId(const char* ext);
	CTF_TLV* alignTLV(char *pBuf, int *pIndex );

	std::string getExtFromName(const char* name);

	/// this function will do these 3 things, for mapping the ext to file id
	/// 1. remove "." if exist
	/// 2. convert lower case char to upper case
	/// 3. if the "ext" is empty, return "mpg", for main file
	std::string convertExt(const char* ext);

	std::string		_strLogHint;

	SubfileInfo		_subFiles[MAX_SUBFILE_COUNT];
	int				_nSubFiles;

	ZQ::common::Log*	_log;
};


#endif

