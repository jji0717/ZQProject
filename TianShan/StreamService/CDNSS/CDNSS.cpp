// CDNSS.cpp : Defines the entry point for the console application.
//

#include <ZQ_common_conf.h>
#include "stdafx.h"
#include "CdnEnv.h"
#include "CdnStreamerManager.h"
#include "CdnSessionScan.h"
#include <ServantFactory.h>
#include <HttpEngine.h>
//#include <embededContentStore.h>
#include <FileLog.h>

namespace ZQ
{
namespace StreamService
{
SsEnvironment* gSsEnvironment = NULL;
}
}



using namespace ZQ::StreamService;

class demoAPP : public Ice::Application
{
public:
	virtual int run( int argc, _TCHAR* argv[] )
	{
		CdnSsEnvironment	 env;
		ZQ::common::FileLog ssLog;
		ssLog.open("c:\\TianShan\\StreamEx\\logs\\Cdn.log",ZQ::common::Log::L_DEBUG,5,50*1024*1024);

		ZQ::common::FileLog scanSessionLog;
		scanSessionLog.open("c:\\TianShan\\StreamEx\\logs\\Cdn.sess.log",ZQ::common::Log::L_DEBUG,5,10*1024*1024);

		ZQ::common::FileLog httpLog;
		httpLog.open("c:\\TianShan\\StreamEx\\logs\\Cdn.http.log",ZQ::common::Log::L_DEBUG,5,10*1024*1024);

// 		gStreamSmithConfig.setLogger(&consoleLog);
// 
// 		gStreamSmithConfig.load("C:\\TianShan\\StreamEx\\streamsmith.xml");

		ZQ::StreamService::gSsEnvironment = &env;

		Ice::CommunicatorPtr ServiceIc = communicator();	//Ice::initialize(argc,argv);
		assert( ServiceIc != NULL );

		env.mainLogger				= &ssLog;
		env.sessLogger				= &scanSessionLog;
		env.mHttpLog				= &httpLog;
		//env.mainAdapter = ServiceIc->createObjectAdapterWithEndpoints("StreamService","default -p 10001");
		env.mainAdapter = ZQADAPTER_CREATE( ServiceIc,"StreamService" ,"default -p 10001", *env.mainLogger);
		assert(env.mainAdapter);

		env.streamsmithConfig.iPreloadTimeInMS		= 5000;
		env.streamsmithConfig.iSupportPlaylist		= LIB_SUPPORT_NORMAL_STREAM;
		env.streamsmithConfig.iPlaylistTimeout		= 60000*1000;
		env.streamsmithConfig.iRenewTicketInterval	= 5*1000;
		env.iceCommunicator							= ServiceIc;

		CDNHttpFactory	httpFactory(&env);
		ZQHttp::Engine e(httpLog);
		e.setEndpoint("0.0.0.0", "5678");
		e.setCapacity( 5 );
		e.registerHandler("/c2cp/transferingresscapacityupdate",&httpFactory);
		e.registerHandler("/c2cp/transferstateupdate",&httpFactory);
		e.registerHandler("/c2cp/transferterresourceupdate",&httpFactory);
		e.start();
		
// 		env.mCsPrx	= TianShanIce::Storage::ContentStorePrx::checkedCast(
// 			ZQ::StreamSmith::NCSBridge::StartContentStore( env.mainAdapter , "" , *env.mainLogger, NULL ));
// 		ZQ::StreamSmith::NCSBridge::mountContentStore();
//		assert( env.mCsPrx != NULL );


		ZQ::StreamService::SsServiceImpl* pSvc = new ZQ::StreamService::SsServiceImpl( &env ,"CdnSS" );
		
		ZQ::StreamService::CdnStreamerManager	streamerManager(&env,*pSvc);
		env.mStreamerManager	=	&streamerManager;
		ZQ::StreamService::CdnSessionScaner sessionScanner(&env,pSvc);
		env.mSessionScaner		=	&sessionScanner;
		streamerManager.startup();
		sessionScanner.start();


// 		env.mStreamerManager.attachServiceInstance( pSvc );
// 		env.mStreamerManager.initialize();	
// 		env.mainScheduler.start();
// 		env.mSessionScaner.attachServiceInstance( pSvc );
// 		env.mSessionScaner.start( );

		pSvc->strCheckpointPeriod		= "5000";
		pSvc->strDbRecoverFatal			= "1";
		pSvc->strSavePeriod				= "5000";
		pSvc->strSaveSizeTrigger		= "1";
		pSvc->iEvictorStreamSize		= 1000;
		pSvc->bShowDBOperationTimeCost	= true;

		assert(pSvc != NULL );

		IceUtil::Handle<ZQ::StreamService::SsServiceImpl> svc= pSvc;
		env.iceCommunicator->addObjectFactory(new ServantFactory(&env,*pSvc),TianShanIce::Streamer::SsPlaylist::ice_staticId());
		assert(svc != NULL );
		//env.mainAdapter->add(svc,env.iceCommunicator->stringToIdentity("StreamSmith"));
		env.getMainAdapter()->ZQADAPTER_ADD(env.iceCommunicator , svc , "CdnSS");

		env.mNetId = "192.168.81.111";
		pSvc->start("C:\\TianShan\\StreamEx\\data\\",
					"tcp -h 192.168.81.118 -p 11000",
					"C2Locator:tcp -h 192.168.81.118 -p 6789",
					"10.15.10.251");

		env.getMainAdapter()->activate();

		env.getMainAdapter()->publishLogger( "C:\\TianShan\\StreamEx\\logs\\cdn.sess.log", "F:\\TianShanTest\\etc\\syntax.xml","CdnSS" ,"FileLog");

		ServiceIc->waitForShutdown();

		pSvc->stop();
		e.stop();

//		ZQ::StreamSmith::NCSBridge::StopContentStore(*env.mainLogger);
// 		env.mSessionScaner.stop();
// 		env.mainScheduler.stop();
// 		env.mStreamerManager.uninitialize();
		sessionScanner.stop();
		streamerManager.shutdown();
		

		env.mainAdapter->deactivate();

		ServiceIc->destroy();

		return 0;
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	demoAPP app;
	app.main(argc,argv);
	return 1;	
}
