// JmsMsgSender.cpp: implementation of the JmsMsgSender class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable : 4786)
#include "JmsMsgSender.h"
#include "ChannelMessageQueue.h"
#include "KeyDefine.h"
#include "Log.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#pragma comment(lib,"jmsc.lib")

#ifdef _DEBUG
#	pragma comment(lib,"jmscpp_d.lib")
#else
#	pragma comment(lib,"jmscpp.lib")
#endif

using namespace ZQ::common;
using namespace ZQ::JMSCpp;



const char*	JmsMsgSender::_requiredFields[]={
	KD_KN_OUTPUT,
};
int	JmsMsgSender::_nRequiredField = sizeof(JmsMsgSender::_requiredFields)/sizeof(const char*);

JmsMsgSender::JmsMsgSender(int iChannelID):BaseMessageReceiver(iChannelID)
{
	m_pJmsContext=NULL;
	m_bJmsInitializeOK=false;
	m_bConnectionOK=false;
	m_iNoneSendMsgFlushCount=1000;
	ZeroMemory(&m_MsgOption,sizeof(ProducerOptions));
}

JmsMsgSender::~JmsMsgSender()
{

}
void JmsMsgSender::requireFields(std::vector<std::string>& fields)
{
	fields.clear();

	for(int i=0;i<_nRequiredField;i++)
	{
		fields.push_back(_requiredFields[i]);
	}
}
bool JmsMsgSender::init(InitInfo& initInfo, const char* szSessionName)
{
	//Initialize configuration
	glog(Log::L_DEBUG,"JmsMsgSender::init():get jms config start");
	
	
	m_strIniFilePath=initInfo.GetIniFilePath();
	glog(Log::L_DEBUG," JmsMsgSender::init():Record ini file path %s",m_strIniFilePath.c_str());

	//Set current segment
	initInfo.setCurrent(szSessionName);
	
	//Get Naming context
	if(!initInfo.getValue(JMS_NAMING_CONTEXT,m_strNamingContext))
	{
		glog(Log::L_ERROR,"JmsMsgSender::init():error when retrieve jms naming context");
		return false;
	}

	//get server IP and port
	if(!initInfo.getValue(JMS_SERVER_IPPORT,m_strServerAddress))
	{
		glog(Log::L_ERROR,"JmsMsgSender::init():error when retrive jms server IP and Port");
		return false;
	}

	// get destination name
	if(!initInfo.getValue(JMS_DEST_NAME,m_strDestinationName))
	{
		glog(Log::L_ERROR,"JmsMsgSender::init():error when retrieve jms destination name");
		return false;
	}

	//get connection factory
	if(!initInfo.getValue(JMS_CONN_FACTORY,m_strConnectionFactory))
	{
		glog(Log::L_ERROR,"JmsMsgSender::init():error when retrieve jms connection factory");
		return false;
	}

	//Get re-connect count
	if(!initInfo.getValue(JMS_RECONNECT_COUNT,m_iReconnectCount))
	{
		glog(Log::L_ERROR,"JmsMsgSender::init():error when retrieve jms re-connect count,set it to default =5");
		m_iReconnectCount=5;
	}

	//Get re-connect interval
	if(!initInfo.getValue(JMS_RECONNECT_INTERVAL,m_iReconnectInterval))
	{
		glog(Log::L_ERROR,"JmsMsgSender::init():error when retrieve jms re-connect interval,set it to default=10*1000");
		m_iReconnectInterval=10*1000;
	}
	
	//Get storage file path
	if(!initInfo.getValue(JMS_STORAGE_FILEPATH,m_strMsgStoreFile))
	{
		glog(Log::L_ERROR,"JmsMsgSender::init():error when retrieve storage file path,set it to c:\\storageFile.txt");
		m_strMsgStoreFile="c:\\storageFile.txt";
	}
	if(m_strMsgStoreFile.empty())
	{
		glog(Log::L_ERROR,"JmsMsgSender::init(): storage file path is empty,set tit to c:\\storageFile.txt");
		m_strMsgStoreFile="c:\\storageFile.txt";
	}
	
	//Get flush count
	if(!initInfo.getValue(JMS_FLUSH_COUNT,m_iNoneSendMsgFlushCount))
	{
		glog(Log::L_ERROR,"JmsMsgSender::init():error when retrieve flush count,set it to 100");
		m_iNoneSendMsgFlushCount=100;

	}
	//get keep alive time
	long	lKeepAliveTime=0;
	if(!initInfo.getValue(JMS_KEEPALIVE_TIME,lKeepAliveTime))
	{
		glog(Log::L_ERROR,"JmsMsgSender::init():error when retrieve keep alive time,set it to 0");
		lKeepAliveTime=0;
	}
	m_MsgOption.flags=PO_TIMETOLIVE;
	m_MsgOption.timeToLive=lKeepAliveTime;

	glog(Log::L_DEBUG,"JmsMsgSender::init():get jms config successfully");

	
	return true;
}
void JmsMsgSender::UnInitializeJMS()
{
	if(m_pJmsContext)
	{
		if(m_bConnectionOK)
			m_JmsConnection.stop();
		delete m_pJmsContext;
		m_pJmsContext=NULL;
	}
}
bool JmsMsgSender::InitializeJMS()
{
	if(m_pJmsContext)
	{//if the m_pJmsContext is initialized,destroy it
		delete m_pJmsContext;
		m_pJmsContext=NULL;
	}
	glog(Log::L_DEBUG,"BOOL JmsMsgSender::InitializeJMS():: before Create Context with Server Address =%s  and NamingContext=%s",m_strServerAddress.c_str(),m_strNamingContext.c_str());
	m_pJmsContext=new Context(m_strServerAddress.c_str(),m_strNamingContext.c_str());
	if(!m_pJmsContext )
	{
		glog(Log::L_ERROR,"BOOL JmsMsgSender::InitilizeJMS()::can't initialize JmsContext with server address is %s and server namingContext is %s and ErrCode is %d",m_strServerAddress.c_str(),m_strNamingContext.c_str(),getLastJmsError());
		return false;
	}
	glog(Log::L_DEBUG,"BOOL JmsMsgSender::InitializeJMS():: after Create Context with Server Address =%s  and NamingContext=%s",m_strServerAddress.c_str(),m_strNamingContext.c_str());

	if( !m_pJmsContext->_context)
	{
		if(m_pJmsContext)
		{
			delete m_pJmsContext;
			m_pJmsContext=NULL;
		}
		glog(Log::L_ERROR,"BOOL JmsMsgSender::InitilizeJMS()::can't initialize JmsContext with server address is %s and server namingContext is %s",m_strServerAddress.c_str(),m_strNamingContext.c_str());
		return false;
	}
	m_bJmsInitializeOK=true;
	glog(Log::L_DEBUG,"BOOL JmsMsgSender::InitializeJMS():: Create Context succesfully  with Server Address =%s  and NamingContext=%s",m_strServerAddress.c_str(),m_strNamingContext.c_str());

	return true;
}
bool JmsMsgSender::ConnectToServer()
{
	glog(Log::L_DEBUG,"BOOL JmsMsgSender::ConnetToHost()::Begin to connect to server");

	m_JmsTxtMessage.DestroyMessage();
	m_JmsProducer.close();
	m_JmsDestination.destroy();
	m_JmsSession.close();
	m_JmsConnection.close();
	m_JmsCNFactory.Destroy();

	//Create connection factory
	glog(Log::L_DEBUG,"ConnectToServer():create connectionFactory with string =%s",m_strConnectionFactory.c_str());
	if(!m_pJmsContext->createConnectionFactory(m_strConnectionFactory.c_str(),m_JmsCNFactory))
	{		
		glog(Log::L_ERROR,"BOOL JmsMsgSender::ConnetToHost()::can't create connnectionfactory with factory=%s And errCode=%d","ConnectionFactory",getLastJmsError());
		return false;
	}

	//create connection	
	if(!m_JmsCNFactory.createConnection(m_JmsConnection))
	{
		glog(Log::L_ERROR,"BOOL JmsMsgSender::ConnetToHost()::Can't create connection using JMSConnectionFactory and ErrCode=%d",getLastJmsError());
		return false;
	}
	
	m_JmsConnection.SetConnectionCallback(ConnectionMonitor,this);
	
	if(!m_JmsConnection.createSession(m_JmsSession))
	{
		glog(Log::L_ERROR,"BOOL JmsMsgSender::ConnetToHost()::Can't create session using JMSConnection and ErrCode=%d",getLastJmsError());
		return false;
	}

	glog(Log::L_DEBUG,"ConnectToServer():Create destination with string=%s",m_strDestinationName.c_str());
	if(!m_pJmsContext->createDestination(m_strDestinationName.c_str(),m_JmsDestination))
	{
		glog(Log::L_ERROR,"BOOL JmsMsgSender::ConnetToHost()::Can't create Destination with destinaition name=%s and ErrCode=%d",m_strDestinationName.c_str(),getLastJmsError());
		return false;
	}

	
	if(!m_JmsSession.createProducer(&m_JmsDestination,m_JmsProducer))
	{
		glog(Log::L_ERROR,"BOOL JmsMsgSender::ConnetToHost()::Can't create producer using JMS destination and ErrCode=%d",getLastJmsError());
		return false;
	}

	if(!m_JmsConnection.start())
	{
		glog(Log::L_ERROR,"BOOL JmsMsgSender::ConnetToHost()::JMSCOnnection start fail and errCode=%d",getLastJmsError());
		return false;
	}
	
	if(!m_JmsSession.textMessageCreate("",m_JmsTxtMessage))
	{
		glog(Log::L_ERROR,"BOOL JmsMsgSender::ConnetToHost()::Create text message fail and ErrCode=%d",getLastJmsError());
		return false;
	}
	if(!CreateMessageProperty())
	{
		glog(Log::L_ERROR,"BOOL JmsMsgSender::ConnetToHost()::## Can't create message property");
		return false;
	}
	//evrything is ok set connection status to TRUE
	SetConnectionStatus(true);

	glog(Log::L_DEBUG,"BOOL JmsMsgSender::ConnetToHost()::End to connect to server  and everything is OK");

	return true;
}
bool JmsMsgSender::CreateMessageProperty()
{
	if(NULL==m_JmsTxtMessage._message)
	{
		glog(Log::L_ERROR,"bool JmsMsgSender::CreateMessageProperty()## text message instance is not availble");
		return false;
	}
	InitInfo	ini;
	if(!ini.init(m_strIniFilePath.c_str()))
	{
		glog(Log::L_ERROR,"bool JmsMsgSender::CreateMessageProperty()## can't init InitInfo with file path =%s",m_strIniFilePath.c_str());
		return false;
	}
	
	ini.setCurrent("JmsTextMessageProperty");
	//get property count;
	int			iPropertyCount=0;
	if(!ini.getValue("PropertyCount",iPropertyCount))
	{
		glog(Log::L_ERROR,"bool JmsMsgSender::CreateMessageProperty()## can't get property count");
		return false;
	}	
	//adjust it to positive value
	iPropertyCount=iPropertyCount>0?iPropertyCount:0;
	glog(Log::L_DEBUG,"bool JmsMsgSender::CreateMessageProperty()## There are %d properties",iPropertyCount);

	for(int i=0;i<iPropertyCount;i++)
	{
		char	szChar[48];
		sprintf(szChar,"MsgProperty%d",i+1);
		ini.setCurrent(szChar);
		{
#define		VALUE_DESC		"value"	
			std::string		strKey;
			ini.getValue("key",strKey);
			//Get Type
			std::string		strType;
			ini.getValue("type",strType);

			if(0==stricmp("int",strType.c_str()))
			{	
				int				iValue;
				ini.getValue(VALUE_DESC,iValue);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%d",strKey.c_str(),iValue);
				m_JmsTxtMessage.setIntProperty((char*)strKey.c_str(),iValue);
			}
			else if(0==stricmp("long",strType.c_str()))
			{
				long			lValue=0;
				ini.getValue(VALUE_DESC,lValue);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%d",strKey.c_str(),lValue);
				m_JmsTxtMessage.setLongProperty((char*)strKey.c_str(),lValue);
			}
			else if(0==stricmp("double",strType.c_str()))
			{
				double			dValue=0;
				ini.getValue(VALUE_DESC,dValue);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%lf",strKey.c_str(),dValue);
				m_JmsTxtMessage.setDoubleProperty((char*)strKey.c_str(),dValue);
			}
			else if(0==stricmp("float",strType.c_str()))
			{
				float			fValue=0.0f;
				ini.getValue(VALUE_DESC,fValue);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%f",strKey.c_str(),fValue);
				m_JmsTxtMessage.setFloatProperty((char*)strKey.c_str(),fValue);
			}
			else if(0==stricmp("bool",strType.c_str()))
			{
				bool			bValue=false;
				ini.getValue(VALUE_DESC,bValue);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%d",strKey.c_str(),bValue);
				m_JmsTxtMessage.setBoolProperty((char*)strKey.c_str(),bValue);
			}
			else if(0==stricmp("byte",strType.c_str()))
			{
				unsigned char			byValue;
				ini.getValue(VALUE_DESC,&byValue,1);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%d",strKey.c_str(),byValue);
				m_JmsTxtMessage.setByteProperty((char*)strKey.c_str(),byValue);
			}
			else if(0==stricmp("short",strType.c_str()))
			{
				short			sValue=0;
				ini.getValue(VALUE_DESC,sValue);
				glog(Log::L_DEBUG,"Set Message Property with name=%s value=%d",strKey.c_str(),sValue);
				m_JmsTxtMessage.setShortProperty((char*)strKey.c_str(),sValue);
			}
			else if(0==stricmp("string",strType.c_str()))
			{
				std::string		strValue;
				ini.getValue(VALUE_DESC,strValue);
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
void JmsMsgSender::ConnectionMonitor(int errType,VOID* lpData)
{
	glog(Log::L_ERROR,"VOID JmsMsgSender::ConnectionMonitor()##Some exception was thowed out,Maybe network issue!");
	JmsMsgSender* pThis=(JmsMsgSender*)lpData;	
	pThis->SetConnectionStatus(false);	
}
void JmsMsgSender::OnMessage(int nMessageID, MessageFields* pMessage)
{	
	if(!m_bJmsInitializeOK)
	{
		if(!InitializeJMS())
		{
			MessageFields::iterator it;	
			for(it=pMessage->begin();it!=pMessage->end();it++)
			{
				if(!stricmp(it->key.c_str(), KD_KN_OUTPUT))
				{
					glog(Log::L_DEBUG,"Push Message tail because JMS initliaze fail::%s",it->value.c_str());
					PushNoneSendMessage(it->value,false);
				}
			}			
			glog(Log::L_CRIT,"JmsMsgSender::OnMessage()##Can't intialize JMS environment");
			return;
		}
	}
	if(!m_bConnectionOK)
	{
		glog(Log::L_DEBUG,"JmsMsgSender::OnMessage()##Connection is NOT available.reconnect to server");
		//re-connect to server
		int		i=0;
		while(i++<m_iReconnectCount && !m_bConnectionOK)
		{			
			if(ConnectToServer())
				break;
			Sleep(m_iReconnectInterval);
		}
	}
	
	if(!m_bConnectionOK)
	{
		MessageFields::iterator it;	
		for(it=pMessage->begin();it!=pMessage->end();it++)
		{
			if(!stricmp(it->key.c_str(), KD_KN_OUTPUT))
			{
				glog(Log::L_DEBUG,"Push message head because connection is NOT OK::%s",it->value.c_str());
				PushNoneSendMessage(it->value,false);
			}
		}
		//record current msg for future use		
		glog(Log::L_DEBUG,"void JmsMsgSender::OnMessage()## can't send message because connection is NOT OK.The message is record for future use");
		return ;
	}
	else
	{
		//check if there is any none send message
		while(HasNoneSendMessage())
		{
			std::string		strMsg=PopNoneSendMessage();
			if(!InternalSendMessage(strMsg))
			{
				glog(Log::L_DEBUG,"Push Message head after get none message and send fail::%s",strMsg.c_str());
				PushNoneSendMessage(strMsg,true);
				break;
			}			
		}
		MessageFields::iterator it;	
		for(it=pMessage->begin();it!=pMessage->end();it++)
		{
			if(!stricmp(it->key.c_str(), KD_KN_OUTPUT))
			{
				if(!m_bConnectionOK)
				{
					glog(Log::L_ERROR,"void JmsMsgSender::OnMessage()## connection is not available");
					glog(Log::L_DEBUG,"Push message tail when send message and connection is not OK::%s",it->value.c_str());
					PushNoneSendMessage(it->value,false);
					break;
				}
				if(!InternalSendMessage(it->value))
				{
					glog(Log::L_ERROR,"void JmsMsgSender::OnMessage()## can't send the message,So push it into none send message queue");
					glog(Log::L_DEBUG,"Push message tail because internal send fail::%s",it->value.c_str());
					PushNoneSendMessage(it->value,false);
					break;
				}		
			}
		}
	}
}
bool JmsMsgSender::HasNoneSendMessage()
{
	bool	bOK=false;
	if(m_vNoneSendMessageHeader.size()>0)
	{
		return true;
	}
	if(HasNoneSendMessageInFile())
	{
		//把文件中的内容读入head
		HANDLE hFile = CreateFileA(m_strMsgStoreFile.c_str(),
									GENERIC_READ,
									FILE_SHARE_READ,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL,
									NULL);
		if (INVALID_HANDLE_VALUE==hFile)
		{
			glog(Log::L_DEBUG, "bool JmsMsgSender::HasNoneSendMessage()##Fail to open file %s",m_strMsgStoreFile.c_str());			
		}
		else
		{			
#define		BUF_SIZE	4097
			char	szBuf[BUF_SIZE];			
			do 
			{
				ZeroMemory(szBuf,BUF_SIZE);
				unsigned long	dwReadSize=0;
				int		pos=0;
				if(!ReadFile(hFile,szBuf,BUF_SIZE-1,&dwReadSize,NULL))
					break;
				if(dwReadSize<=0)
					break;
				char*	pLine=NULL;
				int		iSkipped;
				char	*pbuf=szBuf;
				while ((iSkipped=GetLineLogContent(pbuf,&pLine))>0)
				{
					pbuf=pbuf+iSkipped;
					pos+=iSkipped;
					std::string		str=pLine;
					m_vNoneSendMessageHeader.push_back(str);
				}
				if(pos<dwReadSize-1)
					SetFilePointer(hFile,pos-dwReadSize-1,0,FILE_CURRENT);
				if(m_vNoneSendMessageHeader.size()>m_iNoneSendMsgFlushCount/2)	
					break;
			} while(1);
			
			long	crntPos=SetFilePointer(hFile,0,0,FILE_CURRENT);
			CloseHandle(hFile);
			if(!DeleteLogContent(crntPos,true))
			{
				glog(Log::L_ERROR,"bool JmsMsgSender::HasNoneSendMessage()##Delete read log fail");				
			}
			return true;
#undef		BUF_SIZE
		}
	}
	if(m_vNoneSendMessageTail.size()>0)
	{
		while(m_vNoneSendMessageTail.size()>0)
		{
			m_vNoneSendMessageHeader.push_front(m_vNoneSendMessageTail[m_vNoneSendMessageTail.size()-1]);
			m_vNoneSendMessageTail.pop_back();
		}
		return true;
	}
	return false;
}
int  JmsMsgSender::GetLineLogContent(char* buf, char** pline)
{
	if (buf == NULL)
		return 0;
	
	*pline = buf;
	bool bValidLine = false;
	
	while (**pline == '\r' || **pline =='\n')
		(*pline)++;
	
	char* q = *pline;
	while (*q != '\r' && *q!= '\n' && *q!='\0')
		q++;
	
	while (*q == '\r' || *q == '\n')
	{
		*q++ = '\0';
		bValidLine = true;
	}
	
	if (bValidLine)
		return (q - buf); // found a valid line
	
	// this is a in-completed line
	int stepped = *pline - buf;
	*pline = NULL;
	return stepped;
}
bool JmsMsgSender::HasNoneSendMessageInFile()
{
	HANDLE hFile = CreateFileA(m_strMsgStoreFile.c_str(),
								GENERIC_READ,
								FILE_SHARE_READ,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
	
	if (INVALID_HANDLE_VALUE==hFile)
	{
		glog(Log::L_ERROR, "bool JmsMsgSender::StoreNoneSendMessage():Fail to open file %s and errcode is %d",m_strMsgStoreFile.c_str(),GetLastError());
		return false;
	}
	else
	{
		long	pos=SetFilePointer(hFile,0,0,FILE_END);
		CloseHandle(hFile);
		if(pos>0)
			return true;
		else 
			return false;			 
	}	
}
std::string	JmsMsgSender::PopNoneSendMessage()
{
	if(!HasNoneSendMessage())
		return NULL;
	std::string		str=m_vNoneSendMessageHeader.front();
	m_vNoneSendMessageHeader.pop_front();
	return str;
}
void	JmsMsgSender::PushNoneSendMessage(std::string& strMsg,bool bHead)
{
	if(bHead)
		m_vNoneSendMessageHeader.push_front(strMsg);
	else
		m_vNoneSendMessageTail.push_back(strMsg);
	if(m_vNoneSendMessageHeader.size()>m_iNoneSendMsgFlushCount)
	{
		AddLogContent(m_vNoneSendMessageHeader,true);
	}
	if(m_vNoneSendMessageTail.size()>m_iNoneSendMsgFlushCount)
	{
		AddLogContent(m_vNoneSendMessageTail,false);
	}
}
bool JmsMsgSender::InternalSendMessage(std::string strMsg)
{
	if(m_JmsTxtMessage._message==NULL)
	{
		//should I set the m_bConnectionOK to false because the instance is not available????
		glog(Log::L_CRIT,"JmsMsgSender::InternalSendMessage():text message instance is not available");
		return false;
	}
	if(!m_JmsTxtMessage.setText((char*)strMsg.c_str()))
	{
		glog(Log::L_CRIT,"JmsMsgSender::InternalSendMessage():Can NOT set text for a jms message with text content =%s",strMsg.c_str());
		return false;
	}
	if(!m_bConnectionOK)
	{
		glog(Log::L_ERROR,"JmsMsgSender::InternalSendMessage()##connection lost.");
		return false;
	}	
	if(!m_JmsProducer.send(&m_JmsTxtMessage,&m_MsgOption))
	{
		glog(Log::L_ERROR,"JmsMsgSender::InternalSendMessage():can't send message");		
		return false;				
	}
	glog(Log::L_DEBUG,"Send the message OK::%s",strMsg.c_str());
	return true;
}
void JmsMsgSender::close()
{
	AddLogContent(m_vNoneSendMessageHeader,true);
	AddLogContent(m_vNoneSendMessageTail,false);
	UnInitializeJMS();
}

bool JmsMsgSender::AddLogContent(std::deque<std::string>& vContent,bool bFront)
{
	HANDLE hFile = CreateFileA(m_strMsgStoreFile.c_str(),
								GENERIC_READ|GENERIC_WRITE,
								0,
								NULL,
								OPEN_ALWAYS,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		glog(Log::L_DEBUG,"bool JmsMsgSender::AddLogContent()## can't open file %s",m_strMsgStoreFile.c_str());
		return false;
	}
	//Test	
	if(!bFront)	
	{//add to tail		
	
		SetFilePointer(hFile,0,0,FILE_END);
		unsigned long	lWrite=0;
		while (vContent.size()>0)
		{
			char	szTemp[2]="\r";			
			WriteFile(hFile,vContent.front().c_str(),vContent.front().size(),&lWrite,NULL);
			WriteFile(hFile,szTemp,strlen(szTemp),&lWrite,NULL);
			vContent.pop_front();
		}
		CloseHandle(hFile);
		return true;
	}
	else
	{
		std::string		strNewFile=m_strMsgStoreFile;
		strNewFile+='a';
		HANDLE	hNewFile=CreateFileA(strNewFile.c_str(),
										GENERIC_WRITE|GENERIC_READ,
										0,
										NULL,
										OPEN_ALWAYS,
										FILE_ATTRIBUTE_NORMAL,NULL);
		if(hNewFile==INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			glog(Log::L_ERROR,"bool JmsMsgSender::AddLogContent()## can't open temp file %s",strNewFile.c_str());
			return false;
		}
		SetFilePointer(hNewFile,0,0,FILE_BEGIN);
		SetEndOfFile(hNewFile);
		unsigned long lWrite=0;
		while (vContent.size()>0)
		{
			char	szTemp[2]="\r";
			WriteFile(hNewFile,vContent.front().c_str(),vContent.front().size(),&lWrite,NULL);
			WriteFile(hNewFile,szTemp,strlen(szTemp),&lWrite,NULL);
			vContent.pop_front();
		}
		long pos=SetFilePointer(hNewFile,0,0,FILE_CURRENT);
		if(!CopyFileContent(hFile,0,hNewFile,pos))
		{
			CloseHandle(hNewFile);
			CloseHandle(hFile);
			glog(Log::L_ERROR,"bool JmsMsgSender::AddLogContent()## copy file content fail");
			return false;
		}
		CloseHandle(hNewFile);
		CloseHandle(hFile);
		DeleteFileA(m_strMsgStoreFile.c_str());
		if(0!=rename(strNewFile.c_str(),m_strMsgStoreFile.c_str()))
		{
			glog(Log::L_ERROR,"bool JmsMsgSender::AddLogContent()## rename from %s to %s fail and errno=%d",strNewFile.c_str(),m_strMsgStoreFile.c_str(),errno);
			return false;
		}

		return true;
	}
}
bool JmsMsgSender::DeleteLogContent(long pos,bool bFront)
{
	HANDLE hFile = CreateFileA(m_strMsgStoreFile.c_str(),
								GENERIC_READ,
								0,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		glog(Log::L_DEBUG,"bool JmsMsgSender::DeleteLogContent()## can't open file %s",m_strMsgStoreFile.c_str());
		return false;
	}
	//create a temp file
	std::string	strNewFile=m_strMsgStoreFile;
	strNewFile+='a';
	HANDLE hNewFile = CreateFileA(strNewFile.c_str(),
									GENERIC_WRITE,
									FILE_SHARE_READ,
									NULL,
									OPEN_ALWAYS,
									FILE_ATTRIBUTE_NORMAL,
									NULL);
	if(hNewFile==INVALID_HANDLE_VALUE)
	{
		glog(Log::L_ERROR,"bool JmsMsgSender::DeleteLogContent()## can't open file %s",strNewFile.c_str());
		return false;
	}
	SetFilePointer(hNewFile,0,0,FILE_BEGIN);
	SetEndOfFile(hNewFile);
	if(bFront)
	{
		if(!CopyFileContent(hFile,pos,hNewFile,0))
		{
			CloseHandle(hFile);
			CloseHandle(hNewFile);

			glog(Log::L_ERROR,"bool JmsMsgSender::DeleteLogContent()## copy file content fail");
			return false;
		}
		CloseHandle(hFile);
		CloseHandle(hNewFile);

		DeleteFileA(m_strMsgStoreFile.c_str());
		if(0!=rename(strNewFile.c_str(),m_strMsgStoreFile.c_str()))
		{
			glog(Log::L_ERROR,"bool JmsMsgSender::DeleteLogContent()##rename from %s to %s fail and errno=%d",strNewFile.c_str(),m_strMsgStoreFile.c_str(),errno);
			return false;
		}
	}
	else
	{
		SetFilePointer(hFile,pos,0,FILE_BEGIN);
		SetEndOfFile(hFile);
		CloseHandle(hFile);
		CloseHandle(hNewFile);
	}
	return true;
}
bool JmsMsgSender::CopyFileContent(HANDLE hSrc,long posSrc,HANDLE hDst,long posDst)
{
	if(hSrc==INVALID_HANDLE_VALUE||hDst==INVALID_HANDLE_VALUE)
	{
		glog(Log::L_ERROR,"bool JmsMsgSender::CopyFileContent()## invalid handle pass in");
		return false;
	}
	SetFilePointer(hSrc,posSrc,0,FILE_BEGIN);
	SetFilePointer(hDst,posDst,0,FILE_BEGIN);

#define		BUF_SIZE	4097
	char	szBuf[BUF_SIZE];
	unsigned long	lRead=0;
	unsigned long	lWrite=0;
	do 
	{
		try
		{		
			if(!ReadFile(hSrc,szBuf,BUF_SIZE-1,&lRead,NULL))
			{
				glog(Log::L_ERROR,"bool JmsMsgSender::CopyFileContent()## read file content fail");
				return false;
			}
			if(!WriteFile(hDst,szBuf,lRead,&lWrite,NULL))
			{
				glog(Log::L_ERROR,"bool JmsMsgSender::CopyFileContent()## write file content fail");
				return false;
			}
		}
		catch (...)  
		{			
			glog(Log::L_DEBUG,"bool JmsMsgSender::CopyFileContent()## Exception was threw out and error code=%d",GetLastError());
		}
	} while(lRead==BUF_SIZE-1);
#undef		BUF_SIZE
	return true;

}