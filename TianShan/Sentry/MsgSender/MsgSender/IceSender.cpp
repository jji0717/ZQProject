
#include "IceSender.h"
#include "FileLog.h"
#include "SystemUtils.h"



namespace IceHelper{
	
class PublisherCache
{
public:
    PublisherCache(ZQ::common::Log& log, Ice::CommunicatorPtr ic, int timeout = 0)
        :_communicator(ic), _log(log), _timeout(timeout) 
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

class PublishCmd : public ZQ::common::ThreadRequest
{
public:
	PublishCmd(ZQ::common::NativeThreadPool& thpool, IceSender& sender, const MSGSTRUCT& msgStruct, const MessageIdentity& mid, void* ctx)
		:ThreadRequest(thpool), _iceSender(sender), _msgStruct(msgStruct), 
		_mid(mid), _ctx(ctx)
	{
	}
protected:
	int run()
	{
		while(!_iceSender._bQuit)
		{
		
			if(_iceSender.checkConnect())
			{
				if(_iceSender._bQuit)
					break;
				// step 1, publish the _msg to ICE storm
				bool ret = _iceSender.send(_msgStruct);
				if(!ret)
				{
					SYS::sleep(1000);
					continue;
				}

				// step 2, callback Sentry

				if(g_pIMsgSender)
					g_pIMsgSender->ack(_mid, _ctx);
				break;
			}
			else
			{
				if(_iceSender._bQuit)
					break;

				SYS::sleep(1000);
				continue;
			}

		}
		return 0;
	}

	void final(int retcode =0, bool bCancelled =false)
	{
		delete this;
	}

private:
	IceSender&	_iceSender;
	MSGSTRUCT		_msgStruct;
	MessageIdentity	_mid;
	void*			_ctx;
};

///////////////////////
//class IceSender
///////////////////////
IceSender::IceSender(int poolSize)
	:_thPool(poolSize), _ic(NULL), _pPubCache(NULL), 
	_bICECon(false), _nTimeOut(0), _bQuit(false)
{
	if(poolSize<MSGSENDER_POOLSIZE)
		_thPool.resize(MSGSENDER_POOLSIZE);
	_sysLog = new SysLog("MsgSender");

}

IceSender::~IceSender()
{
	if (NULL != _sysLog)
		delete _sysLog;
	_sysLog = NULL;
}

bool IceSender::init()
{
	int iii=0;
    _ic =  Ice::initialize(iii,NULL);
    if(!_ic)
    {
        LOG(Log::L_ERROR,"ICE communicatorptr initialize error");
        return false;
    }

    // init the publisher cache
    _pPubCache = new IceHelper::PublisherCache(LOG, _ic, _nTimeOut);
	if(!_pPubCache)
	{
		LOG(Log::L_ERROR,"ICE communicatorptr initialize error");
        return false;
	}

    try
    {
        _bICECon = ConnectICEStorm();
    }
    catch(...)
    {
        LOG(Log::L_ERROR,"ICE connect to server catch a exception");
    }
    

	return true;
}

void IceSender::AddMessage(const MSGSTRUCT& msgStruct, const MessageIdentity& mid, void* ctx)
{
	(new PublishCmd(_thPool, *this, msgStruct, mid, ctx))->start();
}

void IceSender::Close()
{
	_bQuit = true;
	SYS::sleep(1000);

	_thPool.stop();

	if(_pPubCache)
	{
		delete _pPubCache;
		_pPubCache = NULL;
	}

	try
    {			
        if(_ic)
		{
            _ic->destroy();
			_ic = 0;
		}
    }
    catch (...){}

}

bool IceSender::ConnectICEStorm()
{
    if(!_ic)
    {
        LOG(Log::L_ERROR,CLOGFMT(IceSender,"ConnectICEStorm() ICE initialize error"));
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
        LOG(Log::L_ERROR,CLOGFMT(IceSender,"ConnectICEStorm() Bad EventChannel endpoint [%s]"), _strManagerCfg.c_str());
        return false;
    }
}

bool IceSender::GetParFromFile(const char *pFileName)
{
    //get configure information from file
    if(pFileName == NULL || strlen(pFileName) == 0)
    {
//         if(plog != NULL)
//             LOG(Log::L_ERROR,"IceSender::GetParFromFile() configuration file path is NULL");
	    if(_sysLog != NULL)
		    (*_sysLog)(Log::L_ERROR,"IceSender::GetParFromFile() configuration file path is NULL");
          return false;
    }
    //load config item form xml config file	
    if(pEventSenderCfg == NULL)
    {
        pEventSenderCfg = new Config::Loader< EventSender >("");

        if(!pEventSenderCfg)
        {	
//             if(plog != NULL)
//                 LOG(Log::L_ERROR,"IceSender::GetParFromFile() Create PlugConfig object error");
		if(_sysLog != NULL)
			(*_sysLog)(Log::L_ERROR,"IceSender::GetParFromFile() Create PlugConfig object error");
            return false;
        }
        if(!pEventSenderCfg->load(pFileName))
        {
//             if(plog != NULL)
//                 LOG(Log::L_ERROR,"ICE not load config item from xml file:%s",pFileName);
		if(_sysLog != NULL)
			(*_sysLog)(Log::L_ERROR,"ICE not load config item from xml file:%s",pFileName);
            return false;	
        }
        pEventSenderCfg->snmpRegister("");
    }

    try
    {
        if(plog == NULL)
        {
			plog = new ZQ::common::FileLog(pEventSenderCfg->logPath.c_str(), pEventSenderCfg->logLevel, pEventSenderCfg->logNumber, pEventSenderCfg->logSize);
	  }
	  if (NULL != plog)
	  {
		  if (NULL != _sysLog)
			  delete _sysLog;
		  _sysLog = NULL;
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

	//config item
	_strManagerCfg = pEventSenderCfg->endPoint;
    _nTimeOut = pEventSenderCfg->timeout;

	return true;
}

bool IceSender::send(const MSGSTRUCT& msg)
{
    Ice::ObjectPrx pub;
    std::string topic = (msg.property.find("#topic") != msg.property.end()) ? msg.property.find("#topic")->second : "";
    if(topic.empty())
    { // send to the default topic
        pub = _pPubCache->getDefault();
        topic = TianShanIce::Events::TopicOfGenericEvent;
    } else if (topic == "#0") {
        // ignore this message
        LOG(Log::L_DEBUG, CLOGFMT(IceSender, "send() Ignore message to [%s]. category=%s, eventName=%s"), topic.c_str(), msg.category.c_str(), msg.eventName.c_str());
        return true;
    } else {
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
            _bICECon = false;
			return false;
        }
    }
    else
    {
        LOG(Log::L_ERROR, CLOGFMT(IceSender, "send() Can't access topic [%s]."), topic.c_str());
        _bICECon = false;
		return false;
    }

}

bool IceSender::checkConnect()
{
	if(_bICECon)
		return true;

	try
    {
		ZQ::common::MutexGuard MG(_lock);
		if(_bICECon)
			return true;
		if(_bQuit)
			return false;
	
        _bICECon = ConnectICEStorm();
    }
    catch(...)
    {
        LOG(Log::L_ERROR,CLOGFMT(IceSender, "checkConnect() ICE connect to server catch a exception"));
		_bICECon = false;
    }

	return _bICECon;
}
