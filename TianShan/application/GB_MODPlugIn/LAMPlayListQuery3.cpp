// LAMGetPlayList.cpp: implementation of the LAMGetPlayList class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "LAMPlayListQuery3.h"
#include "LAMFacade.h"
#include "Surf_Tianshan.h"
#include "SystemUtils.h"
#include "Log.h"

extern ZQ::common::Log * PMHOlog;
#define MLOG (*PMHOlog)

namespace ZQTianShan {
namespace Application{
namespace MOD{
#define LAMPLAYLISTQUERY3 "LAMPlayListQuery3"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LAMPlayListQuery3::LAMPlayListQuery3(::Ice::CommunicatorPtr& _ic)
								: _iceComm(_ic)
{

}

LAMPlayListQuery3::~LAMPlayListQuery3()
{

}

int LAMPlayListQuery3::getPlayList(PlayListInfo& plinfo, AEReturnData& aedata)
{

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery3 ,"[%s] Entry getPlayList() entry='%s'"),
		plinfo.ident.name.c_str(),LAM_PlayList3_Name);

	int retCode;	
	switch(plinfo.nType)
	{
	case getaeList:
	case getaeListWithAppUID:
	case getaeListByPIDandAID:
		return GetPlayListFromILAMFacade(plinfo, aedata);
	case getAeListforSurf:
//		return GetPlayListFromISurfForTianshan(plinfo, aedata);
		MLOG(ZQ::common::Log::L_ERROR, 
			CLOGFMT(LAMPlayListQuery3, "[%s] this entry doesn't support get AElist from surffortianshan interface, use entry 'GetPlayListFromLAM'"),
			plinfo.ident.name.c_str());
		retCode = NOTSUPPORT;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY3, retCode, "this entry doesn't support get AElist from surffortianshan interface, use entry 'GetPlayListFromLAM'");
		return retCode;
	default:
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(LAMPlayListQuery3, "[%s] unknown get aelist type='%d'"),
			plinfo.ident.name.c_str(),plinfo.nType);
		retCode = UNKNOWNTYPE;
		break;
	}
    MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery3, "[%s] Leave getPlayList()"),plinfo.ident.name.c_str());

	ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY3, retCode, "unknown get AE List Type [%d]", plinfo.nType);	

	return retCode;
}
int LAMPlayListQuery3::GetPlayListFromILAMFacade(PlayListInfo& plinfo, AEReturnData& aedata)
{
	MLOG(ZQ::common::Log::L_DEBUG, 
		CLOGFMT(LAMPlayListQuery3, "[%s] Entry GetPlayListFromILAMFacade() LAMFacade endpoint at [%s]"),
		plinfo.ident.name.c_str(), plinfo.endpoint.c_str());

	Ice::Long lstart  = ZQTianShan::now();
	int retCode = PLQUERYSUCCESS;

// 	if(plinfo.nType == getaeList)
// 	{
// 		MLOG(ZQ::common::Log::L_DEBUG, 
// 			CLOGFMT(LAMPlayListQuery3, "[%s] not support get AElist By assetUID[%s][took %d ms]"),
// 			plinfo.ident.name.c_str(), plinfo.UID1.c_str(), ZQTianShan::now() - lstart);
// 		retCode = NOTSUPPORT;
// 
// 		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY3, retCode, "not support get AElist By assetUID[%s]",
// 			plinfo.UID1.c_str());	
// 		return retCode;
// 	}

	// do: get lam interface proxy
	com::izq::am::facade::servicesForIce::LAMFacadePrx lamPrx = NULL;
	com::izq::am::facade::servicesForIce::AEInfo3Collection aeinfo3collection;
	try
	{
		lamPrx = com::izq::am::facade::servicesForIce::LAMFacadePrx::checkedCast(_iceComm->stringToProxy(plinfo.endpoint));
	}
	catch (const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, 
			CLOGFMT(LAMPlayListQuery3, "[%s] fail to get LAMFacade proxy caught [%s] at endpoint[%s][took %d ms]"),
			plinfo.ident.name.c_str(), ex.ice_name().c_str(), plinfo.endpoint.c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY3, retCode, "fail to get LAMFacade proxy caught [%s]",ex.ice_name().c_str());	
		return retCode;	
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, 
			CLOGFMT(LAMPlayListQuery3, "[%s] fail to get LAMFacade proxy caught unknown exception(%d)[took %d ms]"),
			plinfo.ident.name.c_str(),SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY3, retCode, "fail to get LAMFacade proxy caught unknown exception(%d)",
			SYS::getLastErr());	
		return retCode;	
	}

	if(!lamPrx)
	{
		MLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT(LAMPlayListQuery3, "[%s]Invaild LAMFacade proxy[took %d ms]"),
			plinfo.ident.name.c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY3, retCode, "Invaild LAMFacade proxy");	
		return retCode;
	}

	aedata.aeList.clear();
	aedata.netIDList.clear();
	aedata.useNasURL =0;

	try
	{
		switch(plinfo.nType)
		{
		case getaeList:
//		case getaeListWithAppUID:	
			MLOG(ZQ::common::Log::L_INFO, 
				CLOGFMT(LAMPlayListQuery3, "[%s] assetUID=%s, appUID=%s"),
				plinfo.ident.name.c_str(),  plinfo.UID1.c_str(),  plinfo.UID2.c_str());
			aeinfo3collection = lamPrx->getAEListByAUIdAppUId(plinfo.UID1, plinfo.UID2);
			break;
		case getaeListByPIDandAID:	
			MLOG(ZQ::common::Log::L_INFO, 
				CLOGFMT(LAMPlayListQuery3, "[%s] provider-id=%s,provider-asset-id=%s,device-id=%s"),
				plinfo.ident.name.c_str(),  plinfo.UID1.c_str(),  plinfo.UID2.c_str(), plinfo.UID3.c_str());
			aeinfo3collection = lamPrx->getAEListByPIdPAIdSId(plinfo.UID1, plinfo.UID2, plinfo.UID3);
			break;
		default:
			break;
		}
	}
	catch (const com::izq::am::facade::servicesForIce::LogicError& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT(LAMPlayListQuery3, "[%s] fail to get AE list caught %s, %d, %s[took %d ms]"), 
			plinfo.ident.name.c_str(), ex.ice_name().c_str(), ex.errorCode, ex.errorMessage.c_str(), ZQTianShan::now() - lstart);
		retCode = PLNOTEXIST;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY3, retCode, "fail to get AE list caught %s, %d, %s", ex.ice_name().c_str(), ex.errorCode, ex.errorMessage.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT(LAMPlayListQuery3, "[%s]fail to get AE list caught '%s'[took %d ms]"), 
			plinfo.ident.name.c_str(), ex.ice_name().c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY3, retCode, "fail to get AE list caught '%s'",
			ex.ice_name().c_str());
		return retCode;
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, 
			CLOGFMT(LAMPlayListQuery3, "[%s] fail to get AE list caught unknown exception(%d)[took %d ms]"),
			plinfo.ident.name.c_str(), SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY3, retCode, "fail to get AE list caught unknown exception(%d)",
			SYS::getLastErr());	
		return retCode;	
	}

	if(aeinfo3collection.size() < 1)
	{
		MLOG(ZQ::common::Log::L_DEBUG, 
			CLOGFMT(LAMPlayListQuery3, "[%s] Asset element is null[took %d ms]"),
			plinfo.ident.name.c_str(), ZQTianShan::now() - lstart);
		retCode = PLNOTEXIST;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY3, retCode, "Asset element is null");
		return retCode;
	}

	unsigned int i = 0;
	for(i = 0; i < aeinfo3collection.size(); i++)
	{
		com::izq::am::facade::servicesForIce::AEInfo3& assetinfo = aeinfo3collection[i];
		AEInfo aeInfo;
		aeInfo.aeUID = assetinfo.name;				
		aeInfo.bandWidth = assetinfo.bandWidth;
		aeInfo.cueIn = assetinfo.cueIn;
		aeInfo.cueOut = assetinfo.cueOut;
		//				aeInfo.name = assetinfo.name;
		aeInfo.nasUrls = assetinfo.nasUrls;
		aeInfo.volumeList = assetinfo.volumeList;
		aeInfo.attributes = assetinfo.attributes;
		aedata.aeList.push_back(aeInfo);
	}

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(LAMPlayListQuery3, "[%s] get playlist from LAMFacade mode successfully[took %d ms]"),
		plinfo.ident.name.c_str(), ZQTianShan::now() - lstart);

	return retCode;
}
/*
int LAMPlayListQuery3::GetPlayListFromISurfForTianshan(PlayListInfo& plinfo, AEReturnData& aedata)
{
	MLOG(ZQ::common::Log::L_INFO, 
		CLOGFMT(LAMPlayListQuery3, "[%s] Entry GetPlayListFromISurfForTianshan() surffortianshan endpoint at [%s]"),
		plinfo.ident.name.c_str(), plinfo.endpoint.c_str());

	int retCode = PLQUERYSUCCESS;
	unsigned int i =0;
	try
	{
		::com::izq::surf::integration::tianshan::SurfForTianshanPrx SurfPrx = NULL;
		try
		{
			SurfPrx = com::izq::surf::integration::tianshan::SurfForTianshanPrx::checkedCast(_iceComm->stringToProxy(plinfo.endpoint));
			if(!SurfPrx)
			{
				MLOG(ZQ::common::Log::L_ERROR,
					CLOGFMT(LAMPlayListQuery3, "[%s] SurfForTianshan Invaild proxy at endpoint[%s]"),
					plinfo.ident.name.c_str(), plinfo.endpoint.c_str());
				MLOG(ZQ::common::Log::L_INFO,
					CLOGFMT(LAMPlayListQuery3, "[%s] Leave GetPlayListFromISurfForTianshan()"), plinfo.ident.name.c_str());	

				retCode = ICEEXCEPTION;
				return retCode;
			}
		}
		catch (const Ice::ObjectNotExistException&)
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(LAMPlayListQuery3, "[%s] SurfForTianshan proxy not exists at endpoint[%s]"),
				plinfo.ident.name.c_str(), plinfo.endpoint.c_str());
			MLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(LAMPlayListQuery3, "[%s] Leave GetPlayListFromISurfForTianshan()"), plinfo.ident.name.c_str());	

			retCode = ICEEXCEPTION;
			return retCode;
		}
		catch (const Ice::Exception& ex)
		{
			MLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(LAMPlayListQuery3, "[%s] get SurfForTianshan proxy caught [%s] at endpoint[%s]"),
				plinfo.ident.name.c_str(), ex.ice_name().c_str(), plinfo.endpoint.c_str());

			MLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(LAMPlayListQuery3, "[%s] Leave GetPlayListFromISurfForTianshan()"), plinfo.ident.name.c_str());

			retCode = ICEEXCEPTION;
			return retCode;

		}	

		::com::izq::surf::integration::tianshan::AEReturnData SurfAEData;
		SurfAEData.aeList.clear();
		SurfAEData.netIDList.clear();
		aedata.aeList.clear();
		aedata.netIDList.clear();
		try
		{
			SurfAEData  = SurfPrx->getAEList(plinfo.UID1);
		}
		catch (const com::izq::surf::integration::tianshan::SurfException& surfex) 
		{
			MLOG(ZQ::common::Log::L_ERROR, 
				CLOGFMT(LAMPlayListQuery3, "[%s] getAEList() caught %s at endpoint = %s"),
				plinfo.ident.name.c_str(), surfex.errorDescription.c_str(), plinfo.endpoint.c_str());

			MLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(LAMPlayListQuery3, "[%s] Leave GetPlayListFromISurfForTianshan()"), plinfo.ident.name.c_str());

			retCode = ICEEXCEPTION;
			return retCode;
		}
		catch (const Ice::Exception& ex)
		{
			MLOG(ZQ::common::Log::L_ERROR, 
				CLOGFMT(LAMPlayListQuery3, "[%s] getAEList() caught %s at endpoint = %s"),
				plinfo.ident.name.c_str(), ex.ice_name().c_str(), plinfo.endpoint.c_str());

			MLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(LAMPlayListQuery3, "[%s] Leave GetPlayListFromISurfForTianshan()"), plinfo.ident.name.c_str());

			retCode = ICEEXCEPTION;
			return retCode;
		}
        
		if(SurfAEData.aeList.size() < 1)
		{
			MLOG(ZQ::common::Log::L_ERROR, 
				CLOGFMT(LAMPlayListQuery3, "[%s] no element"),
				plinfo.ident.name.c_str());
			
			MLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(LAMPlayListQuery3, "Leave GetPlayListFromISurfForTianshan()"));
			retCode = PLNOTEXIST;
			return retCode;
		}
		for(i = 0; i < SurfAEData.aeList.size(); i++)
		{
			::com::izq::surf::integration::tianshan::AEInfo& surfaeInfo = SurfAEData.aeList[i];
			AEInfo aeInfo;
			aeInfo.aeUID = surfaeInfo.aeUID;
			aeInfo.bandWidth = surfaeInfo.bandWidth;
			aeInfo.cueIn = surfaeInfo.cueIn;
			aeInfo.cueOut = surfaeInfo.cueOut;
			aedata.aeList.push_back(aeInfo);
		}

//		for(i = 0; i < SurfAEData.netIDList.size(); i++)
//		{
//			aedata.netIDList.push_back(SurfAEData.netIDList[i]);
//		}	
	
		aedata.netIDList = SurfAEData.netIDList;
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_INFO,
			CLOGFMT(LAMPlayListQuery3, "[%s] caught unknown exception(%d)"), 
			plinfo.ident.name.c_str(), SYS::getLastErr());
		retCode = INTERNAL;
	}

	MLOG(ZQ::common::Log::L_INFO,
		CLOGFMT(LAMPlayListQuery3, "[%s] Leave GetPlayListFromISurfForTianshan()"), plinfo.ident.name.c_str()); 

	return retCode;
}*/

}}}//end namespace
