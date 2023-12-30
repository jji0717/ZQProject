// OTEAuthorization.cpp: implementation of the OTEAuthorization class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "OTEAuthorization.h"
#include "ote.h"
#include "OTEForTeardownCB.h"
#include "../StreamSmith/RtspRelevant.h"
#include "SiteDefines.h"
#include "SystemUtils.h"
#include "Log.h"

extern ZQ::common::Log * PMHOlog;
#define MLOG (*PMHOlog)

namespace ZQTianShan {
namespace Application{
namespace MOD{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

OTEAuthorization::OTEAuthorization(::Ice::CommunicatorPtr& _ic)
									: _iceComm(_ic)
{
  _lLoop = 0;
}

OTEAuthorization::~OTEAuthorization()
{

}

int OTEAuthorization::OnAuthPurchase(AuthorInfo& authorInfo, ::TianShanIce::ValueMap& privData)
{
	MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization,
		"Entry OnAuthPurchase() Type = %s...[%s]"), OTE_Authorization_NAME, authorInfo.endpoint.c_str());

	Ice::Long lstart  = ZQTianShan::now();
    std::string strAuthorParameter ="";
    int retCode = AUTHORSUCCESS;
	::com::izq::ote::tianshan::SessionData sd;
	::com::izq::ote::tianshan::SessionResultData rd;
	sd.serverSessionId = authorInfo.serverSessionId;
	sd.clientSessionId = authorInfo.clientSessionId;
	int activeconsize = DefaultActiveConnectSize;
	for (::TianShanIce::ValueMap::iterator mItor = privData.begin(); mItor != privData.end(); mItor ++)
	{
		if (((::TianShanIce::Variant)(mItor->second)).strs.size() == 0)
			continue;
		std::string keyStr = mItor->first;

/*		if (stricmp(String::nLeftStr(keyStr, strlen(ClientRequestPrefix)).c_str(), ClientRequestPrefix) == 0)
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
		sd.params.insert(::com::izq::ote::tianshan::Properties::value_type(keyStr, ((::TianShanIce::Variant)(mItor->second)).strs[0]));
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

	privator = privData.find(PD_KEY_ActiveConnectSize);
	if(privator != privData.end() && privator->second.ints.size() == 1)
	{
		activeconsize = privator->second.ints[0];
	}
	if(activeconsize < 1)
	{
		activeconsize = DefaultActiveConnectSize;
	}

	//add extra params
	sd.params[PD_KEY_SiteName]= virtualSiteTemp;
	sd.params[PD_KEY_Path]= apppathTemp;
	sd.params[PD_KEY_URL]= fullurl;
	
	// dump the values
	for (::com::izq::ote::tianshan::Properties::iterator pItor = sd.params.begin(); pItor != sd.params.end(); pItor ++)
	{
// 		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization, "authorize: [%s] = [%s]"), 
// 			pItor->first.c_str(), pItor->second.c_str());
		char temp[512] = "";
		sprintf(temp, "%s[%s],", pItor->first.c_str(), pItor->second.c_str());
		strAuthorParameter += temp;
	}
	
	::com::izq::ote::tianshan::MoDIceInterfacePrx otePrx = NULL;
	char strtemp[65] = "";
	try
	{
		/*		Ice::Long timestamp = ZQTianShan::now();
		Ice::Long  nMod = (timestamp >> 3) % activeconsize;*/
		Ice::Long nMod = 0;
		{
			ZQ::common::MutexGuard guard(_mutex);
			nMod = _lLoop % activeconsize;
			_lLoop++;
			MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization, "setup authorization connectting to MoDIceInterface of TVBS, connId(%lld), %s"), nMod, strAuthorParameter.c_str());
		}
		sprintf(strtemp, "conn_"FMT64"\0", nMod);
		Ice::ObjectPrx prx = _iceComm->stringToProxy(authorInfo.endpoint);
		Ice::ObjectPrx oteConnectPrx = prx->ice_connectionId(strtemp);
		otePrx = ::com::izq::ote::tianshan::MoDIceInterfacePrx::uncheckedCast(oteConnectPrx);
	}
	catch(Ice::Exception&ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization, "failed to connected to MoDIceInterface of TVBS, caught exception(%s) [took %d ms]"),
			ex.ice_name().c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "OTEAuthorization", ICEEXCEPTION, " failed to connected to MoDIceInterface of TVBS(%s)", 
			ex.ice_name().c_str());
	}
	catch(...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization, "failed to connected to MoDIceInterface of TVBS, error(%d)[took %d ms]"),
			SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "OTEAuthorization", INTERNAL, " failed to connected to MoDIceInterface of TVBS caught error(%d)", 
			SYS::getLastErr());
	}

	if(!otePrx)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization, "NULL MoDIceInterface proxy[%s] to TVBS [took %d ms]"),
			authorInfo.endpoint.c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "OTEAuthorization", ICEEXCEPTION, " NULL MoDIceInterface proxy to TVBS");
	}

//	MLOG(ZQ::common::Log::L_INFO, AuthorFmt(OTEAuthorization, "setup authorization connected to MoDIceInterface of TVBS, connId(%s)"), strtemp);

	try
	{
		rd = otePrx->sessionSetup(sd);
	}
	catch (const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization, "setup authorization failed: %s [took %d ms], pass the authorization"), 
			ex.ice_name().c_str(), ZQTianShan::now() - lstart);
		rd.status = "0";

		//		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization, "setup authorization failed: %s [took %d ms]"), 
		//			ex.ice_name().c_str(), ZQTianShan::now() - lstart);
		//		retCode = ICEEXCEPTION;

		//		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "OTEAuthorization", ICEEXCEPTION, " setup authorization failed: %s", 
		//			ex.ice_name().c_str());	
	}

	if (rd.status == "2")
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt( OTEAuthorization, "setup authorization refused with error[%s: %s] [took %d ms]"),
			rd.errorCode.c_str(), oteGetErrorDesc(atoi(rd.errorCode.c_str())), ZQTianShan::now() - lstart);	
		retCode = AUTHORFAILED;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "OTEAuthorization", AUTHORFAILED, " setup authorization refused[%s: %s]", 
			 rd.errorCode.c_str(), oteGetErrorDesc(atoi(rd.errorCode.c_str())));	
	}

	MLOG(ZQ::common::Log::L_INFO, AuthorFmt( OTEAuthorization, "setup authorization successfully [took %lld ms], %s"), ZQTianShan::now() - lstart, strAuthorParameter.c_str());

	retCode = AUTHORSUCCESS;

	return retCode;
}

int OTEAuthorization::OnDestroyPurchase(AuthorInfo& authorInfo, const ::TianShanIce::Properties& prop)
{
	MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization, "Entry OnDestroyPurchase() Type = %s...[%s]"),
		OTE_Authorization_NAME, authorInfo.endpoint.c_str());

	Ice::Long lstart  = ZQTianShan::now();

    int retCode = AUTHORSUCCESS;	
	::com::izq::ote::tianshan::SessionData sd;
	int activeconsize = DefaultActiveConnectSize;
	try
	{
		sd.serverSessionId = authorInfo.serverSessionId;
		sd.clientSessionId = authorInfo.clientSessionId;
		
		SYS::TimeStamp st;
		char strTime[48];
		sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d", st.year,st.month,st.day,st.hour,st.minute,st.second);
		sd.params["ViewEndTime"] = strTime;
		std::string	strTeardownReason = "";
		std::string strTerminateReason = "";
		::TianShanIce::Properties::const_iterator itProp;
		itProp = prop.find( SYS_PROP(teardownReason));
		if (itProp != prop.end())
		{
			strTeardownReason = itProp->second;
		}
		itProp = prop.find (SYS_PROP(terminateReason));
		if (itProp != prop.end())
		{
			strTerminateReason = itProp->second;
		}
		sd.params["teardownReason"] = strTeardownReason;
		sd.params["terminateReason"] = strTerminateReason;
		sd.params["Reason"] = (!strTeardownReason.empty() ? strTeardownReason : strTerminateReason);

		// begin find virtaulsiteName and apppath 
		std::string virtualSiteTemp = "";
		std::string apppathTemp = "";
		std::string fullurl = "";

		itProp = prop.find(PD_KEY_SiteName);
		if(itProp != prop.end())
		{
			virtualSiteTemp = itProp->second;
		}
		itProp = prop.find(PD_KEY_Path);
		if(itProp != prop.end())
		{
			apppathTemp = itProp->second;
		}
		itProp = prop.find(PD_KEY_URL);
		if(itProp != prop.end())
		{
			fullurl = itProp->second;
		}
		// end find virtaulsiteName and apppath 

        itProp = prop.find(PD_KEY_ActiveConnectSize);
		if(itProp != prop.end())
		{
          activeconsize = atoi(itProp->second.c_str());
		}
		if(activeconsize < 1)
		{
			activeconsize = DefaultActiveConnectSize;
		}

		//add extra params
		sd.params[PD_KEY_SiteName]= virtualSiteTemp;
		sd.params[PD_KEY_Path]= apppathTemp;
		sd.params[PD_KEY_URL]= fullurl;
		
		MLOG(ZQ::common::Log::L_INFO, AuthorFmt(OTEAuthorization, "notifying session finish with terminate reason [%s] and teardownReason [%s]"), 
			strTerminateReason.c_str(), strTeardownReason.c_str());
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization,"notifying session finish caught error(%d) [took %d ms]"), SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "OTEAuthorization", INTERNAL, " notifying session finish caught error(%d)", 
			SYS::getLastErr());	
	}

	::com::izq::ote::tianshan::MoDIceInterfacePrx otePrx = NULL;
	char strtemp[65] = "";

#if ICE_INT_VERSION / 100 >= 306   
	OTEForStateCBPtr OTECbPtr;
	Ice::CallbackPtr genericCB;
#else
	OTEForTeardownCBPtr pCB;
#endif
	try
	{
/*		Ice::Long timestamp = ZQTianShan::now();
		Ice::Long  nMod = (timestamp >> 3) % activeconsize;*/

		Ice::Long nMod = 0;
		{
			ZQ::common::MutexGuard guard(_mutex);
			nMod = _lLoop % activeconsize;
			_lLoop++;
			MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization, "session finish notification connectting to MoDIceInterface of TVBS, connId(%lld)"), nMod);
		}
		sprintf(strtemp, "conn_"FMT64"\0", nMod);
		Ice::ObjectPrx prx = _iceComm->stringToProxy(authorInfo.endpoint);
		Ice::ObjectPrx oteConnectPrx = prx->ice_connectionId(strtemp);
		otePrx = ::com::izq::ote::tianshan::MoDIceInterfacePrx::uncheckedCast(oteConnectPrx);

#if ICE_INT_VERSION / 100 >= 306   
	OTECbPtr = new OTEForStateCB(authorInfo.clientSessionId);
	genericCB = Ice::newCallback(OTECbPtr, &OTEForStateCB::sessionTeardown);
#else
		pCB = new OTEForTeardownCB(authorInfo.clientSessionId);
#endif
	}
	catch(Ice::Exception&ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization, "failed to connected to MoDIceInterface of TVBS caught exception(%s)[took %d ms]"),
			ex.ice_name().c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "OTEAuthorization", ICEEXCEPTION, " failed to connected to MoDIceInterface of TVBS(%s)", 
			ex.ice_name().c_str());
	}
	catch(...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization, "failed to connected to MoDIceInterface of TVBS, error(%d) [took %d ms]"),
			SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "OTEAuthorization", INTERNAL, " failed to connected to MoDIceInterface of TVBS caught error(%d)", 
			SYS::getLastErr());
	}

	if(!otePrx)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization, "NULL MoDIceInterface proxy[%s] to TVBS [took %d ms]"),
			authorInfo.endpoint.c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "OTEAuthorization", ICEEXCEPTION, " NULL MoDIceInterface proxy to TVBS");
	}

//	MLOG(ZQ::common::Log::L_INFO, AuthorFmt(OTEAuthorization, "session finish notification connected to MoDIceInterface of TVBS, connId(%s)"), strtemp);

	try
	{

#if ICE_INT_VERSION / 100 >= 306 
		otePrx->begin_sessionTeardown(sd,genericCB);
#else
		otePrx->sessionTeardown_async(pCB, sd);
#endif
		MLOG(ZQ::common::Log::L_INFO, AuthorFmt(OTEAuthorization, "session finish notification has been issued [took %d ms]"), ZQTianShan::now() - lstart);
	}
	catch (const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization, "session finish notification caught(%s) [took %d ms]"), ex.ice_name().c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "OTEAuthorization", ICEEXCEPTION, " session finish notification caught(%s)", 
			ex.ice_name().c_str());
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(OTEAuthorization,"session finish notification caught error(%d) [took %d ms]"),
			SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "OTEAuthorization", INTERNAL, " session finish notification caught error(%d)", 
			SYS::getLastErr());	
	}
    retCode = AUTHORSUCCESS;
	return retCode;
}
	
}}}//end namespace
