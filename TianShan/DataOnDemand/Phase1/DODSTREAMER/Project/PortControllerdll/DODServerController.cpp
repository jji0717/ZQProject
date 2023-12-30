// DODServerController.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>
#include "dodchannel.h"

#include "dodportctrl.h"
#include "DODServerController.h"

#ifdef _DEBUG 
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////////
// Added by Cary
#ifndef _NO_FIX_LOG
extern ISvcLog* service_log;

void setPortControllerLog(ISvcLog* log)
{
	service_log = log;
}
#endif
// Added by Cary
//////////////////////////////////////////////////////////////////////////


typedef ZQSBFCHANNELINFO* PDODBROASCHANNELINFO;
typedef ZQSBFIPPORTINFO* PDODBROASPORTINFO;


static AFX_EXTENSION_MODULE DODServerControllerDLL = { NULL, NULL };

extern "C" int APIENTRY



DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("DODServerController.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(DODServerControllerDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(DODServerControllerDLL);

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("DODServerController.DLL Terminating!\n");

		// Terminate the library before destructors are called
		AfxTermExtensionModule(DODServerControllerDLL);
	}
	return 1;   // ok
}
CDODDeviceController::CDODDeviceController()
{
	m_sLogFileName="";
	m_pCtrlList.clear();

	CoInitialize(NULL);

	CString        strCurDir;
	char           sModuleName[MAX_PATH];
	DWORD dSize = GetModuleFileName(NULL,sModuleName,MAX_PATH);
	sModuleName[dSize] = '\0';
	strCurDir = sModuleName;
	int nIndex = strCurDir.ReverseFind('\\');
	m_sDirName = strCurDir.Left(nIndex); 

	m_sPathForArrange=m_sDirName;
	m_sDirName+="\\ts";
	CreateDirectory(m_sDirName,NULL);	

	m_sPathForArrange+="\\Arrange";
	CreateDirectory(m_sPathForArrange,NULL);	

	//end with "\\"
	//CoInitializeEx(NULL,COINIT_APARTMENTTHREADED );
}
CDODDeviceController::~CDODDeviceController() 
{
	Clog( LOG_DEBUG, _T("PortController .call ~CDODDeviceControlle start"));

	CDODPortCtrl* pTempPort = NULL;
	try
	{
		while( !m_pCtrlList.empty( ) )
		{
			pTempPort = m_pCtrlList.front( );
			if (pTempPort)
			{
				pTempPort->ReleaseAll();
			}
			m_pCtrlList.pop_front();
			Clog( LOG_DEBUG, _T("~PortController .m_pCtrlList.pop_front OK"));

			delete pTempPort;
			Clog( LOG_DEBUG, _T("~PortController .delete pTempPort OK"));

			pTempPort = NULL;
			//Clog( LOG_DEBUG, _T("~PortController .pTempPort = NULL"));

		}
	}
	catch (...) 
	{
		Clog( LOG_ERR, _T("CDODDeviceController::~CDODDeviceController() exception errorcode=%d "),GetLastError());
	}
	CoUninitialize();
	Clog( LOG_DEBUG, _T("PortController .call ~CDODDeviceControlle OK"));
}

DWORD CDODDeviceController::ReleaseAll()
{
/*
	CDODPortCtrl* pTempPort = NULL;
	int id;
	while( !m_pCtrlList.empty( ) )
	{
		pTempPort = m_pCtrlList.front( );
		if (pTempPort)
		{
			id=pTempPort->m_dwID;
			StopPort(id);
			ClosePort(id);
		}
		m_pCtrlList.pop_front();
		delete pTempPort;
		pTempPort = NULL;
	}*/
	return 0;
}
DWORD CDODDeviceController::SetLogFile( LPCTSTR szLogFile )
{
	m_sLogFileName=szLogFile;
	return 0;
}

DWORD CDODDeviceController::Initialize(char* szConfigFile)
{
	ClogEstablishSettings(m_sLogFileName,LOG_DEBUG,LOGFILE_SIZE );

//	memset((void *)&m_DefaultPortInfo,0,sizeof(PPortInfo));
//	m_DefaultPortInfo.wChannelCount=0;
//	m_DefaultPortInfo.cDestIp[0]='\0';
//	strcpy(m_DefaultPortInfo.cSourceIp,"192.168.80.5");
//	m_DefaultPortInfo.wSourcePort=1254;
	return 0;
}

DWORD CDODDeviceController::Parse( CSCMemoryBlock block )
{ 
	return 0;
}

// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê3ÔÂ29ÈÕ 11:09:53

DWORD  CDODDeviceController::GetSessionID(DWORD nSessionID, DWORD *psessionid)
{
	CDODPortCtrl *TempCtrl=NULL;

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	*psessionid=TempCtrl->m_nSessionID;
	return 0;
}
DWORD CDODDeviceController::GetPort(DWORD nSessionID, PPortInfo* pPortInfo)
{
	CDODPortCtrl *TempCtrl=NULL;

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	if (pPortInfo==NULL)
	{
		return 1;

	}
	//*pPortInfo=TempCtrl->m_PortInfo;//	int len=sizeof((TempCtrl)->m_PortInfo);	memcpy(pPortInfo,(void *)&((TempCtrl)->m_PortInfo),len);

	return 0;
}
BOOL CDODDeviceController::FoundnSessionIDFromList(DWORD nID,CDODPortCtrl **pCtrl)
{
	PORTCTRLLIST::iterator it;

	for( it = m_pCtrlList.begin(); it != m_pCtrlList.end();it++)
	{
		if ((*it)==NULL)
		{
			return FALSE;
		}
		if (((*it)->m_dwID) ==nID)
		{
			*pCtrl=(*it);
			return TRUE;
		}
	}
	return FALSE;
}

//Open, Close, Pause, Run, Stop port function.
//Return valuse is 0 indicates that it is successful.
DWORD CDODDeviceController::OpenPort(DWORD nSessionID,PPortInfo *PtInfo)
{
	Clog( LOG_DEBUG, _T("PortController .OpenPort() nSessionID=%d, PID = %d"),
		nSessionID, PtInfo->wPmtPID);

	CDODPortCtrl *TempCtrl=NULL;
	if(FoundnSessionIDFromList(nSessionID,&TempCtrl)==TRUE)
	{
		Clog( LOG_ERR, _T("PortInfo.nSessionID is used overlap.error!"));
		return 1;
	}

	if (PtInfo == NULL)
	{
		Clog( LOG_ERR, _T("PortInfo.PtInfo null.error!"));
		return 1;
	}
	HRESULT hr = S_OK;

	CDODPortCtrl *pCtrl=new CDODPortCtrl;
	pCtrl->m_dwCasterCount=(DWORD)(PtInfo->lstIPPortInfo.size());

	Clog( LOG_DEBUG, _T("PortController .OpenPort() nSessionID=%d ,m_dwCasterCount = %d"),nSessionID,pCtrl->m_dwCasterCount);

	if(pCtrl->m_dwCasterCount<=0)
	{
		Clog( LOG_ERR, _T("pCtrl->m_dwCasterCount <0 is Error."));
		return 1;
	}
	else
	{
        pCtrl->m_pCaster=new PDODBROASPORTINFO[pCtrl->m_dwCasterCount];
	}

	//Clog( LOG_DEBUG, _T("PortController .OpenPort() new nCastCount"));
	int i=0;

	try
	{
		for( DODIPPORTINFLIST::iterator it = PtInfo->lstIPPortInfo.begin(); it!=PtInfo->lstIPPortInfo.end(); it++ )
		{
			ZQSBFIPPORTINF *IPPortTmp=(ZQSBFIPPORTINF*)&(*it);

			if (IPPortTmp==NULL)
				continue;

			ZQSBFIPPORTINFO *ipinfo=new ZQSBFIPPORTINFO;
			strcpy(ipinfo->cSourceIp,IPPortTmp->cSourceIp);
			ipinfo->wSourcePort=IPPortTmp->wSourcePort;
			if(ipinfo->wSourcePort <1)
				ipinfo->wSourcePort=0x200;
			ipinfo->wSendType=IPPortTmp->wSendType;

			ipinfo->wDestPort=IPPortTmp->wDestPort;
			if(ipinfo->wDestPort <1)
			{
				ipinfo->wDestPort=0;
				Clog(LOG_ERR,"OpenPort() nSessionID=%d wDestPort is zero error",nSessionID);
			}
			//TRACE("\nIPPort= %d",);
			//TRACE("\nIPPort= %s",m_XmlDOM.GetAttrib(_T("IPAddress")));
			//CString ssTmp; ssTmp=m_XmlDOM.GetAttrib(_T("IPAddress"));
			strcpy(ipinfo->cDestIp,IPPortTmp->cDestIp);
			if(strlen(ipinfo->cDestIp)<1)
			{
				strcpy(ipinfo->cDestIp,"");
				Clog(LOG_ERR,"OpenPort() nSessionID=%d destIP is null error",nSessionID);
			}

			pCtrl->m_pCaster[i]=ipinfo;
			i++;
			//Clog( LOG_DEBUG, _T("PortController .OpenPort() nSessionID=%d ,IPPort_i = %d"),nSessionID,i);
		}		
	}
	catch (...) 
	{
		Clog(LOG_ERR,"OpenPort() nSessionID=%d parse IPPort error code",nSessionID,GetLastError());
		return 1;
	}
	pCtrl->m_dwPmtPID=PtInfo->wPmtPID;
 
	pCtrl->m_dwID=nSessionID;

	pCtrl->m_dwTotalBitRate=PtInfo->wTotalRate;
	if(pCtrl->m_dwTotalBitRate <1)
		pCtrl->m_dwTotalBitRate=0;

	//Clog( LOG_DEBUG, _T("PortController .OpenPort() :wTotalRate"));

	strcpy(pCtrl->m_szBCFTempFilePath,m_sDirName.GetBuffer(0));
	strcpy(pCtrl->m_sArrangePath,m_sPathForArrange.GetBuffer(0));

//	pCtrl->m_dwChannelCount=1;
	pCtrl->m_dwChannelCount=(DWORD)(PtInfo->lstChannelInfo.size());
	int cardNumber=pCtrl->m_dwChannelCount;
	Clog( LOG_DEBUG, _T("PortController .OpenPort() nSessionID=%d ,m_dwCasterCount = %d"),nSessionID,cardNumber);

	if(cardNumber<=0)
	{
		Clog( LOG_ERR, _T("PortInfo.wChannelCount<=0 is Error."));
		return 1;
	}
	else
	{
		pCtrl->m_cchannelinfo=new PDODBROASCHANNELINFO[cardNumber];
	}

	//Clog( LOG_DEBUG, _T("PortController .OpenPort() :wChannelCount"));
	i=0;
	try
	{

		for( DODCHANNELINFOLIST::iterator it2 = PtInfo->lstChannelInfo.begin(); it2!=PtInfo->lstChannelInfo.end(); it2++ )
		{
			PPChannelInfo *ChannelTmp=(PPChannelInfo*)&(*it2);

			if (ChannelTmp==NULL)
				continue;
			ZQSBFCHANNELINFO *pinfo=new ZQSBFCHANNELINFO;

			pinfo->wChannelType=ChannelTmp->wChannelType;

			pinfo->wPID=ChannelTmp->wPID;

			pinfo->wRepeatTime=ChannelTmp->wRepeatTime;

			strcpy(pinfo->cDescriptor,ChannelTmp->cDescriptor); 

			strcpy(pinfo->szPath,ChannelTmp->szPath); 

			pinfo->wAccessType=1;
			pinfo->wStreamCount=ChannelTmp->wStreamCount;
			pinfo->wDataExchangeType=ChannelTmp->wDataExchangeType;
			pinfo->bEnable=ChannelTmp->bEnable;
			pinfo->wSendWithDestination=ChannelTmp->wPackagingMode;
			pinfo->wRate=ChannelTmp->wRate; 

			//Clog( LOG_DEBUG, _T("OpenPort::current channel ,pid=%d.,rate=%d,pinfo->wRate=%d"),ChannelTmp->wPID,ChannelTmp->wRate,pinfo->wRate);

			pinfo->bBeDetect=ChannelTmp->bBeDetect==1?TRUE:FALSE;

			pinfo->nStreamType=ChannelTmp->nStreamType;

			//Clog( LOG_DEBUG, _T("ChannelTmp->wStreamCount is %d."),ChannelTmp->wStreamCount);

			if (ChannelTmp->nStreamType <1)
			{
				Clog( LOG_ERR, _T("ChannelTmp->nStreamType is error channelPID=%d nStreamType=%d."),pinfo->wPID,pinfo->nStreamType);
				return hr;
			}

			pinfo->wBeDetectInterval=ChannelTmp->wBeDetectInterval;
			pinfo->wBeEncrypted=ChannelTmp->wBeEncrypted;
			pCtrl->m_cchannelinfo[i]=pinfo;
			i++;
			//Clog( LOG_DEBUG, _T("PortController .OpenPort() nSessionID=%d ,channel_i = %d"),nSessionID,i);
		}
	}
	catch (...) 
	{
		Clog(LOG_ERR,"OpenPort() nSessionID=%d parse ChannelInfo error code",nSessionID,GetLastError());
		return 1;
	}


	pCtrl->m_nSessionID=PtInfo->m_nSessionID;
	Clog( LOG_DEBUG, _T("PortController:BuildGraphManager start !"));

	if((hr=pCtrl->BuildGraphManager())!=0)
	{
		Clog( LOG_ERR, _T("pCtrl.BuildGraphManager() Error."));
		return 1;
	}
	if((hr=pCtrl->ApplyChannelParameter())!=0)
	{
		Clog( LOG_ERR, _T("pCtrl.ApplyChannelParameter Error."));
		return 1;
	}

	//pCtrl->m_PortInfo= new PPortInfo;
//	pCtrl->m_PortInfo=PortInfo;

	pCtrl->m_nState=STATEPortopened;
	
	m_pCtrlList.push_back(pCtrl);
	
	Clog( LOG_DEBUG, _T("PortController .OpenPort is successful sessionID=%d"),nSessionID);

	return 0;
}



//DWORD CDODDeviceController::GetCardBandWidth(DWORD dwCardID, LONGLONG *pllCardBandWidth){	if(dwCardID>=m_nDODPortCtrlNumber) 		return 1; 	return 0;}
//DWORD CDODDeviceController::GetIPPortBandWidth(DWORD dwCardID,char* szIpAddress, LONGLONG* pllIPPortBandWidth){if(dwCardID>=m_nDODPortCtrlNumber) return 1; return 0;}
//DWORD CDODDeviceController::GetCardIPPorts(DWORD dwCardID, IPPORTLIST* pIPPortList){if(dwCardID>=m_nDODPortCtrlNumber) return 1; return 0;}
//DWORD CDODDeviceController::GetUsedIPPorts(DWORD dwCardID, IPPORTLIST* pIPPortList){if(dwCardID>=m_nDODPortCtrlNumber) return 1; return 0;}


//Functions to set and get specified port information.
//0 indicates it is successful.
DWORD CDODDeviceController::GetPortState(DWORD nSessionID,DWORD *pdwPortState)
{
	Clog( LOG_DEBUG, _T("PortController .GetPortState nSessionID=%d"),nSessionID);

	CDODPortCtrl *TempCtrl=NULL;

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	//*pdwPortState=m_pDODPortCtrl[nSessionID]->m_state;
	return TempCtrl->m_nState;
}

DWORD CDODDeviceController::GetTotalBitRate(DWORD nSessionID, DWORD *pdwBitRate)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .GetTotalBitRate nSessionID=%d"),nSessionID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	*pdwBitRate=TempCtrl->m_dwTotalBitRate;

	return 0;
}
/*
DWORD CDODDeviceController::GetTmpFilePath(DWORD nSessionID, char* szPath, int nLen)
{
	if(nSessionID>=m_nDODPortCtrlNumber) 
		return 1; 
	
	if(m_pDODPortCtrl[nSessionID]==NULL)
		return 1;

	return 0;
}
DWORD CDODDeviceController::SetTmpFilePath(DWORD nSessionID, char* szPath){if(nSessionID>=m_nDODPortCtrlNumber) return 1; return 0;}*/
//Functions to interactive with Channel

//DWORD CDODDeviceController::GetChannel(DWORD nSessionID, WORD wPMTID, CDODChannel* pDODChannel){if(nSessionID>=m_nDODPortCtrlNumber) return 1; return 0;}
//DWORD CDODDeviceController::GetSubChannel(DWORD nSessionID,WORD wPMTID,pSubchannellist* pDODSubChannelList){if(nSessionID>=m_nDODPortCtrlNumber) return 1; return 0;}

DWORD CDODDeviceController::SetChannelDetectInterval(DWORD nSessionID, WORD wPID, long dwInterval)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .SetChannelDetectInterval nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->SetDetectedInterval(i,dwInterval);	
		}
	}

	return 0;
}

DWORD CDODDeviceController::GetChannelDetectInterval(DWORD nSessionID, WORD wPID, long* pdwInterval)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .GetChannelDetectInterval nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);
		if((WORD)pid == wPID)
		{
			return TempCtrl->GetDetectedInterval(i,pdwInterval);	
		}
	}

	return 0;
}

//DWORD CDODDeviceController::ForceChannelRedetect(DWORD nSessionID, WORD wPID){if(nSessionID>=m_nDODPortCtrlNumber) return 1; return 0;}

DWORD CDODDeviceController::AddChannelVersion(DWORD nSessionID, WORD wPID)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .AddChannelVersion nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->AddVersionNumber(i);				
		}
	}

	return 0;
}

DWORD CDODDeviceController::ResetChannelVersion(DWORD nSessionID, WORD wPID)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .ResetChannelVersion nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->ResetVersionNumber(i);				
		}
	}
	
	return 0;
}

DWORD CDODDeviceController::SetChannelVersion(DWORD nSessionID, WORD wPID,const char * inObjectKey, BYTE inObjectKeyLen, BYTE inVersion)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .SetChannelVersion nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->SetVersionNumber(i,inObjectKey,inObjectKeyLen,inVersion);				
		}
	}

	return 0;
}
DWORD CDODDeviceController::GetChannelVersion(DWORD nSessionID, WORD wPID, const char * inObjectKey, BYTE inObjectKeyLen, BYTE * outVersion)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .GetChannelVersion nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);


		if((WORD)pid == wPID)
		{
			return TempCtrl->GetVersionNumber(i,inObjectKey,inObjectKeyLen,outVersion);				
		}
	}

	return 0;
}

DWORD CDODDeviceController::SetObjectKeyLength(DWORD nSessionID, WORD wPID, BYTE dwObjKeyLen)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .SetObjectKeyLength nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->SetObjectKeyLength(i,dwObjKeyLen);				
		}
	}

	return 0;
}

DWORD CDODDeviceController::GetObjectKeyLength(DWORD nSessionID, WORD wPID, BYTE* pdwObjKeyLen)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .GetObjectKeyLength nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->GetObjectKeyLength(i,pdwObjKeyLen);				
		}
	}

	return 0;
}
DWORD CDODDeviceController::SetChannelIdxDesLen(DWORD nSessionID, WORD wPID, BYTE dwIdxDesLen)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .SetChannelIdxDesLen nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->SetIndexDescLength(i,dwIdxDesLen);				
		}
	}

	return 0;
}

DWORD CDODDeviceController::GetChannelIdxDesLen(DWORD nSessionID, WORD wPID, BYTE* pdwIdxDesLen)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .GetChannelIdxDesLen nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->GetIndexDescLength(i,pdwIdxDesLen);				
		}
	}

	return 0;
}

DWORD CDODDeviceController::EnableChannel(DWORD nSessionID, WORD wPID, BOOL bAnable)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .EnableChannel nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);


		if((WORD)pid == wPID)
		{
			return TempCtrl->EnablePin(wPID,bAnable);				
		}
	}

	return 0;
}
/*
DWORD CDODDeviceController::SetChannelBitRate(DWORD nSessionID, WORD wPID, DWORD dwBitRate)
{
	if(nSessionID>=m_nDODPortCtrlNumber) 
		return 1; 

	if(m_pDODPortCtrl[nSessionID]==NULL)
		return 1;

	for(int i=0;i<(int)(m_pDODPortCtrl[nSessionID]->m_dwChannelCount);i++)
	{
		long pid;m_pDODPortCtrl[nSessionID]->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			CDODPortCtrl * ppp=m_pDODPortCtrl[nSessionID];
			ppp->s(i,bAnable);		



		}
	}

	return 0;
}
*/
//DWORD CDODDeviceController::GetChannelBitRate(DWORD nSessionID, WORD wPID, DWORD* pdwBitRate){if(nSessionID>=m_nDODPortCtrlNumber) return 1; return 0;}

DWORD CDODDeviceController::ClosePort(DWORD nSessionID)
{
	Clog( LOG_DEBUG, _T("PortController .ClosePort nSessionID=%d"),nSessionID);
	PORTCTRLLIST::iterator it;

	try
	{
		for( it = m_pCtrlList.begin(); it != m_pCtrlList.end();it++)
		{
			if ((*it)==NULL)
			{
				return 1;
			}
			if (((*it)->m_dwID)  ==nSessionID)
			{
				(*it)->ReleaseAll();
				delete (*it);
				(*it) = NULL;
				Clog( LOG_DEBUG, _T(".ClosePort,nSessionID=%d"),nSessionID);
			}
		}

		m_pCtrlList.remove(NULL);
	}
	catch (...) {
		Clog( LOG_DEBUG, _T("CDODDeviceController::ClosePort exception!"));

	}

	return 0;
}

DWORD CDODDeviceController:: PausePort(DWORD nSessionID)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .EnableChannel nSessionID=%d "),nSessionID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	if(TempCtrl->m_pMC)
	{
		HRESULT hr = TempCtrl->m_pMC->Pause();
		if (hr != S_OK) {
			glog(ISvcLog::L_ERROR, 
				"%s:\tIMediaControl::PausePort(%d) failed(hr = 0x%08x(%d)", 
				__FUNCTION__, nSessionID, hr, hr);
		}
		return hr;
	}

	return S_FALSE;
}

DWORD CDODDeviceController::RunPort(DWORD nSessionID)
{
	Clog( LOG_DEBUG, _T("PortController .RunPort() nSessionID=%d"),
		nSessionID);

	CDODPortCtrl *TempCtrl=NULL;

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return S_FALSE;
	}

	if(TempCtrl->m_pMC)
	{
		TempCtrl->m_nState=STATEPortRunning;
		HRESULT hr = TempCtrl->m_pMC->Run();
		if (hr != S_OK) {

			glog(ISvcLog::L_ERROR, 
				"%s:\tIMediaControl::RunPort(%d) failed(hr = 0x%08x(%d)", 
				__FUNCTION__, nSessionID, hr, hr);

		} else {

			glog(ISvcLog::L_INFO, 
				"%s:\tIMediaControl::RunPort(%d) ok", 
				__FUNCTION__, nSessionID);

		}

		return hr;
	} else {
		glog(ISvcLog::L_ERROR, "%s:\tTempCtrl->m_pMC == NULL)", __FUNCTION__);
		return S_FALSE;
	}
}

DWORD CDODDeviceController::StopPort(DWORD nSessionID)
{
	Clog( LOG_DEBUG, _T("PortController .StopPort() nSessionID=%d"),
		nSessionID);

	try
	{
		CDODPortCtrl *TempCtrl=NULL;

		if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
		{
			Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
				__FUNCTION__, nSessionID);
			return 1;
		}

		if((TempCtrl)->m_pMC)
		{
			(TempCtrl)->m_nState=STATEPortStop;

			Clog( LOG_DEBUG, _T("StopPort find out,nSessionID=%d"),nSessionID);
			HRESULT hr = (TempCtrl)->m_pMC->Stop();
			if (hr != S_OK) {
				glog(ISvcLog::L_CRIT, "%s:\tIMediaControl::PausePort() failed(hr = %x(%d)", 
					__FUNCTION__, hr, hr);
			}
			Clog( LOG_DEBUG, _T("StopPort end,nSessionID=%d"),nSessionID);
			return hr;
		}
	}
	catch (...) {
		Clog( LOG_ERR, _T("CDODDeviceController::ClosePort exception!"));

	}

	return S_FALSE;
}

DWORD CDODDeviceController::SetCatalogName(DWORD nSessionID, WORD wPID,TCHAR *pszCatalog)
{
	Clog( LOG_DEBUG, _T("PortController .SetCatalogName nSessionID=%d wPID=%d"),nSessionID,wPID);
	CDODPortCtrl *TempCtrl=NULL;

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);
		if((WORD)pid == wPID)
		{
			//CDODPortCtrl * ppp=m_pDODPortCtrl[nSessionID];	
			return TempCtrl->m_pChannel[i]->SetCatalogName(pszCatalog);
		}
	}
	return 0;
}
DWORD CDODDeviceController::GetCatalogName(DWORD nSessionID, WORD wPID,TCHAR *pszCatalog)
{
	Clog( LOG_DEBUG, _T("PortController .GetCatalogName nSessionID=%d wPID=%d"),nSessionID,wPID);

	CDODPortCtrl *TempCtrl=NULL;

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);
		if((WORD)pid == wPID)
		{
			//CDODPortCtrl * ppp=m_pDODPortCtrl[nSessionID];	
			return TempCtrl->m_pChannel[i]->GetCatalogName(pszCatalog);
		}
	}

	return 0;
}
DWORD CDODDeviceController::UpdateCatalog(DWORD nSessionID, WORD wPID)
{
	Clog( LOG_DEBUG, _T("PortController .UpdateCatalog nSessionID=%d wPID=%d"),nSessionID,wPID);

	CDODPortCtrl *TempCtrl=NULL;

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->m_pChannel[i]->UpdateCatalog();
		}
	}
	return 0;
}

DWORD CDODDeviceController::SetDetectedFlag(DWORD nSessionID, WORD wPID,BOOL bDetected)
{
	Clog( LOG_DEBUG, _T("PortController .SetDetectedFlag nSessionID=%d wPID=%d"),nSessionID,wPID);

	CDODPortCtrl *TempCtrl=NULL;

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->m_pChannel[i]->SetDetectedFlag(bDetected);
		}
	}
	return 0;
}

DWORD CDODDeviceController::GetDetectedFlag (DWORD nSessionID, WORD wPID,BOOL* bDetected)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .GetDetectedFlag nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->m_pChannel[i]->GetDetectedFlag(bDetected);
		}
	}
	return 0;
}

DWORD CDODDeviceController::SetChannelEvent(DWORD nSessionID, WORD wPID,HANDLE hStateEvent)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .SetChannelEvent nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);


		if((WORD)pid == wPID)
		{
			return TempCtrl->m_pChannel[i]->SetChannelEvent(hStateEvent);
		}
	}
	return 0;
}
//// ICatalogState methods 

DWORD CDODDeviceController::SetCatalogStateEvent(DWORD nSessionID, WORD wPID,HANDLE hStateEvent)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .SetCatalogStateEvent nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->m_pChannel[i]->SetCatalogStateEvent(hStateEvent);
		}
	}
	return 0;
}

DWORD CDODDeviceController:: GetLastErrorCode(DWORD nSessionID, WORD wPID,int* nError)
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .GetLastErrorCode nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->m_pChannel[i]->GetLastErrorCode(nError);
		}
	}
	return 0;
}

DWORD CDODDeviceController:: GetCurrentStatus(DWORD nSessionID, WORD wPID,int* pnStatus)
{
	Clog( LOG_DEBUG, _T("PortController .GetCurrentStatus nSessionID=%d wPID=%d"),nSessionID,wPID);

	CDODPortCtrl *TempCtrl=NULL;

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);

		if((WORD)pid == wPID)
		{
			return TempCtrl->m_pChannel[i]->GetCurrentStatus(pnStatus);
		}
	}
	return 0;
}


DWORD CDODDeviceController::SetWrapParaSet( DWORD nSessionID, WORD wPID,DW_ParameterSet* inPara )
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .SetWrapParaSet nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);


		if((WORD)pid == wPID)
		{
			return TempCtrl->m_pChannel[i]->SetWrapParaSet(*inPara);
		}
	}
	return 0;
}
DWORD CDODDeviceController::GetWrapParaSet( DWORD nSessionID, WORD wPID,DW_ParameterSet * outPara )
{
	CDODPortCtrl *TempCtrl=NULL;
	Clog( LOG_DEBUG, _T("PortController .GetWrapParaSet nSessionID=%d wPID=%d"),nSessionID,wPID);

	if (FoundnSessionIDFromList(nSessionID,&TempCtrl)==FALSE)
	{
		Clog( LOG_ERR, _T("%s:\ta nSessionID(%d) of CDODPortCtrl is not found"),
			__FUNCTION__, nSessionID);
		return 1;
	}

	for(int i=0;i<(int)(TempCtrl->m_dwChannelCount);i++)
	{
		long pid;TempCtrl->m_pChannel[i]->GetChannelPID(&pid);


		if((WORD)pid == wPID)
		{
			return TempCtrl->m_pChannel[i]->GetWrapParaSet(outPara);
		}
	}
	return 0;
}




