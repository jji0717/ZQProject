
#ifdef ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#else
#include "ZQDaemon.h"
#endif
#include "DiskMonitor.h"

#include "Log.h"
#include <IceLog.h>
#include <TsLayout.h>

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>

#include "../SentryImpl.h"
#include "../SentryEnv.h"
//#include "../HttpYeoman.h"
#include "../LogPaserManagement.h"
#include "../NTPSync/NTPClient.h"
#include "../NTPSync/NTPServer.h"
#include "../WebServer.h"

class SentryService : public ZQ::common::BaseZQServiceApplication
{
public:
	SentryService();
	~SentryService();
	virtual HRESULT OnInit(void);
	virtual HRESULT OnStop(void);
	virtual HRESULT OnStart(void);
	virtual HRESULT OnUnInit(void);	
	virtual bool isHealth(void);
    virtual void doEnumSnmpExports();
    void    refreshModulesTable();
    void    refreshNeighborsTable();
protected:
	
private:	
        //	ConnID								_connID;
        //	ServiceFrm							_serviceFrm;	
	//HttpEnv*							_pHttpEnv;
        //	HttpDialogCreator*					_pDialogCreator;
	WebServer* _websvr;

	::ZqSentryIce::AdapterCollectorPtr	_adapterCollector;
	::ZQTianShan::Sentry::SentryEnv*	_pSentryEnv;
	::ZqSentryIce::SentryServicePtr		_nodeService;
	Ice::CommunicatorPtr				ic;
	ZQ::common::Log*					_pSvcLog;	
	ZQ::common::Log*					_pIceLog;
	ZQ::common::Log*					_pHttpLog;
	Ice::LoggerPtr						_pIceLogPtr;
	::ZQTianShan::Sentry::LogParserManagement*	_logparserman;
	ZQ::common::NativeThreadPool* threadpool;
    NTPSync::NTPClient* _pNtpClient;
    NTPSync::NTPServer* _pNtpServer;
    DiskSpaceMonitor * _pSpaceMonitor;
	DiskIOMonitor* _pIOMonitor;

    // refresh snmp tables
    int64       _lastRefreshTime;
    bool        _bLastRefreshModuleTable;
};
