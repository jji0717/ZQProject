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
// Desc  : define in-memory entry database
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBNil.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBNil.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 10    05-07-28 18:39 Daniel.wang
// 
// 9     05-05-10 16:00 Daniel.wang
// 
// 8     05-05-08 16:05 Daniel.wang
// 
// 7     05-04-29 11:38 Daniel.wang
// 
// 6     4/14/05 10:13a Hui.shao
// 
// 5     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 4     4/12/05 5:38p Hui.shao
// ============================================================================================

#ifndef __EDBNil_h__
#define __EDBNil_h__

#include "EDBImpl.h"

#define EDBNILURLPROT "edbnil"

ENTRYDB_NAMESPACE_BEGIN

class EDBNil : public EDBImpl
{
public:
	EDBNil(const char* dbpath, const char*errf=NULL, ITransaction* txn = NULL);
	EDBNil();
	~EDBNil();

	bool connectDB(const char* url, ITransaction* txn = NULL);
	bool isDBConnected();
	void disconnectDB();
	void free();

	const char* getDBURL();

	bool openEntry(const char* e, bool creatIfNotExist=false, ITransaction* txn = NULL);
	bool setReference(const char* e, ITransaction* txn = NULL);
	bool commitChanges(ITransaction* txn = NULL);
	bool deleteEntry(const char* e=NULL, ITransaction* txn = NULL);
	bool copyEntry(const char* eto, const char* efrom=NULL, bool overwrite=false, ITransaction* txn = NULL);

	bool openRoot(ITransaction* txn = NULL);
	bool openParent(ITransaction* txn = NULL);
	bool openNextSibling(ITransaction* txn = NULL);
	bool openFirstChild(ITransaction* txn = NULL);

	bool openFirstGE(const char* e, ITransaction* txn = NULL);
	bool openNext(ITransaction* txn = NULL);

	const char* getCurrentEntry();

	const char* getAttrName(const int index, ITransaction* txn = NULL);
	const char* getAttribute(const char* attrname, ITransaction* txn = NULL);
	bool setAttribute(const char* attrname, const char*value, ITransaction* txn = NULL);
	bool isReference();
	bool associateReference();

	void createTxn(ITransaction*& txn);
	void deleteTxn(ITransaction*& txn);

	static int64 cInstance;

private:

	// Define the element map
	typedef std::map< std::string, attrs_t > entry_map_t;
	entry_map_t mEDB;
};

IEDB* EDBNilConnect(const char* url);

const char* EDBNilURLHelp(int i);
const char* EDBNilURLProtocol(int i);
const char* EDBNilType(int i);
const char* EDBNilDesc(int i);
const int64 EDBNilCount(void);

ENTRYDB_NAMESPACE_END

#endif // __EDBNil_h__

