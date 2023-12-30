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
// Author: Daniel Wang
// Desc  : entry database berkeley database 4 edition
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBb4.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBb4.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 10    05-07-28 18:39 Daniel.wang
// 
// 9     05-06-24 9:12p Daniel.wang
// 
// 8     05-05-10 16:00 Daniel.wang
// 
// 7     05-05-08 16:05 Daniel.wang
// 
// 6     05-04-29 11:38 Daniel.wang
// 
// 5     4/14/05 10:13a Hui.shao
// 
// 4     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 3     4/12/05 6:44p Hui.shao
// ============================================================================================


#ifndef __EDBb4_h__
#define __EDBb4_h__

#include "EDBImpl.h"
#include <db_cxx.h>

#ifdef WIN32
#  if defined(_MSC_VER) && !defined(_MT)
#     error Please enable multithreading
#  endif
#  ifdef _DEBUG
#     pragma comment(lib, "libdb43sd.lib")
#  else
#     pragma comment(lib, "libdb43s.lib")
#  endif
#endif

ENTRYDB_NAMESPACE_BEGIN

#define EBDB4URLPROT "edbb4"


class EBDB4Env
{
private:
	static DbEnv	s_dbenv;
	static size_t	s_count;

public:
	EBDB4Env();
	~EBDB4Env();

	DbEnv& getInst();
	size_t count();
};

class TxnEdbb4 : public ITransaction
{
private:
	EBDB4Env	env;
	DbTxn*		txn;

public:
	TxnEdbb4();
	virtual ~TxnEdbb4();

	bool commit();
	bool abort();

	DbTxn*& getInst();
};

class EDBb4 : public EDBImpl
{
	friend class EDBb4Mgr;

public:
	EDBb4(const char* dbpath, const char*errf=NULL, ITransaction* txn = NULL);
	EDBb4();
	~EDBb4();

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
//	const char* getRealEntry();

	const char* getAttrName(const int index, ITransaction* txn = NULL);
	const char* getAttribute(const char* attrname, ITransaction* txn = NULL);
	bool setAttribute(const char* attrname, const char*value, ITransaction* txn = NULL);
	bool isReference();
	bool associateReference();

	void createTxn(ITransaction*& txn);
	void deleteTxn(ITransaction*& txn);

#ifdef _DEBUG
	void navigate(ITransaction* txn = NULL);
	void navigateN(int depth, ITransaction* txn = NULL);
#endif //_DEBUG

private:
	int writeUTF8(char*buf, const char*str);
	int readUTF8(const char*buf, char*str);
	void attrs4DBData();
	void populateDBData();
	void initworkdata(int size =-1);

	typedef struct _EntryDB4_CB
	{
		std::string dbpath;
		FILE* errf;
		Db*  dbp;
		//Dbc* cur;
		EBDB4Env env;
		
		//EDBb4Txn* txn;
	} EDBb4_CB_t;
	EDBb4_CB_t mECB;


	uint8 *wk_data;
	int   wk_datasize;
};

#ifndef _sdels
#define _sdels(_X) {if (_X) delete[] _X; _X = NULL; }
#endif
#ifndef _sdel
#define _sdel(_X) {if (_X) delete _X; _X = NULL; }
#endif

ENTRYDB_NAMESPACE_END

#endif // __EDBb4_h__

