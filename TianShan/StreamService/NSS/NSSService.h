#ifndef __NSSService_H__
#define __NSSService_H__

#include "ZQ_common_conf.h"
#ifdef ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#else
#include "ZQDaemon.h"
#endif
#include "FileLog.h"
#include "Ice/Ice.h"
#include "NSSEnv.h"
#include "SsServiceImpl.h"
#include "NGODCSEnv.h"
// #include "snmp/SubAgent.hpp"

class NSSService : public ZQ::common::BaseZQServiceApplication
{
public:
	NSSService();
	~NSSService();

protected:

	HRESULT OnInit();

	HRESULT OnStart();

	HRESULT OnStop();

	HRESULT OnUnInit();	

	void doEnumSnmpExports();

protected:

	bool	InitializeIceRunTime( );

	void	UninitializeIceRunTime( );

	bool	initializeCrashDumpLocation( );

	bool	initializeLogger( );

	void	uninitializeLogger( );

	bool	initializeServiceParameter( );

	std::string	getLoggerFolder( ) const;

private:
	Ice::CommunicatorPtr				mIc;
	ZQ::common::FileLog					mIceFileLogger;
	ZQ::common::FileLog					mSessionLogger;
	ZQTianShan::NGODSS::NSSEnv*			mpNSSEnv;
	ZQ::StreamService::SsServiceImpl*	mpServiceImpl;

	ZQTianShan::ContentStore::NGODCSEnv	mNGODCSEnv;
	ZQADAPTER_DECLTYPE					mainAdapter;
	ZQ::common::NativeThreadPool		mStreamServiceThreadpool;
	ZQ::common::NativeThreadPool		mContentStorethreadpool;
	ZQ::common::NativeThreadPool		_thrdPoolRTSP;

public:
	uint32 getSessionCount()
	{
		if (mpServiceImpl)
			return (uint32) mpServiceImpl->sessionCount();
		return 0;
	}
};

#endif
