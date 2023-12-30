#include "PlaylistEventSinkImpl.h"
#include "CMESOAPConfig.h"

namespace EventGateway{
namespace CMESOAPHelper{

PlaylistEventSinkI::PlaylistEventSinkI(ZQ::common::Log &log, ClientImpl &client)
:_log(log), _client(client)
{
}

PlaylistEventSinkI::~PlaylistEventSinkI()
{
}

void PlaylistEventSinkI::ping(::Ice::Long timestamp, const ::Ice::Current&)
{
    // not implemented
}

static const std::string _prop_prevProviderId("prevProviderId");
static const std::string _prop_prevProviderAssetId("prevProviderAssetId");
static const std::string _prop_currentProviderId("currentProviderId");
static const std::string _prop_currentProviderAssetId("currentProviderAssetId");
static const std::string _prop_prevStreamingSource("prevStreamingSource");
static const std::string _prop_currentStreamingSource("curStreamingSource");
static const std::string _value_streamingSource_local("0");
static const std::string _prop_clusterID("clusterId");
static const std::string _prop_stampUTC("stampUTC");

#define UTIL_FETCH_PROP_EX(props, propName, value, noEmptyValue) {\
    TianShanIce::Properties::const_iterator it = props.find(propName);\
    if(it == props.end() || (noEmptyValue && it->second.empty())){\
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(PlaylistEventSinkI, "OnItemStepped() property [%s] missed."), propName.c_str());\
        break;  /* incorrect data */\
    }\
    value = it->second;\
}
#define UTIL_FETCH_PROP(props, propName, value) UTIL_FETCH_PROP_EX(props, propName, value, true)

#define IsSpecialCtrlNum(num) (num == TianShanIce::Streamer::InvalidCtrlNum || num == TianShanIce::Streamer::PlaylistHeadCtrlNum || num == TianShanIce::Streamer::PlaylistTailCtrlNum)
void PlaylistEventSinkI::OnItemStepped(const std::string& proxy, const std::string& playlistId, Ice::Int currentUserCtrlNum, Ice::Int prevUserCtrlNum, const TianShanIce::Properties& ItemProps, const ::Ice::Current&) const
{
    _log(ZQ::common::Log::L_DEBUG, CLOGFMT(PlaylistEventSinkI,
        "OnItemStepped() sess [%s], [%d] -> [%d]."), playlistId.c_str(), prevUserCtrlNum, currentUserCtrlNum);

    // Session Stop of previous item
    // use the do ... while(false) with break statement to simplify the structure of the processing
    do 
    {
        if(IsSpecialCtrlNum(prevUserCtrlNum))
        {
            // the first stepped
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(PlaylistEventSinkI, "OnItemStepped() sess [%s], [%d] -> [%d]. Stepped from InvalidCtrlNum. No Session Stop of previous item"), playlistId.c_str(), prevUserCtrlNum, currentUserCtrlNum);
            break;
        }

        if(gConfig.ignoreLocal)
        {
            // check the streaming source
            std::string val;
            UTIL_FETCH_PROP_EX(ItemProps, _prop_prevStreamingSource, val, false);
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(PlaylistEventSinkI, "OnItemStepped() sess [%s], [%d] -> [%d]. Previous StreamingSource [%s]"), playlistId.c_str(), prevUserCtrlNum, currentUserCtrlNum, val.c_str());
            if(val.empty() || val == _value_streamingSource_local)
            {
                // ignore local storage source
                _log(ZQ::common::Log::L_INFO, CLOGFMT(PlaylistEventSinkI,
                    "OnItemStepped() sess [%s], [%d] -> [%d]. Ignore local storage source of previous item."), playlistId.c_str(), prevUserCtrlNum, currentUserCtrlNum);
                break;
            }
        }
        // construct the request data
        RequestData::SessionNotification rqstData;
        // previous item is down
        rqstData.func = 2; // 2 ¨C Session Stop
        UTIL_FETCH_PROP(ItemProps, _prop_prevProviderId, rqstData.providerID);
        UTIL_FETCH_PROP(ItemProps, _prop_prevProviderAssetId, rqstData.providerAssetID);
        UTIL_FETCH_PROP(ItemProps, _prop_clusterID, rqstData.clusterID);
        UTIL_FETCH_PROP(ItemProps, _prop_stampUTC, rqstData.timeStamp);
        // construct a unique session ID
        rqstData.sessionID = playlistId + "+" +  rqstData.providerID + "+" + rqstData.providerAssetID;

        // send the request
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(PlaylistEventSinkI, "OnItemStepped() sess [%s], [%d] -> [%d]. Sending SessionStop notification: PID(%s), PAID(%s), ClusterID(%s)"), playlistId.c_str(), prevUserCtrlNum, currentUserCtrlNum, rqstData.providerID.c_str(), rqstData.providerAssetID.c_str(), rqstData.clusterID.c_str());

        _client.sessionNotification(rqstData);
    }while(false);

    // Session Start of current item
    // use the do ... while(false) with break statement to simplify the structure of the processing
    do 
    {
        if(IsSpecialCtrlNum(currentUserCtrlNum))
        {
            // the last stepped
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(PlaylistEventSinkI, "OnItemStepped() sess [%s], [%d] -> [%d]. stepped to InvalidCtrlNum. No Session Start of current item."), playlistId.c_str(), prevUserCtrlNum, currentUserCtrlNum);
            break;
        }

        if(gConfig.ignoreLocal)
        {
            // check the streaming source
            std::string val;
            UTIL_FETCH_PROP_EX(ItemProps, _prop_currentStreamingSource, val, false);
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(PlaylistEventSinkI,
                "OnItemStepped() sess [%s], [%d] -> [%d]. current StreamingSource [%s]"), playlistId.c_str(), prevUserCtrlNum, currentUserCtrlNum, val.c_str());
            if(val.empty() || val == _value_streamingSource_local)
            {
                // ignore local storage source
                _log(ZQ::common::Log::L_INFO, CLOGFMT(PlaylistEventSinkI,
                    "OnItemStepped() sess [%s], [%d] -> [%d]. Ignore local storage source of current item."), playlistId.c_str(), prevUserCtrlNum, currentUserCtrlNum);
                break;
            }
        }
        // construct the request data
        RequestData::SessionNotification rqstData;
        // current item is starting
        rqstData.func = 1; // 1 ¨C Session Start
        UTIL_FETCH_PROP(ItemProps, _prop_currentProviderId, rqstData.providerID);
        UTIL_FETCH_PROP(ItemProps, _prop_currentProviderAssetId, rqstData.providerAssetID);
        UTIL_FETCH_PROP(ItemProps, _prop_clusterID, rqstData.clusterID);
        UTIL_FETCH_PROP(ItemProps, _prop_stampUTC, rqstData.timeStamp);
        // construct a unique session ID
        rqstData.sessionID = playlistId + "+" +  rqstData.providerID + "+" + rqstData.providerAssetID;
        // send the request
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(PlaylistEventSinkI, "OnItemStepped() sess [%s], [%d] -> [%d]. Sending SessionStart notification: PID(%s), PAID(%s), ClusterID(%s)"), playlistId.c_str(), prevUserCtrlNum, currentUserCtrlNum, rqstData.providerID.c_str(), rqstData.providerAssetID.c_str(), rqstData.clusterID.c_str());
        _client.sessionNotification(rqstData);
    }while(false);
}

#undef UTIL_FETCH_PROP
} // namespace CMESOAPHelper
} // namespace EventGateway

