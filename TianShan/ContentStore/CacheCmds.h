// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: CacheCmds.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/CacheCmds.h $
// 
// 6     7/25/12 4:12p Hui.shao
// added api to list hot locals
// 
// 5     5/10/12 6:50p Hui.shao
// kick off cache missed content per popularity
// 
// 4     1/18/12 1:50p Hui.shao
// linkable
// 
// 3     1/17/12 11:35a Hui.shao
// linkable
// 
// 2     1/16/12 10:35p Hui.shao
// defined the counter and topfolder at ice level
// 
// 1     12/22/11 1:38p Hui.shao
// created
// ===========================================================================

#ifndef __ZQTianShan_CacheCmds_H__
#define __ZQTianShan_CacheCmds_H__

#include "../common/TianShanDefines.h"

#include "CacheStoreImpl.h"

namespace ZQTianShan {
namespace ContentStore {

// -----------------------------
// class CacheCmd
// -----------------------------
///
class CacheCmd : protected ZQ::common::ThreadRequest
{
protected:
	/// constructor
	///@note no direct instantiation of ProvisionCmd is allowed
    CacheCmd(CacheStoreImpl& store);
	virtual ~CacheCmd();

public:

	void exec(void) { start(); }

protected: // impls of ThreadRequest

	virtual bool init(void)	{ return true; };
	virtual int run(void) { return 0; }
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected:
	CacheStoreImpl&     _store;
	typedef std::map<std::string, CacheCmd*> Map;
};

// -----------------------------
// class RefreshUnpopularListCmd
// -----------------------------
/// refresh the upopular list of a given top folder about the following fields:
/// totalSpaceMB, freeSpaceMB;
/// LValues                 contentsOfleaves;
/// long                    contentSubtotal;
class RefreshUnpopularListCmd : public CacheCmd
{
public:
	virtual ~RefreshUnpopularListCmd() {}

	static RefreshUnpopularListCmd* newCmd(CacheStoreImpl& store, TopFolderImpl& folder);

protected: // impls of BaseCmd
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
	RefreshUnpopularListCmd(CacheStoreImpl& store, TopFolderImpl& folder);
	
	virtual int run(void);

protected:

	TopFolderImpl& _topFolder;

	static Map _pendingMap; // a hash table to avoid duplicate
	static ::ZQ::common::Mutex _lkPending;
};

// -----------------------------
// class FlushAccessCounterCmd
// -----------------------------
class FlushAccessCounterCmd : public CacheCmd
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
	virtual ~FlushAccessCounterCmd() {}

	static FlushAccessCounterCmd* newCmd(CacheStoreImpl& store, ::TianShanIce::Storage::AccessCounter& countToFlush);

protected: // impls of BaseCmd
	FlushAccessCounterCmd(CacheStoreImpl& store, TianShanIce::Storage::AccessCounter& countToFlush)
		: CacheCmd(store), _countToFlush(countToFlush) {}

	virtual int run(void);
	
protected:
	TianShanIce::Storage::AccessCounter _countToFlush;

	static Map _pendingMap; // a hash table to avoid duplicate
	static ::ZQ::common::Mutex _lkPending;
};

// -----------------------------
// class ListContentsApt
// -----------------------------
class ListContentsApt
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
	ListContentsApt(CacheStoreImpl& store, ::Ice::Int maxNum)
		: _maxNum(maxNum) {}

	virtual ~ListContentsApt() {}

protected: // impls of BaseCmd
	virtual int list(::TianShanIce::Storage::ContentAccessList& results, ::TianShanIce::Storage::AccessRegistrarPtr& acList, uint32 timeWin, std::string& lastError);
	
private:
	::Ice::Int          _maxNum;
};

// -----------------------------
// class ListMissedContentsCmd
// -----------------------------
class ListMissedContentsCmd : public CacheCmd, protected ListContentsApt
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
	ListMissedContentsCmd(CacheStoreImpl& store, const ::TianShanIce::Storage::AMD_CacheStore_listMissedContentsPtr& amdCB, ::Ice::Int maxNum)
		: CacheCmd(store), _amdCB(amdCB), ListContentsApt(store, maxNum) {}

	virtual ~ListMissedContentsCmd() {}

protected: // impls of BaseCmd

	virtual int run(void);
	
protected:
	::TianShanIce::Storage::AMD_CacheStore_listMissedContentsPtr _amdCB;
};

// -----------------------------
// class ListHotLocalsCmd
// -----------------------------
class ListHotLocalsCmd : public CacheCmd, public ListContentsApt
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
	ListHotLocalsCmd(CacheStoreImpl& store, const ::TianShanIce::Storage::AMD_CacheStore_listHotLocalsPtr& amdCB, ::Ice::Int maxNum)
		: CacheCmd(store), _amdCB(amdCB), ListContentsApt(store, maxNum) {}

	virtual ~ListHotLocalsCmd() {}

protected: // impls of BaseCmd

	virtual int run(void);
	
protected:
	::TianShanIce::Storage::AMD_CacheStore_listHotLocalsPtr _amdCB;
};

// -----------------------------
// class ImportContentCmd
// -----------------------------
class ImportContentCmd : public CacheCmd
{
public:
	/// constructor
	///@note no direct instantiation of SessionCommand is allowed
	ImportContentCmd(CacheStoreImpl& store, const std::string& contentName);
	virtual ~ImportContentCmd() {}

protected: // impls of BaseCmd

	virtual int run(void);
	
protected:
	::std::string _contentName;
};

}} // namespace

#endif // __ZQTianShan_CacheCmds_H__

