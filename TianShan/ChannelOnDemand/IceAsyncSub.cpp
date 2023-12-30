
#include "log.h"
#include "IceAsyncSub.h"

using namespace ZQ::common;

#define LOG_MODULE_NAME			"IceAsync"


TodasForCodTeardownCB::TodasForCodTeardownCB(const std::string& cltSession):clientSessionId(cltSession)
{

}

void TodasForCodTeardownCB::ice_response(const ::com::izq::todas::integration::cod::SessionResultData& rd)
{
	if (rd.status == 2)
	{
		// failed
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ClientSession[%s] teardown failed with TODAS error: %s"), 
			clientSessionId.c_str(), rd.errorCode.c_str());
	}
	else
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "ClientSession[%s] TODAS sessionTeardown ok"),
			clientSessionId.c_str());
	}
}

void TodasForCodTeardownCB::ice_exception(const ::Ice::Exception& ext)
{
	try
	{
		ext.ice_throw();
	}
	catch(::com::izq::todas::integration::cod::TodasException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ClientSession[%s] sessionTeardown() failed with TODAS error: %s, %s"), 
			clientSessionId.c_str(), ex.errorCode.c_str(), ex.errorDescription.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ClientSession[%s] sessionTeardown() failed with error: caught Ice Exception, %s"), 
			clientSessionId.c_str(), ex.ice_name().c_str());
	}
}
