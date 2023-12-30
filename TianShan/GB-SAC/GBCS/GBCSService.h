#ifndef __GBCSService_H__
#define __GBCSService_H__

#include "ZQ_common_conf.h"
#ifdef ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#else
#include "ZQDaemon.h"
#endif
#include "FileLog.h"
#include "Ice/Ice.h"
#include "GBCSEnv.h"
// #include "snmp/SubAgent.hpp"

class GBCSService : public ZQ::common::BaseZQServiceApplication
{
public:
	GBCSService();
	~GBCSService();

protected:

	HRESULT OnInit();

	HRESULT OnStart();

	HRESULT OnStop();

	HRESULT OnUnInit();	

protected:

	bool	InitializeIceRunTime( );

	void	UninitializeIceRunTime( );

	bool	initializeCrashDumpLocation( );

//	void	uninitializeLogger( );

	bool	initializeServiceParameter( );

	std::string	getLoggerFolder( ) const;

private:
	Ice::CommunicatorPtr				mIc;
	// ZQ::Snmp::Subagent*                 GBCSSnmpAgent;

	ZQTianShan::ContentStore::GBCSEnv	mGBCSEnv;
	ZQADAPTER_DECLTYPE					mainAdapter;
	ZQ::common::NativeThreadPool		mContentStorethreadpool;
};

#endif
