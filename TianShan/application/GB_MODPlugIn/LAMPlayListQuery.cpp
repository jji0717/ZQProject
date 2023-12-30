// LAMGetPlayList.cpp: implementation of the LAMGetPlayList class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "LAMPlayListQuery.h"
#include "LAMFacade.h"
#include "Surf_Tianshan.h"
#include "SystemUtils.h"
#include "Log.h"

extern ZQ::common::Log * PMHOlog;
#define MLOG (*PMHOlog)

namespace ZQTianShan {
namespace Application{
namespace MOD{
#define LAMPLAYLISTQUERY "LAMPlayListQuery"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LAMPlayListQuery::LAMPlayListQuery(::Ice::CommunicatorPtr& _ic)
								: _iceComm(_ic)
{

}

LAMPlayListQuery::~LAMPlayListQuery()
{

}

int LAMPlayListQuery::getPlayList(PlayListInfo& plinfo, AEReturnData& aedata)
{

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery ,"[%s] Entry getPlayList() entry='%s'"),
		plinfo.ident.name.c_str(),LAM_PlayList_Name);

	int retCode;	
	switch(plinfo.nType)
	{
	case getaeList:
	case getaeListWithAppUID:
	case getaeListByPIDandAID:
		return GetPlayListFromILAMFacade(plinfo, aedata);
	case getAeListforSurf:
		return GetPlayListFromISurfForTianshan(plinfo, aedata);
	default:
		retCode = UNKNOWNTYPE;
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(LAMPlayListQuery, "[%s] unknown get aelist type='%d'"),
			plinfo.ident.name.c_str(),plinfo.nType);
		break;
	}

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s] Leave getPlayList()"),plinfo.ident.name.c_str());

	ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, UNKNOWNTYPE, "unknown get AE ListType[%d]", plinfo.nType);	
	return retCode;
}
int LAMPlayListQuery::GetPlayListFromILAMFacade(PlayListInfo& plinfo, AEReturnData& aedata)
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s] Entry GetPlayListFromILAMFacade() LAMFacade endpoint at [%s]"),
		plinfo.ident.name.c_str(), plinfo.endpoint.c_str());

	Ice::Long lstart  = ZQTianShan::now();

	int retCode = PLQUERYSUCCESS;

	// do: get lam interface proxy
	com::izq::am::facade::servicesForIce::LAMFacadePrx lamPrx = NULL;
	com::izq::am::facade::servicesForIce::AECollectionWithNetID assetElementswithNetID;
	try
	{
		lamPrx = com::izq::am::facade::servicesForIce::LAMFacadePrx::checkedCast(_iceComm->stringToProxy(plinfo.endpoint));
	}
	catch (const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s] fail to get LAMFacade proxy caught [%s] at endpoint[%s][took %d ms]"),
			plinfo.ident.name.c_str(), ex.ice_name().c_str(), plinfo.endpoint.c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, ICEEXCEPTION, "fail to get LAMFacade proxy caught [%s]",ex.ice_name().c_str());	
		return retCode;	
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s] fail to get LAMFacade proxy caught unknown exception(%d)[took %d ms]"),
			plinfo.ident.name.c_str(), SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, INTERNAL, "fail to get LAMFacade proxy caught  unknown exception(%d)",
			SYS::getLastErr());	
		return retCode;	
	}


	if(!lamPrx)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s]Invaild LAMFacade proxy[took %d ms]"),
			plinfo.ident.name.c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, ICEEXCEPTION, "invaild LAMFacade proxy");	
		return retCode;
	}

	aedata.aeList.clear();
	aedata.netIDList.clear();
	aedata.useNasURL = 0;
	try
	{
		switch(plinfo.nType)
		{
		case getaeList:
			MLOG(ZQ::common::Log::L_INFO, CLOGFMT(LAMPlayListQuery, "[%s] assetUID=%s"),
				plinfo.ident.name.c_str(),  plinfo.UID1.c_str());
			assetElementswithNetID = lamPrx->getAEListWithNetID(plinfo.UID1);
			break;
		case getaeListWithAppUID:
			assetElementswithNetID = lamPrx->getAEListWithNetIDByAppUID(plinfo.UID1, plinfo.UID2);
			break;
		case getaeListByPIDandAID:	
			MLOG(ZQ::common::Log::L_INFO, CLOGFMT(LAMPlayListQuery, "[%s] provider-id=%s,provider-asset-id=%s"),
				plinfo.ident.name.c_str(),  plinfo.UID1.c_str(),  plinfo.UID2.c_str());
			assetElementswithNetID = lamPrx->getAEListWithNetIDByPID(plinfo.UID1, plinfo.UID2);
			break;
		default:
			break;
		}
	}
	catch (const com::izq::am::facade::servicesForIce::LogicError& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT(LAMPlayListQuery, "[%s] fail to get AE list caught %s, %d, %s[took %d ms]"), 
			plinfo.ident.name.c_str(), ex.ice_name().c_str(), ex.errorCode, ex.errorMessage.c_str(), ZQTianShan::now() - lstart);
		retCode = PLNOTEXIST;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, retCode, "fail to get AE list caught %s, %d, %s", ex.ice_name().c_str(), ex.errorCode, ex.errorMessage.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s]fail to get AE list caught '%s'[took %d ms]"), 
			plinfo.ident.name.c_str(), ex.ice_name().c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, ICEEXCEPTION, "fail to get AE list caught '%s'",
			ex.ice_name().c_str());
		return retCode;
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s] fail to get AE list caught unknown exception(%d)[took %d ms]"),
			plinfo.ident.name.c_str(), SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, INTERNAL, "fail to get AE list caught unknown exception(%d)",
			SYS::getLastErr());	
		return retCode;	
	}

	if(assetElementswithNetID.aeList.size() < 1)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s] Asset element is null[took %d ms]"),
			plinfo.ident.name.c_str(), ZQTianShan::now() - lstart);
		retCode = PLNOTEXIST;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, PLNOTEXIST, "Asset element is null");	

		return retCode;
	}

	unsigned int i = 0;
	for(i = 0; i < assetElementswithNetID.aeList.size(); i++)
	{
		com::izq::am::facade::servicesForIce::AEInfo& assetinfo = assetElementswithNetID.aeList[i];
		AEInfo aeInfo;
		aeInfo.aeUID = assetinfo.aeUID;			
		aeInfo.bandWidth = assetinfo.bandWidth;
		aeInfo.cueIn = assetinfo.cueIn;
		aeInfo.cueOut = assetinfo.cueOut;
		//				aeInfo.name = "";
		aeInfo.nasUrls.clear();
		aeInfo.volumeList.clear();
		aeInfo.attributes.clear();
		aedata.aeList.push_back(aeInfo);
	}
	aedata.netIDList = assetElementswithNetID.netIDList;

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(LAMPlayListQuery, "[%s] get playlist from LAMFacade mode successfully [took %d ms]"), plinfo.ident.name.c_str(), ZQTianShan::now() - lstart); 

    return retCode;
}
int LAMPlayListQuery::GetPlayListFromISurfForTianshan(PlayListInfo& plinfo, AEReturnData& aedata)
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s] Entry GetPlayListFromISurfForTianshan() surffortianshan endpoint at [%s]"), plinfo.ident.name.c_str(), plinfo.endpoint.c_str());

	Ice::Long lstart  = ZQTianShan::now();
	int retCode = PLQUERYSUCCESS;
	unsigned int i =0;

	::com::izq::surf::integration::tianshan::SurfForTianshanPrx SurfPrx = NULL;
	try
	{
		SurfPrx = com::izq::surf::integration::tianshan::SurfForTianshanPrx::checkedCast(_iceComm->stringToProxy(plinfo.endpoint));
	}
	catch (const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s] fail to get SurfForTianshan proxy caught [%s] at endpoint[%s][took %d ms]"),
			plinfo.ident.name.c_str(), ex.ice_name().c_str(), plinfo.endpoint.c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, ICEEXCEPTION, "fail to get SurfForTianshan proxy caught [%s]",ex.ice_name().c_str());	
		return retCode;	
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s] fail to get SurfForTianshan proxy caught unknown exception(%d)[took %d ms]"),
			plinfo.ident.name.c_str(),SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, INTERNAL, "fail to get SurfForTianshan proxy caught unknown exception(%d)",
			SYS::getLastErr());	
		return retCode;	
	}

	if(!SurfPrx)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s]Invaild SurfForTianshan proxy[took %d ms]"),
			plinfo.ident.name.c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, ICEEXCEPTION, "Invaild SurfForTianshan proxy");	
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
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s]fail to get playlist caught [%s][took %d ms]"),
			plinfo.ident.name.c_str(), surfex.errorDescription.c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, ICEEXCEPTION, "fail to get playlist caught '%s'",
			surfex.errorDescription.c_str());
		return retCode;
	}
	catch (const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s]fail to get playlist caught '%s'[took %d ms]"), 
			plinfo.ident.name.c_str(), ex.ice_name().c_str(), ZQTianShan::now() - lstart);

		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, ICEEXCEPTION, "fail to get playlist caught '%s'",
			ex.ice_name().c_str());
		return retCode;
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s] fail to get playlist caught unknown exception(%d)[took %d ms]"),
			plinfo.ident.name.c_str(),SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, INTERNAL, "fail to get playlist caught unknown exception(%d)",
			SYS::getLastErr());	
		return retCode;	
	}

	if(SurfAEData.aeList.size() < 1)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMPlayListQuery, "[%s] playlist element is null[took %d ms]"),
			plinfo.ident.name.c_str(), ZQTianShan::now() - lstart);

		retCode = PLNOTEXIST;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPLAYLISTQUERY, PLNOTEXIST, "playlist element is null");
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
		//			aeInfo.name = "";
		aeInfo.nasUrls.clear();
		aeInfo.volumeList.clear();
		aeInfo.attributes.clear();
		aedata.aeList.push_back(aeInfo);
	}

	aedata.netIDList = SurfAEData.netIDList;

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(LAMPlayListQuery, "[%s] get playlist from surf mode successfully [took %d ms]"), plinfo.ident.name.c_str(), ZQTianShan::now() - lstart);

	return retCode;
}

}}}//end namespace
