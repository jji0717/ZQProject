// DBSyncServ.cpp: implementation of the DBSyncServ class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <string>
#include "DBSyncServ.h"
#include "DSInterface.h"
#include "LocalDB.h"
#include "log.h"
#include "ZQResource.h"
#include "DBSAdiMan.h"
#include "DSCallBack.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


wchar_t DBSyncServ::m_ITVServerIP[MAXNAMELEN] = L"";
wchar_t DBSyncServ::m_ITVUserName[MAXNAMELEN] = L"";
wchar_t DBSyncServ::m_IZQDB_DSN[MAXNAMELEN] = L"";
wchar_t DBSyncServ::m_IZQDB_UserName[MAXNAMELEN] = L"";
wchar_t DBSyncServ::m_IZQDB_Password[MAXNAMELEN] = L"";
DWORD DBSyncServ::m_IZQ_PlayInterval = DEFAULT_IZQ_PLAYINTERVAL;
wchar_t DBSyncServ::m_IZQDB_Type[MAXNAMELEN] = DEFAULT_DB_TYPE;
wchar_t DBSyncServ::m_MDListString[2048] = _T("");
DWORD DBSyncServ::m_ITVCheckInterval= DEFAULT_ITV_CHECKINTERVAL;

wchar_t DBSyncServ::m_SyncDirectory[4096] = L"\\";
DWORD DBSyncServ::m_TimeWindowThreshold = 600;

DWORD	DBSyncServ::m_SupportNavigation = 1;
wchar_t	DBSyncServ::m_PluginPath[MAXNAMELEN] = L"NULL";
wchar_t DBSyncServ::m_DBSyncIP[MAXNAMELEN] = L"unknown";
wchar_t DBSyncServ::m_ManualSyncAddInPath[MAXNAMELEN] = L"NULL";

IDSUIDUPDATESTAMP DBSyncServ::m_LastUpdateStamp={0,0,0,0,0,0,0,0};
DWORD	DBSyncServ::m_SupportUpdateStamp = 0;

DWORD	DBSyncServ::m_SupportIngestManager = 1;

DWORD	DBSyncServ::m_SupportProcedure =1;

DWORD	DBSyncServ::m_SupportRefcountRatingctrl = 0;

DWORD   DBSyncServ::m_SQLDeadlockRetryCount = 3;

WCHAR g_ITVServerIP[MAXNAMELEN];	//	Global string of ITV server IP address
WCHAR g_ITVUserName[MAXNAMELEN];	//	Global string of ITV server login user name
TCHAR g_IZQDSNName[MAXNAMELEN];	//	Global string of ODBC data source name
TCHAR g_IZQUserName[MAXNAMELEN];	//	Global string of ODBC connecting user name
TCHAR g_IZQPassword[MAXNAMELEN];	//	Global string of ODBC connecting password
CString g_Username;				//	Global string of domain user name while registration
CString g_UserPassword;			//	Global string of password while registration
CString g_ModulePath;			//	Global string of work path of the module
DWORD g_playInterval;			//	Global float of element play interval
TCHAR g_IZQDBType[MAXNAMELEN];		//	Global string of ODBC data source database type

WCHAR g_SyncDirectory[MAXNAMELEN*2];	// Global string of DBSync synchronizing root
std::vector<std::wstring> g_SyncFolders;// Global vector for multiple sync folders
std::vector<std::wstring> g_Applications; // Global vector for the applications that the synced folder belong to 
DWORD    g_SyncedFolderAsNavNode = 0;   // it is only effective when the synced folder count is 1
									    // if it is 0 and only sync one folder, the asset/subfolder under the synced folder in ITV will linked to LAM Application directly
	                                    // if it is 1 and only sync one folder, the synced folder will also as a folder under LAM Nav Application

DWORD    g_SyncAllAtBegin = 1;  // if it is 0, the server update all when it begins.
								// if it is 1, the server don't update all when it begins.

DWORD	 g_TraceTransactionCount = 0;  // flag to trace transaction or not

// Global word of time window threshold within which all metadatas about time are ignored
// these metadatas include ('ActivatedTime', 'DeactivateTime', 'DeleteTime')
DWORD g_TimeWindowThreshold;	

DWORD g_SupportNavigation;		//	Global configure of support NAV database or not.
WCHAR g_PluginPath[MAXNAMELEN];		//  Global string of DBSync plugin
WCHAR g_DBSyncIP[MAXNAMELEN];		// Global string of DBSync IP

IDSUIDUPDATESTAMP g_LastUpdateStamp;	// Global update stamp, 8 bytes
DWORD g_SupportUpdateStamp;		// Global update stamp switch

DWORD g_SupportIngestManager;	// Global configure of support IM xml generation

DWORD g_SupportProcedure;		// Global configure of support SQL stored procedure when starting

DWORD g_SupportRefcountRatingctrl; // Global configure of support KDDI refcount and rating control

DWORD gUpdateThreadId = 0;

CDSInterface g_ds;	//	Global object of class CDSInterface, implementing data synchronization

LocalDB g_localDB;	//	Global object of class LocalDB, implementing local DB access via ODBC

DBSAdiMan	g_AdiMan;	//  Global object of class DBSAdiMan, implementing DBSync Add-in Manager

DWORD       g_SQLDeadlockRetryCount = 3; // Global configure of re-execute the failed sql count

using ZQ::common::BaseSchangeServiceApplication;
using ZQ::common::Log;

ZQ::common::Log* gpDbSyncLog;

DBSyncServ g_server;
BaseSchangeServiceApplication * Application = &g_server;

DWORD gdwServiceType = DBSYNC_SERVICETYPE;
DWORD gdwServiceInstance = 0;

extern DWORD g_dwFullSyncCount;

DBSyncServ::DBSyncServ():BaseSchangeServiceApplication()
{

}

DBSyncServ::~DBSyncServ()
{

}
HRESULT DBSyncServ::OnInit(void)
{
	BaseSchangeServiceApplication::OnInit();

	DWORD size = 0;
	TCHAR	szPath[MAX_PATH];	//	path and file name of current module
	//	Get path and file name of current module
	if (!GetModuleFileName(0, szPath, MAX_PATH))	//	error occurs
		return 1;	//	Exit

	//	parse and get the work path for the module

	g_ModulePath = szPath;
	int index = g_ModulePath.ReverseFind(_T('\\'));
	g_ModulePath = g_ModulePath.Left(index);

	gpDbSyncLog = m_pReporter;

	size = sizeof(m_ITVServerIP);
	getConfigValue(_T("ITVServerAddress"),m_ITVServerIP,m_ITVServerIP,&size,true,true);
	wcscpy(g_ITVServerIP,m_ITVServerIP);
	size = sizeof(m_ITVUserName);
	getConfigValue(_T("ITVUserName"),m_ITVUserName,m_ITVUserName,&size,true,true);
	wcscpy(g_ITVUserName,m_ITVUserName);
	size = sizeof(m_IZQDB_DSN);
	getConfigValue(_T("IZQDBDSN"),m_IZQDB_DSN,m_IZQDB_DSN,&size,true,true);
	wcscpy(g_IZQDSNName,m_IZQDB_DSN);
	size = sizeof(m_IZQDB_UserName);
	getConfigValue(_T("IZQDBUserName"),m_IZQDB_UserName,m_IZQDB_UserName,&size,true,true);
	wcscpy(g_IZQUserName,m_IZQDB_UserName);
	size = sizeof(m_IZQDB_Password);
	getConfigValue(_T("IZQDBPassword"),m_IZQDB_Password,m_IZQDB_Password,&size,true,true);
	wcscpy(g_IZQPassword,m_IZQDB_Password);
	size = sizeof(m_IZQDB_Type);
	getConfigValue(_T("IZQDBType"),m_IZQDB_Type,m_IZQDB_Type,&size,true,true);
	wcscpy(g_IZQDBType,m_IZQDB_Type);
		
	getConfigValue(_T("IZQPlayInterval"),&m_IZQ_PlayInterval,m_IZQ_PlayInterval,true,true);
	g_playInterval = m_IZQ_PlayInterval;
	
	getConfigValue(_T("ITVCheckInterval"),&m_ITVCheckInterval,m_ITVCheckInterval,true,true);

	getConfigValue(_T("TimeWindowThreshold"), &m_TimeWindowThreshold, m_TimeWindowThreshold, true, true);
	g_TimeWindowThreshold = m_TimeWindowThreshold;

	//	---------- modified by KenQ 2006-8-2-------------
	getConfigValue(_T("SyncedFolderAsNavNode"), &g_SyncedFolderAsNavNode, g_SyncedFolderAsNavNode, true, true);

	size = sizeof(m_SyncDirectory);
	getConfigValue(_T("SyncDirectory"),m_SyncDirectory,m_SyncDirectory,&size,true,true);
	//	---------- modified by KenQ 2006-05-18-------------
	// To filter the multiple sync folders which separated by semicolon, 
	// these sync folders are stored in g_SyncFolders
	FilterSyncDireictory(m_SyncDirectory);
	(*gpDbSyncLog)(Log::L_DEBUG, _T("DBSync will synchronize %d folders as follows:"), g_SyncFolders.size());

	for(int i=0; i<g_SyncFolders.size(); i++)
	{
		(*gpDbSyncLog)(Log::L_DEBUG, _T("Synchronized folder - %s"), g_SyncFolders.at(i).c_str());
	}

	for(int j=0; j<g_Applications.size(); j++)
	{
		(*gpDbSyncLog)(Log::L_DEBUG, _T("Synchronized folder included application - %s"), g_Applications.at(j).c_str());
	}

//	CheckSyncDirectory(m_SyncDirectory);
//	(*gpDbSyncLog)(Log::L_DEBUG, _T("[SyncDirectory] has been adjusted to [%s]"), m_SyncDirectory);
//	wcscpy(g_SyncDirectory, m_SyncDirectory);
	
	getConfigValue(_T("SupportNavigation"), &m_SupportNavigation, m_SupportNavigation, true, true);
	g_SupportNavigation = m_SupportNavigation;

	size = sizeof(m_PluginPath);
	getConfigValue(_T("IDSSyncEventPluginPath"), m_PluginPath, m_PluginPath, &size, true, true);
	wcsncpy(g_PluginPath, m_PluginPath, sizeof(g_PluginPath)/sizeof(wchar_t));

//  removed by Ken on 2006-12-25. Use the IP set in IDSSyncEventPlugin registry
//	size = sizeof(m_DBSyncIP);
//	getConfigValue(_T("DBSyncIP"), m_DBSyncIP, m_DBSyncIP, &size, true, true);
//	wcsncpy(g_DBSyncIP, m_DBSyncIP, sizeof(g_DBSyncIP)/sizeof(wchar_t));

	// get last update stamp var
	getConfigValue(_T("SupportUpdateStamp"), &m_SupportUpdateStamp, m_SupportUpdateStamp, true, true);
	g_SupportUpdateStamp = m_SupportUpdateStamp;

	size = sizeof(m_ManualSyncAddInPath);
	getConfigValue(_T("ManualSyncPluginPath"), m_ManualSyncAddInPath, m_ManualSyncAddInPath, &size, true, true);

	getConfigValue(_T("SyncAllAtBegin"),  &g_SyncAllAtBegin, g_SyncAllAtBegin, true, true);

	//	---------- modified by HuangLi 2007-07-02-------------
	// get re-execute the failed sql count
	getConfigValue(_T("SQLDeadlockRetryCount"), &m_SQLDeadlockRetryCount, m_SQLDeadlockRetryCount, true, true);
	g_SQLDeadlockRetryCount = m_SQLDeadlockRetryCount;

	if(g_SupportUpdateStamp)
	{
	
		wchar_t tmpStamp[17]=L"0000000000000000";
		size = sizeof(tmpStamp);
		getConfigValue(_T("LastUpdateStamp"), tmpStamp, tmpStamp, &size, true, false);
		int l = wcslen(tmpStamp);
		swscanf(tmpStamp, L"%02X%02X%02X%02X%02X%02X%02X%02X", 
			&m_LastUpdateStamp[0],
			&m_LastUpdateStamp[1],
			&m_LastUpdateStamp[2],
			&m_LastUpdateStamp[3],
			&m_LastUpdateStamp[4],
			&m_LastUpdateStamp[5],
			&m_LastUpdateStamp[6],
			&m_LastUpdateStamp[7]);
		memcpy(g_LastUpdateStamp, m_LastUpdateStamp, sizeof(g_LastUpdateStamp));

	}
	
	getConfigValue(_T("SupportIngestManager"), &m_SupportIngestManager, m_SupportIngestManager, true, true);
	g_SupportIngestManager = m_SupportIngestManager;

	getConfigValue(_T("SupportProcedure"), &m_SupportProcedure, m_SupportProcedure, true, true);
	g_SupportProcedure = m_SupportProcedure;

	// Whether support KDDI refcount and rating control feature
	getConfigValue(_T("SupportRefcountRatingCtrl"), &m_SupportRefcountRatingctrl, m_SupportRefcountRatingctrl, true, true);
	g_SupportRefcountRatingctrl = m_SupportRefcountRatingctrl;


	getConfigValue(_T("TraceTransactionCount"),  &g_TraceTransactionCount, g_TraceTransactionCount, true, true);
	
	m_pChecker = new ConnChecker(m_ITVCheckInterval*60*1000);

	m_pCBThread = new CallBackProcessThread();

	g_AdiMan.setPluginPath(m_PluginPath);
	
	m_ManSyncAdi.SetAddinPath(m_ManualSyncAddInPath);

	// get mini dump path
	wchar_t logFilePath[MAX_PATH];
	wcscpy(logFilePath, m_wsLogFileName);
	int pos = wcslen(logFilePath)-1;
	
	for(; pos>=0; pos--)
	{
		if(logFilePath[pos] == L'\\' && (pos < MAX_PATH - 2) )
		{
			logFilePath[pos+1] = L'\0';
			break;
		}
	}
	// initialize the mini dump
	(*gpDbSyncLog)(Log::L_DEBUG, _T("DBSync crash dump file path is %s"), logFilePath);

	_minidump.setDumpPath(logFilePath);
	_minidump.enableFullMemoryDump(true);
	_minidump.setExceptionCB(MiniDumpCallback);

	return S_OK;
}
HRESULT DBSyncServ::OnUnInit(void)
{
	//
	(*gpDbSyncLog)(Log::L_DEBUG, _T("Enter DBSyncServ::OnUnInit()"));
	
	if (m_pChecker)
	{
		delete m_pChecker;
		m_pChecker = NULL;
	}

	if(m_pCBThread != NULL)
	{
		delete m_pCBThread;
		m_pCBThread = NULL;
	}

	//
	// IDS un-initialization
	// 
	HRESULT hr = S_OK;
	g_ds.Uninitialize();

	//
	// un-init DB
	//
	(*gpDbSyncLog)(Log::L_DEBUG, _T("Enter DBSyncServ::OnUnInit(): Uninit DBConnection..."));
	DWORD res = g_ds.UninitDB();
	if (res)		//	error occurs
	{
		(*gpDbSyncLog)(Log::L_ERROR, _T("UninitDB ERROR!"));
		hr = S_FALSE;
	}

	//	Log the termination of the service
	(*gpDbSyncLog)(Log::L_NOTICE, _T("********************kernel Exit*************************"));
	hr = BaseSchangeServiceApplication::OnUnInit();

	(*gpDbSyncLog)(Log::L_DEBUG, _T("Leave DBSyncServ::OnUnInit()"));

	return hr;
}
HRESULT DBSyncServ::OnStart(void)
{
	BaseSchangeServiceApplication::OnStart();
	DWORD res = 0;	//	return code
	
	(*gpDbSyncLog)(Log::L_NOTICE ,_T("***********************DBSync Kernel Starting version %S***********************"),ZQ_PRODUCT_VER_STR3);
		
	(*gpDbSyncLog)(Log::L_DEBUG ,_T("Enter DBSyncServ::OnStart()"));

	//  call initialize of Add-in
	if(g_AdiMan.isValid())
	{
		DA_dbsyncInfo	dbsInfo;
		DA_itvInfo		itvInfo;

		wcsncpy(dbsInfo._szIPAddr, m_DBSyncIP, 16);
		wcsncpy(dbsInfo._szVersion, _T(ZQ_PRODUCT_VER_STR1), 16);
		wcsncpy(dbsInfo._szSyncDir, m_SyncDirectory, MAX_PATH);
		dbsInfo._dwInstanceID = m_typeinst->s.dwInst;
		dbsInfo._dwSupportNav = m_SupportNavigation;
		dbsInfo._dwTwThreshold = m_TimeWindowThreshold;

		wcsncpy(itvInfo._szIPAddr, m_ITVServerIP, 16);
		wcsncpy(itvInfo._szVersion, L"1.0", 16);

		g_AdiMan.Init(&dbsInfo, &itvInfo);
	}

	if(m_ManSyncAdi.IsValid())
	{
		bool ret = m_ManSyncAdi.init();
		if (!ret)
		{
			(*gpDbSyncLog)(Log::L_ERROR, _T("Manul Sync Addin failed"));
			OnStop();
			return -1;
		}
	}

	//
	// initialize DB connection
	//
	while(!m_bShutdownFlag && (g_ds.InitDB()) != 0)  // no manually stop, keep connecting to DB until success
	{
		(*gpDbSyncLog)(Log::L_ERROR,_T("Initialize local database connection ERROR! Reconnect it 5 seconds later"));	
		::Sleep(5000);
	}

	//
	// initialize IDS connection, and register trigger
	//
	(*gpDbSyncLog)(Log::L_INFO, _T("DS Initialize began..."));
	bool bRet = g_ds.Initialize();
	if(!bRet)
	{	// IDSConnInitialize() is a block function until IDS connection is ready, 
		// or DBSync is stopping manually. 
		return -1;
	}	
	(*gpDbSyncLog)(Log::L_NOTICE, _T("IDS connection initialization succeed"));

	// -------- Add by KenQ 2006-05-18 
	// To check whether the defined sync folders existed in ITV AM or not
	if(!g_ds.CheckSyncFoldersExistence())
	{
		// (*gpDbSyncLog)(Log::L_ERROR,_T("There is none-existing sync folder, please check the configuration"));
		
		g_ds.Uninitialize();

		return -1;
	}
	
	if(m_ManSyncAdi.IsValid())
	{
		bool ret = m_ManSyncAdi.run();
		if (!ret)
		{
			(*gpDbSyncLog)(Log::L_ERROR, _T("Run ManulSync Addin failed"));

			return -1;
		}
	}

	//
	//	initial a full sync to the queue
	//
	InitialFullSync();

	//
	// start IDS connection checking thread
	//
	(*gpDbSyncLog)(Log::L_DEBUG, L"Start IDS connection checking thread...");
	g_server.m_pChecker->start();

	//
	// start callback process thread
	//
	(*gpDbSyncLog)(Log::L_DEBUG, L"Start Queue processing thread...");
	m_pCBThread->start();
	
	
	(*gpDbSyncLog)(Log::L_DEBUG ,_T("Leave DBSyncServ::OnStart()"));

    return 0;	//	success
}

HRESULT DBSyncServ::OnStop(void)
{
	(*gpDbSyncLog)(Log::L_DEBUG, _T("Enter DBSyncServ::OnStop()"));

	//
	// stop threads
	//
	(*gpDbSyncLog)(Log::L_DEBUG, _T("DBSyncServ::OnStop() stop IDS checking thread..."));

	m_pChecker->StopChecker();	
	m_pChecker->waitHandle(1000);

	(*gpDbSyncLog)(Log::L_DEBUG, _T("DBSyncServ::OnStop() stop IDS Callback processing thread thread..."));
	
	m_pCBThread->StopProcessing();
		
	m_pCBThread->waitHandle(500);

	// call Add-in to Uninit
	if(g_AdiMan.isValid())
	{
		(*gpDbSyncLog)(Log::L_DEBUG, _T("DBSyncServ::OnStop() IDSSyncEventPlugin uninit..."));
		g_AdiMan.Uninit();
	}

	if(m_ManSyncAdi.IsValid())
	{
		(*gpDbSyncLog)(Log::L_DEBUG, _T("DBSyncServ::OnStop() ManualSyncPlugin uninit..."));
		m_ManSyncAdi.final();
	}

	HRESULT hr = BaseSchangeServiceApplication::OnStop();
	
	(*gpDbSyncLog)(Log::L_DEBUG, _T("Leave DBSyncServ::OnStop()"));

	return hr;
}


bool DBSyncServ::isHealth(void)
{
	// add the thread check when time out
	if(!m_pChecker->isRunning())
	{
		delete m_pChecker;
		m_pChecker = new ConnChecker(m_ITVCheckInterval*60*1000);
		m_pChecker->start();
	}
	return true;
}

void DBSyncServ::exitProcess(void)
{
	exit(1);
}

void DBSyncServ::FilterSyncDireictory(IN WCHAR* syncdir)
{
	if(wcscmp(syncdir, _T("\\"))==0)
	{
		g_SyncFolders.push_back(syncdir);
		return;
	}
	POSITION pos = NULL;
	POSITION prepos = NULL;
	CStringList tempFolderList;
	CString strExistedFd = L"";

	CString syncFolders = syncdir;
	
	syncFolders.TrimLeft();
	syncFolders.TrimRight();
	if(syncFolders.Right(1) != _T(";"))
		syncFolders = syncFolders + L";";

	CString syncFd = L"";
	CString syncFdTemp = L"";
	int nLength = 0;

	int nIndex = syncFolders.Find(_T(";"));
	while(nIndex != -1)
	{
		// parse the sync folders which separated by character semicolon
		syncFd = syncFolders.Left(nIndex);
		CheckSyncDirectory(syncFd);

		if(syncFd != L"")
		{
			syncFdTemp = syncFd;
			syncFdTemp.MakeLower();

			// filter the syn folders. 
			BOOL bAdd = TRUE;
			pos = tempFolderList.GetHeadPosition();
			while(pos != NULL)
			{
				prepos = pos;
				strExistedFd = tempFolderList.GetNext(pos);
				
				strExistedFd.MakeLower();

				if(strExistedFd.Find(syncFdTemp) != -1)			// current folder covers previous one
				{
					tempFolderList.RemoveAt(prepos);
					bAdd = TRUE;
				}
				else if(syncFdTemp.Find(strExistedFd) != -1)	// the previous folder covers this one
				{
					bAdd = FALSE;
					break;
				}
				else
					bAdd = TRUE;
			}
			
			// add sync folder to vector
			if(bAdd)
			{
				tempFolderList.AddTail(syncFd);
			}

			// add application name to vector
			int pos = syncFd.Find(_T("\\"), 1);
			if(pos != -1)
			{
				CString appName = syncFd.Mid(1, pos-1);
				for(int i=0; i<g_Applications.size(); i++)
				{
					if(wcscmp(g_Applications[i].c_str(), LPCTSTR(appName)) == 0)
						break;
				}
				if(i == g_Applications.size())
				{
					g_Applications.push_back(LPCTSTR(appName));
				}
			}
		}

		nLength = syncFolders.GetLength();
		syncFolders = syncFolders.Right(nLength-nIndex-1);
		nIndex = syncFolders.Find(_T(";"));
	}

	// adding the sync folder global vector
	pos = tempFolderList.GetHeadPosition();
	while(pos != NULL)
	{
		strExistedFd = tempFolderList.GetNext(pos);
		g_SyncFolders.push_back(LPCTSTR(strExistedFd));
	}
	
	// if there is no avaliable data, just add root.
	if(g_SyncFolders.size() == 0)
	{
		g_SyncFolders.push_back(_T("\\"));
	}
}

void DBSyncServ::CheckSyncDirectory(IN OUT CString& syncDir)
{
	syncDir.TrimLeft();
	syncDir.TrimRight();

	if(syncDir == L"\\")
		return;

	if(syncDir.Left(1) != _T("\\"))
	{
		syncDir = L"\\" + syncDir;
	}

	if(syncDir.Right(1) != _T("\\"))
	{
		syncDir = syncDir + L"\\";
	}

//	int slashindex = syncDir.Find(_T("\\"), 1);
//	slashindex = syncDir.Find(_T("\\"), slashindex+1);
//
//	if(slashindex == -1)	// only has 2 '\', so must be an application path, ignore it
//	{
//		syncDir = L"\\";
//		return;
//	}

	return;
}

void DBSyncServ::CheckSyncDirectory(IN OUT WCHAR* rootdir)
{
	if(wcscmp(rootdir, _T("\\"))==0)
		return;

	CString rootStr = rootdir;

	if(rootStr.Left(1) != _T("\\"))	// does not start with '\'
	{
		wcscpy(rootdir, _T("\\"));
		return;
	}

	if(rootStr.Right(1) != _T("\\"))
	{
		wcscat(rootdir, _T("\\"));
		rootStr = rootdir;
	}

	int slashindex = rootStr.Find(_T("\\"), 1);
	slashindex = rootStr.Find(_T("\\"), slashindex+1);

	if(slashindex == -1)	// only has 2 '\', so must be an application path, ignore it
	{
		wcscpy(rootdir, _T("\\"));
		return;
	}

	return;
}

bool DBSyncServ::SaveLastStamp()
{
	if(!g_SupportUpdateStamp)
		return true;
	
	// copy global stamp to member var
	memcpy(m_LastUpdateStamp, g_LastUpdateStamp, sizeof(m_LastUpdateStamp));
	
	// save it to registry as a 16-characters string
	wchar_t	tmpStamp[17]=L"0000000000000000";
	swprintf(tmpStamp, L"%02X%02X%02X%02X%02X%02X%02X%02X", 
		m_LastUpdateStamp[0],
		m_LastUpdateStamp[1],
		m_LastUpdateStamp[2],
		m_LastUpdateStamp[3],
		m_LastUpdateStamp[4],
		m_LastUpdateStamp[5],
		m_LastUpdateStamp[6],
		m_LastUpdateStamp[7]);

	HRESULT ret = setConfigValue(_T("LastUpdateStamp"), tmpStamp, sizeof(tmpStamp), REG_SZ, true);
	return (S_OK == ret);
}
	
int  DBSyncServ::CompareStamp(IN IDSUPDATESTAMP s1, IN IDSUPDATESTAMP s2)
{
	DWORD s1_h, s1_l, s2_h, s2_l;
	s1_h = s1_l = s2_h = s2_l = 0;		// reset higher and lower part of both stamp

	s1_h = (s1[0]<<24) + (s1[1]<<16) + (s1[2]<<8) + (s1[3]);
	s1_l = (s1[4]<<24) + (s1[5]<<16) + (s1[6]<<8) + (s1[7]);
	s2_h = (s2[0]<<24) + (s2[1]<<16) + (s2[2]<<8) + (s2[3]);
	s2_l = (s2[4]<<24) + (s2[5]<<16) + (s2[6]<<8) + (s2[7]);

	if(s1_h>s2_h)		// s1 higher part greater
	{
		return INT_MAX;
	}
	else if(s1_h<s2_h)	// s2 higher part greater
	{
		return INT_MIN;
	}
	else if(s1_l>s2_l)	// s1 lower part greater
	{
		int nRet = (int)(s1_l-s2_l);
		return ((nRet>0)? nRet : INT_MAX);
	}
	else if(s1_l<s2_l)	// s2 lower part greater
	{
		int nRet = (int)(s1_l-s2_l);
		return ((nRet<0)? nRet : INT_MIN);
	}
	else				// exact the same value
	{
		return 0;
	}
}

// add callback to m_pCBThread's Queue
bool DBSyncServ::AddingCallbackToThread(DSCallBackBase* pCallback)
{
	if(pCallback == NULL || m_pCBThread == NULL)
		return false;

	return m_pCBThread->AddCallback(pCallback);
}

int  DBSyncServ::GetQueuedCallbackCount()
{
	return m_pCBThread->GetQueueSize();
}

void DBSyncServ::InitialFullSync()
{
	//
	// insert a FullSync message to the callback queue
	// 
	if (g_SyncAllAtBegin)
	{
		// remove all the existing callbacks
		m_pCBThread->RemoveAllCallback();
		
		(*gpDbSyncLog)(Log::L_INFO, L"Push (No.%d) full syncing to Callback Queue", g_dwFullSyncCount+1);
		
		FullSyncCallback* fullSync = new FullSyncCallback(g_dwFullSyncCount+1);
		
		AddingCallbackToThread(fullSync);
	}
}

void WINAPI DBSyncServ::MiniDumpCallback(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();

	(*gpDbSyncLog)(ZQ::common::Log::L_ERROR,  "Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
		ExceptionCode, ExceptionAddress, dwThreadID);

	(*gpDbSyncLog).flush();
}

void DBSyncServ::doIdle()
{
	g_ds.RemoveExpiredCACallback();
}