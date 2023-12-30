#ifndef __TIANSHAN_EVENTGATEWAY_H__
#define __TIANSHAN_EVENTGATEWAY_H__
#include <TianShanDefines.h>
#include "EventGwHelper.h"
#include "GenEventSinkI.h"
#include "StreamEventSinkI.h"
#include <Log.h>
#include <NativeThread.h>
#include "SystemUtils.h"
namespace EventGateway
{
#define EventGw_ConnCheck_IntervalMSec_Min 100
#define EventGw_ConnCheck_IntervalMSec_Max 10000

class EventGw : public IEventGateway, public ZQ::common::NativeThread
{
public:
    EventGw(
        Ice::CommunicatorPtr ic,
        Ice::ObjectAdapterPtr adptr,
        ZQ::common::Log &log,
        const std::string &iceStormProxy,
        const std::string &pluginConfigDir,
        const std::string &pluginLogDir,
        size_t connCheckIntervalMSec = EventGw_ConnCheck_IntervalMSec_Min
        );
    ~EventGw();
public:
    /// subscribe a topic
    /// @param[in]  pHelper a generic event helper
    /// @param[in]  topic the topic string
    /// @param[in]  qos QoS
    virtual bool subscribe(
        IGenericEventHelper *pHelper,
        const std::string &topic,
        const Properties &qos = Properties()
        );
    
    /// unsubscribe a topic
    /// @param[in]  pHelper a generic event helper
    /// @param[in]  topic the topic string
    virtual bool unsubscribe(
        IGenericEventHelper *pHelper,
        const std::string& topic
        );
    
    /// subscribe a topic
    /// @param[in]  subscriber a specific event subscriber
    /// @param[in]  topic the topic string
    /// @param[in]  qos QoS
    virtual bool subscribe(
        Ice::ObjectPrx subscriber,
        const std::string &topic,
        const Properties &qos = Properties()
        );
    
    /// unsubscribe a topic
    /// @param[in]  subscriber a specific event subscriber
    /// @param[in]  topic the topic string
    virtual bool unsubscribe(
        Ice::ObjectPrx subscriber,
        const std::string& topic
        );
    
    /// get the ICE object adapter
    virtual Ice::ObjectAdapterPtr getAdapter();

    /// get the config folder
    virtual const std::string& getConfigFolder();
    
    /// get the log folder
    virtual const std::string& getLogFolder();

    virtual ZQ::common::Log& superLogger();
protected:
    
    /// the connection maintenance thread
    virtual int run(void);

private:
    
    void subscribeAll();
    void unsubscribeAll();

    /// notify bad network connection
    void notifyBadConnection();
    
    /// check the connection with IceStorm
    /// @note The function will throw exception to indicate the failure
    void checkConnection();

    /// get the topic proxy from the IceStorm
    /// @param[in]  topicstr the topic string
    /// @param[in]  createIfNotExist true for creating the topic when the topic not exist
    IceStorm::TopicPrx getTopic(const std::string &topicstr, bool createIfNotExist = false);
private:
    Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _adapter;
    ZQ::common::Log &_log;

    // member for connection maintenance
    std::string _iceStormProxy;
    IceStorm::TopicManagerPrx _topicMgr;
    typedef std::map<std::string, IceStorm::TopicPrx> Topics;
    Topics _topicCache;
    SYS::SingleObject _hNotifyBadConn;
    ZQ::common::Mutex _lockConnMgmt;

    // members for generic event helpers
    struct GenSinkInfo
    {
        GenEventSinkI::Ptr obj;
        ::Ice::ObjectPrx prx;
        ::std::string topic;
        ::TianShanIce::Properties qos;
        
        // auxiliary function 
        static bool IsEmpty(GenSinkInfo &sinkInfo)
        {
            return (0 == sinkInfo.obj->count());
        }
    };
    typedef std::vector<GenSinkInfo> GenSinks;
    GenSinks _genSinks;
    ZQ::common::Mutex _lockGenSinks;

	// members for stream event helpers
	struct StreamSinkInfo
	{
		StreamEventSinkImpl::Ptr obj;
		::Ice::ObjectPrx prx;
		::std::string topic;
		::TianShanIce::Properties qos;

		// auxiliary function 
		static bool IsEmpty(StreamSinkInfo &sinkInfo)
		{
			return (0 == sinkInfo.obj->count());
		}
	};
	typedef std::vector<StreamSinkInfo> StreamSinks;
	StreamSinks _streamSinks;
	ZQ::common::Mutex _lockstreamSinks;

	// members for playlist event helpers
	struct PlayListSinkInfo
	{
		PlaylistEventSinkImpl::Ptr obj;
		::Ice::ObjectPrx prx;
		::std::string topic;
		::TianShanIce::Properties qos;

		// auxiliary function 
		static bool IsEmpty(PlayListSinkInfo &sinkInfo)
		{
			return (0 == sinkInfo.obj->count());
		}
	};
	typedef std::vector<PlayListSinkInfo> PlayListSinks;
	PlayListSinks _playlistSinks;
	ZQ::common::Mutex _lockplaylistSinks;
    // members for generic event helpers
    struct SpecSinkInfo
    {
        Ice::ObjectPrx prx;
        std::string topic;
        TianShanIce::Properties qos;
    };
    typedef std::vector<SpecSinkInfo> SpecSinks;
    SpecSinks _specSinks;
    ZQ::common::Mutex _lockSpecSinks;

    std::string _pluginConfigDir; // the plug-in config folder
    std::string _pluginLogDir; // the plug-in log folder

    bool _quit;
    size_t _connCheckIntervalMSec;
};

} // namespace EventGateway
#endif

