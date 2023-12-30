#include "stdafx.h"
#include "Parser.h"
#include "Markup.h"
#include "messagemacro.h"
#include "..\common\ResKey.h"
#import "msxml3.dll"
#include "OnDataEventCB.h"
//#import "msxml4.dll"
using namespace MSXML2;
extern  BOOL	g_bStop;
extern ZQ::common::Log* _logger;
#include "receivejmsmsg.h"
const int  WAIT_TIME =180000;
extern BOOL g_ConnectJBossIsOK;
UINT DODReceiveThread(LPVOID lpParam);
UINT DODReConnectJBossThread(LPVOID lpParam);
extern CJMSPortManager *g_jmsPortManager;
#define CRECEIVEJMSMSG "ReceiveJmsMsg"
// receivejmsmsg.cpp: implementation of the CReceiveJmsMsg class.

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//class CReceiveJmsMsg 
//Construction/Destruction
//////////////////////////////////////////////////////////////////////
CReceiveJmsMsg::CReceiveJmsMsg(CJMSPortManager *client)
{
	m_pJmsPortManage = client;
	m_hStartReceive = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_Session = NULL;  
	m_destination = NULL;
	m_Consumer = NULL;
	m_hSendThread = NULL;
	m_dwThreadID = 0;
	
}
CReceiveJmsMsg::~CReceiveJmsMsg()
{	
	SetEvent(m_hStartReceive);
	
	if( m_hSendThread )	
	{
		WaitForSingleObject( m_hSendThread, INFINITE );
		m_hSendThread = NULL;
	}
	
    m_Session = NULL;  
	m_destination = NULL;
	m_Consumer = NULL;
	CloseHandle(m_hStartReceive);
}
BOOL CReceiveJmsMsg::init()
{
	ResetEvent(m_hStartReceive);
	
	m_hSendThread = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)DODReceiveThread,this,0 , &m_dwThreadID); 
	
	//	m_hSendThread = AfxBeginThread(DODReceiveThread,this,THREAD_PRIORITY_NORMAL);
	//   m_dwThreadID = 0;
	if(m_hSendThread)
	{ 		
		(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CRECEIVEJMSMSG,
			"init() Create Receive Theread success, ThreadId (%d) queuename (%s)"), 
			m_dwThreadID, m_strQueueName);
	}
	else
		return FALSE;
	return TRUE;
}
UINT DODReceiveThread(LPVOID lpParam) 
{
    CReceiveJmsMsg *pRecvMsg = (CReceiveJmsMsg *)lpParam;
	::std::string m_strQueueName = pRecvMsg->m_strQueueName;
	DWORD m_dwThreadID = pRecvMsg->m_dwThreadID;
	while(!g_bStop)
	{
		(*_logger)(ZQ::common::Log::L_INFO,
			"[%d,%s]DODReceiveThread() Wait Receive Message...",
			m_dwThreadID,m_strQueueName.c_str());
		
		WaitForSingleObject(pRecvMsg->m_hStartReceive,INFINITE);
        
		try
		{	
			while((pRecvMsg->m_Consumer)->receive(
				WAIT_TIME,pRecvMsg->m_jmsTextMsg) && !g_bStop)
			{
				if (NULL == pRecvMsg->m_jmsTextMsg._message)
				{			
					(*_logger)(ZQ::common::Log::L_INFO,
						"[%d,%s]DODReceiveThread() Receive Msg TimeOut",
						m_dwThreadID,m_strQueueName.c_str());
					continue;
				}
				(*_logger)(ZQ::common::Log::L_INFO,
					"DODReceiveThread(),receive a msg.-------",
					m_dwThreadID,m_strQueueName.c_str());
				
				char name[MAX_PATH];
				memset(name,0,MAX_PATH);
				CJMSTxMessage *tm = (CJMSTxMessage *)(&pRecvMsg->m_jmsTextMsg);
				
				tm->getStringProperty("MESSAGECLASS",name,MAX_PATH);
				
				if(strcmpi(name,"COMMAND") == 0)
				{
					(*_logger)(ZQ::common::Log::L_DEBUG, 
						"[%d,%s]CReceiveJmsMsg::onMessage ParseCommand",
						m_dwThreadID,m_strQueueName.c_str());
				}
				else if(strcmpi(name,"STATUS") == 0)
				{
					pRecvMsg->ParseStatus(tm);  
				}
				else if(strcmpi(name,"NOTIFICATION") == 0)
				{
					pRecvMsg->parseNotification(tm);
				}
				else
				{
					(*_logger)(ZQ::common::Log::L_DEBUG,
					"[%d,%s] tm->getStringProperty MESSAGECLASS,name,"
					" MAX_PATH is other error",m_dwThreadID,
					m_strQueueName.c_str());
					continue ;
				}
			}
			
			if(g_bStop)
			{
				break;
			}
			
			ResetEvent(pRecvMsg->m_hStartReceive);
			
			(*_logger)(ZQ::common::Log::L_INFO, 
			"[%d,%s]DODReceiveThread()JBoss connect error or service Exit "
			" DCA Service g_bStop = %d",m_dwThreadID,m_strQueueName.c_str(),g_bStop);
			
			if( g_jmsPortManager->m_jms->m_bConnectionOK && !g_bStop)
			{
				(*_logger)(ZQ::common::Log::L_INFO, 
					"[%d,%s]DODReceiveThread()receive error,"
					"SetEvent(CJmsProcThread::m_hReConnectJBoss) "
					,m_dwThreadID,m_strQueueName.c_str());

				SetEvent(CJmsProcThread::m_hReConnectJBoss);
			}			
		}	
		catch (...)
		{
			int nError = GetLastError();
			char strError[500];
			
			GetErrorDescription(nError, strError);
			(*_logger)(ZQ::common::Log::L_DEBUG,
			"[%d,%s]DODReceiveThread() onMessage  error. GetLastError() =%d,"
			" ErrorDescription = %s",m_dwThreadID,
			m_strQueueName.c_str(),nError, strError);
		}
	}
	
	(*_logger)(ZQ::common::Log::L_DEBUG,
	"[%d,%s]DODReceiveThread() Exit DODReceiveThread  OK"
	" g_bStop = %d",m_dwThreadID,m_strQueueName.c_str(),g_bStop);
	return 1;
}

CJMSTxMessage  CReceiveJmsMsg::ParseCommand(CJMSTxMessage* pMessage)
{
	if (pMessage == NULL)
	{
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]ParseCommand() Message is null "),
			m_dwThreadID,m_strQueueName);
		return *pMessage;
	}
	CJMSTxMessage *tm = (CJMSTxMessage *)pMessage;
	
	if (tm->_message == NULL)
	{
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]ParseCommand() fail to get message handle"),
			m_dwThreadID,m_strQueueName);
		return *pMessage;
	}
	int datasize = tm->GetDataSize();
	
	char *name = new char[datasize];
	
	memset(name,0,datasize);
	tm->getText(name,datasize);
	
	CString data;
	data=name;
	
	delete name;
	
	// these codes will used to reply JMSCommand .but now the DCA project was not used it;
	(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
		"[%d,%s]ParseCommand() received a msg ,type=Command "),
		m_dwThreadID,m_strQueueName );
	//	(*_logger)(ZQ::common::Log::L_DEBUG,"%s",data);
	
	CJMSTxMessage destmessage;
	CString replystring = "reply ";
	//replystring+=data;
    destmessage.setStringProperty("MESSAGECLASS","NOTIFICATION");
	
	CMarkup m_XmlDOM;
	m_XmlDOM.AddElem(DODMESSAGEFLAG);
	
	m_XmlDOM.IntoElem();
	m_XmlDOM.AddElem(DODMESSAGEHEADER );	
	m_XmlDOM.AddAttrib( DODMESSAGETIME, GetCurrDateTime()); 
	m_XmlDOM.AddAttrib( DODMESSAGECODE, FULLDATAWITHTCP);
	
	m_XmlDOM.IntoElem();
	m_XmlDOM.AddElem( DODMESSAGEBODY );	
	m_XmlDOM.AddAttrib( CONTENTRESPONSECODE,1);
	
	m_XmlDOM.IntoElem();
	m_XmlDOM.AddElem( DODMESSAGECONTENT );	
	
	int nMessageID = 0;
	m_XmlDOM.AddAttrib( MESSAGEMESSAGEID, nMessageID);
	
	replystring=m_XmlDOM.GetDoc();
	//	destmessage.set(replystring.GetBuffer(0));
	return destmessage;
	
}
int CReceiveJmsMsg::ParseStatus(CJMSTxMessage* pMessage)
{
	if (pMessage == NULL)
	{
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"ParseStatus() Message is null "));
		
		return 0;
	}
	CJMSTxMessage *tm = (CJMSTxMessage *)pMessage;
	
	if (tm->_message == NULL)
	{
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"ParseStatus()  fail to get message handle"));
		
		return 0;
	}
	
	int datasize = tm->GetDataSize();
	
	char *name = new char[datasize];
	
	memset(name,0,datasize);
	tm->getText(name,datasize);
	
	CString data;
	data = name;
	delete name;
	(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
		"[%d,%s]ParseStatus() Leave"),m_dwThreadID,m_strQueueName);
	//	Clog( LOG_DEBUG,data);
	return 0;
}
int CReceiveJmsMsg::parseNotification(CJMSTxMessage* pMessage)
{
	if (pMessage == NULL)
	{
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"parseNotification() Message is null"));
		return 1;
	}
	CJMSTxMessage *tm = (CJMSTxMessage *)pMessage;
	
	if (tm->_message == NULL)
	{
		(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CRECEIVEJMSMSG,
			"parseNotification() fail to get message handle"));
		return 1;
	}
	
	int datasize = tm->GetDataSize();
	
	char *name = new char[datasize];
	
	memset(name,0,datasize);
	
	tm->getText(name,datasize);
	
	CString data;
	data = name;
	delete name;
	
	(*_logger)(ZQ::common::Log::L_DEBUG,  CLOGFMT(CRECEIVEJMSMSG,
	"[%d,%s]parseNotification() received a msg ,type = Notification ,Message "
	" content is below :"),m_dwThreadID,m_strQueueName);
	
	//	printf("%s", data);
	
	CMarkup m_XmlDOM;	
	m_XmlDOM.SetDoc(data);
	
	if( m_XmlDOM.FindElem( CONTENTCONFRESPONSE ) )
	{
		(*_logger)(ZQ::common::Log::L_DEBUG,  CLOGFMT(CRECEIVEJMSMSG,
		"parseNotification() The message flag of Port configuration message is found in this message"));
		
		::std::string  strCurDir;
		char           sModuleName[1025];
		int            nUsingJBoss = 0;
		DWORD dSize = GetModuleFileNameA(NULL,sModuleName,1024);
		sModuleName[dSize] = '\0';
		strCurDir = sModuleName;
		int nIndex = strCurDir.rfind('\\');
		strCurDir = strCurDir.substr(0,nIndex); //end with "\\"

		nIndex = strCurDir.rfind('\\');
		strCurDir = strCurDir.substr(0,nIndex); //end with "\\"

		strCurDir = strCurDir + "\\etc\\DODAppPortConfigration.xml";
		
		(*_logger)(ZQ::common::Log::L_DEBUG,  CLOGFMT(CRECEIVEJMSMSG,
			"Back up PortConfig info, filepath '%s'"), strCurDir.c_str());
		FILE *fp = fopen(strCurDir.c_str(),"w");
		if(fp != NULL)
		{
			fwrite((LPCTSTR)data, data.GetLength(), 1, fp);
			(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"parseNotification()Create PortConfig back up file successfully"));
			fclose(fp);
		}
		
		if(!ReceiverPortConfigMsg(data.GetBuffer(0),0,NULL))
			return 1;
		
		return 0;
	}
	
	//	printf("%s",data);
    (*_logger)(ZQ::common::Log::L_DEBUG, "%s",data);
	
	
	if(!m_XmlDOM.FindElem( DODMESSAGEFLAG ) )
	{
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"parseNotification() ReceiverMsg Parser- FindElem(DODMESSAGEFLAG)error"));
		return 1;
	}
	m_XmlDOM.IntoElem();
	
	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"parseNotification()ReceiverMsg Parser- !FindElem(DODMESSAGEHEADER)"));
		return 1;
	}
	try
	{
		int messagecode = 0;
		messagecode = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( DODMESSAGECODE ));
		
		CString temp = m_XmlDOM.GetDoc();
		switch(messagecode) 
		{	
		case PORTCONFIGURATION:
			(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
				"parseNotification() call ReceiverPortConfigMsg()"));
			ReceiverPortConfigMsg(temp.GetBuffer(0),0,NULL);		
			break;
		case FULLDATAWITHSHAREDFOLDERS:
		case UPDATEINSHAREFOLDERMODE:
			(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]The message's  messageCode note: this message type is "
			" shared folder format "),m_dwThreadID,m_strQueueName);
			ReceiverDataFolder(temp.GetBuffer(0),0);
			break;
		case FULLDATAWITHTCP:
			(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]The message's  messageCode note: this message type is "
			 "message format "),m_dwThreadID,m_strQueueName);
			ReceiverDataTCP(temp.GetBuffer(0),0);
			break;
		default:
			(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
				"[%d,%s]ReceiverMsg unknowable message code identifier"),
				m_dwThreadID,m_strQueueName);
			return 1;
		}
	}
	catch (...) 
	{
		int nError = GetLastError();
		char strError[500];
		
		GetErrorDescription(nError, strError);
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]Process Notification:error(%d)(%s)"),
			m_dwThreadID,m_strQueueName,nError, strError);
	}	
	(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
		"[%d,%s]Parser Notification Message complete"),
		m_dwThreadID,m_strQueueName);
	
	return 0;
}

CString CReceiveJmsMsg::GetCurrDateTime()
{
	SYSTEMTIME time; 
	GetLocalTime(&time);
	CString sTime;
	sTime.Format("%04d%02d%02d %02d:%02d:%02d", 
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, 
		time.wSecond, time.wMilliseconds );
	return sTime;
}

CString CReceiveJmsMsg::GetDataTypeInitialMsg(CString sDataType,int nReserver)
{
	CString str;
	CMarkup m_XmlDOM;
	
	m_XmlDOM.AddElem(DODMESSAGEFLAG );
	m_XmlDOM.IntoElem();
	
	m_XmlDOM.AddElem( DODMESSAGEHEADER );	
	
	m_XmlDOM.AddAttrib( DODMESSAGETIME, GetCurrDateTime()); 
	m_XmlDOM.AddAttrib( DODMESSAGECODE, FIRSTDATATYPEMESSAGE);
	
	m_XmlDOM.AddElem( DODMESSAGEBODY );		
	m_XmlDOM.AddAttrib( MESSAGEDATATYPE,sDataType);	
	m_XmlDOM.AddAttrib( MESSAGEDODSTARTUP,nReserver);	
	
	str = m_XmlDOM.GetDoc();
	return str;
}

CString CReceiveJmsMsg::SendGetFullDateMsg(int id1,CString id2,int id3)
{
	CString str;
	CMarkup m_XmlDOM;
	
	m_XmlDOM.AddElem(DODMESSAGEFLAG );
	m_XmlDOM.IntoElem();
	
	m_XmlDOM.AddElem( DODMESSAGEHEADER );	
	
	m_XmlDOM.AddAttrib( DODMESSAGETIME, GetCurrDateTime()); 
	str.Format("%d",GETFULLDATA);
	m_XmlDOM.AddAttrib( DODMESSAGECODE, str);
	
	m_XmlDOM.AddElem( DODMESSAGEBODY );		
	m_XmlDOM.AddAttrib( MESSAGEDATATYPE,id2);	
	m_XmlDOM.AddAttrib( MESSAGEGROUPID,id1);	
	
	str = m_XmlDOM.GetDoc();
	
	return str;
}


CString CReceiveJmsMsg::SendGetConfig()
{
	CString str = "";	
	
	CMarkup m_XmlDOM;	
	m_XmlDOM.AddElem( MESSAGECONFREQUEST );	
	m_XmlDOM.AddAttrib( MESSAGECONFREQUESTKEY, 0); 
	m_XmlDOM.AddAttrib( MESSAGEGROUPIDAPPNAME, CONTENTMESSAGEDOD); 
	m_XmlDOM.AddAttrib( MESSAGETYPE, 3); 
	m_XmlDOM.AddAttrib(MESSAGEFORMAT, 2); 
	str = m_XmlDOM.GetDoc();
	return str;
}
CString CReceiveJmsMsg::RemoveBlank(CString oldStr)
{
	try
	{
		if (oldStr.GetLength() == 0)
			return "";
		
		oldStr.TrimLeft(" ");
		oldStr.TrimRight(" ");
		
		if (oldStr.GetLength() == 0)
			return "";
		
		int nIndex=0;
		
		while (nIndex=oldStr.Find(" ;") >= 0)
		{
			oldStr.Delete(nIndex);
			if (oldStr.GetLength() == 0)
				return "";
		}
		if (oldStr.GetLength() == 0)
			return "";
		
		while (nIndex=oldStr.Find("; ") >= 0)
		{
			oldStr.Delete(nIndex+1);
			if (oldStr.GetLength() == 0)
				return "";
		}
		if (oldStr.GetLength() == 0)
			return "";
		
	}
	catch (...) 
	{
		int nError = GetLastError();
		char strError[500];
		
		GetErrorDescription(nError, strError);
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
		"[%d,%s]RemoveBlank error:(%s)(%d)(%s)"),m_dwThreadID,m_strQueueName,
		oldStr,nError,strError);
		return "";
	}
	return oldStr;
}
int CReceiveJmsMsg::ReceiverDataFolder(char *buf,int buffersize)
{
	CString str = buf;
	CString strDatatype = _T("");
	CString rootpath = "";
	CMarkup m_XmlDOM;
	CString subPath = "";
	int GroupID;
	int datatype;
	int verNumber = 0;
    TianShanIce::Application::DataOnDemand::DataPointPublisherPrx dataPrx;
	
	(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
		"[%d,%s]ReceiverDataFolder() Begin Receiver Data Folder"),
		m_dwThreadID,m_strQueueName);
	
	str = "";
	
	m_XmlDOM.SetDoc(buf);
	
	if( !m_XmlDOM.FindElem( DODMESSAGEFLAG) )
	{
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]:ReceiverDataFolder() Parser- !FindElem(DODMESSAGEFLAG)"),
			m_dwThreadID,m_strQueueName);
		(*_logger)(ZQ::common::Log::L_WARNING, CLOGFMT(CRECEIVEJMSMSG,
		"[%d,%s]ReceiverDataFolder() fail to  parser"),m_dwThreadID,m_strQueueName);
		return DODRECEIVERDATAFOLDERERROR;
	}
	m_XmlDOM.IntoElem();
	
	if(!m_XmlDOM.FindElem( DODMESSAGEHEADER ))
	{
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]ReceiverDataFolder Parser- !FindElem(DODMESSAGEHEADER)"),
			m_dwThreadID,m_strQueueName );
		(*_logger)(ZQ::common::Log::L_WARNING, CLOGFMT(CRECEIVEJMSMSG,
		"[%d,%s]ReceiverDataFolder() fail to  parser"),
		m_dwThreadID,m_strQueueName );
		return DODRECEIVERDATAFOLDERERROR;
	}
	int messagecode = 0;
	messagecode = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib(DODMESSAGECODE));
		  
	if( !m_XmlDOM.FindElem( DODMESSAGEBODY) )
	{
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]ReceiverDataFolder Parser- !FindElem(DODMESSAGEBODY)"),
			m_dwThreadID ,m_strQueueName);
		(*_logger)(ZQ::common::Log::L_WARNING, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]ReceiverDataFolder() fail to  parser"),
			m_dwThreadID,m_strQueueName );
		return DODRECEIVERDATAFOLDERERROR;
	}
	
	strDatatype = m_XmlDOM.GetAttrib( MESSAGEDATATYPE );
	datatype = atoi((LPCTSTR)strDatatype);
    GroupID = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEGROUPID ));

	CString strVerNum;
	strVerNum = m_XmlDOM.GetAttrib( MESSAGEVERSIONNUM );
	if(strVerNum !="")
		verNumber = _ttoi((LPCTSTR)strVerNum);

	if(verNumber >31 || verNumber < 0)
		verNumber = 0;
	
	int UpdateMode = 0;
	int nReserve = 0;
    m_XmlDOM.IntoElem();
	if( !m_XmlDOM.FindElem(CONTENTFILEOPERATION) )
	{
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]ReceiverDataFolder Parser- !FindElem(CONTENTFILEOPERATION)"),
			m_dwThreadID,m_strQueueName );
		(*_logger)(ZQ::common::Log::L_WARNING, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]ReceiverDataFolder() fail to  parser"),
			m_dwThreadID,m_strQueueName );
		return DODRECEIVERDATAFOLDERERROR;
	}
	rootpath = m_XmlDOM.GetAttrib(CONTENTROOT);
	UpdateMode = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib(CONTENTUPDATEMODE));
	
	(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
		"[%d,%s]ReceiverDataFolder() DataType = %d, GroupID = %d, remotepath = %s"),
		m_dwThreadID ,m_strQueueName,datatype,GroupID,rootpath);
	try
	{	
		dataPrx = m_pJmsPortManage->GetDataPointPublisherPrx();
		if(dataPrx == NULL)
		{
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
				"fail to get datapublisher porxy"));
			return FALSE;
		}

		TianShanIce::Application::DataOnDemand::OnDataEventCBPtr pOnDataEventCB = new TianShanIce::Application::DataOnDemand::OnDataEventCB;
		TianShanIce::Application::DataOnDemand::DataPointPublisherPrx interalPorxy;

		if (UpdateMode == 0)
		{
			(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]ReceiverDataFolder() UpdateMode = 0,means Full update"),
			m_dwThreadID,m_strQueueName);	

			char strTemp[32];
			memset(strTemp, 0 , 32);

			TianShanIce::Properties props;
			itoa(GroupID, strTemp, 10);
			props[SHAREFOLDER_GROUPID] = strTemp;
			memset(strTemp, 0 , 32);

			props[SHAREFOLDER_DATATYPE] = strDatatype.GetBuffer(0);

			itoa(verNumber, strTemp, 10);
			props[SHAREFOLDER_VERSIONNUM] = strTemp;
			memset(strTemp, 0 , 32);

			props[SHAREFOLDER_ROOTPATH] = rootpath.GetBuffer(0);
			props[SHAREFOLDER_ISDELEDIR] ="1";

			interalPorxy = TianShanIce::Application::DataOnDemand::DataPointPublisherPrx::uncheckedCast(dataPrx->ice_collocationOptimized(false));
			interalPorxy->OnDataEvent_async(pOnDataEventCB, TianShanIce::Application::DataOnDemand::onFullUpdate, props);
			/*dataPrx->notifyFolderFullUpdate(GroupID, datatype,
				rootpath.GetBuffer(0),true,verNumber);	*/						
		}
		else 
			if (UpdateMode == 4)
			{
				// is the file with full path to be Modofied	
				(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
				"[%d,%s]ReceiverDataFolder() UpdateMode = 4, means File Modified"),
				m_dwThreadID,m_strQueueName);

				char strTemp[32];
				memset(strTemp, 0 , 32);

				TianShanIce::Properties props;
				itoa(GroupID, strTemp, 10);
				props[SHAREFOLDER_GROUPID] = strTemp;
				memset(strTemp, 0 , 32);

				props[SHAREFOLDER_DATATYPE] = strDatatype.GetBuffer(0);

				itoa(verNumber, strTemp, 10);
				props[SHAREFOLDER_VERSIONNUM] = strTemp;
				memset(strTemp, 0 , 32);

				props[SHAREFOLDER_ROOTPATH] = rootpath.GetBuffer(0);
				props[SHAREFOLDER_SUBPATH] =  rootpath.GetBuffer(0);

				interalPorxy = TianShanIce::Application::DataOnDemand::DataPointPublisherPrx::uncheckedCast(dataPrx->ice_collocationOptimized(false));
				interalPorxy->OnDataEvent_async(pOnDataEventCB,TianShanIce::Application::DataOnDemand::onFileModified, props);

				/*dataPrx->notifyFileModified(GroupID,datatype,
					rootpath.GetBuffer(0),rootpath.GetBuffer(0),verNumber);*/							
			}
			else
			{		
				while(m_XmlDOM.FindChildElem("File"))
				{
					m_XmlDOM.IntoElem();
					subPath = m_XmlDOM.GetAttrib(CONTENTPATH);
					(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
					"[%d,%s]ReceiverDataFolder()Update_format=(%d) file_attrib(%s)")
					,m_dwThreadID,m_strQueueName,
					UpdateMode,subPath);

					char strTemp[32];
					TianShanIce::Properties props;
					memset(strTemp, 0 , 32);

					switch(UpdateMode)
					{
					case 1:
						// the sub folder to be updated
						(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
						"[%d,%s]ReceiverDataFolder()UpdateMode = 1, means Partly update"),
						m_dwThreadID,m_strQueueName);
						
						itoa(GroupID, strTemp, 10);
						props[SHAREFOLDER_GROUPID] = strTemp;
						memset(strTemp, 0 , 32);

						props[SHAREFOLDER_DATATYPE] = strDatatype.GetBuffer(0);

						itoa(verNumber, strTemp, 10);
						props[SHAREFOLDER_VERSIONNUM] = strTemp;
						memset(strTemp, 0 , 32);

						props[SHAREFOLDER_ROOTPATH] = rootpath.GetBuffer(0);
						props[SHAREFOLDER_SUBPATH] = subPath.GetBuffer(0);

						interalPorxy = TianShanIce::Application::DataOnDemand::DataPointPublisherPrx::uncheckedCast(dataPrx->ice_collocationOptimized(false));
						interalPorxy->OnDataEvent_async(pOnDataEventCB,TianShanIce::Application::DataOnDemand::onPartlyUpdate, props);
			
						/*dataPrx->notifyFolderPartlyUpdate
							(GroupID,datatype,rootpath.GetBuffer(0),
							subPath.GetBuffer(0),verNumber);*/						
						
						break;
					case 2:
						(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
						"[%d,%s]ReceiverDataFolder()UpdateMode = 2, means Folder  delete"),
						m_dwThreadID,m_strQueueName);
						//the sub folder to be deleted

						itoa(GroupID, strTemp, 10);
						props[SHAREFOLDER_GROUPID] = strTemp;
						memset(strTemp, 0 , 32);

						props[SHAREFOLDER_DATATYPE] = strDatatype.GetBuffer(0);

						itoa(verNumber, strTemp, 10);
						props[SHAREFOLDER_VERSIONNUM] = strTemp;
						memset(strTemp, 0 , 32);

						props[SHAREFOLDER_SUBPATH] = subPath.GetBuffer(0);

						interalPorxy = TianShanIce::Application::DataOnDemand::DataPointPublisherPrx::uncheckedCast(dataPrx->ice_collocationOptimized(false));
						interalPorxy->OnDataEvent_async(pOnDataEventCB, TianShanIce::Application::DataOnDemand::onFolderDeleted, props);

						/*dataPrx->notifyFolderDeleted(GroupID,datatype,
							subPath.GetBuffer(0),verNumber);*/						
						break;
					case 3:
						(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
						"[%d,%s]ReceiverDataFolder() UpdateMode = 3, means File delete"),
						m_dwThreadID,m_strQueueName);
						// the file with full path to be deleted

						itoa(GroupID, strTemp, 10);
						props[SHAREFOLDER_GROUPID] = strTemp;
						memset(strTemp, 0 , 32);

						props[SHAREFOLDER_DATATYPE] = strDatatype.GetBuffer(0);

						itoa(verNumber, strTemp, 10);
						props[SHAREFOLDER_VERSIONNUM] = strTemp;
						memset(strTemp, 0 , 32);

						props[SHAREFOLDER_ROOTPATH] = rootpath.GetBuffer(0);
						
						interalPorxy = TianShanIce::Application::DataOnDemand::DataPointPublisherPrx::uncheckedCast(dataPrx->ice_collocationOptimized(false));
						interalPorxy->OnDataEvent_async(pOnDataEventCB,TianShanIce::Application::DataOnDemand::onFileDeleted, props);

						/*dataPrx->notifyFileDeleted(GroupID,datatype,
							rootpath.GetBuffer(0),verNumber);*/												
						break;
					case 5:
						(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
						"[%d,%s]ReceiverDataFolder()UpdateMode = 5, means File new"),
						m_dwThreadID,m_strQueueName);
						// the file with full path to be newed
						//if(teport->m_channel[channelIndex]->FullPathToNew(sTmp,nReserve,sTmp))			

						itoa(GroupID, strTemp, 10);
						props[SHAREFOLDER_GROUPID] = strTemp;
						memset(strTemp, 0 , 32);

						props[SHAREFOLDER_DATATYPE] = strDatatype.GetBuffer(0);

						itoa(verNumber, strTemp, 10);
						props[SHAREFOLDER_VERSIONNUM] = strTemp;
						memset(strTemp, 0 , 32);

						props[SHAREFOLDER_ROOTPATH] = rootpath.GetBuffer(0);
						props[SHAREFOLDER_SUBPATH] = subPath.GetBuffer(0);

						interalPorxy = TianShanIce::Application::DataOnDemand::DataPointPublisherPrx::uncheckedCast(dataPrx->ice_collocationOptimized(false));
						interalPorxy->OnDataEvent_async(pOnDataEventCB,TianShanIce::Application::DataOnDemand::onFileAdded, props);

						/*dataPrx->notifyFileAdded(GroupID,datatype,
							rootpath.GetBuffer(0),subPath.GetBuffer(0),verNumber);	*/					
						break;
					case 6:
						(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
						"[%d,%s]ReceiverDataFolder() UpdateMode=6, means Folder Update"),
						m_dwThreadID,m_strQueueName);
						//Only update file ,Don't delete old file

						itoa(GroupID, strTemp, 10);
						props[SHAREFOLDER_GROUPID] = strTemp;
						memset(strTemp, 0 , 32);

						props[SHAREFOLDER_DATATYPE] = strDatatype.GetBuffer(0);

						itoa(verNumber, strTemp, 10);
						props[SHAREFOLDER_VERSIONNUM] = strTemp;
						memset(strTemp, 0 , 32);

						props[SHAREFOLDER_ROOTPATH] = rootpath.GetBuffer(0);
						props[SHAREFOLDER_ISDELEDIR] ="0";

						interalPorxy = TianShanIce::Application::DataOnDemand::DataPointPublisherPrx::uncheckedCast(dataPrx->ice_collocationOptimized(false));
						interalPorxy->OnDataEvent_async(pOnDataEventCB,TianShanIce::Application::DataOnDemand::onFullUpdate, props);

						/*dataPrx->notifyFolderFullUpdate(GroupID, datatype,
							rootpath.GetBuffer(0),false,verNumber);	*/											
						break;						
					default:
						break;
						m_XmlDOM.OutOfElem();	
					}	
					//m_XmlDOM.OutOfElem();		
				}
			}	
	}
	catch (const ::TianShanIce::InvalidParameter & ex) 
	{
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverDataFolder() caught TianShanIce::InvalidParameter(%s)"),
			ex.message.c_str());
		return FALSE;
    }  
	catch (const ::Ice::Exception & ex) 
	{
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverDataFolder() caught Ice::Exception(%s)"),
			ex.ice_name().c_str());
		return FALSE;
	} 
	(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CRECEIVEJMSMSG,
	"[%d,%s]ReceiverDataFolder() parser Receiver Data Folder successfully"),
	m_dwThreadID,m_strQueueName);
	return 0; 
}

int CReceiveJmsMsg::ReceiverDataTCP(char *buf, int buffersize)
{
	(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CRECEIVEJMSMSG,
		"[%d,%s]ReceiverDataTCP() Process ReceiverData message_format"),
		m_dwThreadID,m_strQueueName);
	
	CString str,strMsgContent;
	CMarkup m_XmlDOM;
	CString sDatatype;
	int GroupId;
	int datatype;
	TianShanIce::Application::DataOnDemand::DataPointPublisherPrx dataPrx;
	
	m_XmlDOM.SetDoc(buf);
	
	if( !m_XmlDOM.FindElem(DODMESSAGEFLAG ) )
	{
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]ReceiverDataTCP()Parser- !FindElem(DODMESSAGEFLAG)"),
			m_dwThreadID,m_strQueueName );
		return 1;
	}
	m_XmlDOM.IntoElem();
	
	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]ReceiverDataTCP() Parser- !FindElem(DODMESSAGEHEADER)"),
			m_dwThreadID,m_strQueueName );
		return 1;
	}
	
	int messagecode = 0;
	messagecode = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( DODMESSAGECODE ));
		  
	if (messagecode !=FULLDATAWITHTCP)
		return 1;
		  
	if(!m_XmlDOM.FindElem( DODMESSAGEBODY ))
	{
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]ReceiverDataTCP() Parser- !FindElem(DODMESSAGEBODY)"),
			m_dwThreadID,m_strQueueName);
		return 1;
	}
		  
	sDatatype = m_XmlDOM.GetAttrib( MESSAGEDATATYPE );
	datatype = atoi((LPCTSTR)sDatatype);
	GroupId = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEGROUPID ));
		  
	m_XmlDOM.IntoElem();
		  
	if( !m_XmlDOM.FindElem( DODMESSAGECONTENT))
	{
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
			"[%d,%s]ReceiverDataTCP() Parser- !FindElem(DataList)"),
			m_dwThreadID,m_strQueueName );
		return 1;
	}
	CString strDestinationName,filename,strCancel;
	strDestinationName = m_XmlDOM.GetAttrib( "Destination");
	
	int sssize = strDestinationName.GetLength();
	filename = "000000";
		  
	if (sssize <= JMSMESSAGE_DESTIONATIONFILENAME)
	{	
		filename.Delete(0,sssize);
		filename.Insert(JMSMESSAGE_DESTIONATIONFILENAME-sssize,strDestinationName);
	}
	else
	{
		filename = strDestinationName;
	}
	
	CString sMessageID = m_XmlDOM.GetAttrib( MESSAGEMESSAGEID);
	try
	{
		strCancel = m_XmlDOM.GetAttrib( MESSAGEOPERATION);
		
		if (strCancel.GetLength() >0)
		{
			if (sMessageID.GetLength() == 0)
			{
				(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
					"[%d,%s]ReceiverDataTCP() Parser- nMessageID == 0 error."),
					m_dwThreadID,m_strQueueName );
				return 1;
			}
			if (strCancel.CompareNoCase("cancel") == 0)
			{
				(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CRECEIVEJMSMSG,
					"[%d,%s]ReceiverDataTCP() Parser- received cancel OK."),
					m_dwThreadID,m_strQueueName );
				(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CRECEIVEJMSMSG,
					"Cannel message------StopFileSend!"));	
				
				dataPrx = m_pJmsPortManage->GetDataPointPublisherPrx();
				if(dataPrx == NULL)
				{
					(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
						"ReceiverDataTCP() fail to get datapublisher porxy"));
					return FALSE;
				}
				char strTemp[32];
				memset(strTemp, 0 , 32);

				TianShanIce::Properties props;
				itoa(GroupId, strTemp, 10);
				props[MESSAGE_GROUPID] = strTemp;
				memset(strTemp, 0 , 32);

                props[MESSAGE_DATATYPE] = sDatatype.GetBuffer(0);

				props[MESSAGE_MESSAGEID]=sMessageID.GetBuffer(0);

				TianShanIce::Application::DataOnDemand::OnDataEventCBPtr pOnDataEventCB = new TianShanIce::Application::DataOnDemand::OnDataEventCB();

				TianShanIce::Application::DataOnDemand::DataPointPublisherPrx interalPorxy;
				interalPorxy = TianShanIce::Application::DataOnDemand::DataPointPublisherPrx::uncheckedCast(dataPrx->ice_collocationOptimized(false));
				interalPorxy->OnDataEvent_async(pOnDataEventCB, TianShanIce::Application::DataOnDemand::onMessageDeleted, props);

				/*dataPrx->notifyMessageDeleted(GroupId, datatype,
					sMessageID.GetBuffer(0));*/					
			}
			else
			{
				(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
					"[%d,%s]ReceiverDataTCP() Parser- Unknow command (operation)"),
					m_dwThreadID,m_strQueueName );
				return 1;
			}			
		}
		else
		{
			int leafTime=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "ExpiredTime"));
			
			//Add messageID for filename.
			strDestinationName.Format("_%s",sMessageID);
			filename = strDestinationName + "_" + filename;
			/*
			if (teport->m_channel[channelIndex]->m_bEncrypted)
			{
			strMsgContent = m_XmlDOM.GetData();
			}
			else
			{
			m_XmlDOM.IntoElem();
			bool baaa = m_XmlDOM.FindElem("");
			strMsgContent = m_XmlDOM.GetSubDoc();
			}   
			*/      
			strMsgContent = m_XmlDOM.GetData();
			(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CRECEIVEJMSMSG,
				"ReceiverDataTCP() New message------CreateNewMsg"));
			dataPrx = m_pJmsPortManage->GetDataPointPublisherPrx();
			if(dataPrx == NULL)
			{
				(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
					"ReceiverDataTCP()fail to get datapublisher porxy"));
				return FALSE;
			}
			char strTemp[32];
			memset(strTemp, 0 , 32);

			TianShanIce::Properties props;
			itoa(GroupId, strTemp, 10);
			props[MESSAGE_GROUPID] = strTemp;
			memset(strTemp, 0 , 32);
            
			props[MESSAGE_DATATYPE] = sDatatype.GetBuffer(0);

			props[MESSAGE_MESSAGEID]=sMessageID.GetBuffer(0);
			props[MESSAGE_DESTINATION]=filename.GetBuffer(0);
			props[MESSAGE_MESSAGEBODY]= strMsgContent.GetBuffer(0);

			itoa(leafTime, strTemp, 10);		
			props[MESSAGE_EXPIRATION] = strTemp;
            memset(strTemp, 0 , 32);

			TianShanIce::Application::DataOnDemand::OnDataEventCBPtr pOnDataEventCB = new TianShanIce::Application::DataOnDemand::OnDataEventCB();
			TianShanIce::Application::DataOnDemand::DataPointPublisherPrx interalPorxy;
			interalPorxy = TianShanIce::Application::DataOnDemand::DataPointPublisherPrx::uncheckedCast(dataPrx->ice_collocationOptimized(false));
			interalPorxy->OnDataEvent_async(pOnDataEventCB, TianShanIce::Application::DataOnDemand::onMessageAdded, props);

			/*dataPrx->notifyMessageAdded(GroupId, datatype,
				sMessageID.GetBuffer(0),filename.GetBuffer(0),
				strMsgContent.GetBuffer(0),leafTime);*/						
		}
	}
	catch (const ::TianShanIce::InvalidParameter & ex) 
	{
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverDataTCP() caught TianShanIce::InvalidParameter(%s)"),
			ex.message);
		return FALSE;
	}  
	catch (const ::Ice::Exception & ex) 
	{
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverDataTCP() Ice::Exception errorcode(%s)"),
			ex.ice_name().c_str());
		return FALSE;
	} 
	catch (...) 
	{
		int nError = GetLastError();
		char strError[500];
		
		GetErrorDescription(nError, strError);
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
		"[%d,%s]ReceiverDataTCP()fail to parser, MessageID:(%s) datatype:(%s),(%d)(%s)"),
		m_dwThreadID,m_strQueueName,sMessageID,sDatatype,nError,strError);
		return 0;
	}
	
	(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CRECEIVEJMSMSG,
		"[%d,%s]ReceiverDataTCP() Process ReceiverDataTCP Complete"),
		m_dwThreadID,m_strQueueName);
	return 0;
}
BOOL CReceiveJmsMsg::ReceiverPortConfigMsg(char *buf,
										   int buffersize,int *nReserver)
{
	CString xmlstr;
	CString sTmp = _T("");
	CString strPorttemp;
	ZQMSGPARSER zqmsgparser;
	//BOOL bReturn=FALSE;
	int nCountNumber = 0;
	
	(*_logger)(ZQ::common::Log::L_INFO,  CLOGFMT(CRECEIVEJMSMSG,
		"Enter ReceiverPortConfigMsg()"));
	
	CString name;
	MSXML2::IXMLDOMDocumentPtr pDoc; 
	MSXML2::IXMLDOMElementPtr  childNode ;
	MSXML2::IXMLDOMNodePtr       pNode = NULL;
	MSXML2::IXMLDOMNodePtr       pNodequeue = NULL;
	MSXML2::IXMLDOMNodePtr		 pTmp = 0;
	MSXML2::IXMLDOMNodePtr		 pFather = 0;
	MSXML2::IXMLDOMNodePtr		 pSon = 0;
	MSXML2::IXMLDOMNamedNodeMapPtr pNamedNode = 0;
	COleVariant nodeValue;
	MSXML2::IXMLDOMNodeListPtr childlist; 
	xmlstr.Format("%s",buf);
	
	HRESULT hr = pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument30));
	if(!SUCCEEDED(hr))    
	{	
		//AfxMessageBox("无法创建DOMDocument对象，请检查是否安装了MS XML Parser 运行库!"); 
		(*_logger)(ZQ::common::Log::L_ERROR,  CLOGFMT(CRECEIVEJMSMSG,
		"ReceiverPortConfigMsg() IXMLDOMDocumentPtr::CreateInstance error :Cannot Create "
        "DOMDocument object,Plesae check wheather Setup MS XML Parser runtime Libyary"));
		return FALSE;
	}
	
	//The follow codes was used the programer when pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument));
	
	//pDoc->PutvalidateOnParse(VARIANT_FALSE);
	//pDoc->PutresolveExternals(VARIANT_FALSE);
	//pDoc->PutpreserveWhiteSpace(VARIANT_TRUE);
	
	if(pDoc->loadXML((_bstr_t)CString(xmlstr))!=VARIANT_TRUE)
	{
		
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverPortConfigMsg() pDoc->loadXML((_bstr_t)CString(xmlstr) error"));
		(*_logger)(ZQ::common::Log::L_ERROR,  CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
		return FALSE;
	}
	long nchildNumber = 0;
	
	try
	{	
		//find "CONTENTCONFRESPONSE"
		childNode = (MSXML2::IXMLDOMElementPtr)(pDoc->
			selectSingleNode(CONTENTCONFRESPONSE));
		
		if (childNode == NULL)
		{
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
				"ReceiverPortConfigMsg()pDoc->selectSingleNode(CONTENTCONFRESPONSE erro"));
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
				"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
			return FALSE;
		}
		
		pNode = childNode->GetfirstChild();
		if (pNode == NULL)
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverPortConfigMsg()childNode->GetfirstChild() error"));
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
				"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
			return FALSE;
		}
		name = CString((wchar_t*)pNode->GetnodeName());
		if(name.Compare("ListOfNode")!=0)
		{
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
				"ReceiverPortConfigMsg() Node of ListOfNode is not found"));
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
				"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
			return FALSE;
		}
		
		pNode = pNode->GetfirstChild();
		name = CString((wchar_t*)pNode->GetnodeName());
		if(name.Compare("DCAPortConfiguration")!=0)
		{
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverPortConfigMsg() Node of DCAPortConfiguration not found"));
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
				"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
			return FALSE;
		}
		
		pNamedNode = pNode->Getattributes();
		if(pNamedNode == NULL)
		{	
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverPortConfigMsg Getattributes for DCAPortConfiguration error"));
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
				"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
			return FALSE;
		}
		/*		//the queuename is existed JMS for APP 's configuration
		
		  pTmp = pNamedNode->getNamedItem((_bstr_t)"QueueName");
		  if(pTmp != NULL)
		  {	
		  nodeValue = pTmp->GetnodeValue();
		  m_pm->m_Localqueuename = (CString)((wchar_t*)nodeValue.bstrVal);
		  zqmsgparser.MsgParser = m_Agent->m_Parse;
		  zqmsgparser.QueueName = m_pm->m_Localqueuename;
		  m_Agent->m_VecParser.push_back(zqmsgparser);
		  QueueCount = 1;
		  }	
		*/
		
		pTmp = pNamedNode->getNamedItem((_bstr_t)"DataTypeWithInitial");
		if(pTmp == NULL)
		{	
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverPortConfigMsg() Getattributes for "
			"DataTypeWithInitial error"));
			m_pJmsPortManage->m_sDataTypeInitial="";
		}
		else
		{
			nodeValue = pTmp->GetnodeValue();
			name = RemoveBlank(nodeValue.bstrVal);
			if(name.GetLength()<1)
			{	
				m_pJmsPortManage->m_sDataTypeInitial="";
			}	
			else
				m_pJmsPortManage->m_sDataTypeInitial=name;
		}
	}
	catch (...) 
	{
		
	}
	
	pNodequeue = pNode = pNode->GetfirstChild();
	CString SCheckStr;
	try
	{//try
		while(pNodequeue)
		{//while     
			//get QueueName begin     
			name = CString((wchar_t*)pNodequeue->GetnodeName());

			if(name.Compare("DODQueue") == 0)
			{
				childlist = pNodequeue->GetchildNodes();
				hr = childlist->get_length(&nchildNumber);
				if (nchildNumber < 1)
				{
					(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
						"ReceiverPortConfigMsg() queue number is zero"));
					(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
						"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
					return FALSE;
				}
				(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
					"ReceiverPortConfigMsg() queue number is %d "),nchildNumber);
				
				nCountNumber = nchildNumber;
				
				pSon = pNodequeue->GetfirstChild();
				int k = 0;
				while (pSon)
				{
					name = CString((wchar_t*)pSon->GetnodeName());
					pNamedNode = pSon->Getattributes();
					if(pNamedNode == NULL)
					{	
						(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
						return FALSE;
					}	
					
					pTmp = pNamedNode->getNamedItem((_bstr_t)"Name");
					if(pTmp == NULL)
					{	
						(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
						"ReceiverPortConfigMsg() Get attributes"
						" for queuename %s"), name);
						(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
						return FALSE;
					}	
					nodeValue = pTmp->GetnodeValue();
					CString temp = nodeValue.bstrVal;
					if(temp.GetLength()<1)
					{	
						(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg () IPAddress error"));
						(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
						return FALSE;
					}
					
					(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
						"ReceiverPortConfigMsg() Parser Msg Queue Name='%s'"),temp);
					
					zqmsgparser.QueueName = temp;
					m_pJmsPortManage->m_VecParser.push_back(zqmsgparser);
					
					pSon = pSon->GetnextSibling();
				}				
			}
			//get QueueName end
			else
			{
				if(name.Compare("ChannelList") == 0)
				{
					childlist = pNodequeue->GetchildNodes();
					hr = childlist->get_length(&nchildNumber);
					if (nchildNumber < 1)
					{
						(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() Channel number is zero"));

							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						return FALSE;
					}
								
					pSon = pNodequeue->GetfirstChild();
					int k = 0;
					while (pSon)
					{
						name = CString((wchar_t*)pSon->GetnodeName());
						pNamedNode = pSon->Getattributes();
						if(pNamedNode == NULL)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg()Getattributes error"));

							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
									
						CHANNELINFO *tempchannel = new CHANNELINFO;
						name.MakeLower();
						tempchannel->ChannelName = name;
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)"Encrypted");
						if(pTmp == NULL)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg()fail to get attributes"
							" for Encrypted,channelname=%s"),name);

							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG, 
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() Encrypted error"));

							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						//(*_logger)(ZQ::common::Log::L_DEBUG, ReceiverPortConfigMsg () Encrypted error 1abc %s",SCheckStr.GetBuffer(0) );
						
						nodeValue.ChangeType(VT_INT);
						//(*_logger)(ZQ::common::Log::L_DEBUG, ReceiverPortConfigMsg () Encrypted error 2abc");
						tempchannel->nEncrypted = (ZQDataApp::EncryptMode)nodeValue.intVal;									
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)
							"StreamType");
						if(pTmp == NULL)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to get attributes"
							"for StreamType, portname='%s'"),name);

							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() StreamType error"));

							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue.ChangeType(VT_INT);
						tempchannel->streamType = nodeValue.intVal;
		
						pTmp = pNamedNode->getNamedItem((_bstr_t)"StreamID");
						if(pTmp == NULL)
						{
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() "
							"fail to get attributes for StreamID, portname='%s'"),name);

							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg()fail to get StreamID"));

							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue.ChangeType(VT_INT);
						tempchannel->nstreamId = nodeValue.intVal;
						
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)"Tag");
						if(pTmp == NULL)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() "
							"fail to get attributes for Tag, portname=%s"),name);
							
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(FALSE,SCheckStr))
						{	
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to get Tag"));
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						//	strcpy(tempchannel->Tag,((wchar_t*)nodeValue.bstrVal));
						
						tempchannel->Tag = (CString((wchar_t*)
							nodeValue.bstrVal)).GetBuffer(0);
						pTmp = pNamedNode->getNamedItem((_bstr_t)"QueueName");
						if(pTmp == NULL)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to get attributes for QueueName, portname=%s"),name);

							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG, 
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(SCheckStr.GetLength() <1)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() QueueName error"));

							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}
						tempchannel->strQueueName =
							CString((wchar_t*)nodeValue.bstrVal);

						pTmp = pNamedNode->getNamedItem(
							(_bstr_t)(MESSAGECHANNELTYPE));
						if(pTmp == NULL)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() "
							"failt to get attributes for ChannelType,channel=%s "),name);
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() MESSAGECHANNELTYPE error"));

							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}
						nodeValue.ChangeType(VT_INT);
						tempchannel->nChannelType = nodeValue.intVal;
						
						pTmp = pNamedNode->getNamedItem(
							(_bstr_t)"SendIndexTable");
						if(pTmp == NULL)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg()"
							"fail to get attributes for SendIndexTable,portname=%s"),name);
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() SendIndexTable error"));
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}
						nodeValue.ChangeType(VT_INT);
						tempchannel->nSendWithDestination = 
							nodeValue.intVal;
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)
							"StreamCount");
						if(pTmp == NULL)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() "
							"fail to get attributes for StreamCount,portname=%s"),name);
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to get StreamCount"));
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}
						nodeValue.ChangeType(VT_INT);
						tempchannel->nStreamCount = nodeValue.intVal;
						
						if (tempchannel->nChannelType)
						{
							if (tempchannel->nStreamCount == 0)
							{
								(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg()"
								" Multiplestream=1, but StreamCount=0"));
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
							}
						}
						else
							tempchannel->nStreamCount = 0;
						pTmp = pNamedNode->getNamedItem((_bstr_t)(MESSAGEDATATYPE));
						if(pTmp == NULL)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() "
							"fail to get attributes for MessageDataType,portname=%s"),name);
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = RemoveBlank(nodeValue.bstrVal);
						if (SCheckStr.GetLength() >0)
						{
							tempchannel->strdataType = CString(
								(wchar_t*)nodeValue.bstrVal);
						}
						else
						{
							tempchannel->strdataType = "";
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG, 
							"ReceiverPortConfigMsg()" 
						    "fail to getattributes for m_sDataType,portname=%s"),name);
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}
						
						pTmp = pNamedNode->getNamedItem(
							(_bstr_t)"DataExchangeType");
						if(pTmp == NULL)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() "
							"fail to get attributes for DataExchangeType,portname=%s"),name);
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() DataExchangeType error"));
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}
						nodeValue.ChangeType(VT_INT);
						
						if(nodeValue.intVal == 0)
						{
							tempchannel->DataExchangeType = 
								ZQDataApp::dataSharedFolder;
						}
						else 
							if(nodeValue.intVal == 1)
							{
								tempchannel->DataExchangeType = 
									ZQDataApp::dataMessage ;
							}
							else
								if(nodeValue.intVal == 2)
								{
									tempchannel->DataExchangeType =
										ZQDataApp::dataLocalFolder ;
								}
								else
								{
									(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
									"ReceiverPortConfigMsg() "
									"Getattributes for DataExchangeType,"
								    "portname=%s error,DataExchangeType = %d"),
									name,nodeValue.intVal);
									(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
									return FALSE;
								}
								
								// create ilp file for navigation ....
								pTmp = pNamedNode->getNamedItem((_bstr_t)
									"SendMsgDataType");
								if(pTmp == NULL)
								{	
									(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
									"ReceiverPortConfigMsg()"
									"fail to get attributes for SendMsgDataType,portname=%s"),name);
						        	(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
									"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
									return FALSE;
								}	
								nodeValue = pTmp->GetnodeValue();
								SCheckStr = nodeValue.bstrVal;
								if (SCheckStr.CompareNoCase("0") == 0)
									tempchannel->strSendMsgDataType = "";
								else
								{
									SCheckStr = RemoveBlank(nodeValue.bstrVal);
									if (SCheckStr.GetLength() >0)
									{								
										tempchannel->strSendMsgDataType =
											(LPCSTR) (CString((wchar_t*)
											nodeValue.bstrVal)).GetBuffer(0);
									}
									else
										tempchannel->strSendMsgDataType="";
								}
								
								pTmp = pNamedNode->getNamedItem((_bstr_t)
									"SendMsgExpiredTime");
								if(pTmp == NULL)
								{	
									//				();
									//			
									(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
									"ReceiverPortConfigMsg() "
									"fail to get attributes for SendMsgExpiredTime,portname=%s"),name);
									(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
									"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
									return FALSE;
								}	
								nodeValue = pTmp->GetnodeValue();
								SCheckStr = nodeValue.bstrVal;
								if (SCheckStr.GetLength() >0)
								{
									if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
									{	
										(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg()fail to get SendMsgExpiredTime"));
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
											"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
										return FALSE;
									}
									nodeValue.ChangeType(VT_INT);
									tempchannel->nSendMsgExpiredTime = 
										nodeValue.intVal;
								}
								
								pTmp = pNamedNode->getNamedItem((_bstr_t)
									"MessageCode");
								if(pTmp == NULL)
								{	
									(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg()"
									" fail to get attributes for MessageCode,portname=%s"),name);
									(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
									return FALSE;
								}	
								nodeValue = pTmp->GetnodeValue();
								SCheckStr = nodeValue.bstrVal;
								if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
								{	
									tempchannel->nMessageCode = 0;
									(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
									"ReceiverPortConfigMsg() MessageCode is null"));
								}
								else
								{
									nodeValue.ChangeType(VT_INT);
									tempchannel->nMessageCode = nodeValue.intVal;
								}
								//   			tempchannel->m_channelID = k+1;
								(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() channelname=%s,DataExchangeType=%s, QueueName=%s"),
								tempchannel->ChannelName.c_str(),
								tempchannel->DataExchangeType.c_str(),
								tempchannel->strQueueName.c_str());
								
								m_pJmsPortManage->m_channels[tempchannel->ChannelName] = tempchannel;
								tempchannel = NULL;
								k++;
								pSon = pSon->GetnextSibling();
							}
							
				}//end Channellist
				else
				{
					if(name.Compare("Port") != 0)
					{
						(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() Node of Port is not found"));
						(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
						return FALSE;
					}
					
					// prevent current node is not child or namenode		
					pNamedNode = pNode->Getattributes();
					if(pNamedNode == NULL)
					{	
						(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
						"ReceiverPortConfigMsg() fail to get attributes for"
						"Port error"));
						(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
						return FALSE;
					}	
					childlist = pNode->GetchildNodes();
					hr = childlist->get_length(&nchildNumber);
					if (nchildNumber < 1)
					{
						(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() Port number is zero"));
						(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
						return FALSE;
					}
					
					(*_logger)(ZQ::common::Log::L_INFO,CLOGFMT(CRECEIVEJMSMSG,
						"ReceiverPortConfigMsg() Port number is  %d "),nchildNumber);
					pNode = pNode->GetfirstChild();
					//get all config ports.
					while (pNode)
					{
						DODPORT *tempport = new DODPORT;			//get port layer attrib
						strPorttemp = CString((wchar_t*)pNode->GetnodeName());
					/*	tempport->portname = "Port_" + 
							CString((wchar_t*)pNode->GetnodeName());*/

						tempport->portname = CString((wchar_t*)pNode->GetnodeName());
						
						(*_logger)(ZQ::common::Log::L_INFO,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() get attributes"
						" for portname=%s"),tempport->portname.c_str());
						
						pNamedNode = pNode->Getattributes();
						
						if(pNamedNode == NULL)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() fail to get attributes for portname=%s"),
							tempport->portname.c_str());
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)"PMTPID");
						if(pTmp == NULL)
						{	
							
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() fail to get attributes "
							"for PMTPID, portname=%s"),tempport->portname.c_str());
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}
						//(*_logger)(ZQ::common::Log::L_DEBUG, ReceiverPortConfigMsg : portname = %s ,PMTPID = %s",tempport->portname,pTmp);
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr,
							"getNamedItem((_bstr_t)Encrypted)"))
						{	
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() fail to get PMTPID"));
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}		
						//(*_logger)(ZQ::common::Log::L_DEBUG, ReceiverPortConfigMsg () PMTPID error 1 %s",SCheckStr.GetBuffer(0));
						
						nodeValue.ChangeType(VT_INT);
						tempport->pmtPid = nodeValue.intVal;
						//(*_logger)(ZQ::common::Log::L_DEBUG, ReceiverPortConfigMsg () PMTPID error 2");
						
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)(MESSAGEGROUPID));
						if(pTmp==NULL)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg()fail to get attributes ")
							"for GroupID, portname=%s ",tempport->portname.c_str());
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr=nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to get MESSAGEGROUPID"));
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue.ChangeType(VT_INT);
						tempport->groupId = nodeValue.intVal;
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)"TotalRate");
						if(pTmp == NULL)
						{	
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() fail to get attributes for TotalRate, portname=%s"),
							tempport->portname.c_str());
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							
							(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
							"ReceiverPortConfigMsg() fail to get TotalRate"));
							(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
								"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
							return FALSE;
						}	
						nodeValue.ChangeType(VT_INT);
						tempport->totalBandWidth = nodeValue.intVal;
						
						// get all channels and all castports.
						
						pFather = pNode->GetfirstChild();
						
						//pNode->get
						while (pFather)
						{
							//get child layer attrib,but this layer is possible IPPort or IPChannel.
							name = CString((wchar_t*)pFather->GetnodeName());
							if(name.Compare("IPPort") == 0)
							{
								childlist = pFather->GetchildNodes();
								hr = childlist->get_length(&nchildNumber);
								if (nchildNumber < 1)
								{
									(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg()  IPPort number is zero"));
									(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
									return FALSE;
								}
								
								(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
									"ReceiverPortConfigMsg() PortName='%s', IPPort number is %d "),
									tempport->portname.c_str(), nchildNumber);
								
								pSon = pFather->GetfirstChild();
								
								int k = 0;  //This variable show the number of broadcast ouput
								
								while (pSon)
								{
									IPPORT *pIpPort = new IPPORT;
									name=CString((wchar_t*)pSon->GetnodeName());
									pNamedNode = pSon->Getattributes();
									if(pNamedNode == NULL)
									{	
										(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg() "
										"fail to get attributes for TotalRate, portname=%s "),name);
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
											"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
										return FALSE;
									}	
									
									pTmp = pNamedNode->getNamedItem((_bstr_t)"SendType");
									if(pTmp == NULL)
									{	
										(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg() "
										"fail to get attributes for SendType, portname=%s"),name);
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
											"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
										return FALSE;
									}	
									nodeValue = pTmp->GetnodeValue();
									SCheckStr = nodeValue.bstrVal;
									if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
									{
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg() fail toget SendType"));
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
											"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
										return FALSE;
									}	
									//Note:
									//In Configuration Management System Ver0.0.0.1 interface.if user add a note (IPPort'member),but it's IPAddress.Port and SendType are not configured,
									//their value will be saved to SQL as cstring _ fotmat.Then .ChangeType was be called .there are a error dialog (Type matching). 
									
									nodeValue.ChangeType(VT_INT);
									pIpPort->nSendType = nodeValue.intVal;
									
									pTmp = pNamedNode->getNamedItem((_bstr_t)"IPAddress");
									if(pTmp == NULL)
									{	
										(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg() "
										"fail to get attributes for IPAddress, portname=%s"),name);
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
											"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
										return FALSE;
									}	
									nodeValue = pTmp->GetnodeValue();
									SCheckStr = nodeValue.bstrVal;
									if(SCheckStr.GetLength()<1)
									{	
										(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg() fail to get IPAddress"));
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
											"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
										return FALSE;
									}	
									
									pIpPort->strIp = (LPCSTR) (CString(
										(wchar_t*)nodeValue.bstrVal)).GetBuffer(0); 
									pTmp = pNamedNode->getNamedItem((_bstr_t)"IPPort");
									if(pTmp == NULL)
									{	
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg() "
										"fail to get attributes for IPPort, portname=%s"),name);
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
											"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
										return FALSE;
									}	
									nodeValue = pTmp->GetnodeValue();
									SCheckStr = nodeValue.bstrVal;
									if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
									{	
										(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg()fail to get SendType"));
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
											"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
										return FALSE;
									}	
									nodeValue.ChangeType(VT_INT);
									pIpPort->nPort = nodeValue.intVal;
									pIpPort->IpPortName = name;
									(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CRECEIVEJMSMSG,
									"ReceiverPortConfigMsg() IPPort: name=%s,"
									"SendType=%d, IPAddress=%s ,IPPort=%d"),
									name, pIpPort->nSendType,
									pIpPort->strIp.c_str(),pIpPort->nPort);
									
									tempport->ipportvector.push_back(pIpPort);
									pIpPort = NULL;
									pSon = pSon->GetnextSibling();
									// get a same layer about IPPort attrib
							}
						}
						else
							if(name.Compare("Channel") == 0)
							{
								childlist = pFather->GetchildNodes();
								hr = childlist->get_length(&nchildNumber);
								if (nchildNumber < 1)
								{
									(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg() Channel count is 0"));
									(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
									return FALSE;
								}
								
								(*_logger)(ZQ::common::Log::L_INFO,CLOGFMT(CRECEIVEJMSMSG, 
								"ReceiverPortConfigMsg() PortName=%s, Channel count (%d)"), 
								tempport->portname.c_str(),nchildNumber);
								
								pSon = pFather->GetfirstChild();
								int k = 0;
								while (pSon)
								{
									name = CString((wchar_t*)pSon->GetnodeName());
									pNamedNode = pSon->Getattributes();
									if(pNamedNode == NULL)
									{	
										(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
											"ReceiverPortConfigMsg() fail to get attributes,portname=%s"),name);
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
										return FALSE;
									}	
									CHANNELATTACHINFO *tempchannelattach = 
										new CHANNELATTACHINFO;

									name.MakeLower();
									tempchannelattach->ChannelName = name;
									
									pTmp = pNamedNode->getNamedItem((_bstr_t)
										"RepeatTime");
									if(pTmp == NULL)
									{	
										(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
											"ReceiverPortConfigMsg()fail to get attributes for RepeatTime,portname=%s"),
											name);
										return FALSE;
									}	
									nodeValue = pTmp->GetnodeValue();
									SCheckStr = nodeValue.bstrVal;
									if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
									{	
										(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg() RepeatTime error"));
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
											"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
										return FALSE;
									}	
									nodeValue.ChangeType(VT_INT);
									tempchannelattach->nRepeatetime = 
										nodeValue.intVal;
									
									pTmp = pNamedNode->getNamedItem(
										(_bstr_t)"ChannelRate");
									if(pTmp == NULL)
									{	
										(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg()fail to get attributes for ChannelRate, portname=%s"),
										name);
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
											"ReceiverPortConfigMsg() fail to Parser Port Configuration"));;
										return FALSE;
									}	
									nodeValue = pTmp->GetnodeValue();
									SCheckStr = nodeValue.bstrVal;
									if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
									{	
										(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
										"ReceiverPortConfigMsg()fail to get ChannelRate"));
										(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
											"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
										return FALSE;
									}
									nodeValue.ChangeType(VT_INT);
									tempchannelattach->nChannelRate = 
										nodeValue.intVal;
									
									(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CRECEIVEJMSMSG, 
									"ReceiverPortConfigMsg(): channelname = %s,"
									"RepeatTime = %ld, ChannelRate = %d "),
									tempchannelattach->ChannelName.c_str(),
									tempchannelattach->nRepeatetime,
									tempchannelattach->nChannelRate);
									
									tempport->channelattachMap[tempchannelattach->ChannelName] 
										= tempchannelattach;
									tempchannelattach = NULL;
									k++;
									pSon = pSon->GetnextSibling();
								}

							}						
							//get channel or IPPort 's sibling.
							pFather = pFather->GetnextSibling();		
						}
				     tempport->destAddress = "";
				     m_pJmsPortManage->m_portManager.push_back(tempport);
				     tempport = NULL;				
			     	//get all Port 's sibling.
			     	pNode = pNode->GetnextSibling();
					}//while port
				}// else compare with channellist				
			} //else compare with DOD_Queue
           pNode = pNodequeue =  pNodequeue->GetnextSibling();
		}//while
	 
	}//try
	catch(_com_error &e)
	{	
		CString msg = CString((LPCWSTR)e.Description());
		msg = " " + msg;
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverPortConfigMsg() parse port configration caught excetion(%s)"), msg);	
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverPortConfigMsg() fail to Parser Port Configuration"));
		
		return FALSE;
	}
	if(nCountNumber == 0)
	{
		(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverPortConfigMsg() queue number is zero"));
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
			"ReceiverPortConfigMsg() fail to Parser Port Configuration,Miss Node DODQueue"));
		return 1;
	}

	(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
		"ReceiverPortConfigMsg() parse PortConfiguration message successful"));	
	(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CRECEIVEJMSMSG,
		"Leave ReceiverPortConfigMsg()"));
	
	return TRUE;
}
BOOL CReceiveJmsMsg::CheckDigiteORIsalpha(BOOL Isdigiter,CString &str,
										  CString ElementIDForLog)
{
	int len = str.GetLength();
	
	if (len < 1)
	{
		(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
			"CheckDigiteORIsalpha()- str.=%s error "),
			ElementIDForLog);
		return FALSE;
	}
	
	char a;
	for(int i = 0; i < len; i++)
	{
		a = str.GetAt(i);
		if (Isdigiter)
		{
			if(isdigit(a) == false)
			{
				(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CRECEIVEJMSMSG,
					"[%d,%s]CheckDigiteORIsalpha()- isdigit(a) == false. %s error"),
					m_dwThreadID,m_strQueueName,ElementIDForLog);
				return FALSE;
			}
		}
		else
		{
			if(isalpha(a) == false)
			{
				(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CRECEIVEJMSMSG,
					"[%d,%s]CheckDigiteORIsalpha()- isalpha(a) == false. %s error"),
					m_dwThreadID,m_strQueueName,ElementIDForLog);
				return FALSE;
			}
		}
	}
	return TRUE;
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//class JmsProcThread 
//Construction/Destruction
//////////////////////////////////////////////////////////////////////

HANDLE CJmsProcThread::m_hReConnectJBoss = CreateEvent(NULL,TRUE, FALSE,NULL);
CJmsProcThread::CJmsProcThread()
{
	m_hConnectThread = NULL;
}
CJmsProcThread::~CJmsProcThread()
{
	SetEvent(CJmsProcThread::m_hReConnectJBoss);
	
   	if( m_hConnectThread )	
	{
		WaitForSingleObject( m_hConnectThread, INFINITE );
		m_hConnectThread = NULL;
	}
	CloseHandle(m_hReConnectJBoss);
}
BOOL CJmsProcThread::init()
{
	//	DWORD dwThreadID;
	//	HANDLE hThread = CreateThread(NULL, 0,
	//		(LPTHREAD_START_ROUTINE)DODReConnectJBossThread,this,0 , &dwThreadID); 
	
	m_hConnectThread = AfxBeginThread(DODReConnectJBossThread,this,THREAD_PRIORITY_NORMAL);
	
	if(m_hConnectThread)
	{ 
		(*_logger)(ZQ::common::Log::L_INFO,
			"CJmsProcThread::init():Create DODReConnectJBossThread success");
	}
	else
		return FALSE;
	
	return TRUE;
}
UINT DODReConnectJBossThread(LPVOID lpParam) 
{
	try
	{
		g_jmsPortManager->Initialize();
		
		while(!g_bStop)
		{
			WaitForSingleObject(CJmsProcThread::m_hReConnectJBoss, INFINITE);
            
			if(g_jmsPortManager->m_jms == NULL)
			{
				Sleep(2000);
				continue;
			}
			
			if(g_jmsPortManager->m_jms->m_bConnectionOK == FALSE)
			{
				(*_logger)(ZQ::common::Log::L_DEBUG,
					"ConnectionMonitor is connecting JBoss...");
				Sleep(5000);
				ResetEvent(CJmsProcThread::m_hReConnectJBoss);
				continue;
			}
			g_jmsPortManager->m_jms->m_bConnectionOK = FALSE;
            
			if(g_bStop)
				break;
			
			while(!g_jmsPortManager->ConnectionJBoss() && !g_bStop)
			{
				(*_logger)(ZQ::common::Log::L_DEBUG,
					" DODReConnectJBossThread::ReConnection JBoss error");
			} 
			
			ResetEvent(CJmsProcThread::m_hReConnectJBoss);			
			g_jmsPortManager->m_jms->m_bConnectionOK = TRUE;

			if(!g_bStop)
			{	
				CJMSBaseList::iterator it = g_jmsPortManager->m_jms->m_QueueRecvList.begin();
				for(int i = 0 ; it != g_jmsPortManager->m_jms->m_QueueRecvList.end(); it++)
				{
					SetEvent(g_jmsPortManager->m_VecParser[i].MsgReceive->m_hStartReceive);
					i++;
				}			
			}
			
			(*_logger)(ZQ::common::Log::L_DEBUG,
				" DODReConnectJBossThread::ReConnection JBoss success");
		}
		
		if(!g_jmsPortManager->m_jms)
		{
			(*_logger)(ZQ::common::Log::L_DEBUG," Exit DODReConnectJBossThread OK ");
			
			ResetEvent(CJmsProcThread::m_hReConnectJBoss);
			
			return 1;
		}
		
		if (g_jmsPortManager->m_jms)
		{
			if(g_ConnectJBossIsOK)
				g_jmsPortManager->m_jms->ConnectStop();
		}
		(*_logger)(ZQ::common::Log::L_DEBUG,
			" DODReConnectJBossThread::m_jms->stop() success!");
		
		std::vector<ZQMSGPARSER>::iterator itor;
		for( itor = g_jmsPortManager->m_VecParser.begin(); 
		itor != g_jmsPortManager->m_VecParser.end(); itor++)
		{
			CReceiveJmsMsg *MsgReceive = (*itor).MsgReceive;
			if(MsgReceive)
				delete MsgReceive;
			(*itor).MsgReceive =  NULL;
		}
		
		(*_logger)(ZQ::common::Log::L_DEBUG,
			" DODReConnectJBossThread::Delete ReceiveJmsMsg Object success");
		
		if(g_jmsPortManager->m_jms)
		{
			delete g_jmsPortManager->m_jms;
			g_jmsPortManager->m_jms = NULL;
		}
		
		(*_logger)(ZQ::common::Log::L_DEBUG,
			" DODReConnectJBossThread::Delete Jms Object success");
		
		(*_logger)(ZQ::common::Log::L_DEBUG," Exit DODReConnectJBossThread OK ");
		
		ResetEvent(CJmsProcThread::m_hReConnectJBoss);
	}
	catch (...)
	{
       	int nError = GetLastError();
		char strError[500];
		
		GetErrorDescription(nError, strError);
		
		(*_logger)(ZQ::common::Log::L_DEBUG,
			"DODReConnectJBossThread GetLastError() =%d, ErrorDescription = %s",
			nError, strError);
		ResetEvent(CJmsProcThread::m_hReConnectJBoss);
	}
	return 1;
}
