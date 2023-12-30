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

#ifndef ZQTS_CPEPLG_VVCINDEXHELPER_H
#define ZQTS_CPEPLG_VVCINDEXHELPER_H


#include "vvccommon.h"
#include <string>
#include <map>


class VvcIndexHelper
{
public:
	struct VVCIndexHeader
	{
		int a;
	};

	enum ParseState
	{
		PS_FILEHEADER,
		PS_INDEXHEADER,
		PS_INDEXREC
	};

	enum 
	{
		INDEX_BUFFER_SIZE = 128*1024
	};

	VvcIndexHelper();

	bool appendIndexData(const char* buf, int size);	

	/// if data length is > 0, then it would remove the data, or no any action
	bool removeIndexData();
	
	
	bool moveForwardData(int nParsedBytes);
	
	/// get the current index offset
	int	getInputDataOffset();


	/// get data for output
	const char* getOutputDataPointer();
	int	getOutputDataLength();

	/// get the current index output offset
	int	getOutputDataOffset();

	int getAllDataLength();
	//
	// return value, 0 for data not enough, <0 for error, >0 is the parsed bytes
	//

	int parseFileHeader(VVC_INDEX_FILE_HEADER** ppFileHeader);
	int parseHeaderBlock(char** ppHeadBlock, int& nBlockLen);
	int parseIndexRecord(VVC_INDEX_RECORD_HEADER** ppRecHeader, char** ppRecord, int& nRecordLen);


	ParseState getParseState();


protected:
	CTF_TLV* alignTLV(char *pBuf, int *pIndex );

protected:

	int					_nDataLen;
	char				_buf[INDEX_BUFFER_SIZE];

	int					_nParseStartBytes;		//related to moveforward
//	int					_nParsedDataBytes;		

	int					_nParseState;
	int					_nInputDataOffset;

	int					_nTotalBytes;
};



#endif

