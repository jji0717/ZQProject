

#include <iostream>
#include <fstream>
#include <string>

#include "isa.h"
#include "reporter.h"
#include "itvmessages.h"
#include "CfgPkg.h"
#include "asset_impl.h"
#include "metadata_impl.h"
#include "UtcTime.h"

#include "ContentComponentC.h"
#include "seacContentComponentC.h"
#include "AssetComponentC.h"

#include "ContentRoutine.h"

//create asset fail will delete content, but repair will not
//#define FAIL_NOT_DELETE_CONTENT

using namespace std;
using namespace StreamModule;
using namespace ContentModule;
using namespace AssetModule;
using namespace MetadataModule;
using namespace SessionModule;

#define		DEF_PROVIDER		L"SeaChange"

#define DEFAULT_CONTENTSTORE_ENDPOINT L"SeaChangeContentStore:33333"

#define		DEFAULT_CONTENTSTORE_OBJKEY				L"14010f004e555000000016010000000100000000436f6e74656e7453746f7265000000000001000000436f6e74656e7453746f7265"

// RepositoryId for Asset object
const char * szReposId_metadatalist   = "IDL:isa.twc.com/MetadataModule/seacMetadataList:1.0";
const char * szReposId_asset          = "IDL:isa.twc.com/AssetModule/seacAsset:1.0";
const char * szReposId_assetfactory   = "IDL:isa.twc.com/AssetModule/AssetFactory:1.0";

// RepositoryId for Session ojbect
const char * szReposId_session        = "IDL:isa.twc.com/SessionModule/Session:1.0";
const char * szReposId_sessiongateway = "IDL:isa.twc.com/SessionModule/SessionGateway:1.0";

CORBA::Object_var                       _refObj;
ContentModule::seacContentStore_var     _store         = ContentModule::seacContentStore::_nil();
AssetModule::seacAsset_var              _movie_asset   = AssetModule::seacAsset::_nil();
MetadataList_impl * _MdServant0; 
MetadataList_impl * _MdServant1; 


char                            _szContentStoreNsEntry   [256];
WCHAR                           _wszContentStoreEndPoint  [256];
WCHAR                           _wszContentStoreObjKey  [2048];

WCHAR                           _wszItvImportPath[256];
WCHAR							_wszTemplatePath[256];

WCHAR			_wszProvider[256];			//use for root directory in DS
char			_szProvider[256];

char                            _szProduct  [32];
char                            _szMdSeperator[32];


HANDLE                          _hStartEvent = NULL; // for sync between helper thread and the main thread

Mutex							_hAccess;


void startHelperInterface(LPVOID lpParam);
bool findContentStore();



bool contentInit()
{
     if(!getConfig(_wszORBEndpoint, L"ORBEndpoint", sizeof(_wszORBEndpoint),
                   true, true, true,  false, L"", ISA_CONTENT_OPERATION_ERR)
     || !getConfig(_wszNameService, L"NameService", sizeof(_wszNameService),
                   true, true, true,  false,           DEFAULT_NAME_SERVICE, ISA_CONTENT_OPERATION_ERR)
      || !getConfig(_wszContentStoreEndPoint, L"ContentStoreEndPoint", sizeof(_wszContentStoreEndPoint),
                    true, true, true,     false, DEFAULT_CONTENTSTORE_ENDPOINT, ISA_CONTENT_START_ERR) 
      || !getConfig(_wszContentStoreObjKey, L"ContentStoreObjKey", sizeof(_wszContentStoreObjKey),
                    true, true, true,     false, DEFAULT_CONTENTSTORE_OBJKEY, ISA_CONTENT_START_ERR) 
     || !getConfig(&_dwTaoThreadPool, L"TaoThreadPool", 
                   true, true, true,  false,                             10, ISA_CONTENT_OPERATION_ERR)
//     || !getConfig(&_dwCorbaTimeout, L"CorbaTimeout", 
//                   true, true, true,  false,                           5000, ISA_CONTENT_OPERATION_ERR)
     || !getConfig(_wszItvImportPath, _T("ItvImportPath"),  sizeof(_wszItvImportPath),
				   true, true, true, true, _T(""), ISA_CONTENT_OPERATION_ERR)
      || !getConfig(_wszProvider, _T("Provider"), sizeof(_wszProvider), true, true, true, false,
                    DEF_PROVIDER, ISA_CONTENT_OPERATION_ERR))
    {
        return false;
    }

	WideCharToMultiByte(CP_ACP, 0, _wszProvider, -1, _szProvider, sizeof(_szProvider), 0, 0);
	
	//
	// get template path for itv file write
	//
	{
		 char * pszTemp = getenv("TEMP");
		 if (pszTemp == NULL)
		 {
			 wcscpy(_wszTemplatePath, L"C:\\TEMP");
		 }
		 else
		 {
			 str2wstr(pszTemp, _wszTemplatePath, sizeof(_wszTemplatePath));
			 wcscat(_wszTemplatePath, L"\\isa_template");
		 }
		 
		 LogMsg( L"Template directory : %s", _wszTemplatePath);
		 
		 validatePath(_wszTemplatePath);
		 WCHAR wszDir [256];
		 
		 // %_wszTemplatePath%\\import
		 wsprintf (wszDir,  L"%s\\import", _wszTemplatePath);
		 validatePath(wszDir);  // SPR 3377
		 
		 // %_wszTemplatePath%\\delete
		 wsprintf (wszDir,  L"%s\\delete", _wszTemplatePath);
		 validatePath(wszDir); // SPR 3377	
	}

	
	//
	// orb init
	//
	{
		wstr2str( _wszContentStoreEndPoint, _szContentStoreNsEntry, 
				  sizeof(_szContentStoreNsEntry) );

		if (!_wszORBEndpoint[0])
		{
			char szHost[256];
			gethostname(szHost, sizeof(szHost));

			wsprintf(_wszORBEndpoint, L"iiop://%S:%d", szHost, 0);
			LogMsg(L"ORBEndpoint=%s", _wszORBEndpoint);
		}    

		try
		{
			initOrb();
		}
		catch(CORBA::Exception& ex)
		{
			LogMsg(REPORT_SEVERE, L"Caught exception while Orb init: %S", ex._rep_id());
			return false;
		}		
	}


    _hStartEvent = ::CreateEvent( NULL,     // no security attributes
                                  FALSE,    // auto-reset event
                                  FALSE,    // initial state is unsignaled
                                  NULL      // create without name
                                ); 

	DWORD dw1 = GetTickCount();

    DWORD dwThreadId = 0;
    HANDLE handle = ::CreateThread(NULL, 4 * 4096,
        (LPTHREAD_START_ROUTINE) startHelperInterface, (LPVOID) NULL, 0, &dwThreadId);
    if (NULL == handle)
    {
        LogMsg(REPORT_SEVERE, L"Fail to create Asset thread: %S", getErrMsg().c_str());
        return false;
    }
    ::CloseHandle(handle);

    DWORD dwStatus = ::WaitForSingleObject(_hStartEvent, 10000);

    switch (dwStatus)
    {
    case WAIT_TIMEOUT:
        LogMsg(REPORT_SEVERE, L"Fail to start helper thread in 10000 ms. Bailing out...");
        return false;

    case WAIT_OBJECT_0:
        LogMsg(L"Helper thread is ready, spent %d ms...", GetTickCount()-dw1);
		break;
	default:
		LogMsg(REPORT_SEVERE, L"Fail to WaitForSingleObject StartEvent");
        return false;		
    }

    strcpy(_szMdSeperator, "::");

    // some destructor mal-funtion if exit the program right after the it starts,
    // following line can fix it
    //
#if 1
    if (!findContentStore())
	{
		LogMsg(REPORT_SEVERE, L"Fail to connect with isaContent");
		// not to quit immediately
        //return false;		
	}
#endif

	LogMsg(L"AssetGear service is ready...");

	return true;
}

void contentClose()
{
		
}

bool findContentStore()
{
    if (! CORBA::is_nil(_store)) // invoke this function just once
        return true;

	DWORD dw1 = GetTickCount();

	static int nOutput = 0;

	char szObjIor[2048];

    FillCorbalocUrl(szObjIor, sizeof(szObjIor), 
		_wszContentStoreEndPoint, _wszContentStoreObjKey);

	DWORD dw2 = GetTickCount();
    if (CORBA::is_nil(_store))
    {
        CORBA::Object_var refObj;		
        try {
            refObj = _orb -> string_to_object(szObjIor);
    
			_store = ContentModule::seacContentStore::_narrow(refObj);
        } 
        catch (CORBA::TRANSIENT &)
        {
			if (nOutput < 3)
			{
				LogEvent( REPORT_SEVERE,  ISA_CONTENT_OPERATION_ERR,
						  L"isaContent service is not available" );    
				nOutput ++;
			}
			return false;
        }
        catch (CORBA::Exception & ex)
        {
			if (nOutput < 3)
			{
				LogCorbaException (REPORT_SEVERE, ex, L" when narrow the reference of isaContent");            
				nOutput ++;
			}
			return false;
        }
		catch(...)
		{
			if (nOutput < 3)
			{
				LogMsg(L"Caught exception while find ContentStore");
				nOutput ++;
			}
			return false;
		}


        if (CORBA::is_nil(_store))
        {
			if (nOutput < 3)
			{
				LogEvent( REPORT_SEVERE,  ISA_CONTENT_OPERATION_ERR,
						  L"Fail to narrow the reference of seacContentStore" );            
				nOutput ++;
			}
			return false;
        }

		LogMsg(L"isaContent connected.");
    }	
	nOutput = 0;

    try 
	{
        CORBA::String_var store_name = _store -> name();
    }
    catch (const CORBA::TRANSIENT &)
    {
        LogMsg(REPORT_SEVERE, L"The ContentStore is not available");
        return false;
    }
    catch (const CORBA::Exception & ex)
    {
        LogMsg(REPORT_SEVERE, L"Caught exception in _store->name() : %S", ex._rep_id());
        return false;
    }
	
	LogMsg(L"Find ContentStore spent %d ms, isaContent connected", GetTickCount() - dw2);
	
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void startHelperInterface(LPVOID lpParam)
{
    CORBA::Object_var poaObj = _orb -> resolve_initial_references("RootPOA");
    PortableServer::POA_var root_poa = PortableServer::POA::_narrow(poaObj.in());

    ///////////////////////////////////////////////////////////////////////////
	// Get POA manager
	PortableServer::POAManager_var manager = root_poa -> the_POAManager();

    // Policy List for Asset
	CORBA::PolicyList policy_list;
	policy_list.length(2);

    // LifespanPolitcy := PERSISTENT  
	PortableServer::LifespanPolicy_var lifespan = 
        root_poa -> create_lifespan_policy(PortableServer::PERSISTENT);

	// IdAssignmentPolicy := USER_ID 
	PortableServer::IdAssignmentPolicy_var assign = 
        root_poa -> create_id_assignment_policy(PortableServer::USER_ID);
	
	// LifespanPolitcy := PERSISTENT  
	policy_list[0] = PortableServer::LifespanPolicy::_duplicate(lifespan);

	// IdAssignmentPolicy := USER_ID 
	policy_list[1] = PortableServer::IdAssignmentPolicy::_duplicate(assign);

    // Other policy values are derived by default:
    //
    // ServantRetentionPolicy   := RETAIN 
	// ImplicitActivationPolicy := NO_IMPLICIT_ACTIVATION 
	// IdUniquenessPolicy       := UNIQUE_ID 
	// RequestProcessingPolicy  := USE_ACTIVE_OBJECT_MAP_ONLY 
	// ThreadPolicy             := ORB_CTRL_MODEL 

    ///////////////////////////////////////////////////////////////////////////
	// Create the Asset POA
	PortableServer::POA_var asset_poa 
        = root_poa -> create_POA("Asset", manager, policy_list);

    //
	// Create MetadataList servant for Asset
    //
    _MdServant0 = new MetadataList_impl(NULL, 0);

	// Create our Asset OjbectId
	PortableServer::ObjectId_var oid_md0 
        = PortableServer::string_to_ObjectId("AssetMetadataList");
	CORBA::Object_var obj_md0 =
        asset_poa -> create_reference_with_id(oid_md0, szReposId_metadatalist);
	
    // Activate SeaChange MetadataList
    asset_poa -> activate_object_with_id(oid_md0, _MdServant0);

	// set Asset ref in Content servant
	MetadataModule::seacMetadataList_var mdRef0;
    try 
	{
        mdRef0 = MetadataModule::seacMetadataList::_narrow(obj_md0);
    }
	catch (CORBA::Exception & ex)
    {
		LogMsg(REPORT_SEVERE, L"Caught exception in seacMetadataList::_narrow: %S ", ex._rep_id());        
    }
    CORBA::String_var mdIor0 = _orb -> object_to_string(mdRef0);


    ///////////////////////////////////////////////////////////////////////////
	// Create MetadataList servant 
    //
	_MdServant1 = new MetadataList_impl(NULL, 0);

	// Create our Asset OjbectId
	PortableServer::ObjectId_var oid_md1 
        = PortableServer::string_to_ObjectId("MoviewMetadataList");
	CORBA::Object_var obj_md1 =
        asset_poa -> create_reference_with_id(oid_md1, szReposId_metadatalist);
	
    // Activate SeaChange MetadataList
    asset_poa -> activate_object_with_id(oid_md1, _MdServant1);

	// set Asset ref in Content servant
	MetadataModule::seacMetadataList_var mdRef1;
    try 
	{
        mdRef1 = MetadataModule::seacMetadataList::_narrow(obj_md1);
    }
	catch (CORBA::Exception & ex)
    {
        LogMsg(REPORT_SEVERE, L"Caught exception in seacMetadataList::_narrow: %S ", ex._rep_id());        
    }
    CORBA::String_var mdIor1 = _orb -> object_to_string(mdRef1);


    //
    // Create Asset servant
    //
    Asset_impl * asset_servant = 
        new Asset_impl("Asset", AssetModule::seacAsset::_nil(), mdRef0);

	// Create our Asset OjbectId
	PortableServer::ObjectId_var oid_asset = PortableServer::string_to_ObjectId("Asset");
	CORBA::Object_var obj_asset =
        asset_poa -> create_reference_with_id(oid_asset, szReposId_asset);
	
    // Activate SeaChange Asset
    asset_poa -> activate_object_with_id(oid_asset, asset_servant);

	// set Asset ref in Content servant
	AssetModule::seacAsset_var assetRef;
    try 
	{ 
        assetRef = AssetModule::seacAsset::_narrow(obj_asset);
    }
	catch (CORBA::Exception & ex)
    {
        LogMsg(REPORT_SEVERE, L"Caught exception in seacAsset::_narrow: %S ", ex._rep_id());        
    }

    CORBA::String_var assetIor = _orb -> object_to_string(assetRef);


    //
    // Create Moview Asset servant
    //
	Asset_impl * asset_servant1 = new Asset_impl("MovieAsset", assetRef, mdRef1);

	// Create our Asset OjbectId
	PortableServer::ObjectId_var oid_asset1 
        = PortableServer::string_to_ObjectId("MovieAsset");
	CORBA::Object_var obj_asset1 =
        asset_poa -> create_reference_with_id(oid_asset1, szReposId_asset);
	
    // Activate SeaChange Asset
    asset_poa -> activate_object_with_id(oid_asset1, asset_servant1);

    try 
	{
        _movie_asset = AssetModule::seacAsset::_narrow(obj_asset1);
    }
	catch (CORBA::Exception & ex)
    {
        LogMsg(REPORT_SEVERE, L"Caught exception in seacAsset::_narrow: %S ", ex._rep_id());        
    }

    CORBA::String_var assetIor1 = _orb -> object_to_string(_movie_asset);

    // Destroy policy objects
    //
    lifespan -> destroy();
    assign -> destroy();
 
    // Start rolling
    manager -> activate();
	LogMsg( L"The SeaChange Asset interface is Ready.." );

    // Give main thread the go-ahead
    ::SetEvent(_hStartEvent);

    //_orb -> run();
    if (_dwTaoThreadPool == 0) // thread-per-connection
        _orb -> run();
    else
    {
        ThreadWorker worker (_orb.in ());
        if (worker.activate (THR_NEW_LWP | THR_JOINABLE, _dwTaoThreadPool) != 0)
        {
            LogMsg( L"Cannot activate client threads" );
            return;
        }

        worker.thr_mgr() -> wait();
    }
}

void deleteContent(const char* szObjectName)
{
    try
    {
        if (! findContentStore() )
            return;

        ServerModule::ServantBase_var refBase = _store -> find(szObjectName);
 
        ContentModule::Content_var servant = ContentModule::Content::_narrow(refBase);

        servant -> destroy();
    }
    catch(ServerModule::NameNotFound&)
    {
        return;
    }
    catch(ServerModule::UnspecifiedException& )
    {
        return;
    }
    catch(const CORBA::Exception& ) 
    {
        return;
    }
    catch(...) 
    {
        return;
    }
}

bool createContent(const char* szObjectName,
				   const char* szAssetName,
				   const char* szStartTime,
				   const char* szEndTime,
				   int nBitrate,
				   const char* szProvider,
				   const char* szProviderId,
				   const char* szProviderAssetId,
				   int nPriority,
				   int& dwAssetID, 
				   int& dwAeID, 
				   char* szPushUrl,
				   int  nPushUrlSize,
				   char* szErrMsg,
				   int  nErrMsgSize,
				   const char* szActiveDate/* = "01-Dec-2000"*/)
{
	//
	// create object
	//
	ServerModule::ServantBase_var refBase;
    try
    {
        if (! findContentStore() )
		{
			strncpy(szErrMsg, "isaContent not available", nPushUrlSize);
			return false;
		}

        refBase = _store -> createServant(szObjectName);
    }
    catch(ServerModule::DuplicateServant &)
    {
		sprintf(szErrMsg, "Object name %s already exist", szObjectName);
        return false;
    }
    catch(ServerModule::UnspecifiedException & ex)
    {
        CORBA::String_var msg = ex.message;
		sprintf(szErrMsg, "Caught UnspecifiedException : %s", (const char *) msg);        
        return false;
    }
    catch(const CORBA::Exception& ex) 
    {
		sprintf(szErrMsg, "Caught %s", ex._rep_id());
        return false;
    }

	//
	// provision
	//
	char szUrl[256];
	sprintf(szUrl, "Protocol=FTP::StartTime=%s::StopTime=%s::BitRate=%d::PID=%s::PAID=%s::Provider=%s",
		(szStartTime)?szStartTime:"", (szEndTime)?szEndTime:"", nBitrate, szProviderId, szProviderAssetId, szProvider);

    char szMdProvider[256];
    char szMdProduct[256];
    char szMdLicensing_Window_Start[256];
    char szMdTitle[256];
    char szMdAsset_Name[256];
    char szMdType[256];
	char szMdPropagationPriority[256];
	char szMdAsset_Class[256];
	char szPriority[20];

    sprintf(szMdLicensing_Window_Start, "MOD%sLicensing_Window_Start", _szMdSeperator);
    sprintf(szMdProvider,               "AMS%sProvider",               _szMdSeperator);
    sprintf(szMdProduct,                "AMS%sProduct",                _szMdSeperator);
    sprintf(szMdAsset_Name,             "AMS%sAsset_Name",             _szMdSeperator);
    sprintf(szMdTitle,                  "MOD%sTitle",                  _szMdSeperator);
    sprintf(szMdType,                   "MOD%sType",                   _szMdSeperator);
	sprintf(szMdPropagationPriority,	"AMS%sPropagation_Priority",	_szMdSeperator);
	sprintf(szMdAsset_Class,			"AMS%sAsset_Class",				_szMdSeperator);
	
	sprintf(szPriority, "%d", nPriority);

	AutoMutex op(_hAccess);

    try
    {
        _movie_asset -> resetParentMd();
        _movie_asset -> resetMd();

        _movie_asset -> setParentMd (szMdLicensing_Window_Start, szActiveDate);
        _movie_asset -> setParentMd (szMdProvider, szProvider);
        _movie_asset -> setParentMd (szMdProduct,  "MOD");	//just for asset name
        _movie_asset -> setParentMd (szMdTitle,    "movie");//just for asset name
		
		_movie_asset -> setParentMd (szMdPropagationPriority, szPriority);
		_movie_asset -> setParentMd (szMdAsset_Class, "");

        _movie_asset -> setMd       (szMdAsset_Name, szAssetName);
        _movie_asset -> setMd       (szMdType,     "movie");
    }
    catch(const CORBA::Exception& ex) 
    {
		sprintf(szErrMsg, "Caught %s in movie_asset set Md", ex._rep_id()); 
        return false;
    }

    CORBA::String_var pushUrl;
	ContentModule::Content_var servant=ContentModule::Content::_nil();
    try
    {
        servant = ContentModule::Content::_narrow(refBase);
        servant -> provisionForPush(szUrl, ServerModule::as_InService, _movie_asset, pushUrl );
    }
    catch(ServerModule::ProvisioningFailed & ex)
    {
        CORBA::String_var msg = ex.message;
        sprintf(szErrMsg, "Caught ProvisioningFailed : %s in provisionForPush()", (const char*) msg);

#ifndef FAIL_NOT_DELETE_CONTENT
		if (!CORBA::is_nil(servant))
		{
			try
			{
				servant->destroy();
			}
			catch(const CORBA::Exception&) 
			{
			}	
		}
#endif

        return false;
    }
    catch(ServerModule::UnspecifiedException & ex)
    {
        CORBA::String_var msg = ex.message;
        sprintf(szErrMsg, "Caught UnspecifiedException : %s in provisionForPush()", (const char *) msg);

#ifndef FAIL_NOT_DELETE_CONTENT
		if (!CORBA::is_nil(servant))
		{
			try
			{
				servant->destroy();
			}
			catch(const CORBA::Exception&) 
			{
			}	
		}
#endif
        return false;
    }
    catch(const CORBA::Exception& ex) 
    {
		sprintf(szErrMsg, "Caught %s in provisionForPush()", ex._rep_id());

#ifndef FAIL_NOT_DELETE_CONTENT
		if (!CORBA::is_nil(servant))
		{
			try
			{
				servant->destroy();
			}
			catch(const CORBA::Exception&) 
			{
			}	
		}
#endif
        return false;
    }

	//
	// parse the asset id and remove it from the url
	//
	{
		const char* pPtr = strstr((const char*) pushUrl, "?");
		if (!pPtr)
		{
			sprintf(szErrMsg, "No asset id information in url, maybe the registry setting [EnableAssetGear] of isaContent not enabled");

#ifndef FAIL_NOT_DELETE_CONTENT
			if (!CORBA::is_nil(servant))
			{
				try
				{
					servant->destroy();
				}
				catch(const CORBA::Exception&) 
				{
				}	
			}
#endif

			return false;
		}

		//set url
		int nCopyLen = (pPtr - (const char*) pushUrl);
		strncpy(szPushUrl, (const char*) pushUrl, nCopyLen);
		szPushUrl[nCopyLen]='\0';

		// get asset id
		const char* pPtr2 = strstr(pPtr, "=");
		if (!pPtr2)
		{
			sprintf(szErrMsg, "No asset id information in url, url error");

#ifndef FAIL_NOT_DELETE_CONTENT
			if (!CORBA::is_nil(servant))
			{
				try
				{
					servant->destroy();
				}
				catch(const CORBA::Exception&) 
				{
				}	
			}
#endif
			return false;
		}
		pPtr2++;

		dwAssetID = strtol(pPtr2, NULL, 16);
		
		//ae
		{
			const char* p = szPushUrl + nCopyLen - 1;
			while(p>szPushUrl&&*p!='/')p--;
			p++;
			dwAeID = strtol(p, NULL, 16);
		}
	}
	
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// need to add string "RETRY=1" to SrcUrl, this is the flag for retry
// add value ActiveDate=%s to SrcUrl for set ActiveDate for retry
//

bool repairContent(const char* szObjectName,
				   const char* szAssetName,
				   const char* szStartTime,
				   const char* szEndTime,
				   int nBitrate,
				   const char* szProvider,
				   int nPriority,
				   int& dwAssetID, 
				   int& dwAeID, 
				   char* szPushUrl,
				   int  nPushUrlSize,
				   char* szErrMsg,
				   int  nErrMsgSize,
				   bool& bTryCreateContent,
				   const char* szActiveDate /* = "01-Dec-2000"*/)
{
	bTryCreateContent = false;
	//
	// create object
	//
	ServerModule::ServantBase_var refBase;
    try
    {
        if (! findContentStore() )
		{
			strncpy(szErrMsg, "isaContent not available", nPushUrlSize);
			return false;
		}

        refBase = _store -> find(szObjectName);
    }
    catch(ServerModule::NameNotFound&)
    {
		sprintf(szErrMsg, "The requested Content object %s doesn't exist", szObjectName);
		//////////////////////////////////////////////////////////////////////////
		// not exist, so can try create content instead
		bTryCreateContent = true;
        return false;
    }
    catch(const CORBA::Exception& ex) 
    {
		sprintf(szErrMsg, "Caught %s", ex._rep_id());
        return false;
    }

	//
	// provision
	//
	char szUrl[512];
	sprintf(szUrl, "Protocol=FTP::StartTime=%s::StopTime=%s::BitRate=%d::ActiveDate=%s::Provider=%s::RETRY=1",
		(szStartTime)?szStartTime:"", (szEndTime)?szEndTime:"", nBitrate, szActiveDate, szProvider);

    char szMdProvider[256];
    char szMdProduct[256];
    char szMdLicensing_Window_Start[256];
    char szMdTitle[256];
    char szMdAsset_Name[256];
    char szMdType[256];
	char szMdPropagationPriority[256];
	char szMdAsset_Class[256];
	char szPriority[20];

    sprintf(szMdLicensing_Window_Start, "MOD%sLicensing_Window_Start", _szMdSeperator);
    sprintf(szMdProvider,               "AMS%sProvider",               _szMdSeperator);
    sprintf(szMdProduct,                "AMS%sProduct",                _szMdSeperator);
    sprintf(szMdAsset_Name,             "AMS%sAsset_Name",             _szMdSeperator);
    sprintf(szMdTitle,                  "MOD%sTitle",                  _szMdSeperator);
    sprintf(szMdType,                   "MOD%sType",                   _szMdSeperator);
	sprintf(szMdPropagationPriority,	"AMS%sPropagation_Priority",	_szMdSeperator);
	sprintf(szMdAsset_Class,			"AMS%sAsset_Class",				_szMdSeperator);
	
	sprintf(szPriority, "%d", nPriority);

	AutoMutex op(_hAccess);

    try
    {
        _movie_asset -> resetParentMd();
        _movie_asset -> resetMd();

        _movie_asset -> setParentMd (szMdLicensing_Window_Start, szActiveDate);
        _movie_asset -> setParentMd (szMdProvider, szProvider);
        _movie_asset -> setParentMd (szMdProduct,  "MOD");	//just for asset name
        _movie_asset -> setParentMd (szMdTitle,    "movie");//just for asset name
		
		_movie_asset -> setParentMd (szMdPropagationPriority, szPriority);
		_movie_asset -> setParentMd (szMdAsset_Class, "");

        _movie_asset -> setMd       (szMdAsset_Name, szAssetName);
        _movie_asset -> setMd       (szMdType,     "movie");
    }
    catch(const CORBA::Exception& ex) 
    {
		sprintf(szErrMsg, "Caught %s in movie_asset set Md", ex._rep_id()); 
        return false;
    }

    CORBA::String_var pushUrl;
	ContentModule::Content_var servant=ContentModule::Content::_nil();
    try
    {
        servant = ContentModule::Content::_narrow(refBase);
        servant -> provisionForPush(szUrl, ServerModule::as_InService, _movie_asset, pushUrl );
    }
    catch(ServerModule::ProvisioningFailed & ex)
    {
        CORBA::String_var msg = ex.message;
        sprintf(szErrMsg, "Caught ProvisioningFailed : %s in provisionForPush()", (const char*) msg);

        return false;
    }
    catch(ServerModule::UnspecifiedException & ex)
    {
        CORBA::String_var msg = ex.message;
        sprintf(szErrMsg, "Caught UnspecifiedException : %s in provisionForPush()", (const char *) msg);

        return false;
    }
    catch(const CORBA::Exception& ex) 
    {
		sprintf(szErrMsg, "Caught %s in provisionForPush()", ex._rep_id());
        return false;
    }

	//
	// parse the asset id and remove it from the url
	//
	{
		const char* pPtr = strstr((const char*) pushUrl, "?");
		if (!pPtr)
		{
			sprintf(szErrMsg, "No asset id information in url, maybe the registry setting [EnableAssetGear] of isaContent not enabled");
			return false;
		}

		//set url
		int nCopyLen = (pPtr - (const char*) pushUrl);
		strncpy(szPushUrl, (const char*) pushUrl, nCopyLen);
		szPushUrl[nCopyLen]='\0';

		// get asset id
		const char* pPtr2 = strstr(pPtr, "=");
		if (!pPtr2)
		{
			sprintf(szErrMsg, "No asset id information in url, url error");
			return false;
		}
		pPtr2++;

		dwAssetID = strtol(pPtr2, NULL, 16);
		
		//ae
		{
			const char* p = szPushUrl + nCopyLen - 1;
			while(p>szPushUrl&&*p!='/')p--;
			p++;
			dwAeID = strtol(p, NULL, 16);
		}
	}
	
	return true;
}

