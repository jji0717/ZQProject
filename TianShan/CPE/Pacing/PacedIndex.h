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

#ifndef ZQTS_CPEPLG_PACEDINDEX_H
#define ZQTS_CPEPLG_PACEDINDEX_H



#include "PacingInterface.h"
#include "VvcIndexHelper.h"
#include "PacingLogic.h"
#include <string>
#include <map>

class PacedIndexVvcFactory : public PacedIndexFactory
{
public:
	PacedIndexVvcFactory();
	~PacedIndexVvcFactory();

	virtual PacedIndex*	create(const char* type);

	virtual void setLog(ZQ::common::Log* pLog);

	virtual void setConfig(const char* szName, const char* szValue);
protected:

	ZQ::common::Log*	_log;
};

class PacedIndexVvc : public PacedIndex
{
protected:
	PacedIndexVvc();
	~PacedIndexVvc();

	friend class PacedIndexVvcFactory;
public:	

	virtual void setLogHint(const char* strHint);

	virtual void setLog(ZQ::common::Log* pLog);

	virtual void setIndexWriter(PacedIndexWrite* pWriter);

	virtual bool writeIndex(const char* buf, int size, int offset);

	virtual bool subfileWritten(const char* ext, int size, int64 offset);	

	virtual void close();

	virtual void release();

protected:

	bool paceRecords();

	bool releaseAllRecs();

protected:
	PacedIndexWrite*	_pIndexWriter;

	PacingLogic			_pacingLogic;
	VvcIndexHelper		_indexHelper;

	ZQ::common::Log*	_log;
	std::string			_strLogHint;
};


#endif

