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
// Desc  : define the entry database interface
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/IEDB.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/IEDB.h $
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
// 6     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 5     4/12/05 5:39p Hui.shao
// ============================================================================================

#ifndef __IEDB_h__
#define __IEDB_h__

#include "EntryDB.h"

ENTRYDB_NAMESPACE_BEGIN

class ITransaction
{
public:
	virtual bool commit() = 0;
	virtual bool abort() = 0;
};

class IEDB
{
public:
	virtual ~IEDB() {}

	virtual bool connectDB(const char* url, ITransaction* txn = NULL) =0;
	virtual bool isDBConnected() =0;
	virtual void disconnectDB() =0;
	virtual void free() =0;

	virtual const char* getDBURL() =0;

public:
	virtual bool openEntry(const char* e, bool creatIfNotExist=false, ITransaction* txn = NULL) =0;
	virtual bool commitChanges(ITransaction* txn = NULL) =0;
	virtual bool deleteEntry(const char* e=NULL, ITransaction* txn = NULL) =0;
	virtual bool copyEntry(const char* eto, const char* efrom=NULL, bool overwrite=false, ITransaction* txn = NULL) =0;

	virtual bool openRoot(ITransaction* txn = NULL) =0;
	virtual bool openParent(ITransaction* txn = NULL) =0;
	virtual bool openNextSibling(ITransaction* txn = NULL) =0;
	virtual bool openFirstChild(ITransaction* txn = NULL) =0;

	virtual bool openFirstGE(const char* e, ITransaction* txn = NULL) =0;
	virtual bool openNext(ITransaction* txn = NULL) =0;

	virtual const char* getCurrentEntry() =0;

	virtual const char* getAttrName(const int index, ITransaction* txn = NULL) =0;
	virtual const char* getAttribute(const char* attrname, ITransaction* txn = NULL) =0;
	virtual bool setAttribute(const char* attrname, const char*value, ITransaction* txn = NULL) =0;

//	virtual const char* getRealEntry() =0;
	virtual bool setReference(const char* e, ITransaction* txn = NULL) =0;
	virtual bool isReference() =0;
	virtual bool associateReference() =0;

	virtual void createTxn(ITransaction*& txn) = 0;
	virtual void deleteTxn(ITransaction*& txn) = 0;

};

/*
class IEDBFactory
{
public:
	virtual bool populate(const char* path=NULL, const char* ext=ADDIN_MOD_EXT)=0;

	virtual IEDB* Connect(const char *url)=0;

	//about the addins
	virtual const char* ModuleFile(int i)=0;
	virtual const char* ModuleURLHelp(int i)=0;
	virtual const char* ModuleDBDesc(int i)=0;
	virtual const bool  isModuleInternal(int i)=0;
};
*/

#ifndef LOGIC_SEPS
#  define LOGIC_SEPC '/'
#  define LOGIC_SEPS "/"
#endif

ENTRYDB_NAMESPACE_END

#endif // __IEDB_h__
