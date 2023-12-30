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
// Desc  : define EntryDB implementation by using expatxx
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/ExpatDB.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/ExpatDB.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 5     4/14/05 10:17a Hui.shao
// 
// 4     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 3     4/12/05 5:39p Hui.shao
// ============================================================================================

#ifndef __ExpatDB_h__
#define __ExpatDB_h__

#include "EDB.h"

#include <assert.h>
#include <fstream>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <vector>

ENTRYDB_NAMESPACE_BEGIN

class ENTRYDB_API ExpatDB;
class ExpatDBNesting;

// class ExpatDB
class ExpatDB : public EDB
{
	friend class ExpatDBNesting;
	
public:
	
	ExpatDB(const char *szFilename=NULL, bool preInMemory=true);
	ExpatDB(const char *szBuffer, const int nBufferLen, const int nFinal = 0);
	virtual ~ExpatDB();
	
public:

	bool parseXML(const char *szFilename);
	bool parseXML(const char *szBuffer, const int nBufferLen, const int nFinal = 0);

	// Get/set the content portion of an element
	const char* getContent();
	bool setContent(const char *szContent);

	virtual const char* getElementName();

	virtual bool openFirstChild(char *name);
	virtual bool openNextSibling(bool sameElementType);
	virtual bool newChild(char *name);

	virtual const char* lastError();

public:
	bool bLogicalClosed;

protected:
	
	virtual bool xexp(int depth, std::ostream &out, const char* elyield="  ", const char* atyield=NULL);

	std::string crnt_element_type;
	std::string error;
	ExpatDBNesting *pEMN;

};

#define CONTENT_ATTR		HIDDEN_ATTR_PREF "content"
#define ELEM_TYPE_ATTR		HIDDEN_ATTR_PREF "type"


ENTRYDB_NAMESPACE_END

#endif // __ExpatDB_H__

