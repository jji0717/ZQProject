/*****************************************************************************
File Name:		DSInterface.h
Author:			Interactive ZQ 
Security:		<NVOD over iTV, Kunming>, Confidential
Description:	Definition of Class DSInterface, which is used for synchonization 
                between ITV AM database and Local database
Modification Log:
When		Version		Who			What
-----------------------------------------------------------------------------
11/21/2002	    0		W.Z.	Original Program (Created)
 2/	8/2002		1		W.Z.	Modify to utilize transaction mechanism
 3/ 2/2004      2       Z.X.    Add price definition(ITV_PRICE_STRUCT) to support
                                synchronize price definition from ITV
05/21/2006      4       KenQ    Support to take IDS callback during the full synchronization, 
								these callbacks are stored into a queue for later processing 
								by anther thread.
								So, the original thread(TriggerWorker/TriggerWorkerMgr) are
								not necessary any more, whose purpose is avoiding 
								invoking IDS API in the callback. Process callback in a seperated 
								thread cover the issue. 
*****************************************************************************/


//////////////////////////////////////////////////////////////////////
// Declaratoin of include files and definition of symbols, data types
//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ids.h"
#include "ids_interfaces.h"
#include "itv.h"
#include "SeaChange.h"
#include "DBSyncConf.h"
#include "LocalDB.h"
#include "DsCallBack.h"

#include <queue>

#define MAXNAME MAX_SQL_CHAR

class CDSInterface 
{
friend class TriggerWorkerMgr;
friend class TriggerWorker;
friend class ManualSyncMDDCallback;
friend class ManualSyncFolderCallback;

public:
	CDSInterface();		//	Construction function of class CDSInterface
	virtual ~CDSInterface();		//	Destruction function of class CDSInterface
	DWORD GetMDList(TCHAR* MDListString, DWORD buffbytes);	
	static DWORD SaveMsgtoWorkQueue(const TCHAR* LocalEntryUID,const TCHAR* LocalParentUID,DWORD EntryType,DWORD operation,const TCHAR* MDNAME,const TCHAR* Name);

protected:
	static DWORD UpdateAssetState(DWORD, WORD);	//	Internal method to update the state of an asset
	static DWORD UpdateElementState(DWORD, WORD);	//	Internal method to update the state of an element and composite XML file if the element is newly 'active'
	static int CheckZQEntryState(DWORD, WCHAR *, WORD);	//	Internal method to read ZQ entry state of given entry
	static DWORD WriteXMLFile(SQLTCHAR *, LPTSTR);	//	Internal method to write current status of an element to xml file
	static bool IsAssetUpdated(DWORD AssetUID);
	
	DWORD SaveSitesToDB(SITEINFO *, DWORD);	//	Internal method to save all sites to Local DB
	static DWORD SaveAppsToDB(void);
	static DWORD SaveAppsToDB(APPNAME *, DWORD);	//	Internal method to save all applications to Local DB
	static DWORD SaveOneHierarchy(DWORD, WORD, DWORD, const WCHAR *, DWORD,DWORD);	//	Internal method to save a hierarchy relationship to Local DB
	static DWORD SaveOneComplexAsset(DWORD, DWORD, COMPLEXASSET *);	//	Internal method to save all member relationships with an asset to Local DB
	static DWORD SaveOneComplexAsset_V2(DWORD,DWORD,COMPLEXASSET_V2*);//	Internal method to save all member relationships with an asset to Local DB
	static DWORD SaveOneFolder(DWORD, const WCHAR *);	//	Internal method to save a folder to Local DB
	static DWORD SaveOneAsset(DWORD, const WCHAR *, DWORD, BOOL&);	//	Internal method to save an asset to Local DB
	static DWORD SaveOneElement(DWORD, const WCHAR *);	//	Internal method to save an element to Local DB
	static DWORD SaveOneClip(DWORD, const WCHAR *);//	Internal method to save a clip to Local DB
	static DWORD SaveOneClipEx(CLIP*,DWORD);//Internal method to save an extra clip to local DB

	static DWORD SaveEntriesToDB(DWORD,DWORD, DWORD appuid=0, DWORD manuallySync=0);	//	Internal method to read and save all sub-entries within an entry to Local DB

	// added by Bernie.Zhao, to save all parent entries do DB, but do NOT save any sibling entries/parent's sibling/parent's parent's sibling...
	// and return the uid and type of specified entry
	static DWORD SaveParentEntriesToDB();

	static DWORD SaveElementsToDB(DWORD, BOOL bNew);	//	Internal method to read and save all members within an complex asset to Local DB
	static DWORD SaveClipsToDB(DWORD);	//	Internal method to read and save all member clips within an complex asset to Local DB
	static DWORD SaveEntryMDSelectToDB(METADATADESC *, DWORD, WORD);	//	Internal method to save all metadata selection entries of an object type to Local DB
	static DWORD SaveEntryMDSelectToDBEx(METADATADESC mdd, WORD entryType);

	static DWORD SaveObjectMdToDB(DWORD, WORD, BOOL newAsset = TRUE);	//	Internal method to read and save metadata values of an object to Local DB
	static DWORD SaveSiteMdToDB(METADATA *, DWORD, DWORD);	//	Internal method to save metadata values of a site to Local DB
	static DWORD SaveAppMdToDB(METADATA *, DWORD, DWORD);	//	Internal method to save metadata values of an application to Local DB
	static DWORD SaveFolderMdToDB(METADATA *, DWORD, DWORD);	//	Internal method to save metadata values of a folder to Local DB
	static DWORD SaveAssetMdToDB(METADATA *, DWORD, DWORD, BOOL);	//	Internal method to save metadata values of an asset to Local DB
	static DWORD SaveElementMdToDB(METADATA *, DWORD, DWORD, BOOL);	//	Internal method to save metadata values of an element to Local DB
	static DWORD SaveEntryMDDToDB(void);
	static DWORD SaveEntryMDDToDBEx(WORD entryType, const TCHAR* mddName);

	
	static DWORD SaveClipMdToDB(METADATA*,DWORD,DWORD);//	Internal method to save metadata values of a clip to Local DB

	static DWORD CDSInterface::GetNewLocalUID(int, DWORD *);	//	Internal method to get a new unique ID of designated object type
	static bool GetEntryParentType(DWORD folderUID,DWORD parentUID,DWORD* parentType); //  InterNal method to get a entry's parent Type
	void LogMd(METADATA *);  //	Internal method to log metadata values
	static DWORD PrepareMdSQL(TCHAR*, METADATA *, BOOL);	//	Internal method to prepare a SQL statement according to diffrent data types of metadata value
	static DWORD PrepareSelectionSQL(TCHAR*, WORD, SELECTENTRY *);	//	Internal method to prepare a SQL statement according to diffrent data types of selection entry

//	static DWORD WINAPI ProcessAssetDetailsThreadProc(LPVOID);	//	Thread to process the details (such as read and save metadata and members) of an asset
//	static DWORD WINAPI ProcessFolderDetailsThreadProc(LPVOID);	//	Thread to process the details (such as read and save metadata and members) of an folder
	
	

	bool IsNeedInQueue(DWORD EntryType,WCHAR* MDName);
	static BOOL IsPackaged(TCHAR* folderLUID);
	static int VerifyMD(METADATA* pMd,TCHAR* OldValue);
	static DWORD SetACFlag(DWORD AssetUID);
	DWORD DeleteUnusedObj(void);
	static void NetBreakCallBack(IDSSESS* pIdsSession);

	static int GetEntryHUIDByName(const IN WCHAR* entryName, OUT DWORD& entryHUID, OUT DWORD& entryType);
	static bool AdjustRoot(IN const WCHAR* ori_root, OUT CString& outString);
	static bool validTimeWindow(IN WCHAR* mdname, IN WCHAR* time1, IN WCHAR* time2);
	static bool isMultipleEntry(IN DWORD index, IN METADATA* pMd);
	
	// function for KDDI RatingControl and Reference count 
	static int m_titleSortNameMDUID;
	static int m_folderRatingMDUID;
	static int m_assetRatingMDUID;

	static int  GetFolderRating(IN const SQLTCHAR* folderLUID);
	static int  GetAssetRating(IN const SQLTCHAR* assetLUID);
	static bool GetAssetTitleSortName(IN const SQLTCHAR* assetLUID, INOUT SQLTCHAR* titleSortName, IN int len);
	
	// update the spcified folder's sub folder/asset's ZQENTRYSTATE to be 1
	static bool UpdateFolderSubNodeZQEntryState(DWORD huid);
	/// check whether the specified object is existed or not in ITV
	static bool CheckIDSObjExistence(DWORD entryUID, DWORD entryType, ITVSTATUS& errCode);
	/// Process the unavaliable hierarchy records whose ZQ_ENTRYSTATE=1 in Mannal sync
	static bool RemoveHierarchyForManaulSync(DWORD folderHUID, ITVSTATUS& errCode);
	/// Delete an asset from LAM
	static bool DeleteAsset(IN const SQLTCHAR* assetLUID, IN DWORD entryUID, IN const SQLTCHAR* parentHUID, ITVSTATUS& errCode);
	/// Delete a folder from LAM
	static bool DeleteFolder(IN const SQLTCHAR* folderLUID, DWORD entryUID);


	// for the details of Vx and Rx, please see the comment on the head of DSInterface.cpp
	// for <VB> + <RC>
	static bool FolderRatingRef(IN const SQLTCHAR* folderLUID, IN int titleSortNameMDUID);
	// for <RA> + [RA]
	static bool AssetRefTwoTitles(IN const SQLTCHAR* assetLUID, IN int titleSortNameMDUID, IN const SQLTCHAR* srcTitleSortName, IN const SQLTCHAR* desTitleSortName);
	/// for [VA] + <RB>
	static bool AssetRatingRef(IN const SQLTCHAR* assetLUID, IN int titleSortNameMDUID, bool ratingctrl = true );

	static bool AssetRefOneTitle(IN const SQLTCHAR* assetLUID, IN int titleSortNameMDUID, IN const SQLTCHAR* titleSortName);

	static bool DoesSkipApplication(DWORD appUID, const CString& appName);

private:
	
	/*
	 * Struct definition using in byteUserData of SELECTENTRY struct which defined in ids API.
	 */
	//  PRICE CATEGORY
	//  Asset on ITV dosen't have price but belong to a category. This category will define the price. Each
	//  category is defined as a select entry and its price and tax was stored in user data.
	typedef struct _IZQ_PRICE_STRUCT {
		unsigned char version[8]; // Not sure about first 8 bytes, first 2 bytes wa version, not any document about 6 bytes following.
		double price;             // Price value of a category
		double tax;               // Tax value of a category
	} IZQ_PRICE_STRUCT;

protected:
	static ITVVERSION		m_itvver;	//	Version of ITV
	static IDSSESS			m_idssess;	//	Session of ITV connection
	static ITVSTATUSBLOCK	m_itvsta;	//	Status block of ITV connection

	DWORD			m_dwSiteTriggerUid;	//	UID of site trigger on ITV
	DWORD			m_dwApplicationTriggerUid;	//	UID of application trigger on ITV
	DWORD			m_dwFolderTriggerUid;	//	UID of folder trigger on ITV
	DWORD			m_dwAssetTriggerUid;	//	UID of asset trigger on ITV
	DWORD			m_dwElementTriggerUid;	//	UID of element trigger on ITV
	DWORD			m_dwClipTriggerUid;	// UID of clip trigger on ITV
	DWORD			m_dwCaTriggerUid;	//	UID of complex-asset trigger on ITV
	DWORD			m_dwElementStTriggerUid;	//	UID of element state trigger on ITV
	DWORD			m_dwAssetStTriggerUid;	//	UID of asset state trigger on ITV
	DWORD			m_dwMddTriggerUid;	//	UID of metadata trigger on ITV
private:	

	static ZQ::common::Mutex m_AssetMDTableLock;
	static ZQ::common::Mutex m_WQLock;
	static WORKQUEUE	m_WorkQueue;
	static bool			m_bIsActive;

	// added by Bernie, 2006-02-05
	// These 2 locks are added for multi-thread safety.  When more than one thread
	// is insert/update the [AtomicElement]/[Asset] table, multiple entries of the same asset
	// would be inserted by chance.  So we lock the tables before insert into them.
	static ZQ::common::Mutex m_AssetTableLock;
	static ZQ::common::Mutex m_AtomicElementTableLock;
	
	// Added by Ken Qian, 2007-1-10
	// Move the variable from GetNewLocalUID() to be a member
	// to avoid each time to create a new time consumed connection.
	static LocalDB m_transactionDB;

//	static CList<DWORD, DWORD>		m_busyAssets;	//	List of assets being processed by threads
//	static DWORD m_ThreadLock;

	// Add by KenQ on 2007-01-20, add a map to remeber the disordered callback
	typedef std::map<DWORD, DSCallBackCAsset*> DISORDERCALLBCK;   // <AssetUID, CallbackObject>
	static DISORDERCALLBCK m_disOrderCBs;

	static bool m_isFullSyncing;

	static DWORD m_totalIgnoredCount;   // count to remember how many relace op FolderCallback igored till now. 

	// since new uploaded asset, IDS firstly give element/asset metadata callback before FolderCallback, 
	// in metadata callback processing, the asset always does not exist, here the map is to remember the id
	// to avoid search data anymore
	typedef std::map<DWORD, DWORD> IDMAP;
	static IDMAP  m_notFoundAssetId;
	static IDMAP  m_notFoundElementId;

	typedef std::map<CString, DWORD> ActiveMetadata;
	static ActiveMetadata m_activeElementMD;

	typedef std::map<CString, CString> ASSETLASTPARENTHUID;
	static ASSETLASTPARENTHUID m_assetLastParentHUID;

protected:
	DWORD InitIDS();	//	External method to initialize an IDS connection with ITV
	DWORD UnInitIDS(); 	//	External method to uninitialize an IDS connection with ITV

public:
	bool  Initialize();    // IDS init and Trigger Register 
	void  Uninitialize();  // IDS Trigger Un-Register

	bool  CheckSyncFoldersExistence();	// check whether all the registered sync folders exist or not
	DWORD CheckConn(void);

	DWORD InitDB();     // Initialize the database connection
	DWORD UninitDB();   // Uninitialize the database connection

	DWORD RegisterTrigger();	//	External method to register a callback trigger onto ITV
	DWORD DeregiserTrigger();	//	External method to deregister a callback trigger onto ITV
	DWORD UpdateAll();	//	External method to perform a throughout synchronization between ITV AM database and Local DB
	DWORD ClearAll();	//	External method to perform a throughout clear in Local DB

	static void _cdecl SiteMDCallBack(DWORD, DWORD, DWORD, METADATA *);	//	External method to be called by ITV when a metadata of site is changed
	static void _cdecl ApplicationMDCallBack(DWORD, DWORD, DWORD, METADATA *);	//	External method to be called by ITV when a metadata of application is changed
	static void _cdecl FolderMDCallBack(DWORD, DWORD, DWORD, METADATA *);	//	External method to be called by ITV when a metadata of folder is changed
	static void _cdecl AssetMDCallBack(DWORD, DWORD, DWORD, METADATA *);	//	External method to be called by ITV when a metadata of asset is changed
	static void _cdecl ElementMDCallBack(DWORD, DWORD, DWORD, METADATA *);	//	External method to be called by ITV when a metadata of element is changed
	static void _cdecl ClipMDCallBack(DWORD, DWORD, DWORD, METADATA *);	//	External method to be called by ITV when a metadata of element is changed

	static void _cdecl AssetStateCallBack(DWORD, WORD, DWORD, IDSUIDUPDATESTAMP,WORD, WORD);	//	External method to be called by ITV when state of an asset is changed
	static void _cdecl ElementStateCallBack(DWORD, WORD, DWORD, IDSUIDUPDATESTAMP, WORD, WORD);	//	External method to be called by ITV when state of an element is changed
	
	static void _cdecl CaCallBack(DWORD, DWORD, IDSUIDUPDATESTAMP ,DWORD ,COMPLEXASSET *);	//	External method to be called by ITV when member relationship of an complex asset is changed
	static void _cdecl FolderCallBack(DWORD, ENTRY *, WORD);	//	External method to be called by ITV when hierarchy relationship of a folder is changed
	static void _cdecl MDDCallBack(DWORD dwTriggerUid,WORD wType,METADATADESC *pMdd,WORD wOp);

	static void SiteMDCallBackProcess(DWORD, DWORD, DWORD, METADATA *);	//	External method to be called by ITV when a metadata of site is changed
	static void ApplicationMDCallBackProcess(DWORD, DWORD, DWORD, METADATA *);	//	External method to be called by ITV when a metadata of application is changed
	static void FolderMDCallBackProcess(DWORD, DWORD, DWORD, METADATA *);	//	External method to be called by ITV when a metadata of folder is changed
	static void AssetMDCallBackProcess(DWORD, DWORD, DWORD, METADATA *);	//	External method to be called by ITV when a metadata of asset is changed
	static void ElementMDCallBackProcess(DWORD, DWORD, DWORD, METADATA *);	//	External method to be called by ITV when a metadata of element is changed
	static void ClipMDCallBackProcess(DWORD, DWORD, DWORD, METADATA *);	//	External method to be called by ITV when a metadata of element is changed

	static void AssetStateCallBackProcess(DWORD, WORD, DWORD, IDSUIDUPDATESTAMP,WORD, WORD);	//	External method to be called by ITV when state of an asset is changed
	static void ElementStateCallBackProcess(DWORD, WORD, DWORD, IDSUIDUPDATESTAMP, WORD, WORD);	//	External method to be called by ITV when state of an element is changed
	
	static void CaCallBackProcess(DWORD, DWORD, IDSUIDUPDATESTAMP ,DWORD ,COMPLEXASSET *);	//	External method to be called by ITV when member relationship of an complex asset is changed
	static void FolderCallBackProcess(DWORD, ENTRY *, WORD);	//	External method to be called by ITV when hierarchy relationship of a folder is changed
	static void MDDCallBackProcess(DWORD dwTriggerUid,WORD wType,METADATADESC *pMdd,WORD wOp);


	static void RemoveExpiredCACallback();
};

