
/*
**	FILENAME			DeviceInfoParser.cpp
**
**	PURPOSE				The file is a .cpp file, used along with DeviceInfoParser.h.
**						a pure-virtual class CDeviceInfoParser is declared in the file, and this class is used to provide a 
**						interface for parsing packages that are got from socket connecting with AM.
**						
**						
**
**	CREATION DATE		19-07-2004
**	LAST MODIFICATION	21-07-2004
**
**	AUTHOR				Leon.li (Interactive ZQ)
**
**
*/

#include "StdAfx.h"
#include "clog.h"
#include "DeviceInfoParser.h"
#include "common.h"
//#include "msxmldom.h"
#include "Markup.h"
//#include "SingleConnect.h"
#include "ListenManager.h"
#include "MessageMacro.h"

struIP		m_struIP;

CRITICAL_SECTION	m_csSendProtect;		// protect senddata

LONG g_DeviceID[MAXDEVICE];
LONG g_TransportID[MAXTRANSPORT];
LONG g_PortID[MAXPORT];

void WriteLog(char *fmt, ...)
{
    char buf[1024];
    va_list list;
	va_start(list, fmt);
	vsprintf(buf, fmt, list);
	Clog(LOG_DEBUG, buf);
	va_end(list);
}

int ReportStatus(LONG lDeviceID, LONG lTransportID, LONG lPortID, LONG lStatus)
{
	Clog( LOG_DEBUG, _T("Enter - CSCNVODReporter::Status()"));
	//lDeviceID = (unsigned)lDeviceID;
	//int i1 = lDeviceID<<16;
	//int i2 = lTransportID<<8;
	//int i3 = i1+i2+lPortID;
	//int lTmp = MAKEDEVICEID( lDeviceID, lTransportID, lPortID );
	//i1 = GETDEVICEID(lTmp);
	//i2 = GETTRANSPORTID(lTmp);
	//i3 = GETPORTID(lTmp);
	
	CDeviceInfoParser::GEN_NUMBER++;
	
	CMarkup tmpXmlDom;
	//CDeviceInfoParser::s_pDeviceInfoParser->AddMessageHeader( tmpXmlDom, 0, CDeviceInfoParser::GEN_NUMBER, MESSAGECODE_PORTREPORT );
	
	CString sTmp = _T("");
	WORD wStatus = ((WORD)lStatus << 8) + (WORD)lPortID;
	
	tmpXmlDom.AddElem( _T("Controller") );
	sTmp.Format( _T("%d"), lDeviceID );
	tmpXmlDom.AddAttrib( _T("DeviceID"), sTmp );
	sTmp.Format( _T("%d"), lTransportID );
	tmpXmlDom.AddAttrib( _T("TransportID"), sTmp );
	//sTmp.Format( _T("%d"), MAKEPORTID( lTransportID, lPortID ) );
	sTmp.Format( _T("%d"), lPortID );
	tmpXmlDom.AddAttrib( _T("PortID"), sTmp );
	tmpXmlDom.AddAttrib( _T("StatusFlag"), _T("1") );
	tmpXmlDom.AddAttrib( _T("Ack"), _T("1") );
	sTmp.Format( _T("%d"), wStatus );
	tmpXmlDom.AddAttrib( _T("StatusCode"), sTmp );
	
	CDeviceInfoParser::s_pDeviceInfoParser->SendMessage( tmpXmlDom );
	
	// update status in ListCtrl in the dialog.
	//if( CDeviceInfoParser::s_pDeviceInfoParser->m_bPermitStatus )
	//::SendMessage( CDeviceInfoParser::s_pDeviceInfoParser->m_hWnd, UPDATEITEMTOLISTCTRL, MAKEDEVICEID( lDeviceID, lTransportID, lPortID ), lStatus );
	
	//TRACE( _T("UpdatePortStatus MessageCode: %d. DeviceID: %d MAKEPORTID: %d (TransportID: %d PortID: %d)  Status: 0x%X \r\n"), MESSAGECODE_PORTREPORT, lDeviceID, MAKEPORTID( lTransportID, lPortID ), lTransportID, lPortID, lStatus );	
	Clog( LOG_DEBUG, _T("UpdatePortStatus. DeviceID: %d MAKEPORTID: %d (TransportID: %d PortID: %d)  Status: 0x%X \r\n"), lDeviceID, MAKEPORTID( lTransportID, lPortID ), lTransportID, lPortID, lStatus );		
	
	return 0;
}

int ReportError(LONG lDeviceID, LONG lTransportID, LONG lPortID, DWORD dwError, LPCTSTR strMsg)
{
	ASSERT( strMsg != NULL );
	Clog( LOG_DEBUG, _T("Enter - CSCNVODReporter::Error()"));
	
	if( lDeviceID >= 0 && lTransportID >= 0 && lPortID >= 0 )
	{
		
		CDeviceInfoParser::GEN_NUMBER++;
		
		CMarkup tmpXmlDom;
		//CDeviceInfoParser::s_pDeviceInfoParser->AddMessageHeader( tmpXmlDom, 0, CDeviceInfoParser::GEN_NUMBER, MESSAGECODE_PORTERROR );
		
		//XmlError.Format(_T("<Message Time=\"%s\" Flag=\"0\" NO=\"%u\" Source=\"%u\" Code=\"%u\">"),
		//	CSCUtils::TimeToString(time(NULL)), GEN_NUMBER, 
		//	IP_CONTROLLER_APP_TYPE, MESSAGECODE_PORTERROR);
		
		CString sTmp = _T("");
		
		tmpXmlDom.AddElem( _T("Controller") );
		sTmp.Format( _T("%d"), lDeviceID );
		tmpXmlDom.AddAttrib( _T("DeviceID"), sTmp );
		sTmp.Format( _T("%d"), lTransportID );
		tmpXmlDom.AddAttrib( _T("TransportID"), sTmp );
		//sTmp.Format( _T("%d"), MAKEPORTID( lTransportID, lPortID ) );
		sTmp.Format( _T("%d"), lPortID );
		tmpXmlDom.AddAttrib( _T("PortID"), sTmp );
		sTmp.Format( _T("%d"), dwError );
		tmpXmlDom.AddAttrib( _T("ErrorCode"), sTmp );
		tmpXmlDom.AddAttrib( _T("ErrorMessage"), strMsg );
		
		CDeviceInfoParser::s_pDeviceInfoParser->SendMessage( tmpXmlDom );
		
		TRACE(_T("ReportError MessageCode: %d. DeviceID: %d MAKEPORTID: %d (TransportID: %d PortID: %d)  Error: 0x%X  Msg:%s \r\n"), MESSAGECODE_PORTERROR, lDeviceID, MAKEPORTID( lTransportID, lPortID ), lTransportID, lPortID, dwError, strMsg );
		
		// update error to the ListCtrl in the dialog.
		//_tcscpy( g_szErrorText, strMsg );
		//if( CDeviceInfoParser::s_pDeviceInfoParser->m_bPermitStatus )
		//::SendMessage( CDeviceInfoParser::s_pDeviceInfoParser->m_hWnd, UPDATEERRORTOLISTCTRL, MAKEDEVICEID( lDeviceID, lTransportID, lPortID ), dwError );	
		
		Clog( LOG_DEBUG, _T("ReportError. DeviceID: %d MAKEPORTID: %d (TransportID: %d PortID: %d)  Error: 0x%X  Msg:%s \r\n"), lDeviceID, MAKEPORTID( lTransportID, lPortID ), lTransportID, lPortID, dwError, strMsg );	
	}
	else
	{
		//CDeviceInfoParser::s_pDeviceInfoParser->SendControllerError( MESSAGECODE_CONTROLLERERROR, dwError, strMsg );
	}
	return 0;
}


//--------------------
bool CDeviceInfoParser::m_bHeartBeatThreadAlive = false;
bool CDeviceInfoParser::m_bHeartBeatThreadExit = true;
DWORD CDeviceInfoParser::m_iHeartBeatInterval = 3;
DWORD CDeviceInfoParser::m_iHeartBeatReferVal = 0;

// heart beat thread function.
UINT CDeviceInfoParser::HeartBeatThread(LPVOID pParam)
{
	CDeviceInfoParser *pMainControl = (CDeviceInfoParser *)pParam;
	m_bHeartBeatThreadAlive = true;
	m_bHeartBeatThreadExit = false;
	
	// because heartbeat report is in other thread, it is impossible that both thread call p_mConnectManager->SendData() in the same time, 
	// so i use m_csSendProtect to protect the message list that will be sent.
	
	DWORD wCurrent;
	
	Clog( LOG_DEBUG, "Start HeartBeat Thread." );
	while( m_bHeartBeatThreadAlive )
	{
		wCurrent = GetTickCount();
		
		//EnterCriticalSection( &m_csSendProtect );
		if( (wCurrent-m_iHeartBeatReferVal) >= m_iHeartBeatInterval*1000 )
		{//send heart beat package
			m_iHeartBeatReferVal = GetTickCount();
			if( CListenManager::s_ListenManager->m_bConnected )
				pMainControl->ReportHeartBeat();	
		}
		//LeaveCriticalSection( &m_csSendProtect );		
		
		Sleep( 200 );
	}
	
	m_bHeartBeatThreadExit = true;
	Clog( LOG_DEBUG, _T("Set flag for Exit HeartBeat Thread.") );
	
	return 0x11;
}

void CDeviceInfoParser::EndHeartBeatThread()
{
	// close heartbeat thread
	m_bHeartBeatThreadAlive = false;
	if( WaitForSingleObject( m_HeartBeatThread, 1000 ) != WAIT_OBJECT_0 )
	{
		if( !m_bHeartBeatThreadExit )
		{
			Clog( LOG_DEBUG, "CMainControl::Unload(): Exit Hearbeat Thread by call TerminateThread()." );
			TerminateThread( m_HeartBeatThread, 0x01 );
		}
	}	
	
}
////////////////////////////////////////////////////

UINT32 CDeviceInfoParser::GEN_NUMBER = 0;
CDeviceInfoParser * CDeviceInfoParser::s_pDeviceInfoParser = NULL;

//for test
void CDeviceInfoParser::Test( LPCTSTR sFileName )
{
	CMarkup xmlDOM;
	
	//////////////////////////////////////////////////////
	Clog( LOG_DEBUG, _T("Load File: %s"), sFileName );
	xmlDOM.Load( sFileName );
	
	ParseCommand( xmlDOM );
}

/*
Parse Message Received:
*/

VOID CDeviceInfoParser::Parse(CSCTCPSocket * pSocket, CSCMemoryBlockPtr block )
{
	if( block->GetSize() <= 0 )
		return;	
	
	CMarkup xmlDOM;
	
	if(!xmlDOM.SetDoc(CString(block->GetBlock(), block->GetSize())))
	{
		Clog( LOG_DEBUG, _T("Info - CDeviceInfoParser::Parse() - !Parse Message Error."));
		return;
	}	
	
	xmlDOM.FindChildElem( "MessageHeader" );
	UINT32 MessageCode = _ttoi((LPCTSTR)xmlDOM.GetChildAttrib( MESSAGECODE ));
	if( MessageCode == 0 )
		return;
	
	//xmlDOM.Save( _T("D:\\tmp3.xml"));
	//TRACE( _T("%s \r\n"), CString(block->GetBlock(), block->GetSize()) );
	//TRACE( _T("%s \r\n"), xmlDOM.GetDoc() );
	
	////////////////////////////////////////////////////////////
	//m_XmlDOM.Load( sFileName );
	
	//xmlDOM.ResetPos();
	
	ParseCommand( xmlDOM );
}

void CDeviceInfoParser::ParseCommand(CMarkup m_XmlDOM )
{
	TCHAR sLog[128] = {0};
	TCHAR sTmp[128] = {0};
	
	m_XmlDOM.ResetPos();
	
	if(!m_XmlDOM.FindElem( MESSAGEFLAG ) )
	{
		Clog( LOG_DEBUG,_T("Info - CDeviceInfoParser::Parse() - !FindElem(MESSAGEFLAG).") );
		return;
	}
	
	m_XmlDOM.IntoElem();
	
	// Do parse stuff
	if( !m_XmlDOM.FindElem( MESSAGEHEADER ) )
	{
		Clog( LOG_DEBUG,_T("Info - CDeviceInfoParser::Parse() - !FindElem(MESSAGEHEADER).") );
		return;
	}	
	
	UINT32 MessageCode = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECODE ));
	CString strTime = m_XmlDOM.GetAttrib( MESSAGETIME );
	UINT32 BeReturn = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGERETURN ));
	UINT32 nCommandID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "CommandID" ));
	UINT32 nSessionID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "SessionID" ));
	DWORD m_dwPortID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "PortID" ));
	
	TRACE(_T("Receive command: %d***\r\n"), MessageCode );
	_stprintf( sLog, _T("Receive command: %d***\r\n"), MessageCode );
	Clog( LOG_DEBUG, _T("Receive command: %d***\r\n, commandid: %d"), MessageCode, nCommandID );
	
	if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
		return;
	
	m_XmlDOM.Save( _T("D:\\tmp3.xml"));
	
	CString strTmp = _T("");
	
	if( MessageCode == MESSAGECODE_CONFIG )
	{
		if( !m_XmlDOM.FindChildElem( MESSAGECONFIG ) )
		{
			Clog( LOG_DEBUG, _T("Info - CDeviceInfoParser::Parse() - !FindElem(MESSAGECONFIG)."));
			return;
		}
		
		strTmp = m_XmlDOM.GetChildSubDoc();
		
		CMarkup xmlTmp;
		xmlTmp.SetDoc( strTmp );
		xmlTmp.Save( CListenManager::s_ListenManager->m_sysConfig.szConfigPath );
		//SplitConfig( xmlTmp );
		return;
	}
	
	DWORD dwCardID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECARDID ));
	DWORD dwPortID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEPORTID ));
	DWORD dwChannelID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECHANNELID ));
	
	//CString Filename=_T(""), sType=_T("");
	//Filename = m_XmlDOM.GetChildAttrib(_T("FileName"));		
	
	bool  bType = false;
	DWORD dwRlt = 0, dwCode = 0;
	
	Clog( LOG_DEBUG, _T("Receive a command. CardID: %d  PortID: %d  ChannelID: %d "), dwCardID, dwPortID, dwChannelID );
	switch(MessageCode)
	{
	case MESSAGECODE_INITDEVICE:
		m_DeviceManager.Initialize( CListenManager::s_ListenManager->m_sysConfig.szConfigPath ); 
		break;
	case MESSAGECODE_GETBANDWIDTH:
		break;
	case MESSAGECODE_GETCARDIPPORT:
		break;
	case MESSAGECODE_GETUSEDIPPORT:
		break;
	case MESSAGECODE_OPEN:
		{ 
			PPortInfo pp; 
			
			m_XmlDOM.IntoElem();		
			
			if( !m_XmlDOM.FindElem( MESSAGEDODPORT ) )
			{
				break;
			}
			// _T("PMTPID")
			pp.wPmtPID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEPMTPID ));
			// _T("TotalBitRate")
			pp.wTotalRate = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGETOTALBITRATE ));
			// _T("TmpPath")
			strTmp = m_XmlDOM.GetAttrib( MESSAGETMPPATH );
			_tcscpy( pp.szTempPath, strTmp.GetBuffer( 0 ) );
			
			m_XmlDOM.IntoElem();
			
			// _T("IPPort") 
			if( !m_XmlDOM.FindElem( MESSAGEIPPORT ) )
			{
				break;
			}
			
			// _T("DODIPPort")
			int i=0;
			
			m_XmlDOM.IntoElem();
			
			while( m_XmlDOM.FindElem( "DODIPPort" ) )
			{
				strTmp = m_XmlDOM.GetAttrib("IPAddress");
				_tcscpy( pp.m_castPort[i].cDestIp, strTmp.GetBuffer( 0 ) ); 
				pp.m_castPort[i].wDestPort = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib("IPPort"));
				pp.m_castPort[i].wSendType = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib("SendType"));
				i++;
			}
			m_XmlDOM.OutOfElem();
			
			pp.nCastCount=i;
			// _T("IPPort") 
			if( !m_XmlDOM.FindElem( "Channel" ) )
			{
				break;
			}
			
			i=0;
			while( m_XmlDOM.FindChildElem( "DODChannel" ) )
			{
				pp.m_Chanel[i].wChannelType = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGECHANNELTYPE ));
				pp.m_Chanel[i].wPID = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGESTREAMID ));
				pp.m_Chanel[i].wRepeatTime = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGEREPEATTIME ));
				strTmp = m_XmlDOM.GetChildAttrib( MESSAGECHANNELTAG );
				_tcscpy(pp.m_Chanel[i].cDescriptor, strTmp.GetBuffer(0) );
				pp.m_Chanel[i].wRate = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGEBITRATE ));
				pp.m_Chanel[i].nStreamType = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGESTREAMTYPE ));
				
				pp.m_Chanel[i].bBeDetect = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGEDETECT ));
				pp.m_Chanel[i].wBeDetectInterval = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGEDETECTINTERVAL ));
				pp.m_Chanel[i].wBeEncrypted = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGEENCRYPTED ));
				
				strTmp = m_XmlDOM.GetChildAttrib("path");
				_tcscpy( pp.m_Chanel[i].szPath, strTmp.GetBuffer( 0 ) );
				pp.m_Chanel[i].bEnable=TRUE;
				i++;
			}  
			pp.wChannelCount = i;   
			pp.m_nSessionID = nSessionID;
			
			dwRlt = m_DeviceManager.OpenPort(m_dwPortID, pp );
			ReportReturn( m_dwPortID, nCommandID, nSessionID, MESSAGECODE_OPEN, dwRlt );
			
			
			/*
			strcpy(pp.cDestIp,"192.168.80.81");
			pp.wDestPort=1234;
			strcpy(pp.szTempPath,"d:\\temp");
			pp.wChannelCount=2;
			pp.wPmtPID=0x128;
			pp.wTotalRate=500;// 800k/sec
			
			  pp.m_Chanel[0].bBeDetect=TRUE;
			  pp.m_Chanel[0].bEnable=TRUE;
			  strcpy(pp.m_Chanel[0].cDescriptor,"PIC");
			  pp.m_Chanel[0].nIndex=0;
			  strcpy(pp.m_Chanel[0].szPath,"d:\\temp\\doc1");
			  pp.m_Chanel[0].wBeDetectInterval=5000;
			  pp.m_Chanel[0].wBeEncrypted=FALSE;
			  pp.m_Chanel[0].wChannelType=0;
			  pp.m_Chanel[0].wPID=0x111;
			  pp.m_Chanel[0].wRate=200;
			  
				pp.m_Chanel[1].bBeDetect=TRUE; 
				pp.m_Chanel[1].bEnable=TRUE;
				strcpy(pp.m_Chanel[1].cDescriptor,"PIC");
				pp.m_Chanel[0].nIndex=1;
				strcpy(pp.m_Chanel[1].szPath,"d:\\temp\\doc2");
				pp.m_Chanel[1].wBeDetectInterval=5000;
				pp.m_Chanel[1].wBeEncrypted=FALSE;
				pp.m_Chanel[1].wChannelType=0;
				pp.m_Chanel[1].wPID=0x222;
				pp.m_Chanel[1].wRate=200;
				
				  pp.m_nSessionID=1;
				  
					dwRlt = m_DeviceManager.OpenPort(dwPortID, pp );
					ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_OPEN, dwRlt );
			*/
			}
			
			break;
		case MESSAGECODE_CLOSE:
			dwRlt = m_DeviceManager.ClosePort( dwPortID );
			ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_CLOSE, dwRlt );
			break;
		case MESSAGECODE_PAUSE:
			dwRlt = m_DeviceManager.PausePort( dwPortID );
			ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_PAUSE, dwRlt );
			break;
		case MESSAGECODE_RUN:			
			dwRlt = m_DeviceManager.RunPort( dwPortID );			
			ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_RUN, dwRlt );
			break;
		case MESSAGECODE_STOP:
			dwRlt = m_DeviceManager.StopPort( dwPortID );
			ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_STOP, dwRlt );
			break;
		case MESSAGECODE_GETPORTSTATE:
			{
				DWORD dwState;
				dwRlt = m_DeviceManager.GetPortState( dwPortID, &dwState );
				ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_GETPORTSTATE, dwRlt );
			}
			break;
		case MESSAGECODE_SETPORTPMTPID:
			//dwRlt = m_DeviceManager.StopPort( dwPortID );
			break;
		case MESSAGECODE_GETPORTPMTPID:
			break;
		case MESSAGECODE_SETTOTALBITRATE:
			{
				DWORD dwTotalBitrate = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEBITRATE ));
				//dwRlt = m_DeviceManager.SetTotalBitRate( dwPortID, dwTotalBitrate );
			}
			break;
		case MESSAGECODE_GETTMPFILEPATH:
			break;
		case MESSAGECODE_SETTMPFILEPATH:
			strTmp = m_XmlDOM.GetAttrib( MESSAGETMPFILEPATH );
			//dwRlt = m_DeviceManager.SetTotalBitRate( dwPortID, dwTotalBitrate );
			break;
		case MESSAGECODE_GETPORT:
			{				
				DWORD dwPortID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEPORTID ));
				ReportPortInfo( dwPortID, nSessionID, nCommandID );
			}
			break;
		case MESSAGECODE_GETCHANNEL:
			break;
		case MESSAGECODE_ENABLECHANNELDETECTED:
			dwCode = MESSAGECODE_ENABLECHANNELDETECTED;
		case MESSAGECODE_DISABLECHANNELDETECTED:
			{
				dwCode = MESSAGECODE_DISABLECHANNELDETECTED;
				
				bool bEnabled = (bool)_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEENABLED ));
				dwRlt = m_DeviceManager.SetDetectedFlag( dwPortID, dwChannelID, bEnabled );
				ReportReturn( dwPortID, nCommandID, nSessionID, dwCode, dwRlt );
			}
			break;
		case MESSAGECODE_SETDETECTINTERVAL:
			{
				long lInterval = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEINTERVAL ));
				dwRlt = m_DeviceManager.SetChannelDetectInterval( dwPortID, dwChannelID, lInterval );
			}
			break;
		case MESSAGECODE_FORCEREDETECTED:
			{
				DWORD dwForce = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEFORCED ));
				if( dwForce )
				{
					dwRlt = m_DeviceManager.UpdateCatalog( dwPortID, dwChannelID );
					ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_FORCEREDETECTED, dwRlt );
				}
			}
			break;
		case MESSAGECODE_ADDCHANNELVERSION:
			dwRlt = m_DeviceManager.AddChannelVersion( dwPortID, dwChannelID );
			break;
		case MESSAGECODE_RESETCHANNELVERSION:
			dwRlt = m_DeviceManager.ResetChannelVersion( dwPortID, dwChannelID );
			break;
		case MESSAGECODE_SETCHANNELVERSION:
			{
				BYTE byVersion = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECHVERSION ));
				dwRlt = m_DeviceManager.SetChannelVersion( dwPortID, dwChannelID, NULL, 0, byVersion );
			}
			break;
		case MESSAGECODE_SETOBJKEYLENGTH:
			{
				BYTE byObjkeylen = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEOBJKEYLEN ));
				dwRlt = m_DeviceManager.SetObjectKeyLength( dwPortID, dwChannelID, byObjkeylen );
			}
			break;
		case MESSAGECODE_SETIDXDESLEN:
			{
				DWORD dwIdxLen = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECHIDXDESLEN ));
				dwRlt = m_DeviceManager.SetChannelIdxDesLen( dwPortID, dwChannelID, dwIdxLen );
			}
			break;
		case MESSAGECODE_ENABLECHANNEL:
			{
				bool bEnabled = (bool)_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEENABLED ));
				dwRlt = m_DeviceManager.EnableChannel( dwPortID, dwChannelID, bEnabled );
				ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_ENABLECHANNEL, dwRlt );
			}
			break; 
		case MESSAGECODE_SETCHANNELBITRATE:
			{
				DWORD dwBitrate = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEBITRATE ));
				//dwRlt = m_DeviceManager.SetChannelDetectInterval( dwPortID, 0x222, dwInterval );
			}
			break;
		case MESSAGECODE_GETSUBCHANNEL:
			break;
		case MESSAGECODE_SETSUBCHANNELFREQ:
			{
				DWORD dwFreq = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEFREQUENCY ));
				//dwRlt = m_DeviceManager.SetChannelDetectInterval( dwPortID, 0x222, dwInterval );
			}
			break;
		case MESSAGECODE_GETSUBCHANNELFREQ:
			break;
			// append
		case MESSAGECODE_SETCATALOGNAME:
			{
				strTmp = m_XmlDOM.GetAttrib( MESSAGECATALOGNAME );
				dwRlt = m_DeviceManager.SetCatalogName( dwPortID, dwChannelID, strTmp.GetBuffer(0) );
				ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_SETCATALOGNAME, dwRlt );
			}
			break;
		case MESSAGECODE_GETCATALOGNAME:
			{			
				dwRlt = m_DeviceManager.GetCatalogName( dwPortID, dwChannelID, strTmp.GetBuffer(0) );
			}
			break;
		case MESSAGECODE_GETCHANNELDETECTED:
			{			
				BOOL bEnabled;
				dwRlt = m_DeviceManager.GetDetectedFlag( dwPortID, dwChannelID, &bEnabled );
			}
			break;
		case MESSAGECODE_GETTOTALBITRATE:
			{
				DWORD dwBitRate;
				dwRlt = m_DeviceManager.GetTotalBitRate( dwPortID, &dwBitRate );
			}
			break;
		case MESSAGECODE_GETDETECTINTERVAL:
			{
				long lInterval;
				dwRlt = m_DeviceManager.GetChannelDetectInterval( dwPortID, dwChannelID, &lInterval );
			}
			break;
		case MESSAGECODE_GETCHANNELVERSION:
			{
				BYTE byVersion;// = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECHVERSION ));
				dwRlt = m_DeviceManager.GetChannelVersion( dwPortID, dwChannelID, NULL, 0, &byVersion );
			}
			break;
		case MESSAGECODE_GETOBJKEYLENGTH:
			{
				BYTE byObjkeylen; // = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEOBJKEYLEN ));
				dwRlt = m_DeviceManager.GetObjectKeyLength( dwPortID, dwChannelID, &byObjkeylen );
			}
			break;
		case MESSAGECODE_GETCHANNELIDXDESLEN:
			{
				BYTE byIdxDesLen; // = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEOBJKEYLEN ));
				dwRlt = m_DeviceManager.GetChannelIdxDesLen( dwPortID, dwChannelID, &byIdxDesLen );
			}
			break;
		}
		Clog( LOG_DEBUG, _T("Finish executing a command. Return: %d"), dwRlt );
		CString sMessage; 
		sMessage.Format( _T("%s  %s\r\n"), GetCurrTime(), sLog );
		//AddMessage( sMessage );
		//((CDeviceControllerDlg*)(AfxGetApp()->m_pMainWnd))->AddMessage( sMessage );
}

void CDeviceInfoParser::ReportReturn( DWORD dwPortID, DWORD dwFlag, DWORD dwSessionID, DWORD dwMessageCode, DWORD dwRtn )
{
	CMarkup tmpXmlDom;
	CString sTmp = _T("");
	
	AddMessageHeader( tmpXmlDom, dwFlag, dwSessionID, dwMessageCode );
	
	sTmp.Format( _T("%d"), dwPortID );
	tmpXmlDom.AddAttrib( _T("PortID"), sTmp );
	sTmp.Format( _T("%d"), dwRtn );
	tmpXmlDom.AddAttrib( _T("Return"), sTmp );
	
	SendMessage( tmpXmlDom );
}
// Report ports information by call GetPort() of PortController.
// MessageCode   1213
void CDeviceInfoParser::ReportPortInfo( DWORD dwPortID, DWORD dwSessionID, DWORD dwCommandID )
{
	PPortInfo PortInfo; 
	m_DeviceManager.GetPort( dwPortID, &PortInfo );

	CMarkup tmpXmlDOM;
	tmpXmlDOM.AddElem( _T("Message") );
	tmpXmlDOM.IntoElem();
	CString sessionid,scommandid;
	sessionid.Format("%d",dwSessionID);
	scommandid.Format("%d",dwCommandID);
	//sTmp.Format( "%d", CSingleConnect::s_SingleConnect->wLocalPort );
	tmpXmlDOM.AddElem( _T("MessageHeader") );
	tmpXmlDOM.AddAttrib( _T("MessageCode"), MESSAGECODE_GETPORT);  
	tmpXmlDOM.AddAttrib( _T("SessionID"), sessionid );
	// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê6ÔÂ6ÈÕ 17:54:02
	tmpXmlDOM.AddAttrib( _T("PortID"),dwPortID );
	
	tmpXmlDOM.AddAttrib( _T("CommandID"),scommandid );
	tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
	tmpXmlDOM.AddAttrib( _T("BeReturn"),  _T("1") ); 

	tmpXmlDOM.AddElem( _T("MessageBody") );  
	tmpXmlDOM.IntoElem();
	
	tmpXmlDOM.AddElem( _T("DODPort") );
	tmpXmlDOM.AddAttrib( _T("PMTPID"),PortInfo.wPmtPID);  
	tmpXmlDOM.AddAttrib( _T("TotalBitRate"),PortInfo.wTotalRate );
	tmpXmlDOM.AddAttrib( _T("TmpPath"), PortInfo.szTempPath); 
	tmpXmlDOM.IntoElem();
	
	tmpXmlDOM.AddElem( _T("IPPort") );
	tmpXmlDOM.IntoElem();
	
	for (int i=0;i<PortInfo.nCastCount;i++)
	{
		tmpXmlDOM.AddElem( _T("DODIPPort") );
		
		tmpXmlDOM.AddAttrib( _T("IPAddress"),PortInfo.m_castPort[i].cDestIp );
		tmpXmlDOM.AddAttrib( _T("IPPort"), PortInfo.m_castPort[i].wDestPort); 
		tmpXmlDOM.AddAttrib( _T("SendType"), PortInfo.m_castPort[i].wSendType); 
	}
	tmpXmlDOM.OutOfElem();
	
	tmpXmlDOM.AddElem( _T("Channel") );
	tmpXmlDOM.IntoElem();
	for (i=0;i<PortInfo.wChannelCount;i++)
	{
		tmpXmlDOM.AddElem( _T("DODChannel") );
		
		tmpXmlDOM.AddAttrib( _T("ChannelType"),PortInfo.m_Chanel[i].wChannelType);  
		tmpXmlDOM.AddAttrib( _T("StreamID"),PortInfo.m_Chanel[i].wPID);
		tmpXmlDOM.AddAttrib( _T("StreamType"),PortInfo.m_Chanel[i].nStreamType);
		tmpXmlDOM.AddAttrib( _T("BitRate"),PortInfo.m_Chanel[i].wRate);
		tmpXmlDOM.AddAttrib( _T("path"),PortInfo.m_Chanel[i].szPath);
		tmpXmlDOM.AddAttrib( _T("RepeatTime"),PortInfo.m_Chanel[i].wRepeatTime);
		tmpXmlDOM.AddAttrib( _T("Tag"),PortInfo.m_Chanel[i].cDescriptor);
		tmpXmlDOM.AddAttrib( _T("BeDetectInterval"),PortInfo.m_Chanel[i].wBeDetectInterval);
		tmpXmlDOM.AddAttrib( _T("BeEncrypted"),PortInfo.m_Chanel[i].wBeEncrypted);
		tmpXmlDOM.AddAttrib( _T("BeDetect"),PortInfo.m_Chanel[i].bBeDetect);
	}
				/*
				CMarkup tmpXmlDom;
				CString sTmp = _T("");
				
				AddMessageHeader( tmpXmlDom, 0, 0, MESSAGECODE_GETPORT );
				
				tmpXmlDom.AddElem( _T("DODPort") );
				sTmp.Format( _T("%d"), pp.wPmtPID );
				tmpXmlDom.AddAttrib( _T("PMTID"), sTmp );
				sTmp.Format( _T("%d"), pp.wTotalRate );
				tmpXmlDom.AddAttrib( _T("TotalBitRate"), sTmp );
				tmpXmlDom.AddAttrib( _T("TmpPath"), pp.szTempPath );
				tmpXmlDom.AddAttrib( _T("SourceDirectory"), _T("") );
				
				tmpXmlDom.IntoElem();
				for( int i=0; i<pp.wChannelCount; i++ )
				{
					tmpXmlDom.AddElem( _T("DODChannel") );
					sTmp.Format( _T("%d"), pp.m_Chanel[i].wChannelType );
					tmpXmlDom.AddAttrib( _T("ChannelType"), sTmp );
					sTmp.Format( _T("%d"), pp.m_Chanel[i].wPID );
					tmpXmlDom.AddAttrib( _T("StreamID"), sTmp );
					sTmp.Format( _T("%d"), pp.m_Chanel[i].wRepeatTime );
					tmpXmlDom.AddAttrib( _T("RepeatTime"), sTmp );
					tmpXmlDom.AddAttrib( _T("Tag"), pp.m_Chanel[i].cDescriptor );
					sTmp.Format( _T("%d"), pp.m_Chanel[i].wRate );
					tmpXmlDom.AddAttrib( _T("BitRate"), sTmp );
					sTmp.Format( _T("%d"), pp.m_Chanel[i].bBeDetect );
					tmpXmlDom.AddAttrib( _T("BeDetect"), sTmp );
					sTmp.Format( _T("%d"), pp.m_Chanel[i].wBeDetectInterval );
					tmpXmlDom.AddAttrib( _T("BeDetectInterval"), sTmp );
					sTmp.Format( _T("%d"), pp.m_Chanel[i].wBeEncrypted );
					tmpXmlDom.AddAttrib( _T("BeEncrypted"), sTmp );
				}
				*/
				SendMessage( tmpXmlDOM );
}

CString CDeviceInfoParser::GetCurrTime()
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	CString sTime;
	sTime.Format("[%02d:%02d:%02d::%003d] ", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	return sTime;
}


CString CDeviceInfoParser::GetCurrDateTime()
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	CString sTime;
	sTime.Format("%04d%02d%02d%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	return sTime;
}

void CDeviceInfoParser::AddMessage( LPCTSTR text )
{
	//_tcscpy( g_szRichText, text );
	//::SendMessage( m_hWnd, ADDTEXTTORICHEDIT, 0, 0 );
	/*
	CRichEditCtrl * pRichEdit = (CRichEditCtrl *)GetDlgItem( m_hWnd, IDC_RICHEDIT21 );		
	int iLen = pRichEdit->GetTextLength();
	if( iLen > MAX_RICHEDIT )
	{
	pRichEdit->SetWindowText( "" );
	iLen = 0;
	}
	pRichEdit->SetSel( iLen, iLen );
	pRichEdit->ReplaceSel( text );		//write richedit
	*/
}


/*
organize a header of message.
*/
void CDeviceInfoParser::AddMessageHeader( CMarkup & tmpXmlDOM,  DWORD flag, DWORD no, WORD code )
{
	CString sTmp;
	
	tmpXmlDOM.AddElem( _T("Message") );
	tmpXmlDOM.IntoElem();
	
	tmpXmlDOM.AddElem( _T("MessageHeader") );
	sTmp.Format( _T("%d"), code );
	tmpXmlDOM.AddAttrib( _T("MessageCode"), sTmp );  
	sTmp.Format( _T("%d"), no );
	tmpXmlDOM.AddAttrib( _T("SessionID"), sTmp );
	sTmp.Format( _T("%d"), flag );
	tmpXmlDOM.AddAttrib( _T("CommandID"), sTmp );
	tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
	//sTmp.Format( _T("%d"), _T("1") );
	tmpXmlDOM.AddAttrib( _T("BeReturn"),  _T("1") ); 
	
	tmpXmlDOM.AddElem( _T("MessageBody") );
}

void CDeviceInfoParser::Handshake( )
{
	if( !m_iHaveConfig )
	{
		CString sTmp=_T("");
		GEN_NUMBER++;
		//m_SendXmlDOM.RemoveElem();
		//m_SendXmlDOM.ResetPos();
		CMarkup m_SendXmlDOM;
		AddMessageHeader( m_SendXmlDOM, 0, GEN_NUMBER, MESSAGECODE_HANDSHAKE );
		
		m_SendXmlDOM.AddElem( _T("Controller") );
		sTmp.Format( _T("%d"), m_iControllerID );
		m_SendXmlDOM.AddAttrib( _T("ID"), sTmp );
		sTmp.Format( _T("%d"), m_iHaveConfig );
		m_SendXmlDOM.AddAttrib( _T("Configuration"), sTmp );
		
		SendData( m_SendXmlDOM );
		TRACE( _T("Send Handshake Information to playback server.") );
	}
	else //if( !m_bHaveReportList )
	{
		ReportDeviceList();
	}
}

void CDeviceInfoParser::ReportDeviceList()
{
	if( m_dwDManagerOpen != MEDIAPUMP_NOERROR )
	{
		TRACE( _T("OpenManager Fail, Send fail info to playback server.") );
		Clog(LOG_DEBUG, _T("OpenManager Fail, Send fail info to playback server.") );
		_stprintf( g_szErrorText, _T("%s"), _T("InitManager Fail.") );
		SendControllerError( MESSAGECODE_CONTROLLERERROR, m_dwDManagerOpen, g_szErrorText );
		return;
	}
	//for( DeviceVector::iterator i = vcDeviceItem.begin(); i != vcDeviceItem.end(); ++i )
	{
		SendDevicePortInfo(); // (*i).lDeviceID, MAKEPORTID( (*i).lTransportID, (*i).lPortID ) );		
	}
	TRACE(_T("ReportDeviceList to Playback Server.\r\n"));
	//m_bHaveReportList = true;
}

void CDeviceInfoParser::SendDevicePortInfo()//int iDeviceID, WORD wPort)
{
	CString sTmp=_T("");
	GEN_NUMBER++;
	
	CMarkup tmpXmlDOM;
	AddMessageHeader( tmpXmlDOM, 0, GEN_NUMBER, MESSAGECODE_HANDSHAKE );
	
	tmpXmlDOM.AddElem( _T("Controller") );
	sTmp.Format( _T("%d"), m_iControllerID );
	tmpXmlDOM.AddAttrib( _T("ID"), sTmp );
	tmpXmlDOM.AddAttrib( _T("Configuration"), _T("1") );	
	
	//long lCurrStatus = 0;
	for( DeviceVector::iterator i = vcDeviceItem.begin(); i != vcDeviceItem.end(); ++i )
	{
		tmpXmlDOM.AddChildElem( _T("Port") );
		sTmp.Format( _T("%d"), (*i).lDeviceID );
		tmpXmlDOM.AddChildAttrib( _T("DeviceID"), sTmp );
		sTmp.Format( _T("%d"), (*i).lTransportID );
		tmpXmlDOM.AddChildAttrib( _T("TransportID"), sTmp );
		//sTmp.Format( _T("%d"), MAKEPORTID( (*i).lTransportID, (*i).lPortID ) );
		sTmp.Format( _T("%d"), (*i).lPortID );
		tmpXmlDOM.AddChildAttrib( _T("PortID"), sTmp );
		sTmp.Format( _T("%d"), (*i).wStatus );
		tmpXmlDOM.AddChildAttrib( _T("Status"), sTmp );
		//Clog(LOG_DEBUG, _T("Send Init DevicePort List Information, DeviceID: %d  MAKEPORTID: %d (TransportID: %d  PortID: %d) Status: %d\r\n"), (*i).lDeviceID, MAKEPORTID((*i).lTransportID, (*i).lPortID), (*i).lTransportID, (*i).lPortID, (*i).wStatus );
		Clog(LOG_DEBUG, _T("Send Init DevicePort List Information, DeviceID: %d  TransportID: %d  PortID: %d  Status: %d\r\n"), (*i).lDeviceID, (*i).lTransportID, (*i).lPortID, (*i).wStatus );
	}
	
	SendData( tmpXmlDOM );
	//tmpXmlDOM.Save( _T("D:\\WORK\\DeviceController\\Message.XML") );
}

void CDeviceInfoParser::SplitConfig( CMarkup & tmpXmlDOM )
{
	bool bHave = false;
	if( tmpXmlDOM.FindChildElem(_T("Playback")) )
	{
		tmpXmlDOM.IntoElem();
		while( tmpXmlDOM.FindChildElem(_T("MediaPumper")) )
		{			
			int iID = _ttoi((LPCTSTR)tmpXmlDOM.GetChildAttrib(_T("ID")));
			if( iID == m_struIP.iControllerID )
			{
				CString strTmp = tmpXmlDOM.GetChildSubDoc();
				
				CMarkup xmlTmp;
				xmlTmp.SetDoc( strTmp );
				xmlTmp.Save( CListenManager::s_ListenManager->m_sysConfig.szConfigPath );
				xmlTmp.ResetPos();
				bHave = true;
				break;
			}
		}
	}
	
	if( !bHave )
		return;
	
	// start DeviceManager
	Clog(LOG_DEBUG, _T("Call CloseManager.") );
	//m_DeviceManager.CloseManager();
	Clog(LOG_DEBUG, _T("Call OpenManager.") );
	//Sleep( 1000 );
	//m_dwDManagerOpen = m_DeviceManager.InitManager( CSingleConnect::s_SingleConnect->m_sysConfig.szConfigPath );
	if( m_dwDManagerOpen != MEDIAPUMP_NOERROR )
	{
		TRACE( _T("InitManager Fail.") );
		Clog(LOG_DEBUG, _T("OpenManager Fail.") );
		_stprintf( g_szErrorText, _T("%s"), _T("InitManager Fail.") );
		SendControllerError( MESSAGECODE_CONTROLLERERROR, m_dwDManagerOpen, g_szErrorText );
	}
	else
	{
		//::SendMessage( m_hWnd, DELETEITEMINLISTCTRL, 0, 0 );
		QueryDeviceList();		
		SendDevicePortInfo();
		TRACE( _T("ReInitManager Successful..") );
	}
}
void CDeviceInfoParser::QueryDeviceList()
{
	m_bPermitStatus = false;
	
	vcDeviceItem.clear();
	/*
	int iDeviceLen = m_DeviceManager.GetDeviceIDList( g_DeviceID, MAXDEVICE );
	if( iDeviceLen <= 0 )
	return;
	
	  int iTranLen = 0, iPortLen = 0;
	  for( int i=0; i<iDeviceLen; i++ )
	  {
	  iTranLen = m_DeviceManager.GetTransportIDList( g_DeviceID[i], g_TransportID, MAXTRANSPORT );
	  if( iTranLen <= 0 )
	  continue;
	  for( int j=0; j<iTranLen; j++ )
	  {
	  iPortLen = m_DeviceManager.GetPortIDList(  g_DeviceID[i], g_TransportID[j], g_PortID, MAXPORT );
	  if( iPortLen <= 0 )
	  continue;
	  for( int k=0; k<iPortLen; k++ )
	  {
	  DeviceInfo tmpSps;
	  tmpSps.lDeviceID = g_DeviceID[i];		
	  tmpSps.lTransportID = g_TransportID[j];
	  tmpSps.lPortID = g_PortID[k];
	  tmpSps.wStatus = (WORD)m_DeviceManager.GetPortStatus( tmpSps.lDeviceID, tmpSps.lTransportID, tmpSps.lPortID );
	  tmpSps.lID = MAKEDEVICEID(tmpSps.lDeviceID, tmpSps.lTransportID, tmpSps.lPortID );
	  vcDeviceItem.push_back( tmpSps );
	  
		::SendMessage( m_hWnd, UPDATEITEMTOLISTCTRL, MAKEDEVICEID( tmpSps.lDeviceID, tmpSps.lTransportID, tmpSps.lPortID ), tmpSps.wStatus ); //PORT_STATUS_INIT );	// 
		
		  Clog( LOG_DEBUG, _T("QueryDeviceList. DeviceID: %d TransportID: %d PortID: %d  Status: 0x%X \r\n"), tmpSps.lDeviceID, tmpSps.lTransportID, tmpSps.lPortID, tmpSps.wStatus );	
		  }
		  }
}*/
	m_bPermitStatus = true;
}

void CDeviceInfoParser::SendControllerError( int messagecode, int errorcode, LPCTSTR message )
{
	CDeviceInfoParser::GEN_NUMBER++;
	
	CMarkup tmpXmlDom;
	AddMessageHeader( tmpXmlDom, 0, CDeviceInfoParser::GEN_NUMBER, messagecode );
	
	CString sTmp = _T("");
	
	tmpXmlDom.AddElem( _T("Controller") );
	sTmp.Format( _T("%d"), errorcode );
	tmpXmlDom.AddAttrib( _T("ErrorCode"), sTmp );
	tmpXmlDom.AddAttrib( _T("ErrorMessage"), message );
	
	SendMessage( tmpXmlDom );
	
	// update error to the ListCtrl in the dialog.
}

/*
send a message got from the doc of tmpXmlDOM.
*/
void CDeviceInfoParser::SendMessage(  CMarkup & tmpXmlDOM, int iPackageNumber )
{
	if ( CListenManager::s_ListenManager )
	{
		if( CListenManager::s_ListenManager->m_bConnected )
		{
			SendData( tmpXmlDOM, iPackageNumber );
			tmpXmlDOM.Save( "d:\\tmp4.xml" );
		}
	}
}
void CDeviceInfoParser::SendData( CMarkup & tmpXmlDOM, int iPackageNumber )
{
	CString sTmp = tmpXmlDOM.GetDoc();
	int length = sTmp.GetLength();
	char * pcData = CSCMemoryBlock::AllocBlock(length+1);	
	strncpy( pcData, sTmp.GetBuffer(length), length );
	pcData[length] = MSG_END_FLAG;
	CSCMemoryBlock * mBlock = new CSCMemoryBlock( pcData, length+1 );
	mBlock->m_iBlockNumber = iPackageNumber;	
	
	CSCMemoryBlockPtr pFred( mBlock );
	
	//EnterCriticalSection( &m_csSendProtect );
	CListenManager::s_ListenManager->SendDataToAll( pFred );
	//LeaveCriticalSection( &m_csSendProtect );	
	
	Clog( LOG_DEBUG, _T("Send message successfully.") );
}
// called in heart beat thread for proving alive itself.
void CDeviceInfoParser::ReportHeartBeat()
{
	//WORD wTmp = m_iHeartBeatInterval;
	//TCHAR * pcData = CSCMemoryBlock::AllocBlock(sizeof(wTmp)+3);
	//memcpy( pcData, &wTmp, sizeof(wTmp) );
	
	TCHAR * pcData = CSCMemoryBlock::AllocBlock(3);
	pcData[0] = '\r';
	pcData[1] = '\n';
	pcData[2] = 127;
	//_stprintf( pcData, _T("\r\n") );
	CSCMemoryBlock * mBlock = new CSCMemoryBlock( pcData, 3 );
	
	CSCMemoryBlockPtr pFred( mBlock );
	
	EnterCriticalSection( &m_csSendProtect );		
	CListenManager::s_ListenManager->SendData( pFred );
	LeaveCriticalSection( &m_csSendProtect );
}
