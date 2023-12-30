// LAMProviderQuery.cpp: implementation of the LAMGetPlayList class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "LAMProviderQuery.h"
#include "LAMFacade.h"
#include "SystemUtils.h"
#include "Log.h"

extern ZQ::common::Log * PMHOlog;
#define MLOG (*PMHOlog)

namespace ZQTianShan {
namespace Application{
namespace MOD{
#define LAMPROVIDERQUERY "LAMProviderQuery"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LAMProviderQuery::LAMProviderQuery(::Ice::CommunicatorPtr& _ic)
								: _iceComm(_ic)
{

}

LAMProviderQuery::~LAMProviderQuery()
{

}

int LAMProviderQuery::getProviderId(ProviderInfo& pidInfo)
{

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMProviderQuery ,"[%s] Entry getProviderId() entry='%s'"),
		        pidInfo.ident.name.c_str(),LAM_ProviderQuery_Name);


	Ice::Long lstart  = ZQTianShan::now();
	int retCode = PIDQUERYSUCCESS;

	// do: get lam interface proxy
	com::izq::am::facade::servicesForIce::LAMFacadePrx lamPrx = NULL;
	try
	{
		lamPrx = com::izq::am::facade::servicesForIce::LAMFacadePrx::checkedCast(_iceComm->stringToProxy(pidInfo.endpoint));
	}
	catch (const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMProviderQuery, "[%s] fail to get LAMFacade proxy caught [%s] at endpoint[%s][took %d ms]"),
			pidInfo.ident.name.c_str(), ex.ice_name().c_str(), pidInfo.endpoint.c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPROVIDERQUERY, retCode, "fail to get LAMFacade proxy caught [%s]",ex.ice_name().c_str());	
		return retCode;	
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMProviderQuery, "[%s] fail to get LAMFacade proxy caught unknown exception(%d)[took %d ms]"),
			pidInfo.ident.name.c_str(),SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPROVIDERQUERY, retCode, "fail to get LAMFacade proxy caught unknown exception(%d)",
			SYS::getLastErr());	
		return retCode;	
	}

	if(!lamPrx)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMProviderQuery, "[%s]Invaild LAMFacade proxy[took %d ms]"),
			pidInfo.ident.name.c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPROVIDERQUERY, retCode, "Invaild LAMFacade proxy");	
		return retCode;
	}

	try
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(LAMProviderQuery, "[%s]lookupPid for assetId=%s"), pidInfo.ident.name.c_str(), pidInfo.providerAssetId.c_str());
		pidInfo.providerId = lamPrx->lookupPID(pidInfo.providerAssetId);
	}
	catch (const com::izq::am::facade::servicesForIce::LogicError& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMProviderQuery, "[%s] fail to lookupPID caught %s, %d, %s[took %d ms]"), 
			pidInfo.ident.name.c_str(), ex.ice_name().c_str(), ex.errorCode, ex.errorMessage.c_str(), ZQTianShan::now() - lstart);
		retCode = PIDNOTEXIST;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPROVIDERQUERY, retCode, "fail to lookupPID caught %s, %d, %s", ex.ice_name().c_str(), ex.errorCode, ex.errorMessage.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMProviderQuery, "[%s]fail to lookupPID caught '%s'[took %d ms]"), 
			pidInfo.ident.name.c_str(), ex.ice_name().c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPROVIDERQUERY, retCode, "fail to lookupPID caught '%s'",
			ex.ice_name().c_str());
		return retCode;
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(LAMProviderQuery, "[%s] fail to lookupPID caught unknown exception(%d)[took %d ms]"),
			pidInfo.ident.name.c_str(), SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, LAMPROVIDERQUERY, retCode, "fail to lookupPID caught unknown exception(%d)",
			SYS::getLastErr());	
		return retCode;	
	}

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(LAMProviderQuery, "[%s] lookup Aesst[%s]PID is[%s]successfully[took %d ms]"),
		pidInfo.ident.name.c_str(), pidInfo.providerAssetId.c_str(), pidInfo.providerId.c_str(), ZQTianShan::now() - lstart);

	return retCode;
}


}}}//end namespace
