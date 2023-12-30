/*****************************************************************************
File Name:		DSInterface.cpp
Author:			Interactive ZQ 
Security:		<NVOD over iTV, Kunming>, Confidential
Description:	Implementation of Class DSInterface, which is used for synchronization 
                between ITV AM database and Local database
Modification Log:
When		Version		Who			What
-----------------------------------------------------------------------------
11/21/2002	    0		W.Z.	Original Program (Created)
 2/	8/2002		1		W.Z.	Modify to utilize transaction mechanism
 3/ 2/2004      2       Z.X.    Add support for synchronize user data in select
                                entry. Now only support price category.
								SaveEntryMDSelectToDB() changed.
								PrepareSelectionSQL() changed.
08/25/2004		3		Z.X.	Add logic to ignore metadata operation when value
                                is null and type is time
								
05/21/2006      4       Ken.Q	Version: 3.6.1
								Support to take IDS callback during the full synchronization, 
								these callbacks are stored into a queue for later processing 
								by anther thread.
								So, the original thread(TriggerWorker/TriggerWorkerMgr) are
								not necessary any more, whose purpose is avoiding 
								invoking IDS API in the callback. Process callback in a separated 
								thread cover the issue. 
								
08/03/2006      5       Ken.Q   Version 3.6.2
								Adding a switch to support in case of only syncing one folder, 
                                whether original folder itself is to linked to LAM Navigation Application.
								variable "g_SyncedFolderAsNavNode" working as switch flag, and two places 
								codes are changed regarding last version.
								1. In the UpdateAll(), to determine whether the synced folder is added to LAM hierarchy or not
								2. In AdjustRoot(), to determine whether the path in LAM includes the synced folder name
								3. Rollback to previous version with condition judgement in SaveEntriesToDB(), 
								   but this version strength the comparing condition cases to support the swith g_SyncedFolderAsNavNode,
								   coz it supports the synced folder itself not linked to LAM Navigation Application again.
                                
								Support LocalDB reconnection once it lost without restarting the service. 
								The modification is focusing on LocalDB.cpp on TraceDBError() function and the functions who invoke TraceDBError().

11/21/2006              Ken.Q   Version 3.6.6.1
                                To implement KDDI Reference Count and Rating Control feature, 
								assset metadata TitleSortName(MDUID:6) is added for identifying Reference Count,
								asset metadata rating(MDUID:21), folder metadata rating(MDUID:1512) are added for rating control. 
								The rating control purpose is to visible/invisible an asset. So in Hierarchy table, 
								an new column "isVisible" is introduced. 
								
								To make the implementation easier, 5 new store procedures are provided by Multiverse as following group:
								
								Group Vx
                                VA -	LAM_UPDATE_ASSET_VISIBILITY_FROMASSET (@localAssetUID Varchar( 20 ))
                                VB -	LAM_UPDATE_ASSET_VISIBILITY_FROMFOLDER (@localFolderUID Varchar( 20 ))

								Group Rx
								RA -	LAM_UPDATE_ASSET_REFERENCECOUNT_FROMASSETVALUE (@targetValue nvarchar( 500 ), @uniqueMDUID Int)
								RB -	LAM_UPDATE_ASSET_REFERENCECOUNT_FROMASSETID (@localAssetUID Varchar( 20 ), @uniqueMDUID Int)
                                RC -	LAM_UPDATE_ASSET_REFERENCECOUNT_FROMFOLDER (@localFolderUID Varchar( 20 ), @uniqueMDUID Int) 
								
		                        * RA sp is for case of Asset's TitleSortName change from 'A' to 'B', 
								calculating asset whose TitleSortName is 'A', so the @targetValue is 'A'

								Following is the table of cases that Rx and Vx sp MUST invoked, 
								in case of Vx and Rx are all must be invoked, Vx first, then Rx. 

																Rating control	          reference count
																--------------            ----------------
								Folder	 MD change Ispackage	VB	                      RC
									     MD change Rating	    VB                        RC

								Asset	 MD change TitleSortName	¡Á	                  RA RA (different targent) / RA RB
									     MD change Rating	    VA                        RB

								         Hierarchy Link	        VA                        RB
								         Hierarchy unlink	    ¡Á	                      RB  

									     Active	                VA                        RB
									     Inactive               VA                        RB

                                * Since Rating control and Reference count is only applying on active asset.
								  So, asset create/delete operation has been covered by Active/Inactive.
								* Hierarchy Link
								  We only process the case of one existed Asset was moved/copied to another folder.
								  To the new asset linking to hierarchy case, which is covered by Active case.
								* Hierarchy Unlink
								  1) It happens in case of moving an existed asset to another folder, 
								     that means, in the Hierarchy link case, have covered this case.
								  2) To the unlink but without linking, is to delete an existing hierarchy

12/26/2006              Ken.Q   Add feature to support manually sync specified folder, the key to this feature is DBSync 
                                must know which folders/assets are need to be deleted from LAM. So it must check ITV to see
								whether there is any link to folder/asset/atomicelement in ITV, etc. 
								The function is done by following routines:
								bool UpdateFolderSubNodeZQEntryState();
								bool CheckIDSObjExistence();
								bool RemoveHierarchyForManaulSync();
								bool DeleteAsset();
								bool DeleteFolder();

1/10/2007               Ken.Q   Change GetNewLocalUID() to move its internal Transcation DB connection to be class member variable, 
                                this will improve the sync speed, coz each time connection is time consumed, needs 1 or 2 seconds.

3/9/2007                Ken.Q   Adding parameter to SaveOneAsset/SaveElementsToDB/SaveObjectMdToDB/SaveAssetMdToDB/SaveElementMdToDB to 
								identify whether the asset is a new one or not. 
								The parameter are only take effective during DBSYnc starting. 
								To the case: The asset is in ITV AM, but is not at LAM, and this asset also is an OLD asset(its metadata
								timestamp is earlier than DBSysnc LastUpdateStamp), with original logic, these asset/element metadata will
								skipped and the result is that in LAM, the asset metadata is losted. With the parameter, to the new asset(not 
								existing in LAM), DBSync does not check its metadata and element metadata timestamp.
								This case could caused by: delete Asset from LAM, but LAM exported itv flat file did not got execution or execution failed.
								Then restart DBSync, this case happens. 
4/12/2007               Ken.Q   Changing original logic that IDS connection lost will trigger DBSync restart, 
                                from this version on (3.6.9.1), DBSync will reconnect IDS directly and then do Full Sync without restarting servcie.
								Change Points:
								1. Push FullSync to the Queue, then it process in the callback thread instead of original main thread.
								2. Original ConnChecker logic changing
9/19/2007               Ken.Q   Change the logic of delete EntryMDD > 2000, coz this will cause the MDUID did not match with previous after restart
                                To accomplish this, 
								1) No delete from EntryMDD > 2000 at the begining of UpdateAll
								2) Delete the useless EntryMDD after sync all
								3) In Metadata callback, if no such metadata, re-fetch it by SaveEntryMDDToDBEx() and SaveEntryMDSelectToDBEx()
*****************************************************************************/


//////////////////////////////////////////////////////////////////////
// Declaration of include files and definition of symbols
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DBSyncServ.h"
#include "DSInterface.h"
#include "sclog.h"
#include "strsafe.h"
#include "DBSAdiMan.h"
#include <string>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#define ISNETBREAK(X)  ((X == IDS_NETWORK_ERROR) || (X == ITV_TCP_STATE_ERROR))


#define ACTIVE_STATE		1

#define ELEMENT_CREATED		1
#define ELEMENT_CAPTURING	2
#define ELEMENT_CAPTUREFAIL	3
#define ELEMENT_CAPTURED	4
#define ELEMENT_UPLOADING	5
#define ELEMENT_UPLOADED	6
#define ELEMENT_DELETED		7
#define ELEMENT_UPLOADEDEXPIRED	8
#define ELEMENT_CREATEDEXPIRED	9

#define ASSET_STATE_ACTIVE     1
#define ASSET_STATE_INACTIVE   2
/*
 * UID of metadta
 */
#define MDUID_PLAYTIME		       1009     // Actual element playtime, unit is second
#define MDUID_PLAYFRACTION	       1010     // Actual element playfraction, its unit seems 1/10000 second
#define MDUID_ASSET_PRICE_CATEGORY   31     // Price category of asset
#define MDUID_ASSET_VCR_PRICE		617

// UID for KDDI Reference Count and Rating control
#define MDNAME_ASSET_TITLESORTNAME     _T("TitleSortName")   // TitleSortName metadata to calculate reference count
#define MDNAME_RATING                 _T("Rating")         // metadata Rating for Folder and Asset
#define MDUID_FOLDER_RATING           21    // Folder Rating, MDUID is hardcoded
#define MDUID_ASSET_RATING            1512  // Asset Rating, MDUID is hardcoded


#define IDS_METADATA        0
#define IDS_HIERARCHY		9
#define IDS_WORKQUEUE		10
#define IDS_PACKAGE			11
extern DWORD g_playInterval;			//	Global float of element play interval
extern TCHAR g_IZQDBType[MAXNAME];		//	Global string of ODBC data source database type

extern DWORD g_SyncAllAtBegin;

#define WORKQUEUE_ADD							1
#define WORKQUEUE_DELETE						2
#define	WORKQUEUE_LINK							3
#define WORKQUEUE_UNLINK						4
#define WORKQUEUE_RENAME						5
#define WORKQUEUE_UPDATE						6

#pragma  comment(lib,"idsapi" VODLIBEXT)
#pragma warning(disable : 4786)
#define MAXTHREADCOUNT							1

#define STAMPDIFF_THRESHOLD						10

#define DISORDER_CB_KEEP_TIMES                  10
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



ITVVERSION		CDSInterface::m_itvver;
IDSSESS			CDSInterface::m_idssess;
ITVSTATUSBLOCK	CDSInterface::m_itvsta;
//DWORD CDSInterface::m_ThreadLock = 0;
extern DWORD gUpdateThreadId;
//CList<DWORD, DWORD>	CDSInterface::m_busyAssets(sizeof(DWORD));
extern ZQ::common::Log * gpDbSyncLog;
using namespace ZQ::common;
ZQ::common::Mutex CDSInterface::m_WQLock;
WORKQUEUE CDSInterface::m_WorkQueue;
bool CDSInterface::m_bIsActive;
extern TCHAR g_PrepareStr[16];
ZQ::common::Mutex CDSInterface::m_AssetMDTableLock;

ZQ::common::Mutex CDSInterface::m_AssetTableLock;
ZQ::common::Mutex CDSInterface::m_AtomicElementTableLock;

// global add-in manager
extern DBSAdiMan g_AdiMan;

// global service object
extern DBSyncServ g_server;

// TitleSortNameMDUID's default value is 6
int CDSInterface::m_titleSortNameMDUID = 6;
int CDSInterface::m_folderRatingMDUID = 1512;
int CDSInterface::m_assetRatingMDUID = 21;

std::map<CString, DWORD> CDSInterface::m_activeElementMD;

LocalDB CDSInterface::m_transactionDB;

std::map<DWORD, DSCallBackCAsset*> CDSInterface::m_disOrderCBs;

std::map<DWORD, DWORD>CDSInterface::m_notFoundAssetId;
std::map<DWORD, DWORD>CDSInterface::m_notFoundElementId;

bool CDSInterface::m_isFullSyncing = false;

DWORD g_dwFullSyncCount = 0;    // working as a counter of full sync times

DWORD CDSInterface::m_totalIgnoredCount = 0;

extern DWORD g_SupportUpdateStamp;

/*****************************************************************************
Function Name:	CDSInterface
Arguments:		None
Returns:		None
Description:	Construction of CDSInterface objects.
*****************************************************************************/
CDSInterface::CDSInterface()
{
	m_dwSiteTriggerUid = 0;
	m_dwApplicationTriggerUid = 0;
	m_dwFolderTriggerUid = 0;
	m_dwAssetTriggerUid = 0;
	m_dwElementTriggerUid = 0;
	m_dwCaTriggerUid = 0;
	m_dwElementStTriggerUid = 0;
	m_dwAssetStTriggerUid = 0;

	m_bIsActive=false;
}

/*****************************************************************************
Function Name:	~CDSInterface
Arguments:		None
Returns:		None
Description:	Destruction of CDSInterface objects.
*****************************************************************************/
CDSInterface::~CDSInterface()
{	

}


//////////////////////////////////////////////////////////////////////
// Declaration of Global Variables and Functions
//////////////////////////////////////////////////////////////////////

extern WCHAR g_ITVServerIP[MAXNAME];	//	Global string of IP of ITV server
extern WCHAR g_ITVUserName[MAXNAME];	//	Global string of user name to log onto ITV server
extern TCHAR g_IZQDSNName[MAXNAME];		//	Global string of data source string of ODBC connection with Local DB
extern TCHAR g_IZQUserName[MAXNAME];	//	Global string of user name to open ODBC connection with Local DB
extern TCHAR g_IZQPassword[MAXNAME];	//	Global string of password to open ODBC connection with Local DB
extern WCHAR g_SyncDirectory[MAXNAME*2];	// Global string of DBSync synchronizing root

extern std::vector<std::wstring> g_SyncFolders;	// Global vector for multiple sync folders
extern std::vector<std::wstring> g_Applications; // Global vector for the applications that the synced folder belong to 
extern DWORD    g_SyncedFolderAsNavNode;// it is only effective when the synced folder count is 1
									    // if it is 0 and only sync one folder, the asset/subfolder under the synced folder in ITV will linked to LAM Application directly
	                                    // if it is 1 and only sync one folder, the synced folder will also as a folder under LAM Nav Application

extern DWORD g_TimeWindowThreshold;	
extern DWORD g_SupportNavigation;		//	Global configure of support NAV database or not.
extern IDSUIDUPDATESTAMP g_LastUpdateStamp;	// Global update stamp, 8 bytes
IDSUIDUPDATESTAMP	g_CurrUpdateStamp;	// Global temp update stamp for starting use, 8 bytes
extern DWORD g_SupportIngestManager;
extern DWORD g_SupportProcedure;

extern DWORD g_SupportRefcountRatingctrl; // Global configure of support KDDI refcount and rating control


#define		TIMEWINDOW_COUNT	3
std::wstring g_TimeWindowList[TIMEWINDOW_COUNT] = 
{
	L"ActivateTime",
	L"DeactivateTime",
	L"DeleteTime"
};

extern CString g_ModulePath;			//	Global string of work path of the module

extern LocalDB g_localDB;	//	Global local DB operation object

extern ZQ::common::Log* gpDbSyncLog;
//////////////////////////////////////////////////////////////////////
// Implementation of Methods
//////////////////////////////////////////////////////////////////////


/*****************************************************************************
Function Name:	Initialize
Arguments:		None
Returns:		res (bool) (true:success false:fail)
Description:	This function initializes the IDS connection 
                and register trigger.
*****************************************************************************/
bool CDSInterface::Initialize()  
{
	DWORD ret = 0;
	// do until IDS connection succeed
	while(!g_server.IsShutingDown())
	{
		try
		{
			ret= InitIDS();
		}
		catch (...) 
		{
			// in the exception case, DBSync must restart and to init IDS context, then InitDS could succeed

			(*gpDbSyncLog)(Log::L_WARNING,_T("Exception catched when init IDS. Restart DBSync itself now."));	

			// add by Ken on 2007-3-20, Uninitialize IDS before restart service, 
			// coz occasionally, the UninitDS() needs long time. 
			Uninitialize();

			// Modified by Ken on 2006-12-14, when met exception, restart DBSync directly 
			HANDLE handle = OpenEvent(EVENT_ALL_ACCESS,false,_T("DBSYNCAPP_Stop"));
			SetEvent(handle);
			
			return false;
		}
		
		// retry if not succeed
		if(ret != 0)
		{
			(*gpDbSyncLog)(Log::L_ERROR,_T("Initialize IDS connection ERROR! Reconnect it 5 seconds later"));	
			::Sleep(5000);
			
			continue;
		}	
			
		(*gpDbSyncLog)(Log::L_INFO,_T("InitIDS() OK!"));

		//	Register all IDS triggers			
		if(RegisterTrigger() != 0)
		{
			(*gpDbSyncLog)(Log::L_ERROR,_T("RegisterTrigger ERROR! Going to Uninit IDS connection"));
			
			//	Uninitialize IDS connection
			Uninitialize();
			
			continue;
		}

		(*gpDbSyncLog)(Log::L_INFO,_T("RegisterTrigger OK!"));

		return true;
	} 
	return false;
}

/*****************************************************************************
Function Name:	Uninitialize
Arguments:		None
Returns:		void
Description:	This function un-init the IDS connection 
                and register trigger.
*****************************************************************************/
void CDSInterface::Uninitialize()
{
	(*gpDbSyncLog)(Log::L_DEBUG,_T("DeregiserTrigger..."));
	DeregiserTrigger();

	UnInitIDS();
}


/*****************************************************************************
Function Name:	InitIDS
Arguments:		None
Returns:		res (DWORD) (0:success !0:fail)
Description:	This function initializes the DS connection with ITV.
*****************************************************************************/
DWORD CDSInterface::InitIDS()
{
	DWORD res = 0;	//return code

	//  Set version info
	m_itvver.VersionComponents.byteMajor = 1;
	m_itvver.VersionComponents.byteMinor = 0;

	//	Initializes the DS connection with ITV
	(*gpDbSyncLog)(Log::L_DEBUG,_T("Began IdsInitialize()..."));

	res = IdsInitialize(&m_itvver, NULL);
	if (res != 0)
	{
		(*gpDbSyncLog)(Log::L_ERROR,_T("IdsInitialize Error "));
		
		return res;
	}
	
	(*gpDbSyncLog)(Log::L_DEBUG,_T("Began IdsBind()..."));
	res = IdsBind(g_ITVServerIP, g_ITVUserName,	IDS_READ_ONLY, 0, &m_idssess, &m_itvsta, 0,	NetBreakCallBack);
	if (res != 0)
	{
		(*gpDbSyncLog)(Log::L_ERROR,_T("IdsBind Error with return code as 0X%X"),res);
	
		return res;
	}

	// Do not set the limitation
//	(*gpDbSyncLog)(Log::L_DEBUG,_T("Began IdsSetOptions()"));
//	res = IdsSetOptions(&m_idssess, IDS_OPT_ENTRYLIMIT, 1024);
//	if (res)	//	error occurs
//	{
//		//	Log error message
//		
//		(*gpDbSyncLog)(Log::L_ERROR,_T("IdsSetOptions Error"));
//	
//		return res;	//	Exit
//	}

	m_bIsActive = true;
	(*gpDbSyncLog)(Log::L_INFO,_T("InitDS done."));
	return res;
}

/*****************************************************************************
Function Name:	UnInitIDS
Arguments:		None
Returns:		res (DWORD) (0:success !0:fail)
Description:	This function closes the DS connection with ITV.
*****************************************************************************/
DWORD CDSInterface::UnInitIDS()
{
	if(!m_bIsActive)
		return 0;
	
	DWORD res = 0;	//	return code

	//	Close the DS connection with ITV
	res = IdsUnbind(&m_idssess, IDS_ABORT, &m_itvsta, 0);
	if (res != 0)	
	{
		(*gpDbSyncLog)(Log::L_ERROR,_T("IdsUnbind Error"));
	}
	res = IdsUninitialize();
	if (res != 0)	
	{
		(*gpDbSyncLog)(Log::L_ERROR,_T("IdsUninitialize ERROR!"));
	}

	m_bIsActive = false;

	(*gpDbSyncLog)(Log::L_INFO,_T("UnInitDS done."));
	return res;
}

DWORD CDSInterface::InitDB()
{
	DWORD res = 0;	//return code

	//	Open connection for Local DB
	(*gpDbSyncLog)(Log::L_DEBUG,_T("Began Open local DB"));
	res = g_localDB.OpenDB(SQL_AUTOCOMMIT_ON);
	if (res)	//	error occurs
	{
		(*gpDbSyncLog)(Log::L_ERROR,_T("OpenLocalDB Error."));	
	}
	else
	{
		// open transaction database connection
		res = m_transactionDB.OpenDB(SQL_AUTOCOMMIT_OFF);
		
		(*gpDbSyncLog)(Log::L_DEBUG,_T("OpenLocalDB Success."));
	}

	return res;
}

DWORD CDSInterface::UninitDB()
{
	if(0 == g_localDB.IsConnected())
	{
		g_localDB.CloseDB();
		m_transactionDB.CloseDB();
	}

	return 0;
}

/*****************************************************************************
Function Name:	CheckSyncFoldersExistence
Arguments:		None
bool:		res (bool) (true:success false:fail)
Description:	This function verify whether all the sync folders are existed or not
*****************************************************************************/
bool  CDSInterface::CheckSyncFoldersExistence()
{
	bool bRet = true;
	WCHAR syncFolderName[MAXNAMELEN*2];	
	DWORD entryHUID, entryType; 
	
	int nSyncLoop = 0;
	for(nSyncLoop=0; nSyncLoop<g_SyncFolders.size(); nSyncLoop++)
	{
		// get the sync folder name
		StringCbCopy(syncFolderName, MAXNAMELEN*2, g_SyncFolders.at(nSyncLoop).c_str());

		int nObjCount = GetEntryHUIDByName(syncFolderName, entryHUID, entryType);
		if(0 == nObjCount)
		{
			(*gpDbSyncLog)(Log::L_ERROR,_T("Specified Synchronizing Directory \"%s\" does not exist! The folder name is character sensitive, please check configuration"),syncFolderName);
			bRet = false;
		}
		else if(nObjCount > 1)
		{
			(*gpDbSyncLog)(Log::L_ERROR,_T("Specified Synchronizing Directory \"%s\" have multiple objects in IDS Hierarchy. Please remove duty data first"),syncFolderName);
			bRet = false;
		}
	}
	return bRet;
}

/*****************************************************************************
Function Name:	RegisterTrigger
Arguments:		None
Returns:		res (DWORD) (0:success !0:fail)
Description:	This function register all needed DS triggers to ITV.
*****************************************************************************/
DWORD CDSInterface::RegisterTrigger()
{
	DWORD res = 0;

////Registering FolderTriggers
	DWORD dwAppUID = 0; // uid of SeaChange Movies On Demand
	APPNAME *pApps = 0;	//	structure to store application info
	DWORD appNum = 0;		//	Number of applications
	//	Get number of applications and all application info
	res = IdsListApplications(&m_idssess, &pApps, &appNum, &m_itvsta, 0);
	if (res)	//	error occurs
	{
		//	Log error message
		(*gpDbSyncLog)( Log::L_ERROR,_T("IdsListApplications ERROR!"));
	
		return res;	//	Exit
	}

//	---------- modified by KenQ 2006-05-18-------------
//	Support multiple folder callback trigger
//  Get AppUID of sync folders and save to vector
	WCHAR syncFolderName[MAXNAMELEN*2];		
	int nSyncLoop = 0;

	std::vector<DWORD> vAppUIDs;
	for(nSyncLoop=0; nSyncLoop<g_SyncFolders.size(); nSyncLoop++)
	{
		// get the sync folder name
		StringCbCopy(syncFolderName, MAXNAMELEN*2, g_SyncFolders.at(nSyncLoop).c_str());
	
		CString strApp = syncFolderName;
		DWORD dwAppUID = 0;
		int appIndex = strApp.Find(_T('\\'), 1);
		if(appIndex!=-1)
		{
			strApp = strApp.Left(appIndex);
			strApp = strApp.Right(strApp.GetLength()-1);
		}
		else
		{
			strApp = "";
			
			// the synced folder is root, so sync all application
			for (int i=0; i<appNum; i++)
			{
				if((pApps+i)->dwUid != 0)
				{
					vAppUIDs.push_back((pApps+i)->dwUid);		
				}
			}
			break; // No additional process anymore
		}
		
		for (int i=0; i<appNum; i++)		//	For each application
		{
			if(strApp == (pApps+i)->wszName)
			{
				dwAppUID = (pApps+i)->dwUid;			
			}
		}
		bool bAdd = true;
		for(int j=0; j<vAppUIDs.size(); j++)
		{
			if(vAppUIDs.at(j) == dwAppUID)
			{
				bAdd = false;
				break;
			}
		}
		if(bAdd)
		{
			vAppUIDs.push_back(dwAppUID);
		}
	}
// following block is commented by KenQ, cosz it is replace by above block.
	
//	for (DWORD i=0; i<appNum; i++)	//	for each application
//	{
//		WCHAR appName[MAX_SQL_CHAR];	//	name of application
//		// Added by kaliven lee 2005-2-21
//		// because IdsRegisterMddTrigger must specify the application uid
//		// I must write hard code to get the application UID of Moive On Demand.
//		// idiot ids
//		if(wcsicmp((pApps+i)->wszName,_T("SeaChange Movies On Demand"))==0)
//			dwAppUID = (pApps+i)->dwUid;
//		//	Insert a '\' before the application name
//
//		StringCbCopyW(appName, MAX_SQL_CHAR, _T("\\"));
//		StringCbCatW(appName, MAX_SQL_CHAR, _T("\\"));
//	}

//	---------- end of block modified by KenQ 2006-05-18-------------
	
//	---------- modified by Bernie Zhao 2005-07-18-------------
//	 user an extra configuration SyncDirecotry to identify the DBSync synchronizing root

//	---------- modified by KenQ 2006-05-18-------------
//	 Support multiple folder callback trigger

//	res = IdsRegisterFolderTrigger(&m_idssess, _T("\\"), FolderCallBack,
//									   &m_dwFolderTriggerUid, &m_itvsta, 0);

	for(nSyncLoop=0; nSyncLoop<g_SyncFolders.size(); nSyncLoop++)
	{
		// get the sync folder name
		StringCbCopy(syncFolderName, MAXNAMELEN*2, g_SyncFolders.at(nSyncLoop).c_str());

		WCHAR syncdir[MAXNAME*2]={0};
		wcsncpy(syncdir, syncFolderName, wcslen(syncFolderName)-1);
		res = IdsRegisterFolderTrigger(&m_idssess, syncdir, FolderCallBack,
										   &m_dwFolderTriggerUid, &m_itvsta, 0);
	}
	
//	----------------------------------------------------------

////Registering CaTrigger
	res = IdsRegisterCaTrigger(&m_idssess, 0, CaCallBack, &m_dwCaTriggerUid, &m_itvsta, 0);

////Registering StateTriggers
	
	//	Register StateTrigger for Asset
	res = IdsRegisterStateTriggerEx(&m_idssess, IDS_ASSET, 0, 0, AssetStateCallBack,
								    &m_dwAssetStTriggerUid, &m_itvsta, 0);
	//	Register StateTrigger for Element
	res = IdsRegisterStateTriggerEx(&m_idssess, IDS_ATOMIC_ELEMENT, 0, 0, ElementStateCallBack,
								    &m_dwElementStTriggerUid, &m_itvsta, 0);

	//Registering MDD Trigger

	

	//Registering MdTriggers

	//	Prepare a structure contain query info which will be used to get metadata info
	METADATA *inMd = new METADATA;		//	pointer to the structure containing query info
	memset(inMd, 0, sizeof(METADATA));
	memcpy(&(inMd->Version), &m_itvver, sizeof(ITVVERSION));
	WCHAR wszMdName[MAXNAME] = L"";
	memcpy(inMd->wszMdName, L"*", sizeof(L"*"));


	// Added by kaliven lee 2005-2-21
	// because IdsRegisterMddTrigger do not support wildcard
	// stupid IDS
//	if(g_SupportNavigation)
//	{
	
		SQLTCHAR sql[1024] = _T("");
		DBRECORD selectCol;
		DBFIELD field;
		DBRESULTSETS resultSets;
		
		field.lSize = MAXNAME;
		field.lType = SQL_C_TCHAR;
		selectCol.push_back(field);

		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDNAME FROM MDLIST"));
		g_localDB.SelectAllSQL(sql,selectCol,&resultSets);
		
		for(DWORD j = 0; j < resultSets.size(); j++)
		{
			// ---------------- Modified by KenQ on 2006-5-18
			DBRECORD record = resultSets.at(j);
			StringCbCopyW(wszMdName, MAXNAME*2, (wchar_t*)record.at(0).pValue);
			for(int i=0; i<vAppUIDs.size(); i++)
			{
				dwAppUID = vAppUIDs.at(i);
				res = IdsRegisterMddTrigger(&m_idssess,IDS_FOLDER,wszMdName,dwAppUID,MDDCallBack,&m_dwMddTriggerUid,&m_itvsta,0);
				res = IdsRegisterMddTrigger(&m_idssess,IDS_ASSET,wszMdName,dwAppUID,MDDCallBack,&m_dwMddTriggerUid,&m_itvsta,0);
			}
			res = IdsRegisterMddTrigger(&m_idssess,IDS_FOLDER,wszMdName,0,MDDCallBack,&m_dwMddTriggerUid,&m_itvsta,0);
			res = IdsRegisterMddTrigger(&m_idssess,IDS_ASSET,wszMdName,0,MDDCallBack,&m_dwMddTriggerUid,&m_itvsta,0);
			// ---------- end of modification by KenQ on 2006-5-18
		}
		g_localDB.FreeResultSets(&resultSets);
// 	}

	// Add by KenQ 2008-7-29, only register the defined element metadata
	// get the active element metadata, and only register them
	//
	m_activeElementMD.clear();

	TCHAR objname[256];
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT name FROM sysobjects WHERE id = object_id('dbo.ActiveMetadata') AND sysstat & 0xf = 3"));
	RETCODE retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, &objname, sizeof(objname));
	
	if (retCode == SQL_SUCCESS)
	{
		(*gpDbSyncLog)(Log::L_DEBUG, _T("ActiveMetadata table exist, only metadata defined in this table are synced"));

		DBRECORD selectCol;
		DBFIELD field;
		DBRESULTSETS resultSets;
		
		field.lSize = MAXNAME;
		field.lType = SQL_C_TCHAR;
		selectCol.push_back(field);

		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT Name FROM ActiveMetadata WHERE EntryType=%hu AND Registered=1"), IDS_ATOMIC_ELEMENT);
		retCode = g_localDB.SelectAllSQL(sql,selectCol,&resultSets);
		
		WCHAR wszMdName[MAXNAME] = L"";
		for(DWORD i = 0; i < resultSets.size(); i ++)
		{
			DBRECORD record = resultSets.at(i);
			StringCbCopyW(wszMdName, MAXNAME*2, (wchar_t*)record.at(0).pValue);

			m_activeElementMD.insert(ActiveMetadata::value_type(CString(wszMdName), 0));
		}
		g_localDB.FreeResultSets(&resultSets);

		// make sure PlayTime and PlayTimeFraction are in the map
		if(m_activeElementMD.size() > 0)
		{
			m_activeElementMD.insert(ActiveMetadata::value_type(CString(L"PlayTime"), 0));
			m_activeElementMD.insert(ActiveMetadata::value_type(CString(L"PlayTimeFraction"), 0));
		}
	}
	else
	{
		(*gpDbSyncLog)(Log::L_DEBUG, _T("ActiveMetadata table does not exist!"));
	}

	//	Register MdTrigger for Site
	res = IdsRegisterMdTrigger(&m_idssess, IDS_SITE, 0, inMd, SiteMDCallBack,
							   &m_dwSiteTriggerUid, &m_itvsta, 0);
	//	Register MdTrigger for Application
	res = IdsRegisterMdTrigger(&m_idssess, IDS_APPLICATION, 0, inMd, ApplicationMDCallBack,
							   &m_dwApplicationTriggerUid, &m_itvsta, 0);
	//	Register MdTrigger for Folder
	res = IdsRegisterMdTrigger(&m_idssess, IDS_FOLDER, 0, inMd, FolderMDCallBack,
							   &m_dwFolderTriggerUid, &m_itvsta, 0);
	//	Register MdTrigger for Asset
	res = IdsRegisterMdTrigger(&m_idssess, IDS_ASSET, 0, inMd, AssetMDCallBack,
							   &m_dwAssetTriggerUid, &m_itvsta, 0);
	//	Register MdTrigger for Element
	if(m_activeElementMD.size() == 0)
	{
		res = IdsRegisterMdTrigger(&m_idssess, IDS_ATOMIC_ELEMENT, 0, inMd, ElementMDCallBack,
								   &m_dwElementTriggerUid, &m_itvsta, 0);
	}
	else
	{
		METADATA *aeMD = new METADATA;		//	pointer to the structure containing query info
		memset(aeMD, 0, sizeof(METADATA));
		memcpy(&(aeMD->Version), &m_itvver, sizeof(ITVVERSION));

		ActiveMetadata::iterator it = m_activeElementMD.begin();
		while(it != m_activeElementMD.end())
		{
			StringCbCopyW(aeMD->wszMdName, MAXNAME*2, (LPCTSTR)it->first);
			
			(*gpDbSyncLog)(Log::L_DEBUG, _T("Register AtomicElement metadata %s"), aeMD->wszMdName);

			res = IdsRegisterMdTrigger(&m_idssess, IDS_ATOMIC_ELEMENT, 0, aeMD, ElementMDCallBack,
									   &m_dwElementTriggerUid, &m_itvsta, 0);

			it++;
		}
		delete aeMD;
	}
	//	Register MdTrigger for Clip
	res = IdsRegisterMdTrigger(&m_idssess, IDS_CLIP, 0, inMd, ClipMDCallBack,
						   &m_dwClipTriggerUid, &m_itvsta, 0);

	delete inMd;	//	Free memory

	IdsFree(pApps);
	
	return 0;	//	success
}


/*****************************************************************************
Function Name:	DeregisterTrigger
Arguments:		None
Returns:		res (DWORD) (0:success !0:fail)
Description:	This function deregister all registered DS triggers to ITV.
*****************************************************************************/
DWORD CDSInterface::DeregiserTrigger()
{
	//	Deregister Site trigger if registered
	if (m_dwSiteTriggerUid)
	{
		IdsDeregisterTrigger(&m_idssess, m_dwSiteTriggerUid, &m_itvsta, 0);
		m_dwSiteTriggerUid = 0;
	}
	//	Deregister Application trigger if registered
	if (m_dwApplicationTriggerUid)
	{
		IdsDeregisterTrigger(&m_idssess, m_dwApplicationTriggerUid, &m_itvsta, 0);
		m_dwApplicationTriggerUid = 0;
	}
	//	Deregister Folder trigger if registered
	if (m_dwFolderTriggerUid)
	{
		IdsDeregisterTrigger(&m_idssess, m_dwFolderTriggerUid, &m_itvsta, 0);
		m_dwFolderTriggerUid = 0;
	}
	//	Deregister Asset trigger if registered
	if (m_dwAssetTriggerUid)
	{
		IdsDeregisterTrigger(&m_idssess, m_dwAssetTriggerUid, &m_itvsta, 0);
		m_dwAssetTriggerUid = 0;
	}
	//	Deregister Element trigger if registered
	if (m_dwElementTriggerUid)
	{
		IdsDeregisterTrigger(&m_idssess, m_dwElementTriggerUid, &m_itvsta, 0);
		m_dwElementTriggerUid = 0;
	}
	//	Deregister Clip trigger if registered
	if (m_dwClipTriggerUid)
	{
		IdsDeregisterTrigger(&m_idssess, m_dwClipTriggerUid, &m_itvsta, 0);	
		m_dwClipTriggerUid = 0;
	}
	//	Deregister Complex-asset trigger if registered
	if (m_dwCaTriggerUid)
	{
		IdsDeregisterTrigger(&m_idssess, m_dwCaTriggerUid, &m_itvsta, 0);
		m_dwCaTriggerUid = 0;
	}
	//	Deregister Asset state trigger if registered
	if (m_dwAssetStTriggerUid)
	{
		IdsDeregisterTrigger(&m_idssess, m_dwAssetStTriggerUid, &m_itvsta, 0);
		m_dwAssetStTriggerUid = 0;
	}
	//	Deregister Element state trigger if registered
	if (m_dwElementStTriggerUid)
	{
		IdsDeregisterTrigger(&m_idssess, m_dwElementStTriggerUid, &m_itvsta, 0);	
		m_dwElementStTriggerUid = 0;
	}
	
	return 0;	//	success
}


/*****************************************************************************
Function Name:	FolderCallBackProcess
Arguments:		Trigger ID (DWORD), Entry Info (ENTY), Operation Type (WORD)
Returns:		None
Description:	This function is called by ITV trigger when hierarchy is changed. 
				It gets the the structure of the changed entry, and update Local
				DB according to:
				1. If the change is to add, pass to SaveOneHierarchy to save this
				   hierarchy into HIERARCHY table, and pass to SaveOneFolder or
				   SaveOneAsset to save this entry into FOLDER or ASSET respectively
				   according to the type of the entry;
				2. If the change is to delete, delete this hierarchy from HIERARCHY
				   table, and if this entry is a folder, delete it from FOLDER table;
				3. If the change is to replace or put, update HIERARCHY table using
				   Entry Info, and update FOLDER or ASSET respectively according to 
				   the type of the entry.
Called By:		(ITV callback)
*****************************************************************************/
void CDSInterface::FolderCallBackProcess(DWORD dwTriggerUid, ENTRY *pFolderEntry, WORD wOp)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR     sql[1024];	//	string to store SQL statement
	CString		entryParentName;	// for DBSA judge the type of this entry


    (*gpDbSyncLog)(Log::L_INFO,_T("%d:Entering FolderCallBackProcess:  wszName = %ls, AssetID = %ld, wOp = %d"),
		 GetCurrentThreadId(), pFolderEntry->wszName, pFolderEntry->dwAssetUid, wOp);
	try{
	
	if (wOp == IDS_DELETE)	//	operation is to delete
	{
		//	Delete the hierarchy from HIERARCHY table
			// process application
		CString csName;
		AdjustRoot(pFolderEntry->wszName, csName);
		csName.Replace(L"'", L"''");
		int nIndex = csName.ReverseFind(_T('\\'));
		CString csParentName = csName.Left(nIndex);
		csName = csName.Right(csName.GetLength() - nIndex - 1);

		entryParentName = csParentName;
		
		if(csParentName == _T(""))// application
		{
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("DELETE FROM APPLICATIONMD WHERE APPLICATIONUID IN (SELECT APPLICATIONUID FROM APPLICATION WHERE NAME = N'%s')"),
				csName);
			g_localDB.ExecuteSQL(sql);

			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("DELETE FROM APPLICATION WHERE NAME = N'%s'"),
				csName);
			g_localDB.ExecuteSQL(sql);

			// delete hierarchy
//			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM HIERARCHY WHERE HIERARCHYUID = %ld"),
//				 pFolderEntry->dwHierarchyUid);

			// modified at 2007-3-2 by Ken Qian, coz in SaveAppsToDB(), we could not get the Application hierarchy UID, 
			// so in Hierarchy Table, we use application id as the hierarchy uid. 
			// Here in pFolderEntry, the dwAppUid is for folder/asset, but to Application deletion, its value aways 0,
			// so the only choice is to delete with EntryName.
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM HIERARCHY WHERE EntryType = %d AND EntryName = N'%s' "),
				 IDS_APPLICATION, csName);

			retCode = g_localDB.ExecuteSQL(sql);
		}
		
		else 
		if (pFolderEntry->dwAssetUid==0)		//It's a folder or application
		{

			//	Delete the metadata of the folder from FOLDERMD table
			WCHAR LocalFolderUID[64] = _T("");
			
			
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALFOLDERUID FROM FOLDER WHERE FOLDERUID = %ld"),
					 pFolderEntry->dwHierarchyUid);
			retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,LocalFolderUID,sizeof(LocalFolderUID));
			if(IsPackaged(LocalFolderUID))
			{				
				SaveMsgtoWorkQueue(LocalFolderUID,L"",IDS_PACKAGE,WORKQUEUE_DELETE,L"",L"");
			}

			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM FOLDERMD WHERE LOCALFOLDERUID = '%s'"),
					 LocalFolderUID);
			retCode = g_localDB.ExecuteSQL(sql);

			//	Delete the folder from FOLDER table
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM FOLDER WHERE FOLDERUID = %ld"),
					 pFolderEntry->dwHierarchyUid);
			retCode = g_localDB.ExecuteSQL(sql);

			// delete hierarchy
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM HIERARCHY WHERE HIERARCHYUID = %ld"),
				 pFolderEntry->dwHierarchyUid);
			retCode = g_localDB.ExecuteSQL(sql);
		}
		else	//	It's an asset
		{
			bool bLinkedByOthers = false;
			SQLTCHAR oldTSN[1024] = _T("");

			// get local entry uid of asset
			SQLTCHAR assetLUID[128] = _T("");	//	Local Asset UID

			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALENTRYUID FROM HIERARCHY WHERE HIERARCHYUID = %ld AND ENTRYTYPE = %hu"),
					 pFolderEntry->dwHierarchyUid, IDS_ASSET);
			retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, assetLUID, sizeof(assetLUID));

			DWORD assetUID = 0;	//	UID of the asset
			//	Check whether the asset is linked by other hierarchy
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ENTRYUID FROM HIERARCHY WHERE ENTRYTYPE = %hu AND ENTRYUID = %ld AND NOT HIERARCHYUID = %ld"),
					 IDS_ASSET, pFolderEntry->dwAssetUid,pFolderEntry->dwHierarchyUid);
			retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &assetUID, 0);
			if (retCode == SQL_NO_DATA)	//	not linked by other hierarchy
			{
				//	Check whether the asset is linked by other asset
				WCHAR LocalAssetUID[64];
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALASSETUID FROM ASSET WHERE ASSETUID = %ld"),
						 pFolderEntry->dwAssetUid);
				retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, LocalAssetUID, sizeof(LocalAssetUID));

				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALMEMBERUID FROM COMPLEXASSET WHERE MEMBERTYPE = %hu AND LOCALMEMBERUID = '%s'"),
						 IDS_ASSET, LocalAssetUID);
				retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, LocalAssetUID, sizeof(LocalAssetUID));
				if (retCode == SQL_NO_DATA)	//	not linked by other asset
				{
					// this asset was not linked by others, the asset itself will be deleted.
					bLinkedByOthers = false;

					// get the titleSortName
					GetAssetTitleSortName(assetLUID, oldTSN, 1000);

/*	
					// 2008-8-11 by Ken Q
					// move following block codes to AssetStateCallBackProcess() in case of wOP=3(IDS_DELETE) 
					// to fix bug 7728: asset's LocalAssetUID will change after move asset to another folder in ITV AM
					//

					//	Delete the member relationship of the asset to delete
					SaveOneComplexAsset(pFolderEntry->dwAssetUid, 0, NULL);

					//	Delete the metadata of the asset from ASSETMD table
					
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM ASSETMD WHERE LOCALASSETUID = '%s'"),
							 LocalAssetUID);
					retCode = g_localDB.ExecuteSQL(sql);

					//	Delete the asset from ASSET table

					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM ASSET WHERE ASSETUID = %ld"),
							 pFolderEntry->dwAssetUid);
					(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);
					retCode = g_localDB.ExecuteSQL(sql);
					if(retCode == SQL_SUCCESS)
						SaveMsgtoWorkQueue(LocalAssetUID,L"",IDS_ASSET,WORKQUEUE_DELETE,L"",L"");
*/
				}else if(retCode == SQL_SUCCESS)
				{
					// this asset was linked by others
					bLinkedByOthers = true;

					WCHAR ParentLHUID[64];
				
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALPARENTHUID FROM HIERARCHY WHERE HIERARCHYUID = %ld AND ENTRYTYPE= %ld"),
						 pFolderEntry->dwHierarchyUid,IDS_ASSET);
					retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,ParentLHUID,sizeof(ParentLHUID));

					SaveMsgtoWorkQueue(LocalAssetUID,ParentLHUID,IDS_ASSET,WORKQUEUE_UNLINK,L"",L"");
				}
			}
			else if(retCode == SQL_SUCCESS)
			{				
				// this asset is still linked to other by others. 
				bLinkedByOthers = true;

				WCHAR ParentLHUID[64];
				WCHAR LocalAssetUID[64];
					
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALASSETUID FROM ASSET WHERE ASSETUID = %ld"),
					 pFolderEntry->dwAssetUid);
				retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, LocalAssetUID, sizeof(LocalAssetUID));
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALPARENTHUID FROM HIERARCHY WHERE HIERARCHYUID = %ld"),
					 pFolderEntry->dwHierarchyUid);
				retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,ParentLHUID,sizeof(ParentLHUID));

			//	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALHIERARCHYUID FROM HIERARCHY WHERE LOCALHIERARCHYUID = '%s'"),
			//		 ParentLHUID);
			//	retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,ParentLUID,sizeof(ParentLUID));
				SaveMsgtoWorkQueue(LocalAssetUID,ParentLHUID,IDS_ASSET,WORKQUEUE_UNLINK,L"",L"");
			
			}
			// delete hierarchy
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM HIERARCHY WHERE HIERARCHYUID = %ld"),
				 pFolderEntry->dwHierarchyUid);
			retCode = g_localDB.ExecuteSQL(sql);

	//	----------- modified by KenQ 2006-12-01 for KDDI requirement: ReferenceCount and RatingControl -------------
	//  This case is lost in version 3.6.6.1 - first release of supporting this requirement
			if(g_SupportRefcountRatingctrl)
			{
				if(bLinkedByOthers)
				{
					(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s was unlinked from Hierarchy, need to recalculate ReferenceCount"), assetLUID);

					AssetRatingRef(assetLUID, m_titleSortNameMDUID, false);
				}
				else//if(wcsicmp(oldTSN, _T(""))!=0)  
				{	// sometime, when coming here, the asset metadata has been deleted in the MDCallback
					// there's logic in MDCallback processing now. 
					(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s was deleted from Hierarchy, need to recalculate ReferenceCount for TitleSortName %s"), assetLUID, oldTSN);
					
					AssetRefOneTitle(assetLUID, m_titleSortNameMDUID, oldTSN);
				}
			}
		}
	} // end of if (wOp == IDS_DELETE)
	else	//	operation is to add, replace or put
	{
		// Parse the path string, to get parent ID, parent name and parent type
		WCHAR myName[MAXNAME];	//	currently parsed path's name
		CString leftPath, parentName;	//	currently parsed path string and parent path's name
		int index;	//	position indication in the path string
		WORD parentType;	//	parent path's type, could be IDS_APPLICATION or IDS_FOLDER
		CString csStr;
		AdjustRoot(pFolderEntry->wszName, csStr);
		index = csStr.Find(_T("\\"));
		CString csEntryName ; 
		DWORD dwEntryUID = 0;
		DWORD dwParentEntryUID = 0;
		while(index != -1)
		{
			csEntryName = csStr.Left(index);
			if(csEntryName == _T(""))
			{	
				dwParentEntryUID = -1;
				dwEntryUID = -1;
				//csEntryName = _T("Interactive Television");
				dwParentEntryUID = dwEntryUID;
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT ENTRYTYPE FROM HIERARCHY WHERE LOCALHIERARCHYUID = 'H0'"));
								
				g_localDB.SelectSQL(sql,SQL_C_ULONG,&parentType,0);
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT HIERARCHYUID FROM HIERARCHY WHERE LOCALHIERARCHYUID = 'H0'"));
								
				g_localDB.SelectSQL(sql,SQL_C_ULONG,&dwEntryUID,0);
			}
			else
			{
				csEntryName.Replace(_T("'"),_T("''"));
				parentName  = csEntryName;
				dwParentEntryUID = dwEntryUID;
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT ENTRYTYPE FROM HIERARCHY WHERE PARENTHUID = %d AND ENTRYNAME = N'%s'"),
					dwEntryUID,csEntryName);			
				g_localDB.SelectSQL(sql,SQL_C_ULONG,&parentType,0);
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT HIERARCHYUID FROM HIERARCHY WHERE ENTRYNAME = N'%s' AND PARENTHUID = %d"),
					csEntryName,dwEntryUID);			
				g_localDB.SelectSQL(sql,SQL_C_ULONG,&dwEntryUID,0);
			}
			csStr = csStr.Right(csStr.GetLength() - index - 1);
			index = csStr.Find(_T("\\"));
		};
		dwParentEntryUID = dwEntryUID;

		// if can not find the entry, it is not under sync directory, so ignore it
		if(dwParentEntryUID == -1)
		{
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:End of FolderCallBackProcess:  wszName = %ls, AssetID = %ld, wOp = %d"),
				GetCurrentThreadId(), pFolderEntry->wszName, pFolderEntry->dwAssetUid, wOp);
			
			IdsFree(pFolderEntry);	//	Free memory

			return;
		}
		

		//	Get currently parsed path's name from the end of the path string

		StringCbCopyW(myName, MAXNAME*2, csStr.GetBuffer(MAXNAME));

		csStr.ReleaseBuffer();

		//	Convert parent name into SQL format
		entryParentName = parentName;
		parentName.Replace(L"'", L"''");

		// 
		if(parentName == _T(""))// is a application
		{
			if(wOp == IDS_ADD)
			{
				SaveAppsToDB();

				(*gpDbSyncLog)(Log::L_INFO,_T("%d:Add APPLICATION %d to work queue."),GetCurrentThreadId(),dwTriggerUid);
			}
			else//replace
			{
				CString csPrevious = pFolderEntry->wszPrevious;
				csPrevious = csPrevious.Right(csPrevious.GetLength() -1);
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("UPDATE APPLICATION SET NAME = N'%s' WHERE NAME = N'%s'"),myName,csPrevious);
				retCode = g_localDB.ExecuteSQL(sql);
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("UPDATE HIERARCHY SET ENTRYNAME = N'%s' WHERE HIERARCHYUID = %d"),myName,pFolderEntry->dwHierarchyUid);
				retCode = g_localDB.ExecuteSQL(sql);
			}
		}
		else if (pFolderEntry->dwAssetUid == 0)		//	It's a folder
		{
			//	Write the folder to DB
			
			retCode = SaveOneFolder(pFolderEntry->dwHierarchyUid, myName);
			//	Write the folder as a hierarchy to DB
			retCode = SaveOneHierarchy(pFolderEntry->dwHierarchyUid, IDS_FOLDER, 
									   pFolderEntry->dwHierarchyUid, myName, dwParentEntryUID,parentType);
			

			if (wOp == IDS_ADD)	//	operation is to add a folder
			{
				DWORD dwUIDParam = pFolderEntry->dwHierarchyUid;	//	argument of thread
				{
					SaveObjectMdToDB(pFolderEntry->dwHierarchyUid, IDS_FOLDER);
					SaveEntriesToDB(pFolderEntry->dwHierarchyUid, IDS_FOLDER);
				}
			}
		}
		else	//	It's an asset
		{
			BOOL bNewAsset = true;
			//	Write the asset to DB
			retCode = SaveOneAsset(pFolderEntry->dwAssetUid, myName, pFolderEntry->dwAppUid, bNewAsset);
			
			bool bErased = false;
			if (wOp == IDS_ADD)	//	operation is to add an asset
			{
				DWORD nCount =0;
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT COUNT(*) FROM ASSETMD WHERE LOCALASSETUID IN (SELECT LOCALASSETUID FROM ASSET WHERE ASSETUID = %d)"),pFolderEntry->dwAssetUid);
				retCode = g_localDB.SelectSQL(sql,SQL_C_ULONG,&nCount,0);
				if(nCount <= 5 /* == 0 */)  // generally there should be more than 5 md, use 0 to do comparation, sometime it is not right, some time LAM auto insert MD
				{
					SaveObjectMdToDB(pFolderEntry->dwAssetUid, IDS_ASSET, bNewAsset);
					SaveElementsToDB(pFolderEntry->dwAssetUid, bNewAsset);
					
					// 2007-11-29 by Ken Qian, 
					// This case mostly the asset ingested from ITV AM, so in LAM there is no metadata, 
					// so above two lines codes is to fetch the asset's metadata and element
					// to this case, we can ignore the CaCallback, coz mostly CaCallback comes before folder callback,
					// to avoid to do the same thing.
					DISORDERCALLBCK::iterator it = m_disOrderCBs.find(pFolderEntry->dwAssetUid);
					if(it != m_disOrderCBs.end())
					{
						DSCallBackCAsset* pCallback = (DSCallBackCAsset*)it->second;
						delete pCallback;
						
						m_disOrderCBs.erase(it);

						bErased = true;
						(*gpDbSyncLog)(Log::L_INFO,_T("%d:Discard the previously kept CaCallback for asset %d."), 
							GetCurrentThreadId(), pFolderEntry->dwAssetUid); 
					}
				}
			}

			if(!bErased)
			{
				// 2007-11-29 by Ken Qian, 
				// This case mostly the asset ingested from LAM, we need to re-process the callback
				// coz, to this case, mostly of CaCallback callback comes after folder callback, 
				// but sometime it comes before, without re-process it, element status maybe is not right.

				DISORDERCALLBCK::iterator it = m_disOrderCBs.find(pFolderEntry->dwAssetUid);
				if(it != m_disOrderCBs.end())
				{
					(*gpDbSyncLog)(Log::L_INFO,_T("%d:put previously kept CaCallback for asset %d in to queue again."), 
						GetCurrentThreadId(), pFolderEntry->dwAssetUid); 
					
					DSCallBackCAsset* pCallback = (DSCallBackCAsset*)it->second;
					g_server.AddingCallbackToThread(pCallback);	
					m_disOrderCBs.erase(it);
				}
			}
			//	Write the asset as a hierarchy to DB
			retCode = SaveOneHierarchy(pFolderEntry->dwHierarchyUid, IDS_ASSET, 
									   pFolderEntry->dwAssetUid, myName, dwParentEntryUID,parentType);
		}
	}
	}catch(...)
	{
		(*gpDbSyncLog)(Log::L_WARNING,_T("%d:FolderCallBackProcess catch an exception when processing."),GetCurrentThreadId()); 
	}

	//////////////////////////////////////////////////////////////////////////
	// Added by Bernie, 2006-Feb-5
	// call Add-in
//	if(g_AdiMan.isValid() && gUpdateThreadId != GetCurrentThreadId())
	if(g_AdiMan.isValid() && !m_isFullSyncing)
	{
		DA_hierarchyDb	hrchyBlock;
		
		if(entryParentName==_T(""))
			hrchyBlock._dwEntryType = DBSA_ENTRY_APPLICATION;
		else if(pFolderEntry->dwAssetUid==0)
			hrchyBlock._dwEntryType = DBSA_ENTRY_FOLDER;
		else
			hrchyBlock._dwEntryType = DBSA_ENTRY_ASSET;
		
		hrchyBlock._dwHierarchyUID = pFolderEntry->dwHierarchyUid;

		SQLTCHAR hierarchyLUID [21] = _T("");
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALHIERARCHYUID FROM HIERARCHY WHERE HIERARCHYUID = %ld AND ENTRYTYPE = %ld"), 
			pFolderEntry->dwHierarchyUid , hrchyBlock._dwEntryType);
		retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, hierarchyLUID, sizeof(hierarchyLUID));
	
		if(retCode == SQL_SUCCESS)
			StringCbCopyW(hrchyBlock._szLocalHUID, DBSA_MAXLUID*sizeof(wchar_t), hierarchyLUID);
		else
			StringCbCopyW(hrchyBlock._szLocalHUID, DBSA_MAXLUID*sizeof(wchar_t), _T(""));
		
		g_AdiMan.TrggHrchy(wOp, &hrchyBlock);
	}
	// end 
	//////////////////////////////////////////////////////////////////////////

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:End of FolderCallBackProcess:  wszName = %ls, AssetID = %ld, wOp = %d"),
		GetCurrentThreadId(), pFolderEntry->wszName, pFolderEntry->dwAssetUid, wOp);
	

	IdsFree(pFolderEntry);	//	Free memory

	return;
}                       

/*****************************************************************************
Function Name:	CaCallBackProcess
Arguments:		Trigger ID (DWORD), Asset ID (DWORD), Time Stamp (IDSUIDUPDATESTAMP),
				Complex Member Number (DWORD), Complex Member Info (COMPLEXASSET)
Returns:		None
Description:	This function is called by ITV trigger when complex elements within
				an asset is changed. 
				It gets the asset ID and the structure of complex member thereof, and
				passed them to SaveElementsToDB to update corresponding local DB.
Called By:		(ITV callback)
*****************************************************************************/
void CDSInterface::CaCallBackProcess(DWORD dwTriggerUid, DWORD dwAssetUid, IDSUIDUPDATESTAMP assetStamp, DWORD dwNumCa, COMPLEXASSET *pOutCa)
{   


	(*gpDbSyncLog)(Log::L_INFO, _T("%d:Entering CaCallBackProcess:  AssetID = %ld, dwNumCa = %ld"),GetCurrentThreadId(), dwAssetUid, dwNumCa);
	try{
	
	//	Save the complex member info of the asset into Local DB
	if (CheckZQEntryState(dwAssetUid,NULL,IDS_ASSET) != -1)
	{
	
		SaveOneComplexAsset(dwAssetUid, dwNumCa, pOutCa);

		SetACFlag(dwAssetUid);

		SaveObjectMdToDB(dwAssetUid, IDS_ASSET, FALSE);
		SaveElementsToDB(dwAssetUid, FALSE);
		
		// comment by KenQ on 2007-11-29, since logic is changed. 
	    // --------- Add by KenQ on 2007-01-20, add a map to avoid the disorder callback are always there. 
/*		DISORDERCALLBCK::iterator it = m_disOrderCBs.find(dwAssetUid);
		if(it != m_disOrderCBs.end())
		{
			m_disOrderCBs.erase(it);
		}
*/
	}
	//---------- Modified by KenQ on 2006-08-07, for callback disorder issue between CaCallBack and FolderCallBack (for asset)
	// in disorder case, the Asset EntryUID is still NULL when this callback happens,
	// which cause the Asset and AtomicElement status & AtomicEntryUID failed to be set.
	// Since even the two callbacks are disordered, they are coming almost at the smae time(in a ms). 
	// I would like to sleep awhile, then insert into the queue again, for later processing
	else
	{
		// comment by KenQ on 2007-11-29, since logic is changed. 
		(*gpDbSyncLog)(ZQ::common::Log::L_INFO, "Found CaCallBack coming before FolderCallback, kept for later processing");

		DSCallBackCAsset* pCallback = new DSCallBackCAsset(DSCallBackBase::CASSET, dwTriggerUid, dwAssetUid, dwNumCa, pOutCa, assetStamp);
		m_disOrderCBs.insert(DISORDERCALLBCK::value_type(dwAssetUid, pCallback));
		return;
	}
	
	}catch(...)
	{
		(*gpDbSyncLog)(Log::L_WARNING, _T("%d:CaCallBackProcess catch an exception when processing."),GetCurrentThreadId());
	}
	(*gpDbSyncLog)(Log::L_INFO, _T("%d:Leaving CaCallBackProcess:  AssetID = %ld"),GetCurrentThreadId(),dwAssetUid);
	
	IdsFree(pOutCa);	//	Free memory

	return;
}

/*****************************************************************************
Function Name:	SiteMDCallBackProcess
Arguments:		Trigger ID (DWORD), Site ID (DWORD), Metadata Number (DWORD)
				Metadata Info (METADATA *)
Returns:		None
Description:	This function is called by ITV trigger when site metadata is changed. 
				It gets the site ID and the structure of metadatas, and passed them 
				to SaveSiteMdToDB to update corresponding local DB.
Called By:		(ITV callback)
Calls:			SaveSiteMdToDB
				LocalDB::OpenLocalDB
				LocalDB::CloseLocalDB
*****************************************************************************/
void CDSInterface::SiteMDCallBackProcess(DWORD dwTriggerUid, DWORD dwUid, DWORD dwNumMd, METADATA *pOutMd)
{
	DWORD threadid = GetCurrentThreadId();
	(*gpDbSyncLog)(Log::L_INFO, _T("%d:Entering SiteMDCallBackProcess:  SiteID = %ld, NumMD = %ld, MDName = %s"),
		 threadid,dwUid, dwNumMd, pOutMd->wszMdName);
	try{	
		SaveSiteMdToDB(pOutMd, dwNumMd, dwUid);
	}catch(...)
	{
		(*gpDbSyncLog)(Log::L_WARNING,_T("%d:SiteMDCallBackProcess cath an exception when processing"),threadid); 
	}
	(*gpDbSyncLog)(Log::L_INFO, _T("%d:Leaving SiteMDCallBackProcess:  SiteID = %ld, NumMD = %ld"),
		 threadid,dwUid, dwNumMd);
	
	IdsFreeMd(pOutMd, dwNumMd);	//	Free memory

	return;
}

/*****************************************************************************
Function Name:	ApplicationMDCallBackProcess
Arguments:		Trigger ID (DWORD), Application ID (DWORD), Metadata Number (DWORD)
				Metadata Info (METADATA *)
Returns:		None
Description:	This function is called by ITV trigger when application metadata is changed. 
				It gets the application ID and the structure of metadatas, and passed them 
				to SaveAppMdToDB to update corresponding local DB.
Called By:		(ITV callback)
*****************************************************************************/
void CDSInterface::ApplicationMDCallBackProcess(DWORD dwTriggerUid, DWORD dwUid, DWORD dwNumMd, METADATA *pOutMd)
{

	DWORD threadid = GetCurrentThreadId();
	SQLTCHAR sql[1024];
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Entering ApplicationMDCallBackProcess: ApplicationID = %ld, NumMD = %ld, MDName = %s"),
		 threadid,dwUid, dwNumMd, pOutMd->wszMdName);
	try{
	//	Save the metadata info of the application into Local DB
		SQLTCHAR Name[256];		
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT NAME FROM APPLICATION WHERE APPLICATIONUID = %d"),dwUid);
		RETCODE ret = g_localDB.SelectSQL(sql,SQL_C_TCHAR,Name,sizeof(Name));
		
		if(ret == SQL_SUCCESS)
		{	
			SaveAppMdToDB(pOutMd, dwNumMd, dwUid);
		}	
	}catch(...)
	{
		(*gpDbSyncLog)(Log::L_WARNING,_T("%d:ApplicationMDCallBackProcess catch an exception when processing."),threadid);
	}
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Leaving ApplicationMDCallBackProcess: ApplicationID = %ld, NumMD = %ld"),
		 threadid,dwUid, dwNumMd);
	
	IdsFreeMd(pOutMd, dwNumMd);	//	Free memory
	return;
}

/*****************************************************************************
Function Name:	FolderMDCallBackProcess
Arguments:		Trigger ID (DWORD), Folder ID (DWORD), Metadata Number (DWORD)
				Metadata Info (METADATA *)
Returns:		None
Description:	This function is called by ITV trigger when folder metadata is changed. 
				It gets the folder ID and the structure of metadatas, and passed them 
				to SaveFolderMdToDB to update corresponding local DB.
Called By:		(ITV callback)
*****************************************************************************/
void CDSInterface::FolderMDCallBackProcess(DWORD dwTriggerUid, DWORD dwUid, DWORD dwNumMd, METADATA *pOutMd)
{
	DWORD threadid = GetCurrentThreadId();

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Entering FolderMDCallBackProcess:  FolderID = %ld, NumMD = %ld, MDName = %s"),
		 threadid,dwUid, dwNumMd, pOutMd->wszMdName);
	try{	
		SaveFolderMdToDB(pOutMd, dwNumMd, dwUid);
	}catch(...)
	{
		(*gpDbSyncLog)(Log::L_WARNING,_T("%d:FolderMDCallBackProcess catch an exception when processing"),threadid); 
	}	
	
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Leaving of FolderMDCallBackProcess:  FolderID = %ld, NumMD = %ld"),
		 threadid,dwUid, dwNumMd);
	
	IdsFreeMd(pOutMd, dwNumMd);	//	Free memory

	return;
}

/*****************************************************************************
Function Name:	AssetMDCallBackProcess
Arguments:		Trigger ID (DWORD), Asset ID (DWORD), Metadata Number (DWORD)
				Metadata Info (METADATA *)
Returns:		None
Description:	This function is called by ITV trigger when asset metadata is changed. 
				It gets the asset ID and the structure of metadatas, and passed them 
				to SaveAssetMdToDB to update corresponding local DB.
Called By:		(ITV callback)
*****************************************************************************/
void CDSInterface::AssetMDCallBackProcess(DWORD dwTriggerUid, DWORD dwUid, DWORD dwNumMd, METADATA *pOutMd)
{

	DWORD threadid = GetCurrentThreadId();

	// do not log 'PsUseInfo', all garbage!!
	if(wcsicmp(pOutMd->wszMdName,_T("PsUseInfo"))!=0)
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:Entering AssetMDCallBackProcess:  AssetID = %ld, NumMD = %ld, MDName = %s"),
			 threadid,dwUid, dwNumMd, pOutMd->wszMdName);
	try{	
		// do not process psuseInfo
		if(wcsicmp(pOutMd->wszMdName,_T("PsUseInfo"))!=0)		
			SaveAssetMdToDB(pOutMd, dwNumMd, dwUid, FALSE);
	}catch(...)
	{
		(*gpDbSyncLog)(Log::L_WARNING,_T("%d:AssetMDCallBackProcess catch an exception when processing."),threadid); 
	}
    
	// do not log 'PsUseInfo', all garbage!!
	if(wcsicmp(pOutMd->wszMdName,_T("PsUseInfo"))!=0)
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:Leaving AssetMDCallBackProcess:  AssetID = %ld, NumMD = %ld"),
			 threadid,dwUid, dwNumMd);
	

	IdsFreeMd(pOutMd, dwNumMd);	//	Free memory

	return;
}

/*****************************************************************************
Function Name:	ElementMDCallBackProcess
Arguments:		Trigger ID (DWORD), Element ID (DWORD), Metadata Number (DWORD)
				Metadata Info (METADATA *)
Returns:		None
Description:	This function is called by ITV trigger when element metadata is changed. 
				It gets the element ID and the structure of metadatas, and passed them 
				to SaveElementMdToDB to update corresponding local DB.
Called By:		(ITV callback)
*****************************************************************************/
void CDSInterface::ElementMDCallBackProcess(DWORD dwTriggerUid, DWORD dwUid, DWORD dwNumMd, METADATA *pOutMd)
{

	DWORD threadid = GetCurrentThreadId();
    (*gpDbSyncLog)(Log::L_INFO, _T("%d:Entering ElementMDCallBackProcess:  ElementID = %ld, NumMD = %ld, MDName = %s"),
		 threadid,dwUid, dwNumMd, pOutMd->wszMdName);  //added by W.Z. for monitoring callback
	try{
		SaveElementMdToDB(pOutMd, dwNumMd, dwUid, FALSE);
	}
	catch(...)
	{
		(*gpDbSyncLog)(Log::L_WARNING,_T("%d:ElementMDCallBackProcess catch an exception when processing."),threadid); 
	}
    (*gpDbSyncLog)(Log::L_INFO, _T("%d:Leaving ElementMDCallBackProcess:  ElementID = %ld, NumMD = %ld"),threadid
		 ,dwUid, dwNumMd);  //added by W.Z. for monitoring callback
	
	IdsFreeMd(pOutMd, dwNumMd);	//	Free memory

	return;
}

/*****************************************************************************
Function Name:	ClipMDCallBackProcess
Arguments:		Trigger ID (DWORD), CLIP ID (DWORD), Metadata Number (DWORD)
				Metadata Info (METADATA *)
Returns:		None
Description:	This function is called by ITV trigger when element metadata is changed. 
				It gets the element ID and the structure of metadatas, and passed them 
				to SaveElementMdToDB to update corresponding local DB.
Called By:		(ITV callback)
*****************************************************************************/
void CDSInterface::ClipMDCallBackProcess(DWORD dwTriggerUid, DWORD dwUid, DWORD dwNumMd, METADATA *pOutMd)
{
	DWORD threadid = GetCurrentThreadId();
    (*gpDbSyncLog)(Log::L_INFO, _T("%d:Entering ClipMDCallBackProcess:  CLIPID = %ld, NumMD = %ld, MDName = %s"),
		 threadid,dwUid, dwNumMd, pOutMd->wszMdName);  //added by W.Z. for monitoring callback
	try{	
		SaveClipMdToDB(pOutMd, dwNumMd, dwUid);
	}catch(...)
	{
		(*gpDbSyncLog)(Log::L_WARNING,_T("%d:ClipMDCallBackProcess catch an exception when processing."),threadid); 		
	}
    (*gpDbSyncLog)(Log::L_INFO, _T("%d:Leaving ClipMDCallBackProcess:  CLIPID = %ld, NumMD = %ld"),
		 threadid,dwUid, dwNumMd);  //added by W.Z. for monitoring callback
	

	IdsFreeMd(pOutMd, dwNumMd);	//	Free memory

	return;
}

/*****************************************************************************
Function Name:	AssetStateCallBackProcess
Arguments:		Trigger ID (DWORD), Type (WORD), Asset ID (DWORD), 
				Time Stamp (IDSUIDUPDATESTAMP), Current State (WORD),
				Operation Type (WORD)
Returns:		None
Description:	This function is called by ITV trigger when asset state is changed. 
				It gets the asset ID and the current state, and update local ASSET 
				and HIERARCHY table accordingly.
Called By:		(ITV callback)
*****************************************************************************/
void CDSInterface::AssetStateCallBackProcess(DWORD dwTriggerUid, WORD wType, DWORD dwUid, IDSUIDUPDATESTAMP pAssetStamp, WORD wState, WORD wOperation)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR     sql[1024];	//	string to store SQL statement
	DWORD threadid = GetCurrentThreadId();
    (*gpDbSyncLog)(Log::L_DEBUG, _T("%d:Entering AssetStateCallBackProcess:  AssetID = %ld, wType = %ld, wState = %ld, wOp = %d"),
		 threadid,dwUid, wType, wState, wOperation);  //added by W.Z. for monitoring callback
	try{	
		SQLTCHAR assetLUID[21];	//	Local UID of asset
		//	Get local UID of the asset from table ASSET
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALASSETUID FROM ASSET WHERE ASSETUID = %ld"), 
				 dwUid);
		retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, assetLUID, sizeof(assetLUID));
		if (retCode == SQL_NO_DATA)	//	LUID of the asset got
		{
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:Asset %ld does not exist."),threadid, dwUid); 
			return;
		}

		if(IDS_DELETE == wOperation)
		{
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:delete asset %s since wOp is %d(IDS_DELETE)."),threadid, assetLUID, wOperation); 

			// Find out this asset's parent HUID
			WCHAR ParentLHUID[64];

			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALPARENTHUID FROM HIERARCHY WHERE LocalEntryUID = '%s' AND ENTRYTYPE= %ld"),
				 assetLUID,IDS_ASSET);
			retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,ParentLHUID,sizeof(ParentLHUID));

			//	Delete the member relationship of the asset to delete
			SaveOneComplexAsset(dwUid, 0, NULL);

			//	Delete the metadata of the asset from ASSETMD table
			
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM ASSETMD WHERE LOCALASSETUID = '%s'"),
					 assetLUID);
			retCode = g_localDB.ExecuteSQL(sql);

			//	Delete the asset from ASSET table

			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM ASSET WHERE ASSETUID = %ld"),
					 dwUid);
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);
			retCode = g_localDB.ExecuteSQL(sql);
			if(retCode == SQL_SUCCESS)
				SaveMsgtoWorkQueue(assetLUID,ParentLHUID,IDS_ASSET,WORKQUEUE_DELETE,L"",L"");
		}
		else if (CheckZQEntryState(dwUid, NULL, IDS_ASSET) != -1)	//	the asset is in Local DB
		{
			//	Save the current state of the asset into Local DB
			UpdateAssetState(dwUid, wState);
		}
		
		SQLINTEGER inIndicate = SQL_NTS, outIndicate = 0;	//	indication for the length of parameter buffer
		SQLSMALLINT procRet = 0;	//	return code of the stored procudure
		//	Call an stored procedure to update entrystate of the asset according to the entrystate of its members
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("{CALL UPDATE_ENTRYSTATE_WITH_ASSET(?,?)}"));
		retCode = g_localDB.CallStoreProcedure(sql, 
											   LocalDB::INPUT, SQL_C_TCHAR, SQL_VARCHAR, sizeof(assetLUID), 0, assetLUID, sizeof(assetLUID), &inIndicate,
											   LocalDB::OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &procRet, 0, &outIndicate,
											   LocalDB::INOUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, NULL, 0, NULL);
		(*gpDbSyncLog)(Log::L_INFO, _T("%d:Result of UPDATE_ENTRYSTATE_WITH_ASSET for asset %s: %hu."),threadid, assetLUID, procRet);

	}
	catch(...)
	{
		(*gpDbSyncLog)(Log::L_WARNING, _T("%d:AssetStateCallBackProcess catch an exception when processing."),threadid);
	}
    (*gpDbSyncLog)(Log::L_DEBUG, _T("%d:Leaving AssetStateCallBackProcess:  AssetID = %ld, wType = %ld, wState = %ld, wOp = %d"),threadid,
		 dwUid, wType, wState, wOperation);  //added by W.Z. for monitoring callback
	

	return;
}

/*****************************************************************************
Function Name:	ElementStateCallBackProcess
Arguments:		Trigger ID (DWORD), Type (WORD), Element ID (DWORD), 
				Time Stamp (IDSUIDUPDATESTAMP), Current State (WORD),
				Operation Type (WORD)
Returns:		None
Description:	This function is called by ITV trigger when element state is changed. 
				It gets the asset ID and the current state, and update local ATOMICELEMENT 
				table accordingly.
Called By:		(ITV callback)
*****************************************************************************/
void CDSInterface::ElementStateCallBackProcess(DWORD dwTriggerUid, WORD wType, DWORD dwUid, IDSUIDUPDATESTAMP pAssetStamp, WORD wState, WORD wOperation)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR     sql[1024];	//	string to store SQL statement
	DWORD threadid = GetCurrentThreadId();
    (*gpDbSyncLog)(Log::L_DEBUG, _T("%d:Entering ElementStateCallBackProcess:  ElementID = %ld, wType = %ld, wState = %ld, wOp = %d"),
		 threadid,dwUid, wType, wState, wOperation);  //added by W.Z. for monitoring callback
	try{	
	if (CheckZQEntryState(dwUid, NULL, IDS_ATOMIC_ELEMENT) != -1)	//	the element is in Local DB
		//	Save the current state of the element into Local DB
		UpdateElementState(dwUid, wState);

	SQLTCHAR elementLUID[21];	//	Local UID of element
	//	Get local UID of the element from table ATOMICELEMENT
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALATOMICELEMENTUID FROM ATOMICELEMENT WHERE ATOMICELEMENTUID = %ld"), 
			 dwUid);
	retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, elementLUID, sizeof(elementLUID));
	if (retCode == SQL_SUCCESS)	//	LUID of the element got
	{
		SQLINTEGER inIndicate = SQL_NTS, outIndicate = 0;	//	indication for the length of parameter buffer
		SQLSMALLINT procRet = 0;	//	return code of the stored procudure
		//	Call an stored procudure to update entrystate of the asset containing this element
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("{CALL UPDATE_ENTRYSTATE_WITH_ELEMENT(?,?)}"));
		retCode = g_localDB.CallStoreProcedure(sql, 
			                                   LocalDB::INPUT, SQL_C_TCHAR, SQL_VARCHAR, sizeof(elementLUID), 0, elementLUID, sizeof(elementLUID), &inIndicate,
											   LocalDB::OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &procRet, 0, &outIndicate,
											   LocalDB::INOUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, NULL, 0, NULL);
		(*gpDbSyncLog)(Log::L_INFO, _T("%d:Result of UPDATE_ENTRYSTATE_WITH_ELEMENT for element %s: %hu."),threadid, elementLUID, procRet);
		
	}
	}catch(...)
	{
		(*gpDbSyncLog)(Log::L_WARNING,_T("%d:ElementStateCallBackProcess catch an exception when processing."),threadid); 
	}
    (*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Leaving ElementStateCallBackProcess:  ElementID = %ld, wType = %ld, wState = %ld, wOp = %d"),
		 threadid,dwUid, wType, wState, wOperation);  //added by W.Z. for monitoring callback
	

	return;
}


/*****************************************************************************
Function Name:	ClearAll
Arguments:		None
Returns:		res (DWORD) (0:success !0:fail)
Description:	This function clear all local DB tables.
*****************************************************************************/
DWORD CDSInterface::ClearAll()
{
	DWORD res = 0;	//	return code

	//	Clear all local tables 
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM HIERARCHY"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM COMPLEXASSET"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM SITEMD"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM APPLICATIONMD"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM FOLDERMD"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ASSETMD"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ATOMICELEMENTMD"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM CLIPMD"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM SITE"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM APPLICATION"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM FOLDER"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ASSET"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM CLIP"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ATOMICELEMENT"));	
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDSELECT WHERE MDUID NOT IN (SELECT MDUID FROM ENTRYMDD WHERE MDUID<=2000)"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM WQ_INFO"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDD WHERE MDUID>2000"));  

	res = g_localDB.ExecuteSQL((SQLTCHAR*)_T("UPDATE GETSEQUENCE SET CURVALUE = MINVALUE"));

	return res;
}

/*****************************************************************************
Function Name:	UpdateAll
Arguments:		None
Returns:		res (DWORD) (0:success !0:fail)
Description:	This function goes through the whole hierachy in ITV, reads all 
				objects and metadatas, and then updates local DB accordingly.
				1. Set updating flag for local DB
				2. Update all sites
				3. Update all applications
				4. Update all application defined select entries of asset
				5. Update all folders, assets and elements according to hierarchy tree
				6. Clear all deleted records whose updating flag was not reset
*****************************************************************************/
DWORD CDSInterface::UpdateAll()
{
	(*gpDbSyncLog)(Log::L_NOTICE, _T("Enter UpdateAll()"));
	
	// counting times
	g_dwFullSyncCount++;

	// begin full syncing
	m_isFullSyncing = true;

	DWORD res = 0;	//	return code
	DWORD i;	//	variable to control loop
	gUpdateThreadId = GetCurrentThreadId();
	(*gpDbSyncLog)(Log::L_NOTICE,_T("%d:Call UpdateALL"),gUpdateThreadId);
	
	m_totalIgnoredCount = 0;  // reset to re-count it since full sync happens. 

	//////////////////////////////////////////////////////////////////////////
	// Added by Bernie, 2005-Nov-24
	// call Add-in
	if(g_AdiMan.isValid())
	{
		g_AdiMan.SyncBegin();
	}
	// end 
	//////////////////////////////////////////////////////////////////////////	

	// 2007-11-29 by Ken Qian, clear the kept DSCallBackCAsset callback if there is.
	DISORDERCALLBCK::iterator it = m_disOrderCBs.begin();
	while(it != m_disOrderCBs.end())
	{
		DSCallBackCAsset* pCallBack = (DSCallBackCAsset*)it->second;

		it++;
		
		delete pCallBack;
	}
	// 2009-02-16 fix JIRA ACE-3033, DBSync thread exit at the 3rd time full sync if there is something in the map.
	m_disOrderCBs.clear();
	
	// reset temp update stamp
	memset(g_CurrUpdateStamp, 0, sizeof(g_CurrUpdateStamp));

	// Set all ZQ_ENTRYSTATE before updating, which means to set all records unavailable temporarily
	// changed by kaliven.lee 05.1.19 oh my busy birthday
	// original logic:
	// add the ZQ_Entrystate up 1, and if found reduce it 1. delete all ZQ_Entrystate = 1
	// current logic:
	// set all the ZQ_Entrystate to 1, if found reset it to 0.
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("UPDATE SITE SET ZQ_ENTRYSTATE = 1 WHERE ZQ_ENTRYSTATE = 0 "));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("UPDATE APPLICATION SET ZQ_ENTRYSTATE = 1 WHERE ZQ_ENTRYSTATE = 0"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("UPDATE ComplexAsset SET ZQ_ENTRYSTATE = 1 WHERE ZQ_ENTRYSTATE = 0"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("UPDATE CLIP SET ZQ_ENTRYSTATE = 1 WHERE ZQ_ENTRYSTATE = 0"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("UPDATE FOLDER SET ZQ_ENTRYSTATE = 1  WHERE ZQ_ENTRYSTATE = 0"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("UPDATE ASSET SET ZQ_ENTRYSTATE = 1 WHERE ZQ_ENTRYSTATE = 0"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("UPDATE ATOMICELEMENT SET ZQ_ENTRYSTATE = 1 WHERE ZQ_ENTRYSTATE = 0"));
	
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("UPDATE HIERARCHY SET ZQ_ENTRYSTATE = 1 WHERE ZQ_ENTRYSTATE = 0"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("UPDATE ENTRYMDSELECT SET ZQ_ENTRYSTATE = 1 WHERE ZQ_ENTRYSTATE = 0"));
	// clear all the METADATA table 


	//  Modified By KenQ at 2007-09-19	
	// res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDSELECT WHERE MDUID NOT IN (SELECT MDUID FROM ENTRYMDD WHERE MDUID<=2000)"));
	// res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("UPDATE GETSEQUENCE SET  CURVALUE = MINVALUE WHERE SEQNAME = 'Seq_MDUID'"));
	// res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDD WHERE MDUID >2000"));

	// and after launched, only throw a record to WORKQUENE.
//	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM SITEMD"));
	//res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM APPLICATIONMD"));
	//res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM FOLDERMD"));
//	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ASSETMD"));
	//res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ATOMICELEMENTMD"));
	//res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM CLIPMD"));

	//Updating for all sites

	SITEINFO *pSites = 0;	//	structure to store site info
	DWORD siteNum = 0;		//	Number of sites
	//	Get number of sites and all site info
	res = IdsListSites(&m_idssess, &pSites, &siteNum, &m_itvsta, 0);
	if (res)	//	error occurs
	{
		//	Log error message
		(*gpDbSyncLog)(Log::L_ERROR, _T("IdsListSites ERROR!"));
		
		m_isFullSyncing = false;
		return res;
	}

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Save sites info to DB..."), gUpdateThreadId);
	//	Save all site info into Local DB
	SaveSitesToDB(pSites, siteNum);

	for (i=0; i<siteNum; i++)	//	For each site
	{
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:SaveObjectMdToDB(%d, IDS_SITE)"), gUpdateThreadId, (pSites+i)->wSiteUid);
		//	Save all the metadata into Local DB per site ID
		res = SaveObjectMdToDB((pSites+i)->wSiteUid, IDS_SITE);
	}

	IdsFree(pSites);	//	Free memory


////	Updating for all applications
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Get Applications info from IDS..."), gUpdateThreadId);

	APPNAME *pApps = 0;	//	structure to store application info
	DWORD appNum = 0;		//	Number of applications
	//	Get number of applications and all application info
	res = IdsListApplications(&m_idssess, &pApps, &appNum, &m_itvsta, 0);
	if (res)	//	error occurs
	{
		//	Log error message
		(*gpDbSyncLog)(Log::L_ERROR, _T("IdsListApplications ERROR!0X%X"),res);

		m_isFullSyncing = false;
		
		return res;	//	Exit
	}

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Save %d Applications info to DB..."), gUpdateThreadId, appNum);
	
	//	Save all application info into Local DB
	res = SaveAppsToDB(pApps, appNum);
	if(res != ITV_SUCCESS)
	{
		(*gpDbSyncLog)(Log::L_NOTICE, _T("SaveAppsToDB() failed!0X%X"), res);
		m_isFullSyncing = false;
		return res;
	}
	
	CString appName;
	for (i=0; i<appNum; i++)		//	For each application
	{	
		appName = (pApps+i)->wszName;
		appName.Replace(_T("'"), _T("''"));
		
		// only save the synced applications
		if(DoesSkipApplication((pApps+i)->dwUid, appName))
			continue;

		(*gpDbSyncLog)(Log::L_INFO,_T("%d:SaveObjectMdToDB(%d, IDS_APPLICATION)"), gUpdateThreadId, (pApps+i)->dwUid);
		//	Save all the metadata into Local DB per application ID
		res = SaveObjectMdToDB((pApps+i)->dwUid, IDS_APPLICATION);
	}

	// Updating for all application defined select entries of asset
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:SaveEntryMDDToDB() ..."), gUpdateThreadId);
	res = SaveEntryMDDToDB();
	if(res != ITV_SUCCESS)
	{
		(*gpDbSyncLog)(Log::L_NOTICE, _T("SaveEntryMDDToDB() failed!0X%X"), res);
		m_isFullSyncing = false;
		return res;
	}

//  ---------- modifyed by KenQ 2006-05-18 ---------------
//  To support multiple sync folders
	WCHAR syncFolderName[MAXNAMELEN*2];	
	DWORD entryHUID, entryType; 
	
	int nSyncLoop = 0;
	int nSyncFolderCount = g_SyncFolders.size();
	for(nSyncLoop=0; nSyncLoop < nSyncFolderCount; nSyncLoop++)
	{
		// get the sync folder name
		StringCbCopy(syncFolderName, MAXNAMELEN*2, g_SyncFolders.at(nSyncLoop).c_str());

		// which AppUID belongs to
		CString strApp = syncFolderName;
		DWORD appUid = 0;
		int appIndex = strApp.Find(_T('\\'), 1);
		if(appIndex!=-1)
		{
			strApp = strApp.Left(appIndex);
			strApp = strApp.Right(strApp.GetLength()-1);
		}
		else
		{
			strApp = "";
		}
		
		for (i=0; i<appNum; i++)		//	For each application
		{
			if(strApp == (pApps+i)->wszName)
				appUid = (pApps+i)->dwUid;			
		}


		//	---------- modified by Bernie Zhao 2005-07-18-------------
		//	 convert synchronizing root got from registry to hierarchy uid
		
		int nObjCount = GetEntryHUIDByName(syncFolderName, entryHUID, entryType);
		if(0 == nObjCount)
		{
			(*gpDbSyncLog)(Log::L_ERROR,_T("Specified Synchronizing Directory \"%s\" does not exist in IDS Hierarchy!"),syncFolderName);

			m_isFullSyncing = false;
			return -1;
		}
		else if(nObjCount > 1)
		{
			(*gpDbSyncLog)(Log::L_ERROR,_T("Specified Synchronizing Directory \"%s\" have multiple objects in IDS Hierarchy. Please remove duty data first"),syncFolderName);

			m_isFullSyncing = false;
			return -1;
		}
		
		if(entryType==IDS_SITE)
		{
			entryType = IDS_APPLICATION;
		}
	
		(*gpDbSyncLog)(Log::L_INFO,_T("SyncFolder=%s, EntryHUID=%d, EntryType=%d, AppUID=%d"), syncFolderName, entryHUID, entryType, appUid);
		
		//	---------- modified by KenQ 2006-05-17-------------
		//	 If we sync the specified folder(entrytype=3), the folder info is required 
		//   insert into FOLDER and Hierarchy Table. 
		//   Following block is not added in routine SaveEntriesToDB() coz it is a recursive function.
		
		//	---------- modified by KenQ 2006-08-03-------------
		//  A switch to support in case of only syncing one folder, whether original folder itself 
		//	is to linked to LAM Navigation Application.
		
		if(entryType == IDS_FOLDER && ( (nSyncFolderCount > 1) || (1 == nSyncFolderCount && g_SyncedFolderAsNavNode > 0) ) )
		{
			CString folderName = syncFolderName;
			int nIndex = folderName.Left(folderName.GetLength()-1).ReverseFind(_T('\\'));
			folderName = folderName.Right(folderName.GetLength() - nIndex - 1);
			folderName = folderName.Left(folderName.GetLength()-1);

			//	Write the folder to DB
			res = SaveOneFolder(entryHUID, (LPCTSTR)folderName);

			//	Write the metadata of the folder to DB
			res = SaveObjectMdToDB(entryHUID, IDS_FOLDER);

			//	Write the hierarchy relationship to DB - the specified folder's parent is the Application
			res = SaveOneHierarchy(entryHUID, IDS_FOLDER, entryHUID, (LPCTSTR)folderName, appUid,IDS_APPLICATION);
		}// End of block added by KenQ on 2006-05-17
		

		res = SaveEntriesToDB(entryHUID , entryType, appUid);
		//	end of block by bernie ----------------------------------------------------------
		
		if(res != ITV_SUCCESS)
		{
			m_isFullSyncing = false;
			return res;
		}
	}// End of block added by KenQ on 2006-05-18 for support multiple ...
	
	IdsFree(pApps);		//	Free memory

	res = DeleteUnusedObj();

	// update global update stamp if necessary
	if(g_server.CompareStamp(g_CurrUpdateStamp, g_LastUpdateStamp)>0)
	{
		memcpy(g_LastUpdateStamp, g_CurrUpdateStamp, sizeof(g_LastUpdateStamp));
		g_server.SaveLastStamp();
	}

	//////////////////////////////////////////////////////////////////////////
	// Added by Bernie, 2005-Nov-24
	// call Add-in
	if(g_AdiMan.isValid())
	{
		g_AdiMan.SyncEnd();
	}
	// end 
	//////////////////////////////////////////////////////////////////////////

	(*gpDbSyncLog)(Log::L_NOTICE, _T("Leave UpdateAll()"));

	m_isFullSyncing = false;

	SaveMsgtoWorkQueue(L"",L"",0,99,L"",L"");

	return res;
}


/*****************************************************************************
Function Name:	SaveSitesToDB
Arguments:		Site Info (SITEINFO *), Site Number (DWORD)
Returns:		0 (DWORD)
Description:	This function goes through all sites, and for each site, local 
				table SITE is updated according to:
				1. Look for site record per siteuid;
				2. If found, update this record using site info;
				3. If not found, insert a new record using site info.
Called By:		UpdateAll
*****************************************************************************/
DWORD CDSInterface::SaveSitesToDB(SITEINFO *pSites, DWORD dwNum)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement

	CString     name, server;	//	temp string for convert site name and server name into SQL format

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Call SaveSitesToDB to Save %d site"),GetCurrentThreadId(),dwNum);
	for (DWORD i=0; i<dwNum; i++)	//	for each site
	{
		//	Convert site name and server name into SQL format
		name = (pSites+i)->wszName;
		name.Replace(_T("'"),_T( "''"));
		server = (pSites+i)->wszServer;
		server.Replace(_T("'"), _T("''"));

	////	Writing into table SITE
		DBRECORD record,SelectCols;
		DBRESULTSETS resultSets;
		DBFIELD field2,field3,field4;
		
		field2.lType = SQL_C_TCHAR;
		field2.lSize = 64;
		field3.lType = SQL_C_TCHAR;	
		field3.lSize = 256;
		field4.lType = SQL_C_ULONG;
		
		SelectCols.push_back(field2);
		SelectCols.push_back(field3);
		SelectCols.push_back(field4);
		
		//	To check if this site record is in table SITE
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT NAME,SERVERNAME,ZQ_ENTRYSTATE FROM SITE WHERE SITEUID = %hu"), 
				 (pSites+i)->wSiteUid);
		retCode = g_localDB.SelectAllSQL(sql,SelectCols,&resultSets);		
		
		//TO DO:Throw operation into work queue
		if (retCode == SQL_SUCCESS)		//	found in table SITE
		{
			//read DB and compare it with the data from IDS
			wchar_t wsName[255];
			wchar_t wsServer[501];
			DWORD entryState;
			record = resultSets.at(0);

			StringCbCopy(wsName, 255*2, (wchar_t*)record.at(0).pValue);
			StringCbCopy(wsServer, 501*2, (wchar_t*)record.at(1).pValue);
			memcpy(&entryState,record.at(2).pValue,4);
			
			CString str;
			wchar_t tmpStr[1024];
			if(name!= wsName)
			{
				// throw the operation to WORKQUEUE
				StringCbPrintf(tmpStr, 1024*sizeof(SQLTCHAR), _T("NAME = N'%s',"),name);
				str += tmpStr;
			}
			if(server != wsServer)
			{
				// throw the operation to WORKQUEUE
				StringCbPrintf(tmpStr, 1024*2, _T("SERVER = '%s',"),server);
				str += tmpStr;
			}
			if(entryState != 0)
			{
				// throw the operation to WORKQUEUE
				StringCbPrintf(tmpStr, 1024*2,_T("ZQ_ENTRYSTATE = 0,"));
				str += tmpStr;
			}
			if(!str.IsEmpty())
			{
				str = str.Left(str.GetLength()-1);			
			//	Update this record with site info	
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE SITE SET %s WHERE SITEUID = %hu"), //ls to s
					 str, (pSites+i)->wSiteUid);
				retCode = g_localDB.ExecuteSQL(sql);

			}			
		}
		else	//	not found in table SITE
		{
			//	Insert a new record using site info
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO SITE (SITEUID,NAME,SERVERNAME,ZQ_ENTRYSTATE) VALUES (%hu,N'%s',N'%s',0)"), //ls to s
					 (pSites+i)->wSiteUid, name, server);
			retCode = g_localDB.ExecuteSQL(sql);
		}
		g_localDB.FreeResultSets(&resultSets);
	}	//	end for
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Call SaveSitesToDB to Save %d site"),GetCurrentThreadId(),dwNum);
	return 0;	//	success
}



/*****************************************************************************
Function Name:	SaveAppsToDB
Arguments:		App Info (APPNAME *), App Number (DWORD)
Returns:		0 (DWORD)
Description:	This function goes through all applications, and for each 
				application, local table APPLICATION and HIERARCHY is updated
				according to:
				1. Look for application record per applicationuid;
				2. If found, update this record using application info;
				3. If not found, insert a new record using application info.
Called By:		UpdateAll
*****************************************************************************/
DWORD CDSInterface::SaveAppsToDB(APPNAME *pApps, DWORD dwNum)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement

	CString     name;	//	temp string for convert site name into SQL format
	DWORD		appUID;	//	UID of application
	(*gpDbSyncLog)(Log::L_INFO,_T("%dCall SaveAppsToDB to Save %d site"),GetCurrentThreadId(),dwNum);
	for (DWORD i=0; i<dwNum; i++)	//	for each application
	{
		//	Convert application name into SQL format
		name = (pApps+i)->wszName;
		name.Replace(_T("'"), _T("''"));

		// only save the synced applications
		if(DoesSkipApplication((pApps+i)->dwUid, name))
		{
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:SaveAppsToDB does not Save application %s (%ld)"),GetCurrentThreadId(), (LPCTSTR)name, (pApps+i)->dwUid);

			continue;
		}
	////	Writing into table APPLICATION
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:Save application:%s"),GetCurrentThreadId(),(pApps+i)->wszName);

		//	To check if this application record is in table APPLICATION
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT APPLICATIONUID FROM APPLICATION WHERE APPLICATIONUID = %ld"), 
				 (pApps+i)->dwUid);
		retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &appUID, 0);
		
		if (retCode == SQL_SUCCESS)		//	found in table APPLICATION
		{
			//	Update this record with application info
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE APPLICATION SET NAME = N'%s',ZQ_ENTRYSTATE = 0 WHERE APPLICATIONUID = %ld"), //ls to s
					 name, (pApps+i)->dwUid);
			retCode = g_localDB.ExecuteSQL(sql);
			
		}
		else	//	not found in table APPLICATION
		{
			//	Insert a new record using application info
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO APPLICATION (APPLICATIONUID,NAME,ZQ_ENTRYSTATE) VALUES (%ld,N'%s',0)"), //ls to s
					 (pApps+i)->dwUid, name);
			retCode = g_localDB.ExecuteSQL(sql);
		}

	////	Writing into table HIERARCHY

		if ((pApps+i)->dwUid == 0)	//	It is the root record
		{
			//	To check if this root record is in table HIERARCHY
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ENTRYUID FROM HIERARCHY WHERE LOCALHIERARCHYUID = 'H0'"));
			retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &appUID, 0);
			
			if (retCode == SQL_NO_DATA)		//	not found in table HIERARCHY
			{
				//	Insert the root record into table HIERARCHY
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO HIERARCHY (LOCALHIERARCHYUID,HIERARCHYUID,LOCALENTRYUID,ENTRYUID,ENTRYTYPE,ENTRYNAME,ZQ_ENTRYSTATE,LOCALPARENTHUID,PARENTHUID,STATE) VALUES ('%s',%ld,'%s',%ld,%hu,'%s',0,'%s',%ld,%hu)"), //ls to s
			 			 _T("H0"), (pApps+i)->dwUid, _T("H0"), (pApps+i)->dwUid, IDS_APPLICATION, name, _T("-1"), -1, 0);
				retCode = g_localDB.ExecuteSQL(sql);
			}
			else		//	found in table HIERARCHY
			{
				//	Update the state of the root record
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE HIERARCHY SET ENTRYUID = 0, ZQ_ENTRYSTATE = 0  WHERE LOCALHIERARCHYUID = 'H0'"));
				retCode = g_localDB.ExecuteSQL(sql);
				
			}
		}	
		else	//	It is not the root record
			//	Save this hierarchy relationship into Local DB
		// this will be called by application add callback
		{
			// 2007-12-29 by KenQ, originally use application uid as its hierarchy uid. 
			// Correct it by get entry uid by GetEntryHUIDByName. 
			DWORD entryHUID = (pApps+i)->dwUid; 
			DWORD entryType = IDS_APPLICATION;
			CString entryFullName;
			entryFullName = CString("\\") + name + CString("\\");
			
			int nObjCount = GetEntryHUIDByName((LPCTSTR)entryFullName, entryHUID, entryType);
			if(0 == nObjCount)
			{
				(*gpDbSyncLog)(Log::L_ERROR,_T("Specified Application  \"%s\" does not exist in IDS Hierarchy! Use ApplicationID as HierarchyUID"),
					entryFullName);
			}
			else if(nObjCount > 1)
			{
				(*gpDbSyncLog)(Log::L_ERROR,_T("Specified Application  \"%s\" has multiple objects in IDS Hierarchy! Use ApplicationID as HierarchyUID"),
					entryFullName);
			}

			(*gpDbSyncLog)(Log::L_INFO,_T("Application=%s, ApplicationHUID=%d, EntryHUID=%d"), (LPCTSTR)name, (pApps+i)->dwUid, entryHUID);

			retCode = SaveOneHierarchy(entryHUID, IDS_APPLICATION, 
				   (pApps+i)->dwUid, (LPCTSTR)name, 0,IDS_APPLICATION);

		}
	}
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit SaveAppsToDB to Save %d application"),GetCurrentThreadId(),dwNum);
	return 0;	//	success
}


/*****************************************************************************
Function Name:	SaveEntriesToDB
Arguments:		Entry ID (DWORD)
Returns:		0 (DWORD)
Description:	This function is recusive. It goes through the hierarchy tree 
				and gets information of all folders and assets. For each folder
				or asset, updates local table according to:
				1. Update table FOLDER or ASSET;
				2. Update table HIERARCHY;
				3. Read metadata of folder or asset and update corresponding
				local table accordingly;
				4. If this is an asset, read its elements and update corresponding
				local table accordingly.
				
				manullaySync - is for the purpose to sync the specified folder 
				               at running time. 
Called By:		UpdateAll
*****************************************************************************/
DWORD CDSInterface::SaveEntriesToDB(DWORD entryHUID,DWORD entryType, DWORD appuid, DWORD manuallySync)
{
	DWORD	retCode = 0;	//	return code
	
	ENTRY *pEntries = 0;		//	pointer to the structure storing entry info
	DWORD entryNum = 0;			//	number of entries

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Call SaveEntriesToDB to Save entries under %d:%d"),GetCurrentThreadId(),entryType,entryHUID);
	
	//	Get all entries under given folder 
	retCode = IdsListFolderEntriesByUid(&m_idssess, entryHUID, &pEntries, &entryNum, &m_itvsta, 0);
	if (retCode)		//	IDS call fail
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("List the hierarchy under %X Failed:IdsListFolderEntriesByUid 0X%X"),entryHUID,retCode);

		return retCode;		//	Exit
	}

//	----------- modified by KenQ 2006-12-26 for Manually sync specified folder -------------
//  Before process each folder, set its direct child node's EntryState for later check whether they are need to be deleted
	if(manuallySync != 0 && (IDS_FOLDER == entryType || IDS_APPLICATION == entryType) )
	{
		UpdateFolderSubNodeZQEntryState(entryHUID);
	} 

	for (DWORD i=0; i<entryNum; i++)		//	for each entry
	{
		if(entryHUID == 0)// list root
		{
			//get application uid by name
			CString csName = (pEntries+i)->wszName;
			//csName = csName.Right(csName.GetLength() -1);
			TCHAR sql[1024];
			DWORD dwAppUID = 0;			

			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT APPLICATIONUID FROM APPLICATION WHERE NAME = N'%s'"),csName);
			g_localDB.SelectSQL(sql,SQL_C_ULONG,&dwAppUID,0);

			retCode = SaveObjectMdToDB((pEntries+i)->dwHierarchyUid, IDS_APPLICATION);
				//	Write the hierarchy relationship to DB
			retCode = SaveOneHierarchy((pEntries+i)->dwHierarchyUid, IDS_APPLICATION, 
								   dwAppUID, (pEntries+i)->wszName, entryHUID,entryType);
			retCode = SaveEntriesToDB((pEntries+i)->dwHierarchyUid,IDS_APPLICATION);

			if (retCode)		//	recursive call failed, exit
			{
				(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Call SaveEntriesToDB to Save entries under %d:%d"),GetCurrentThreadId(),entryType,entryHUID);
				return retCode;		//	Exit
			}
		}
		else if ((pEntries+i)->dwAssetUid == 0)	//	It's a folder
		{
			CString folderName;
			AdjustRoot((pEntries+i)->wszName, folderName);
			//	Write the folder to DB
			retCode = SaveOneFolder((pEntries+i)->dwHierarchyUid, (LPCTSTR)folderName);

			//	Write the metadata of the folder to DB
			retCode = SaveObjectMdToDB((pEntries+i)->dwHierarchyUid, IDS_FOLDER);

//	---------- modified by KenQ 2006-05-19-------------
//  Commented some lines.
//  To Folder, it parent's is identified by entryHUID and entryTye.
//  Original appUID is to identify where the call coming from: outside or recursive(alway set to 0)
//  To specified folder(NOT ROOT), now the folder(in the folder)'s Hierarchy parent is this folder, 
//  instead of link all these asset to Application. 
//	---------- modified by KenQ 2006-08-03-------------
//  rollback to previous version with condition judgement, 
//	But this version strength the comparing condition cases to support the swith g_SyncedFolderAsNavNode
			int nSyncFolderCount = g_SyncFolders.size();
			if(appuid == 0 || nSyncFolderCount > 1 || (1 == nSyncFolderCount && g_SyncedFolderAsNavNode > 0) )
			{
				//	Write the hierarchy relationship to DB
				retCode = SaveOneHierarchy((pEntries+i)->dwHierarchyUid, IDS_FOLDER, 
										   (pEntries+i)->dwHierarchyUid, (LPCTSTR)folderName, entryHUID,entryType);
			}
			else
			{
				// append the hierarchy directly to the application
				retCode = SaveOneHierarchy((pEntries+i)->dwHierarchyUid, IDS_FOLDER, 
										   (pEntries+i)->dwHierarchyUid, (LPCTSTR)folderName, appuid,IDS_APPLICATION);
			}

			//	Get entries of the folder and write to DB
			retCode = SaveEntriesToDB((pEntries+i)->dwHierarchyUid,IDS_FOLDER, 0, manuallySync);	//	Note: recusive
			if (retCode)		//	recursive call failed, exit
			{
				(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Call SaveEntriesToDB to Save entries under %d:%d"),GetCurrentThreadId(),entryType,entryHUID);
				return retCode;		//	Exit
			}

		}
		else	//It's an asset
		{
			// 2005-3-6  add by kaliven to reduce the load 
			// when restart if the asset is saved to DB, only save the hierarchy.

			CString assetName;
			AdjustRoot((pEntries+i)->wszName, assetName);
			if(gUpdateThreadId != GetCurrentThreadId())
			{
				BOOL bNewAsset = true;

				retCode = SaveOneAsset((pEntries+i)->dwAssetUid, (LPCTSTR)assetName, (pEntries+i)->dwAppUid, bNewAsset);
				//	Write the metadata of the asset to DB
				retCode = SaveObjectMdToDB((pEntries+i)->dwAssetUid, IDS_ASSET, bNewAsset);
				//	Get all members of the asset and write to DB
				retCode = SaveElementsToDB((pEntries+i)->dwAssetUid, bNewAsset);
					//	Write the asset to DB
			}
			else{

				if(!IsAssetUpdated((pEntries+i)->dwAssetUid))
				{
					BOOL bNewAsset = true;

					retCode = SaveOneAsset((pEntries+i)->dwAssetUid, (LPCTSTR)assetName, (pEntries+i)->dwAppUid, bNewAsset);
					//	Write the metadata of the asset to DB
					retCode = SaveObjectMdToDB((pEntries+i)->dwAssetUid, IDS_ASSET, bNewAsset);
					//	Get all members of the asset and write to DB
					retCode = SaveElementsToDB((pEntries+i)->dwAssetUid, bNewAsset);
				}
				else
					int k =0;
			}			
//	---------- modified by KenQ 2006-05-17-------------
//  Commented some lines.
//  To Asset, it parent's is identified by entryHUID and entryTye.
//  To specified folder(NOT ROOT), now the assets(in the folder)'s Hierarchy parent is this folder, 
//  instead of link all these asset to Application. 
//	---------- modified by KenQ 2006-08-03-------------
//  rollback to previous version with condition judgement, 
//	But this version strength the comparing condition cases to support the swith g_SyncedFolderAsNavNode
			int nSyncFolderCount = g_SyncFolders.size();
			if(appuid == 0 || nSyncFolderCount > 1 || (1 == nSyncFolderCount && g_SyncedFolderAsNavNode > 0) )
			{
				//	Write the hierarchy relationship to DB
				retCode = SaveOneHierarchy((pEntries+i)->dwHierarchyUid, IDS_ASSET, 
										 (pEntries+i)->dwAssetUid, (LPCTSTR)assetName, entryHUID,entryType);			
			}
			else
			{
				// append the hierarchy directly to the application
				retCode = SaveOneHierarchy((pEntries+i)->dwHierarchyUid, IDS_ASSET, 
										 (pEntries+i)->dwAssetUid, (LPCTSTR)assetName, appuid,IDS_APPLICATION);			
			}
		}
	}

//	----------- modified by KenQ 2006-12-26 for Manually sync specified folder -------------
//  Before process folder, check its direct child node's EntryState, if it is 1, delete it, including Hierarchy, 
//  Asset(Asset, AssetMD, Complex Asset, AtomicElement), Folder(Folder, FolderMD). To Folder/Asset, we need to 
//  query whether it is linked by others, if yes, does not delete them, if not, we need to delete all related data from LAM.
	if(manuallySync != 0 && (IDS_FOLDER == entryType || IDS_APPLICATION == entryType) )
	{
		ITVSTATUS errCode = ITV_SUCCESS;
		RemoveHierarchyForManaulSync(entryHUID, errCode);
		if(errCode != ITV_SUCCESS)
		{
			return errCode;
		}
	}

	IdsFree(pEntries);	//	Free memory
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Call SaveEntriesToDB to Save entries under %d:%d"),GetCurrentThreadId(),entryType,entryHUID);
	return retCode;	
}

/*****************************************************************************
Function Name:	SaveElementsToDB
Arguments:		Asset ID (DWORD) 
				bNew - only take effective during starting time
Returns:		0 (DWORD) (0:success 1:fail)
Description:	This function is recusive. For one asset, it goes through the 
				complex-asset tree and gets information of all assets and elements. 
				For each asset or element, updates local table according to:
				1. Update table FOLDER or ASSET;
				2. Update table HIERARCHY;
				3. Read metadata of folder or asset and update corresponding
				local table accordingly;
				4. If this is an asset, read its elements and update corresponding
				local table accordingly.
Called By:		SaveEntriesToDB
*****************************************************************************/
DWORD CDSInterface::SaveElementsToDB(DWORD assetUID, BOOL bNew)
{
	DWORD		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement

	COMPLEXASSET *pComplex = 0;	//	pointer to the structure storing member info
	DWORD complexNum = 0;	//	number of members

	WORD  memberState;	//	current state of member
	IDSUIDUPDATESTAMP memberStamp;	//	time stamp info
	//	Get current state of the asset
	retCode = IdsGetAssetState(&m_idssess, assetUID, &memberState, &memberStamp, &m_itvsta, 0);
	if (!retCode)	//	no error occurs
		//	Write the current state of the asset to DB
		retCode = UpdateAssetState(assetUID, memberState);
	else
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("Query the state of %d Failed:IdsGetAssetState ERROR!0X%X"),assetUID,retCode);

		return retCode;
		
	}
	//	Get all members of the asset
	retCode = IdsGetAsset(&m_idssess, assetUID, &memberStamp, &pComplex, &complexNum, &m_itvsta, 0);
	if (retCode)		//	IDS call fail

	{
		(*gpDbSyncLog)(Log::L_INFO, _T("Query the Metadata of Asset %d Failed:!0X%X"),assetUID,retCode);

		return retCode;
	}
	
	DWORD appUID = 0;	//	UID of application of the asset
	//	Get the asset's application UID from local DB
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT APPLICATIONUID FROM ASSET WHERE ASSETUID = %ld"), 
			 assetUID);
	retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &appUID, 0);

	for (DWORD i=0; i<complexNum; i++)	//	for each member
	{
		switch ((pComplex+i)->wType)		//	for member's type, either an asset or an element
		{
		case IDS_ASSET:		//	It's an asset
			//	Write the asset to DB
			{
				BOOL bNewAsset = TRUE;
				retCode = SaveOneAsset((pComplex+i)->dwUid, (pComplex+i)->wszName, appUID, bNewAsset);
				//	Write the metadata of the asset to DB
				retCode = SaveObjectMdToDB((pComplex+i)->dwUid, IDS_ASSET, bNewAsset);
				//	Get members of the asset and write to DB
				retCode = SaveElementsToDB((pComplex+i)->dwUid, bNewAsset);	//	Note: recusive
			}
			break;
		case IDS_ATOMIC_ELEMENT:	//	It's an element
			
			//	Write the element to DB
			retCode = SaveOneElement((pComplex+i)->dwUid, (pComplex+i)->wszName);
			//	Write the metadata of the element to DB
			
			retCode = SaveObjectMdToDB((pComplex+i)->dwUid, IDS_ATOMIC_ELEMENT, bNew);
			
			//	Get current state of the element
			retCode = IdsGetAtomicElementState(&m_idssess, (pComplex+i)->dwUid, &memberState, &memberStamp, &m_itvsta, 0);
			if (!retCode)	//	no error occurs
				//	Write the current state of the element to DB
				retCode = UpdateElementState((pComplex+i)->dwUid, memberState);
			else
			{
				(*gpDbSyncLog)(Log::L_INFO, _T("Query the state of AE %d Failed:IdsGetAtomicElementState 0X%X"),(pComplex+i)->dwUid,retCode);
			}
			// get all clips in the asset  
			// added by kaliven.lee
			retCode = SaveClipsToDB((pComplex+i)->dwUid);
			break;
		default:
			continue;	// it should be either an asset or an element
		}
	}
	
	//	Write the whole complex-asset relationship of the asset to DB

	retCode = SaveOneComplexAsset(assetUID, complexNum, pComplex);
	retCode = SetACFlag(assetUID);
	IdsFree(pComplex);	//	Free memory

	return 0;	//	success
}
/*****************************************************************************
Function Name:	SaveClipsToDB
Arguments:		AtomicElement ID (DWORD) 
Returns:		0 (DWORD) (0:success 1:fail)
Description:	This function is recusive. For one asset, it goes through the 
				complex-asset tree and gets information of all assets and elements. 
				For each asset or element, updates local table according to:
				1. Update table ATOMICELEMENT;
				2. Update table HIERARCHY;
				3. Read metadata of Asset element and update corresponding
				local table accordingly;
				//4. If this is an asset, read its elements and update corresponding
				//local table accordingly.
Called By:		SaveElementsToDB
*****************************************************************************/
DWORD CDSInterface::SaveClipsToDB(DWORD ElementUID)
{
	DWORD		retCode;	//	return code
//	SQLTCHAR		sql[1024];	//	string to put SQL statement

	METADATA_V2 *inMETADATA = new METADATA_V2();	//	pointer to the structure storing member info
	memset(inMETADATA,0,sizeof(METADATA_V2));
	wchar_t wszMetaDataName[] = L"Type";
	inMETADATA->pwszMdName = wszMetaDataName;
	memcpy(&inMETADATA->Version,&m_itvver,sizeof(ITVVERSION));

	DWORD dwInNum = 0;	//	number of members
	CLIPLIST ** clipList = 0;	// list of clips
	DWORD dwOutNum = 0;		// number of output

	WORD  memberState;	//	current state of member
	IDSUIDUPDATESTAMP memberStamp;	//	time stamp info
	//	Get current state of the asset
	retCode = IdsGetAtomicElementState(&m_idssess, ElementUID, &memberState, &memberStamp, &m_itvsta, 0);
	if (!retCode)	//	no error occurs
		//	Write the current state of the asset to DB
		retCode = UpdateElementState(ElementUID, memberState);
	else
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("Query the state of Asset %d Failed:IdsGetAtomicElementState ERROR!0X%X"),ElementUID,retCode);
		return retCode;
	}

	// get the clip list and metadata 
	retCode = IdsListClipAndMd(&m_idssess,ElementUID,&inMETADATA,dwInNum,&clipList,&dwOutNum,&m_itvsta,0);	
	if(retCode)
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("IdsListClipAndMd ERROR!0X%X"),retCode);
		if(inMETADATA)
			delete inMETADATA;

		return retCode;
	}
	
	for (DWORD i =0 ; i < dwOutNum; i++)
	{
		// write clip to DB  
		CLIP * pClip = (*(clipList+i))->pClip;
		retCode = SaveOneClipEx(pClip,ElementUID);
	//	retCode = SaveOneClip(pClip->dwUid, pClip->wszName);
		// write clip metadata to DB 
		retCode = SaveObjectMdToDB(pClip->dwUid,IDS_CLIP);
		// write extra information to DB
		
	}
	IdsFreeClipList(clipList,dwOutNum);
	if(inMETADATA)
		delete inMETADATA;
	return 0;	//	success
}

/*****************************************************************************
Function Name:	SaveEntryMDSelectToDB
Arguments:		Metadata Description Info (METADATADESC *), Metadata Number (DWORD), 
				Entry Type (WORD)
Returns:		0 (DWORD)
Description:	This function goes through all metadata of the Entry Type with selection, 
				and for each such metadata, local table ENTRYMDD and ENTRYMDSELECT are 
				updated according to:
				1. Look for metadata record per metadata name and type from ENTRYMDD;
				2. If not found, insert a new record into ENTYMDD using matadata info, 
				   setting ISSELECT = 1;
				3. If found, update this record in ENTYMDD using matadata info, setting 
				   ISSELECT = 1;
				4. Go through the select entry of the metadata, for each select entry, 
				   insert a new record into ENTRYMDSELECT using the select entry info.
			    Special workflow:
				1. When mdUID is 31(Price category), will update column USERDATA1 and
				   USERDATA2 of table ENTRYMDSELECT  with price and tax of the category.
Called By:		UpdateAll
*****************************************************************************/
DWORD CDSInterface::SaveEntryMDSelectToDB(METADATADESC *pMd, DWORD dwNum, WORD entryType)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement

	//	Write all selection entries to Local DB according to got description info
	for (DWORD i=0; i<dwNum; i++)	//	for each metadata
	{
	//	Updating table ENTRYMDD

		CString mdName, mdDesc;	//	string to store converted metadata name and description
		DWORD mdUID = 0;		//	UID of metadata

		//	Convert metadata name into SQL format
		mdName = (pMd+i)->wszMdName;
		mdName.Replace(_T("'"), _T("''"));
		//	Convert metadata description into SQL format
		mdDesc = (pMd+i)->wszMdDesc;
		mdDesc.Replace(_T("'"), _T("''"));
		DWORD IsSelected;
		if((pMd+i)->wMdListEntries == 0)
			IsSelected = 0;
		else 
			IsSelected = 1;
		DWORD IsMultiple = 0;
		if((pMd+i)->lMdUserData[0] > 1)   // Changed by KenQ on 2007-01-20, 0 also should be regard as single value
		{
			IsMultiple = 1;
		}
		if ((pMd+i)->dwMdAppUid == 0)	//	it's a system metadata
		{
			//	Check if the metadata exists in table ENTRYMDD and get its UID if exists

			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDUID FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu AND APPLICATIONUID = 0"), //ls to s
					 mdName, (pMd+i)->wMdType, entryType);
			retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &mdUID, 0);
		}
		else	//	it's an application specified metadata
		{
			//	Check if the metadata exists in table ENTRYMDD and get its UID if exists
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDUID FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu AND (APPLICATIONUID <> 0 OR APPLICATIONUID IS NULL)"), //ls to s
					 mdName, (pMd+i)->wMdType, entryType);
			retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &mdUID, 0);
		}

		if (retCode == SQL_NO_DATA)	//	not found in table ENTRYMDD
		{
			//	Create a new UID for the metadata
			GetNewLocalUID(IDS_METADATA, &mdUID);

			//	Insert a new record into table ENTRYMDD using metadata info, setting ISSELECT to 1, indicating it has selection entries
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO ENTRYMDD (MDUID,ENTRYTYPE,MDNAME,MDTYPE,MDDESCRIPTION,MDMAXLENGTH,ISSELECT,ZQ_ISINMD,ISMULTIPLE,APPLICATIONUID,ISLOCAL,MDDTYPE,ISVIEW,ISEDIT) VALUES (%ld,%hu,N'%s',%hu,N'%s',%hu,%d,1,%d,%ld,0,1,1,0)"), //ls to s
					 mdUID, entryType, mdName, (pMd+i)->wMdType, mdDesc, (pMd+i)->wMdMaxLength,IsSelected,IsMultiple, (pMd+i)->dwMdAppUid);
			retCode = g_localDB.ExecuteSQL(sql);
		}	
		else	//	found in table ENTRYMDD
		{
			//	Update the record in table ENTRYMDD using metadata info, setting ISSELECT to 1, indicating it has selection entries
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ENTRYMDD SET ISSELECT = %d,ISMULTIPLE =%d, MDDESCRIPTION = N'%s' , MDMAXLENGTH = %hu WHERE MDUID = %ld"), //ls to s
					 IsSelected,IsMultiple,mdDesc, (pMd+i)->wMdMaxLength,mdUID);
			retCode = g_localDB.ExecuteSQL(sql);
		}

//	----------- modified by KenQ 2006-11-22 for KDDI requirement: ReferenceCount and RatingControl -------------
//  Here to get the MDUID of TitleSortName
		if(g_SupportRefcountRatingctrl)
		{
			if(IDS_ASSET == entryType && wcscmp((pMd+i)->wszMdName, MDNAME_ASSET_TITLESORTNAME) == 0)
			{
				(*gpDbSyncLog)(Log::L_NOTICE, _T("TitleSortName MDUID is %d"), mdUID);
				m_titleSortNameMDUID = mdUID;
			}
			else if(IDS_FOLDER == entryType && wcscmp((pMd+i)->wszMdName, MDNAME_RATING) == 0)
			{
				(*gpDbSyncLog)(Log::L_NOTICE, _T("Folder Rating MDUID is %d"), mdUID);
				m_folderRatingMDUID = mdUID;
			}
			else if(IDS_ASSET == entryType && wcscmp((pMd+i)->wszMdName, MDNAME_RATING) == 0)
			{
				(*gpDbSyncLog)(Log::L_NOTICE, _T("Asset Rating MDUID is %d"), mdUID);
				m_assetRatingMDUID = mdUID;
			}
		}

		// Add by KenQ at 2007-2-14, set the ZQ_EntryState to be 1 for later deletion
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ENTRYMDSELECT SET ZQ_EntryState = 1 WHERE MDUID=%d"), mdUID);
		retCode = g_localDB.ExecuteSQL(sql);

	//	Updating table ENTRYMDSELECT

		SELECTENTRY *pSelectionEntryBase;	//	pointer to the structure to store selection entry info
		SELECTENTRY *pSelectionEntryCurrent;	//	pointer to selection entry info
		pSelectionEntryBase = (pMd+i)->pMdSelection;

		for (WORD j=0; j<(pMd+i)->wMdListEntries; j++)	//	for each selection entry within the metadata
		{
			pSelectionEntryCurrent = pSelectionEntryBase+j;
			CString mdDisplay;	//	string to store converted display value of the selection entry
			SQLTCHAR mdUserData1[20]; // String to store user data of select entry.
			SQLTCHAR mdUserData2[20];

			//	Convert display value of the selection entry into SQL format
			mdDisplay = pSelectionEntryCurrent->wszName;
			mdDisplay.Replace(_T("'"), _T("''"));

			int entryState;	//	ZQ_EntryState of the selection
			//	Check whether the selection entry is in table ENTRYMDSELECT
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ZQ_ENTRYSTATE FROM ENTRYMDSELECT WHERE MDUID = %ld AND MDVALUE = %s"),
					 mdUID, _T("%s"));
			retCode = PrepareSelectionSQL(sql, (pMd+i)->wMdType, pSelectionEntryCurrent);
			retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &entryState, 0);

			//Extract special user data from byteUserData[] of the select enrty into user data fields.
			if ((mdUID == MDUID_ASSET_PRICE_CATEGORY)||(mdUID == MDUID_ASSET_VCR_PRICE)) { // If select entry belong to Price category
				IZQ_PRICE_STRUCT *mdPriceCategoryDef;
				// Price category length 24 bytes, and byteUserData has 256 bytes and ITV use 80 bytes (10 doubles). Pointer cast
				// will not access invalid data.
				mdPriceCategoryDef = (IZQ_PRICE_STRUCT*) pSelectionEntryCurrent->byteUserData; 
				StringCbPrintf(mdUserData1, 20*sizeof(SQLTCHAR), _T("%.2f"), mdPriceCategoryDef->price);
				StringCbPrintf(mdUserData2, 20*sizeof(SQLTCHAR), _T("%.2f"), mdPriceCategoryDef->tax);
			} else {
				StringCbPrintf(mdUserData1, 20*sizeof(SQLTCHAR), _T(""));
				StringCbPrintf(mdUserData2, 20*sizeof(SQLTCHAR), _T(""));
			}

			if (retCode == SQL_SUCCESS)	//	found in ENTRYMDSELECT
			{
				//	Restore ZQ_ENTRYSTATE for the selection entry, if it's not a deleted one

				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ENTRYMDSELECT SET MDDISPLAY = N'%s', USERDATA1 = '%s', USERDATA2 = '%s',ZQ_ENTRYSTATE = 0 WHERE MDUID = %ld AND MDVALUE = %s "),
						 mdDisplay, mdUserData1, mdUserData2, mdUID, _T("%s"));
				retCode = PrepareSelectionSQL(sql, (pMd+i)->wMdType, pSelectionEntryCurrent);
				retCode = g_localDB.ExecuteSQL(sql);
			}
			else	//	not found in ENTRYMDSELECT
			{
				//	Insert a new record into table ENTRYMDSELECT using the selection entry info
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO ENTRYMDSELECT (MDUID,MDVALUE,MDDISPLAY,ZQ_ENTRYSTATE, USERDATA1, USERDATA2) VALUES (%ld,%s,N'%s',0, N'%s', N'%s')"), //ls to s
						 mdUID, _T("%s"), mdDisplay, mdUserData1, mdUserData2);
				retCode = PrepareSelectionSQL(sql, (pMd+i)->wMdType, pSelectionEntryCurrent);
				retCode = g_localDB.ExecuteSQL(sql);
			}

		}	// end of j

		// Add by KenQ at 2007-2-14, set the ZQ_EntryState to be 1 for later deletion
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE ENTRYMDSELECT WHERE MDUID=%d AND ZQ_EntryState = 1 "), mdUID);
		retCode = g_localDB.ExecuteSQL(sql);

		//////////////////////////////////////////////////////////////////////////
		// Added by Bernie, 2006-Feb-5
		// call Add-in
//		if(g_AdiMan.isValid() && gUpdateThreadId != GetCurrentThreadId())
		if(g_AdiMan.isValid() && !m_isFullSyncing)
		{
			DA_mddListDb	mddBlock;
			
			mddBlock._dwMdUID = mdUID;
			mddBlock._dwAppUID = (pMd+i)->dwMdAppUid;
			StringCbCopyW(mddBlock._szMdName, DBSA_MAXMDNAME*sizeof(wchar_t), (pMd+i)->wszMdName);
			
			g_AdiMan.TrggMdd(&mddBlock);
		}
		// end 
		//////////////////////////////////////////////////////////////////////////

	}	// end of i

	return 0;	//	success
}

DWORD CDSInterface::SaveEntryMDSelectToDBEx(METADATADESC mdd, WORD entryType)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement

//	Updating table ENTRYMDD

	CString mdName, mdDesc;	//	string to store converted metadata name and description
	DWORD mdUID = 0;		//	UID of metadata

	//	Convert metadata name into SQL format
	mdName = mdd.wszMdName;
	mdName.Replace(_T("'"), _T("''"));
	//	Convert metadata description into SQL format
	mdDesc = mdd.wszMdDesc;
	mdDesc.Replace(_T("'"), _T("''"));
	DWORD IsSelected;
	if(mdd.wMdListEntries == 0)
		IsSelected = 0;
	else 
		IsSelected = 1;
	DWORD IsMultiple = 0;
	if(mdd.lMdUserData[0] > 1)   // Changed by KenQ on 2007-01-20, 0 also should be regard as single value
	{
		IsMultiple = 1;
	}
	if (mdd.dwMdAppUid == 0)	//	it's a system metadata
	{
		//	Check if the metadata exists in table ENTRYMDD and get its UID if exists

		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDUID FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu AND APPLICATIONUID = 0"), //ls to s
				 mdName, mdd.wMdType, entryType);
		retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &mdUID, 0);
	}
	else	//	it's an application specified metadata
	{
		//	Check if the metadata exists in table ENTRYMDD and get its UID if exists
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDUID FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu AND (APPLICATIONUID <> 0 OR APPLICATIONUID IS NULL)"), //ls to s
				 mdName, mdd.wMdType, entryType);
		retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &mdUID, 0);
	}

	if (retCode == SQL_NO_DATA)	//	not found in table ENTRYMDD
	{
		//	Create a new UID for the metadata
		GetNewLocalUID(IDS_METADATA, &mdUID);

		//	Insert a new record into table ENTRYMDD using metadata info, setting ISSELECT to 1, indicating it has selection entries
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO ENTRYMDD (MDUID,ENTRYTYPE,MDNAME,MDTYPE,MDDESCRIPTION,MDMAXLENGTH,ISSELECT,ZQ_ISINMD,ISMULTIPLE,APPLICATIONUID,ISLOCAL,MDDTYPE,ISVIEW,ISEDIT) VALUES (%ld,%hu,N'%s',%hu,N'%s',%hu,%d,1,%d,%ld,0,1,1,0)"), //ls to s
				 mdUID, entryType, mdName, mdd.wMdType, mdDesc, mdd.wMdMaxLength,IsSelected,IsMultiple, mdd.dwMdAppUid);
		retCode = g_localDB.ExecuteSQL(sql);
	}	
	else	//	found in table ENTRYMDD
	{
		//	Update the record in table ENTRYMDD using metadata info, setting ISSELECT to 1, indicating it has selection entries
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ENTRYMDD SET ISSELECT = %d,ISMULTIPLE =%d, MDDESCRIPTION = N'%s' , MDMAXLENGTH = %hu WHERE MDUID = %ld"), //ls to s
				 IsSelected,IsMultiple,mdDesc, mdd.wMdMaxLength,mdUID);
		retCode = g_localDB.ExecuteSQL(sql);
	}

//	----------- modified by KenQ 2006-11-22 for KDDI requirement: ReferenceCount and RatingControl -------------
//  Here to get the MDUID of TitleSortName
	if(g_SupportRefcountRatingctrl)
	{
		if(IDS_ASSET == entryType && wcscmp(mdd.wszMdName, MDNAME_ASSET_TITLESORTNAME) == 0)
		{
			(*gpDbSyncLog)(Log::L_NOTICE, _T("TitleSortName MDUID is %d"), mdUID);
			m_titleSortNameMDUID = mdUID;
		}
		else if(IDS_FOLDER == entryType && wcscmp(mdd.wszMdName, MDNAME_RATING) == 0)
		{
			(*gpDbSyncLog)(Log::L_NOTICE, _T("Folder Rating MDUID is %d"), mdUID);
			m_folderRatingMDUID = mdUID;
		}
		else if(IDS_ASSET == entryType && wcscmp(mdd.wszMdName, MDNAME_RATING) == 0)
		{
			(*gpDbSyncLog)(Log::L_NOTICE, _T("Asset Rating MDUID is %d"), mdUID);
			m_assetRatingMDUID = mdUID;
		}
	}

	// Add by KenQ at 2007-2-14, set the ZQ_EntryState to be 1 for later deletion
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ENTRYMDSELECT SET ZQ_EntryState = 1 WHERE MDUID=%d"), mdUID);
	retCode = g_localDB.ExecuteSQL(sql);

//	Updating table ENTRYMDSELECT

	SELECTENTRY *pSelectionEntryBase;	//	pointer to the structure to store selection entry info
	SELECTENTRY *pSelectionEntryCurrent;	//	pointer to selection entry info
	pSelectionEntryBase = mdd.pMdSelection;

	for (WORD j=0; j<mdd.wMdListEntries; j++)	//	for each selection entry within the metadata
	{
		pSelectionEntryCurrent = pSelectionEntryBase+j;
		CString mdDisplay;	//	string to store converted display value of the selection entry
		SQLTCHAR mdUserData1[20]; // String to store user data of select entry.
		SQLTCHAR mdUserData2[20];

		//	Convert display value of the selection entry into SQL format
		mdDisplay = pSelectionEntryCurrent->wszName;
		mdDisplay.Replace(_T("'"), _T("''"));

		int entryState;	//	ZQ_EntryState of the selection
		//	Check whether the selection entry is in table ENTRYMDSELECT
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ZQ_ENTRYSTATE FROM ENTRYMDSELECT WHERE MDUID = %ld AND MDVALUE = %s"),
				 mdUID, _T("%s"));
		retCode = PrepareSelectionSQL(sql, mdd.wMdType, pSelectionEntryCurrent);
		retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &entryState, 0);

		//Extract special user data from byteUserData[] of the select enrty into user data fields.
		if ((mdUID == MDUID_ASSET_PRICE_CATEGORY)||(mdUID == MDUID_ASSET_VCR_PRICE)) { // If select entry belong to Price category
			IZQ_PRICE_STRUCT *mdPriceCategoryDef;
			// Price category length 24 bytes, and byteUserData has 256 bytes and ITV use 80 bytes (10 doubles). Pointer cast
			// will not access invalid data.
			mdPriceCategoryDef = (IZQ_PRICE_STRUCT*) pSelectionEntryCurrent->byteUserData; 
			StringCbPrintf(mdUserData1, 20*sizeof(SQLTCHAR), _T("%.2f"), mdPriceCategoryDef->price);
			StringCbPrintf(mdUserData2, 20*sizeof(SQLTCHAR), _T("%.2f"), mdPriceCategoryDef->tax);
		} else {
			StringCbPrintf(mdUserData1, 20*sizeof(SQLTCHAR), _T(""));
			StringCbPrintf(mdUserData2, 20*sizeof(SQLTCHAR), _T(""));
		}

		if (retCode == SQL_SUCCESS)	//	found in ENTRYMDSELECT
		{
			//	Restore ZQ_ENTRYSTATE for the selection entry, if it's not a deleted one

			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ENTRYMDSELECT SET MDDISPLAY = N'%s', USERDATA1 = '%s', USERDATA2 = '%s',ZQ_ENTRYSTATE = 0 WHERE MDUID = %ld AND MDVALUE = %s "),
					 mdDisplay, mdUserData1, mdUserData2, mdUID, _T("%s"));
			retCode = PrepareSelectionSQL(sql, mdd.wMdType, pSelectionEntryCurrent);
			retCode = g_localDB.ExecuteSQL(sql);
		}
		else	//	not found in ENTRYMDSELECT
		{
			//	Insert a new record into table ENTRYMDSELECT using the selection entry info
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO ENTRYMDSELECT (MDUID,MDVALUE,MDDISPLAY,ZQ_ENTRYSTATE, USERDATA1, USERDATA2) VALUES (%ld,%s,N'%s',0, N'%s', N'%s')"), //ls to s
					 mdUID, _T("%s"), mdDisplay, mdUserData1, mdUserData2);
			retCode = PrepareSelectionSQL(sql, mdd.wMdType, pSelectionEntryCurrent);
			retCode = g_localDB.ExecuteSQL(sql);
		}

	}	// end of j

	// Add by KenQ at 2007-2-14, set the ZQ_EntryState to be 1 for later deletion
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE ENTRYMDSELECT WHERE MDUID=%d AND ZQ_EntryState = 1 "), mdUID);
	retCode = g_localDB.ExecuteSQL(sql);

	//////////////////////////////////////////////////////////////////////////
	// Added by Bernie, 2006-Feb-5
	// call Add-in
//		if(g_AdiMan.isValid() && gUpdateThreadId != GetCurrentThreadId())
	if(g_AdiMan.isValid() && !m_isFullSyncing)
	{
		DA_mddListDb	mddBlock;
		
		mddBlock._dwMdUID = mdUID;
		mddBlock._dwAppUID = mdd.dwMdAppUid;
		StringCbCopyW(mddBlock._szMdName, DBSA_MAXMDNAME*sizeof(wchar_t), mdd.wszMdName);
		
		g_AdiMan.TrggMdd(&mddBlock);
	}
	// end 
	//////////////////////////////////////////////////////////////////////////
	return 0;	//	success
}
/*****************************************************************************
Function Name:	SaveOneHierarchy
Arguments:		Hierarchy ID (DWORD), Entry Type (WORD), Entry ID (DWORD),
				Entry Name (WCHAR *), Parent ID (DWORD)
Returns:		0 (DWORD) (0:success 1:fail)
Description:	This function is for one hierarchy, 
				Local table HIERARCHY is updated according to:
				1. Get local UID and itv name of the entry and local UID of the 
				   parent from corresponding table;
				2. Look for hierarchy record in table HIERARCHY by hierarchy ID;
				3. If found, update this record using hierarchy info;
				4. If not found, look for hierarchy record in table HIERARCHY by 
				   entry itv name and parent uid;
				5. If found, update this record using hierarchy info;
				6. If not found, generate a new local hierarchy ID and insert a 
				   new record using hierarchy info.
				7. Finally, restore ZQ_ENTRYSTATE if it's not a deleted one
*****************************************************************************/
DWORD CDSInterface::SaveOneHierarchy(DWORD hierarchyUID, WORD entryType, DWORD entryUID, const WCHAR *entryName, DWORD parentUID,DWORD ParentType)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024] = _T("");	//	string to put SQL statement

	SQLTCHAR hierarchyLUID[21] = _T("");	//	Local UID of hierarchy
	SQLTCHAR entryLUID[21] =_T("");	//	Local UID of entry
	SQLTCHAR entryLName[MAXNAME] = _T("");	//	local name of entry
	SQLTCHAR parentLUID[21] = _T("");	//	Local UID of parent of the entry
	DWORD  assetState = 0;	//	current state of asset

	
	
	//	Convert itv name of entry into SQL format
	CString	itvName = entryName;	//	string to store converted itv name of entry
	itvName.Replace(_T("'"), _T("''"));	

	DBRESULTSETS resultset;
	DBRECORD SelectRecord,record;
	DBFIELD field1,field2,field3,field4,field5;
	

	
	//	Get Local UID of the entry according to its object type
	switch (entryType)	//	chose the object type of the entry
	{
	case IDS_APPLICATION:	//	It's an application
		
		StringCbPrintf(entryLUID, 21*sizeof(SQLTCHAR), _T("%ld"), entryUID);	//	application has no local UID, so set it the same as UID
		StringCbPrintf(entryLName, MAXNAME*sizeof(SQLTCHAR), _T("%s"), itvName);		//	application has no local name, so set it the same as itv name
	
		break;

	case IDS_FOLDER:	//	It's a folder
	
		//	Get Local UID of the folder
		field1.lType = SQL_C_TCHAR;
		field1.lSize = 64;
		field2.lType = SQL_C_TCHAR;
		field2.lSize = 256;
		SelectRecord.push_back(field1);
		SelectRecord.push_back(field2);
		
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALFOLDERUID,NAME FROM FOLDER WHERE FOLDERUID = %ld"), 
				 entryUID);
		retCode = g_localDB.SelectAllSQL(sql,SelectRecord,&resultset);
		if(retCode == SQL_SUCCESS)
		{
			record = resultset.at(0);
			StringCbCopyW(entryLUID, 21*sizeof(SQLTCHAR), (wchar_t*)record.at(0).pValue);
			StringCbCopyW(entryLName,MAXNAME*sizeof(SQLTCHAR), (wchar_t*)record.at(1).pValue);
		}

		g_localDB.FreeResultSets(&resultset);
		resultset.clear();

		break;
	case IDS_ASSET:	//	It's an asset
		//	Get Local UID of the asset
		
		field1.lType = SQL_C_TCHAR;
		field1.lSize = 64;
		field2.lType = SQL_C_TCHAR;
		field2.lSize = 256;
		field3.lType = SQL_C_ULONG;
		SelectRecord.push_back(field1);
		SelectRecord.push_back(field2);
		SelectRecord.push_back(field3);
		

			
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALASSETUID,NAME,STATE FROM ASSET WHERE ASSETUID = %ld"), 
				 entryUID);
		retCode = g_localDB.SelectAllSQL(sql,SelectRecord,&resultset);
		if(retCode == SQL_SUCCESS)
		{
			record = resultset.at(0);
			StringCbCopyW(entryLUID, 21*sizeof(SQLTCHAR), (wchar_t*)record.at(0).pValue);
			StringCbCopyW(entryLName, MAXNAME*sizeof(SQLTCHAR), (wchar_t*)record.at(1).pValue);
			memcpy(&assetState,record.at(2).pValue,4);
			g_localDB.FreeResultSets(&resultset);
		}
		SelectRecord.clear();
		break;
	default:
		break;
	}
	CString	name = entryLName;	//	string to store converted local name of entry	
	name.Replace(_T("'"), _T("''"));	
	//	Get Local UID of the parent of the entry
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALHIERARCHYUID FROM HIERARCHY WHERE HIERARCHYUID = %ld AND ENTRYTYPE = %ld"), 
			 parentUID,ParentType);
	retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, parentLUID, sizeof(parentLUID));

	//	Check whether the entry exists in table HIERARCHY by hierarchy UID and get its Local UID if exists
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALHIERARCHYUID FROM HIERARCHY WHERE HIERARCHYUID = %ld AND ENTRYTYPE = %ld"), 
			 hierarchyUID , entryType);
	retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, hierarchyLUID, sizeof(hierarchyLUID));
	if(retCode == SQL_NO_DATA)// if can not found by hierarchy UID perhaps it is add by ZQ AM UI should check it by name 
	{
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALHIERARCHYUID FROM HIERARCHY WHERE LOCALPARENTHUID = '%s' AND ENTRYNAME = N'%s' AND ENTRYTYPE = %ld"), 
			parentLUID, itvName , entryType);
		retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, hierarchyLUID, sizeof(hierarchyLUID));
		
		//////////////////////////////////////////////////////////////////////////
		// modified by Bernie Zhao, 2005/July/5
		// If the asset is added by LAM UI, no HierarchyUID available
		// this code block is to add it.

		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE HIERARCHY SET HIERARCHYUID=%ld WHERE LOCALHIERARCHYUID='%s'"),
			hierarchyUID, hierarchyLUID);
		DWORD re = g_localDB.ExecuteSQL(sql);
		//////////////////////////////////////////////////////////////////////////
		
	}
	

	if (retCode == SQL_SUCCESS)	//	found in table HIERARCHY
	{
		//	Update the hierarchy relationship in table HIERARCHY
		DBRESULTSETS resultSets;
		DBRECORD selectCol;
		field1.lType = SQL_C_TCHAR;
		field1.lSize = 64;
		field2.lType = SQL_C_ULONG;
		field3.lType = SQL_C_TCHAR;
		field3.lSize = 256;
		field4.lType = SQL_C_TCHAR;
		field4.lSize = 64;
		field5.lType = SQL_C_ULONG;
		selectCol.push_back(field1);
		selectCol.push_back(field2);
		selectCol.push_back(field3);
		selectCol.push_back(field4);
		selectCol.push_back(field5);

		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALENTRYUID , ENTRYUID, ENTRYNAME, LOCALPARENTHUID, PARENTHUID FROM HIERARCHY WHERE HIERARCHYUID = %ld AND ENTRYTYPE = %ld"), //ls to s
				 hierarchyUID,entryType);

		retCode = g_localDB.SelectAllSQL(sql,selectCol,&resultSets);
		
		WCHAR OldEntryLUID[21];
		WCHAR OldName[256];
		WCHAR OldParentLUID[21];
		DWORD OldEntryUID,OldParentUID;
		if(resultSets.size() != 0)
		{
			record = resultSets.at(0);
			StringCbCopyW(OldEntryLUID, 21*sizeof(WCHAR), (wchar_t*)record.at(0).pValue);
			memcpy(&OldEntryUID,record.at(1).pValue,4);
		
			StringCbCopyW(OldName, 256*sizeof(WCHAR), (wchar_t*)record.at(2).pValue);
			StringCbCopyW(OldParentLUID, 21*sizeof(WCHAR), (wchar_t*)record.at(3).pValue);
			memcpy(&OldParentUID,record.at(4).pValue,4);
		}
		
		//empty the vector and free memory
		g_localDB.FreeResultSets(&resultSets);
		selectCol.clear();

		CString str;
		WCHAR tmpStr[512];
		if(wcscmp(OldEntryLUID,entryLUID) != 0)
		{

			// TODO: throw hierarchy update operation to WorkQueue
			StringCbPrintf(tmpStr, 512*sizeof(WCHAR), L"LOCALENTRYUID = '%s',",entryLUID);
			str += tmpStr;
		}
		if( OldEntryUID!= entryUID)
		{
			// TODO: throw hierarchy update operation to WorkQueue
			StringCbPrintf(tmpStr, 512*sizeof(WCHAR), L"ENTRYUID = %ld,",entryUID);
			str += tmpStr;
		}
		if(wcsicmp(OldName,name) != 0)
		{
			// TODO: throw hierarchy update operation to WorkQueue
			StringCbPrintf(tmpStr, 512*sizeof(WCHAR), L"ENTRYNAME = N'%s',",name);
			str += tmpStr;
		}
		if(wcscmp(parentLUID,OldParentLUID)!= 0)
		{
			// TODO: throw hierarchy update operation to WorkQueue
			// unlink and link operation
			if(entryType == IDS_ASSET)
			{
				SaveMsgtoWorkQueue(entryLUID,OldParentLUID,IDS_ASSET,WORKQUEUE_UNLINK,L"",L"");
				SaveMsgtoWorkQueue(entryLUID,parentLUID,IDS_ASSET,WORKQUEUE_LINK,L"",L"");
			}
			else if(entryType == IDS_FOLDER)
			{
				if(IsPackaged(entryLUID) == FALSE)
				{
					//SaveMsgtoWorkQueue(entryLUID,OldParentLUID,IDS_FOLDER,WORKQUEUE_DELETE,L"",L"");
					//SaveMsgtoWorkQueue(entryLUID,parentLUID,IDS_FOLDER,WORKQUEUE_ADD,L"",L"");
				}else
				{
					SaveMsgtoWorkQueue(entryLUID,OldParentLUID,IDS_PACKAGE,WORKQUEUE_UNLINK,L"",L"");
					SaveMsgtoWorkQueue(entryLUID,parentLUID,IDS_PACKAGE,WORKQUEUE_LINK,L"",L"");
				}
				
			}
			StringCbPrintf(tmpStr, 512*sizeof(WCHAR), L"LOCALPARENTHUID = '%s',",parentLUID);
			str += tmpStr;
		}
		if(parentUID!= OldParentUID)
		{			
			// unlink and link operation 
			// from the point of view of ITV
			StringCbPrintf(tmpStr, 512*sizeof(WCHAR), L"PARENTHUID = %ld,",parentUID);
			str += tmpStr;
		}
		
		if(str.IsEmpty() == FALSE)
		{	
			str = str.Left(str.GetLength() -1);		
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE HIERARCHY SET %s WHERE HIERARCHYUID = %ld  AND ENTRYTYPE= %ld"), //ls to s
				 str, hierarchyUID,entryType);
			retCode = g_localDB.ExecuteSQL(sql);

//	----------- modified by KenQ 2006-11-22 for KDDI requirement: ReferenceCount and RatingControl -------------
//  This case of Asset Link/Unlink operation
//  This is the case move the existing asset to a new folder, so its case of Hierarchy Link/Unlink
//  **** I don't know why the WQ operation is executed before Updating Hierarchy... ANY special reason? Who Knows.
			if(g_SupportRefcountRatingctrl && entryType == IDS_ASSET)
			{
				(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s was moved from folder %s to folder %s, need to recalculate Rating and ReferenceCount"), entryLUID, OldParentLUID, parentLUID);
				AssetRatingRef(entryLUID, m_titleSortNameMDUID, true);
			}
		}
	}
	else	//	not found in table HIERARCHY
	{
		//prepare the argument
		DBRESULTSETS resultsets;
		DBRECORD record,SelectCols;
		DBFIELD field1,field2,field3,field4,field5,field6;
		field1.lType = SQL_C_TCHAR;
		field1.lSize = 64;
		field2.lType = SQL_C_ULONG;
		field3.lType = SQL_C_TCHAR;
		field3.lSize = 64;
		field4.lType = SQL_C_ULONG;
		field5.lType = SQL_C_TCHAR;
		field5.lSize = 64;
		field6.lType = SQL_C_ULONG;
		SelectCols.push_back(field1);
		SelectCols.push_back(field2);
		SelectCols.push_back(field3);
		SelectCols.push_back(field4);
		SelectCols.push_back(field5);
		SelectCols.push_back(field6);

		//	Check whether the entry exists in table HIERARCHY by entry name and type and local parent UID, and get its Local UID if exists
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT LOCALHIERARCHYUID,HIERARCHYUID,LOCALENTRYUID,ENTRYUID,LOCALPARENTHUID,PARENTHUID FROM HIERARCHY WHERE ENTRYNAME = N'%s' AND ENTRYTYPE = %lu AND LOCALPARENTHUID = '%s'"),
			name,entryType,parentLUID);
		retCode = g_localDB.SelectAllSQL(sql,SelectCols,&resultsets);
/*
		//	Check whether the entry exists in table HIERARCHY by entry name and type and local parent UID, and get its Local UID if exists
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALHIERARCHYUID FROM HIERARCHY WHERE ENTRYNAME = '%s' AND ENTRYTYPE = %lu AND LOCALPARENTHUID = '%s'"), 
				 name, entryType, parentLUID);
		retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, hierarchyLUID, sizeof(hierarchyLUID));
*/
		if (retCode == SQL_SUCCESS)	//	found in table HIERARCHY
		{
			// compare the data with the data from IDS
			//	Upadte the hierarchy relationship in table HIERARCHY
			// retrieve data
			record = resultsets.at(0);
			DWORD OldHUID,OldEUID,OldParentHUID;
			WCHAR OldELUID[64];
			WCHAR OldParentLUID[64];

			memcpy(&OldHUID,record.at(1).pValue,4);
			StringCbCopyW(OldELUID, 64*sizeof(WCHAR), (WCHAR*)record.at(2).pValue);
			memcpy(&OldEUID,record.at(3).pValue,4);
			StringCbCopyW(OldParentLUID, 64*sizeof(WCHAR), (WCHAR*)record.at(4).pValue);
			memcpy(&OldParentHUID,record.at(5).pValue,4);
			
			//release memory
			g_localDB.FreeResultSets(&resultsets);
			resultsets.clear();
			// compare data
			CString str;
			WCHAR tmpStr[1024];
			if(OldHUID != hierarchyUID)
			{
				// 
				StringCbPrintf(tmpStr, 1024*sizeof(WCHAR), L"HIERARCHYUID = %ld,",hierarchyUID);
				str += tmpStr;
			}
			if(wcscmp(OldELUID,entryLUID) != 0)
			{
				StringCbPrintf(tmpStr, 1024*sizeof(WCHAR), L"LOCALENTRYUID = '%s',",entryLUID);
				str += tmpStr;
			}
			if(OldEUID != entryUID)
			{
				StringCbPrintf(tmpStr, 1024*sizeof(WCHAR), L"ENTRYUID = %ld,",entryUID);
				str += tmpStr;
			}
			if(wcscmp(OldParentLUID,parentLUID)!= 0)
			{
				if(entryType == IDS_ASSET)
				{
					SaveMsgtoWorkQueue(entryLUID,OldParentLUID,IDS_ASSET,WORKQUEUE_UNLINK,L"",L"");
					SaveMsgtoWorkQueue(entryLUID,parentLUID,IDS_ASSET,WORKQUEUE_LINK,L"",L"");
				}
				else if(entryType == IDS_FOLDER)
				{
					if(!IsPackaged(entryLUID))
					{
//						SaveMsgtoWorkQueue(entryLUID,OldParentLUID,IDS_FOLDER,WORKQUEUE_DELETE,L"",L"");
//						SaveMsgtoWorkQueue(entryLUID,parentLUID,IDS_ASSET,WORKQUEUE_ADD,L"",L"");
					}else
					{
						SaveMsgtoWorkQueue(entryLUID,OldParentLUID,IDS_PACKAGE,WORKQUEUE_UNLINK,L"",L"");
						SaveMsgtoWorkQueue(entryLUID,parentLUID,IDS_PACKAGE,WORKQUEUE_LINK,L"",L"");
					}
					
				}
				StringCbPrintf(tmpStr, 1024*sizeof(WCHAR), L"LOCALPARENTHUID = '%s',",parentLUID);
				str += tmpStr;
			}
			if(OldParentHUID != parentUID)
			{
				
				StringCbPrintf(tmpStr, 1024*sizeof(WCHAR), L"PARENTHUID = %ld,",hierarchyUID);
				str += tmpStr;
			}
			if(!str.IsEmpty())
			{
				str = str.Left(str.GetLength() -1);
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE HIERARCHY SET %s WHERE ENTRYNAME = N'%s' AND ENTRYTYPE = %lu AND LOCALPARENTHUID = '%s'"), //ls to s
					 str, name, entryType, parentLUID);
				retCode = g_localDB.ExecuteSQL(sql);

	//	----------- modified by KenQ 2006-11-22 for KDDI requirement: ReferenceCount and RatingControl -------------
	//  This case of Asset Link/Unlink operation
	//  This is the case move the existing asset to a new folder, so its case of Hierarchy Link/Unlink
    //  But in the test, the move/copy operation on ITV AM will come here. Anyway, left the coding here, 
    //  who knows when will happen.
    //  **** I don't know why the WQ operation is executed before Updating Hierarchy... ANY special reason? Who Knows.
				if(g_SupportRefcountRatingctrl && entryType == IDS_ASSET)
				{
					(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s was moved from folder %s to folder %s, need to recalculate Rating and ReferenceCount"), entryLUID, OldParentLUID, parentLUID);
					AssetRatingRef(entryLUID, m_titleSortNameMDUID, true);
				}
			}
				
		}
		else	//	not found in table HIERARCHY, it's a new hierarchy
		{
			//	Generate a new Local hierarchy UID
	
			DWORD tempLUID = 0;	//	temp UID created from DB sequence
			GetNewLocalUID(IDS_HIERARCHY, &tempLUID);
			StringCbPrintf(hierarchyLUID,  21*sizeof(WCHAR), _T("H%ld"), tempLUID);
			if(entryType == IDS_ASSET)
			{
				SaveMsgtoWorkQueue(entryLUID,parentLUID,IDS_ASSET,WORKQUEUE_LINK,L"",L"");
			}
			else if(entryType == IDS_FOLDER)
			{
				if(IsPackaged(entryLUID))
					SaveMsgtoWorkQueue(entryLUID,parentLUID,IDS_PACKAGE,WORKQUEUE_LINK,L"",L"");
			}
		
			//	Insert a new record of the hierarchy relationship into table HIERARCHY
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO HIERARCHY (LOCALHIERARCHYUID,HIERARCHYUID,LOCALENTRYUID,ENTRYUID,ENTRYTYPE,ENTRYNAME,ZQ_ENTRYSTATE,LOCALPARENTHUID,PARENTHUID,STATE) VALUES (N'%s',%ld,N'%s',%ld,%hu,N'%s',0,N'%s',%ld,%hu)"), //ls to s
			 		 hierarchyLUID, hierarchyUID, entryLUID, entryUID, entryType, name, parentLUID, parentUID, assetState);
			retCode = g_localDB.ExecuteSQL(sql);

//	----------- modified by KenQ 2006-11-22 for KDDI requirement: ReferenceCount and RatingControl -------------
//  This case of Asset Link/Unlink operation
//  This is the case copy the existing asset new Asset is added to Hierarchy, so its case of Hierarchy Link
			if(g_SupportRefcountRatingctrl)
			{
				SQLTCHAR hierarchyLUIDTemp[21] = _T("");
				// here source block is to know whether it is copying or new asset
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALHIERARCHYUID FROM HIERARCHY WHERE EntryUID = %ld AND ENTRYTYPE = %ld AND EntryName = N'%s'"), 
						 entryUID, entryType, entryName);
				retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, hierarchyLUIDTemp, sizeof(hierarchyLUIDTemp));
				
				if(retCode == SQL_SUCCESS)
				{
					// it's a adding opertion
					(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s was added to Hierarchy, need to recalculate Rating and ReferenceCount"), entryLUID);
					AssetRatingRef(entryLUID, m_titleSortNameMDUID, true);
				}
			}
		}
	}

	//	Restore ZQ_ENTRYSTATE for the hierarchy, if it's not a deleted one
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE HIERARCHY SET ZQ_ENTRYSTATE = 0 WHERE HIERARCHYUID = %ld AND ZQ_ENTRYSTATE = 1"),
			 hierarchyUID);
	retCode = g_localDB.ExecuteSQL(sql);
	
	return 0;	//	success
}


/*****************************************************************************
Function Name:	SaveOneComplexAsset
Arguments:		Asset ID (DWORD), Complex Member Number (DWORD),
				Complex Member Info (COMPLEXASSET)
Returns:		0 (DWORD) (0:success 1:fail)
Description:	This function is for one complex asset, 
				Local table COMPLEXASSET is updated according to:
				1. Disable all member of the asset;
				2. Go through the structure of complex elements of the asset;
				3. Search the local member ID in ASSET or ATOMICELEMENT according to
				   the member is an asset or an element;
				4. Insert a new record into COMPLEXASSET table using omplex member info;
				5. For each disabled member, check whether it is still used by others, 
				   and delete it and its metadata if not used any more;
				6. Delete all disabled member of the asset from COMPLEXASSET table;
				7. Call a stored procudure to update entrystate of the asset, according 
				   to entrystates of its members.
*****************************************************************************/
DWORD CDSInterface::SaveOneComplexAsset(DWORD assetUID, DWORD complexNum, COMPLEXASSET *pComplex)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement
	SQLSMALLINT	rollbackFlag = SQL_COMMIT;	//	Flag to indicate rollback
	RETCODE		outCode = SQL_COMMIT;	//	return code of the whole function

	///////////////////////////////////////////update complexasset table
	
	// SQLTCHAR assetLUID[21];	//	Local UID of asset
	SQLTCHAR assetLUID[128];	//	Local UID of asset
	// SQLTCHAR memberLUID[21];	//	Local UID of member
	SQLTCHAR memberLUID[128] = _T("");	//	Local UID of member
	DWORD threadid = GetCurrentThreadId();

	//	Get Local UID of the asset
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALASSETUID FROM ASSET WHERE ASSETUID = %ld"), 
			 assetUID);
	retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, assetLUID, sizeof(assetLUID));
	// The asset should have been saved, so if not found in table ASSET, just exit.
	
	if (retCode == SQL_NO_DATA)		//	not found in table ASSET
		return outCode;		//	Exit

	//////////////////////////////////////////////////////////////////////////
	// This transaction will sometimes block DB because some triggers on DB 
	// invoked by other thread will try to obtain X lock on Asset table, which
	// the following UPDATE COMPLEXASSET will also obtain X lock on Asset table.
	// So we decided to use g_localDB instead of transactionDB.
	//													Bernie, 2006-04-28
	//////////////////////////////////////////////////////////////////////////
	
//	//	Begin a transaction
//	
//	LocalDB transactionDB;	//	ODBC connection which can utilize transaction
//	transactionDB.OpenDB(SQL_AUTOCOMMIT_OFF);	//	Open the connection in Manual-commit mode

	//	Disable all member relationship of the asset from table COMPLEXASSET
	
	// 2008-7-23 Add KenQ to make sure the Element can be deleted and other element Local UID was not changed
	// by re-change the sequence
	if(complexNum > 0)
	{
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE COMPLEXASSET SET ZQ_ENTRYSTATE = 1, SEQUENCE = SEQUENCE + %hu WHERE LOCALASSETUID = '%s' AND NOT ZQ_ENTRYSTATE = 2 "), 
				 complexNum+100, assetLUID);
	}
	else
	{
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE COMPLEXASSET SET ZQ_ENTRYSTATE = 1 WHERE LOCALASSETUID = '%s' AND NOT ZQ_ENTRYSTATE = 2 "), 
				 assetLUID);
	}
	retCode = g_localDB.ExecuteSQL(sql);
	
	//	Write all member relationship to DB using got complex-asset info
	for (DWORD i=0; i<complexNum; i++)	//	for each member
	{
	
		(*gpDbSyncLog)(Log::L_INFO, _T("%d:Get Element %d, Sequence %d"),threadid, i, (pComplex+i)->wSequence);
		WCHAR	oldName[256];

		//	Get LocalUID of the member, as it would be either an asset or an element
		switch ((pComplex+i)->wType)		//	choose member's object type
		{
		case IDS_ASSET:	//	It's an asset
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALASSETUID FROM ASSET WHERE ASSETUID = %ld"), 
					 (pComplex+i)->dwUid);
			retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
			break;
		case IDS_ATOMIC_ELEMENT:	//	It's an element
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALATOMICELEMENTUID FROM ATOMICELEMENT WHERE ATOMICELEMENTUID = %ld"), 
					 (pComplex+i)->dwUid);
			retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
			// can not find element by UID.
			// But maybe it is created by LAM, so should update the UID according to name
			//           --- by Bernie.Zhao
			if(retCode==SQL_NO_DATA && (pComplex+i)->wszName)
			{
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ATOMICELEMENT SET ATOMICELEMENTUID = %ld WHERE ITVNAME = N'%s' AND ATOMICELEMENTUID IS NULL"), 
					(pComplex+i)->dwUid, (pComplex+i)->wszName);
//				retCode = transactionDB.ExecuteSQL(sql);
				retCode = g_localDB.ExecuteSQL(sql);

				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALATOMICELEMENTUID FROM ATOMICELEMENT WHERE ATOMICELEMENTUID = %ld"), 
					(pComplex+i)->dwUid);
//				retCode = transactionDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
				retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
			}

			break;
		default:
			continue;	// it should be either an asset or an element
		}
		
		// this member can not be found, continue to next
		if(retCode==SQL_NO_DATA)
		{
			continue;
		}

		//	Check if the member relationship exists in the complex-asset
		//2008-7-23 changed by KenQ since sequence was reset
		//StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALASSETUID FROM COMPLEXASSET WHERE LOCALASSETUID = '%s' AND SEQUENCE = %hu"), 
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALASSETUID FROM COMPLEXASSET WHERE LOCALASSETUID = '%s' AND LOCALMEMBERUID = '%s' AND ZQ_ENTRYSTATE = 1"), 
				 assetLUID, memberLUID);

		retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, assetLUID, sizeof(assetLUID));
		if (retCode == SQL_SUCCESS)	//	the member relationship found
		{
			//	Enable the record into table COMPLEXASSET
// set acflag will be block here. so when acflag is 4,left other process to reset it

			//2008-7-23 changed by KenQ since sequence was reset
//			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE COMPLEXASSET SET  MEMBERTYPE = %hu,  LOCALMEMBERUID = '%s', ZQ_ENTRYSTATE = 0 ,ACFLAG = %d WHERE LOCALASSETUID = '%s' AND SEQUENCE = %hu AND ZQ_ENTRYSTATE = 1"), 
//				  (pComplex+i)->wType,  memberLUID,(pComplex+i)->dwFlags, assetLUID, (pComplex+i)->wSequence);
			DWORD sequence = 0;
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MIN(SEQUENCE) FROM COMPLEXASSET WHERE LOCALASSETUID = '%s' AND LOCALMEMBERUID = '%s' AND ZQ_ENTRYSTATE = 1"), 
				  assetLUID, memberLUID);
			g_localDB.SelectSQL(sql, SQL_C_ULONG, &sequence, sizeof(sequence));

			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE COMPLEXASSET SET  MEMBERTYPE = %hu,  SEQUENCE = %hu, ZQ_ENTRYSTATE = 0, ACFLAG = %d WHERE LOCALASSETUID = '%s' AND LOCALMEMBERUID = '%s' AND ZQ_ENTRYSTATE = 1 AND SEQUENCE=%hu"), 
				  (pComplex+i)->wType, (pComplex+i)->wSequence,(pComplex+i)->dwFlags, assetLUID, memberLUID, sequence);
			
			retCode = g_localDB.ExecuteSQL(sql);
			if (retCode == SQL_ERROR)	//	error occurs while executing SQL
				rollbackFlag = SQL_ROLLBACK;	//	Set rollback flag
		}
		else	//	the member relationship not found
		{
			//	Insert a new record into table COMPLEXASSET using the member info
			
			
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO COMPLEXASSET (LOCALASSETUID,LOCALMEMBERUID,MEMBERTYPE,SEQUENCE,ACFLAG,ZQ_ENTRYSTATE) VALUES (N'%s',N'%s',%hu,%hu,%ld,0)"), 
 					 assetLUID,  memberLUID,  (pComplex+i)->wType, (pComplex+i)->wSequence, (pComplex+i)->dwFlags);
			retCode = g_localDB.ExecuteSQL(sql);
			if (retCode == SQL_ERROR)	//	error occurs while executing SQL
				rollbackFlag = SQL_ROLLBACK;	//	Set rollback flag
		}

		//////////////////////////////////////////////////////////////////////////
		/// added by Bernie 2005-10-20, update element name
		if((pComplex+i)->wType==IDS_ATOMIC_ELEMENT)
		{
			memset(oldName, 0, sizeof(oldName));
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT NAME FROM ATOMICELEMENT WHERE ATOMICELEMENTUID = %ld"),
				(pComplex+i)->dwUid);
			retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, oldName, sizeof(oldName));

			if(retCode == SQL_ERROR)
				continue;

			if(wcscmp(oldName, (pComplex+i)->wszName)!=0)
			{
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ATOMICELEMENT SET NAME = N'%s' WHERE ATOMICELEMENTUID = %ld"),
					(pComplex+i)->wszName, (pComplex+i)->dwUid);

				retCode = g_localDB.ExecuteSQL(sql);
			}

			
		}
		//////////////////////////////////////////////////////////////////////////
		

	}	//	end for

	//	Commit or Rollback the transaction according to the value of rollbackFlag
//	retCode = transactionDB.EndTransaction(rollbackFlag);
	if (rollbackFlag == SQL_ROLLBACK)	//	any error occurs
	{
		outCode = SQL_ROLLBACK;		//	Set rollback flag for the whole function
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// Added by Bernie, 2006-Feb-5
		// call Add-in
//		if(g_AdiMan.isValid() && gUpdateThreadId != GetCurrentThreadId())
		if(g_AdiMan.isValid() && !m_isFullSyncing)
		{
			DA_entryDb	entryBlock;

			entryBlock._dwEntryType = DBSA_ENTRY_ASSET;
			entryBlock._dwEntryUID = assetUID;
			StringCbCopyW(entryBlock._szLocalEntryUID, DBSA_MAXLUID*sizeof(wchar_t), assetLUID);

			g_AdiMan.TrggCa(&entryBlock);
		}
		// end 
		//////////////////////////////////////////////////////////////////////////
	}
	
	//	Reset rollbackflag
	rollbackFlag = SQL_COMMIT;

	//	Delete all disabled member asset which is no longer used by others
	
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALMEMBERUID FROM COMPLEXASSET WHERE LOCALASSETUID = '%s' AND ZQ_ENTRYSTATE = 1 AND MEMBERTYPE = %hu"), 
			 assetLUID, IDS_ASSET);
//	retCode = transactionDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
	retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));

	while (retCode == SQL_SUCCESS)	//	deleted asset found
	{
/*	2008-7-23 By KenQ, since asset deletion was handled in AssetStateChangeCallback, so the checking of whether there is
    other asset linked in this asset is not necessary. 
	Also since Element deletion is handled in CaCallback and use this function to delete the element, 
	this block logic will cause problem: if there is an another asset is linked in this asset, the linked asset whose element will be deleted first, 
	and add then later, as a result the Element Local UID will be changed. The key reason was there is nested call of this function in following logic.

		//TODO: ZQ_ENTRYSTATE = 0? HOWTO: when restart?perhaps some useful asset will be delete from here
		//when restarting
		//	Check whether the asset is linked by other hierarchy
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALENTRYUID FROM HIERARCHY WHERE ENTRYTYPE = %hu AND LOCALENTRYUID = '%s'"),
				 IDS_ASSET, memberLUID);
//		retCode = transactionDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
		retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
		
		if (retCode == SQL_NO_DATA)	//	not linked by other hierarchy
		{
			//	Check whether the asset is linked by other asset
			//  2008-8-11 By Ken Q, add condition of "AND LOCALASSETUID <> '%s" to check whether it is linked by other asset
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALMEMBERUID FROM COMPLEXASSET WHERE MEMBERTYPE = %hu AND LOCALMEMBERUID = '%s' AND LOCALASSETUID <> '%s'"),
					 IDS_ASSET, memberLUID, assetLUID);
			retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
			
			if (retCode == SQL_NO_DATA)	//	not linked by other asset
			{
				
				DWORD memberUID = 0;	//	UID of the member asset to delete
				//	Get UID of the member asset to delete
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ASSETUID FROM ASSET WHERE LOCALASSETUID = '%s'"),
						 memberLUID);
				retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &memberUID, 0);

				//	Delete the member relationship of the member asset to delete
				retCode = SaveOneComplexAsset(memberUID, 0, NULL);	//	recursive
				if (retCode == SQL_ROLLBACK)	//	error occurs while dealing with member asset
					rollbackFlag = SQL_ROLLBACK;	//	Set rollback flag
				
				//	Delete the metadata of the asset from ASSETMD table
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM ASSETMD WHERE LOCALASSETUID = '%s'"),
						 memberLUID);
				retCode = g_localDB.ExecuteSQL(sql);
				if (retCode == SQL_ERROR)	//	error occurs while executing SQL
					rollbackFlag = SQL_ROLLBACK;	//	Set rollback flag

				//	Delete the asset from ASSET table
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM ASSET WHERE LOCALASSETUID = '%s'"),
						 memberLUID);
				(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);
				retCode = g_localDB.ExecuteSQL(sql);
				if (retCode == SQL_ERROR)	//	error occurs while executing SQL
					rollbackFlag = SQL_ROLLBACK;	//	Set rollback flag
				else
				{
					SaveMsgtoWorkQueue(memberLUID,L"",1,WORKQUEUE_DELETE,L"",L"");
				}
			}
		}
*/
		//	Delete the member relationship of the asset from table COMPLEXASSET
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM COMPLEXASSET WHERE LOCALASSETUID = '%s' AND ZQ_ENTRYSTATE = 1 AND MEMBERTYPE = %hu AND LOCALMEMBERUID = '%s'"), 
				 assetLUID, IDS_ASSET, memberLUID);
//		retCode = transactionDB.ExecuteSQL(sql);
		retCode = g_localDB.ExecuteSQL(sql);
//		if (retCode == SQL_ERROR)	//	error occurs while executing SQL
//			rollbackFlag = SQL_ROLLBACK;	//	Set rollback flag

//		//	Commit or Rollback the transaction accoring to the value of rollbackFlag
//		retCode = transactionDB.EndTransaction(rollbackFlag);
		if (rollbackFlag == SQL_ROLLBACK)	//	any error occurs
			outCode = SQL_ROLLBACK;		//	Set rollback flag for the whole function
		//	Reset rollbackflag
		rollbackFlag = SQL_COMMIT;

		//	Check if there is any member asset disabled
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALMEMBERUID FROM COMPLEXASSET WHERE LOCALASSETUID = '%s' AND ZQ_ENTRYSTATE = 1 AND MEMBERTYPE = %hu"), 
				 assetLUID, IDS_ASSET);
//		retCode = transactionDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
		retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
	}

	//	Delete all disabled member element which is no longer used by others
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALMEMBERUID FROM COMPLEXASSET WHERE LOCALASSETUID = '%s' AND ZQ_ENTRYSTATE = 1 AND MEMBERTYPE = %hu AND NOT ZQ_ENTRYSTATE = 0"), 
			 assetLUID, IDS_ATOMIC_ELEMENT);
//	retCode = transactionDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
	retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
	while (retCode == SQL_SUCCESS)	//	deleted asset found
	{
		//	Check whether the element is linked by other asset
		//  2008-8-11 By Ken Q, add condition of "LOCALASSETUID <> '%s" to check whether it is linked by others
		//                          condition of "LOCALASSETUID = '%s' AND ZQ_ENTRYSTATE = 0" is to check whether there is duplicated data, if yes, no deletion.
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALMEMBERUID FROM COMPLEXASSET WHERE MEMBERTYPE = %hu AND LOCALMEMBERUID = '%s' AND (LOCALASSETUID <> '%s' OR (LOCALASSETUID = '%s' AND ZQ_ENTRYSTATE = 0))"),
				 IDS_ATOMIC_ELEMENT, memberLUID, assetLUID, assetLUID);
//		retCode = transactionDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
		retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
		if (retCode == SQL_NO_DATA)	//	not linked by other asset
		{
			//	Delete the metadata of the element from ATOMICELEMENTMD table
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM ATOMICELEMENTMD WHERE LOCALATOMICELEMENTUID = '%s'"),
					 memberLUID);
//			retCode = transactionDB.ExecuteSQL(sql);
			retCode = g_localDB.ExecuteSQL(sql);
			if (retCode == SQL_ERROR)	//	error occurs while executing SQL
				rollbackFlag = SQL_ROLLBACK;	//	Set rollback flag

			//	Delete the element from ATOMICELEMENT table
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM ATOMICELEMENT WHERE LOCALATOMICELEMENTUID = '%s'"),
					 memberLUID);
//			retCode = transactionDB.ExecuteSQL(sql);
			retCode = g_localDB.ExecuteSQL(sql);
			if (retCode == SQL_ERROR)	//	error occurs while executing SQL
				rollbackFlag = SQL_ROLLBACK;	//	Set rollback flag
		}

		//	Delete the member relationship of the element from table COMPLEXASSET
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM COMPLEXASSET WHERE LOCALASSETUID = '%s' AND ZQ_ENTRYSTATE = 1 AND MEMBERTYPE = %hu AND LOCALMEMBERUID = '%s'"), 
				 assetLUID, IDS_ATOMIC_ELEMENT, memberLUID);
//		retCode = transactionDB.ExecuteSQL(sql);
		retCode = g_localDB.ExecuteSQL(sql);
//		if (retCode == SQL_ERROR)	//	error occurs while executing SQL
//			rollbackFlag = SQL_ROLLBACK;	//	Set rollback flag

//		//	Commit or Rollback the transaction accoring to the value of rollbackFlag
//		retCode = transactionDB.EndTransaction(rollbackFlag);
		if (rollbackFlag == SQL_ROLLBACK)	//	any error occurs
			outCode = SQL_ROLLBACK;		//	Set rollback flag for the whole function
		//	Reset rollbackflag
		rollbackFlag = SQL_COMMIT;

		//	Check if there is any member element disabled
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALMEMBERUID FROM COMPLEXASSET WHERE LOCALASSETUID = '%s' AND ZQ_ENTRYSTATE = 1 AND MEMBERTYPE = %hu"), 
				 assetLUID, IDS_ATOMIC_ELEMENT);
//		retCode = transactionDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
		retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, memberLUID, sizeof(memberLUID));
	}
	

	SQLINTEGER inIndicate = SQL_NTS, outIndicate = 0;	//	indication for the length of parameter buffer
	SQLSMALLINT procRet = 0;	//	return code of the stored procudure
	SQLSMALLINT valueRet = 1;	//	temp value of the stored procudure

	//	Call an stored procudure to update entrystate of the asset according to the entrystate of its members
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("{CALL UPDATE_ENTRYSTATE_WITH_ASSET(?,?)}"));
//	retCode = transactionDB.CallStoreProcedure(sql, SQL_C_TCHAR, SQL_VARCHAR, sizeof(assetLUID), 0, assetLUID, sizeof(assetLUID), &inIndicate,
//											   SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &procRet, 0, &outIndicate,
//											   SQL_C_SSHORT, SQL_SMALLINT, 0, 0, NULL, 0, NULL);
	retCode = g_localDB.CallStoreProcedure(sql, 
										   LocalDB::INPUT, SQL_C_TCHAR, SQL_VARCHAR, sizeof(assetLUID), 0, assetLUID, sizeof(assetLUID), &inIndicate,
										   LocalDB::OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &procRet, 0, &outIndicate,
										   LocalDB::INOUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, NULL, 0, NULL);
	(*gpDbSyncLog)(Log::L_INFO, _T("%d:Result of UPDATE_ENTRYSTATE_WITH_ASSET for asset %s: %hu."),threadid, assetLUID, procRet);
	
	
	if ((retCode == SQL_ERROR) || (procRet != 0)) {	//	error occurs while executing SQL
		rollbackFlag = SQL_ROLLBACK;	//	Set rollback flag

	}

//	//	Commit or Rollback the transaction accoring to the value of rollbackFlag
//	retCode = transactionDB.EndTransaction(rollbackFlag);
	if (rollbackFlag == SQL_ROLLBACK)	//	any error occurs
		outCode = SQL_ROLLBACK;		//	Set rollback flag for the whole function
	

	//	Call an stored procudure to update actual_playtime of the asset according to the actual_playtime of its members
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("{CALL SET_ACTUALDURATION_WITH_ASSET(?,?,?)}"));

//	retCode = transactionDB.CallStoreProcedure(sql, SQL_C_TCHAR, SQL_VARCHAR, sizeof(assetLUID), 0, assetLUID, sizeof(assetLUID), &inIndicate,
//											   SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &procRet, 0, &outIndicate,
//											   SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &valueRet, 0, &outIndicate);

	retCode = g_localDB.CallStoreProcedure(sql, 
		                                   LocalDB::INPUT, SQL_C_TCHAR, SQL_VARCHAR, sizeof(assetLUID), 0, assetLUID, sizeof(assetLUID), &inIndicate,
										   LocalDB::OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &procRet, 0, &outIndicate,
										   LocalDB::INOUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &valueRet, 0, &outIndicate);

	(*gpDbSyncLog)(Log::L_INFO, _T("%d:Result of SET_ACTUALDURATION_WITH_ASSET for asset %s: %hu."),threadid, assetLUID, procRet);
	if ((retCode == SQL_ERROR) || (procRet != 0)) {	//	error occurs while executing SQL
		rollbackFlag = SQL_ROLLBACK;	//	Set rollback flag
	}

//	//	Commit or Rollback the transaction accoring to the value of rollbackFlag
//	retCode = transactionDB.EndTransaction(rollbackFlag);
	if (rollbackFlag == SQL_ROLLBACK)	//	any error occurs
		outCode = SQL_ROLLBACK;		//	Set rollback flag for the whole function

//	//	Close the connection
//	transactionDB.CloseDB();

	return outCode;	//	success
}


/*****************************************************************************
Function Name:	SaveOneFolder
Arguments:		Folder ID (DWORD), Folder Name (WCHAR *)
Returns:		retCode (DWORD) (0:success 1:fail)
Description:	This function is for one folder, 
				Local table FOLDER is updated according to:
				1. Look for folder record in table FOLDER by folder UID;
				2. If found, update this record using folder info;
				3. If not found, look for folder record in table FOLDER by folder name;
				4. If found, update this record using folder info;
				5. If not found, generate a local hierarchy id and insert a new record
				   using folder info.
				6. Finally, restore ZQ_ENTRYSTATE if it's not a deleted one
*****************************************************************************/
DWORD CDSInterface::SaveOneFolder(DWORD folderUID, const WCHAR *folderName)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement

	//	Convert folder itv name into SQL format	
	CString	itvName = folderName;	//	itv name of folder
	itvName.Replace(_T("'"), _T("''"));
	

	
	SQLTCHAR folderLUID[21];	//	Local UID of folder
	DWORD isLocal = 0;	//	Owner of folder
	
	memset(folderLUID,0,sizeof(folderLUID));
	
	DBFIELD field1,field2,field3;
	DBRECORD record,selectCols;
	DBRESULTSETS resultSets;
	
	field1.lType = SQL_C_ULONG;
	field2.lType = SQL_C_TCHAR;
	field2.lSize = 255;
	field3.lType = SQL_C_TCHAR;
	field3.lSize = 64;
	selectCols.push_back(field1);
	selectCols.push_back(field2);
	selectCols.push_back(field3);

		//	Check whether the folder exists in table FOLDER by folder UID and get its local flag if exists
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT ISLOCAL,ITVNAME,LOCALFOLDERUID FROM FOLDER WHERE FOLDERUID = %ld"),
		folderUID);
	retCode = g_localDB.SelectAllSQL(sql,selectCols,&resultSets);
	
	if (retCode == SQL_SUCCESS)	//	found in table FOLDER
	{
		record = resultSets.at(0);
		WCHAR OldName[256];
		SQLTCHAR OldFolderLUID[64];
		memcpy(&isLocal,record.at(0).pValue,4);
		StringCbCopyW(OldName, 256*sizeof(WCHAR), (WCHAR*)record.at(1).pValue);
		StringCbCopyW(OldFolderLUID, 64*sizeof(SQLTCHAR), (WCHAR*)record.at(2).pValue);
		g_localDB.FreeResultSets(&resultSets);
		if(wcscmp(OldName,folderName)!=0)
		{	
			//work queue
			if(IsPackaged(OldFolderLUID))
				SaveMsgtoWorkQueue(OldFolderLUID,L"",IDS_PACKAGE,WORKQUEUE_RENAME,L"",folderName);

			if (isLocal)	//	It's a local folder and the name should not be updated
			{
			//	Update the folder record in table FOLDER using got folder info		
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE FOLDER SET ITVNAME = N'%s' WHERE FOLDERUID = %ld"), //ls to s
					 itvName, folderUID);
				retCode = g_localDB.ExecuteSQL(sql);			
			}
			else	//	It's not a local element and the name should be updated
			{
				//	Update the folder record in table FOLDER using got folder info
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE FOLDER SET NAME = N'%s', ITVNAME = N'%s' WHERE FOLDERUID = %ld"), //ls to s
						 itvName, itvName, folderUID);
				retCode = g_localDB.ExecuteSQL(sql);
			}
		}
	}
	else	//	not found in table FOLDER
	{
		//check if the folder is preset by AM
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT LOCALFOLDERUID FROM FOLDER WHERE NAME = N'%s' AND ISLOCAL = 1 AND FOLDERUID IS NULL"),
			itvName);
		retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,folderLUID,sizeof(folderLUID));
		if(retCode != SQL_SUCCESS)
		{
			
			DWORD tempLUID = 0;	//	temp UID created from DB sequence
			GetNewLocalUID(IDS_HIERARCHY, &tempLUID);

			StringCbPrintf(folderLUID,  21*sizeof(WCHAR), _T("H%ld"), tempLUID);
			// TODO: add to workqueue reserve

			//	Insert a new folder record into table FOLDER using got folder info
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO FOLDER (LOCALFOLDERUID,FOLDERUID,NAME,ITVNAME,ZQ_ENTRYSTATE,ISLOCAL,ISPACKAGE) VALUES (N'%s',%ld,N'%s',N'%s',0,0,0)"), //ls to s
		 			 folderLUID, folderUID, itvName, itvName);
			retCode = g_localDB.ExecuteSQL(sql);		
		}
		else
		{
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE FOLDER SET FOLDERUID = %ld WHERE LOCALFOLDERUID = '%s'"), //ls to s
						 folderUID, folderLUID);
						// modified by Bernie,  July/04/2005, added var 'folderUID' to StringCbCopyW() to avoid exception
				retCode = g_localDB.ExecuteSQL(sql);
		}
		
	}

	//	Restore ZQ_ENTRYSTATE for the folder record, if it's not a deleted one
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE FOLDER SET ZQ_ENTRYSTATE = 0 WHERE FOLDERUID = %ld AND ZQ_ENTRYSTATE = 1 "),
			 folderUID);
	retCode = g_localDB.ExecuteSQL(sql);

	return 0;	//	success
}


/*****************************************************************************
Function Name:	SaveOneAsset
Arguments:		Asset ID (DWORD), Asset Name (WCHAR *), Application ID (DWORD)
                bNewAsset - output, true - new asset to LAM
				                    false - existing asset to LAM
Returns:		retCode (DWORD) (0:success 1:fail)
Description:	This function is for one asset, 
				Local table ASSET is updated according to:
				1. Look for asset record in table ASSET by asset ID;
				2. If found, update this record using asset info;
				3. If not found, look for asset record in table ASSET by asset name;
				4. If found, update this record using asset info;
				5. If not found, generate a local asset id and insert a new record 
				   using asset info;
				6. Finally, restore ZQ_ENTRYSTATE if it's not a deleted one
*****************************************************************************/
DWORD CDSInterface::SaveOneAsset(DWORD assetUID, const WCHAR *assetName, DWORD appUid, BOOL& bNewAsset)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement

	//	Convert asset itv name into SQL format
	CString	itvName = assetName;	//	itv name of asset
	itvName.Replace(_T("'"), _T("''"));

	SQLTCHAR assetLUID[21];	//	Local UID of asset
	DWORD isLocal = 0;	//	Local flag of asset
	
	memset(assetLUID,0,sizeof(assetLUID));

	if(!m_isFullSyncing)
	{
		// KenQ 2007-11-29 the asset is coming, just remove it
		IDMAP::iterator it = m_notFoundAssetId.find(assetUID);
		if(it != m_notFoundAssetId.end())
		{
			m_notFoundAssetId.erase(it);
		}
	}

	// lock the [Asset] table first, to avoid multi-threads inserting duplicate records
	ZQ::common::MutexGuard	assetGd(m_AssetTableLock);	


	// prepare the select
	DBRESULTSETS resultSets;
	DBRECORD record,selectCols;
	DBFIELD field1,field2,field3;
	
	field1.lType = SQL_C_ULONG;
	field2.lType = SQL_C_TCHAR;
	field2.lSize = 256;
	field3.lType = SQL_C_TCHAR;
	field3.lSize = 64;

	selectCols.push_back(field1);
	selectCols.push_back(field2);
	selectCols.push_back(field3);
//	Check whether the asset exists in table ASSET by asset UID and get its local flag if exists
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT ISLOCAL,ITVNAME,LOCALASSETUID FROM ASSET WHERE ASSETUID = %ld"),
		assetUID);
	retCode = g_localDB.SelectAllSQL(sql,selectCols,&resultSets);
	
	if (retCode == SQL_SUCCESS)	//	found in table ASSET
	{
		bNewAsset = false;

		record = resultSets.at(0);
		WCHAR OldName[256];
		WCHAR tmpStr[1024];
		WCHAR OldLUID[64];
		memcpy(&isLocal,record.at(0).pValue,4);
		StringCbCopyW(OldName, 256*sizeof(WCHAR), (WCHAR*)record.at(1).pValue);
		StringCbCopyW(OldLUID, 64*sizeof(WCHAR), (WCHAR*)record.at(2).pValue);
		g_localDB.FreeResultSets(&resultSets);
		StringCbCopyW(assetLUID, 21*sizeof(SQLTCHAR), OldLUID);
		if (isLocal)	//	It's a local asset, whose name should not be updated
		{
			//	Update the asset record in table ASSET using got asset info, without updating name
			if(wcscmp(OldName,assetName)!= 0)
			{
				SaveMsgtoWorkQueue(OldLUID,L"",IDS_ASSET,WORKQUEUE_RENAME,L"",assetName);
				StringCbPrintf(tmpStr, 1024*sizeof(WCHAR), _T(",ITVNAME = N'%s'"),itvName);
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ASSET SET APPLICATIONUID = %ld %s WHERE ASSETUID = %ld"), //ls to s
					 appUid, tmpStr, assetUID);
			}
			else	//modified by wei.sun 2005/06/17
			{
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ASSET SET APPLICATIONUID = %ld WHERE ASSETUID = %ld"), //ls to s
					 appUid, assetUID);
			}
			retCode = g_localDB.ExecuteSQL(sql);
		}
		else	//	It's not a local asset and the name should be updated
		{
			//	Update the asset record in table ASSET using got asset info
			
			if(wcscmp(OldName,assetName)!= 0)
			{
				SaveMsgtoWorkQueue(OldLUID,L"",IDS_ASSET,WORKQUEUE_RENAME,L"",assetName);
				StringCbPrintf(tmpStr, 1024*sizeof(WCHAR), _T(",ITVNAME = N'%s', NAME = N'%s'"),itvName,itvName);		
			
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ASSET SET APPLICATIONUID = %ld %s WHERE ASSETUID = %ld"), //ls to s
					 appUid, tmpStr, assetUID);		
				retCode = g_localDB.ExecuteSQL(sql);
			}
			else
			{
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ASSET SET APPLICATIONUID = %ld WHERE ASSETUID = %ld"), //ls to s
					 appUid,  assetUID);		
				retCode = g_localDB.ExecuteSQL(sql);
			}
		}
	}
	else	//	not found in table ASSET by asset UID
	{
		//	Check whether the asset exists in table ASSET by asset name and get its local flag if exists
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALASSETUID FROM ASSET WHERE ITVNAME = N'%s'"), 
				 itvName);
		retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, assetLUID, sizeof(assetLUID));
		if (retCode == SQL_SUCCESS)	//	found in table ASSET
		{
			bNewAsset = false;

			//	Update the asset record in table ASSET using got asset info
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ASSET SET APPLICATIONUID = %ld, ASSETUID = %ld  WHERE ITVNAME = N'%s'"), //ls to s
					 appUid, assetUID, itvName);
			retCode = g_localDB.ExecuteSQL(sql);
			// move to here to avoid execute this sql when insert
			//	Update the hierarchy record in table HIERARCHY using got asset local UID
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE HIERARCHY SET ENTRYUID = %ld WHERE ENTRYTYPE = %hu AND LOCALENTRYUID = '%s'"), //ls to s
				 assetUID, IDS_ASSET, assetLUID);
			retCode = g_localDB.ExecuteSQL(sql);
		}
		else	//	not found in table ASSET by name, it's a new asset
		{
			bNewAsset = true;

			//	Generate a new Local UID for the asset
			DWORD tempLUID = 0;	//	temp UID created from DB sequence
			GetNewLocalUID(IDS_ASSET, &tempLUID);

			StringCbPrintf(assetLUID,  21*sizeof(WCHAR), _T("A%ld"), tempLUID);
			//	Insert a new asset record into table ASSET using got asset info
			// TODO: work queue asset add reserve
			
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO ASSET (LOCALASSETUID,ASSETUID,APPLICATIONUID,NAME,ITVNAME,ZQ_ENTRYSTATE,ISLOCAL) VALUES (N'%s',%ld,%ld,N'%s',N'%s',0,0)"), //ls to s
	 				 assetLUID, assetUID, appUid, itvName, itvName);
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);
			retCode = g_localDB.ExecuteSQL(sql);
		}

	}
	//	Restore ZQ_ENTRYSTATE for the asset record, if it's not a deleted one
	// left the flag when ZQ_EntryState = 2
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ASSET SET ZQ_ENTRYSTATE = 0 WHERE LOCALASSETUID = '%s' AND ZQ_ENTRYSTATE = 1"),
			 assetLUID);
	retCode = g_localDB.ExecuteSQL(sql);

	return 0;	//	success
}


/*****************************************************************************
Function Name:	UpdateAssetState
Arguments:		Asset ID (DWORD), Asset Current State (WORD)
Returns:		retCode (DWORD) (0:success 1:fail)
Description:	This function is to update the state of one asset, 
				It updates local ASSET and HIERARCHY table accordingly using
				the current state of given asset
*****************************************************************************/
DWORD CDSInterface::UpdateAssetState(DWORD assetUID, WORD assetState)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR	sql[1024];	//	string to put SQL statement
	WCHAR		localAssetUID[24];	// local asset uid
	DWORD       dwLocalState;

// ----------- Modified by KenQ on 2006-6-5  --------------//
// In case of DBSync startup, all state changings are required to send by Add-in, //
// so, it'd better to compare the current status with original one to avoid too much message //

	DBRECORD record,SelectCols;
	DBRESULTSETS resultSets;
	DBFIELD field1,field2;

	field1.lType = SQL_C_TCHAR;
	field1.lSize = 64;
	field2.lType = SQL_C_ULONG;
	
	SelectCols.push_back(field1);
	SelectCols.push_back(field2);
	
	// get local asset uid
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LocalAssetUID, STATE FROM Asset WHERE ASSETUID = %ld"),
			 assetUID);
	retCode = g_localDB.SelectAllSQL(sql, SelectCols, &resultSets);

	if(retCode != SQL_SUCCESS)
	{
		(*gpDbSyncLog)(Log::L_DEBUG, _T("%d:Asset %d did not exist in LAM"), GetCurrentThreadId(),assetUID);
		
		return retCode;
	}
	record = resultSets.at(0);
	
	StringCbCopy(localAssetUID, 64*sizeof(WCHAR), (wchar_t*)record.at(0).pValue);
	memcpy(&dwLocalState,record.at(1).pValue,4);
	
	if(assetState == dwLocalState)
	{
		(*gpDbSyncLog)(Log::L_DEBUG, _T("%d:Asset %d status(=%d) did not changed, no state updating to LAM."),
						GetCurrentThreadId(), assetUID, assetState);
		return SQL_SUCCESS;
	}
	
	//	Update the asset record in table ASSET using got asset state
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ASSET SET STATE = %hu WHERE ASSETUID = %ld"),
			 assetState, assetUID);
	retCode = g_localDB.ExecuteSQL(sql);

	//	Update the asset record in table HIRRARCHY using got asset state
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE HIERARCHY SET STATE = %hu WHERE ENTRYUID = %ld AND ENTRYTYPE = %hu"),
			 assetState, assetUID, IDS_ASSET);
	retCode = g_localDB.ExecuteSQL(sql);


//	----------- modified by KenQ 2006-11-22 for KDDI requirement: ReferenceCount and RatingControl -------------
//  This case of Asset State change to be Active or Inactive
	if(g_SupportRefcountRatingctrl && (ASSET_STATE_ACTIVE == assetState || ASSET_STATE_INACTIVE == assetState) )
	{
		(*gpDbSyncLog)(Log::L_DEBUG, _T("%d:Asset %d status changed from %d to %d, need to recalculate Rating and ReferenceCount"),
						GetCurrentThreadId(), assetUID, dwLocalState, assetState);

		AssetRatingRef(localAssetUID, m_titleSortNameMDUID, true);
	}
	//////////////////////////////////////////////////////////////////////////
	// Added by Bernie, 2005-Nov-24
	// call Add-in
	// changed by KenQ, 2006-6-5, what ever in callback or startup, the state changing event must be sent out.
	// otherwise, OTE can not get the status changing during DBSync stopping period. 
	if(g_AdiMan.isValid() /*&& gUpdateThreadId != GetCurrentThreadId()*/)
	{
		wchar_t provID[DBSA_MAXMDVALUE]={0};
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT amd.MDValue FROM ASSETMD amd, EntryMDD emd WHERE amd.MDUID=emd.MDUID AND emd.MDName='ProviderId' AND emd.EntryType=%hu AND amd.LocalAssetUID='%s'"), 
				 IDS_ASSET, localAssetUID);
		g_localDB.SelectSQL(sql, SQL_C_TCHAR, provID, sizeof(provID));
		
		wchar_t provAssetID[DBSA_MAXMDVALUE]={0};
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT amd.MDValue FROM ASSETMD amd, EntryMDD emd WHERE amd.MDUID=emd.MDUID AND emd.MDName='ProviderAssetId' AND emd.EntryType=%hu AND amd.LocalAssetUID='%s'"), 
				 IDS_ASSET, localAssetUID);
		g_localDB.SelectSQL(sql, SQL_C_TCHAR, provAssetID, sizeof(provAssetID));

		DA_entryDb	entryBlock;
		DA_stateDb	stateBlock;

		entryBlock._dwEntryType = DBSA_ENTRY_ASSET;
		entryBlock._dwEntryUID = assetUID;
		StringCbCopyW(entryBlock._szLocalEntryUID, DBSA_MAXLUID*sizeof(wchar_t), localAssetUID);

		StringCbCopyW(entryBlock._szProviderID, DBSA_MAXMDVALUE*sizeof(wchar_t), provID);
		StringCbCopyW(entryBlock._szProviderAssetID, DBSA_MAXMDVALUE*sizeof(wchar_t), provAssetID);

		stateBlock._dwEntryState = assetState;
		
		g_AdiMan.TrggStat(&entryBlock, &stateBlock);
	}
	// end 
	//////////////////////////////////////////////////////////////////////////

	// Added by Bernie, 2006-Feb-9
	// upon asset state change, we shall also let NSSync do the work for some
	// entry count update.
	SaveMsgtoWorkQueue(localAssetUID,L"",IDS_ASSET,WORKQUEUE_UPDATE,L"",L"");
	
	return retCode;
}


/*****************************************************************************
Function Name:	SaveOneElement
Arguments:		Element ID (DWORD), Element Name (WCHAR *)
Returns:		retCode (DWORD) (0:success 1:fail)
Description:	This function is for one element, 
				Local table ATOMICELEMENT is updated according to:
				1. Look for element record in table ATOMICELEMENT by element ID;
				2. If found, update this record using element info;
				3. If not found, look for element record in table ATOMICELEMENT
				   by element name;
				4. If found, update this record using element info;
				5. If not found, generate a local element id and insert a new record 
				   using element info.
				6. Finally, restore ZQ_ENTRYSTATE if it's not a deleted one
*****************************************************************************/
DWORD CDSInterface::SaveOneElement(DWORD elementUID, const WCHAR *elementName)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement

	//	Convert element itv name into SQL format
	CString	itvName = elementName;	//	itv name of element
	itvName.Replace(_T("'"), _T("''"));

	if(!m_isFullSyncing)
	{
		// KenQ 2007-11-29 the element is coming, just remove it
		IDMAP::iterator it = m_notFoundElementId.find(elementUID);
		if(it != m_notFoundElementId.end())
		{
			m_notFoundElementId.erase(it);
		}
	}

	// lock the [AtomicElement] table first, to avoid multi-threads inserting duplicate records
	ZQ::common::MutexGuard	elementGd(m_AtomicElementTableLock);	

	SQLTCHAR elementLUID[21];	//	Local UID of element
	DWORD isLocal = 0;	//	Local flag of element
	memset(elementLUID,0,sizeof(elementLUID));
	//	Check whether the element exists in table ATOMICELEMENT by element UID and get its local flag if exists
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ISLOCAL FROM ATOMICELEMENT WHERE ATOMICELEMENTUID = %ld"), 
			 elementUID);
	retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &isLocal, 0);

	if (retCode == SQL_SUCCESS)	//	found in table ATOMICELEMENT
	{
		if (isLocal)	//	It's a local element and the name should not be updated
		{
			//	Update the element record in table ATOMICELEMENT using got element info
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ATOMICELEMENT SET ITVNAME = N'%s' WHERE ATOMICELEMENTUID = %ld"), //ls to s
					 itvName, elementUID);
			retCode = g_localDB.ExecuteSQL(sql);
		}
		else	//	It's not a local element and the name should be updated
		{
			//	Update the element record in table ATOMICELEMENT using got element info
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ATOMICELEMENT SET NAME = N'%s', ITVNAME = N'%s' WHERE ATOMICELEMENTUID = %ld"), //ls to s
					 itvName, itvName, elementUID);
			retCode = g_localDB.ExecuteSQL(sql);
		}
	}
	else	//	not found in table ATOMICELEMENT
	{
		//	Check whether the element exists in table ATOMICELEMENT by element itv name and get its local flag if exists
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALATOMICELEMENTUID FROM ATOMICELEMENT WHERE ITVNAME = N'%s'"), 
				 itvName);
		retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, elementLUID, sizeof(elementLUID));
		if (retCode == SQL_SUCCESS)	//	found in table ATOMICELEMENT
		{
			//	Update the element record in table ATOMICELEMENT using got element info
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ATOMICELEMENT SET ATOMICELEMENTUID = %ld WHERE ITVNAME = N'%s'"), //ls to s
					 elementUID, itvName);
			retCode = g_localDB.ExecuteSQL(sql);
		}
		else	//	not found in table ATOMICELEMENT
		{
			//	Generate a new Local UID for the element
			DWORD tempLUID = 0;	//	temp UID created from DB sequence
			GetNewLocalUID(IDS_ATOMIC_ELEMENT, &tempLUID);

			StringCbPrintf(elementLUID,  21*sizeof(WCHAR), _T("T%ld"), tempLUID);
			//	Insert a new asset record into table ATOMICELEMENT using got element info
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO ATOMICELEMENT (LOCALATOMICELEMENTUID,ATOMICELEMENTUID,NAME,ITVNAME,ZQ_ENTRYSTATE,ISLOCAL) VALUES (N'%s',%ld,N'%s',N'%s',0,0)"), //ls to s
	 				 elementLUID, elementUID, itvName, itvName);
			retCode = g_localDB.ExecuteSQL(sql);
		}
		//	Update the metadata record in table ATOMICELEMENTMD using got element local UID
//		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), "UPDATE ATOMICELEMENTMD SET ATOMICELEMENTUID = %ld WHERE LOCALATOMICELEMENTUID = '%s'", //ls to s
//				 elementUID, elementLUID);
// 		retCode = g_localDB.ExecuteSQL(sql);
		//	Update the complexasset record in table COMPLEXASSET using got element local UID
//		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), "UPDATE COMPLEXASSET SET MEMBERUID = %ld WHERE MEMBERTYPE = %hu AND LOCALMEMBERUID = '%s'", //ls to s
//				 elementUID, IDS_ATOMIC_ELEMENT, elementLUID);
// 		retCode = g_localDB.ExecuteSQL(sql);
	}

	//	Restore ZQ_ENTRYSTATE for the element record, if it's not a deleted one
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ATOMICELEMENT SET ZQ_ENTRYSTATE = 0 WHERE ATOMICELEMENTUID = %ld "),
			 elementUID);
	retCode = g_localDB.ExecuteSQL(sql);

	return 0;	//	success
}
/*****************************************************************************
Function Name:	SaveOneClipEx
Arguments:		clip (CLIP*)
Returns:		retCode (DWORD) (0:success 1:fail)
Description:	This function is for one clip, 
				SaveOneClip by uid and name
				Save Extra info in structure CLIP
*****************************************************************************/

DWORD CDSInterface::SaveOneClipEx(CLIP* pClip,DWORD elementUID)
{
	RETCODE retCode = SaveOneClip(pClip->dwUid,pClip->wszName);
	if (retCode)
		return retCode;
	
	SQLTCHAR		sql[1024];
	SQLTCHAR clipLUID[21] = _T(""); 
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALCLIPUID FROM CLIP WHERE CLIPUID = %ld"), 
			 pClip->dwUid);
	retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, clipLUID, sizeof(clipLUID));
	switch(retCode)
	{
	
	case SQL_SUCCESS:
		{
			SQLTCHAR LAeUID [21];
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT LOCALATOMICELEMENTUID FROM ATOMICELEMENT WHERE ATOMICELEMENTUID = %d"),
				elementUID);
			retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,LAeUID,sizeof(LAeUID));
			TCHAR startTime[64] = _T("");
			TCHAR endTime[64] = _T("");
			TCHAR startId [33]= _T("");
			TCHAR endId [33]= _T("");
			DWORD ms = pClip->dwStartTime;
			DWORD second = ms / 1000;
			DWORD minute = second / 60;
			DWORD hour = minute /60;
			ms %= 1000;
			second %= 60;
			minute %= 60;
			StringCbPrintf(startTime, 64*sizeof(WCHAR), _T("%02d:%02d:%02d:%03d:00"),hour,minute,second,ms);

			ms = pClip->dwEndTime;
			second = ms / 1000;
			minute = second / 60;
			hour = minute /60;
			ms %= 1000;
			second %= 60;
			minute %= 60;
			StringCbPrintf(endTime, 64*sizeof(WCHAR), _T("%02d:%02d:%02d:%03d:00"),hour,minute,second,ms);
			
			_ui64tow(pClip->qwStartId,startId,16);
			_ui64tow(pClip->qwEndId,endId,16);
			
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE CLIP SET STARTTIME = '%s' ,ENDTIME = '%s', STARTID = '%016s', ENDID = '%016s' ,LOCALATOMICELEMENTUID = '%s' WHERE LOCALCLIPUID = '%s'"), 		
				 startTime,endTime,startId,endId,LAeUID,clipLUID);
			retCode = g_localDB.ExecuteSQL(sql);
			break;
		}
	case SQL_NO_DATA://when it will happend 
	default:
		break;
	
	}
	return 0;
	
}
/*****************************************************************************
Function Name:	SaveOneClip
Arguments:		Clip ID (DWORD), Clip Name (WCHAR *)
Returns:		retCode (DWORD) (0:success 1:fail)
Description:	This function is for one element, 
				Local table Clip is updated according to:
				1. Look for element record in table CLIP by element ID;
				2. If found, update this record using element info;
				3. If not found, look for element record in table CLIP
				   by element name;
				4. If found, update this record using element info;
				5. If not found, generate a local element id and insert a new record 
				   using element info.
				6. Finally, restore ZQ_ENTRYSTATE if it's not a deleted one
*****************************************************************************/
DWORD CDSInterface::SaveOneClip(DWORD clipUID, const WCHAR *clipName)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement

	//	Convert clip name into SQL format
	CString	Name = clipName;	// name of clip
	Name.Replace(_T("'"), _T("''"));

	SQLTCHAR clipLUID[21];	//	Local UID of clip
	DWORD isLocal = 0;	//	Local flag of clip
	memset(clipLUID,0,sizeof(clipLUID));
	//	Check whether the clip exists in table clip by clip UID and get its local flag if exists
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ISLOCAL FROM CLIP WHERE CLIPUID = %ld"), 
			 clipUID);
	retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &isLocal, 0);

	switch (retCode )	//	found in table CLIP
	{	
	case SQL_SUCCESS:
		{
			//	Update the CLIP record in table CLIP using got CLIP info
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE CLIP SET NAME = N'%s' WHERE CLIPUID = %ld"), //ls to s
					 Name, clipUID);
			retCode = g_localDB.ExecuteSQL(sql);	
			break;
		}
	
	case SQL_NO_DATA:	//	not found in table CLIP
		{
			//	Check whether the clip exists in table CLIP by Clip name and get its local flag if exists
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALCLIPUID FROM CLIP WHERE NAME = N'%s'"), 
					 Name);
			retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, clipLUID, sizeof(clipLUID));
			switch(retCode)
			{
			
				case SQL_SUCCESS:	//	found in table CLIP
				{
					//	Update the clip record in table CLIP using got clip info
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE CLIP SET CLIPUID = %ld WHERE NAME = N'%s'"), //ls to s
							 clipUID, Name);
					retCode = g_localDB.ExecuteSQL(sql);
					break;
				}
				case SQL_NO_DATA://	not found in table CLIP
				{
					//	Generate a new Local UID for the clip
					DWORD tempLUID = 0;	//	temp UID created from DB sequence
					GetNewLocalUID(IDS_CLIP, &tempLUID);

					StringCbPrintf(clipLUID,  21*sizeof(WCHAR), _T("C%ld"), tempLUID);
					//	Insert a new asset record into table CLIP using got clip info
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO CLIP (LOCALCLIPUID,CLIPUID,NAME,ZQ_ENTRYSTATE,ISLOCAL) VALUES (N'%s',%ld,N'%s',0,0)"), //ls to s
	 						 clipLUID, clipUID, Name);
					retCode = g_localDB.ExecuteSQL(sql);
					break;
				}
				//	Update the metadata record in table CLIPMD using got element local UID
		//		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), "UPDATE CLIPMD SET CLIPUID = %ld WHERE LOCALCLIPUID = '%s'", //ls to s
		//				 clipUID, clipLUID);
		//		retCode = g_localDB.ExecuteSQL(sql);
				//	Update the complexasset record in table COMPLEXASSET using got element local UID
				default:
					break;
			}
			break;
		}
	default:
		break;
	}
	//	Restore ZQ_ENTRYSTATE for the element record, if it's not a deleted one
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE CLIP SET ZQ_ENTRYSTATE = 0 WHERE CLIPUID = %ld "),
			 clipUID);
	retCode = g_localDB.ExecuteSQL(sql);

	return 0;	//	success
}

/*****************************************************************************
Function Name:	UpdateElementState
Arguments:		Element ID (DWORD), Element Current State (WORD)
Returns:		retCode (DWORD) (0:success 1:fail)
Description:	This function is to update the state and entrystate of one element, 
				It compares the current state of given element with that in local
				DB first, if current state is 1(active) and that in local DB is 
				not, it will composite an XML file indicating that the media of
				the element has been uploaded, and then it updates local table 
				ATOMICELEMENT accordingly using the current state of given element.
				As for entrystate, it's updated according to:
				1. If current state is 'active', update entrystate to 'uploaded';
				2. If current state is 'inactive' and the element is local, update
				   entrystate to 'created'.
*****************************************************************************/
DWORD CDSInterface::UpdateElementState(DWORD elementUID, WORD elementState)
{
	RETCODE		retCode;	//	return code
	RETCODE		foundElement;
	SQLTCHAR	sql[1024];	//	string to put SQL statement
	SQLTCHAR	elementLUID[21];	//	Local UID of element
	//	Get local UID of the element from table ATOMICELEMENT
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT LOCALATOMICELEMENTUID FROM ATOMICELEMENT WHERE ATOMICELEMENTUID = %ld"), 
			 elementUID);
	foundElement = g_localDB.SelectSQL(sql, SQL_C_TCHAR, elementLUID, sizeof(elementLUID));

	if (elementState == ACTIVE_STATE)	//	current state of the element is 'active'
	{
		DWORD originState = ACTIVE_STATE;	//	assumed origin state of the element in local DB
		//	Read the state of the element from table ATOMICELEMENT
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT STATE FROM ATOMICELEMENT WHERE ATOMICELEMENTUID = %ld"),
				 elementUID);
		retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &originState, 0);

		if (originState != ACTIVE_STATE)	//	origin state of the element is not 'active'
		{
			if (foundElement == SQL_SUCCESS && g_SupportIngestManager)	//	LUID of the element got
				//	Composite an XML file indicating that the media of the element has been uploaded
				WriteXMLFile(elementLUID, _T("Uploaded"));
		}

		//	Update the element record in table ATOMICELEMENT using got element info
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ATOMICELEMENT SET STATE = %hu, ENTRYSTATE = %hu WHERE ATOMICELEMENTUID = %ld"),
				 elementState, ELEMENT_UPLOADED, elementUID);
		retCode = g_localDB.ExecuteSQL(sql);
	}
	else	//	current state of the element is 'inactive'
	{
		WORD isLocal = 0;	//	islocal flag of the element in local DB
		//	Read the islocal flag of the element from table ATOMICELEMENT
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ISLOCAL FROM ATOMICELEMENT WHERE ATOMICELEMENTUID = %ld"),
				 elementUID);
		retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &isLocal, 0);
		if (isLocal)	//	it's a local element
		{
			//	Update the element record in table ATOMICELEMENT using got element info
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ATOMICELEMENT SET STATE = %hu WHERE ATOMICELEMENTUID = %ld"),
					 elementState, elementUID);
			retCode = g_localDB.ExecuteSQL(sql);
		}
		else	//	it's not a local element
		{
			//	Update the element record in table ATOMICELEMENT using got element info
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ATOMICELEMENT SET STATE = %hu, ENTRYSTATE = %hu WHERE ATOMICELEMENTUID = %ld"),
					 elementState, ELEMENT_CREATED, elementUID);
			retCode = g_localDB.ExecuteSQL(sql);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Added by Bernie, 2005-Nov-24
	// call Add-in
	// changed by KenQ, 2006-8-30, what ever in callback or startup, the state changing event must be sent out.
	// otherwise, OTE can not get the status changing during DBSync stopping period. 
	if(foundElement == SQL_SUCCESS && g_AdiMan.isValid() /* && gUpdateThreadId != GetCurrentThreadId() */)
	{
		wchar_t provID[DBSA_MAXMDVALUE]={0};
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT amd.MDValue FROM AtomicElementMD amd, EntryMDD emd WHERE amd.MDUID=emd.MDUID AND emd.MDName='ProviderId' AND emd.EntryType=%hu AND amd.LocalAtomicElementUID='%s'"), 
				 IDS_ATOMIC_ELEMENT, elementLUID);
		g_localDB.SelectSQL(sql, SQL_C_TCHAR, provID, sizeof(provID));
		
		wchar_t provAssetID[DBSA_MAXMDVALUE]={0};
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT amd.MDValue FROM AtomicElementMD amd, EntryMDD emd WHERE amd.MDUID=emd.MDUID AND emd.MDName='ProviderAssetId' AND emd.EntryType=%hu AND amd.LocalAtomicElementUID='%s'"), 
				 IDS_ATOMIC_ELEMENT, elementLUID);
		g_localDB.SelectSQL(sql, SQL_C_TCHAR, provAssetID, sizeof(provAssetID));

		DA_entryDb	entryBlock;
		DA_stateDb	stateBlock;

		entryBlock._dwEntryType = DBSA_ENTRY_ELEMENT;
		entryBlock._dwEntryUID = elementUID;
		StringCbCopyW(entryBlock._szLocalEntryUID, DBSA_MAXLUID*sizeof(wchar_t), elementLUID);

		StringCbCopyW(entryBlock._szProviderID, DBSA_MAXMDVALUE*sizeof(wchar_t), provID);
		StringCbCopyW(entryBlock._szProviderAssetID, DBSA_MAXMDVALUE*sizeof(wchar_t), provAssetID);

		stateBlock._dwEntryState = elementState;
		
		g_AdiMan.TrggStat(&entryBlock, &stateBlock);
	}
	// end 
	//////////////////////////////////////////////////////////////////////////

	return retCode;
}


/*****************************************************************************
Function Name:	CheckZQEntryState
Arguments:		Entry ID (DWORD), Entry Name (WCHAR *), Entry Type (WORD)
Returns:		entryZQState (int) 
				(0:normal 1:disabled 2:locally deleted 3:deleted -1:not found)
Description:	This function is to check ZQ_EntryState of given entry, It searchs
				the entry id in corresponding local table according to entry type
				and read the ZQ_ENTRYSTATE if found. If not found, return -1.
*****************************************************************************/
int CDSInterface::CheckZQEntryState(DWORD entryUID, WCHAR *entryName, WORD entryType)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Call CheckZQEntryState"),GetCurrentThreadId());
	int entryZQState = -1;	//	ZQ entry state of the entry
	//	Convert entry name into SQL format
	CString itvName = entryName;	//	entry name in SQL format
	itvName.Replace(_T("'"), _T("''"));

	//	Process according to the type of the entry
	switch (entryType)
	{
	case IDS_APPLICATION:
		if (entryName == NULL)	//	got no entry name
			//	Read ZQ entry state from table HIERARCHY by entry id
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ZQ_ENTRYSTATE FROM HIERARCHY WHERE ENTRYTYPE = %hu AND ENTRYUID = %ld"), 
					 IDS_APPLICATION, entryUID);
		else	//	got entry name
			//	Read ZQ entry state from table FOLDER by entry id or name
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ZQ_ENTRYSTATE FROM HIERARCHY WHERE ENTRYTYPE = %hu AND (ENTRYUID = %ld OR ENTRYNAME = N'%s')"), 
					 IDS_APPLICATION, entryUID, itvName);
		break;
	case IDS_FOLDER:
		if (entryName == NULL)	//	got no folder name
			//	Read ZQ entry state from table FOLDER by folder id
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ZQ_ENTRYSTATE FROM FOLDER WHERE FOLDERUID = %ld"), 
					 entryUID);
		else	//	got folder name
			//	Read ZQ entry state from table FOLDER by folder id or name
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ZQ_ENTRYSTATE FROM FOLDER WHERE FOLDERUID = %ld OR ITVNAME = N'%s'"), 
					 entryUID, itvName);
		break;
	case IDS_ASSET:
		if (entryName == NULL)	//	got no asset name
			//	Read ZQ entry state from table ASSET
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ZQ_ENTRYSTATE FROM ASSET WHERE ASSETUID = %ld"), 
					 entryUID);
		else	//	got asset name
			//	Read ZQ entry state from table ASSET
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ZQ_ENTRYSTATE FROM ASSET WHERE ASSETUID = %ld OR ITVNAME = N'%s'"), 
					 entryUID, itvName);
		break;
	case IDS_ATOMIC_ELEMENT:
		if (entryName == NULL)	//	got no element name
			//	Read ZQ entry state from table ATOMICELEMENT
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ZQ_ENTRYSTATE FROM ATOMICELEMENT WHERE ATOMICELEMENTUID = %ld"), 
					 entryUID);
		else	//	got element name
			//	Read ZQ entry state from table ATOMICELEMENT
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ZQ_ENTRYSTATE FROM ATOMICELEMENT WHERE ATOMICELEMENTUID = %ld OR ITVNAME = N'%s'"), 
					 entryUID, itvName);
		break;
	}
	retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &entryZQState, 0);

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Call CheckZQEntryState"),GetCurrentThreadId());	
	return entryZQState;
}


/*****************************************************************************
Function Name:	SaveObjectMdToDB
Arguments:		Entry ID (DWORD), Entry Type (WORD)
                newAsset - only take effective for asset and element md
Returns:		retCode (DWORD) (0:success 1:fail)
Description:	For one entry (which could be asset, atomic element, folder, 
				application or site), this function reads all of its metadata
				and call corresponding function to write them into local table.
*****************************************************************************/
DWORD CDSInterface::SaveObjectMdToDB(DWORD entryUID, WORD entryType, BOOL newAsset)
{
	DWORD retCode;	//	return code

	METADATA *pMd = 0;	//	pointer to the structure storing metadata info
	DWORD mdNum = 0;	//	number of matadata

	//	Prepare a structure contain query info which will be used to get metadata info
	METADATA *inMd = new METADATA;		//	pointer to the structure containing query info
	memset(inMd, 0, sizeof(METADATA));
	memcpy(&(inMd->Version), &m_itvver, sizeof(ITVVERSION));
	WCHAR wszMdName[] = L"*";
	memcpy(inMd->wszMdName, wszMdName, sizeof(wszMdName));

	switch (entryType)	//	chose type of the entry
	{
	case IDS_ASSET:		//	It's an asset
		//	Read all metadata of the asset

		retCode = IdsReadAsset(&m_idssess, entryUID, &inMd, 1, &pMd, &mdNum, &m_itvsta, 0);	
		if (!retCode)	//	IDS call success
		{
			//	Save all metadata of the asset into DB
			retCode = SaveAssetMdToDB(pMd, mdNum, entryUID, newAsset);
		}else
		{
			(*gpDbSyncLog)(Log::L_INFO, _T("Query the MetaData of Asset %d Failed:IdsReadAsset ERROR!0X%X"),entryUID,retCode);
		}
		break;
	case IDS_ATOMIC_ELEMENT:		//	It's an element
		//	Read all metadata of the element
		retCode = IdsReadAtomicElement(&m_idssess, entryUID, &inMd, 1, &pMd, &mdNum, &m_itvsta, 0);	
		if (!retCode)	//	IDS call success
		{
			//	Save all metadata of the element into DB
			retCode = SaveElementMdToDB(pMd, mdNum, entryUID, newAsset);
		}
		else
		{
			(*gpDbSyncLog)(Log::L_INFO, _T("Query the MetaData of AE %d Failed:IdsReadAtomicElement ERROR!0X%X"),entryUID,retCode);
		}
		break;
	case IDS_FOLDER:		//	It's an folder
		//	Read all metadata of the folder
		retCode = IdsReadFolder(&m_idssess, entryUID, &inMd, 1, &pMd, &mdNum, &m_itvsta, 0);	
		if (!retCode)	//	IDS call success
		{
			//	Save all metadata of the folder into DB
			retCode = SaveFolderMdToDB(pMd, mdNum, entryUID);
		}else
		{
			(*gpDbSyncLog)(Log::L_INFO, _T("Query the MetaData of Folder %d Failed:IdsReadFolder ERROR!0X%X"),entryUID,retCode);
		}
		break;
	case IDS_APPLICATION:		//	It's an application
		//	Read all metadata of the application
		retCode = IdsReadApplication(&m_idssess, entryUID, &inMd, 1, &pMd, &mdNum, &m_itvsta, 0);	
		if (!retCode)	//	IDS call success
		{
			//	Save all metadata of the application into DB
			SaveAppMdToDB(pMd, mdNum, entryUID);
		}else
		{
			(*gpDbSyncLog)(Log::L_INFO, _T("Query the MetaData of APPLICATION %d Failed:IdsReadApplication ERROR!0X%X"),entryUID,retCode);
		}
		break;
	case IDS_CLIP:	// It's an clip
		//  Read all metadata of the clip
		retCode = IdsReadClip(&m_idssess,entryUID,&inMd,1,&pMd,&mdNum,&m_itvsta,0);
		if(!retCode)	// IDS call success
		{
			// Save all metadata of the clip into DB
			SaveClipMdToDB(pMd, mdNum, entryUID);
		}else
		{
			(*gpDbSyncLog)(Log::L_INFO, _T("IdsReadClip ERROR!0X%X"),retCode);
		}
		break;
	case IDS_SITE:		//	It's an site
		//	Read all metadata of the site
		retCode = IdsReadSite(&m_idssess, entryUID, &inMd, 1, &pMd, &mdNum, &m_itvsta, 0);	
		if (!retCode)	//	IDS call success
		{
			//	Save all metadata of the site into DB
			SaveSiteMdToDB(pMd, mdNum, entryUID);
		}else
		{
			(*gpDbSyncLog)(Log::L_INFO, _T("IdsReadSite ERROR!0X%X"),retCode);
		}
		break;
	default:
		retCode = 1;	//	no entry type is matching, something wrong
		break;
	}	//	end switch

	delete inMd;	//	free memory

	IdsFreeMd(pMd, mdNum);	//	free memory

	return retCode;
}


/*****************************************************************************
Function Name:	SaveSiteMdToDB
Arguments:		Site Metadata (METADATA *), Metadata Number (DWORD), Site UID (DWORD)
Returns:		0 (DWORD)
Description:	This function goes through all metadata of the site, and for each
				metadata, local table SITEMD and ENTRYMDD are updated according to:
				1. Look for metadata record per metadata name and type from ENTRYMDD;
				2. If not found, insert a new record into ENTYMDD and SITEMD using 
				   matadata info.
				3. If found, get the ZQ_IsInMD flag;
				4. If opration is to delete, delete all metadata of the site from SITEMD;
				5. If ZQ_IsInMD = 1 and is to delete, delete metadata from SITEMD;
				6. If ZQ_IsInMD = 1 and is not to delete, insert a new record into SITEMD
				   using matadata info.
				7. If ZQ_IsInMD = 0 and is to delete, update the corresponding metadata 
				   field in SITE to "";
				8. If ZQ_IsInMD = 0 and is not to delete, update the corresponding metadata
				   field in SITE using metadata info.
*****************************************************************************/
DWORD CDSInterface::SaveSiteMdToDB(METADATA *pMd, DWORD dwNum, DWORD siteUID)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement
//	if(gUpdateThreadId == GetCurrentThreadId())
	if(m_isFullSyncing)
	{
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("DELETE FROM SITEMD WHERE SITEUID = %d"),siteUID);
		g_localDB.ExecuteSQL(sql);
	}
	
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Call SaveSiteMdToDB."),GetCurrentThreadId());

	for (DWORD i=0; i<dwNum; i++)	//	for each metadata
	{
//		LogMd(pMd+i); //added by W.Z. to trace

		int  isInmd = 1;		//	flag indicating whether the metadata stored in metadata table or master table
		int  isMultiple = 0;	//	flag indicating whether the metadata has multiple value
		int  isLocal = 0;		//	flag indication whether the metadata is locally managed
		TCHAR fieldName[55];	//	string to store ZQ_FIELDNAME
		CString mdName;		//	string to store converted metadata name
		DWORD mdUID = 0;	//	UID of the metadata

		//	Convert metadata name into SQL format
		mdName = (pMd+i)->wszMdName;
		mdName.Replace(_T("'"), _T("''"));
		//optimize code 
		DBRECORD SelectRecord;
		DBRESULTSETS resultsets;
		DBFIELD field1,field2, field3,field4,field5;
		field1.lType = SQL_C_ULONG;
		field2.lType = SQL_C_ULONG;
		field3.lType = SQL_C_ULONG;
		field4.lType = SQL_C_TCHAR;
		field5.lType = SQL_C_ULONG;
		field4.lSize = 64;
		SelectRecord.push_back(field1);
		SelectRecord.push_back(field2);
		SelectRecord.push_back(field3);
		SelectRecord.push_back(field4);
		SelectRecord.push_back(field5);

		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID,ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME,ISMULTIPLE FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu"),
			mdName,	(pMd+i)->wMdType, IDS_SITE);
		retCode =g_localDB.SelectAllSQL(sql,SelectRecord,&resultsets);
		
		if (retCode == SQL_NO_DATA)	//	not found in table ENTRYMDD
		{
			SaveEntryMDDToDBEx(IDS_SITE, (LPCTSTR)mdName);

			//	re-select the MDUID
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID,ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME,ISMULTIPLE FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu"),
				mdName,	(pMd+i)->wMdType, IDS_SITE);
			retCode =g_localDB.SelectAllSQL(sql,SelectRecord,&resultsets);
		}	

		if(retCode == SQL_NO_DATA)
		{
			(*gpDbSyncLog)(Log::L_ERROR,_T("%d:Site MDD %s does not exist in database"),GetCurrentThreadId(), mdName);
			continue;
		}
		else	//	found in table ENTRYMDD
		{
			DBRECORD record = resultsets.at(0);
			memcpy(&mdUID,record.at(0).pValue,4);
			memcpy(&isLocal,record.at(1).pValue,4);
			if(isLocal)
				continue;
			memcpy(&isInmd,record.at(2).pValue,4);			

			if(!isInmd)
				StringCbCopyW(fieldName, 55*sizeof(TCHAR), (wchar_t*)record.at(3).pValue);

			memcpy(&isMultiple,record.at(4).pValue,4);
			g_localDB.FreeResultSets(&resultsets);
		}			
		
		if ((pMd+i)->wMdOp == IDS_DELETE)  // operation is to delete, only happens when callback
		{
			if (isInmd) // the metadata is stored in matadata table
			{
				//	Delete corresponding metadata value record from metadata table
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM SITEMD WHERE SITEUID = %ld AND MDUID = %ld"),
						 siteUID, mdUID);
				retCode = g_localDB.ExecuteSQL(sql);
			}
			else // the metadata is stored in master table
			{
				//	Set corresponding metadata field in master table to NULL
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE SITE SET %s = NULL WHERE SITEUID = %ld"),
						 fieldName, siteUID);
				retCode = g_localDB.ExecuteSQL(sql);
			}
			continue;	//	end processing for current metadata
		}

		if (isInmd) // the metadata is stored in matadata table
		{
			DWORD siteID = 0;	//	Local UID of site
			if (((pMd+i)->wMdOp == IDS_ADD)&&((pMd+i)->wMdOp == 0))	//	the metadata has multiple value
			{
				//	Check if the metadata value exists in metadata table
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT SITEUID FROM SITEMD WHERE SITEUID = %ld AND MDUID = %ld AND MDVALUE = %s"),
						 siteUID, mdUID, _T("N'%s'"));
				retCode = PrepareMdSQL(sql, pMd+i, true);
				retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &siteID, 0);

				if (retCode == SQL_SUCCESS)	//	metadata record exists in metadata table
					continue;	//	skip to process the next metadata
				else	//	metadata record does not exist in metadata table
					//	Insert metadata value in metadata table
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO SITEMD (SITEUID,MDUID,MDVALUE) VALUES (%ld,%ld,%s)"),
							 siteUID, mdUID, _T("N'%s'"));
			}
			else	// IDS_PUT or IDS_REPLACE will not happened as the explain from maynard
			{
				//	Check if the metadata exists in metadata table
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT SITEUID FROM SITEMD WHERE SITEUID = %ld AND MDUID = %ld"),
						 siteUID, mdUID);
				retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &siteID, 0);

				if (retCode == SQL_SUCCESS)	//	metadata record exists in metadata table
					//	Update metadata value in metadata table
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE SITEMD SET MDVALUE = %s WHERE SITEUID = %ld AND MDUID = %ld"),
							 _T("N'%s'"), siteUID, mdUID);
				else	//	metadata record does not exist in metadata table
					//	Insert metadata value in metadata table
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO SITEMD (SITEUID,MDUID,MDVALUE) VALUES (%ld,%ld,%s)"),
							 siteUID, mdUID, _T("N'%s'"));
			}
			retCode = PrepareMdSQL(sql, pMd+i, true);
			if (retCode == 0) {
				retCode = g_localDB.ExecuteSQL(sql);	
			}
		}
		else // the metadata is stored in master table
		{
			//	Update metadata value in master table
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE SITE SET %s = %s WHERE SITEUID = %ld"),
					 fieldName, _T("%s"), siteUID);
			retCode = PrepareMdSQL(sql, pMd+i, false);
			retCode = g_localDB.ExecuteSQL(sql);	
		}
	}  //end of for
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Call SaveSiteMdToDB."),GetCurrentThreadId());
	return 0;	//	success
}


/*****************************************************************************
Function Name:	SaveAppMdToDB
Arguments:		App Metadata (METADATA *), Metadata Number (DWORD), App UID (DWORD)
Returns:		0 (DWORD)
Description:	This function goes through all metadata of the application, and for each
				metadata, local table APPLICATIONMD and ENTRYMDD are updated according to:
				1. Look for metadata record per metadata name and type from ENTRYMDD;
				2. If not found, insert a new record into ENTYMDD and APPLICATIONMD using 
				   matadata info.
				3. If found, get the ZQ_IsInMD flag;
				4. If opration is to delete, delete all metadata of the application from
				   APPLICATIONMD;
				5. If ZQ_IsInMD = 1 and is to delete, delete metadata from APPLICATIONMD;
				6. If ZQ_IsInMD = 1 and is not to delete, insert a new record into 
				   APPLICATIONMD using matadata info.
				7. If ZQ_IsInMD = 0 and is to delete, update the corresponding metadata 
				   field in APPLICATION to "";
				8. If ZQ_IsInMD = 0 and is not to delete, update the corresponding metadata
				   field in APPLICATION using metadata info.
*****************************************************************************/
DWORD CDSInterface::SaveAppMdToDB(METADATA *pMd, DWORD dwNum, DWORD appUID)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement
//	if(gUpdateThreadId == GetCurrentThreadId())
	if(m_isFullSyncing)
	{	
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), L"DELETE FROM APPLICATIONMD WHERE APPLICATIONUID = %ld", appUID);
		retCode = g_localDB.ExecuteSQL(sql);
	}
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Call SaveAppMdToDB to Save Application %d MetaData."),GetCurrentThreadId(),appUID);
	for (DWORD i=0; i<dwNum; i++)	//	for each metadata
	{
//		LogMd(pMd+i); //added by W.Z. to trace

		int  isInmd = 1;		//	flag indicating whether the metadata stored in metadata table or master table
		int  isMultiple = 0;	//	flag indicating whether the metadata has multiple value
		int  isLocal = 0;		//	flag indication whether the metadata is locally managed
		TCHAR fieldName[55];	//	string to store ZQ_FIELDNAME
		CString mdName;		//	string to store converted metadata name
		DWORD mdUID = 0;	//	UID of the metadata

		//	Convert metadata name into SQL format
		mdName = (pMd+i)->wszMdName;
		mdName.Replace(_T("'"), _T("''"));
		//optimize code 
		DBRECORD SelectRecord;
		DBRESULTSETS resultsets;
		DBFIELD field1,field2, field3,field4,field5;
		field1.lType = SQL_C_ULONG;
		field2.lType = SQL_C_ULONG;
		field3.lType = SQL_C_ULONG;
		field4.lType = SQL_C_TCHAR;
		field5.lType = SQL_C_ULONG;
		
		field4.lSize = 64;
		SelectRecord.push_back(field1);
		SelectRecord.push_back(field2);
		SelectRecord.push_back(field3);
		SelectRecord.push_back(field4);
		SelectRecord.push_back(field5);
		
		//	Check if the metadata exists in table ENTRYMDD and get its UID if exists
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID,ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME,ISMULTIPLE FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu"),
			mdName,	(pMd+i)->wMdType, IDS_APPLICATION);
		retCode =g_localDB.SelectAllSQL(sql,SelectRecord,&resultsets);

		if (retCode == SQL_NO_DATA)	//	not found in table ENTRYMDD
		{
			SaveEntryMDDToDBEx(IDS_APPLICATION, (LPCTSTR)mdName);

			//	re-select the MDUID
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID,ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME,ISMULTIPLE FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu"),
				mdName,	(pMd+i)->wMdType, IDS_APPLICATION);
			retCode =g_localDB.SelectAllSQL(sql,SelectRecord,&resultsets);
		}	

		if(retCode == SQL_NO_DATA)
		{
			(*gpDbSyncLog)(Log::L_ERROR,_T("%d:Application MDD %s does not exist in database"),GetCurrentThreadId(), mdName);
			continue;
		}
		else	//	found in table ENTRYMDD
		{
			DBRECORD record = resultsets.at(0);
			memcpy(&mdUID,record.at(0).pValue,4);
			memcpy(&isLocal,record.at(1).pValue,4);
			if(isLocal)
				continue;
			memcpy(&isInmd,record.at(2).pValue,4);			

			if(!isInmd)
				StringCbCopyW(fieldName, 55*sizeof(TCHAR), (wchar_t*)record.at(3).pValue);
			
			memcpy(&isMultiple,record.at(4).pValue,4);			
			g_localDB.FreeResultSets(&resultsets);

		}			
		switch((pMd+i)->wMdOp)
		{
			case IDS_DELETE:  // operation is to delete, only happens when callback

				if (isInmd) // the metadata is stored in matadata table
				{
					//	Delete corresponding metadata value record from metadata table
				//	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM APPLICATIONMD WHERE APPLICATIONUID = %ld AND MDUID = %ld AND MDVALUE = %s"),
				//			 appUID, mdUID, _T("'%s'"));
				//	retCode = PrepareMdSQL(sql, pMd+i, true);
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM APPLICATIONMD WHERE APPLICATIONUID = %ld AND MDUID = %ld "),
							 appUID, mdUID);
					retCode = g_localDB.ExecuteSQL(sql);
				}
				else // the metadata is stored in master table
				{
					//	Set corresponding metadata field in master table to NULL
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE APPLICATION SET %s = NULL WHERE APPLICATIONUID = %ld"),
							 fieldName, appUID);
					retCode = g_localDB.ExecuteSQL(sql);
				}
				break;	//	end processing for current metadata
			case 0:// only happened in service launching
			case IDS_ADD:// happened when call back
			case IDS_PUT:		
				if (isInmd)
				{
					SQLTCHAR MDVALUE[501] = _T("");
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDVALUE FROM APPLICATIONMD WHERE APPLICATIONUID = %ld AND MDUID = %ld AND MDVALUE = %s "),
								 appUID, mdUID, _T("N'%s'"));
					retCode = PrepareMdSQL(sql, pMd+i, true);					
					retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,MDVALUE,sizeof(MDVALUE));	
					if(retCode == SQL_NO_DATA)
					{
						StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO APPLICATIONMD (APPLICATIONUID,MDUID,MDVALUE) VALUES (%ld,%ld,%s)"),
									 appUID, mdUID, _T("N'%s'"));
						retCode = PrepareMdSQL(sql, pMd+i, true);

						if (retCode == 0) {
							retCode = g_localDB.ExecuteSQL(sql);	
						}
					}
				}
				else{
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE APPLICATION SET %s = %s WHERE APPLICATIONUID = %ld"),
						 fieldName, _T("%s"), appUID);
					retCode = PrepareMdSQL(sql, pMd+i, false);
					retCode = g_localDB.ExecuteSQL(sql);
				}			
				break;
			case IDS_REPLACE:			//happened when call back
				if(isInmd)
				{				
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE APPLICATIONMD SET MDVALUE = %s WHERE APPLICATIONUID = %ld AND MDUID = %ld"),
						_T( "N'%s'"), appUID, mdUID);
					retCode = PrepareMdSQL(sql, pMd+i, true);
					retCode = g_localDB.ExecuteSQL(sql);
				}else
				{
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE APPLICATION SET %s = %s WHERE APPLICATIONUID = %ld"),
						 fieldName, _T("%s"), appUID);
					retCode = PrepareMdSQL(sql, pMd+i, false);
					retCode = g_localDB.ExecuteSQL(sql);
				}
				break;
			default:
				break;
		}		
	}  //end of for
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Call SaveAppMdToDB to Save Application %d MetaData."),GetCurrentThreadId(),appUID);
	return 0;	//	success
}

/*****************************************************************************
Function Name:	SaveFolderMdToDB
Arguments:		Folder Metadata (METADATA *), Metadata Number (DWORD), Folder UID (DWORD)
Returns:		0 (DWORD)
Description:	This function goes through all metadata of the folder, and for each
				metadata, local table FOLDERMD and ENTRYMDD are updated according to:
				1. Look for metadata record per metadata name and type from ENTRYMDD;
				2. If not found, insert a new record into ENTYMDD and FOLDERMD using 
				   matadata info.
				3. If found, get the ZQ_IsInMD flag;
				4. If opration is to delete, delete all metadata of the folder from
				   FOLDERMD;
				5. If ZQ_IsInMD = 1 and is to delete, delete metadata from FOLDERMD;
				6. If ZQ_IsInMD = 1 and is not to delete, insert a new record into 
				   FOLDERMD using matadata info.
				7. If ZQ_IsInMD = 0 and is to delete, update the corresponding metadata 
				   field in FOLDER to "";
				8. If ZQ_IsInMD = 0 and is not to delete, update the corresponding metadata
				   field in FOLDER using metadata info.
*****************************************************************************/
DWORD CDSInterface::SaveFolderMdToDB(METADATA *pMd, DWORD dwNum, DWORD folderUID)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement

//	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), "DELETE FROM FOLDERMD WHERE FOLDERUID = %ld", folderUID);
//	retCode = g_localDB.ExecuteSQL(sql);
//	if(gUpdateThreadId == GetCurrentThreadId())
	if(m_isFullSyncing)
	{
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("DELETE FROM FOLDERMD WHERE LOCALFOLDERUID IN (SELECT LOCALFOLDERUID FROM FOLDER WHERE FOLDERUID = %d )"),
			folderUID);
		g_localDB.ExecuteSQL(sql);
	}
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Call SaveFolderMdToDB to Save Folder %d MetaData."),GetCurrentThreadId(),folderUID);
	
	SQLTCHAR folderLUID[21];	//	Local UID of folder
	memset(folderLUID,0,sizeof(folderLUID));
	
	//	Get Local UID of the folder
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALFOLDERUID FROM FOLDER WHERE FOLDERUID = %ld"),
		folderUID);
	retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, folderLUID, sizeof(folderLUID));
	if (retCode == SQL_NO_DATA)	//	folder not found
		return 0;		//	end processing for current metadata

	BOOL isThisPackage = IsPackaged(folderLUID);
	bool saveToWQ = false;
	
	for (DWORD i = 0; i<dwNum; i++)	//	for each metadata
	{
//		LogMd(pMd+i); //added by W.Z. to trace

		int  isInmd = 1;		//	flag indicating whether the metadata stored in metadata table or master table
		int  isMultiple = 0;	//	flag indicating whether the metadata has multiple value
		int  isLocal = 0;		//	flag indication whether the metadata is locally managed
		TCHAR fieldName[55];	//	string to store ZQ_FIELDNAME
		CString mdName;		//	string to store converted metadata name
		DWORD mdUID = 0;	//	UID of the metadata

		//	Convert metadata name into SQL format
		mdName = (pMd+i)->wszMdName;
		mdName.Replace(_T("'"), _T("''"));
		//optimize code 
		DBRECORD SelectRecord;
		DBRESULTSETS resultsets;
		DBFIELD field1,field2, field3,field4,field5;
		field1.lType = SQL_C_ULONG;
		field2.lType = SQL_C_ULONG;
		field3.lType = SQL_C_ULONG;
		field4.lType = SQL_C_TCHAR;
		field5.lType = SQL_C_ULONG;
		field4.lSize = 64;
		SelectRecord.push_back(field1);
		SelectRecord.push_back(field2);
		SelectRecord.push_back(field3);
		SelectRecord.push_back(field4);
		SelectRecord.push_back(field5);

		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID,ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME,ISMULTIPLE  FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu"),
			mdName,	(pMd+i)->wMdType, IDS_FOLDER);
		retCode =g_localDB.SelectAllSQL(sql,SelectRecord,&resultsets);

		if (retCode == SQL_NO_DATA)	//	not found in table ENTRYMDD
		{
			SaveEntryMDDToDBEx(IDS_FOLDER, (LPCTSTR)mdName);

			//	re-select the MDUID
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID,ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME,ISMULTIPLE  FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu"),
				mdName,	(pMd+i)->wMdType, IDS_FOLDER);
			retCode =g_localDB.SelectAllSQL(sql,SelectRecord,&resultsets);
		}	
		else	//	found in table ENTRYMDD
		{
			DBRECORD record = resultsets.at(0);
			memcpy(&mdUID,record.at(0).pValue,4);
			memcpy(&isLocal,record.at(1).pValue,4);
			if(isLocal)
				continue;
			memcpy(&isInmd,record.at(2).pValue,4);			

			if(!isInmd)
				StringCbCopyW(fieldName, 55*sizeof(TCHAR), (wchar_t*)record.at(3).pValue);

			memcpy(&isMultiple,record.at(4).pValue,4);

			g_localDB.FreeResultSets(&resultsets);
		}			
		
		if((pMd+i)->wMdOp ==IDS_DELETE)
		{
			if (isInmd) // the metadata is stored in matadata table
			{						
					//	Delete corresponding metadata value record from metadata table
					//StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM FOLDERMD WHERE LOCALFOLDERUID = '%s' AND MDUID = %ld AND MDVALUE = %s"),
					//		 folderLUID, mdUID, _T("'%s'"));
					//retCode = PrepareMdSQL(sql, pMd+i, true);
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM FOLDERMD WHERE LOCALFOLDERUID = '%s' AND MDUID = %ld "),
							 folderLUID, mdUID);
					retCode = g_localDB.ExecuteSQL(sql);
				
			}
			else // the metadata is stored in master table
			{
				//	Set corresponding metadata field in master table to NULL
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE FOLDER SET %s = NULL WHERE LOCALFOLDERUID = '%s'"),
						 fieldName, folderLUID);
				retCode = g_localDB.ExecuteSQL(sql);
			}
			continue;
		}
		// throw to workqueue only when MetaData IsPackage changed
		// main handle when packet metadata is set.
		if(wcscmp((pMd+i)->wszMdName,_T("IsPackage"))==0)
		{
			// get old MDVALUE				
// --------- commented by KenQ at 2007-1-30, coz LAM change this value actively if the entry is from LAM, so actually, 
//			 most of the time the value in callback and local db are same, so still need to operate WQ and RefcountRating control

//			if(isThisPackage != (BOOL)(pMd+i)->bitVal)   
//			{
				SQLTCHAR parentLUID[21];
				if((pMd+i)->bitVal)
				{
					SQLTCHAR parentHUID[21];
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("UPDATE FOLDER SET ISPACKAGE = 1 WHERE LOCALFOLDERUID = '%s'"),folderLUID);
					retCode = g_localDB.ExecuteSQL(sql);
					

					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT LOCALPARENTHUID FROM HIERARCHY WHERE LOCALENTRYUID = '%s'"),folderLUID);
					retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,parentHUID,sizeof(parentHUID));

					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT LOCALHIERARCHYUID FROM HIERARCHY WHERE LOCALHIERARCHYUID = '%s'"),parentHUID);
					retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,parentLUID,sizeof(parentLUID));
		
//	----------- modified by KenQ 2006-11-21 for KDDI requirement -------------
					// The folder's IsPackage metadata changed, it is recommended to invoked before WQ operation
					if(g_SupportRefcountRatingctrl)
					{
						(*gpDbSyncLog)(Log::L_INFO, _T("Folder %s IsPackage changed, need to recalculate Rating and ReferenceCount"), folderLUID);
						FolderRatingRef(folderLUID, m_titleSortNameMDUID);
					}

					if(!m_isFullSyncing) 
					{   // condition added at 2007-12-7, otherwise there always WQ inserted to each folder/package during full syncing
						SaveMsgtoWorkQueue(folderLUID,parentLUID,IDS_PACKAGE,WORKQUEUE_LINK,L"",L"");
					}
				}
				else
				{
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("UPDATE FOLDER SET ISPACKAGE = 0 WHERE LOCALFOLDERUID = '%s'"),folderLUID);
					retCode = g_localDB.ExecuteSQL(sql);

//	----------- modified by KenQ 2006-11-21 for KDDI requirement -------------
					// The folder's IsPackage metadata changed, it is recommended to invoked before WQ operation
					if(g_SupportRefcountRatingctrl)
					{
						(*gpDbSyncLog)(Log::L_INFO, _T("Folder %s IsPackage changed, need to recalculate Rating and ReferenceCount"), folderLUID);
						FolderRatingRef(folderLUID, m_titleSortNameMDUID);
					}

					if(!m_isFullSyncing) 
					{   // condition added at 2007-12-7, otherwise there always WQ inserted to each folder/package during full syncing
						SaveMsgtoWorkQueue(folderLUID,L"",IDS_PACKAGE,WORKQUEUE_DELETE,L"",L"");
					}
				}
// 			} // end of comparing old/new isPackage value
		}					

		int isObjectLocal = 0;		//	flag indication whether the folder is locally managed
		//	Get isLocal flag of the folder
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ISLOCAL FROM FOLDER WHERE LOCALFOLDERUID = '%s'"),
				 folderLUID);
		retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &isObjectLocal, 0);
		if (isObjectLocal && isLocal)	//	the metadata and the folder are both locally managed
			continue;	//	skip to process the next metadata

		if (isInmd) // the metadata is stored in metadata table
		{
			DWORD folderID = 0;	//	UID of folder

			// changed by Ken Qian at 2007-3-6, we need to use isMultiple as the condition.
			// Original condition is not right when there have been data in lam. 
			// if (((pMd+i)->wMdOp == IDS_ADD)||((pMd+i)->wMdOp == 0))	//	the metadata has multiple value
			if(1 == isMultiple)
			{
				//	Check if the metadata value exists in metadata table
				SQLTCHAR MDVALUE[501] = _T("");
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDVALUE FROM FOLDERMD WHERE LOCALFOLDERUID = '%s' AND MDUID = %ld AND MDVALUE = %s"),
						 folderLUID,  mdUID, _T("N'%s'"));
				retCode = PrepareMdSQL(sql, pMd+i, true);
				
				retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,MDVALUE,sizeof(MDVALUE));	
				if(retCode== SQL_NO_DATA)
				{
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO FOLDERMD (LOCALFOLDERUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
							 folderLUID,  mdUID, _T("N'%s'"));
					retCode = PrepareMdSQL(sql, pMd+i, true);
					if (retCode == 0) {
						retCode = g_localDB.ExecuteSQL(sql);	
						if(isThisPackage)
						{
							if(wcscmp((pMd+i)->wszMdName,L"IsPackage") != 0)// Package add and delete is set previously.
							{		
	//							SaveMsgtoWorkQueue(folderLUID,L"",IDS_PACKAGE,WORKQUEUE_UPDATE,(pMd+i)->wszMdName,L"");
								saveToWQ = true;
							}
						}
					}
				}
			}
			else	//	the metadata has only one value
			{
				//	Check if the metadata exists in metadata table
				SQLTCHAR MDVALUE[500];
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDVALUE FROM FOLDERMD WHERE LOCALFOLDERUID = '%s' AND MDUID = %ld"),
						 folderLUID, mdUID);
				retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, MDVALUE, sizeof(MDVALUE));

				if (retCode == SQL_SUCCESS)	//	metadata record exists in metadata table
					//	Update metadata value in metadata table
				{
					if(VerifyMD(pMd+i,MDVALUE) != 0)// MdVALUE changed
					{						
						if(isThisPackage)
						{
							if(wcscmp((pMd+i)->wszMdName,L"IsPackage") != 0)// Package add and delete is set previously.
							{
//								SaveMsgtoWorkQueue(folderLUID,L"",IDS_PACKAGE,WORKQUEUE_UPDATE,(pMd+i)->wszMdName,L"");
								saveToWQ = true;
							}
						}
						StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE FOLDERMD SET MDVALUE = %s WHERE LOCALFOLDERUID = '%s' AND MDUID = %ld"),
								 _T("N'%s'"), folderLUID, mdUID);
						retCode = PrepareMdSQL(sql, pMd+i, true);
						if (retCode == 0) {
							retCode = g_localDB.ExecuteSQL(sql);	
						}
					}
				}
				else	//	metadata record does not exist in metadata table
					//	Insert metadata value in metadata table
				{
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO FOLDERMD (LOCALFOLDERUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
							 folderLUID,  mdUID, _T("N'%s'"));
					retCode = PrepareMdSQL(sql, pMd+i, true);
					if (retCode == 0) {
						retCode = g_localDB.ExecuteSQL(sql);	
					}
				}
			}
//	----------- modified by KenQ 2006-11-21 for KDDI requirement: ReferenceCount and RatingControl -------------
			if(g_SupportRefcountRatingctrl && wcscmp((pMd+i)->wszMdName,MDNAME_RATING)==0)
			{  
				// Folder's Rating is changed 
				if(isThisPackage)
				{
					(*gpDbSyncLog)(Log::L_INFO, _T("Received Folder %s Rating MD callback, new rating is %d, need to recalculate Rating and ReferenceCount"), folderLUID, (pMd+i)->iVal);
					
					FolderRatingRef(folderLUID, m_titleSortNameMDUID);
				}
			}
		}
		else // the metadata is stored in master table
		{
			//	Update metadata value in master table
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE FOLDER SET %s = %s WHERE LOCALFOLDERUID = '%s'"),
					 fieldName, _T("%s"), folderLUID);
			retCode = PrepareMdSQL(sql, pMd+i, false);
			retCode = g_localDB.ExecuteSQL(sql);	
		}
	}  //end of for

	if(saveToWQ) {
		SaveMsgtoWorkQueue(folderLUID,L"",IDS_PACKAGE,WORKQUEUE_UPDATE,L"",L"");
	}


	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Call SaveFolderMdToDB to Save Folder %d MetaData."),GetCurrentThreadId(),folderUID);
	return 0;	//	success
}


/*****************************************************************************
Function Name:	SaveAssetMdToDB
Arguments:		Asset Metadata (METADATA *), Metadata Number (DWORD), Asset UID (DWORD)
                newAsset - whether to it is a new asset
Returns:		0 (DWORD)
Description:	This function goes through all metadata of the asset, and for each
				metadata, local table ASSETMD and ENTRYMDD are updated according to:
				1. Look for metadata record per metadata name and type from ENTRYMDD;
				2. If not found, insert a new record into ENTYMDD and ASSETMD using 
				   metadata info.
				3. If found, get the ZQ_IsInMD flag;
				4. If operation is to delete, delete all metadata of the asset from
				   ASSETMD;
				5. If ZQ_IsInMD = 1 and is to delete, delete metadata from ASSETMD;
				6. If ZQ_IsInMD = 1 and is not to delete, insert a new record into 
				   ASSETMD using metadata info.
				7. If ZQ_IsInMD = 0 and is to delete, update the corresponding metadata 
				   field in ASSET to "";
				8. If ZQ_IsInMD = 0 and is not to delete, update the corresponding metadata
				   field in ASSET using metadata info.
*****************************************************************************/
DWORD CDSInterface::SaveAssetMdToDB(METADATA *pMd, DWORD dwNum, DWORD assetUID, BOOL newAsset)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024] = _T("");	//	string to put SQL statement

//	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), "DELETE FROM ASSETMD WHERE ASSETUID = %ld", assetUID);
//	retCode = g_localDB.ExecuteSQL(sql);
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Call SaveAssetMdToDB to Save Metadate of asset %d"),GetCurrentThreadId(),assetUID);
	
	bool noLastStamp = false;
	IDSUPDATESTAMP dummy={0,0,0,0,0,0,0,0};
	if(0==memcmp(g_LastUpdateStamp, dummy, sizeof(g_LastUpdateStamp)))
		noLastStamp = true;

	if(m_isFullSyncing)
	{
		if(!g_SupportUpdateStamp || noLastStamp)
		{
			// only delete all metadata if no stamp is found
			// otherwise take care of it after checking stamp
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("DELETE FROM ASSETMD WHERE LOCALASSETUID IN (SELECT LOCALASSETUID FROM ASSET WHERE ASSETUID = %d )"),
				assetUID);
			g_localDB.ExecuteSQL(sql);
		}
	}
	// KenQ 2007-11-29 the check whether the asset is existing or not
	// coz if a new asset is uploaded, the metadata callback comes before FolderCallback. 
	// remember the asset id, to avoid the query database for continued metadata
	if(!m_isFullSyncing && m_notFoundAssetId.find(assetUID) != m_notFoundAssetId.end())
	{
		(*gpDbSyncLog)(Log::L_DEBUG,_T("asset %ld does not exist, skip it."), assetUID);
		return 0;
	}
	
	SQLTCHAR assetLUID[21];	//	Local UID of asset
	memset(assetLUID,0,sizeof(assetLUID));
	//	Get Local UID of the asset
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALASSETUID FROM ASSET WHERE ASSETUID = %ld"),
		assetUID);
	retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, assetLUID, sizeof(assetLUID));
	if (retCode == SQL_NO_DATA)	//	asset not found
	{
		// KenQ 2007-11-29 the asset does not exist, remember it,
		// coz if a new asset is uploaded, the metadata callback comes before FolderCallback. 
		// remember the asset id, to avoid the query database for continued metadata
		m_notFoundAssetId.insert(IDMAP::value_type(assetUID, assetUID));
		
		return 0;
	}
	int isObjectLocal = 0;		//	flag indication whether the asset is locally managed

	//	Get isLocal flag of the asset
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ISLOCAL FROM ASSET WHERE LOCALASSETUID = '%s'"),
		assetLUID);
	retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &isObjectLocal, 0);

	DA_entryDb	entryBlock;
	DA_metaDb*	metaBlockList = NULL;

//	if(g_AdiMan.isValid() && gUpdateThreadId != GetCurrentThreadId())
	if(g_AdiMan.isValid() && !m_isFullSyncing)
	{
		// prepare entry block for DBSync Add-in use
		entryBlock._dwEntryType = DBSA_ENTRY_ASSET;
		entryBlock._dwEntryUID = assetUID;
		StringCbCopyW(entryBlock._szLocalEntryUID, DBSA_MAXLUID*sizeof(wchar_t), assetLUID);

		metaBlockList = new DA_metaDb[dwNum];
		if(metaBlockList)
			memset(metaBlockList, 0, sizeof(DA_metaDb)*dwNum);
	}
	
	bool saveToWQ = false;
	for (DWORD i=0; i<dwNum; i++)	//	for each metadata
	{
//		LogMd(pMd+i); //added by W.Z. to trace

		int  isInmd = 1;		//	flag indicating whether the metadata stored in metadata table or master table
		int  isMultiple = 0;	//	flag indicating whether the metadata has multiple value
		int  isLocal = 0;		//	flag indication whether the metadata is locally managed
		TCHAR fieldName[55] = _T("");	//	string to store ZQ_FIELDNAME
		CString mdName;		//	string to store converted metadata name
		DWORD mdUID = 0;	//	UID of the metadata

		//	Convert metadata name into SQL format
		mdName = (pMd+i)->wszMdName;
		mdName.Replace(_T("'"), _T("''"));

		//////////////////////////////////////////////////////////////////////////
		// ----------------------------------------+------------------------------
		// Extra update logic when DBSync starts.  |     By Bernie.Z, 2006-Feb-20
		//                                         | getting a cold, sleepy... T_T
		// ----------------------------------------+------------------------------
		// When DBSync starts, it will perform a full synchronization which would
		// consume hours of time depending on the asset and element number.
		// The process itself is slow because it re-fetch all the metadata values
		// of all the assets/elements.  Each of the fetch would take around 
		// 10-20ms, thus making the entire process time a linear sum of metadata
		// number multiple this 10-20ms.
		//
		// The following logic ignores those metadata updates when its time stamp
		// is older than the one DBSync recorded in the registry during the last
		// run.  These metadatas has not been changed since the last run so we can
		// simply skip them.
		//
		// the stamp of DBSync recorded would be updated into the registry upon 
		// every metadata change callback.
		// 
		//
		// --------------------------------- begin -------------------------------
		
		if((pMd+i)==NULL)	// memory invalid
			continue;
		
		// add by KenQ at 2007-6-5, ignore RefCount in case of support KDDI
		if(g_SupportRefcountRatingctrl)
		{
			if(wcscmp(mdName, L"RefCount") == 0)
			{
				continue;
			}
		}

		IDSUPDATESTAMP	tmpStamp = {0,0,0,0,0,0,0,0};
		if((pMd+i)->MdStamp)
			memcpy(tmpStamp, (pMd+i)->MdStamp, sizeof(tmpStamp));
		
		if(m_isFullSyncing && g_SupportUpdateStamp && !newAsset) // only skip metadata update when start, not on callbacks
		{			
			if(g_server.CompareStamp(tmpStamp, g_CurrUpdateStamp)>0)
			{
				// save the greatest stamp into temp stamp, this temp stamp will be
				// copied into LastUpdateStamp after finishing the starting
				memcpy(g_CurrUpdateStamp, tmpStamp, sizeof(g_CurrUpdateStamp));
			}

			int comRet = g_server.CompareStamp(tmpStamp, g_LastUpdateStamp);
			if(comRet <= 0)
			{
				// this metadata is updated earlier than last stamp
				(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:metadata \"%s\" update is skipped during starting, stamp=%02X%02X%02X%02X%02X%02X%02X%02X"),
					GetCurrentThreadId(), (pMd+i)->wszMdName, tmpStamp[0], tmpStamp[1], tmpStamp[2], tmpStamp[3], tmpStamp[4], tmpStamp[5], tmpStamp[6], tmpStamp[7]);
				continue;
			}
		}
		else if(!m_isFullSyncing && g_SupportUpdateStamp && g_SyncAllAtBegin != 0)	// if callback comes, update the last stamp
			    /*
					This condition is added on 2006-12-26, the timestamp ONLY be able to update 
					in case of have full syncing at the beginning.
				*/
		{
			memcpy(g_LastUpdateStamp, tmpStamp, sizeof(g_LastUpdateStamp));
			g_server.SaveLastStamp();
		}

		// ---------------------------------- end --------------------------------
		//////////////////////////////////////////////////////////////////////////
		
		DBRECORD queryRecord;
		DBFIELD field1,field2,field3,field4,field5;
		DBRESULTSETS ResultSets;
		
		if ((pMd+i)->dwMdAppUid == 0)	//	it's a system metadata
		{
			//	Check if the metadata exists in table ENTRYMDD and get its UID if exists
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID, ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME, ISMULTIPLE FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu AND APPLICATIONUID = 0"),
				mdName, (pMd+i)->wMdType, IDS_ASSET);
			
			field1.lType = SQL_C_ULONG;
			field2.lType = SQL_C_ULONG;
			field3.lType = SQL_C_ULONG;
			field4.lType = SQL_C_TCHAR;
			field5.lType = SQL_C_ULONG;
			field4.lSize = 64;
			
			queryRecord.push_back(field1);
			queryRecord.push_back(field2);
			queryRecord.push_back(field3);
			queryRecord.push_back(field4);
			queryRecord.push_back(field5);
			
			
			retCode = g_localDB.SelectAllSQL(sql,queryRecord,&ResultSets);
		}
		else	//	it's an application specified metadata
		{
			//	Check if the metadata exists in table ENTRYMDD and get its UID if exists
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID, ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME, ISMULTIPLE FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu AND (APPLICATIONUID <> 0 OR APPLICATIONUID IS NULL)"),
				mdName, (pMd+i)->wMdType, IDS_ASSET);
			
			field1.lType = SQL_C_ULONG;
			field2.lType = SQL_C_ULONG;
			field3.lType = SQL_C_ULONG;
			field4.lType = SQL_C_TCHAR;
			field5.lType = SQL_C_ULONG;
			field4.lSize = 64;
			
			queryRecord.push_back(field1);
			queryRecord.push_back(field2);
			queryRecord.push_back(field3);
			queryRecord.push_back(field4);
			queryRecord.push_back(field5);
						
			retCode = g_localDB.SelectAllSQL(sql,queryRecord,&ResultSets);
		}

		if (retCode == SQL_NO_DATA)	//	not found in table ENTRYMDD
		{			
			SaveEntryMDDToDBEx(IDS_ASSET, (LPCTSTR)mdName);

			//	re-select the MDUID
			if ((pMd+i)->dwMdAppUid == 0)	//	it's a system metadata
			{
				//	Check if the metadata exists in table ENTRYMDD and get its UID if exists
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID, ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME, ISMULTIPLE FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu AND APPLICATIONUID = 0"),
					mdName, (pMd+i)->wMdType, IDS_ASSET);
				
				field1.lType = SQL_C_ULONG;
				field2.lType = SQL_C_ULONG;
				field3.lType = SQL_C_ULONG;
				field4.lType = SQL_C_TCHAR;
				field5.lType = SQL_C_ULONG;
				field4.lSize = 64;
				
				queryRecord.push_back(field1);
				queryRecord.push_back(field2);
				queryRecord.push_back(field3);
				queryRecord.push_back(field4);
				queryRecord.push_back(field5);
				
				
				retCode = g_localDB.SelectAllSQL(sql,queryRecord,&ResultSets);
			}
			else	//	it's an application specified metadata
			{
				//	Check if the metadata exists in table ENTRYMDD and get its UID if exists
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID, ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME, ISMULTIPLE FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu AND (APPLICATIONUID <> 0 OR APPLICATIONUID IS NULL)"),
					mdName, (pMd+i)->wMdType, IDS_ASSET);
				
				field1.lType = SQL_C_ULONG;
				field2.lType = SQL_C_ULONG;
				field3.lType = SQL_C_ULONG;
				field4.lType = SQL_C_TCHAR;
				field5.lType = SQL_C_ULONG;
				field4.lSize = 64;
				
				queryRecord.push_back(field1);
				queryRecord.push_back(field2);
				queryRecord.push_back(field3);
				queryRecord.push_back(field4);
				queryRecord.push_back(field5);
							
				retCode = g_localDB.SelectAllSQL(sql,queryRecord,&ResultSets);
			}
		}
		
		if(retCode == SQL_NO_DATA)
		{
			(*gpDbSyncLog)(Log::L_ERROR,_T("%d:Asset MDD %s does not exist in database"),GetCurrentThreadId(), mdName);
			continue;
		}
		else	//	found in table ENTRYMDD
		{
			DBRECORD record = ResultSets.at(0);
			
			memcpy(&mdUID,record.at(0).pValue,4);
			memcpy(&isLocal,record.at(1).pValue,4);
			if(isLocal)
				continue;
			memcpy(&isInmd,record.at(2).pValue,4);
			if(!isInmd)
			{
				StringCbCopyW(fieldName, 55*sizeof(TCHAR), (wchar_t*)record.at(3).pValue);
			}
			memcpy(&isMultiple,record.at(4).pValue,4);
			g_localDB.FreeResultSets(&ResultSets);
		}			

//		if(g_AdiMan.isValid() && gUpdateThreadId != GetCurrentThreadId() && metaBlockList!=NULL)
		if(g_AdiMan.isValid() && !m_isFullSyncing && metaBlockList!=NULL)
		{
		
			// prepare metadata block for DBSync Add-in use
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("%s"),_T("%s"));
			PrepareMdSQL(sql, pMd+i, true);

			StringCbCopyW(metaBlockList[i]._szMdName, DBSA_MAXMDNAME*sizeof(wchar_t), (pMd+i)->wszMdName);
			StringCbCopyNW(metaBlockList[i]._szMdValue, DBSA_MAXMDVALUE*sizeof(wchar_t), sql, (DBSA_MAXMDVALUE-1)*sizeof(wchar_t));
			metaBlockList[i]._dwMdUID = mdUID;
			metaBlockList[i]._dwAppUID = (pMd+i)->dwMdAppUid;
			metaBlockList[i]._dwOp = (pMd+i)->wMdOp;
			
		}
		
	
		if ((pMd+i)->wMdOp == IDS_DELETE)  // operation is to delete, only happens when callback
		{
			if (isInmd) // the metadata is stored in metadata table
			{
				bool hasTSN = false;
				SQLTCHAR oldTSN[1024] = _T("");
				if(g_SupportRefcountRatingctrl && (mdUID == m_titleSortNameMDUID) )
				{						
					// get the titleSortName
					hasTSN = GetAssetTitleSortName(assetLUID, oldTSN, 1000);
				}

				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM ASSETMD WHERE LOCALASSETUID = '%s' AND MDUID = %ld "),
						 assetLUID, mdUID);
				//retCode = PrepareMdSQL(sql, pMd+i, true);
				retCode = g_localDB.ExecuteSQL(sql);
				if(retCode == SQL_SUCCESS) {
			//		SaveMsgtoWorkQueue(assetLUID,L"",IDS_ASSET,WORKQUEUE_UPDATE,(pMd+i)->wszMdName,L"");	
					saveToWQ = true;
				}

				if(hasTSN)
				{
					(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s TitleSortName MD was deleted, need to recalculate ReferenceCount for TitleSortName %s"), assetLUID, oldTSN);
					
					AssetRefOneTitle(assetLUID, m_titleSortNameMDUID, oldTSN);
				}
			}
			else // the metadata is stored in master table
			{
				//	Set corresponding metadata field in master table to NULL
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ASSET SET %s = NULL WHERE LOCALASSETUID = '%s'"),
						 fieldName, assetLUID);
				retCode = g_localDB.ExecuteSQL(sql);
			}

			continue;	//	end processing for current metadata
		}

		
		if (isObjectLocal && isLocal)	//	the metadata and the asset are both locally managed
			continue;	//	skip to process the next metadata

		if (isInmd) // the metadata is stored in metadata table
		{
			DWORD assetID = 0;	//	UID of asset
			if (((pMd+i)->wMdOp == IDS_ADD)||((pMd+i)->wMdOp ==0))	//	the metadata has multiple value
			{
				//	Check if the metadata value exists in metadata table
				SQLTCHAR MDVALUE[501] = _T("");
				// add by kaliven lee 05.4.20 boring problem
				// add a lock here to avoid multiple threads security problem
				// in extreme condition, it will cause duplicate metadata when import lots of asset into ITV
				ZQ::common::MutexGuard tmpGd(m_AssetMDTableLock);

				if(m_isFullSyncing)
				{
					// changed by KenQ 2007-4-12, even though these md has been deleted incase of noLastStamp, 
					// but if LAM insert md, that would cause single value type md have multiple values,
					// RefCount is a case in KDDI. 
					// but this changing will reduce performance, coz to single type, must delete MD again, 
					// actually they all have been deleted. 

					// if(noLastStamp || isMultipleEntry(i, pMd) )
					if(1 == isMultiple)
					{
						//add here
						StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDVALUE FROM ASSETMD WHERE LOCALASSETUID = '%s' AND MDUID = %ld AND MDVALUE = %s"),
							assetLUID, mdUID,_T("N'%s'"));
					
						retCode = PrepareMdSQL(sql, pMd+i, true);
						retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,MDVALUE,sizeof(MDVALUE));
						
						if(retCode == SQL_NO_DATA)  // no existing one there, insert new one
						{
							
							// case 1: If no last update stamp found, meaning all md of this asset have been deleted
							// case 2: this md is the same as previous one, so it is multi-value, and all md of this name have been deleted
							// result: do NOT delete metadatas with this name here, JUST insert
							StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO ASSETMD (LOCALASSETUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
								assetLUID, mdUID, _T("N'%s'"));
							retCode = PrepareMdSQL(sql, pMd+i, true);
							retCode = g_localDB.ExecuteSQL(sql);
							
							//	----------- modified by KenQ 2006-11-22 for KDDI requirement: ReferenceCount and RatingControl -------------
							//  here is the case of could new asset was created / or new metadata was added.
							//  To the creating case, following block will execute twice in this function, anyway, forget it.
							if(g_SupportRefcountRatingctrl && (wcscmp((pMd+i)->wszMdName,MDNAME_RATING)==0 || wcscmp((pMd+i)->wszMdName,MDNAME_ASSET_TITLESORTNAME)==0) )
							{
								(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s metadata %s was created, need to recalculate Rating and ReferenceCount"), assetLUID, (pMd+i)->wszMdName);
								AssetRatingRef(assetLUID, m_titleSortNameMDUID, true);
							}
						}
					}
					else
					{
	//	----------- modified by KenQ 2006-11-22 for KDDI requirement: ReferenceCount and RatingControl -------------
	//  here is the case of Rating/TitleSortName changing
						bool bProcessRating = false;
						bool bProcessTitleSortName = false;
						bool bHasTSN = true;
						SQLTCHAR oldTSN[1024] = _T("");

						if(g_SupportRefcountRatingctrl && wcscmp((pMd+i)->wszMdName,MDNAME_ASSET_TITLESORTNAME)==0)
						{
							// comment by Ken to remove the comparing logic at 2007-6-4
							bHasTSN = GetAssetTitleSortName(assetLUID, oldTSN, 1000);
							bProcessTitleSortName = true;
						}

						// delete metadatas with this name, and insert new one
					/*	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
						_T("DELETE FROM ASSETMD WHERE LOCALASSETUID='%s' AND MDUID=%ld  INSERT INTO ASSETMD (LOCALASSETUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
							assetLUID, mdUID, assetLUID, mdUID, _T("N'%s'"));
						retCode = PrepareMdSQL(sql, pMd+i, true);
						retCode = g_localDB.ExecuteSQL(sql);
                   */
						StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
						_T("DELETE FROM ASSETMD WHERE LOCALASSETUID='%s' AND MDUID=%ld"),
							assetLUID, mdUID);
						retCode = g_localDB.ExecuteSQL(sql);

						StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
							_T("INSERT INTO ASSETMD (LOCALASSETUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
							assetLUID, mdUID, _T("N'%s'"));
						retCode = PrepareMdSQL(sql, pMd+i, true);
						retCode = g_localDB.ExecuteSQL(sql);

						
	//	----------- modified by KenQ 2006-11-22 for KDDI requirement: ReferenceCount and RatingControl -------------
	//  here is the case of Rating/TitleSortName changing
						if(g_SupportRefcountRatingctrl && wcscmp((pMd+i)->wszMdName,MDNAME_RATING)==0)
						{
							(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s Rating is %d, recalculate Rating and ReferenceCount"), assetLUID, (pMd+i)->iVal);
							AssetRatingRef(assetLUID, m_titleSortNameMDUID, true);
						}
						if(bProcessTitleSortName)
						{
							// changed by Ken to remove the comparing above logic at 2007-6-4
							(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s TitleSortName is changed from %s to %s."), assetLUID, oldTSN, (pMd+i)->sVal);
							AssetRefTwoTitles(assetLUID, m_titleSortNameMDUID, bHasTSN ? oldTSN : NULL, (pMd+i)->sVal);
						}
					}
				}
				else
				{
//	---------- modified by KenQ 2006-11-14-------------
//  here I add logic to process case: two assets in ITV AM with same name but in different character case, (ITV AM is case-sensitive, LAM is not)
//  in LAM, there is only one asset but with some duplicate metadata, that caused by original sql set the value as select condition, if value differnt, 
//	just insert a new one. There is problem to work like this. It should take consideration of whether the metadata has multiple value. 
//  Now the logic is: in case metadata has multiple value, if value is not same with one in callback, insert it.
//					  in case of sigle value, if no such metadata, insert new one. If have it, delete original one and insert a new one. 

					// if(isMultipleEntry(i, pMd))   // isMultipleEntry() is not right during callback processing, should be based variable "isMultiple"
					if(1 == isMultiple)
					{
						StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDVALUE FROM ASSETMD WHERE LOCALASSETUID = '%s' AND MDUID = %ld AND MDVALUE = %s"),
							assetLUID, mdUID,_T("N'%s'"));
					
						retCode = PrepareMdSQL(sql, pMd+i, true);
						retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,MDVALUE,sizeof(MDVALUE));	
						if(retCode == SQL_NO_DATA)
						{
							StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO ASSETMD (LOCALASSETUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
								assetLUID, mdUID, _T("N'%s'"));
							retCode = PrepareMdSQL(sql, pMd+i, true);
							retCode = g_localDB.ExecuteSQL(sql);
							
							//	update NS work queue
						//	SaveMsgtoWorkQueue(assetLUID,L"",IDS_ASSET,WORKQUEUE_UPDATE,(pMd+i)->wszMdName, L"");
							saveToWQ = true;
						}
					}
					else	// this metadata has single value
					{
						StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDVALUE FROM ASSETMD WHERE LOCALASSETUID = '%s' AND MDUID = %ld"),
							assetLUID, mdUID);
					
						retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,MDVALUE,sizeof(MDVALUE));	
						if(retCode == SQL_NO_DATA)  // no existing one there, insert new one
						{
							StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO ASSETMD (LOCALASSETUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
								assetLUID, mdUID, _T("N'%s'"));
							retCode = PrepareMdSQL(sql, pMd+i, true);
							retCode = g_localDB.ExecuteSQL(sql);
				
		//	----------- modified by KenQ 2006-11-22 for KDDI requirement: ReferenceCount and RatingControl -------------
		//  here is the case of could new asset was created / or new metadata was added.
		//  To the creating case, following block will execute twice in this function, anyway, forget it.
							if(g_SupportRefcountRatingctrl && (wcscmp((pMd+i)->wszMdName,MDNAME_RATING)==0 || wcscmp((pMd+i)->wszMdName,MDNAME_ASSET_TITLESORTNAME)==0) )
							{
								(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s metadata %s was created, need to recalculate Rating and ReferenceCount"), assetLUID, (pMd+i)->wszMdName);
								AssetRatingRef(assetLUID, m_titleSortNameMDUID, true);
							}
							
							//	update NS work queue
			//				SaveMsgtoWorkQueue(assetLUID,L"",IDS_ASSET,WORKQUEUE_UPDATE,(pMd+i)->wszMdName,L"");
						}
						else						// have an existing one, delete then insert.
						{
							// delete metadata with this name, and insert new one
							StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 

							// ----------- modified by HuangLi 2007-07-02  -----------
							// don't delete metadata with this name, update the metadata
//							_T("DELETE FROM ASSETMD WHERE LOCALASSETUID='%s' AND MDUID=%ld  INSERT INTO ASSETMD (LOCALASSETUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
//								assetLUID, mdUID, assetLUID, mdUID, _T("N'%s'"));
							_T("UPDATE ASSETMD SET MDVALUE = %s WHERE LOCALASSETUID='%s' AND MDUID=%ld"),_T("N'%s'"),assetLUID, mdUID);

							retCode = PrepareMdSQL(sql, pMd+i, true);
							retCode = g_localDB.ExecuteSQL(sql);

							//	update NS work queue
			//				SaveMsgtoWorkQueue(assetLUID,L"",IDS_ASSET,WORKQUEUE_UPDATE,(pMd+i)->wszMdName,L"");
						}
						saveToWQ = true;
					} // else of single value

//	----------- modified by KenQ 2006-11-22 for KDDI requirement: ReferenceCount and RatingControl -------------
//  here is the case of Rating/TitleSortName changing
					if(g_SupportRefcountRatingctrl && wcscmp((pMd+i)->wszMdName,MDNAME_RATING)==0)
					{
						AssetRatingRef(assetLUID, m_titleSortNameMDUID, true);
					}
					if(g_SupportRefcountRatingctrl && wcscmp((pMd+i)->wszMdName,MDNAME_ASSET_TITLESORTNAME)==0)
					{
						SQLTCHAR oldTSN[1024] = _T("");
						bool hasTSN = GetAssetTitleSortName(assetLUID, oldTSN, 1000);

						AssetRefTwoTitles(assetLUID, m_titleSortNameMDUID, hasTSN ? oldTSN : NULL, (pMd+i)->sVal);
					}
				}
			}
			else	//	the metadata replace
			{
				if(1 == isMultiple)
				{
					WCHAR MDVALUE[512];
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDVALUE FROM ASSETMD WHERE LOCALASSETUID = '%s' AND MDUID = %ld AND MDVALUE = '%s'"),
						assetLUID, mdUID,_T("N'%s'"));
					PrepareMdSQL(sql, pMd+i, true);
					retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,MDVALUE,sizeof(MDVALUE));	
					if (retCode == SQL_SUCCESS)
					{
						//	----------- modified by KenQ 2006-11-22 for KDDI requirement: ReferenceCount and RatingControl -------------
						//  here is the case of Rating/TitleSortName changing, 
						//  here VerifyMD() has got known the metadata value is changed, so no more comparing anymore for its value
						if(g_SupportRefcountRatingctrl && wcscmp((pMd+i)->wszMdName,MDNAME_RATING)==0)
						{
							(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s Rating is changed from %s to %d, need to recalculate Rating and ReferenceCount"), assetLUID, MDVALUE, (pMd+i)->iVal);
							AssetRatingRef(assetLUID, m_titleSortNameMDUID, true);
							
						}
						if(g_SupportRefcountRatingctrl && wcscmp((pMd+i)->wszMdName,MDNAME_ASSET_TITLESORTNAME)==0)
						{
							(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s TitleSortName is changed from %s to %s."), assetLUID, MDVALUE, (pMd+i)->sVal);
							AssetRefTwoTitles(assetLUID, m_titleSortNameMDUID, MDVALUE, (pMd+i)->sVal);
						}
					}
					else if(SQL_NO_DATA == retCode)
					{
						StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
							_T("INSERT INTO ASSETMD (LOCALASSETUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
							assetLUID, mdUID, _T("N'%s'"));
						retCode = PrepareMdSQL(sql, pMd+i, true);
						retCode = g_localDB.ExecuteSQL(sql);

						if(g_SupportRefcountRatingctrl && wcscmp((pMd+i)->wszMdName,MDNAME_RATING)==0)
						{
							(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s Rating is %d, need to recalculate Rating and ReferenceCount"), assetLUID, (pMd+i)->iVal);
							AssetRatingRef(assetLUID, m_titleSortNameMDUID, true);
							
						}
						if(g_SupportRefcountRatingctrl && wcscmp((pMd+i)->wszMdName,MDNAME_ASSET_TITLESORTNAME)==0)
						{
							(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s TitleSortName is %s."), assetLUID, (pMd+i)->sVal);
							AssetRefOneTitle(assetLUID, m_titleSortNameMDUID, (pMd+i)->sVal);
						}
						//	update NS work queue
			//			SaveMsgtoWorkQueue(assetLUID,L"",IDS_ASSET,WORKQUEUE_UPDATE,(pMd+i)->wszMdName,L"");
						saveToWQ = true;
					}					
				}
				else
				{
					//	Check if the metadata exists in metadata table
					
					// modified by kaliven lee 041011 because of the modified of the AM DB
					WCHAR OldMDValue[512];
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDVALUE FROM ASSETMD WHERE LOCALASSETUID = '%s' AND MDUID = %ld"),
						assetLUID, mdUID);
					retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, OldMDValue, sizeof(OldMDValue));
					
					if (retCode == SQL_SUCCESS)	//	metadata record exists in metadata table
						//	Update metadata value in metadata table
					{
						if(VerifyMD(pMd+i,OldMDValue) == 1)
						{
							if((pMd+i)->wMdType == IDS_DATETIME)	// if is date time type value, valid time window first
							{
								CString dVal;
								TCHAR NewMDValue[512];
								CTime tVal((pMd+i)->tVal);		//	temp structure in converting DATETIME type
								if(tVal.GetYear() < 1970)
									dVal = _T("1970-01-01 00:00:00");	//	both IDS and LAM use local time, but if convered string is before 1970, 
																		//  the exported itv file from LAM will fail to imported by ITV
								else
									dVal = tVal.Format(_T("%Y-%m-%d %H:%M:%S"));	//	convert to SQL format
								StringCbPrintf(NewMDValue, 512*sizeof(WCHAR), _T("%s"), dVal);
								
								if(validTimeWindow((pMd+i)->wszMdName, NewMDValue, OldMDValue))
								{
					//				SaveMsgtoWorkQueue(assetLUID,L"",IDS_ASSET,WORKQUEUE_UPDATE,(pMd+i)->wszMdName,L"");
									saveToWQ = true;
								}
							}
							else
							{
		//						SaveMsgtoWorkQueue(assetLUID,L"",IDS_ASSET,WORKQUEUE_UPDATE,(pMd+i)->wszMdName,L"");
								saveToWQ = true;
							}
							
							StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ASSETMD SET MDVALUE = %s WHERE LOCALASSETUID = '%s' AND MDUID = %ld"),
								_T("N'%s'"), assetLUID, mdUID);
							retCode = PrepareMdSQL(sql, pMd+i, true);
							retCode = g_localDB.ExecuteSQL(sql);	
						} // end of verify()
						//	----------- modified by KenQ 2006-11-22 for KDDI requirement: ReferenceCount and RatingControl -------------
						//  here is the case of Rating/TitleSortName changing, 
						//  here VerifyMD() has got known the metadata value is changed, so no more comparing anymore for its value
						if(g_SupportRefcountRatingctrl && wcscmp((pMd+i)->wszMdName,MDNAME_RATING)==0)
						{
							(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s Rating is changed from %s to %d, need to recalculate Rating and ReferenceCount"), assetLUID, OldMDValue, (pMd+i)->iVal);
							AssetRatingRef(assetLUID, m_titleSortNameMDUID, true);
							
						}
						if(g_SupportRefcountRatingctrl && wcscmp((pMd+i)->wszMdName,MDNAME_ASSET_TITLESORTNAME)==0)
						{
							(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s TitleSortName is changed from %s to %s."), assetLUID, OldMDValue, (pMd+i)->sVal);
							AssetRefTwoTitles(assetLUID, m_titleSortNameMDUID, OldMDValue, (pMd+i)->sVal);
						}
					} //end of if (retCode == SQL_SUCCESS)
					
					// this case added by Ken Qian at 2007-03-09 for the issue of asset existing, but MD does not. 
					else if(SQL_NO_DATA == retCode)
					{
						StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
							_T("INSERT INTO ASSETMD (LOCALASSETUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
							assetLUID, mdUID, _T("N'%s'"));
						retCode = PrepareMdSQL(sql, pMd+i, true);
						retCode = g_localDB.ExecuteSQL(sql);
						
						if(g_SupportRefcountRatingctrl && wcscmp((pMd+i)->wszMdName,MDNAME_RATING)==0)
						{
							(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s Rating is %d, need to recalculate Rating and ReferenceCount"), assetLUID, (pMd+i)->iVal);
							AssetRatingRef(assetLUID, m_titleSortNameMDUID, true);
							
						}
						if(g_SupportRefcountRatingctrl && wcscmp((pMd+i)->wszMdName,MDNAME_ASSET_TITLESORTNAME)==0)
						{
							(*gpDbSyncLog)(Log::L_INFO, _T("Asset %s TitleSortName is %s."), assetLUID, (pMd+i)->sVal);
							AssetRefOneTitle(assetLUID, m_titleSortNameMDUID, (pMd+i)->sVal);
						}
						//	update NS work queue
		//				SaveMsgtoWorkQueue(assetLUID,L"",IDS_ASSET,WORKQUEUE_UPDATE,(pMd+i)->wszMdName,L"");
						saveToWQ = true;
					}					
				}

			}
		}
		else // the metadata is stored in master table
		{
			//	Update metadata value in master table
//			SaveMsgtoWorkQueue(assetLUID,L"",IDS_ASSET,WORKQUEUE_UPDATE,(pMd+i)->wszMdName,L"");
			saveToWQ = true;
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ASSET SET %s = %s WHERE LOCALASSETUID = '%s'"),
					 fieldName, _T("%s"), assetLUID);
			retCode = PrepareMdSQL(sql, pMd+i, false);
			retCode = g_localDB.ExecuteSQL(sql);	
		}
	}  //end of for

	if(saveToWQ) {
		SaveMsgtoWorkQueue(assetLUID,L"",IDS_ASSET,WORKQUEUE_UPDATE,L"",L"");
	}

	//////////////////////////////////////////////////////////////////////////
	// Added by Bernie, 2006-Feb-5
	// call Add-in
//	if(g_AdiMan.isValid() && gUpdateThreadId != GetCurrentThreadId())
	if(g_AdiMan.isValid() && !m_isFullSyncing)
	{
		g_AdiMan.TrggMd(&entryBlock, dwNum, metaBlockList);

		if(metaBlockList)
			delete[] metaBlockList;
	}
	// end 
	//////////////////////////////////////////////////////////////////////////

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Call SaveAssetMdToDB to Save Metadate of asset %d"),GetCurrentThreadId(),assetUID);	
	return 0;	//	success
}


/*****************************************************************************
Function Name:	SaveElementMdToDB
Arguments:		Element Metadata (METADATA *), Metadata Number (DWORD), Element UID (DWORD)
                newAsset - whether to it is a new asset 
Returns:		0 (DWORD)
Description:	This function goes through all metadata of the element, and for each
				metadata, local table ATOMICELEMENTMD and ENTRYMDD are updated according to:
				1. Look for metadata record per metadata name and type from ENTRYMDD;
				2. If not found, insert a new record into ENTYMDD and ATOMICELEMENTMD using 
				   matadata info.
				3. If found, get the ZQ_IsInMD flag;
				4. If opration is to delete, delete all metadata of the element from
				   ASSETMD;
				5. If ZQ_IsInMD = 1 and is to delete, delete metadata from ATOMICELEMENTMD;
				6. If ZQ_IsInMD = 1 and is not to delete, insert a new record into 
				   ATOMICELEMENTMD using matadata info.
				7. If ZQ_IsInMD = 0 and is to delete, update the corresponding metadata 
				   field in ATOMICELEMENT to "";
				8. If ZQ_IsInMD = 0 and is not to delete, update the corresponding metadata
				   field in ATOMICELEMENT using metadata info.
*****************************************************************************/
DWORD CDSInterface::SaveElementMdToDB(METADATA *pMd, DWORD dwNum, DWORD elementUID, BOOL newAsset)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024] = _T("");	//	string to put SQL statement

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Call SaveElementMdToDB to Save Metadata of element %d"),GetCurrentThreadId(),elementUID);

	bool noLastStamp = false;
	IDSUPDATESTAMP dummy={0,0,0,0,0,0,0,0};
	if(0==memcmp(g_LastUpdateStamp, dummy, sizeof(g_LastUpdateStamp)))
		noLastStamp = true;
	
	if(m_isFullSyncing)
	{
		if(!g_SupportUpdateStamp || noLastStamp)
		{
			// only delete all metadata if no stamp is found
			// otherwise take care of it after checking stamp
		
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("DELETE FROM ATOMICELEMENTMD WHERE LOCALATOMICELEMENTUID IN (SELECT LOCALATOMICELEMENTUID FROM ATOMICELEMENT WHERE ATOMICELEMENTUID = %d )"),
				elementUID);
			g_localDB.ExecuteSQL(sql);
		}
	}

	// KenQ 2007-11-29 the check whether the asset is existing or not
	// coz if a new asset is uploaded, the metadata callback comes before FolderCallback. 
	// remember the asset id, to avoid the query database for continued metadata
	if(!m_isFullSyncing && m_notFoundElementId.find(elementUID) != m_notFoundElementId.end())
	{
		(*gpDbSyncLog)(Log::L_DEBUG,_T("element %ld does not exist, skip it."), elementUID);
		return 0;
	}

	SQLTCHAR elementLUID[21];	//	Local UID of element
	memset(elementLUID,0,sizeof(elementLUID));
	//	Get Local UID of the element
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALATOMICELEMENTUID FROM ATOMICELEMENT WHERE ATOMICELEMENTUID = %ld"),
		elementUID);
	retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, elementLUID, sizeof(elementLUID));
	if (retCode == SQL_NO_DATA)	//	element not found
	{
		// KenQ 2007-11-29 the element does not exist, remember it,
		// coz if a new asset is uploaded, the element metadata callback comes before FolderCallback. 
		// remember the asset id, to avoid the query database for continued metadata
		m_notFoundElementId.insert(IDMAP::value_type(elementUID, elementUID));

		return 0;
	}

	int isObjectLocal = 0;		//	flag indication whether the element is locally managed
	//	Get isLocal flag of the element
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ISLOCAL FROM ATOMICELEMENT WHERE LOCALATOMICELEMENTUID = '%s'"),
		elementLUID);
	retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &isObjectLocal, 0);
	
	
	DA_entryDb	entryBlock;
	DA_metaDb*	metaBlockList = NULL;
	
//	if(g_AdiMan.isValid() && gUpdateThreadId != GetCurrentThreadId())
	if(g_AdiMan.isValid() && !m_isFullSyncing)
	{
		
		// prepare entry block for DBSync Add-in use
		entryBlock._dwEntryType = DBSA_ENTRY_ELEMENT;
		entryBlock._dwEntryUID = elementUID;
		StringCbCopyW(entryBlock._szLocalEntryUID, DBSA_MAXLUID*sizeof(wchar_t), elementLUID);
		
		metaBlockList = new DA_metaDb[dwNum];
		if(metaBlockList)
			memset(metaBlockList, 0, sizeof(DA_metaDb)*dwNum);
		
	}

	for (DWORD i=0; i<dwNum; i++)	//	for each metadata
	{
//		LogMd(pMd+i); //added by W.Z. to trace

		int  isInmd = 1;		//	flag indicating whether the metadata stored in metadata table or master table
		int  isMultiple = 0;	//	flag indicating whether the metadata has multiple value
		int  isLocal = 0;		//	flag indication whether the metadata is locally managed
		TCHAR fieldName[55];	//	string to store ZQ_FIELDNAME
		CString mdName;		//	string to store converted metadata name
		DWORD mdUID = 0;	//	UID of the metadata

		//	Convert metadata name into SQL format
		mdName = (pMd+i)->wszMdName;
		mdName.Replace(_T("'"), _T("''"));

		// Add by KenQ 2008-7-29, only handle the specified element metadata
		// check whether the element metadata should be ignored
		if(m_activeElementMD.size() != 0)
		{
			if(m_activeElementMD.find(mdName) == m_activeElementMD.end())
			{
				continue;
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// ----------------------------------------+------------------------------
		// Extra update logic when DBSync starts.  |     By Bernie.Z, 2006-Feb-20
		//                                         | getting a cold, sleepy... T_T
		// ----------------------------------------+------------------------------
		// When DBSync starts, it will perform a full synchronization which would
		// consume hours of time depending on the asset and element number.
		// The process itself is slow because it re-fetch all the metadata values
		// of all the assets/elements.  Each of the fetch would take around 
		// 10-20ms, thus making the entire process time a linear sum of metadata
		// number multiple this 10-20ms.
		//
		// The following logic ignores those metadata updates when its time stamp
		// is older than the one DBSync recorded in the registry during the last
		// run.  These metadatas has not been changed since the last run so we can
		// simply skip them.
		//
		// the stamp of DBSync recorded would be updated into the registry upon 
		// every metadata change callback.
		// 
		//
		// --------------------------------- begin -------------------------------
		
		if((pMd+i)==NULL)	// memory invalid
			continue;
		
		IDSUPDATESTAMP	tmpStamp = {0,0,0,0,0,0,0,0};
		if((pMd+i)->MdStamp)
			memcpy(tmpStamp, (pMd+i)->MdStamp, sizeof(tmpStamp));
		
		if(m_isFullSyncing && g_SupportUpdateStamp && !newAsset)	// only skip metadata update when start, not on callbacks
		{
			if(g_server.CompareStamp(tmpStamp, g_CurrUpdateStamp)>0)
			{
				// save the greatest stamp into temp stamp, this temp stamp will be
				// copied into LastUpdateStamp after finishing the starting
				memcpy(g_CurrUpdateStamp, tmpStamp, sizeof(g_CurrUpdateStamp));
			}
			
			int comRet = g_server.CompareStamp(tmpStamp, g_LastUpdateStamp);
			if(comRet <= 0)
			{
				// this metadata is updated earlier than last stamp
				(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:metadata \"%s\" update is skipped during starting, stamp=%02X%02X%02X%02X%02X%02X%02X%02X"),
					GetCurrentThreadId(), (pMd+i)->wszMdName, tmpStamp[0], tmpStamp[1], tmpStamp[2], tmpStamp[3], tmpStamp[4], tmpStamp[5], tmpStamp[6], tmpStamp[7]);
				continue;
			}
		}
		else if(!m_isFullSyncing && g_SupportUpdateStamp && g_SyncAllAtBegin != 0)	// if callback comes, update the last stamp
			    /*
					This condition is added on 2006-12-26, the timestamp ONLY be able to update 
					in case of have full syncing at the beginning.
				*/
		{
			memcpy(g_LastUpdateStamp, tmpStamp, sizeof(g_LastUpdateStamp));
			g_server.SaveLastStamp();
		}
		
		// ---------------------------------- end --------------------------------
		//////////////////////////////////////////////////////////////////////////
		
		//optimize code 
		DBRECORD SelectRecord;
		DBRESULTSETS resultsets;
		DBFIELD field1,field2, field3,field4,field5;
		field1.lType = SQL_C_ULONG;
		field2.lType = SQL_C_ULONG;
		field3.lType = SQL_C_ULONG;
		field4.lType = SQL_C_TCHAR;
		field5.lType = SQL_C_ULONG;
		field4.lSize = 64;
		SelectRecord.push_back(field1);
		SelectRecord.push_back(field2);
		SelectRecord.push_back(field3);
		SelectRecord.push_back(field4);
		SelectRecord.push_back(field5);

		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID,ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME,ISMULTIPLE FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu"),
			mdName,	(pMd+i)->wMdType, IDS_ATOMIC_ELEMENT);
		retCode =g_localDB.SelectAllSQL(sql,SelectRecord,&resultsets);

		if (retCode == SQL_NO_DATA)	//	not found in table ENTRYMDD
		{
			SaveEntryMDDToDBEx(IDS_ATOMIC_ELEMENT, (LPCTSTR)mdName);

			//	re-select the MDUID
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID,ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME,ISMULTIPLE FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu"),
				mdName,	(pMd+i)->wMdType, IDS_ATOMIC_ELEMENT);
			retCode =g_localDB.SelectAllSQL(sql,SelectRecord,&resultsets);
		}	

		if(retCode == SQL_NO_DATA)
		{
			(*gpDbSyncLog)(Log::L_ERROR,_T("%d:AtomicElement MDD %s does not exist in database"),GetCurrentThreadId(), mdName);
			continue;
		}
		else	//	found in table ENTRYMDD
		{
			//	Get the isLocal flag
			DBRECORD record = resultsets.at(0);
			memcpy(&mdUID,record.at(0).pValue,4);
			memcpy(&isLocal,record.at(1).pValue,4);
			if(isLocal)
				continue;
			memcpy(&isInmd,record.at(2).pValue,4);			

			if(!isInmd)
				StringCbCopyW(fieldName, 55*sizeof(WCHAR), (wchar_t*)record.at(3).pValue);
			memcpy(&isMultiple,record.at(4).pValue,4);
			g_localDB.FreeResultSets(&resultsets);
		}			

//		if(g_AdiMan.isValid() && gUpdateThreadId != GetCurrentThreadId() && metaBlockList!=NULL)
		if(g_AdiMan.isValid() && !m_isFullSyncing && metaBlockList!=NULL)
		{
			
			// prepare metadata block for DBSync Add-in use
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("%s"),_T("%s"));
			PrepareMdSQL(sql, pMd+i, true);
			
			StringCbCopyW(metaBlockList[i]._szMdName, DBSA_MAXMDNAME*sizeof(wchar_t), (pMd+i)->wszMdName);
			StringCbCopyNW(metaBlockList[i]._szMdValue, DBSA_MAXMDVALUE*sizeof(wchar_t), sql, (DBSA_MAXMDVALUE-1)*sizeof(wchar_t));
			metaBlockList[i]._dwMdUID = mdUID;
			metaBlockList[i]._dwAppUID = (pMd+i)->dwMdAppUid;
			metaBlockList[i]._dwOp = (pMd+i)->wMdOp;
			
		}
		
		if ((pMd+i)->wMdOp == IDS_DELETE)  // operation is to delete, only happens when callback
		{
			if (isInmd) // the metadata is stored in matadata table
			{
				//	Delete corresponding metadata value record from metadata table
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM ATOMICELEMENTMD WHERE LOCALATOMICELEMENTUID = '%s' AND MDUID = %ld"),
						 elementLUID, mdUID);
				retCode = g_localDB.ExecuteSQL(sql);

				//	check how many element md in database
				DWORD mdCount = 0;
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT COUNT(*) FROM ATOMICELEMENTMD WHERE LOCALATOMICELEMENTUID = '%s' "),
						 elementLUID, mdUID);
				g_localDB.SelectSQL(sql,SQL_C_ULONG,&mdCount,0);

				if(0 == mdCount)
				{
					(*gpDbSyncLog)(Log::L_INFO,_T("Element %s all metadata were deleted, delete it from AtomicElement too"), elementLUID);
					
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM ATOMICELEMENT WHERE LOCALATOMICELEMENTUID = '%s'"),
							 elementLUID);
					g_localDB.ExecuteSQL(sql);
				}
			}
			else // the metadata is stored in master table
			{
				//	Set corresponding metadata field in master table to NULL
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ATOMICELEMENT SET %s = NULL WHERE LOCALATOMICELEMENTUID = '%s'"),
						 fieldName, elementLUID);
				retCode = g_localDB.ExecuteSQL(sql);
			}
			continue;	//	end processing for current metadata
		}

		if (isObjectLocal && isLocal)	//	the metadata and the element are both locally managed
			continue;	//	skip to process the next metadata

		if (isInmd) // the metadata is stored in matadata table
		{
			DWORD elementID = 0;	//	UID of element
			if (((pMd+i)->wMdOp == IDS_ADD)||((pMd+i)->wMdOp == 0))	//	metadata add and put
			{
				SQLTCHAR MDVALUE[501] = _T("");
				if(m_isFullSyncing)
				{
					// changed by KenQ 2007-4-12, even though these md has been deleted incase of noLastStamp, 
					// but if LAM insert md, that would cause single value type md have multiple values,
					// but this changing will reduce performance, coz to single type, must delete MD again, 
					// actually they all have been deleted. 
					// if(noLastStamp || isMultipleEntry(i, pMd) )
					if(1 == isMultiple)
					{
						// case 1: If no last update stamp found, meaning all md of this element have been deleted
						// case 2: this md is the same as previous one, so it is multi-value, and all md of this name have been deleted
						// result: do NOT delete metadatas with this name here, JUST insert
						StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO ATOMICELEMENTMD (LOCALATOMICELEMENTUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
							elementLUID, mdUID, _T("N'%s'"));
					}
					else
					{
						// delete metadatas with this name, and insert new one
					/*	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
						_T("DELETE FROM ATOMICELEMENTMD WHERE LOCALATOMICELEMENTUID='%s' AND MDUID=%ld  INSERT INTO ATOMICELEMENTMD (LOCALATOMICELEMENTUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
						elementLUID, mdUID, elementLUID, mdUID, _T("N'%s'"));
                    */

						StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
							_T("DELETE FROM ATOMICELEMENTMD WHERE LOCALATOMICELEMENTUID='%s' AND MDUID=%ld "),
							elementLUID, mdUID);
						
						retCode = g_localDB.ExecuteSQL(sql);

						StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
							_T("INSERT INTO ATOMICELEMENTMD (LOCALATOMICELEMENTUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
							elementLUID, mdUID, _T("N'%s'"));
					}
				}
				else
				{
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDVALUE FROM ATOMICELEMENTMD WHERE LOCALATOMICELEMENTUID = '%s' AND MDUID = %ld AND MDVALUE = %s"),
						 elementLUID,  mdUID, _T("N'%s'"));
					retCode = PrepareMdSQL(sql, pMd+i, true);
					retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,MDVALUE,sizeof(MDVALUE));	
					if(retCode == SQL_NO_DATA)
						StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO ATOMICELEMENTMD (LOCALATOMICELEMENTUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
								 elementLUID,  mdUID, _T("N'%s'"));
					else 
						continue;
				}
			}
			else	//	metadata replace
			{
				
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ATOMICELEMENTMD SET MDVALUE = %s WHERE LOCALATOMICELEMENTUID = '%s' AND MDUID = %ld"),
						 _T("N'%s'"), elementLUID, mdUID);

			}
			retCode = PrepareMdSQL(sql, pMd+i, true);
			if (retCode == 0) {

				retCode = g_localDB.ExecuteSQL(sql);	
			}
		}
		else // the metadata is stored in master table
		{
			//	Update metadata value in master table
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ATOMICELEMENT SET %s = %s WHERE LOCALATOMICELEMENTUID = '%s'"),
					 fieldName, _T("%s"), elementLUID);
			retCode = PrepareMdSQL(sql, pMd+i, false);
			retCode = g_localDB.ExecuteSQL(sql);	
		}


		if ((mdUID == MDUID_PLAYTIME) || (mdUID == MDUID_PLAYFRACTION))	//	the metadata is about to change actual_playtime
		{
			DWORD playTime = 0;		//	playtime value of the element
			DWORD playFraction = 0;		//	playfraction value of the element
			float actualDuration = 0.0f;		//	actualDuration value of the element
			if (mdUID == MDUID_PLAYTIME)	//	the metadata is Playtime
			{
				//	Get Playtime of the element
				playTime = (pMd+i)->iVal;

				//	Get Playfraction of the element
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDVALUE FROM ATOMICELEMENTMD WHERE LOCALATOMICELEMENTUID = '%s' AND MDUID = %ld"),
						 elementLUID, MDUID_PLAYFRACTION);
				retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &playFraction, 0);
			}
			else	//	the metadata is Playfraction
			{
				//	Get Playtime of the element
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDVALUE FROM ATOMICELEMENTMD WHERE LOCALATOMICELEMENTUID = '%s' AND MDUID = %ld"),
						 elementLUID, MDUID_PLAYTIME);
				retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &playTime, 0);

				//	Get Playfraction of the element
				playFraction = (pMd+i)->iVal;
			}

			//	Calculate new actualduration for the element
			actualDuration = (float)playTime + (float)((float)playFraction/(float)1000000) + (float)((float)g_playInterval/(float)1000000);
			//	Update actualduration value in element table

			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE ATOMICELEMENT SET ACTUALDURATION = %.6f WHERE LOCALATOMICELEMENTUID = '%s'"),
					 actualDuration, elementLUID);
			retCode = g_localDB.ExecuteSQL(sql);	

			SQLINTEGER inIndicate = SQL_NTS, outIndicate = 0;	//	indication for the length of parameter buffer
			SQLSMALLINT procRet = 0;	//	return code of the stored procudure
			//	Call an stored procudure to update actual_playtime of the complex asset containing this element
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("{CALL SET_ACTUALDURATION_WITH_ELE(?,?)}"));
			retCode = g_localDB.CallStoreProcedure(sql, 
				                                   LocalDB::INPUT, SQL_C_TCHAR, SQL_VARCHAR, sizeof(elementLUID), 0, elementLUID, sizeof(elementLUID), &inIndicate,
												   LocalDB::OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &procRet, 0, &outIndicate,
												   LocalDB::INOUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, NULL, 0, NULL);
			(*gpDbSyncLog)(Log::L_INFO, _T("%d:Result of SET_ACTUALDURATION_WITH_ELE for element %s: %hu."),GetCurrentThreadId(), elementLUID, procRet);
			
		}

	}  //end of for

	//////////////////////////////////////////////////////////////////////////
	// Added by Bernie, 2006-Feb-5
	// call Add-in
//	if(g_AdiMan.isValid() && gUpdateThreadId != GetCurrentThreadId())
	if(g_AdiMan.isValid() && !m_isFullSyncing)
	{
		g_AdiMan.TrggMd(&entryBlock, dwNum, metaBlockList);
		
		if(metaBlockList)
			delete[] metaBlockList;
	}
	// end 
	//////////////////////////////////////////////////////////////////////////

	
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Call SaveElementMdToDB to Save Metadata of element %d"),GetCurrentThreadId(),elementUID);
	return 0;	//	success
}
/*****************************************************************************
Function Name:	SaveClipMdToDB
Arguments:		Clip Metadata (METADATA *), Metadata Number (DWORD), Clip UID (DWORD)
Returns:		0 (DWORD)
Description:	This function goes through all metadata of the clip, and for each
				metadata, local table CLIPMD and ENTRYMDD are updated according to:
				1. Look for metadata record per metadata name and type from ENTRYMDD;
				2. If not found, insert a new record into ENTYMDD and ATOMICELEMENTMD using 
				   metadata info.
				3. If found, get the ZQ_IsInMD flag;
				4. If operation is to delete, delete all metadata of the element from
				   ASSETMD;
				5. If ZQ_IsInMD = 1 and is to delete, delete metadata from ATOMICELEMENTMD;
				6. If ZQ_IsInMD = 1 and is not to delete, insert a new record into 
				   ATOMICELEMENTMD using metadata info.
				7. If ZQ_IsInMD = 0 and is to delete, update the corresponding metadata 
				   field in ATOMICELEMENT to "";
				8. If ZQ_IsInMD = 0 and is not to delete, update the corresponding metadata
				   field in ATOMICELEMENT using metadata info.
*****************************************************************************/
DWORD CDSInterface::SaveClipMdToDB(METADATA*pMd,DWORD dwNum,DWORD clipUID)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Call SaveClipMdToDB to Save Metadata of clip %d"),GetCurrentThreadId(),clipUID);
//	if(gUpdateThreadId == GetCurrentThreadId())
	if(m_isFullSyncing)
	{
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("DELETE FROM CLIP WHERE LOCALCLIPUID IN (SELECT LOCALCLIPUID FROM CLIP WHERE CLIPUID = %d )"),
			clipUID);
		g_localDB.ExecuteSQL(sql);
	}
	for (DWORD i=0; i<dwNum; i++)	//	for each metadata
	{
		int  isInmd;		//	flag indicating whether the metadata stored in metadata table or master table
		int  isMultiple;	//	flag indicating whether the metadata has multiple value
		int  isLocal;		//	flag indication whether the metadata is locally managed
		TCHAR fieldName[55];	//	string to store ZQ_FIELDNAME
		CString mdName;		//	string to store converted metadata name
		DWORD mdUID = 0;	//	UID of the metadata

		
		mdName = (pMd+i)->wszMdName;
		mdName.Replace(_T("'"), _T("''"));
		//optimize code 
		DBRECORD SelectRecord;
		DBRESULTSETS resultsets;
		DBFIELD field1,field2, field3,field4,field5;
		field1.lType = SQL_C_ULONG;
		field2.lType = SQL_C_ULONG;
		field3.lType = SQL_C_ULONG;
		field4.lType = SQL_C_TCHAR;
		field5.lType = SQL_C_ULONG;
		field4.lSize = 64;
		SelectRecord.push_back(field1);
		SelectRecord.push_back(field2);
		SelectRecord.push_back(field3);
		SelectRecord.push_back(field4);
		SelectRecord.push_back(field5);

		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID,ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME,ISMULTIPLE FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu"),
			mdName,	(pMd+i)->wMdType, IDS_CLIP);
		retCode =g_localDB.SelectAllSQL(sql,SelectRecord,&resultsets);

		if (retCode == SQL_NO_DATA)	//	not found in table ENTRYMDD
		{
			SaveEntryMDDToDBEx(IDS_CLIP, (LPCTSTR)mdName);

			//	re-select the MDUID
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID,ISLOCAL,ZQ_ISINMD,ZQ_FIELDNAME,ISMULTIPLE FROM ENTRYMDD WHERE MDNAME = N'%s' AND MDTYPE = %hu AND ENTRYTYPE = %hu"),
				mdName,	(pMd+i)->wMdType, IDS_CLIP);
			retCode =g_localDB.SelectAllSQL(sql,SelectRecord,&resultsets);
		}

		if(retCode == SQL_NO_DATA)
		{
			(*gpDbSyncLog)(Log::L_ERROR,_T("%d:Site MDD %s does not exist in database"),GetCurrentThreadId(), mdName);
			continue;
		}
		else	//	found in table ENTRYMDD
		{
			DBRECORD record = resultsets.at(0);
			memcpy(&mdUID,record.at(0).pValue,4);
			memcpy(&isLocal,record.at(1).pValue,4);
			if(isLocal)
				continue;
			memcpy(&isInmd,record.at(2).pValue,4);			

			if(!isInmd)
				StringCbCopyW(fieldName, 55*sizeof(WCHAR), (wchar_t*)record.at(3).pValue);
			memcpy(&isMultiple,record.at(4).pValue,4);
			g_localDB.FreeResultSets(&resultsets);
		}
		
		SQLTCHAR clipLUID[21] =_T("");	//	Local UID of clip
		//	Get Local UID of the element
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALCLIPUID FROM CLIP WHERE CLIPUID = %ld"),
				 clipUID);
		retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, clipLUID, sizeof(clipLUID));
		if (retCode == SQL_NO_DATA)	//	element not found
			continue;		//	end processing for current metadata

		if ((pMd+i)->wMdOp == IDS_DELETE)  // operation is to delete, only happens when callback
		{
			if (isInmd) // the metadata is stored in metadata table
			{
//				if (isMultiple)	//	the metadata has multiple value
//				{
//					//	Delete corresponding metadata value record from metadata table
//					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM CLIPMD WHERE LOCALCLIPUID = '%s' AND MDUID = %ld AND MDVALUE = %s"),
//							 clipLUID, mdUID, _T("'%s'"));
//					retCode = PrepareMdSQL(sql, pMd+i, true);
//					retCode = g_localDB.ExecuteSQL(sql);
//				}
//				else	//	the metadata has only one value
//				{
					//	Delete corresponding metadata value record from metadata table
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM CLIPMD WHERE LOCALCLIPUID = '%s' AND MDUID = %ld"),
							 clipLUID, mdUID);
					retCode = g_localDB.ExecuteSQL(sql);
// 				}
			}
			else // the metadata is stored in master table
			{
				//	Set corresponding metadata field in master table to NULL
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE CLIP SET %s = NULL WHERE LOCALCLIPUID = '%s'"),
						 fieldName, clipLUID);
				retCode = g_localDB.ExecuteSQL(sql);
			}
			continue;	//	end processing for current metadata
		}

		int isObjectLocal = 0;		//	flag indication whether the element is locally managed
		//	Get isLocal flag of the element
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT ISLOCAL FROM CLIP WHERE LOCALCLIPUID = '%s'"),
				 clipLUID);
		retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &isObjectLocal, 0);
		if (isObjectLocal && isLocal)	//	the metadata and the element are both locally managed
			continue;	//	skip to process the next metadata

		if (isInmd) // the metadata is stored in metadata table
		{
			DWORD clipID = 0;	//	UID of element
			if (((pMd+i)->wMdOp == IDS_ADD)&&((pMd+i)->wMdOp == 0))	//	the metadata has multiple value
			{
				SQLTCHAR MDVALUE[501] = _T("");
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDVALUE FROM CLIPMD WHERE LOCALCLIPUID = '%s',MDUID = %ld,MDVALUE = %s"),
						 clipLUID, mdUID, _T("N'%s'"));
				retCode = PrepareMdSQL(sql,pMd+i, true);
				retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,MDVALUE,sizeof(MDVALUE));
				if(retCode == SQL_NO_DATA)
				{
					StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("INSERT INTO CLIPMD (LOCALCLIPUID,MDUID,MDVALUE) VALUES ('%s',%ld,%s)"),
						 clipLUID, mdUID, _T("N'%s'"));
					retCode = PrepareMdSQL(sql, pMd+i, true);
					if (retCode == 0) {
						retCode = g_localDB.ExecuteSQL(sql);	
					}
				}
			}
			else	//	the metadata has only one value
			{
				//	Check if the metadata exists in metadata table
				StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE CLIPMD SET MDVALUE = %s WHERE LOCALCLIPUID = '%s' AND MDUID = %ld"),
						 _T("N'%s'"), clipLUID, mdUID);
				retCode = PrepareMdSQL(sql, pMd+i, true);
				if (retCode == 0) {
					retCode = g_localDB.ExecuteSQL(sql);	
				}
			}

		}
		else // the metadata is stored in master table
		{
			//	Update metadata value in master table
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE CLIP SET %s = %s WHERE LOCALCLIPUID = '%s'"),
					 fieldName, _T("%s"), clipLUID);
			retCode = PrepareMdSQL(sql, pMd+i, false);
			retCode = g_localDB.ExecuteSQL(sql);	
		}
		
	}  //end of for
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Call SaveClipMdToDB to Save Metadata of clip %d"),GetCurrentThreadId(),clipUID);
	return 0;	//	success
}
/*****************************************************************************
Function Name:	GetNewLocalID
Arguments:		UID type (int), Buffer of UID value (DWORD *)
Returns:		0 (DWORD)
Description:	This function gets a new unique ID for designated object type, 
				according to different type of local database (Oracle or SQL 
				Server).
*****************************************************************************/
DWORD CDSInterface::GetNewLocalUID(int typeUID, DWORD *bufferUID)
{
	RETCODE		retCode;	//	return code
	SQLTCHAR		sql[1024];	//	string to put SQL statement
	CString		seqName;	//	Name of the sequence to create

	//	Get name of the sequence to create
	switch (typeUID)
	{
	case IDS_METADATA:
		seqName = "SEQ_MDUID";
		break;
	case IDS_HIERARCHY:
		seqName = "SEQ_HIERARCHYUID";
		break;
	case IDS_ASSET:
		seqName = "SEQ_ASSETUID";
		break;
	case IDS_ATOMIC_ELEMENT:
		seqName = "SEQ_ATOMICELEMENTUID";
		break;
	case IDS_CLIP:
		seqName = "SEQ_CLIPUID";
		break;
	case IDS_WORKQUEUE:
		seqName = "Seq_WQ_Queue_UID";
		break;
	}

	if (wcsicmp(g_IZQDBType, _T("SQL Server"))==0)	// it's a SQL Server database
	{
		//	Begin a transaction
		
//	---------- modified by KenQ 2007-01-10-------------
//  Move transactionDB to class member variable, coz it is time consumed operation. 
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE GETSEQUENCE SET CURVALUE = CURVALUE + STEP WHERE SEQNAME = '%s'"),
				 seqName.GetBuffer(MAXNAME));
		retCode = m_transactionDB.ExecuteSQL(sql);
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("UPDATE GETSEQUENCE SET CURVALUE = MINVALUE WHERE SEQNAME = '%s' AND CURVALUE > MAXVALUE"),
				 seqName.GetBuffer(MAXNAME));
		retCode = m_transactionDB.ExecuteSQL(sql);
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT CURVALUE FROM GETSEQUENCE WHERE SEQNAME = '%s'"),
				 seqName.GetBuffer(MAXNAME));
		retCode = m_transactionDB.SelectSQL(sql, SQL_C_ULONG, bufferUID, 0);


		//	Commit the transaction
		retCode = m_transactionDB.EndTransaction(SQL_COMMIT);
	}
	else	// It's not a SQL Server database
	{
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT %s.NEXTVAL FROM DUAL"),
				 seqName.GetBuffer(MAXNAME));
		retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, bufferUID, 0);
	}

	return 0;	//	success
}


/*****************************************************************************
Function Name:	LogMd
Arguments:		Metadata Info (METADATA *)
Returns:		None
Description:	This function logs the infomation inside a METADATA structure, 
				according to different data	types of the metadata.
*****************************************************************************/
void CDSInterface::LogMd(METADATA *pMd)
{

	(*gpDbSyncLog)(Log::L_INFO, _T("Md name: %s, type: %d, op: %d,"), pMd->wszMdName, pMd->wMdType, pMd->wMdOp);
	
	//	Process according to the data type of the metadata value
	switch (pMd->wMdType)
	{
	case IDS_INTEGER:
		(*gpDbSyncLog)(Log::L_INFO, _T("IDS_INTEGER: %ld"), pMd->iVal);
		
		break;
	case IDS_REAL:
		(*gpDbSyncLog)(Log::L_INFO, _T("IDS_REAL: %f"), pMd->rVal);
		
		break;
	case IDS_FLOAT:
		(*gpDbSyncLog)(Log::L_INFO, _T("IDS_FLOAT: %f"), pMd->fVal);
		
		break;
	case IDS_STRING:
		{
			CString sVal(pMd->sVal);
			(*gpDbSyncLog)(Log::L_INFO, _T("IDS_STRING: '%s'"), sVal);
			
		}
		break;
	case IDS_DATETIME:
		{
			CTime tVal(pMd->tVal);
			CString dVal = tVal.Format(_T("%Y-%m-%d %H:%M:%S"));
			(*gpDbSyncLog)(Log::L_INFO, _T("IDS_DATETIME: '%s'"), dVal);
			
		}
		break;
	case IDS_BOOLEAN:
		(*gpDbSyncLog)(Log::L_INFO, _T("IDS_BOOLEAN: %ld"), pMd->bitVal);
		
		break;
	default:
		(*gpDbSyncLog)(Log::L_INFO, _T("IDS_OTHERS: "));
		
		break;
	}	//	end switch
  
  return;
}


/*****************************************************************************
Function Name:	PrepareMdSQL
Arguments:		SQL String (LPTSTR), Metadata Info (METADATA *), Tag for quotation (BOOL)
Returns:		retCode (DWORD) (0:success 1:fail)
Description:	This function prepares the SQL string which relates to metadata
				and will be used to operate	local DB, according to different data
				types of the metadata.
*****************************************************************************/
DWORD CDSInterface::PrepareMdSQL(TCHAR* sqlString, METADATA *pMd, BOOL quotationTag)
{
	DWORD retCode = 0;		//	return code
	TCHAR formatString[1024];	//	temp string for format converting process

//	Clog(0, "Prepare before: %s", sqlString);
//	(*gpDbSyncLog)(Log::L_DEBUG,_T("Prepare before: %s"), sqlString);
	//	Prepare SQL statement according to the data type of the metadata
	switch (pMd->wMdType)	//	chose the data type of the metadata
	{
	case IDS_INTEGER:
		StringCbPrintf(formatString, 1024*sizeof(TCHAR), sqlString, _T("%ld"));
		StringCbPrintf(sqlString, 1024*sizeof(TCHAR), formatString, pMd->iVal);
		break;
	case IDS_REAL:
		StringCbPrintf(formatString, 1024*sizeof(TCHAR), sqlString, _T("%f"));
		StringCbPrintf(sqlString, 1024*sizeof(TCHAR), formatString, pMd->rVal);
		break;
	case IDS_FLOAT:
		StringCbPrintf(formatString, 1024*sizeof(TCHAR), sqlString, _T("%f"));
		StringCbPrintf(sqlString, 1024*sizeof(TCHAR), formatString, pMd->fVal);
		break;
	case IDS_STRING:
		{
			CString sVal(pMd->sVal);	//	temp string in converting STRING type
			sVal.Replace(_T("'"), _T("''"));	//	convert to SQL format
			if (quotationTag)	//	it's a value in quotation
				StringCbPrintf(formatString, 1024*sizeof(TCHAR), sqlString, _T("%s"));
			else	//	it's a value out of quotation
				StringCbPrintf(formatString, 1024*sizeof(TCHAR), sqlString, _T("N'%s'"));

			StringCbPrintf(sqlString, 1024*sizeof(TCHAR), formatString, sVal);
		}
		break;
	case IDS_DATETIME:
		{
	/*		if (pMd->tVal == 0) {
				retCode = 1;	//Data field value is empty, ignore
				StringCbCopyW(formatString, sqlString,_T(""));
				StringCbCopyW(sqlString, formatString);
				break;
			}*/
			//pMd->tVal =0;
	//		(*gpDbSyncLog)(Log::L_DEBUG,_T("Prepare Time Value: %ld"), pMd->tVal);
			CString dVal;
			CTime tVal(pMd->tVal);		//	temp structure in converting DATETIME type
			if(tVal.GetYear() < 1970)
				dVal = _T("1970-01-01 00:00:00");	//	both IDS and LAM use local time, but if convered string is before 1970, 
			                                        //  the exported itv file from LAM will fail to imported by ITV
			else
				dVal = tVal.Format(_T("%Y-%m-%d %H:%M:%S"));	//	convert to SQL format
			
			if (quotationTag)	//	it's a value in quotation
				StringCbPrintf(formatString, 1024*sizeof(TCHAR), sqlString, _T("%s"));
			else	//	it's a value out of quotation
				StringCbPrintf(formatString, 1024*sizeof(TCHAR), sqlString, _T("'%s'"));

			StringCbPrintf(sqlString, 1024*sizeof(TCHAR), formatString, dVal);
		}
		break;
	case IDS_BOOLEAN:
		StringCbPrintf(formatString, 1024*sizeof(TCHAR), sqlString, _T("%ld"));
		StringCbPrintf(sqlString, 1024*sizeof(TCHAR), formatString, pMd->bitVal);
		break;
	case IDS_BINARY:
		StringCbPrintf(formatString, 1024*sizeof(TCHAR), sqlString, _T("Binary"));
		StringCbPrintf(sqlString, 1024*sizeof(TCHAR), formatString);
	default:
		retCode = 1;	//	no data type is matching, something wrong
		break;
	}	//	end switch
//	(*gpDbSyncLog)(Log::L_DEBUG,_T("Prepare after: %s"), sqlString); 
//	Clog(0, "Prepare after: %s", sqlString);
	return retCode;
}


/*****************************************************************************
Function Name:	PrepareSelectionSQL
Arguments:		SQL String (LPTSTR), Select Entry Info (SELECTENTRY *)
Returns:		retCode (DWORD) (0:success 1:fail)
Description:	This function prepares the SQL string which relates to selection
				and will be used to operate	local DB, according to different data
				types of the selection.
*****************************************************************************/
DWORD CDSInterface::PrepareSelectionSQL(TCHAR* sqlString, WORD mdType, SELECTENTRY *pSelection)
{
	DWORD retCode = 0;		//	return code
	TCHAR formatString[1024];	//	temp string for format converting

//	Clog(0, "Selection before: %s", sqlString);

	//	Prepare statement according to the data type of the selection entry
	switch (mdType)	//	chose the data type of the selection entry
	{
	case IDS_INTEGER:
		StringCbPrintf(formatString, 1024*sizeof(WCHAR), sqlString, _T("%ld"));
		StringCbPrintf(sqlString, 1024*sizeof(WCHAR), formatString, pSelection->iVal);
		break;
	case IDS_REAL:
		StringCbPrintf(formatString, 1024*sizeof(WCHAR), sqlString, _T("%f"));
		StringCbPrintf(sqlString, 1024*sizeof(WCHAR), formatString, pSelection->rVal);
		break;
	case IDS_FLOAT:
		StringCbPrintf(formatString, 1024*sizeof(WCHAR), sqlString, _T("%f"));
		StringCbPrintf(sqlString, 1024*sizeof(WCHAR), formatString, pSelection->fVal);
		break;
	case IDS_BOOLEAN:
		StringCbPrintf(formatString, 1024*sizeof(WCHAR), sqlString, _T("%ld"));
		StringCbPrintf(sqlString, 1024*sizeof(WCHAR), formatString, pSelection->bitVal);
		break;
	default:
		retCode = 1;	//	no data type is matching, something wrong
		break;
	}	//	end switch
//	Clog(0, "Selection after: %s", sqlString);
	return retCode;
}
/*****************************************************************************
Function Name:	VerifyParent
Arguments:		EntryUid (DWORD),EntryType(DWORD), ParentUid (LPTSTR),ParentUid(DWORD)
Returns:		bool
Description:	this function check if the entryuid is the child of parentuid
*****************************************************************************/
bool CDSInterface::GetEntryParentType(DWORD folderUID,DWORD parentUID,DWORD* parentType)
{
	SQLTCHAR sql[1024];
	SQLTCHAR LocalHierarchyUID [32];
	DWORD hierarchyUID = 0;
	ENTRY * pEntry = NULL;
	DWORD dwFound = 0;
	bool bFoundFlag = false;
	memset(LocalHierarchyUID,0,sizeof(LocalHierarchyUID));
	// search parent 
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT HIERARCHYUID FROM HIERARCHY WHERE HIERARCHYUID = %ld AND ENTRYTYPE = %ld "), 
			 parentUID , IDS_FOLDER );
	DWORD retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &hierarchyUID, sizeof(hierarchyUID));
	if(retCode == SQL_NO_DATA)
	{	
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT HIERARCHYUID FROM HIERARCHY WHERE HIERARCHYUID = %ld AND ENTRYTYPE = %ld "),
			parentUID,IDS_APPLICATION);
		retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &hierarchyUID, sizeof(hierarchyUID));
		if(retCode == SQL_NO_DATA)
		{
			return false;
		}
		else if(retCode == SQL_SUCCESS)
		{
			*parentType = IDS_APPLICATION;
			return true;
		}
	}
	else
	{
		DWORD tmpParentHUID = 0;
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT HIERARCHYUID FROM HIERARCHY WHERE HIERARCHYUID = %ld AND ENTRYTYPE = %ld "),
			parentUID,IDS_APPLICATION);
		retCode = g_localDB.SelectSQL(sql, SQL_C_ULONG, &tmpParentHUID, sizeof(tmpParentHUID));
		if(retCode == SQL_NO_DATA)
		{
			*parentType = IDS_FOLDER;
			return true;
		}
		else if(retCode == SQL_SUCCESS)
		{
			RTNDEF res = IdsListFolderEntriesByUid(&m_idssess,hierarchyUID,&pEntry,&dwFound,&m_itvsta,NULL);
			if (res != ITV_SUCCESS)	//	error occurs
			{
				//	Log error message
				(*gpDbSyncLog)(Log::L_INFO, _T("IdsListFolderEntriesByUid ERROR when found conflict parentuid's type!"));			
				return false;
			}
			for(DWORD i = 0; i < dwFound; i ++)
			{
				if(pEntry[i].dwHierarchyUid == folderUID)
				{
					bFoundFlag = true;
					break;
				}
			}
			IdsFree(pEntry);
			if(bFoundFlag)
			{
				*parentType = IDS_FOLDER;
			}
			else
				*parentType = IDS_APPLICATION;
			return true;
		}
	}
	return false;
}

/*****************************************************************************
Function Name:	WriteXMLFile
Arguments:		Element Id (SQLTCHAR *), Element Status (LPTSTR)
Returns:		(DWORD) (0:success 1:fail)
Description:	This function writes the current status of an element to xml
				file, based on xml schema "NoI_Sync_to_CMS".
*****************************************************************************/
DWORD CDSInterface::WriteXMLFile(SQLTCHAR *id, LPTSTR status)
{
	// convert element id and status to mbcs string
	char mbcsid[256] = {0};
	char mbcsstatus[256] = {0};
	wcstombs(mbcsid, id, 256);
	wcstombs(mbcsstatus, status, 256);

	CString xmlFileName;	//	name of xml file
	CFile xmlFile;	//	handle of xml file
	WIN32_FIND_DATA FindFileData;

	//	Get the name for xml file
	int nIndex = g_ModulePath.ReverseFind(_T('\\'));
	CString path = g_ModulePath.Left(nIndex) + _T("\\DBSXML");
	//exam that if path had been create
	HANDLE handle = FindFirstFile(path,&FindFileData);
	if(handle == INVALID_HANDLE_VALUE )
		CreateDirectory(path,NULL);

	xmlFileName.Format(_T("%s\\%s.xml"), path, id);	
	
	//	Establish the xml file to write
	
	BOOL bRtn = xmlFile.Open(xmlFileName, CFile::modeCreate|CFile::modeReadWrite|CFile::shareDenyNone, NULL);
	if(bRtn == FALSE)
	{
		return 1;
	}
	xmlFileName.ReleaseBuffer();

	(*gpDbSyncLog)(Log::L_INFO, _T("Begin to write XML file: %s"), xmlFileName);	//	added for tracing
	
	char	writeStr[MAXNAME];	//	String to write into the xml file
	char	sender[MAXNAME], receiver[MAXNAME];	//	Computer name of sender and receiver
	DWORD	bufferSize;	//	buffer size of computer name

	//	Get the computer name of sender and receiver
	bufferSize = sizeof(sender);
	GetComputerNameA(sender, &bufferSize);
	bufferSize = sizeof(receiver);
	GetComputerNameA(receiver, &bufferSize);

	//	Write the head of the xml file
	StringCbPrintfA(writeStr, MAXNAME, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	DWORD len = strlen(writeStr);

	xmlFile.Write(writeStr, strlen(writeStr));
	StringCbPrintfA(writeStr, MAXNAME, "<NoI_Sync_to_CMS xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"NoI_Sync_to_CMS.xsd\">\n");
	xmlFile.Write(writeStr, strlen(writeStr));

	//	Write the infohead element of the xml file
	StringCbPrintfA(writeStr, MAXNAME, "\t<NoIInfoHeader commandType=\"0\" sequence=\"0\">\n");
	xmlFile.Write(writeStr, strlen(writeStr));
	StringCbPrintfA(writeStr, MAXNAME, "\t\t<Sender computerName=\"%s\"/>\n", sender);
	xmlFile.Write(writeStr, strlen(writeStr));
	StringCbPrintfA(writeStr, MAXNAME, "\t\t<Receiver computerName=\"%s\"/>\n", receiver);
	xmlFile.Write(writeStr, strlen(writeStr));
	StringCbPrintfA(writeStr, MAXNAME, "\t</NoIInfoHeader>\n");
	xmlFile.Write(writeStr, strlen(writeStr));

	//	Write the element status of the xml file
	StringCbPrintfA(writeStr, MAXNAME, "\t<ElementStatus elementID=\"%s\" status=\"%s\"/>\n", mbcsid, mbcsstatus);
	xmlFile.Write(writeStr, strlen(writeStr));

	//	Write the end tag of the xml file
	StringCbPrintfA(writeStr, MAXNAME, "</NoI_Sync_to_CMS>\n");
	xmlFile.Write(writeStr, strlen(writeStr));

	//	Close the xml file
	xmlFile.Close();

	(*gpDbSyncLog)(Log::L_INFO, _T("End of writing XML file: %s"), xmlFileName);	//	added for tracing
		
	return 0;	//	success
}
DWORD CDSInterface::SaveMsgtoWorkQueue(const TCHAR* LocalEntryUID,const TCHAR* LocalParentUID,DWORD EntryType,DWORD operation,const TCHAR* MDNAME,const TCHAR* Name)
{
	if(!g_SupportNavigation)
		return 0;

	SQLTCHAR	sql[1024];	//	string to put SQL statement
	DWORD RetCode = 0;
	
	DWORD WQUID = 0;
	SQLTCHAR WQLUID[21];
	if(MDNAME != NULL && _tcsicmp(MDNAME, _T("")) != 0) // filter
	{
		SQLTCHAR MDName[500]={0};
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDNAME FROM MDLIST WHERE MDNAME = '%s'"),MDNAME);
		RetCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,MDName,sizeof(MDName));
		if(RetCode != SQL_SUCCESS) 
			return RetCode;
	}

	// filter the same WQ item what ever the operation type is
	SQLTCHAR entryID[500] = {0};
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCAL_ENTRY_UID FROM WQ_INFO WHERE LOCAL_ENTRY_UID = '%s' AND OPERATION_TYPE = %d"), 
		LocalEntryUID, operation);

	RetCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, entryID, sizeof(entryID));

	if(RetCode == SQL_SUCCESS) {
		return RetCode;
	}

	CString newMdName = MDNAME;
	newMdName.Replace(L"'", L"''");		// replace all comma to SQL style
	
	GetNewLocalUID(IDS_WORKQUEUE,&WQUID);
	StringCbPrintf(WQLUID, 21*sizeof(WCHAR), _T("W%ld"), WQUID);
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("INSERT INTO WQ_INFO (QUEUEUID,SOURCE_TYPE,LOCAL_ENTRY_UID,PARENT_HUID,ENTRY_TYPE,OPERATION_TYPE,MD_NAME,ENTRY_NAME) VALUES(N'%s',0,N'%s',N'%s',%ld,%ld,N'%s',N'%s')"),
		WQLUID,LocalEntryUID,LocalParentUID,EntryType,operation,newMdName,Name);
	return g_localDB.ExecuteSQL(sql);	
}

BOOL CDSInterface::IsPackaged(TCHAR* folderLUID)
{
	bool bRtn = false;
	DWORD mdUID = 0;// IsPackage 's MDUID
	DWORD retCode = 0;
	SQLTCHAR sql[1024];
	SQLINTEGER IsPackage;
	
	StringCbPrintf(sql, 1024*sizeof(WCHAR), _T("SELECT ISPACKAGE FROM FOLDER WHERE LOCALFOLDERUID = '%s'"),
		folderLUID);
	retCode = g_localDB.SelectSQL(sql,SQL_C_ULONG,&IsPackage,0);
	if(SQL_SUCCESS == retCode)
	{
		if(!IsPackage)
			return false;
		else 
			return true;
	}
	else 
		return false;
}
 int CDSInterface::VerifyMD(METADATA* pMd,TCHAR* OldValue)
{
	TCHAR bufffer[512];
	switch (pMd->wMdType)	//	chose the data type of the selection entry
	{
	case IDS_INTEGER:	
		StringCbPrintf(bufffer, 512*sizeof(WCHAR), _T("%ld"), pMd->iVal);
		break;
	case IDS_REAL:
		
		StringCbPrintf(bufffer, 512*sizeof(WCHAR), _T("%f"), pMd->rVal);
		
		break;
	case IDS_DATETIME:
		{		
			CString dVal;
			CTime tVal(pMd->tVal);		//	temp structure in converting DATETIME type
			if(tVal.GetYear() < 1970)
				dVal = _T("1970-01-01 00:00:00");	//	both IDS and LAM use local time, but if convered string is before 1970, 
			                                        //  the exported itv file from LAM will fail to imported by ITV
			else
				dVal = tVal.Format(_T("%Y-%m-%d %H:%M:%S"));	//	convert to SQL format
			StringCbPrintf(bufffer, 512*sizeof(WCHAR), _T("%s"), dVal);
			break;
		}
	case IDS_FLOAT:

		StringCbPrintf(bufffer, 512*sizeof(WCHAR), _T("%f"), pMd->fVal);
		break;
	case IDS_BOOLEAN:

		StringCbPrintf(bufffer, 512*sizeof(WCHAR), _T("%ld"), pMd->bitVal);
		break;
	case IDS_STRING:
		StringCbPrintf(bufffer, 512*sizeof(WCHAR), _T("%s"),pMd->sVal);
		break;
	default:
		return -1;	//	no data type is matching, something wrong
		break;
	}	//	end switch}
	if(wcscmp(bufffer,OldValue)== 0)
	{
		return 0;
	}
	else
		return 1;
}
// Description:
//
//
//
//
//
DWORD CDSInterface::DeleteUnusedObj(void)
{
////	Clear all local tables after synchronization
	//	Clear the deleted folders and its metadata

	// to avoid clear the exist data caused by ODBC error IDS error.
	// when need restart the service,do not clear the unusedobj
	// SQLTCHAR sql[1024];
	DWORD res = 0;	

	//	Clear the deleted elements and its metadata
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM CLIPMD WHERE LOCALCLIPUID IN (SELECT LOCALCLIPUID FROM CLIP WHERE ZQ_ENTRYSTATE = 1 OR ZQ_ENTRYSTATE = 2)"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM CLIP WHERE ZQ_ENTRYSTATE = 1  OR ZQ_ENTRYSTATE = 2"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ATOMICELEMENTMD WHERE (LOCALATOMICELEMENTUID NOT IN (SELECT LOCALATOMICELEMENTUID FROM ATOMICELEMENT) )  OR ( LOCALATOMICELEMENTUID IN (SELECT LOCALATOMICELEMENTUID FROM ATOMICELEMENT WHERE ZQ_ENTRYSTATE = 1  OR ZQ_ENTRYSTATE = 2) )"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ATOMICELEMENT WHERE ZQ_ENTRYSTATE = 1  OR ZQ_ENTRYSTATE = 2"));

	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM FOLDERMD WHERE LOCALFOLDERUID IN (SELECT LOCALFOLDERUID FROM FOLDER WHERE ZQ_ENTRYSTATE = 1 OR ZQ_ENTRYSTATE = 2)"));
	//	Clear the deleted selection entries
	res = g_localDB.ExecuteSQL(_T("DELETE FROM COMPLEXASSET WHERE ZQ_ENTRYSTATE = 1 OR ZQ_ENTRYSTATE = 2"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ASSETMD WHERE ( LOCALASSETUID NOT IN (SELECT LOCALASSETUID FROM ASSET) ) OR ( LOCALASSETUID IN (SELECT LOCALASSETUID FROM ASSET WHERE ZQ_ENTRYSTATE = 1  OR ZQ_ENTRYSTATE = 2) )"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ASSET WHERE ZQ_ENTRYSTATE = 1 OR ZQ_ENTRYSTATE = 2"));

	
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM FOLDERMD WHERE LOCALFOLDERUID IN (SELECT LOCALFOLDERUID FROM FOLDER WHERE ZQ_ENTRYSTATE = 1  OR ZQ_ENTRYSTATE = 2)"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM FOLDER WHERE ZQ_ENTRYSTATE = 1 OR ZQ_ENTRYSTATE = 2"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDSELECT WHERE ZQ_ENTRYSTATE = 1 OR ZQ_ENTRYSTATE = 2"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM APPLICATIONMD WHERE APPLICATIONUID IN (SELECT APPLICATIONUID FROM APPLICATION WHERE ZQ_ENTRYSTATE = 1 OR ZQ_ENTRYSTATE = 2)"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM APPLICATION WHERE ZQ_ENTRYSTATE = 1 OR ZQ_ENTRYSTATE = 2"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM SITEMD WHERE SITEUID IN (SELECT SITEUID FROM SITE WHERE ZQ_ENTRYSTATE = 1 OR ZQ_ENTRYSTATE = 2)"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM SITE WHERE ZQ_ENTRYSTATE = 1 OR ZQ_ENTRYSTATE = 2"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM HIERARCHY WHERE ZQ_ENTRYSTATE = 1 OR ZQ_ENTRYSTATE = 2"));

	// added by KenQ at 2007-09-19
	// a. Delete IDS_ASSET EntryMDD
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDSELECT WHERE MDUID IN (SELECT MDUID FROM ENTRYMDD WHERE MDUID>2000 AND ENTRYTYPE=1) AND MDUID NOT IN (SELECT DISTINCT MDUID FROM ASSETMD)"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDD WHERE MDUID > 2000 AND ENTRYTYPE=1 AND MDUID NOT IN (SELECT DISTINCT MDUID FROM ASSETMD)"));

	// b. Delete IDS_ATOMIC_ELEMENT EntryMDD
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDSELECT WHERE MDUID IN (SELECT MDUID FROM ENTRYMDD WHERE MDUID>2000 AND ENTRYTYPE=2) AND MDUID NOT IN (SELECT DISTINCT MDUID FROM ATOMICELEMENTMD)"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDD WHERE MDUID > 2000 AND ENTRYTYPE=2 AND MDUID NOT IN (SELECT DISTINCT MDUID FROM ATOMICELEMENTMD)"));

	// c. Delete IDS_FOLDER EntryMDD
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDSELECT WHERE MDUID IN (SELECT MDUID FROM ENTRYMDD WHERE MDUID>2000 AND ENTRYTYPE=3) AND MDUID NOT IN (SELECT DISTINCT MDUID FROM FOLDERMD)"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDD WHERE MDUID > 2000 AND ENTRYTYPE=3 AND MDUID NOT IN (SELECT DISTINCT MDUID FROM FOLDERMD)"));
	
	// d. Delete IDS_APPLICATION EntryMDD
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDSELECT WHERE MDUID IN (SELECT MDUID FROM ENTRYMDD WHERE MDUID>2000 AND ENTRYTYPE=4) AND MDUID NOT IN (SELECT DISTINCT MDUID FROM APPLICATIONMD)"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDD WHERE MDUID > 2000 AND ENTRYTYPE=4 AND MDUID NOT IN (SELECT DISTINCT MDUID FROM APPLICATIONMD)"));
	
	// e. Delete IDS_SITE EntryMDD
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDSELECT WHERE MDUID IN (SELECT MDUID FROM ENTRYMDD WHERE MDUID>2000 AND ENTRYTYPE=5) AND MDUID NOT IN (SELECT DISTINCT MDUID FROM SITEMD)"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDD WHERE MDUID > 2000 AND ENTRYTYPE=5 AND MDUID NOT IN (SELECT DISTINCT MDUID FROM SITEMD)"));
	
	// f. Delete IDS_CLIP EntryMDD
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDSELECT WHERE MDUID IN (SELECT MDUID FROM ENTRYMDD WHERE MDUID>2000 AND ENTRYTYPE=7) AND MDUID NOT IN (SELECT DISTINCT MDUID FROM CLIPMD)"));
	res = g_localDB.ExecuteSQL((SQLTCHAR *)_T("DELETE FROM ENTRYMDD WHERE MDUID > 2000 AND ENTRYTYPE=7 AND MDUID NOT IN (SELECT DISTINCT MDUID FROM CLIPMD)"));
	
	return res;
}
DWORD CDSInterface::SetACFlag(DWORD AssetUID)
{
	SQLTCHAR sql[1024];
	DWORD retCode = 0;
	SQLTCHAR AssetLUID[21];
	SQLTCHAR SubAssetLUID[21];
	SQLTCHAR FirstAELUID[21];
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Set AcFlag of Asset %d"),GetCurrentThreadId(),AssetUID);
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT LOCALASSETUID FROM ASSET WHERE ASSETUID = %ld"),AssetUID);
	retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,AssetLUID,sizeof(AssetLUID));
	if(retCode != SQL_SUCCESS)
	{
		return retCode;
	}
	DBRESULTSETS resultSets;
	DBRECORD selectCol,record;
	DBFIELD field1;
	field1.lSize = sizeof(SubAssetLUID);
	field1.lType = SQL_C_TCHAR;
	selectCol.push_back(field1);
	
	// condition of ACFLAG=5 is added by Ken.Q on 2007-1-15
	// The reason is, the 4 and 5's behaviour are the same in ITV, LAM needs to regarded as the same.
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT LOCALMEMBERUID FROM COMPLEXASSET WHERE LOCALASSETUID = '%s' AND MEMBERTYPE = 1 AND (ACFLAG = 4 OR ACFLAG = 5)"),
		AssetLUID);
	retCode = g_localDB.SelectAllSQL(sql,selectCol,&resultSets);
	if(retCode != SQL_SUCCESS)
		return retCode;
	for(DWORD i = 0;i < resultSets.size(); i ++)
	{
		record = resultSets.at(i);
		StringCbCopyW(SubAssetLUID, 21*sizeof(WCHAR), (wchar_t*)record.at(0).pValue);
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT LOCALMEMBERUID FROM COMPLEXASSET WHERE LOCALASSETUID = '%s' AND MEMBERTYPE = %ld AND SEQUENCE = 0"),
			SubAssetLUID,IDS_ATOMIC_ELEMENT);
		retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,FirstAELUID,sizeof(FirstAELUID));
		if(retCode != SQL_SUCCESS)
		{
			return retCode;
		}
		
		DWORD mdUID = 0;
		SQLTCHAR mdValue[500] = L"";
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDUID FROM ENTRYMDD WHERE MDNAME = 'ContentClass' AND ENTRYTYPE = 2"));
		retCode = g_localDB.SelectSQL(sql,SQL_C_ULONG,&mdUID,0);

		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT MDVALUE FROM ATOMICELEMENTMD WHERE LOCALATOMICELEMENTUID ='%s' AND MDUID = %ld "),
			FirstAELUID,mdUID);
		retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,mdValue,sizeof(mdValue));
		if(retCode != SQL_SUCCESS)
		{
			return retCode;
		}
		if((wcscmp(mdValue,_T("1")) == 0)||(wcscmp(mdValue,_T("")) == 0))
		{
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("UPDATE COMPLEXASSET SET ACFLAG = 1 WHERE  LOCALASSETUID = '%s' AND LOCALMEMBERUID = '%s' AND MEMBERTYPE = 1 "),
				AssetLUID,SubAssetLUID);
			retCode = g_localDB.ExecuteSQL(sql);
			
		}
		else if(wcscmp(mdValue,_T("2")) == 0)
		{
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("UPDATE COMPLEXASSET SET ACFLAG = 4 WHERE LOCALASSETUID = '%s' AND LOCALMEMBERUID = '%s' AND MEMBERTYPE = 1 "),
				AssetLUID,SubAssetLUID);
			retCode = g_localDB.ExecuteSQL(sql);
		}
		else if (wcscmp(mdValue,_T("8")) == 0)
		{
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("UPDATE COMPLEXASSET SET ACFLAG = 500 WHERE LOCALASSETUID = '%s' AND LOCALMEMBERUID = '%s' AND MEMBERTYPE = 1 "),
				AssetLUID,SubAssetLUID);
			retCode = g_localDB.ExecuteSQL(sql);

		}else 
		{
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("UPDATE COMPLEXASSET SET ACFLAG = 501 WHERE  LOCALASSETUID = '%s' AND LOCALMEMBERUID = '%s' AND MEMBERTYPE = 1 "),
				AssetLUID,SubAssetLUID);
			retCode = g_localDB.ExecuteSQL(sql);

		}
	}
	g_localDB.FreeResultSets(&resultSets);
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Set AcFlag of Asset %d"),GetCurrentThreadId(),AssetUID);
	return retCode;
}

void CDSInterface::MDDCallBackProcess(DWORD dwTriggerUid,WORD wType,METADATADESC *pMdd,WORD wOp)
{
	DWORD threadid = GetCurrentThreadId();
	(*gpDbSyncLog)(Log::L_INFO, _T("%d:Entering MDDCallBackProcess:  TriggerUID = %ld"),threadid, dwTriggerUid);
	try{	
		RETCODE retcode = SaveEntryMDSelectToDB(pMdd,1,wType);
	}
	catch(...)
	{
		(*gpDbSyncLog)(Log::L_WARNING,_T("%d:MDDCallBackProcess catch an exception when processing."),threadid); 
	}
	(*gpDbSyncLog)(Log::L_INFO, _T("%d:Leaving MDDCallBackProcess:  TriggerUID = %ld"),threadid, dwTriggerUid);

	IdsFreeMdDesc(pMdd, 1);
	
}
DWORD CDSInterface::GetMDList(TCHAR* MDListString, DWORD buffbytes)
{
	RETCODE retCode = 0;
	DBRESULTSETS resultSet;
	DBFIELD field;
	DBRECORD selectCol;
	field.lType = SQL_C_TCHAR;
	field.lSize = 64;

	selectCol.push_back(field);

	retCode = g_localDB.SelectAllSQL(_T("SELECT MDNAME FROM MDLIST"),selectCol,&resultSet);
	if(retCode != SQL_SUCCESS)
	{
		return retCode;
	}
	for(DWORD i = 0; i < resultSet.size(); i ++)
	{
		DBRECORD record = resultSet.at(i);
		if(wcslen(MDListString)!= 0)
			StringCbCatW(MDListString,buffbytes,_T("_"));
		StringCbCatW(MDListString,buffbytes,(wchar_t*)record.at(0).pValue);
	}
	g_localDB.FreeResultSets(&resultSet);
	return retCode;
}

DWORD CDSInterface::CheckConn(void)
{
	(*gpDbSyncLog)(Log::L_DEBUG,_T("Start to check the connection to IDS."));
	ITVVERSION itvver;
	RETCODE res = IdsGetServiceVersion(&m_idssess,&itvver,&m_itvsta,NULL);
	return res;
}
DWORD CDSInterface::SaveEntryMDDToDB(void)
{
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Save Entry MDD to DB"),GetCurrentThreadId());
	DWORD res = 0;
	DWORD descNum;		//	Number of select entries
	METADATADESC *pMdescEx = 0;		//	Structure to store select entry description info
	res = IdsGetMd (&m_idssess, IDS_SITE, &pMdescEx, &descNum, &m_itvsta, 0);
	if (!res)
	{
		//	Save all select entry descriptions into Local DB
		res = SaveEntryMDSelectToDB(pMdescEx, descNum, IDS_SITE);
		IdsFreeMdDesc(pMdescEx, descNum);	//	Free memory
	}else
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("IdsGetMd ERROR!0X%X"),res);
	}
	//	Get number of select entries and all description info


	
	//	Get number of select entries and all description info
	res = IdsGetMd (&m_idssess, IDS_FOLDER, &pMdescEx, &descNum, &m_itvsta, 0);
	if (!res)
	{
		//	Save all select entry descriptions into Local DB
		res = SaveEntryMDSelectToDB(pMdescEx, descNum, IDS_FOLDER);
		IdsFreeMdDesc(pMdescEx, descNum);	//	Free memory
	}else
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("IdsGetMd ERROR!0X%X"),res);
	}

	res = IdsGetMd (&m_idssess, IDS_APPLICATION, &pMdescEx, &descNum, &m_itvsta, 0);
	if (!res)
	{
		//	Save all select entry descriptions into Local DB
		res = SaveEntryMDSelectToDB(pMdescEx, descNum, IDS_APPLICATION);
		IdsFreeMdDesc(pMdescEx, descNum);	//	Free memory
	}else
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("IdsGetMd ERROR!0X%X"),res);
	}
	res = IdsGetMd (&m_idssess, IDS_ASSET, &pMdescEx, &descNum, &m_itvsta, 0);
	if (!res)
	{
		//	Save all select entry descriptions into Local DB
		res = SaveEntryMDSelectToDB(pMdescEx, descNum, IDS_ASSET);
		IdsFreeMdDesc(pMdescEx, descNum);	//	Free memory
	}else
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("IdsGetMd ERROR!0X%X"),res);
	}
	res = IdsGetMd (&m_idssess, IDS_ATOMIC_ELEMENT, &pMdescEx, &descNum, &m_itvsta, 0);
	if (!res)
	{
		//	Save all select entry descriptions into Local DB
		res = SaveEntryMDSelectToDB(pMdescEx, descNum, IDS_ATOMIC_ELEMENT);
		IdsFreeMdDesc(pMdescEx, descNum);	//	Free memory
	}else
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("IdsGetMd ERROR!0X%X"),res);
	}
	
	res = IdsGetMd (&m_idssess, IDS_CLIP, &pMdescEx, &descNum, &m_itvsta, 0);
	if (!res)
	{
		//	Save all select entry descriptions into Local DB
		res = SaveEntryMDSelectToDB(pMdescEx, descNum, IDS_CLIP);
		IdsFreeMdDesc(pMdescEx, descNum);	//	Free memory
	}else
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("IdsGetMd ERROR!0X%X"),res);
	}
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Save Entry MDD to DB"),GetCurrentThreadId());
	return res;
}

DWORD CDSInterface::SaveEntryMDDToDBEx(WORD entryType, const TCHAR* mddName)
{	
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Save Entry MDD to DB for Type: %d, MDDName: %s"),
		GetCurrentThreadId(), entryType, mddName);
	
	DWORD res = 0;
	DWORD descNum;		//	Number of select entries
	METADATADESC *pMdescEx = 0;		//	Structure to store select entry description info
	switch(entryType) 
	{
	case IDS_SITE:
		res = IdsGetMd (&m_idssess, IDS_SITE, &pMdescEx, &descNum, &m_itvsta, 0);
		if (!res)
		{
			for (DWORD i=0; i<descNum; i++)	//	for each metadata
			{
				if(_tcsicmp((pMdescEx+i)->wszMdName, mddName) == 0)
				{
					break;
				}
			}
			if(i < descNum)
			{
				//	Save all select entry descriptions into Local DB
				res = SaveEntryMDSelectToDBEx(*(pMdescEx+i), IDS_SITE);
			}
			IdsFreeMdDesc(pMdescEx, descNum);	//	Free memory
		}else
		{
			(*gpDbSyncLog)(Log::L_INFO, _T("IdsGetMd ERROR!0X%X"),res);
		}

		break;
	case IDS_FOLDER:
		//	Get number of select entries and all description info
		res = IdsGetMd (&m_idssess, IDS_FOLDER, &pMdescEx, &descNum, &m_itvsta, 0);
		if (!res)
		{
			for (DWORD i=0; i<descNum; i++)	//	for each metadata
			{
				if(_tcsicmp((pMdescEx+i)->wszMdName, mddName) == 0)
				{
					break;
				}
			}
			if(i < descNum)
			{
				//	Save all select entry descriptions into Local DB
				res = SaveEntryMDSelectToDBEx(*(pMdescEx+i), IDS_FOLDER);
			}
			IdsFreeMdDesc(pMdescEx, descNum);	//	Free memory
		}else
		{
			(*gpDbSyncLog)(Log::L_INFO, _T("IdsGetMd ERROR!0X%X"),res);
		}

		break;
	case IDS_APPLICATION:
		res = IdsGetMd (&m_idssess, IDS_APPLICATION, &pMdescEx, &descNum, &m_itvsta, 0);
		if (!res)
		{
			//	Save all select entry descriptions into Local DB
			for (DWORD i=0; i<descNum; i++)	//	for each metadata
			{
				if(_tcsicmp((pMdescEx+i)->wszMdName, mddName) == 0)
				{
					break;
				}
			}
			if(i < descNum)
			{
				res = SaveEntryMDSelectToDBEx(*(pMdescEx+i), IDS_APPLICATION);
			}
			IdsFreeMdDesc(pMdescEx, descNum);	//	Free memory
		}else
		{
			(*gpDbSyncLog)(Log::L_INFO, _T("IdsGetMd ERROR!0X%X"),res);
		}
		break;
	case IDS_ASSET:
		res = IdsGetMd (&m_idssess, IDS_ASSET, &pMdescEx, &descNum, &m_itvsta, 0);
		if (!res)
		{
			for (DWORD i=0; i<descNum; i++)	//	for each metadata
			{
				if(_tcsicmp((pMdescEx+i)->wszMdName, mddName) == 0)
				{
					break;
				}
			}
			if(i < descNum)
			{
				//	Save all select entry descriptions into Local DB
				res = SaveEntryMDSelectToDBEx(*(pMdescEx+i), IDS_ASSET);
			}
			IdsFreeMdDesc(pMdescEx, descNum);	//	Free memory
		}else
		{
			(*gpDbSyncLog)(Log::L_INFO, _T("IdsGetMd ERROR!0X%X"),res);
		}
		break;
	case IDS_ATOMIC_ELEMENT:
		res = IdsGetMd (&m_idssess, IDS_ATOMIC_ELEMENT, &pMdescEx, &descNum, &m_itvsta, 0);
		if (!res)
		{
			for (DWORD i=0; i<descNum; i++)	//	for each metadata
			{
				if(_tcsicmp((pMdescEx+i)->wszMdName, mddName) == 0)
				{
					break;
				}
			}
			if(i < descNum)
			{
				//	Save all select entry descriptions into Local DB
				res = SaveEntryMDSelectToDBEx(*(pMdescEx+i), IDS_ATOMIC_ELEMENT);
			}
			IdsFreeMdDesc(pMdescEx, descNum);	//	Free memory
		}else
		{
			(*gpDbSyncLog)(Log::L_INFO, _T("IdsGetMd ERROR!0X%X"),res);
		}
		break;
	case IDS_CLIP:
		res = IdsGetMd (&m_idssess, IDS_CLIP, &pMdescEx, &descNum, &m_itvsta, 0);
		if (!res)
		{
			for (DWORD i=0; i<descNum; i++)	//	for each metadata
			{
				if(_tcsicmp((pMdescEx+i)->wszMdName, mddName) == 0)
				{
					break;
				}
			}
			if(i < descNum)
			{
				//	Save all select entry descriptions into Local DB
				res = SaveEntryMDSelectToDBEx(*(pMdescEx+i), IDS_CLIP);
			}
			IdsFreeMdDesc(pMdescEx, descNum);	//	Free memory
		}else
		{
			(*gpDbSyncLog)(Log::L_INFO, _T("IdsGetMd ERROR!0X%X"),res);
		}
		break;
	default:
		break;
	}

		
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exit Entry MDD to DB for Type: %d, MDDName: %s"),
		GetCurrentThreadId(), entryType, mddName);
	return res;	
}

bool CDSInterface::IsAssetUpdated(DWORD AssetUID)
{
	DWORD dwEntryState = 0;
	DWORD dwNum = 0;
	SQLTCHAR sql[1024] = _T("");
	
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR),_T("SELECT ZQ_ENTRYSTATE FROM ASSET WHERE ASSETUID = %d "),AssetUID);
	DWORD ret = g_localDB.SelectSQL(sql,SQL_C_ULONG,&dwEntryState,0);
	if(ret!= ITV_SUCCESS)
		return false;
	// only when the record is in the table and the METADATA is not empty 
	// do not update the infomation which ZQ_Entrystate = 2
	if(dwEntryState == 0 || dwEntryState == 2 )
		return true;
	else 
		return false;
	
}

DWORD CDSInterface::SaveAppsToDB(void)
{
	// comment by KenQ at 2007-2-14, IdsListApplications() could not fetch the 
	// concrete Application's metadata, coz its metadata (appname+i)->dwNumMd is always 0
	// so, SaveAppMdToDB() directly will not able to save the metadata.
	DWORD dwNum;
	APPNAME* appname;
	RTNDEF rtn = IdsListApplications(&m_idssess,&appname,&dwNum,&m_itvsta,NULL);

	rtn = SaveAppsToDB(appname,dwNum);
	for(DWORD i =0; i < dwNum;i ++)
	{
		// SaveAppMdToDB((appname+i)->pMd,(appname+i)->dwNumMd,(appname+i)->dwUid);
		SaveObjectMdToDB((appname+i)->dwUid, IDS_APPLICATION);
	}
	IdsFree(appname);
	return rtn;
}

void CDSInterface::NetBreakCallBack(IDSSESS* pIdsSession)
{
//	---------- modified by KenQ 2006-06-09-------------
//	When this happens, the DBSync will not auto start
//  So trigger the check thread to take over the reconnection job
//  instead of reconnection in the callback itself

	(*gpDbSyncLog)(Log::L_WARNING,_T("Connection to IDS server lost trigger by NetBreakCallBack, Check thread will do the check again"));

	ConnChecker* pChecker = g_server.GetConnChecker();
	if(pChecker != NULL)
	{
		pChecker->TriggerCheck();
	}
}

bool CDSInterface::AdjustRoot(const WCHAR* ori_root, CString& outString)
{
	outString = ori_root;

//	---------- modified by KenQ 2006-08-03-------------
//  A switch to support in case of sync one folder, whether original folder itself 
//	is to linked to LAM Navigation Application.

	int nSyncFolderCount = g_SyncFolders.size();

	// If only sync one folder and the synced folder did not as the nav node
	if(1 == nSyncFolderCount && g_SyncedFolderAsNavNode == 0) 
	{
		CString syncFolderName = g_SyncFolders.at(0).c_str();
		if(wcscmp(syncFolderName, _T("\\"))==0)
			return true;

		int rootlen = wcslen(syncFolderName);
		if(outString.Left(rootlen).CompareNoCase(syncFolderName) == 0)
		{	// this is what we care about, replace the root to the Sync Directory
			int appindex = outString.Find(_T("\\"), 1);
			outString.Delete(appindex+1, rootlen-appindex-1);
			return true;
		}
		else
		{
			return false;
		}	
	}

	// Multiple folders syncing or one folder syncing which will as the nav folder
	WCHAR syncFolderName[MAXNAMELEN*2]={0};	

	int nSyncLoop = 0;
	for(nSyncLoop=0; nSyncLoop < nSyncFolderCount; nSyncLoop++)
	{
		// get the sync folder name
		CString syncFolderName = g_SyncFolders.at(nSyncLoop).c_str();

		// if sync directory is root, do not adjust
		if(syncFolderName == L"\\")
			return true;

		// check whether it includes the sync folder
		if(outString.Find(syncFolderName) == -1)
			continue;
		
		int rootlen = syncFolderName.GetLength();
		if(outString.Left(rootlen).CompareNoCase(syncFolderName) == 0)
		{	// this is what we care about, replace the root to the Sync Directory
			int appindex = outString.Find(_T("\\"), 1);

			// remove the last \ in the root
			syncFolderName.SetAt(rootlen-1, _T('\0'));
			int syncFdIndex = syncFolderName.ReverseFind(_T('\\'));
			if(appindex<syncFdIndex)
			{
				outString.Delete(appindex+1, syncFdIndex-appindex);
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

int CDSInterface::GetEntryHUIDByName(const IN WCHAR* entryName, OUT DWORD& entryHUID, OUT DWORD& entryType)
{
	int nRet = 0;

	entryHUID =0;
	entryType=IDS_APPLICATION;

	if(entryName == NULL)
		return 0;

	// if entry is root, just return 1
	if(wcscmp(entryName, L"\\")==0)
	{
		entryHUID = 0;
		entryType=IDS_SITE;
		return 1;
	}

	CString strName = entryName;
	strName.Replace(L"'", L"''");
	
	if(strName.Left(1)!=_T("\\"))
		return 0;
	
	int nIndex = strName.Left(strName.GetLength()-1).ReverseFind(_T('\\'));
	CString strParentName = strName.Left(nIndex);
	strName = strName.Right(strName.GetLength() - nIndex - 1);
	strName = strName.Left(strName.GetLength()-1);
	if(strParentName == _T(""))// application
		strParentName = "\\";

	WORD queryType = (strParentName=="\\")? IDS_APPLICATION : IDS_FOLDER;

	// prepare search string
	WCHAR stringSql[256];

//	---------- modified by KenQ 2006-06-09-------------
//	Original sql filter statement only suit for type is IDS_FOLDER, 
//	once the type is IDS_APPLICATION, the IdsFindObjectEx() function will failed
//  So here adding the case for IDS_APPLICATION

//  ---------- modified by KenQ 2009-01-16-------------
//  For application, we have to check its parent too, coz in IDS, maybe there multiple applicaiton entry name are same, but parent are different. 
//  So, do NOT seperate IDS_APPLICATION
	/*
	if(queryType == IDS_APPLICATION)
	{	// find a application 
		StringCbPrintf(stringSql, 256*sizeof(WCHAR), _T("ENTRY = N'%s'"), strName);  

//	---------- end of modified by KenQ -------------
	}
	else
	{	// find a folder 
		StringCbPrintf(stringSql, 256*sizeof(WCHAR), _T("ENTRY = N'%s' and Hierarchy.PARENT = N'%s' and Hierarchy.ASSET_UID = 0"), strName, strParentName);  
	}
	*/
	StringCbPrintf(stringSql, 256*sizeof(WCHAR), _T("ENTRY = N'%s' and Hierarchy.PARENT = N'%s' and Hierarchy.ASSET_UID = 0"), strName, strParentName);  

	OBJECTLIST *pObjectList = NULL;
	DWORD dwCount = 0;
	
	ITVSTATUS itvStatus;
	if (ITV_SUCCESS != (itvStatus = ::IdsFindObjectEx(&m_idssess,
											IDS_FOLDER,	// find the data in hierarchy
											0,			// not asset, so app uid=0
											stringSql,	// filter
											&pObjectList,
											&dwCount,
											&m_itvsta,
											NULL)))
	{
		// query failed
		nRet = 0;
	}
	else if(dwCount==0 || dwCount>1)
	{
		// query result more than one.  That's impossible, must be something wrong
		nRet = dwCount;
	}
	else
	{
		entryType	= queryType;
		entryHUID	= pObjectList[0].dwUid;
		nRet		= 1;
	}

	// free memory
	if (pObjectList != NULL && dwCount > 0)
	{
		::IdsFreeObjectList(pObjectList, dwCount);
	}

	if(dwCount > 0)
	{	
		(*gpDbSyncLog)(Log::L_DEBUG, _T("%d object(s) found in IDS Hierachy for sync folder %s"), dwCount, entryName);
		int i=0;
		for(i=0; i<dwCount; i++)
		{
			(*gpDbSyncLog)(Log::L_DEBUG, _T("ObjectName=%s, ObjectHUID=%d"), pObjectList[0].wszName, pObjectList[0].dwUid);
		}
	}

	return nRet;
}

bool CDSInterface::validTimeWindow(IN WCHAR* mdname, IN WCHAR* time1, IN WCHAR* time2)
{
	bool goon=false;

	for(int i=0; i<TIMEWINDOW_COUNT; i++)	// check if metadata is what we care about
	{
		if(g_TimeWindowList[i]==mdname)
		{
			goon = true;
			break;
		}
	}

	if(!goon)
		return true;

	if(g_TimeWindowThreshold==0 || g_TimeWindowThreshold>=86400)	// if threshold is 0 or longer than 1 day, all time window differences are valid
		return true;
	
	std::wstring  strT1 = time1;
	std::wstring  strT2 = time2;
	
	if(strT1.empty() || strT2.empty())
		return true;
	
	if(strT1.length()!=19 || strT2.length()!=19)
		return true;
	
	tm	tb1,tb2;
	time_t	tval1,tval2;
	
	tb1.tm_year = _wtoi(strT1.substr(0,4).c_str())-1900;
	tb2.tm_year	= _wtoi(strT2.substr(0,4).c_str())-1900;
	tb1.tm_mon	= _wtoi(strT1.substr(5,2).c_str())-1;
	tb2.tm_mon	= _wtoi(strT2.substr(5,2).c_str())-1;
	tb1.tm_mday	= _wtoi(strT1.substr(8,2).c_str());
	tb2.tm_mday	= _wtoi(strT2.substr(8,2).c_str());
	tb1.tm_hour	= _wtoi(strT1.substr(11,2).c_str());
	tb2.tm_hour	= _wtoi(strT2.substr(11,2).c_str());
	tb1.tm_min	= _wtoi(strT1.substr(14,2).c_str());
	tb2.tm_min	= _wtoi(strT2.substr(14,2).c_str());
	tb1.tm_sec	= _wtoi(strT1.substr(17,2).c_str());
	tb2.tm_sec	= _wtoi(strT2.substr(17,2).c_str());
	
	tval1 = mktime(&tb1);
	tval2 = mktime(&tb2);
	
	if(tval1==-1 || tval2==-1)
		return true;
	
	int diff = 0;
	if((DWORD)tval1 > (DWORD)tval2)
		diff = tval1 - tval2;
	else
		diff = tval2 - tval1;
	
	if(diff>g_TimeWindowThreshold)
		return true;
	else
		return false;
}

bool CDSInterface::isMultipleEntry(IN DWORD index, IN METADATA* pMd)
{
	bool bRet = false;

	if(index-1<0)	// no previous metadata entry
		return false;

	WCHAR* pOriName = (pMd+index)->wszMdName;

	for(int i=index-1; i>=0; i--)
	{
		WCHAR* pNowName = (pMd+i)->wszMdName;
		if(0==wcscmp(pOriName, pNowName))
		{
			bRet = true;
			break;
		}
	}

	return bRet;
}

bool CDSInterface::CheckIDSObjExistence(DWORD entryUID, DWORD entryType, ITVSTATUS& errCode)
{
	errCode = ITV_SUCCESS;
	
	if(IDS_FOLDER == entryType )
	{
		// for folder, it is just a record in hierarchy, so if a folder was deleted
		// from hierarchy, the object must have not been there anymore.
		return false;
	}
	else if(IDS_ASSET == entryType)
	{
		APPNAME *pApps = 0;	
		DWORD appNum = 0;

		//	Get number of applications and all application info
		errCode = IdsListApplications(&m_idssess, &pApps, &appNum, &m_itvsta, NULL);
		if (errCode != ITV_SUCCESS)
		{
			(*gpDbSyncLog)( Log::L_ERROR,_T("IdsListApplications() ERROR in CheckIDSObjExistence()"));
		
			return false;	
		}
		if(0 == appNum)
		{
			(*gpDbSyncLog)( Log::L_ERROR,_T("IdsListApplications() return with NO application existing"));
		
			return false;	
		}
		// prepare search string
		WCHAR stringSql[256];
		StringCbPrintf(stringSql, 256*sizeof(WCHAR), _T("Asset.ASSET_UID = %d"), entryUID);  


		for(int i=0; i<appNum; i++)
		{
			OBJECTLIST *pObjectList = NULL;
			DWORD dwCount = 0;

			errCode = IdsFindObjectEx(&m_idssess,
								IDS_ASSET,	
								pApps->dwUid,
								stringSql,	// filter
								&pObjectList,
								&dwCount,
								&m_itvsta,
								NULL);
			if(errCode != ITV_SUCCESS)
			{
				IdsFree(pApps);
				return false;
			}

			// exit when found the asset
			if (pObjectList != NULL && dwCount > 0)
			{
				IdsFree(pApps);
				IdsFreeObjectList(pObjectList, dwCount);
				return true;
			}
		}
		
		IdsFree(pApps);
		return false;
	}
	else if(IDS_ATOMIC_ELEMENT == entryType)
	{
		DWORD dwCount = 0; 
		OBJECTLIST *pObjectList = NULL;

		errCode = IdsFindAssetByMember(&m_idssess, 
									 IDS_ATOMIC_ELEMENT, 
									 entryUID, 
									 &pObjectList, 
									 &dwCount, 
									 &m_itvsta, 
									 NULL);
		if(errCode != ITV_SUCCESS)
		{
			//	Log error message
			(*gpDbSyncLog)( Log::L_ERROR,_T("IdsFindAssetByMember() ERROR in CheckIDSObjExistence()"));
			
			return false;
		}

		if (pObjectList != NULL && dwCount > 0)
		{
			::IdsFreeObjectList(pObjectList, dwCount);
		}
		// has this asset, return true;
		return (dwCount > 0) ? true : false;
	}
	return false;
}

bool CDSInterface::UpdateFolderSubNodeZQEntryState(DWORD huid)
{
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Update ZQ_ENTRYSTATE to 1 to folder %d sub folder"),
		GetCurrentThreadId(), huid);

	TCHAR sql[1024];
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
				   _T("UPDATE HIERARCHY SET ZQ_ENTRYSTATE = 1 WHERE ParentHUID = %d AND ZQ_ENTRYSTATE = 0"), 
				   huid);

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);

	g_localDB.ExecuteSQL(sql);

	return true;
}

bool CDSInterface::RemoveHierarchyForManaulSync(DWORD folderHUID, ITVSTATUS& errCode)
{
	TCHAR sql[1024];

	// get parent LocalHUID
	SQLTCHAR szFolderHUID[21] = _T("");
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALHIERARCHYUID FROM HIERARCHY WHERE HIERARCHYUID = %ld"), 
		folderHUID);
	g_localDB.SelectSQL(sql, SQL_C_TCHAR, szFolderHUID, sizeof(szFolderHUID));


	//
	// fetching all the hierarchy record which gonna be deleted by ZQEntryState
	//
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT HierarchyUID, LocalEntryUID, EntryUID, EntryType FROM HIERARCHY WHERE ZQ_ENTRYSTATE = 1 AND ParentHUID = %d"), folderHUID);
	
	DBRESULTSETS resultset;
	DBRECORD SelectRecord,record;
	DBFIELD field1,field2, field3, field4;
	
	field1.lType = SQL_C_ULONG;
	field2.lType = SQL_C_TCHAR;
	field2.lSize = 256;
	field3.lType = SQL_C_ULONG;
	field4.lType = SQL_C_ULONG;
	SelectRecord.push_back(field1);
	SelectRecord.push_back(field2);
	SelectRecord.push_back(field3);
	SelectRecord.push_back(field4);
	
	RETCODE retCode = g_localDB.SelectAllSQL(sql, SelectRecord, &resultset);

	for(DWORD i = 0; i < resultset.size(); i ++)
	{
		DBRECORD record = resultset.at(i);
		
		DWORD objHUID = 0;
		WCHAR objEntryLUID[256];
		DWORD objEntryUID = 0;
		DWORD objEntryType = 0;

		memcpy(&objHUID, record.at(0).pValue,4);
		StringCbCopyW(objEntryLUID, 256*sizeof(WCHAR), (WCHAR*)record.at(1).pValue);
		memcpy(&objEntryUID, record.at(2).pValue,4);
		memcpy(&objEntryType, record.at(3).pValue,4);

		//
		// check each object to see whether it is existed in IDS, if NOT existing, delete it(with related data) from LAM
		//
		ITVSTATUS errCode = ITV_SUCCESS;
		if(!CheckIDSObjExistence(objEntryUID, objEntryType, errCode))
		{
			if(errCode != ITV_SUCCESS)
			{
				return errCode;
			}
			
			if(IDS_FOLDER == objEntryType)
			{
				// recursive call, coz the folder may have child nodes(folder/asset)
				// by recursiving, 
				// 1) Set the folder's child node's entry state
				UpdateFolderSubNodeZQEntryState(objHUID);
				// 2) Remove all asset from the folder
				RemoveHierarchyForManaulSync(objHUID, errCode);
				if(errCode != ITV_SUCCESS)
				{
					return false;
				}
				// 3) Then remove the empty folder
				DeleteFolder(objEntryLUID, objEntryUID);
			}
			else if(IDS_ASSET)
			{
				DeleteAsset(objEntryLUID, objEntryUID, szFolderHUID, errCode);
				if(errCode != ITV_SUCCESS)
				{
					return false;
				}
			}
		}
	}

	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM HIERARCHY WHERE ZQ_ENTRYSTATE = 1 AND ParentHUID = %d"), folderHUID);

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);

	g_localDB.ExecuteSQL(sql);

	return true;
}

// Delete an asset from LAM, need to consider ComplexAsset, AtomicElement, etc
bool CDSInterface::DeleteAsset(IN const SQLTCHAR* assetLUID, DWORD entryUID, IN const SQLTCHAR* parentHUID, ITVSTATUS& errCode)
{
	errCode = ITV_SUCCESS;

	// fetching all the hierarchy record which gonna be deleted
	(*gpDbSyncLog)(Log::L_INFO, _T("%d:Enter DeleteAsset(%s, %d) in manually sync process"), 
		           GetCurrentThreadId(), assetLUID, entryUID);

	//
	// Get the AtomicElementUID list and check whether it is existed in ITV, 
	// if yes, just remove the relationship at ComplexAsset table between Asset and AtomicElement without delete AtomicElement related data
	// if no, remove the relationship at ComplexAsset and AtomicElement related data
	//
	TCHAR sql[1024];
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
		           _T("SELECT e.LocalAtomicElementUID, e.AtomicElementUID FROM Asset a, ComplexAsset c, AtomicElement e WHERE a.LocalAssetUID = c.LocalAssetUID AND c.LocalMemberUID = e.LocalAtomicElementUID AND c.MemberType = %d AND a.AssetUID = %d"), 
				   IDS_ATOMIC_ELEMENT, entryUID);
	
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:List all the Element belong to asset %s"), GetCurrentThreadId(), assetLUID);
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);
	
	DBRESULTSETS resultset;
	DBRECORD SelectRecord,record;
	DBFIELD field1,field2;
	
	field1.lType = SQL_C_TCHAR;
	field1.lSize = 256;
	field2.lType = SQL_C_ULONG;
	SelectRecord.push_back(field1);
	SelectRecord.push_back(field2);
	
	RETCODE retCode = g_localDB.SelectAllSQL(sql, SelectRecord, &resultset);

	for(DWORD i = 0; i < resultset.size(); i ++)
	{
		DBRECORD record = resultset.at(i);
		
		WCHAR elementLUID[256];
		DWORD elementUID = 0;

		StringCbCopyW(elementLUID, 256*sizeof(WCHAR), (WCHAR*)record.at(0).pValue);
		memcpy(&elementUID, record.at(1).pValue,4);

		(*gpDbSyncLog)(Log::L_INFO,_T("%d:Asset %s found Element, its ElementUID = %d"), 
										GetCurrentThreadId(), assetLUID, elementUID);

		// check the existence of atomic element
		if(!CheckIDSObjExistence(elementUID, IDS_ATOMIC_ELEMENT, errCode))
		{
			if(errCode != ITV_SUCCESS)
			{
				return false;
			}
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:Asset %s Element %d did not existed in ITV, will delete related data from AtomicElement and ComplexAsset table"), 
											GetCurrentThreadId(), assetLUID, elementUID);

			// delete the record in ComplexAsset which related with this element
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
						   _T("DELETE ComplexAsset WHERE LocalMemberUID = '%s'"), elementLUID);

			(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);

			g_localDB.ExecuteSQL(sql);

			// delete the local atomic element from AtomicElementMD talbe
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
						   _T("DELETE AtomicElementMD WHERE LocalAtomicElementUID = '%s'"), elementLUID);

			(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);

			g_localDB.ExecuteSQL(sql);			

			// delete the local atomic element from AtomicElement talbe
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
						   _T("DELETE AtomicElement WHERE AtomicElementUID = %d"), elementUID);

			(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);

			g_localDB.ExecuteSQL(sql);			
		}
	}

	//
	// Check each asset's sub asset(complex asset), and check whether it is existed in ITV
	// if yes, just remove the relationship at ComplexAsset table between Asset and sub asset without delete the sub asset related data
	// if no, remove the relationship at ComplexAsset and sub asset related data, to the sub asset, it require recursive checking. 
	//
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
						_T("SELECT c.LocalMemberUID FROM Asset a, ComplexAsset c WHERE a.LocalAssetUID = c.LocalAssetUID AND c.MemberType = %d AND a.AssetUID = %d"), 
						IDS_ASSET, entryUID);

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:List all the Asset belong to Asset %s"), GetCurrentThreadId(), assetLUID);
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);

	SelectRecord.clear();
	resultset.clear();
	
	field1.lType = SQL_C_TCHAR;
	field1.lSize = 256;
	SelectRecord.push_back(field1);

	retCode = g_localDB.SelectAllSQL(sql, SelectRecord, &resultset);

	for(i = 0; i < resultset.size(); i ++)
	{
		// Get each asset's sub asset's LocalAssetUID
		DBRECORD record = resultset.at(i);
		
		DWORD dwSubAssetEntryUID = 0;
		WCHAR subAssetLUID[256];
		StringCbCopyW(subAssetLUID, 256*sizeof(WCHAR), (WCHAR*)record.at(0).pValue);

		// Get each asset's sub asset's AssetUID
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT AssetUID FROM Asset WHERE LocalAssetUID = '%s'"), subAssetLUID);
						
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);

		RETCODE retCode = g_localDB.SelectSQL(sql,SQL_C_ULONG, &dwSubAssetEntryUID, 0);

		if(SQL_NO_DATA == retCode)
		{
			continue;
		}

		(*gpDbSyncLog)(Log::L_INFO,_T("%d:Asset %s includes sub Asset %s, %d"),
						GetCurrentThreadId(), assetLUID, subAssetLUID, dwSubAssetEntryUID);

		// check the sub asset's existence
		if(SQL_SUCCESS == retCode && !CheckIDSObjExistence(dwSubAssetEntryUID, IDS_ASSET, errCode))
		{
			if(errCode != ITV_SUCCESS)
			{
				return false;
			}

			(*gpDbSyncLog)(Log::L_INFO,_T("%d:Asset %s 's sub Asset %s, %d also did not exist in ITV, recursive delete the sub asset"),
							GetCurrentThreadId(), assetLUID, subAssetLUID, dwSubAssetEntryUID);
			// recursive invoking to the sub asset
			bool bRet = DeleteAsset(subAssetLUID, dwSubAssetEntryUID, L"", errCode);
			if(!bRet)
			{
				if(errCode != ITV_SUCCESS)
				{
					return false;
				}
			}
			// delete the record in ComplexAsset which related with this element
			StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
						   _T("DELETE ComplexAsset WHERE LocalMemberUID = '%s'"), subAssetLUID);
			
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);

			g_localDB.ExecuteSQL(sql);
		}
	}

	//
	// delete the asset information, coz its ComplexAsset & AtomicElement has been processed above
	//
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:Asset %s did not existed in ITV, will delete related data from ASSETMD and ASSET table"), 
									GetCurrentThreadId(), assetLUID);

	// delete the record in ComplexAsset which related with this asset
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), 
				   _T("DELETE ComplexAsset WHERE LocalAssetUID = '%s'"), assetLUID);
	
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);

	g_localDB.ExecuteSQL(sql);
	
	// delete AssetMD
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM ASSETMD WHERE LOCALASSETUID = '%s'"), assetLUID);

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);

	retCode = g_localDB.ExecuteSQL(sql);
	
	// delete Asset
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM ASSET WHERE ASSETUID = %ld"), entryUID);
	
	(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);

	retCode = g_localDB.ExecuteSQL(sql);

	if(retCode == SQL_SUCCESS)
	{
		SaveMsgtoWorkQueue(assetLUID,parentHUID,IDS_ASSET,WORKQUEUE_DELETE,L"",L"");
	}
	
	(*gpDbSyncLog)(Log::L_INFO, _T("%d:Leave DeleteAsset(%s, %d) in manually sync process"), 
		           GetCurrentThreadId(), assetLUID, entryUID);

	return true;
}

/// Delete an empty folder from LAM
bool CDSInterface::DeleteFolder(IN const SQLTCHAR* folderLUID, DWORD entryUID)
{
	TCHAR sql[1024];
	//
	// Following codes is copied from FolderCallBackProcess() 
	//
	(*gpDbSyncLog)(Log::L_INFO, _T("%d:Enter DeleteFolder(%s, %d) in manually sync process"), 
		           GetCurrentThreadId(), folderLUID, entryUID);

	//	Delete the metadata of the folder from FOLDERMD table
	WCHAR LocalFolderUID[64] = _T("");

	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT LOCALFOLDERUID FROM FOLDER WHERE FOLDERUID = %ld"),
			 entryUID);
	RETCODE retCode = g_localDB.SelectSQL(sql,SQL_C_TCHAR,LocalFolderUID,sizeof(LocalFolderUID));
	if(IsPackaged(LocalFolderUID))
	{				
		SaveMsgtoWorkQueue(LocalFolderUID,L"",IDS_PACKAGE,WORKQUEUE_DELETE,L"",L"");
	}

	// delete table FolderMD
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM FOLDERMD WHERE LOCALFOLDERUID = '%s'"),
			 LocalFolderUID);

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);

	retCode = g_localDB.ExecuteSQL(sql);

	//	Delete the folder from FOLDER table
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("DELETE FROM FOLDER WHERE FOLDERUID = %ld"),
			 entryUID);

	(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s"),GetCurrentThreadId(),sql);

	retCode = g_localDB.ExecuteSQL(sql);

	(*gpDbSyncLog)(Log::L_INFO, _T("%d:Leave DeleteFolder(%s, %d) in manually sync process"), 
		           GetCurrentThreadId(), folderLUID, entryUID);

	return true;
}


void _cdecl CDSInterface::SiteMDCallBack(DWORD dwTriggerUid, DWORD dwUid, DWORD dwNumMd, METADATA *pOutMd)
{
	(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Received SiteMDCallBack(%d, %d, %d), push to queue."), 
									GetCurrentThreadId(), dwTriggerUid, dwUid, dwNumMd);
	DSCallBackMatadata* pCallback = new DSCallBackMatadata(DSCallBackBase::SITEMD, dwTriggerUid, dwUid, dwNumMd, pOutMd);
	g_server.AddingCallbackToThread(pCallback);
}

void _cdecl CDSInterface::ApplicationMDCallBack(DWORD dwTriggerUid, DWORD dwUid, DWORD dwNumMd, METADATA *pOutMd)
{
	(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Received ApplicationMDCallBack(%d, %d, %d), push to queue."),
									GetCurrentThreadId(), dwTriggerUid, dwUid, dwNumMd);
	DSCallBackMatadata* pCallback = new DSCallBackMatadata(DSCallBackBase::APPMD, dwTriggerUid, dwUid, dwNumMd, pOutMd);
	g_server.AddingCallbackToThread(pCallback);
}

void _cdecl CDSInterface::FolderMDCallBack(DWORD dwTriggerUid, DWORD dwUid, DWORD dwNumMd, METADATA *pOutMd)
{
	(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Received FolderMDCallBack(%d, %d, %d), push to queue."), 
									GetCurrentThreadId(), dwTriggerUid, dwUid, dwNumMd);
	DSCallBackMatadata* pCallback = new DSCallBackMatadata(DSCallBackBase::FOLDERMD, dwTriggerUid, dwUid, dwNumMd, pOutMd);
	g_server.AddingCallbackToThread(pCallback);
}

void _cdecl CDSInterface::AssetMDCallBack(DWORD dwTriggerUid, DWORD dwUid, DWORD dwNumMd, METADATA *pOutMd)
{
	(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Received AssetMDCallBack(%d, %d, %d), push to queue."), 
									GetCurrentThreadId(), dwTriggerUid, dwUid, dwNumMd);
	DSCallBackMatadata* pCallback = new DSCallBackMatadata(DSCallBackBase::ASSETMD, dwTriggerUid, dwUid, dwNumMd, pOutMd);
	g_server.AddingCallbackToThread(pCallback);
}

void _cdecl CDSInterface::ElementMDCallBack(DWORD dwTriggerUid, DWORD dwUid, DWORD dwNumMd, METADATA *pOutMd)
{
	(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Received ElementMDCallBack(%d, %d, %d), push to queue."), 
									GetCurrentThreadId(), dwTriggerUid, dwUid, dwNumMd);
	DSCallBackMatadata* pCallback = new DSCallBackMatadata(DSCallBackBase::AELEMENTMD, dwTriggerUid, dwUid, dwNumMd, pOutMd);
	g_server.AddingCallbackToThread(pCallback);
}

void _cdecl CDSInterface::ClipMDCallBack(DWORD dwTriggerUid, DWORD dwUid, DWORD dwNumMd, METADATA *pOutMd)
{
	(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Received ClipMDCallBack(%d, %d, %d), push to queue."), 
									GetCurrentThreadId(), dwTriggerUid, dwUid, dwNumMd);
	DSCallBackMatadata* pCallback = new DSCallBackMatadata(DSCallBackBase::CLIPMD, dwTriggerUid, dwUid, dwNumMd, pOutMd);
	g_server.AddingCallbackToThread(pCallback);
}

void _cdecl CDSInterface::AssetStateCallBack(DWORD dwTriggerUid, WORD wType, DWORD dwUid, IDSUIDUPDATESTAMP assetStamp, WORD wState, WORD wOperation)
{
	(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Received AssetStateCallBack(%d, %d, %d, %d, %d), push to queue."), 
									GetCurrentThreadId(), dwTriggerUid, wType, dwUid, wState, wOperation);
	DSCallBackState* pCallback = new DSCallBackState(DSCallBackBase::ASSETSTATE, dwTriggerUid, dwUid, wType, wState, wOperation, assetStamp);
	g_server.AddingCallbackToThread(pCallback);
}

void _cdecl CDSInterface::ElementStateCallBack(DWORD dwTriggerUid, WORD wType, DWORD dwUid, IDSUIDUPDATESTAMP assetStamp, WORD wState, WORD wOperation)
{
	(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Received ElementStateCallBack(%d, %d, %d, %d, %d), push to queue."), 
									GetCurrentThreadId(), dwTriggerUid, wType, dwUid, wState, wOperation);
	DSCallBackState* pCallback = new DSCallBackState(DSCallBackBase::AELEMENTSTATE, dwTriggerUid, dwUid, wType, wState, wOperation, assetStamp);
	g_server.AddingCallbackToThread(pCallback);
}

void _cdecl CDSInterface::CaCallBack(DWORD dwTriggerUid, DWORD dwAssetUid, IDSUIDUPDATESTAMP assetStamp, DWORD dwNumCa, COMPLEXASSET *pOutCa)
{
	(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Received CaCallBack(%d, %d, %d), push to queue."),
									GetCurrentThreadId(), dwTriggerUid, dwAssetUid, dwNumCa);
	DSCallBackCAsset* pCallback = new DSCallBackCAsset(DSCallBackBase::CASSET, dwTriggerUid, dwAssetUid, dwNumCa, pOutCa, assetStamp);
	g_server.AddingCallbackToThread(pCallback);
}

void _cdecl CDSInterface::FolderCallBack(DWORD dwTriggerUid, ENTRY *pFolderEntry, WORD wOp)
{
	if(IDS_REPLACE == wOp && wcscmp(pFolderEntry->wszName, pFolderEntry->wszPrevious) == 0)
	{
		(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Received FolderCallBack(%d, %s, %d), the operation is IDS_REPLACE, no change on Hierarchy Name, ignore it. (Total ignored count: %d)."), 
									GetCurrentThreadId(), dwTriggerUid, pFolderEntry->wszName, wOp, m_totalIgnoredCount++);
		return;
	}

	(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Received FolderCallBack(%d, %s, %d), push to queue."), 
								GetCurrentThreadId(), dwTriggerUid, pFolderEntry->wszName, wOp);
	DSCallBackFolder* pCallback = new DSCallBackFolder(DSCallBackBase::FOLDER, dwTriggerUid, pFolderEntry, wOp);
	g_server.AddingCallbackToThread(pCallback);
}

void _cdecl CDSInterface::MDDCallBack(DWORD dwTriggerUid,WORD wType,METADATADESC *pMdd,WORD wOp)
{
	(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Received MDDCallBack(%d, %d, %d), push to queue."), 
									GetCurrentThreadId(), dwTriggerUid, wType, wOp);
	DSCallBackMDDescriptor* pCallback = new DSCallBackMDDescriptor(DSCallBackBase::MDDESCRIPTOR, dwTriggerUid, wType, wOp, pMdd);
	g_server.AddingCallbackToThread(pCallback);
}


int  CDSInterface::GetFolderRating(IN const SQLTCHAR* folderLUID)
{
	if(folderLUID == NULL)
	{
		return 0;
	}

	bool bRtn = false;
	DWORD retCode = 0;
	SQLINTEGER oldRating = 0;

	SQLTCHAR sql[1024] = _T("");

	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDValue FROM FOLDERMD WHERE MDUID = %d AND LOCALFOLDERUID = '%s'"),
		m_folderRatingMDUID, folderLUID);
	retCode = g_localDB.SelectSQL(sql,SQL_C_SSHORT,&oldRating,0);

	return oldRating;
}

int  CDSInterface::GetAssetRating(IN const SQLTCHAR* assetLUID)
{
	if(assetLUID == NULL)
	{
		return 0;
	}

	bool bRtn = false;
	DWORD retCode = 0;
	SQLINTEGER oldRating = 0;

	SQLTCHAR sql[1024] = _T("");

	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDValue FROM ASSETMD WHERE MDUID = %d AND LOCALASSETUID = '%s'"),
		m_assetRatingMDUID, assetLUID);
	retCode = g_localDB.SelectSQL(sql,SQL_C_SSHORT,&oldRating,0);

	return oldRating;
}

bool CDSInterface::GetAssetTitleSortName(IN const SQLTCHAR* assetLUID, INOUT SQLTCHAR* titleSortName, IN int len)

{
	if(assetLUID == NULL || titleSortName == NULL)
	{
		return false;
	}

	bool bRtn = false;
	DWORD retCode = 0;

	SQLTCHAR sql[1024] = _T("");

	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("SELECT MDValue FROM ASSETMD WHERE MDUID = %d AND LOCALASSETUID = N'%s'"),
		m_titleSortNameMDUID, assetLUID);
	retCode = g_localDB.SelectSQL(sql, SQL_C_TCHAR, (SQLPOINTER)titleSortName, len*sizeof(SQLTCHAR));
	if(retCode == SQL_NO_DATA)
	{
		return false;
	}
	return true;
}

// for <VB> + <RC>
bool CDSInterface::FolderRatingRef(IN const SQLTCHAR* folderLUID, IN int titleSortNameMDUID)
{
	if(folderLUID == NULL)
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("FolderRatingRef() with NULL folderLUID"));

		return false;
	}
	
	RETCODE		retCode;	//	return code

	SQLTCHAR sql[1024] = _T("");

	// invoke LAM_UPDATE_ASSET_VISIBILITY_FROMFOLDER to do rating control
	(*gpDbSyncLog)(Log::L_INFO, _T("Call LAM_UPDATE_ASSET_VISIBILITY_FROMFOLDER sp to do Rating control on folder %s ..."), folderLUID);
	
	SQLINTEGER inIndicate = SQL_NTS;
	SQLINTEGER outIndicate = 0;
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("{CALL LAM_UPDATE_ASSET_VISIBILITY_FROMFOLDER(?)}"));
	retCode = g_localDB.CallStoreProcedure(sql, 
		                                   LocalDB::INPUT, SQL_C_TCHAR, SQL_VARCHAR, wcslen(folderLUID)*sizeof(SQLTCHAR), 0, (SQLPOINTER)folderLUID, wcslen(folderLUID)*sizeof(SQLTCHAR), &inIndicate,
										   LocalDB::INOUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, NULL, 0, NULL,
										   LocalDB::INOUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, NULL, 0, NULL);
	(*gpDbSyncLog)(Log::L_INFO, _T("LAM_UPDATE_ASSET_VISIBILITY_FROMFOLDER sp execution on folder %s completed"), folderLUID);

	// invoke LAM_UPDATE_ASSET_REFERENCECOUNT_FROMFOLDER to calculate reference count
	outIndicate = 0;
	inIndicate = SQL_NTS;

	(*gpDbSyncLog)(Log::L_INFO, _T("Call LAM_UPDATE_ASSET_REFERENCECOUNT_FROMFOLDER sp to calculate reference count on folder %s with TitleSortName MDUID = %d ..."), folderLUID, titleSortNameMDUID);

	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("{CALL LAM_UPDATE_ASSET_REFERENCECOUNT_FROMFOLDER(?,?)}"));
	retCode = g_localDB.CallStoreProcedure(sql, 
		                                   LocalDB::INPUT, SQL_C_TCHAR, SQL_VARCHAR, wcslen(folderLUID)*sizeof(SQLTCHAR), 0, (SQLPOINTER)folderLUID, wcslen(folderLUID)*sizeof(SQLTCHAR), &inIndicate,
										   LocalDB::INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &titleSortNameMDUID, 0, &outIndicate,
										   LocalDB::INOUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, NULL, 0, NULL,
										   TRUE);
	(*gpDbSyncLog)(Log::L_INFO, _T("LAM_UPDATE_ASSET_REFERENCECOUNT_FROMFOLDER sp execution on folder %s with TitleSortName MDUID = %d completed"), folderLUID, titleSortNameMDUID);

	return retCode == SQL_SUCCESS ? true : false;
}

// for <RA> + [RA]
bool CDSInterface::AssetRefTwoTitles(IN const SQLTCHAR* assetLUID, IN int titleSortNameMDUID, IN const SQLTCHAR* srcTitleSortName, IN const SQLTCHAR* desTitleSortName)
{
	if(NULL == srcTitleSortName && NULL == desTitleSortName)
	{
		return true;
	}

	bool bret = false;
	bool calcBoth = true;
	
	if(srcTitleSortName != NULL && desTitleSortName != NULL && wcscmp(srcTitleSortName, desTitleSortName) != 0)
	{
		if(srcTitleSortName != NULL)
		{
			bret = AssetRefOneTitle(assetLUID, titleSortNameMDUID, srcTitleSortName);
		}
		if(desTitleSortName != NULL)
		{
			bret = AssetRefOneTitle(assetLUID, titleSortNameMDUID, desTitleSortName);
		}
	}
	else
	{
		bret = AssetRefOneTitle(assetLUID, titleSortNameMDUID, desTitleSortName);
	}

	return bret;
}

bool CDSInterface::AssetRefOneTitle(IN const SQLTCHAR* assetLUID, IN int titleSortNameMDUID, IN const SQLTCHAR* titleSortName)
{
	if(NULL == assetLUID)
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("AssetRefOneTitle() with NULL assetLUID"));

		return false;
	}
	if(NULL == titleSortName)
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("AssetRefOneTitle() with NULL titleSortName, do not calculate"));
		return false;
	}
	RETCODE		retCode;	//	return code

	SQLTCHAR sql[1024] = _T("");

	SQLINTEGER inIndicate = SQL_NTS;
	SQLINTEGER outIndicate = 0;

	// invoke LAM_UPDATE_ASSET_REFERENCECOUNT_FROMFOLDER to calculate reference count
	outIndicate = 0;
	inIndicate = SQL_NTS;

	if(titleSortName != NULL && wcscmp(titleSortName, L"") != 0)
	{
		inIndicate = SQL_NTS;
		outIndicate = 0;

		(*gpDbSyncLog)(Log::L_INFO, _T("Call LAM_UPDATE_ASSET_REFERENCECOUNT_FROMASSETVALUE sp to calculate reference count on asset TitleSortName %s with TitleSortName MDUID = %d ..."), 
							titleSortName, titleSortNameMDUID);

		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("{CALL LAM_UPDATE_ASSET_REFERENCECOUNT_FROMASSETVALUE(?,?)}"));
		retCode = g_localDB.CallStoreProcedure(sql, 
			                                   LocalDB::INPUT, SQL_C_TCHAR, SQL_VARCHAR, wcslen(titleSortName)*sizeof(SQLTCHAR), 0, (SQLPOINTER)titleSortName, wcslen(titleSortName)*sizeof(SQLTCHAR), &inIndicate,
											   LocalDB::INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &titleSortNameMDUID, 0, &outIndicate,
											   LocalDB::INOUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, NULL, 0, NULL,
											   TRUE);
		(*gpDbSyncLog)(Log::L_INFO, _T("LAM_UPDATE_ASSET_REFERENCECOUNT_FROMASSETVALUE sp execution on asset TitleSortName %s with TitleSortName MDUID = %d completed"), 
						titleSortName, titleSortNameMDUID);

	}
	else if(titleSortName != NULL && wcscmp(titleSortName, L"") == 0)
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("Call LAM_UPDATE_ASSET_REFERENCECOUNT_FROMASSETID sp to calculate reference count on asset %s with TitleSortName MDUID = %d ..."), assetLUID, titleSortNameMDUID);

		SQLSMALLINT procRet = 0;	//	return code of the stored procudure
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("{CALL LAM_UPDATE_ASSET_REFERENCECOUNT_FROMASSETID(?,?)}"));
		retCode = g_localDB.CallStoreProcedure(sql, 
											   LocalDB::INPUT, SQL_C_TCHAR, SQL_VARCHAR, wcslen(assetLUID)*sizeof(SQLTCHAR), 0, (SQLPOINTER)assetLUID, wcslen(assetLUID)*sizeof(SQLTCHAR), &inIndicate,
											   LocalDB::INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &titleSortNameMDUID, sizeof(int), &outIndicate,
											   LocalDB::INOUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, NULL, 0, NULL,
											   //LocalDB::OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &procRet, 0, &outIndicate,
											   TRUE);
		(*gpDbSyncLog)(Log::L_INFO, _T("LAM_UPDATE_ASSET_REFERENCECOUNT_FROMASSETID sp execution on asset %s with TitleSortName MDUID = %d completed, return %d"), assetLUID, titleSortNameMDUID, procRet);
	}

	return retCode == SQL_SUCCESS ? true : false;	
}


// for [VA] + <RB>
bool CDSInterface::AssetRatingRef(IN const SQLTCHAR* assetLUID, IN int titleSortNameMDUID, bool ratingctrl)
{
	if(assetLUID == NULL)
	{
		(*gpDbSyncLog)(Log::L_INFO, _T("AssetRatingRef() with NULL assetLUID"));

		return false;
	}
	
	RETCODE		retCode;	//	return code

	SQLTCHAR sql[1024] = _T("");

	SQLINTEGER inIndicate = SQL_NTS;
	SQLINTEGER outIndicate = 0;
	
	if(ratingctrl)
	{
		// invoke LAM_UPDATE_ASSET_VISIBILITY_FROMASSET to do rating control
		(*gpDbSyncLog)(Log::L_INFO, _T("Call LAM_UPDATE_ASSET_VISIBILITY_FROMASSET sp to do Rating control on asset %s ..."), assetLUID);
		
		StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("{CALL LAM_UPDATE_ASSET_VISIBILITY_FROMASSET(?)}"));
		retCode = g_localDB.CallStoreProcedure(sql, 
			                                   LocalDB::INPUT, SQL_C_TCHAR, SQL_VARCHAR, wcslen(assetLUID)*sizeof(SQLTCHAR), 0, (SQLPOINTER)assetLUID, wcslen(assetLUID)*sizeof(SQLTCHAR), &inIndicate,
											   LocalDB::INOUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, NULL, 0, NULL,
											   LocalDB::INOUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, NULL, 0, NULL);
		(*gpDbSyncLog)(Log::L_INFO, _T("LAM_UPDATE_ASSET_VISIBILITY_FROMASSET sp execution on asset %s completed"), assetLUID);
	}
	// invoke LAM_UPDATE_ASSET_REFERENCECOUNT_FROMFOLDER to calculate reference count
	outIndicate = 0;
	inIndicate = SQL_NTS;

	(*gpDbSyncLog)(Log::L_INFO, _T("Call LAM_UPDATE_ASSET_REFERENCECOUNT_FROMASSETID sp to calculate reference count on asset %s with TitleSortName MDUID = %d ..."), assetLUID, titleSortNameMDUID);

	SQLSMALLINT procRet = 0;	//	return code of the stored procudure
	StringCbPrintf(sql, 1024*sizeof(SQLTCHAR), _T("{CALL LAM_UPDATE_ASSET_REFERENCECOUNT_FROMASSETID(?,?)}"));
	retCode = g_localDB.CallStoreProcedure(sql, 
		                                   LocalDB::INPUT, SQL_C_TCHAR, SQL_VARCHAR, wcslen(assetLUID)*sizeof(SQLTCHAR), 0, (SQLPOINTER)assetLUID, wcslen(assetLUID)*sizeof(SQLTCHAR), &inIndicate,
										   LocalDB::INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &titleSortNameMDUID, sizeof(int), &outIndicate,
										   LocalDB::INOUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, NULL, 0, NULL,
										   //LocalDB::OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &procRet, 0, &outIndicate,
										   TRUE);
	(*gpDbSyncLog)(Log::L_INFO, _T("LAM_UPDATE_ASSET_REFERENCECOUNT_FROMASSETID sp execution on asset %s with TitleSortName MDUID = %d completed, return %d"), assetLUID, titleSortNameMDUID, procRet);

	return retCode == SQL_SUCCESS ? true : false;
}

bool CDSInterface::DoesSkipApplication(DWORD appUID, const CString& appName)
{
	if(0 == appUID || g_Applications.size() == 0)
		return false;
	
	for(int i=0; i<g_Applications.size(); i++)
	{
		if(wcscmp(g_Applications[i].c_str(), LPCTSTR(appName)) == 0)
		{
			return false;
		}
	}

	return true;
}

void CDSInterface::RemoveExpiredCACallback()
{
	DISORDERCALLBCK::iterator it = m_disOrderCBs.begin();
	DISORDERCALLBCK::iterator ittemp = it;
	while(it != m_disOrderCBs.end())
	{
		DSCallBackCAsset* pCallBack = (DSCallBackCAsset*)it->second;

		ittemp=it;
		it++;

		if(pCallBack->expired())
		{
			delete pCallBack;
			m_disOrderCBs.erase(ittemp);
		}
	}		
}