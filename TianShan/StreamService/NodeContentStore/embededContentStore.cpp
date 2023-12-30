
#include <FileLog.h>
#include <NativeThreadPool.h>
#include <TianShanDefines.h>
#include <StreamSmithConfig.h>
#include "vstrmContentStore.h"
#include "embededContentStore.h"
#include "seafileInfo.h"
namespace ZQ
{
namespace StreamSmith
{



ZQTianShan::ContentStore::ContentStoreImpl::Ptr NCSBridge::embedContentStore = NULL;


VHANDLE		NCSBridge::_vstrmHandle = NULL;

ZQ::common::NativeThreadPool EmbededNCSThreadPool;
ZQ::common::FileLog		ncsMainLog ;
ZQ::common::FileLog		ncsEventLog ;

VHANDLE			NCSBridge::getVstrmHandle( )
{
	return _vstrmHandle;
}

bool NCSBridge::mountContentStore( )
{
	if(!embedContentStore)
	{
		return false;
	}
	
	if( gStreamSmithConfig.embededContenStoreCfg.ctntAttr.supportVolume <= 0 )
	{
		embedContentStore->mountStoreVolume( DEFAULT_VOLUME_STRING , "" , true );
	}
	else
	{
		SfuInformation seafileInformation(ncsMainLog);
		SfuInformation::sfVolumeInfos sfInofs;
		if(!seafileInformation.retrieveVolumeInformation(sfInofs))
		{
			ncsMainLog(ZQ::common::Log::L_WARNING,"can't get volume information");
		}
		else
		{
			SfuInformation::sfVolumeInfos::const_iterator itVol = sfInofs.begin();
			for( ; itVol != sfInofs.end() ; itVol++ )
			{
				std::string volLable = (char*)itVol->volumeLabel;
				volLable = volLable + "\\";
				embedContentStore->mountStoreVolume( std::string((char*)itVol->volumeLabel ), volLable, false );
				ncsMainLog(ZQ::common::Log::L_INFO, "MOUNT volume[%s] to [%s]",itVol->volumeLabel,itVol->volumeLabel );
			}
		}
	}
	return true;
}

Ice::ObjectPrx	NCSBridge::StartContentStore(  ZQADAPTER_DECLTYPE& objAdapter , const std::string confFolder ,ZQ::common::Log& log , VHANDLE vstrmHandle )
{
	_vstrmHandle = vstrmHandle;

	
	if( 1 )
	{
		try
		{
			ncsMainLog.open(	GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csMainLogPath.c_str(),
								GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csMainLogLevel,
								GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csMainLogCount,
								GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csMainLogSize);
		}
		catch(const ZQ::common::FileLogException& ex)
		{
			log(ZQ::common::Log::L_ERROR,"can't create CsMain log :%s",ex.what());
			return NULL;
		}
	}
	if( 1 )
	{
		ncsMainLog(ZQ::common::Log::L_INFO, "======================== Start MediaServer ContentStore =====================" );
		try
		{
			ncsEventLog.open(	GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEventLogPath.c_str(),
								GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEventLogLevel,
								GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEventLogCount,
								GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEventLogSize);
		}
		catch(const ZQ::common::FileLogException& ex)
		{
			log(ZQ::common::Log::L_ERROR,"can't create CsEvent log :%s",ex.what());
			return NULL;
		}
	}

	try
	{

		embedContentStore =	new ZQTianShan::ContentStore::vstrmContentStoreImpl(ncsMainLog, 
																			ncsEventLog,
																			EmbededNCSThreadPool, 
																			objAdapter , 
																			GAPPLICATIONCONFIGURATION.szFailOverDatabasePath );
		embedContentStore->_netId		=	GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csStrNetId;
		//embedContentStore->_storeType	=	GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csStrStoreType;
		embedContentStore->_replicaGroupId=	GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csStrReplicaGroupId;
		embedContentStore->_replicaId	=	GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csStrReplicaId;
		embedContentStore->_replicaPriority=GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csIReplicaPriority;
		embedContentStore->_replicaTimeout=	GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csIDefaultReplicaUpdateInterval;
		embedContentStore->_cacheLevel	=	GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csCacheLevel;
		embedContentStore->_cacheable	=	( GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csCacheMode >= 1 );
		embedContentStore->_contentEvictorSize= GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEvictorContentSize;
		embedContentStore->_volumeEvictorSize= GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEvictorVolumeSize;
		
		//Turn on if it is EdgeServer
		embedContentStore->_checkResidentialInFileDeleteEvent = ( gStreamSmithConfig.serverMode == 2 );
		embedContentStore->_timeoutNotProvisioned			=	GAPPLICATIONCONFIGURATION.embededContenStoreCfg.timeoutNotProvisioned;
		embedContentStore->_timeoutIdleProvisioning			=	GAPPLICATIONCONFIGURATION.embededContenStoreCfg.timeoutIdleProvisoning;
		embedContentStore->_edgeMode = embedContentStore->_autoFileSystemSync	=	( gStreamSmithConfig.serverMode == 2 );//turn autoFileSystemSync on in EdgeServer mode
								

		ncsMainLog( ZQ::common::Log::L_INFO , "turn[%s] because mode [%s] EdgeServer",
			embedContentStore->_autoFileSystemSync ? "on":"off",
			(gStreamSmithConfig.serverMode == 2) ? "is" : "isn't");


		embedContentStore->_storeType = "SeaChange.MediaServer";
		if (embedContentStore->_cacheable)
					embedContentStore->_storeType += ".cache";

		if(!embedContentStore->initializeContentStore() )
		{
			ncsMainLog(ZQ::common::Log::L_ERROR,"failed to initialize content store");
			return NULL;
		}


		//add master replica subscriber
		std::string& strMasterStoreReplica = GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csMasterReplicaSubscriber.masterReplicaSubscriber;
		if( !strMasterStoreReplica.empty() )
		{
			if( strMasterStoreReplica.find(":") != std::string::npos )
			{
				embedContentStore->subscribeStoreReplica(  strMasterStoreReplica , true );
			}
			else
			{
				embedContentStore->subscribeStoreReplica(  std::string(SERVICE_NAME_ContentStore) + ":" + strMasterStoreReplica , true );
			}
		}
	}
	catch( const Ice::Exception& ex)
	{
		log( ZQ::common::Log::L_ERROR , "Create ContenStoreImpl failed:%s", ex.ice_name().c_str() );
		return NULL;
	}
	Ice::Identity	Id = objAdapter->getCommunicator()->stringToIdentity(SERVICE_NAME_ContentStore);
	return objAdapter->createProxy( Id );
}

bool NCSBridge::StopContentStore (  ZQ::common::Log& log  )
{	
	ncsMainLog(ZQ::common::Log::L_INFO, "******************************* Stop MediaServer ContentStore *******************************" );
	
	if( embedContentStore != NULL )
	{
		EmbededNCSThreadPool.stop();
		int iCount = 0;
		while( iCount++ < 100 )
		{
			if( EmbededNCSThreadPool.activeCount() <= 1  )
			{
				ncsMainLog(ZQ::common::Log::L_INFO,"all threads are idle, quit waiting");
				break;
			}
			Sleep(100);
			ncsMainLog(ZQ::common::Log::L_INFO,
				"there is some thread busy, just waiting , current running thread count [%d]",
				EmbededNCSThreadPool.activeCount() );
		}

		embedContentStore->unInitializeContentStore();
		embedContentStore = NULL;
	}
// 	ncsMainLog.clear();
// 	ncsEventLog.clear();

// 	if( ncsMainLog )
// 	{
// 		delete ncsMainLog;
// 		ncsMainLog =NULL;
// 	}
// 
// 	if( ncsEventLog )
// 	{
// 		delete ncsEventLog;
// 		ncsEventLog = NULL;
// 	}

	return true;
}


}}//namespace ZQ::StreamSmith
