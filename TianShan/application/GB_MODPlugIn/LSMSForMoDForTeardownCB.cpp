// LSMSForMoDForTeardownCB.cpp: implementation of the LSMSForMoDForTeardownCB class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "LSMSForMoDForTeardownCB.h"
#include "Log.h"

extern ZQ::common::Log * PMHOlog;
#define MLOG (*PMHOlog)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace ZQTianShan {
namespace Application{
namespace MOD{

const char* LSMSGetErrorDesc(int nErrorCode)
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
LSMSForMoDForStateCB::LSMSForMoDForStateCB(const std::string& cltSession):clientSessionId(cltSession) {}
void LSMSForMoDForStateCB::sessionTeardown(const Ice::AsyncResultPtr& r)	
{
		::com::izq::lsms::integration::mod::LSMSForMoDPrx LSMSFPrx = ::com::izq::lsms::integration::mod::LSMSForMoDPrx::uncheckedCast(r->getProxy());
try
	{
		LSMSFPrx->end_sessionTeardown(r);
	}
	catch(const Ice::Exception& ex)
	{
		handleException(ex);
	}
	//end_sessionTeardown();
}
#else
LSMSForMoDForTeardownCB::LSMSForMoDForTeardownCB(const std::string& cltSession): clientSessionId(cltSession)
{
}

void LSMSForMoDForTeardownCB::ice_response(const com::izq::lsms::integration::mod::SessionResultData& rd)
{
	MLOG(ZQ::common::Log::L_INFO,
		CLOGFMT(LSMSForMoDForTeardownCB, "Entry LSMSForMoDForTeardownCB::ice_response()..."));

	if (rd.status == "2")
	{
		// failed
		MLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(LSMSForMoDForTeardownCB, "ClientSession[%s] authorization teardown failed with error[%s: %s]"),
			clientSessionId.c_str(), rd.errorCode.c_str(), LSMSGetErrorDesc(atoi(rd.errorCode.c_str())));
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO, 
			CLOGFMT(LSMSForMoDForTeardownCB, "ClientSession[%s] authorization teardown OK"),
			clientSessionId.c_str());
	}

	MLOG(ZQ::common::Log::L_INFO,
		CLOGFMT(LSMSForMoDForTeardownCB, "Leave LSMSForMoDForTeardownCB::ice_response()"));
}

void LSMSForMoDForTeardownCB::ice_exception(const ::Ice::Exception& ext)
{
	MLOG(ZQ::common::Log::L_INFO,
		CLOGFMT(LSMSForMoDForTeardownCB, "Entry LSMSForMoDForTeardownCB::ice_exception()..."));

	try
	{
		ext.ice_throw();
	}
	catch(const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR, 
			CLOGFMT(LSMSForMoDForTeardownCB, "ClientSession[%s] sessionTeardown() failed with error: caught Ice Exception, %s"),
			clientSessionId.c_str(), ex.ice_name().c_str());
	}

	MLOG(ZQ::common::Log::L_INFO, 
		CLOGFMT(LSMSForMoDForTeardownCB, "Leave LSMSForMoDForTeardownCB::ice_exception()"));
}
#endif
}}}//end namespace
