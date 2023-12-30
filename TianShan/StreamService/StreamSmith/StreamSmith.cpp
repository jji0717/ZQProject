// StreamSmith.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "VstrmStreamerManager.h"
#include "StreamSmithEnv.h"
#include <ServantFactory.h>
#include "StreamSmithConfig.h"
#include <embededContentStore.h>
#include <FileLog.h>

#include <memoryDebug.h>

//ZQ::common::Config::Loader<StreamSmithCfg> gStreamSmithConfig;
ZQ::common::Config::Loader<StreamSmithCfg> gStreamSmithConfig("StreamSmith.xml");

using namespace ZQ::StreamService;

ZQ::common::Log	consoleLog(ZQ::common::Log::L_DEBUG);

namespace ZQ{
namespace StreamService{

ZQ::StreamService::SsServiceImpl* pServiceInstance = NULL;
SsEnvironment*	gSsEnvironment = NULL;

}}

class demoAPP : public Ice::Application
{
public:
	virtual int run( int argc, _TCHAR* argv[] )
	{
		char* p = new char[1000];
		StreamSmithEnv	 env;

		ZQ::common::FileLog ssLog;
		ssLog.open("c:\\TianShan\\StreamEx\\logs\\StreamSmith.log",ZQ::common::Log::L_DEBUG,5,50*1024*1024);
		ZQ::common::FileLog scanSessionLog;
		scanSessionLog.open("c:\\TianShan\\StreamEx\\logs\\StreamSmith.sess.log",ZQ::common::Log::L_DEBUG,5,10*1024*1024);

		gStreamSmithConfig.setLogger(&consoleLog);

		gStreamSmithConfig.load("C:\\TianShan\\StreamEx\\streamsmith.xml");

		ZQ::StreamService::gSsEnvironment = &env;

		Ice::CommunicatorPtr ServiceIc = communicator();	//Ice::initialize(argc,argv);
		assert( ServiceIc != NULL );

		env.mainLogger				= &ssLog;
		env.sessLogger				= &scanSessionLog;
		//env.mainAdapter = ServiceIc->createObjectAdapterWithEndpoints("StreamService","default -p 10001");
		env.mainAdapter = ZQADAPTER_CREATE( ServiceIc,"StreamService" ,"default -p 10001", *env.mainLogger);
		assert(env.mainAdapter);

		env.streamsmithConfig.iPreloadTimeInMS		= 5000;
		env.streamsmithConfig.iSupportPlaylist		= LIB_SUPPORT_NORMAL_PLAYLIST;
		env.streamsmithConfig.iPlaylistTimeout		= 60*1000;
		env.streamsmithConfig.iRenewTicketInterval	= 5*1000;
		env.iceCommunicator							= ServiceIc;


		env.mCsPrx	= TianShanIce::Storage::ContentStorePrx::checkedCast(
			ZQ::StreamSmith::NCSBridge::StartContentStore( env.mainAdapter , "" , *env.mainLogger, NULL ));
		ZQ::StreamSmith::NCSBridge::mountContentStore();
		assert( env.mCsPrx != NULL );


		ZQ::StreamService::SsServiceImpl* pSvc = new ZQ::StreamService::SsServiceImpl( &env ,"StreamSmith" );
		pServiceInstance	= pSvc;

		env.mStreamerManager.attachServiceInstance( pSvc );
		env.mStreamerManager.initialize();	
		env.mainScheduler.start();
		env.mSessionScaner.attachServiceInstance( pSvc );
		env.mSessionScaner.start( );
		
		pSvc->strCheckpointPeriod		= "60000";
		pSvc->strDbRecoverFatal			= "1";
		pSvc->strSavePeriod				= "60000";
		pSvc->strSaveSizeTrigger		= "10";
		pSvc->iEvictorStreamSize		= 1000;
		pSvc->bShowDBOperationTimeCost	= true;

		assert(pSvc != NULL );

		IceUtil::Handle<ZQ::StreamService::SsServiceImpl> svc= pSvc;
		env.iceCommunicator->addObjectFactory(new ServantFactory(&env,*pSvc),TianShanIce::Streamer::SsPlaylist::ice_staticId());
		assert(svc != NULL );
		//env.mainAdapter->add(svc,env.iceCommunicator->stringToIdentity("StreamSmith"));
		env.getMainAdapter()->ZQADAPTER_ADD(env.iceCommunicator , svc , "StreamSmith");

		pSvc->start("C:\\TianShan\\StreamEx\\data\\",
					gStreamSmithConfig.szIceStormEndpoint,
					gStreamSmithConfig.spigotReplicaConfig.listenerEndpoint,
					gStreamSmithConfig.szServiceID);

		env.getMainAdapter()->activate();

		ServiceIc->waitForShutdown();

		pSvc->stop();

		ZQ::StreamSmith::NCSBridge::StopContentStore(*env.mainLogger);
		env.mSessionScaner.stop();
		env.mainScheduler.stop();
		env.mStreamerManager.uninitialize();

		env.mainAdapter->deactivate();

		ServiceIc->destroy();

		return 0;
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	REG_DEBUG_NEW
	demoAPP app;
	app.main(argc,argv);
	return 1;	
}

