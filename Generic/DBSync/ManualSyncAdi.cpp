#include "stdafx.h"
#include "DBSyncServ.h"
#include "ManualSyncAdi.h"
#include "ids_def.h"
#include "LocalDB.h"
#include "strsafe.h"

using namespace ZQ::common;

extern DBSyncServ g_server;
extern ZQ::common::Log * gpDbSyncLog;
extern std::vector<std::wstring> g_SyncFolders; // Global vector for multiple sync folders
extern DWORD    g_SyncedFolderAsNavNode;// it is only effective when the synced folder count is 1
									    // if it is 0 and only sync one folder, the asset/subfolder under the synced folder in ITV will linked to LAM Application directly
	                                    // if it is 1 and only sync one folder, the synced folder will also as a folder under LAM Nav Application
extern CDSInterface g_ds;	//	Global object of class CDSInterface, implementing data synchronization

extern DWORD g_dwFullSyncCount;

extern LocalDB g_localDB;	//	Global local DB operation object
extern bool m_isBuy;

#define  MANUALLY_SYNC_TYPE_MDD      1
#define  MANUALLY_SYNC_TYPE_FOLDER   2

#define  MANUALLY_SYNC_ERR_SUCCESS           0
#define  MANUALLY_SYNC_ERR_FOLD_NOT_EXIST    1
#define  MANUALLY_SYNC_ERR_FAIL              2
#define  MANUALLY_SYNC_ERR_INVALIDT_TYPE     3

#define  MANUALLY_SYNC_MAX_CONSUME_TIME      3600*1000   // 1 hour

DWORD  ManualSyncAdi::m_syncID = 0;

ManualSyncAdi::ManualSyncAdi()
: m_enabled(false), m_hLib(NULL)
{
	wcsnset(m_addinPath, 0x00, MAX_PATH*sizeof(wchar_t));

	m_procNameVec.clear();
	m_procNameVec.resize(MANUAL_SYNC_COUNT + 1);
	m_procNameVec[0] = "NULL";
	m_procNameVec[1] = MANUAL_SYNC_INIT;
	m_procNameVec[2] = MANUAL_SYNC_RUN;
	m_procNameVec[3] = MANUAL_SYNC_STOP;
}

ManualSyncAdi::~ManualSyncAdi()
{
	if (m_hLib)
	{
		::FreeLibrary(m_hLib);
		m_hLib = NULL;
	}
}

void ManualSyncAdi::SetAddinPath(wchar_t* addinPath)
{
	if(addinPath)
	{
		wcsncpy(m_addinPath, addinPath, wcslen(addinPath));
	}

	// check path
	if( wcscmp(addinPath, L"")     == 0 || 
		wcscmp(addinPath, L"NULL") == 0 )
	{
		m_enabled = false;
	}
	else
	{
		m_enabled = true;
	}
	return ;
}

bool ManualSyncAdi::init()
{
	(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"ManualSyncAdi::init()");

	m_hLib = LoadLibrary(m_addinPath);
	if (m_hLib == NULL)
	{
		int err = GetLastError();
		(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"ManualSync plug-in can not load (\"%s\"), error=%d.", 
												   m_addinPath, err);

		m_enabled = false;
		
		return false;
	}
	else
	{
		(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"ManualSync plug-in (\"%s\") Loaded successfully.", 
												  m_addinPath);
	}

	ManualSync_Proto_Init pfnInit = 
		(ManualSync_Proto_Init)GetProcAddress(m_hLib, m_procNameVec[1].c_str());

	if (pfnInit)
	{
		try
		{
			void (*pCallBack)(_LAM_to_DBSync* ltb, _DBSync_to_LAM* btl) = &ProcData;
			
			if (!(pfnInit)(pCallBack, gpDbSyncLog))
			{
				(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"ManualSyncAdi Add-In init failed, so the Server will stop");
				return false;
			}
		}
		catch(...)
		{
			(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, 
						   L"Unknown exception caught when calling (%s) and the Server will stop", 
						   m_procNameVec[1].c_str());
			return false;
		}
	}
	else
	{
		int err = GetLastError();
		(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, L"Get [%s] failed, Error is %d", 
					   m_procNameVec[1].c_str(), err);
		return false;
	}
	
	return true;
}

int ManualSyncAdi::run()
{
	if (m_enabled == false)
	{
		return 0;
	}

	(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"ManualSyncAdi::run()");

	ManualSync_Proto_Run pfnRun = 
		(ManualSync_Proto_Run)GetProcAddress(m_hLib, m_procNameVec[2].c_str());

	if (pfnRun)
	{	
		try
		{
			if(!(pfnRun)())
			{
				(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"ManualSyncAdi Add-In run failed, so the Server will stop");
				return 0;
			}
		}
		catch(...)
		{
			(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, 
						   L"Unknown exception caught when calling (%s) and the Server will stop", 
						   m_procNameVec[2].c_str());
			return 0;
		}
	}
	else
	{
		int err = GetLastError();
		(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, L"Get [%s] failed, Error is %d", 
					   m_procNameVec[2].c_str(), err);
		return 0;
	}
	return 1;
}

void ManualSyncAdi::final()
{
	if (m_enabled == false)
	{
		return;
	}

	(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"ManualSyncAdi::final()...");
	
	ManualSync_Proto_Stop pfnFinal = 
		(ManualSync_Proto_Stop)GetProcAddress(m_hLib, m_procNameVec[3].c_str());

	if (pfnFinal)
	{
		try
		{
			(pfnFinal)();
		}
		catch(...)
		{
			(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, 
						   L"Unknown exception caught when calling (%s).",
						   m_procNameVec[3].c_str());
		}
	}
	else
	{
		int err = GetLastError();
		(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, 
					   L"Get [%s] failed, Error is %d", 
					   m_procNameVec[3].c_str(), err);
	}
	(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"Leaving ManualSyncAdi::UnInit()");
}


void ManualSyncAdi::ProcData(_LAM_to_DBSync* ltb, _DBSync_to_LAM* btl)
{
	// sync function
	if (MANUALLY_SYNC_TYPE_MDD == ltb->_SyncType)  //it's a MDD
	{
		HANDLE syncEvent = ::CreateEvent(NULL,FALSE,FALSE,NULL);

		DWORD syncResult = 0;

		m_syncID++;

		(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"ManualSyncThd::ProcData() - received a MDD manually sync request(triggerId: %d), put it into callback queue", m_syncID);

		// put the manually MDD sync request to callback queue
		ManualSyncMDDCallback* msyncMDD = new ManualSyncMDDCallback(syncEvent, m_syncID, syncResult);
		g_server.AddingCallbackToThread(msyncMDD);

		(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"ManualSyncThd::ProcData() - waiting for MDD manually sync(triggerId: %d) completion. ", m_syncID);

		// waiting the MDD sync completion.
		WaitForSingleObject(syncEvent, MANUALLY_SYNC_MAX_CONSUME_TIME);

		SetErrCode(btl, syncResult);

		CloseHandle(syncEvent);
		
		(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"ManualSyncThd::ProcData() - MDD manually sync(triggerId: %d) completed. ", m_syncID);
	}
	else if (MANUALLY_SYNC_TYPE_FOLDER == ltb->_SyncType)  //it's a path
	{
		HANDLE syncEvent = ::CreateEvent(NULL,FALSE,FALSE,NULL);

		DWORD syncResult = 0;

		m_syncID++;

		(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"ManualSyncThd::ProcData() - received a Folder manually sync request(triggerId: %d, syncFolder: %s), put it into callback queue", m_syncID, ltb->_SyncPath);

		CString syncFolder(ltb->_SyncPath);

		ManualSyncFolderCallback* msyncFolder = new ManualSyncFolderCallback(syncEvent, m_syncID, syncFolder, syncResult);
		g_server.AddingCallbackToThread(msyncFolder);

		(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"ManualSyncThd::ProcData() - waiting for Folder manually sync(triggerId: %d, syncFolder: %s) completion. ", m_syncID, ltb->_SyncPath);

		// waiting the MDD sync completion.
		WaitForSingleObject(syncEvent, MANUALLY_SYNC_MAX_CONSUME_TIME);

		SetErrCode(btl, syncResult);

		CloseHandle(syncEvent);

		(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"ManualSyncThd::ProcData() - Folder manually sync(triggerId: %d, syncFolder: %s) completed. ", m_syncID, ltb->_SyncPath);
	}
	else
	{
		(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, L"Not Existed Sync Type");
		SetErrCode(btl, 3);
	}
}


void ManualSyncAdi::SetErrCode(IN     _DBSync_to_LAM* btl,
							   IN     DWORD err)
{
	if (MANUALLY_SYNC_ERR_FOLD_NOT_EXIST == err)
	{
		StringCbPrintf(btl->_errorCode, MAX_PATH*sizeof(wchar_t), L"%d", MANUALLY_SYNC_ERR_FOLD_NOT_EXIST);
		StringCbPrintf(btl->_errorDescription, MAX_PATH*sizeof(wchar_t), L"Not Find Sync Folder");
	}
	else if (MANUALLY_SYNC_ERR_FAIL == err)
	{
		StringCbPrintf(btl->_errorCode, MAX_PATH*sizeof(wchar_t), L"%d", MANUALLY_SYNC_ERR_FAIL);
		StringCbPrintf(btl->_errorDescription, MAX_PATH*sizeof(wchar_t), L"Sync Failed", 40);
	}
	else if (MANUALLY_SYNC_ERR_INVALIDT_TYPE == err)
	{
		StringCbPrintf(btl->_errorCode, MAX_PATH*sizeof(wchar_t), L"%d", MANUALLY_SYNC_ERR_INVALIDT_TYPE);
		StringCbPrintf(btl->_errorDescription, MAX_PATH*sizeof(wchar_t), L"Not Existed Sync Type");
	}
	else // succeed
	{
		StringCbPrintf(btl->_errorCode, MAX_PATH*sizeof(wchar_t), L"%d", MANUALLY_SYNC_ERR_SUCCESS);
		StringCbPrintf(btl->_errorDescription, MAX_PATH*sizeof(wchar_t), L"");
	}
}

ManualSyncFolderCallback::ManualSyncFolderCallback(HANDLE notifyHandle, DWORD triggerID, CString& syncFolder, DWORD& syncResult)
: DSCallBackBase(MANUALLYSYNC, triggerID), m_hCompleteNotify(notifyHandle), m_triggerID(triggerID), m_syncFolder(syncFolder), m_syncResult(syncResult)
{
}

ManualSyncFolderCallback::~ManualSyncFolderCallback()
{
}

bool ManualSyncFolderCallback::ReserveAdjustFolder(const WCHAR* ori_root, CString& outString)
{
	outString = ori_root;
	TRACE(L"%s\n", outString);

	// remove space on head and tail
	outString.TrimLeft();
	outString.TrimRight();

	// head must be '\'
	if (outString.Left(1) != _T('\\'))
	{
		outString.Insert(0, _T('\\'));
	}

	// tail must be '\' 
	if (outString.Right(1) != _T('\\'))
	{
		outString.Insert(outString.GetLength(), _T('\\'));
	}

	TRACE(L"%s\n", outString);
	
	int nSyncFolderCount = g_SyncFolders.size();

	// If only sync one folder and the synced folder did not as the nav node
	if (nSyncFolderCount == 1 && g_SyncedFolderAsNavNode == 0)
	{
		(*gpDbSyncLog)(ZQ::common::Log::L_DEBUG, L"One Folder And Nav Node");

		// get the sync folder name
		CString syncFolderName = g_SyncFolders.at(0).c_str();
		
		if (wcscmp(syncFolderName, _T("\\")) == 0)
		{
			return true;
		}

		if (!CombinePath(outString, syncFolderName))
		{
			return false;
		}

		return true;
	}


	// Multiple folders syncing or one folder syncing which will as the nav folder
	int nSyncLoop = 0;
	for (nSyncLoop = 0; nSyncLoop < nSyncFolderCount; nSyncLoop++)
	{
		(*gpDbSyncLog)(ZQ::common::Log::L_DEBUG, L"Multiple Folders or SyncedFolderNavNode=1");

		// get the sync folder name
		CString syncFolderName = g_SyncFolders.at(nSyncLoop).c_str();
		TRACE(L"%s\n", syncFolderName);
		TRACE(L"%s\n", outString);
		
		int appIndex = outString.Find(syncFolderName);
		if (appIndex != -1)
		{
			return true;
		}
	}

	return false;
}

bool ManualSyncFolderCallback::CombinePath(CString& outString, CString& syncFolderName)
{
	TRACE(L"%s\n", outString);

	int outPrefIndex = outString.Find(_T("\\"), 1);
	int outNextIndex = outPrefIndex;
	CString outNode  = outString.Left(outPrefIndex);

	int syncPrefIndex = syncFolderName.Find(_T("\\"), 1);
	int syncNextIndex = syncPrefIndex;
	CString syncNode  = syncFolderName.Left(syncPrefIndex);
	
	if (outNode != syncNode)
	{
		/* if the first directory is not equal, it is a wrong path  */
		return false;
	}

	outPrefIndex  = outString.Find(_T("\\"), outNextIndex + 1);
	syncPrefIndex = syncFolderName.Find(_T("\\"), syncNextIndex + 1);
	
	while ((outPrefIndex != -1) && (syncPrefIndex != -1))
	{
		outNode  = outString.Left(outPrefIndex);
		outNode  = outNode.Right(outPrefIndex - outNextIndex);
		
		syncNode = syncFolderName.Left(syncPrefIndex);
		syncNode = syncNode.Right(syncPrefIndex - syncNextIndex);

		if (outNode != syncNode)
		{
			outString.Insert(outNextIndex, syncNode);
			outNextIndex += syncNode.GetLength();
			outPrefIndex += syncNode.GetLength();
		}
		else
		{
			outNextIndex = outPrefIndex;
			outPrefIndex = outString.Find(_T("\\"), outNextIndex + 1);
		}

		syncNextIndex = syncPrefIndex;
		syncPrefIndex = syncFolderName.Find(_T("\\"), syncNextIndex + 1);
	}
	
	if ( (outPrefIndex == -1) && (syncPrefIndex != -1) )
	{
		outString = syncFolderName;
	}

	TRACE(L"%s\n", outString);
	return true;
}

DWORD ManualSyncFolderCallback::GetParentUID(CString& path)
{
	SQLTCHAR     sql[1024];

	CString strPath = path;		

	CString entryName;
	CString leftPath, parentName;
	
	int index = strPath.Find(L"\\");

	DWORD dwEntryUID       = 0;
	DWORD dwParentEntryUID = 0;
	WORD parentType;

	while (index != -1)
	{
		entryName = strPath.Left(index);
		if (entryName == _T(""))
		{
			dwParentEntryUID = -1;
			dwEntryUID       = -1;
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ENTRYTYPE FROM HIERARCHY WHERE LOCALHIERARCHYUID = 'H0'"));
			g_localDB.SelectSQL(sql, SQL_C_ULONG, &parentType, 0);

			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT HIERARCHYUID FROM HIERARCHY WHERE LOCALHIERARCHYUID = 'H0'"));
			g_localDB.SelectSQL(sql, SQL_C_ULONG, &dwEntryUID, 0);
		}
		else
		{
			entryName.Replace(_T("'"), _T("''"));
			parentName = entryName;
			dwParentEntryUID = dwEntryUID;
			
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT ENTRYTYPE FROM HIERARCHY WHERE PARENTHUID = %d AND ENTRYNAME = N'%s'"),
						   dwEntryUID, entryName);

			g_localDB.SelectSQL(sql, SQL_C_ULONG, &parentType, 0);

			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT HIERARCHYUID FROM HIERARCHY WHERE ENTRYNAME = N'%s' AND PARENTHUID = %d"),
						   entryName,dwEntryUID);
			
			g_localDB.SelectSQL(sql,SQL_C_ULONG,&dwEntryUID,0);
		}

		strPath = strPath.Right(strPath.GetLength() - index - 1);
		index = strPath.Find(_T("\\"));
	}
	dwParentEntryUID = dwEntryUID;

	return dwParentEntryUID;
}

int ManualSyncFolderCallback::ManuallySyncFolder(CString& syncFolderName)
{
	CString folderName = syncFolderName;

	APPNAME *pApps = 0;	//	structure to store application info
	
	DWORD res    = 0;
	DWORD appNum = 0;
	
	//	Get number of applications and all application info
	res = IdsListApplications(&g_ds.m_idssess, 
		                      &pApps, &appNum,
							  &g_ds.m_itvsta, 0);
	if (res)
	{
		(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, _T("IdsListApplications ERROR!0X%X"),res);

		return MANUALLY_SYNC_ERR_FAIL;
	}

	DWORD entryUid, entryType;

	//	 convert synchronizing root got from registry to hierarchy uid
	if (!g_ds.GetEntryHUIDByName((LPTSTR)(LPCTSTR)folderName, entryUid, entryType))
	{
		(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, _T("Specified Synchronizing Directory \"%s\" does not exist!"),syncFolderName);
		return MANUALLY_SYNC_ERR_FOLD_NOT_EXIST;
	}

	(*gpDbSyncLog)(ZQ::common::Log::L_INFO, _T("entry Uid is %d, entry Type is %d"), entryUid, entryType);

	DWORD parentUID = GetParentUID(folderName);
	
	(*gpDbSyncLog)(ZQ::common::Log::L_INFO, _T("parent UID is %d"), parentUID);

	(*gpDbSyncLog)(ZQ::common::Log::L_NOTICE, _T("Begin manually sync folder %s"), syncFolderName);
	
	res = g_ds.SaveEntriesToDB(entryUid , entryType, parentUID, 1);
	if(res != ITV_SUCCESS)
	{
		(*gpDbSyncLog)(ZQ::common::Log::L_NOTICE, _T("Met error during sync folder %s"), syncFolderName);
		return MANUALLY_SYNC_ERR_FAIL;
	}

	(*gpDbSyncLog)(ZQ::common::Log::L_NOTICE, _T("End of manually sync folder %s"), syncFolderName);

	return MANUALLY_SYNC_ERR_SUCCESS;
}


DWORD ManualSyncFolderCallback::Process(void)
{
	(*gpDbSyncLog)(ZQ::common::Log::L_INFO, _T("%d:Entering ManualSyncFolderCallback: triggerId: %d, syncFolder: %s"), 
		GetCurrentThreadId(), m_triggerID, m_syncFolder);

	m_syncResult = MANUALLY_SYNC_ERR_SUCCESS;

	if (m_syncFolder == L"\\")
	{
		DWORD dwStart = GetTickCount();

		// doing full syncing
		DWORD ret = g_ds.UpdateAll();

		DWORD timespan = (GetTickCount() - dwStart) / (DWORD)1000;

		if(ret != 0)  // sync fail
		{
			m_syncResult = MANUALLY_SYNC_ERR_FAIL;
			(*gpDbSyncLog)(Log::L_ERROR, "[ManualSync] UpdateAll() for No.%d full syncing failed, spent %d seconds", g_dwFullSyncCount, timespan);
		}
		else
		{
			(*gpDbSyncLog)(Log::L_NOTICE, "[ManualSync] UpdateAll() for No.%d full syncing succeed, spent %d seconds", g_dwFullSyncCount, timespan);
		}
		
		SetEvent(m_hCompleteNotify);

		return 0;
	}
	
	(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"Adjust sync folder according the syncDirectory configuration");
	
	CString folderName;

	if(!ReserveAdjustFolder(m_syncFolder, folderName))
	{
		(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, L"Sync Folder Name <%s> does not exist", m_syncFolder);

		m_syncResult = MANUALLY_SYNC_ERR_FOLD_NOT_EXIST;

		SetEvent(m_hCompleteNotify);

		return 0;
	}

	(*gpDbSyncLog)(ZQ::common::Log::L_INFO, L"%s was adjust to be %s", m_syncFolder, folderName);

	m_syncResult = ManuallySyncFolder(folderName);
	
	(*gpDbSyncLog)(ZQ::common::Log::L_INFO, _T("%d:Leaving ManualSyncFolderCallback: triggerId: %d, syncFolder: %s"), 
		GetCurrentThreadId(), m_triggerID, m_syncFolder);

	SetEvent(m_hCompleteNotify);
	
	return 0;
}


ManualSyncMDDCallback::ManualSyncMDDCallback(HANDLE notifyHandle, DWORD triggerID, DWORD& syncResult)
: DSCallBackBase(MANUALLYSYNC, triggerID), m_hCompleteNotify(notifyHandle), m_triggerID(triggerID), m_syncResult(syncResult)
{
}
	
ManualSyncMDDCallback::~ManualSyncMDDCallback()
{
}

DWORD ManualSyncMDDCallback::Process(void)
{
	(*gpDbSyncLog)(ZQ::common::Log::L_INFO, _T("%d:Entering ManualSyncMDDCallback: triggerId: %d, MDD sync"), 
		GetCurrentThreadId(), m_triggerID);

	DWORD res = g_ds.SaveEntryMDDToDB();
	if(res != ITV_SUCCESS)
	{
		(*gpDbSyncLog)(ZQ::common::Log::L_ERROR, L"SaveEntryMDDToDB() error");
	}
	
	SetEvent(m_hCompleteNotify);

	(*gpDbSyncLog)(ZQ::common::Log::L_INFO, _T("%d:Leaving ManualSyncMDDCallback: triggerId: %d, MDD sync"), 
		GetCurrentThreadId(), m_triggerID);


	return 0;
}
