
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

//#include "StdAfx.h"
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

int ReportError(LONG lDeviceID, LONG lTransportID, LONG lPortID, DWORD dwError, LPCTSTR strMsg)
{
	ASSERT( strMsg != NULL );
	Clog( LOG_DEBUG, _T("Enter - CSCNVODReporter::Error()"));
	
	if( lDeviceID >= 0 && lTransportID >= 0 && lPortID >= 0 )
	{		
		CMarkup tmpXmlDom;
		CString sTmp = _T("");
		
		tmpXmlDom.AddElem( _T("Controller") );
		sTmp.Format( _T("%d"), lDeviceID );
		tmpXmlDom.AddAttrib( _T("DeviceID"), sTmp );
		sTmp.Format( _T("%d"), lTransportID );
		tmpXmlDom.AddAttrib( _T("TransportID"), sTmp );
		sTmp.Format( _T("%d"), lPortID );
		tmpXmlDom.AddAttrib( _T("PortID"), sTmp );
		sTmp.Format( _T("%d"), dwError );
		tmpXmlDom.AddAttrib( _T("ErrorCode"), sTmp );
		tmpXmlDom.AddAttrib( _T("ErrorMessage"), strMsg );
		
		CDeviceInfoParser::s_pDeviceInfoParser->SendMessage( tmpXmlDom );
		
		TRACE(_T("ReportError MessageCode: %d. DeviceID: %d MAKEPORTID: %d (TransportID: %d PortID: %d)  Error: 0x%X  Msg:%s \r\n"), MESSAGECODE_PORTERROR, lDeviceID, MAKEPORTID( lTransportID, lPortID ), lTransportID, lPortID, dwError, strMsg );
		Clog( LOG_DEBUG, _T("ReportError. DeviceID: %d MAKEPORTID: %d (TransportID: %d PortID: %d)  Error: 0x%X  Msg:%s \r\n"), lDeviceID, MAKEPORTID( lTransportID, lPortID ), lTransportID, lPortID, dwError, strMsg );	
	}
	return 0;
}


//--------------------
bool CDeviceInfoParser::m_bHeartBeatThreadAlive = false;
bool CDeviceInfoParser::m_bHeartBeatThreadExit = true;
DWORD CDeviceInfoParser::m_iHeartBeatInterval = 3;
DWORD CDeviceInfoParser::m_iHeartBeatReferVal = 0;

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
		return;
	}
	
	DWORD dwCardID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECARDID ));
	DWORD dwPortID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEPORTID ));
	DWORD dwChannelID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECHANNELID ));
	
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
			try
			{
			PPortInfo pp; 
			pp.lstChannelInfo.clear();
			pp.lstIPPortInfo.clear();
			m_XmlDOM.IntoElem();		
			
			if( !m_XmlDOM.FindElem( MESSAGEDODPORT ) )
			{
				break;
			}
			pp.wPmtPID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEPMTPID ));
			pp.wTotalRate = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGETOTALBITRATE ));
			strTmp = m_XmlDOM.GetAttrib( MESSAGETMPPATH );
			_tcscpy( pp.szTempPath, strTmp.GetBuffer( 0 ) );
			
			m_XmlDOM.IntoElem();
			
			if( !m_XmlDOM.FindElem( MESSAGEIPPORT ) )
			{
				break;
			}
			
			int i=0;
			
			m_XmlDOM.IntoElem();
			
			while( m_XmlDOM.FindElem( "DODIPPort" ) )
			{
				ZQSBFIPPORTINF tmpIPPort;

				strTmp = m_XmlDOM.GetAttrib("IPAddress");
				_tcscpy( tmpIPPort.cDestIp, strTmp.GetBuffer( 0 ) ); 
				tmpIPPort.wDestPort = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib("IPPort"));
				tmpIPPort.wSendType = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib("SendType"));
				_tcscpy( tmpIPPort.cSourceIp, CListenManager::s_ListenManager->m_sysConfig.cAddr ); 

				pp.lstIPPortInfo.push_back(tmpIPPort);
				i++;
			}
			m_XmlDOM.OutOfElem();
			
			pp.nCastCount=i;
			if( !m_XmlDOM.FindElem( "Channel" ) )
			{
				break;
			}
			
			i=0;
			while( m_XmlDOM.FindChildElem( "DODChannel" ) )
			{
				PPChannelInfo TmpChannel;
				TmpChannel.wChannelType = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGECHANNELTYPE ));
				TmpChannel.wPID = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGESTREAMID ));
				TmpChannel.wRepeatTime = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGEREPEATTIME ));
				strTmp = m_XmlDOM.GetChildAttrib( MESSAGECHANNELTAG );
				_tcscpy(TmpChannel.cDescriptor, strTmp.GetBuffer(0) );
				TmpChannel.wRate = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGEBITRATE ));
				TmpChannel.nStreamType = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGESTREAMTYPE ));
				
				TmpChannel.bBeDetect = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGEDETECT ));
				TmpChannel.wBeDetectInterval = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGEDETECTINTERVAL ));
				TmpChannel.wBeEncrypted = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( MESSAGEENCRYPTED ));

				TmpChannel.wDataExchangeType = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( _T("DataExchangeType") ));
				TmpChannel.wStreamCount = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( _T("StreamCount") ));
				TmpChannel.wPackagingMode = _ttoi((LPCTSTR)m_XmlDOM.GetChildAttrib( _T("PackagingMode") ));

				strTmp = m_XmlDOM.GetChildAttrib("path");
				_tcscpy( TmpChannel.szPath, strTmp.GetBuffer( 0 ) );
				TmpChannel.bEnable=TRUE;

				pp.lstChannelInfo.push_back(TmpChannel);
				i++;
			}  
			pp.wChannelCount = i;   
			pp.m_nSessionID = nSessionID;
			
			dwRlt = m_DeviceManager.OpenPort( nSessionID, &pp); //m_dwPortID, pp ); // -- modified by leon: m_dwPortID -> nSessionID
			ReportReturn( m_dwPortID, nCommandID, nSessionID, MESSAGECODE_OPEN, dwRlt );
			
			}
			catch(...)
			{
				Clog( LOG_DEBUG, _T("OpenPort. CardID: %d  PortID: %d  ChannelID: %d error code=%d"), dwCardID, dwPortID, dwChannelID ,GetLastError());
			}
			
			}
			
			break;
		case MESSAGECODE_CLOSE:
			dwRlt = m_DeviceManager.ClosePort( nSessionID );// dwPortID ); // -- modified by leon: dwPortID -> nSessionID
			ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_CLOSE, dwRlt );
			break;
		case MESSAGECODE_PAUSE:
			dwRlt = m_DeviceManager.PausePort( nSessionID ); // -- modified by leon: dwPortID -> nSessionID
			ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_PAUSE, dwRlt );
			break;
		case MESSAGECODE_RUN:			
			dwRlt = m_DeviceManager.RunPort( nSessionID );	// -- modified by leon: dwPortID -> nSessionID		
			ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_RUN, dwRlt );
			break;
		case MESSAGECODE_STOP:
			dwRlt = m_DeviceManager.StopPort( nSessionID ); // -- modified by leon: dwPortID -> nSessionID		
			ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_STOP, dwRlt );
			break;
		case MESSAGECODE_GETPORTSTATE:
			{
				DWORD dwState;
				dwRlt = m_DeviceManager.GetPortState( nSessionID, &dwState );	// -- modified by leon: dwPortID -> nSessionID		
				ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_GETPORTSTATE, dwRlt );
			}
			break;
		case MESSAGECODE_SETPORTPMTPID:
			//dwRlt = m_DeviceManager.StopPort( nSessionID ); // -- modified by leon: dwPortID -> nSessionID		
			break;
		case MESSAGECODE_GETPORTPMTPID:
			break;
		case MESSAGECODE_SETTOTALBITRATE:
			{
				DWORD dwTotalBitrate = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEBITRATE ));
				//dwRlt = m_DeviceManager.SetTotalBitRate( nSessionID, dwTotalBitrate ); // -- modified by leon: dwPortID -> nSessionID		
			}
			break;
		case MESSAGECODE_GETTMPFILEPATH:
			break;
		case MESSAGECODE_SETTMPFILEPATH:
			strTmp = m_XmlDOM.GetAttrib( MESSAGETMPFILEPATH );
			//dwRlt = m_DeviceManager.SetTotalBitRate( nSessionID, dwTotalBitrate ); // -- modified by leon: dwPortID -> nSessionID		
			break;
		case MESSAGECODE_GETPORT:
			{				
				DWORD dwPortID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEPORTID ));
				ReportPortInfo( nSessionID, nSessionID, nCommandID );	// -- modified by leon: dwPortID -> nSessionID		
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
				dwRlt = m_DeviceManager.SetDetectedFlag( nSessionID, dwChannelID, bEnabled ); // -- modified by leon: dwPortID -> nSessionID		
				ReportReturn( dwPortID, nCommandID, nSessionID, dwCode, dwRlt ); 
			}
			break;
		case MESSAGECODE_SETDETECTINTERVAL:
			{
				long lInterval = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEINTERVAL ));
				dwRlt = m_DeviceManager.SetChannelDetectInterval( nSessionID, dwChannelID, lInterval ); // -- modified by leon: dwPortID -> nSessionID
			}
			break;
		case MESSAGECODE_FORCEREDETECTED:
			{
				DWORD dwForce = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEFORCED ));
				if( dwForce )
				{
					dwRlt = m_DeviceManager.UpdateCatalog( nSessionID, dwChannelID ); // -- modified by leon: dwPortID -> nSessionID
					ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_FORCEREDETECTED, dwRlt ); 
				}
			}
			break;
		case MESSAGECODE_ADDCHANNELVERSION:
			dwRlt = m_DeviceManager.AddChannelVersion( nSessionID, dwChannelID ); // -- modified by leon: dwPortID -> nSessionID
			break;
		case MESSAGECODE_RESETCHANNELVERSION:
			dwRlt = m_DeviceManager.ResetChannelVersion( nSessionID, dwChannelID ); // -- modified by leon: dwPortID -> nSessionID
			break;
		case MESSAGECODE_SETCHANNELVERSION:
			{
				BYTE byVersion = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECHVERSION ));
				dwRlt = m_DeviceManager.SetChannelVersion( nSessionID, dwChannelID, NULL, 0, byVersion ); // -- modified by leon: dwPortID -> nSessionID
			}
			break;
		case MESSAGECODE_SETOBJKEYLENGTH:
			{
				BYTE byObjkeylen = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEOBJKEYLEN ));
				dwRlt = m_DeviceManager.SetObjectKeyLength( nSessionID, dwChannelID, byObjkeylen ); // -- modified by leon: dwPortID -> nSessionID
			}
			break;
		case MESSAGECODE_SETIDXDESLEN:
			{
				DWORD dwIdxLen = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECHIDXDESLEN ));
				dwRlt = m_DeviceManager.SetChannelIdxDesLen( nSessionID, dwChannelID, dwIdxLen ); // -- modified by leon: dwPortID -> nSessionID
			}
			break;
		case MESSAGECODE_ENABLECHANNEL:
			{
				bool bEnabled = (bool)_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEENABLED ));
				dwRlt = m_DeviceManager.EnableChannel( nSessionID, dwChannelID, bEnabled ); // -- modified by leon: dwPortID -> nSessionID
				ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_ENABLECHANNEL, dwRlt );
			}
			break; 
		case MESSAGECODE_SETCHANNELBITRATE:
			{
				DWORD dwBitrate = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEBITRATE ));
				//dwRlt = m_DeviceManager.SetChannelDetectInterval( nSessionID, 0x222, dwInterval ); // -- modified by leon: dwPortID -> nSessionID
			}
			break;
		case MESSAGECODE_GETSUBCHANNEL:
			break;
		case MESSAGECODE_SETSUBCHANNELFREQ:
			{
				DWORD dwFreq = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEFREQUENCY ));
				//dwRlt = m_DeviceManager.SetChannelDetectInterval( nSessionID, 0x222, dwInterval ); // -- modified by leon: dwPortID -> nSessionID
			}
			break;
		case MESSAGECODE_GETSUBCHANNELFREQ:
			break;
			// append
		case MESSAGECODE_SETCATALOGNAME:
			{
				strTmp = m_XmlDOM.GetAttrib( MESSAGECATALOGNAME );
				dwRlt = m_DeviceManager.SetCatalogName( nSessionID, dwChannelID, strTmp.GetBuffer(0) ); // -- modified by leon: dwPortID -> nSessionID
				ReportReturn( dwPortID, nCommandID, nSessionID, MESSAGECODE_SETCATALOGNAME, dwRlt );
			}
			break;
		case MESSAGECODE_GETCATALOGNAME:
			{			
				dwRlt = m_DeviceManager.GetCatalogName( nSessionID, dwChannelID, strTmp.GetBuffer(0) ); // -- modified by leon: dwPortID -> nSessionID
			}
			break;
		case MESSAGECODE_GETCHANNELDETECTED:
			{			
				BOOL bEnabled;
				dwRlt = m_DeviceManager.GetDetectedFlag( nSessionID, dwChannelID, &bEnabled ); // -- modified by leon: dwPortID -> nSessionID
			}
			break;
		case MESSAGECODE_GETTOTALBITRATE:
			{
				DWORD dwBitRate;
				dwRlt = m_DeviceManager.GetTotalBitRate( nSessionID, &dwBitRate ); // -- modified by leon: dwPortID -> nSessionID
			}
			break;
		case MESSAGECODE_GETDETECTINTERVAL:
			{
				long lInterval=0;
				dwRlt = m_DeviceManager.GetChannelDetectInterval( nSessionID, dwChannelID, &lInterval ); // -- modified by leon: dwPortID -> nSessionID
			}
			break;
		case MESSAGECODE_GETCHANNELVERSION:
			{
				BYTE byVersion;// = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECHVERSION ));
				dwRlt = m_DeviceManager.GetChannelVersion( nSessionID, dwChannelID, NULL, 0, &byVersion ); // -- modified by leon: dwPortID -> nSessionID
			}
			break;
		case MESSAGECODE_GETOBJKEYLENGTH:
			{
				BYTE byObjkeylen; // = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEOBJKEYLEN ));
				dwRlt = m_DeviceManager.GetObjectKeyLength( nSessionID, dwChannelID, &byObjkeylen ); // -- modified by leon: dwPortID -> nSessionID
			}
			break;
		case MESSAGECODE_GETCHANNELIDXDESLEN:
			{
				BYTE byIdxDesLen; // = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEOBJKEYLEN ));
				dwRlt = m_DeviceManager.GetChannelIdxDesLen( nSessionID, dwChannelID, &byIdxDesLen ); // -- modified by leon: dwPortID -> nSessionID
			}
			break;
		}
		Clog( LOG_DEBUG, _T("Finish executing a command. Return: %d"), dwRlt );
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
	
	for( DODIPPORTINFLIST::iterator it = PortInfo.lstIPPortInfo.begin(); it!=PortInfo.lstIPPortInfo.end(); it++ )
	{
		ZQSBFIPPORTINF *IPPortTmp=(ZQSBFIPPORTINF*)&(*it);
		if (IPPortTmp==NULL)
			continue;
		tmpXmlDOM.AddElem( _T("DODIPPort") );
		
		tmpXmlDOM.AddAttrib( _T("IPAddress"),IPPortTmp->cDestIp );
		tmpXmlDOM.AddAttrib( _T("IPPort"), IPPortTmp->wDestPort); 
		tmpXmlDOM.AddAttrib( _T("SendType"),IPPortTmp->wSendType); 
		tmpXmlDOM.AddAttrib( _T("SourceIPAddress"),IPPortTmp->cSourceIp);
		tmpXmlDOM.AddAttrib( _T("SourceIPPort"),IPPortTmp->wSourcePort);

	}
	tmpXmlDOM.OutOfElem();
	
	tmpXmlDOM.AddElem( _T("Channel") );
	tmpXmlDOM.IntoElem();
	for( DODCHANNELINFOLIST::iterator it2 = PortInfo.lstChannelInfo.begin(); it2!=PortInfo.lstChannelInfo.end(); it2++ )
	{
		PPChannelInfo *ChannelTmp=(PPChannelInfo*)&(*it2);
		if (ChannelTmp==NULL)
			continue;
		
		tmpXmlDOM.AddElem( _T("DODChannel") );
		
		tmpXmlDOM.AddAttrib( _T("ChannelType"),ChannelTmp->wChannelType);  
		tmpXmlDOM.AddAttrib( _T("StreamID"),ChannelTmp->wPID);
		tmpXmlDOM.AddAttrib( _T("StreamType"),ChannelTmp->nStreamType);
		tmpXmlDOM.AddAttrib( _T("BitRate"),ChannelTmp->wRate);
		tmpXmlDOM.AddAttrib( _T("path"),ChannelTmp->szPath);
		tmpXmlDOM.AddAttrib( _T("RepeatTime"),ChannelTmp->wRepeatTime);
		tmpXmlDOM.AddAttrib( _T("Tag"),ChannelTmp->cDescriptor);
		tmpXmlDOM.AddAttrib( _T("BeDetectInterval"),ChannelTmp->wBeDetectInterval);
		tmpXmlDOM.AddAttrib( _T("BeEncrypted"),ChannelTmp->wBeEncrypted);
		tmpXmlDOM.AddAttrib( _T("BeDetect"),ChannelTmp->bBeDetect);
	}
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
	tmpXmlDOM.AddAttrib( _T("BeReturn"),  _T("1") ); 
	
	tmpXmlDOM.AddElem( _T("MessageBody") );
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
	TCHAR * pcData = CSCMemoryBlock::AllocBlock(3);
	pcData[0] = '\r';
	pcData[1] = '\n';
	pcData[2] = 127;
	CSCMemoryBlock * mBlock = new CSCMemoryBlock( pcData, 3 );
	
	CSCMemoryBlockPtr pFred( mBlock );
	
	EnterCriticalSection( &m_csSendProtect );		
	CListenManager::s_ListenManager->SendData( pFred );
	LeaveCriticalSection( &m_csSendProtect );
}
