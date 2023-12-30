#include "Authorization.h"
#include "Environment.h"
#include "ote.h"

const char* AuthorizationGetErrorDesc(int nErrorCode)
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

TeardownCB::TeardownCB(const std::string& cltSession) : clientSessionId(cltSession)
{
}

void TeardownCB::ice_response(const NAMESPACE(SessionResultData)& rd)
{
	if (rd.status == "2")
	{
		// failed
		glog(ZQ::common::Log::L_ERROR, CLOGFMT("Teardown", "ClientSession[%s] authorization teardown failed with error[%s: %s]"), 
			clientSessionId.c_str(), rd.errorCode.c_str(), AuthorizationGetErrorDesc(atoi(rd.errorCode.c_str())));
	}
	else
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT("Teardown", "ClientSession[%s] authorization teardown OK"),
			clientSessionId.c_str());
	}
}

void TeardownCB::ice_exception(const ::Ice::Exception& ext)
{
	try
	{
		ext.ice_throw();
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT("Teardown", "ClientSession[%s] sessionTeardown() failed with error: caught Ice Exception, %s"), 
			clientSessionId.c_str(), ex.ice_name().c_str());
	}
}

ZQ::common::Mutex Authorization::_mutex;
Ice::Long Authorization::_lLoop = 0;

INTERFACEPRX Authorization::getAuthorization(::Ice::CommunicatorPtr communicator)
{
	INTERFACEPRX authorizePrx = NULL;
	char strtemp[65] = "";
	try
	{
		Ice::Long nMod = 0;
		{
			ZQ::common::MutexGuard guard(_mutex);
			nMod = _lLoop % _tsConfig._dwh._activeConnections;
			_lLoop++;
			glog(ZQ::common::Log::L_DEBUG, "session finish notification connecting to Authorization of TVBS, connId(%lld)", nMod);
		}
		snprintf(strtemp, sizeof(strtemp), "conn_%lld\0", nMod);
		Ice::ObjectPrx prx = communicator->stringToProxy(_tsConfig._dwh._endpoint);
		Ice::ObjectPrx conntectPrx = prx->ice_connectionId(strtemp);
		authorizePrx = INTERFACEPRX::uncheckedCast(conntectPrx);
	}
	catch(Ice::Exception&ex)
	{
	}
	catch(...)
	{
	}
	return authorizePrx;
}

bool Authorization::sessionSetup(::Ice::CommunicatorPtr communicator, const NAMESPACE(SessionData)& sd)
{
	if (_tsConfig._dwh._enable)
	{
		NAMESPACE(SessionResultData) rd;
		// dump the values
		for (NAMESPACE(Properties)::const_iterator pItor = sd.params.begin(); pItor != sd.params.end(); pItor ++)
		{
			glog(ZQ::common::Log::L_DEBUG, "authorize: [%s] = [%s]", pItor->first.c_str(), pItor->second.c_str());
		}

		INTERFACEPRX authorizePrx = getAuthorization(communicator);
		try
		{
			glog(ZQ::common::Log::L_INFO, "calling authorization sessionSetup");

			rd = authorizePrx->sessionSetup(sd);
			if (rd.status == "2")
			{
				glog(ZQ::common::Log::L_ERROR, "[%s] authorization setup failed with error [%s: %s]", 
					sd.clientSessionId.c_str(), rd.errorCode.c_str(), AuthorizationGetErrorDesc(atoi(rd.errorCode.c_str())));
				return false;
			}
			glog(ZQ::common::Log::L_INFO, "authorization setup successfully");
		}
		catch (const TianShanIce::BaseException& ex)
		{
			return false;
		}
		catch (const Ice::ObjectNotExistException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, "[%s] authorization setup caught %s, endpoint is %s", 
				sd.clientSessionId.c_str(), ex.ice_name().c_str(), _tsConfig._dwh._endpoint.c_str());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, "[%s] authorization setup caught %s, endpoint is %s", 
				sd.clientSessionId.c_str(), ex.ice_name().c_str(), _tsConfig._dwh._endpoint.c_str());
			return false;
		}
	}
	return true;
}

void Authorization::sessionTeardown(::Ice::CommunicatorPtr communicator, const NAMESPACE(SessionData)& sd)
{
	if (_tsConfig._dwh._enable)
	{
		for (NAMESPACE(Properties)::const_iterator pItor = sd.params.begin(); pItor != sd.params.end(); pItor ++)
		{
			glog(ZQ::common::Log::L_DEBUG, "authorize: [%s] = [%s]", pItor->first.c_str(), pItor->second.c_str());
		}

		std::string OTEendpoint = _tsConfig._dwh._endpoint;
		std::string strTerminateReason, strTeardownReason;
		NAMESPACE(Properties)::const_iterator iter = sd.params.find("terminateReason");
		if(iter != sd.params.end())
			strTerminateReason = iter->second;
		iter = sd.params.find("teardownReason");
		if(iter != sd.params.end())
			strTeardownReason = iter->second;

		INTERFACEPRX authorizePrx = getAuthorization(communicator);
		try
		{
			glog(ZQ::common::Log::L_INFO, "calling authorization teardown with terminate reason [%s] and teardownReason [%s]", 
				strTerminateReason.c_str (),strTeardownReason.c_str ());

			TeardownCBPtr pCB = new TeardownCB(sd.clientSessionId);
			authorizePrx->sessionTeardown_async(pCB, sd);
		}
		catch (const Ice::ObjectNotExistException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, "authorization teardown caught %s, endpoint is %s", ex.ice_name().c_str(), OTEendpoint.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, "authorization teardown caught %s, endpoint is %s", ex.ice_name().c_str(), OTEendpoint.c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, "authorization teardown caught unexpect exception");
		}
	}
}
