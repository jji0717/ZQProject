// ADMPlacement.cpp: implementation of the LAMGetPlayList class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ADMPlacement.h"
#include "ADM.h"
#include "../StreamSmith/RtspRelevant.h"
#include "SiteDefines.h"
#include "SystemUtils.h"
#include "Log.h"

#define ADMFmt(_C, _X) CLOGFMT(_C, "[%s][%s] " _X), adsinfo.serverSessionId.c_str(), adsinfo.ident.name.c_str()

extern ZQ::common::Log * PMHOlog;
#define MLOG (*PMHOlog)

namespace ZQTianShan {
	namespace Application{
		namespace MOD{
#define ADMPLACEMET "ADMPlacement"
			//////////////////////////////////////////////////////////////////////
			// Construction/Destruction
			//////////////////////////////////////////////////////////////////////

			ADMPlacement::ADMPlacement(::Ice::CommunicatorPtr& _ic)
				: _iceComm(_ic)
			{

			}

			ADMPlacement::~ADMPlacement()
			{

			}

			int ADMPlacement::getAdsReplacement(AdsInfo& adsinfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData)
			{
				MLOG(ZQ::common::Log::L_DEBUG, ADMFmt(ADMPlacement, "Entry getAdsReplacement() ADM endpoint at [%s]"),
					adsinfo.endpoint.c_str());

				Ice::Long lstart  = ZQTianShan::now();

				int retCode = ADSQUERYSUCCESS;

				// do: get lam interface proxy
				com::izq::ads::tianshan::ADSForTianshanPrx adsPrx = NULL;
				com::izq::ads::tianshan::AEInfo3Collection aeinfo3collection;
				try
				{
					adsPrx = com::izq::ads::tianshan::ADSForTianshanPrx::checkedCast(_iceComm->stringToProxy(adsinfo.endpoint));
				}
				catch (const Ice::Exception& ex)
				{
					MLOG(ZQ::common::Log::L_DEBUG, ADMFmt(ADMPlacement, "connect to TVBS[%s] caught [%s], took %d ms"),
						adsinfo.endpoint.c_str(), ex.ice_name().c_str(), ZQTianShan::now() - lstart);
					retCode = ICEEXCEPTION;
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, ADMPLACEMET, ICEEXCEPTION, "connect to TVBS caught [%s]", ex.ice_name().c_str());	
					return retCode;	
				}
				catch (...)
				{
					MLOG(ZQ::common::Log::L_DEBUG, ADMFmt(ADMPlacement, "connect to TVBS caught err(%d), took %dms"),
						SYS::getLastErr(), ZQTianShan::now() - lstart);
					retCode = INTERNAL;
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, ADMPLACEMET, INTERNAL, "connect to TVBS caught err(%d)",
						SYS::getLastErr());	
					return retCode;	
				}
				if(!adsPrx)
				{
					MLOG(ZQ::common::Log::L_DEBUG, ADMFmt(ADMPlacement, "TVBS not connected[took %d ms]"), ZQTianShan::now() - lstart);
					retCode = ICEEXCEPTION;
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, ADMPLACEMET, ICEEXCEPTION, "TVBS not connected");	
					return retCode;
				}

				Ice::Long lTimeGetProxy  = ZQTianShan::now();

				::com::izq::ads::tianshan::SessionData sd;
				sd.clientSessionId = adsinfo.clientSessionId;
				sd.serverSessionId = adsinfo.serverSessionId;


				for (::TianShanIce::ValueMap::iterator mItor = privData.begin(); mItor != privData.end(); mItor ++)
				{
					if (((::TianShanIce::Variant)(mItor->second)).strs.size() == 0)
						continue;
					std::string keyStr = mItor->first;

					/*if (stricmp(String::nLeftStr(keyStr, strlen(ClientRequestPrefix)).c_str(), ClientRequestPrefix) == 0)
					{// if key has prefix of "ClientRequest#", omit it.
					keyStr = String::getRightStr(keyStr, "#", true);
					}*/
					int nlen = strlen(ClientRequestPrefix);
					std::string strPrx = keyStr.substr(0, nlen);
					if(stricmp(strPrx.c_str(), ClientRequestPrefix) == 0)
					{
						keyStr = keyStr.substr(nlen,keyStr.size() - nlen);				
					}
					// use insert can avoid overwritting the existing key-value
					sd.params.insert(::com::izq::ads::tianshan::Properties::value_type(keyStr, ((::TianShanIce::Variant)(mItor->second)).strs[0]));
				}
				// dump the values
				std::string strAdsParamerter = "";
				for (::com::izq::ads::tianshan::Properties::iterator pItor = sd.params.begin(); pItor != sd.params.end(); pItor ++)
				{
					char temp[512] = "";
					sprintf(temp, "%s[%s],", pItor->first.c_str(), pItor->second.c_str());
					strAdsParamerter += temp;
				}

				for(unsigned int i = 0; i < aedata.aeList.size(); i++)
				{
					::com::izq::ads::tianshan::AEInfo3 aeInfo;
					aeInfo.name = aedata.aeList[i].aeUID;
					aeInfo.bandWidth = aedata.aeList[i].bandWidth;
					aeInfo.cueIn = aedata.aeList[i].cueIn;
					aeInfo.cueOut = aedata.aeList[i].cueOut;
					aeInfo.volumeList= aedata.aeList[i].volumeList;
					aeInfo.nasUrls = aedata.aeList[i].nasUrls;
					aeInfo.attributes = aedata.aeList[i].attributes;
					aeinfo3collection.push_back(aeInfo);
				}
				
				try
				{
					MLOG(ZQ::common::Log::L_DEBUG, ADMFmt(ADMPlacement, "calling TVBS::PlacementReqeust() clientSessionId[%s] AdsParamerter: %s"),
						  adsinfo.clientSessionId.c_str(), strAdsParamerter.c_str());
					aeinfo3collection = adsPrx->PlacementReqeust(sd, aeinfo3collection);

					MLOG(ZQ::common::Log::L_INFO, ADMFmt(ADMPlacement, "get ads proxy took %d ms and calling TVBS::PlacementReqeust() successfully took %dms"), 
						(int)(lTimeGetProxy - lstart), (int)(ZQTianShan::now() - lTimeGetProxy)); 
				}
				catch (const Ice::Exception& ex)
				{
					MLOG(ZQ::common::Log::L_DEBUG, ADMFmt(ADMPlacement, "call TVBS::PlacementReqeust() caught %s, took %dms"), 
						 ex.ice_name().c_str(), ZQTianShan::now() - lstart);
					retCode = ICEEXCEPTION;
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, ADMPLACEMET, ICEEXCEPTION, "call TVBS::PlacementReqeust() caught %s",
						ex.ice_name().c_str());
					return retCode;
				}
				catch (...)
				{
					MLOG(ZQ::common::Log::L_DEBUG, ADMFmt(ADMPlacement, "call TVBS::PlacementReqeust() caught err(%d), took %dms]"),
						SYS::getLastErr(), ZQTianShan::now() - lstart);
					retCode = INTERNAL;
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, ADMPLACEMET, INTERNAL, "call TVBS::PlacementReqeust() caught err(%d)",
						SYS::getLastErr());	
					return retCode;	
				}

				if(aeinfo3collection.size() < 1)
				{
					MLOG(ZQ::common::Log::L_DEBUG, ADMFmt(ADMPlacement, "NULL list received from TVBS::PlacementReqeust(), took %dms"),
						 ZQTianShan::now() - lstart);
					retCode = PLNOTEXIST;
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, ADMPLACEMET, PLNOTEXIST, "NULL list received from TVBS::PlacementReqeust()");	

					return retCode;
				}

				aedata.aeList.clear();
				aedata.netIDList.clear();
				aedata.useNasURL = 0;
				unsigned int i = 0;
				for(i = 0; i < aeinfo3collection.size(); i++)
				{
					com::izq::ads::tianshan::AEInfo3& assetinfo = aeinfo3collection[i];
					AEInfo aeInfo;
					aeInfo.aeUID = assetinfo.name;				
					aeInfo.bandWidth = assetinfo.bandWidth;
					aeInfo.cueIn = assetinfo.cueIn;
					aeInfo.cueOut = assetinfo.cueOut;
					//aeInfo.name = assetinfo.name;
					aeInfo.nasUrls = assetinfo.nasUrls;
					aeInfo.volumeList = assetinfo.volumeList;
					aeInfo.attributes = assetinfo.attributes;
					aedata.aeList.push_back(aeInfo);
				}

				MLOG(ZQ::common::Log::L_DEBUG, ADMFmt(ADMPlacement, "Leave getAdsReplacement()"));
				return retCode;
			}

}
}
}//end namespace
