#include "AMQPEventHelper.h"
#include <TimeUtil.h>

static std::string isoTimeToMonitorCenterTime(const std::string& stamp);
namespace EventGateway{
namespace AMQP{

EventFilter::EventFilter(ZQ::common::Log& log)
:_log(log)
{
}
bool EventFilter::init(const EventFilterConfig& conf)
{
    _props.clear();
    // eventName
    try
    {
        _eventName.assign(conf.eventName);
    }
    catch( const boost::bad_expression&)
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventFilter, "init() Bad expression for eventName:[%s]"), conf.eventName.c_str());
        return false;
    }

    // category
    try
    {
        _category.assign(conf.category);
    }
    catch(const boost::bad_expression&)
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventFilter, "init() Bad expression for category:[%s]"), conf.category.c_str());
        return false;
    }

    // sourceNetId
    try
    {
        _sourceNetId.assign(conf.sourceNetId);
    }
    catch(const boost::bad_expression&)
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventFilter, "init() Bad expression for sourceNetId:[%s]"), conf.sourceNetId.c_str());
        return false;
    }

    // properties
    std::map<std::string, std::string>::const_iterator it;
    for(it = conf.props.begin(); it != conf.props.end(); ++it)
    {
        Prop matcher;
        matcher.key = it->first;
        try
        {
            matcher.value.assign(it->second);
        }
        catch(const boost::bad_expression&)
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventFilter, "init() Bad expression for parameter [%s]:[%s]"), it->first.c_str(), it->second.c_str());
            return false;
        }
        _props.push_back(matcher);
    }
    return true;
}

bool EventFilter::fit(const std::string& eventName, const std::string& category, const std::string sourceNetId, const Properties& params)
{
    boost::smatch m;
    // eventName
    if(!boost::regex_match(eventName, m, _eventName))
        return false;

    // category
    if(!boost::regex_match(category, m, _category))
        return false;

    // sourceNetId
    if(!boost::regex_match(sourceNetId, m, _sourceNetId))
        return false;

    // properties
    for(Props::const_iterator it = _props.begin(); it != _props.end(); ++it)
    {
        Properties::const_iterator itParam = params.find(it->key);
        if(itParam == params.end())
            return false;

        if(!boost::regex_match(itParam->second, m, it->value))
            return false;
    }

    return true;
}

class FormattingException: public ZQ::common::Exception
{
public:
    FormattingException(const std::string& what_arg) throw()
        :ZQ::common::Exception(what_arg)
    {
    }
};
using ZQ::common::throwf;
// throw FormattingException to indicate the error
static std::string format(const std::string& fmt, const std::map<std::string, std::string>& parameters);
#define OPTVAR_Code_MonitorCenterTime 0x00000001
#define OPTVAR_Name_MonitorCenterTime "MonitorCenterTime"

EventHelper::EventHelper(ZQ::common::Log& log, MessageChannel* channel)
    :_log(log), _channel(channel), _filter(log)
{
    _vars = 0;
}
EventHelper::~EventHelper(){}

bool EventHelper::init(const EventFilterConfig &conf, const MessageTemplate& msgTemplate)
{
    if(!_filter.init(conf))
        return false;

    _tmpl.clear();
    if(msgTemplate.type == "text")
    {
        if(!msgTemplate.txtContent.empty())
        {
            _tmpl[""] = msgTemplate.txtContent;
        }
        else
        { // bad template definition
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventHelper, "init() No content of text message template"));
            return false;
        }
    }
    else if(msgTemplate.type == "map")
    {
        // check the content
        if(msgTemplate.mapContent.empty())
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventHelper, "init() No content of map message template"));
            return false;
        }

        for(std::map<std::string, std::string>::const_iterator it = msgTemplate.mapContent.begin(); it != msgTemplate.mapContent.end(); ++it)
        {
            if(it->first.empty())
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventHelper, "init() Empty property key of map message template"));
                return false;
            }
        }

        _tmpl = msgTemplate.mapContent;
    }
    else
    { // bad message type
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventHelper, "init() Bad message template type:%s"), msgTemplate.type.c_str());
        return false;
    }

    if(msgTemplate.vars.find(OPTVAR_Name_MonitorCenterTime) != std::string::npos) {
        _vars |= OPTVAR_Code_MonitorCenterTime;
    }
    return true;
}

void EventHelper::onEvent(
                          const ::std::string& category,
                          ::Ice::Int eventId,
                          const ::std::string& eventName,
                          const ::std::string& stampUTC,
                          const ::std::string& sourceNetId,
                          const Properties& params
                          )
{
    if(!_filter.fit(eventName, category, sourceNetId, params))
    {
        return;
    }

    std::map<std::string, std::string> ctx = params;
    // setup context
    ctx["Category"] = category;
    ctx["EventName"] = eventName;
    ctx["StampUTC"] = stampUTC;
    ctx["SourceNetId"] = sourceNetId;
    char buf[20] = {0};
    ctx["EventId"] = itoa(eventId, buf, 10);
    if(_vars & OPTVAR_Code_MonitorCenterTime) {
        ctx[OPTVAR_Name_MonitorCenterTime] = isoTimeToMonitorCenterTime(stampUTC);
        if(ctx[OPTVAR_Name_MonitorCenterTime].empty()) {
            _log(ZQ::common::Log::L_WARNING, CLOGFMT(EventHelper, "onEvent() failed to convert ISO timestamp(%s) to MonitorCenterTime"), stampUTC.c_str());
            return;
        }
    }
    Message msg;
    try
    {
        std::map<std::string, std::string>::const_iterator it;
        for(it = _tmpl.begin(); it != _tmpl.end(); ++it)
        {
            msg[it->first] = format(it->second, ctx);
        }
    }catch(const FormattingException& e)
    {
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(EventHelper, "onEvent() FormmatingException caught during generating the messsage. detail:%s"), e.getString());
        return;
    }
    _channel->push(msg);
}

std::string format(const std::string& fmt, const std::map<std::string, std::string>& parameters)
{
    std::string param; // the current param that need to be interpret
    param.reserve(100); // the parameter shouldn't very long
    std::stringstream buf;
    enum{Begin, InParam, OutParam, End} state = Begin; // the current state
    size_t i = 0;
    char c = 0; // the current character

#define NextChar() if(i < fmt.size()) c = fmt[i++]; else state = End // will be invoked after the current character been processed
    bool bContinue = true;
    while(bContinue)
    {
        switch(state)
        {
        case Begin:
            state = OutParam; // just go into the OutParam state
            NextChar(); // get the first character
            break;
        case InParam:
            if(isalpha(c))
            { // continue the InParam state
                param.push_back(c);
                NextChar();
            }
            else
            { // end of the parameter
                // we may have a parameter, resolve it
                if(!param.empty())
                {
                    std::map<std::string, std::string>::const_iterator it;
                    it = parameters.find(param);
                    if(it != parameters.end())
                    {
                        buf << it->second;
                    }
                    else
                    {
                        throwf<FormattingException>("Parameter [%s] not found. fmt=[%s]", param.c_str(), fmt.c_str());
                    }
                    param.clear();
                }
                else
                {
                    throwf<FormattingException>("Bad format definition at position [%d]. fmt=[%s]", i, fmt.c_str());
                }
                state = OutParam; // the current c haven't been processed yet
            }
            break;
        case OutParam:
            if(c == '$')
            {
                state = InParam; // start of the parameter
            }
            else
            {
                buf << c; // just copy
            }
            NextChar();
            break;
        case End:
            // we may have a parameter, resolve it
            if(!param.empty())
            {
                std::map<std::string, std::string>::const_iterator it;
                it = parameters.find(param);
                if(it != parameters.end())
                {
                    buf << it->second;
                }
                else
                {
                    throwf<FormattingException>("Parameter [%s] not found. fmt=[%s]", param.c_str(), fmt.c_str());
                }
                param.clear();
            }
            bContinue = false; // all work done
            break;
        }
    }
    return buf.str();
}

} // namespace AMQP
} // names EventGateway


static std::string isoTimeToMonitorCenterTime(const std::string& stamp)
{
    if(stamp.empty()) {
        return "";
    }
    const std::string& utctime = stamp;
    std::string strTime;
    char Buf[64] ="";

    // conver utc time 2009-08-12T08:00:00.000Z to local time.
    //format: 2009-08-12 08:00:00.000
    time_t _time;
    struct tm* _tm;

#ifdef ZQ_OS_MSWIN
    bool bret = ZQ::common::TimeUtil::Iso2TimeEx(utctime.c_str(), _time);
    if(!bret)
        return "";
    _tm = localtime(&_time);
#else
    bool bret = ZQ::common::TimeUtil::Iso2Time(utctime.c_str(), _time);
    if(!bret)
        return "";
    struct tm _tmstorage;
    _tm = localtime_r(&_time, &_tmstorage);
#endif
    sprintf(Buf, "%04d-%02d-%02d %02d:%02d:%02d.000\0", 
        _tm->tm_year + 1900, _tm->tm_mon +1, _tm->tm_mday,
        _tm->tm_hour, _tm->tm_min, _tm->tm_sec);
    strTime = Buf;
    return strTime;
}
