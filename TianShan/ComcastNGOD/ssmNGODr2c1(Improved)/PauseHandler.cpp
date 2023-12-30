#include "./PauseHandler.h"

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

PauseHandler::PauseHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
{
	_method = "PAUSE";
	_inoutMap[MAP_KEY_METHOD] = _method;
#ifdef _DEBUG
	cout<<"construct PAUSE handler"<<endl;
#endif

}

PauseHandler::~PauseHandler()
{
#ifdef _DEBUG
	cout<<"deconstruct PAUSE handler"<<endl;
#endif

}

RequestProcessResult PauseHandler::process()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PauseHandler, "start to be processed"));

	if (false == _canProcess)
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PauseHandler, "we can't process the request because of [%s]"), szBuf);
		return RequestError;
	}

	if (false == getContext())
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	if (false == renewSession())
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	::Ice::Long timeOffset;

	// DO: get stream proxy in order to pause()
	if (false == getStream())
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	try
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(PauseHandler, "to perform pauseEx() on stream: [%s]"), _pContext->streamFullID);
		_streamPrx->pauseEx(timeOffset);
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PauseHandler, "perform pauseEx() on stream: [%s] successfully"), _pContext->streamFullID);
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform pauseEx() on stream: [%s] caught [%s]:[%s]", _pContext->streamFullID, ex.ice_name().c_str(), ex.message.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PauseHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	catch(const ::Ice::Exception& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform pauseEx() on stream: [%s] caught [%s]", _pContext->streamFullID, ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PauseHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, "%lld.%03I64d-", timeOffset / 1000, timeOffset % 1000);
	_retRange = "npt=";
	_retRange += szBuf;

	_pResponse->setHeader(NGOD_HEADER_RANGE, _retRange.c_str());
	responseOK();
	
	return RequestProcessed;
}

