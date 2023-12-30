// File Name : TeardownHandler.cpp

#include "TeardownHandler.h"

#include "Environment.h"

#include "CRGSessionManager.h"

#include "RtspHeaderDefines.h"

namespace GBss
{

TeardownHandler::TeardownHandler(ZQ::common::Log& fileLog, Environment& env, 
								 IStreamSmithSite* pSite, IClientRequestWriter* pReq)
: RequestHandler(fileLog, env, pSite, pReq)
{
	_method = "TEARDOWN";
}

TeardownHandler::~TeardownHandler()
{

}

RequestProcessResult TeardownHandler::doContentHandler()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(TeardownHandler, "start processing"));
	
	// session exist ?
	if (!findSession())
	{
		return RequestError;
	}
	int playPosition, npt;
	if (!getStopNPT(playPosition, npt))
	{
		return RequestError;
	}
	if (npt < 0)
	{
		playPosition = -1;
	}
	char stopNptBuf[32];
	if( playPosition >=0 && playPosition % 1000 )
	{
		sprintf(stopNptBuf,"%d.%03d",playPosition/1000,playPosition%1000);
	}
	else
	{
		sprintf(stopNptBuf,"%d",playPosition);
	}
	_response->setHeader(HeaderStopNPT, stopNptBuf);
	_response->setHeader(HeaderGlobalSessId, _sessionProxy->getGlobalSessId().c_str());
	try
	{
		// destroy stream instance
		_statusCode = _sessionProxy->destroy() ? 200 : 503;
		_sessionProxy = NULL;
	}
	catch (const Ice::Exception& ex)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when destoried stream instance", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR,HANDLERLOGFMT(TeardownHandler, "%s"), _szBuf);
		return RequestError; 
	}
	
	return RequestProcessed;
}

} // end GBss
