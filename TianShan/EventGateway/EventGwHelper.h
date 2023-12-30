#ifndef __TIANSHAN_EVENTGW_HELPER_H__
#define __TIANSHAN_EVENTGW_HELPER_H__
//#include <TianShanDefines.h>
#include <ZQ_common_conf.h>
#include <Log.h>
#include <Ice/Ice.h>
namespace EventGateway{

typedef std::map<std::string, std::string> Properties;

class IEventGateway;

#define EventGw_Module_Name_Prefix  "EGH_"
#define EventGw_Module_Entry_init   init
#define EventGw_Module_Entry_uninit uninit
/// dll entry
/// initialization entry with name 'init'
typedef bool (*libInit)(IEventGateway* gateway);

/// uninitialization entry with name 'uninit'
typedef void (*libUninit)();

/// helper for TianShan generic event
class IGenericEventHelper
{
public:
    virtual ~IGenericEventHelper() {}
public:
    virtual void onEvent(
        const ::std::string& category,
        ::Ice::Int eventId,
        const ::std::string& eventName,
        const ::std::string& stampUTC,
        const ::std::string& sourceNetId,
        const Properties& params
        ) = 0;
};

/// an interface exported from the gateway to plugin
class IEventGateway
{
public:
    virtual ~IEventGateway() {}
public:
    /// subscribe a topic
    /// @param[in]  pHelper a generic event helper
    /// @param[in]  topic the topic string
    /// @param[in]  qos QoS
    virtual bool subscribe(
        IGenericEventHelper *pHelper,
        const std::string &topic,
        const Properties &qos = Properties()
        ) = 0;
    
    /// unsubscribe a topic
    /// @param[in]  pHelper a generic event helper
    /// @param[in]  topic the topic string
    virtual bool unsubscribe(
        IGenericEventHelper *pHelper,
        const std::string& topic
        ) = 0;
    
    /// subscribe a topic
    /// @param[in]  subscriber a specific event subscriber
    /// @param[in]  topic the topic string
    /// @param[in]  qos QoS
    virtual bool subscribe(
        Ice::ObjectPrx subscriber,
        const std::string &topic,
        const Properties &qos = Properties()
        ) = 0;
    
    /// unsubscribe a topic
    /// @param[in]  subscriber a specific event subscriber
    /// @param[in]  topic the topic string
    virtual bool unsubscribe(
        Ice::ObjectPrx subscriber,
        const std::string& topic
        ) = 0;
    
    /// get the ICE object adapter
    virtual Ice::ObjectAdapterPtr getAdapter() = 0;
    
    /// get the config folder
    virtual const std::string& getConfigFolder() = 0;

    /// get the log folder
    virtual const std::string& getLogFolder() = 0;

    /// the logger instance provided by gateway for 
    /// reporting emergency error during initialization.
    virtual ZQ::common::Log& superLogger() = 0;
};
}
#endif
// 
// 
// ///sample for a plugin
// 
// class MyHelper: public IGenericEventHelper
//           {
// 
// } self;
// 
// class MyEvent :...
// {
// 
// 
// } mysink;
// 
// bool init(IEventGateway* gateway)
// {
//     config = gateway->getConfigFolder()
//     //    ...
// 
//     // do initiliaze self
// 
//     // for each genenric topic
//     gateway->subscribe(&self, topic, qos);
// 
//     // for each userdefined Topic
//     gateway->subscribe(mysinkPrx, topic, qos);
//
//     return true;
// }
// 
// void uninit()
// {
//     // for each genenric topic
//     gateway->unsubscribe((&self, topic);
//     
//     // for each userdefined Topic
//     gateway->unsubscribe(mysinkPrx, topic);
//
//     // clear the sink proxy
//     gateway->getAdapter()->remove(mysinkIdent);
//     
//     // uninitialize self
// }
