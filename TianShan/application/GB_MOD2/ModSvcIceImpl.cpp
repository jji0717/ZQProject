#include "ModService.h"
#include "ModSvcIceImpl.h"
#include "MODHelperMgr.h"
#include "SiteDefines.h"
#include "TimeUtil.h"
#include "../GB_MODPlugIn/LAMPlayListQuery3.h"
#include "../GB_MODPlugIn/AAAQuery.h"
#include "../GB_MODPlugIn/HeNanAAAQuery.h"
#include "MODDefines.h"
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
#define AAAError          "authorize.aaaerror"
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
}

ModPurchaseImpl::~ModPurchaseImpl()
{
}

::TianShanIce::SRM::SessionPrx ModPurchaseImpl::getSession(const ::Ice::Current& c) const
{
	ServantMutex::Lock lk(*this);
	return weiwoo;
}

void ModPurchaseImpl::provision(const ::Ice::Current& c)
{
	ServantMutex::Lock lk(*this);
  Ice::Long lStart = ZQTianShan::now();
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
		boost::cmatch m; 
		if(!boost::regex_search(appPath.c_str(), m , ApppathRegex))
		{  
			continue;
		}
		else
		{
			if (m[1].matched && m[2].matched) 
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
	

	bool bUseAAA = false;
	if(apppathinfo.aaa.aaaEntry.size() > 0)
		bUseAAA = true;

	// if not serfpath, get asset info and fill it to private data map
	if((stricmp(pathStr.c_str(), SerfAppPath) != 0) && !bUseAAA)
	{
		if(!getAesstInfo(urlObj, privData))
		{
			return;
		}
		purPrivData = privData;
		enableAuthorize = apppathinfo.Authenable;//如果不是SerfPath， 则enableAuthorize = 配置项
	}
	else
	{
    enableAuthorize = false;//SerfPath, enableAuthorize=fasle;
	}

	if (apppathinfo.Authenable && enableAuthorize && !bUseAAA)
	{
		if(!AuthorOnOTE(apppathinfo, privData))
		{
			return;
		}
	}

	if (stricmp(pathStr.c_str(), SerfAppPath) == 0)
	{
		if(!getAEForTianShanSurf(apppathinfo, urlObj, privData))
		{
			return;
		}
	}
	else 
	{
		if(!getAEFromLAM(apppathinfo, urlObj, privData))
		{
			return;
		}
	}

  purPrivData = privData;
	// if no elements is found according to the specified url, throw exception
	if (assetElements.size() == 0)
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_310, "[%s] no element is found according to the url [%s]", 
			ident.name.c_str(), strUrl.c_str());
	}
	int playAdOnce = 0;
	// find apppath config;
	for(pathitor = gNewCfg.urlpattern.begin(); pathitor != gNewCfg.urlpattern.end(); pathitor++)
	{
		boost::regex ApppathRegex((*pathitor).pattern);
		std::string temp = (*pathitor).pattern;
		boost::cmatch m; 
		if(!boost::regex_search(appPath.c_str(), m , ApppathRegex))
		{  
			continue;
		}
		else
		{
			if (m[1].matched && m[2].matched) 
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

	Ice::Long  remainingPlayTime = 0;
	int maxBW = -1;
	for (unsigned int i = 0, count = assetElements.size(); i < count; i ++)
	{
		if (assetElements[i].bandWidth > maxBW)
			maxBW = assetElements[i].bandWidth;
		if(assetElements[i].cueIn == -1)
			assetElements[i].cueIn = 0;
		if(assetElements[i].cueOut == -1)
			assetElements[i].cueOut = 0;
		
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

		Ice::Int ctrlNum = i + 1;
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

			glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "item[%s] attributes[%s] flags[%s]"), 	
				assetElements[i].name.c_str(), strAttributes.c_str(),flag2str(info.flags).c_str());

		}
		else
		{
			info.privateData.clear();
		}

		if (bValidNptSeq)
		{
			if (!bIsAD)
				primaryNPTs.push_back(npt); 

			npt += itemLen;

			if (!bIsAD)
				primaryNPTs.push_back(npt); 
		}

		_playListItemInfos.push_back(info);
		glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "AEID[%s], cueIn[%d], cueOut[%d], Bitrate[%d], ItemLen[%I64d]"), 
			assetElements[i].name.c_str(), 
			assetElements[i].cueIn, 
			assetElements[i].cueOut, 
			assetElements[i].bandWidth,
			itemLen);
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
		TianShanIce::ValueMap externalPD;

		if (bValidNptSeq && bAdsFound && primaryNPTs.size() >1 && 0 == primaryNPTs.size()%2)
		{
			Ice::Long primaryStart = primaryNPTs[0];
			Ice::Long primaryEnd   = primaryNPTs[primaryNPTs.size()-1];

			::TianShanIce::Variant varPrimaryStart;
			varPrimaryStart.bRange = false;
			varPrimaryStart.type = TianShanIce::vtLongs;
			varPrimaryStart.lints.push_back(primaryStart); //ms
			MAPSET(TianShanIce::ValueMap, externalPD, SYS_PROP(primaryStart), varPrimaryStart);

			TianShanIce::Variant varPrimaryEnd;
			varPrimaryEnd.bRange = false;
			varPrimaryEnd.type = TianShanIce::vtLongs;
			varPrimaryEnd.lints.push_back(primaryEnd); //ms
			MAPSET(TianShanIce::ValueMap, externalPD, SYS_PROP(primaryEnd), varPrimaryEnd);
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
	_env._pWatchDog->watch(ident.name, gNewCfg.noticeTime);

	glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "provision() completed took %d ms"), ZQTianShan::now() - lStart);
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
		TianShanIce::SRM::ResourceMap resmap =  weiwooSessPrx-> getReources();
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
		playlist = ::TianShanIce::Streamer::PlaylistPrx::checkedCast(streamPrx);
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


	for (unsigned int i = 0; i < _playListItemInfos.size(); i ++)
	{
		Ice::Int ctrlNum = i + 1;
		try
		{
			_playListItemInfos[i].contentName = volumename + _playListItemInfos[i].contentName;
			playlist->pushBack(ctrlNum, _playListItemInfos[i]);
			glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "pushBack %s on stream %s"), 
				_playListItemInfos[i].contentName.c_str(), streamId.c_str());
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "pushBack() [%s] on playlist [%s] caught %s: %s"), 
				_playListItemInfos[i].contentName.c_str(), streamId.c_str(), ex.ice_name().c_str(), ex.message.c_str());

			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_703, "pushBack() [%s] on playlist [%s] caught %s: %s", 
				_playListItemInfos[i].contentName.c_str(), streamId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "pushBack() [%s] on playlist [%s] caught %s"), 
				_playListItemInfos[i].contentName.c_str(), streamId.c_str(), ex.ice_name().c_str());

			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, err_704, "pushBack() [%s] on playlist [%s] caught %s", 
				_playListItemInfos[i].contentName.c_str(), streamId.c_str(), ex.ice_name().c_str());
		}
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
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "detach() enter"));
    _stampClean = ZQTianShan::now();
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
		boost::cmatch m; 
		if(!boost::regex_search(appPath.c_str(), m , ApppathRegex))
		{  
			continue;
		}
		else
		{			
			if (m[1].matched && m[2].matched) 
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

	bool bUseAAA = false;
	if(apppathinfo.aaa.aaaEntry.size() > 0)
		bUseAAA = true;

	if (apppathinfo.Authenable && enableAuthorize  || bUseAAA)
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
		if(0)//永远走PlugIn
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
						
			TianShanIce::Properties extraProps;
			extraProps = prop;
			            
			if(bIspurInternal && !purInternalMsg.empty())
			{
				TianShanIce::Properties::iterator itProp;
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
				if(!bUseAAA)
				{
					ZQTianShan::Application::MOD::IAuthorization::AuthorInfo authorinfo;
					authorinfo.endpoint = OTEendpoint;
					authorinfo.ident = ident;
					authorinfo.clientSessionId = clientSessionId;
					authorinfo.serverSessionId = serverSessionId;
					retCode = _env._MHOHelperMgr.OnDestroyPurchase(apppathinfo.authEntry.c_str(), authorinfo, extraProps);
				}
				else
				{
					ZQTianShan::Application::MOD::IAAA::AAAInfo aaaInfo;
					aaaInfo.endpoint = authEndpoint;
					aaaInfo.ident = ident;
					aaaInfo.clientSessionId = clientSessionId;
					aaaInfo.serverSessionId = serverSessionId;
					aaaInfo.sessionID = serverSessionId;

					PARAMMAP::iterator itorpara;
					for(itorpara = apppathinfo.aaa.aaaParams.begin(); itorpara != apppathinfo.aaa.aaaParams.end(); itorpara++)
					{
						aaaInfo.prop[itorpara->second.name] = itorpara->second.value;
					}

					if(apppathinfo.aaa.aaaEntry == LAM_AAAQUERY_Name)
					{
						std::string key = ClientRequestPrefix "EntitlementCode";
						TianShanIce::ValueMap::iterator privateItor =  purPrivData.find(key);
						if(privateItor != purPrivData.end() && privateItor->second.strs.size() > 0)
						{
							aaaInfo.entitlementCode = privateItor->second.strs[0];
						}

						key = ClientRequestPrefix "x-userID";
						privateItor =  purPrivData.find(key);
						if(privateItor != purPrivData.end() && privateItor->second.strs.size() > 0)
						{
							aaaInfo.userID = privateItor->second.strs[0];
						}

						if(assetElements.size() > 0)
						{
							com::izq::am::facade::servicesForIce::AttributesMap::iterator itor = assetElements[0].attributes.find(PD_KEY_ContentID);
							if(itor != assetElements[0].attributes.end())
								aaaInfo.contentId = itor->second;
						}
					}
					else if(apppathinfo.aaa.aaaEntry == HENAN_AAAQUERY_Name)
					{
						std::string key = PD_KEY_purchaseToken;
						TianShanIce::ValueMap::iterator privateItor =  purPrivData.find(key);
						if(privateItor != purPrivData.end() && privateItor->second.strs.size() > 0)
						{
							aaaInfo.entitlementCode = privateItor->second.strs[0];
						}

						privateItor = purPrivData.find(ClientRequestPrefix "client");
						if (purPrivData.end() != privateItor && privateItor->second.strs.size() > 0)
						{
							aaaInfo.deviceId = privateItor->second.strs[0];
						}

						privateItor =  purPrivData.find(ClientRequestPrefix NodeGroupID);
						if(privateItor != purPrivData.end() && privateItor->second.strs.size() > 0)
						{
							aaaInfo.locality = atoi(privateItor->second.strs[0].c_str());	
						}

						privateItor =  purPrivData.find(PD_KEY_PLAYLISTID);
						if(privateItor != purPrivData.end() && privateItor->second.strs.size() > 0)
						{
							aaaInfo.playListId = privateItor->second.strs[0];
						}
						aaaInfo.stopAssetIndex = 1;
					}

					TianShanIce::Properties::const_iterator itorProp = prop.find("TEARDOWN.StopNPT");
					if(itorProp != prop.end())
					{
						aaaInfo.stopNPT = (float) atof(itorProp->second.c_str());
					}
					else
					{
						aaaInfo.stopNPT =  -1;
					}

					//aaaInfo.stopNPT = -2;
					aaaInfo.contentId = "";
					aaaInfo.commmand = "Release";
					if(assetElements.size() > 0)
					{
						com::izq::am::facade::servicesForIce::AttributesMap::iterator itor = assetElements[0].attributes.find(PD_KEY_ContentID);
						if(itor != assetElements[0].attributes.end())
							aaaInfo.contentId = itor->second;
					}
					std::string utctime;
					time_t currentTime;
					time(&currentTime);
					char buftime[65]="";
					ZQ::common::TimeUtil::Time2Iso(currentTime, buftime, 64);
					utctime = buftime;

					char localTime[65]="";
					ZQ::common::TimeUtil::Iso2Local(buftime, localTime, sizeof(localTime) -1);

					aaaInfo.timeStamp = localTime;
					aaaInfo.tearDownReason = strTeardownReason;

					TianShanIce::ValueMap::iterator itorVmap;
					itorVmap = purPrivData.find(AAAError);
					if(bIspurInternal && !purInternalMsg.empty() && strTerminateReason.size() > 6  && itorVmap != purPrivData.end())
					{
						TianShanIce::Variant var;
						var = itorVmap->second;
						int aaaErrrcode = var.ints[0];
						glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "***Authorize AAAerror:[%d]"), var.ints[0]);

						char strErrorCode[64] = "";
						sprintf(strErrorCode, "%d\0", aaaErrrcode);
						std::string strTemp = strTerminateReason;
						strTerminateReason.replace(2, strlen(strErrorCode), strErrorCode);
						glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "***Change TerminateReason[%s] to [%s]"), strTemp.c_str(), strTerminateReason.c_str());
					}

					aaaInfo.terminateReason = strTerminateReason;

					retCode = _env._MHOHelperMgr.OnStatusNotice(apppathinfo.aaa.aaaEntry.c_str(), aaaInfo, extraProps);

				}
			}
			catch(TianShanIce::ServerError&ex)
			{
				glog(ZQ::common::Log::L_ERROR,PurchaseFmt(ModPurchaseImpl,"%s)"),ex.message.c_str());
			}
			catch(...)
			{

				glog(ZQ::common::Log::L_ERROR,PurchaseFmt(ModPurchaseImpl,"authorization teardown caught unknown exception(%d)"),SYS::getLastErr());
			}

			if(retCode == ZQTianShan::Application::MOD::IAuthorization::AUTHORSUCCESS ||
			   retCode == ZQTianShan::Application::MOD::IAAA::AAAQUERYSUCCESS)
			{
				glog(ZQ::common::Log::L_INFO,PurchaseFmt(ModPurchaseImpl,"authorization teardown successful"));
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

void ModPurchaseImpl::sendStatusNotice(const ::Ice::Current& c)
{
	ServantMutex::Lock lk(*this);
	Ice::Long lStart = ZQTianShan::now();

	::TianShanIce::ValueMap::iterator privateItor =  purPrivData.find(PD_KEY_SENDNOTICE);
	if (privateItor != purPrivData.end() && !privateItor->second.bin.empty() &&  privateItor->second.bin[0] == 1)
		return;

	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "sendStatusNotice() enter"));

	// find apppath config;
	bool bResult = false;  
	Config::Holder<Urlpattern> apppathinfo;
	URLPATLIST::iterator pathitor; 
	for (pathitor = gNewCfg.urlpattern.begin(); pathitor != gNewCfg.urlpattern.end(); pathitor++)
	{
		boost::regex ApppathRegex((*pathitor).pattern);
		std::string temp = (*pathitor).pattern;
		boost::cmatch m; 
		if(!boost::regex_search(appPath.c_str(), m , ApppathRegex))
		{  
			continue;
		}
		else
		{			
			if (m[1].matched && m[2].matched) 
			{
				bResult = true;
				apppathinfo = *pathitor; 
				break;
			}
		}
	}

	if (!bResult)
	{
		glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl, "[%s] No URL pattern is matched for path[%s]"), 
			ident.name.c_str(), appPath.c_str());
		apppathinfo.Authenable = false;
	}

	bool bUseAAA = false;
	if (apppathinfo.aaa.aaaEntry.size() > 0)
		bUseAAA = true;

	if (!bUseAAA)
		return;

	try
	{	
		ZQTianShan::Application::MOD::IAAA::AAAInfo aaaInfo;
		aaaInfo.endpoint = authEndpoint;
		aaaInfo.ident = ident;
		aaaInfo.clientSessionId = clientSessionId;
		aaaInfo.serverSessionId = serverSessionId;
		aaaInfo.sessionID = serverSessionId;

		aaaInfo.stopNPT = -2;
		aaaInfo.contentId = "";
		aaaInfo.commmand = "Setup";

		std::string utctime;
		time_t currentTime;
		time(&currentTime);
		char buftime[65]="";
		ZQ::common::TimeUtil::Time2Iso(currentTime, buftime, 64);
		utctime = buftime;
		aaaInfo.timeStamp = utctime;

		PARAMMAP::iterator itorpara;
		for(itorpara = apppathinfo.aaa.aaaParams.begin(); itorpara != apppathinfo.aaa.aaaParams.end(); itorpara++)
		{
			aaaInfo.prop[itorpara->second.name] = itorpara->second.value;
		}
		if(apppathinfo.aaa.aaaEntry == LAM_AAAQUERY_Name)
		{
			std::string key = ClientRequestPrefix "EntitlementCode";
			privateItor =  purPrivData.find(key);
			if(privateItor == purPrivData.end() || privateItor->second.strs.size() < 1)
			{
				return;
			}
			aaaInfo.entitlementCode = privateItor->second.strs[0];

			key = ClientRequestPrefix "x-userID";
			privateItor =  purPrivData.find(key);
			if(privateItor == purPrivData.end() || privateItor->second.strs.size() < 1)
			{
				return;
			}
			aaaInfo.userID = privateItor->second.strs[0];

			if(assetElements.size() > 0)
			{
				com::izq::am::facade::servicesForIce::AttributesMap::iterator itor = assetElements[0].attributes.find(PD_KEY_ContentID);
				if(itor != assetElements[0].attributes.end())
					aaaInfo.contentId = itor->second;
			}
		}
		else if(apppathinfo.aaa.aaaEntry == HENAN_AAAQUERY_Name)
		{
			privateItor =  purPrivData.find(PD_KEY_purchaseToken);
			if(privateItor == purPrivData.end() || privateItor->second.strs.size() < 1)
			{
				return;
			}
			aaaInfo.entitlementCode = privateItor->second.strs[0];

			privateItor = purPrivData.find(ClientRequestPrefix "client");
			if (purPrivData.end() == privateItor || privateItor->second.strs.size() == 0)
			{
				return;
			}
			aaaInfo.deviceId = privateItor->second.strs[0];

			privateItor =  purPrivData.find(ClientRequestPrefix NodeGroupID);
			if(privateItor == purPrivData.end() || privateItor->second.strs.size() < 1)
			{
                return;
			}
			aaaInfo.locality = atoi(privateItor->second.strs[0].c_str());	

			privateItor =  purPrivData.find(PD_KEY_PLAYLISTID);
			if(privateItor == purPrivData.end() || privateItor->second.strs.size() < 1)
			{
				return;
			}
			aaaInfo.playListId = privateItor->second.strs[0];
		}

		TianShanIce::Properties extraProps;
		int retCode = _env._MHOHelperMgr.OnStatusNotice(apppathinfo.aaa.aaaEntry.c_str(), aaaInfo, extraProps);

		if(retCode != ZQTianShan::Application::MOD::IAAA::AAAQUERYSUCCESS)
		{
			glog(ZQ::common::Log::L_INFO,PurchaseFmt(ModPurchaseImpl,"failed to send Status Notice setup message"));
		}
		else
			glog(ZQ::common::Log::L_INFO,PurchaseFmt(ModPurchaseImpl,"send Status Notice setup message successful"));

		TianShanIce::Variant varSendNotice;
		varSendNotice.bRange = false;
		varSendNotice.bin.push_back(1);
		purPrivData[PD_KEY_SENDNOTICE] = varSendNotice;
	}
	catch(TianShanIce::ServerError&ex)
	{
		glog(ZQ::common::Log::L_ERROR,PurchaseFmt(ModPurchaseImpl,"send Status Notice setup message caught exception[%s])"),ex.message.c_str());
	}
	catch(...)
	{

		glog(ZQ::common::Log::L_ERROR,PurchaseFmt(ModPurchaseImpl,"send StatusNotice setup message caught unknown exception(%d)"),SYS::getLastErr());
	}
	glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "sendStatusNotice() leave took %d ms"), ZQTianShan::now() - lStart);
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
bool ModPurchaseImpl::getAEFromLAM(ZQ::common::Config::Holder<Urlpattern>& apppathinfo, ZQ::common::URLStr& urlObj, ::TianShanIce::ValueMap& privData)
{
	glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, "Enter getAEFromLAM()"));
	bool bRet = true;

	
    bool bUseAAA = false;
	if(apppathinfo.aaa.aaaEntry.size() > 0)
		bUseAAA = true;

	std::string lamendpoint = "";
	PARAMMAP::iterator itor;
	if(!bUseAAA) //using getaelist interface
	{
		itor = apppathinfo.playlistParams.find(LAMENDPOINT);
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
		lamendpoint = itor->second.value;
	}
	else //using AAA interface
	{
		itor = apppathinfo.aaa.aaaParams.find(ENDPOINT);
		if (itor == apppathinfo.aaa.aaaParams.end())
		{
			bRet = false;

			char errMsg[1024]="";
			sprintf(errMsg, "applicatoin-path[%s] has no [%s] parameter", apppathinfo.pattern.c_str(), ENDPOINT);
			::TianShanIce::Variant varError;
			varError.bRange = false;
			varError.type = TianShanIce::vtStrings;
			varError.strs.push_back(errMsg);

			MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

			purPrivData = privData;

			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_332, "[%s] applicatoin-path[%s] has no [%s] parameter", 
				ident.name.c_str(), apppathinfo.pattern.c_str(), ENDPOINT);
		}
		lamendpoint = itor->second.value;
	}
	
	if(!bUseAAA)
	{
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
	}

	glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "Get LAM endpoint is [%s]"), lamendpoint.c_str());

	const char* assetv = NULL;
	const char* assetUIDv = NULL;
	std::string provdId;
	std::string provdAssetId; 
	std::string assetUid;
	std::string deviceID;
	ZQTianShan::Application::MOD::IPlayListQuery::PlayListInfo plinfo;
	ZQTianShan::Application::MOD::IAAA::AAAInfo aaaInfo;
	com::izq::am::facade::servicesForIce::NetIDCollection netIdcollection;
	TianShanIce::StrValues volumeslist, volumesTemp;
    bool bAllHasNasUrl = false;
    
	if(!bUseAAA) //using getaelist interface
	{
		plinfo.endpoint = lamendpoint;
		plinfo.ident = ident;
		assetv = urlObj.getVar("asset");
		assetUIDv = urlObj.getVar("assetuid");

		if(assetv != NULL && strlen(assetv) > 0)
		{
			std::string tempProAssetID;
			provdId = String::getLeftStr(assetv, "#");
			provdAssetId = String::getRightStr(assetv, "#");

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
	} //using AAA interface
	else
	{
		aaaInfo.errorCode = 0;
		aaaInfo.endpoint = lamendpoint;
		aaaInfo.ident = ident;
		aaaInfo.clientSessionId = clientSessionId;
		aaaInfo.serverSessionId = serverSessionId;
		PARAMMAP::iterator itorpara;
		for(itorpara = apppathinfo.aaa.aaaParams.begin(); itorpara != apppathinfo.aaa.aaaParams.end(); itorpara++)
		{
            aaaInfo.prop[itorpara->second.name] = itorpara->second.value;
		}
		authEndpoint = lamendpoint;
        aaaInfo.sessionID = serverSessionId;

		TianShanIce::ValueMap::iterator privateItor;
		if(apppathinfo.aaa.aaaEntry == LAM_AAAQUERY_Name)
		{
			std::string key = ClientRequestPrefix "EntitlementCode";
			privateItor =  privData.find(key);
			if(privateItor == privData.end() || privateItor->second.strs.size() < 1)
			{
				bRet = false;

				char errMsg[1024]="";
				sprintf(errMsg, "no EntitlementCode in url");
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_307, "[%s] no entitlementCode found in url", ident.name.c_str());
			}
			else
				aaaInfo.entitlementCode = privateItor->second.strs[0];

			key = ClientRequestPrefix "x-userID";
			privateItor =  privData.find(key);
			if(privateItor == privData.end() || privateItor->second.strs.size() < 1)
			{

				bRet = false;

				char errMsg[1024]="";
				sprintf(errMsg, "no x-userID in url");
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_307, "[%s] no userID found in url", ident.name.c_str());
			}
			aaaInfo.userID = privateItor->second.strs[0];	

		}
		else if(apppathinfo.aaa.aaaEntry == HENAN_AAAQUERY_Name)
		{
			const char * purToken = urlObj.getVar("purchasetoken");
			if(NULL == purToken || strlen(purToken) < 1)
			{
				bRet = false;

				char errMsg[1024]="";
				sprintf(errMsg, "no purchaseToken in url");
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;

				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_307, "[%s] no purchaseToken in url", ident.name.c_str());

			}
			aaaInfo.entitlementCode = purToken;

			// add purchaseToken to privatedata
			TianShanIce::Variant varPT;
			varPT.bRange = false;
			varPT.type = TianShanIce::vtStrings;
			varPT.strs.push_back(purToken);
			MAPSET(TianShanIce::ValueMap, privData, PD_KEY_purchaseToken, varPT);

			privateItor = privData.find(ClientRequestPrefix "client");
			if (privData.end() == privateItor || privateItor->second.strs.size() == 0)
			{
				bRet =false;
				char errMsg[1024]="";
				sprintf(errMsg, "no client");
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;

				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_316, "[%s] no client", ident.name.c_str());
			}
			aaaInfo.deviceId = privateItor->second.strs[0];

			privateItor =  privData.find(ClientRequestPrefix NodeGroupID);
			if(privateItor == privData.end() || privateItor->second.strs.size() < 1)
			{
				bRet = false;

				char errMsg[1024]="";
				sprintf(errMsg, "no node-group-id in url");
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				purPrivData = privData;
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, err_307, "[%s] no node-group-id found in url", ident.name.c_str());
			}
			aaaInfo.locality = atoi(privateItor->second.strs[0].c_str());	
			aaaInfo.usage = "start";
		}
	}

	ZQTianShan::Application::MOD::AEReturnData aeRetData;
	::TianShanIce::SRM::ResourceAttribute resAttribute = TianShanIce::SRM::raMandatoryNonNegotiable;

	aeRetData.aeList.clear();
	aeRetData.netIDList.clear();

//	if(stricmp(apppathinfo.playlistModule.c_str(), "Internal") == 0)
	if(0)//永远走PlugIn
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
				if(!bUseAAA)
				{
				  retCode = _env._MHOHelperMgr.getPlayList(apppathinfo.playlistEntry.c_str(), plinfo, aeRetData);
				}
				else
				{
					retCode = _env._MHOHelperMgr.OnAuthorize(apppathinfo.aaa.aaaEntry.c_str(), aaaInfo, aeRetData, privData);
				}
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

				if(aaaInfo.errorCode  >= 1000)
				{
					::TianShanIce::Variant varErrorCode;
					varErrorCode.bRange = false;
					varErrorCode.type = TianShanIce::vtInts;
					varErrorCode.ints.push_back(aaaInfo.errorCode);
					MAPSET(TianShanIce::ValueMap, privData, AAAError, varErrorCode);
				}
				purPrivData = privData;

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

				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, retCode, "[%s] get play list caught unknown exception(%d)", 
					ident.name.c_str(), SYS::getLastErr());
			}

			pErrDesc = _env._MHOHelperMgr.getErrorMsg(retCode);
			if(retCode != ZQTianShan::Application::MOD::IPlayListQuery::PLQUERYSUCCESS && 
				retCode !=  ZQTianShan::Application::MOD::IAAA::AAAQUERYSUCCESS)
			{
				bRet = false;

				char errMsg[1024]="";
				sprintf(errMsg, "get play list caught exception(%s)", pErrDesc);
				::TianShanIce::Variant varError;
				varError.bRange = false;
				varError.type = TianShanIce::vtStrings;
				varError.strs.push_back(errMsg);
				MAPSET(TianShanIce::ValueMap, privData, GetAELastError, varError);

				if(aaaInfo.errorCode  >= 1000)
				{
					::TianShanIce::Variant varErrorCode;
					varErrorCode.bRange = false;
					varErrorCode.type = TianShanIce::vtInts;
					varErrorCode.ints.push_back(aaaInfo.errorCode);
					MAPSET(TianShanIce::ValueMap, privData, AAAError, varErrorCode);
				}
				purPrivData = privData;

				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog,LOG_PURCHASE, retCode, "[%s]get play list caught exception(%s)", 
					ident.name.c_str(), pErrDesc);
			}
		}
		else
		{
			TestElementMod(aeRetData, privData);
		}
	
	  glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl,"get play list successfully from [%s]"), lamendpoint.c_str());

	  // add ADSReplacement logic

	  std::string adsEndpoint = "";
	  try
	  {
		  int retCode =-1;
		  if(apppathinfo.adsReplacement.adsEntry!= "" && apppathinfo.adsReplacement.adsParams.size() > 0 )
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
			  
			  if(retCode != ZQTianShan::Application::MOD::IPlayListQuery::PLQUERYSUCCESS)
			  {
					  const char* pErrDesc = _env._MHOHelperMgr.getErrorMsg(retCode);
				  glog(ZQ::common::Log::L_ERROR, PurchaseFmt(ModPurchaseImpl,"[%s]get ADS replacement list caught exception(%s)"), 
					  ident.name.c_str(), pErrDesc);
			  }
			  else
			  {
				  glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl,"get ADS replacement list successfully from [%s]"), adsEndpoint.c_str());
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
	  // end ADSReplacement
	  if(apppathinfo.playlistEntry == LAM_PlayList3_Name)
	  {	
		  //如果Asset的NasUrl不为空，则往每个Asset的 volumelist 中插入一个 临时的 REDUNDANCY_NETIDVOLUME
		  //如果取完交集后，REDUNDANCY_NETIDVOLUME这个存在， 说明所有Asset的NasUrl不为空
		 // NarURL 存在：    1> volumeList 为空，  raMandatoryNonNegotiable （部分Asset有NasUrl）
		 //	                 2> volumeList 不为空，raNonMandatoryNonNegotiable（VoumleList有可能为空）
		 // NarURL 不存在：  1> volumeList 为空，  raNonMandatoryNegotiable
		 //	                 2> volumeList 不为空，raMandatoryNonNegotiable
          if(!gNewCfg.mandatory) //for gehua dizhonghai
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
		if(gNewCfg.mandatory)
			resAttribute = TianShanIce::SRM::raMandatoryNonNegotiable;

		if (bAllHasNasUrl || gNewCfg.mandatory)
		{
			TianShanIce::Variant vstoragelink;
			vstoragelink.type = TianShanIce::vtStrings;
			vstoragelink.bRange = false;
			vstoragelink.strs.clear();
			vstoragelink.strs.push_back(gNewCfg.allowedstoragelink);
			resResult.resourceData.insert(::TianShanIce::ValueMap::value_type("LinkType", vstoragelink));
			glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "add LinkType [%s] to resource of rtStorage [resAttribute=%d]"), gNewCfg.allowedstoragelink.c_str(), resAttribute);
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
				"add resource to weiwoo caught ice exception(%s)", ex.ice_name().c_str());
		}
	}//end if (netIdcollection.size() > 0)
	else
	{
		if(gNewCfg.mandatory)
		{
			resAttribute = TianShanIce::SRM::raMandatoryNonNegotiable;
			TianShanIce::Variant vstoragelink;
			vstoragelink.type = TianShanIce::vtStrings;
			vstoragelink.bRange = false;
			vstoragelink.strs.clear();
			vstoragelink.strs.push_back(gNewCfg.allowedstoragelink);
			resResult.resourceData.insert(::TianShanIce::ValueMap::value_type("LinkType", vstoragelink));
			glog(ZQ::common::Log::L_INFO, PurchaseFmt(ModPurchaseImpl, "add LinkType [%s] to resource of rtStorage [resAttribute=%d]"), gNewCfg.allowedstoragelink.c_str(), resAttribute);

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
					"add resource to weiwoo caught ice exception(%s)", ex.ice_name().c_str());
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
	if(0)//永远走PlugIn
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

				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, retCode, "[%s]get play list (%s)", 
					ident.name.c_str(), ex.message.c_str() );
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

				ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, retCode, "[%s]get play list caught unknown exception(%d)", 
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

				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(glog, LOG_PURCHASE, retCode, "[%s]get play list caught exception(%s)", 
					ident.name.c_str(), pErrDesc);
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
					"add resource to weiwoo caught ice exception(%s)", ex.ice_name().c_str());
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
			boost::cmatch result;

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
	if(0)//永远走PlugIn
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

			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_PURCHASE, retCode, "[%s]Authorization (%s)",
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
bool ModPurchaseImpl::getAesstInfo(ZQ::common::URLStr& urlObj, ::TianShanIce::ValueMap& privData)
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

		// save information here in order to authorize futher
		TianShanIce::Variant vProvdId, vProvdAssetId;
		vProvdId.type = TianShanIce::vtStrings;
		vProvdId.strs.push_back(provdId);
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

		glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, 
			"TestMod asset info  [assetname=%s, bandwidth = %d, cueIn=%d, cueOut=%d]"),
			aeInfo.aeUID.c_str(), aeInfo.bandWidth, aeInfo.cueIn, aeInfo.cueOut);
		ZQTianShan::Application::MOD::AttributesMap::iterator itor;
		for(itor = aeInfo.attributes.begin();  itor != aeInfo.attributes.end(); itor++)
		{
			glog(ZQ::common::Log::L_DEBUG, PurchaseFmt(ModPurchaseImpl, 
				"TestMod asset info  [assetname=%s, %s = %s]"),
				aeInfo.aeUID.c_str(), itor->first.c_str(), itor->second.c_str());
		}
	}	
	aeRetData.netIDList.clear();
}

TianShanIce::Application::PlaylistInfo ModPurchaseImpl::getPlaylistInfo(const ::Ice::Current& c) const
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
} // namespace ZQMODApplication
