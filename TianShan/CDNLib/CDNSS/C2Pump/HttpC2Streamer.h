#ifndef __ZQ_CDN_C2HttpStreamer_H__
#define __ZQ_CDN_C2HttpStreamer_H__

#include "C2StreamerLib.h"
#include "CdnSessionScan.h"
#include "C2EventUpdater.h"
namespace ZQ {
namespace StreamService {
    class CdnStreamerManager;
}}
namespace C2Streamer {
class UpdateHandlerFactory;
class HttpC2StreamerEnv {
public:
    HttpC2StreamerEnv();
    bool start();
    void stop();

    ZQ::common::Log* getLogger() {
        return logger;
    }

    ZQ::common::Log* logger;
    std::string endpoint;

    ZQ::StreamService::CdnStreamerManager* mgr;
private:
    ZQ::StreamService::CdnSessionScaner* sessScaner;
    C2EventUpdater* eventUpdater;
    ZQHttp::Engine* statusReportSvr;
    UpdateHandlerFactory* handlerFac;
};
extern HttpC2StreamerEnv* getEnvironment( );
};
#endif