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
// Desc  : generic entry database usage
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDB.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDB.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 12    05-07-28 18:39 Daniel.wang
// 
// 11    05-05-10 16:00 Daniel.wang
// 
// 10    05-05-08 16:05 Daniel.wang
// 
// 9     05-04-29 11:38 Daniel.wang
// 
// 8     4/14/05 10:24a Hui.shao
// 
// 7     4/14/05 10:17a Hui.shao
// 
// 6     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 5     05-04-12 20:26 Daniel.wang
// 
// 4     4/12/05 5:46p Hui.shao
// ============================================================================================

#ifndef __EDB_h__
#define __EDB_h__

#include "EntryDB.h"
#include "IEDB.h"
#include "Addins.h"

#include <iostream>

ENTRYDB_NAMESPACE_BEGIN

class ENTRYDB_API EDB;
class ENTRYDB_API EDBAddin;
class ENTRYDB_API EDBFactory;
class ENTRYDB_API EDBTxn;

#define HIDDEN_ATTR_PREF	".#$"

class Transaction
{
	friend class EDB;

private:
	ITransaction*	m_pTransaction;

	Transaction(ITransaction* pTxn);

public:
	bool commit();
	bool abort();
};

class EDB
{
public:
	EDB(const char* url = NULL);
	~EDB();

	virtual bool connect(const char* url);
	virtual bool isConnected();
	virtual void disconnect();

	virtual const char* URL();
	
public:
	virtual bool openEntry(const char* e, bool creatIfNotExist=false, Transaction* txn = NULL);
	virtual bool rollbackAttrs(Transaction* txn = NULL);
	virtual bool deleteEntry(const char* e=NULL, Transaction* txn = NULL);
	virtual bool copyEntry(const char* eto, const char* efrom=NULL, bool overwrite=false, Transaction* txn = NULL);

	virtual bool openRoot(Transaction* txn = NULL);
	virtual bool openParent(Transaction* txn = NULL);
	virtual bool openNextSibling(Transaction* txn = NULL);
	virtual bool openFirstChild(Transaction* txn = NULL);

	virtual const char* getCurrentEntry(bool basename=false);

	virtual const char* getAttrName(const int index, Transaction* txn = NULL);
	virtual const char* getAttribute(const char* attrname, Transaction* txn = NULL);
	virtual bool setAttribute(const char* attrname, const char*value, Transaction* txn = NULL);

	// own methods
	//@depth the depth of children, -1 means all the level until the leaves, 0 means no children
	virtual bool import(EDB& edb, const char *at=NULL, int depth = -1 /*bool copychild = false*/, Transaction* txn = NULL); // @at=NULL means current entry
	virtual bool export(EDB& edb, const char *at=NULL, int depth = -1, Transaction* txn = NULL);
	virtual bool xexport(std::ostream &out, const char *at=NULL, Transaction* txn = NULL);
	virtual bool xexport(const char *filename=NULL, const char *at=NULL, Transaction* txn = NULL);

	virtual bool createTxn(Transaction*& txn);
	virtual void deleteTxn(Transaction*& txn);
	
	static EDBFactory* pFactory;

protected:

	virtual bool xexp(int depth, std::ostream &out, const char* elyield="  ", const char* atyield=NULL, Transaction* txn = NULL);
	static bool copybranch(EDB& from, EDB& to, int depth, Transaction* txn = NULL);
	IEDB*	   pIEDBImpl;
	bool	bDirty;
};

class EDBAddin : public DSOInterface
{
	friend class EDBSupplyCtrl;

	DECLARE_PROC_SOI(EDBAddin, DSOInterface)

	DECLARE_MANAGED_OBJ(EDBAddin)

	virtual int64 instanceCount(void);

	DEFINE_PROC(IEDB*, Connect, (const char* url))
	DEFINE_PROC(const char*, URLProtocol, (int i))
	DEFINE_PROC(const char*, URLHelp, (int i))
	DEFINE_PROC(const char*, EDBType, (int i))
	DEFINE_PROC(const char*, EDBDesc, (int i))
	DEFINE_PROC(const int64, insCount, (void))

	PROC_TABLE_BEGIN()
//		IMPLEMENT_DSOI_PROC(EDBConnect)
		IMPLEMENT_DSOI_PROC_SPECIAL(Connect, "EDBConnect")
		IMPLEMENT_DSOI_PROC(URLProtocol)
		IMPLEMENT_DSOI_PROC(URLHelp)
		IMPLEMENT_DSOI_PROC(EDBType)
		IMPLEMENT_DSOI_PROC(EDBDesc)
		IMPLEMENT_DSOI_PROC(insCount)
	PROC_TABLE_END()

public:
	bool isInternal;
	virtual bool isValid() { return (isInternal ||DSOInterface::isValid()); };
};

class EDBFactory : public FactoryModule<EDBAddin>
{
public:
	EDBFactory();
	EDBFactory(AddinManager& admg);
	~EDBFactory();

	IEDB* Connect(const char *url);

	const char* ModuleFile(int i);
	const char* ModuleURLHelp(int i);
	const char* ModuleDBDesc(int i);
	const bool  isModuleInternal(int i);
	const int64 ModuleInstance(int i);
private:
	void init();
};

ENTRYDB_NAMESPACE_END

#endif // __EDB_h__
