#include "StreamEventSinkImpl.h"
#include "CMESOAPConfig.h"

namespace EventGateway{
namespace CMESOAPHelper{

StreamEventSinkI::StreamEventSinkI(ZQ::common::Log &log, ClientImpl &client)
:_log(log), _client(client)
{
}

StreamEventSinkI::~StreamEventSinkI()
{
}

void StreamEventSinkI::ping(::Ice::Long timestamp, const ::Ice::Current&)
{
    // not implemented
}
void StreamEventSinkI::OnEndOfStream(const std::string &proxy, const std::string &id, const TianShanIce::Properties &, const ::Ice::Current&) const
{
    // not implemented
}
void StreamEventSinkI::OnBeginningOfStream(const std::string &proxy, const std::string &id, const TianShanIce::Properties &, const ::Ice::Current&) const
{
    // not implemented
}
void StreamEventSinkI::OnStateChanged(const std::string &proxy, const std::string &id, TianShanIce::Streamer::StreamState prevState, TianShanIce::Streamer::StreamState currentState, const TianShanIce::Properties &, const ::Ice::Current&) const
{
    // not implemented
}
void StreamEventSinkI::OnSpeedChanged(const std::string &proxy, const std::string &id, Ice::Float prevSpeed, Ice::Float currentSpeed, const TianShanIce::Properties &, const ::Ice::Current&) const
{
    // not implemented
}
void StreamEventSinkI::OnExit(const std::string &proxy, const std::string &id, Ice::Int exitCode, const std::string &reason, const ::Ice::Current&) const
{
    // not implemented
}


static const std::string _prop_providerID("providerId");
static const std::string _prop_providerAssetID("providerAssetId");
static const std::string _prop_streamingSource("streamingSource");
static const std::string _value_streamingSource_local("0");
static const std::string _prop_exitWhilePlaying("exitWhilePlaying");
static const std::string _value_exit_normally("0");
static const std::string _prop_clusterID("clusterId");
static const std::string _prop_stampUTC("stampUTC");

#define UTIL_FETCH_PROP_EX(props, propName, value, noEmptyValue) {\
    TianShanIce::Properties::const_iterator it = props.find(propName);\
    if(it == props.end() || (noEmptyValue && it->second.empty())){\
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinkI, "OnExit2() property [%s] missed."), propName.c_str());\
        return;  /* incorrect data */\
    }\
    value = it->second;\
}
#define UTIL_FETCH_PROP(props, propName, value) UTIL_FETCH_PROP_EX(props, propName, value, true)

void StreamEventSinkI::OnExit2(const std::string &proxy, const std::string &id, Ice::Int exitCode, const std::string &reason ,const ::TianShanIce::Properties& props, const ::Ice::Current&) const
{
    std::string val;
    if(gConfig.ignoreLocal)
    {
        UTIL_FETCH_PROP_EX(props, _prop_streamingSource, val, false);
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "OnExit2() sess [%s], StreamingSource [%s]"), id.c_str(), val.c_str());
        if(val.empty() || val == _value_streamingSource_local)
        {
            // ignore local storage source
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "OnExit2() sess [%s], ignore local storage source."), id.c_str());
            return;
        }
    }

    val = "";
    UTIL_FETCH_PROP(props, _prop_exitWhilePlaying, val);
    _log(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "OnExit2() sess [%s], exitWhilePlaying [%s]"), id.c_str(), val.c_str());
    if(val == _value_exit_normally)
    {
        // ignore the normal exit
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "OnExit2() sess [%s], ignore the normal exit."), id.c_str());
        return;
    }

    RequestData::SessionNotification rqstData;
    rqstData.func = 2; // 2 ¨C Session Stop
    UTIL_FETCH_PROP(props, _prop_providerID, rqstData.providerID);
    UTIL_FETCH_PROP(props, _prop_providerAssetID, rqstData.providerAssetID);
    UTIL_FETCH_PROP(props, _prop_clusterID, rqstData.clusterID);
    UTIL_FETCH_PROP(props, _prop_stampUTC, rqstData.timeStamp);

    // construct a unique session ID
    rqstData.sessionID = id + "+" +  rqstData.providerID + "+" + rqstData.providerAssetID;
    _log(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "onExit2() sess [%s]. Sending SessionStop notification: PID(%s), PAID(%s), ClusterID(%s)"), id.c_str(),  rqstData.providerID.c_str(), rqstData.providerAssetID.c_str(), rqstData.clusterID.c_str());
    _client.sessionNotification(rqstData);
}
#undef UTIL_FETCH_PROP
} // namespace CMESOAPHelper
} // namespace EventGateway

