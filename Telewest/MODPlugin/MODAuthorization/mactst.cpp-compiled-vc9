#include <assert.h>
#include <stdio.h>
#include <io.h>
#include <process.h>

#include "mactst.h"
#include "ModAuthReporter.h"
#include "ModSoapWrapper.h"
#include "Mod.h"

#include "AppSite.h"

// Windows debugging tools
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Globals
WCHAR g_wszLogFileName[MAX_PATH];
DWORD g_dwLogFileSize;
DWORD g_dwLogFileTrace;

// the full path of soap wsdl file
WCHAR g_wszSOAPWSDLFileName[MAX_PATH];
WCHAR g_wszSOAPWSMLFileName[MAX_PATH];
DWORD g_dwSOAPConnectTimeout;          // millisecond
DWORD g_dwSOAPRetryCount;			   // retry times for soap invoking
WCHAR g_wszAppSitesCfgFileName[MAX_PATH];

CModAuthReporter* g_pAuthReporter;


// This points to the one and only one instance of the MOD Authorization Check Plug-In DLL object.
CModAuth *CModAuth::m_instance = NULL;


CModAuth* CModAuth::Instance(MACHANDLES *pHandles, MACCALLBACKS *pCallbacks )
{
	if(m_instance == NULL)
	{
		m_instance = new CModAuth(pHandles, pCallbacks );
	}
	return m_instance;
}

CModAuth* CModAuth::Instance()
{
	return m_instance;
}

void CModAuth::FreeInstance()
{
	if(m_instance != NULL)
	{
		delete m_instance;
		m_instance = NULL;
	}
}

/*
 * Procedure:   CModAuth Constructor
 *
 * Description: This constructor initializes the the MbsDst Dll object.  Only 
 *              one instance of this object should be created.
 *
 * Arguments:   pHandles
 *                  Pointer to management handle of MOD Billing Service
 *
 *              pCallbacks
 *                  Pointer to MOD Billing Service routine to signal an unload request
 *
 * Returns:     Pointer to a newly created CMbsDst object.
 *
 * Effects:     None.
 */

CModAuth::CModAuth ( MACHANDLES *pHandles, MACCALLBACKS *pCallbacks ) {

    WCHAR wszServiceName[MAX_PATH] = {0};
    WCHAR wszEnvName[MAX_PATH] = {0};
    WORD  wID;

    InitializeCriticalSection ( &m_Lock );
    m_pConfig       = NULL;
    g_pAuthReporter = NULL;
    m_hManPkg       = NULL;
    m_pfnUnloadRtn  = NULL;
    m_bValid        = FALSE;
    m_pwszName      = L"MACDLL";
	m_dwPrefixLength = 2;

    // Save handles and callbacks
    if ( ( pHandles->hMgmt != NULL ) && ( pHandles->hMgmt != INVALID_HANDLE_VALUE ) ) {
        m_hManPkg = pHandles->hMgmt;
        wID = pHandles->wID;
    }

    if ( pCallbacks != NULL ) {
        m_pfnUnloadRtn = pCallbacks->pfnUnloadRtn;
        m_pfnAllocRtn  = pCallbacks->pfnAllocRtn;
        m_pfnUpdateRtn = pCallbacks->pfnUpdateRtn;
    }

    // Configuration,
    wsprintf( wszServiceName, L"MOVIES ON DEMAND" );
    if ( wID == 2 )
        wcscat( wszServiceName, L"2" );

	// create the config instance
    m_pConfig = new CConfig ( L"ITV Applications", wszServiceName );
    if ( m_pConfig == NULL )
	{
		throw ( ITV_OUT_OF_MEMORY );
	}

    if ( ExpandEnvironmentStrings(L"%ITVROOT%\\Log\\ModAuthorization", wszEnvName, MAX_PATH) == 0  || *wszEnvName == '%')
        wcscpy ( wszEnvName, L"c:\\itv\\log\\ModAuthorization" );
    if ( wID == 2 )
        wcscat( wszEnvName, L"2" );
    wcscat ( wszEnvName, L".log" );

    m_pConfig->GetString(L"AuthorizationDLL\\LogFilePath",       g_wszLogFileName,    wszEnvName, MAX_PATH );
    m_pConfig->GetInteger(L"AuthorizationDLL\\LogFileSize",       &g_dwLogFileSize,    64*1024 );
    m_pConfig->GetInteger(L"AuthorizationDLL\\LogFileTraceLevel", &g_dwLogFileTrace,   TRACE_DEBUG );


    // create Reporter instance
	g_pAuthReporter= new CModAuthReporter();
	if ( g_pAuthReporter == NULL )
	{
		delete m_pConfig;
		m_pConfig = NULL;
		throw ( ITV_OUT_OF_MEMORY );
	}

    // Set the Circular Log FileSize
    // Note: this value must be set PRIOR to calling the Init() method on this object
    if (g_dwLogFileSize)
        g_pAuthReporter->SetSvcLogMaxFileSize ( g_dwLogFileSize );

    g_pAuthReporter->SetLogLevel ( ALL_LOGS, g_dwLogFileTrace ); // set the log level
    RPTSTATUS rptRet = g_pAuthReporter->Init ( g_wszLogFileName );
    WCHAR wszDllName [MAX_PATH];
    wcscpy ( wszDllName, m_pwszName );
    if ( wID == 2 )
        wcscat ( wszDllName, L"2" );
    rptRet = g_pAuthReporter->Register (wszDllName, wszServiceName );

	g_pAuthReporter->ReportLog(REPORT_DEBUG, L"===================== Loading MOD Plugin =====================");

	m_pConfig->GetString(L"AuthorizationDLL\\SOAPWsdlPath", g_wszSOAPWSDLFileName, L"", MAX_PATH);
    m_pConfig->GetInteger(L"AuthorizationDLL\\SOAPConnectTimeout", &g_dwSOAPConnectTimeout, 0);
    m_pConfig->GetInteger(L"AuthorizationDLL\\SOAPRetryCount", &g_dwSOAPRetryCount, 5);
	m_pConfig->GetString(L"AuthorizationDLL\\AppSitesConfigurationFile", g_wszAppSitesCfgFileName, L"", MAX_PATH);

	// Does this MOD plugin use Application configuration file.
	if(wcscmp(g_wszAppSitesCfgFileName, L"") == 0)
	{
		m_bHasAppSiteCfg = FALSE;
	}
	else
	{
		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"MOD Plug-in support multiple WFES");
		m_bHasAppSiteCfg = TRUE;
	}

	if(g_dwSOAPRetryCount < 1)
	{
		g_dwSOAPRetryCount = 1;
	}
	
	int i = 0;

    // Select desired DS TLV(s) for MacProcess
    for (i = 0; i < pHandles->wDsInfo; i++) {

        MACINFO *pDsInfo = pHandles->ppDsInfo[i];

        switch (pDsInfo->wTag) {
			case MAC_TAG_DS_ASSET_LENGTH:
			case MAC_TAG_DS_ASSET_COMPUTED_PRICE:
			case MAC_TAG_DS_ASSET_PREVIEW_TIME:
			case MAC_TAG_DS_ASSET_RENTAL_TIME:
			case MAC_TAG_DS_ASSET_RENTAL_TYPE:
			case MAC_TAG_DS_ASSET_PROVIDER_ID:
			case MAC_TAG_DS_ASSET_PROVIDER_ASSET_ID:
                pDsInfo->bRequested = TRUE;
                g_pAuthReporter->ReportLog ( REPORT_DEBUG, L"CModAuth::CModAuth - Selected DS Tag=%d", pDsInfo->wTag );
                break;
        }
    }

    // Select desired AD TLV(s) for MacReport
    for (i = 0; i < pHandles->wAdInfo; i++) {

        MACINFO *pAdInfo = pHandles->ppAdInfo[i];

        switch (pAdInfo->wTag) {
			case MAC_TAG_AD_MAC_ADDRESS:
			case MAC_TAG_AD_BILLING_ID:
			case MAC_TAG_AD_HOME_ID:
			case MAC_TAG_AD_SMART_CARD_ID:
			case MAC_TAG_AD_PURCHASE_ID:
			case MAC_TAG_AD_PURCHASE_TIME:
			case MAC_TAG_AD_RELEASE_CODE:
			case MAC_TAG_AD_PLAY_TIME:
			case MAC_TAG_AD_PROVIDER_ID:
		    case MAC_TAG_AD_PROVIDER_ASSET_ID:
				pAdInfo->bRequested = TRUE;
                g_pAuthReporter->ReportLog( REPORT_DEBUG, L"CModAuth::CModAuth - Selected AD Tag=%d", pAdInfo->wTag );
                break;
        }
    }

    // It worked!
    m_bValid = TRUE;
}

/*
 * Procedure:   CModAuth Destructor
 *
 * Description: This destructor removes the MacTst Dll object.
 *
 * Arguments:   None
 *
 * Returns:     None
 *
 * Effects:     None
 */

CModAuth::~CModAuth () {

    // Its gone.
    m_bValid = FALSE;

    // Close out CfgPkg and Report interfaces
    if ( m_pConfig ) {
        delete m_pConfig;
    }

    if ( g_pAuthReporter ) {
        g_pAuthReporter->Flush();
        g_pAuthReporter->Uninit();
        delete g_pAuthReporter;        // last because others log throughout shutdown
		g_pAuthReporter = NULL;
    }

    DeleteCriticalSection ( &m_Lock );
}

/*
 * Procedure:   Initialize
 *
 * Description: This routine is called during initialization to populate
 *              global variables with information from configuration file.
 *
 * Arguments:   None
 *
 * Returns:     None
 *
 * Effects:     None
 */

ITVSTATUS CModAuth::Initialize () {

    ITVSTATUS err = ITV_SUCCESS;

    g_pAuthReporter->ReportLog( REPORT_DEBUG, L"Entering CModAuth::Initialize" );

	// read the application sites from XML file.
	if(m_bHasAppSiteCfg)
	{
		if(!m_appSites.ReadConfiguration(g_wszAppSitesCfgFileName))
		{
			g_pAuthReporter->ReportLog( REPORT_CRITICAL, L"Failed to load application sites configuration file %s", g_wszAppSitesCfgFileName);
			err = ITV_NT_ERROR;
		}
		else
		{
			m_dwPrefixLength = m_appSites.GetPrefixLength();
			// the length is not in the bound [1, 4]
			if(m_dwPrefixLength < 1 || m_dwPrefixLength >4)
			{
				g_pAuthReporter->ReportLog(REPORT_WARNING, L"PrefixLength in configuration file is not in [1, 4], set to be default value 2");
				m_dwPrefixLength = 2;
			}
			g_pAuthReporter->ReportLog( REPORT_DEBUG, L"Succeed to load application sites configuration file %s", g_wszAppSitesCfgFileName);
		}
	}

    // Done reading so shutdown config interface
    delete m_pConfig;
    m_pConfig = NULL;

    g_pAuthReporter->ReportLog( REPORT_DEBUG, L"Exiting CModAuth::Initialize" );
    return ( err );
}

/*
 * Procedure:   Uninitialize
 *
 * Description: This routine is called during shutdown.
 *
 * Arguments:   None
 *
 * Returns:     None
 *
 * Effects:     None
 */

ITVSTATUS CModAuth::Uninitialize () {

    g_pAuthReporter->ReportLog( REPORT_DEBUG, L"Entering CModAuth::Uninitialize" );

    g_pAuthReporter->ReportLog( REPORT_DEBUG, L"Exiting CModAuth::Uninitialize" );
    return ( ITV_SUCCESS );
}

/*
 * Procedure:   Process
 *
 * Description: This routine is called by MOD when a credit check is needed
 * *
 * Arguments:   pMacData
 *                  Block of input data on which authorization check will be performed
 *
 *              pbStatus
 *                  Address of flag used to indicate results of credit check
 *
 * Returns:     None
 *
 * Effects:     None
 */

ITVSTATUS CModAuth::Process (BYTE *pRequest, BOOL *pbResponse) {

    ITVSTATUS Status = ITV_SUCCESS;

    WCHAR    wszMacAddress[30] = L"";
    DWORD    dwBillingId       = 0;
    DWORD    dwHomeId          = 0;
    DWORD    dwSmartCardId     = 0;
    WCHAR    wszSid[MAX_PATH]  = L"";
    STREAMID Sid               = {0};

	DWORD    dwPurchaseID	   = 0;
	DWORD    dwPurchaseTime    = 0;
	DWORD    dwAssetLength     = 0; // in minutes
	DWORD    dwRentalType      = 0; 
	DWORD    dwOriginalRentalTime = 0;  

    WCHAR    wszProviderID[256] = L"";
    WCHAR    wszProviderAssetID[256] = L"";

    WCHAR    wszData [MAX_PATH];
    struct tm *pTime;
    union {
        BYTE    byteVal;
        WORD    wVal;
        DWORD   dwVal;
        float   rVal;
        double  fVal;
        time_t  tVal;
        BYTE    bVal [MAX_PATH];
        WCHAR   sVal [MAX_PATH];
    };


    g_pAuthReporter->ReportLog( REPORT_DEBUG, L"Entering CModAuth::Process" );

    // Initialize response
    *pbResponse = FALSE;

    // Parse request
    BYTE *pPtr = pRequest;
    WORD wPos  = 0;
    WORD wVer  = 0;
    WORD wCnt  = 0;
    int  i;

    Decode(pPtr, &wPos, sizeof (BYTE), (BYTE *) &wVer);
    g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Process Decode - Lth=%d,Val=0x%02x", sizeof (BYTE), wVer);

    if ( wVer != 2)
        return ( ITV_INVALID_VERSION );

    Decode(pPtr, &wPos, sizeof (WORD), (BYTE *) &wCnt);
    g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Process Decode - Lth=%d,Val=0x%02x", sizeof (WORD), wCnt);

    for ( i = 0; i < wCnt; i++ ) {

        WORD wLth = 0;
        WORD wTag = 0;

        Decode(pPtr, &wPos, &wTag, &wLth);

        switch ( wTag ) {
            
            /* Session setup values */
            case MAC_TAG_MACADDRESS:
                Decode(pPtr, &wPos, wLth, (BYTE *) wszMacAddress);
                wszMacAddress [ wLth / sizeof (WCHAR ) ] = 0; // null terminate
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Process Decode - Tag(MAC_TAG_MACADDRESS)=%d,Lth=%d,Val=%s", wTag, wLth, wszMacAddress);
                break;

            case MAC_TAG_BILLINGID:
                Decode(pPtr, &wPos, wLth, (BYTE *) &dwBillingId);
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Process Decode - Tag(MAC_TAG_BILLINGID)=%d,Lth=%d,Val=%d", wTag, wLth, dwBillingId);
                break;

            case MAC_TAG_HOMEID:
                Decode(pPtr, &wPos, wLth, (BYTE *) &dwHomeId);
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Process Decode - Tag(MAC_TAG_HOMEID)=%d,Lth=%d,Val=%d", wTag, wLth, dwHomeId);
                break;

            case MAC_TAG_SMARTCARDID:
                Decode(pPtr, &wPos, wLth, (BYTE *) &dwSmartCardId);
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Process Decode - Tag(MAC_TAG_SMARTCARDID)=%d,Lth=%d,Val=%d", wTag, wLth, dwSmartCardId);
                break;

            case MAC_TAG_STREAMID:
                Decode(pPtr, &wPos, wLth, (BYTE *) &Sid);
                wsprintf(wszSid, L"%d.%08x/%x", Sid.dwStreamIdNumber, Sid.TypeInst.s.dwType, Sid.TypeInst.s.dwInst);
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Process Decode - Tag(MAC_TAG_STREAMID)=%d,Lth=%d,Val=%s", wTag, wLth, wszSid);
                break;
            
			case MAC_TAG_DS_ASSET_PROVIDER_ID:
                Decode(pPtr, &wPos, wLth, (BYTE *) wszProviderID);
                wszProviderID [ wLth / sizeof (WCHAR ) ] = 0; // null terminate
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Process Decode - Tag(MAC_TAG_DS_ASSET_PROVIDER_ID)=%d,Lth=%d,Val=%s", wTag, wLth, wszProviderID);
                break;

            case MAC_TAG_DS_ASSET_PROVIDER_ASSET_ID:
                Decode(pPtr, &wPos, wLth, (BYTE *) wszProviderAssetID);
                wszProviderAssetID [ wLth / sizeof (WCHAR ) ] = 0; // null terminate
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Process Decode - Tag(MAC_TAG_DS_ASSET_PROVIDER_ASSET_ID)=%d,Lth=%d,Val=%s", wTag, wLth, wszProviderAssetID);
                break;
			
			case MAC_TAG_DS_ASSET_LENGTH:
                Decode(pPtr, &wPos, wLth, (BYTE *) &dwAssetLength);
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Process Decode - Tag(MAC_TAG_DS_ASSET_LENGTH)=%d,Lth=%d,Val=%d", wTag, wLth, dwAssetLength);
				break;

			case MAC_TAG_DS_ASSET_RENTAL_TYPE:
                Decode(pPtr, &wPos, wLth, (BYTE *) &dwRentalType);
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Process Decode - Tag(MAC_TAG_DS_ASSET_RENTAL_TYPE)=%d,Lth=%d,Val=%d", wTag, wLth, dwRentalType);
				break;

			case MAC_TAG_DS_ASSET_RENTAL_TIME:
                Decode(pPtr, &wPos, wLth, (BYTE *) &dwOriginalRentalTime);
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Process Decode - Tag(MAC_TAG_DS_ASSET_RENTAL_TIME)=%d,Lth=%d,Val=%d", wTag, wLth, dwOriginalRentalTime);

				break;
            /*  DWORD */
            case MAC_TAG_DS_ASSET_PREVIEW_TIME:
            case MAC_TAG_DS_ASSET_VIEWING_TIME:
            case MAC_TAG_DS_ASSET_VIEWING_TYPE:
            case MAC_TAG_DS_ASSET_SCRAMBLED_TIME:
            case MAC_TAG_DS_ASSET_SCRAMBLED_TYPE:
            case MAC_TAG_DS_ASSET_ANALOG_COPY_ALLOWED:
            case MAC_TAG_DS_FOLDER_PRICINGOPTIONS:
            case MAC_TAG_DS_FOLDER_SUSPENDLISTOPTIONS:
            case MAC_TAG_DS_FOLDER_PACKAGEID:
                Decode(pPtr, &wPos, wLth, (BYTE *) &dwVal);
                g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Process Decode - Tag=%d,Lth=%d,Val=%d (DWORD)", wTag, wLth, dwVal);
                break;

            /* DOUBLE */
            case MAC_TAG_DS_ASSET_PRICE:
            case MAC_TAG_DS_ASSET_ANALOG_COPY_CHARGE:
            case MAC_TAG_DS_ASSET_COMPUTED_PRICE:
            case MAC_TAG_DS_FOLDER_PRICINGVALUE:
            case MAC_TAG_DS_FOLDER_SUSPENDLISTLIFETIME:
                Decode(pPtr, &wPos, wLth, (BYTE *) &fVal);
                g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Process Decode - Tag=%d,Lth=%d,Val=%f (DOUBLE)", wTag, wLth, fVal);
                break;

            /* WCHAR */
            case MAC_TAG_DS_ASSET_CONTENT_PROVIDER:
            case MAC_TAG_DS_ASSET_GENRE:
            case MAC_TAG_DS_ASSET_SHORT_TITLE:
            case MAC_TAG_DS_ASSET_EVENT_ID:
            case MAC_TAG_DS_ASSET_PROVIDER:
            case MAC_TAG_DS_ASSET_RATING:
            case MAC_TAG_DS_ASSET_ASSET_TITLE:
            case MAC_TAG_DS_ASSET_ASSET_BRIEF_TITLE:
            case MAC_TAG_DS_FOLDER_BILLINGTAG:
                Decode(pPtr, &wPos, wLth, (BYTE *) sVal);
                sVal[wLth/sizeof(WCHAR)] = 0;
                g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Process Decode - Tag=%d,Lth=%d,Val=%s (WCHAR)", wTag, wLth, sVal);
                break;

            /* TIME */
            case MAC_TAG_DS_ASSET_DEACTIVATE_TIME:
                Decode(pPtr, &wPos, wLth, (BYTE *) &tVal);
                pTime = localtime ( &tVal );
                wcscpy( wszData, _wasctime ( pTime ) );
                wszData[wcslen(wszData) -1] = '\0'; // remove \n from string
                g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Process Decode - Tag=%d,Lth=%d,Val=%d (TIME) - %s", wTag, wLth, tVal, wszData);
                break;

            /* BYTE */
            case MAC_TAG_DS_ASSET_VCR_ALLOWED:
            case MAC_TAG_DS_ASSET_SCRAMBLED:
            case MAC_TAG_DS_ASSET_SUSPENDABLE:
                Decode(pPtr, &wPos, wLth, (BYTE *) &byteVal);
                g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Process Decode - Tag=%d,Lth=%d,Val=%d (BYTE)", wTag, wLth, byteVal);
                break;

            case MAC_TAG_DS_ASSET_VCR_CHARGE:   // TEST: ignore this
            default:
                g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Process Decode - Tag=%d,Lth=%d ignored", wTag, wLth );
                wPos += wLth;
        }
    }

	dwPurchaseID = dwBillingId;	// pass PurchaseID via BillingID

    // Set up soap communication
	DWORD dwCurrentCount = 0;
	BOOL bRet = FALSE;
	DWORD dwRentalTime = 0;
	double fComputedPrice = 0.0f;

	// WSDL comes from configuration file
	if(m_bHasAppSiteCfg)
	{
		// the ticketid is composed of AppID & TicketID
		char chSiteID[24];
		GetAppIDFromTicketID(dwPurchaseID, chSiteID);
		
		CAppSiteData* pSiteData = m_appSites.GetSiteDataByID(chSiteID);
		if(pSiteData == NULL)
		{
			WCHAR wchID[24];
			mbstowcs(wchID, chSiteID, 24);

			g_pAuthReporter->ReportLog(REPORT_CRITICAL, L"CModAuth::Process failed to find WFES for AppID=%s", wchID);
			*pbResponse = FALSE;
			return ( Status );
		}
		else
		{
			WCHAR wchID[256], wchName[256], wchWSDL[256];
			mbstowcs(wchID, pSiteData->GetID().c_str(), 256);
			mbstowcs(wchName, pSiteData->GetName().c_str(), 256);
			mbstowcs(wchWSDL, pSiteData->GetWSDL().c_str(), 256);

			g_pAuthReporter->ReportLog(REPORT_DEBUG, L"CModAuth::Process BillingID = %d - AppID = %s, Name=%s, WSDL=%s", 
				dwPurchaseID, wchID, wchName, wchWSDL);

			wcscpy(g_wszSOAPWSDLFileName, wchWSDL);
		}
	}

	CModSoapWrapper	pSoapWrapper(g_dwSOAPConnectTimeout, g_wszSOAPWSDLFileName, g_wszSOAPWSMLFileName, 
			                    wszProviderID, wszProviderAssetID, wszMacAddress, dwPurchaseID, Sid, dwPurchaseTime);
	while((dwCurrentCount < g_dwSOAPRetryCount) && !bRet)
	{
		dwCurrentCount++;

		bRet = pSoapWrapper.SetupSOAPCommunicate(*pbResponse, fComputedPrice, dwRentalTime);

		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"CModAuth::Process SetupSOAPCommunicate() with %d/%d", dwCurrentCount, g_dwSOAPRetryCount);
	}
	// update Asset Computed Price & Rental time(in minutes)
	if(bRet && *pbResponse)
	{
		if(dwRentalTime == 0)
		{	// to support Telewest SVOD issue.
			g_pAuthReporter->ReportLog(REPORT_DEBUG, L"RentalTime from OTE is 0, No updating operation");
			g_pAuthReporter->ReportLog(REPORT_DEBUG, L"Leaving CModAuth::Process");
			return ( Status );
		}
		// for preview, the rental time set to be asset length
		if(dwPurchaseID == 0)
			dwRentalTime = dwAssetLength;

		Status = Update(Sid, pbResponse, fComputedPrice, dwRentalTime, dwRentalType, dwAssetLength, dwOriginalRentalTime);
	}
	g_pAuthReporter->ReportLog(REPORT_DEBUG, L"Leaving CModAuth::Process");
    return ( Status );
}

/*
 * Procedure:   Report
 *
 * Description: This routine is called by MOD when a credit check is needed
 * *
 * Arguments:   pMacData
 *                  Block of input data on which authorization check will be performed
 *
 *              pbStatus
 *                  Address of flag used to indicate results of credit check
 *
 * Returns:     None
 *
 * Effects:     None
 */

ITVSTATUS CModAuth::Report (BYTE *pRequest, BOOL *pbResponse) {

    ITVSTATUS Status = ITV_SUCCESS;
  
    WCHAR    wszMacAddress[30] = L"";
    DWORD    dwBillingId       = 0;
    DWORD    dwHomeId          = 0;
    DWORD    dwSmartCardId     = 0;
    WCHAR    wszSid[MAX_PATH]  = L"";
    STREAMID Sid               = {0};
	DWORD    dwPurchaseTime    = 0;
	DWORD    dwPurchaseID      = 0;
	DWORD    dwReleaseCode     = 0;

    WCHAR    wszProviderID[256] = L"";
    WCHAR    wszProviderAssetID[256] = L"";

    WCHAR     wszData [MAX_PATH];
    struct tm *pTime;
    union {
        BYTE    byteVal;
        WORD    wVal;
        DWORD   dwVal;
        float   rVal;
        double  fVal;
        time_t  tVal;
        BYTE    bVal [MAX_PATH];
        WCHAR   sVal [MAX_PATH];
    };


    g_pAuthReporter->ReportLog( REPORT_DEBUG, L"Entering CModAuth::Report" );

    // Initialize response
    *pbResponse = FALSE;

    // Parse request
    BYTE *pPtr = pRequest;
    WORD wPos  = 0;
    WORD wVer  = 0;
    WORD wCnt  = 0;

    Decode(pPtr, &wPos, sizeof (BYTE), (BYTE *) &wVer);
    g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Report Decode - Lth=%d,Val=0x%02x", sizeof (BYTE), wVer);

    if ( wVer != 1 )
        return ( ITV_INVALID_VERSION );

    Decode(pPtr, &wPos, sizeof (WORD), (BYTE *) &wCnt);
    g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Report Decode - Lth=%d,Val=0x%02x", sizeof (WORD), wCnt);

    for ( int i = 0; i < wCnt; i++ ) {

        WORD wLth = 0;
        WORD wTag = 0;

        Decode(pPtr, &wPos, &wTag, &wLth);

        switch ( wTag ) {
            
            case MAC_TAG_STREAMID:
                Decode(pPtr, &wPos, wLth, (BYTE *) &Sid);
                wsprintf(wszSid, L"%d.%08x/%x", Sid.dwStreamIdNumber, Sid.TypeInst.s.dwType, Sid.TypeInst.s.dwInst);
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Report Decode - Tag(MAC_TAG_STREAMID)=%d,Lth=%d,Val=%s", wTag, wLth, wszSid);
                break;

			case MAC_TAG_AD_MAC_ADDRESS:
                Decode(pPtr, &wPos, wLth, (BYTE *) wszMacAddress);
                wszMacAddress [ wLth / sizeof (WCHAR ) ] = 0; // null terminate
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Report Decode - Tag(MAC_TAG_AD_MAC_ADDRESS)=%d,Lth=%d,Val=%s", wTag, wLth, wszMacAddress);
                break;

            case MAC_TAG_AD_BILLING_ID:
                Decode(pPtr, &wPos, wLth, (BYTE *) &dwBillingId);
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Report Decode - Tag(MAC_TAG_AD_BILLING_ID)=%d,Lth=%d,Val=%d", wTag, wLth, dwBillingId);
                break;

            case MAC_TAG_AD_HOME_ID:
                Decode(pPtr, &wPos, wLth, (BYTE *) &dwHomeId);
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Report Decode - Tag(MAC_TAG_AD_HOME_ID)=%d,Lth=%d,Val=%d", wTag, wLth, dwHomeId);
                break;

            case MAC_TAG_AD_SMART_CARD_ID:
                Decode(pPtr, &wPos, wLth, (BYTE *) &dwSmartCardId);
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Report Decode - Tag(MAC_TAG_AD_SMART_CARD_ID)=%d,Lth=%d,Val=%d", wTag, wLth, dwSmartCardId);
                break;

 			case MAC_TAG_AD_PURCHASE_ID:
				Decode(pPtr, &wPos, wLth, (BYTE *) &dwPurchaseID);
				g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Report Decode - Tag(MAC_TAG_AD_PURCHASE_ID)=%d,Lth=%d,Val=%d", wTag, wLth, dwPurchaseID);
				break;

            case MAC_TAG_AD_PURCHASE_TIME:
                Decode(pPtr, &wPos, wLth, (BYTE *) &dwPurchaseTime);
				g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Report Decode - Tag(MAC_TAG_AD_PURCHASE_TIME)=%d,Lth=%d,Val=%d", wTag, wLth, dwPurchaseTime);
                break;

            case MAC_TAG_AD_RELEASE_CODE:
                Decode(pPtr, &wPos, wLth, (BYTE *) &dwReleaseCode);
				g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Report Decode - Tag(MAC_TAG_AD_RELEASE_CODE)=%d,Lth=%d,Val=%d", wTag, wLth, dwReleaseCode);
                break;

            case MAC_TAG_AD_PLAY_TIME:
                Decode(pPtr, &wPos, wLth, (BYTE *) &tVal);
                pTime = localtime ( &tVal );
                wcscpy( wszData, _wasctime ( pTime ) );
                wszData[wcslen(wszData) -1] = '\0'; // remove \n from string
                g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Report Decode - Tag(MAC_TAG_AD_PLAY_TIME)=%d,Lth=%d,Val=%d - %s", wTag, wLth, tVal, wszData);
                break;

			case MAC_TAG_AD_PROVIDER_ID:
                Decode(pPtr, &wPos, wLth, (BYTE *) wszProviderID);
                wszProviderID [ wLth / sizeof (WCHAR ) ] = 0; // null terminate
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Report Decode - Tag(MAC_TAG_AD_PROVIDER_ID)=%d,Lth=%d,Val=%s", wTag, wLth, wszProviderID);
                break;

			case MAC_TAG_AD_PROVIDER_ASSET_ID:
                Decode(pPtr, &wPos, wLth, (BYTE *) wszProviderAssetID);
                wszProviderAssetID [ wLth / sizeof (WCHAR ) ] = 0; // null terminate
				g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Report Decode - Tag(MAC_TAG_AD_PROVIDER_ASSET_ID)=%d,Lth=%d,Val=%s", wTag, wLth, wszProviderAssetID);
                break;

            /* DWORD */
            case MAC_TAG_AD_TYPE:
            case MAC_TAG_AD_ASSET_ID:
            case MAC_TAG_AD_ANALOG_COPY_ALLOWED:
            case MAC_TAG_AD_FAST_FORWARD_COUNT:
            case MAC_TAG_AD_REWIND_COUNT:
            case MAC_TAG_AD_PAUSE_COUNT:
            //case MAC_TAG_AD_PLAY_TIME:
            case MAC_TAG_AD_RENTAL_TIME:
            case MAC_TAG_AD_PACKAGE_ID:
            case MAC_TAG_AD_CONTEXT_ID:
                Decode(pPtr, &wPos, wLth, (BYTE *) &dwVal);
                g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Report Decode - Tag=%d,Lth=%d,Val=%d (DWORD)", wTag, wLth, dwVal);
                break;

            /* DOUBLE */
            case MAC_TAG_AD_PRICE:
            case MAC_TAG_AD_VCR_CHARGE:
            case MAC_TAG_AD_COPY_CHARGE:
                Decode(pPtr, &wPos, wLth, (BYTE *) &fVal);
                g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Report Decode - Tag=%d,Lth=%d,Val=%f (DOUBLE)", wTag, wLth, fVal);
                break;

            /* WCHAR */
            case MAC_TAG_AD_TITLE:
            case MAC_TAG_AD_SHORT_TITLE:
            case MAC_TAG_AD_ASSET_PROVIDER:
            case MAC_TAG_AD_ASSET_GENRE:
            case MAC_TAG_AD_EVENT_ID:
            case MAC_TAG_AD_PROVIDER:
            case MAC_TAG_AD_RATING:
                Decode(pPtr, &wPos, wLth, (BYTE *) sVal);
                sVal[wLth/sizeof(WCHAR)] = 0;
                g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Report Decode - Tag=%d,Lth=%d,Val=%s (WCHAR)", wTag, wLth, sVal);
                break;

            /* BYTE */
            case MAC_TAG_AD_BILLABLE:
            case MAC_TAG_AD_NEW_PURCHASE:
                Decode(pPtr, &wPos, wLth, (BYTE *) &byteVal);
                g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Report Decode - Tag=%d,Lth=%d,Val=%d (BYTE)", wTag, wLth, byteVal);
                break;

            default:
                g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Report Decode - Tag=%d,Lth=%d ignored", wTag, wLth );
                wPos += wLth;

        }
    }

	DWORD dwNewReleaseCode = GetRealReasonCode(dwReleaseCode);

	// WSDL comes from configuration file
	if(m_bHasAppSiteCfg)
	{
		// the ticketid is composed of AppID & TicketID
		// The last 6 number is for the real ticketid
		char chSiteID[10];
		memset(chSiteID, 0x0, 10*sizeof(char));
		GetAppIDFromTicketID(dwPurchaseID, chSiteID);

		CAppSiteData* pSiteData = m_appSites.GetSiteDataByID(chSiteID);
		if(pSiteData == NULL)
		{
			g_pAuthReporter->ReportLog(REPORT_CRITICAL, L"CModAuth::Report failed to find WFES for AppID=%s", chSiteID);
			*pbResponse = FALSE;
			return ( Status );
		}
		else
		{
			WCHAR wchID[256], wchName[256], wchWSDL[256];
			mbstowcs(wchID, pSiteData->GetID().c_str(), 256);
			mbstowcs(wchName, pSiteData->GetName().c_str(), 256);
			mbstowcs(wchWSDL, pSiteData->GetWSDL().c_str(), 256);

			g_pAuthReporter->ReportLog(REPORT_DEBUG, L"CModAuth::Report BillingID = %d - AppID = %s, Name=%s, WSDL=%s", 
				dwPurchaseID, wchID, wchName, wchWSDL);

			wcscpy(g_wszSOAPWSDLFileName, wchWSDL);
		}
	}

	// the version before 1.0.8 are use dwPurchaseID as the parameter. 
	// from 1.0.9, it use dwBillingId
	CModSoapWrapper	pSoapWrapper(g_dwSOAPConnectTimeout, g_wszSOAPWSDLFileName, g_wszSOAPWSMLFileName, 
								wszProviderID, wszProviderAssetID, wszMacAddress, dwBillingId, Sid, dwPurchaseTime, dwNewReleaseCode);

	DWORD dwCurrentCount = 0;
	BOOL bRet = FALSE;
	while( (dwCurrentCount < g_dwSOAPRetryCount) && !bRet)
	{
		dwCurrentCount++;

	    bRet = pSoapWrapper.TearDownSOAPCommunicate(*pbResponse);

		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"CModAuth::Report TearDownSOAPCommunicate() with %d/%d", dwCurrentCount, g_dwSOAPRetryCount);
	}

    return ( Status );
}

/*
 * Procedure:   Update
 *
 * Description: This routine is called by MOD when a credit check is needed
 * *
 * Arguments:   pMacData
 *                  Block of input data on which authorization check will be performed
 *
 *              pbStatus
 *                  Address of flag used to indicate results of credit check
 *
 * Returns:     None
 *
 * Effects:     None
 */

ITVSTATUS CModAuth::Update(STREAMID Sid, BOOL *pbResponse, DOUBLE fPrice, 
						  DWORD dwRentalTime, DWORD dwRentalType, DWORD dwAssetLength, DWORD dwOrigalRentalTime)
{
	ITVSTATUS Status = ITV_SUCCESS;

	double rRentalTime;
    DWORD dwOriDays, dwOriTimeInADay;
	DWORD dwCurDays, dwCurTimeInADay;
	switch(dwRentalType)
	{
	case 1:		// n times of content length
		if(dwAssetLength <= 0)
		{
		    g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Update() without updating RentalTime cosz invalid Asset Length value %d", dwAssetLength);
			return Status;
		}
		rRentalTime = (double)((double)dwRentalTime / (double)dwAssetLength);
		break;
	case 2:		// fixed time
		rRentalTime = dwRentalTime;
		break;
	case 3:		// n days till a specific time. 
		// Get specific time in a day in minutes
		dwOriDays = dwOrigalRentalTime / (24*60);		
		dwOriTimeInADay = dwOrigalRentalTime - dwOriDays*24*60;
		// Get days from OTE
		dwCurDays = dwRentalTime / (24*60);
		dwCurTimeInADay = dwRentalTime - dwCurDays*24*60;

		rRentalTime = (dwCurDays*24*60 + dwOriTimeInADay)*60;  // should be in seconds
		break;
	default:
		g_pAuthReporter->ReportLog(REPORT_TRACE, L"CModAuth::Update() without updating RentalTime cosz invalid dwRentalType value %d", dwRentalType);		
		return Status;
	}

    WORD wLth  = sizeof(BYTE)   // Ver
               + sizeof(WORD)   // Cnt

               // Tag, Length & Value
               + sizeof(WORD) * 2 + sizeof(double)      // Price
			   + sizeof(WORD) * 2 + sizeof(double);      // RentalTime

    BYTE *pTLV = (m_pfnAllocRtn)(wLth);

    WORD wVer  = 1;
    WORD wCnt  = 2;
    WORD wPos  = 0;

    Encode(pTLV, wLth, &wPos, sizeof ( BYTE ), (BYTE *) &wVer);
    Encode(pTLV, wLth, &wPos, sizeof ( WORD ), (BYTE *) &wCnt);

    Encode(pTLV, wLth, &wPos, MAC_TAG_DS_ASSET_COMPUTED_PRICE, sizeof(double),  (BYTE *) &fPrice);
	Encode(pTLV, wLth, &wPos, MAC_TAG_DS_ASSET_RENTAL_TIME,    sizeof(double),  (BYTE *) &rRentalTime);

    g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Update Encode - Lth=%d,Val=0x%02x", sizeof (BYTE), wVer);
    g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Update Encode - Lth=%d,Val=0x%02x", sizeof (WORD), wCnt);

    g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Update Encode - Tag(MAC_TAG_DS_ASSET_COMPUTED_PRICE)=%d,Lth=%d,Val=%f (DOUBLE)",
        MAC_TAG_DS_ASSET_COMPUTED_PRICE, sizeof(double),  fPrice);
    g_pAuthReporter->ReportLog( REPORT_TRACE, L"CModAuth::Update Encode - Tag(MAC_TAG_DS_ASSET_RENTAL_TIME)=%d,Lth=%d,Val=%f (DOUBLE) with RentalType %d",
        MAC_TAG_DS_ASSET_RENTAL_TIME,    sizeof(double),  rRentalTime, dwRentalType);

    Status = (m_pfnUpdateRtn)(Sid, pTLV, pbResponse);

	if(!(*pbResponse))
	{
		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"CModAuth::Update failed to update Computed Price=%f, RentalTime=%f with Rentaltype=%d", 
								fPrice, rRentalTime, dwRentalType);
	}
	else
	{
		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"CModAuth::Update succeeded to update Computed Price=%f, RentalTime=%f with Rentaltype=%d", 
								fPrice, rRentalTime, dwRentalType);
	}

    return(Status);
}

/*
 * Procedure:   MacManUtilCB
 *
 * Description: Represents all of the MOD Authorization Check DLL information.  Called when
 *              ManUtil requests to access the complex variable for this DLL.
 *
 * Arguments:   pwszCmd
 *                  ManUtil command
 *
 *              ppwszResponse
 *                  Response that contains all data
 *
 *              pwLength
 *                  Length of response
 *
 * Returns:     None
 *
 * Effects:     None
 */
MANSTATUS MacManUtilCB(WCHAR *pwszCmd, WCHAR **ppwszResponse, DWORD *pwLength) {

    CModAuth *pModAuth = CModAuth::Instance();
    WORD  wCommand;
    DWORD dwSimpleVars = 6;
    DWORD dwBufSize;
    WCHAR *pwszTmp;
    WCHAR *pwszBuffer;

    // Parse out command byte.
    swscanf ( pwszCmd, L"%c\t", &wCommand );

    // Dispatch based on requested operation
    switch (wCommand) {

        case MAN_READ_VARS:

            // Make sure we are initialized first
            if ( pModAuth == NULL && pModAuth->IsValid() == FALSE ) {

                dwSimpleVars = 1;                 // Number of simple variables
                dwBufSize = ( dwSimpleVars * MAX_PATH );

                try { pwszBuffer = new WCHAR[dwBufSize]; }
                catch (...) { return ( MAN_OUT_OF_MEMORY ); }

                *pwszBuffer = 0;            // start with a null string
                pwszTmp = pwszBuffer;

                // Output buffer header to tell ManUtil how many simple variables are being provided
                pwszTmp += wsprintf ( pwszTmp, L"%d\t0\n", dwSimpleVars );
                pwszTmp += wsprintf ( pwszTmp, L"%d\t%s\t%s\n", MAN_STR, L"STATUS", L"NOT INITIALIZED" );

                *ppwszResponse = pwszBuffer;
                *pwLength = wcslen ( pwszBuffer );
                return ( MAN_SUCCESS );
            }

            // Calculate size of buffer needed.
            // Assumes 50 bytes is the maximum tag size
            // Assumes simple variables are pathname
            dwBufSize = ( dwSimpleVars * ( MAX_PATH + 50 ) );
            try { pwszBuffer = new WCHAR[dwBufSize]; }
            catch (...) { return ( MAN_OUT_OF_MEMORY ); }

            *pwszBuffer = 0;            // start with a null string
            pwszTmp = pwszBuffer;       // convenient pointer for string manipulation

            // Output buffer header to tell ManUtil how many simple variables are being provide
            pwszTmp += wsprintf ( pwszTmp, L"%d\t0\n", dwSimpleVars );

            // Fill in simple variable information
            pwszTmp += wsprintf ( pwszTmp, L" %d\t%s\t%s\n", MAN_STR, 
                                  L"Description",       L"MOD Authorization Checking DLL" );
            pwszTmp += wsprintf ( pwszTmp, L" %d\t%s\t%s\n", MAN_STR, 
                                  L"LogFilePath",       g_wszLogFileName );
            pwszTmp += wsprintf ( pwszTmp, L"%d\t%s\t%d\n", MAN_INT, 
                                  L"LogFileSize",       g_dwLogFileSize );
            pwszTmp += wsprintf ( pwszTmp, L"%d\t%s\t%d\n", MAN_INT, 
                                  L"LogFileTraceLevel", g_dwLogFileTrace );
            pwszTmp += wsprintf ( pwszTmp, L" %d\t%s\t%s\n", MAN_STR, 
                                  L"SOAPWsdlPath",       g_wszSOAPWSDLFileName );
            pwszTmp += wsprintf ( pwszTmp, L" %d\t%s\t%s\n", MAN_STR, 
                                  L"SOAPWsmlPath",       g_wszSOAPWSMLFileName );


            // Fill in data about the response buffer
            // Do not include the null terminator in the size of the buffer
            *pwLength = wcslen(pwszBuffer);
            *ppwszResponse = pwszBuffer;
            break;

        case MAN_FREE:

            // Free the allocated response buffer
            delete [] *ppwszResponse;
            break;

        default:
            SC_CASSERT ( FALSE );
            return ( MAN_BAD_PARAM );
    }

    return ( MAN_SUCCESS );
}

// the release code came from two ways:
// 1. From MOD_TERM_* (defined in MOD.h)
// 2. From Dsmcc reason code. (defined in 
DWORD CModAuth::GetRealReasonCode(DWORD dwOrgReleaseCode)
{

	switch(dwOrgReleaseCode)
	{
	case MOD_TERM_NOT_AUTHORIZED :
	case MOD_TERM_MOVIE_DONE :
	case MOD_TERM_DURATION_TIMEOUT :
	case MOD_TERM_SUSPENDED :
	case MOD_TERM_CLIENT_CANCELED :
    case MOD_TERM_SYSTEM_CANCELED :
	case MOD_TERM_INTERNAL_SERVICE_ERROR :
	case MOD_TERM_BAD_ASSET :
	case MOD_TERM_BLOCKED_ASSET :
	case MOD_TERM_PACKAGE_INVALID_ASSET :
	case MOD_TERM_PACKAGE_UNAVAILABLE :
	case MOD_TERM_VIEW_LIMIT_REACHED :

		return dwOrgReleaseCode;

	default:
		return APPSTATUSGETVALUE(dwOrgReleaseCode);
	}
}

// Get AppID from TicketID
char* CModAuth::GetAppIDFromTicketID(DWORD dwTicketID, char* szAppID)
{
	DWORD dwAppID = (DWORD)(dwTicketID / (DWORD)pow((long double)10, (long double)(10-m_dwPrefixLength)));
	
	char zeropre[10] = {0};
	sprintf(zeropre, "%%0%dd", m_dwPrefixLength);

	sprintf(szAppID, zeropre, dwAppID);
	
	return szAppID;
}

