#ifndef __CMESOAP_PlaylistEventSinkImpl_H__
#define __CMESOAP_PlaylistEventSinkImpl_H__
#include <ZQ_common_conf.h>
#include "CMESOAPClientImpl.h"

#include <TsStreamer.h>

namespace EventGateway{
namespace CMESOAPHelper{
class PlaylistEventSinkI : public ::TianShanIce::Streamer::PlaylistEventSink
{
public:
    explicit PlaylistEventSinkI(ZQ::common::Log &log, ClientImpl&);
    virtual ~PlaylistEventSinkI();
    virtual void ping(::Ice::Long timestamp, const ::Ice::Current&);
    virtual void OnItemStepped(const std::string& proxy, const std::string& playlistId, Ice::Int currentUserCtrlNum, Ice::Int prevUserCtrlNum, const TianShanIce::Properties& ItemProps, const ::Ice::Current&) const;
private:
    ZQ::common::Log &_log;
    ClientImpl &_client;
};

} // namespace CMESOAPHelper
} // namespace EventGateway
#endif

