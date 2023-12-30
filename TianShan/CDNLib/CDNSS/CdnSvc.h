
#ifndef _cdn_streaming_service_wrapper_header_file_h__
#define _cdn_streaming_service_wrapper_header_file_h__

#include <ZQ_common_conf.h>
#include "CdnEnv.h"
#include "CdnStreamerManager.h"
#include <StreamFactory.h>
#include <HttpEngine.h>
#include <IceLog.h>
#include <FileLog.h>
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <snmp/ZQSnmp.h>
#ifdef ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#define ZQ_App_LogDir m_wsLogFolder
#define ZQ_App_SvcName m_sServiceName
#else
#include "ZQDaemon.h"
#include <sys/resource.h>
#define ZQ_App_LogDir _logDir
#define ZQ_App_SvcName getServiceName()
#endif

namespace ZQ
{
namespace StreamService
{

class CdnSSSerice : public ZQ::common::BaseZQServiceApplication
{
public:
	CdnSSSerice( );
	virtual ~CdnSSSerice( );

	// ZQ::Snmp::Subagent* getCdnssSnmp(void)
	// {
	//	return mCdnssSnmp;
	// }

	// ZQ::common::ServiceMIB::Ptr		getCdnssMIB(){ return _pServiceMib; }
protected:

	HRESULT OnInit();
	HRESULT OnStart();
	HRESULT OnStop();
	HRESULT OnUnInit();	
	bool isHealth(void);

	// void	OnSnmpSet(const char *varName);

	virtual void doEnumSnmpExports();
	
protected:

	bool	InitializeIceRunTime( );

	void	UninitializeIceRunTime( );

	bool	initializeCrashDumpLocation( );

	bool	initializeLogger( );

	void	uninitializeLogger( );

	bool	initializeServiceParameter( );

	int		run( );

	std::string	getLoggerFolder( ) const;

	bool	startServer( );

private:
	
	Ice::CommunicatorPtr					mIc;
	ZQ::common::FileLog						mIceFileLogger;
	ZQ::common::FileLog						mSessionLogger;
	ZQ::common::FileLog						mHttpLogger;
	ZQ::common::NativeThreadPool			mThreadPool;
	ZQ::StreamService::CdnSsEnvironment*	mCdnEnv;
	ZQ::StreamService::SsServiceImpl*		mpServiceImpl;
	bool									mbOnStopInvoked;

public:
	uint32	snmpDummyGet() { return 0; }
	void    snmpResetCacheStat(const uint32& );
};

}}

#endif//_cdn_streaming_service_wrapper_header_file_h__
