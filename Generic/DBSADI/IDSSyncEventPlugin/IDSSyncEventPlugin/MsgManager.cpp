// MsgManager.cpp: implementation of the CMsgManager class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786)

#include "stdafx.h"
#include <ScLog.h>
#include <cfgpkg.h>
#include <WinSock2.h>
#include "MsgManager.h"

#ifdef TEST
#include <iostream.h>
#endif




//NOTE: Log file path can't exceed 1024 characters


#if CFG_FROM_REGISTRY
#	ifdef _DEBUG
#		pragma comment(lib,"cfgpkgU_d.lib")
#	else
#		pragma	comment(lib,"cfgpkgU.lib")
#	endif
#endif

#pragma comment(lib,"jmsc.lib")

#ifdef _DEBUG
#	pragma comment(lib,"jmscpp_d.lib")
#else
#	pragma comment(lib,"jmscpp.lib")
#endif

using namespace ZQ::common;
using namespace ZQ::JMSCpp;

char*	WChartoAnsi(wchar_t* pwChar,char* pBuf,int bufLen)
{
	if(!pwChar)	return NULL;
	ZeroMemory(pBuf,bufLen);
	WideCharToMultiByte(CP_ACP,0,pwChar,wcslen(pwChar),pBuf,bufLen,NULL,NULL);
	//wcstombs(pBuf,pwChar,bufLen);
	return pBuf;
}
wchar_t* AnsitoWChar(char* pAnsi,wchar_t* pWbuf,int wBuflen)
{
	if(!pAnsi)	return NULL;
	ZeroMemory(pWbuf,wBuflen);
	MultiByteToWideChar(CP_ACP,0,pAnsi,strlen(pAnsi),pWbuf,wBuflen);
	//mbstowcs(pWbuf,pAnsi,wBuflen);	
	return pWbuf;
}

CMsgManager::CMsgManager()
{
#ifdef TEST
	m_dwCount=0;
#endif
	m_bThreadRunning=FALSE;
	m_bInitializeOK=FALSE;
	m_pJmsContext=NULL;
	m_bConnectionOK=FALSE;
	m_vMsgStack.clear();
	m_strVersion="0.1.9";
	
	if(!GetConfiguration())
	{
		m_bInitializeOK=FALSE;
		return;
	}
	pGlog=new ScLog(m_strLogPath.c_str(),Log::L_DEBUG,m_dwLogFileSize,m_dwLogBufferSize,m_dwLogWriteTimeOut);
	if(!pGlog)
	{
		m_bInitializeOK=FALSE;
		return;
	}
	
	//m_bInitializeOK=InitializeJMS();
	
	SetThreadStatus(TRUE);
	m_hSendMessage=CreateThread(NULL,0,SendMessageThread,this,0,NULL);

	m_bInitializeOK=TRUE;

	InitializeCriticalSection(&m_Section);
}

CMsgManager::~CMsgManager()
{
	SetThreadStatus(FALSE);
	//Wait the send message thread,if it can't end in the specify time,ignore it!
	WaitForSingleObject(m_hSendMessage,m_dwShutDownWaitTime);

	UnitializeJMS();
	
	if(pGlog)
	{
		delete pGlog;
		pGlog=NULL;
	}
	DeleteCriticalSection(&m_Section);	
}

BOOL CMsgManager::GetConfiguration()
{
#if	CFG_FROM_REGISTRY

#define	WBUFSIZE	2048
#define	BUF_SIZE	1024

	wchar_t		wszBuf[WBUFSIZE];
	wchar_t		wszBuf2[WBUFSIZE];
	BYTE		szBuf[WBUFSIZE];
	char		szRetBuf[BUF_SIZE];
	memset(wszBuf,0,WBUFSIZE);
	memset(szBuf,0,BUF_SIZE);
	memset(szRetBuf,0,BUF_SIZE);

	DWORD		dwRegValue;
	//Initliaze cfg handle
	DWORD		ValueCount;
	
	m_hCfgHandle=CFG_INITEx(AnsitoWChar("DBSync",wszBuf,WBUFSIZE),&ValueCount,AnsitoWChar("ITV",wszBuf2,WBUFSIZE));

	if(!m_hCfgHandle)
		return FALSE;

	DWORD num_count = 0;
	CFGSTATUS status = CFG_SUBKEY(m_hCfgHandle, L"\\IDSSyncEventPlugin", &num_count);
	if (status != CFG_SUCCESS)
	{
		return false;
	}
	
	DWORD	regType=REG_SZ;
	DWORD	szBufSize=WBUFSIZE;
	//Get m_strNamingContext
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("NamingContext",wszBuf,WBUFSIZE),szBuf,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_strNamingContext="org.jnp.interfaces.NamingContextFactory";
	}
	else
	{
//		memset(szRetBuf,0,BUF_SIZE);
//		strncpy(szRetBuf,(char*)szBuf,szBufSize);		
		m_strNamingContext=WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE);
	}

	//get m_strServerIP
	szBufSize=WBUFSIZE;
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("JmsServerIP",wszBuf,WBUFSIZE),szBuf,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_strServerIP="";
	}
	else
	{
		m_strServerIP=WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE);
	}
	
	//Get m_dwServerPort
	szBufSize=sizeof(DWORD);
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("JmsServerPort",wszBuf,WBUFSIZE),(BYTE*)&dwRegValue,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_dwServerPort=1099;
	}
	else
	{
		m_dwServerPort=dwRegValue;
	}

	//Get addin IP
	szBufSize=WBUFSIZE;
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("DBSyncIP",wszBuf,WBUFSIZE),szBuf,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_strAddInIP="";
	}
	else
	{
		m_strAddInIP=WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE);
	}

	//Get m_strDestinationName
	szBufSize=WBUFSIZE;
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("DestinationName",wszBuf,WBUFSIZE),szBuf,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_strDestinationName="topic/testTopic";
	}
	else
	{
		m_strDestinationName=WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE);
	}

	//Get m_strConnectionFactory
	szBufSize=WBUFSIZE;
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("ConnectionFactory",wszBuf,WBUFSIZE),szBuf,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_strConnectionFactory="ConnectionFactory";
	}
	else
	{
		m_strConnectionFactory=WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE);
	}


	//Get m_strLogPath
	szBufSize=WBUFSIZE;
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("LogPath",wszBuf,WBUFSIZE),szBuf,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_strLogPath="c:\\log.txt";
	}
	else
	{
		m_strLogPath=WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE);
	}


	szBufSize=WBUFSIZE;
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("IniFilePath",wszBuf,WBUFSIZE),szBuf,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_strIniFilePath="c:\\IDSSyncEventPlugin.ini";
	}
	else
	{
		m_strIniFilePath=WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE);
	}

	//Get m_dwLogBufferSize
	szBufSize=sizeof(DWORD);
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("LogBufferSize",wszBuf,WBUFSIZE),(BYTE*)&dwRegValue,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_dwLogBufferSize=64*1024;
	}
	else
	{
		m_dwLogBufferSize=dwRegValue;/*atoi(WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE));*/
		
	}
	
	//Get m_dwLogFileSize
	szBufSize=sizeof(DWORD);
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("LogFileSize",wszBuf,WBUFSIZE),(BYTE*)&dwRegValue,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_dwLogFileSize=10*1024*1024;
	}
	else
	{
		m_dwLogFileSize=dwRegValue;/*atoi(WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE));*/
	}

	//Get m_dwLogWriteTimeOut
	szBufSize=sizeof(DWORD);
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("LogWriteTimeOut",wszBuf,WBUFSIZE),(BYTE*)&dwRegValue,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_dwLogWriteTimeOut=2;
	}
	else
	{
		m_dwLogWriteTimeOut=dwRegValue;/*atoi(WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE));*/
	}

	//Get m_dwSendMsgCycleCount
	szBufSize=sizeof(DWORD);
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("SendMsgCycleCount",wszBuf,WBUFSIZE),(BYTE*)&dwRegValue,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_dwSendMsgCycleCount=3;
	}
	else
	{
		m_dwSendMsgCycleCount=dwRegValue;/*atoi(WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE));*/
	}

	//Get m_dwMillCountReConnect
	szBufSize=sizeof(DWORD);
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("MillCountReConnect",wszBuf,WBUFSIZE),(BYTE*)&dwRegValue,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_dwMillCountReConnect=5*1000;
	}
	else
	{
		m_dwMillCountReConnect=dwRegValue;/*atoi(WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE));*/
	}

	//Get m_dwMillIntervalSendMsg
	szBufSize=sizeof(DWORD);
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("MillIntervalSendMsg",wszBuf,WBUFSIZE),(BYTE*)&dwRegValue,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_dwMillIntervalSendMsg=100;
	}
	else
	{
		m_dwMillIntervalSendMsg=dwRegValue;/*atoi(WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE));*/
	}
	//Get m_dwShutDownWaitTime
	szBufSize=sizeof(DWORD);
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("ShutDownWaitTime",wszBuf,WBUFSIZE),(BYTE*)&dwRegValue,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_dwShutDownWaitTime=5*1000;
	}
	else
	{
		m_dwShutDownWaitTime=dwRegValue;/*atoi(WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE));*/
	}

	//Get m_KeepAliveTime
	szBufSize=sizeof(DWORD);
	if(CFG_GET_VALUE(m_hCfgHandle,AnsitoWChar("KeepAliveTime",wszBuf,WBUFSIZE),(BYTE*)&dwRegValue,&szBufSize,&regType)!=CFG_SUCCESS)
	{
		m_KeepAliveTime=5*1000;
	}
	else
	{
		m_KeepAliveTime=dwRegValue;/*atoi(WChartoAnsi((wchar_t*)szBuf,szRetBuf,BUF_SIZE));*/
	}
	return TRUE;

#undef	WBUFSIZE	
#undef	BUF_SIZE	

#else
	m_strNamingContext="org.jnp.interfaces.NamingContextFactory";
	//m_strServerAddress="11.11.0.22:1099";
	m_strServerAddress="192.168.80.138:1099";
	m_strDestinationName="topic/testTopic";
	m_strConnectionFactory="ConnectionFactory";
	m_strLogPath="D:\\work\\project\\ZQProjs\\Generic\\DBSADI\\JMS_DBSA\\JMS_DBSyncAddIn_TEST\\Debug\\log.txt";
	m_strIniFilePath="D:\\JMSDBSA\\JMS_DBSyncAddIn.ini";
	m_dwLogBufferSize=20*1024;
	m_dwLogFileSize=2000*1024;
	m_dwLogWriteTimeOut=2;
	m_dwSendMsgCycleCount=3;
	m_dwMillCountReConnect=5*1000;
	m_dwMillIntervalSendMsg=100;
	m_dwShutDownWaitTime=5*1000;
	m_strAddInIP="192.168.80.138";
	m_KeepAliveTime=5000;
	
	return TRUE;
#endif	
}
VOID CMsgManager::UnitializeJMS()
{
	if(m_pJmsContext)
	{
		if(m_bInitializeOK)
			m_JmsConnection.stop();
		delete m_pJmsContext;
		m_pJmsContext=NULL;
	}
}

VOID CMsgManager::ConnectionMonitor(int errType,VOID* lpData)
{
	glog(Log::L_DEBUG,"VOID CMsgManager::ConnectionMonitor(int errType,VOID* lpData)::Some exception was thowed out,Mabe network issue!");
	CMsgManager* pThis=(CMsgManager*)lpData;
	pThis->SetConnectionStatus(FALSE);	
}
BOOL CMsgManager::ConnetToHost()
{
	
	glog(Log::L_DEBUG,"BOOL CMsgManager::ConnetToHost()::Begin to connect to server");

	m_JmsTxtMessage.DestroyMessage();
	m_JmsProducer.close();
	m_JmsDestination.destroy();
	m_JmsSession.close();
	m_JmsConnection.close();
	m_JmsCNFactory.Destroy();

	glog(Log::L_DEBUG,"ConnetToHost():create connection factory with=%s",m_strConnectionFactory.c_str());

	if(!m_pJmsContext->createConnectionFactory(m_strConnectionFactory.c_str(),m_JmsCNFactory))
	//if(!m_pJmsContext->createConnectionFactory("TopicConnectFactory",m_JmsCNFactory))
	{		
		glog(Log::L_ERROR,"BOOL CMsgManager::ConnetToHost()::can't create connnectionfactory with factory=%s And errCode=%d","ConnectionFactory",getLastJmsError());
		return FALSE;
	}
	if(!m_JmsCNFactory.createConnection(m_JmsConnection))
	{
		glog(Log::L_ERROR,"BOOL CMsgManager::ConnetToHost()::Can't create connection using JMSConnectionFactory and ErrCode=%d",getLastJmsError());
		return FALSE;
	}
	if(!m_JmsConnection._connection)
	{
		glog(Log::L_ERROR,"BOOL CMsgManager::ConnetToHost()::Invalid connection handle after createConnection");
		return FALSE;
	}
	
	m_JmsConnection.SetConnectionCallback(ConnectionMonitor,this);
	
	if(!m_JmsConnection.createSession(m_JmsSession))
	{
		glog(Log::L_ERROR,"BOOL CMsgManager::ConnetToHost()::Can't create session using JMSConnection and ErrCode=%d",getLastJmsError());
		return FALSE;
	}

	glog(Log::L_DEBUG,"ConnetToHost():Create destination with string=%s",m_strDestinationName.c_str());

	if(!m_pJmsContext->createDestination(m_strDestinationName.c_str(),m_JmsDestination))
	{
		glog(Log::L_ERROR,"BOOL CMsgManager::ConnetToHost()::Can't create Destination with destinaition name=%s and ErrCode=%d",m_strDestinationName.c_str(),getLastJmsError());
		return FALSE;
	}

	
	if(!m_JmsSession.createProducer(&m_JmsDestination,m_JmsProducer))
	{
		glog(Log::L_ERROR,"BOOL CMsgManager::ConnetToHost()::Can't create producer using JMS destination and ErrCode=%d",getLastJmsError());
		return FALSE;
	}

	if(!m_JmsConnection.start())
	{
		glog(Log::L_ERROR,"BOOL CMsgManager::ConnetToHost()::JMSCOnnection start fail and errCode=%d",getLastJmsError());
		return FALSE;
	}
	
	if(!m_JmsSession.textMessageCreate("",m_JmsTxtMessage))
	{
		glog(Log::L_ERROR,"BOOL CMsgManager::ConnetToHost()::Create text message fail and ErrCode=%d",getLastJmsError());
		return FALSE;
	}
	if(!CreateMessageProperty())
	{
		glog(Log::L_DEBUG,"BOOL CMsgManager::ConnetToHost()::Create message property fail");
		return false;
	}

	glog(Log::L_DEBUG,"BOOL CMsgManager::ConnetToHost()::End to connect to server  and everything is OK");

	SetConnectionStatus(TRUE);
	

	//Just for Test
	EnterCriticalSection(&m_Section);	
	glog(Log::L_DEBUG,"BOOL CMsgManager::ConnetToHost()::Now ,We can send message.And there are %d messages in the message stack",m_vMsgStack.size());
	LeaveCriticalSection(&m_Section);
	//
	
	return TRUE;
}
#include "IniInfo.h"
BOOL CMsgManager::CreateMessageProperty()
{
	if(NULL==m_JmsTxtMessage._message)
	{
		glog(Log::L_ERROR,"BOOL CMsgManager::CreateMessageProperty()## Text message is not available");
		return false;
	}
	IniInfo	ini;
	ini.SetFile(m_strIniFilePath);
	//Get property count
	int		iPropertyCount=0;
	
	ini.SetSection("TextMessageProperty");
	
	ini.GetValue("PropertyCount",iPropertyCount);

	glog(Log::L_DEBUG,"Get property count=%d",iPropertyCount);
	for(int i=0;i<iPropertyCount;i++)
	{
		char	szChar[48];
		sprintf(szChar,"MsgProperty%d",i+1);
		ini.SetSection(szChar);
		{
#define		VALUE_DESC		"value"	
			std::string		strKey;
			ini.GetValue("key",strKey);
			//Get Type
			std::string		strType;
			ini.GetValue("type",strType);

			if(0==stricmp("int",strType.c_str()))
			{	
				int				iValue;
				ini.GetValue(VALUE_DESC,iValue);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%d",strKey.c_str(),iValue);
				m_JmsTxtMessage.setIntProperty((char*)strKey.c_str(),iValue);
			}
			else if(0==stricmp("long",strType.c_str()))
			{
				long			lValue=0;
				ini.GetValue(VALUE_DESC,lValue);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%d",strKey.c_str(),lValue);
				m_JmsTxtMessage.setLongProperty((char*)strKey.c_str(),lValue);
			}
			else if(0==stricmp("double",strType.c_str()))
			{
				double			dValue=0;
				ini.GetValue(VALUE_DESC,dValue);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%lf",strKey.c_str(),dValue);
				m_JmsTxtMessage.setDoubleProperty((char*)strKey.c_str(),dValue);
			}
			else if(0==stricmp("float",strType.c_str()))
			{
				float			fValue=0.0f;
				ini.GetValue(VALUE_DESC,fValue);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%f",strKey.c_str(),fValue);
				m_JmsTxtMessage.setFloatProperty((char*)strKey.c_str(),fValue);
			}
			else if(0==stricmp("bool",strType.c_str()))
			{
				bool			bValue=false;
				ini.GetValue(VALUE_DESC,bValue);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%d",strKey.c_str(),bValue);
				m_JmsTxtMessage.setBoolProperty((char*)strKey.c_str(),bValue);
			}
			else if(0==stricmp("byte",strType.c_str()))
			{
				unsigned char			byValue;
				ini.GetValue(VALUE_DESC,&byValue,1);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%d",strKey.c_str(),byValue);
				m_JmsTxtMessage.setByteProperty((char*)strKey.c_str(),byValue);
			}
			else if(0==stricmp("short",strType.c_str()))
			{
				short			sValue=0;
				ini.GetValue(VALUE_DESC,sValue);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%d",strKey.c_str(),sValue);
				m_JmsTxtMessage.setShortProperty((char*)strKey.c_str(),sValue);
			}
			else if(0==stricmp("string",strType.c_str()))
			{
				std::string		strValue;
				ini.GetValue(VALUE_DESC,strValue);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%s",strKey.c_str(),strValue.c_str());
				m_JmsTxtMessage.setStringProperty((char*)strKey.c_str(),(char*)strValue.c_str());
			}
			else
			{
				glog(Log::L_ERROR,"None support type %s",strType.c_str());
				return false;
			}
#undef	VALUE_DESC
		}
	}
	return true;
}
BOOL CMsgManager::InitializeJMS()
{
	if(m_pJmsContext)
	{
		delete m_pJmsContext;
		m_pJmsContext=NULL;
	}

	char ipport[256] = {0};
	sprintf(ipport, "%s:%d", m_strServerIP.c_str(), m_dwServerPort);
	m_strServerAddress = ipport;
	
	glog(Log::L_DEBUG,"BOOL CMsgManager::InitializeJMS():: before Create Context with Server Address =%s  and NamingContext=%s",m_strServerAddress.c_str(),m_strNamingContext.c_str());
	m_pJmsContext=new ZQ::JMSCpp::Context(m_strServerAddress.c_str(),m_strNamingContext.c_str());
	if(!m_pJmsContext )
	{
		glog(Log::L_ERROR,"BOOL CMsgManager::InitilizeJMS()::can't initialize JmsContext with server address is %s and server namingContext is %s and ErrCode is %d",m_strServerAddress.c_str(),m_strNamingContext.c_str(),getLastJmsError());
		return FALSE;
	}
	glog(Log::L_DEBUG,"BOOL CMsgManager::InitializeJMS():: after Create Context with Server Address =%s  and NamingContext=%s",m_strServerAddress.c_str(),m_strNamingContext.c_str());

	if( !m_pJmsContext->_context)
	{
		if(m_pJmsContext)
		{
			delete m_pJmsContext;
			m_pJmsContext=NULL;
		}
		glog(Log::L_ERROR,"BOOL CMsgManager::InitilizeJMS()::can't initialize JmsContext with server address is %s and server namingContext is %s",m_strServerAddress.c_str(),m_strNamingContext.c_str());
		return FALSE;
	}
	glog(Log::L_DEBUG,"BOOL CMsgManager::InitializeJMS():: Create Context succesfully  with Server Address =%s  and NamingContext=%s",m_strServerAddress.c_str(),m_strNamingContext.c_str());

	ConnetToHost();	
	

	return TRUE;
}
VOID CMsgManager::PushMessage(JMSMsg& strMsg)
{
	EnterCriticalSection(&m_Section);	
	m_vMsgStack.push_back(strMsg);
//	if(!m_bThreadRunning)
//	{
//		m_bThreadRunning=TRUE;
//		m_hSendMessage=CreateThread(NULL,0,SendMessageThread,this,0,NULL);		
//	}
	LeaveCriticalSection(&m_Section);
}
JMSMsg CMsgManager::PopMessage()
{
	EnterCriticalSection(&m_Section);

	if(m_vMsgStack.size()<=0)
	{
		glog(Log::L_WARNING,"JMSMsg CMsgManager::PopMessage()::No message in the stack");
		LeaveCriticalSection(&m_Section);
		return NULL;
	}
	JMSMsg	msg=m_vMsgStack[0];
	m_vMsgStack.erase(m_vMsgStack.begin());
	LeaveCriticalSection(&m_Section);
	return msg;
}
BOOL CMsgManager::HasMessage()
{
	EnterCriticalSection(&m_Section);
	BOOL bHas=m_vMsgStack.size()>0;
	LeaveCriticalSection(&m_Section);
	return bHas;
}
DWORD WINAPI CMsgManager::SendMessageThread(LPVOID lpData)
{
	CMsgManager* pThis=(CMsgManager*)lpData;
	if(!pThis->InitializeJMS())
	{
		glog(Log::L_CRIT,"NOTICE:The send message thread can't initialize successfully!So No meessage can be sent");
		return 0;
	}
	while (pThis->GetThreadStatus())
	{
		if(!pThis->GetConnectionStatus())
		{
			Sleep(pThis->GetReconnectTime());
			pThis->ConnetToHost();
		}
		else
		{
			if(pThis->HasMessage())
				pThis->SendJmsMessage();		
		}
		Sleep(pThis->GetSendMsgIntervalTime());
	}	
	
	while (pThis->HasMessage())
	{
		pThis->SendJmsMessage();			
	}
	
	pThis->UnitializeJMS();
	CloseHandle(pThis->m_hSendMessage);

	return 1;
}
VOID CMsgManager::SendJmsMessage()
{
#define		LOG_SINGLE_LENGTH	1000

	if(m_JmsTxtMessage._message==NULL)
	{
		glog(Log::L_CRIT,"CRITIC>>instance m_JmstxtMessage is NULL ,Something is wrong");
		return;
	}
	JMSMsg	msg=PopMessage();	
	if(!m_JmsTxtMessage.setText((char*)msg.c_str()))
	{
		glog(Log::L_ERROR,"BOOL CMsgManager::SendJmsMessage()::Can't set text message with msg=%s",msg.c_str());
		return;
	}
	
	int		iMsgLen=msg.size();
	for(int i=0;i<iMsgLen;i+=LOG_SINGLE_LENGTH)
	{
		int iCount=(i+LOG_SINGLE_LENGTH<iMsgLen?i+LOG_SINGLE_LENGTH:iMsgLen)-i;
		JMSMsg	msgTemp=msg.substr(i,iCount);
		glog(Log::L_DEBUG,"VOID CMsgManager::SendJmsMessage()::Now Send message %s",msgTemp.c_str());
	}

#ifdef TEST
	cout<<"Send message "<<m_dwCount++<<endl;
#endif
	ProducerOptions	pOption;
	ZeroMemory(&pOption,sizeof(ProducerOptions));
	pOption.flags=PO_TIMETOLIVE;
	pOption.timeToLive=m_KeepAliveTime;

	for( i=0;i<m_dwSendMsgCycleCount;i++)
	{
		glog(Log::L_DEBUG,"BOOL CMsgManager::SendJmsMessage()::send message with KeepAliveTime=%d",m_KeepAliveTime);
		if(!m_JmsProducer.send(&m_JmsTxtMessage,&pOption))
		{			
			glog(Log::L_ERROR,"BOOL CMsgManager::SendJmsMessage()::send message fail with JMSMessage is %X and ErrCode=%d",&m_JmsTxtMessage,getLastJmsError());		
		}
		else
		{
			break;
		}
	}
}




//////////////////////////////////////////////////////////////////////////
//Generate message
VOID   CMsgManager::PutTabbledChar(std::string& str,int n)
{
	for(int i=0;i<n;i++)
	{
		str+='\t';
	}
}
VOID CMsgManager::BreakeLine(std::string& str)
{
	str+="\n";
}
VOID CMsgManager::GenerateMsgFooter(JMSMsg& str)
{
	BreakeLine(str);
	str+= "</DBSAMessage>";
}
VOID CMsgManager::CreateBodyHeader(JMSMsg& str,int ID)
{
	char	szBuf[16];
	BreakeLine(str);
	PutTabbledChar(str,1);
	str+="<DBSAMessageBody TopicID=\"";
	itoa(ID,szBuf,10);
	str+=szBuf;
	str+= "\" >";
	BreakeLine(str);
}
VOID CMsgManager::CreatreBodyFooter(JMSMsg& str)
{
	BreakeLine(str);
	PutTabbledChar(str,1);
	str+="</DBSAMessageBody>";
}
JMSMsg CMsgManager::GenerateMsgHeader()
{
//	if(!(pDbsInfo&&pItvInfo))
//	{
//		glog(Log::L_ERROR,"JMSMsg CMsgManager::GenerateMsgHeader()::NULL pDbsInfo or pItvInfo pass in");
//		return NULL;
//	}
	DA_dbsyncInfo* pDbsInfo=&m_DbsInfo;
	DA_itvInfo*		pItvInfo=&m_ItvInfo;
	
#define		BUF_SIZE	64
	char	szbuf[BUF_SIZE];	

	m_strMessageHeader="";
	//put tag DBSAMessage
	m_strMessageHeader="<DBSAMessage>";
	BreakeLine(m_strMessageHeader);
	
	//Put tag DBSAMessageHeader SystemTime
	//Get system time at first	
	//format time to string
	char	szTime[16];
	char	szDate[16];
	LCID	loc=MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),SORT_DEFAULT);
	GetDateFormat(loc,0,NULL,("yyyy-MM-dd"),szDate,16);
	GetTimeFormat(loc,0,NULL,("HH:mm:ss"),szTime,16);
	//end format
	
	PutTabbledChar(m_strMessageHeader,1);
	m_strMessageHeader+= "<DBSAMessageHeader SystemTime=\"";
//		ZeroMemory(szbuf,BUF_SIZE);
//		wcstombs(szbuf,szDate,BUF_SIZE);
	m_strMessageHeader+= szDate;
	m_strMessageHeader+= " ";
//		ZeroMemory(szbuf,BUF_SIZE);
//		wcstombs(szbuf,szTime,BUF_SIZE);	
	m_strMessageHeader+= szTime;
	m_strMessageHeader+= "\">";
	BreakeLine(m_strMessageHeader);

	//Put tag ADDIN
	PutTabbledChar(m_strMessageHeader,2);
	m_strMessageHeader+= "<Addin ";
	m_strMessageHeader+= "IPAddress=\"";
	m_strMessageHeader+= m_strAddInIP;
	m_strMessageHeader+= "\" Version=\"";
	m_strMessageHeader+= m_strVersion;
	m_strMessageHeader+= "\"/>";
	BreakeLine(m_strMessageHeader);

	//Put  DBSync tag
	PutTabbledChar(m_strMessageHeader,2);
	m_strMessageHeader+= "<DBSync ";
	m_strMessageHeader+= "IPAddress=\"";
	
	// Modified by Ken on 2006-12-25 
	// use m_strAddInIP instead of pItvInfo->_szIPAddr to remove the DBSyncIP registry from DBSync
//		ZeroMemory(szbuf,BUF_SIZE);
//		wcstombs(szbuf,pDbsInfo->_szIPAddr,BUF_SIZE);
//	m_strMessageHeader+= szbuf;
	m_strMessageHeader+= m_strAddInIP;

	m_strMessageHeader+= "\" Version=\"";
		ZeroMemory(szbuf,BUF_SIZE);
		wcstombs(szbuf,pDbsInfo->_szVersion,BUF_SIZE);
	m_strMessageHeader+= szbuf;
	m_strMessageHeader+= "\"/>";
	BreakeLine(m_strMessageHeader);

	//Put ITVDS tag
	PutTabbledChar(m_strMessageHeader,2);
	m_strMessageHeader+= "<ITVDS ";
	m_strMessageHeader+= "IPAddress=\"";

	// Modified by Ken on 2006-12-25 
	// use m_strAddInIP instead of pItvInfo->_szIPAddr to remove the DBSyncIP registry from DBSync
//		ZeroMemory(szbuf,BUF_SIZE);
//		wcstombs(szbuf,pItvInfo->_szIPAddr,BUF_SIZE);
//	m_strMessageHeader+= szbuf;
	m_strMessageHeader+= m_strAddInIP;

	m_strMessageHeader+= "\" Version=\"";
		ZeroMemory(szbuf,BUF_SIZE);
		wcstombs(szbuf,pItvInfo->_szVersion,BUF_SIZE);
	m_strMessageHeader+= szbuf;
	m_strMessageHeader+= "\" />";
	BreakeLine(m_strMessageHeader);
	
	//End of message header
	PutTabbledChar(m_strMessageHeader,1);
	m_strMessageHeader+="</DBSAMessageHeader>";
	//BreakeLine(m_strMessageHeader);
	return m_strMessageHeader;
#undef BUF_SIZE
}

JMSMsg CMsgManager::GenerateInitialize(DA_dbsyncInfo* pDbsInfo, DA_itvInfo* pItvInfo)
{
	if(!(pDbsInfo&&pItvInfo))
	{
		glog(Log::L_EMERG,"CMsgManager::GenerateInitialize():NULL DbsInfo or ITVinfo pass in");
		return NULL;
	}
#define		BUF_SIZE	256
	m_DbsInfo=*pDbsInfo;
	m_ItvInfo=*pItvInfo;
	char		szBuf[BUF_SIZE];
	JMSMsg	strMessage=GenerateMsgHeader();
	CreateBodyHeader(strMessage,1);

	PutTabbledChar(strMessage,2);
	//Put Initialize  tag
	strMessage+= "<Initialize ";
	strMessage+= "Instance=\"";
	itoa(pDbsInfo->_dwInstanceID,szBuf,10);
	strMessage+= szBuf;

	strMessage+= "\" SupportNavigation=\"";
	
	itoa(pDbsInfo->_dwSupportNav,szBuf,10);
	strMessage+= szBuf;
	strMessage+= "\" SyncDirectory=\"";
	//Need change it into UTF-8
		int		iCharSize=WideCharToMultiByte(CP_UTF8, 0, pDbsInfo->_szSyncDir, wcslen(pDbsInfo->_szSyncDir), szBuf, 0,NULL,NULL);
		char	*pNewBuf=new char[iCharSize+1];
		ZeroMemory(pNewBuf,iCharSize+1);
		WideCharToMultiByte(CP_UTF8, 0, pDbsInfo->_szSyncDir, wcslen(pDbsInfo->_szSyncDir), pNewBuf, iCharSize,NULL,NULL);
	strMessage+= pNewBuf;
		delete[]	pNewBuf;
		pNewBuf=NULL;
	
	strMessage+= "\" TimeWindowThreshold=\"";

	itoa(pDbsInfo->_dwTwThreshold,szBuf,10);
	strMessage+= szBuf;
	strMessage+= "\"/>";
	CreatreBodyFooter(strMessage);
	GenerateMsgFooter(strMessage);

	return strMessage;
#undef	BUF_SIZE
}

JMSMsg CMsgManager::GenerateUninitialize()
{
	JMSMsg strMsg=GenerateMsgHeader();
	CreateBodyHeader(strMsg,2);
	
	PutTabbledChar(strMsg,2);

	strMsg+= "<Uninitialize/>";
	CreatreBodyFooter(strMsg);
	GenerateMsgFooter(strMsg);
	return strMsg;	
}


JMSMsg CMsgManager::GenerateSyncBegin()
{
	JMSMsg	strMsg=GenerateMsgHeader();
	CreateBodyHeader(strMsg,3);
	
	PutTabbledChar(strMsg,2);

	strMsg+= "<SyncBegin/>";
	
	CreatreBodyFooter(strMsg);
	GenerateMsgFooter(strMsg);
	return strMsg;
}

JMSMsg CMsgManager::GenerateSyncEnd()
{
	JMSMsg	strMsg=GenerateMsgHeader();
	CreateBodyHeader(strMsg,4);

	PutTabbledChar(strMsg,2);

	strMsg+= "<SyncEnd/>";

	CreatreBodyFooter(strMsg);
	GenerateMsgFooter(strMsg);
	return strMsg;
}

JMSMsg CMsgManager::GenerateTriggerMd( DA_entryDb* pEntryBlock, DWORD dwMdNumber, DA_metaDb* pFirstMdBlock)
{
	if(!(pEntryBlock&&pFirstMdBlock))
	{
		glog(Log::L_ERROR,"CMsgManager::GenerateTriggerMd():NULL pEntryBlock or pFirstMdBlock pass in");
		return NULL;
	}
	return NULL;
}

JMSMsg CMsgManager::GenerateTriggerState(DA_entryDb* pEntryBlock, DA_stateDb* pStateBlock)
{
	if(!(pEntryBlock&&pStateBlock))
	{
		glog(Log::L_ERROR,"CMsgManager::GenerateTriggerState()::NULL pEntryBlock or pStateBlock pass in");
		return NULL;
	}
#define		BUF_SIZE 16
	char	szBuf[BUF_SIZE];

	JMSMsg strMsg=GenerateMsgHeader();
	CreateBodyHeader(strMsg,9);
	
	PutTabbledChar(strMsg,2);

	strMsg+= "<TriggerState ";

	strMsg+= "EntryType=\"";
	itoa(pEntryBlock->_dwEntryType,szBuf,10);
	strMsg+= szBuf;

	strMsg+= "\" EntryUID=\"";
	itoa(pEntryBlock->_dwEntryUID,szBuf,10);
	strMsg+= szBuf;

	strMsg+= "\" LocalEntryUID=\"";
	ZeroMemory(szBuf,BUF_SIZE);
	wcstombs(szBuf,pEntryBlock->_szLocalEntryUID,BUF_SIZE);
	strMsg+= szBuf;
	
	strMsg+= "\" ProviderID=\"";
	ZeroMemory(szBuf,BUF_SIZE);
	wcstombs(szBuf,pEntryBlock->_szProviderID,BUF_SIZE);
	strMsg+= szBuf;
	
	strMsg+= "\" ProviderAssetID=\"";
	ZeroMemory(szBuf,BUF_SIZE);
	wcstombs(szBuf,pEntryBlock->_szProviderAssetID,BUF_SIZE);
	strMsg+= szBuf;

	strMsg+= "\" State=\"";
	itoa(pStateBlock->_dwEntryState,szBuf,10);
	strMsg+= szBuf;

	strMsg+= "\"/>";
	CreatreBodyFooter(strMsg);
	GenerateMsgFooter(strMsg);
	return strMsg;
#undef		BUF_SIZE		
}









//////////////////////////////////////////////////////////////////////////
#ifdef TEST
JMSMsg CMsgManager::GenerateTestMsg()
{
	JMSMsg	strMsg;
	char	szBUf[]={'0','1','2','3','4','5','6','7','8','9'};
	for(int i=0;i<1000;i++)
	{	
		strMsg+=szBUf[i%10];
	}
	return strMsg;
}

#endif