#ifndef __GBVSSService_H__
#define __GBVSSService_H__

#include "ZQ_common_conf.h"
#ifdef ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#else
#include "ZQDaemon.h"
#endif
#include "FileLog.h"
#include "Ice/Ice.h"
#include "GBVSSEnv.h"
#include "SsServiceImpl.h"
/* CS
#include "NGODCSEnv.h"
*/
class GBVSSService : public ZQ::common::BaseZQServiceApplication
{
public:
	GBVSSService();
	virtual ~GBVSSService();

protected:

	HRESULT OnInit();

	HRESULT OnStart();

	HRESULT OnStop();

	HRESULT OnUnInit();	

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
	ZQ::StreamService::GBVSSEnv*		_pGBVSSEnv;
	ZQ::StreamService::SsServiceImpl*	mpServiceImpl;

	ZQ::common::NativeThreadPool		_thrdPoolRTSP;

/* CS
	ZQTianShan::ContentStore::NGODCSEnv	mNGODCSEnv;
    */
	ZQADAPTER_DECLTYPE					mainAdapter;
};

#endif
