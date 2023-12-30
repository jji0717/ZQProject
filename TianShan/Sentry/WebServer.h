#ifndef __ZQ_Sentry_WebServer_H__
#define __ZQ_Sentry_WebServer_H__

#include <ZQ_common_conf.h>
#include <HttpEngine.h>
#include <SentryEnv.h>

//////////////////////////////////////////////////////////////////////////
class WebServer
{
public:
    explicit WebServer(ZQTianShan::Sentry::SentryEnv& env);
    ~WebServer();
public:
    void start();
    void stop();
private:
    ZQTianShan::Sentry::SentryEnv& _env;
    ZQHttp::Engine _engine;

    ZQHttp::IRequestHandlerFactory* _fileReaderFac;
    ZQHttp::IRequestHandlerFactory* _sysFuncFac;
    ZQHttp::IRequestHandlerFactory* _tswlExtFac;
};
#endif // __ZQ_Sentry_WebServer_H__
