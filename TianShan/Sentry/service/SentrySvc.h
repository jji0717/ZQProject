#include "ZQDaemon.h"
#include "DiskMonitor.h"

#include "Log.h"
#include "IceLog.h"
#include "TsLayout.h"

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>

#include "../SentryImpl.h"
#include "../SentryEnv.h"
#include "../LogPaserManagement.h"
#include "../NTPSync/NTPClient.h"
#include "../NTPSync/NTPServer.h"
#include "../WebServer.h"

class SentryService : public ZQ::common::ZQDaemon {

public:
	SentryService();
	~SentryService();

public:
	virtual bool OnInit(void);
	virtual bool OnStart(void);
	virtual void OnStop(void);
	virtual void OnUnInit(void);

	virtual bool isHealth(void);
	
private:	

	ZQTianShan::Sentry::SentryEnv*   _pSentryEnv;
	ZqSentryIce::SentryServicePtr	 _nodeService;
	Ice::CommunicatorPtr				ic;
	ZqSentryIce::AdapterCollectorPtr _adapterCollector;
	ZQ::common::Log* _pSvcLog;	
	ZQ::common::Log* _pIceLog;
	ZQ::common::Log* _pHttpLog;
	Ice::LoggerPtr _pIceLogPtr;
	ZQTianShan::Sentry::LogParserManagement* _logparserman;
	WebServer* _websvr;
	ZQ::common::NativeThreadPool* threadpool;
	NTPSync::NTPClient* _pNtpClient;
	NTPSync::NTPServer* _pNtpServer;
	DiskSpaceMonitor* 	_pSpaceMonitor;
	DiskIOMonitor*		_pIOMonitor;
};
