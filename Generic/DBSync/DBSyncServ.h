/*****************************************************************************
File Name:		DBSyncServ.h
Author:			Interactive ZQ 
Security:		<NVOD over iTV, Kunming>, Confidential
Description:	Sevice application entrience. 

Modification Log:
When		Version		Who			What
-----------------------------------------------------------------------------

 05/21/2006      4       KenQ   Support to take IDS callback during the full synchronization, 
								these callbacks are stored into a queue for later processing 
								by anther thread.
								So, the original thread(TriggerWorker/TriggerWorkerMgr) are
								not necessary any more, whose purpose is avoiding 
								invoking IDS API in the callback. Process callback in a seperated 
								thread cover the issue. 
*****************************************************************************/

#if !defined(AFX_DBSYNCSERV_H__95E43CAF_5C0C_407F_9C54_A3DBE6E2220C__INCLUDED_)
#define AFX_DBSYNCSERV_H__95E43CAF_5C0C_407F_9C54_A3DBE6E2220C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "connchecker.h"
#include "baseSchangeServiceApplication.h"
#include "LocalDB.h"
#include "ids_def.h"
#include "CallBackProcessThread.h"
#include "DSCallBack.h"
#include "ManualSyncAdi.h"
#include "MiniDump.h"

#define DEFAULT_DB_TYPE				L"Oracle"
#define DEFAULT_IZQ_PLAYINTERVAL	3000000// microsecond
#define DEFAULT_ITV_CHECKINTERVAL	60	   //minutes

#define MAXNAMELEN						256

#define DBSYNC_SERVICETYPE			0

class ManualSyncAdi;

class DBSyncServ:public ZQ::common::BaseSchangeServiceApplication
{
	friend void LocalDB::RetryConnection(void);
public:
	DBSyncServ();
	virtual ~DBSyncServ();

public:
	
	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);
	bool isHealth(void);
	void exitProcess(void);
	
protected :
	static wchar_t m_ITVServerIP[MAXNAMELEN];
	static wchar_t m_ITVUserName[MAXNAMELEN];
	static wchar_t m_IZQDB_DSN[MAXNAMELEN];
	static wchar_t m_IZQDB_UserName[MAXNAMELEN];
	static wchar_t m_IZQDB_Password[MAXNAMELEN];
	static DWORD m_IZQ_PlayInterval;
	static wchar_t m_IZQDB_Type[MAXNAMELEN];
	static wchar_t m_MDListString[2048];
	static DWORD m_ITVCheckInterval;

	static wchar_t	m_SyncDirectory[4096];
	static DWORD	m_TimeWindowThreshold;
	static DWORD	m_SupportNavigation;
	static wchar_t	m_PluginPath[MAXNAMELEN];
	static wchar_t	m_DBSyncIP[MAXNAMELEN];
	static wchar_t  m_ManualSyncAddInPath[MAXNAMELEN];

	static IDSUPDATESTAMP	m_LastUpdateStamp;
	static DWORD	m_SupportUpdateStamp;

	static DWORD	m_SupportIngestManager;

	static DWORD	m_SupportProcedure;

	static DWORD    m_SupportRefcountRatingctrl;

    static DWORD    m_SQLDeadlockRetryCount;

	ConnChecker* m_pChecker;
//	TriggerWorkerMgr* m_pWorkerMgr;

	CallBackProcessThread* m_pCBThread;

	ManualSyncAdi  m_ManSyncAdi;		// the ManuallySync API interface

	ZQ::common::MiniDump _minidump;

	static void WINAPI MiniDumpCallback(DWORD ExceptionCode, PVOID ExceptionAddress);

private:
	DWORD LoadFilterList(void);

	void FilterSyncDireictory(IN WCHAR* syncdir);
	void CheckSyncDirectory(IN OUT WCHAR* rootdir);
	void CheckSyncDirectory(IN OUT CString& syncDir);


public:
	// initial a full sync to the queue.
	void InitialFullSync();

	bool IsShutingDown() { return m_bShutdownFlag; };

	// add callback to m_pCBThread's Queue
	bool AddingCallbackToThread(DSCallBackBase* pCallback);
	int  GetQueuedCallbackCount();

	/// save current database update stamp to registry
	bool SaveLastStamp();

	/// compare two stamp values
	///@return positive number if s1>s2, negative number if s1<s2, else 0
	int  CompareStamp(IN IDSUPDATESTAMP s1, IN IDSUPDATESTAMP s2);

	ConnChecker* GetConnChecker() { return m_pChecker; };

	void doIdle();
};

#endif // !defined(AFX_DBSYNCSERV_H__95E43CAF_5C0C_407F_9C54_A3DBE6E2220C__INCLUDED_)
