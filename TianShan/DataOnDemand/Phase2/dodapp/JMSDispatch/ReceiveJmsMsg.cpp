#include "stdafx.h"
#include "Parser.h"
#include "Markup.h"
#include "messagemacro.h"
#import "msxml3.dll"
//#import "msxml4.dll"
using namespace MSXML2;
extern  BOOL	g_bStop;

#include "receivejmsmsg.h"
const int  WAIT_TIME =180000;
extern BOOL g_ConnectJBossIsOK;
UINT DODReceiveThread(LPVOID lpParam);
UINT DODReConnectJBossThread(LPVOID lpParam);
extern CJMSPortManager *g_jmsPortManager;
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
		writeLog(ZQ::common::Log::L_INFO,
			"CReceiveJmsMsg::init() Create Receive Theread success, ThreadId (%d) queuename (%s)", 
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
		writeLog(ZQ::common::Log::L_INFO,
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
					writeLog(ZQ::common::Log::L_INFO,
						"[%d,%s]DODReceiveThread() Receive Msg TimeOut",
						m_dwThreadID,m_strQueueName.c_str());
					continue;
				}
				writeLog(ZQ::common::Log::L_INFO,
					"DODReceiveThread(),receive a msg.-------",
					m_dwThreadID,m_strQueueName.c_str());
				
				char name[MAX_PATH];
				memset(name,0,MAX_PATH);
				CJMSTxMessage *tm = (CJMSTxMessage *)(&pRecvMsg->m_jmsTextMsg);
				
				tm->getStringProperty("MESSAGECLASS",name,MAX_PATH);
				
				if(strcmpi(name,"COMMAND") == 0)
				{
					writeLog(ZQ::common::Log::L_DEBUG, 
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
					writeLog(ZQ::common::Log::L_DEBUG,
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
			
			writeLog(ZQ::common::Log::L_INFO, 
			"[%d,%s]DODReceiveThread()JBoss connect error or service Exit "
			" DCA Service g_bStop = %d",m_dwThreadID,m_strQueueName.c_str(),g_bStop);
			
			if( g_jmsPortManager->m_jms->m_bConnectionOK && !g_bStop)
			{
				writeLog(ZQ::common::Log::L_INFO, 
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
			writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]DODReceiveThread() onMessage  error. GetLastError() =%d,"
			" ErrorDescription = %s",m_dwThreadID,
			m_strQueueName.c_str(),nError, strError);
		}
	}
	
	writeLog(ZQ::common::Log::L_DEBUG,
	"[%d,%s]DODReceiveThread() Exit DODReceiveThread  OK"
	" g_bStop = %d",m_dwThreadID,m_strQueueName.c_str(),g_bStop);
	return 1;
}

CJMSTxMessage  CReceiveJmsMsg::ParseCommand(CJMSTxMessage* pMessage)
{
	if (pMessage == NULL)
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]CReceiveJmsMsg::ParseCommand() Message is null ",
			m_dwThreadID,m_strQueueName);
		return *pMessage;
	}
	CJMSTxMessage *tm = (CJMSTxMessage *)pMessage;
	
	if (tm->_message == NULL)
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]CReceiveJmsMsg::ParseCommand() fail to get message handle)",
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
	writeLog(ZQ::common::Log::L_DEBUG,
		"[%d,%s]CReceiveJmsMsg::ParseCommand() received a msg ,type=Command ",
		m_dwThreadID,m_strQueueName );
	//	writeLog(ZQ::common::Log::L_DEBUG,"%s",data);
	
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
		writeLog(ZQ::common::Log::L_DEBUG,
			"CReceiveJmsMsg::ParseStatus() Message is null ");
		
		return 0;
	}
	CJMSTxMessage *tm = (CJMSTxMessage *)pMessage;
	
	if (tm->_message == NULL)
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"CReceiveJmsMsg::ParseStatus()  fail to get message handle");
		
		return 0;
	}
	
	int datasize = tm->GetDataSize();
	
	char *name = new char[datasize];
	
	memset(name,0,datasize);
	tm->getText(name,datasize);
	
	CString data;
	data = name;
	delete name;
	writeLog(ZQ::common::Log::L_DEBUG,
		"[%d,%s]CReceiveJmsMsg::ParseStatus() Leave",m_dwThreadID,m_strQueueName);
	//	Clog( LOG_DEBUG,data);
	return 0;
}
int CReceiveJmsMsg::parseNotification(CJMSTxMessage* pMessage)
{
	if (pMessage == NULL)
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"CReceiveJmsMsg::parseNotification() Message is null");
		return 1;
	}
	CJMSTxMessage *tm = (CJMSTxMessage *)pMessage;
	
	if (tm->_message == NULL)
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"CReceiveJmsMsg::parseNotification() fail to get message handle");
		return 1;
	}
	
	int datasize = tm->GetDataSize();
	
	char *name = new char[datasize];
	
	memset(name,0,datasize);
	
	tm->getText(name,datasize);
	
	CString data;
	data = name;
	delete name;
	
	writeLog(ZQ::common::Log::L_DEBUG, 
	"[%d,%s]CReceiveJmsMsg::parseNotification() received a msg ,type = Notification ,Message "
	" content is below :",m_dwThreadID,m_strQueueName);
	
	//	printf("%s", data);
	
	CMarkup m_XmlDOM;	
	m_XmlDOM.SetDoc(data);
	
	if( m_XmlDOM.FindElem( CONTENTCONFRESPONSE ) )
	{
		writeLog(ZQ::common::Log::L_DEBUG, 
		"CReceiveJmsMsg::parseNotification() The message flag of Port configuration message is found in "
		" this message");
		
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
		
		writeLog(ZQ::common::Log::L_DEBUG, 
			"Back up PortConfig info, filepath '%s'", strCurDir.c_str());
		FILE *fp = fopen(strCurDir.c_str(),"w");
		if(fp != NULL)
		{
			fwrite((LPCTSTR)data, data.GetLength(), 1, fp);
			writeLog(ZQ::common::Log::L_DEBUG,
			"CReceiveJmsMsg::parseNotification()Create PortConfig back up file successfully,filename "
			"'DODAppPortConfigration.xml'");
			fclose(fp);
		}
		
		if(!ReceiverPortConfigMsg(data.GetBuffer(0),0,NULL))
			return 1;
		
		return 0;
	}
	
	//	printf("%s",data);
    writeLog(ZQ::common::Log::L_DEBUG, "%s",data);
	
	
	if(!m_XmlDOM.FindElem( DODMESSAGEFLAG ) )
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"CReceiveJmsMsg::parseNotification() ReceiverMsg Parser- FindElem(DODMESSAGEFLAG)  error.");
		return 1;
	}
	m_XmlDOM.IntoElem();
	
	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"CReceiveJmsMsg::parseNotification()ReceiverMsg Parser- !FindElem(DODMESSAGEHEADER).");
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
			writeLog(ZQ::common::Log::L_DEBUG,
				"RCReceiveJmsMsg::parseNotification() call ReceiverPortConfigMsg()");
			ReceiverPortConfigMsg(temp.GetBuffer(0),0,NULL);		
			break;
		case FULLDATAWITHSHAREDFOLDERS:
		case UPDATEINSHAREFOLDERMODE:
			writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]The message's  messageCode note: this message type is "
			" shared folder format ",m_dwThreadID,m_strQueueName);
			ReceiverDataFolder(temp.GetBuffer(0),0);
			break;
		case FULLDATAWITHTCP:
			writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]The message's  messageCode note: this message type is "
			 "message format ",m_dwThreadID,m_strQueueName);
			ReceiverDataTCP(temp.GetBuffer(0),0);
			break;
		default:
			writeLog(ZQ::common::Log::L_DEBUG,
				"[%d,%s]ReceiverMsg   unknowable message code identifier",
				m_dwThreadID,m_strQueueName);
			return 1;
		}
	}
	catch (...) 
	{
		int nError = GetLastError();
		char strError[500];
		
		GetErrorDescription(nError, strError);
		writeLog(ZQ::common::Log::L_DEBUG,
		"[%d,%s]Process Notification:error,GetLastError() = %d , "
		" ErrorDescription = %s",m_dwThreadID,m_strQueueName,nError, strError);
	}	
	writeLog(ZQ::common::Log::L_DEBUG,
		"[%d,%s]Parser Notification Message complete",
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
		writeLog(ZQ::common::Log::L_DEBUG,
		"[%d,%s]RemoveBlank error:(%s),GetLastError() = %d, "
		"ErrorDescription = %s",m_dwThreadID,m_strQueueName,
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
    ::DataOnDemand::DataPublisherPrx dataPrx;
	
	writeLog(ZQ::common::Log::L_DEBUG,
		"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder() Begin Receiver Data Folder",
		m_dwThreadID,m_strQueueName);
	
	str = "";
	
	m_XmlDOM.SetDoc(buf);
	
	if( !m_XmlDOM.FindElem( DODMESSAGEFLAG) )
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder() Parser- !FindElem(DODMESSAGEFLAG).",
			m_dwThreadID,m_strQueueName);
		writeLog(ZQ::common::Log::L_WARNING,
		"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder() fail to  parser",m_dwThreadID,m_strQueueName);
		return DODRECEIVERDATAFOLDERERROR;
	}
	m_XmlDOM.IntoElem();
	
	if(!m_XmlDOM.FindElem( DODMESSAGEHEADER ))
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]ReceiverDataFolder Parser- !FindElem(DODMESSAGEHEADER).",
			m_dwThreadID,m_strQueueName );
		writeLog(ZQ::common::Log::L_WARNING,
		"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder() fail to  parser",
		m_dwThreadID,m_strQueueName );
		return DODRECEIVERDATAFOLDERERROR;
	}
	int messagecode = 0;
	messagecode = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib(DODMESSAGECODE));
		  
	if( !m_XmlDOM.FindElem( DODMESSAGEBODY) )
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]ReceiverDataFolder Parser- !FindElem(DODMESSAGEBODY).",
			m_dwThreadID ,m_strQueueName);
		writeLog(ZQ::common::Log::L_WARNING,
			"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder() fail to  parser",
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
		writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]ReceiverDataFolder Parser- !FindElem(CONTENTFILEOPERATION).",
			m_dwThreadID,m_strQueueName );
		writeLog(ZQ::common::Log::L_WARNING,
			"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder() fail to  parser",
			m_dwThreadID,m_strQueueName );
		return DODRECEIVERDATAFOLDERERROR;
	}
	rootpath = m_XmlDOM.GetAttrib(CONTENTROOT);
	UpdateMode = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib(CONTENTUPDATEMODE));
	
	writeLog(ZQ::common::Log::L_DEBUG,
		"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder() DataType = %d, GroupID = %d, remotepath = %s!",
		m_dwThreadID ,m_strQueueName,datatype,GroupID,rootpath);
	try
	{	
		dataPrx = GetDataPublisherPrx();
		if(dataPrx == NULL)
		{
			writeLog(ZQ::common::Log::L_ERROR,"fail to get datapublisher porxy");
			return FALSE;
		}
		
		if (UpdateMode == 0)
		{
			writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder() UpdateMode = 0,"
			" means Full update!",m_dwThreadID,m_strQueueName);			
			dataPrx->notifyFolderFullUpdate(GroupID, datatype,
				rootpath.GetBuffer(0),true,verNumber);							
		}
		else 
			if (UpdateMode == 4)
			{
				// is the file with full path to be Modofied	
				writeLog(ZQ::common::Log::L_DEBUG,
				"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder() UpdateMode "
				" = 4, means File Modified!",m_dwThreadID,m_strQueueName);
				
				dataPrx->notifyFileModified(GroupID,datatype,
					rootpath.GetBuffer(0),rootpath.GetBuffer(0),verNumber);								
			}
			else
			{		
				while(m_XmlDOM.FindChildElem("File"))
				{
					m_XmlDOM.IntoElem();
					subPath = m_XmlDOM.GetAttrib(CONTENTPATH);
					writeLog(ZQ::common::Log::L_DEBUG,
					"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder()Update_format = (%d) :"
					" file_attrib(%s) ",m_dwThreadID,m_strQueueName,
					UpdateMode,subPath);
					switch(UpdateMode)
					{
					case 1:
						// the sub folder to be updated
						writeLog(ZQ::common::Log::L_DEBUG,
						"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder() "
						" UpdateMode = 1, means Partly update",
						m_dwThreadID,m_strQueueName);
						
						dataPrx->notifyFolderPartlyUpdate
							(GroupID,datatype,rootpath.GetBuffer(0),
							subPath.GetBuffer(0),verNumber);						
						
						break;
					case 2:
						writeLog(ZQ::common::Log::L_DEBUG,
						"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder()"
						" UpdateMode = 2, means Folder  delete",
						m_dwThreadID,m_strQueueName);
						//the sub folder to be deleted
						dataPrx->notifyFolderDeleted(GroupID,datatype,
							subPath.GetBuffer(0),verNumber);						
						break;
					case 3:
						writeLog(ZQ::common::Log::L_DEBUG,
						"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder()"
						" UpdateMode = 3, means File delete",
						m_dwThreadID,m_strQueueName);
						// the file with full path to be deleted
						dataPrx->notifyFileDeleted(GroupID,datatype,
							rootpath.GetBuffer(0),verNumber);												
						break;
					case 5:
						writeLog(ZQ::common::Log::L_DEBUG,
						"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder()"
						"UpdateMode = 5, means File new",
						m_dwThreadID,m_strQueueName);
						// the file with full path to be newed
						//if(teport->m_channel[channelIndex]->FullPathToNew(sTmp,nReserve,sTmp))			
						dataPrx->notifyFileAdded(GroupID,datatype,
							rootpath.GetBuffer(0),subPath.GetBuffer(0),verNumber);						
						break;
					case 6:
						writeLog(ZQ::common::Log::L_DEBUG,
						"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder() "
						" UpdateMode = 6, means Folder Update",
						m_dwThreadID,m_strQueueName);
						//Only update file ,Don't delete old file
						dataPrx->notifyFolderFullUpdate(GroupID, datatype,
							rootpath.GetBuffer(0),false,verNumber);												
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
		writeLog(ZQ::common::Log::L_DEBUG,
			"CReceiveJmsMsg::ReceiverDataFolder() caught TianShanIce::InvalidParameter(%s)",
			ex.message.c_str());
		return FALSE;
    }  
	catch (const ::Ice::Exception & ex) 
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"CReceiveJmsMsg::ReceiverDataFolder() caught Ice::Exception(%s)",
			ex.ice_name().c_str());
		return FALSE;
	} 
	writeLog(ZQ::common::Log::L_DEBUG,
	"[%d,%s]CReceiveJmsMsg::ReceiverDataFolder() parser Receiver Data Folder successfully",
	m_dwThreadID,m_strQueueName);
	return 0; 
}

int CReceiveJmsMsg::ReceiverDataTCP(char *buf, int buffersize)
{
	writeLog(ZQ::common::Log::L_DEBUG,
		"[%d,%s]CReceiveJmsMsg::ReceiverDataTCP() Process ReceiverData message_format",
		m_dwThreadID,m_strQueueName);
	
	CString str,strMsgContent;
	CMarkup m_XmlDOM;
	CString sDatatype;
	int GroupId;
	int datatype;
	::DataOnDemand::DataPublisherPrx dataPrx;
	
	m_XmlDOM.SetDoc(buf);
	
	if( !m_XmlDOM.FindElem(DODMESSAGEFLAG ) )
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]CReceiveJmsMsg::ReceiverDataTCP()Parser- !FindElem(DODMESSAGEFLAG).",
			m_dwThreadID,m_strQueueName );
		return 1;
	}
	m_XmlDOM.IntoElem();
	
	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]CReceiveJmsMsg::ReceiverDataTCP() Parser- !FindElem(DODMESSAGEHEADER).",
			m_dwThreadID,m_strQueueName );
		return 1;
	}
	
	int messagecode = 0;
	messagecode = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( DODMESSAGECODE ));
		  
	if (messagecode !=FULLDATAWITHTCP)
		return 1;
		  
	if(!m_XmlDOM.FindElem( DODMESSAGEBODY ))
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]CReceiveJmsMsg::ReceiverDataTCP() Parser- !FindElem(DODMESSAGEBODY).",
			m_dwThreadID,m_strQueueName);
		return 1;
	}
		  
	sDatatype = m_XmlDOM.GetAttrib( MESSAGEDATATYPE );
	datatype = atoi((LPCTSTR)sDatatype);
	GroupId = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEGROUPID ));
		  
	m_XmlDOM.IntoElem();
		  
	if( !m_XmlDOM.FindElem( DODMESSAGECONTENT))
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"[%d,%s]CReceiveJmsMsg::ReceiverDataTCP() Parser- !FindElem(DataList).",
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
				writeLog(ZQ::common::Log::L_DEBUG,
					"[%d,%s]CReceiveJmsMsg::ReceiverDataTCP() Parser- nMessageID == 0 error.",
					m_dwThreadID,m_strQueueName );
				return 1;
			}
			if (strCancel.CompareNoCase("cancel") == 0)
			{
				writeLog(ZQ::common::Log::L_INFO,
					"[%d,%s]CReceiveJmsMsg::ReceiverDataTCP() Parser- received cancel OK.",
					m_dwThreadID,m_strQueueName );
				writeLog(ZQ::common::Log::L_INFO,
					"Cannel message------StopFileSend!");	
				
				dataPrx = GetDataPublisherPrx();
				if(dataPrx == NULL)
				{
					writeLog(ZQ::common::Log::L_ERROR,
						"CReceiveJmsMsg::ReceiverDataTCP() fail to get datapublisher porxy");
					return FALSE;
				}
				dataPrx->notifyMessageDeleted(GroupId, datatype,
					sMessageID.GetBuffer(0));						
			}
			else
			{
				writeLog(ZQ::common::Log::L_DEBUG,
					"[%d,%s]CReceiveJmsMsg::ReceiverDataTCP() Parser- Unknow command (operation).",
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
			writeLog(ZQ::common::Log::L_INFO,"CReceiveJmsMsg::ReceiverDataTCP() New message------CreateNewMsg!");
			dataPrx = GetDataPublisherPrx();
			if(dataPrx == NULL)
			{
				writeLog(ZQ::common::Log::L_ERROR,"CReceiveJmsMsg::ReceiverDataTCP()fail to get datapublisher porxy");
				return FALSE;
			}
			dataPrx->notifyMessageAdded(GroupId, datatype,
				sMessageID.GetBuffer(0),filename.GetBuffer(0),
				strMsgContent.GetBuffer(0),leafTime);						
		}
	}
	catch (const ::TianShanIce::InvalidParameter & ex) 
	{
		writeLog(ZQ::common::Log::L_ERROR,
			"CReceiveJmsMsg::ReceiverDataTCP() caught TianShanIce::InvalidParameter(%s)",
			ex.message);
		return FALSE;
	}  
	catch (const ::Ice::Exception & ex) 
	{
		writeLog(ZQ::common::Log::L_ERROR,
			"CReceiveJmsMsg::ReceiverDataTCP() Ice::Exception errorcode(%s)",
			ex.ice_name().c_str());
		return FALSE;
	} 
	catch (...) 
	{
		int nError = GetLastError();
		char strError[500];
		
		GetErrorDescription(nError, strError);
		writeLog(ZQ::common::Log::L_ERROR,
		"[%d,%s]CReceiveJmsMsg::ReceiverDataTCP()fail to parser, MessageID:(%s) datatype:(%s),(%d)(%s)",
		m_dwThreadID,m_strQueueName,sMessageID,sDatatype,nError,strError);
		return 0;
	}
	
	writeLog(ZQ::common::Log::L_DEBUG,
		"[%d,%s]CReceiveJmsMsg::ReceiverDataTCP() Process ReceiverDataTCP Complete!",
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
	
	writeLog(ZQ::common::Log::L_INFO, 
		"Enter CReceiveJmsMsg::ReceiverPortConfigMsg");
	
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
		writeLog(ZQ::common::Log::L_ERROR, 
		"CReceiveJmsMsg::ReceiverPortConfigMsg() IXMLDOMDocumentPtr::CreateInstance error :Cannot Create "
        "DOMDocument object,Plesae check wheather Setup MS XML Parser runtime Libyary!");
		return FALSE;
	}
	
	//The follow codes was used the programer when pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument));
	
	//pDoc->PutvalidateOnParse(VARIANT_FALSE);
	//pDoc->PutresolveExternals(VARIANT_FALSE);
	//pDoc->PutpreserveWhiteSpace(VARIANT_TRUE);
	
	if(pDoc->loadXML((_bstr_t)CString(xmlstr))!=VARIANT_TRUE)
	{
		
		writeLog(ZQ::common::Log::L_ERROR, 
			"CReceiveJmsMsg::ReceiverPortConfigMsg() pDoc->loadXML((_bstr_t)CString(xmlstr) error");
		writeLog(ZQ::common::Log::L_ERROR, 
			"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
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
			writeLog(ZQ::common::Log::L_ERROR, 
				"CReceiveJmsMsg::ReceiverPortConfigMsg()pDoc->selectSingleNode(CONTENTCONFRESPONSE erro");
			writeLog(ZQ::common::Log::L_ERROR, 
				"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
			return FALSE;
		}
		
		pNode = childNode->GetfirstChild();
		if (pNode == NULL)
		{
			writeLog(ZQ::common::Log::L_ERROR,
			"CReceiveJmsMsg::ReceiverPortConfigMsg()"
			"childNode->GetfirstChild() error");
			writeLog(ZQ::common::Log::L_ERROR, 
				"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
			return FALSE;
		}
		name = CString((wchar_t*)pNode->GetnodeName());
		if(name.Compare("ListOfNode")!=0)
		{
			writeLog(ZQ::common::Log::L_ERROR, 
				"CReceiveJmsMsg::ReceiverPortConfigMsg() Node of "
			"ListOfNode is not found");
			writeLog(ZQ::common::Log::L_ERROR, 
				"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
			return FALSE;
		}
		
		pNode = pNode->GetfirstChild();
		name = CString((wchar_t*)pNode->GetnodeName());
		if(name.Compare("DCAPortConfiguration")!=0)
		{
			writeLog(ZQ::common::Log::L_ERROR, 
			"CReceiveJmsMsg::ReceiverPortConfigMsg() Node of  "
			"DCAPortConfiguration not found");
			writeLog(ZQ::common::Log::L_ERROR, 
				"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
			return FALSE;
		}
		
		pNamedNode = pNode->Getattributes();
		if(pNamedNode == NULL)
		{	
			writeLog(ZQ::common::Log::L_ERROR,
			"CReceiveJmsMsg::ReceiverPortConfigMsg Getattributes "
			"for DCAPortConfiguration error");
			writeLog(ZQ::common::Log::L_ERROR, 
				"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
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
			writeLog(ZQ::common::Log::L_ERROR, 
			"CReceiveJmsMsg::ReceiverPortConfigMsg() Getattributes for "
			"DataTypeWithInitial error");
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
					writeLog(ZQ::common::Log::L_ERROR,
						"CReceiveJmsMsg::ReceiverPortConfigMsg() queue number is zero");
					writeLog(ZQ::common::Log::L_ERROR, 
						"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
					return FALSE;
				}
				writeLog(ZQ::common::Log::L_DEBUG, 
					"CReceiveJmsMsg::ReceiverPortConfigMsg() queue number is %d ",nchildNumber);
				
				nCountNumber = nchildNumber;
				
				pSon = pNodequeue->GetfirstChild();
				int k = 0;
				while (pSon)
				{
					name = CString((wchar_t*)pSon->GetnodeName());
					pNamedNode = pSon->Getattributes();
					if(pNamedNode == NULL)
					{	
						writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
						return FALSE;
					}	
					
					pTmp = pNamedNode->getNamedItem((_bstr_t)"Name");
					if(pTmp == NULL)
					{	
						writeLog(ZQ::common::Log::L_ERROR, 
						"CReceiveJmsMsg::ReceiverPortConfigMsg() Get attributes"
						" for queuename %s", name);
						writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
						return FALSE;
					}	
					nodeValue = pTmp->GetnodeValue();
					CString temp = nodeValue.bstrVal;
					if(temp.GetLength()<1)
					{	
						writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg () IPAddress error");
						writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
						return FALSE;
					}
					
					writeLog(ZQ::common::Log::L_DEBUG, 
						"CReceiveJmsMsg::ReceiverPortConfigMsg() Parser Msg Queue Name='%s'",temp);
					
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
						writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() Channel number is zero");

							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
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
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg()Getattributes error");

							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
									
						CHANNELINFO *tempchannel = new CHANNELINFO;
						name.MakeLower();
						tempchannel->ChannelName = name;
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)"Encrypted");
						if(pTmp == NULL)
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg()fail to get attributes"
							" for Encrypted,channelname=%s",name);

							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() Encrypted error");

							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						//writeLog(ZQ::common::Log::L_DEBUG, "CReceiveJmsMsg::ReceiverPortConfigMsg () Encrypted error 1abc %s",SCheckStr.GetBuffer(0) );
						
						nodeValue.ChangeType(VT_INT);
						//writeLog(ZQ::common::Log::L_DEBUG, "CReceiveJmsMsg::ReceiverPortConfigMsg () Encrypted error 2abc");
						tempchannel->nEncrypted = (::DataOnDemand::EncryptMode)nodeValue.intVal;									
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)
							"StreamType");
						if(pTmp == NULL)
						{	
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to get attributes"
							"for StreamType, portname='%s'",name);

							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() StreamType error");

							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue.ChangeType(VT_INT);
						tempchannel->streamType = nodeValue.intVal;
		
						pTmp = pNamedNode->getNamedItem((_bstr_t)"StreamID");
						if(pTmp == NULL)
						{
							writeLog(ZQ::common::Log::L_ERROR,
								"CReceiveJmsMsg::ReceiverPortConfigMsg() "
							"fail to get attributes for StreamID, portname='%s'",name);

							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg()fail to get StreamID");

							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue.ChangeType(VT_INT);
						tempchannel->nstreamId = nodeValue.intVal;
						
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)"Tag");
						if(pTmp == NULL)
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() "
							"fail to get attributes for Tag, portname=%s",name);
							
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(FALSE,SCheckStr))
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to get Tag");
							writeLog(ZQ::common::Log::L_ERROR, 
								"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						//	strcpy(tempchannel->Tag,((wchar_t*)nodeValue.bstrVal));
						
						tempchannel->Tag = (CString((wchar_t*)
							nodeValue.bstrVal)).GetBuffer(0);
						pTmp = pNamedNode->getNamedItem((_bstr_t)"QueueName");
						if(pTmp == NULL)
						{	
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to get attributes for QueueName, portname=%s",name);

							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(SCheckStr.GetLength() <1)
						{	
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() QueueName error");

							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}
						tempchannel->strQueueName =
							CString((wchar_t*)nodeValue.bstrVal);

						pTmp = pNamedNode->getNamedItem(
							(_bstr_t)(MESSAGECHANNELTYPE));
						if(pTmp == NULL)
						{	
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() "
							"failt to get attributes for ChannelType,channel=%s ",name);
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() MESSAGECHANNELTYPE error");

							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}
						nodeValue.ChangeType(VT_INT);
						tempchannel->nChannelType = nodeValue.intVal;
						
						pTmp = pNamedNode->getNamedItem(
							(_bstr_t)"SendIndexTable");
						if(pTmp == NULL)
						{	
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg()"
							"fail to get attributes for SendIndexTable,portname=%s",name);
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() SendIndexTable error");
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}
						nodeValue.ChangeType(VT_INT);
						tempchannel->nSendWithDestination = 
							nodeValue.intVal;
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)
							"StreamCount");
						if(pTmp == NULL)
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() "
							"fail to get attributes for StreamCount,portname=%s",name);
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to get StreamCount");
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}
						nodeValue.ChangeType(VT_INT);
						tempchannel->nStreamCount = nodeValue.intVal;
						
						if (tempchannel->nChannelType)
						{
							if (tempchannel->nStreamCount == 0)
							{
								writeLog(ZQ::common::Log::L_ERROR,
								"CReceiveJmsMsg::ReceiverPortConfigMsg()"
								" Multiplestream=1, but StreamCount=0");
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
							}
						}
						else
							tempchannel->nStreamCount = 0;
						pTmp = pNamedNode->getNamedItem((_bstr_t)(MESSAGEDATATYPE));
						if(pTmp == NULL)
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() "
							"fail to get attributes for MessageDataType,portname=%s",name);
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
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
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg()" 
						    "fail to getattributes for m_sDataType,portname=%s",name);
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}
						
						pTmp = pNamedNode->getNamedItem(
							(_bstr_t)"DataExchangeType");
						if(pTmp == NULL)
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() "
							"fail to get attributes for DataExchangeType,portname=%s",name);
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() "
							" DataExchangeType error");
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}
						nodeValue.ChangeType(VT_INT);
						
						if(nodeValue.intVal == 0)
						{
							tempchannel->nDataExchangeType = 
								DataOnDemand::dodSharedFolder;
						}
						else 
							if(nodeValue.intVal == 1)
							{
								tempchannel->nDataExchangeType = 
									DataOnDemand::dodMessage ;
							}
							else
								if(nodeValue.intVal == 2)
								{
									tempchannel->nDataExchangeType =
										DataOnDemand::dodLocalFolder ;
								}
								else
								{
									writeLog(ZQ::common::Log::L_ERROR,
									"CReceiveJmsMsg::ReceiverPortConfigMsg() "
									"Getattributes for DataExchangeType,"
								    "portname=%s error,DataExchangeType = %d",
									name,nodeValue.intVal);
									writeLog(ZQ::common::Log::L_ERROR,
										"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
									return FALSE;
								}
								
								// create ilp file for navigation ....
								pTmp = pNamedNode->getNamedItem((_bstr_t)
									"SendMsgDataType");
								if(pTmp == NULL)
								{	
									writeLog(ZQ::common::Log::L_ERROR, 
									"CReceiveJmsMsg::ReceiverPortConfigMsg()"
									"fail to get attributes for SendMsgDataType,portname=%s",name);
						        	writeLog(ZQ::common::Log::L_ERROR,
									"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
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
									writeLog(ZQ::common::Log::L_ERROR, 
									"CReceiveJmsMsg::ReceiverPortConfigMsg() "
									"fail to get attributes for SendMsgExpiredTime,portname=%s",name);
									writeLog(ZQ::common::Log::L_ERROR,
									"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
									return FALSE;
								}	
								nodeValue = pTmp->GetnodeValue();
								SCheckStr = nodeValue.bstrVal;
								if (SCheckStr.GetLength() >0)
								{
									if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
									{	
										writeLog(ZQ::common::Log::L_ERROR, 
										"CReceiveJmsMsg::ReceiverPortConfigMsg()fail to get SendMsgExpiredTime");
										writeLog(ZQ::common::Log::L_ERROR,
											"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
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
									writeLog(ZQ::common::Log::L_ERROR, 
										"CReceiveJmsMsg::ReceiverPortConfigMsg()"
									" fail to get attributes for MessageCode,portname=%s",name);
									writeLog(ZQ::common::Log::L_ERROR,
										"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
									return FALSE;
								}	
								nodeValue = pTmp->GetnodeValue();
								SCheckStr = nodeValue.bstrVal;
								if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
								{	
									tempchannel->nMessageCode = 0;
									writeLog(ZQ::common::Log::L_ERROR, 
									"CReceiveJmsMsg::ReceiverPortConfigMsg() MessageCode is null");
								}
								else
								{
									nodeValue.ChangeType(VT_INT);
									tempchannel->nMessageCode = nodeValue.intVal;
								}
								//   			tempchannel->m_channelID = k+1;
								writeLog(ZQ::common::Log::L_DEBUG, 
								"CReceiveJmsMsg::ReceiverPortConfigMsg() channelname=%s,"
								"DataExchangeType=%ld, QueueName=%s ",
								tempchannel->ChannelName.c_str(),
								tempchannel->nDataExchangeType,
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
						writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() Node of Port is not found");
						writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
						return FALSE;
					}
					
					// prevent current node is not child or namenode		
					pNamedNode = pNode->Getattributes();
					if(pNamedNode == NULL)
					{	
						writeLog(ZQ::common::Log::L_ERROR, 
						"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to get attributes for"
						"Port error");
						writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
						return FALSE;
					}	
					childlist = pNode->GetchildNodes();
					hr = childlist->get_length(&nchildNumber);
					if (nchildNumber < 1)
					{
						writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() Port number is zero");
						writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
						return FALSE;
					}
					
					writeLog(ZQ::common::Log::L_INFO,
						"CReceiveJmsMsg::ReceiverPortConfigMsg() Port number is  %d ",nchildNumber);
					pNode = pNode->GetfirstChild();
					//get all config ports.
					while (pNode)
					{
						DODPORT *tempport = new DODPORT;			//get port layer attrib
						strPorttemp = CString((wchar_t*)pNode->GetnodeName());
					/*	tempport->portname = "Port_" + 
							CString((wchar_t*)pNode->GetnodeName());*/

						tempport->portname = CString((wchar_t*)pNode->GetnodeName());
						
						writeLog(ZQ::common::Log::L_INFO,
							"CReceiveJmsMsg::ReceiverPortConfigMsg() get attributes"
						" for portname=%s",tempport->portname.c_str());
						
						pNamedNode = pNode->Getattributes();
						
						if(pNamedNode == NULL)
						{	
							writeLog(ZQ::common::Log::L_ERROR,
								"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to get attributes for portname=%s",
							tempport->portname.c_str());
							writeLog(ZQ::common::Log::L_ERROR, 
								"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)"PMTPID");
						if(pTmp == NULL)
						{	
							
							writeLog(ZQ::common::Log::L_ERROR, 
								"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to get attributes "
							"for PMTPID, portname=%s",tempport->portname.c_str());
							writeLog(ZQ::common::Log::L_ERROR, 
								"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}
						//writeLog(ZQ::common::Log::L_DEBUG, "CReceiveJmsMsg::ReceiverPortConfigMsg : portname = %s ,PMTPID = %s",tempport->portname,pTmp);
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr,
							"getNamedItem((_bstr_t)Encrypted)"))
						{	
							writeLog(ZQ::common::Log::L_ERROR,
								"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to get PMTPID");
							writeLog(ZQ::common::Log::L_ERROR,
								"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}		
						//writeLog(ZQ::common::Log::L_DEBUG, "CReceiveJmsMsg::ReceiverPortConfigMsg () PMTPID error 1 %s",SCheckStr.GetBuffer(0));
						
						nodeValue.ChangeType(VT_INT);
						tempport->pmtPid = nodeValue.intVal;
						//writeLog(ZQ::common::Log::L_DEBUG, "CReceiveJmsMsg::ReceiverPortConfigMsg () PMTPID error 2");
						
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)(MESSAGEGROUPID));
						if(pTmp==NULL)
						{	
							writeLog(ZQ::common::Log::L_ERROR,
							"CReceiveJmsMsg::ReceiverPortConfigMsg()fail to get attributes "
							"for GroupID, portname=%s ",tempport->portname.c_str());
							writeLog(ZQ::common::Log::L_ERROR, 
								"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr=nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to get MESSAGEGROUPID");
							writeLog(ZQ::common::Log::L_ERROR, 
								"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue.ChangeType(VT_INT);
						tempport->groupId = nodeValue.intVal;
						
						pTmp = pNamedNode->getNamedItem((_bstr_t)"TotalRate");
						if(pTmp == NULL)
						{	
							writeLog(ZQ::common::Log::L_ERROR, 
								"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to get attributes for TotalRate, portname=%s",
							tempport->portname.c_str());
							writeLog(ZQ::common::Log::L_ERROR, 
								"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
							return FALSE;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr = nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							
							writeLog(ZQ::common::Log::L_ERROR, 
							"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to get TotalRate");
							writeLog(ZQ::common::Log::L_ERROR,
								"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
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
									writeLog(ZQ::common::Log::L_ERROR, 
										"CReceiveJmsMsg::ReceiverPortConfigMsg()  IPPort number is zero");
									writeLog(ZQ::common::Log::L_ERROR, 
										"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
									return FALSE;
								}
								
								writeLog(ZQ::common::Log::L_DEBUG, 
									"CReceiveJmsMsg::ReceiverPortConfigMsg() PortName='%s', IPPort number is %d ",
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
										writeLog(ZQ::common::Log::L_ERROR, 
											"CReceiveJmsMsg::ReceiverPortConfigMsg() "
										"fail to get attributes for TotalRate, portname=%s ",name);
										writeLog(ZQ::common::Log::L_ERROR,
											"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
										return FALSE;
									}	
									
									pTmp = pNamedNode->getNamedItem((_bstr_t)"SendType");
									if(pTmp == NULL)
									{	
										writeLog(ZQ::common::Log::L_ERROR, 
										"CReceiveJmsMsg::ReceiverPortConfigMsg "
										"fail to get attributes for SendType, portname=%s",name);
										writeLog(ZQ::common::Log::L_ERROR,
											"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
										return FALSE;
									}	
									nodeValue = pTmp->GetnodeValue();
									SCheckStr = nodeValue.bstrVal;
									if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
									{
										writeLog(ZQ::common::Log::L_ERROR,
										"CReceiveJmsMsg::ReceiverPortConfigMsg() fail toget SendType");
										writeLog(ZQ::common::Log::L_ERROR,
											"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
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
										writeLog(ZQ::common::Log::L_ERROR, 
										"CReceiveJmsMsg::ReceiverPortConfigMsg() "
										"fail to get attributes for IPAddress, portname=%s",name);
										writeLog(ZQ::common::Log::L_ERROR,
											"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
										return FALSE;
									}	
									nodeValue = pTmp->GetnodeValue();
									SCheckStr = nodeValue.bstrVal;
									if(SCheckStr.GetLength()<1)
									{	
										writeLog(ZQ::common::Log::L_ERROR, 
										"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to get IPAddress");
										writeLog(ZQ::common::Log::L_ERROR,
											"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
										return FALSE;
									}	
									
									pIpPort->strIp = (LPCSTR) (CString(
										(wchar_t*)nodeValue.bstrVal)).GetBuffer(0); 
									pTmp = pNamedNode->getNamedItem((_bstr_t)"IPPort");
									if(pTmp == NULL)
									{	
										writeLog(ZQ::common::Log::L_ERROR,
										"CReceiveJmsMsg::ReceiverPortConfigMsg() "
										"fail to get attributes for IPPort, portname=%s",name);
										writeLog(ZQ::common::Log::L_ERROR,
											"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
										return FALSE;
									}	
									nodeValue = pTmp->GetnodeValue();
									SCheckStr = nodeValue.bstrVal;
									if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
									{	
										writeLog(ZQ::common::Log::L_ERROR, 
										"CReceiveJmsMsg::ReceiverPortConfigMsg()fail to get SendType");
										writeLog(ZQ::common::Log::L_ERROR,
											"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");;
										return FALSE;
									}	
									nodeValue.ChangeType(VT_INT);
									pIpPort->nPort = nodeValue.intVal;
									pIpPort->IpPortName = name;
									writeLog(ZQ::common::Log::L_INFO, 
									"CReceiveJmsMsg::ReceiverPortConfigMsg() IPPort: name=%s,"
									"SendType=%d, IPAddress=%s ,IPPort=%d",
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
									writeLog(ZQ::common::Log::L_ERROR,
										"CReceiveJmsMsg::ReceiverPortConfigMsg() Channel count is 0");
									writeLog(ZQ::common::Log::L_ERROR,
										"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
									return FALSE;
								}
								
								writeLog(ZQ::common::Log::L_INFO, 
								"CReceiveJmsMsg::ReceiverPortConfigMsg() PortName=%s, Channel count (%d)", 
								tempport->portname.c_str(),nchildNumber);
								
								pSon = pFather->GetfirstChild();
								int k = 0;
								while (pSon)
								{
									name = CString((wchar_t*)pSon->GetnodeName());
									pNamedNode = pSon->Getattributes();
									if(pNamedNode == NULL)
									{	
										writeLog(ZQ::common::Log::L_ERROR, 
											"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to get attributes,portname=%s",name);
										writeLog(ZQ::common::Log::L_ERROR,
										"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
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
										writeLog(ZQ::common::Log::L_ERROR, 
											"CReceiveJmsMsg::ReceiverPortConfigMsg() "
										     "fail to get attributes for RepeatTime,portname=%s",name);
										return FALSE;
									}	
									nodeValue = pTmp->GetnodeValue();
									SCheckStr = nodeValue.bstrVal;
									if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
									{	
										writeLog(ZQ::common::Log::L_ERROR, 
										"CReceiveJmsMsg::ReceiverPortConfigMsg() RepeatTime error");
										writeLog(ZQ::common::Log::L_ERROR,
											"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
										return FALSE;
									}	
									nodeValue.ChangeType(VT_INT);
									tempchannelattach->nRepeatetime = 
										nodeValue.intVal;
									
									pTmp = pNamedNode->getNamedItem(
										(_bstr_t)"ChannelRate");
									if(pTmp == NULL)
									{	
										writeLog(ZQ::common::Log::L_ERROR, 
										"CReceiveJmsMsg::ReceiverPortConfigMsg()"
										"fail to get attributes for ChannelRate, portname=%s",name);
										writeLog(ZQ::common::Log::L_ERROR,
											"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");;
										return FALSE;
									}	
									nodeValue = pTmp->GetnodeValue();
									SCheckStr = nodeValue.bstrVal;
									if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
									{	
										writeLog(ZQ::common::Log::L_ERROR, 
										"CReceiveJmsMsg::ReceiverPortConfigMsg()fail to get ChannelRate");
										writeLog(ZQ::common::Log::L_ERROR,
											"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
										return FALSE;
									}
									nodeValue.ChangeType(VT_INT);
									tempchannelattach->nChannelRate = 
										nodeValue.intVal;
									
									writeLog(ZQ::common::Log::L_DEBUG, 
									"CReceiveJmsMsg::ReceiverPortConfigMsg(): channelname = %s,"
									"RepeatTime = %ld, ChannelRate = %d ",
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
		writeLog(ZQ::common::Log::L_ERROR, 
			"CReceiveJmsMsg::ReceiverPortConfigMsg() parse port configration caught excetion(%s)", msg);	
		writeLog(ZQ::common::Log::L_ERROR, 
			"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration");
		
		return FALSE;
	}
	if(nCountNumber == 0)
	{
		writeLog(ZQ::common::Log::L_ERROR,
			"CReceiveJmsMsg::ReceiverPortConfigMsg() queue number is zero");
		writeLog(ZQ::common::Log::L_ERROR, 
			"CReceiveJmsMsg::ReceiverPortConfigMsg() fail to Parser Port Configuration,Miss Node DODQueue");
		return 1;
	}

	writeLog(ZQ::common::Log::L_DEBUG, 
		"CReceiveJmsMsg::ReceiverPortConfigMsg() parse PortConfiguration message successful");	
	writeLog(ZQ::common::Log::L_DEBUG, 
		"Leave CReceiveJmsMsg::ReceiverPortConfigMsg()");
	
	return TRUE;
}
BOOL CReceiveJmsMsg::CheckDigiteORIsalpha(BOOL Isdigiter,CString &str,
										  CString ElementIDForLog)
{
	int len = str.GetLength();
	
	if (len < 1)
	{
		writeLog(ZQ::common::Log::L_ERROR,
			"CReceiveJmsMsg::CheckDigiteORIsalpha()- str.=%s error ",
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
				writeLog(ZQ::common::Log::L_ERROR,
					"[%d,%s]CReceiveJmsMsg::CheckDigiteORIsalpha()- isdigit(a) == false. %s error ",
					m_dwThreadID,m_strQueueName,ElementIDForLog);
				return FALSE;
			}
		}
		else
		{
			if(isalpha(a) == false)
			{
				writeLog(ZQ::common::Log::L_ERROR, 
					"[%d,%s]CReceiveJmsMsg::CheckDigiteORIsalpha()- isalpha(a) == false. %s error ",
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
		writeLog(ZQ::common::Log::L_INFO,
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
				writeLog(ZQ::common::Log::L_DEBUG,
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
				writeLog(ZQ::common::Log::L_DEBUG,
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
			
			writeLog(ZQ::common::Log::L_DEBUG,
				" DODReConnectJBossThread::ReConnection JBoss success");
		}
		
		if(!g_jmsPortManager->m_jms)
		{
			writeLog(ZQ::common::Log::L_DEBUG," Exit DODReConnectJBossThread OK ");
			
			ResetEvent(CJmsProcThread::m_hReConnectJBoss);
			
			return 1;
		}
		
		if (g_jmsPortManager->m_jms)
		{
			if(g_ConnectJBossIsOK)
				g_jmsPortManager->m_jms->ConnectStop();
		}
		writeLog(ZQ::common::Log::L_DEBUG,
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
		
		writeLog(ZQ::common::Log::L_DEBUG,
			" DODReConnectJBossThread::Delete ReceiveJmsMsg Object success");
		
		if(g_jmsPortManager->m_jms)
		{
			delete g_jmsPortManager->m_jms;
			g_jmsPortManager->m_jms = NULL;
		}
		
		writeLog(ZQ::common::Log::L_DEBUG,
			" DODReConnectJBossThread::Delete Jms Object success");
		
		writeLog(ZQ::common::Log::L_DEBUG," Exit DODReConnectJBossThread OK ");
		
		ResetEvent(CJmsProcThread::m_hReConnectJBoss);
	}
	catch (...)
	{
       	int nError = GetLastError();
		char strError[500];
		
		GetErrorDescription(nError, strError);
		
		writeLog(ZQ::common::Log::L_DEBUG,
			"DODReConnectJBossThread GetLastError() =%d, ErrorDescription = %s",
			nError, strError);
		ResetEvent(CJmsProcThread::m_hReConnectJBoss);
	}
	return 1;
}
