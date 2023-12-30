// DODDevKit.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>

#include "DODDevKit.h"
#include "basecommand.h"
#include "MessageMacro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CSCMemoryBlockQueue *m_pKitSenderQueue=NULL;
bool m_bConnected;

static AFX_EXTENSION_MODULE DODDevKitDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("DODDevKit.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(DODDevKitDLL, hInstance))
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

		new CDynLinkLibrary(DODDevKitDLL);

		// Sockets initialization
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove the following lines from DllMain and put them in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.
		if (!AfxSocketInit())
		{
			return FALSE;
		}
	
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("DODDevKit.DLL Terminating!\n");

		// Terminate the library before destructors are called
		AfxTermExtensionModule(DODDevKitDLL);
	}
	return 1;   // ok
}

CDODDevKit::CDODDevKit(char * logPath)
{
	m_pSingleConnect=NULL;
	m_Parser=NULL;
	m_CcommandList=new CPtrList;
	m_pKitSenderQueue = new CSCMemoryBlockQueue;

	CString temps;
	if (logPath==NULL)
		temps="DODDevkit.log";
	else
	{
		temps.Format("%s",logPath);
		temps=temps+"\\"+"DODDevkit.log";
	}

	ClogEstablishSettings(temps,LOG_DEBUG,LOGMAXSIZE );
}
CDODDevKit::~CDODDevKit()
{	
	
	try
	{
		// ------------------------------------------------------ Modified by zhenan_ji at 2005年5月18日 17:09:36
		if (m_CcommandList)
		{
			POSITION old1,pos1=m_CcommandList->GetHeadPosition();
			while(old1=pos1)
			{
				CBaseCommand* iteme1=(CBaseCommand *)m_CcommandList->GetNext(pos1);
				if(iteme1)
				{
					iteme1->m_bStop=TRUE;
					Sleep(1);
					delete iteme1;
					iteme1=NULL;
				}
				m_CcommandList->RemoveAt(old1);
			}
			m_CcommandList->RemoveAll();
			delete m_CcommandList;
			m_CcommandList=NULL;
			Clog( LOG_DEBUG, _T("~CDODDevKit: clear commandlist ok !"));
		}
		try
		{
			if(m_pSingleConnect)
			{
				m_pSingleConnect->Stop();
				Sleep(2);
				delete m_pSingleConnect;
				m_pSingleConnect=NULL;
			}

			Clog( LOG_DEBUG, _T("~CDODDevKit: delete m_pSingleConnect ok !"));
		}
		catch (...)
		{
			Clog( LOG_DEBUG, _T("CDODDevKit::~CDODDevKit: delete m_pSingleConnect exception !"));
			return ;
		}
		if( m_pKitSenderQueue )		
		{		
			INT32 size = m_pKitSenderQueue->Size();

			while(size>0)
			{
				CSCMemoryBlock::FreeBlock(m_pKitSenderQueue->Front()->GetBlock());
				m_pKitSenderQueue->Pop();
				size--;
			}

			delete m_pKitSenderQueue;
			m_pKitSenderQueue = NULL;
		}	
		Clog( LOG_DEBUG, _T("~CDODDevKit: clear SenderQueue ok !"));

		if(m_Parser)
		{
			delete m_Parser;
			m_Parser=NULL;
		}
		Clog( LOG_DEBUG, _T("~CDODDevKit:delete  ParserClass ok !"));
	}
	catch (...)
	{
		Clog( LOG_DEBUG, _T("CDODDevKit::~CDODDevKit: delete CBaseCommand exception !"));
		return ;
	}
	// ------------------------------------------------------ Modified by zhenan_ji at 2005年6月1日 9:38:46
	Clog( LOG_DEBUG, _T("CDODDevKit::~CDODDevKit: end!"));
}

int CDODDevKit::setSRMAdd(char *RemoteIP,int Port,BOOL IsSettingIPPort,int nTimeOut)
{
	if (IsSettingIPPort)
	{
		Clog( LOG_DEBUG, _T("DOD Develop Kit:-------------------------------------------!"));
		Clog( LOG_DEBUG, _T("CDODDevKit: set connection SRM address(%s:%d)"),RemoteIP,Port);
		if (strlen(RemoteIP)==0)
		{
			return 1; 
		}
		if (Port<1)
		{
			return 1;
		}

		struIP temp;
		temp.iReConnInterval=1; 

		m_Parser=new CDeviceInfoParser();
		m_Parser->m_CommandList=m_CcommandList;
		m_pSingleConnect=new CSingleConnect(&temp,m_Parser);
		m_pSingleConnect->SetServer(RemoteIP,Port);
		m_pSingleConnect->m_bContinueConnect=TRUE;
		m_pSingleConnect->Start();
	}

	//	Clog( LOG_DEBUG, _T("setSRMAdd Connect Start! "));

	for (int i=0;i<nTimeOut;i++)
	{
		if(m_bConnected)
		{
			return 0;
		}
		else
		{
			Sleep(2);
		}		
	}
	if (i==nTimeOut)
	{
		Clog( LOG_DEBUG, _T("Connect SRM timeout! "));
		return 1;
	}	
	return 0;
}

int CDODDevKit::GetSessionID(int &nSessionID)
{	
	try
	{

		int commandid=m_Parser->GetCommandID();

		CMarkup tmpXmlDOM;
		tmpXmlDOM.AddElem( _T("Message") );
		tmpXmlDOM.IntoElem();
		CString sessionid,scommandid;
		sessionid.Format("%d",22);
		scommandid.Format("%d",commandid);
		//sTmp.Format( "%d", CSingleConnect::s_SingleConnect->wLocalPort );
		tmpXmlDOM.AddElem( _T("MessageHeader") );
		tmpXmlDOM.AddAttrib( _T("MessageCode"), MESSAGECODE_GETSESSIONID);  
		tmpXmlDOM.AddAttrib( _T("SessionID"),sessionid );
		tmpXmlDOM.AddAttrib( _T("CommandID"),scommandid );
		tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
		tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 

		Clog( LOG_DEBUG, _T("Send:GetSessionID msg "));
		m_pSingleConnect->SendXML(tmpXmlDOM);

		CBaseCommand* iteme1=new CBaseCommand;
		iteme1->m_nCommandID=commandid;
		m_CcommandList->AddHead(iteme1);

		if(iteme1->Execute()<0)
		{
			Clog( LOG_DEBUG, _T("Get SessionID msg was overtime"));
			return -1;
		}

		nSessionID=iteme1->m_nSessionID;
		Clog( LOG_DEBUG, _T("return SessionID=%d "),nSessionID);

		if(iteme1)
		{
			delete iteme1;
			iteme1=NULL;
		}

		POSITION old1=m_CcommandList->GetHeadPosition();
		if (old1)
		{
			m_CcommandList->RemoveAt(old1);
		}
	}
	catch (...)
	{
		Clog( LOG_DEBUG, _T("get SessionID exception error=%d  "),GetLastError());
	}
	return 0;
}
int CDODDevKit::UpdateCatalog(int nSessionID,int PortID,int channelID)
{
	int nReturn = 0;
	try
	{

		if (nSessionID<0)
		{
			return 1;
		}
		Clog( LOG_DEBUG, _T("CDODDevKit: UpdateCatalog nSessionID=%d ,channelID=%d"),nSessionID,channelID);

		int commandid=m_Parser->GetCommandID();

		CMarkup tmpXmlDOM;
		tmpXmlDOM.AddElem( _T("Message") );
		tmpXmlDOM.IntoElem();
		//sTmp.Format( "%d", CSingleConnect::s_SingleConnect->wLocalPort );
		tmpXmlDOM.AddElem( _T("MessageHeader") );
		tmpXmlDOM.AddAttrib( _T("MessageCode"), MESSAGECODE_FORCEREDETECTED);  
		tmpXmlDOM.AddAttrib( _T("SessionID"),nSessionID );
		tmpXmlDOM.AddAttrib( _T("CommandID"),commandid );
		tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
		tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 

		tmpXmlDOM.AddElem( _T("MessageBody") );

		tmpXmlDOM.AddAttrib( _T("PortID"), PortID ); 
		tmpXmlDOM.AddAttrib( _T("ChannelID"),channelID ); 
		tmpXmlDOM.AddAttrib( _T("Forced"),1 ); 

		m_pSingleConnect->SendXML(tmpXmlDOM);

		CBaseCommand* iteme1=new CBaseCommand;
		iteme1->m_nCommandID=commandid;
		m_CcommandList->AddHead(iteme1);

		if(iteme1->Execute()<0)
		{
			Clog( LOG_DEBUG, _T("A message about UpdateCatalog  was overtime"));
			return DODUPDATECATALOGERROR;
		}

		nReturn =iteme1->m_nReturn1;
		if(iteme1)
		{
			delete iteme1;
			iteme1=NULL;
		}

		POSITION old1=m_CcommandList->GetHeadPosition();
		if (old1)
		{
			m_CcommandList->RemoveAt(old1);
		}
		Clog( LOG_DEBUG, _T("CDODDevKit: UpdateCatalog nSessionID=%d ,channelID=%d end"),nSessionID,channelID);
	}
	catch (...)
	{
		Clog( LOG_DEBUG, _T("UpdateCatalog nSessionID=%d ,channelID=%d exception ,errorcode=%d"),nSessionID,channelID,GetLastError());
	}
	return nReturn;
}


DWORD CDODDevKit::EnableChannel(int nSessionID,DWORD dwPortID, WORD wPID, BOOL bAnable)
{
	if (nSessionID<0)
	{
		return 1;
	}
	Clog( LOG_DEBUG, _T("CDODDevKit: EnableChannel nSessionID=%d "),nSessionID);

	int commandid=m_Parser->GetCommandID();

	CMarkup tmpXmlDOM;
	tmpXmlDOM.AddElem( _T("Message") );
	tmpXmlDOM.IntoElem();
	//sTmp.Format( "%d", CSingleConnect::s_SingleConnect->wLocalPort );
	tmpXmlDOM.AddElem( _T("MessageHeader") );
	tmpXmlDOM.AddAttrib( _T("MessageCode"), MESSAGECODE_ENABLECHANNEL);  
	tmpXmlDOM.AddAttrib( _T("SessionID"),nSessionID );
	tmpXmlDOM.AddAttrib( _T("CommandID"),commandid );
	tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
	tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 

	tmpXmlDOM.AddElem( _T("MessageBody") );

	tmpXmlDOM.AddAttrib( _T("PortID"), dwPortID ); 
	tmpXmlDOM.AddAttrib( _T("ChannelID"),wPID ); 
	int ee=bAnable;
	tmpXmlDOM.AddAttrib( _T("Enabled"),ee ); 

	m_pSingleConnect->SendXML(tmpXmlDOM);

	CBaseCommand* iteme1=new CBaseCommand;
	iteme1->m_nCommandID=commandid;
	m_CcommandList->AddHead(iteme1);

	if(iteme1->Execute()<0)
	{
		Clog( LOG_DEBUG, _T("A message about EnableChannel  was overtime"));
		return DODENABLECHANNEOVERTIMELERROR;
	}

	int nReturn =iteme1->m_nReturn1;

	if(iteme1)
	{
		delete iteme1;
		iteme1=NULL;
	}

	POSITION old1=m_CcommandList->GetHeadPosition();
	m_CcommandList->RemoveAt(old1);

	return nReturn;
}
DWORD CDODDevKit::SetCatalogName(int nSessionID,DWORD dwPortID, WORD wPID,TCHAR *pszCatalog)
{
	if (nSessionID<0)
	{
		return 1;
	}	
	//Clog( LOG_DEBUG, _T("CDODDevKit: SetCatalogName"));
	Clog( LOG_DEBUG, _T("CDODDevKit: SetCatalogName nSessionID=%d ,channelPID=%d"),nSessionID,wPID);

// ------------------------------------------------------ Modified by zhenan_ji at 2005年5月24日 9:52:00
	int commandid=m_Parser->GetCommandID();

	CMarkup tmpXmlDOM;
	tmpXmlDOM.AddElem( _T("Message") );
	tmpXmlDOM.IntoElem();
	//sTmp.Format( "%d", CSingleConnect::s_SingleConnect->wLocalPort );
	tmpXmlDOM.AddElem( _T("MessageHeader") );
	tmpXmlDOM.AddAttrib( _T("MessageCode"), MESSAGECODE_ENABLECHANNEL);  
	tmpXmlDOM.AddAttrib( _T("SessionID"),nSessionID );
	tmpXmlDOM.AddAttrib( _T("CommandID"),commandid );
	tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
	tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 

	tmpXmlDOM.AddElem( _T("MessageBody") );

	tmpXmlDOM.AddAttrib( _T("PortID"), dwPortID ); 
	tmpXmlDOM.AddAttrib( _T("ChannelID"),wPID ); 
	tmpXmlDOM.AddAttrib( _T("Catalog"),pszCatalog ); 

	m_pSingleConnect->SendXML(tmpXmlDOM);

	CBaseCommand* iteme1=new CBaseCommand;
	iteme1->m_nCommandID=commandid;
	m_CcommandList->AddHead(iteme1);

	if(iteme1->Execute()<0)
	{
		Clog( LOG_DEBUG, _T("A message about SetCatalogName  was overtime"));
		return DODSETCATALOGERROR;
	}

	int nReturn =iteme1->m_nReturn1;

	if(iteme1)
	{
		delete iteme1;
		iteme1=NULL;
	}

	POSITION old1=m_CcommandList->GetHeadPosition();
	m_CcommandList->RemoveAt(old1);

	return nReturn;
}

CString CDODDevKit::GetCurrDateTime()
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	CString sTime;
	sTime.Format("%04d%02d%02d%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	return sTime;
}
DWORD CDODDevKit::OpenPort(int nSessionID,DWORD dwPortID,PPortInfo *PtInfo)
{
	Clog( LOG_DEBUG, _T("OpenPort: nSessionID=%d"),nSessionID);

	if (nSessionID<0)
	{
		return 1;
	}

	if (PtInfo == NULL)
	{
		return 1;
	}

	int commandid=m_Parser->GetCommandID();

	CMarkup tmpXmlDOM;
	tmpXmlDOM.AddElem( _T("Message") );
	tmpXmlDOM.IntoElem();
	CString sessionid,scommandid;
	sessionid.Format("%d",nSessionID);
	scommandid.Format("%d",commandid);
	//sTmp.Format( "%d", CSingleConnect::s_SingleConnect->wLocalPort );
	tmpXmlDOM.AddElem( _T("MessageHeader") );
	tmpXmlDOM.AddAttrib( _T("MessageCode"), MESSAGECODE_OPEN);  
	tmpXmlDOM.AddAttrib( _T("SessionID"),sessionid );
// ------------------------------------------------------ Modified by zhenan_ji at 2005年6月6日 17:54:02
	tmpXmlDOM.AddAttrib( _T("PortID"),dwPortID );

	tmpXmlDOM.AddAttrib( _T("CommandID"),scommandid );
	tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 

	tmpXmlDOM.AddElem( _T("MessageBody") );
	tmpXmlDOM.IntoElem();

	tmpXmlDOM.AddElem( _T("DODPort") );
	tmpXmlDOM.AddAttrib( _T("PMTPID"),PtInfo->wPmtPID);  
	tmpXmlDOM.AddAttrib( _T("TotalBitRate"),PtInfo->wTotalRate );
	tmpXmlDOM.AddAttrib( _T("TmpPath"), PtInfo->szTempPath); 
	tmpXmlDOM.IntoElem();

	tmpXmlDOM.AddElem( _T("IPPort") );
	tmpXmlDOM.IntoElem();

	try
	{

		for( DODIPPORTINFLIST::iterator it = PtInfo->lstIPPortInfo.begin(); it!=PtInfo->lstIPPortInfo.end(); it++ )
		{
			ZQSBFIPPORTINF *IPPortTmp=(ZQSBFIPPORTINF*)&(*it);

			if (IPPortTmp==NULL)
				continue;


			tmpXmlDOM.AddElem( _T("DODIPPort") );

			tmpXmlDOM.AddAttrib( _T("SourceIPAddress"),IPPortTmp->cSourceIp);
			tmpXmlDOM.AddAttrib( _T("SourceIPPort"),IPPortTmp->wSourcePort);
			tmpXmlDOM.AddAttrib( _T("IPAddress"),IPPortTmp->cDestIp );
			tmpXmlDOM.AddAttrib( _T("IPPort"), IPPortTmp->wDestPort); 
			tmpXmlDOM.AddAttrib( _T("SendType"), IPPortTmp->wSendType); 
		}
		tmpXmlDOM.OutOfElem();

		tmpXmlDOM.AddElem( _T("Channel") );
		tmpXmlDOM.IntoElem();
		for( DODCHANNELINFOLIST::iterator it2 = PtInfo->lstChannelInfo.begin(); it2!=PtInfo->lstChannelInfo.end(); it2++ )
		{
			PPChannelInfo *ChannelTmp=(PPChannelInfo*)&(*it2);

			if (ChannelTmp==NULL)
				continue;

			tmpXmlDOM.AddElem( _T("DODChannel") );

			tmpXmlDOM.AddAttrib( _T("ChannelType"),ChannelTmp->wMultiplestream);  
			tmpXmlDOM.AddAttrib( _T("StreamID"),ChannelTmp->wPID);
			tmpXmlDOM.AddAttrib( _T("StreamType"),ChannelTmp->nStreamType);
			tmpXmlDOM.AddAttrib( _T("BitRate"),ChannelTmp->wRate);
			tmpXmlDOM.AddAttrib( _T("path"),ChannelTmp->szPath);
			tmpXmlDOM.AddAttrib( _T("RepeatTime"),ChannelTmp->wRepeatTime);
			tmpXmlDOM.AddAttrib( _T("Tag"),ChannelTmp->cDescriptor);
			tmpXmlDOM.AddAttrib( _T("BeDetectInterval"),ChannelTmp->wBeDetectInterval);
			tmpXmlDOM.AddAttrib( _T("BeEncrypted"),ChannelTmp->wBeEncrypted);
			tmpXmlDOM.AddAttrib( _T("BeDetect"),ChannelTmp->bBeDetect);
			tmpXmlDOM.AddAttrib( _T("DataExchangeType"),ChannelTmp->wChannelType);
			tmpXmlDOM.AddAttrib( _T("StreamCount"),ChannelTmp->wStreamCount);
			tmpXmlDOM.AddAttrib( _T("PackagingMode"),ChannelTmp->wPackagingMode);
		}
	}
	catch (...) 
	{
		Clog( LOG_DEBUG, _T("OpenPort: nSessionID=%d error code=%d"),nSessionID,GetLastError());
		return 1;
	}
	m_pSingleConnect->SendXML(tmpXmlDOM);

	CBaseCommand* iteme1=new CBaseCommand;
	iteme1->m_nCommandID=commandid;
	m_CcommandList->AddHead(iteme1);

	if(iteme1->Execute()<0)
	{
		Clog( LOG_DEBUG, _T("A message about OpenPort  was overtime"));
		return DODSTOPPORTERROR;
	}

	int nReturn =iteme1->m_nReturn1;

	if(iteme1)
	{
		delete iteme1;
		iteme1=NULL;
	}

	POSITION old1=m_CcommandList->GetHeadPosition();
	if (old1)
	{
		m_CcommandList->RemoveAt(old1);
	}

	return nReturn;
}
DWORD CDODDevKit::GetPort(int nSessionID,DWORD dwPortID, PPortInfo* pPortInfo)
{
	int commandid=m_Parser->GetCommandID();

	Clog( LOG_DEBUG, _T("CDODDevKit: GetPort nSessionID=%d "),nSessionID);


	if (pPortInfo == NULL)
	{
		Clog( LOG_DEBUG, _T("CDODDevKit: GetPort nSessionID=%d ,pPortInfo is null error"),nSessionID);
		return 1;
	}
	pPortInfo->lstChannelInfo.clear();
	pPortInfo->lstIPPortInfo.clear();

	CMarkup tmpXmlDOM,XMLReturn;
	tmpXmlDOM.AddElem( _T("Message") );
	tmpXmlDOM.IntoElem();
	//sTmp.Format( "%d", CSingleConnect::s_SingleConnect->wLocalPort );
	tmpXmlDOM.AddElem( _T("MessageHeader") );
	tmpXmlDOM.AddAttrib( _T("MessageCode"), MESSAGECODE_GETPORT);  
	tmpXmlDOM.AddAttrib( _T("SessionID"),nSessionID );
	tmpXmlDOM.AddAttrib( _T("CommandID"),commandid );
	tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
	tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 

	tmpXmlDOM.AddElem( _T("MessageBody") );
	tmpXmlDOM.AddAttrib( _T("PortID"), dwPortID ); 

	m_pSingleConnect->SendXML(tmpXmlDOM);

	CBaseCommand* iteme1=new CBaseCommand;
	iteme1->m_nCommandID=commandid;
	m_CcommandList->AddHead(iteme1);

	if(iteme1->Execute()<0)
	{	
		Clog( LOG_DEBUG, _T("A message about GetPort  was overtime"));
		return DODRUNOVERTIMELERROR;
	}

	int nReturn =0;

	CString str=iteme1->m_strReturn1;

	if (str.GetLength()==0)
	{
		Clog( LOG_DEBUG, _T("A message about GetPort  strReturn.getleng =0"));
		nReturn=1;
	}
	else
	{
		try
		{

			for(int kkk=0;kkk<1;kkk++)
			{
				XMLReturn.SetDoc(iteme1->m_strReturn1.GetBuffer(0));

				if( !XMLReturn.FindElem( MESSAGEBODY ) )
				{
					Clog( LOG_DEBUG, _T("Receive Connect error- MESSAGEBODY was not found in the XML_file"));
					nReturn=1;
					return 1;
				}
				XMLReturn.IntoElem();
				if( !XMLReturn.FindElem( MESSAGEDODPORT ) )
				{
					Clog( LOG_DEBUG, _T("A message MESSAGEDODPORT is not found"));
					nReturn=1;
					break;
				}
				// _T("PMTPID")
				pPortInfo->wPmtPID = _ttoi((LPCTSTR)XMLReturn.GetAttrib( MESSAGEPMTPID ));
				// _T("TotalBitRate")
				pPortInfo->wTotalRate = _ttoi((LPCTSTR)XMLReturn.GetAttrib( MESSAGETOTALBITRATE ));
				// _T("TmpPath")
				CString strTmp = XMLReturn.GetAttrib( MESSAGETMPPATH );
				_tcscpy( pPortInfo->szTempPath, strTmp.GetBuffer( 0 ) );

				XMLReturn.IntoElem();

				// _T("IPPort") 
				if( !XMLReturn.FindElem( MESSAGEIPPORT ) )
				{
					Clog( LOG_DEBUG, _T("A message MESSAGEDODPORT is not found"));
					nReturn=1;
					break;
				}

				// _T("DODIPPort")
				int i=0;

				XMLReturn.IntoElem();

				while( XMLReturn.FindElem( "DODIPPort" ) )
				{
					ZQSBFIPPORTINF PortInfoTmp;
					strTmp = XMLReturn.GetAttrib("IPAddress");
					if (strTmp.GetLength() >0)
					{
						_tcscpy(PortInfoTmp.cDestIp, strTmp.GetBuffer( 0 ) ); 
					}
					else
					{
						_tcscpy( PortInfoTmp.cDestIp, ""); 
					}

					strTmp = XMLReturn.GetAttrib("SourceIPAddress");
					if (strTmp.GetLength() >0)
					{
						_tcscpy( PortInfoTmp.cSourceIp, strTmp.GetBuffer( 0 ) ); 
					}
					else
					{
						_tcscpy( PortInfoTmp.cSourceIp, ""); 
					}



					PortInfoTmp.wSourcePort = _ttoi((LPCTSTR)XMLReturn.GetAttrib("SourceIPPort"));
					PortInfoTmp.wDestPort = _ttoi((LPCTSTR)XMLReturn.GetAttrib("IPPort"));
					PortInfoTmp.wSendType = _ttoi((LPCTSTR)XMLReturn.GetAttrib("SendType"));
					pPortInfo->lstIPPortInfo.push_back(PortInfoTmp);
					i++;
				}
				XMLReturn.OutOfElem();

				pPortInfo->nCastCount=i;
				// _T("IPPort") 
				if( !XMLReturn.FindElem( "Channel" ) )
				{			
					Clog( LOG_DEBUG, _T("A message Channel is not found"));
					nReturn=1;
					break;
				}

				i=0;
				while( XMLReturn.FindChildElem( "DODChannel" ) )
				{
					PPChannelInfo TmpChannel;

					TmpChannel.wChannelType = _ttoi((LPCTSTR)XMLReturn.GetChildAttrib( MESSAGECHANNELTYPE ));
					TmpChannel.wPID = _ttoi((LPCTSTR)XMLReturn.GetChildAttrib( MESSAGESTREAMID ));
					TmpChannel.wRepeatTime = _ttoi((LPCTSTR)XMLReturn.GetChildAttrib( MESSAGEREPEATTIME ));
					strTmp = XMLReturn.GetChildAttrib( MESSAGECHANNELTAG );
					_tcscpy(TmpChannel.cDescriptor, strTmp.GetBuffer(0) );
					TmpChannel.wRate = _ttoi((LPCTSTR)XMLReturn.GetChildAttrib( MESSAGEBITRATE ));
					TmpChannel.nStreamType = _ttoi((LPCTSTR)XMLReturn.GetChildAttrib( MESSAGESTREAMTYPE ));

					TmpChannel.bBeDetect = _ttoi((LPCTSTR)XMLReturn.GetChildAttrib( MESSAGEDETECT ));
					TmpChannel.wBeDetectInterval = _ttoi((LPCTSTR)XMLReturn.GetChildAttrib( MESSAGEDETECTINTERVAL ));
					TmpChannel.wBeEncrypted = _ttoi((LPCTSTR)XMLReturn.GetChildAttrib( MESSAGEENCRYPTED ));

					strTmp = XMLReturn.GetChildAttrib("path");
					_tcscpy( TmpChannel.szPath, strTmp.GetBuffer( 0 ) );
					TmpChannel.bEnable=TRUE;

					pPortInfo->lstChannelInfo.push_back(TmpChannel);
					i++;
				}  
				pPortInfo->wChannelCount = i;   
				pPortInfo->m_nSessionID = nSessionID;
			}
		}
		catch (...) 
		{
			Clog( LOG_DEBUG, _T("CDODDevKit: GetPort nSessionID=%d ,pPortInfo is null error errorcode=%d"),nSessionID,GetLastError());
			nReturn=1;
		}
	}

	if(iteme1)
	{
		delete iteme1;
		iteme1=NULL;
	}

	POSITION old1=m_CcommandList->GetHeadPosition();
	m_CcommandList->RemoveAt(old1);

	return nReturn;
}	

DWORD CDODDevKit::DeleteSession(int nSessionID)
{
	int commandid=m_Parser->GetCommandID();

	Clog( LOG_DEBUG, _T("CDODDevKit: DeleteSession nSessionID=%d"),nSessionID);

	CMarkup tmpXmlDOM;
	tmpXmlDOM.AddElem( _T("Message") );
	tmpXmlDOM.IntoElem();
	//sTmp.Format( "%d", CSingleConnect::s_SingleConnect->wLocalPort );
	tmpXmlDOM.AddElem( _T("MessageHeader") );
	tmpXmlDOM.AddAttrib( _T("MessageCode"), MESSAGECODE_DELETESESSIONID);  
	tmpXmlDOM.AddAttrib( _T("SessionID"),nSessionID );
	tmpXmlDOM.AddAttrib( _T("CommandID"),commandid );
	tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
	tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 

	m_pSingleConnect->SendXML(tmpXmlDOM);

	CBaseCommand* iteme1=new CBaseCommand;
	iteme1->m_nCommandID=commandid;
	m_CcommandList->AddHead(iteme1);

	if(iteme1->Execute()<0)
	{
		Clog( LOG_DEBUG, _T("A message about ClosePort  was overtime"));
		return DODCOCLOSEPORTOVERTIMERMERROR;
	}
	int nReturn =iteme1->m_nReturn1;

	if(iteme1)
	{
		delete iteme1;
		iteme1=NULL;
	}

	POSITION old1=m_CcommandList->GetHeadPosition();
	m_CcommandList->RemoveAt(old1);  

	return nReturn;
}

DWORD CDODDevKit::ClosePort(int nSessionID,DWORD dwPortID)
{
	if (nSessionID<0)
	{
		return 1;
	}
	Clog( LOG_DEBUG, _T("CDODDevKit: ClosePort nSessionID=%d"),nSessionID);

	int commandid=m_Parser->GetCommandID();

	CMarkup tmpXmlDOM;
	tmpXmlDOM.AddElem( _T("Message") );
	tmpXmlDOM.IntoElem();
	//sTmp.Format( "%d", CSingleConnect::s_SingleConnect->wLocalPort );
	tmpXmlDOM.AddElem( _T("MessageHeader") );
	tmpXmlDOM.AddAttrib( _T("MessageCode"), MESSAGECODE_CLOSE);  
	tmpXmlDOM.AddAttrib( _T("SessionID"),nSessionID );
	tmpXmlDOM.AddAttrib( _T("CommandID"),commandid );
	tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
	tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 

	tmpXmlDOM.AddElem( _T("MessageBody") );

	tmpXmlDOM.AddAttrib( _T("PortID"), dwPortID ); 

	m_pSingleConnect->SendXML(tmpXmlDOM);

	CBaseCommand* iteme1=new CBaseCommand;
	iteme1->m_nCommandID=commandid;
	m_CcommandList->AddHead(iteme1);

	if(iteme1->Execute()<0)
	{
		Clog( LOG_DEBUG, _T("A message about ClosePort  was overtime"));
		return DODCOCLOSEPORTOVERTIMERMERROR;
	}
	int nReturn =iteme1->m_nReturn1;

	if(iteme1)
	{
		delete iteme1;
		iteme1=NULL;
	}

	POSITION old1=m_CcommandList->GetHeadPosition();
	m_CcommandList->RemoveAt(old1);    

	DeleteSession(nSessionID);
	Clog( LOG_DEBUG, _T("CDODDevKit: ClosePort OK "));
	return nReturn;
}
DWORD CDODDevKit::RunPort(int nSessionID,DWORD dwPortID)
{
	int commandid=m_Parser->GetCommandID();

	Clog( LOG_DEBUG, _T("CDODDevKit: RunPort nSessionID=%d"),nSessionID);

	CMarkup tmpXmlDOM;
	tmpXmlDOM.AddElem( _T("Message") );
	tmpXmlDOM.IntoElem();
	//sTmp.Format( "%d", CSingleConnect::s_SingleConnect->wLocalPort );
	tmpXmlDOM.AddElem( _T("MessageHeader") );
	tmpXmlDOM.AddAttrib( _T("MessageCode"), MESSAGECODE_RUN);  
	tmpXmlDOM.AddAttrib( _T("SessionID"),nSessionID );
	tmpXmlDOM.AddAttrib( _T("CommandID"),commandid );
	tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
	tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 

	tmpXmlDOM.AddElem( _T("MessageBody") );
	tmpXmlDOM.AddAttrib( _T("PortID"), dwPortID ); 

	m_pSingleConnect->SendXML(tmpXmlDOM);

	CBaseCommand* iteme1=new CBaseCommand;
	iteme1->m_nCommandID=commandid;
	m_CcommandList->AddHead(iteme1);

	if(iteme1->Execute()<0)
	{	
		Clog( LOG_DEBUG, _T("A message about RunPort  was overtime"));
		return DODRUNOVERTIMELERROR;
	}
	
	int nReturn =iteme1->m_nReturn1;

	if(iteme1)
	{
		delete iteme1;
		iteme1=NULL;
	}

	POSITION old1=m_CcommandList->GetHeadPosition();
	m_CcommandList->RemoveAt(old1);
	return nReturn;
}
DWORD CDODDevKit::StopPort(int nSessionID,DWORD dwPortID)
{
	Clog( LOG_DEBUG, _T("StopPort: %d"),nSessionID);

	if (nSessionID<0)
	{
		return 1;
	}
//	Clog( LOG_DEBUG, _T("CDODDevKit: StopPort"));

	int commandid=m_Parser->GetCommandID();

// ------------------------------------------------------ Modified by zhenan_ji at 2005年5月12日 10:17:28

	CMarkup tmpXmlDOM;
	tmpXmlDOM.AddElem( _T("Message") );
	tmpXmlDOM.IntoElem();
	tmpXmlDOM.AddElem( _T("MessageHeader") );
	tmpXmlDOM.AddAttrib( _T("MessageCode"), MESSAGECODE_STOP);  
	tmpXmlDOM.AddAttrib( _T("SessionID"),nSessionID );
	tmpXmlDOM.AddAttrib( _T("CommandID"),commandid );
	tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
	tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 

	tmpXmlDOM.AddElem( _T("MessageBody") );
	tmpXmlDOM.AddAttrib( _T("PortID"), dwPortID ); 

	m_pSingleConnect->SendXML(tmpXmlDOM);

	CBaseCommand* iteme1=new CBaseCommand;
	iteme1->m_nCommandID=commandid;
	m_CcommandList->AddHead(iteme1);

	if(iteme1->Execute()<0)
	{	
		Clog( LOG_DEBUG, _T("A message about StopPort  was overtime"));
		return DODSTOPPORTERROR;
	}
	int nReturn =iteme1->m_nReturn1;

	if(iteme1)
	{
		delete iteme1;
		iteme1=NULL;
	}

	POSITION old1=m_CcommandList->GetHeadPosition();
	m_CcommandList->RemoveAt(old1);
	return nReturn;
}

int CDODDevKit::GetState(int nSessionID,DWORD dwPortID)
{
	Clog( LOG_DEBUG, _T("StopPort: %d"),nSessionID);

	if (nSessionID<0)
	{
		return 1;
	}
	int commandid=m_Parser->GetCommandID();

	// ------------------------------------------------------ Modified by zhenan_ji at 2005年5月12日 10:17:28

	CMarkup tmpXmlDOM;
	tmpXmlDOM.AddElem( _T("Message") );
	tmpXmlDOM.IntoElem();
	tmpXmlDOM.AddElem( _T("MessageHeader") );
	tmpXmlDOM.AddAttrib( _T("MessageCode"), MESSAGECODE_GETPORTSTATE);  
	tmpXmlDOM.AddAttrib( _T("SessionID"),nSessionID );
	tmpXmlDOM.AddAttrib( _T("CommandID"),commandid );
	tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
	tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 

	tmpXmlDOM.AddElem( _T("MessageBody") );
	tmpXmlDOM.AddAttrib( _T("PortID"), dwPortID ); 

	m_pSingleConnect->SendXML(tmpXmlDOM);

	CBaseCommand* iteme1=new CBaseCommand;
	iteme1->m_nCommandID=commandid;
	m_CcommandList->AddHead(iteme1);

	if(iteme1->Execute()<0)
	{	
		Clog( LOG_DEBUG, _T("A message about StopPort  was overtime"));
		return DODSTOPPORTERROR;
	}
	int nReturn =iteme1->m_nReturn1;

	if(iteme1)
	{
		delete iteme1;
		iteme1=NULL;
	}

	POSITION old1=m_CcommandList->GetHeadPosition();
	m_CcommandList->RemoveAt(old1);

	return nReturn;
}
DWORD CDODDevKit::DisConnectSRM()
{
	if(m_pSingleConnect)
	{
		m_pSingleConnect->m_bContinueConnect=FALSE;
	}
	return 0;
}
