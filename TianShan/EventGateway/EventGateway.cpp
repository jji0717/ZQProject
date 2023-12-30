#include "EventGateway.h"
#include "Sentinel.h"
#include "SystemUtils.h"
#include "TsStreamer.h"
namespace EventGateway{
    
// constructor
EventGw::EventGw(
                 Ice::CommunicatorPtr ic,
                 Ice::ObjectAdapterPtr adptr,
                 ZQ::common::Log &log,
                 const std::string &iceStormProxy,
                 const std::string &pluginConfigDir,
                 const std::string &pluginLogDir,
                 size_t connCheckIntervalMSec
                 )
                 :_communicator(ic),_adapter(adptr),_log(log)
{
    _iceStormProxy = iceStormProxy;
    _pluginConfigDir = pluginConfigDir;
    _pluginLogDir = pluginLogDir;

    _connCheckIntervalMSec = connCheckIntervalMSec;
    if(_connCheckIntervalMSec < EventGw_ConnCheck_IntervalMSec_Min)
        _connCheckIntervalMSec = EventGw_ConnCheck_IntervalMSec_Min;
    else if (EventGw_ConnCheck_IntervalMSec_Max < _connCheckIntervalMSec)
        _connCheckIntervalMSec = EventGw_ConnCheck_IntervalMSec_Max;

    _quit = false;
    notifyBadConnection(); // force connecting the IceStorm
    start();
}

// destructor
EventGw::~EventGw()
{
    _quit = true;
    if(isRunning())
    {
        notifyBadConnection(); // signal to quit maintenance thread
        
        _log(ZQ::common::Log::L_INFO, CLOGFMT(EventGw, "~EventGw() The connection maintenance thread is quiting..."));
        waitHandle(-1);
        _log(ZQ::common::Log::L_INFO, CLOGFMT(EventGw, "~EventGw() The connection maintenance thread quit"));
    }
    else
    {
        _log(ZQ::common::Log::L_INFO, CLOGFMT(EventGw, "~EventGw() The connection maintenance thread is not running"));
    }
}

static bool QoSEqual(const TianShanIce::Properties &left, const TianShanIce::Properties &right)
{
    return (left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin()));
}

bool EventGw::subscribe(
                        IGenericEventHelper *pHelper,
                        const std::string &topic,
                        const Properties &qos
                        )
{
    if(NULL == pHelper || topic.empty())
        return false;
    
	if(topic == TianShanIce::Streamer::TopicOfStream)
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EventGw,
			"subscribe topic [%s] with stream event helper."), topic.c_str());
		ZQ::common::MutexGuard sync(_lockstreamSinks);
		StreamEventSinkImpl::Ptr sink;
		// search in the current sink objects
		for(StreamSinks::iterator it = _streamSinks.begin(); it != _streamSinks.end(); ++it)
		{
			if(topic == it->topic && QoSEqual(qos, it->qos))
			{
				sink = it->obj;
				break;
			}
		}
		// create if not found
		if(!sink)
		{
			sink = new StreamEventSinkImpl(_log);
			Ice::ObjectPrx prx = _adapter->addWithUUID(sink);
			try
			{
				IceStorm::TopicPrx topicPrx = getTopic(topic, true);
			
				#if  ICE_INT_VERSION / 100 >= 306
					topicPrx->subscribeAndGetPublisher(qos, prx);
				#else
					topicPrx->subscribe(qos, prx);
				#endif
			}
			catch(...)
			{
				// notify bad connection
				notifyBadConnection();
			}
			StreamSinkInfo info;
			info.obj = sink;
			info.prx = prx;
			info.topic = topic;
			info.qos = qos;
			_streamSinks.push_back(info);
		}
		// register the helper
		sink->add(pHelper);
	}
	else if(topic == TianShanIce::Streamer::TopicOfPlaylist)
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EventGw,
			"subscribe topic [%s] with playlist event helper."), topic.c_str());
		ZQ::common::MutexGuard sync(_lockplaylistSinks);
		PlaylistEventSinkImpl::Ptr sink;
		// search in the current sink objects
		for(PlayListSinks::iterator it = _playlistSinks.begin(); it != _playlistSinks.end(); ++it)
		{
			if(topic == it->topic && QoSEqual(qos, it->qos))
			{
				sink = it->obj;
				break;
			}
		}
		// create if not found
		if(!sink)
		{
			sink = new PlaylistEventSinkImpl(_log);
			Ice::ObjectPrx prx = _adapter->addWithUUID(sink);
			try
			{
				IceStorm::TopicPrx topicPrx = getTopic(topic, true);
				
				#if  ICE_INT_VERSION / 100 >= 306
					topicPrx->subscribeAndGetPublisher(qos, prx);
				#else
					topicPrx->subscribe(qos, prx);
				#endif
			}
			catch(...)
			{
				// notify bad connection
				notifyBadConnection();
			}
			PlayListSinkInfo info;
			info.obj = sink;
			info.prx = prx;
			info.topic = topic;
			info.qos = qos;
			_playlistSinks.push_back(info);
		}
		// register the helper
		sink->add(pHelper);
	}
	else
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EventGw,
			"subscribe topic [%s] with generic event helper."), topic.c_str());
		ZQ::common::MutexGuard sync(_lockGenSinks);
		GenEventSinkI::Ptr sink;
		// search in the current sink objects
		for(GenSinks::iterator it = _genSinks.begin(); it != _genSinks.end(); ++it)
		{
			if(topic == it->topic && QoSEqual(qos, it->qos))
			{
				sink = it->obj;
				break;
			}
		}

		// create if not found
		if(!sink)
		{
			sink = new GenEventSinkI(_log);
			Ice::ObjectPrx prx = _adapter->addWithUUID(sink);
			try
			{
				IceStorm::TopicPrx topicPrx = getTopic(topic, true);
				#if  ICE_INT_VERSION / 100 >= 306
					topicPrx->subscribeAndGetPublisher(qos, prx);
				#else
					topicPrx->subscribe(qos, prx);
				#endif
			}
			catch(...)
			{
				// notify bad connection
				notifyBadConnection();
			}
			GenSinkInfo info;
			info.obj = sink;
			info.prx = prx;
			info.topic = topic;
			info.qos = qos;
			_genSinks.push_back(info);
		}
		// register the helper
		sink->add(pHelper);
	}
    return true;
}


bool EventGw::unsubscribe(
                          IGenericEventHelper *pHelper,
                          const std::string& topic
                          )
{
    if(NULL == pHelper || topic.empty())
        return false;

	if(topic == TianShanIce::Streamer::TopicOfStream)
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EventGw,
			"unsubscribe topic [%s] with stream event helper."), topic.c_str());
		StreamSinks poorSinks;
		{
			ZQ::common::MutexGuard sync(_lockstreamSinks);
			for(StreamSinks::iterator it = _streamSinks.begin(); it != _streamSinks.end(); ++it)
			{
				if(topic == it->topic)
				{
					// unregister the helper
					it->obj->remove(pHelper);
				}
			}

			// remove the sink object if no helper left
			{
				StreamSinks::iterator itEmpty = std::remove_if(_streamSinks.begin(), _streamSinks.end(), StreamSinkInfo::IsEmpty);
				poorSinks.assign(itEmpty, _streamSinks.end());
				_streamSinks.erase(itEmpty, _streamSinks.end());
			}
		}

		{
			// unsubscribe
			try{
				for(StreamSinks::iterator it = poorSinks.begin(); it != poorSinks.end(); ++it)
				{
					IceStorm::TopicPrx topicPrx = getTopic(it->topic, false);
					if(topicPrx)
						topicPrx->unsubscribe(it->prx);
				}
			}catch(...)
			{
				notifyBadConnection();
			}

			// destroy the proxy
			{
				for(StreamSinks::iterator it = poorSinks.begin(); it != poorSinks.end(); ++it)
				{
					try{
						_adapter->remove(it->prx->ice_getIdentity());
					}catch(...)
					{
					}
				}
			}
		}
	}
	else if(topic == TianShanIce::Streamer::TopicOfPlaylist)
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EventGw,
			"unsubscribe topic [%s] with playlist event helper."), topic.c_str());
		PlayListSinks poorSinks;
		{
			ZQ::common::MutexGuard sync(_lockplaylistSinks);
			for(PlayListSinks::iterator it = _playlistSinks.begin(); it != _playlistSinks.end(); ++it)
			{
				if(topic == it->topic)
				{
					// unregister the helper
					it->obj->remove(pHelper);
				}
			}

			// remove the sink object if no helper left
			{
				PlayListSinks::iterator itEmpty = std::remove_if(_playlistSinks.begin(), _playlistSinks.end(), PlayListSinkInfo::IsEmpty);
				poorSinks.assign(itEmpty, _playlistSinks.end());
				_playlistSinks.erase(itEmpty, _playlistSinks.end());
			}
		}

		{
			// unsubscribe
			try{
				for(PlayListSinks::iterator it = poorSinks.begin(); it != poorSinks.end(); ++it)
				{
					IceStorm::TopicPrx topicPrx = getTopic(it->topic, false);
					if(topicPrx)
						topicPrx->unsubscribe(it->prx);
				}
			}catch(...)
			{
				notifyBadConnection();
			}

			// destroy the proxy
			{
				for(PlayListSinks::iterator it = poorSinks.begin(); it != poorSinks.end(); ++it)
				{
					try{
						_adapter->remove(it->prx->ice_getIdentity());
					}catch(...)
					{
					}
				}
			}
		}
	}
	else
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(EventGw,
			"unsubscribe topic [%s] with generic event helper."), topic.c_str());
		GenSinks poorSinks;
		{
			ZQ::common::MutexGuard sync(_lockGenSinks);
			for(GenSinks::iterator it = _genSinks.begin(); it != _genSinks.end(); ++it)
			{
				if(topic == it->topic)
				{
					// unregister the helper
					it->obj->remove(pHelper);
				}
			}

			// remove the sink object if no helper left
			{
				GenSinks::iterator itEmpty = std::remove_if(_genSinks.begin(), _genSinks.end(), GenSinkInfo::IsEmpty);
				poorSinks.assign(itEmpty, _genSinks.end());
				_genSinks.erase(itEmpty, _genSinks.end());
			}
		}

		{
			// unsubscribe
			try{
				for(GenSinks::iterator it = poorSinks.begin(); it != poorSinks.end(); ++it)
				{
					IceStorm::TopicPrx topicPrx = getTopic(it->topic, false);
					if(topicPrx)
						topicPrx->unsubscribe(it->prx);
				}
			}catch(...)
			{
				notifyBadConnection();
			}

			// destroy the proxy
			{
				for(GenSinks::iterator it = poorSinks.begin(); it != poorSinks.end(); ++it)
				{
					try{
						_adapter->remove(it->prx->ice_getIdentity());
					}catch(...)
					{
					}
				}
			}
		}
	}

    return true;
}

/// subscribe a topic
/// @param[in]  subscriber a specific event subscriber
/// @param[in]  topic the topic string
/// @param[in]  qos QoS
bool EventGw::subscribe(
                        Ice::ObjectPrx subscriber,
                        const std::string &topic,
                        const Properties &qos
                        )
{
    if(!subscriber || topic.empty())
        return false;
    
    _log(ZQ::common::Log::L_INFO, CLOGFMT(EventGw,
            "subscribe topic [%s] with specific event helper."), topic.c_str());
    ZQ::common::MutexGuard sync(_lockSpecSinks);

    try
    {
        IceStorm::TopicPrx topicPrx = getTopic(topic, true);
       
	   #if  ICE_INT_VERSION / 100 >= 306
       		topicPrx->subscribeAndGetPublisher(qos, subscriber);
		#else				 
			topicPrx->subscribe(qos, subscriber);
		#endif
    }
    catch(...)
    {
        // notify bad connection
        notifyBadConnection();
    }

    // add the subcriber info
    SpecSinkInfo info;
    info.prx = subscriber;
    info.topic = topic;
    info.qos = qos;
    _specSinks.push_back(info);
    return true;
}

/// unsubscribe a topic
/// @param[in]  subscriber a specific event subscriber
/// @param[in]  topic the topic string
bool EventGw::unsubscribe(
                          Ice::ObjectPrx subscriber,
                          const std::string& topic
                          )
{
    if(!subscriber || topic.empty())
        return false;
    
    _log(ZQ::common::Log::L_INFO, CLOGFMT(EventGw,
            "unsubscribe topic [%s] with specific event helper."), topic.c_str());
    ZQ::common::MutexGuard sync(_lockSpecSinks);
    
    // find the subscriber
    for(SpecSinks::iterator it = _specSinks.begin(); it != _specSinks.end(); ++it)
    {
        if(it->prx == subscriber && it->topic == topic)
        {
            try
            {
                IceStorm::TopicPrx topicPrx = getTopic(topic, false);
                if(topicPrx)
                    topicPrx->unsubscribe(subscriber);
            }
            catch(...)
            {
                // notify bad connection
                notifyBadConnection();
            }
            _specSinks.erase(it);
            return true;
        }
    }
    // not found the subscriber
    return false;
}


/// get the ICE object adapter
Ice::ObjectAdapterPtr EventGw::getAdapter()
{
    return _adapter;
}

/// get the config folder
const std::string& EventGw::getConfigFolder()
{
    return _pluginConfigDir;
}

/// get the log folder
const std::string& EventGw::getLogFolder()
{
    return _pluginLogDir;
}

ZQ::common::Log& EventGw::superLogger()
{
    return _log;
}

class ConnectionSentinelController: public EventChannel::Sentinel::ExternalControlData
{
public:
    bool connOK;

    ConnectionSentinelController():connOK(false) {}
    virtual void reportBadConnection()
    {
        connOK = false;
    }
};
/// the connection maintenance thread
int EventGw::run(void)
{
    ConnectionSentinelController sentinelCtrl;
    EventChannel::Sentinel* sentinelThrd = new EventChannel::Sentinel(_log, _iceStormProxy, &sentinelCtrl);
    sentinelThrd->start(); // sentinel thread

    while(true)
    {
        SYS::SingleObject::STATE st = _hNotifyBadConn.wait(_connCheckIntervalMSec);
        if(_quit)
            break; // thread quit
        
        bool needResubscribe = false;
        
        switch(st)
        {
        case SYS::SingleObject::TIMEDOUT:
            // regular check
            needResubscribe = false;
            break;
        case SYS::SingleObject::SIGNALED:
            // bad connection, need resubscribe all
            needResubscribe = true;
            break;
        default:
            // what happened?
            needResubscribe = true;
            break;
        }

        // check the connection
        try
        {
            checkConnection();
        }
        catch (...)
        {
            {
                // force to reconnect IceStorm
                ZQ::common::MutexGuard sync(_lockConnMgmt);
                _topicMgr = NULL;
            }
            _log(ZQ::common::Log::L_WARNING, CLOGFMT(EventGw,
                "Bad connection detected. Retry after %u msec"), _connCheckIntervalMSec);
            SYS::sleep(_connCheckIntervalMSec);
            notifyBadConnection();
            continue;
        }

        // connection is ok now
        if(!sentinelCtrl.connOK)
        { // the sentinel thread found the connetion broken
            needResubscribe = true;
            sentinelCtrl.connOK = true;
        }

        if(needResubscribe)
        {
            // subscribe all
            try
            {
                subscribeAll();
            }
            catch(const Ice::Exception &e)
            {
                _log(ZQ::common::Log::L_WARNING, CLOGFMT(EventGw,
                    "Caught [%s] during subscribeAll(). Retry immediately."), e.ice_name().c_str());

                // retry immediately
                notifyBadConnection();
            }
            catch(...)
            {
                _log(ZQ::common::Log::L_WARNING, CLOGFMT(EventGw,
                    "Unknown exception during subscribeAll(). Retry immediately."));

                // retry immediately
                notifyBadConnection();
            }
        }
    }

    delete sentinelThrd;
    return 0;
}

/// notify bad network connection
void EventGw::notifyBadConnection()
{
    _hNotifyBadConn.signal();
}

void EventGw::subscribeAll()
{
    {
        ZQ::common::MutexGuard sync(_lockGenSinks);
        for(GenSinks::iterator it = _genSinks.begin(); it != _genSinks.end(); ++it)
        {
			 #if  ICE_INT_VERSION / 100 >= 306
            	getTopic(it->topic, true)->subscribeAndGetPublisher(it->qos, it->prx);
			 #else
			 	getTopic(it->topic, true)->subscribe(it->qos, it->prx);
			 #endif
			 	
        }
    }
	{
		ZQ::common::MutexGuard sync(_lockstreamSinks);
		for(StreamSinks::iterator it = _streamSinks.begin(); it != _streamSinks.end(); ++it)
		{
			#if  ICE_INT_VERSION / 100 >= 306
				getTopic(it->topic, true)->subscribeAndGetPublisher(it->qos, it->prx);
			#else
				getTopic(it->topic, true)->subscribe(it->qos, it->prx);
			#endif
		}
	}
	{
		ZQ::common::MutexGuard sync(_lockplaylistSinks);
		for(PlayListSinks::iterator it = _playlistSinks.begin(); it != _playlistSinks.end(); ++it)
		{
			#if  ICE_INT_VERSION / 100 >= 306
				getTopic(it->topic, true)->subscribeAndGetPublisher(it->qos, it->prx);
			#else
				getTopic(it->topic, true)->subscribe(it->qos, it->prx);
			#endif
		}
	}
    {
        ZQ::common::MutexGuard sync(_lockSpecSinks);
        for(SpecSinks::iterator it = _specSinks.begin(); it != _specSinks.end(); ++it)
        {
			#if  ICE_INT_VERSION / 100 >= 306
            	getTopic(it->topic, true)->subscribeAndGetPublisher(it->qos, it->prx);
			#else
				 getTopic(it->topic, true)->subscribe(it->qos, it->prx);
			#endif
        }
    }
}

void EventGw::unsubscribeAll()
{
    {
        ZQ::common::MutexGuard sync(_lockGenSinks);
        for(GenSinks::iterator it = _genSinks.begin(); it != _genSinks.end(); ++it)
        {
            IceStorm::TopicPrx topicPrx = getTopic(it->topic, false);
            if(topicPrx)
                topicPrx->unsubscribe(it->prx);
        }
    }

	{
		ZQ::common::MutexGuard sync(_lockstreamSinks);
		for(StreamSinks::iterator it = _streamSinks.begin(); it != _streamSinks.end(); ++it)
		{
			IceStorm::TopicPrx topicPrx = getTopic(it->topic, false);
			if(topicPrx)
				topicPrx->unsubscribe(it->prx);
		}
	}

	{
		ZQ::common::MutexGuard sync(_lockplaylistSinks);
		for(PlayListSinks::iterator it = _playlistSinks.begin(); it != _playlistSinks.end(); ++it)
		{
			IceStorm::TopicPrx topicPrx = getTopic(it->topic, false);
			if(topicPrx)
				topicPrx->unsubscribe(it->prx);
		}
	}

    {
        ZQ::common::MutexGuard sync(_lockSpecSinks);
        for(SpecSinks::iterator it = _specSinks.begin(); it != _specSinks.end(); ++it)
        {
            IceStorm::TopicPrx topicPrx = getTopic(it->topic, false);
            if(topicPrx)
                topicPrx->unsubscribe(it->prx);
        }
    }
}

/// check the connection with IceStorm
/// @note The function will throw exception to indicate the failure
void EventGw::checkConnection()
{
    ZQ::common::MutexGuard sync(_lockConnMgmt);
    if(!_topicMgr)
    {
        // connect the IceStorm
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventGw,
            "checkConnection() try to connect the EventChannel [%s]."), _iceStormProxy.c_str());
        Ice::ObjectPrx prx = _communicator->stringToProxy(_iceStormProxy);
        if(!prx)
            throw 3; // report the bad connection
        _topicMgr = IceStorm::TopicManagerPrx::uncheckedCast(prx);
        // clear the proxy cache
        _topicCache.clear();
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventGw,
            "checkConnection() clear the topic cache."));
    }
    _topicMgr->ice_ping();
}

/// get the topic proxy from the IceStorm
/// @param[in]  topicstr the topic string
/// @param[in]  createIfNotExist true for creating the topic when the topic not exist
IceStorm::TopicPrx EventGw::getTopic(const std::string &topicstr, bool createIfNotExist)
{
    ZQ::common::MutexGuard sync(_lockConnMgmt);

    // check the cache first
    Topics::iterator it = _topicCache.find(topicstr);
    if(it != _topicCache.end())
    {
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventGw,
            "got topic [%s] from cache."), topicstr.c_str());
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
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventGw, 
            "no topic [%s] found in EventChannel [%s]."), topicstr.c_str(), _iceStormProxy.c_str());
        if(createIfNotExist)
        {
            try{
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventGw,
                    "trying to create topic [%s] in EventChannel [%s]."), topicstr.c_str(), _iceStormProxy.c_str());
                // create the topic
                topic = _topicMgr->create(topicstr);
            }catch(const IceStorm::TopicExists &)
            {
                // someone may create the topic recently, retry and don't care the failure this time
                topic = _topicMgr->retrieve(topicstr);
            }
        }
    }
    
    // cache the topic proxy
    if(topic)
    {
        _topicCache[topicstr] = topic;
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventGw,
            "cached topic [%s]."), topicstr.c_str());
    }
    
    return topic;
}
} // namespace EventGateway
