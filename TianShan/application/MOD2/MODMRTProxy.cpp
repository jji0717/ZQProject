#include "MODMRTProxy.h"
#include "ModService.h"
#include "ModSvcIce.h"

#define MODProxyLog(_C, _X) CLOGFMT(_C, "[%s] " _X), streamId.c_str()
namespace ZQMODApplication
{ 
	MODMRTProxy::MODMRTProxy(ModEnv& env, Ice::ObjectAdapterPtr& Adapter,const std::string& dbPath ,const std::string& netId,const std::vector<std::string>& streamers, const StreamNetIDToMRTEndpoints& mrtEndpintInfos, ZQ::common::Log& log,int target )
		:_env(env), MRTProxy(Adapter,dbPath,netId,streamers,mrtEndpintInfos, log,target)
	{
	}

	MODMRTProxy::~MODMRTProxy(void)
	{
	}
	bool MODMRTProxy::findMRTSetupInfoByStreamId(const std::string& streamId, std::string& AssetName, std::string& destIp, int& destPort,
		int& bitrate, std::string& srmSessionId, std::string& streamNetId)
	{
//		glog(ZQ::common::Log::L_DEBUG, MODProxyLog(MODMRTProxy, "findMRTSetupInfoByStreamId() enter"));

		int64 lstart = ZQ::common::TimeUtil::now();

		Ice::Identity idPurchase;
		idPurchase.category = Servant_ModPurchase;
		idPurchase.name = streamId;

		// check if this BcastChannelPublishPoint exist
		ZQTianShan::Application::MOD::ModPurchasePrx purchasePrx = NULL;
		try
		{
			if(!_env._evctPurchase->hasObject(idPurchase))
			{
				return false;
			}
			purchasePrx = ZQTianShan::Application::MOD::ModPurchasePrx::checkedCast(_env._iceAdap->createProxy(idPurchase));
              Ice::ObjectPrx objPrx;
              #if ICE_INT_VERSION / 100 >= 306
                  objPrx      =  purchasePrx->ice_collocationOptimized(false);
              #else
                 objPrx      =  purchasePrx->ice_collocationOptimization(false);
              #endif


			purchasePrx =  ZQTianShan::Application::MOD::ModPurchasePrx::uncheckedCast(objPrx);
		}
		catch(::Freeze::DatabaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSetupInfoByStreamId() find purchase caught Freeze::DatabaseException[%s]"), ex.ice_name().c_str());
			return false;
		}
		catch(Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSetupInfoByStreamId() find purchase caught Ice::Exception [%s]"), ex.ice_name().c_str());
			return false;
		}

		try
		{
			TianShanIce::ValueMap purPrivData = purchasePrx->getPrivataData();
			TianShanIce::SRM::SessionPrx session = purchasePrx->getSession();
			if(NULL == session)
			{
				return false;
			}

			///get AssetName info
            if(purPrivData.find(SOAP_AssetID) ==  purPrivData.end() || purPrivData[SOAP_AssetID].strs.size() < 1)
			{
				glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSetupInfoByStreamId() failed to find asset info"));
				return false;
			}
			AssetName = purPrivData[SOAP_AssetID].strs[0];

			TianShanIce::SRM::ResourceMap resources;
			TianShanIce::ValueMap::iterator itRsData;

			resources = session->getReources();

			///get srmSessionId info
			srmSessionId =  session->getId();

			///get bitrate info
			TianShanIce::SRM::ResourceMap::iterator itRes = resources.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
			if(itRes == resources.end()
				|| resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData.find("bandwidth") == resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData.end()
				|| resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData["bandwidth"].lints.size() < 1)
			{
				glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSetupInfoByStreamId() failed to find resource TianShanIce::SRM::rtTsDownstreamBandwidth"));
				return false;
			}
			bitrate = resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData["bandwidth"].lints[0];

			itRes = resources.find(TianShanIce::SRM::rtEthernetInterface);
			if(itRes == resources.end())
			{
				glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSetupInfoByStreamId() failed to find resource TianShanIce::SRM::rtEthernetInterface "));
				return false;
			}
			TianShanIce::ValueMap& rsData = itRes->second.resourceData;

			///get destIp info
			itRsData = rsData.find("destIP");
			if(itRsData == rsData.end() || itRsData->second.strs.empty())
			{
				glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSetupInfoByStreamId() failed to find despIp info"));
				return false;
			}
			destIp = itRsData->second.strs[0];

			///get destPort info
			itRsData = rsData.find("destPort");
			if(itRsData == rsData.end() || itRsData->second.ints.empty())
			{
				glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSetupInfoByStreamId() failed to find destPort info"));
				return false;
			}
			destPort = itRsData->second.ints[0];

			///get streamNetId info
			itRes = resources.find(TianShanIce::SRM::rtStreamer);
			if(itRes == resources.end())
			{
				glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSetupInfoByStreamId() failed to find resource TianShanIce::SRM::rtStreamer"));
				return false;
			}
			rsData = itRes->second.resourceData;
			itRsData = rsData.find("NetworkId");
			if(itRsData == rsData.end() || itRsData->second.strs.empty())
			{
				glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSetupInfoByStreamId() failed to find despIp info"));
				return false;
			}
			streamNetId = itRsData->second.strs[0];
		}
		catch(const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSetupInfoByStreamId()  caught ice exception[%s]"), 
				ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSetupInfoByStreamId() caught unknown exception"));
			return false;
		}

		glog(ZQ::common::Log::L_INFO, MODProxyLog(MODMRTProxy, "findMRTSetupInfoByStreamId() srmSessionId[%s],AssetName[%s],bitrate[%d]destIp[%s]destPort[%d],streamNetId[%s] took %dms"),
			srmSessionId.c_str(), AssetName.c_str(), bitrate, destIp.c_str(), destPort, streamNetId.c_str(), (int)(ZQ::common::now() - lstart));

		return true;
	}
	bool MODMRTProxy::findMRTSessionIdByStreamId(const std::string& streamId, std::string& mrtSessionId, std::string& srmSessionId, std::string& streamNetId)
	{
//		glog(ZQ::common::Log::L_DEBUG, MODProxyLog(MODMRTProxy, "findMRTSessionIdByStreamId() enter"));

		int64 lstart = ZQ::common::TimeUtil::now();

		Ice::Identity idPurchase;
		idPurchase.category = Servant_ModPurchase;
		idPurchase.name = streamId;

		// check if this BcastChannelPublishPoint exist
		ZQTianShan::Application::MOD::ModPurchasePrx purchasePrx = NULL;
		try
		{
			if(!_env._evctPurchase->hasObject(idPurchase))
			{
				return false;
			}
			purchasePrx = ZQTianShan::Application::MOD::ModPurchasePrx::checkedCast(_env._iceAdap->createProxy(idPurchase));
              Ice::ObjectPrx objPrx;
              #if ICE_INT_VERSION / 100 >= 306
                  objPrx      =  purchasePrx->ice_collocationOptimized(false);
              #else
                 objPrx      =  purchasePrx->ice_collocationOptimization(false);
              #endif
			purchasePrx =  ZQTianShan::Application::MOD::ModPurchasePrx::uncheckedCast(objPrx);
		}
		catch(::Freeze::DatabaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSessionIdByStreamId() find purchase caught Freeze::DatabaseException[%s]"), ex.ice_name().c_str());
			return false;
		}
		catch(Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSessionIdByStreamId() find purchase caught Ice::Exception [%s]"), ex.ice_name().c_str());
			return false;
		}

		try
		{
			bool bInService = purchasePrx->isInService();
			TianShanIce::SRM::SessionPrx session = purchasePrx->getSession();
			if(!bInService || NULL == session)
			{
//				glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSessionIdByStreamId() stream not playing"));
				return false;
			}

			///get srmSessionId info
			srmSessionId =  session->getId();

			TianShanIce::ValueMap purPrivData = purchasePrx->getPrivataData();

			///get mrtSessionId info
			if(purPrivData.find(SOAP_SessionID) ==  purPrivData.end() || purPrivData[SOAP_SessionID].strs.size() < 1)
			{
				glog(ZQ::common::Log::L_WARNING, MODProxyLog(MODMRTProxy, "findMRTSessionIdByStreamId() failed to find mrt sessionId in purchase privatedata"));
				return false;
			}
			mrtSessionId = purPrivData[SOAP_SessionID].strs[0];

			///get streamNetId info
			if(purPrivData.find(SOAP_StreamNetID) ==  purPrivData.end() || purPrivData[SOAP_StreamNetID].strs.size() < 1)
			{
				glog(ZQ::common::Log::L_WARNING, MODProxyLog(MODMRTProxy, "findMRTSessionIdByStreamId() failed to find mrt sessionId in purchase privatedata"));
				return false;
			}
			streamNetId = purPrivData[SOAP_StreamNetID].strs[0];
		}
		catch(const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSessionIdByStreamId()  caught ice exception[%s]"), 
				ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSessionIdByStreamId() caught unknown exception"));
			return false;
		}

		glog(ZQ::common::Log::L_INFO, MODProxyLog(MODMRTProxy, "findMRTSessionIdByStreamId() srmSessionId[%s],mrtSessionId[%s], streamNetId[%s] took %dms"),
			srmSessionId.c_str(), mrtSessionId.c_str(), streamNetId.c_str(), (int)(ZQ::common::now() - lstart));

		return true;
	}
	bool MODMRTProxy::updateMRTSessionIdToStore(const std::string& streamId, const std::string& mrtSessionId, const std::string& streamNetId)
	{
		int64 lstart = ZQ::common::TimeUtil::now();

		Ice::Identity idPurchase;
		idPurchase.category = Servant_ModPurchase;
		idPurchase.name = streamId;

		// check if this BcastChannelPublishPoint exist
		ZQTianShan::Application::MOD::ModPurchasePrx purchasePrx = NULL;
		try
		{
			if(!_env._evctPurchase->hasObject(idPurchase))
			{
				return false;
			}
			purchasePrx = ZQTianShan::Application::MOD::ModPurchasePrx::checkedCast(_env._iceAdap->createProxy(idPurchase));
              Ice::ObjectPrx objPrx;
              #if ICE_INT_VERSION / 100 >= 306
                  objPrx      =  purchasePrx->ice_collocationOptimized(false);
              #else
                 objPrx      =  purchasePrx->ice_collocationOptimization(false);
              #endif
			purchasePrx =  ZQTianShan::Application::MOD::ModPurchasePrx::uncheckedCast(objPrx);
		}
		catch(::Freeze::DatabaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "updateMRTSessionIdToStore() find purchase caught Freeze::DatabaseException[%s]"), ex.ice_name().c_str());
			return false;
		}
		catch(Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "updateMRTSessionIdToStore() find purchase caught Ice::Exception [%s]"), ex.ice_name().c_str());
			return false;
		}

		try
		{
			bool bInService = purchasePrx->isInService();
			TianShanIce::SRM::SessionPrx session = purchasePrx->getSession();
			if(!bInService || NULL == session)
			{
				glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "findMRTSessionIdByStreamId() stream not playing"));
				return false;
			}

			TianShanIce::ValueMap purPrivData = purchasePrx->getPrivataData();

			TianShanIce::Variant varMRTSessionId;
			varMRTSessionId.bRange = false;
			varMRTSessionId.type = TianShanIce::vtStrings;
			varMRTSessionId.strs.push_back(mrtSessionId);
			purchasePrx->setPrivateData(SOAP_SessionID, varMRTSessionId);

			TianShanIce::Variant varStreamNetId;
			varStreamNetId.bRange = false;
			varStreamNetId.type = TianShanIce::vtStrings;
			varStreamNetId.strs.push_back(streamNetId);	
			purchasePrx->setPrivateData(SOAP_StreamNetID, varStreamNetId);

			//MAPSET(TianShanIce::ValueMap, purPrivData, SOAP_SessionID, varMRTSessionId);
			//MAPSET(TianShanIce::ValueMap, purPrivData, SOAP_StreamNetID, varStreamNetId);
			//purchasePrx->setPrivateData2(purPrivData);
		}
		catch(const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "updateMRTSessionIdToStore()  caught ice exception[%s]"), 
				ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, MODProxyLog(MODMRTProxy, "updateMRTSessionIdToStore() caught unknown exception"));
			return false;
		}

		glog(ZQ::common::Log::L_INFO, MODProxyLog(MODMRTProxy, "updateMRTSessionIdToStore() mrtSessionId[%s], streamNetId[%s] took %dms"),
			  mrtSessionId.c_str(), streamNetId.c_str(), (int)(ZQ::common::now() - lstart));

		return true;
	}

}///end namespace ZQMODApplication
