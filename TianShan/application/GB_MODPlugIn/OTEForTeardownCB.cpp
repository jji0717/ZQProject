// OTEForTeardownCB.cpp: implementation of the OTEForTeardownCB class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "OTEForTeardownCB.h"
#include "Log.h"

extern ZQ::common::Log * PMHOlog;
#define MLOG (*PMHOlog)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace ZQTianShan {
namespace Application{
namespace MOD{

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
		::com::izq::ote::tianshan::MoDIceInterfacePrx OTEPrx = ::com::izq::ote::tianshan::MoDIceInterfacePrx::uncheckedCast(r->getProxy());
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
	MLOG(ZQ::common::Log::L_INFO,
		CLOGFMT(OTEForTeardownCB, "Entry OTEForTeardownCB::ice_response()..."));

	if (rd.status == "2")
	{
		// failed
		MLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(OTEForTeardownCB, "ClientSession[%s] authorization teardown failed with error[%s: %s]"),
			clientSessionId.c_str(), rd.errorCode.c_str(), oteGetErrorDesc(atoi(rd.errorCode.c_str())));
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO, 
			CLOGFMT(OTEForTeardownCB, "ClientSession[%s] authorization teardown OK"),
			clientSessionId.c_str());
	}

	MLOG(ZQ::common::Log::L_INFO,
		CLOGFMT(OTEForTeardownCB, "Leave OTEForTeardownCB::ice_response()"));
}

void OTEForTeardownCB::ice_exception(const ::Ice::Exception& ext)
{
	MLOG(ZQ::common::Log::L_INFO,
		CLOGFMT(OTEFORTEARDOWNCB, "Entry OTEForTeardownCB::ice_exception()..."));

	try
	{
		ext.ice_throw();
	}
	catch(const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR, 
			CLOGFMT(OTEForTeardownCB, "ClientSession[%s] sessionTeardown() failed with error: caught Ice Exception, %s"),
			clientSessionId.c_str(), ex.ice_name().c_str());
	}

	MLOG(ZQ::common::Log::L_INFO, 
		CLOGFMT(OTEForTeardownCB, "Leave OTEForTeardownCB::ice_exception()"));
}
#endif
}}}//end namespace
