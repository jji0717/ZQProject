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
// Desc  : generic entry database impl
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBImpl.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBImpl.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 13    05-07-28 18:39 Daniel.wang
// 
// 12    05-05-10 16:00 Daniel.wang
// 
// 11    05-05-08 16:05 Daniel.wang
// 
// 10    05-04-29 11:38 Daniel.wang
// 
// 9     4/14/05 10:13a Hui.shao
// 
// 8     4/13/05 6:51p Hui.shao
// changed namespace
// 
// 7     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 6     4/12/05 5:46p Hui.shao
// ============================================================================================

#ifndef __IEDBIMPL_h__
#define __IEDBIMPL_h__

#include "EntryDB.h"
#include "IEDB.h"

#include <map>
#include <string>

#ifdef _DEBUG
#define ASSET_BOOL(_X) if (!(_X)) return false
#else
#define ASSET_BOOL NULL
#endif

typedef struct edbimp_decl_
{
	const char *urlpt;
	const char *edbtype, *urlhlp;
	const char * edbdesc;
} edbimp_decl_t;

#ifdef ENTRYDB_WITH_NAMESPACE
#  define EDBIMPL_DECLARE_BEGIN \
		using namespace ENTRYDB_NAMESPACE; \
		int64 _nInstance_in_EDM =0; \
		const edbimp_decl_t decl_tbl[] = { 
#else // ENTRYDB_WITH_NAMESPACE
#  define EDBIMPL_DECLARE_BEGIN \
		int64 _nInstance_in_EDM =0; \
		const edbimp_decl_t decl_tbl[] = { 
#endif // ENTRYDB_WITH_NAMESPACE

#define EDBIMPL_DECLARE_DB(_DBTYPE, _URL, _URLHELP, _DBDESC) \
{_URL, _DBTYPE, _URLHELP, _DBDESC },

#define EDBIMPL_DECLARE_END \
}; \
const char* URLHelp(int i) { return (i<(sizeof(decl_tbl)/sizeof(edbimp_decl_t)) ? decl_tbl[i].urlhlp : NULL); } \
const char* URLProtocol(int i) { return (i<(sizeof(decl_tbl)/sizeof(edbimp_decl_t)) ? decl_tbl[i].urlpt : NULL); } \
const char* EDBType(int i) { return (i<(sizeof(decl_tbl)/sizeof(edbimp_decl_t)) ? decl_tbl[i].edbtype : NULL); } \
const char* EDBDesc(int i) { return (i<(sizeof(decl_tbl)/sizeof(edbimp_decl_t)) ? decl_tbl[i].edbdesc : NULL); } \
const int64 insCount(void) { return _nInstance_in_EDM; }

#define EDBIMPL_DECLARE_DBITEM(_I) decl_tbl[i]

#define EDBIMPL_EXPORT_BEGIN \
	IEDB* EDBConnect(const char* url) { \
		if (url ==NULL || *url==0x00) return NULL;

#define EDBIMPL_EXPORT_DB(_DB) { \
	_DB* __edb_ptr = new _DB; \
	if (__edb_ptr!=NULL) { \
	if (__edb_ptr->connectDB(url)) return __edb_ptr; \
	delete __edb_ptr; __edb_ptr = NULL; } }

#define EDBIMPL_EXPORT_END return NULL; }

#ifdef ENTRYDB_WITH_NAMESPACE
  extern "C" __declspec(dllexport) ENTRYDB_NAMESPACE::IEDB* EDBConnect(const char* url);
#else // ENTRYDB_WITH_NAMESPACE
  extern "C" __declspec(dllexport) IEDB* EDBConnect(const char* url);
#endif // ENTRYDB_WITH_NAMESPACE
extern "C" __declspec(dllexport) const char* URLHelp(int i);
extern "C" __declspec(dllexport) const char* URLProtocol(int i);
extern "C" __declspec(dllexport) const char* EDBType(int i);
extern "C" __declspec(dllexport) const char* EDBDesc(int i);
extern "C" __declspec(dllexport) const int64 insCount(void);

extern int64 _nInstance_in_EDM;

ENTRYDB_NAMESPACE_BEGIN

#ifndef attrs_t
   typedef std::map<std::string, std::string> attrs_t;
#endif

#define ENTRY_TYPE "_$et"

/*
class TransactionImpl : ITransaction
{
public:
	bool commit() = 0;
	bool abort() = 0;
};
*/

class EDBImpl : public IEDB
{
public:
	EDBImpl() {};
	virtual ~EDBImpl() {};

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

	attrs_t mAttrs;
	std::string crntentry;
	//std::string realentry;

	std::string mUrl;

};

ENTRYDB_NAMESPACE_END

#endif // __IEDBIMPL_h__
