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

#ifndef ZQTS_CPE_PACINGINTERFACE_H
#define ZQTS_CPE_PACINGINTERFACE_H


#include "ZQ_common_conf.h"
#include "Log.h"


class PacedIndexWrite
{
public:
        virtual ~PacedIndexWrite(){}
	virtual bool write(const char* buf, int len) = 0;
	virtual bool seek(int offset) = 0;
};

class PacedIndex
{
public:
	virtual ~PacedIndex(){}

	virtual void setLogHint(const char* strHint) = 0;

	/// tell pacing module that subfiles data written
	///@param[in] ext is the sub file extension name, for main file is empty, "." will be skipped
	///@param[in] size is the sub file buffer written size
	///@param[in] offset is the sub file buffer write offset
	///@return	  return false only if error happen
	virtual bool subfileWritten(const char* ext, int size, int64 offset) = 0;	

	/// the caller need to inherit from PacedIndexWrite and set to it	
	///@param[in] pWriter is the index writer callback
	virtual void setIndexWriter(PacedIndexWrite* pWriter) = 0;

	/// write the index data
	///@return	  return false only if error happen
	virtual bool writeIndex(const char* buf, int size, int offset) = 0;

	virtual void close() = 0;

	virtual void release() = 0;

};

class PacedIndexFactory
{
public:
	virtual ~PacedIndexFactory(){};

	virtual PacedIndex*	create(const char* type) = 0;

	virtual void setLog(ZQ::common::Log* pLog) = 0;

	virtual void setConfig(const char* szName, const char* szValue) = 0;
};
#endif
