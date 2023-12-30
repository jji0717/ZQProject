// LSMSForMODAuthorization.cpp: implementation of the LSMSForMODAuthorization class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "LSMSForMoDAuthorization.h"
#include "LSMSForMoD.h"
#include "LSMSForMoDForTeardownCB.h"
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

LSMSForMODAuthorization::LSMSForMODAuthorization(::Ice::CommunicatorPtr& _ic)
									: _iceComm(_ic)
{
   _lLoop = 0;
}

LSMSForMODAuthorization::~LSMSForMODAuthorization()
{

}

int LSMSForMODAuthorization::OnAuthPurchase(AuthorInfo& authorInfo, ::TianShanIce::ValueMap& privData)
{
	MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization,"Entry OnAuthPurchase() Type = %s...[%s]"), 
		LSMS_Authorization_NAME, authorInfo.endpoint.c_str());

    std::string strAuthorParameter ="";
    int retCode = AUTHORSUCCESS;
	::com::izq::lsms::integration::mod::SessionData sd;
	::com::izq::lsms::integration::mod::SessionResultData rd;
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
		sd.params.insert(::com::izq::lsms::integration::mod::Properties::value_type(keyStr, ((::TianShanIce::Variant)(mItor->second)).strs[0]));
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
	for (::com::izq::lsms::integration::mod::Properties::iterator pItor = sd.params.begin(); pItor != sd.params.end(); pItor ++)
	{
// 		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization, "authorize: [%s] = [%s]"), 
// 			pItor->first.c_str(), pItor->second.c_str());

		char temp[512] = "";
		sprintf(temp, "%s[%s],", pItor->first.c_str(), pItor->second.c_str());
		strAuthorParameter += temp;
	}

	Ice::Long lstart  = ZQTianShan::now();
	::com::izq::lsms::integration::mod::LSMSForMoDPrx lsmsprx = NULL;
	char strtemp[65] = "";
	try
	{
		/*Ice::Long timestamp = ZQTianShan::now();
		Ice::Long  nMod = (timestamp >> 3) % activeconsize;*/
		Ice::Long nMod = 0;
		{
			ZQ::common::MutexGuard guard(_mutex);
			nMod = _lLoop++ % activeconsize;
			sprintf(strtemp, "conn_%lld\0", nMod);
			MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization, "conn[%s] connecting to LSMSForMoD of TVBS for setup-auth: %s"), strtemp, strAuthorParameter.c_str());
		}
		Ice::ObjectPrx prx = _iceComm->stringToProxy(authorInfo.endpoint);
		Ice::ObjectPrx LsmsConnectPrx = prx->ice_connectionId(strtemp);
		lsmsprx = ::com::izq::lsms::integration::mod::LSMSForMoDPrx::uncheckedCast(LsmsConnectPrx);
	}
	catch(Ice::Exception&ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization, "failed to connected to LSMSForMoD of TVBS, caught exception(%s) [took %d ms]"),
			ex.ice_name().c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "LSMSForMODAuthorization", ICEEXCEPTION, " failed to connected to LSMSForMoD of TVBS(%s)", 
			ex.ice_name().c_str());
	}
	catch(...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization, "failed to connected to LSMSForMoD of TVBS, error(%d) [took %d ms]"),
			SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "LSMSForMODAuthorization", INTERNAL, " failed to connected to LSMSForMoD of TVBS caught error(%d)", 
			SYS::getLastErr());
	}

	if(!lsmsprx)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization, "NULL LSMSForMoD proxy[%s] to TVBS [took %d ms]"),
			authorInfo.endpoint.c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "LSMSForMODAuthorization", ICEEXCEPTION, " NULL LSMSForMoD proxy to TVBS");
	}

	Ice::Long lTimeGetProxy  = ZQTianShan::now();

//	MLOG(ZQ::common::Log::L_INFO,AuthorFmt(LSMSForMODAuthorization, "setup authorization connected to LSMSForMoD of TVBS, connId(%s)"),strtemp);

	try
	{
		rd = lsmsprx->sessionSetup(sd);
	}
	catch (const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_WARNING, AuthorFmt(LSMSForMODAuthorization, "setup authorization failed: %s [took %d ms], pass the authorization"), 
			ex.ice_name().c_str(), ZQTianShan::now() - lstart);
		rd.status = "0";

		//		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization, "setup authorization failed: %s [took %d ms]"), 
		//			ex.ice_name().c_str(), ZQTianShan::now() - lstart);
		//		retCode = ICEEXCEPTION;

		//		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "LSMSForMODAuthorization", ICEEXCEPTION, " setup authorization failed: %s ", 
		//			ex.ice_name().c_str());
	}

	if (rd.status == "2")
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt( LSMSForMODAuthorization, "setup authorization refused with error[%s: %s] [took %d ms]"),
			rd.errorCode.c_str(), LSMSGetErrorDesc(atoi(rd.errorCode.c_str())), ZQTianShan::now() - lstart);	
		retCode = AUTHORFAILED;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "LSMSForMODAuthorization", AUTHORFAILED, " setup authorization refused[%s: %s]", 
			rd.errorCode.c_str(), LSMSGetErrorDesc(atoi(rd.errorCode.c_str())));
	}

	MLOG(ZQ::common::Log::L_INFO, AuthorFmt( LSMSForMODAuthorization, "get LSMSForMoD proxy took %d ms and setup authorization successfully [took %d ms],author parameter:[%s]"), 
		(int)(lTimeGetProxy - lstart), (int)(ZQTianShan::now() - lTimeGetProxy), strAuthorParameter.c_str());

	retCode = AUTHORSUCCESS;

	return retCode;
}

int LSMSForMODAuthorization::OnDestroyPurchase(AuthorInfo& authorInfo, const ::TianShanIce::Properties& prop)
{
	MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization, "Entry OnDestroyPurchase() Type = %s...[%s]"),
		LSMS_Authorization_NAME, authorInfo.endpoint.c_str());
	
    int retCode = AUTHORSUCCESS;	
	::com::izq::lsms::integration::mod::SessionData sd;
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


		std::string strFinalNPT="";
		itProp = prop.find(SYS_PROP(FinalNPT));
		if(itProp != prop.end())
		{
			strFinalNPT = itProp->second;
		}
		sd.params["FinalNPT"]= strFinalNPT;

		MLOG(ZQ::common::Log::L_INFO, AuthorFmt(LSMSForMODAuthorization, "notifying session finish with terminate reason [%s] and teardownReason [%s] and FinalNPT[%s]"), 
			strTerminateReason.c_str(), strTeardownReason.c_str(), strFinalNPT.c_str());
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_DEBUG,AuthorFmt(LSMSForMODAuthorization,"notifying session finish caught error(%d)"), SYS::getLastErr());
		retCode = INTERNAL;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "LSMSForMODAuthorization", INTERNAL, " notifying session finish caught error(%d)", 
			SYS::getLastErr());
	}

	Ice::Long lstart  = ZQTianShan::now();

#if ICE_INT_VERSION / 100 >= 306
	LSMSForMoDForStateCBPtr LSMCbPtr;
#else
	LSMSForMoDForTeardownCBPtr pCB;
#endif
	::com::izq::lsms::integration::mod::LSMSForMoDPrx lsmsprx = NULL;	
	char strtemp[65] = "";
	try
	{
/*		Ice::Long timestamp = ZQTianShan::now();
		Ice::Long  nMod = (timestamp >> 3) % activeconsize;*/
		Ice::Long nMod = 0;
		{
			ZQ::common::MutexGuard guard(_mutex);
			nMod = _lLoop++ % activeconsize;
			sprintf(strtemp, "conn_%lld\0", nMod);
			MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization, "conn[%s] connecting to LSMSForMoD of TVBS for session finish notification"), strtemp);
		}
		Ice::ObjectPrx prx = _iceComm->stringToProxy(authorInfo.endpoint);
		Ice::ObjectPrx LsmsConnectPrx = prx->ice_connectionId(strtemp);
		lsmsprx = ::com::izq::lsms::integration::mod::LSMSForMoDPrx::uncheckedCast(LsmsConnectPrx);
#if ICE_INT_VERSION / 100 >= 306
		//LSMSForMoDForStateCBPtr LSMCbPtr = new LSMSForMoDForStateCB(authorInfo.clientSessionId);
		LSMCbPtr = new LSMSForMoDForStateCB(authorInfo.clientSessionId);
		Ice::CallbackPtr genericCB = Ice::newCallback(LSMCbPtr, &LSMSForMoDForStateCB::sessionTeardown);
#else	
		pCB = new LSMSForMoDForTeardownCB(authorInfo.clientSessionId);
#endif
	}
	catch(Ice::Exception&ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization, "failed to connected to LSMSForMoD of TVBS caught exception(%s) [took %d ms]"),
			ex.ice_name().c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "LSMSForMODAuthorization", ICEEXCEPTION, " failed to connected to LSMSForMoD of TVBS(%s)", 
			ex.ice_name().c_str());
	}
	catch(...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization, "failed to connected to LSMSForMoD of TVBS, error(%d) [took %d ms]"),
			SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "LSMSForMODAuthorization", INTERNAL, " failed to connected to LSMSForMoD of TVBS caught error(%d)", 
			SYS::getLastErr());
	}

	if(!lsmsprx)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization, "NULL LSMSForMoD proxy[%s] to TVBS [took %d ms]"),
			authorInfo.endpoint.c_str(), ZQTianShan::now() - lstart);

		retCode = ICEEXCEPTION;

		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "LSMSForMODAuthorization", ICEEXCEPTION, "NULL LSMSForMoD proxy to TVBS");	
	}

	Ice::Long lTimeGetProxy  = ZQTianShan::now();

//	MLOG(ZQ::common::Log::L_INFO, AuthorFmt(LSMSForMODAuthorization, "session finish notification connected to LSMSForMoD of TVBS, connId(%s)"), strtemp);

	try
	{
#if ICE_INT_VERSION / 100 >= 306
		LSMSForMoDForStateCBPtr LSMCbPtr = new LSMSForMoDForStateCB(authorInfo.clientSessionId);
		Ice::CallbackPtr genericCB = Ice::newCallback(LSMCbPtr, &LSMSForMoDForStateCB::sessionTeardown);
		lsmsprx->begin_sessionTeardown(sd,genericCB);
#else
		lsmsprx->sessionTeardown_async(pCB, sd);
#endif
	}
	catch (const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization, "session finish notification caught(%s) [took %d ms]"), ex.ice_name().c_str(), ZQTianShan::now() - lstart);
		retCode = ICEEXCEPTION;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "LSMSForMODAuthorization", ICEEXCEPTION, " session finish notification caught(%s)", 
			ex.ice_name().c_str());
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_DEBUG, AuthorFmt(LSMSForMODAuthorization,"session finish notification caught error(%d) [took %d ms]"),
			SYS::getLastErr(), ZQTianShan::now() - lstart);
		retCode = INTERNAL;
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "LSMSForMODAuthorization", INTERNAL, " session finish notification caught error(%d)", 
			SYS::getLastErr());	
	}
	retCode = AUTHORSUCCESS;
	MLOG(ZQ::common::Log::L_INFO, AuthorFmt(LSMSForMODAuthorization, "get LSMSForMOD proxy took %d ms and session finish notification has been issued [took %d ms]"),
		(int)(lTimeGetProxy - lstart), (int)(ZQTianShan::now() - lTimeGetProxy));	
	return retCode;
}
	
}}}//end namespace
