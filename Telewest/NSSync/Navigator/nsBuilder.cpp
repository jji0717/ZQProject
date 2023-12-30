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
// Name  : nsBuilder.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/nsBuilder.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 26    08-12-11 11:42 Li.huang
// Remove some restriction on MAX_WQ_HISTORY, since we dont have to follow
// this
// 
// 25    08-09-19 10:39 Ken.qian
// 
// 24    08-09-18 10:49 Li.huang
// 
// 23    08-09-04 2:02 Ken.qian
// new codes for new logic
// 
// 22    08-08-01 19:56 Ken.qian
// 
// 21    08-04-17 13:44 Ken.qian
// make MaxWQOptRebuild configurable
// 
// 20    07-05-17 12:17 Ken.qian
// Add Log in _pTheNavFactory and _pTheQAFactory threads re-creation
// 
// 19    07-05-16 23:16 Bernie.zhao
// Enhanced navFactory thread restart strategy.  Enhanced excpetion
// logging to include thread id.
// 
// 18    07-03-16 15:44 Ken.qian
// 
// 
// 15    07-02-12 21:02 Bernie.zhao
// fixed bug when in telewest environment getQAFlag() meets error
// 
// 14    07-02-12 15:09 Bernie.zhao
// fixed bug: when memory WQ count>=500, an error was raised and WQ resets
// 
// 13    06-12-22 15:59 Bernie.zhao
// fixed connection reset bug
// 
// 12    06-12-14 18:16 Ken.qian
// 
// 11    12/09/06 5:21a Bernie.zhao
// modified architechture to support QA Navigation.
// 
// 10    06-10-19 11:23 Ken.qian
// 
// 9     06-10-19 11:13 Ken.qian
// Change the Optimizer
// 
// 8     5/16/06 8:32a Bernie.zhao
// fixed entrycount calc bug, added auto-detect of 'ns_folderupdate'
// parameter count
// 
// 7     06-04-20 16:18 Bernie.zhao
// moved exception catch from nsBuild to every single 'exec'
// 
// 6     06-01-12 21:11 Bernie.zhao
// added logic to support PM
// 
// 5     05-12-20 11:42 Bernie.zhao
// 
// 4     05-10-13 6:26 Bernie.zhao
// quick sp to telewest, with same version number as 1.1.2
// 
// 2     05-07-22 11:57 Bernie.zhao
// ver 1.1.1,
// 1. modified rebuild strategy
// 2. update view/viewfolders when update space
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================


#include "stdafx.h"
#include "nsBuilder.h"
#include "NavigationService.h"

extern NavigationService gNavigator;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

nsBuilder::nsBuilder()
{
	_status = DISABLE;
	_pTheOptimizer = NULL;
	_pTheWorkerProvider = NULL;
	_pTheDBSource = NULL;
	_pTheDBWQ = NULL;
	_pTheNavFactory = NULL;
	_pTheQAFactory = NULL;

	_dsn = _T("SyncDSN");
	_uid = _T("multiverse");
	_pwd = _T("multiverse");
	_database = _T("multiverse");

	_bShouldTerm = FALSE;

	_navWQList.clear();
	_qaWQList.clear();
	_navLastCmpltEntry	= -1;
	_qaLastCmpltEntry	= -1;

	_lastDBCheckTime = NullCTime();
}

nsBuilder::~nsBuilder()
{
	if(_pTheOptimizer)
		delete _pTheOptimizer;
	
	if(_pTheWorkerProvider)
		delete _pTheWorkerProvider;
	
	if(_pTheDBWQ) 
	{
		if(_pTheDBWQ->IsOpen())
			_pTheDBWQ->Close();
		try{
			delete _pTheDBWQ;
		} catch(...) {}
	}
	
	if(_pTheDBSource)
	{
		if(_pTheDBSource->IsOpen())
			_pTheDBSource->Close();
		
		try{
			delete _pTheDBSource;
		} catch(...) {}
		_pTheDBSource = NULL;
	}

	if(_pTheNavFactory)
	{
		delete _pTheNavFactory;
		_pTheNavFactory = NULL;
	}

	if(_pTheQAFactory)
	{
		delete _pTheQAFactory;
		_pTheQAFactory = NULL;
	}
	
	_status = DISABLE;
}

bool nsBuilder::init()
{
	if(!_dsn.IsEmpty())	{ _connectStr = _T("ODBC;DSN=")+_dsn; }
	else				{ _connectStr = _T("ODBC;DSN=SyncDSN"); }

	if(!_uid.IsEmpty())	{ _connectStr += _T(";UID=")+_uid; }
	else				{ _connectStr += _T(";UID=multiverse"); }

	if(!_pwd.IsEmpty())	{ _connectStr += _T(";PWD=")+_pwd; }
	else				{ _connectStr += _T(";PWD=multiverse"); }

	if(!_database.IsEmpty())	{ _connectStr += _T(";DATABASE=")+_database; }
	else				{ _connectStr += _T(";DATABASE=multiverse"); }

	_connectDBStr = _connectStr.Right(_connectStr.GetLength()-wcslen(L"ODBC;"));

	if(!_pTheOptimizer)
		_pTheOptimizer = new nsOptimizer(this);
	if(!_pTheWorkerProvider)
		_pTheWorkerProvider = new navWorkerProvider(this);
	if(!_pTheNavFactory)
		_pTheNavFactory = new navFactory(*this, FactoryNAV_t);
	if(!_pTheQAFactory)
		_pTheQAFactory = new navFactory(*this, FactoryQA_t);
	
	ZQ::common::MutexGuard dbgd(_theDBSourceLock);

	try
	{
		if(!_pTheDBSource)
			_pTheDBSource = new CDatabase();
		_pTheDBSource->SetQueryTimeout(_timeout);
		_pTheDBSource->OpenEx(_connectDBStr, CDatabase::noOdbcDialog);

		if(!_pTheDBWQ) 
		{
			if(SP_FOLDER_UPDATE_TYPE_TARGET_SYNC == NavigationService::m_folderUpdateSPType)
				_pTheDBWQ = new CWQ_INFO(_pTheDBSource, _connectStr, CWQ_INFO::MODE_SQL_WITH_WAITTIME, NavigationService::m_dwUpdateWaitTime);
			else
				_pTheDBWQ = new CWQ_INFO(_pTheDBSource, _connectStr);
			
		}
		_pTheDBWQ->SetTimeout(_timeout);
	}
	catch (CDBException* pDBexcep) {
		glog(Log::L_ERROR, L"(tid=%d) nsBuilder::init()  A database exception occurs during database init, with error string: %s", ::GetCurrentThreadId(), pDBexcep->m_strError);
		
		pDBexcep->Delete();

		if(_pTheDBSource) {
			try{
				delete _pTheDBSource;
			} catch(...) {}
			_pTheDBSource = NULL;
		}
		if(_pTheDBWQ) {
			try{
				delete _pTheDBWQ;
			} catch(...) {}
			_pTheDBWQ = NULL;
		}

		return false;
	}

	// -- first, figure out whether the DB supports Program Manager
	// because if POMS supports Program Manager, the parameter count of the the sp "ns_FolderUpdate" is 2
	// while previous version has parameter count of 1
	try
	{
		if(NavigationService::m_folderUpdateParamCount == 0)
		{
			CRecordset testSet(_pTheDBSource);
			CString openString = L"select colid, name from syscolumns where id = OBJECT_ID('"  _T(DB_SP_NS_FOLDERUPDATE)  L"')";
			testSet.Open(CRecordset::dynaset, openString, CRecordset::readOnly);
			short nIndex=1;
			CString paramName = L"";
			while(!testSet.IsEOF())
			{
				testSet.GetFieldValue(nIndex, paramName);
				NavigationService::m_folderUpdateParamCount++;
				testSet.MoveNext();
			}
			testSet.Close();
			if(NavigationService::m_folderUpdateParamCount<=0 || NavigationService::m_folderUpdateParamCount>3)
			{
				// invalid parameter count for stored procedure 'ns_FolderUpdate'
				glog(Log::L_WARNING, L"Invalid parameter count [%d] for stored procedure 'ns_FolderUpdate', set to default value [1]", NavigationService::m_folderUpdateParamCount);
				NavigationService::m_folderUpdateParamCount = 1;
			}
			else
			{
				glog(Log::L_NOTICE, L"Parameter count for stored procedure 'ns_FolderUpdate' set to value [%d]", NavigationService::m_folderUpdateParamCount);
			}
		}
	}
	catch (CDBException* pDBexcep) 
	{
		glog(Log::L_ERROR, L"(tid=%d) nsBuilder::init()  A database exception occurs during database init, with error string: %s", ::GetCurrentThreadId(), pDBexcep->m_strError);
		glog(Log::L_WARNING, L"Can not get parameter count for stored procedure 'ns_FolderUpdate', set to default value [1]");
		pDBexcep->Delete();

		if(_pTheDBSource) {
			try{
				delete _pTheDBSource;
			} catch(...) {}
			_pTheDBSource = NULL;
		}

		return false;
	}

	// -- second, figure out if QA Navigation function is enabled in this DB
	// we do this by checking DB for 2 tables and 2 SPs, which are essential in QA Navigation function
	try
	{
		CRecordset testSet(_pTheDBSource);
		CString openString = L"select COUNT(1) from sysobjects where " 
			L"id=OBJECT_ID('"  _T(DB_TABLE_NAV_HIERARCHY_QA)  L"')"
			L" OR id=OBJECT_ID('"  _T(DB_TABLE_NAV_FOLDER_CONDITION_QA)  L"')"
			L" OR id=OBJECT_ID('"  _T(DB_SP_NAV_GETQAENABLEFLAG)  L"')"
			L" OR id=OBJECT_ID('"  _T(DB_SP_NAV_GETQAGENERATINGSTATUS)  L"')";

		testSet.Open(CRecordset::dynaset, openString, CRecordset::readOnly);
		short nIndex=0;
		CDBVariant totalCount;
		if(!testSet.IsEOF())
		{
			testSet.GetFieldValue(nIndex, totalCount);
			if(totalCount.m_lVal==4)
			{
				NavigationService::m_bQANavigationEnabled = true;
				glog(Log::L_NOTICE, L"QA Navigation function is enabled, essential TABLEs and SPs found in DB");
			}
			else
			{
				NavigationService::m_bQANavigationEnabled = false;
				glog(Log::L_NOTICE, L"QA Navigation is disabled, because at least one of TABLEs and SPs [%s],[%s],'%s()','%s()' does not exist in DB.", 
					_T(DB_TABLE_NAV_HIERARCHY_QA), _T(DB_TABLE_NAV_FOLDER_CONDITION_QA), _T(DB_SP_NAV_GETQAENABLEFLAG), _T(DB_SP_NAV_GETQAGENERATINGSTATUS) );
			}
		}
		else
		{
			NavigationService::m_bQANavigationEnabled = false;
			glog(Log::L_NOTICE, L"QA Navigation is disabled, because at least one of TABLEs and SPs [%s],[%s],'%s()','%s()' does not exist in DB.", 
				_T(DB_TABLE_NAV_HIERARCHY_QA), _T(DB_TABLE_NAV_FOLDER_CONDITION_QA), _T(DB_SP_NAV_GETQAENABLEFLAG), _T(DB_SP_NAV_GETQAGENERATINGSTATUS) );
		}
		testSet.Close();
	
	}
	catch (CDBException* pDBexcep) 
	{
		glog(Log::L_ERROR, L"(tid=%d) nsBuilder::init()  A database exception occurs during database init, with error string: %s", ::GetCurrentThreadId(), pDBexcep->m_strError);
		glog(Log::L_NOTICE, L"QA Navigation is disabled, because of the exception.");
		NavigationService::m_bQANavigationEnabled = false;
		pDBexcep->Delete();

		if(_pTheDBSource) {
			try{
				delete _pTheDBSource;
			} catch(...) {}
			_pTheDBSource = NULL;
		}

		return false;
	}
	
	// check whether there is the sp
	try
	{
		// Flag add by KenQ to determine which mode NSSync running on
		NavigationService::m_folderUpdateSPType = SP_FOLDER_UPDATE_TYPE_SAMPLE_SYNC;

		CRecordset testSet(_pTheDBSource);
		CString openString = L"SELECT COUNT(1) FROM sysobjects WHERE name = 'ns_FolderUpdateEx' AND type = 'P'";

		testSet.Open(CRecordset::dynaset, openString, CRecordset::readOnly);
		short nIndex=0;
		CDBVariant totalCount;
		if(!testSet.IsEOF())
		{
			testSet.GetFieldValue(nIndex, totalCount);
			if(totalCount.m_lVal==1)
			{
				glog(Log::L_INFO, L"(tid=%d) nsBuilder::init() NSSync use store procedure %s to do folder update", 
					::GetCurrentThreadId(), L"ns_FolderUpdateEx");

				NavigationService::m_folderUpdateSPType = SP_FOLDER_UPDATE_TYPE_TARGET_SYNC;
			}
		}
		testSet.Close();

	}
	catch (CDBException* pDBexcep) 
	{
		glog(Log::L_ERROR, L"(tid=%d) nsBuilder::init()  A database exception occurs during database init, with error string: %s", ::GetCurrentThreadId(), pDBexcep->m_strError);
		pDBexcep->Delete();

		if(_pTheDBSource) {
			try{
				delete _pTheDBSource;
			} catch(...) {}
			_pTheDBSource = NULL;
		}

		return false;
	}
	

	// start 2 factories, for workers to work inside
	_pTheNavFactory->start();
	_pTheQAFactory->start();

	_status = IDLE;
	_bShouldTerm = FALSE;
	glog(Log::L_INFO, L"nsBuilder initialized with connect string \"%s\"", _connectStr);

	return true;
}

void nsBuilder::uninit()
{
	// stop factories
	_pTheNavFactory->signalTerm();
	_pTheNavFactory->waitHandle(1000);
	_pTheQAFactory->signalTerm();
	_pTheQAFactory->waitHandle(1000);

	if(_pTheOptimizer) {
		delete _pTheOptimizer;
		_pTheOptimizer = NULL;
	}
	if(_pTheWorkerProvider) {
		delete _pTheWorkerProvider;
		_pTheWorkerProvider = NULL;
	}
	if(_pTheDBWQ) {
		if(_pTheDBWQ->IsOpen())
			_pTheDBWQ->Close();
		try{
			delete _pTheDBWQ;
		} catch(...) {}
		_pTheDBWQ = NULL;
	}

	ZQ::common::MutexGuard dbgd(_theDBSourceLock);
	if(_pTheDBSource) {
		if(_pTheDBSource->IsOpen())
			_pTheDBSource->Close();
		try{
			delete _pTheDBSource;
		} catch(...) {}
		_pTheDBSource = NULL;
	}
	if(_pTheNavFactory)	{
		delete _pTheNavFactory;
		_pTheNavFactory = NULL;
	}

	if(_pTheQAFactory)	{
		delete _pTheQAFactory;
		_pTheQAFactory = NULL;
	}
	glog(Log::L_INFO, "nsBuilder uninitialized");
	_status = DISABLE;
	_bShouldTerm = TRUE;
}

int nsBuilder::build()
{
	// lock both memory lists
	ZQ::common::MutexGuard	gdNav(_navWQLock);
	ZQ::common::MutexGuard	gdQA(_qaWQLock);
	ZQ::common::MutexGuard	dbgd(_theDBSourceLock);

	int entryNum;
	_status = BUILDING;

	// update the tea time each time
	gNavigator.updateTeaTime();

	bool QAfunction = true;
	if(!NavigationService::m_bQANavigationEnabled || !getQAFlag(_T(DB_SP_NAV_GETQAENABLEFLAG)))
	{
		QAfunction = false;
	}

	// fetch entries into local memory lists
	entryNum = fetchDBWQ(QAfunction);

	// error happened
	if(entryNum == -1)
	{
		_status = IDLE;

		// delete wq recordset
		if(_pTheDBWQ)
		{
			if(_pTheDBWQ->IsOpen())
				_pTheDBWQ->Close();
			try{
				delete _pTheDBWQ;
			} catch(...) {}
		}
		_pTheDBWQ = NULL;

		if(_pTheDBSource)
		{
			if(_pTheDBSource->IsOpen())
				_pTheDBSource->Close();
			try{
				delete _pTheDBSource;
			} catch(...) {}
		}
		_pTheDBSource = NULL;

		return NS_ERROR;
	}

	if(entryNum ==0) 
	{	// no entry fetched from db
		_status = IDLE;
		return NS_NOENTRY;
	}

	int nRet = 0; 
	
	nRet = _pTheOptimizer->optimize();
	if(nRet!=NS_SUCCESS) 
	{
		_status = IDLE;
		return nRet;
	}

	//
	// check factory threads status first, maybe they have been screwed up.
	// restart them if so.
	//           - 05/16/07  Bernie Zhao
	{
		if(_pTheNavFactory && !_pTheNavFactory->isRunning())
		{
			try {
				delete _pTheNavFactory;
			} catch(...) {}
			_pTheNavFactory = NULL;
		}
		if(_pTheQAFactory && !_pTheQAFactory->isRunning())
		{
			try {
				delete _pTheQAFactory;
			} catch(...) {}
			_pTheQAFactory = NULL;
		}
		
		if(!_pTheNavFactory)
		{
			_pTheNavFactory = new navFactory(*this, FactoryNAV_t);
			_pTheNavFactory->start();

			glog(Log::L_NOTICE, L"Commercial Navigation building thread recovered. New ThreadId is %d", _pTheNavFactory->id());
		}
		if(!_pTheQAFactory)
		{
			_pTheQAFactory = new navFactory(*this, FactoryQA_t);
			_pTheQAFactory->start();

			glog(Log::L_NOTICE, L"QA Navigation building thread recovered. New ThreadId is %d", _pTheQAFactory->id());
		}
	}
	//
	
	// wake up factory to work
	_pTheNavFactory->signalWakeup();
	if(QAfunction)
		_pTheQAFactory->signalWakeup();

	_status = IDLE;
	
	return NS_SUCCESS;
}

int nsBuilder::fetchDBWQ(bool QAfunction)
{
	ZQ::common::MutexGuard dbgd(_theDBSourceLock);

	int wqEntryCount = -1;
	try
	{
		// check if database need construct
		if(_pTheDBSource==NULL)
		{
			_pTheDBSource = new CDatabase();
			_pTheDBSource->SetQueryTimeout(_timeout);
			_pTheDBSource->OpenEx(_connectDBStr, CDatabase::noOdbcDialog);
		}

		// first, let's see how many work queue entrys are in DB now
		// if it is larger than _maxWQOptRebuild (by default 100), we do a 'rebuild' instead of dealing with them separately
		// this optimization is added by Ken on 2006-10-18
			
		CRecordset testSet(_pTheDBSource);
		CString openString = L"SELECT COUNT(1) FROM WQ_INFO";
		testSet.Open(CRecordset::dynaset, openString, CRecordset::readOnly);
		short nIndex=0;
		CDBVariant totalCount;
		if(!testSet.IsEOF())
		{
			testSet.GetFieldValue(nIndex, totalCount);
			wqEntryCount = totalCount.m_lVal;	// the return value should be a INTEGER, so use m_lVal
		}
		testSet.Close();
	}
	catch (CDBException* pDBexcep) 
	{
		glog(Log::L_ERROR, L"(tid=%d) nsBuilder::fetchDBWQ()  A database exception occurs during counting of WQ entries, with error string: %s", ::GetCurrentThreadId(), pDBexcep->m_strError);
		pDBexcep->Delete();

		if(_pTheDBSource) {
			try{
				delete _pTheDBSource;
			} catch(...) {}
			_pTheDBSource = NULL;
		}
	}
	catch(...)
	{
		glog(Log::L_ERROR, L"(tid=%d) nsBuilder::fetchDBWQ()  An unknown exception occurs during counting of WQ entries", ::GetCurrentThreadId());
	}

	// now check the count
	if(wqEntryCount>=_maxWQOptRebuild)
	{
		glog(Log::L_INFO, L"Work queue optimized to a rebuild due to too many work queue entries, current entries=%d, max entries=%d", wqEntryCount, _maxWQOptRebuild);

		// too many WQ entries
		// 1.mark all waiting entry status to skipped
		// 2.add a special rebuild entry into NAV and QA memory queue
		// 3.update index cursors
		// 4.delete all items in the DB
		size_t i = 0;
		for(i=_navLastCmpltEntry+1; i<_navWQList.size(); i++)	// don't count last completed index itself, start from next one
		{
			if(_navWQList[i].Status == wq_waiting)
				_navWQList[i].Status = wq_skipped;
		}
		for(i=_qaLastCmpltEntry+1; i<_qaWQList.size(); i++)
		{
			if(_qaWQList[i].Status == wq_waiting)
				_qaWQList[i].Status = wq_skipped;
		}

		WQ_Entry tmpEntry;
		tmpEntry.Queue_UID = _T(WQ_OPT_REBUILD_QUEUEID);
		tmpEntry.Source_type = 1;
		tmpEntry.Operation_type = 99;
		tmpEntry.Entry_Name = L"";
		tmpEntry.Operation_time = CTime::GetCurrentTime();
		tmpEntry.Status = wq_waiting;
	
		addNAVMemWQ(tmpEntry);
		if(QAfunction)
			addQAMemWQ(tmpEntry);

		deleteDBWQ(L"*");	// * stands for deleting all entries in the DB

		_lastDBCheckTime = CTime::GetCurrentTime();
		return 1;
	}
	else if(wqEntryCount==0 || wqEntryCount==-1)	// nothing is there or something bad happened, don't build
	{
		_lastDBCheckTime = CTime::GetCurrentTime();
		return 0;
	}

	// if we reach here, means that we do have a bunch of entries to deal with
	// 1.fetch them one by one
	// 2.compare them with memory lists, see if we get something new
	// 3.add new entry into both memory lists (operation 98 only add to qa list)
	// 4.update index cursors
	try
	{
		if(_pTheDBWQ==NULL) 
		{
			if(SP_FOLDER_UPDATE_TYPE_TARGET_SYNC == NavigationService::m_folderUpdateSPType)
				_pTheDBWQ = new CWQ_INFO(_pTheDBSource, _connectStr, CWQ_INFO::MODE_SQL_WITH_WAITTIME, NavigationService::m_dwUpdateWaitTime);
			else
				_pTheDBWQ = new CWQ_INFO(_pTheDBSource, _connectStr);

			_pTheDBWQ->SetTimeout(_timeout);
		}
		if(!_pTheDBWQ->IsOpen()) 
		{
			if(_pTheDBWQ->Open(CRecordset::snapshot , NULL, CRecordset::none)==0)
				return -1;	// open error
		}
		else
		{
			_pTheDBWQ->Close();
			if(_pTheDBWQ->Open(CRecordset::snapshot , NULL, CRecordset::none)==0)
				return -1;	// open error
		}
		// any data in work queue?
		if(_pTheDBWQ->IsBOF()) {
			_pTheDBWQ->Close();
			return 0;
		}

		// start from the top DB entry
		_pTheDBWQ->MoveFirst();
		while(!_pTheDBWQ->IsEOF()) 
		{
			WQ_Entry tmpEntry;

			tmpEntry.Queue_UID			= _pTheDBWQ->m_QueueUID;
			tmpEntry.Source_type		= _pTheDBWQ->m_Source_type;
			tmpEntry.local_entry_UID	= _pTheDBWQ->m_local_entry_UID;
			tmpEntry.Parent_HUID		= _pTheDBWQ->m_Parent_HUID;
			tmpEntry.Entry_Name			= _pTheDBWQ->m_Entry_Name;
			tmpEntry.Entry_type			= _pTheDBWQ->m_Entry_type;
			tmpEntry.Operation_type		= _pTheDBWQ->m_Operation_type;
			tmpEntry.Operation_time		= _pTheDBWQ->m_Operation_time;
			tmpEntry.MD_name			= _pTheDBWQ->m_MD_name;
			tmpEntry.Status				= wq_waiting;
			tmpEntry.Start_time			= NullCTime();
			tmpEntry.End_time			= NullCTime();

			size_t i = 0;
			bool isNewEntry = true;
			for(i=_navLastCmpltEntry+1; i<_navWQList.size(); i++)
			{
				if(_navWQList[i].Queue_UID == tmpEntry.Queue_UID)
				{
					isNewEntry = false;
					break;
				}
			}
			if(isNewEntry && tmpEntry.Operation_type!=98)
			{
				addNAVMemWQ(tmpEntry);
			}

			isNewEntry = true;
			for(i=_qaLastCmpltEntry+1; i<_qaWQList.size(); i++)
			{
				if(_qaWQList[i].Queue_UID == _pTheDBWQ->m_QueueUID)
				{
					isNewEntry = false;
					break;
				}
			}
			if(isNewEntry)
			{
				if(QAfunction)	// only add into QA WQ when QA is running.
					addQAMemWQ(tmpEntry);
				else if(NavigationService::m_bQANavigationEnabled == false && tmpEntry.Operation_type==98)
					deleteDBWQ(tmpEntry.Queue_UID);
			}
			
			// fetch next DB entry
			_pTheDBWQ->MoveNext();
		}

		_pTheDBWQ->Close();
	}
	catch (CDBException* pDBexcep) 
	{
		glog(Log::L_ERROR, L"(tid=%d) nsBuilder::fetchDBWQ()  A database exception occurs during WQ entries fetching, with error string: %s", ::GetCurrentThreadId(), pDBexcep->m_strError);
		pDBexcep->Delete();

		if(_pTheDBWQ)
		{
			if(_pTheDBWQ->IsOpen())
				_pTheDBWQ->Close();
			try{
				delete _pTheDBWQ;
			} catch(...) {}
		}
		_pTheDBWQ = NULL;

		if(_pTheDBSource)
		{
			if(_pTheDBSource->IsOpen())
				_pTheDBSource->Close();
			try{
				delete _pTheDBSource;
			} catch(...) {}
		}
		_pTheDBSource = NULL;

		return 0;
	}
	catch (...) 
	{
		glog(Log::L_ERROR, L"(tid=%d) nsBuilder::fetchDBWQ()  An unknown exception occurs during WQ entries fetching",::GetCurrentThreadId());
		return -1;
	}

	_lastDBCheckTime = CTime::GetCurrentTime();
	return 1;
}

bool nsBuilder::deleteDBWQ(CString queueID)
{
	CString execStr = L"";

	if(queueID==L"*")	// delete all
	{
		execStr=L"DELETE FROM WQ_INFO";
	}
	else	// delete 1 entry
	{
		execStr=L"DELETE FROM WQ_INFO WHERE QueueUID='"+queueID+L"'";
	}

	ZQ::common::MutexGuard dbgd(_theDBSourceLock);

	try
	{
		// check if database need construct
		if(_pTheDBSource==NULL)
		{
			_pTheDBSource = new CDatabase();
			_pTheDBSource->SetQueryTimeout(_timeout);
			_pTheDBSource->OpenEx(_connectDBStr, CDatabase::noOdbcDialog);
		}
	
		if(NavigationService::m_dwSQLTraceEnabled)
			glog(Log::L_DEBUG, L"WQ_INFO << SQL << \"%s\"\t%s",(LPCTSTR)execStr, __WFUNC__);
		_pTheDBSource->ExecuteSQL( execStr );
	}
	catch (CDBException* pDBexcep)
	{
		glog(Log::L_ERROR, L"(tid=%d) nsBuilder::deleteDBWQ()  A database exception occurs when trying to delete entry from WQ_INFO, with error string: %s", ::GetCurrentThreadId(), pDBexcep->m_strError);
		pDBexcep->Delete();

		if(_pTheDBSource) {
			try{
				delete _pTheDBSource;
			} catch(...) {}
			_pTheDBSource = NULL;
		}

		return false;
	}
	catch (...)
	{
		glog(Log::L_ERROR, L"(tid=%d) nsBuilder::deleteDBWQ()  An unknown exception occurs when trying to delete entry from WQ_INFO", ::GetCurrentThreadId());
		return false;
	}

	glog(Log::L_DEBUG, L"WQ entry '%s' deleted", queueID);
	return true;
}

bool nsBuilder::addNAVMemWQ(WQ_Entry& entry)
{
	// note!  this does not lock list.  Be sure to lock it before call this
	if( /* _navWQList.size() > MAX_WQ_HISTORY || */ _navLastCmpltEntry>=(int)_navWQList.size())	// we got trouble! something must be wrong!
	{
		glog(Log::L_ERROR, L"nsBuilder::addNAVMemWQ() NAV memory work queue is corrupted: size=%d, LastCompletedEntry=%d", _navWQList.size(), _navLastCmpltEntry);
		glog(Log::L_NOTICE, L"NAV memory work queue reset because of unexpected corruption");
		_navWQList.clear();
		_navLastCmpltEntry = -1;
	}
/*
	// because we restrict this vector to be <=MAX_WQ_HISTORY (500), so if it's size is already 500, we delete oldest one
	for(size_t count=_navWQList.size(); count>=MAX_WQ_HISTORY; count=_navWQList.size())
	{
		_navWQList.pop_front();
		if(_navLastCmpltEntry>0)
			_navLastCmpltEntry--;
	}
*/
	_navWQList.push_back(entry);
	glog(Log::L_INFO, L"Found entry '%s' in table WQ_INFO", (LPCTSTR)entry.Queue_UID);

	return true;
}

bool nsBuilder::addQAMemWQ(WQ_Entry& entry)
{
	// note!  this does not lock list.  Be sure to lock it before call this
	if( /* _qaWQList.size() > MAX_WQ_HISTORY || */ _qaLastCmpltEntry>=(int)_qaWQList.size())	// we got trouble! something must be wrong!
	{
		glog(Log::L_ERROR, L"nsBuilder::addQAMemWQ() QA memory work queue is corrupted: size=%d, LastCompletedEntry=%d", _qaWQList.size(), _qaLastCmpltEntry);
		glog(Log::L_NOTICE, L"QA memory work queue reset because of unexpected corruption");
		_qaWQList.clear();
		_qaLastCmpltEntry = -1;
	}
/*
	// because we restrict this vector to be <=MAX_WQ_HISTORY (500), so if it's size is already 500, we delete oldest one
	for(size_t count=_qaWQList.size(); count>=MAX_WQ_HISTORY; count=_qaWQList.size())
	{
		_qaWQList.pop_front();
		if(_qaLastCmpltEntry>0)
			_qaLastCmpltEntry--;
	}
*/
	_qaWQList.push_back(entry);
	if(entry.Operation_type==98)
		glog(Log::L_INFO, L"Found entry '%s' in table WQ_INFO", (LPCTSTR)entry.Queue_UID);
	return true;
}

int nsBuilder::getQAFlag(CString spName)
{
	ZQ::common::MutexGuard dbgd(_theDBSourceLock);

	int	retVal = INT_MAX;

	// check if database need construct
	try
	{
		if(_pTheDBSource==NULL)
		{
			_pTheDBSource = new CDatabase();
			_pTheDBSource->SetQueryTimeout(_timeout);
			_pTheDBSource->OpenEx(_connectDBStr, CDatabase::noOdbcDialog);
		}

		CRecordset testSet(_pTheDBSource);
		CString openString = L"SELECT dbo." + spName + L"()";
		testSet.Open(CRecordset::dynaset, openString, CRecordset::readOnly);
		short nIndex=0;
		CDBVariant result;
		if(!testSet.IsEOF())
		{
			testSet.GetFieldValue(nIndex, result);
			retVal = result.m_lVal;	// the return value should be a INTEGER, so use m_lVal
		}
		testSet.Close();
	}
	catch (CDBException* pDBexcep) 
	{
		glog(Log::L_ERROR, L"(tid=%d) nsBuilder::getQAFlag()  A database exception occurs during calling function [%s], with error string: %s", ::GetCurrentThreadId(), spName, pDBexcep->m_strError);
		pDBexcep->Delete();

		if(_pTheDBSource) {
			try{
				delete _pTheDBSource;
			} catch(...) {}
			_pTheDBSource = NULL;
		}
	}
	catch (...)
	{
		glog(Log::L_ERROR, L"(tid=%d) nsBuilder::getQAFlag()  An unknown exception occurs during calling function [%s]", ::GetCurrentThreadId(), spName);
		return false;
	}

	return retVal;
}

CString nsBuilder::qaFlagToString(CString spName, int flagValue)
{
	CString retVal = L"";
	if(flagValue==INT_MAX)
	{
		retVal= L"Unknown";
	}
	else if(spName==DB_SP_NAV_GETQAENABLEFLAG || spName==DB_SP_NAV_GETOFFERINGWINDOWENABLEFLAG)
	{
		if(flagValue==1)
			retVal = L"On";
		else if(flagValue==0)
			retVal = L"Off";
	}
	else if(spName==DB_SP_NAV_GETQAGENERATINGSTATUS)
	{
		if(flagValue==2)
			retVal = L"Error";
		else if(flagValue==1)
			retVal = L"Generating";
		else if(flagValue==0)
			retVal = L"Generated";
		else if(flagValue==-1)
			retVal = L"Not generated";
	}
	else if(spName==DB_SP_NAV_GETOFFERINGWINDOWENDOFFSETDAYS)
	{
		retVal.Format(L"%d Day(s)", flagValue);
	}
	return retVal;
}

MANSTATUS FAR WINAPI nsBuilder::wqInfoCallBack(WCHAR * pwszCmd, WCHAR ** ppwszResponse, DWORD * pdwLength )
{
	WORD wCommand;
	swscanf(pwszCmd, L"%c\t", &wCommand);

	nsBuilder&	bld = *(gNavigator._theBuilder);
	
	switch (wCommand)
	{
	case MAN_READ_VARS :
		{    
			ZQ::common::MutexGuard gd(bld._navWQLock);

			int rowCount = bld._navWQList.size();

			DWORD bufsize = 500 + (1000 * rowCount);        

			WCHAR* buf = new WCHAR[bufsize];
			WCHAR* tmp = buf;

			*buf = 0;                    // start with a null string 

			DWORD SimpleVars = 1;        // Number of simple variables
			DWORD ColCount   = 12;        // Number of complex table COLUMNS in complex var
			// Output buffer header
			tmp += wsprintf(tmp, L"%d\t%d\n", SimpleVars, ColCount);

			// Simple vars
			CString lastTimeStr = (bld._lastDBCheckTime.GetYear()<=1970)? L"N/A" : bld._lastDBCheckTime.Format("%Y-%m-%d %H:%M:%S");
			tmp += wsprintf(tmp, L"%d\t%s\t%s\n", MAN_STR, L"Last Database Work Queue Check", (LPCTSTR)lastTimeStr);

			// Complex Variable:
			// Each Column
			// <Type><tab><Width><tab><RowCount><tab><Name><newline>
			int i;
			WQEntryList::const_iterator it;

			// Column 1: Queue UID
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Queue ID");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", (LPCTSTR)bld._navWQList[i].Queue_UID);
			}

			// Column 2: Status
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Status");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", WQStatusToString(bld._navWQList[i]));
			}

			// Column 3: Source type
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Source Type");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", WQSourceTypeToString(bld._navWQList[i]));
			}

			// Column 4: Entry type
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Entry Type");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", WQEntryTypeToString(bld._navWQList[i]));
			}

			// Column 5: Entry name
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Entry Name");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", NASTRING(bld._navWQList[i].Entry_Name));
			}

			// Column 6: Operation type
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Operation Type");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", WQOperationTypeToString(bld._navWQList[i]));
			}

			// Column 7: Local entry UID
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"LocalEntryUID");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", NASTRING(bld._navWQList[i].local_entry_UID));
			}

			// Column 8: Parent HUID
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"ParentHUID");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", NASTRING(bld._navWQList[i].Parent_HUID));
			}

			// Column 9: Metadata Name
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"MDName");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", NASTRING(bld._navWQList[i].MD_name));
			}

			// Column 10: Operation time (create time)
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Create Time");

			for (i=rowCount-1; i>=0; i--) 
			{
				CTIMETOSTRING(tmp, bld._navWQList[i].Operation_time);
			}

			// Column 11: Start Time
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Start Time");

			for (i=rowCount-1; i>=0; i--) 
			{
				CTIMETOSTRING(tmp, bld._navWQList[i].Start_time);
			}

			// Column 12: End Time
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"End Time");

			for (i=rowCount-1; i>=0; i--) 
			{
				CTIMETOSTRING(tmp, bld._navWQList[i].End_time);
			}

			*pdwLength = wcslen(buf);    // not including the null terminator in the size
			*ppwszResponse = buf;        // return the buffer
		}
		break;

	case MAN_FREE:
		delete[] *ppwszResponse;
		break;

	default:
		return MAN_BAD_PARAM;
	}

	return MAN_SUCCESS;
}

MANSTATUS FAR WINAPI nsBuilder::qaWqInfoCallBack(WCHAR * pwszCmd, WCHAR ** ppwszResponse, DWORD * pdwLength )
{
	WORD wCommand;
	swscanf(pwszCmd, L"%c\t", &wCommand);

	nsBuilder&	bld = *(gNavigator._theBuilder);

	switch (wCommand)
	{
	case MAN_READ_VARS :
		{    
			ZQ::common::MutexGuard gd(bld._qaWQLock);

			int rowCount = bld._qaWQList.size();
			if(!NavigationService::m_bQANavigationEnabled)
				rowCount = 0;	// do not show any entry for disabled QA

			DWORD bufsize = 500 + (1000 * rowCount);        

			WCHAR* buf = new WCHAR[bufsize];
			WCHAR* tmp = buf;

			*buf = 0;                    // start with a null string

            DWORD SimpleVars = 6;        // Number of simple variables
			if(!NavigationService::m_bQANavigationEnabled)
				SimpleVars = 1;

			DWORD ColCount   = 12;        // Number of complex table COLUMNS in complex var
			// Output buffer header
			tmp += wsprintf(tmp, L"%d\t%d\n", SimpleVars, ColCount);

			// Simple vars
			// Var 1: QA support
			tmp += wsprintf(tmp, L"%d\t%s\t%s\n", MAN_STR, L"QA Navigation Support", ((NavigationService::m_bQANavigationEnabled)?L"Enabled":L"Disabled") );

			if(NavigationService::m_bQANavigationEnabled)
			{
				// Var 2: QA Enabled
				int flagValue = bld.getQAFlag(_T(DB_SP_NAV_GETQAENABLEFLAG));
				tmp += wsprintf(tmp, L"%d\t%s\t%s\n", MAN_STR, L"Current QA Status", (LPCTSTR)bld.qaFlagToString(_T(DB_SP_NAV_GETQAENABLEFLAG), flagValue));

				// Var 3: QA generating status
				flagValue = bld.getQAFlag(_T(DB_SP_NAV_GETQAGENERATINGSTATUS));
				tmp += wsprintf(tmp, L"%d\t%s\t%s\n", MAN_STR, L"QA Navigation Generating Status", (LPCTSTR)bld.qaFlagToString(_T(DB_SP_NAV_GETQAGENERATINGSTATUS), flagValue));

				// Var 4: Offering window enabled
				flagValue = bld.getQAFlag(_T(DB_SP_NAV_GETOFFERINGWINDOWENABLEFLAG));
				tmp += wsprintf(tmp, L"%d\t%s\t%s\n", MAN_STR, L"Offering Window Status", (LPCTSTR)bld.qaFlagToString(_T(DB_SP_NAV_GETOFFERINGWINDOWENABLEFLAG), flagValue));

				// Var 5: Offering window offset days
				flagValue = bld.getQAFlag(_T(DB_SP_NAV_GETOFFERINGWINDOWENDOFFSETDAYS));
				tmp += wsprintf(tmp, L"%d\t%s\t%s\n", MAN_STR, L"Offering Window End Offset", (LPCTSTR)bld.qaFlagToString(_T(DB_SP_NAV_GETOFFERINGWINDOWENDOFFSETDAYS), flagValue));
			
				// Var 6: last DB check time
				CString lastTimeStr = (bld._lastDBCheckTime.GetYear()<=1970)? L"N/A" : bld._lastDBCheckTime.Format("%Y-%m-%d %H:%M:%S");
				tmp += wsprintf(tmp, L"%d\t%s\t%s\n", MAN_STR, L"Last Database Work Queue Check", (LPCTSTR)lastTimeStr);
			}
			

			// Complex Variable:
			// Each Column
			// <Type><tab><Width><tab><RowCount><tab><Name><newline>
			int i;
			WQEntryList::const_iterator it;

			// Column 1: Queue UID
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Queue ID");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", (LPCTSTR)bld._qaWQList[i].Queue_UID);
			}

			// Column 2: Status
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Status");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", WQStatusToString(bld._qaWQList[i]));
			}

			// Column 3: Source type
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Source Type");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", WQSourceTypeToString(bld._qaWQList[i]));
			}

			// Column 4: Entry type
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Entry Type");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", WQEntryTypeToString(bld._qaWQList[i]));
			}

			// Column 5: Entry name
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Entry Name");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", NASTRING(bld._qaWQList[i].Entry_Name));
			}

			// Column 6: Operation type
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Operation Type");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", WQOperationTypeToString(bld._qaWQList[i]));
			}

			// Column 7: Local entry UID
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"LocalEntryUID");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", NASTRING(bld._qaWQList[i].local_entry_UID));
			}

			// Column 8: Parent HUID
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"ParentHUID");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", NASTRING(bld._qaWQList[i].Parent_HUID));
			}

			// Column 9: Metadata Name
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"MDName");

			for (i=rowCount-1; i>=0; i--) 
			{
				tmp += wsprintf(tmp, L"%s\n", NASTRING(bld._qaWQList[i].MD_name));
			}

			// Column 10: Operation time (create time)
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Create Time");

			for (i=rowCount-1; i>=0; i--) 
			{
				CTIMETOSTRING(tmp, bld._qaWQList[i].Operation_time);
			}

			// Column 11: Start Time
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"Start Time");

			for (i=rowCount-1; i>=0; i--) 
			{
				CTIMETOSTRING(tmp, bld._qaWQList[i].Start_time);
			}

			// Column 12: End Time
			tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_COMPLEX, rowCount, L"End Time");

			for (i=rowCount-1; i>=0; i--) 
			{
				CTIMETOSTRING(tmp, bld._qaWQList[i].End_time);
			}

			*pdwLength = wcslen(buf);    // not including the null terminator in the size
			*ppwszResponse = buf;        // return the buffer
		}
		break;

	case MAN_FREE:
		delete[] *ppwszResponse;
		break;

	default:
		return MAN_BAD_PARAM;
	}

	return MAN_SUCCESS;
}