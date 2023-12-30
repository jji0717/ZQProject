#ifndef __CMESOAP_StreamEventSinkImpl_H__
#define __CMESOAP_StreamEventSinkImpl_H__
#include <ZQ_common_conf.h>
#include "CMESOAPClientImpl.h"

#include <TsStreamer.h>

namespace EventGateway{
namespace CMESOAPHelper{
class StreamEventSinkI : public ::TianShanIce::Streamer::StreamEventSink
{
public:
    explicit StreamEventSinkI(ZQ::common::Log &log, ClientImpl&);
    virtual ~StreamEventSinkI();
    virtual void ping(::Ice::Long timestamp, const ::Ice::Current&);
    virtual void OnEndOfStream(const std::string &proxy, const std::string &id, const TianShanIce::Properties &, const ::Ice::Current&) const;
    virtual void OnBeginningOfStream(const std::string &proxy, const std::string &id, const TianShanIce::Properties &, const ::Ice::Current&) const;
    virtual void OnStateChanged(const std::string &proxy, const std::string &id, TianShanIce::Streamer::StreamState prevState, TianShanIce::Streamer::StreamState currentState, const TianShanIce::Properties &, const ::Ice::Current&) const;
    virtual void OnSpeedChanged(const std::string &proxy, const std::string &id, Ice::Float prevSpeed, Ice::Float currentSpeed, const TianShanIce::Properties &, const ::Ice::Current&) const;
    virtual void OnExit(const std::string &proxy, const std::string &id, Ice::Int exitCode, const std::string &reason, const ::Ice::Current&) const;
    virtual void OnExit2(const std::string &proxy, const std::string &id, Ice::Int exitCode, const std::string &reason ,const ::TianShanIce::Properties& props, const ::Ice::Current&) const;
private:
    ZQ::common::Log &_log;
    ClientImpl &_client;
};

} // namespace CMESOAPHelper
} // namespace EventGateway
#endif

