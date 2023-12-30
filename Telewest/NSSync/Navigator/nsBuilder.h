// ===========================================================================
// Copyright (c) 2004 by
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
// Name  : nsBuilder.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : class for builder NAV database
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/nsBuilder.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 6     08-12-11 11:42 Li.huang
// Remove some restriction on MAX_WQ_HISTORY, since we dont have to follow
// this
// 
// 5     07-03-16 15:25 Ken.qian
// 
// 4     12/09/06 5:21a Bernie.zhao
// modified architechture to support QA Navigation.
// 
// 3     06-10-19 11:13 Ken.qian
// Change the Optimizer
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================


#if !defined(AFX_NSBUILDER_H__12E49297_8BD2_4EBF_9216_D251349DE6A9__INCLUDED_)
#define AFX_NSBUILDER_H__12E49297_8BD2_4EBF_9216_D251349DE6A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// external include
#include <deque>

#include "manpkg.h"

// common include
#include "Locks.h"

using namespace ZQ::common;

// local include
#include "nsOptimizer.h"
#include "navWorkerProvider.h"
#include "WQ_INFO.h"
#include "navFactory.h"


// max work queue entries in memory
// #define MAX_WQ_HISTORY			500

// special queue_ID for NSSync itself generated optimization 'rebuild' entry
#define	WQ_OPT_REBUILD_QUEUEID	"TooManyEntriesInWQ"


class nsTimer;

typedef		std::deque<WQ_Entry>	WQEntryList;

class nsBuilder  
{
public:
	enum Status
	{
		/// the builder is not active
		DISABLE		=1,
		/// the builder is idle
		IDLE,
		/// the builder is working	
		BUILDING,
	};
	
//////////////////////////////////////////////////////////////////////////
// constructor and destructor
public:
	nsBuilder();
	virtual ~nsBuilder();

//////////////////////////////////////////////////////////////////////////
// object reference methods	
public:
	/// method to reference optimizer object
	///@return	pointer to the optimizer object
	nsOptimizer* getOptimizer() { return _pTheOptimizer; }

	/// method to reference worker factory object
	///@return	pointer to the factory object
	navWorkerProvider* getWorkerProvider() { return _pTheWorkerProvider; }

	/// method to reference work queue record set objec
	///@return	pointer to work queue record set object
	CWQ_INFO* getDBWQ() { return _pTheDBWQ; }

//////////////////////////////////////////////////////////////////////////
// operations
public:
	/// initial builder
	bool init();

	/// uninitial builder
	void uninit();

	/// build the NE DB according to Work-Queue
	///@return	error code.
	int build();

	/// signal the builder to stop working
	void signalTerm() { _bShouldTerm = TRUE; _status = DISABLE; }

//////////////////////////////////////////////////////////////////////////
// attribute methods
public:
	/// get builder status
	///@return	status
	Status getStatus() { return _status; }

	/// get ODBC DSN name string
	///@return	Local AM ODBC DSN name
	CString getDSN() { return _dsn; }

	/// set ODBC DSN name
	///@param[in]	dsn		Local AM ODBC DSN name
	void setDSN(CString dsn) { _dsn = dsn; }

	/// get DB user name string
	///@return	Local AM DB user name
	CString getUID() { return _uid; }

	/// set DB user name
	///@param[in]	uid		Local AM DB user name
	void setUID(CString uid) { _uid = uid; }

	/// get DB user password
	///@return	Local AM DB user password
	CString getPWD() { return _pwd; }

	/// set DB user password
	///@param[in]	pwd		Local AM DB user password
	void setPWD(CString pwd) { _pwd = pwd; }

	/// get DB name
	///@return	Local AM DB name
	CString getDATABASE() { return _database; }

	/// set DB name
	///@param[in]	database	Local AM DB name
	void setDATABASE(CString database) { _database = database; }

	/// get DB timeout
	DWORD getTimeout() { return _timeout; }

	/// set DB timeout
	void setTimeout(DWORD timeout) { _timeout = timeout; }

	/// set/get the _maxWQOptRebuild
	void setMaxWQOptRebuild(DWORD max) { _maxWQOptRebuild = max; }
	DWORD getMaxWQOptRebuild() { return _maxWQOptRebuild; }

//////////////////////////////////////////////////////////////////////////
// work queue operations
public:
	/// get all work queue into local container
	///@return 1 stands for we have work to do; 0 stands for we have no work queue in DB; -1 stands for error
	int fetchDBWQ(bool QAfunction);

	/// delete specified work queue entry in db
	///@param[in]	queueID		the UID of the queue entry to be deleted
	///@return				TRUE if successfully, FALSE else
	bool deleteDBWQ(CString queueID);

	/// add an entry into NAV memory work queue list
	///@param[in]	entry		the entry to be added
	bool addNAVMemWQ(WQ_Entry& entry);

	/// add an entry into QA memory work queue list
	///@param[in]	entry		the entry to be added
	bool addQAMemWQ(WQ_Entry& entry);

//////////////////////////////////////////////////////////////////////////
// help functions to check QA flags
public:
	int	getQAFlag(CString spName);

	CString	qaFlagToString(CString spName, int flagValue);
	
	//////////////////////////////////////////////////////////////////////////
	// ManUtil callbacks
public:
	static MANSTATUS FAR WINAPI wqInfoCallBack(WCHAR *  pwszCmd, WCHAR ** ppwszResponse, DWORD *  pdwLength );

	static MANSTATUS FAR WINAPI qaWqInfoCallBack(WCHAR *  pwszCmd, WCHAR ** ppwszResponse, DWORD *  pdwLength );

//////////////////////////////////////////////////////////////////////////
// related objects
protected:

	friend class nsOptimizer;
	friend class navWorkerProvider;
	friend class navFactory;

	/// pointer to work queue execute optimizer
	nsOptimizer* _pTheOptimizer;

	/// pointer to db builder worker factory
	navWorkerProvider* _pTheWorkerProvider;

	/// pointer to NAV factory object
	navFactory*	_pTheNavFactory;

	/// pointer to QA factory object
	navFactory*	_pTheQAFactory;
	
	/// pointer to the db source of below record set
	CDatabase*	_pTheDBSource;

	/// lock for DB source
	ZQ::common::Mutex	_theDBSourceLock;

	/// WQ_INFO table record object
	CWQ_INFO* _pTheDBWQ;

	/// local memory copy of the work queue list
	WQEntryList	_navWQList;

	/// index of the last completed work queue entry
	int _navLastCmpltEntry;

	/// lock for the work queue list
	Mutex	_navWQLock;

	/// local memory copy of the QA work queue list
	WQEntryList	_qaWQList;

	/// index of the last completed QA work queue entry
	int _qaLastCmpltEntry;

	/// lock for the QA work queue list
	Mutex	_qaWQLock;

	/// latest DB work queue check time
	CTime	_lastDBCheckTime;

//////////////////////////////////////////////////////////////////////////
// database information
	
	/// DB timeout
	DWORD _timeout;

	/// Local AM ODBC name
	CString _dsn;

	/// Local AM user id
	CString _uid;

	/// Local AM password
	CString _pwd;

	/// Local AM database name
	CString _database;

	/// the formatted database connect string
	CString _connectStr;

	/// the formatted database connect string
	CString _connectDBStr;

	/// the max WQ records if it reaches, optimization with rebuild
	DWORD _maxWQOptRebuild;

//////////////////////////////////////////////////////////////////////////
// attributes
	/// status
	Status _status;

	/// whether should stop working
	bool _bShouldTerm;
};

#endif // !defined(AFX_NSBUILDER_H__12E49297_8BD2_4EBF_9216_D251349DE6A9__INCLUDED_)
