#include <ZQ_common_conf.h>
#include "OpenVBOClient.h"
OpenVBOClient::OpenVBOClient(ZQ::common::Log& log, Ice::CommunicatorPtr comm)
    :log_(log), comm_(comm) {
}
OpenVBOClient::~OpenVBOClient() {
    clear();
    comm_ = NULL;
}
// connect to the open vbo server
bool OpenVBOClient::connect(const std::string& ep) {
    if(prx_) {
        log_(ZQ::common::Log::L_WARNING, CLOGFMT(OpenVBOClient, "connect() Already connected"));
        return false;
    }
    if(!comm_) {
        log_(ZQ::common::Log::L_WARNING, CLOGFMT(OpenVBOClient, "connect() need communicator"));
        return false;
    }

    try {
        prx_ = SsmOpenVBO::OpenVBOServantPrx::checkedCast(comm_->stringToProxy(ep));
        return true;
    } catch (const Ice::Exception& e) {
        log_(ZQ::common::Log::L_WARNING, CLOGFMT(OpenVBOClient, "connect() got %s when connect to %s"), e.ice_name().c_str(), ep.c_str());
        return false;
    }
}
void OpenVBOClient::clear() {
    prx_ = NULL;
}

bool OpenVBOClient::getStreamers(StreamersInfo& info) {
    if(!prx_) {
        log_(ZQ::common::Log::L_WARNING, CLOGFMT(OpenVBOClient, "getStreamers() Must connect the server first"));
        return false;
    }
    try {
        info = prx_->getStreamerInfos();
        return true;
    } catch (const Ice::Exception& e) {
        log_(ZQ::common::Log::L_WARNING, CLOGFMT(OpenVBOClient, "getStreamers() Got %s"), e.ice_name().c_str());
        return false;
    }
}
bool OpenVBOClient::getImportChannels(ImportChannelsInfo& info) {
    if(!prx_) {
        log_(ZQ::common::Log::L_WARNING, CLOGFMT(OpenVBOClient, "getImportChannels() Must connect the server first"));
        return false;
    }
    try {
        info = prx_->getImportChannelStat();
        return true;
    } catch (const Ice::Exception& e) {
        log_(ZQ::common::Log::L_WARNING, CLOGFMT(OpenVBOClient, "getImportChannels() Got %s"), e.ice_name().c_str());
        return false;
    }
}
/// for administration purpose, to enable/disable a streamer
bool OpenVBOClient::enableStreamers(TianShanIce::StrValues streamerNetIds, bool enable) {
    if(!prx_) {
        log_(ZQ::common::Log::L_WARNING, CLOGFMT(OpenVBOClient, "enableStreamers() Must connect the server first"));
        return false;
    }
    try {
        prx_->enableStreamers(streamerNetIds, enable);
        return true;
    } catch (const Ice::Exception& e) {
        log_(ZQ::common::Log::L_WARNING, CLOGFMT(OpenVBOClient, "enableStreamers() Got %s"), e.ice_name().c_str());
        return false;
    }
}

///reset counters record the setup/remote count
bool OpenVBOClient::resetCounters() {
    if(!prx_) {
        log_(ZQ::common::Log::L_WARNING, CLOGFMT(OpenVBOClient, "resetCounters() Must connect the server first"));
        return false;
    }
    try {
        prx_->resetCounters();
        return true;
    }catch (const Ice::Exception& e) {
        log_(ZQ::common::Log::L_WARNING, CLOGFMT(OpenVBOClient, "resetCounters() Got %s"), e.ice_name().c_str());
    }
    return false;
}
