#ifndef __ZQ_OpenVBOClient_H__
#define __ZQ_OpenVBOClient_H__
#include <ZQ_common_conf.h>
#include <Log.h>
#include <SsmOpenVBO.h>
#include <Ice/Ice.h>

typedef SsmOpenVBO::StreamersStatisticsWithStamp StreamersInfo;
typedef SsmOpenVBO::ImportChannelsStatisticsWithStamp ImportChannelsInfo;
class OpenVBOClient {
public:
    OpenVBOClient(ZQ::common::Log& log, Ice::CommunicatorPtr comm);
    ~OpenVBOClient();
    // connect to the open vbo server
    bool connect(const std::string& ep);
    void clear();

    bool getStreamers(StreamersInfo& info);
    bool getImportChannels(ImportChannelsInfo& info);
    /// for administration purpose, to enable/disable a streamer
    bool enableStreamers(TianShanIce::StrValues streamerNetIds, bool enable);

    ///reset counters record the setup/remote count
    bool resetCounters();
private:
    ZQ::common::Log& log_;
    Ice::CommunicatorPtr comm_;
    SsmOpenVBO::OpenVBOServantPrx prx_;
};
#endif
