#include "BcastMRTProxy.h"
#include "BroadCastChannelEnv.h"
#define BcastProxyLog(_C, _X) CLOGFMT(_C, "[%s] " _X), streamId.c_str()
namespace ZQBroadCastChannel
{ 
	BcastMRTProxy::BcastMRTProxy(BroadCastChannelEnv& env, Ice::ObjectAdapterPtr& Adapter,const std::string& dbPath ,const std::string& netId,const std::vector<std::string>& streamers, const StreamNetIDToMRTEndpoints& mrtEndpintInfos, ZQ::common::Log& log,int target )
		:_env(env), MRTProxy(Adapter,dbPath,netId,streamers,mrtEndpintInfos, log,target)
	{
	}

	BcastMRTProxy::~BcastMRTProxy(void)
	{
	}
	bool BcastMRTProxy::findMRTSetupInfoByStreamId(const std::string& streamId, std::string& aesstName, std::string& destIp, int& destPort,
		int& bitrate, std::string& srmSessionId, std::string& streamNetId)
	{
//		glog(ZQ::common::Log::L_DEBUG, BcastProxyLog(BcastPublishPointImpl, "findMRTSetupInfoByStreamId() enter"));

		int64 lstart = ZQ::common::TimeUtil::now();

		Ice::Identity idBcastCh;
		idBcastCh.category = ICE_BcastChannelPublishPoint;
		idBcastCh.name = streamId;

		// check if this BcastChannelPublishPoint exist
		TianShanIce::Application::Broadcast::BcastPublishPointExPrx pointPrx;
		try
		{
			if(!_env._evitBcastChannelPublishPoint->hasObject(idBcastCh))
			{
				return false;
			}
			pointPrx = IdentityToObj(BcastPublishPointEx, idBcastCh);

			Ice::ObjectPrx objPrx;
			#if ICE_INT_VERSION / 100 >= 306
				objPrx      =  pointPrx->ice_collocationOptimized(false);
			#else
				objPrx      =  pointPrx->ice_collocationOptimization(false);
			#endif
			pointPrx =  TianShanIce::Application::Broadcast::BcastPublishPointExPrx::uncheckedCast(objPrx);
		}
		catch(::Freeze::DatabaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSetupInfoByStreamId() find bacstpublishpoint caught Freeze::DatabaseException[%s]"), ex.ice_name().c_str());
			return false;
		}
		catch(Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSetupInfoByStreamId() find bacstpublishpoint caught Ice::Exception [%s]"), ex.ice_name().c_str());
			return false;
		}

		try
		{
			TianShanIce::SRM::SessionPrx session = pointPrx->getSession();
			if(NULL == session)
			{
				return false;
			}

			TianShanIce::Properties properties;
			properties = pointPrx->getProperties();

			///get aesstName info, ssm_livech 将SOAP_AssetID放入BcastpublicPoint的properties中了 
			if(properties.find(SOAP_AssetID) == properties.end())
			{
				glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSetupInfoByStreamId() failed to find asset info"));
				return false;
			}
			aesstName = properties[SOAP_AssetID];

			///get bitrate info
			//bitrate = pointPrx->getMaxBitrate();

			TianShanIce::SRM::ResourceMap resources;
			TianShanIce::ValueMap privateMap;

			resources = session->getReources();

			///get srmSessionId info
			srmSessionId =  session->getId();

			///get bitrate info
			TianShanIce::SRM::ResourceMap::iterator itRes = resources.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
			if(itRes == resources.end()
				|| resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData.find("bandwidth") == resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData.end()
				|| resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData["bandwidth"].lints.size() < 1)
			{
				glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSetupInfoByStreamId() failed to find resource TianShanIce::SRM::rtTsDownstreamBandwidth"));
				return false;
			}
			bitrate = resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData["bandwidth"].lints[0];


			itRes = resources.find(TianShanIce::SRM::rtEthernetInterface);
			if(itRes == resources.end())
			{
				glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSetupInfoByStreamId() failed to find resource TianShanIce::SRM::rtEthernetInterface "));
				return false;
			}
			TianShanIce::ValueMap& rsData = itRes->second.resourceData;

			TianShanIce::ValueMap::iterator itRsData;

			///get destIp info
			itRsData = rsData.find("destIP");
			if(itRsData == rsData.end() || itRsData->second.strs.empty())
			{
				glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSetupInfoByStreamId() failed to find despIp info"));
				return false;
			}
			destIp = itRsData->second.strs[0];

			///get destPort info
			itRsData = rsData.find("destPort");
			if(itRsData == rsData.end() || itRsData->second.ints.empty())
			{
				glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSetupInfoByStreamId() failed to find destPort info"));
				return false;
			}
			destPort = itRsData->second.ints[0];

			///get streamNetId info
			itRes = resources.find(TianShanIce::SRM::rtStreamer);
			if(itRes == resources.end())
			{
				glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSetupInfoByStreamId() failed to find resource TianShanIce::SRM::rtStreamer"));
				return false;
			}
			rsData = itRes->second.resourceData;
			itRsData = rsData.find("NetworkId");
			if(itRsData == rsData.end() || itRsData->second.strs.empty())
			{
				glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSetupInfoByStreamId() failed to find despIp info"));
				return false;
			}
			streamNetId = itRsData->second.strs[0];
		}
		catch(const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSetupInfoByStreamId()  caught ice exception[%s]"), 
				ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSetupInfoByStreamId() caught unknown exception"));
			return false;
		}

		glog(ZQ::common::Log::L_INFO, BcastProxyLog(BcastMRTProxy, "findMRTSetupInfoByStreamId() srmSessionId[%s],AssetName[%s],bitrate[%d]destIp[%s]destPort[%d],streamNetId[%s] took %dms"),
			srmSessionId.c_str(), aesstName.c_str(), bitrate, destIp.c_str(), destPort, streamNetId.c_str(), (int)(ZQ::common::now() - lstart));

		return true;
	}
	bool BcastMRTProxy::findMRTSessionIdByStreamId(const std::string& streamId, std::string& mrtSessionId, std::string& srmSessionId, std::string& streamNetId)
	{
//		glog(ZQ::common::Log::L_DEBUG, BcastProxyLog(BcastPublishPointImpl, "findMRTSessionIdByStreamId() enter"));

		int64 lstart = ZQ::common::TimeUtil::now();

		Ice::Identity idBcastCh;
		idBcastCh.category = ICE_BcastChannelPublishPoint; 
		idBcastCh.name = streamId;

		// check if this BcastChannelPublishPoint exist
		TianShanIce::Application::Broadcast::BcastPublishPointExPrx pointPrx;
		try
		{
			if(!_env._evitBcastChannelPublishPoint->hasObject(idBcastCh))
			{
				return false;
			}
			pointPrx = IdentityToObj(BcastPublishPointEx, idBcastCh);
			Ice::ObjectPrx objPrx;
			#if ICE_INT_VERSION / 100 >= 306
				objPrx      =  pointPrx->ice_collocationOptimized(false);
			#else
				objPrx      =  pointPrx->ice_collocationOptimization(false);
			#endif
			pointPrx =  TianShanIce::Application::Broadcast::BcastPublishPointExPrx::uncheckedCast(objPrx);
		}
		catch(::Freeze::DatabaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSessionIdByStreamId() find bacstpublishpoint caught Freeze::DatabaseException[%s]"), ex.ice_name().c_str());
			return false;
		}
		catch(Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSessionIdByStreamId() find bacstpublishpoint caught Ice::Exception [%s]"), ex.ice_name().c_str());
			return false;
		}

		try
		{
			bool bInService = pointPrx->isInService();
			TianShanIce::SRM::SessionPrx session = pointPrx->getSession();
			if(!bInService || NULL == session)
			{
				glog(ZQ::common::Log::L_INFO, BcastProxyLog(BcastMRTProxy, "updateMRTSessionIdToStore() bcastChannel not inservice"));
				return false;
			}

			///get srmSessionId info
			srmSessionId =  session->getId();

			TianShanIce::Properties properties;
			properties = pointPrx->getProperties();
			if(properties.find(SOAP_SessionID) == properties.end())
			{
				glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSessionIdByStreamId() failed to find mrt sessionId in bcastpublishpoint properties"));
				return false;
			}
			mrtSessionId  = properties[SOAP_SessionID];

			///get streamNetId info
			if(properties.find(SOAP_StreamNetID) == properties.end())
			{
				glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSessionIdByStreamId() failed to find mrt sessionId in bcastpublishpoint properties"));
				return false;
			}
			streamNetId  = properties[SOAP_StreamNetID];
		}
		catch(const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSessionIdByStreamId()  caught ice exception[%s]"), 
				ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "findMRTSessionIdByStreamId() caught unknown exception"));
			return false;
		}

		glog(ZQ::common::Log::L_INFO, BcastProxyLog(BcastMRTProxy, "findMRTSessionIdByStreamId() srmSessionId[%s],mrtSessionId[%s], streamNetId[%s] took %dms"),
			srmSessionId.c_str(), mrtSessionId.c_str(), streamNetId.c_str(), (int)(ZQ::common::now() - lstart));
		return true;
	}
	bool BcastMRTProxy::updateMRTSessionIdToStore(const std::string& streamId, const std::string& mrtSessionId, const std::string& streamNetId)
	{
		int64 lstart = ZQ::common::TimeUtil::now();

		Ice::Identity idBcastCh;
		idBcastCh.category = ICE_BcastChannelPublishPoint;
		idBcastCh.name = streamId;

		// check if this BcastChannelPublishPoint exist
		TianShanIce::Application::Broadcast::BcastPublishPointExPrx pointPrx;
		try
		{
			if(!_env._evitBcastChannelPublishPoint->hasObject(idBcastCh))
			{
				return false;
			}
			pointPrx = IdentityToObj(BcastPublishPointEx, idBcastCh);
			Ice::ObjectPrx objPrx;
			#if ICE_INT_VERSION / 100 >= 306
				objPrx      =  pointPrx->ice_collocationOptimized(false);
			#else
				objPrx      =  pointPrx->ice_collocationOptimization(false);
			#endif
			pointPrx =  TianShanIce::Application::Broadcast::BcastPublishPointExPrx::uncheckedCast(objPrx);
		}
		catch(::Freeze::DatabaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "updateMRTSessionIdToStore() find bacstpublishpoint caught Freeze::DatabaseException[%s]"), ex.ice_name().c_str());
			return false;
		}
		catch(Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "updateMRTSessionIdToStore() find bacstpublishpoint caught Ice::Exception [%s]"), ex.ice_name().c_str());
			return false;
		}

		try
		{
			bool bInService = pointPrx->isInService();
			TianShanIce::SRM::SessionPrx session = pointPrx->getSession();
			if(!bInService || NULL == session)
			{
				glog(ZQ::common::Log::L_INFO, BcastProxyLog(BcastMRTProxy, "updateMRTSessionIdToStore() bcastChannel not inservice"));
				return true;
			}
			TianShanIce::Properties properties;

			properties = pointPrx->getProperties();
			properties[SOAP_SessionID] =  mrtSessionId;
			properties[SOAP_StreamNetID] =  streamNetId;

			pointPrx->setProperties(properties);
		}
		catch(const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "updateMRTSessionIdToStore()  caught ice exception[%s]"), 
				ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, BcastProxyLog(BcastMRTProxy, "updateMRTSessionIdToStore() caught unknown exception"));
			return false;
		}

		glog(ZQ::common::Log::L_INFO, BcastProxyLog(BcastMRTProxy, "updateMRTSessionIdToStore() mrtSessionId[%s], streamNetId[%s] took %dms"),
			  mrtSessionId.c_str(), streamNetId.c_str(), (int)(ZQ::common::now() - lstart));
		return true;
	}

}///end namespace ZQBroadCastChannel
