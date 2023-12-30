// ChannelOnDemandAppImpl.cpp: implementation of the ChannelOnDemandAppImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ChannelOnDemandAppImpl.h"
#include "PurchaseImpl.h"
#include "ChODSvcEnv.h"
#include "CODConfig.h"
#include "urlstr.h"
// include files which stores a lot of mac.ros about rtsp fields
#include "../StreamSmith/RtspRelevant.h"
#include "stroprt.h"
#include <boost/regex.hpp>
#include "SiteDefines.h"


#define LOG_MODULE_NAME			"ChODApp"

#define err_300 300
#define err_301 301
#define err_302 302
#define err_303 303
#define err_304 304
#define err_305 305
#define err_306 306
#define err_307 307
#define err_308 308
#define err_309 309
#define err_310 310
#define err_311 311
#define err_312 312
#define err_313 313
#define err_314 314
#define err_315 315
#define err_317 317
#define err_332 332

using namespace ZQ::common;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace ZQChannelOnDemand {


ChannelOnDemandAppImpl::ChannelOnDemandAppImpl(ChODSvcEnv& env)
	: _env(env)
{
	
}

ChannelOnDemandAppImpl::~ChannelOnDemandAppImpl()
{

}

NS_PREFIX(ChannelOnDemand::ChannelPurchasePrx) ChannelOnDemandAppImpl::createPurchaseByCR(const ::TianShanIce::Properties& prop, const ::TianShanIce::Properties&, const ::Ice::Current& iCur)
{
	ZQTianShan::_IceThrow<TianShanIce::NotImplemented>(glog, "CodApp", 400, "createPurchaseByCR not implemented now");
	NS_PREFIX(ChannelOnDemand::ChannelPurchasePrx) purPrx = NULL;
	return purPrx;
	/*
	//
	// create object
	//
	PurchaseImplPtr pImpl = new PurchaseImpl(_env);
	pImpl->ident.name = IceUtil::generateUUID();
	pImpl->ident.category = ICE_ChannelPurchase;
	pImpl->bNeedSyncChannel = false;
	pImpl->nCreateTime = time(0);

	std::string channelId;
	//
	// get parameter
	//
	{
		::TianShanIce::Properties::const_iterator it = prop.find(ChannelID);
		if (it != prop.end())
		{
			channelId = it->second;
		}	
		
		if (channelId.empty())
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "No channel information while createPurchaseByCR"), channelId.c_str());

			::TianShanIce::InvalidParameter e;
			e.message = std::string("Parameter " ChannelID) + " not found";
			throw e;
		}
		
		pImpl->channelName = channelId;

		it = prop.find(NodeGroupID);
		if (it != prop.end())
		{
			pImpl->nNodeGroupId = atoi(it->second.c_str());	
		}
		else
		{
			pImpl->nNodeGroupId = 0;
		}

		it = prop.find(ClientSessionID);
		if (it != prop.end())
		{
			pImpl->clientSessionId = it->second;
		}

		it = prop.find(HomeID);
		if (it != prop.end())
		{
			pImpl->homeId = it->second;
		}
		it = prop.find(SmartCardID);
		if (it != prop.end())
		{
			pImpl->smardcardId = it->second;
		}
		it = prop.find(MacAddress);
		if (it != prop.end())
		{
			pImpl->macAddress = it->second;
		}
		it = prop.find("providerId");
		if (it != prop.end())
		{
			pImpl->providerId = it->second;
		}
		it = prop.find("providerAssetId");
		if (it != prop.end())
		{
			pImpl->providerAssetId = it->second;
		}

		SYSTEMTIME st;
		GetLocalTime(&st);
		char ss[32];
		sprintf(ss, "%04d%02d%02dT%02d%02d%02d", st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
		pImpl->broadcastTime = ss;

		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Create purchase on channel [%s],node_group_id=%d,"
			"client_session_id=%s,home_id=%s,smartcard_id=%s,mac_address=%s,provider_id=%s,provider_asset_id=%s,broadcast_time=%s"),
			channelId.c_str(), pImpl->nNodeGroupId, pImpl->clientSessionId.c_str(), 
			pImpl->homeId.c_str(), pImpl->smardcardId.c_str(), pImpl->macAddress.c_str(),
			pImpl->providerId.c_str(), pImpl->providerAssetId.c_str(), ss);
	}

	//
	// check the channel if exist
	//
	{		
		std::vector<Ice::Identity> idents1;
		idents1 = _env._idxChannelNameIndex->find(channelId);
		if (idents1.size()==0)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "channel name [%s] not found"), channelId.c_str());
			::TianShanIce::InvalidParameter e;
			e.message = std::string("channel name [") + channelId + "] not found";
			throw e;
		}
	}

	//
	//TODO: Do validating on TODAS
	//
	::TianShanIce::Properties::const_iterator itMap;
	if (_gCODCfg._dwTodasEnable)
	{
		::com::izq::todas::integration::cod::SessionData sd;
		::com::izq::todas::integration::cod::SessionResultData rd;
		
		sd.serverSessionId = "";
		sd.clientSessionId = pImpl->clientSessionId;

		itMap = prop.begin();
		while(itMap != prop.end())
		{
			sd.params[itMap->first] = itMap->second;
			itMap++;
		}

		SYSTEMTIME st;
		GetLocalTime(&st);
		char strTime[48];
		sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d", st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
		sd.params["ViewBeginTime"] = strTime;


		//dump the values
		{
			std::map<std::string, std::string>::iterator ita = sd.params.begin();
			while(ita!=sd.params.end())
			{
				glog(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "TODSetup [%s] = [%s]"), ita->first.c_str(), ita->second.c_str());
				ita++;
			}
		}

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "Calling TODAS sessionSetup for Client session %s ..."), pImpl->clientSessionId.c_str());
		
		try
		{
			rd = _env._todasPrx->sessionSetup(sd);
		}
		catch(::com::izq::todas::integration::cod::TodasException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ClientSession[%s] setup failed with TOD error code[%s], %s"), 
				pImpl->clientSessionId.c_str(), ex.errorCode.c_str(), ex.errorDescription.c_str());

			TianShanIce::ServerError ex1;
			ex1.message = "Client session setup Caught TODAS Exception: " + ex.errorCode + "," + ex.errorDescription;
			throw ex1;
		}
		catch(Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ClientSession[%s] setup failed with error: caught Ice Exception, %s"), 
				pImpl->clientSessionId.c_str(), ex.ice_name().c_str());

			TianShanIce::ServerError ex1;
			ex1.message = "Client session setup caught Ice Exception: " + ex.ice_name();
			throw ex1;
		}
		catch(...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ClientSession[%s] setup failed with error: caught unknown exception"), 
				pImpl->clientSessionId.c_str());

			TianShanIce::ServerError ex1;
			ex1.message = "Client session setup caught unknown exception";
			throw ex1;
		}

		if (rd.status == 2)
		{
			// failed
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ClientSession[%s] setup failed with TODAS error code: %s"), 
				pImpl->clientSessionId.c_str(), rd.errorCode.c_str());

			TianShanIce::ServerError ex1;
			ex1.message = "Client session setup failed with TOD error code: " + rd.errorCode;
			throw ex1;					
		}

		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "ClientSession[%s] TODAS sessionSetup ok"),
			pImpl->clientSessionId.c_str());
	}


	::Ice::ObjectPrx prx;
	try
	{
		LockT<RecMutex> lk(_env._evitPurLock);
		prx = _env._evitPurchase->add(pImpl, pImpl->ident);
	}
	catch(::Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught DatabaseException, %s"), ex.message.c_str());

		TianShanIce::ServerError ex1;
		ex1.message = "Caught DatabaseException: " + ex.message;
		throw ex1;
	}
	catch(::Freeze::EvictorDeactivatedException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught EvictorDeactivatedException, %s"),
			ex.ice_name().c_str());

		TianShanIce::ServerError ex1;
		ex1.message = "Caught EvictorDeactivatedException: " + ex.ice_name();
		throw ex1;
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught Ice Exception, %s"), 
			ex.ice_name().c_str());

		TianShanIce::ServerError ex1;
		ex1.message = "Caught Ice Exception: " + ex.ice_name();
		throw ex1;
	}

	if (!prx)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Purchase evictor add failed"));

		::TianShanIce::ServerError e;
		e.message = "Failed to add purchase object";
		throw e;
	}

	return ::ChannelOnDemand::ChannelPurchasePrx::uncheckedCast(prx);
	*/
}

#define CrtPurFmt(_C, _X) CLOGFMT(_C, "[%s] " _X), sessionid.c_str()

::TianShanIce::Application::PurchasePrx ChannelOnDemandAppImpl::createPurchase(const ::TianShanIce::SRM::SessionPrx& weiSession, const ::TianShanIce::Properties&, const ::Ice::Current&)
{
	std::string sessionid;
	try
	{
		sessionid = weiSession->getId();
	}
	catch (Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_300, CrtPurFmt(LOG_MODULE_NAME, "getId() caught %s"), ex.ice_name().c_str());
	}
	
	glog(ZQ::common::Log::L_INFO, CrtPurFmt(LOG_MODULE_NAME, "create purchase enter"));

	// new an object of ChannelPurchase and initialize some of the data members
	PurchaseImplPtr pImpl = new PurchaseImpl(_env);
	pImpl->weiwoo = weiSession;
	pImpl->ident.name = IceUtil::generateUUID();
	pImpl->ident.category = ICE_ChannelPurchase;
	pImpl->bNeedSyncChannel = false;
	pImpl->nCreateTime = time(0);
	pImpl->bInService = false;

	// get private data from weiwoo session
	TianShanIce::ValueMap::const_iterator vMap_itor; // value map iterator
	::TianShanIce::ValueMap privData;
	try
	{
		pImpl->serverSessionId = sessionid;
		privData = weiSession->getPrivateData();
	}
	catch (Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_300, CrtPurFmt(LOG_MODULE_NAME, "getPrivateData() caught %s"), ex.ice_name().c_str());
	}

	// dump the private data
	char szBuf[1024];
	szBuf[sizeof(szBuf) - 1] = '\0';
	snprintf(szBuf, sizeof(szBuf) - 1, CLOGFMT(LOG_MODULE_NAME, "[%s] PrivateData."), sessionid.c_str());
	ZQTianShan::dumpValueMap(privData, szBuf, dumpLine);

	// continue initializing data member of ChannelPurchase according to private data.
	vMap_itor = (privData).find(ClientRequestPrefix ClientSessionID);
	if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
		pImpl->clientSessionId = ((::TianShanIce::Variant)(vMap_itor->second)).strs[0];

	vMap_itor = (privData).find(ClientRequestPrefix NodeGroupID);
	if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
		pImpl->nNodeGroupId = atoi(((::TianShanIce::Variant)(vMap_itor->second)).strs[0].c_str());
	else 
		pImpl->nNodeGroupId = 0;

	vMap_itor = (privData).find(ClientRequestPrefix SmartCardID);
	if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
		pImpl->smardcardId = ((::TianShanIce::Variant)(vMap_itor->second)).strs[0];

	vMap_itor = (privData).find(ClientRequestPrefix HomeID);
	if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
		pImpl->homeId = ((::TianShanIce::Variant)(vMap_itor->second)).strs[0];

	vMap_itor = (privData).find(ClientRequestPrefix MacAddress);
	if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
		pImpl->macAddress = ((::TianShanIce::Variant)(vMap_itor->second)).strs[0];

#pragma message(__MSGLOC__"TODO: make sure providerId is right?")
	vMap_itor = (privData).find(ClientRequestPrefix "providerId");
	if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
		pImpl->providerId = ((::TianShanIce::Variant)(vMap_itor->second)).strs[0];

	vMap_itor = (privData).find(ClientRequestPrefix "providerAssetId");
	if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
		pImpl->providerAssetId = ((::TianShanIce::Variant)(vMap_itor->second)).strs[0];

	vMap_itor = (privData).find(PD_KEY_SiteName);
	if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
		pImpl->setProperties(PD_KEY_SiteName, ((::TianShanIce::Variant)(vMap_itor->second)).strs[0]);

	vMap_itor = (privData).find(PD_KEY_Path);
	if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
		pImpl->setProperties(PD_KEY_Path, ((::TianShanIce::Variant)(vMap_itor->second)).strs[0]);

	// get resources from weiwoo session, because url and ContentStore 
	// netId of replica stored here.
	TianShanIce::SRM::ResourceMap rsMap;
	TianShanIce::SRM::ResourceMap::const_iterator rsMap_itor; // resource map iterator
	try
	{
		rsMap = weiSession->getReources();
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_301, CrtPurFmt(LOG_MODULE_NAME, "getResource() caught %s"), ex.ice_name().c_str());
	}

	// get bandwidth 
	__int64 lBandwidth = 0;
	rsMap_itor= rsMap.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
	if (rsMap.end() != rsMap_itor)
	{
		const TianShanIce::SRM::Resource& bandwidthResource = rsMap_itor->second;
		vMap_itor = bandwidthResource.resourceData.find("bandwidth");
		if (bandwidthResource.resourceData.end() != vMap_itor)
		{
			const TianShanIce::Variant& bandwidthVar = vMap_itor->second;
			if (TianShanIce::vtLongs == bandwidthVar.type && bandwidthVar.lints.size() > 0)
				lBandwidth = bandwidthVar.lints[0]; // here bandwidth gained
		}
	}

	// get url string
	std::string strUrl;
	rsMap_itor= rsMap.find(TianShanIce::SRM::rtURI);
	if (rsMap.end() != rsMap_itor)
	{
		const TianShanIce::SRM::Resource& urlResource = rsMap_itor->second;
		vMap_itor = urlResource.resourceData.find("uri");
		if (urlResource.resourceData.end() != vMap_itor)
		{
			const TianShanIce::Variant& urlVar = vMap_itor->second;
			if (TianShanIce::vtStrings == urlVar.type && urlVar.strs.size() > 0)
				strUrl = urlVar.strs[0]; // here url string gained
		}
	}

	pImpl->setProperties(PD_KEY_URL, strUrl);

	// parse url
	ZQ::common::URLStr urlParser(strUrl.c_str(), true);
	std::string pathStr = NULL != urlParser.getPath() ? urlParser.getPath() : "";
	if (0 == stricmp(pathStr.c_str(), "60020000"))
	{
		// if path string equal to "60020000", needn't to authorization on ia component
		// bAuthorize is a data member of class ChannelPurchaseEx which produced by "ChannelPurchaseEx.ICE"
		// bAuthorize's default value is true, which assigned in class PurchaseImpl's construct function.
		pImpl->bAuthorize = false;
	}

	// get on demand name(channel id) from url.
	// there are multi actual channels on different content store associated with 
	// the same on demand name. so you must decide which actual channel you'll to use
	std::vector<Ice::Identity> chlPubIdents;
	const char* pStrChid = urlParser.getVar(ChannelID);
	std::string onDemandName = NULL != pStrChid ? pStrChid : "";
	try
	{
		chlPubIdents = _env._idxChannelNameIndex->find(onDemandName);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, "CodApp", err_302, CrtPurFmt(LOG_MODULE_NAME, "find channle name index caught %s:%s"), 
			ex.ice_name().c_str(), ex.message.c_str());
	}
	if (chlPubIdents.size() == 0)
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, "CodApp", err_303, CrtPurFmt(LOG_MODULE_NAME, "channel [%s] not exist"), onDemandName.c_str());
	}

	// log the channel publish point associated with the given on demand name
	glog(ZQ::common::Log::L_INFO, CrtPurFmt(LOG_MODULE_NAME,"There are %d ChannelPublishPoint associated with onDemandName[%s]"), 
		chlPubIdents.size(), onDemandName.c_str());
	for (uint chlPubNum = 0; chlPubNum < chlPubIdents.size(); chlPubNum ++)
	{
		glog(ZQ::common::Log::L_DEBUG, CrtPurFmt(LOG_MODULE_NAME,"%d. %s:%s"), 
			chlPubNum + 1, chlPubIdents[chlPubNum].category.c_str(), chlPubIdents[chlPubNum].name.c_str());
	}

	// get the ContentStore netId of replica from the session resource[rtStorage]
	rsMap_itor = rsMap.find(TianShanIce::SRM::rtStorage);
	TianShanIce::Variant varNetId; // current weiwoo session's netIds
	if (rsMap_itor != rsMap.end())
	{
		const TianShanIce::SRM::Resource& netIdSrc = rsMap_itor->second;
		vMap_itor = netIdSrc.resourceData.find("NetworkId");
		if (vMap_itor != netIdSrc.resourceData.end())
			varNetId = vMap_itor->second;
	}

	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) chlPubPrx = NULL;
	::TianShanIce::Application::PublishPointPrx pubPrx = NULL;
	std::vector<Ice::Identity> maxchlPubIdents;
	std::vector<Ice::Identity> secondmaxchlPubIdents;
	::Ice::Long maxStartTime = 0;
	::Ice::Long secondmaxStartTime = 0;
	::Ice::Long timeAhead = _config.ItemStartTimeAhead;
	std::vector<Ice::Identity> chlPubIdentCandidates;

	std::vector<Ice::Identity>::iterator chlPubIdents_iter = chlPubIdents.begin();
	for(; chlPubIdents_iter!= chlPubIdents.end(); ++chlPubIdents_iter)
	{
		Ice::Identity chlPubIdentTemp = *chlPubIdents_iter;
		TianShanIce::StrValues itemSequenceOnChlPub;
		bool available = true;
		try
		{	
#ifdef USE_OLD_NS
			chlPubPrx = _env._publisher->open(chlPubIdentTemp.name);
			available = chlPubPrx->ifEnable();
#else

			pubPrx = _env._publisher->open(chlPubIdentTemp.name);
			chlPubPrx = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
			available = chlPubPrx->isAvailableOnDemand();
#endif //USE_OLD_NS
			itemSequenceOnChlPub = chlPubPrx->getItemSequence();
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			ex.ice_throw();
		}
		catch (const TianShanIce::ServerError& ex)
		{
			ex.ice_throw();
		}
		catch (const Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_304, CrtPurFmt(LOG_MODULE_NAME, "open channel [%s] caught %s"), ex.ice_name().c_str());
		}

		if (itemSequenceOnChlPub.size() <= 0 || !available)
		{
			continue;
		}
		NS_PREFIX(ChannelOnDemand::ChannelItemEx) item;
		std::string itemKey;
		try
		{
			if(timeAhead == 0)
			{
				chlPubIdentCandidates.push_back(chlPubIdentTemp);
				continue;
			}
			// find the channel item key for seek and set playoffset
			LockT<RecMutex> lk(_env._dictLock);
			::ChannelOnDemand::ChannelItemDict::iterator dictIt = _env._pChannelItemDict->end();
			::TianShanIce::StrValues::iterator sit;
			sit = itemSequenceOnChlPub.end()-1;
			itemKey = chlPubIdentTemp.name + CHANNELITEM_KEY_SEPARATOR + *sit;
			dictIt = _env._pChannelItemDict->find(itemKey);
			if (_env._pChannelItemDict->end() != dictIt)
				item = dictIt->second;
			if((item.broadcastStart - timeAhead*60*1000) > maxStartTime)
			{
				secondmaxchlPubIdents.clear();
				maxchlPubIdents.clear();
				maxchlPubIdents.push_back(chlPubIdentTemp);
				maxStartTime = item.broadcastStart;		
				secondmaxStartTime = maxStartTime - timeAhead*60*1000;
			}
			else if((item.broadcastStart < maxStartTime) && ((item.broadcastStart + timeAhead*60*1000) >=  maxStartTime))
			{
				if(item.broadcastStart > secondmaxStartTime)
				{
					secondmaxStartTime = item.broadcastStart;
					secondmaxchlPubIdents.clear();
					secondmaxchlPubIdents.push_back(chlPubIdentTemp);
				}
				else if(item.broadcastStart == secondmaxStartTime)
				{
					secondmaxchlPubIdents.push_back(chlPubIdentTemp);
				}
			}
			else if((item.broadcastStart > maxStartTime) && ((item.broadcastStart - timeAhead*60*1000) <= maxStartTime))
			{
				secondmaxchlPubIdents = maxchlPubIdents;
				secondmaxStartTime = maxStartTime;
				maxchlPubIdents.clear();
				maxchlPubIdents.push_back(chlPubIdentTemp);
				maxStartTime = item.broadcastStart;			
			}
			else if(item.broadcastStart == maxStartTime)
			{
				maxchlPubIdents.push_back(chlPubIdentTemp);
			}
			else if(item.broadcastStart == secondmaxStartTime)
			{
				secondmaxchlPubIdents.push_back(chlPubIdentTemp);
			}

		}
		catch (const Freeze::DatabaseException& ex)
		{
			continue;
		}
	}

	if(timeAhead != 0)
	{
		chlPubIdentCandidates.clear();
		for(chlPubIdents_iter=maxchlPubIdents.begin(); chlPubIdents_iter!=maxchlPubIdents.end(); chlPubIdents_iter++)
		{
			chlPubIdentCandidates.push_back(*chlPubIdents_iter);
		}
		for(chlPubIdents_iter=secondmaxchlPubIdents.begin(); chlPubIdents_iter!=secondmaxchlPubIdents.end(); chlPubIdents_iter++)
		{
			chlPubIdentCandidates.push_back(*chlPubIdents_iter);
		}
	}

	if (chlPubIdentCandidates.size() == 0)
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(glog, "CodApp", err_303, CrtPurFmt(LOG_MODULE_NAME, "empty in channel[%s]"), onDemandName.c_str());
	}


	TianShanIce::StrValues netIds; // 存放netIds的交集
	// random pickup an channel publish point to which the purchase will be attached
	int rnd_cur = rand() % chlPubIdentCandidates.size();
	Ice::Identity chlPubIdent = chlPubIdentCandidates[rnd_cur]; // here chlPubIdents.size() > 0
	pImpl->chlPubName = chlPubIdent.name; // save channel publish point's ident.name to purchase's context

	TianShanIce::StrValues netIdsOnChlPub; // channel publish point's netIds
	try
	{
#ifdef USE_OLD_NS
		chlPubPrx = _env._publisher->open(chlPubIdent.name);
#else
		pubPrx = _env._publisher->open(chlPubIdent.name);
		chlPubPrx = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS
		netIdsOnChlPub = chlPubPrx->listReplica();
	}
	catch (const TianShanIce::InvalidParameter& ex)
	{
		ex.ice_throw();
	}
	catch (const TianShanIce::ServerError& ex)
	{
		ex.ice_throw();
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_304, CrtPurFmt(LOG_MODULE_NAME, "open channel [%s] caught %s"), ex.ice_name().c_str());
	}

	// 取channel publish point上的netIds和weiwoo session上的netIds的交集
	// 如果其中一个为空，则取另外一个作为交集的结果
	//////////////////////////////////////////////////////////////////////////
	//	ChannelPub		Weiwoo		Result
	//	empty			4, 5, 6		4, 5, 6
	//	1, 2, 3			empty		1, 2, 3
	//	1, 2, 3			4, 5, 6		empty		throw exception
	//	1, 2, 3			3, 4, 5		3
	//	empty			empty		empty
	//////////////////////////////////////////////////////////////////////////	
	if (netIdsOnChlPub.size())
	{
		netIds = netIdsOnChlPub;
		if (varNetId.strs.size())
		{
		netIds.clear();
		for (uint i = 0; i < varNetId.strs.size(); i ++)
		{
			bool bFound =false;
			for (uint j = 0; j < netIdsOnChlPub.size(); j ++)
			{
			if (netIdsOnChlPub[j] == varNetId.strs[i])
			{
				bFound = true;
				break;
			}
			}
			netIds.push_back(varNetId.strs[i]);
		}
		// 如果两个集合都不为空，但是子集为空，则抛异常
		if (netIds.size())
		{
			ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_305, CrtPurFmt(LOG_MODULE_NAME, "subset on ChannelPub and weiwoo is empty"));
		}
		}
	}
	else // 若channel publish point 上的netIds为空，则交集为weiwoo session上的netIds
		netIds = varNetId.strs;
	// 这里netIds已经存放了channel publish point 的netIds和 weiwoo session 的netIds的交集
	// 接下来还有可能要和todas setup的结果再次取交集

	SYSTEMTIME st;
	GetLocalTime(&st);
	char ss[32];
	sprintf(ss, "%04d%02d%02dT%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	pImpl->broadcastTime = ss;

	// _dwTodasEnable and bAuthorize both are true, call todas setup
	if (_config.authInfo.enable && pImpl->bAuthorize)
	{
		glog(ZQ::common::Log::L_INFO, CrtPurFmt(LOG_MODULE_NAME, "call todas sessionSetup ..."));

		Authorization::AuthorizationParams::iterator itor = _config.authInfo.authorizationParams.find(ENDPOINT);
		if (itor ==  _config.authInfo.authorizationParams.end())
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_MODULE_NAME, err_332, "Authorization [%s] has no [%s] parameter", 
				_config.authInfo.entry.c_str(), ENDPOINT);
		}

		std::string& TODASendpoint = itor->second.value;
		/*add here*/
		for(AppDataPatternMAP::iterator it_authappdata = _config.authAppDataMap.begin();
			it_authappdata != _config.authAppDataMap.end(); ++it_authappdata)
		{
			::TianShanIce::ValueMap::iterator privateItor;

			string AppKey = ClientRequestPrefix + it_authappdata->second.param;
			privateItor =  privData.find(AppKey);

			if(privateItor != privData.end())
			{
				::TianShanIce::Variant var = privateItor->second;
				string MatchStr = var.strs[0];

				boost::regex AppDataRegex(it_authappdata->second.pattern);
				boost::cmatch result;

				if(!boost::regex_match(MatchStr.c_str(), AppDataRegex))
				{
					continue;
				}

				PARAMMAP::iterator itor = it_authappdata->second.appDataParammap.find(ENDPOINT);
				if (itor != it_authappdata->second.appDataParammap.end())
				{
					TODASendpoint = itor->second.value;
					break;
				}	
			}
		}
		pImpl->setProperties(TODAS_ENDPOINT, TODASendpoint);
		glog(ZQ::common::Log::L_INFO, CrtPurFmt(LOG_MODULE_NAME, 
			"Get authorize  endpoint is  [%s]"), TODASendpoint.c_str());
		

		::com::izq::todas::integration::cod::SessionData sd;
		::com::izq::todas::integration::cod::SessionResultData rd;
		sd.serverSessionId = sessionid;
		sd.clientSessionId = pImpl->clientSessionId;
		sd.params[ChannelID] = pImpl->chlPubName; // add channelId, because channelId is not in private data map.
		vMap_itor = privData.begin();
		while(vMap_itor != privData.end()) // attention please, here keys may has "ClientRequest#" prefix, so you must omit them.
		{
			if(((::TianShanIce::Variant)(vMap_itor->second)).strs.size()<=0)
				continue;
			std::string keyStr = vMap_itor->first;
			if (strcmp(::COD::String::nLeftStr(keyStr, strlen(ClientRequestPrefix)).c_str(), ClientRequestPrefix) == 0)
			{// if key has prefix of "ClientRequest#", omit it.
				keyStr = ::COD::String::getRightStr(keyStr, "#", true);
			}
			// use insert can avoid overwritting the existing key-value
			sd.params.insert(::com::izq::todas::integration::cod::Properties::value_type(keyStr, ((::TianShanIce::Variant)(vMap_itor->second)).strs[0]));
			vMap_itor ++;
		}

		SYSTEMTIME st;
		GetLocalTime(&st);
		char strTime[48];
		sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d", st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
		sd.params["ViewBeginTime"] = strTime;

		//full URI of the request;AppPath;Virtual Site name
		TianShanIce::Properties prop = pImpl->getProperties();
		TianShanIce::Properties::iterator pit = prop.find(PD_KEY_SiteName);
		if(pit != prop.end())
		{
			sd.params[PD_KEY_SiteName] = pit->second;
		}
		else sd.params[PD_KEY_SiteName] = "";

		pit = prop.find(PD_KEY_Path);
		if(pit != prop.end())
		{
			sd.params[PD_KEY_Path] = pit->second;
		}
		else sd.params[PD_KEY_Path] = "";

		pit =prop.find(PD_KEY_URL);
		if(pit != prop.end())
		{
			sd.params[PD_KEY_URL] = pit->second;
		}
		else sd.params[PD_KEY_URL] = "";

		for (::com::izq::todas::integration::cod::Properties::iterator sd_itor = sd.params.begin(); sd_itor != sd.params.end(); sd_itor ++)
		{
			glog(ZQ::common::Log::L_DEBUG, CrtPurFmt(LOG_MODULE_NAME, "%s: %s"), sd_itor->first.c_str(), sd_itor->second.c_str());
		}

		::com::izq::todas::integration::cod::TodasForCodPrx		_todasPrx;
		try
		{
			_todasPrx = ::com::izq::todas::integration::cod::TodasForCodPrx::checkedCast(_env._communicator->stringToProxy(TODASendpoint));
			rd = _todasPrx->sessionSetup(sd);
			glog(ZQ::common::Log::L_INFO, CrtPurFmt(LOG_MODULE_NAME, "todas sessionSetup OK"));
		}
		catch (::com::izq::todas::integration::cod::TodasException& ex)
		{
			ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_306, CrtPurFmt(LOG_MODULE_NAME, "todas sessionSetup caught %s:%s"), ex.errorCode.c_str(), ex.errorDescription.c_str());
		}
		catch (Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_307, CrtPurFmt(LOG_MODULE_NAME, "todas sessionSetup caught %s"), ex.ice_name().c_str());
		}
		catch(...)
		{
			ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_315, CrtPurFmt(LOG_MODULE_NAME, "todas sessionSetup caught unknown exception"));
		}
		if (rd.status == 2)
		{
			ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_308, CrtPurFmt(LOG_MODULE_NAME, "todas sessionSetup returned with todas error code: %s"), rd.errorCode.c_str());
		}
	}

	// add rtStorage resource
	try
	{
		::TianShanIce::ValueMap netIdsMap;
		::TianShanIce::Variant netIdsVar;
		netIdsMap.clear();
		netIdsVar.bRange = false;
		netIdsVar.type = ::TianShanIce::vtStrings;
		netIdsVar.strs.clear();
		for (uint cur = 0; cur < netIds.size(); cur ++)
		{
			netIdsVar.strs.push_back(netIds[cur]);
			glog(ZQ::common::Log::L_DEBUG, CrtPurFmt(LOG_MODULE_NAME, "restrict purchase on netId [%s]"), 
				netIds[cur].c_str());
		}
		netIdsMap["NetworkId"] = netIdsVar;
		if (netIds.size()) // 只有当交集不为空时，才addResource
			weiSession->addResource(::TianShanIce::SRM::rtStorage, ::TianShanIce::SRM::raMandatoryNonNegotiable, netIdsMap);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, ex.category.c_str(), ex.errorCode, CrtPurFmt(LOG_MODULE_NAME, "addResource() caught %s:%s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_309, CrtPurFmt(LOG_MODULE_NAME, "addResource() caught %s"), ex.ice_name().c_str());
	}

	// add service group resource
	try
	{
		::TianShanIce::ValueMap ServiceGroupMap;
		::TianShanIce::Variant ServiceGroupVar;
		ServiceGroupMap.clear();
		ServiceGroupVar.bRange = false;
		ServiceGroupVar.type = ::TianShanIce::vtInts;
		ServiceGroupVar.ints.clear();
		ServiceGroupVar.ints.push_back(pImpl->nNodeGroupId);
		ServiceGroupMap["id"] = ServiceGroupVar;
		weiSession->addResource(::TianShanIce::SRM::rtServiceGroup, ::TianShanIce::SRM::raMandatoryNonNegotiable, ServiceGroupMap);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, ex.category.c_str(), ex.errorCode, CrtPurFmt(LOG_MODULE_NAME, "addResource() caught %s:%s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_310, CrtPurFmt(LOG_MODULE_NAME, "addResource() caught %s"), ex.ice_name().c_str());
	}

	// add bandwidth resource
	try
	{
		int nBandWidth = 0;
#ifdef USE_OLD_NS
		::ChannelOnDemand::ChannelPublishPointPrx pubPointPrx = NULL;
#else
		::TianShanIce::Application::PublishPointPrx pubPointPrx = NULL;
#endif //USE_OLD_NS
		pubPointPrx = _env._publisher->open(pImpl->chlPubName);
		nBandWidth = pubPointPrx->getMaxBitrate();
		if(lBandwidth > 0 && lBandwidth < nBandWidth)
		{
			glog(ZQ::common::Log::L_WARNING, CrtPurFmt(LOG_MODULE_NAME,"bandwidth(%lld) already in the session resource, but it is smaller than the expected values(%d)"), 
			lBandwidth, nBandWidth);
		}
		if(lBandwidth <= 0)
		{
			::TianShanIce::ValueMap bandWidthMap;
			::TianShanIce::Variant bandWidthVar;
			bandWidthMap.clear();
			bandWidthVar.bRange =false;
			bandWidthVar.type = ::TianShanIce::vtLongs;
			bandWidthVar.lints.clear();
			bandWidthVar.lints.push_back(nBandWidth);
			bandWidthMap["bandwidth"] = bandWidthVar;
			weiSession->addResource(::TianShanIce::SRM::rtTsDownstreamBandwidth, ::TianShanIce::SRM::raMandatoryNonNegotiable, bandWidthMap);
		}
	}
	catch (const TianShanIce::BaseException& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, ex.category.c_str(), ex.errorCode, CrtPurFmt(LOG_MODULE_NAME, "addResource() caught %s:%s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_311, CrtPurFmt(LOG_MODULE_NAME, "addResource() caught %s"), ex.ice_name().c_str());
	}

	// DO: Add purchase to evictor
	::Ice::ObjectPrx objPrx = NULL;
	try
	{
//		LockT<RecMutex> lk(_env._evitPurLock);
		objPrx = _env._evitPurchase->add(pImpl, pImpl->ident);
	}
	catch(::Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_312, CrtPurFmt(LOG_MODULE_NAME, "save purchase caught %s:%s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_313, CrtPurFmt(LOG_MODULE_NAME, "save purchase caught %s"), ex.ice_name().c_str());
	}
	::TianShanIce::Application::PurchasePrx purchasePrx = NULL;
	purchasePrx = ::TianShanIce::Application::PurchasePrx::uncheckedCast(objPrx);
	if (!purchasePrx)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(glog, "CodApp", err_314, CrtPurFmt(LOG_MODULE_NAME, "result purchase is null"));
	}

	// add purchase to watch dog.
	_env._pWatchDog->watch(pImpl->ident.name, _config.purchaseTimeout * 1000);

	glog(ZQ::common::Log::L_INFO, CrtPurFmt(LOG_MODULE_NAME, "purchase [%s] created, client session [%s], channel publish point [%s]"), 
		pImpl->ident.name.c_str(), pImpl->clientSessionId.c_str(), pImpl->chlPubName.c_str());

	return purchasePrx;
}

::std::string ChannelOnDemandAppImpl::getAdminUri(const ::Ice::Current& )
{
	throw TianShanIce::NotImplemented();
}

::TianShanIce::State ChannelOnDemandAppImpl::getState(const ::Ice::Current& )
{	
	return ::TianShanIce::stInService;
}

}