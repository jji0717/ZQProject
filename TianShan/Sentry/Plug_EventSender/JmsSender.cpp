// JmsSender.cpp: implementation of the JmsSender class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include <FileLog.h>
#include "JmsSender.h"
#include <FileSystemOp.h>
#ifdef ZQ_OS_MSWIN
#include <io.h>
#endif

#ifdef ZQ_OS_MSWIN
#   define REL_JVM_JDK "jre\\bin\\client\\jvm.dll" // windows jvm rel_path for jdk installation
#   define REL_JVM_JRE "bin\\client\\jvm.dll" // windows jvm rel_path for jre installation
#	define REL_JVM_CLIENT "client\\jvm.dll"
#else
#   ifdef __x86_64
#       define __a "amd64/server"
#   else
#       define __a "i386/client"
#   endif

#   define REL_JVM_JDK "/jre/lib/"__a"/libjvm.so" // linux jvm rel_path 
#   define REL_JVM_JRE "/lib/"__a"/libjvm.so" // linux jvm rel_path
#	define REL_JVM_CLIENT __a"/libjvm.so"
#endif

using namespace ZQ::common;

#define MAX_BUFSIZE  8192  //8k

JmsSender::JmsSender():
_dwPos(0),_pJmsContext(0),_bJmsInitializeOK(false),_bConnectionOK(false),
_iNoneSendMsgFlushCount(1000),_nDequeSize (50),_hFile(0),_pJmsSenderSession(0),_bQuit(false)
{
}

JmsSender::~JmsSender()
{
	Close();
}

bool JmsSender::init()
{
	return start();
}

void JmsSender::onConnected()
{
	_bConnectionOK = true;
	LOG(ZQ::common::Log::L_INFO,CLOGFMT(JmsSender,"onConnected() connection to jboss is good"));
	mSemMsg.signal();
}

void JmsSender::onDisconnected()
{
	_bConnectionOK = false;
	LOG(ZQ::common::Log::L_INFO,CLOGFMT(JmsSender,"onDisconnected() no connection to jboss"));
}

void JmsSender::AddMessage(const MSGSTRUCT& msgStruct)
{
	{
		ZQ::common::MutexGuard MG(_lock);
		mMsgQue.push_back(msgStruct);
	}
	mSemMsg.signal();	
}

int JmsSender::run()
{
	if(!InitializeJMS())
	{
		LOG(Log::L_ERROR,"JMS initialize failed,[InitializeJMS] function return false");
		return false;
	}

	bool bRead = true;
	while(!_bQuit)
	{
		mSemMsg.wait();

		if(_bQuit) break;

		if( bRead)
		{//if file has record to send ,send it first
			bRead = ReadEventFromFile();
		}
		while ( mMsgQue.size() )
		{
			if( int( mMsgQue.size()) > _nDequeSize )
			{
				bRead = true;
				SaveEventToFile(mMsgQue);
			}			
			else if( _bConnectionOK && !bRead )
			{
				bool bGetMsg = false;
				MSGSTRUCT msg;
				{			
					ZQ::common::MutexGuard MG(_lock);
					if( mMsgQue.size() <= 0)
						break;

					msg = mMsgQue.front();
					mMsgQue.pop_front();
					bGetMsg = true;
				}
				if(!bGetMsg)
					continue;

				if(!InternalSendMessage(msg))
				{
					onDisconnected();					
					{
						ZQ::common::MutexGuard MG(_lock);
						mMsgQue.push_front(msg);
					}
					break;
				}
			}
			else
			{
				break;
			}
		}
	}
	return 0;
}

bool JmsSender::CreateMessageProperty( ::ZQ::JndiClient::JmsSession::MapMessage& mapMsg)
{	
	//set property
	EventSender::JmsMsgProps::const_iterator it ;
	for(it = pEventSenderCfg->jmsmsgprops.begin(); it != pEventSenderCfg->jmsmsgprops.end(); it++)
	{
		const std::string& strType = it->type;
		const std::string& strKey = it->key;
		const std::string& strValue = it->value;
		
		if(strType.length()==0 || strKey.length()==0 || strValue.length()==0)
		{
			LOG(Log::L_ERROR,"Set jms message property wrong, not get right property");
			return false;
		}
		if( stricmp( strType.c_str() , "string" ) == 0 )
		{
			::ZQ::JndiClient::JmsSession::setProperty( mapMsg , strKey , strValue );
		}
		else if( stricmp( strType.c_str() , "int" ) == 0 )
		{
			::ZQ::JndiClient::JmsSession::setIntegerProperty( mapMsg , strKey.c_str() ,strValue.empty() ? 0 : atoi(strValue.c_str()));
		}
		else
		{
			LOG(Log::L_ERROR,"JmsSender::CreateMessageProperty() Not know the type %s",strType.c_str());
			//return false;
		}

// 		if(stricmp(strType.c_str(),"string") == 0)
// 			mapMsg.setStringProperty((char*)strKey.c_str(),(char*)strValue.c_str());
// 		else if(stricmp(strType.c_str(),"int") == 0)
// 			mapMsg.setIntProperty((char*)strKey.c_str(),atoi(strValue.c_str()));
// 		else if(stricmp(strType.c_str(),"short") == 0)
// 			mapMsg.setShortProperty((char*)strKey.c_str(),(short)atoi(strValue.c_str()));
// 		else if(stricmp(strType.c_str(),"long") == 0)
// 			mapMsg.setLongProperty((char*)strKey.c_str(),atol(strValue.c_str()));
// 		else if(stricmp(strType.c_str(),"float") == 0)
// 			mapMsg.setFloatProperty((char*)strKey.c_str(),(float)atof(strValue.c_str()));
// 		else if(stricmp(strType.c_str(),"double") == 0)
// 			mapMsg.setDoubleProperty((char*)strKey.c_str(),atof(strValue.c_str()));
// 		else if(stricmp(strType.c_str(),"byte") == 0)
// 			mapMsg.setByteProperty((char*)strKey.c_str(),strValue[0]);
// 		else if(stricmp(strType.c_str(),"bool") == 0)
// 			mapMsg.setBoolProperty((char*)strKey.c_str(),(bool)strValue.c_str());
// 		else
// 		{
// 			LOG(Log::L_ERROR,"JmsSender::CreateMessageProperty() Not know the type %s",strType.c_str());
// 			return false;
// 		}
	}
	
	return true;
}

bool JmsSender::GetParFromFile(const char* pFileName)
{
	//get configure information from file
	if(pFileName == NULL || strlen(pFileName) == 0)
	{
		if(plog != NULL)
			LOG(Log::L_ERROR,"JmsSender::GetParFromFile Configuration file path is NULL");
		return false;
	}
	//load config item from xml config file

	if(pEventSenderCfg == NULL)
	{
		pEventSenderCfg = new Config::Loader< EventSender >("");

		if(!pEventSenderCfg)
		{	
			if(plog != NULL)
				LOG(Log::L_ERROR,"JmsSender::GetParFromFile() Create PlugConfig object error");
			return false;
		}
		if(!pEventSenderCfg->load(pFileName))
		{
			if(plog != NULL)
				LOG(Log::L_ERROR,"JMS not load config item from xml file:%s",pFileName);
			return false;	
		}
		pEventSenderCfg->snmpRegister("");
	}

	try
	{
		if(plog == NULL)
		{
			plog = new ZQ::common::FileLog(pEventSenderCfg->logPath.c_str(),pEventSenderCfg->logLevel,5,pEventSenderCfg->logSize);
		}
	}
	catch(FileLogException& )
	{
		return false;			
	}
	catch(...)
	{
		return false;
	}

//	_MsgOption.flags = PO_TIMETOLIVE;
//	_MsgOption.timeToLive = pEventSenderCfg->timeToLive;
	_msgTTL	= pEventSenderCfg->timeToLive;
	_strNamingContext = pEventSenderCfg->context;
	_strServerAddress = pEventSenderCfg->ipPort;
	_strDestinationName = pEventSenderCfg->destinationName;
	_strConnectionFactory = pEventSenderCfg->connectionFactory;
	_nDequeSize = pEventSenderCfg->jmsDequeSize;
	if( _nDequeSize < 50 )
		_nDequeSize = 50;
	if( _nDequeSize >2000 )
		_nDequeSize = 2000;
	
	_strSaveName = pEventSenderCfg->jmsSavePath;

	if( !CreateMessageProperty(mMessageProperties))
	{
		return false;
	}

	_hFile = fopen(_strSaveName.c_str(), "w+");
	if(!_hFile)
	{
		LOG(Log::L_ERROR,"JMS create file %s failed",_strSaveName.c_str());
		return false;
	}

	return true;
}

void JmsSender::Close()
{
	if(!_bQuit)
	{
		_bQuit = true;
		mSemMsg.signal();

		mMsgQue.clear();

		UnInitializeJMS();
		if(_hFile)
		{
			fclose(_hFile);
			_hFile = 0;
		}
		remove(_strSaveName.c_str());
	}	
}
static std::string pathCat(const std::string &dir, const std::string& sub)
{
	std::string path;
	path.reserve(dir.size() + 1 + sub.size());
	if(!dir.empty() && dir[dir.size() - 1] != FNSEPC)
	{
		path = dir + FNSEPS;
	}
	else
	{
		path = dir;
	}

	path += sub;

	return path;
}
bool JmsSender::InitializeJMS()
{
	if(_pJmsContext)
	{//if the m_pJmsContext is initialized,destroy it
		delete _pJmsContext;
		_pJmsContext=NULL;
	}
	if( _pJmsSenderSession )
	{
		delete _pJmsSenderSession;
		_pJmsSenderSession = NULL;
	}

	// get the jvm path
	std::string jvmPath;
	{
		const char* envJavaHome = getenv("JAVA_HOME");
		if(envJavaHome) 
		{
			jvmPath = pathCat(envJavaHome, REL_JVM_JDK);
			if(!FS::FileAttributes(jvmPath).exists()) 
			{
				jvmPath = pathCat(envJavaHome, REL_JVM_JRE);
				if(!FS::FileAttributes(jvmPath).exists()) 
				{
					jvmPath = pathCat(envJavaHome,REL_JVM_CLIENT);
					if(!FS::FileAttributes(jvmPath).exists()) 
					{
						LOG(ZQ::common::Log::L_ERROR, CLOGFMT(InitializeJMS, "Bad java environment settings: can't get the JVM path.need sys env 'JAVA_HOME'"));
						return false;
					}
				}
			}
		} 
		else
		{ // bad setting!
			LOG(ZQ::common::Log::L_ERROR, CLOGFMT(InitializeJMS, "Bad java environment settings: can't get the JVM path.need sys env 'JAVA_HOME'"));
			return false;
		}
	}
	
	LOG(ZQ::common::Log::L_INFO,CLOGFMT(JmsSender,"InitializeJMS() init jvm[%s] with classpath[%s]"),jvmPath.c_str() , getenv("CLASSPATH") );
	if( !ZQ::JndiClient::ClientContext::initJVM( *plog, getenv("CLASSPATH"), jvmPath.c_str()) )
	{
		LOG(ZQ::common::Log::L_ERROR,CLOGFMT(JmsSender,"InitializeJMS() failed to init jvm[%s] classpath[%s]"),
			jvmPath.c_str() , getenv("CLASSPATH") );
	}
	try
	{
		_pJmsContext = new ZQ::JndiClient::ClientContext(_strServerAddress.c_str());
	}
	catch( ZQ::JndiClient::JndiException& ex)
	{
		LOG(ZQ::common::Log::L_ERROR,CLOGFMT(JmsSender,"InitializeJMS() caught exception while creating JndiClient context: %s"),ex.what() );
		return false;
	}
	
	LOG( Log::L_INFO, CLOGFMT(JmsSender,"InitializeJMS() Create context succesfully  with Server Address =%s  and NamingContext=%s"),_strServerAddress.c_str(),_strNamingContext.c_str());

	_pJmsSenderSession = new SenderSession(*this,*_pJmsContext,ZQ::JndiClient::JmsSession::DT_Queue,_strDestinationName);

	_pJmsSenderSession->setProducerOptions( 5, _msgTTL, ::ZQ::JndiClient::JmsSession::DM_Persisent); 

	LOG(ZQ::common::Log::L_INFO,CLOGFMT(JmsSender,"InitializeJMS() create session : dest[%s] ttl[%d]"),
		_strDestinationName.c_str() , _msgTTL);

#ifdef ZQ_OS_MSWIN
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
#endif

	return true;
}

void JmsSender::UnInitializeJMS()
{
	if( _pJmsSenderSession)
	{
		delete _pJmsSenderSession;
		_pJmsSenderSession = NULL;
	}

	if(_pJmsContext)
	{		
		delete _pJmsContext;
		_pJmsContext=NULL;
	}
	ZQ::JndiClient::ClientContext::uninitJVM();
}

bool JmsSender::InternalSendMessage(const MSGSTRUCT& msgStruct)
{
	try
	{
	ZQ::JndiClient::JmsSession::MapMessage mapMsg;
	
	char szEventId[10] = {0};
	sprintf(szEventId,"%04d",msgStruct.id);
	ZQ::JndiClient::JmsSession::setProperty( mapMsg , "eventId" , szEventId );	

	ZQ::JndiClient::JmsSession::setProperty( mapMsg , "category" , msgStruct.category.c_str() );
	
	ZQ::JndiClient::JmsSession::setProperty( mapMsg , "stampUTC" , msgStruct.timestamp.c_str() );
	
	ZQ::JndiClient::JmsSession::setProperty( mapMsg , "eventName" , msgStruct.eventName.c_str() );
	
	ZQ::JndiClient::JmsSession::setProperty( mapMsg , "sourceNetId" , msgStruct.sourceNetId.c_str() );
		
	for( std::map<std::string,std::string>::const_iterator it=msgStruct.property.begin(); it!=msgStruct.property.end(); it++)
	{
		ZQ::JndiClient::JmsSession::setProperty( mapMsg , it->first,it->second );		
	}	

	int64 dwStart = SYS::getTickCount();
	if( !_pJmsSenderSession->sendMapMessage( mapMsg , mMessageProperties ) )
	{
		LOG(Log::L_INFO,"JmsSender::InternalSendMessage() Not sendout this message");	
		return false;				
	}
#ifdef _DEBUG
	static int g_testCount =0;
	printf("Sendout jms message %d\n",++g_testCount);
#endif
	
	char	szBuf[1024];
	std::map<std::string,std::string>::const_iterator itMap = msgStruct.property.begin();
	int iBufSize =sizeof(szBuf)-1;
	int iCurSize=0;
	memset(szBuf,0,sizeof(szBuf));
	//msgStruct	
	for( ; itMap!= msgStruct.property.end();itMap++ )
	{
		int iTemp = snprintf(szBuf+iCurSize,iBufSize,"[%s]=[%s] ",itMap->first.c_str(),itMap->second.c_str());
		iBufSize-= iTemp;
		iCurSize+= iTemp;
	}
	LOG(Log::L_DEBUG,"Send a message using time[%04lld]:\t eventId[%d] category[%s] stampUTC[%s] eventName[%s] sourceNetId[%s] Property: %s",
		SYS::getTickCount()-dwStart, msgStruct.id,msgStruct.category.c_str(),msgStruct.timestamp.c_str(),msgStruct.eventName.c_str(),msgStruct.sourceNetId.c_str(),
		szBuf);	
	}
	catch(...)
	{
		LOG(Log::L_ERROR,"JMS send message catch an exception");
		return false;
	}
	return true;
}

bool JmsSender::SaveEventToFile(std::deque<MSGSTRUCT>& deq)
{
	if(!_hFile)
	{
		LOG(Log::L_ERROR,"Save event to file error ,JMS file handle is invalid");
		return false;
	}

	fseek(_hFile, 0, SEEK_END);
	int count = _nDequeSize>100 ? 100 : _nDequeSize;
	
	while(count--)
	{
		MSGSTRUCT msg;
		{
			ZQ::common::MutexGuard MG(_lock);
			msg = mMsgQue.front();
			mMsgQue.pop_front();
		}

		char a[10] = {0};
		sprintf(a,"%d",msg.id);
		std::string text = "";
		text = a;				//id
		text += "\n";
		text += msg.category;	//category
		text += "\n";
		text += msg.timestamp;	//timestamp
		text += "\n";
		text += msg.eventName;  //eventName
		text += "\n";
		text += msg.sourceNetId;//sourceNetId
		text += "\n";

		for(std::map<std::string,std::string>::iterator itmap=msg.property.begin(); itmap!=msg.property.end(); itmap++)
		{
			text += itmap->first + "\n";
			text += itmap->second + "\n";
		}
		text += "\r\n";
		size_t dwByte = fwrite(text.c_str(), 1, text.size(), _hFile);
		if(!dwByte)
		{
			LOG(Log::L_ERROR,"JMS write file error");
			return false;
		}
			
	}
	
	return true;
}

bool JmsSender::ReadEventFromFile()
{
	if(!_hFile)  //failed
	{
		LOG(Log::L_ERROR,"Read event from file error ,JMS file handle is invalid");
		return false;
	}
	
	fseek(_hFile, 0, SEEK_END);
	long dwS = ftell(_hFile);
	if(dwS == 0)
	{
		LOG(Log::L_DEBUG,"JMS read file exit ,file size is 0");
		return false;
	}
	if(dwS == _dwPos) //set file size zero
	{
#ifdef ZQ_OS_MSWIN
		_chsize(fileno(_hFile), 0);
#else
		ftruncate(fileno(_hFile), 0);
#endif
		_dwPos = 0;
		
		LOG(Log::L_DEBUG,"JMS set file length zero");
		return false;
	}
	
	char* buf = new char[sizeof(char)*MAX_BUFSIZE];
	memset(buf,0,sizeof(char)*MAX_BUFSIZE);

	fseek(_hFile, 0, SEEK_SET);
	
	bool bHasOne = false;
	char* pBP = NULL;
	char* pBeg = NULL;
	char* pSec = NULL;
	char* pMM = NULL;
	size_t dwByte = fread(buf, 1, sizeof(char)*MAX_BUFSIZE-1, _hFile);
	if(dwByte > 0)
	{
		pMM = buf;
		pBP = buf;
		pBeg = buf;
		pSec = buf;		

		do{	
			MSGSTRUCT msg;
			bHasOne = false;
			pMM++;
			while(*pMM != '\n' && *pMM != '\0')
			{
				pMM++;
			}
			if(*pMM == '\n')  //id
			{
				*pMM = '\0';
				msg.id = atoi(pBeg);
			}
			else
				break;
			
			pMM++;
			pBeg = pMM;
			while(*pMM != '\n' && *pMM != '\0')
			{
				pMM++;
			}
			if(*pMM == '\n')  //category
			{
				*pMM = '\0';
				msg.category = pBeg;
			}
			else
				break;
			
			pMM++;
			pBeg = pMM;
			while(*pMM != '\n' && *pMM != '\0')
			{
				pMM++;
			}
			if(*pMM == '\n')  //timestamp
			{
				*pMM = '\0';
				msg.timestamp = pBeg;
			}
			else
				break;

			pMM++;
			pBeg = pMM;
			while(*pMM != '\n' && *pMM != '\0')
			{
				pMM++;
			}
			if(*pMM == '\n')  //eventName
			{
				*pMM = '\0';
				msg.eventName = pBeg;
			}
			else
				break;

			pMM++;
			pBeg = pMM;
			while(*pMM != '\n' && *pMM != '\0')
			{
				pMM++;
			}
			if(*pMM == '\n')  //sourceNetId
			{
				*pMM = '\0';
				msg.sourceNetId = pBeg;
			}
			else
				break;


			pMM++;
			pBeg = pMM;
			while(*pMM != '\r' && *pMM != '\0')
			{
				pMM++;				
				while(*pMM != '\n' && *pMM != '\0')
				{
					pMM++;
				}
				if(*pMM == '\n') //first
				{
					*pMM = '\0';
				}
				else
					break;

				pMM++;
				pSec = pMM;
				while(*pMM != '\r' && *pMM != '\n' && *pMM != '\0')
				{
					pMM++;
				}
				if(*pMM == '\n' && *(pMM+1) == '\r')  //second
				{
					bHasOne = true;
					*pMM = '\0';
					msg.property[pBeg] = pSec;
					pMM += 3;
				}
				if(*pMM == '\n' && *(pMM+1) != '\r')  
				{
					*pMM = '\0';
					msg.property[pBeg] = pSec;
					++pMM;
				}
				else
					break;

				pBeg = pMM;
				if(bHasOne)
					break;
			}
			if(bHasOne) //send event
			{
				if(!InternalSendMessage(msg))
				{
					//SetConnectionStatus(false);
					onDisconnected();//connection is broken
					break;
				}
									
				_dwPos += pMM-pBP;
				pBP = pMM;
				pBeg = pMM;
			}
			
		}while(*pMM != '\0');
	}
	if(dwS == _dwPos) //read end of the file
	{
#ifdef ZQ_OS_MSWIN
		_chsize(fileno(_hFile), 0);
#else
		ftruncate(fileno(_hFile), 0);
#endif
		_dwPos = 0;
		LOG(Log::L_DEBUG,"JMS set file length zero");
	}

	delete[] buf;
	return true;
}

#ifdef ZQ_OS_MSWIN
BOOL WINAPI JmsSender::HandlerRoutine(DWORD dwCtrlType)
{
	switch( dwCtrlType ) 
	{
	case CTRL_LOGOFF_EVENT: // for LOGOFF event , do nothing but return true to notify system do not process this event any longer
		return TRUE;
		break;
	default://we only care about LOGOFF event, so for other event just ignore and return false
		return FALSE;
		break;
	}
}
#endif


//////////////////////////////////////////////////////////////////////////
void SenderSession::OnConnected(const std::string& notice)
{
	mConnectionStatusReceiver.onConnected();
}

void SenderSession::OnConnectionLost(const std::string& notice)
{
	mConnectionStatusReceiver.onDisconnected();
}
