// Parser.cpp: implementation of the CJMSParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Parser.h"
#include "DODClientAgent.h"
#include "clog.h"
#include "Markup.h"
#include "messagemacro.h"
#include "createilp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#import "msxml3.dll"
//#import "msxml4.dll"
using namespace MSXML2;

//extern BOOL g_bNotInternalTest;

typedef CDODChannel* CDODCHANNELARRAY;
typedef ZQSBFIPPORTINF* ZQSBFIPPORTINFARRAY;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJMSParser::CJMSParser(CJMS *inJms,CDODClientAgent *client)
{
	m_jms=inJms;
	m_Agent=client;
	m_bStartReceive=FALSE;
	m_bIsLocalConfig=FALSE;
	return; 
}
CJMSParser::~CJMSParser(void)
{

}


void CJMSParser::onMessage(Message *ms)
{
	Clog( LOG_DEBUG,_T(" CJMSParser::onMessage ,receive a msg.-------"));
	
	try
	{
		if (ms==NULL)
		{
			Clog( LOG_DEBUG,_T(" CJMSParser::onMessage is null error"));
			return ;
		}
		char name[MAX_PATH];
		memset(name,0,MAX_PATH);
		CJMSTxMessage *tm = (CJMSTxMessage *)ms;

		if (tm->_message==NULL)
		{
			Clog( LOG_DEBUG,_T("CJMSParser::onMessage(Message *ms)tm->_message==NULL  error"));
			return ;
		}
		//int stringSize=0;
		tm->getStringProperty("MESSAGECLASS",name,MAX_PATH);

		if (m_bStartReceive==FALSE)
		{
			int datasize=tm->GetDataSize();

			char *name = new char[datasize];

			tm->getText(name,datasize);

			CString data;
			data=name;
			delete name;

			Clog( LOG_DEBUG,_T(" CJMSParser::onMessage ,clear a out of time msg.content is below:"));
			Clog( LOG_DETAIL,data);
			return ;
		}

		if(strcmpi(name,"COMMAND")==0)
		{
			Clog( LOG_DEBUG,_T(" CJMSParser::onMessage ParseCommand"));
		}
		else if(strcmpi(name,"STATUS")==0)
		{
			ParseStatus(tm);  
		}
		else if(strcmpi(name,"NOTIFICATION")==0)
		{
			parseNotification(tm);
		}
		else
		{
			Clog( LOG_DEBUG,_T(" tm->getStringProperty MESSAGECLASS,name,MAX_PATH is other error"));
			return ;
		}
	}
	catch (...)
	{
		Clog( LOG_DEBUG,_T("JMSParser::onMessage  error errorcode =%d"),GetLastError());
	}
}

CJMSTxMessage  CJMSParser::ParseCommand(CJMSTxMessage* pMessage)
{
	if (pMessage==NULL)
	{
		Clog( LOG_DEBUG,_T(" CJMSParser::onMessage is null ParseCommand"));
		return *pMessage;
	}
	CJMSTxMessage *tm = (CJMSTxMessage *)pMessage;

	if (tm->_message==NULL)
	{
		Clog( LOG_DEBUG,_T(" CJMSTxMessage  CJMSParser::ParseCommand( handle error"));
		return *pMessage;
	}
	int datasize=tm->GetDataSize();

	char *name = new char[datasize];

	tm->getText(name,datasize);

	CString data;
	data=name;

	delete name;

	Clog( LOG_DEBUG,_T("CJMSParser:: received a msg ,type=Command !") );
	Clog( LOG_DEBUG,data);
// these codes will used to reply JMSCommand .but now the DCA project was not used it;

	CJMSTxMessage destmessage;
	CString replystring="reply ";
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
	
	int nMessageID=0;
	m_XmlDOM.AddAttrib( MESSAGEMESSAGEID, nMessageID);

	replystring=m_XmlDOM.GetDoc();
//	destmessage.se(replystring.GetBuffer(0));
	return destmessage;

}
int CJMSParser::ParseStatus(CJMSTxMessage* pMessage)
{
	if (pMessage==NULL)
	{
		printf("\n CJMSParser::onMessage \n");
		return 0;
	}
	CJMSTxMessage *tm = (CJMSTxMessage *)pMessage;

	if (tm->_message==NULL)
	{
		printf("\n CJMSParser::ParseStatus  get message handle error\n");
		return 0;
	}

	int datasize=tm->GetDataSize();

	char *name = new char[datasize];

	tm->getText(name,datasize);

	CString data;
	data=name;
	delete name;
	Clog( LOG_DEBUG,_T("CJMSMessage ParseStatus") );
	Clog( LOG_DEBUG,data);
	return 0;
}
int CJMSParser::parseNotification(CJMSTxMessage* pMessage)
{
	if (pMessage==NULL)
	{
		Clog( LOG_DEBUG,_T(" CJMSParser::onMessage is null error"));
		return 1;
	}
	CJMSTxMessage *tm = (CJMSTxMessage *)pMessage;

	if (tm->_message==NULL)
	{
		Clog( LOG_DEBUG,_T(" CJMSParser::onMessage  get message handle error"));
		return 1;
	}

	int datasize=tm->GetDataSize();

	char *name = new char[datasize];

	tm->getText(name,datasize);

	CString data;
	data=name;
	delete name;

	Clog( LOG_DEBUG,_T("CJMSParser:: received a msg ,type=Notification ,Message content is below :") );
	Clog( LOG_DEBUG,data);
	CMarkup m_XmlDOM;
	
	m_XmlDOM.SetDoc(data);
/*
	if (m_bIsLocalConfig)
	{	
		m_XmlDOM.SetDoc(data);
	}
	else
	{
		m_bIsLocalConfig=TRUE;
		CString str;
		str=m_pm->m_path+"\\"+"config.xml";
		m_XmlDOM.Load(str);
		data=m_XmlDOM.GetDoc(); 
	}
	*/

	if( m_XmlDOM.FindElem( CONTENTCONFRESPONSE ) )
	{
		Clog( LOG_DEBUG,_T("The message flag of configuration message is found in this message !") );		

		if (m_pm)
		{
			m_pm->Stop();
			Sleep(1);
			m_pm->ClosePort();
			Sleep(1);

			m_pm->m_PortVector.clear();
		}

		ReceiverPortConfigMsg(data.GetBuffer(0),0,NULL);	

		m_Agent->AddChannelQueue();
		return 1;
	}

	if(!m_XmlDOM.FindElem( DODMESSAGEFLAG ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverMsg Parser- FindElem(DODMESSAGEFLAG).") );
		return 1;
	}
	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverMsg Parser- !FindElem(DODMESSAGEHEADER).") );
		return 1;
	}
	try
	{
		int messagecode=0;
		messagecode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( DODMESSAGECODE ));

		CString temp=m_XmlDOM.GetDoc();
		int nGroupIDIsZero=0;
		switch(messagecode) 
		{	
		case PORTCONFIGURATION:

			Clog( LOG_DEBUG,_T("ReceiverMsg  : call ReceiverPortConfigMsg() ") );
			ReceiverPortConfigMsg(temp.GetBuffer(0),0,NULL);		
			break;
		case FULLDATAWITHSHAREDFOLDERS:
		case UPDATEINSHAREFOLDERMODE:
			Clog( LOG_DEBUG,_T("The message's  messageCode note: this message type is shared folder format ") );
			ReceiverDataFolder(temp.GetBuffer(0),0);
			break;
		case FULLDATAWITHTCP:

			Clog( LOG_DEBUG,_T("The message's  messageCode note: this message type is message format ") );
			ReceiverDataTCP(temp.GetBuffer(0),0,0,nGroupIDIsZero);
			if (nGroupIDIsZero==1)
			{
				//here,process group is not zero.
				ReceiverDataTCPToAllChannel(temp.GetBuffer(0));
			}else
				ReceiverDataTCP(temp.GetBuffer(0),0,1,nGroupIDIsZero);
			break;
		default:		
			Clog( LOG_DEBUG,_T("ReceiverMsg   unknowable message code identifier") );
			return 1;
		}
	}
	catch (...) 
	{
		Clog( LOG_DEBUG,_T("Process Notification:error,lasterror=%d "),GetLastError());
	}

	return 0;
}

CString CJMSParser::GetCurrDateTime()
{
	SYSTEMTIME time; 
	GetLocalTime(&time);
	CString sTime;
	sTime.Format("%04d%02d%02d %02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	return sTime;
}

CString CJMSParser::GetDataTypeInitialMsg(CString sDataType,int nReserver)
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

	str=m_XmlDOM.GetDoc();
	return str;}

CString CJMSParser::SendGetFullDateMsg(int id1,CString id2,int id3)
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

	str=m_XmlDOM.GetDoc();
	return str;
}


CString CJMSParser::SendGetConfig()
{
	//Clog( LOG_DEBUG,_T("CJMSParser::SendGetConfig") );

	CString str="";	

	CMarkup m_XmlDOM;	
	m_XmlDOM.AddElem( MESSAGECONFREQUEST );	
	m_XmlDOM.AddAttrib( MESSAGECONFREQUESTKEY, 0); 
	m_XmlDOM.AddAttrib( MESSAGEGROUPIDAPPNAME, CONTENTMESSAGEDOD); 
	m_XmlDOM.AddAttrib( MESSAGETYPE, 3); 
	m_XmlDOM.AddAttrib(MESSAGEFORMAT, 2); 
	str=m_XmlDOM.GetDoc();
	return str;
}
CString CJMSParser::RemoveBlank(CString oldStr)
{
	try
	{
		if (oldStr.GetLength()==0)
			return "";

		oldStr=oldStr.TrimLeft();
		oldStr=oldStr.TrimRight();

		if (oldStr.GetLength()==0)
			return "";

		int nIndex=0;

		while (nIndex=oldStr.Find(" ;") >=0)
		{
			oldStr.Delete(nIndex);
			if (oldStr.GetLength()==0)
				return "";
		}
		if (oldStr.GetLength()==0)
			return "";

		while (nIndex=oldStr.Find("; ") >=0)
		{
			oldStr.Delete(nIndex+1);
			if (oldStr.GetLength()==0)
				return "";
		}
		if (oldStr.GetLength()==0)
			return "";

	}
	catch (...) 
	{
		Clog( LOG_DEBUG,_T("RemoveBlank error:(%s),lasterror=%d "),oldStr,GetLastError());
		return "";
	}
	return oldStr;
}

BOOL CJMSParser::FindOutPortIndexAndChannelIndex(CString dataType,int GroupID,CDODPort **portaddress,int &ChannelIndex)
{
	if (m_pm==NULL)
	{
		return FALSE;
	}

	if(1>(int)(m_pm->m_PortVector.size()))
	{
		Clog( LOG_DEBUG,_T("CJMSParser::FindOutPortIndexAndChannelIndex PortVector is zero error.") );
		return FALSE;
	}

	try
	{
		CPORTVECTOR::iterator iter;

		for(iter=m_pm->m_PortVector.begin();iter!=m_pm->m_PortVector.end();iter++)
		{
			CDODPort *pPort=(*iter);
			if(pPort==NULL)
			{
				return FALSE;
			}
			//By jerroy request,if groupID is zero,the channel dateType verily is exclusive, So mark compare about GroupID
			//By andrew command, remove this judgement.
			//if (GroupID>0)
			{
				if (GroupID != pPort->m_nGroupID)
					continue;
			}

			for(int i=0;i<pPort->m_ChannelNumber;i++)
			{
				CDODChannel *channel=pPort->m_channel[i];
				if(channel==NULL)
				{
					return FALSE;
				}

				CString strConfig,stemp;
				strConfig=channel->m_sDataType;
				stemp=RemoveBlank(dataType);

				if (stemp.GetLength()==0 || strConfig.GetLength()==0)
				{
					return FALSE;
				}

				if (strConfig.Find(";") >=0)
				{
					// include multiple dataType;
					strConfig=";"+strConfig+";";
					stemp=";"+stemp+";";
					if (strConfig.Find(stemp)>=0)
					{
						if (portaddress == NULL)
						{
							Clog( LOG_DEBUG,_T("FindOutPortIndexAndChannelIndex include(;) dataType:(%s) GroupID:(%d),*portaddress is null ,error"),dataType,GroupID);
							return FALSE;
						}
						*portaddress=pPort;
						ChannelIndex=i;
						return TRUE;
					}
				}
				else
				{
					if (stemp.CompareNoCase(strConfig)==0)
					{
						if (portaddress == NULL)
						{
							Clog( LOG_DEBUG,_T("FindOutPortIndexAndChannelIndex dataType:(%s) GroupID:(%d),*portaddress is null ,error"),dataType,GroupID);
							return FALSE;
						}
						*portaddress=pPort;
						ChannelIndex=i;
						return TRUE;
					}
				}
			}
		}
	}
	catch (...) 
	{
		Clog( LOG_DEBUG,_T("FindOutPortIndexAndChannelIndex error:dataType:(%s) GroupID:(%d),lasterror=%d "),dataType,GroupID,GetLastError());
		return FALSE;
	}
	return FALSE;
}
BOOL CJMSParser::FindOutChannelIndex(CString dataType,CDODPort *portaddress,int &ChannelIndex)
{
	if (m_pm==NULL)
	{
		return FALSE;
	}

	if(1>(int)(m_pm->m_PortVector.size()))
	{
		Clog( LOG_DEBUG,_T("CJMSParser::FindOutChannelIndex PortVector is zero error.") );
		return FALSE;
	}

	CDODPort *pPort=portaddress;
	if(pPort==NULL)
	{
		return FALSE;
	}
	try
	{

		for(int i=0;i<pPort->m_ChannelNumber;i++)
		{
			CDODChannel *channel=pPort->m_channel[i];
			if(channel==NULL)
			{
				return FALSE;
			}

			CString strConfig,stemp;
			strConfig=channel->m_sDataType;
			stemp=RemoveBlank(dataType);

			if (stemp.GetLength()==0 || strConfig.GetLength()==0)
			{
				return FALSE;
			}

			if (strConfig.Find(";") >=0)
			{
				// include multiple dataType;
				strConfig=";"+strConfig+";";
				stemp=";"+stemp+";";
				if (strConfig.Find(stemp)>=0)
				{
					ChannelIndex=i;
					return TRUE;
				}
			}
			else
			{
				if (stemp.CompareNoCase(strConfig)==0)
				{
					ChannelIndex=i;
					return TRUE;
				}
			}
		}
	}
	catch (...) 
	{
		Clog( LOG_DEBUG,_T("FindOutChannelIndex error:dataType:(%s),lasterror=%d "),dataType,GetLastError());
		return FALSE;
	}
	return FALSE;
}
int CJMSParser::ReceiverDataFolder(char *buf,int buffersize)
{
	CString str=buf;
	CString sTmp = _T("");
	CMarkup m_XmlDOM;

	//Clog( LOG_DEBUG,_T("CJMSParser::ReceiverDataFolder") );

	str="";
		
	m_XmlDOM.SetDoc(buf);

	if( !m_XmlDOM.FindElem( DODMESSAGEFLAG) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(DODMESSAGEFLAG).") );
		return DODRECEIVERDATAFOLDERERROR;
	}
	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(DODMESSAGEHEADER).") );
		return 1;
	}
	int messagecode=0;
	messagecode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib(DODMESSAGECODE));

	if( !m_XmlDOM.FindElem( DODMESSAGEBODY) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(DODMESSAGEBODY).") );
		return 1;
	}
	int channelIndex=0;
	sTmp= m_XmlDOM.GetAttrib( MESSAGEDATATYPE );
	messagecode= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEGROUPID ));

	channelIndex=0;
	CDODPort *teport=NULL;

	if(FindOutPortIndexAndChannelIndex(sTmp,messagecode,&teport,channelIndex)==FALSE)
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder FindOutPortIndexAndChannelIndex- nDataType or groupID error") );
		return 1;
	}

// ------------------------------------------------------ Modified by zhenan_ji at 2006年6月9日 16:09:25
	//To bug3221 ,
	if (teport->m_channel[channelIndex]->m_nType ==NO_USE_JMS)
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder :current channel's nDataType is local_file") );
		return 0;
	}

	int UpdateMode=0;
	int nReserve=0;
	m_XmlDOM.IntoElem();
	if( !m_XmlDOM.FindElem(CONTENTFILEOPERATION) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(CONTENTFILEOPERATION).") );
		return 1;
	}
	sTmp = m_XmlDOM.GetAttrib(CONTENTROOT);
	UpdateMode= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib(CONTENTUPDATEMODE));
	CString subPath="";
	if (UpdateMode==0)
	{
// ------------------------------------------------------ Modified by zhenan_ji at 2006年1月20日 18:18:25
//Add delete all file ,if this message is update all data.
		int ret=DeleteDirectory((teport->m_channel[channelIndex]->m_strCachingDir).GetBuffer(0));
		if (!ret)
		{
			Clog(LOG_DEBUG,_T(" ProcessShareFolder: delete rCachingDir directory error.lasterror=%d"),GetLastError());
			Clog(LOG_DEBUG,_T("[%s-%s] This directory is %s "),teport->m_sPortName,teport->m_channel[channelIndex]->m_sChannelName,teport->m_channel[channelIndex]->m_strCachingDir);
			return 1;
		}
		Sleep(3);
		CString  osTofDir=teport->m_channel[channelIndex]->m_strCachingDir;
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		if (CreateDirectory((LPCTSTR)osTofDir, NULL) == FALSE)		
		{
			Clog(LOG_DEBUG,_T("[%s-%s]ProcessShareFolder: CreateDirectory is error.lasterror=%d"),teport->m_sPortName,teport->m_channel[channelIndex]->m_sChannelName,GetLastError());
			return 1;
		}

		//new full data location
		if(teport->m_channel[channelIndex]->MoveAllFile(sTmp,nReserve,sTmp))
		{
			Clog(LOG_DEBUG,_T("[%s-%s]ProcessShareFolder: MoveAllFile is error.lasterror=%d"),teport->m_sPortName,teport->m_channel[channelIndex]->m_sChannelName,GetLastError());
			return 1;;
		}
	}
	else if (UpdateMode==4)
	{
		// is the file with full path to be Modofied
		if(teport->m_channel[channelIndex]->FullPathModofied(sTmp,nReserve,sTmp))
		{
			Clog(LOG_DEBUG,_T("[%s-%s]ProcessShareFolder: FullPathModofied is error.lasterror=%d"),teport->m_sPortName,teport->m_channel[channelIndex]->m_sChannelName,GetLastError());
			return 1;
		}
	}
	else
	{
		//m_XmlDOM.IntoElem();
		//Clog(LOG_DEBUG,_T("[%s-%s]ProcessShareFolder:Other Update_format :Into to Elem(file element) "));

		while(m_XmlDOM.FindChildElem("File"))
		{
			//Clog( LOG_DEBUG,_T("ReceiverDataFolder Parser- !FindElem(file_path).") );
			m_XmlDOM.IntoElem();
			subPath = m_XmlDOM.GetAttrib(CONTENTPATH);
			Clog(LOG_DEBUG,_T("ProcessShareFolder:Update_format=(%d) :file_attrib(%s) "),UpdateMode,subPath);
			switch(UpdateMode)
			{
			case 1:
				// the sub folder to be updated
				if(teport->m_channel[channelIndex]->UpdateSubfolder(sTmp,nReserve,subPath))
					return 1;		
				break;
			case 2:
				//the sub folder to be deleted
				if(teport->m_channel[channelIndex]->DeleteFullPath(sTmp,nReserve,subPath))
					return 1;	
				break;
			case 3:
				// the file with full path to be deleted
				if(teport->m_channel[channelIndex]->DeleteSubFile(sTmp,nReserve,subPath))	
					return 1;
				break;
			case 5:
				// the file with full path to be newed
				if(teport->m_channel[channelIndex]->FullPathToNew(sTmp,nReserve,sTmp))
					return 1;	
				break;
			case 6:
				//Only update file ,Don't delete old file
				if(teport->m_channel[channelIndex]->CreateOrReplaceFile(sTmp,nReserve,subPath))	
					return 1;
				break;

			default:
				break;
				m_XmlDOM.OutOfElem();	
			}
	
			//m_XmlDOM.OutOfElem();		
		}
	}	
	
	m_pm->m_kit->UpdateCatalog(teport->m_nSessionID,teport->m_nID,teport->m_channel[channelIndex]->m_nStreamID);
	Clog( LOG_DEBUG,_T("[%s-%s]::UpdateChannel will start!"),teport->m_sPortName,teport->m_channel[channelIndex]->m_sChannelName);

	Sleep(2);
// ------------------------------------------------------ Modified by zhenan_ji at 2006年2月28日 17:47:37
	if (teport->m_channel[channelIndex]->m_sSendMsgDataType.GetLength() >0)
	{
		CString strContent;
		CCreateILP tmpilp;
		CString filename;

		if(!CheckDigiteORIsalpha(TRUE,teport->m_channel[channelIndex]->m_sDataType,"checkDataType"))
		{	
			Clog( LOG_DEBUG,_T("CJMSParser::ReceiverdataFolder DataType error") );
			return 1;
		}	
		int nDataType=0,nodeID,nExpiredTime;
		CString sDateType;
		try
		{
			nDataType= atoi(teport->m_channel[channelIndex]->m_sDataType);
			strContent=tmpilp.CreateILPFile("",nDataType);
			if (strContent.GetLength()<=0)
				return 1;

			nodeID=teport->m_nGroupID;
			sDateType=teport->m_channel[channelIndex]->m_sSendMsgDataType;
			nExpiredTime=teport->m_channel[channelIndex]->m_nSendMsgExpiredTime;

		}
		catch (...)
		{
			Clog( LOG_DEBUG,_T("CJMSParser::ReceiverdataFolder DataType errorcode=%d") ,GetLastError());
		}
		try
		{
			CPORTVECTOR::iterator iter;

			for(iter=m_pm->m_PortVector.begin();iter!=m_pm->m_PortVector.end();iter++)
			{
				CDODPort *pPorttmp=(*iter);
				if(pPorttmp==NULL)
				{
					return FALSE;
				}

				if (nodeID ==0 || pPorttmp->m_nGroupID==0)
					;
				else
				{
					if (nodeID != pPorttmp->m_nGroupID)
						continue;
				}
				int nNewchannelIndex=0;			

				if(FindOutChannelIndex(sDateType,pPorttmp,nNewchannelIndex)==FALSE)
					continue;

				pPorttmp->m_channel[nNewchannelIndex]->CreateMsgFile(strContent,"000000",nExpiredTime,"00",0);	

				pPorttmp->m_channel[nNewchannelIndex]->m_bNeedUpdateChannel=TRUE;
			}
		}
		catch (...) 
		{
			Clog( LOG_DEBUG,_T("ReceiverDataFolder error:SendMsgDataType:(%s) GroupID:(%d),lasterror=%d "),sDateType,nodeID,GetLastError());
			return 0;
		}
	}

	return 0; 
}

int CJMSParser::ReceiverDataTCP(char *buf,int buffersize,int nGroupID_zero,int &GroupID_is_zero)
{
	CString str,sTmp;
	CMarkup m_XmlDOM;

	m_XmlDOM.SetDoc(buf);

	if( !m_XmlDOM.FindElem(DODMESSAGEFLAG ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DODMESSAGEFLAG).") );
		return 1;
	}
	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DODMESSAGEHEADER).") );
		return 1;
	}
	int Port= 0;

	int messagecode=0;
	messagecode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( DODMESSAGECODE ));

	if (messagecode !=FULLDATAWITHTCP)
		return 1;

	if( !m_XmlDOM.FindElem( DODMESSAGEBODY ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DODMESSAGEBODY).") );
		return 1;
	}

	int channelIndex=0;
	CString sDatatype= m_XmlDOM.GetAttrib( MESSAGEDATATYPE );
	messagecode= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEGROUPID ));

	if (nGroupID_zero)
	{
		if (messagecode==0)
			return 0;
		messagecode=0;
	}

	channelIndex=0;
	CDODPort *teport=NULL;

	if(FindOutPortIndexAndChannelIndex(sDatatype,messagecode,&teport,channelIndex)==FALSE)
	{
		//V1,0,1,6	2006.06.20	zhenan_ji
		if (nGroupID_zero)
			return 0;
		Clog( LOG_DEBUG,_T("ReceiverDataTCP FindOutPortIndexAndChannelIndex- nDataType or groupID error") );
		return 1;
	}
	if (messagecode==0)
		GroupID_is_zero=1;

	if (teport->m_channel[channelIndex]->m_nType ==NO_USE_JMS)
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP :current channel's nDataType is local_file") );
		return 0;
	}

	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGECONTENT))
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DataList).") );
		return 1;
	}
	CString strDestinationName,filename,strCancel;
	strDestinationName=m_XmlDOM.GetAttrib( "Destination");

	int sssize=strDestinationName.GetLength();
// ------------------------------------------------------ Modified by zhenan_ji at 2005年8月30日 10:33:32
	filename="000000";

	if (sssize<=JMSMESSAGE_DESTIONATIONFILENAME)
	{	
		filename.Delete(0,sssize);
		filename.Insert(JMSMESSAGE_DESTIONATIONFILENAME-sssize,strDestinationName);
	}
	else
	{
		filename=strDestinationName;
	}
	CString sMessageID = m_XmlDOM.GetAttrib( MESSAGEMESSAGEID);
	try
	{
		// ------------------------------------------------------ Modified by zhenan_ji at 2005年10月14日 15:59:59
		strCancel=m_XmlDOM.GetAttrib( MESSAGEOPERATION);

		if (strCancel.GetLength() >0)
		{
			if (sMessageID.GetLength() ==0)
			{
				Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- nMessageID==0 error.") );
				return 1;
			}
			if (strCancel.CompareNoCase("cancel")==0)
			{
				Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- received cancel OK.") );
				teport->m_channel[channelIndex]->StopFileSend(sMessageID);
			}
			else
			{
				Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- Unknow command (operation).") );
				return 1;
			}			
		}
		else
		{
			int leafTime=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "ExpiredTime"));
			// ------------------------------------------------------ Modified by zhenan_ji at 2006年1月11日 14:23:43
			//Add messageID for filename.
			strDestinationName.Format("_%s",sMessageID);
			filename=strDestinationName+"_"+filename;

			if (teport->m_channel[channelIndex]->m_bEncrypted)
			{
				//m_XmlDOM.IntoElem();

				//bool baaa=m_XmlDOM.FindElem("");
				sTmp=m_XmlDOM.GetData();
				//		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- GetData.=%s"),sTmp );
			}
			else
			{
				m_XmlDOM.IntoElem();
				bool baaa=m_XmlDOM.FindElem("");
				sTmp=m_XmlDOM.GetSubDoc();
			}
		teport->m_channel[channelIndex]->CreateMsgFile(sTmp,filename,leafTime,sMessageID,0);
	}

		teport->m_channel[channelIndex]->m_bNeedUpdateChannel=TRUE;
	//Clog( LOG_DEBUG,_T(" teport->m_channel[channelIndex]->m_bNeedUpdateChannel is true)!"));
	}
	catch (...) 
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP error:sMessageID:(%s) datatype:(%s),lasterror=%d "),sMessageID,sDatatype,GetLastError());
		return 0;
	}
	return 0;
}

int CJMSParser::ReceiverDataTCPToAllChannel(char *buf)
{

	if (m_pm==NULL)
	{
		return FALSE;
	}

	if(1>(int)(m_pm->m_PortVector.size()))
	{
		Clog( LOG_DEBUG,_T("CJMSParser::FindOutPortIndexAndChannelIndex PortVector is zero error.") );
		return FALSE;
	}

	CPORTVECTOR::iterator iter;

	for(iter=m_pm->m_PortVector.begin();iter!=m_pm->m_PortVector.end();iter++)
	{
		CDODPort *pPort=(*iter);
		if(pPort==NULL)
		{
			return FALSE;
		}

		if (pPort->m_nGroupID ==0)
			continue;

		CString str;
		CString sTmp = _T("");
		CMarkup m_XmlDOM;

		m_XmlDOM.SetDoc(buf);

		if( !m_XmlDOM.FindElem(DODMESSAGEFLAG ) )
		{
			Clog( LOG_DEBUG,_T("ReceiverDataTCPToAllChannel Parser- !FindElem(DODMESSAGEFLAG).") );
			return 1;
		}
		m_XmlDOM.IntoElem();

		if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
		{
			Clog( LOG_DEBUG,_T("ReceiverDataTCPToAllChannel Parser- !FindElem(DODMESSAGEHEADER).") );
			return 1;
		}
		int Port= 0;

		int messagecode=0;
		messagecode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( DODMESSAGECODE ));

		if (messagecode !=FULLDATAWITHTCP)
			return 1;

		if( !m_XmlDOM.FindElem( DODMESSAGEBODY ) )
		{
			Clog( LOG_DEBUG,_T("ReceiverDataTCPToAllChannel Parser- !FindElem(DODMESSAGEBODY).") );
			return 1;
		}

		int channelIndex=0;
		CString strDataType= m_XmlDOM.GetAttrib( MESSAGEDATATYPE );
		messagecode= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEGROUPID ));
		try
		{
			if (messagecode !=0 )
			{
				Clog( LOG_DEBUG,_T("ReceiverDataTCPToAllChannel Parser- find messagecode is not zero.") );
				return 1;
			}

			channelIndex=0;
			CDODPort *teport=pPort;

			if(FindOutChannelIndex(strDataType,teport,channelIndex)==FALSE)
				continue;

			Clog( LOG_DEBUG,_T("ReceiverDataTCPToAllChannel :Current channel's groupID=%d "),pPort->m_nGroupID);

			if (teport->m_channel[channelIndex]->m_nType ==NO_USE_JMS)
			{
				Clog( LOG_DEBUG,_T("ReceiverDataTCPToAllChannel :current channel's nDataType is local_file") );
				return 0;
			}

			m_XmlDOM.IntoElem();

			if( !m_XmlDOM.FindElem( DODMESSAGECONTENT))
			{
				Clog( LOG_DEBUG,_T("ReceiverDataTCPToAllChannel Parser- !FindElem(DataList).") );
				return 1;
			}
			CString strDestinationName,filename,strCancel;
			strDestinationName=m_XmlDOM.GetAttrib( "Destination");

			int sssize=strDestinationName.GetLength();
			// ------------------------------------------------------ Modified by zhenan_ji at 2005年8月30日 10:33:32
			filename="000000";

			if (sssize<=JMSMESSAGE_DESTIONATIONFILENAME)
			{	
				filename.Delete(0,sssize);
				filename.Insert(JMSMESSAGE_DESTIONATIONFILENAME-sssize,strDestinationName);
			}
			else
			{
				filename=strDestinationName;
			}

			CString sMessageID=m_XmlDOM.GetAttrib( MESSAGEMESSAGEID);

			// ------------------------------------------------------ Modified by zhenan_ji at 2005年10月14日 15:59:59
			strCancel=m_XmlDOM.GetAttrib( MESSAGEOPERATION);

			if (strCancel.GetLength() >0)
			{
				if (sMessageID.GetLength() ==0)
				{
					Clog( LOG_DEBUG,_T("ReceiverDataTCPToAllChannel Parser- nMessageID==0 error.") );
					return 1;
				}
				if (strCancel.CompareNoCase("cancel")==0)
				{
					Clog( LOG_DEBUG,_T("ReceiverDataTCPToAllChannel Parser- received cancel OK.") );
					teport->m_channel[channelIndex]->StopFileSend(sMessageID);
				}
				else
				{
					Clog( LOG_DEBUG,_T("ReceiverDataTCPToAllChannel Parser- Unknow command (operation).") );
					return 1;
				}			
			}
			else
			{
				int leafTime=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "ExpiredTime"));
				// ------------------------------------------------------ Modified by zhenan_ji at 2006年1月11日 14:23:43
				//Add messageID for filename.
				strDestinationName.Format("_%s",sMessageID);
				filename=strDestinationName+"_"+filename;

				if (teport->m_channel[channelIndex]->m_bEncrypted)
				{				
					sTmp=m_XmlDOM.GetData();
					//		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- GetData.=%s"),sTmp );
				}
				else
				{
					m_XmlDOM.IntoElem();
					bool baaa=m_XmlDOM.FindElem("");
					sTmp=m_XmlDOM.GetSubDoc();
					//		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- GetSubDoc.=%s"),sTmp );
				}			
				teport->m_channel[channelIndex]->CreateMsgFile(sTmp,filename,leafTime,sMessageID,0);
			}

			teport->m_channel[channelIndex]->m_bNeedUpdateChannel=TRUE;
			//Clog( LOG_DEBUG,_T(" teport->m_channel[channelIndex]->m_bNeedUpdateChannel is true)!"));

		}
		catch (...) 
		{
			Clog( LOG_DEBUG,_T("ReceiverDataTCPToAllChannel error:GroupID(%d) datatype:(%s),lasterror=%d "),messagecode,strDataType,GetLastError());
			return 0;
		}

	}
	return 0;
}

int CJMSParser::ReceiverPortConfigMsg(char *buf,int buffersize,PPsortInfo *info)
{
	CString xmlstr;
	CString sTmp = _T("");
	//BOOL bReturn=FALSE;

	//Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg") );

	CString name;
	MSXML2::IXMLDOMDocumentPtr pDoc; 
	MSXML2::IXMLDOMElementPtr  childNode ;
	MSXML2::IXMLDOMNodePtr       pNode = NULL;
	MSXML2::IXMLDOMNodePtr		 pTmp = 0;
	MSXML2::IXMLDOMNodePtr		 pFather = 0;
	MSXML2::IXMLDOMNodePtr		 pSon = 0;
	MSXML2::IXMLDOMNamedNodeMapPtr pNamedNode = 0;
	COleVariant nodeValue;
	MSXML2::IXMLDOMNodeListPtr childlist; 
	xmlstr.Format("%s",buf);

	HRESULT hr = pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument30));
	if(!SUCCEEDED(hr))    
	{		//AfxMessageBox("无法创建DOMDocument对象，请检查是否安装了MS XML Parser 运行库!"); 
		Clog( LOG_DEBUG,_T("IXMLDOMDocumentPtr::CreateInstance error !") );
		return 1;
	}

//The follow codes was used the programer when pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument));

	//pDoc->PutvalidateOnParse(VARIANT_FALSE);
	//pDoc->PutresolveExternals(VARIANT_FALSE);
	//pDoc->PutpreserveWhiteSpace(VARIANT_TRUE);

	if(pDoc->loadXML((_bstr_t)CString(xmlstr))!=VARIANT_TRUE)
	{
		Clog( LOG_DEBUG,_T("pDoc->loadXML((_bstr_t)CString(xmlstr) error") );
		return 1;
	}
	long nchildNumber=0;

	try
	{	
		//find "CONTENTCONFRESPONSE"
		childNode = (MSXML2::IXMLDOMElementPtr)(pDoc->selectSingleNode(CONTENTCONFRESPONSE));

		if (childNode==NULL)
		{
			Clog( LOG_DEBUG,_T("pDoc->selectSingleNode(CONTENTCONFRESPONSE error") );
			return 1;
		}

		pNode = childNode->GetfirstChild();
		if (pNode==NULL)
		{
			Clog( LOG_DEBUG,_T("  childNode->GetfirstChild() error") );
			return 1;
		}
		name = CString((wchar_t*)pNode->GetnodeName());
		if(name.Compare("ListOfNode")!=0)
		{
			Clog( LOG_DEBUG,_T("ListOfNode is not found") );
			return 1;
		}

		pNode = pNode->GetfirstChild();
		name = CString((wchar_t*)pNode->GetnodeName());
		if(name.Compare("DCAPortConfiguration")!=0)
		{
			Clog( LOG_DEBUG,_T("DCAPortConfiguration is not found") );
			return 1;
		}

		pNamedNode = pNode->Getattributes();
		if(pNamedNode==NULL)
		{	
			Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for DCAPortConfiguration error") );
			return 1;
		}
		//the queuename is existed JMS for APP 's configuration

		pTmp = pNamedNode->getNamedItem((_bstr_t)"QueueName");
		if(pTmp==NULL)
		{	
			Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for QueueName error") );
			return 1;
		}	
		nodeValue = pTmp->GetnodeValue();

		m_pm->m_Localqueuename=CString((wchar_t*)nodeValue.bstrVal);

// ------------------------------------------------------ Modified by zhenan_ji at 2006年7月5日 11:15:48
		pTmp = pNamedNode->getNamedItem((_bstr_t)"DataTypeWithInitial");
		if(pTmp==NULL)
		{	
			Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for DataTypeWithInitial error"));
			m_pm->m_sDataTypeInitial="";
		}
		else
		{
			nodeValue = pTmp->GetnodeValue();
			name=RemoveBlank(nodeValue.bstrVal);
			if(name.GetLength()<1)
			{	
				m_pm->m_sDataTypeInitial="";
			}	
			else
				m_pm->m_sDataTypeInitial=name;
		}

		pNode = pNode->GetfirstChild();
		name = CString((wchar_t*)pNode->GetnodeName());
		if(name.Compare("Port")!=0)
		{
			Clog( LOG_DEBUG,_T("Port is not found") );
			return 1;
		}

	// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月28日 11:08:00
	// prevent current node is not child or namenode		
		pNamedNode = pNode->Getattributes();
		if(pNamedNode==NULL)
		{	
			Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for Port error") );
			return 1;
		}	
		childlist=pNode->GetchildNodes();
		hr=childlist->get_length(&nchildNumber);
		if (nchildNumber <1)
		{
			Clog( LOG_DEBUG,_T("CJMSParser Portnumber is zero") );
			return 1;
		}

		pNode = pNode->GetfirstChild();

		//get all config ports.
		//Clog( LOG_DEBUG,_T("CJMSParser::") );
	}
	catch (...) 
	{

	}
	try
	{
		CString SCheckStr;
		while (pNode)
		{
			CDODPort *tempport=new CDODPort;			//get port layer attrib

			tempport->m_sPortName="Port:"+CString((wchar_t*)pNode->GetnodeName());
			pNamedNode = pNode->Getattributes();
			if(pNamedNode==NULL)
			{	
				Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for portname=%s error"),tempport->m_sPortName);
				return 1;
			}	

			pTmp = pNamedNode->getNamedItem((_bstr_t)"PMTPID");
			if(pTmp==NULL)
			{	
				Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for PMTPID, portname=%s error"),tempport->m_sPortName);
				return 1;
			}	
			nodeValue = pTmp->GetnodeValue();
			SCheckStr=nodeValue.bstrVal;
			if(!CheckDigiteORIsalpha(TRUE,SCheckStr,"getNamedItem((_bstr_t)Encrypted)"))
			{	
				Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () PMTPID error") );
				return 1;
			}		
		//	Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () PMTPID error 1 %s"),SCheckStr.GetBuffer(0) );

			nodeValue.ChangeType(VT_INT);
			tempport->m_nPmtPID = nodeValue.intVal;
			//Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () PMTPID error 2") );


			pTmp = pNamedNode->getNamedItem((_bstr_t)(MESSAGEGROUPID));
			if(pTmp==NULL)
			{	
				Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for GroupID, portname=%s error"),tempport->m_sPortName);
				return 1;
			}	
			nodeValue = pTmp->GetnodeValue();
			SCheckStr=nodeValue.bstrVal;
			if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
			{	
				Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () MESSAGEGROUPID error") );
				return 1;
			}	
			nodeValue.ChangeType(VT_INT);
			tempport->m_nGroupID = nodeValue.intVal;

			pTmp = pNamedNode->getNamedItem((_bstr_t)"TotalRate");
			if(pTmp==NULL)
			{	
				Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for TotalRate, portname=%s error"),tempport->m_sPortName);
				return 1;
			}	
			nodeValue = pTmp->GetnodeValue();
			SCheckStr=nodeValue.bstrVal;
			if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
			{	
				Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () TotalRate error") );
				return 1;
			}	
			nodeValue.ChangeType(VT_INT);
			tempport->m_nTotalRate = nodeValue.intVal;

			// get all channels and all castports.

			pFather=pNode->GetfirstChild();

			//pNode->get
			while (pFather)
			{

				//BSTR bStr; 
				//CString strrrr;
				//pFather->get_xml(&bStr);
				//strrrr=bStr;
				
				//get child layer attrib,but this layer is possible IPPort or IPChannel.
				name=CString((wchar_t*)pFather->GetnodeName());
				if(name.Compare("IPPort")==0)
				{
					childlist=pFather->GetchildNodes();
					hr=childlist->get_length(&nchildNumber);
					if (nchildNumber <1)
					{
						Clog( LOG_DEBUG,_T("CJMSParser IPPort number is zero") );
						return 1;
					}
					tempport->m_castPort= new ZQSBFIPPORTINFARRAY[nchildNumber];

					pSon=pFather->GetfirstChild();
					
					int k=0;  //This variable show the number of broadcast ouput
					
					while (pSon)
					{
						tempport->m_castPort[k]=new ZQSBFIPPORTINF;
						strcpy(tempport->m_castPort[k]->cSourceIp,"");
						tempport->m_castPort[k]->wDestPort=0;
						name=CString((wchar_t*)pSon->GetnodeName());
						pNamedNode = pSon->Getattributes();
						if(pNamedNode==NULL)
						{	
							Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for TotalRate, portname=%s error"),name);
							return 1;
						}	

						pTmp = pNamedNode->getNamedItem((_bstr_t)"SendType");
						if(pTmp==NULL)
						{	
							Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for SendType, portname=%s error"),name);
							return 1;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr=nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () SendType error") );
							return 1;
						}	
						//Note:
						//In Configuration Management System Ver0.0.0.1 interface.if user add a note (IPPort'member),but it's IPAddress.Port and SendType are not configured,
						//their value will be saved to SQL as cstring _ fotmat.Then .ChangeType was be called .there are a error dialog (Type matching). 

						nodeValue.ChangeType(VT_INT);
						tempport->m_castPort[k]->wSendType = nodeValue.intVal;

						pTmp = pNamedNode->getNamedItem((_bstr_t)"IPAddress");
						if(pTmp==NULL)
						{	
							Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for IPAddress, portname=%s error"),name);
							return 1;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr=nodeValue.bstrVal;
						if(SCheckStr.GetLength()<1)
						{	
							Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () IPAddress error") );
							return 1;
						}	
						strcpy(tempport->m_castPort[k]->cDestIp,(CString((wchar_t*)nodeValue.bstrVal)).GetBuffer(0));

						pTmp = pNamedNode->getNamedItem((_bstr_t)"IPPort");
						if(pTmp==NULL)
						{	
							Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for IPPort, portname=%s error"),name);
							return 1;
						}	
						nodeValue = pTmp->GetnodeValue();
						SCheckStr=nodeValue.bstrVal;
						if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
						{	
							Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () SendType error") );
							return 1;
						}	
						nodeValue.ChangeType(VT_INT);
						tempport->m_castPort[k]->wDestPort = nodeValue.intVal;

						k++;
						pSon = pSon->GetnextSibling();
						// get a same layer about IPPort attrib
					}
					tempport->m_castcount=k;
				}
				else
					if(name.Compare("Channel")==0)
					{
						childlist=pFather->GetchildNodes();
						hr=childlist->get_length(&nchildNumber);
						if (nchildNumber <1)
						{
							Clog( LOG_DEBUG,_T("CJMSParser Channel number is zero") );
							return 1;
						}
						tempport->m_channel= new CDODCHANNELARRAY[nchildNumber];

						tempport->m_ChannelNumber=0;

						pSon=pFather->GetfirstChild();
						int k=0;
						while (pSon)
						{
							name=CString((wchar_t*)pSon->GetnodeName());
							pNamedNode = pSon->Getattributes();
							if(pNamedNode==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes, portname=%s error"),name);
								return 1;
							}	

							CDODChannel *tempchannel=new CDODChannel();

							tempchannel->m_sChannelName="channel:"+name;
							tempchannel->m_sPortName=tempport->m_sPortName;
						//	tempchannel->m_nUpdateInterval=m_nUpdateInterval;

							pTmp = pNamedNode->getNamedItem((_bstr_t)"Encrypted");
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for Encrypted, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () Encrypted error") );
								return 1;
							}	
						//	Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () Encrypted error 1abc %s"),SCheckStr.GetBuffer(0) );

							nodeValue.ChangeType(VT_INT);
						//	Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () Encrypted error 2abc ") );
							tempchannel->m_bEncrypted = nodeValue.intVal;


							pTmp = pNamedNode->getNamedItem((_bstr_t)"StreamType");
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for StreamType, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () StreamType error") );
								return 1;
							}	
							nodeValue.ChangeType(VT_INT);
							tempchannel->nStreamType = nodeValue.intVal;

// ------------------------------------------------------ Modified by zhenan_ji at 2006年6月26日 11:00:02
							pTmp = pNamedNode->getNamedItem((_bstr_t)"RepeatTime");
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for RepeatTime, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () RepeatTime error") );
								return 1;
							}	
							nodeValue.ChangeType(VT_INT);
							tempchannel->m_nRepeateTime = nodeValue.intVal;

							pTmp = pNamedNode->getNamedItem((_bstr_t)"StreamID");
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for StreamID, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () StreamID error") );
								return 1;
							}	
							nodeValue.ChangeType(VT_INT);
							tempchannel->m_nStreamID = nodeValue.intVal;


							pTmp = pNamedNode->getNamedItem((_bstr_t)"Tag");
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for Tag, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if(!CheckDigiteORIsalpha(FALSE,SCheckStr))
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () Tag error") );
								return 1;
							}	
							strcpy(tempchannel->m_strTag,(CString((wchar_t*)nodeValue.bstrVal)).GetBuffer(0));

							pTmp = pNamedNode->getNamedItem((_bstr_t)"QueueName");
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for QueueName, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if(SCheckStr.GetLength() <1)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () QueueName error") );
								return 1;
							}
							tempchannel->m_QueueName=CString((wchar_t*)nodeValue.bstrVal);


							pTmp = pNamedNode->getNamedItem((_bstr_t)"ChannelRate");
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for ChannelRate, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () ChannelRate error") );
								return 1;
							}
							nodeValue.ChangeType(VT_INT);
							tempchannel->m_nRate = nodeValue.intVal;

							pTmp = pNamedNode->getNamedItem((_bstr_t)(MESSAGECHANNELTYPE));
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for ChannelType, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () MESSAGECHANNELTYPE error") );
								return 1;
							}
							nodeValue.ChangeType(VT_INT);
							tempchannel->m_nMultiplestream = nodeValue.intVal;

							pTmp = pNamedNode->getNamedItem((_bstr_t)"SendWithDestination");
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for SendWithDestination, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () SendWithDestination error") );
								return 1;
							}
							nodeValue.ChangeType(VT_INT);
							tempchannel->m_nSendWithDestination = nodeValue.intVal;

							pTmp = pNamedNode->getNamedItem((_bstr_t)"StreamCount");
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for StreamCount, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () StreamCount error") );
								return 1;
							}
							nodeValue.ChangeType(VT_INT);
							tempchannel->m_nStreamCount = nodeValue.intVal;

							if (tempchannel->m_nMultiplestream)
							{
								if (tempchannel->m_nStreamCount==0)
								{
									Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg m_nMultiplestream=1,but m_nStreamCount=0 error"));
									return 1;
								}
							}
							else
								tempchannel->m_nStreamCount=0;
							pTmp = pNamedNode->getNamedItem((_bstr_t)(MESSAGEDATATYPE));
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for MessageDataType, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=RemoveBlank(nodeValue.bstrVal);
							if (SCheckStr.GetLength() >0)
							{
								tempchannel->m_sDataType = nodeValue.bstrVal;
							}
							else
							{
								tempchannel->m_sDataType="";
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for m_sDataType, portname=%s error"),name);
								return 1;
							}
// ------------------------------------------------------ Modified by zhenan_ji at 2005年9月22日 10:27:46 

							pTmp = pNamedNode->getNamedItem((_bstr_t)"DataExchangeType");
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for DataExchangeType, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () DataExchangeType error") );
								return 1;
							}
							nodeValue.ChangeType(VT_INT);
							tempchannel->m_nType = nodeValue.intVal;

							// ------------------------------------------------------ Modified by zhenan_ji at 2006年2月28日 17:22:18
							// create ilp file for navigation ....
							pTmp = pNamedNode->getNamedItem((_bstr_t)"SendMsgDataType");
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for SendMsgDataType, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if (SCheckStr.CompareNoCase("0")==0)
								tempchannel->m_sSendMsgDataType="";
							else
							{
								SCheckStr=RemoveBlank(nodeValue.bstrVal);
								if (SCheckStr.GetLength() >0)
								{								
									tempchannel->m_sSendMsgDataType = nodeValue.bstrVal;
								}
								else
									tempchannel->m_sSendMsgDataType="";
							}

							pTmp = pNamedNode->getNamedItem((_bstr_t)"SendMsgExpiredTime");
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for SendMsgExpiredTime, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if (SCheckStr.GetLength() >0)
							{
								if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
								{	
									Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () SendMsgExpiredTime error") );
									return 1;
								}
								nodeValue.ChangeType(VT_INT);
								tempchannel->m_nSendMsgExpiredTime = nodeValue.intVal;
							}
							// ------------------------------------------------------ Modified by zhenan_ji at 2005年12月29日 15:16:16

							pTmp = pNamedNode->getNamedItem((_bstr_t)"MessageCode");
							if(pTmp==NULL)
							{	
								Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg Getattributes for MessageCode, portname=%s error"),name);
								return 1;
							}	
							nodeValue = pTmp->GetnodeValue();
							SCheckStr=nodeValue.bstrVal;
							if(!CheckDigiteORIsalpha(TRUE,SCheckStr))
							{	
								tempchannel->m_nMessageType = 0;
							//	Clog( LOG_DEBUG,_T("CJMSParser::ReceiverPortConfigMsg () MessageCode is null ") );
							}
							else
							{
								nodeValue.ChangeType(VT_INT);
								tempchannel->m_nMessageType = nodeValue.intVal;
							}

							tempchannel->m_channelID=k+1;
							tempport->m_channel[k]=tempchannel;
							tempport->m_ChannelNumber ++ ;
							k++;
							pSon = pSon->GetnextSibling();
						}
					}

					//get channel or IPPort 's sibling.
					pFather = pFather->GetnextSibling();

			}
			tempport->m_nID=m_pm->m_PortNumber+1;
			m_pm->m_PortVector.push_back(tempport);
			m_pm->m_PortNumber ++;	

			//get all Port 's sibling.

			pNode = pNode->GetnextSibling();

		}

	}
	catch(_com_error &e)
	{
		Clog( LOG_DEBUG,"ReceiverPortConfigMsg ::parse excetion !");

		CString msg = CString((LPCWSTR)e.Description());
		msg = " "+msg;
	Clog( LOG_DEBUG,msg);
	}

	Clog( LOG_DEBUG,_T("CJMSParser::parse PortConfiguration message successful") );
	return 1;
}

int CJMSParser::ReceiverUpDataFolder(char *buf,int buffersize)
{
	CString str;
	CString sTmp = _T("");
	CMarkup m_XmlDOM;

	Clog( LOG_DEBUG,_T("CJMSParser::Receiver a UpDataFolder msg") );

	m_XmlDOM.SetDoc(buf);

	if( !m_XmlDOM.FindElem( DODMESSAGEFLAG ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverUpDataFolder Parser- !FindElem(DODMESSAGEFLAG).") );
		return DODRECEIVERDATAFOLDERERROR;
	}
	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverUpDataFolder Parser- !FindElem(DODMESSAGEHEADER).") );
		return 1;
	}
	int Port=0;
	int messagecode=0;
	messagecode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( DODMESSAGECODE ));

	if (messagecode !=FULLDATAWITHSHAREDFOLDERS)
		return DODRECEIVERDATAFOLDERERROR;

	if( !m_XmlDOM.FindElem( DODMESSAGEBODY ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverUpDataFolder Parser- !FindElem(DODMESSAGEBODY).") );
		return 1;
	}
	int channelIndex=0;
	sTmp= m_XmlDOM.GetAttrib( MESSAGEDATATYPE );
	messagecode= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEGROUPID ));

	channelIndex=0;
	CDODPort *teport=NULL;

	if(FindOutPortIndexAndChannelIndex(sTmp,messagecode,&teport,channelIndex)==FALSE)
	{
		Clog( LOG_DEBUG,_T("ReceiverUpDataFolder Parser- nDataType or MESSAGEGROUPID error") );
		return 1;
	}

	int UpdateMode=0;
	m_XmlDOM.IntoElem();
	if( !m_XmlDOM.FindElem( CONTENTFILEOPERATION ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverUpDataFolder Parser- !FindElem(CONTENTFILEOPERATION).") );
		return 1;
	}
	sTmp = m_XmlDOM.GetAttrib(CONTENTROOT);
	UpdateMode= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( CONTENTUPDATEMODE));
	CString subPath="";
	Clog( LOG_DEBUG,_T("current port is %s"), teport->m_sPortName);
	if (UpdateMode==0)
	{
		//new full data location
		teport->m_channel[channelIndex]->MoveAllFile(sTmp,Port,sTmp);
	}

	else
	{
		m_XmlDOM.IntoElem();

		if( !m_XmlDOM.FindElem() )
		{
			Clog( LOG_DEBUG,_T("ReceiverUpDataFolder Parser- !FindElem(CONTENTFILE).") );
			return 1;
		}
		subPath = m_XmlDOM.GetAttrib(CONTENTPATH);
		switch(UpdateMode)
		{
		case 1:
			// the sub folder to be updated
			teport->m_channel[channelIndex]->UpdateSubfolder(subPath,Port,sTmp);		
			break;
		case 2:
			//the sub folder to be deleted
			teport->m_channel[channelIndex]->DeleteSubFile(subPath,Port,sTmp);	
			break;
		case 3:
			// the file with full path to be deleted
			teport->m_channel[channelIndex]->DeleteFullPath(sTmp,Port,sTmp);	
			break;
		case 4:
			// is the file with full path to be Modofied
			teport->m_channel[channelIndex]->FullPathModofied(sTmp,Port,sTmp);	
			break;
		case 5:
			// the file with full path to be newed
			teport->m_channel[channelIndex]->FullPathToNew(sTmp,Port,sTmp);	
			break;

		default:
			break;
		}
	}

	m_pm->m_kit->UpdateCatalog(teport->m_nSessionID,teport->m_nID,teport->m_channel[channelIndex]->m_nStreamID);

	return 0; 
}

int CJMSParser::ReceiverUpDataTCP(char *buf,int buffersize)
{
	CString str;
	CString sTmp = _T("");
	CMarkup m_XmlDOM;

	Clog( LOG_DEBUG,_T("CJMSParser::parse a UPDATAMSG of  msg") );

	m_XmlDOM.SetDoc(buf);

	if( !m_XmlDOM.FindElem( DODMESSAGEFLAG ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DODMESSAGEFLAG).") );
		return 1;
	}
	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGEHEADER ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DODMESSAGEHEADER).") );
		return 1;
	}
	int Port= 0;

	int messagecode=0;
	messagecode=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( DODMESSAGECODE ));

	if (messagecode !=FULLDATAWITHTCP)
		return 1;

	if( !m_XmlDOM.FindElem( DODMESSAGEBODY ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DODMESSAGEBODY).") );
		return 1;
	}
	CString ss,filename;
	int channelIndex=0;
	ss= m_XmlDOM.GetAttrib( MESSAGEDATATYPE );
	messagecode= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEGROUPID ));

	channelIndex=0;
	CDODPort *teport=NULL;

	if(FindOutPortIndexAndChannelIndex(ss,messagecode,&teport,channelIndex)==FALSE)
	{
		Clog( LOG_DEBUG,_T("ReceiverUpDataTCP Parser- MESSAGEDATATYPE or MESSAGEGROUPID error") );
		return 1;
	}

	m_XmlDOM.IntoElem();

	if( !m_XmlDOM.FindElem( DODMESSAGECONTENT ) )
	{
		Clog( LOG_DEBUG,_T("ReceiverDataTCP Parser- !FindElem(DataList).") );
		return 1;
	}
	
	ss=m_XmlDOM.GetAttrib( "Destination");

	int sssize=ss.GetLength();
	// ------------------------------------------------------ Modified by zhenan_ji at 2005年8月30日 10:55:18
	filename="000000";
	if (sssize<=JMSMESSAGE_DESTIONATIONFILENAME || sssize>0)
	{	
		filename.Insert(JMSMESSAGE_DESTIONATIONFILENAME-sssize,ss);
	}

	int leafTime=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "ExpiredTime"));
	CString sMessageID=m_XmlDOM.GetAttrib( MESSAGEMESSAGEID);
	int datatype=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGEDATATYPE));
	
	sTmp=m_XmlDOM.GetSubDoc();
	Clog( LOG_DEBUG,_T("current port is %s"), teport->m_sPortName);
	teport->m_channel[channelIndex]->CreateMsgFile(sTmp,filename,leafTime,sMessageID,0);

	m_pm->m_kit->UpdateCatalog(teport->m_nSessionID,teport->m_nID,teport->m_channel[channelIndex]->m_nStreamID);

//	Clog( LOG_DEBUG,_T("CJMSParser::ReceiverUpDataTCP end") );

	return 0;
}
BOOL CJMSParser::CheckDigiteORIsalpha(BOOL Isdigiter,CString &str,CString ElementIDForLog)
{
	int len=str.GetLength();
	if (len <1)
	{
		//Clog( LOG_DEBUG,_T("ReceiverDataTCP CheckDigiteORIsalpha- str.=%s error "),ElementIDForLog);
		return FALSE;
	}

	char a;
	for(int i=0;i<len;i++)
	{
		a=str.GetAt(i);
		if (Isdigiter)
		{
			if(isdigit(a)==false)
			{
				Clog( LOG_DEBUG,_T("ReceiverDataTCP CheckDigiteORIsalpha- isdigit(a)==false. %s error "),ElementIDForLog);
				return FALSE;
			}
		}
		else
		{
			if(isalpha(a)==false)
			{
				Clog( LOG_DEBUG,_T("ReceiverDataTCP CheckDigiteORIsalpha- isalpha(a)==false. %s error "),ElementIDForLog);
				return FALSE;
			}
		}
	}
	return TRUE;
}