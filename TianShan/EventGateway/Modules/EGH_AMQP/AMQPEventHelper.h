#ifndef __EGH_AMQP_EventHelper_H__
#define __EGH_AMQP_EventHelper_H__
#include <EventGwHelper.h>
#include "AMQPConfig.h"
#include "AMQPMessageChannel.h"
#include <boost/regex.hpp>

namespace EventGateway{
namespace AMQP{
class EventFilter
{
public:
    explicit EventFilter(ZQ::common::Log& log);
    bool init(const EventFilterConfig& conf);

    bool fit(const std::string& eventName, const std::string& category, const std::string sourceNetId, const Properties& params);
private:
    ZQ::common::Log& _log;
    boost::regex _eventName;
    boost::regex _category;
    boost::regex _sourceNetId;
    struct Prop
    {
        std::string key;
        boost::regex value;
    };
    typedef std::list<Prop> Props;
    Props _props;
};

class EventHelper: public EventGateway::IGenericEventHelper
{
public:
    EventHelper(ZQ::common::Log& log, MessageChannel* channel);
    virtual ~EventHelper();

    bool init(const EventFilterConfig &conf, const MessageTemplate& msgTemplate);
    virtual void onEvent(
                         const ::std::string& category,
                         ::Ice::Int eventId,
                         const ::std::string& eventName,
                         const ::std::string& stampUTC,
                         const ::std::string& sourceNetId,
                         const Properties& params
                         );
private:
    ZQ::common::Log& _log;
    MessageChannel* _channel;
    EventFilter _filter;
    std::map<std::string, std::string> _tmpl;
    uint32 _vars;
};
} // namespace AMQP
} // names EventGateway
#endif
