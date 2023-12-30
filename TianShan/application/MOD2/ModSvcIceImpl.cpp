#include "ModService.h"
#include "ModSvcIceImpl.h"
#include "MODHelperMgr.h"
#include "SiteDefines.h"
#include "ServiceGroupPump.h"
#include "../MODPlugIn/LAMPlayListQuery3.h"
#include "MODDefines.h"
#include "HttpClient.h"

#include <json/json.h>

#undef max
#include <boost/regex.hpp>

#include <algorithm>
#include <functional> 
#include "SystemUtils.h"
extern ZQ::common::Config::Holder<ModCfg> gNewCfg;
namespace ZQMODApplication
{
#define PurchaseFmt(_C, _X) CLOGFMT(_C, "[%s] " _X), ident.name.c_str()
#define LOG_PURCHASE "ModPurchaseImpl"

#define REDUNDANCY_NETIDVOLUME "$RedundancyNetId$/$Volume$"
#define REDUNDANCY_NETID  "$RedundancyNetId$"
#define REDUNDANCY_VOLUME "$Volume$"
#define SKIP_ADS    "skip-ads"

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
//#define err_314 314
#define err_315 315
#define err_316 316
#define err_317 317
#define err_318 318
#define err_319 319
//error define for surf
#define err_320 320
#define err_321 321
#define err_322 322
#define err_323 323
#define err_324 324
#define err_325 325
#define err_326 326
#define err_327 327

#define err_330 330
#define err_331 331
#define err_332 332
#define err_333 333
#define err_334 334

#define err_400 400
#define err_401 401
#define err_402 402

#define err_500 500
#define err_501 501

#define err_600 600
#define err_601 601

#define err_700 700
#define err_701 701
#define err_702 702
#define err_703 703
#define err_704 704

#define AuthorLastError   "authorize.lasterror"
#define GetAELastError    "getaelist.lasterror"

#define JSON_HAS(OBJ, CHILDNAME) (OBJ.isObject() && OBJ.isMember(CHILDNAME)) // for jsoncpp bug that didn't test type in isMember()

const char* oteGetErrorDesc(int nErrorCode)
{
	static const char* szErrors[] = {
		"It is null when status is successful",
		"ERR_PARAMETER_MISSING",
		"ERR_CONNECT_TO_DB_FAILED",
		"ERR_TICKET_NOT_FOUND",
		"ERR_NOT_A_PREVIEW",
		"ERR_ASSET_UNBELONGING_TO_TICKET",
		"ERR_TICKET_EXPIRED",
		"ERR_ASSET_NOT_FOUND",
		"ERR_TICKET_NOT_EFFECTIVE"
	};

	//error code is from -1 - -8
	if (nErrorCode > 0 || nErrorCode < -8)
		return "Unknown error";
	
	return szErrors[0 - nErrorCode];
}

#if  ICE_INT_VERSION / 100 >= 306
OTEForStateCB::OTEForStateCB(const std::string& cltSession): clientSessionId(cltSession){}
void OTEForStateCB::sessionTeardown(const Ice::AsyncResultPtr& r)
{
		::com::izq::ote::tianshan::MoDIceInterfacePrx OTEPrx = ::com::izq::ote::tianshan::MoDIceInterfacePrx::checkedCast(r->getProxy());
		try
		{
				OTEPrx->end_sessionTeardown(r);
		}       
		catch(const Ice::Exception& ex)
		{   
				handleException(ex);
		}       

}
#else
OTEForTeardownCB::OTEForTeardownCB(const std::string& cltSession) : clientSessionId(cltSession)
{
}

void OTEForTeardownCB::ice_response(const ::com::izq::ote::tianshan::SessionResultData& rd)
{
	if (rd.status == "2")
	{
		// failed
		glog(ZQ::common::Log::L_ERROR, CLOGFMT("OTETeardown", "ClientSession[%s] authorization teardown failed with error[%s: %s]"), 
			clientSessionId.c_str(), rd.errorCode.c_str(), oteGetErrorDesc(atoi(rd.errorCode.c_str())));
	}
	else
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT("OTETeardown", "ClientSession[%s] authorization teardown OK"),
			clientSessionId.c_str());
	}
}

void OTEForTeardownCB::ice_exception(const ::Ice::Exception& ext)
{
	try
	{
		ext.ice_throw();
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT("OTETeardown", "ClientSession[%s] sessionTeardown() failed with error: caught Ice Exception, %s"), 
			clientSessionId.c_str(), ex.ice_name().c_str());
	}
}
#endif

//////////////////////////////////////////////////////////////////////////
// implementaion of Mod purchase
//////////////////////////////////////////////////////////////////////////

std::string flag2str( Ice::Long flag )
{
	std::string str;
	if( flag & TianShanIce::Streamer::PLISFlagNoPause )
		str = str + "NoPause(P) ";

	if( flag & TianShanIce::Streamer::PLISFlagNoFF )
		str = str + "NoFF(F) ";

	if( flag & TianShanIce::Streamer::PLISFlagNoRew )
		str = str + "NoRew(R) ";

	if( flag & TianShanIce::Streamer::PLISFlagNoSeek )
		str = str + "NoSeek(S) ";

	// 	if( flag & TianShanIce::Streamer::PLISFlagOnce )
	// 		str = str + "PlayOnce ";
	if( flag & TianShanIce::Streamer::PLISFlagSkipAtFF )
		str = str + "SkipAtFF(K) ";

	if( flag & TianShanIce::Streamer::PLISFlagSkipAtRew )
		str = str + "SkipAtRew(W) ";

	long playtimes = (flag & TianShanIce::Streamer::PLISFlagPlayTimes)>>4;
	if( playtimes > 0 && playtimes < 10)
	{
		char Cplaytimes[2] = "";
		sprintf(Cplaytimes,"%ld",playtimes);
		str = str + "PlayTimes(" + Cplaytimes + ") ";
	}
	return str;
}

ModPurchaseImpl::ModPurchaseImpl(ModEnv& env)
: _env(env), _stampClean(0)
{
	_providerId = "";
}

ModPurchaseImpl::~ModPurchaseImpl()
{
}

::TianShanIce::SRM::SessionPrx ModPurchaseImpl::getSession(const ::Ice::Current& c) const
{
	ServantMutex::Lock lk(*this);
	return weiwoo;
}
bool ModPurchaseImpl::checkProvisionTimeout()
{
	bool bret = false;
	if(_env._IceTimeout > 0  && (ZQTianShan::now() - _lProvisionStart) >  _env._IceTimeout)
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseFmt(ModPurchaseImpl, "provision() ice timeout reached"));
		bret = true;
	}
	return bret;
}
void ModPurchaseImpl::provision(const ::Ice::Current& c)
{
	ServantMutex::Lock lk(*this);
    Ice::Long lStart = _lProvisionStart = ZQTianShan::now();
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "provision() enter"));

	TianShanIce::ValueMap::iterator vMap_itor;
	::TianShanIce::ValueMap privData;
	try
	{
		privData = weiwoo->getPrivateData();
	}
	catch (Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_301, "[%s] getPrivateData() caught %s", ident.name.c_str(), ex.ice_name().c_str());
	}
	char szBuf[1024];
	szBuf[sizeof(szBuf) - 1] = '\0';
	snprintf(szBuf, sizeof(szBuf) - 1, "[%s] PrivateData.", ident.name.c_str());
	ZQTianShan::dumpValueMap(privData, szBuf, dumpLine);

	vMap_itor = (privData).find(ClientRequestPrefix ClientSessionID);
	if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
		clientSessionId = ((::TianShanIce::Variant)(vMap_itor->second)).strs[0];
	else 
		glog(ZQ::common::Log::L_WARNING, PurchaseFmt(ModPurchaseImpl, "no client session found in weiwoo session's private data"));
	if (gNewCfg.serviceGroupPumpCfg.enable)
	{
		std::string smartcardId,nodeGroupId;
		if(nodeGroupPump(privData,smartcardId,nodeGroupId))
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_310, "[%s] test asset, pump node-group-Id[%s] and smartcard-id[%s] to udp server", 
				ident.name.c_str(),nodeGroupId.c_str(),smartcardId.c_str());
		}
	}

	TianShanIce::SRM::ResourceMap rsMap;
	TianShanIce::SRM::ResourceMap::const_iterator rsMap_itor;
	try
	{
		rsMap = weiwoo->getReources();
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_303, "[%s] getReources() caught %s", ident.name.c_str(), ex.ice_name().c_str());
	}

	// get url string from resource map
	std::string strUrl;
	TianShanIce::SRM::Resource urlResource;
	rsMap_itor= rsMap.find(TianShanIce::SRM::rtURI);
	if (rsMap.end() != rsMap_itor)
	{
		urlResource = rsMap_itor->second;
		vMap_itor = urlResource.resourceData.find("uri");
		if (urlResource.resourceData.end() != vMap_itor)
		{
			TianShanIce::Variant urlVar = vMap_itor->second;
			if (TianShanIce::vtStrings == urlVar.type && urlVar.strs.size() > 0)
			{
				strUrl = urlVar.strs[0];
				privData[PD_KEY_URL] = urlVar;
			}
		}
	}
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "url is [%s]"), strUrl.c_str());

	ZQ::common::URLStr urlObj(strUrl.c_str(), false);
	std::string pathStr = (NULL != urlObj.getPath()) ? urlObj.getPath() : "";
    appPath = strUrl;
	//
	// find apppath config;
	bool bResult = false;
	URLPATLIST::iterator pathitor; 
	Config::Holder<Urlpattern> apppathinfo;
	for(pathitor = gNewCfg.urlpattern.begin(); pathitor != gNewCfg.urlpattern.end(); pathitor++)
	{
		boost::regex ApppathRegex((*pathitor).pattern);
		std::string temp = (*pathitor).pattern;
		boost::cmatch result; 
		if(!boost::regex_search(appPath.c_str(), result , ApppathRegex))
		{  
			continue;
		}
		else
		{
			if (result.size() > 2 && result[1].matched && result[2].matched) 
			{
				bResult = true;
				apppathinfo = *pathitor; 
				break;
			}
		}
	}
    
  if(!bResult)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "[%s] No URL pattern is matched for path[%s]"), 
			ident.name.c_str(), strUrl.c_str());

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_317, "[%s] No URL pattern is matched for path[%s]", 
			ident.name.c_str(), strUrl.c_str());
	}
/*	if (0 == stricmp(pathStr.c_str(), "60010000"))
	{
		// if appPath equal to "60010000", needn't authorize
		enableAuthorize = false;
	}*/

	// the following logic is to get asset elements
	assetElements.clear();
	authEndpoint ="";
	//assetProps.clear();
	

	//if pathStr = SerfPath or  playlist entry = "NameFormatter", enableAuthorize= false;
	enableAuthorize = false;

	// if not serfpath and   playlist entry != "NameFormatter" , get asset info and fill it to private data map
	if((stricmp(pathStr.c_str(), SerfAppPath) != 0) 
		&& stricmp(apppathinfo.playlistEntry.c_str(), AquaStorageLibEntry) != 0 
		&& stricmp(apppathinfo.playlistEntry.c_str(), MRTStreamEntry) != 0)
	{
		if(!getAssetInfo(apppathinfo, urlObj, privData))
		{
			return;
		}
		purPrivData = privData;

		enableAuthorize = apppathinfo.Authenable;
	}

	if(checkProvisionTimeout())
		return;
	if (apppathinfo.Authenable && enableAuthorize)
	{
		if(!AuthorOnOTE(apppathinfo, privData))
		{
			return;
		}
	}
	if(checkProvisionTimeout())
		return;

	if (stricmp(pathStr.c_str(), SerfAppPath) == 0)
	{
		if(!getAEForTianShanSurf(apppathinfo, urlObj, privData))
		{
			return;
		}
	}
	else if(stricmp(apppathinfo.playlistEntry.c_str(), AquaStorageLibEntry) == 0)
	{
		if(!getAEFromNameFormatter(apppathinfo, urlObj, privData))
			return;
	}
	else if(stricmp(apppathinfo.playlistEntry.c_str(), MRTStreamEntry) == 0)
	{
		if(!getAEFromMRTStream(apppathinfo, urlObj, privData))
			return;
	}
	else if(stricmp(apppathinfo.playlistEntry.c_str(), IPTVEntry) == 0)
	{
		if(!getAEFromIPTV(apppathinfo, urlObj, privData))
			return;
	}
	else 
	{
		if(!getAEFromLAM(apppathinfo, urlObj, privData))
		{
			return;
		}
	}
	if(checkProvisionTimeout())
		return;

   purPrivData = privData;
	// if no elements is found according to the specified url, throw exception
	if (assetElements.size() == 0)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_310, "[%s] no element is found according to the url [%s]", 
			ident.name.c_str(), strUrl.c_str());
	}
	// iterator all asset element from lam	
	int maxBW = -1;
	for (unsigned int i = 0, count = assetElements.size(); i < count; i ++)
	{
		if (assetElements[i].bandWidth > maxBW)
			maxBW = assetElements[i].bandWidth;
		if(assetElements[i].cueIn == -1)
			assetElements[i].cueIn = 0;
		if(assetElements[i].cueOut == -1)
			assetElements[i].cueOut = 0;
		
		// dump the attributes
		std::string strAdsAttributes = "";
		for (::com::izq::am::facade::servicesForIce::AttributesMap::iterator pItor = assetElements[i].attributes.begin(); pItor != assetElements[i].attributes.end(); pItor ++)
		{
			char temp[512] = "";
			sprintf(temp, "[%s]=[%s],", pItor->first.c_str(), pItor->second.c_str());
			strAdsAttributes += temp;
		}

		// dump the volumes
		std::string strVolumeList = "";
		for (::com::izq::am::facade::servicesForIce::StringCollection::iterator pItor = assetElements[i].volumeList.begin(); pItor != assetElements[i].volumeList.end(); pItor ++)
		{
			char temp[512] = "";
			sprintf(temp, "[%s],", (*pItor).c_str());
			strVolumeList += temp;
		}

		// dump the nasurls
		std::string strnasURL = "";
		for (::com::izq::am::facade::servicesForIce::StringCollection::iterator pItor = assetElements[i].nasUrls.begin(); pItor != assetElements[i].nasUrls.end(); pItor ++)
		{
			char temp[512] = "";
			sprintf(temp, "[%s],", (*pItor).c_str());
			strnasURL += temp;
		}

		glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "AEID[%s], cueIn[%d], cueOut[%d], Bitrate[%d], Attributes[%s], VolumeLists[%s], nasURLs[%s]"), 
			assetElements[i].name.c_str(), 
			assetElements[i].cueIn, 
			assetElements[i].cueOut, 
			assetElements[i].bandWidth,
			strAdsAttributes.c_str(), strVolumeList.c_str(), strnasURL.c_str());
	}
	glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "There are [%d] asset elements, max bandwidth is [%d]"), 
		assetElements.size(), maxBW);

	try
	{
		// add node group resource
		TianShanIce::ValueMap	vmNG;
		TianShanIce::Variant	varNG;
		varNG.type=TianShanIce::vtInts;
		varNG.bRange=false;
		TianShanIce::ValueMap::iterator mItor;
		Ice::Int nodeGroupId;
		mItor = privData.find(ClientRequestPrefix NodeGroupID);
		if (privData.end() == mItor || mItor->second.strs.size() == 0)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_316, "[%s] no node-group-id found", ident.name.c_str());
		}
		nodeGroupId = atoi(mItor->second.strs[0].c_str());
		varNG.ints.push_back(nodeGroupId);
		vmNG["id"] = varNG;
		weiwoo->addResource(::TianShanIce::SRM::rtServiceGroup, TianShanIce::SRM::raMandatoryNonNegotiable, vmNG);
		
		// get Bandwidth string from resource map
		TianShanIce::Variant	varBW;
		TianShanIce::ValueMap	vmBW;
		rsMap_itor= rsMap.find(::TianShanIce::SRM::rtTsDownstreamBandwidth);
		if (rsMap.end() == rsMap_itor)
		{
			if(maxBW <= 0 && gNewCfg.defaultBandWidth > 0)
			{
				glog(ZQ::common::Log::L_WARNING,
					PurchaseFmt(ModPurchaseImpl, "LAM resource bandwidth is [%d] , use configration default bandwidth [%d]"), 
					maxBW, gNewCfg.defaultBandWidth);
				maxBW = gNewCfg.defaultBandWidth;	
			}

			// add bandwidth resource			
			varBW.type=TianShanIce::vtLongs;
			varBW.bRange=false;
			varBW.lints.push_back(maxBW);
			vmBW["bandwidth"] = varBW;
			weiwoo->addResource(::TianShanIce::SRM::rtTsDownstreamBandwidth, TianShanIce::SRM::raMandatoryNonNegotiable, vmBW);
		}
		else
		{
			Ice::Long nBw = 0;
			TianShanIce::SRM::Resource bwResource;
			TianShanIce::ValueMap::iterator vMap_itor;
			bwResource = rsMap_itor->second;
			vMap_itor = bwResource.resourceData.find("bandwidth");

			if (bwResource.resourceData.end() != vMap_itor)
			{
				TianShanIce::Variant urlVar = vMap_itor->second;
				if (TianShanIce::vtStrings == urlVar.type && urlVar.lints.size() > 0)
				{
					nBw = urlVar.lints[0];
				}
			}
			if(maxBW <=0 && nBw <=0 && gNewCfg.defaultBandWidth > 0)
			{
				glog(ZQ::common::Log::L_WARNING,
					PurchaseFmt(ModPurchaseImpl, "Weiwoo and LAM doesn't have bandwidth resource, use configration default bandwidth [%d]"), 
					gNewCfg.defaultBandWidth);

				maxBW = gNewCfg.defaultBandWidth;
			}
			if(maxBW > nBw)
			{
				glog(ZQ::common::Log::L_WARNING,
					PurchaseFmt(ModPurchaseImpl, "Weiwoo Resource bandwidth is [%d], Need max bandwidth is [%d]"), 
					nBw, maxBW);

				varBW.type=TianShanIce::vtLongs;
				varBW.bRange=false;
				varBW.lints.push_back(maxBW);
				vmBW["bandwidth"] = varBW;
				weiwoo->addResource(::TianShanIce::SRM::rtTsDownstreamBandwidth, TianShanIce::SRM::raMandatoryNonNegotiable, vmBW);
			}
		}
			
		// get MacAddress string from resource map
		bool bIsExistsDestMac = false;
		rsMap_itor= rsMap.find(::TianShanIce::SRM::rtEthernetInterface);
		if(rsMap.end() != rsMap_itor)//  find rtEthernetInterface from weiwoo resource
		{
			TianShanIce::SRM::Resource descmacResource;
			TianShanIce::ValueMap::iterator vMap_itor;
			descmacResource = rsMap_itor->second;
			vMap_itor = descmacResource.resourceData.find("destMac");
			
			if (descmacResource.resourceData.end() != vMap_itor)// find mac-address key from weiwoo resource
				bIsExistsDestMac = true;
		}
		if(!bIsExistsDestMac)
		{
			// add MacAddress resource
			mItor = privData.find(ClientRequestPrefix MacAddress);
			if (privData.end() != mItor && mItor->second.strs.size() >= 1)
			{
				std::string strDeviceID = mItor->second.strs[0];
				
				int npos = strDeviceID.find_first_not_of("0123456789abcdefABCDEF");
				if(npos < 0 && strDeviceID.size() == 12) 
					//not find other letter except "0123456789abcdefABCDEF", and accord with mac address
				{	
					TianShanIce::ValueMap vmMacAddr; // dest mac address
					TianShanIce::Variant varMacAddr;
					varMacAddr.type = TianShanIce::vtStrings;
					varMacAddr.bRange = false;
					varMacAddr.strs.push_back(mItor->second.strs[0]);
					vmMacAddr["destMac"] = varMacAddr;
					weiwoo->addResource(TianShanIce::SRM::rtEthernetInterface, TianShanIce::SRM::raMandatoryNonNegotiable, vmMacAddr);									
				}
				else
				{
					glog(ZQ::common::Log::L_WARNING, PurchaseFmt(ModPurchaseImpl, "invalid device-id = %s"),strDeviceID.c_str());						
				}
			}
			else
			{
				glog(ZQ::common::Log::L_WARNING, PurchaseFmt(ModPurchaseImpl, "Can't find device-id from weiwoo private data"));
			}
		}

		if(gNewCfg.allowedStreams.mandatory)
		{
			TianShanIce::SRM::ResourceAttribute resAttribute = TianShanIce::SRM::raMandatoryNonNegotiable;
			TianShanIce::Variant vsStreams;
			TianShanIce::ValueMap vmSreams;
			vsStreams.type = TianShanIce::vtStrings;
			vsStreams.bRange = false;
			vsStreams.strs.clear();
			vsStreams.strs = gNewCfg.allowedStreams.streams;
			vmSreams["NetworkId"] = vsStreams;
			std::string streams;
			for(size_t  i = 0; i < gNewCfg.allowedStreams.streams.size(); ++i)
			{
				streams += gNewCfg.allowedStreams.streams[i];
				streams += ";";
			}
			glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "add NetworkId [%s] to resource of rtStreamer [resAttribute=%d]"),streams.c_str(), resAttribute);
	
			weiwoo->addResource(TianShanIce::SRM::rtStreamer, resAttribute, vmSreams);
		}
	}
	catch (const TianShanIce::BaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_312, "[%s] addResource() caught %s: %s", 
			ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_313, "[%s] addResource() caught %s", 
			ident.name.c_str(), ex.ice_name().c_str());
	}

	// DO: renew the purchase to the config value
	_env._pWatchDog->watch(ident.name, gNewCfg.purchaseTimeout);

	glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "provision() completed took %d ms"), ZQTianShan::now() - lStart);
}

bool ModPurchaseImpl::nodeGroupPump(const TianShanIce::ValueMap& privData,std::string& outSmartCardID,std::string& outNodeGroupID)
{
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "nodeGroupPump() enter"));
	std::string AssetName;
	TianShanIce::ValueMap::const_iterator vMap_itor;
	vMap_itor = (privData).find("ClientRequest#assetUID");
	if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
	{
		AssetName = ((::TianShanIce::Variant)(vMap_itor->second)).strs[0];
		glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, " get ClientRequest#assetUID[%s]"),AssetName.c_str());
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, PurchaseFmt(ModPurchaseImpl, "ClientRequest#assetUID not found in weiwoo session's private data"));
		return false;
	}

	bool bSend  = false;
	std::vector<std::string>::iterator ruleitor;
	for(ruleitor = gNewCfg.serviceGroupPumpCfg.AssetRules.begin();ruleitor != gNewCfg.serviceGroupPumpCfg.AssetRules.end();ruleitor++)
	{
		boost::regex AssetRegex(*ruleitor);
		if(boost::regex_match(AssetName.c_str(),AssetRegex))
		{
			bSend = true;
			break;
		}
	}
	if(bSend)
	{
		std::string smartCardID ,nodeGroupID;
		vMap_itor = (privData).find(ClientRequestPrefix SmartCardID);
		if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
			smartCardID = ((::TianShanIce::Variant)(vMap_itor->second)).strs[0];

		vMap_itor = (privData).find(ClientRequestPrefix NodeGroupID);
		if(vMap_itor != (privData).end() && ((::TianShanIce::Variant)(vMap_itor->second)).strs.size()>0)
			nodeGroupID = ((::TianShanIce::Variant)(vMap_itor->second)).strs[0];
		if (_env._serviceGroupPump != NULL)
		{
			_env._serviceGroupPump->add(nodeGroupID,smartCardID);

	}
		outSmartCardID = smartCardID;
		outNodeGroupID = nodeGroupID;
	return true;
	}
	return false;
}

void ModPurchaseImpl::render(const ::TianShanIce::Streamer::StreamPrx& streamPrx, 
							 const ::TianShanIce::SRM::SessionPrx& weiwooSessPrx, 
							 const ::Ice::Current& c)
{
	ServantMutex::Lock lk(*this);
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "render() enter"));
	
	Ice::Long lStart = ZQTianShan::now();
	std::string volumename ="";
	std::string strStreamproxy ="";
	try
	{
		strStreamproxy = _env._iceComm->proxyToString(streamPrx);
		TianShanIce::SRM::ResourceMap resmap =  weiwooSessPrx->getReources();
		TianShanIce::SRM::ResourceMap::iterator itorResMap;
		itorResMap = resmap.find(TianShanIce::SRM::rtStorage);
		if(itorResMap != resmap.end())
		{
			TianShanIce::SRM::Resource resStorge = itorResMap->second;

			char szBuf[1024];
			memset(szBuf, 0, 1024);
			snprintf(szBuf, sizeof(szBuf) - 1, "[%s]Storage resource of Weiwoo  ",ident.name.c_str());
			ZQTianShan::dumpValueMap(resStorge.resourceData, szBuf, dumpLine);

			::TianShanIce::ValueMap::iterator It_storgeMap;
			It_storgeMap  = resStorge.resourceData.find("NetworkId");
			std::string storgeID;
			if(It_storgeMap != resStorge.resourceData.end() && It_storgeMap->second.strs.size() > 0)
			{
				storgeID =  It_storgeMap->second.strs[0];
				glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "Storage NetID(%s) from Weiwoo resource"), storgeID.c_str());	

				TianShanIce::StrValues::iterator It_volume = volumesLists.begin();
				while(It_volume != volumesLists.end())
				{
					size_t nLen = storgeID.size();
					volumename = *It_volume;
					if(volumename.substr(0, nLen) == storgeID)
					{
						if(volumename.size() == nLen)
						{
							volumename = "";
						}
						else
							volumename = volumename.substr(nLen, volumename.size() - nLen) + "/";
						break;
					}
					else
						volumename = "";
					It_volume++;
				}				
			}	
			glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl,"Storage volume (%s)"), volumename.c_str());
		}
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_700, "[%s]get ticket proxy caught %s", 
			ident.name.c_str(), ex.ice_name().c_str());
	}

	stream = streamPrx;
	TianShanIce::Streamer::PlaylistPrx playlist = NULL;
	try
	{
		streamId = streamPrx->getIdent().name;
		playlist = TianShanIce::Streamer::PlaylistPrx::checkedCast(streamPrx);
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_700, "[%s] get playlist [%s] proxy caught %s", 
			ident.name.c_str(), streamId.c_str(), ex.ice_name().c_str());
	}

	try
	{
		playlist->enableEoT(false);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_701, "[%s] enableEoT() playlist [%s] caught %s: %s", 
			ident.name.c_str(), streamId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_702, "[%s] enableEoT() playlist [%s] caught %s", 
			ident.name.c_str(), streamId.c_str(), ex.ice_name().c_str());
	}

	int playAdOnce = 0;
	// find apppath config;
	URLPATLIST::iterator pathitor; 
	for(pathitor = gNewCfg.urlpattern.begin(); pathitor != gNewCfg.urlpattern.end(); pathitor++)
	{
		boost::regex ApppathRegex((*pathitor).pattern);
		std::string temp = (*pathitor).pattern;
		boost::cmatch result; 
		if(!boost::regex_search(appPath.c_str(), result , ApppathRegex))
		{  
			continue;
		}
		else
		{
			if (result.size() >2 && result[1].matched && result[2].matched) 
			{
				playAdOnce = (*pathitor).plRender.playAdOnce;
				break;
			}
		}
	}

	// iterator all asset element from lam
	std::vector<Ice::Long> primaryNPTs;
	Ice::Long npt =0;
	bool bValidNptSeq = true, bAdsFound = false;
	int  cPrimary =0;
	for (unsigned int i = 0, count = assetElements.size(); i < count; i ++)
	{
		Ice::Int ctrlNum = i + 1;

		Ice::Long itemLen = 0;

		TianShanIce::Properties::iterator itorAttribute;
		itorAttribute = assetElements[i].attributes.find("DURATION");
		Ice::Long duration = 0;
		if(itorAttribute != assetElements[i].attributes.end())
		{
			sscanf(itorAttribute->second.c_str(), "%lld", &duration);
			duration = duration * 1000;
		}

		if(assetElements[i].cueIn  == 0  && assetElements[i].cueOut == 0)
			itemLen = duration;
		else if(assetElements[i].cueOut != 0)
			itemLen = (Ice::Long)(assetElements[i].cueOut - assetElements[i].cueIn);
		else if(assetElements[i].cueIn != 0)
			itemLen = (Ice::Long)(duration - assetElements[i].cueIn);

		TianShanIce::Streamer::PlaylistItemSetupInfo info;
		//info.contentName = assetElements[i].aeUID;
		info.contentName = volumename + assetElements[i].name;
		info.inTimeOffset = assetElements[i].cueIn;
		info.outTimeOffset = assetElements[i].cueOut;
		info.forceNormal = false;
		info.flags = 0;
		info.spliceIn = false;
		info.spliceOut = false;
		info.criticalStart = 0;
		info.privateData.clear();

		if (itemLen <=0)
			bValidNptSeq = false;

		std::string strAttributes = "";
		bool bIsAD = false;
		if (assetElements[i].attributes.size() > 0)
		{
			com::izq::am::facade::servicesForIce::AttributesMap::iterator itor;
			for(itor = assetElements[i].attributes.begin(); itor !=  assetElements[i].attributes.end(); itor++)
			{
				strAttributes += std::string("[") + itor->first + std::string(":") + itor->second + std::string("];");

				com::izq::am::facade::servicesForIce::AttributesMap::iterator itorforAds;
				itorforAds = assetElements[i].attributes.find(ADM_ISAD);
				if (itorforAds != assetElements[i].attributes.end() && itorforAds->second == "1")
				{
					bAdsFound = true;
					bIsAD = true;
				}
				TianShanIce::Variant userVar;
				userVar.bRange = false;
				userVar.type = TianShanIce::vtStrings;
				userVar.strs.push_back(itor->second);
				info.privateData[itor->first] = userVar;
				if(bIsAD)
				{
					if(itor->first == ADM_FORBIDFF)
					{
						if(itor->second == "1")
							info.flags |= TianShanIce::Streamer::PLISFlagNoFF;
					}
					else  if(itor->first == ADM_FORBIDREW)
					{
						if(itor->second == "1")
							info.flags |= TianShanIce::Streamer::PLISFlagNoRew;
					}
					else  if(itor->first == ADM_FORBIDPAUSE)
					{
						if(itor->second == "1")
							info.flags |= TianShanIce::Streamer::PLISFlagNoPause;
					}
					else  if(itor->first == ADM_FORBIDSTOP)
					{
						//if(itor->second == "1")
						//	info.flags |=;
					}
					else  if(itor->first == ADM_FORBIDNOSEEK)
					{
						if(itor->second == "1")
							info.flags |= TianShanIce::Streamer::PLISFlagNoSeek;
					}
					else  if(itor->first == ADM_SKIPATFF)
					{
						if(itor->second == "1")
							info.flags |= TianShanIce::Streamer::PLISFlagSkipAtFF;
					}
					else  if(itor->first == ADM_SKIPATREW)
					{
						if(itor->second == "1")
							info.flags |= TianShanIce::Streamer::PLISFlagSkipAtRew;
					}
				}
			}

			if(bIsAD)
			{
				int adPlayTimes = 0;
				itor = assetElements[i].attributes.find(ADM_PLAYTIMES);
				if(itor != assetElements[i].attributes.end())
				{
					adPlayTimes = atoi(itor->second.c_str());
					if(adPlayTimes > 9)
						adPlayTimes = 9;
					else if(adPlayTimes < 0)
						adPlayTimes = 0;
				}
				else if(playAdOnce)
					adPlayTimes = 1;

				if(adPlayTimes > 0  && adPlayTimes <= 9)
					info.flags |= adPlayTimes << 4;
			}

			glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "render() item[%s] attributes[%s] flags[%s]"), 	
										assetElements[i].name.c_str(), strAttributes.c_str(),flag2str(info.flags).c_str());

		}

		std::string iptvKey = ClientRequestPrefix + std::string("IPTVURL");
		if(purPrivData.find(iptvKey) != purPrivData.end())
		{
			info.privateData["IPTVURL"] =  purPrivData[iptvKey];
		}

		if(!bIsAD)
			cPrimary++;

		if (!bIsAD && (bValidNptSeq || primaryNPTs.empty()))
			primaryNPTs.push_back(npt); 

		npt += itemLen;

		if (!bIsAD && bValidNptSeq)
			primaryNPTs.push_back(npt); 

		try
		{
			playlist->pushBack(ctrlNum, info);
			glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "pushBack %s itemLength [%lld]on stream %s"), 
				info.contentName.c_str(), itemLen, streamId.c_str());
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "pushBack() [%s] on playlist [%s] caught %s: %s"), 
				info.contentName.c_str(), streamId.c_str(), ex.ice_name().c_str(), ex.message.c_str());

			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_703, "pushBack() [%s] on playlist [%s] caught %s: %s", 
				info.contentName.c_str(), streamId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "pushBack() [%s] on playlist [%s] caught %s"), 
				info.contentName.c_str(), streamId.c_str(), ex.ice_name().c_str());

			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_704, "pushBack() [%s] on playlist [%s] caught %s", 
				info.contentName.c_str(), streamId.c_str(), ex.ice_name().c_str());
		}
	}
	try
	{
		TianShanIce::ValueMap externalPD;

		if (bAdsFound && !primaryNPTs.empty())
		{
			Ice::Long primaryStart = primaryNPTs[0];
			Ice::Long primaryEnd = 0;

			::TianShanIce::Variant varPrimaryStart;
			varPrimaryStart.bRange = false;
			varPrimaryStart.type = TianShanIce::vtLongs;
			varPrimaryStart.lints.push_back(primaryStart); //ms
			MAPSET(TianShanIce::ValueMap, externalPD, SYS_PROP(primaryStart), varPrimaryStart);

			if (cPrimary *2 == primaryNPTs.size())
			{
				primaryEnd   = primaryNPTs[primaryNPTs.size()-1];
				TianShanIce::Variant varPrimaryEnd;
				varPrimaryEnd.bRange = false;
				varPrimaryEnd.type = TianShanIce::vtLongs;
				varPrimaryEnd.lints.push_back(primaryEnd); //ms
				MAPSET(TianShanIce::ValueMap, externalPD, SYS_PROP(primaryEnd), varPrimaryEnd);
			}

			glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "render() primaryStart[%lld]primaryEnd[%lld]"), primaryStart, primaryEnd);
		}

		//add the maximal time to live of the purchase
		Ice::Int ForcedExpiration = gNewCfg.maxTTL*1000;
		TianShanIce::Variant varMaxTTL;
		varMaxTTL.bRange = false;
		varMaxTTL.type = TianShanIce::vtInts;
		varMaxTTL.ints.push_back(ForcedExpiration);
		MAPSET(TianShanIce::ValueMap, externalPD, SYS_PROP(overrideTimeout), varMaxTTL);

		//add the remainingPlayTime time of the purchase
		TianShanIce::Variant varRemainingPlayTime;
		varRemainingPlayTime.bRange = false;
		varRemainingPlayTime.type = TianShanIce::vtLongs;
		varRemainingPlayTime.lints.push_back(npt); //ms
		MAPSET(TianShanIce::ValueMap, externalPD, SYS_PROP(fullPlayTime), varRemainingPlayTime);

		weiwoo->setPrivateData2(externalPD);

		glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "render() fullPlayTime[%lld]maxTTL[%d]"), npt, ForcedExpiration);
	}
	catch (const TianShanIce::BaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_312, "[%s] add privatedata() caught %s: %s", 
			ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_313, "[%s] add privatedata() caught %s", 
			ident.name.c_str(), ex.ice_name().c_str());
	}
	// set the purchase into service status
	bInService = true;
	glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "render() completed at stream [%s] took %d ms"), strStreamproxy.c_str(), ZQTianShan::now() - lStart);
}

void ModPurchaseImpl::detach(const ::std::string& sessId, 
			const ::TianShanIce::Properties& prop, 
			const ::Ice::Current& c)
{
	ServantMutex::Lock lk(*this);
	if (_stampClean >0)
		return;
    _stampClean = ZQTianShan::now();
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "detach() enter"));
	std::string purInternalMsg="";
	bool bIspurInternal = false;
	TianShanIce::ValueMap::iterator itorVmap;
	itorVmap = purPrivData.find(AuthorLastError);

	if(itorVmap != purPrivData.end())
	{
		TianShanIce::Variant var;
		var = itorVmap->second;
		purInternalMsg = var.strs[0];
		glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "***Authorize lasterror:[%s]"), var.strs[0].c_str());
	}
	itorVmap = purPrivData.find(GetAELastError);
	if(itorVmap != purPrivData.end())
	{
		TianShanIce::Variant var;
		var = itorVmap->second;
		purInternalMsg = var.strs[0];
		glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "***GetAEList lasterror:[%s]"), var.strs[0].c_str());
	}

	//find the PurchaseTimeoutInternal key, 
	//if find, indicate the purchase timeout from watchDog
	//          fill the authorize and getaelist errormsg to terminateReason
	::TianShanIce::Properties::const_iterator itProp;
	itProp = prop.find( SYS_PROP(PurchaseTimeoutInternal));
	if (itProp != prop.end())
	{
		bIspurInternal = true;
	}

	// find apppath config;
	bool bResult = false;  
	Config::Holder<Urlpattern> apppathinfo;
	URLPATLIST::iterator pathitor; 
	for(pathitor = gNewCfg.urlpattern.begin(); pathitor != gNewCfg.urlpattern.end(); pathitor++)
	{
		boost::regex ApppathRegex((*pathitor).pattern);
		std::string temp = (*pathitor).pattern;
		boost::cmatch result; 
		if(!boost::regex_search(appPath.c_str(), result , ApppathRegex))
		{  
			continue;
		}
		else
		{			
			if (result.size() >2 && result[1].matched && result[2].matched) 
			{
				bResult = true;
				apppathinfo = *pathitor; 
				break;
			}
		}
	}

	if(!bResult)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "[%s] No URL pattern is matched for path[%s]"), 
			ident.name.c_str(), appPath.c_str());

		/*ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_317, "[%s] No URL pattern is matched for path[%s]", 
      ident.name.c_str(), appPath.c_str());*/
		apppathinfo.Authenable = false;
	}

	_env._pWatchDog->unwatch(ident.name);

	if (apppathinfo.Authenable && enableAuthorize)
	{
        std::string OTEendpoint = authEndpoint;
    	
		glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "detach() authorize  endpoint is  [%s]"), OTEendpoint.c_str());

		// begin find virtaulsiteName and apppath 
		std::string virtualSiteTemp = "";
		std::string apppathTemp = "";
		std::string fullurl = "";

		TianShanIce::ValueMap::iterator privator;
		privator = purPrivData.find(PD_KEY_SiteName);
		if(privator != purPrivData.end())
		{
			TianShanIce::Variant urlVar = privator->second;
			if (TianShanIce::vtStrings == urlVar.type && urlVar.strs.size() > 0)
			{
				virtualSiteTemp = urlVar.strs[0];
			}
		}

		privator = purPrivData.find(PD_KEY_Path);
		if(privator != purPrivData.end())
		{
			TianShanIce::Variant urlVar = privator->second;
			if (TianShanIce::vtStrings == urlVar.type && urlVar.strs.size() > 0)
			{
				apppathTemp = urlVar.strs[0];
			}
		}

		privator = purPrivData.find(PD_KEY_URL);
		if(privator != purPrivData.end())
		{
			TianShanIce::Variant urlVar = privator->second;
			if (TianShanIce::vtStrings == urlVar.type && urlVar.strs.size() > 0)
			{
				fullurl = urlVar.strs[0];
			}
		}
		// end find virtaulsiteName and apppath 

		int activeconsize = DefaultActiveConnectSize;
		TianShanIce::ValueMap::iterator purPrivItor = purPrivData.find(PD_KEY_ActiveConnectSize);
		if(purPrivItor != purPrivData.end() && purPrivItor->second.ints.size() == 1)
		{
			activeconsize = purPrivItor->second.ints[0];
		}
		if(activeconsize < 1)
		{
			activeconsize = DefaultActiveConnectSize;
		}
		glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "detach() activeconsize [%d]"), activeconsize);

		std::string	strTeardownReason = "";
		std::string strTerminateReason = "";

		::TianShanIce::Properties::const_iterator itProp;
		itProp = prop.find( SYS_PROP(teardownReason) );
		if (itProp != prop.end())
		{
			strTeardownReason = itProp->second;
		}
		itProp = prop.find (SYS_PROP(terminateReason));
		if (itProp != prop.end())
		{
			strTerminateReason = itProp->second;
		}

		if(bIspurInternal && !purInternalMsg.empty())
		{
			strTerminateReason +=   " " + purInternalMsg;
		}
//		if(stricmp(apppathinfo.playlistModule.c_str(), "Internal") == 0)
		if(0) //永远都走PlugIn调用
		{
			try
			{
				::com::izq::ote::tianshan::SessionData sd;
				sd.serverSessionId = serverSessionId;
				sd.clientSessionId = clientSessionId;
				
				SYS::TimeStamp st;
				char strTime[48];
				sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d", st.year,st.month,st.day,st.hour,st.minute,st.second);
				sd.params["ViewEndTime"] = strTime;
				sd.params["teardownReason"] = strTeardownReason;
				sd.params["terminateReason"] = strTerminateReason;
				sd.params["Reason"] = (!strTeardownReason.empty() ? strTeardownReason : strTerminateReason);
				
				//add extra params
				sd.params[PD_KEY_SiteName]= virtualSiteTemp;
				sd.params[PD_KEY_Path]= apppathTemp;
				sd.params[PD_KEY_URL]= fullurl;

				glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "calling authorization teardown with terminate reason [%s] and teardownReason [%s]"), 
					strTerminateReason.c_str (),strTeardownReason.c_str ());
			#if ICE_INT_VERSION / 100 >= 306  
				OTEForStateCBPtr OTECbPtr = new OTEForStateCB(clientSessionId);
				Ice::CallbackPtr  genericCB = Ice::newCallback(OTECbPtr, &OTEForStateCB::sessionTeardown);
				::com::izq::ote::tianshan::MoDIceInterfacePrx otePrx = NULL;
				otePrx = ::com::izq::ote::tianshan::MoDIceInterfacePrx::checkedCast(_env._iceComm->stringToProxy(OTEendpoint));
				otePrx->begin_sessionTeardown(sd,genericCB);
			#else	
				OTEForTeardownCBPtr pCB = new OTEForTeardownCB(clientSessionId);
				::com::izq::ote::tianshan::MoDIceInterfacePrx otePrx = NULL;
				otePrx = ::com::izq::ote::tianshan::MoDIceInterfacePrx::checkedCast(_env._iceComm->stringToProxy(OTEendpoint));
				otePrx->sessionTeardown_async(pCB, sd);
			#endif
			}
			catch (const Ice::ObjectNotExistException& ex)
			{
				glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "authorization teardown caught %s, endpoint is %s"), 
					ex.ice_name().c_str(), OTEendpoint.c_str());
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "authorization teardown caught %s, endpoint is %s"), ex.ice_name().c_str(), OTEendpoint.c_str());
			}
			catch (...)
			{
				glog(ZQ::common::Log::L_ERROR,
					PurchaseFmt(ModPurchaseImpl, 
					"authorization teardown caught unexpect exception"));
			}
		}
		else
		{
			int retCode;
			ZQTianShan::Application::MOD::IAuthorization::AuthorInfo authorinfo;
			
			authorinfo.endpoint = OTEendpoint;
			authorinfo.ident = ident;
			authorinfo.clientSessionId = clientSessionId;
			authorinfo.serverSessionId = serverSessionId;
						
			TianShanIce::Properties extraProps;
			extraProps = prop;
			            
			if(bIspurInternal && purInternalMsg.size() > 0)
			{
				::TianShanIce::Properties::iterator itProp;
				itProp = extraProps.find (SYS_PROP(terminateReason));
				if (itProp != extraProps.end())
				{
					extraProps.erase(itProp);
					extraProps[SYS_PROP(terminateReason)] = strTerminateReason;
				}
			}

			//add extra params
			extraProps[PD_KEY_SiteName]= virtualSiteTemp;
			extraProps[PD_KEY_Path]= apppathTemp;
			extraProps[PD_KEY_URL]= fullurl;

			char strTemp[32] = "";
			itoa(activeconsize, strTemp, 10);
			extraProps[PD_KEY_ActiveConnectSize] = strTemp;

			try
			{	
				retCode = _env._MHOHelperMgr.OnDestroyPurchase(apppathinfo.authEntry.c_str(), authorinfo, extraProps);
			}
			catch(TianShanIce::ServerError&ex)
			{
				glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl,"%s)"),ex.message.c_str());
			}
			catch(...)
			{
				glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl,"authorization teardown caught unknown exception(%d)"),SYS::getLastErr());
			}

			if(retCode == ZQTianShan::Application::MOD::IAuthorization::AUTHORSUCCESS)
			{
				glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl,"authorization teardown successful"));
			}
		}
	}

	// do: remove purchase from the evictor
	try
	{
//		ZQ::common::MutexGuard lk(_env._lockPurchase);
		_env._evctPurchase->remove(ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "remove purchase caught %s: %s"), 
			ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "remove purchase caught %s"), 
			ex.ice_name().c_str());
	}

	glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "detach() leave took %d ms"), (int)(ZQTianShan::now() - _stampClean));
}

void ModPurchaseImpl::bookmark(const ::std::string& title,
			const ::TianShanIce::SRM::SessionPrx& weiwooSessPrx, 
			const ::Ice::Current& c)
{
	ServantMutex::Lock lk(*this);
	return;
}

::Ice::Int ModPurchaseImpl::getParameters(const ::TianShanIce::StrValues& params, const ::TianShanIce::ValueMap& inMap, ::TianShanIce::ValueMap& outMap, const ::Ice::Current& c) const
{
	ServantMutex::Lock lk(*this);
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "getParameters() enter"));

	char szBuf[1024];
	szBuf[sizeof(szBuf) - 1] = '\0';
	snprintf(szBuf, sizeof(szBuf) - 1, PurchaseFmt(ModPurchaseImpl, "inMap."));
	::ZQTianShan::dumpValueMap(inMap, szBuf, dumpLine);

	outMap.clear();
	::TianShanIce::ValueMap tmpMap;
	bool bGetInfoCalled = false;
	unsigned int i = 0;
	for (i = 0; i < params.size(); i ++)
	{
		if (0 == stricmp(params[i].c_str(), "scale") || 0 == stricmp(params[i].c_str(), "position") || 0 == stricmp(params[i].c_str(), "Position.npt"))
		{
			if (bGetInfoCalled)
				continue;
			try
			{
				TianShanIce::ValueMap valMap;
				TianShanIce::Streamer::PlaylistPrx playlist = TianShanIce::Streamer::PlaylistPrx::checkedCast(stream);
				playlist->getInfo(TianShanIce::Streamer::infoSTREAMNPTPOS, valMap);

				TianShanIce::ValueMap::iterator curItor = valMap.find("playposition");
				TianShanIce::ValueMap::iterator totalItor = valMap.find("totalplaytime");
				TianShanIce::ValueMap::iterator scaleItor = valMap.find("scale");
				if (curItor != valMap.end() && curItor->second.ints.size() > 0 &&
					totalItor != valMap.end() && totalItor->second.ints.size() > 0 &&
					scaleItor != valMap.end() && scaleItor->second.strs.size() > 0)
				{
					TianShanIce::Variant varNptPos, varScale;
					snprintf(szBuf, sizeof(szBuf), "%d.%d-%d.%d", 
						curItor->second.ints[0] / 1000, 
						curItor->second.ints[0] % 1000, 
						totalItor->second.ints[0] / 1000, 
						totalItor->second.ints[0] % 1000);
					varNptPos.type = TianShanIce::vtStrings;
					varNptPos.strs.push_back(szBuf);
					varScale.type = TianShanIce::vtStrings;
					varScale.strs.push_back(scaleItor->second.strs[0]);
					tmpMap["scale"] = varScale;
					tmpMap["position"] = varNptPos;
				}
				else 
				{
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_402, "[%s] stream [%s] getInfo() failed to get all needed information",
						ident.name.c_str(), streamId.c_str());
				}
			}
			catch (const ::TianShanIce::BaseException& ex)
			{
				_IceReThrow(TianShanIce::ServerError, ex);
			}
			catch (const Ice::Exception& ex)
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_401, "[%s] stream [%s] getInfo() caught %s", 
					ident.name.c_str(), streamId.c_str(), ex.ice_name().c_str());
			}
			bGetInfoCalled = true;
			continue;
		}

		// not supported parameters requested.
//		ZQTianShan::_IceThrow<TianShanIce::NotSupported>(glog, LOG_PURCHASE, err_400, "[%s] parameter [%s] not supported", 
//			ident.name.c_str(), params[i].c_str());
	}

	for (i = 0; i <params.size(); i ++)
	{
		if (0 == stricmp(params[i].c_str(), "scale"))
			outMap[params[i]] = tmpMap["scale"];
		if (0 == stricmp(params[i].c_str(), "position"))
			outMap[params[i]] = tmpMap["position"];
		if (0 == stricmp(params[i].c_str(), "Position.npt"))
			outMap[params[i]] = tmpMap["position"];
	}

	snprintf(szBuf, sizeof(szBuf) - 1, PurchaseFmt(ModPurchaseImpl, "outMap."));
	::ZQTianShan::dumpValueMap(outMap, szBuf, dumpLine);

	return outMap.size();
}

::std::string ModPurchaseImpl::getId(const ::Ice::Current& c) const
{
	ServantMutex::Lock lk(*this);
	return ident.name;
}

::Ice::Identity ModPurchaseImpl::getIdent(const ::Ice::Current& c) const
{
	ServantMutex::Lock lk(*this);
	return ident;
}

::TianShanIce::Streamer::StreamPrx ModPurchaseImpl::getStream(const ::Ice::Current& c) const
{
	ServantMutex::Lock lk(*this);
	return stream;
}

::std::string ModPurchaseImpl::getClientSessionId(const ::Ice::Current& c) const
{
	ServantMutex::Lock lk(*this);
	return clientSessionId;
}

::std::string ModPurchaseImpl::getServerSessionId(const ::Ice::Current& c) const
{
	ServantMutex::Lock lk(*this);
	return serverSessionId;
}

bool ModPurchaseImpl::isInService(const ::Ice::Current& c) const
{
	ServantMutex::Lock lk(*this);
	return bInService;
}
// Return whether modulus of elem1 is less than modulus of elem2
bool mod_lesser (std::string elem1, std::string elem2 ) 
{
	return elem1 < elem2;
}
bool ModPurchaseImpl::getProviderId(ZQ::common::Config::Holder<Urlpattern>& apppathinfo, std::string provdAssetId, ::TianShanIce::ValueMap& privData)
{
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Enter getProviderId()"));

	bool bRet = true;


	PARAMMAP::iterator itor = apppathinfo.plAPID.apidParams.find("endpoint");
	if (itor == apppathinfo.plAPID.apidParams.end())
	{
		bRet = false;

		char errMsg[1024]="";
		sprintf(errMsg, "applicatoin-path[%s] has no [%s] parameter", apppathinfo.pattern.c_str(), "endpoint");
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);

		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_332, "[%s] applicatoin-path[%s] has no [%s] parameter", 
			ident.name.c_str(), apppathinfo.pattern.c_str(), "endpoint");
	}

	ZQTianShan::Application::MOD::IProviderQuery::ProviderInfo pidInfo;
	pidInfo.endpoint = itor->second.value;
	pidInfo.ident = ident;
	pidInfo.providerAssetId = provdAssetId;
	pidInfo.providerId = "";

	int retCode;
	const char *pErrDesc;

	try
	{
		retCode = _env._MHOHelperMgr.getProviderId(apppathinfo.plAPID.apidEntry.c_str(), pidInfo);
	}
	catch(TianShanIce::ServerError&ex)
	{
		bRet = false;
		glog(ZQ::common::Log::L_ERROR,PurchaseFmt(ModPurchaseImpl,"getProviderId()lookupPid caught exception(%s:%d:%s)"),
			ex.category.c_str(), ex.errorCode, ex.message.c_str());

		char errMsg[1024]="";
		sprintf(errMsg, "%s", ex.message.c_str());
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);
		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		retCode = ex.errorCode;
		if(retCode == ZQTianShan::Application::MOD::IProviderQuery::PIDNOTEXIST)
			retCode = err_310;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, retCode, "[%s]lookupPid caught exception (%s)", 
			ident.name.c_str(),ex.message.c_str());
	}
	catch(...)
	{
		bRet = false;
		glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl,"getProviderId()lookupPid caught unknown exception(%d)"), SYS::getLastErr());

		char errMsg[1024]="";
		sprintf(errMsg, "get play list caught unknown exception(%d)", SYS::getLastErr());
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);
		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		retCode = err_322;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, retCode, "[%s] lookupPid caught unknown exception(%d)", 
			ident.name.c_str(), SYS::getLastErr());
	}

	pErrDesc = _env._MHOHelperMgr.getErrorMsg(retCode);
	if(retCode != ZQTianShan::Application::MOD::IProviderQuery::PIDQUERYSUCCESS)
	{
		bRet = false;

		char errMsg[1024]="";
		sprintf(errMsg, "get play list caught exception(%s)", pErrDesc);
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);
		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		if(retCode == ZQTianShan::Application::MOD::IProviderQuery::PIDNOTEXIST)
			retCode = err_310;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,LOG_PURCHASE, retCode, "[%s]lookupPid caught exception(%s)", 
			ident.name.c_str(), pErrDesc);
	}
	_providerId = pidInfo.providerId;

	glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl,"getProviderId()lookupPid providerId[%s] providerAssetId[%s]"),pidInfo.providerId.c_str(), pidInfo.providerAssetId.c_str());

	return bRet;
}
bool ModPurchaseImpl::getAEFromLAM(ZQ::common::Config::Holder<Urlpattern>& apppathinfo, ZQ::common::URLStr& urlObj, ::TianShanIce::ValueMap& privData)
{
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Enter getAEFromLAM()"));
	int bRet = true;
	//assetProps.clear();
	PARAMMAP::iterator itor = apppathinfo.playlistParams.find(LAMENDPOINT);
	if (itor == apppathinfo.playlistParams.end())
	{
		bRet = false;

		char errMsg[1024]="";
		sprintf(errMsg, "applicatoin-path[%s] has no [%s] parameter", apppathinfo.pattern.c_str(), LAMENDPOINT);
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);

		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_332, "[%s] applicatoin-path[%s] has no [%s] parameter", 
			ident.name.c_str(), apppathinfo.pattern.c_str(), LAMENDPOINT);
	}

	std::string& lamendpoint = itor->second.value;

	/*add here*/
	for(AppDataPatternMAP::iterator it_plappdata = apppathinfo.playListAppDataMap.begin();
		it_plappdata != apppathinfo.playListAppDataMap.end(); ++it_plappdata)
	{
		::TianShanIce::ValueMap::iterator privateItor;
		std::string AppKey = ClientRequestPrefix + it_plappdata->second.param;
		privateItor =  privData.find(AppKey);

		if(privateItor != privData.end())
		{
			::TianShanIce::Variant var = privateItor->second;
			std::string MatchStr = var.strs[0];

			boost::regex AppDataRegex(it_plappdata->second.pattern);

			if(!boost::regex_match(MatchStr.c_str(),AppDataRegex))
			{
				continue;
			}

			PARAMMAP::iterator itor = it_plappdata->second.appDataParammap.find(LAMENDPOINT);
			if (itor != it_plappdata->second.appDataParammap.end())
			{
				lamendpoint = itor->second.value;		
				break;
			}	
		}
	}

	glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "Get LAM  endpoint is  [%s]"), lamendpoint.c_str());

	const char* assetv = NULL;
	const char* assetUIDv = NULL;
	std::string provdId;
	std::string provdAssetId; 
	std::string assetUid;
	std::string deviceID;
	ZQTianShan::Application::MOD::IPlayListQuery::PlayListInfo plinfo;
	com::izq::am::facade::servicesForIce::NetIDCollection netIdcollection;
	TianShanIce::StrValues volumeslist, volumesTemp;
    bool bAllHasNasUrl = false;
	plinfo.endpoint = lamendpoint;
	plinfo.ident = ident;

	assetv = urlObj.getVar("asset");
	assetUIDv = urlObj.getVar("assetuid");

	if(assetv != NULL && strlen(assetv) > 0)
	{
		std::string tempProAssetID;
		provdId = String::getLeftStr(assetv, "#");
		provdAssetId = String::getRightStr(assetv, "#");

		if(provdId.empty() && apppathinfo.plAPID.enable && !_providerId.empty())
		{
			provdId = _providerId;
		}

		TianShanIce::ValueMap::iterator mItor = privData.find(ClientRequestPrefix DeviceID);
		if (privData.end() == mItor || mItor->second.strs.size() == 0)
		{
			bRet =false;
			char errMsg[1024]="";
			sprintf(errMsg, "no device-id");
			::TianShanIce::Variant varError;
			varError.bRange = false;
			varError.type = TianShanIce::vtStrings;
			varError.strs.push_back(errMsg);
			MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

			purPrivData = privData;

			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_316, "[%s] no device-id", ident.name.c_str());
		}
		deviceID = mItor->second.strs[0];

		// save information here in order to authorize futher
		TianShanIce::Variant vProvdId, vProvdAssetId;
		vProvdId.type = TianShanIce::vtStrings;
		vProvdId.strs.push_back(provdId);
		vProvdAssetId.type = TianShanIce::vtStrings;
		vProvdAssetId.strs.push_back(provdAssetId);
		privData[ProviderId] = vProvdId;
		privData[ProviderAssetId] = vProvdAssetId;

		plinfo.nType = ZQTianShan::Application::MOD::IPlayListQuery::getaeListByPIDandAID;
		plinfo.UID1 = provdId;
		plinfo.UID2 = provdAssetId;
		plinfo.UID3 = deviceID;

		//assetProps["PID"] = provdId;
		//assetProps["PAID"] = provdAssetId;
	}
	else if(assetUIDv != NULL && strlen(assetUIDv) > 0)
	{
		assetUid = assetUIDv;

		// save information here in order to authorize futher
		TianShanIce::Variant vAssetUid;
		vAssetUid.type = TianShanIce::vtStrings;
		vAssetUid.strs.push_back(assetUid);
		privData[AssetId] = vAssetUid;

		plinfo.nType = ZQTianShan::Application::MOD::IPlayListQuery::getaeList;
		plinfo.UID1 = assetUid;

		ZQ::common::URLStr urlObj(appPath.c_str(), false);
		std::string pathStr = (NULL != urlObj.getPath()) ? urlObj.getPath() : "";
		plinfo.UID2 = pathStr;
		plinfo.UID3 ="";
	}
	else
	{
		bRet = false;

		char errMsg[1024]="";
		sprintf(errMsg, "no asset or assetUID in url");
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);
		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_307, "[%s] no asset or assetUID found in url", ident.name.c_str());
	}

	ZQTianShan::Application::MOD::AEReturnData aeRetData;
//	::TianShanIce::SRM::ResourceAttribute resAttribute = TianShanIce::SRM::raMandatoryNegotiable;
	::TianShanIce::SRM::ResourceAttribute resAttribute = TianShanIce::SRM::raMandatoryNonNegotiable;

	aeRetData.aeList.clear();
	aeRetData.netIDList.clear();

//	if(stricmp(apppathinfo.playlistModule.c_str(), "Internal") == 0)
	if(0)//永远都不会调到这里来，只会走Plugin
	{
		// do: get lam interface proxy
		::com::izq::am::facade::servicesForIce::AEInfo3 aeInfo;
		com::izq::am::facade::servicesForIce::LAMFacadePrx lamPrx = NULL;
		com::izq::am::facade::servicesForIce::AECollectionWithNetID assetElementswithNetID;
		try
		{
			lamPrx = com::izq::am::facade::servicesForIce::LAMFacadePrx::checkedCast(_env._iceComm->stringToProxy(lamendpoint));
		}
		catch (const Ice::ObjectNotExistException&)
		{
			bRet = false;
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_305, "[%s] LAMFacade proxy not exists at endpoint[%s]", 
				ident.name.c_str(), lamendpoint.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			bRet = false;
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_306, "[%s] get LAMFacade proxy caught [%s] at endpoint[%s]", 
				ident.name.c_str(), ex.ice_name().c_str(), lamendpoint.c_str());
		}

		if (assetv != NULL && strlen(assetv) > 0) // url like rtsp://CatvOfChangNing/MOD?asset=providerId1#asset1
		{
			try
			{
				assetElementswithNetID = lamPrx->getAEListWithNetIDByPID(provdId, provdAssetId);
			}
			catch (const Ice::Exception& ex)
			{
				bRet = false;
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_308, "[%s] getAEListByProviderIdAndPAssetId() caught %s", 
					ident.name.c_str(), ex.ice_name().c_str());
			}
		}
		else if (assetUIDv != NULL && strlen(assetUIDv) > 0) // url like rtsp://CatvOfChangNing/MOD?assetUID=asset1
		{			
			try
			{
				assetElementswithNetID = lamPrx->getAEListWithNetID(assetUid);
			}
			catch (const Ice::Exception& ex)
			{
				bRet = false;
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_309, "[%s] getAEList() caught %s", 
					ident.name.c_str(), ex.ice_name().c_str());
			}
		}
		for(unsigned int i = 0; i < assetElementswithNetID.aeList.size(); i++)
		{
			//			::com::izq::surf::integration::tianshan::AEInfo& assetInfo = assetElementswithNetID.aeList[i];
			::com::izq::am::facade::servicesForIce::AEInfo3 aeInfo;
			aeInfo.name = assetElementswithNetID.aeList[i].aeUID;
			aeInfo.bandWidth = assetElementswithNetID.aeList[i].bandWidth;
			aeInfo.cueIn = assetElementswithNetID.aeList[i].cueIn;
			aeInfo.cueOut = assetElementswithNetID.aeList[i].cueOut;
			aeInfo.volumeList.clear();
			aeInfo.nasUrls.clear();
			aeInfo.attributes.clear();
			assetElements.push_back(aeInfo);
		}
		netIdcollection = assetElementswithNetID.netIDList;
		volumeslist.clear();
	}
	else
	{
		if (!gNewCfg.testItems.enable)
		{
			int retCode;
			const char *pErrDesc;

			try
			{
				retCode = _env._MHOHelperMgr.getPlayList(apppathinfo.playlistEntry.c_str(), plinfo, aeRetData);
			}
			catch(TianShanIce::ServerError&ex)
			{
				bRet = false;
				glog(ZQ::common::Log::L_ERROR,PurchaseFmt(ModPurchaseImpl,"getAEFromLAM()get play list caught exception(%s:%d:%s)"),
					ex.category.c_str(), ex.errorCode, ex.message.c_str());

				char errMsg[1024]="";
				sprintf(errMsg, "%s", ex.message.c_str());
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;

				retCode = ex.errorCode;
				if(retCode == ZQTianShan::Application::MOD::IPlayListQuery::PLNOTEXIST)
					retCode = err_310;

				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, retCode, "[%s]get play list caught exception (%s)", 
					ident.name.c_str(),ex.message.c_str());
			}
			catch(...)
			{
				bRet = false;
				glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl,"getAEFromLAM()get play list caught unknown exception(%d)"), SYS::getLastErr());

				char errMsg[1024]="";
				sprintf(errMsg, "get play list caught unknown exception(%d)", SYS::getLastErr());
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;

				retCode = err_322;
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, retCode, "[%s] get play list caught unknown exception(%d)", 
					ident.name.c_str(), SYS::getLastErr());
			}

			pErrDesc = _env._MHOHelperMgr.getErrorMsg(retCode);
			if(retCode != ZQTianShan::Application::MOD::IPlayListQuery::PLQUERYSUCCESS)
			{
				bRet = false;

				char errMsg[1024]="";
				sprintf(errMsg, "get play list caught exception(%s)", pErrDesc);
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;

				if(retCode == ZQTianShan::Application::MOD::IPlayListQuery::PLNOTEXIST)
					retCode = err_310;

				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,LOG_PURCHASE, retCode, "[%s]get play list caught exception(%s)", 
					ident.name.c_str(), pErrDesc);
			}
		}
		else
		{
			TestElementMod(aeRetData, privData);
		}
	
	  glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl,"get play list successfully from [%s]"), lamendpoint.c_str());

	  {
		  int overrideCueIn = -1, overrideCueOut = -1;

		  PARAMMAP::iterator itor = apppathinfo.playlistParams.find("overrideCueIn");
		  if (itor != apppathinfo.playlistParams.end())
		  {
			  overrideCueIn = atoi((itor->second.value).c_str());
		  }
		  itor = apppathinfo.playlistParams.find("overrideCueOut");
		  if (itor != apppathinfo.playlistParams.end())
		  {
			  overrideCueOut = atoi((itor->second.value).c_str());
		  }
		  if(overrideCueIn >= 0  && overrideCueOut > overrideCueIn)
			  overrideCueInCueOut(aeRetData, overrideCueIn, overrideCueOut);
	  }
	  if(checkProvisionTimeout())
		  return false;

	  /// add ADSReplacement logic
	  std::string adsEndpoint = "";
	  try
	  {
		  int retCode = -1;
		  if(apppathinfo.adsReplacement.adsEntry!= "" && apppathinfo.adsReplacement.adsParams.size() > 0 )
		  { 
			  //In MOD, if the private data of Weiwoo session carries ["skip-ads"] AND the value is no-empty and non-zero, MOD would skip calling the entry defined via AdsReplacement but directly return the result of GetAEList
			  TianShanIce::ValueMap::iterator mItor = privData.find(ClientRequestPrefix SKIP_ADS);
			  if (privData.end() != mItor && mItor->second.strs.size() > 0 && mItor->second.strs[0] != "0")
			  {
				  glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl,"[%s] skip to call get ADSReplacement interface"), ident.name.c_str());
			  }
			  else
			  {
				  ZQTianShan::Application::MOD::IAdsReplacement::AdsInfo adsinfo;

				  PARAMMAP::iterator itor = apppathinfo.adsReplacement.adsParams.find(ADMENDPOINT);
				  if (itor == apppathinfo.adsReplacement.adsParams.end())
				  {
					  glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl,"[%s] applicatoin-path[%s] has no [%s] parameter"), 
						  ident.name.c_str(), apppathinfo.pattern.c_str(), ADMENDPOINT);
				  }
				  else
				  {
					  adsinfo.ident = ident;
					  adsinfo.clientSessionId = clientSessionId;
					  adsinfo.serverSessionId = serverSessionId;
					  adsEndpoint =  itor->second.value;
					  adsinfo.endpoint = adsEndpoint;
					  retCode = _env._MHOHelperMgr.getAdsRepalcement(apppathinfo.adsReplacement.adsEntry.c_str(), adsinfo, aeRetData, privData);
				  }

				  if(retCode != ZQTianShan::Application::MOD::IAdsReplacement::ADSQUERYSUCCESS)
				  {
					  const char* pErrDesc = _env._MHOHelperMgr.getErrorMsg(retCode);
					  glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl,"[%s]get ADS replacement list caught exception(%s)"), 
						  ident.name.c_str(), pErrDesc);
				  }
				  else
				  {
					  /////在此加入ADS额外的aAttributes.
					  size_t i = 0;
					  ZQTianShan::Application::MOD::AssetElementCollection& aeinfolist = aeRetData.aeList;
					  std::vector<int>adsPos;
					  bool bAssetFound = false;
					  std::string adsType = "";
					  while(apppathinfo.adsReplacement.adsProps.size() > 0 && i < aeinfolist.size())
					  {
						  ZQTianShan::Application::MOD::AttributesMap& attrib = aeinfolist[i].attributes;
						  if(attrib.find(ADM_ISAD) != attrib.end() && attrib[ADM_ISAD] == "1")
						  {
							  adsPos.push_back(i);
							  bAssetFound = false;
						  }
						  else
							  bAssetFound = true;

						  if(bAssetFound && adsPos.size() > 0)//发现Asset并且有广告， 判断广告类型
						  {
							  if(adsPos[0] == 0)//广告在第一个位置，肯定是前置广告
							  { 
								  //加入前置广告props
								  adsType = "FRONT";
							  }
							  else if(adsPos[adsPos.size() -1] == aeinfolist.size() -1) ///广告的最后一个位置 等于aeinfolist总长，那么肯定是后置 
							  { 
								  //加入后置广告props
								  adsType = "REAR";
							  }
							  else
							  {
								  adsType = "MIDDLE";
							  }
						  }
						  else  if(adsPos.size() == aeinfolist.size()) //全部是广告
							  adsType = "FRONT";
						  else if(adsPos.size() > 0 &&  adsPos[adsPos.size() -1] == aeinfolist.size() -1)//后置广告
							  adsType = "REAR";

						  if(adsPos.size() > 0 && (adsType =="FRONT" || adsType=="REAR" || adsType =="MIDDLE"))
						  {
							  //根据配置找到广告类型，加入进去;
                              if(apppathinfo.adsReplacement.adsProps.find(adsType) != apppathinfo.adsReplacement.adsProps.end())
							  {
								  PARAMMAP&adsprops = apppathinfo.adsReplacement.adsProps[adsType].adsprops;
								  int& bitwiseOr = apppathinfo.adsReplacement.adsProps[adsType].bitwiseOr;
								  for(int k = 0; k < adsPos.size(); k++)
								  {
									  // dump the attributes
									  std::string strAdsAttributes = "";
									  for (::com::izq::am::facade::servicesForIce::AttributesMap::iterator pItor = aeinfolist[adsPos[k]].attributes.begin(); pItor != aeinfolist[adsPos[k]].attributes.end(); pItor ++)
									  {
										  char temp[512] = "";
										  sprintf(temp, "[%s]=[%s],", pItor->first.c_str(), pItor->second.c_str());
										  strAdsAttributes += temp;
									  }

									  PARAMMAP::iterator it;
									  for(it = adsprops.begin(); it!=adsprops.end();++it)
									  {
										  if(attrib.find(it->first) != attrib.end()) //如果ads中有这个Attribute：则 bitwiseOr= 0， 返回API中的; 如果bitwiseOr=1， 则两个取OR
										  {
											  if(bitwiseOr != 0)
											  {
												  int adsAPIFlags = atoi((attrib[it->first]).c_str());
												  int adsConfigFlags = atoi((it->second.value.c_str()));
												  int finalAdsFlags = adsAPIFlags | adsConfigFlags;

												  char buf[16] = "";
												  sprintf(buf, "%d", finalAdsFlags);
												  aeinfolist[adsPos[k]].attributes[it->first] = buf;
											  }
										  }
										  else
											  aeinfolist[adsPos[k]].attributes[it->first] = it->second.value;
									  }

									  // dump the attributes
									  std::string strAdsAttributesModify = "";
									  for (::com::izq::am::facade::servicesForIce::AttributesMap::iterator pItor = aeinfolist[adsPos[k]].attributes.begin(); pItor != aeinfolist[adsPos[k]].attributes.end(); pItor ++)
									  {
										  char temp[512] = "";
										  sprintf(temp, "[%s]=[%s],", pItor->first.c_str(), pItor->second.c_str());
										  strAdsAttributesModify += temp;
									  }
									  glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl,"[%s]ads[%s] is [%s],original attributes[%s] modify attributes[%s]"), 
										  ident.name.c_str(), aeinfolist[adsPos[k]].aeUID.c_str(), adsType.c_str(), strAdsAttributes.c_str(), strAdsAttributesModify.c_str());
								  }
							  }
							  adsPos.clear();
							  bAssetFound = false;
							  adsType = "";
						  }
						  i++;
					  }
					  glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl,"get ADS replacement list successfully from [%s]"), adsEndpoint.c_str());
				  }
			  }
		  }
		 }
	  catch(TianShanIce::ServerError&ex)
	  {
		  glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl,"[%s]get ADS replacement list caught exception (%s)"), 
			  ident.name.c_str(),ex.message.c_str());
	  }
	  catch(...)
	  {
		  glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl,"[%s] get ADS replacement list caught unknown exception(%d)"), 
			  ident.name.c_str(), SYS::getLastErr());
	  }
	  /// end ADSReplacement
	  if(checkProvisionTimeout())
		  return false;

	  /// add Asset Location Interface logic
	  std::string assetLocEndpoint = "";
	  try
	  {
		  int retCode = -1;
		  if(apppathinfo.assetLocation.alEntry!= "" && apppathinfo.assetLocation.alParams.size() > 0 )
		  { 
			  ZQTianShan::Application::MOD::IAssetLocation::AssetLocationInfo alinfo;

			  PARAMMAP::iterator itor = apppathinfo.assetLocation.alParams.find(ALENDPOINT);
			  if (itor == apppathinfo.assetLocation.alParams.end())
			  {
				  glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl,"[%s] applicatoin-path[%s] has no [%s] parameter"), 
					  ident.name.c_str(), apppathinfo.pattern.c_str(), ALENDPOINT);
			  }
			  else
			  {
				  alinfo.ident = ident;
				  alinfo.onDemandSessionId = serverSessionId;
				  assetLocEndpoint =  itor->second.value;
				  alinfo.endpoint = assetLocEndpoint;
				  retCode = _env._MHOHelperMgr.getAssetLocation(apppathinfo.assetLocation.alEntry.c_str(), alinfo, aeRetData, privData);
			  }

			  if(retCode != ZQTianShan::Application::MOD::IAssetLocation::ALSUCCESS)
			  {
				  const char* pErrDesc = _env._MHOHelperMgr.getErrorMsg(retCode);
				  glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl,"[%s]get asset location caught exception(%s)"), 
					  ident.name.c_str(), pErrDesc);
			  }

			  glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl,"get asset location successfully from [%s]"), adsEndpoint.c_str());
		  }
	  }
	  catch(TianShanIce::ServerError&ex)
	  {
		  glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl,"[%s]get asset location caught exception (%s)"), 
			  ident.name.c_str(),ex.message.c_str());
	  }
	  catch(...)
	  {
		  glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl,"[%s]get asset location caught unknown exception(%d)"), 
			  ident.name.c_str(), SYS::getLastErr());
	  }
	  /// end Asset Location Interface

	  if(checkProvisionTimeout())
		  return false;


	  if(apppathinfo.playlistEntry == LAM_PlayList3_Name)
	  {	
		  //如果Asset的NasUrl不为空，则往每个Asset的 volumelist 中插入一个 临时的 REDUNDANCY_NETIDVOLUME
		  //如果取完交集后，REDUNDANCY_NETIDVOLUME这个存在， 说明所有Asset的NasUrl不为空
		 // NarURL 存在：    1> volumeList 为空，  raMandatoryNonNegotiable （部分Asset有NasUrl）
		 //	                 2> volumeList 不为空，raNonMandatoryNonNegotiable（VoumleList有可能为空）
		 // NarURL 不存在：  1> volumeList 为空，  raNonMandatoryNegotiable
		 //	                 2> volumeList 不为空，raMandatoryNonNegotiable
          if(!gNewCfg.libraryAsset.mandatory) //for gehua dizhonghai
		  {
			  for(size_t i = 0; i < aeRetData.aeList.size(); i++)
			  {
				  if(aeRetData.aeList[i].volumeList.size() > 0)
				  { 
					  ::com::izq::am::facade::servicesForIce::StringCollection aevolumelists = aeRetData.aeList[i].volumeList;
					  aeRetData.aeList[i].volumeList.clear();
					  for(size_t j = 0; j < aevolumelists.size(); j++)
					  {
						  std::string& volume = aevolumelists[j];
						  int npos = volume.find('/');
						  if(npos < 0)
						  {
							  volume += "/$";
						  }
						  aeRetData.aeList[i].volumeList.push_back(volume);
					  }
				  }
			  }

			  glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl,"Asset[%s] nasUrls[%d], volumesize[%d]"), aeRetData.aeList[0].aeUID.c_str(), aeRetData.aeList[0].nasUrls.size(), aeRetData.aeList[0].volumeList.size());
			  if(aeRetData.aeList[0].nasUrls.size() > 0)
			  {
				  aeRetData.aeList[0].volumeList.push_back(REDUNDANCY_NETIDVOLUME);
			  }
			  volumesTemp = aeRetData.aeList[0].volumeList;

			TianShanIce::StrValues strResult;
			TianShanIce::StrValues::iterator subsetResult;

			TianShanIce::ValueMap resourceData;

			TianShanIce::Variant storeNetIds;
			storeNetIds.type = TianShanIce::vtStrings;
			storeNetIds.bRange = false;

			sort (aeRetData.aeList[0].volumeList.begin(), aeRetData.aeList[0].volumeList.end(), mod_lesser);
			storeNetIds.strs = aeRetData.aeList[0].volumeList;
			resourceData.insert(::TianShanIce::ValueMap::value_type("NetworkId", storeNetIds));
			char szBuf[1024];
			memset(szBuf, 0, 1024);
			snprintf(szBuf, sizeof(szBuf) - 1, "[%s]Storage volumes collection From LAM, AE[0] ",ident.name.c_str());
			ZQTianShan::dumpValueMap(resourceData, szBuf, dumpLine);

		  if(aeRetData.aeList.size() > 1)
		  {
			  for(size_t i= 1; i < aeRetData.aeList.size(); i++)
			  {
				  glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl,"Asset[%s] nasUrls[%d], volumesize[%d]"), aeRetData.aeList[i].aeUID.c_str(), aeRetData.aeList[i].nasUrls.size(), aeRetData.aeList[i].volumeList.size());
				  if(aeRetData.aeList[i].nasUrls.size() > 0)
				  {
                    aeRetData.aeList[i].volumeList.push_back(REDUNDANCY_NETIDVOLUME);
				  }
				  sort (volumesTemp.begin(), volumesTemp.end(), mod_lesser);
				  sort (aeRetData.aeList[i].volumeList.begin(), aeRetData.aeList[i].volumeList.end(), mod_lesser);

					storeNetIds.strs.clear();
					storeNetIds.strs = aeRetData.aeList[i].volumeList;
					resourceData.clear();
					resourceData.insert(::TianShanIce::ValueMap::value_type("NetworkId", storeNetIds));
					char szBuf[1024];
					memset(szBuf, 0, 1024);
					snprintf(szBuf, sizeof(szBuf) - 1, "[%s]Storage volumes collection From LAM, AE[%d] ",ident.name.c_str(), i);
					ZQTianShan::dumpValueMap(resourceData, szBuf, dumpLine);

					strResult.clear();

					if(aeRetData.aeList[i].volumeList.size() > volumeslist.size())
					{
						strResult.resize(aeRetData.aeList[i].volumeList.size());
					}
					else
					{
						strResult.resize(volumesTemp.size());					 
					}

					subsetResult = set_intersection ( volumesTemp.begin (), volumesTemp.end (),
						aeRetData.aeList[i].volumeList.begin (), aeRetData.aeList[i].volumeList.end (), strResult.begin(),mod_lesser);

				  if(subsetResult == strResult.begin() && (!volumesTemp.empty() || !aeRetData.aeList[i].volumeList.empty()))
				  {
					  // add logic to deal with no subset storage ID
					  bRet = false;
					  netIdcollection.clear();
					  char errMsg[1024]="";
					  sprintf(errMsg, "No apposite volume/lib supplies the playlist");
					  ::TianShanIce::Variant varError;
					  varError.bRange = false;
					  varError.type = TianShanIce::vtStrings;
					  varError.strs.push_back(errMsg);
					  MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

						purPrivData = privData;

					  ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_318, "No apposite volume/lib supplies the playlist");
					}
					else
					{
						volumesTemp.clear();
						volumesTemp.assign(strResult.begin(), subsetResult);
					}
				}
			}

			//dump the subset storage volumes from LAM
			storeNetIds.strs.clear();
			storeNetIds.strs = volumesTemp;
			resourceData.clear();
			resourceData.insert(::TianShanIce::ValueMap::value_type("NetworkId", storeNetIds));

		  memset(szBuf, 0, 1024);
		  snprintf(szBuf, sizeof(szBuf) - 1, "[%s]subset storage volumes collection from LAM AEList ",ident.name.c_str());
		  ZQTianShan::dumpValueMap(resourceData, szBuf, dumpLine);

		  //get stroage netId from LAM
		  volumesLists = volumesTemp;
		  netIdcollection.clear();
		  TianShanIce::StrValues::iterator subItor;
          for(subItor = volumesTemp.begin(); subItor != volumesTemp.end(); subItor++)
		  {
			com::izq::am::facade::servicesForIce::NetIDCollection::iterator itor;
			TianShanIce::StrValues::iterator itorVolume;
			std::string StorageNetIdTemp = *subItor;
			std::string volume = "";
			int npos = StorageNetIdTemp.find('/');
			if(npos >= 1)
			{
				StorageNetIdTemp = StorageNetIdTemp.substr(0, npos);
				volume = (*subItor).substr(npos + 1, (*subItor).size() - npos - 1);	
			}
            
			itor = std::find(netIdcollection.begin(), netIdcollection.end(),StorageNetIdTemp);
			if(itor == netIdcollection.end())
			{
				netIdcollection.push_back(StorageNetIdTemp);
			}
			if(volume.size() > 0)
			{
				itorVolume = std::find(volumeslist.begin(), volumeslist.end(), volume);
				if(itorVolume == volumeslist.end())
				{
					volumeslist.push_back(volume);
				}
			}
		    //如果存在这个 REDUNDANCY_NETIDVOLUME Voumle， 说明所有Asset的NarURL都有值 
			if (*subItor == REDUNDANCY_NETIDVOLUME)
			{
				bAllHasNasUrl = true;
//				resAttribute = TianShanIce::SRM::raNonMandatoryNegotiable;
//				resAttribute = TianShanIce::SRM::raNonMandatoryNonNegotiable;
			}
		  } 

			//dump the subset storage from LAM.
			storeNetIds.strs.clear();
			storeNetIds.strs = netIdcollection;
			resourceData.clear();
			resourceData.insert(::TianShanIce::ValueMap::value_type("NetworkId", storeNetIds));

			  memset(szBuf, 0, 1024);
			  snprintf(szBuf, sizeof(szBuf) - 1, "[%s]subset storage netID collection from LAM AEList ",ident.name.c_str());
			  ZQTianShan::dumpValueMap(resourceData, szBuf, dumpLine);
		  }
	  } //end if(apppathinfo.playlistEntry == LAM_PlayList3_Name)
	  else
	  {		
		  netIdcollection = aeRetData.netIDList;
		  bAllHasNasUrl   = false;
		  volumeslist.clear();
	  }
      
	  for(unsigned int i = 0; i < aeRetData.aeList.size(); i++)
	  {
		  std::string aeUID;
		  ::com::izq::am::facade::servicesForIce::AEInfo3 aeInfo;
		  aeInfo.name = aeRetData.aeList[i].aeUID;
		  aeInfo.bandWidth = aeRetData.aeList[i].bandWidth;
		  aeInfo.cueIn = aeRetData.aeList[i].cueIn;
		  aeInfo.cueOut = aeRetData.aeList[i].cueOut;
		  aeInfo.volumeList= aeRetData.aeList[i].volumeList;
		  aeInfo.nasUrls = aeRetData.aeList[i].nasUrls;
		  aeInfo.attributes = aeRetData.aeList[i].attributes;
		  assetElements.push_back(aeInfo);
	  }
	}

	TianShanIce::SRM::Resource resResult;
	resResult.status = TianShanIce::SRM::rsRequested;
	resResult.attr = TianShanIce::SRM::raMandatoryNegotiable;

	TianShanIce::Variant storeNetIds, storeVolumes;
	storeNetIds.type = TianShanIce::vtStrings;
	storeNetIds.bRange = false;

	storeVolumes.type = TianShanIce::vtStrings;
	storeVolumes.bRange = false;

	//add netid list to weiwoo resource. 
	if (netIdcollection.size() > 0)
	{			
		glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "subset storage netID collection from weiwoo and LAM"));	
		TianShanIce::SRM::Resource rsStorageFromLAM;
		rsStorageFromLAM.status = TianShanIce::SRM::rsRequested;
		rsStorageFromLAM.attr = TianShanIce::SRM::raMandatoryNegotiable;

		storeNetIds.strs = netIdcollection;
		storeVolumes.strs = volumeslist;

		rsStorageFromLAM.resourceData.insert(::TianShanIce::ValueMap::value_type("NetworkId", storeNetIds));
		rsStorageFromLAM.resourceData.insert(::TianShanIce::ValueMap::value_type("Volume", storeVolumes));

		char szBuf[1024];
		memset(szBuf, 0, 1024);
        snprintf(szBuf, sizeof(szBuf) - 1, "[%s]storage netID collection from LAM  ",ident.name.c_str());
		ZQTianShan::dumpValueMap(rsStorageFromLAM.resourceData, szBuf, dumpLine);
		TianShanIce::SRM::ResourceMap rsMap;
		try
		{	
			rsMap = weiwoo->getReources();
		}
		catch (Ice::Exception& ex)
		{	
			glog(ZQ::common::Log::L_WARNING, PurchaseFmt(ModPurchaseImpl, "get resource from weiwoo caught ice exception(%s)"), ex.ice_name().c_str());
		}

		//Weiwoo中的Resource和从LAM返回的Resource取交集
		//如果Weiwoo中没有Resource：TianShanIce::SRM::rtStorage， 则直接使用从LAM返回的Resource 
		TianShanIce::SRM::ResourceMap::iterator itStorge = rsMap.find(TianShanIce::SRM::rtStorage);
		if (rsMap.end() != itStorge)
		{
			memset(szBuf, 0, 1024);
            snprintf(szBuf, sizeof(szBuf) - 1, "[%s]storage netID collection from Weiwoo ",ident.name.c_str());
			ZQTianShan::dumpValueMap(itStorge->second.resourceData, szBuf, dumpLine);

			if (!bAllHasNasUrl && !ZQTianShan::InterRestrictResource(rsStorageFromLAM, itStorge->second, resResult))
			{
				bRet = false;
				char errMsg[1024] = "";
				sprintf(errMsg, "No apposite volume/lib supplies the playlist");
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;

				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_318, "No apposite volume/lib supplies the playlist");
			}

			//取完交集后,需要将(NetId交集)跟从LAM得到 volumesLists做对比,
			//将volumesLists中NetId不在(NetId交集)中的多余的元素去掉
			TianShanIce::ValueMap::iterator itorNetId = resResult.resourceData.find("NetworkId");
			if(itorNetId != resResult.resourceData.end() && itorNetId->second.strs.size() > 0)
			{
				TianShanIce::StrValues& strNetIds = itorNetId->second.strs;
				for(TianShanIce::StrValues::iterator itor = volumesLists.begin(); itor < volumesLists.end();)
				{
					std::string StorageNetIdTemp = *itor;
					int npos = (*itor).find('/');
					if(npos >= 1)
					{
						StorageNetIdTemp = StorageNetIdTemp.substr(0, npos);
					}
					TianShanIce::StrValues::iterator itorNetId = std::find(strNetIds.begin(), strNetIds.end(), StorageNetIdTemp);
					if(itorNetId == strNetIds.end())
					{
						itor = volumesLists.erase(itor);
						continue;
					}
					itor++;
				}
			}
		} //end if Weiwoo 和 LAM Resouce取交集
		else
		{
			glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "weiwoo doesn't have storage netID resource, use storage netID resource from LAM"));

			com::izq::am::facade::servicesForIce::NetIDCollection::iterator itorNetId;
			TianShanIce::StrValues::iterator itorVolume;
			//去掉加入的NetID： REDUNDANCY_NETID
			itorNetId = std::find(netIdcollection.begin(), netIdcollection.end(), REDUNDANCY_NETID);
			if(itorNetId != netIdcollection.end())
			{
				netIdcollection.erase(itorNetId);
			}
			//去掉加入的Volume： REDUNDANCY_VOLUME
			itorVolume = std::find(volumeslist.begin(), volumeslist.end(), REDUNDANCY_VOLUME);
			if(itorVolume != volumeslist.end())
			{
				volumeslist.erase(itorVolume);
			} 

			if(netIdcollection.size() > 0)
			{
				TianShanIce::Variant storeNetIds, storeVolumes;
				storeNetIds.type = TianShanIce::vtStrings;
				storeNetIds.bRange = false;
				storeNetIds.strs = netIdcollection;		
				resResult.resourceData.insert(::TianShanIce::ValueMap::value_type("NetworkId", storeNetIds));
			}
			if(volumeslist.size()> 0)
			{
				storeVolumes.type = TianShanIce::vtStrings;
				storeVolumes.bRange = false;
				storeVolumes.strs = volumeslist;
				resResult.resourceData.insert(::TianShanIce::ValueMap::value_type("Volume", storeVolumes));
			}
		} //end if use LAM resource

		// determine the resource attributes:
		if (bAllHasNasUrl)
			resAttribute = volumeslist.empty() ? TianShanIce::SRM::raMandatoryNonNegotiable : TianShanIce::SRM::raNonMandatoryNonNegotiable;
		else 
			resAttribute = volumeslist.empty() ? TianShanIce::SRM::raNonMandatoryNegotiable : TianShanIce::SRM::raMandatoryNonNegotiable;

		memset(szBuf, 0, 1024);
		snprintf(szBuf, sizeof(szBuf) - 1, "[%s]Subset Storage NetID collection  ",ident.name.c_str());
		ZQTianShan::dumpValueMap(resResult.resourceData, szBuf, dumpLine);

		/*LAM与Weiwoo的Resource取完交集后, 再将LinkType加进去
		If none of the AE’s has nasURL and the volumeList is not empty, 
		the MovieOnDemand application would leave empty in resource[rtStorage]["LinkType"] 
		and set the ResourceAttribute of resource[rtStorage] as raMandatoryNegotiable
		If an AE’s nasURL is not empty, add the configured <AllowedStorageLink> type values 
		into resource[rtStorage]["LinkType"] and the ResourceAttribute of resource[rtStorage] 
		as raNonMandatoryNegotiable*/
//		if(resAttribute == TianShanIce::SRM::raNonMandatoryNegotiable)
		if(gNewCfg.libraryAsset.mandatory)
			resAttribute = TianShanIce::SRM::raMandatoryNonNegotiable;

		if (bAllHasNasUrl || gNewCfg.libraryAsset.mandatory)
		{
			TianShanIce::Variant vstoragelink;
			vstoragelink.type = TianShanIce::vtStrings;
			vstoragelink.bRange = false;
			vstoragelink.strs.clear();
			vstoragelink.strs = gNewCfg.libraryAsset.allowedStorageLinks;
			resResult.resourceData.insert(::TianShanIce::ValueMap::value_type("LinkType", vstoragelink));
			std::string storageLinktype;
			for(size_t  i = 0; i < gNewCfg.libraryAsset.allowedStorageLinks.size(); ++i)
			{
				storageLinktype += gNewCfg.libraryAsset.allowedStorageLinks[i];
				storageLinktype += ";";
			}
			glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "add LinkType [%s] to resource of rtStorage [resAttribute=%d]"), storageLinktype.c_str(), resAttribute);
		}
		try
		{	
		   weiwoo->addResource(TianShanIce::SRM::rtStorage, resAttribute, resResult.resourceData);
		}
		catch (Ice::Exception& ex)
		{	
			glog(ZQ::common::Log::L_WARNING, PurchaseFmt(ModPurchaseImpl, "add resource to weiwoo caught ice exception(%s)"), ex.ice_name().c_str());
			
			bRet = false;

			char errMsg[1024]="";
			sprintf(errMsg, "add resource to weiwoo caught ice exception(%s)", ex.ice_name().c_str());
			::TianShanIce::Variant varError;
			varError.bRange = false;
			varError.type = TianShanIce::vtStrings;
			varError.strs.push_back(errMsg);
			MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

			purPrivData = privData;

			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog,LOG_PURCHASE, err_319, 
				"[%s] add resource to weiwoo caught ice exception(%s)", ident.name.c_str(), ex.ice_name().c_str());
		}
	}//end if (netIdcollection.size() > 0)
	else
	{
		if(gNewCfg.libraryAsset.mandatory)
		{
			resAttribute = TianShanIce::SRM::raMandatoryNonNegotiable;
			TianShanIce::Variant vstoragelink;
			vstoragelink.type = TianShanIce::vtStrings;
			vstoragelink.bRange = false;
			vstoragelink.strs.clear();
			vstoragelink.strs = gNewCfg.libraryAsset.allowedStorageLinks;
			resResult.resourceData.insert(::TianShanIce::ValueMap::value_type("LinkType", vstoragelink));
			std::string storageLinktype;
			for(size_t  i = 0; i < gNewCfg.libraryAsset.allowedStorageLinks.size(); ++i)
			{
				storageLinktype += gNewCfg.libraryAsset.allowedStorageLinks[i];
				storageLinktype += ";";
			}
			glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "add LinkType [%s] to resource of rtStorage [resAttribute=%d]"),storageLinktype.c_str(), resAttribute);

			try
			{	
				weiwoo->addResource(TianShanIce::SRM::rtStorage, resAttribute, resResult.resourceData);
			}
			catch (Ice::Exception& ex)
			{	
				glog(ZQ::common::Log::L_WARNING, PurchaseFmt(ModPurchaseImpl, "add resource to weiwoo caught ice exception(%s)"), ex.ice_name().c_str());

				bRet = false;

				char errMsg[1024]="";
				sprintf(errMsg, "add resource to weiwoo caught ice exception(%s)", ex.ice_name().c_str());
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;

				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog,LOG_PURCHASE, err_319, 
					"[%s]add resource to weiwoo caught ice exception(%s)", ident.name.c_str(), ex.ice_name().c_str());
			}
		}
		glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "none of NetID lists from LAM "));
}

	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Leave getAEFromLAM()"));
	return bRet;
}

bool ModPurchaseImpl::getAEForTianShanSurf(ZQ::common::Config::Holder<Urlpattern>& apppathinfo, ZQ::common::URLStr& urlObj, ::TianShanIce::ValueMap& privData)
{
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Enter getAEForTianShanSurf()"));

	// DO: make surf mode not to authorize
	enableAuthorize = false;

	bool bRet = true;
	::com::izq::surf::integration::tianshan::AEReturnData SurfAEData;
	assetElements.clear();
	SurfAEData.aeList.clear();
	SurfAEData.netIDList.clear();

	PARAMMAP::iterator itor = apppathinfo.playlistParams.find(LAMENDPOINT);
	if (itor == apppathinfo.playlistParams.end())
	{
		bRet = false;

		char errMsg[1024]="";
		sprintf(errMsg, "applicatoin-path[%s] has no [%s] parameter", apppathinfo.pattern.c_str(), LAMENDPOINT);
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);
		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_331, "[%s] applicatoin-path[%s] has no [%s] parameter", 
			ident.name.c_str(), apppathinfo.pattern.c_str(), LAMENDPOINT);
	}

	std::string& lamendpoint = itor->second.value;

	/*add here*/
	for(AppDataPatternMAP::iterator it_plappdata = apppathinfo.playListAppDataMap.begin();
		it_plappdata != apppathinfo.playListAppDataMap.end(); ++it_plappdata)
	{
		::TianShanIce::ValueMap::iterator privateItor;

		std::string AppKey = ClientRequestPrefix + it_plappdata->second.param;
		privateItor =  privData.find(AppKey);

		if(privateItor != privData.end())
		{
			::TianShanIce::Variant var = privateItor->second;
			std::string MatchStr = var.strs[0];

			boost::regex AppDataRegex(it_plappdata->second.pattern);

			if(!boost::regex_match(MatchStr.c_str(), AppDataRegex))
			{
				continue;
			}

			PARAMMAP::iterator itor = it_plappdata->second.appDataParammap.find(LAMENDPOINT);
			if (itor != it_plappdata->second.appDataParammap.end())
			{
				lamendpoint = itor->second.value;		
				break;
			}	
		}
	}
	glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "Get LAM getplay list interface endpoint is [%s]"), lamendpoint.c_str());

	const char* playlistId = NULL;
	playlistId = urlObj.getVar("playlistid");// rtsp://CatvOfChangNing/60010003?playlist=1
	if(playlistId == NULL || strlen(playlistId) < 1)
	{	
		bRet = false;

		char errMsg[1024]="";
		sprintf(errMsg, "no playlistid in url");
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);
		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_324, "[%s] no playlistid in url", ident.name.c_str());
	}

//	if(stricmp(apppathinfo.playlistModule.c_str(), "Internal") == 0)
	if(0)//永远都走PlugIn调用
	{
		// do: get SurfForTianshan interface proxy
		::com::izq::surf::integration::tianshan::SurfForTianshanPrx SurfPrx = NULL;
		try
		{
			SurfPrx = com::izq::surf::integration::tianshan::SurfForTianshanPrx::checkedCast(_env._iceComm->stringToProxy(lamendpoint));
		}
		catch (const Ice::ObjectNotExistException&)
		{
			bRet = false;
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_320, "[%s] SurfForTianshan proxy not exists at endpoint[%s]", 
				ident.name.c_str(), lamendpoint.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			bRet = false;
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_321, "[%s] get SurfForTianshan proxy caught [%s] at endpoint[%s]", 
				ident.name.c_str(), ex.ice_name().c_str(), lamendpoint.c_str());
		}

		try
		{
			SurfAEData  = SurfPrx->getAEList(playlistId);
		}
		catch (const com::izq::surf::integration::tianshan::SurfException& surfex) 
		{
			bRet = false;
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_322, "[%s] getAEList() caught SurfException %s", 
				ident.name.c_str(), surfex.errorDescription.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			bRet = false;
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_323, "[%s] getAEList() caught ICEException%s", 
				ident.name.c_str(), ex.ice_name().c_str());
		}

		for(unsigned int i = 0; i < SurfAEData.aeList.size(); i++)
		{
			::com::izq::surf::integration::tianshan::AEInfo& surfaeInfo = SurfAEData.aeList[i];
			::com::izq::am::facade::servicesForIce::AEInfo3 aeInfo;
			aeInfo.name = surfaeInfo.aeUID;
			aeInfo.bandWidth = surfaeInfo.bandWidth;
			aeInfo.cueIn = surfaeInfo.cueIn;
			aeInfo.cueOut = surfaeInfo.cueOut;
			aeInfo.volumeList.clear();
			aeInfo.nasUrls.clear();
			aeInfo.attributes.clear();
			assetElements.push_back(aeInfo);
		}
		bRet = true;
	}
	else
	{     
		int retCode;
		const char * pErrDesc;
		ZQTianShan::Application::MOD::AEReturnData aeRetData;
		ZQTianShan::Application::MOD::IPlayListQuery::PlayListInfo plinfo;

		aeRetData.aeList.clear();
		aeRetData.netIDList.clear();

		plinfo.endpoint = lamendpoint;
		plinfo.ident = ident;
		plinfo.nType = ZQTianShan::Application::MOD::IPlayListQuery::getAeListforSurf;
		plinfo.UID1 = playlistId;
		plinfo.UID2 = "";
		plinfo.UID3 ="";

		if (!gNewCfg.testItems.enable)
		{
			try
			{
				retCode = _env._MHOHelperMgr.getPlayList(apppathinfo.playlistEntry.c_str(), plinfo, aeRetData);
			}
			catch(TianShanIce::ServerError&ex)
			{
				bRet = false;
				glog(ZQ::common::Log::L_ERROR,
					PurchaseFmt(ModPurchaseImpl,"getAEForTianShanSurf()get play list(%s,%d,%s)"),
					ex.category.c_str(), ex.errorCode, ex.message.c_str());

				char errMsg[1024]="";
				sprintf(errMsg, "%s",ex.message.c_str());
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;

				if(ex.errorCode == ZQTianShan::Application::MOD::IPlayListQuery::PLNOTEXIST)
					ex.errorCode = err_310;

				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,LOG_PURCHASE, ex.errorCode, "[%s]failed to get play list with error(%s)", ident.name.c_str(), ex.message.c_str() );
			}
			catch(...)
			{
				bRet = false;
				glog(ZQ::common::Log::L_ERROR,
					PurchaseFmt(ModPurchaseImpl,"getAEFromLAM()get play list caught unknown exception(%d)"),
					SYS::getLastErr());

				char errMsg[1024]="";
				sprintf(errMsg, "get play list caught unknown exception(%d)", SYS::getLastErr());
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;

				retCode = err_322;
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,LOG_PURCHASE, retCode, "[%s]get play list caught unknown exception(%d)", ident.name.c_str(), SYS::getLastErr());
			}

			pErrDesc = _env._MHOHelperMgr.getErrorMsg(retCode);

			if(retCode != ZQTianShan::Application::MOD::IPlayListQuery::PLQUERYSUCCESS)
			{
				bRet = false;

				char errMsg[1024]="";
				sprintf(errMsg, "get play list caught exception(%s)", pErrDesc);
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;

				if(retCode == ZQTianShan::Application::MOD::IPlayListQuery::PLNOTEXIST)
					retCode = err_310;

				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,LOG_PURCHASE, retCode, "[%s]get play list caught exception(%s)", ident.name.c_str(), pErrDesc);
			}
		}
		else
		{
			TestElementMod(aeRetData, privData);
		}

		glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl," GetPlayList successfully"));

		for(unsigned int i = 0; i < aeRetData.aeList.size(); i++)
		{
			ZQTianShan::Application::MOD::AEInfo& surfaeInfo = aeRetData.aeList[i];
			::com::izq::am::facade::servicesForIce::AEInfo3 aeInfo;
			aeInfo.name = surfaeInfo.aeUID;
			aeInfo.bandWidth = surfaeInfo.bandWidth;
			aeInfo.cueIn = surfaeInfo.cueIn;
			aeInfo.cueOut = surfaeInfo.cueOut;
			aeInfo.volumeList.clear();
			aeInfo.nasUrls.clear();
			aeInfo.attributes.clear();
			assetElements.push_back(aeInfo);
		}
		//add netid list to weiwoo resource. 
		if (aeRetData.netIDList.size() > 0)
		{				
			TianShanIce::SRM::Resource rsStorageFromLAM;
			rsStorageFromLAM.status = TianShanIce::SRM::rsRequested;
			rsStorageFromLAM.attr = TianShanIce::SRM::raMandatoryNegotiable;
			TianShanIce::Variant storeNetIds;
			storeNetIds.type = TianShanIce::vtStrings;
			storeNetIds.bRange = false;
			storeNetIds.strs = SurfAEData.netIDList;
			rsStorageFromLAM.resourceData.insert(::TianShanIce::ValueMap::value_type("NetworkId", storeNetIds));	

			char szBuf[1024];
			memset(szBuf, 0, 1024);
			snprintf(szBuf, sizeof(szBuf) - 1, "[%s]storage netID collection from LAM  ",ident.name.c_str());
			ZQTianShan::dumpValueMap(rsStorageFromLAM.resourceData, szBuf, dumpLine);

			TianShanIce::SRM::Resource resResult;
			TianShanIce::SRM::ResourceMap rsMap;
			try
			{	
				rsMap = weiwoo->getReources();
			}
			catch (Ice::Exception& ex)
			{	
				glog(ZQ::common::Log::L_WARNING, PurchaseFmt(ModPurchaseImpl, "get resource from weiwoo caught ice exception(%s)"), ex.ice_name().c_str());
			}
			TianShanIce::SRM::ResourceMap::iterator itStorge = rsMap.find(TianShanIce::SRM::rtStorage);
			if (rsMap.end() != itStorge)
			{
				memset(szBuf, 0, 1024);
				snprintf(szBuf, sizeof(szBuf) - 1, "[%s]weiwoo storage netID collection  ",ident.name.c_str());
				ZQTianShan::dumpValueMap(itStorge->second.resourceData, szBuf, dumpLine);

				if (!ZQTianShan::InterRestrictResource(rsStorageFromLAM, itStorge->second, resResult))
				{
					bRet =false;

					char errMsg[1024]="";
					sprintf(errMsg, "No apposite volume/lib supplies the playlist");
					::TianShanIce::Variant varError;
					varError.bRange = false;
					varError.type = TianShanIce::vtStrings;
					varError.strs.push_back(errMsg);
					MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

					purPrivData = privData;

					ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_318, "No apposite volume/lib supplies the playlist");
				}
			}
			else
			{
				glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "weiwoo doesn't have storage netID resource, use storage netID resource from LAM"));
				resResult = rsStorageFromLAM;
			}

			memset(szBuf, 0, 1024);
			snprintf(szBuf, sizeof(szBuf) - 1, "[%s]subset storage netID collection  ",ident.name.c_str());
			ZQTianShan::dumpValueMap(resResult.resourceData, szBuf, dumpLine);

			try
			{	
				weiwoo->addResource(TianShanIce::SRM::rtStorage, TianShanIce::SRM::raMandatoryNonNegotiable, resResult.resourceData);
			}
			catch (Ice::Exception& ex)
			{	
				glog(ZQ::common::Log::L_WARNING, PurchaseFmt(ModPurchaseImpl, "add resource to weiwoo caught ice exception(%s)"), ex.ice_name().c_str());

				bRet = false;

				char errMsg[1024]="";
				sprintf(errMsg, "add resource to weiwoo caught ice exception(%s)", ex.ice_name().c_str());
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;

				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog,LOG_PURCHASE, err_319, 
					"[%s]add resource to weiwoo caught ice exception(%s)",ident.name.c_str(), ex.ice_name().c_str());
			}
		}
		bRet = true;
	}

	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Leave getAEForTianShanSurf()"));
	return bRet;
}

bool ModPurchaseImpl::AuthorOnOTE(ZQ::common::Config::Holder<Urlpattern>& apppathinfo, ::TianShanIce::ValueMap& privData)
{
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Enter Authorization()"));

	int bRet = true;
	
	PARAMMAP::iterator itor = apppathinfo.authParams.find(OTEENDPOINT);
	if (itor ==  apppathinfo.authParams.end())
	{
		enableAuthorize = false;
		bRet = false;

		char errMsg[1024]="";
		sprintf(errMsg, "application-path[%s] has no [%s] parameter", apppathinfo.pattern.c_str(), OTEENDPOINT);
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);
		MAPSET(TianShanIce::ValueMap, privData, AuthorLastError, varError);

		purPrivData = privData;

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_332, "Authorization() [%s] application-path[%s] has no [%s] parameter", 
			ident.name.c_str(), apppathinfo.pattern.c_str(), OTEENDPOINT);
	}
	
	std::string& OTEendpoint = itor->second.value;

	int activeconsize = DefaultActiveConnectSize;
	itor = apppathinfo.authParams.find(PD_KEY_ActiveConnectSize);
	if(itor != apppathinfo.authParams.end())
	{
		activeconsize= atoi((itor->second.value).c_str());
	}
	if(activeconsize < 1)
	{
		activeconsize = DefaultActiveConnectSize;
	}
	TianShanIce::Variant vActiveConnectSize;
	vActiveConnectSize.type = TianShanIce::vtInts;
	vActiveConnectSize.ints.push_back(activeconsize);
	privData[PD_KEY_ActiveConnectSize] = vActiveConnectSize;

	/*add here*/
	for(AppDataPatternMAP::iterator it_authappdata = apppathinfo.authAppDataMap.begin();
		it_authappdata != apppathinfo.authAppDataMap.end(); ++it_authappdata)
	{
		::TianShanIce::ValueMap::iterator privateItor;

		std::string AppKey = ClientRequestPrefix + it_authappdata->second.param;
		privateItor =  privData.find(AppKey);

		if(privateItor != privData.end())
		{
			::TianShanIce::Variant var = privateItor->second;
			std::string MatchStr = var.strs[0];

			boost::regex AppDataRegex(it_authappdata->second.pattern);

			if(!boost::regex_match(MatchStr.c_str(), AppDataRegex))
			{
				continue;
			}

			PARAMMAP::iterator itor = it_authappdata->second.appDataParammap.find(OTEENDPOINT);
			if (itor != it_authappdata->second.appDataParammap.end())
			{
				OTEendpoint = itor->second.value;		
				break;
			}	
		}
	}
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, 
		"Authorization() authorize  endpoint is  [%s], ActiveConnectSize [%d]"), OTEendpoint.c_str(), activeconsize);

	authEndpoint = OTEendpoint;

//	if(stricmp(apppathinfo.authModule.c_str(), "Internal") == 0)
	if(0)//永远都走PlugIn调用
	{
		::com::izq::ote::tianshan::SessionData sd;
		::com::izq::ote::tianshan::SessionResultData rd;
		sd.serverSessionId = serverSessionId;
		sd.clientSessionId = clientSessionId;
		
		for (::TianShanIce::ValueMap::iterator mItor = privData.begin(); mItor != privData.end(); mItor ++)
		{
			if (((::TianShanIce::Variant)(mItor->second)).strs.size() == 0)
				continue;
			std::string keyStr = mItor->first;
			if (stricmp(String::nLeftStr(keyStr, strlen(ClientRequestPrefix)).c_str(), ClientRequestPrefix) == 0)
			{// if key has prefix of "ClientRequest#", omit it.
				keyStr = String::getRightStr(keyStr, "#", true);
			}
			// use insert can avoid overwritting the existing key-value
			sd.params.insert(::com::izq::ote::tianshan::Properties::value_type(keyStr, ((::TianShanIce::Variant)(mItor->second)).strs[0]));
		}
		
		// DO: load configure authorization parameter
		for (PARAMMAP::iterator itTestAuth = gNewCfg.testAuthor.authorParam.begin(); 
		itTestAuth != gNewCfg.testAuthor.authorParam.end() && gNewCfg.testAuthor.enable; itTestAuth ++)
		{
			sd.params.insert(::com::izq::ote::tianshan::Properties::value_type(itTestAuth->first, itTestAuth->second.value));
		}
		
		SYS::TimeStamp st;
		char strTime[48];
		sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d", st.year,st.month,st.day,st.hour,st.minute,st.second);
		sd.params["ViewBeginTime"] = strTime;

		// begin find virtaulsiteName and apppath 
		std::string virtualSiteTemp = "";
		std::string apppathTemp = "";
		std::string fullurl = "";

		TianShanIce::ValueMap::iterator privator;
		privator = privData.find(PD_KEY_SiteName);
		if(privator != privData.end())
		{
			TianShanIce::Variant urlVar = privator->second;
			if (TianShanIce::vtStrings == urlVar.type && urlVar.strs.size() > 0)
			{
				virtualSiteTemp = urlVar.strs[0];
			}
		}

		privator = privData.find(PD_KEY_Path);
		if(privator != privData.end())
		{
			TianShanIce::Variant urlVar = privator->second;
			if (TianShanIce::vtStrings == urlVar.type && urlVar.strs.size() > 0)
			{
				apppathTemp = urlVar.strs[0];
			}
		}

		privator = privData.find(PD_KEY_URL);
		if(privator != privData.end())
		{
			TianShanIce::Variant urlVar = privator->second;
			if (TianShanIce::vtStrings == urlVar.type && urlVar.strs.size() > 0)
			{
				fullurl = urlVar.strs[0];
			}
		}
		// end find virtaulsiteName and apppath 

		//add extra params
		sd.params[PD_KEY_SiteName]= virtualSiteTemp;
		sd.params[PD_KEY_Path]= apppathTemp;
		sd.params[PD_KEY_URL]= fullurl;

		
		// dump the values
		for (::com::izq::ote::tianshan::Properties::iterator pItor = sd.params.begin(); pItor != sd.params.end(); pItor ++)
		{
			glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "authorize: [%s] = [%s]"), pItor->first.c_str(), pItor->second.c_str());
		}
		
		::com::izq::ote::tianshan::MoDIceInterfacePrx otePrx = NULL;
		try
		{
			otePrx = ::com::izq::ote::tianshan::MoDIceInterfacePrx::checkedCast(_env._iceComm->stringToProxy(OTEendpoint));
			rd = otePrx->sessionSetup(sd);
			if (rd.status == "2")
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_315, "[%s] authorization setup failed with error [%s: %s]", 
					ident.name.c_str(), rd.errorCode.c_str(), oteGetErrorDesc(atoi(rd.errorCode.c_str())));
			}
			glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "authorization setup successfully"));
		}
		catch (const TianShanIce::BaseException& ex)
		{
			bRet = false;
			_IceReThrow(TianShanIce::ServerError, ex);
		}
		catch (const Ice::ObjectNotExistException& ex)
		{
			bRet = false;
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_302, "[%s] authorization setup caught %s, endpoint is %s", 
				ident.name.c_str(), ex.ice_name().c_str(), OTEendpoint.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			bRet = false;
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_302, "[%s] authorization setup caught %s, endpoint is %s", 
				ident.name.c_str(), ex.ice_name().c_str(), OTEendpoint.c_str());
		}
		
		bRet = true;
	}
	else
	{
		int retCode;
		const char* pErrDesc = NULL;
		ZQTianShan::Application::MOD::IAuthorization::AuthorInfo authorinfo;
		
		authorinfo.endpoint = OTEendpoint;
		authorinfo.ident = ident;
		authorinfo.clientSessionId = clientSessionId;
		authorinfo.serverSessionId = serverSessionId;
		
		try
		{	
			retCode = _env._MHOHelperMgr.OnAuthPurchase(apppathinfo.authEntry.c_str(), authorinfo, privData);
		}
		catch(TianShanIce::ServerError&ex)
		{
			bRet = false;
			glog(ZQ::common::Log::L_ERROR,
				PurchaseFmt(ModPurchaseImpl,"Authorization(%s,%d,%s)"),
				ex.category.c_str(), ex.errorCode, ex.message.c_str());

			char errMsg[1024]="";
			sprintf(errMsg, "%s",ex.message.c_str());
			::TianShanIce::Variant varError;
			varError.bRange = false;
			varError.type = TianShanIce::vtStrings;
			varError.strs.push_back(errMsg);
			MAPSET(TianShanIce::ValueMap, privData, AuthorLastError, varError);

			purPrivData = privData;

			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog,LOG_PURCHASE, ex.errorCode, "[%s]Authorization (%s)",
				ident.name.c_str(),  ex.message.c_str() );
		}
		catch(...)
		{
			bRet = false;
			glog(ZQ::common::Log::L_ERROR,
				PurchaseFmt(ModPurchaseImpl,"Authorization caught unknown exception(%d)"),
				SYS::getLastErr());

			char errMsg[1024]="";
			sprintf(errMsg, "Authorization caught unknown exception(%d)", SYS::getLastErr());
			::TianShanIce::Variant varError;
			varError.bRange = false;
			varError.type = TianShanIce::vtStrings;
			varError.strs.push_back(errMsg);
			MAPSET(TianShanIce::ValueMap, privData, AuthorLastError, varError);

			purPrivData = privData;

			retCode = err_322;
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, retCode, "[%s] Authorization caught unknown excepiotn(%d)",
				ident.name.c_str(), SYS::getLastErr());
		}
    pErrDesc = _env._MHOHelperMgr.getErrorMsg(retCode);
		if(retCode != ZQTianShan::Application::MOD::IAuthorization::AUTHORSUCCESS)
		{
			bRet = false;

			char errMsg[1024]="";
			sprintf(errMsg, "Authorization caught exception(%s)", pErrDesc);
			::TianShanIce::Variant varError;
			varError.bRange = false;
			varError.type = TianShanIce::vtStrings;
			varError.strs.push_back(errMsg);
			MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

			purPrivData = privData;

			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, retCode, "[%s] Authorization caught exception[%s]", 
				ident.name.c_str(), pErrDesc);
		}	

		glog(ZQ::common::Log::L_INFO,
			PurchaseFmt(ModPurchaseImpl,"Authorization successful at [%s], ActiveConnectSize [%d]"),authEndpoint.c_str(),activeconsize );
		bRet = true;
	}
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Leave Authorization()"));
	return bRet;
}
bool ModPurchaseImpl::getAssetInfo(ZQ::common::Config::Holder<Urlpattern>& apppathinfo, ZQ::common::URLStr& urlObj, ::TianShanIce::ValueMap& privData)
{
	bool bRet = true;
	const char* assetv = NULL;
	const char* assetUIDv = NULL;
	std::string provdId;
	std::string provdAssetId; 
	std::string assetUid;

	assetv = urlObj.getVar("asset");
	assetUIDv = urlObj.getVar("assetuid");

	if(assetv != NULL && strlen(assetv) >0)
	{
		provdId = String::getLeftStr(assetv, "#");
		provdAssetId = String::getRightStr(assetv, "#");

		//stupid call before authorization for get ProviderId (JiangsuYouxian)
		if(apppathinfo.plAPID.enable && provdId.empty())
		{
			_providerId = "";
			if(!getProviderId(apppathinfo, provdAssetId, privData))
			{
				bRet = false;
				return bRet;
			}
		}
		else
			_providerId = provdId;

		// save information here in order to authorize futher
		TianShanIce::Variant vProvdId, vProvdAssetId;
		vProvdId.type = TianShanIce::vtStrings;
		vProvdId.strs.push_back(_providerId);
		vProvdAssetId.type = TianShanIce::vtStrings;
		vProvdAssetId.strs.push_back(provdAssetId);
		privData[ProviderId] = vProvdId;
		privData[ProviderAssetId] = vProvdAssetId;
	}
	else if(assetUIDv != NULL && strlen(assetUIDv) > 0)
	{
		assetUid = assetUIDv;

		// save information here in order to authorize futher
		TianShanIce::Variant vAssetUid;
		vAssetUid.type = TianShanIce::vtStrings;
		vAssetUid.strs.push_back(assetUid);
		privData[AssetId] = vAssetUid;
	}
	else
	{
		bRet = false;
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_307, "[%s] no asset or assetUID found in url", ident.name.c_str());
	}
	return bRet;
}
void fixup(::TianShanIce::ValueMap& privData,  const std::string& oldValue, std::string& newValue)
{
	std::string tempOldvalue = oldValue;
	std::string::iterator itor = tempOldvalue.begin();
	std::string::size_type index = 0;

	newValue = oldValue;
	while (itor != tempOldvalue.end())
	{
		int n_begin_pos = tempOldvalue.find("${", index);
		if(n_begin_pos < 0)
			break;
		int n_end_pos = tempOldvalue.find("}", index);

		if(n_end_pos <= n_begin_pos)
			break;
		std::string tempKey = tempOldvalue.substr(n_begin_pos, n_end_pos - n_begin_pos + 1 );
		std::string AppKey = ClientRequestPrefix + tempKey;
		::TianShanIce::ValueMap::iterator  privateItor =  privData.find(AppKey);
		if(privateItor != privData.end())
		{
			newValue.replace(n_begin_pos, n_end_pos  - n_begin_pos + 1, privateItor->second.strs[0]);
		}
		else
		   index = n_end_pos + 1;
		tempOldvalue = newValue;
		itor = tempOldvalue.begin();
	}
}
void ModPurchaseImpl::TestElementMod(ZQTianShan::Application::MOD::AEReturnData& aeRetData,::TianShanIce::ValueMap& privData)
{
	// if testMode is on, get the asset element from the config file
	for (unsigned int i = 0, count =gNewCfg.testItems.testitems.size(); i < count; i ++)
	{
		/*std::string assetname = gNewCfg.testItems.testitems[i].name;
		std::string strbandwidth = gNewCfg.testItems.testitems[i].bandWidth;
		{
			::TianShanIce::ValueMap::iterator privateItor;

			std::string AppKey = ClientRequestPrefix + assetname;
			privateItor =  privData.find(AppKey);

			if(privateItor != privData.end())
			{
				::TianShanIce::Variant var = privateItor->second;
				assetname = var.strs[0];	
			}

			AppKey = ClientRequestPrefix + strbandwidth;
			privateItor =  privData.find(AppKey);

			if(privateItor != privData.end())
			{
				::TianShanIce::Variant var = privateItor->second;
				strbandwidth = var.strs[0];	
			}
		}*/
		std::string strAssetname, strBandwidth, strCueIn, strCueOut;
		
		fixup(privData, gNewCfg.testItems.testitems[i].name, strAssetname);
		fixup(privData, gNewCfg.testItems.testitems[i].bandWidth, strBandwidth);
		char strTemp[64] ="";
		sprintf(strTemp, "%d\0", gNewCfg.testItems.testitems[i].cueIn);
		fixup(privData, strTemp, strCueIn);

		memset(strTemp, 0, 64);
		sprintf(strTemp, "%d\0", gNewCfg.testItems.testitems[i].cueOut);
		fixup(privData, strTemp, strCueOut);
		 
		int nbandwidth = atoi(strBandwidth.c_str());
		int ncuein = atoi(strCueIn.c_str());
		int ncueout = atoi(strCueOut.c_str());

		ZQTianShan::Application::MOD::AEInfo aeInfo;
		aeInfo.aeUID = strAssetname;
		aeInfo.cueIn = ncuein;
		aeInfo.cueOut = ncueout;
		aeInfo.bandWidth = nbandwidth;
		aeInfo.volumeList = gNewCfg.testItems.testitems[i].volumes;
		if(gNewCfg.testItems.testitems[i].nasurls.size() > 0)
			aeInfo.nasUrls.push_back(gNewCfg.testItems.testitems[i].nasurls);
		else
			aeInfo.nasUrls.clear();
		for(PARAMMAP::const_iterator it_user = gNewCfg.testItems.testitems[i].userpropmap.begin(); it_user != gNewCfg.testItems.testitems[i].userpropmap.end(); it_user++)
		{
			std::string newParam;
			fixup(privData, it_user->second.value, newParam);
			aeInfo.attributes[it_user->second.name] = newParam;
		}
		aeRetData.aeList.push_back(aeInfo);

		std::string strAttrs;
		ZQTianShan::Application::MOD::AttributesMap::iterator itor;
		for(itor = aeInfo.attributes.begin();  itor != aeInfo.attributes.end(); itor++)
		{
			strAttrs += itor->first.c_str() + std::string("=") + itor->second.c_str() + std::string(";");
		}
		glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "TestMod() assetname[%s] bandwidth[%d] cueIn[%d] cueOut[%d] attributes[%s]"),
			aeInfo.aeUID.c_str(), aeInfo.bandWidth, aeInfo.cueIn, aeInfo.cueOut, strAttrs.c_str());

	}	
	aeRetData.netIDList.clear();
}
void ModPurchaseImpl::sendStatusNotice(const ::Ice::Current& c)
{

}


::TianShanIce::Application::PlaylistInfo ModPurchaseImpl::getPlaylistInfo(const ::Ice::Current& c) const
{
	::TianShanIce::Application::PlaylistInfo plInfo;
	if (assetElements.size() <= 0)
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(glog, "ModPurchase", 500, "getPlaylistInfo() not allowed pirior to provision()");

#pragma message("TODO: Implement code to flush logs etc. in the face of an unhandled exception")
//	ZQTianShan::_IceThrow<TianShanIce::NotSupported>(glog, "ModPurchase", 500, "getPlaylistInfo() not implemented");
	int playAdOnce = 0;
	// find apppath config;
	URLPATLIST::iterator pathitor; 
	for(pathitor = gNewCfg.urlpattern.begin(); pathitor != gNewCfg.urlpattern.end(); pathitor++)
	{
		boost::regex ApppathRegex((*pathitor).pattern);
		std::string temp = (*pathitor).pattern;
		boost::cmatch result; 
		if(!boost::regex_search(appPath.c_str(), result , ApppathRegex))
		{  
			continue;
		}
		else
		{
			if (result.size() >2 && result[1].matched && result[2].matched) 
			{
				playAdOnce = (*pathitor).plRender.playAdOnce;
				break;
			}
		}
	}

	for(uint i = 0; i < assetElements.size(); i++)
	{		
		TianShanIce::Streamer::PlaylistItemSetupInfo info;
		//info.contentName = assetElements[i].aeUID;
		info.contentName = assetElements[i].name;
		info.inTimeOffset = assetElements[i].cueIn;
		info.outTimeOffset = assetElements[i].cueOut;
		info.forceNormal = false;
		info.flags = 0;
		info.spliceIn = false;
		info.spliceOut = false;
		info.criticalStart = 0;
		info.privateData.clear();

		bool bIsAD = false;
		if (assetElements[i].attributes.size() > 0)
		{
			com::izq::am::facade::servicesForIce::AttributesMap::const_iterator itor;
			for(itor = assetElements[i].attributes.begin(); itor !=  assetElements[i].attributes.end(); itor++)
			{
				com::izq::am::facade::servicesForIce::AttributesMap::const_iterator itorforAds;
				itorforAds = assetElements[i].attributes.find(ADM_ISAD);
				if (itorforAds != assetElements[i].attributes.end() && itorforAds->second == "1")
				{
					bIsAD = true;
				}
				TianShanIce::Variant userVar;
				userVar.bRange = false;
				userVar.type = TianShanIce::vtStrings;
				userVar.strs.push_back(itor->second);
				info.privateData[itor->first] = userVar;
				if(bIsAD)
				{
					if(itor->first == ADM_FORBIDFF)
					{
						if(itor->second == "1")
							info.flags |= TianShanIce::Streamer::PLISFlagNoFF;
					}
					else  if(itor->first == ADM_FORBIDREW)
					{
						if(itor->second == "1")
							info.flags |= TianShanIce::Streamer::PLISFlagNoRew;
					}
					else  if(itor->first == ADM_FORBIDPAUSE)
					{
						if(itor->second == "1")
							info.flags |= TianShanIce::Streamer::PLISFlagNoPause;
					}
					else  if(itor->first == ADM_FORBIDSTOP)
					{
						//if(itor->second == "1")
						//	info.flags |=;
					}
					else  if(itor->first == ADM_FORBIDNOSEEK)
					{
						if(itor->second == "1")
							info.flags |= TianShanIce::Streamer::PLISFlagNoSeek;
					}
					else  if(itor->first == ADM_SKIPATFF)
					{
						if(itor->second == "1")
							info.flags |= TianShanIce::Streamer::PLISFlagSkipAtFF;
					}
					else  if(itor->first == ADM_SKIPATREW)
					{
						if(itor->second == "1")
							info.flags |= TianShanIce::Streamer::PLISFlagSkipAtRew;
					}
				}
			}

			if(bIsAD)
			{
				int adPlayTimes = 0;
				itor = assetElements[i].attributes.find(ADM_PLAYTIMES);
				if(itor != assetElements[i].attributes.end())
				{
					adPlayTimes = atoi(itor->second.c_str());
					if(adPlayTimes > 9)
						adPlayTimes = 9;
					else if(adPlayTimes < 0)
						adPlayTimes = 0;
				}
				else if(playAdOnce)
					adPlayTimes = 1;

				if(adPlayTimes > 0  && adPlayTimes <= 9)
					info.flags |= adPlayTimes << 4;
			}
		}
		else
		{
			info.privateData.clear();
		}

		int npos = assetElements[i].name.find("_");
		if(npos > 0)
		{
			std::string paid = assetElements[i].name.substr(0, npos);
			std::string pid = assetElements[i].name.substr(npos +1);

			TianShanIce::Variant userVar;
			userVar.bRange = false;
			userVar.type = TianShanIce::vtStrings;

			userVar.strs.push_back(paid);
			info.privateData["ProviderAssetId"] = userVar;

			userVar.strs.clear();
			userVar.strs.push_back(pid);
			info.privateData["ProviderId"] = userVar;
		}

		plInfo.push_back(info);
	}
	return plInfo;
}

std::string fixupAssetName(const std::string& paid, const std::string& pid, const std::string& aeNameFormat)
{
	std::string aeName = aeNameFormat;

	int npos = aeName.find("${PAID}");
	if(npos >= 0)
	{
		aeName.replace(npos, strlen("${PAID}"), paid);
	}
	npos = aeName.find("${PID}");
	if(npos >= 0)
	{
		aeName.replace(npos, strlen("${PID}"), pid);
	}
	return aeName;
}
bool ModPurchaseImpl::getAEFromNameFormatter(ZQ::common::Config::Holder<Urlpattern>& apppathinfo, ZQ::common::URLStr& urlObj, ::TianShanIce::ValueMap& privData)
{
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Enter getAEFromNameFormatter()"));
	int bRet = true;

	const char* assetv = NULL;
	std::string provdId;
	std::string provdAssetId; 

	assetv = urlObj.getVar("asset");

	if(assetv != NULL && strlen(assetv) > 0)
	{
		std::string tempProAssetID;
		provdId = String::getLeftStr(assetv, "#");
		provdAssetId = String::getRightStr(assetv, "#");

		if(provdId.empty()|| provdAssetId.empty())
		{
			bRet = false;

			char errMsg[1024]="";
			sprintf(errMsg, "invaild url, no RegionID or coursehourID");
			::TianShanIce::Variant varError;
			varError.bRange = false;
			varError.type = TianShanIce::vtStrings;
			varError.strs.push_back(errMsg);
			MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

			purPrivData = privData;

			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_307, "[%s] no asset parameter found in url", ident.name.c_str());

			provdId = _providerId;
		}

		// save information here in order to authorize futher
		TianShanIce::Variant vProvdId, vProvdAssetId;
		vProvdId.type = TianShanIce::vtStrings;
		vProvdId.strs.push_back(provdId);

		vProvdAssetId.type = TianShanIce::vtStrings;
		vProvdAssetId.strs.push_back(provdAssetId);

		privData[ProviderId] = vProvdId;
		privData[ProviderAssetId] = vProvdAssetId;
	}
	else
	{
		bRet = false;

		char errMsg[1024]="";
		sprintf(errMsg, "no asset parameter in url");
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);
		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_307, "[%s] no asset parameter found in url", ident.name.c_str());
	}

	try
	{	
		TianShanIce::ValueMap resourceData;
		::TianShanIce::SRM::ResourceAttribute resAttribute = TianShanIce::SRM::raMandatoryNonNegotiable;
		if(!apppathinfo.resRestriction.storageNetIds.empty())
		{
			TianShanIce::Variant storeNetIds;
			storeNetIds.type = TianShanIce::vtStrings;
			storeNetIds.bRange = false;
			storeNetIds.strs = apppathinfo.resRestriction.storageNetIds;
			resourceData.insert(::TianShanIce::ValueMap::value_type("NetworkId", storeNetIds));
			weiwoo->addResource(TianShanIce::SRM::rtStorage, resAttribute, resourceData);
		}
	}
	catch (Ice::Exception& ex)
	{	
		glog(ZQ::common::Log::L_WARNING, PurchaseFmt(ModPurchaseImpl, "add resource to weiwoo caught ice exception(%s)"), ex.ice_name().c_str());

		bRet = false;

		char errMsg[1024]="";
		sprintf(errMsg, "add resource to weiwoo caught ice exception(%s)", ex.ice_name().c_str());
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);
		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog,LOG_PURCHASE, err_319, 
			"[%s] add resource to weiwoo caught ice exception(%s)", ident.name.c_str(), ex.ice_name().c_str());
	}

	for(size_t i = 0; i < apppathinfo.playlistItems.size(); i++)
	{
		std::string& aeNameFormat = apppathinfo.playlistItems[i].name;
		glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "asset name format is [%s]"), aeNameFormat.c_str());

		std::string aeUID = fixupAssetName(provdAssetId, provdId, aeNameFormat);

		::com::izq::am::facade::servicesForIce::AEInfo3 aeInfo;

		aeInfo.name = aeUID;
		aeInfo.bandWidth = (Ice::Int)atoi(apppathinfo.playlistItems[i].bandWidth.c_str());
		aeInfo.cueIn = apppathinfo.playlistItems[i].cueIn;
		aeInfo.cueOut = apppathinfo.playlistItems[i].cueOut;

		if(apppathinfo.playlistItems[i].volumes.size() > 0)
			aeInfo.volumeList = apppathinfo.playlistItems[i].volumes;

		if(apppathinfo.playlistItems[i].nasurls.size() > 0)
			aeInfo.nasUrls.push_back(apppathinfo.playlistItems[i].nasurls);

		for(PARAMMAP::const_iterator it_user = apppathinfo.playlistItems[i].userpropmap.begin(); it_user != apppathinfo.playlistItems[i].userpropmap.end(); it_user++)
		{
			std::string newParam;
			fixup(privData, it_user->second.value, newParam);
			aeInfo.attributes[it_user->second.name] = newParam;
		}
		assetElements.push_back(aeInfo);
	}

	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Leave getAEFromNameFormatter()"));
	return bRet;
}

bool ModPurchaseImpl::getAEFromMRTStream(ZQ::common::Config::Holder<Urlpattern>& apppathinfo, ZQ::common::URLStr& urlObj, ::TianShanIce::ValueMap& privData)
{
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Enter getAEFromMRTStream()"));
	int bRet = true;

	const char* assetv = NULL;
	std::string provdId;
	std::string provdAssetId; 

	std::string assetname= "";
	assetv = urlObj.getVar("asset");

	if(assetv != NULL && strlen(assetv) > 0)
	{
		assetname = assetv;
		provdId = String::getLeftStr(assetv, "#");
		provdAssetId = String::getRightStr(assetv, "#");

		if(provdId.empty()|| provdAssetId.empty())
		{
			bRet = false;

			char errMsg[1024]="";
			sprintf(errMsg, "invaild url, no PID or PAID");
			::TianShanIce::Variant varError;
			varError.bRange = false;
			varError.type = TianShanIce::vtStrings;
			varError.strs.push_back(errMsg);
			MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

			purPrivData = privData;
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_307, "[%s] no asset parameter found in url", ident.name.c_str());
		}
	}
	else
	{
		bRet = false;

		char errMsg[1024]="";
		sprintf(errMsg, "no asset parameter in url");
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);
		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_307, "[%s] no asset parameter found in url", ident.name.c_str());
	}

	try
	{	
		TianShanIce::ValueMap resourceData;
		::TianShanIce::SRM::ResourceAttribute resAttribute = TianShanIce::SRM::raMandatoryNonNegotiable;
		if(!apppathinfo.resRestriction.storageNetIds.empty())
		{
			TianShanIce::Variant streamNetIds;
			streamNetIds.type = TianShanIce::vtStrings;
			streamNetIds.bRange = false;
			streamNetIds.strs.push_back(gNewCfg.mrtSSnetId + "/*");
			resourceData.insert(::TianShanIce::ValueMap::value_type("NetworkId", streamNetIds));
			weiwoo->addResource(TianShanIce::SRM::rtStreamer, resAttribute, resourceData);
		}
	}
	catch (Ice::Exception& ex)
	{	
		glog(ZQ::common::Log::L_WARNING, PurchaseFmt(ModPurchaseImpl, "add resource to weiwoo caught ice exception(%s)"), ex.ice_name().c_str());

		bRet = false;

		char errMsg[1024]="";
		sprintf(errMsg, "add resource to weiwoo caught ice exception(%s)", ex.ice_name().c_str());
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);
		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog,LOG_PURCHASE, err_319, 
			"[%s] add resource to weiwoo caught ice exception(%s)", ident.name.c_str(), ex.ice_name().c_str());
	}

	for(size_t i = 0; i < apppathinfo.playlistItems.size(); i++)
	{
		::com::izq::am::facade::servicesForIce::AEInfo3 aeInfo;

		aeInfo.name = provdAssetId;
		aeInfo.bandWidth = (Ice::Int)atoi(apppathinfo.playlistItems[i].bandWidth.c_str());
		aeInfo.cueIn = apppathinfo.playlistItems[i].cueIn;
		aeInfo.cueOut = apppathinfo.playlistItems[i].cueOut;

		if(apppathinfo.playlistItems[i].volumes.size() > 0)
			aeInfo.volumeList = apppathinfo.playlistItems[i].volumes;

		if(apppathinfo.playlistItems[i].nasurls.size() > 0)
			aeInfo.nasUrls.push_back(apppathinfo.playlistItems[i].nasurls);

		for(PARAMMAP::const_iterator it_user = apppathinfo.playlistItems[i].userpropmap.begin(); it_user != apppathinfo.playlistItems[i].userpropmap.end(); it_user++)
		{
			std::string newParam;
			fixup(privData, it_user->second.value, newParam);
			aeInfo.attributes[it_user->second.name] = newParam;
		}
		assetElements.push_back(aeInfo);
	}

	TianShanIce::Variant varAssetName;
	varAssetName.bRange = false;
	varAssetName.type = TianShanIce::vtStrings;
	varAssetName.strs.push_back(provdAssetId);
	MAPSET(TianShanIce::ValueMap, privData, SOAP_AssetID, varAssetName);

	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Leave getAEFromMRTStream() took %dms"));
	return bRet;
}
bool ModPurchaseImpl::getURLattribute(ZQ::common::Config::Holder<Urlpattern>& apppathinfo, const std::string& uri, std::string& url, Ice::Int& bitRate)
{
    glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Entry getURLattribute()"));
	Ice::Long lStart = ZQTianShan::now();
	bool bRet = true;
	std::string endpoint, locateRequestIP = "";
	int nport = 0;
	int recvTimeout = 20;//second
  
	PARAMMAP::iterator itor;

	itor =  apppathinfo.playlistParams.find(APSENDPOINT);
	if(itor !=  apppathinfo.playlistParams.end())
		endpoint = itor->second.value;

	itor =  apppathinfo.playlistParams.find(PD_KEY_LocalBind);
	if(itor !=  apppathinfo.playlistParams.end())
      locateRequestIP = itor->second.value;
    
	itor =  apppathinfo.playlistParams.find(PD_KEY_Port);
	if(itor !=  apppathinfo.playlistParams.end())
		nport = atoi(itor->second.value.c_str());

	itor =  apppathinfo.playlistParams.find(PD_KEY_TimeOut);
	if(itor !=  apppathinfo.playlistParams.end())
		recvTimeout = atoi(itor->second.value.c_str());

	
	std::auto_ptr<ZQ::common::HttpClient>	pAutoHttpClient;
	ZQ::common::HttpClient* phttpclient = new ZQ::common::HttpClient();
	if(!phttpclient)
		return false; 
	pAutoHttpClient.reset(phttpclient); 
	pAutoHttpClient->init(ZQ::common::HttpClient::HTTP_IO_KEEPALIVE);
	pAutoHttpClient->setLog(&glog);
	pAutoHttpClient->setLocalHost(locateRequestIP, nport);
    pAutoHttpClient->setKeepAlive(true);
	pAutoHttpClient->setRecvTimeout(recvTimeout);

	pAutoHttpClient->setHeader(NULL,NULL);
//	pAutoHttpClient->setHeader("Content-Type", "application/json");
	pAutoHttpClient->setHeader("Accept", "application/json");

//	ZQ::common::URLStr strURL(endpoint.c_str());
//	strURL.setPath(uri.c_str());
//	std::string urlstr  = strURL.generate();
//	char strEncodeURL[1024] = "";
//	memset(strEncodeURL, 0, sizeof(strEncodeURL));
//	ZQ::common::URLStr::encode(urlstr.c_str(),strEncodeURL, sizeof(strEncodeURL) -1);

	if(endpoint.size() >  0 && endpoint[endpoint.size() - 1] == '/')
		endpoint = endpoint.substr(0, endpoint.size() -1);
	std::string urlstr = endpoint + uri;

	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "connect to Aqua PaaS[%s]"), urlstr.c_str());

	std::string strErrMsg;
	if (pAutoHttpClient->httpConnect(urlstr.c_str(), ZQ::common::HttpClient::HTTP_GET) || pAutoHttpClient->httpEndSend())
	{
		strErrMsg = "failed to connect to url " + urlstr + " with error: " + pAutoHttpClient->getErrorstr();
		glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "get url attribute()[%s]"), strErrMsg.c_str());
		return false;
	}

	std::string strResponseConent;
	if (pAutoHttpClient->httpBeginRecv())
	{
		strErrMsg = std::string("begin receive get url attribute request response with error:") + pAutoHttpClient->getErrorstr();
		pAutoHttpClient->uninit();		
		glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "get url attribute()[%s]"), strErrMsg.c_str());
		return false;
	}

	int status = pAutoHttpClient->getStatusCode();

	if(status != 200)
	{
		strErrMsg = std::string("send get url attribute request with error:") + pAutoHttpClient->getMsg();
		pAutoHttpClient->uninit();	
		glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "get url attribute()[%s]"), strErrMsg.c_str());
		return false;
	}

	std::string strRC = "";
	while(!pAutoHttpClient->isEOF())
	{
		strRC.clear();
		if(pAutoHttpClient->httpContinueRecv())
		{
			strErrMsg = std::string("continue receiver get url attribute request response with error:") + pAutoHttpClient->getErrorstr();
			pAutoHttpClient->uninit();	
			glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "get url attribute()[%s]"), strErrMsg.c_str());
			return false;
		}
		pAutoHttpClient->getContent(strRC);
		strResponseConent += strRC;
	}

	if ( pAutoHttpClient->httpEndRecv() )
	{
		strErrMsg = std::string("finished receiver get url attribute request response with error:")+ pAutoHttpClient->getErrorstr();
		pAutoHttpClient->uninit();	
		glog(ZQ::common::Log::L_ERROR,  PurchaseFmt(ModPurchaseImpl, "get url attribute()[%s]"), strErrMsg.c_str());
		return false;
	}

	pAutoHttpClient->getContent(strRC);
	strResponseConent += strRC;
	pAutoHttpClient->uninit();

	char buf[4096];
	sprintf(buf,  PurchaseFmt(ModPurchaseImpl, "get url attribute response:"));
	glog.hexDump(ZQ::common::Log::L_INFO, strResponseConent.c_str(), strResponseConent.size(), buf, true);

	Json::Value result;
	Json::Reader reader;
	try
	{	
		if(!strResponseConent.empty() && !reader.parse(strResponseConent, result))
		{
			glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "failed to parse response"));
			return false;
		}

		if (JSON_HAS(result, "url"))
			url = result["url"].asString();
		if (JSON_HAS(result, "metadata"))
		{
			Json::Value& metadatas =  result["metadata"];
			if(JSON_HAS(metadatas, "Bit_Rate"))
			{
               bitRate = atoi((metadatas["Bit_Rate"].asString()).c_str());
			}
		}
	}
	catch(std::exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,PurchaseFmt(ModPurchaseImpl, "failed to parse response caught exception[%s]"), ex.what());
		return false;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "failed to parse response caught unknown exception[%d]"),SYS::getLastErr());
		return false;
	}
	glog(ZQ::common::Log::L_INFO,  PurchaseFmt(ModPurchaseImpl, "get url[%s] from Aqua PaaS took %dms"), url.c_str(), ZQ::common::now() - lStart); 
    return bRet;
}
bool ModPurchaseImpl::getAEFromIPTV(ZQ::common::Config::Holder<Urlpattern>& apppathinfo, ZQ::common::URLStr& urlObj, ::TianShanIce::ValueMap& privData)
{
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Enter getAEFromIPTV()"));
	int bRet = true;

	Ice::Long lStart = ZQTianShan::now();
	const char* assetv = NULL;
	std::string provdId;
	std::string provdAssetId; 

	std::string assetname= "";
	assetv = urlObj.getVar("asset");

	if(assetv != NULL && strlen(assetv) > 0)
	{
		assetname = assetv;

		provdId = String::getLeftStr(assetv, "#");
		provdAssetId = String::getRightStr(assetv, "#");

		if(provdId.empty()|| provdAssetId.empty())
		{
			bRet = false;

			char errMsg[1024]="";
			sprintf(errMsg, "invaild url, no PID or PAID");
			::TianShanIce::Variant varError;
			varError.bRange = false;
			varError.type = TianShanIce::vtStrings;
			varError.strs.push_back(errMsg);
			MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

			purPrivData = privData;
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_307, "[%s] no asset parameter found in url", ident.name.c_str());
		}
		// save information here in order to authorize futher
		TianShanIce::Variant vProvdId, vProvdAssetId;
		vProvdId.type = TianShanIce::vtStrings;
		vProvdId.strs.push_back(provdId);

		vProvdAssetId.type = TianShanIce::vtStrings;
		vProvdAssetId.strs.push_back(provdAssetId);

		privData[ProviderId] = vProvdId;
		privData[ProviderAssetId] = vProvdAssetId;
	}
	else
	{
		bRet = false;

		char errMsg[1024]="";
		sprintf(errMsg, "no asset parameter in url");
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);
		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_307, "[%s] no asset parameter found in url", ident.name.c_str());
	}

	Ice::Int bandWidth = 0;
	std::string iptvKey = ClientRequestPrefix + std::string("IPTVURL");
	if(privData.find(iptvKey) == privData.end())
	{
		std::string url, uri;
		uri = std::string("/aquapaas/rest/content/by_ext_id:") + provdId + "%23" + provdAssetId;
		if(!getURLattribute(apppathinfo, uri,url, bandWidth))
		{
			bRet = false;

			char errMsg[1024]="";
			sprintf(errMsg, "failed to find url parameter on Aqua PaaS");
			::TianShanIce::Variant varError;
			varError.bRange = false;
			varError.type = TianShanIce::vtStrings;
			varError.strs.push_back(errMsg);
			MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

			purPrivData = privData;

			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_327, "[%s] failed to find url parameter on Aqua PaaS", ident.name.c_str());
		}

		TianShanIce::Variant  vURL;
		vURL.type = TianShanIce::vtStrings;
		vURL.strs.push_back(url);

		privData[iptvKey] =  vURL;
	}

	if(bandWidth <= 0)
	{
	// get Bandwidth string from resource map
	TianShanIce::SRM::ResourceMap resources;

	try
	{	
		resources = weiwoo->getReources();
	}
	catch (Ice::Exception& ex)
	{	
		bRet = false;

		char errMsg[1024]="";
		sprintf(errMsg, "get resource caught exception(%s)", ex.ice_name().c_str());
		::TianShanIce::Variant varError;
		varError.bRange = false;
		varError.type = TianShanIce::vtStrings;
		varError.strs.push_back(errMsg);
		MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

		purPrivData = privData;

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog,LOG_PURCHASE, err_319, 
			"[%s] get resource caught exception(%s)", ident.name.c_str(), ex.ice_name().c_str());
	}

	TianShanIce::SRM::ResourceMap::iterator itRes = resources.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
	if(itRes == resources.end()
		|| resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData.find("bandwidth") == resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData.end()
		|| resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData["bandwidth"].lints.size() < 1)
	{
		if(gNewCfg.defaultBandWidth > 0)
			bandWidth = gNewCfg.defaultBandWidth;
		else
		{
			bRet = false;

			char errMsg[1024]="";
			sprintf(errMsg, "missing bandwith in resource");
			::TianShanIce::Variant varError;
			varError.bRange = false;
			varError.type = TianShanIce::vtStrings;
			varError.strs.push_back(errMsg);
			MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

			purPrivData = privData;

			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_326, "[%s] missing bandwith in resource, and modsvc defaultBandwidth=0", ident.name.c_str());

			return false;	

		}
	}
	else
		bandWidth = resources[TianShanIce::SRM::rtTsDownstreamBandwidth].resourceData["bandwidth"].lints[0];
	}

	for(size_t i = 0; i < apppathinfo.playlistItems.size(); i++)
	{
		::com::izq::am::facade::servicesForIce::AEInfo3 aeInfo;

		std::string& aeNameFormat = apppathinfo.playlistItems[i].name;
		glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "asset name format is [%s]"), aeNameFormat.c_str());

		std::string aeUID = fixupAssetName(provdAssetId, provdId, aeNameFormat);

		aeInfo.name = aeUID;
		aeInfo.bandWidth = bandWidth;
		aeInfo.cueIn = apppathinfo.playlistItems[i].cueIn;
		aeInfo.cueOut = apppathinfo.playlistItems[i].cueOut;

		if(apppathinfo.playlistItems[i].volumes.size() > 0)
			aeInfo.volumeList = apppathinfo.playlistItems[i].volumes;

		if(apppathinfo.playlistItems[i].nasurls.size() > 0)
			aeInfo.nasUrls.push_back(apppathinfo.playlistItems[i].nasurls);

		for(PARAMMAP::const_iterator it_user = apppathinfo.playlistItems[i].userpropmap.begin(); it_user != apppathinfo.playlistItems[i].userpropmap.end(); it_user++)
		{
			std::string newParam;
			fixup(privData, it_user->second.value, newParam);
			aeInfo.attributes[it_user->second.name] = newParam;
		}
		aeInfo.attributes["PID"] = provdId;
		aeInfo.attributes["PAID"] = provdAssetId;

		assetElements.push_back(aeInfo);
	}

	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Leave getAEFromIPTV() took %dms"), ZQ::common::now() - lStart);
	return bRet;
}
::TianShanIce::ValueMap ModPurchaseImpl::getPrivataData(const ::Ice::Current& c) const
{
   ServantMutex::Lock lk(*this);
   return purPrivData;
}
void ModPurchaseImpl::setPrivateData(const ::std::string& key, const ::TianShanIce::Variant& variant,const ::Ice::Current& c)
{
	ServantMutex::Lock lk(*this);
	MAPSET(TianShanIce::ValueMap, purPrivData, key, variant);
}
void ModPurchaseImpl::setPrivateData2(const ::TianShanIce::ValueMap& privateData, const ::Ice::Current& c)
{
	ServantMutex::Lock lk(*this);
	purPrivData = privateData;
}
void ModPurchaseImpl::overrideCueInCueOut(ZQTianShan::Application::MOD::AEReturnData& aeRetData, int overrideCueIn, int overrideCueOut)
{
	Ice::Long lStart = ZQTianShan::now();
	glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "entry overrideCueInCueOut() overrideCueIn[%d] overrideCueOut[%d]"), overrideCueIn, overrideCueOut);
	for(int i = 0; i  < aeRetData.aeList.size(); i++)
	{
		int maxCueIn = aeRetData.aeList[i].cueIn > overrideCueIn ? aeRetData.aeList[i].cueIn : overrideCueIn;
		int minCueOut =aeRetData.aeList[i].cueOut < overrideCueOut ? aeRetData.aeList[i].cueOut : overrideCueOut;
		//特殊处理 cueOut==0; 直接调整为overrideCueOut 
		if(aeRetData.aeList[i].cueOut  == 0)
			minCueOut  = overrideCueOut;
		if(maxCueIn < minCueOut && (maxCueIn != aeRetData.aeList[i].cueIn  || minCueOut != aeRetData.aeList[i].cueOut))
		{
			glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "[%s] overrideCueIn[%d=>%d] overrideCueOut[%d=>%d]"),
					aeRetData.aeList[i].aeUID.c_str(), aeRetData.aeList[i].cueIn, maxCueIn, aeRetData.aeList[i].cueOut, minCueOut);
			aeRetData.aeList[i].cueIn =  maxCueIn;
			aeRetData.aeList[i].cueOut = minCueOut;
		}
	}
	glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "leave overrideCueInCueOut() took %dms"), ZQ::common::now() - lStart);

}
} // namespace ZQMODApplication
