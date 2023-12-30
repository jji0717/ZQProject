#ifndef __ZQ_Sentry_WebServer_H__
#define __ZQ_Sentry_WebServer_H__

#include <ZQ_common_conf.h>
#include "HttpServer.h"
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
	ZQ::eloop::HttpServer _httpSvr;

// 	ZQHttp::IRequestHandlerFactory* _fileReaderFac;
// 	ZQHttp::IRequestHandlerFactory* _sysFuncFac;
// 	ZQHttp::IRequestHandlerFactory* _tswlExtFac;
	ZQ::eloop::HttpBaseApplication::Ptr	_fileReaderFac;
	ZQ::eloop::HttpBaseApplication::Ptr	_sysFuncFac;
	ZQ::eloop::HttpBaseApplication::Ptr	_tswlExtFac;
};
#endif // __ZQ_Sentry_WebServer_H__
