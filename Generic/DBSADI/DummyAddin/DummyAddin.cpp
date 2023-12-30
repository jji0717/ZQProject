// DummyAddin.cpp : Defines the initialization routines for the DLL.
//

// NOTICE:
// This is a dummy DBSync Add-in example.  The DLL simply exports each callback functions
// and logs down corresponding information.

#include "stdafx.h"
#include "DummyAddin.h"
#include "../doc/DBSAdi_def.h"
#include "sclog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CDummyAddinApp

BEGIN_MESSAGE_MAP(CDummyAddinApp, CWinApp)
	//{{AFX_MSG_MAP(CDummyAddinApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



using namespace ZQ::common;

ScLog*	pTheLog=NULL;

#define _TAB	L"\t"
#define _TAB2	L"\t\t"



/////////////////////////////////////////////////////////////////////////////
// CDummyAddinApp construction
CDummyAddinApp::CDummyAddinApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	pTheLog = new ScLog("DummyAddin.log",Log::L_DEBUG, 32*1024*1024);
	pGlog = pTheLog;

	glog(Log::L_NOTICE, "DBSync Dummy Add-in loaded.");
}

CDummyAddinApp::~CDummyAddinApp()
{
	if(pTheLog)
		delete pTheLog;
}
/////////////////////////////////////////////////////////////////////////////
// The one and only CDummyAddinApp object

CDummyAddinApp theApp;



DBSACALLBACK 
DBSA_Initialize(	
		DA_dbsyncInfo*		pDbsInfo,
		DA_itvInfo*			pItvInfo
		)
{
	glog(Log::L_NOTICE, "[Callback]	DBSA_Initialize");
	
	glog(Log::L_DEBUG, _TAB L"DBSync Information:");
	glog(Log::L_DEBUG, _TAB2 L"IP=%s, Version=%s, SyncDir=%s, Inst=%d, SupportNAV=%d, TimeWindowThreshold=%d", 
		pDbsInfo->_szIPAddr, pDbsInfo->_szVersion, pDbsInfo->_szSyncDir, pDbsInfo->_dwInstanceID, pDbsInfo->_dwSupportNav, pDbsInfo->_dwTwThreshold);
	
	glog(Log::L_DEBUG, _TAB L"ITV DS Information:");
	glog(Log::L_DEBUG, _TAB2 L"IP=%s, Version=%s",
		pItvInfo->_szIPAddr, pItvInfo->_szVersion);
}


DBSACALLBACK 
DBSA_Uninitialize(
		)
{
	glog(Log::L_NOTICE, "[Callback]	DBSA_Uninitialize");
}


DBSACALLBACK 
DBSA_SyncBegin(
		)
{
	glog(Log::L_NOTICE, "[Callback]	DBSA_SyncBegin");
}


DBSACALLBACK 
DBSA_SyncEnd(
		)
{
	glog(Log::L_NOTICE, "[Callback]	DBSA_SyncEnd");
}


DBSACALLBACK 
DBSA_TriggerMd(
		DA_entryDb*			pEntryBlock,
		DWORD				dwMdNumber,
		DA_metaDb*			pFirstMdBlock
		)
{
	glog(Log::L_NOTICE, "[Callback]	DBSA_TriggerMd");

	glog(Log::L_DEBUG, _TAB L"Entry Information:");
	glog(Log::L_DEBUG, _TAB2 L"Type=%ld, UID=%ld(0x%08X), LocalUID=%s", 
		pEntryBlock->_dwEntryType, pEntryBlock->_dwEntryUID, pEntryBlock->_dwEntryUID, pEntryBlock->_szLocalEntryUID);
	
	glog(Log::L_DEBUG, _TAB L"Metadata Information: Number = %d", dwMdNumber);
	for(DWORD i=0; i<dwMdNumber; i++)
	{
		glog(Log::L_DEBUG, _TAB2 L"NO%d. Name=%s, Value=%s, UID=%ld, AppUID=%ld(0x%08X), OP=%d",
			i, (pFirstMdBlock+i)->_szMdName, (pFirstMdBlock+i)->_szMdValue, (pFirstMdBlock+i)->_dwMdUID, (pFirstMdBlock+i)->_dwAppUID, (pFirstMdBlock+i)->_dwAppUID, (pFirstMdBlock+i)->_dwOp);
	}
}


DBSACALLBACK 
DBSA_TriggerMdd(
		DA_mddListDb*		pMddList
		)
{
	glog(Log::L_NOTICE, "[Callback]	DBSA_TriggerMdd");
	
	glog(Log::L_DEBUG, _TAB L"MDD Information:");
	glog(Log::L_DEBUG, _TAB2 L"Name=%s, UID=%ld, AppUID=%ld(0x%08X)", 
		pMddList->_szMdName, pMddList->_dwMdUID, pMddList->_dwAppUID, pMddList->_dwAppUID);
	
}


DBSACALLBACK 
DBSA_TriggerHierarchy(
		DWORD				dwOperation,
		DA_hierarchyDb*		pHierarchyBlock
		)
{
	glog(Log::L_NOTICE, "[Callback]	DBSA_TriggerHierarchy");
	
	glog(Log::L_DEBUG, _TAB L"Hierarchy Information:  OP=%d", dwOperation);
	glog(Log::L_DEBUG, _TAB2 L"Type=%d, HUID=%ld(0x%08X), LocalHUID=%s", 
		pHierarchyBlock->_dwEntryType, pHierarchyBlock->_dwHierarchyUID, pHierarchyBlock->_dwHierarchyUID, pHierarchyBlock->_szLocalHUID);

}


DBSACALLBACK 
DBSA_TriggerCa(
		DA_entryDb*			pAsset
		)
{
	glog(Log::L_NOTICE, "[Callback]	DBSA_TriggerCa");
	
	glog(Log::L_DEBUG, _TAB L"Complex Asset Information:");
	glog(Log::L_DEBUG, _TAB2 L"Type=%ld, UID=%ld(0x%08X), LocalUID=%s", 
		pAsset->_dwEntryType, pAsset->_dwEntryUID, pAsset->_dwEntryUID, pAsset->_szLocalEntryUID);

}


DBSACALLBACK 
DBSA_TriggerState(
		DA_entryDb*			pEntryBlock,
		DA_stateDb*			pStateBlock
		)
{
	glog(Log::L_NOTICE, "[Callback]	DBSA_TriggerState");
	
	glog(Log::L_DEBUG, _TAB L"Entry Information:");
	glog(Log::L_DEBUG, _TAB2 L"Type=%ld, UID=%ld(0x%08X), LocalUID=%s", 
		pEntryBlock->_dwEntryType, pEntryBlock->_dwEntryUID, pEntryBlock->_dwEntryUID, pEntryBlock->_szLocalEntryUID);

	glog(Log::L_DEBUG, _TAB L"State Information:");
	glog(Log::L_DEBUG, _TAB2 L"State=%d", 
		pStateBlock->_dwEntryState);

}
