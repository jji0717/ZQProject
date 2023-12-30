// IceSender.cpp: implementation of the IceSender class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "IceSender.h"
#include "FileLog.h"
#include "SystemUtils.h"
#include "FileSystemOp.h"

#ifdef ZQ_OS_MSWIN
#include <io.h>
#endif


/// the implement of IceHelper::PublisherCache

namespace IceHelper{
    class PublisherCache
    {
    public:
        PublisherCache(ZQ::common::Log& log, Ice::CommunicatorPtr ic, int timeout = 0)
            :_log(log), _communicator(ic), _timeout(timeout)
        {
        }

        ~PublisherCache()
        {
            try
            {
                _pubCache.clear();
                _topicMgr = NULL;
                _communicator = NULL;
            }
            catch(...){}
        }

        bool connect(const std::string& endpoint)
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(PublisherCache, "Connecting to EventChannel %s..."), endpoint.c_str());
            if(_communicator)
            {
                try
                {
                    Ice::ObjectPrx prx = _communicator->stringToProxy(std::string("TianShanEvents/TopicManager:") + endpoint);
                    if(prx)
                    {
                        ZQ::common::MutexGuard sync(_lock);
                        _topicMgr = IceStorm::TopicManagerPrx::checkedCast(prx);
                        _pubCache.clear(); // clear the cache
                        _defaultPub = NULL;
                        _defaultPub = get(TianShanIce::Events::TopicOfGenericEvent);
                        _log(ZQ::common::Log::L_INFO, CLOGFMT(PublisherCache, "Connect to EventChannel [%s] successfully."), endpoint.c_str());
                        return true;
                    }
                    else
                    {
                        _log(ZQ::common::Log::L_ERROR, CLOGFMT(PublisherCache, "Failed to connect to EventChannel [%s]. Bad endpoint."), endpoint.c_str());
                        return false;
                    }
                }
                catch(const Ice::Exception& e)
                {
                    _log(ZQ::common::Log::L_ERROR, CLOGFMT(PublisherCache, "Failed to connect to EventChannel [%s]. Exception [%s]."), endpoint.c_str(), e.ice_name().c_str());
                    return false;
                }
            }
            else
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(PublisherCache, "Failed to connect to EventChannel [%s]. Communicator not initialized."),  endpoint.c_str());
                return false;
            }
        }

        Ice::ObjectPrx get(const std::string& topicstr)
        {
            ZQ::common::MutexGuard sync(_lock);
            Publishers::iterator it = _pubCache.find(topicstr);
            if(it != _pubCache.end())
            {
                return it->second;
            }
    
            // retrieve from the topic manager
            IceStorm::TopicPrx topic;
            try
            {
                topic = _topicMgr->retrieve(topicstr);
            }
            catch(const IceStorm::NoSuchTopic &)
            {
                try
                {
                    // create the topic
                    topic = _topicMgr->create(topicstr);
                }catch(const IceStorm::TopicExists &)
                {
                    // someone may create the topic recently, retry and don't care the failure this time
                    topic = _topicMgr->retrieve(topicstr);
                }
            }
    
            // cache the publisher proxy
            Ice::ObjectPrx pubPrx;
            if(topic)
            {
                try
                {
                    pubPrx = topic->getPublisher();
                    if(_timeout > 0)
                        pubPrx = pubPrx->ice_timeout(_timeout);

                    _pubCache[topicstr] = pubPrx;
                    _log(ZQ::common::Log::L_DEBUG, CLOGFMT(PublisherCache,"Cache publisher [%s]."), topicstr.c_str());
                }
                catch(const Ice::Exception& e)
                {
                    _log(ZQ::common::Log::L_ERROR, CLOGFMT(PublisherCache,"Caught [%s] during cache publisher [%s]."), e.ice_name().c_str(), topicstr.c_str());
                }
                
            }
            return pubPrx;
        }
        Ice::ObjectPrx getDefault()
        {
            ZQ::common::MutexGuard sync(_lock);
            return _defaultPub;
        }
    private:
        Ice::CommunicatorPtr _communicator;
        IceStorm::TopicManagerPrx _topicMgr;
        typedef std::map<std::string, Ice::ObjectPrx> Publishers;
        Publishers _pubCache;
        Ice::ObjectPrx _defaultPub; // the default publisher : TianShanIce::Events::TopicOfGenericEvent
        ZQ::common::Log& _log;
        ZQ::common::Mutex _lock;

        int _timeout; // timeout of the message
    };
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::common;

//Log* plog = new ZQ::common::FileLog("EventSenderPlugIn.log",Log::L_DEBUG,5,1024*100);

#define MAX_BUFSIZE  8192  //8k

IceSender::IceSender():
_hExit(false),_hMsgSem(0),_dwPos(0),_bICECon(false),_ic(0),
_nTimeOut(0),_pPubCache(0),_nDequeSize(50),_hFile(0)
{
}

IceSender::~IceSender()
{
    Close();
}

bool IceSender::init()
{
    return start();
}

int IceSender::run()
{
    int iii=0;
    _ic =  Ice::initialize(iii,NULL);
    if(!_ic)
    {
        LOG(Log::L_ERROR,"ICE communicatorptr initialize error");
        return -1;
    }

    // init the publisher cache
    _pPubCache = new IceHelper::PublisherCache(LOG, _ic, _nTimeOut);
    try
    {
        _bICECon = ConnectICEStorm();
    }
    catch(...)
    {
        LOG(Log::L_ERROR,"ICE connect to server catch a exception");
    }
	
    bool bRead = false;

	
    while(true)
    {
        //if ice init fail, init until succeed
        if(!_bICECon)
        {	
            sleep(1000);
			_hMsgSem.post();	
            try
            {
                _bICECon = ConnectICEStorm();
            }
            catch(...)
            {
                LOG(Log::L_ERROR,"ICE connect to server catch a exception");
            }			
        }
        //wait for a message
        _hMsgSem.wait();
        if (_hExit) //exit
        {
            LOG(Log::L_DEBUG,"ICE wait a exit object");
            break;
        }

        //post from file first		
        if(_bICECon && bRead)
        {
            bRead = ReadEventFromFile();
        }
        while (_msgQue.size())
        {	
            if(int(_msgQue.size()) > _nDequeSize)
            {
                bRead = true;
                SaveEventToFile(_msgQue);
            }
			
            if(_bICECon && bRead)
            {
                bRead = ReadEventFromFile();
            }
            //use ice to post message
            else if(_bICECon && !bRead)
            {				
                bool bGetMsg = false;
                MSGSTRUCT msg;
                {			
                    ZQ::common::MutexGuard MG(_lock);
                    msg = _msgQue.front();
                    _msgQue.pop_front();
                    bGetMsg = true;
                }
                if(!bGetMsg)
                    continue;
                if(!send(msg))
                {
                    //post faile do what?post again or give up
                    _bICECon = false;
                    {
                        ZQ::common::MutexGuard MG(_lock);
                        _msgQue.push_front(msg);
                    }
                    break;
                }
            }
            else
                break;			
        }
    }
	
    //save the queue message
    try
    {
        if(_msgQue.size())
            SaveEventToFile(_msgQue,true);
    }
    catch(...){}

    delete _pPubCache;
    _pPubCache = NULL;
    return 0;
}

void IceSender::AddMessage(const MSGSTRUCT& msgStruct)
{
    {	
        ZQ::common::MutexGuard MG(_lock);
        _msgQue.push_back(msgStruct);	
    }
	_hMsgSem.post();
}

void IceSender::Close()
{
	_hExit = true;
	_hMsgSem.post();
		
	_msgQue.clear();
		
        //ice uninit
	try
	{			
		if(_ic)
			_ic->destroy();
			_ic = 0;
	}
	catch (...)
	{

	}
		
	if(_hFile)
	{
		fclose(_hFile);
		_hFile = 0;
	}
	remove(_strSaveName.c_str());
}

bool IceSender::ConnectICEStorm()
{
    if(!_ic)
    {
        LOG(Log::L_ERROR,"ICE initialize error");
        return false;
    }

    // get the endpoint from the proxy string
    std::string::size_type pos  = _strManagerCfg.find(":");
    std::string endpoint = (pos != std::string::npos) ? _strManagerCfg.substr(pos + 1) : _strManagerCfg;
    if(!endpoint.empty())
    {
        return _pPubCache->connect(endpoint);
    }
    else
    {
        LOG(Log::L_ERROR,"Bad EventChannel endpoint [%s]", _strManagerCfg.c_str());
        return false;
    }
}

void IceSender::SetCfgName(const char *pFileName)
{
    _strCfgName = pFileName;
}

bool IceSender::GetParFromFile(const char *pFileName)
{
    //get configure information from file
    if(pFileName == NULL || strlen(pFileName) == 0)
    {
        if(plog != NULL)
            LOG(Log::L_ERROR,"IceSender::GetParFromFile() configuration file path is NULL");
        return false;
    }
    //load config item form xml config file	
    if(pEventSenderCfg == NULL)
    {
        pEventSenderCfg = new Config::Loader< EventSender >("");

        if(!pEventSenderCfg)
        {	
            if(plog != NULL)
                LOG(Log::L_ERROR,"IceSender::GetParFromFile() Create PlugConfig object error");
            return false;
        }
        if(!pEventSenderCfg->load(pFileName))
        {
            if(plog != NULL)
                LOG(Log::L_ERROR,"ICE not load config item from xml file:%s",pFileName);
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
    catch(FileLogException& ex)
    {
#ifdef _DEBUG
        printf("IceSender::GetParFromFile() Catch a file log exception: %s\n",ex.getString());
#endif	
        return false;			
    }
    catch(...)
    {
        return false;
    }	

    _strManagerCfg = pEventSenderCfg->endPoint;
    _nTimeOut = pEventSenderCfg->timeout;
    _nDequeSize = pEventSenderCfg->iceDequeSize;
    if(_nDequeSize < 50 )
        _nDequeSize = 50;
    if(_nDequeSize >1000)
        _nDequeSize = 1000;

    _strSaveName = pEventSenderCfg->iceSavePath;
	size_t pos = _strSaveName.find_last_of(FNSEPS);
	if (std::string::npos != pos && pos > 3)
		FS::createDirectory(_strSaveName.substr(0, pos), true);
	
    //set file size zero if exist
	_hFile = fopen(_strSaveName.c_str(), "w+");
    if(!_hFile)
    {
        LOG(Log::L_ERROR,"CreateFile %s failed",_strSaveName.c_str());
        return false;
    }

    return true;
}

bool IceSender::SaveEventToFile(std::deque<MSGSTRUCT>& deq,bool bSaveAll)
{

    if(!_hFile)
    {
        LOG(Log::L_ERROR,"ICE save event to file error ,file handle is invalid");
        return false;
    }

	fseek(_hFile, 0, SEEK_END);

    int count = _nDequeSize>100 ? 100 : _nDequeSize;
    if(bSaveAll)
        count = deq.size();

    while(count--)
    {
        MSGSTRUCT msg;
        {
            ZQ::common::MutexGuard MG(_lock);
            msg = _msgQue.front();
            _msgQue.pop_front();
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
        size_t dwByte = fwrite(text.c_str(),1,text.size(),_hFile);
		if(!dwByte)
        {
            LOG(Log::L_ERROR,"ICE write file error code[%d]",SYS::getLastErr());
            return false;
        }
			
    }

    return true;
}

bool IceSender::ReadEventFromFile()
{
    if(!_hFile)  //failed
    {
        LOG(Log::L_ERROR,"Read event from file error ,ICE file handle is invalid");
        return false;
    }
	
	fseek(_hFile,0,SEEK_END);
	long dwS = ftell(_hFile);
    if(dwS == 0)
    {
        LOG(Log::L_DEBUG,"ICE read file exit ,file size is 0");
        return false;
    }
    if(dwS == _dwPos) //set file size zero
    {
#ifdef ZQ_OS_MSWIN		
		_chsize(fileno(_hFile), 0);
#else
		ftruncate(fileno(_hFile),0);
#endif
        _dwPos = 0;

        LOG(Log::L_DEBUG,"ICE set file length zero");
        return false;
    }
	
    char* buf = new char[sizeof(char)*MAX_BUFSIZE];
    memset(buf,0,sizeof(char)*MAX_BUFSIZE);

	fseek(_hFile,_dwPos,SEEK_SET);
	
    bool bHasOne = false;
    char* pBP = NULL;
    char* pBeg = NULL;
    char* pSec = NULL;
    char* pMM = NULL;
	size_t dwByte = fread(buf, 1, sizeof(char)*MAX_BUFSIZE-3, _hFile);
	if(dwByte > 0)
    {
        pMM = buf;
        pBP = buf;
        pBeg = buf;
        pSec = buf;		

        do{	
            MSGSTRUCT msg;
            bHasOne = false;
        
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

			if(*pMM == '\r')
			{
				pMM += 2;
				bHasOne = true;
			}
			else
			{
				while(*pMM != '\r' && *pMM != '\0')
				{
							
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
			}
            if(bHasOne) //post event
            {
                if(!send(msg))
                { // fail to send out message, is abort a good solution?
#pragma message(__MSGLOC__" fail to send out message, is abort a good solution?")
                    _bICECon = false;
                    break;
                }
                LOG(Log::L_DEBUG,"ICE send a message,eventid: %d",msg.id);
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
		ftruncate(fileno(_hFile),0);
#endif
        _dwPos = 0;
        LOG(Log::L_DEBUG,"ICE set file length zero");
    }

    delete[] buf;
    return true;
}


bool IceSender::send(const MSGSTRUCT& msg)
{
    Ice::ObjectPrx pub;
    std::string topic = (msg.property.find("#topic") != msg.property.end()) ? msg.property.find("#topic")->second : "";
    if(topic.empty())
    { // send to the default topic
        pub = _pPubCache->getDefault();
    }
    else
    {
        pub = _pPubCache->get(topic);
    }

    if(pub)
    {
        try
        {
            TianShanIce::Events::GenericEventSinkPrx::uncheckedCast(pub)->
                post(msg.category,(Ice::Int)msg.id,msg.eventName,msg.timestamp,msg.sourceNetId,msg.property);
            LOG(Log::L_DEBUG, CLOGFMT(IceSender, "send() Send message to [%s] successfully. category=%s, eventName=%s"), topic.c_str(), msg.category.c_str(), msg.eventName.c_str());
            return true;
        }
        catch(const Ice::Exception& ex)
        {
            LOG(Log::L_ERROR, CLOGFMT(IceSender, "send() Caught %s during sending the message to [%s]."), ex.ice_name().c_str(), topic.c_str());
            return false;
        }
    }
    else
    {
        LOG(Log::L_ERROR, CLOGFMT(IceSender, "send() Can't access topic [%s]."), topic.c_str());
        return false;
    }

}
