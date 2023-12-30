// JmsMsgSender.cpp: implementation of the JmsMsgSender class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable : 4786)
#pragma warning(disable : 4018)
#include "JmsMsgSender.h"
#include "Log.h"


#define LOG_MODULE_NAME			"JmsMsgSender"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


using namespace ZQ::common;

JmsMsgSender::JmsMsgSender(::ZQ::JndiClient::ClientContext& context, const std::string& dstType, const std::string& destination):
JmsSession(context, (dstType.empty() ? (destination.find("queue") == 0 ? JmsSession::DT_Queue : JmsSession::DT_Topic) : (dstType.find("queue") == 0 ? JmsSession::DT_Queue : JmsSession::DT_Topic)), destination, true, false)

{
	m_iNoneSendMsgFlushCount=1000;
	// create message properties
	buildMessageProperties(_msgProperties);
	m_bConnectionOK = true;
}

JmsMsgSender::~JmsMsgSender()
{

}

bool JmsMsgSender::init(InitInfo& initInfo)
{
	m_strNamingContext = initInfo.strNamingCtx;
	m_iReconnectCount = initInfo.nReConnectCount;
	m_strDestinationName = initInfo.strDestName;		
	m_strConnectionFactory = initInfo.strConnFactory;
	m_strServerAddress = initInfo.strSrvIpPort;
	m_strMsgStoreFile = initInfo.strSafestoreFile;
	m_iReconnectInterval = initInfo.nReConnectInterval;
	m_iNoneSendMsgFlushCount = initInfo.nFlushToFileCount;

	//Get storage file path
	if(m_strMsgStoreFile.empty())
	{
		glog(Log::L_DEBUG,  "set SafestoreFile to default c:\\storageFile.txt");
		m_strMsgStoreFile="c:\\storageFile.txt";
	}
	
	//Get flush count
	if(!m_iNoneSendMsgFlushCount)
	{
		glog(Log::L_DEBUG,  "set NoneSendMsgFlushCount to default 100");
		m_iNoneSendMsgFlushCount=100;

	}
	
	return true;
}

// send all none-sent message, true for all none-sent message sent, false for stil exist none-sent message
bool JmsMsgSender::SendAllMsg()
{
	if(!m_bConnectionOK)
	{
		glog(Log::L_DEBUG,"JmsMsgSender::OnMessage()##Connection is NOT available, reconnect to server");
        return false;
	}

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

	return !(HasNoneSendMessage());
}

void JmsMsgSender::SendMsg(const std::string& strMsg)
{		
	if(!m_bConnectionOK)
	{
		glog(Log::L_DEBUG,"Push message head because connection is NOT OK::%s",strMsg.c_str());
		PushNoneSendMessage(strMsg,false);
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

		if(!m_bConnectionOK)
		{
			glog(Log::L_ERROR,"void JmsMsgSender::OnMessage()## connection is not available");
			glog(Log::L_DEBUG,"Push message tail when send message and connection is not OK::%s",strMsg.c_str());
			PushNoneSendMessage(strMsg,false);
			return;
		}
		if(!InternalSendMessage(strMsg))
		{
			glog(Log::L_ERROR,"void JmsMsgSender::OnMessage()## can't send the message,So push it into none send message queue");
			glog(Log::L_DEBUG,"Push message tail because internal send fail::%s",strMsg.c_str());
			PushNoneSendMessage(strMsg,false);
			return;
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
void	JmsMsgSender::PushNoneSendMessage(const std::string& strMsg,bool bHead)
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
   return sendTextMessage(strMsg, _msgProperties);
}
void JmsMsgSender::close()
{
	glog(Log::L_DEBUG,"void JmsMsgSender::close()##Server closed,reserve none send message for next use");
	AddLogContent(m_vNoneSendMessageHeader,true);
	AddLogContent(m_vNoneSendMessageTail,false);
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
void JmsMsgSender::buildMessageProperties(ZQ::JndiClient::ClientContext::Properties& props)
{
	props.clear();
	
	//propertys in string
	std::string strMsgPropery = "string,MESSAGECLASS,NOTIFICATION;int,MESSAGECODE,1101";
	::std::vector<std::string> strTypes, strKeys, strValues;
	int iPropertyCount = 0;
	{
		// parse the string to the vectors
		strTypes.push_back("string");
		strTypes.push_back("int");
		strKeys.push_back("MESSAGECLASS");
		strKeys.push_back("MESSAGECODE");
		strValues.push_back("NOTIFICATION");
		strValues.push_back("1101");
	}
	iPropertyCount = 2;

	glog(Log::L_DEBUG,"bool JmsMsgSender::CreateMessageProperty()## There are %d properties",iPropertyCount);

	for(int i=0;i<iPropertyCount;i++)
	{
		std::string		strKey = strKeys[i];
		std::string		strType = strTypes[i];

		if(0==stricmp("int",strType.c_str()))
		{	
			int				iValue = atoi(strValues[i].c_str());
			glog(Log::L_DEBUG,"Set Message Property with name=%s value=%d",strKey.c_str(),iValue);
			JmsSession::setIntegerProperty(props, (char*)strKey.c_str(),iValue);
		}
		else if(0==stricmp("string",strType.c_str()))
		{
			std::string 			strValue = strValues[i];
			glog(Log::L_DEBUG,"Set Message Property with name=%s value=%s",strKey.c_str(),strValue.c_str());
			JmsSession::setProperty(props,(char*)strKey.c_str(),strValue);
		}
	}
}
void JmsMsgSender::OnConnected(const std::string& notice)
{
	m_bConnectionOK = true;
}
void JmsMsgSender::OnConnectionLost(const std::string& notice)
{
	m_bConnectionOK = false;
}