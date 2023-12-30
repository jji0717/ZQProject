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
// Desc  : entry db implementation with berkeley database 2
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBb2.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBb2.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 6     05-05-08 16:05 Daniel.wang
// 
// 5     4/14/05 10:13a Hui.shao
// 
// 4     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 3     4/12/05 5:59p Hui.shao
// ============================================================================================

#ifndef __EDBb2_h__
#define __EDBb2_h__

#include "EDBImpl.h"
#include "db_cxx.h"

#ifdef WIN32
#  if defined(_MSC_VER) && !defined(_MT)
#     error Please enable multithreading
#  endif
#  pragma comment(lib, "libdb2s.lib")
#endif

ENTRYDB_NAMESPACE_BEGIN

#define EBDB2URLPROT "edbb2"

class EDBb2 : public EDBImpl
{
	friend class EDBb2Mgr;

public:
	EDBb2(const char* dbpath, const char*errf=NULL);
	EDBb2();
	~EDBb2();

	bool connectDB(const char* url);
	bool isDBConnected();
	void disconnectDB();
	void free();
	const char* getDBURL();

	bool openEntry(const char* e, bool creatIfNotExist=false);
	bool setReference(const char* e);
	bool commitChanges();
	bool deleteEntry(const char* e=NULL);
	bool copyEntry(const char* eto, const char* efrom=NULL, bool overwrite=false);

	bool openRoot();
	bool openParent();
	bool openNextSibling();
	bool openFirstChild();

	bool openFirstGE(const char* e);
	bool openNext();

	const char* getCurrentEntry();
//	const char* getRealEntry();

	const char* getAttrName(const int index);
	const char* getAttribute(const char* attrname);
	bool setAttribute(const char* attrname, const char*value);
	bool isReference();
	bool associateReference();

	virtual EDBTxnImpl* createTransaction() { return NULL; };
	virtual bool setTransaction(EDBTxnImpl& txn) { return false; };
	virtual bool getCurrentTransaction(EDBTxnImpl& txn) { return false; };

#ifdef _DEBUG
	void navigate();
	void navigateN(int depth);
#endif //_DEBUG

private:
	int writeUTF8(char*buf, const char*str);
	int readUTF8(const char*buf, char*str);
	void attrs2DBData();
	void populateDBData();
	void initworkdata(int size =-1);

	typedef struct _EntryDB2_CB
	{
		std::string dbpath;
		FILE* errf;
		Db*  dbp;
		Dbc* cur;
	} EDBb2_CB_t;
	EDBb2_CB_t mECB;

	uint8 *wk_data;
	int   wk_datasize;
};

ENTRYDB_NAMESPACE_END

#endif // __EDBb2_h__

