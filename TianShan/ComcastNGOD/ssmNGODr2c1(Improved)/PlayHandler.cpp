#include "./PlayHandler.h"

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif


PlayHandler::PlayHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
{
	_method = "PLAY";
	_inoutMap[MAP_KEY_METHOD] = _method;
#ifdef _DEBUG
	cout<<"construct PLAY handler"<<endl;
#endif

}

PlayHandler::~PlayHandler()
{
#ifdef _DEBUG
	cout<<"deconstruct PLAY handler"<<endl;
#endif

}

RequestProcessResult PlayHandler::process()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PlayHandler, "start to be processed"));

	if (false == _canProcess)
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "we can't process the request because of [%s]"), szBuf);
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

	// DO: get stream proxy
	if (false == getStream())
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	
	_reqRange = getRequestHeader(NGOD_HEADER_RANGE);
	if (false == _reqRange.empty())
	{
		if (stricmp(ZQ::StringOperation::nLeftStr(_reqRange, 4).c_str(), "npt=") != 0)
		{
			snprintf(szBuf, MY_BUFFER_SIZE - 1, "Range: [%s] doesn't start with [npt=]", _reqRange.c_str());
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
			responseError(RESPONSE_BAD_REQUEST);
			return RequestError;
		}
		
		std::string right_str;
		right_str = ZQ::StringOperation::getRightStr(_reqRange, "=", true);
		
		std::string second_str;
		int pos_tmp;
		if (true == ZQ::StringOperation::hasChar(right_str, '-', pos_tmp))
		{
			second_str = ZQ::StringOperation::midStr(right_str, -1, pos_tmp);
		}
		else 
		{
			second_str = right_str;
		}

		if (false == second_str.empty() && stricmp(second_str.c_str(), "now") != 0)
		{
			Ice::Long milliSecs = atof(second_str.c_str()) * 1000;
			try
			{
				HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(PlayHandler, "Seek stream: [%s] to %lld"), _pContext->streamFullID.c_str(), milliSecs);
				Ice::Long ret_msecs = _streamPrx->seekStream(milliSecs, 1);
				HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PlayHandler, "Seek stream: [%s] to %lld successfully"), _pContext->streamFullID.c_str(), ret_msecs);
			}
			catch(const ::TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, MY_BUFFER_SIZE - 1,"Seek stream caught [%s]:[%s]", ex.ice_name().c_str(), ex.message.c_str());
				HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
				responseError(RESPONSE_INTERNAL_ERROR);
				return RequestError;
			}
			catch(const ::Ice::Exception& ex)
			{
				snprintf(szBuf, MY_BUFFER_SIZE - 1,"Seek stream caught [%s]", ex.ice_name().c_str());
				HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
				responseError(RESPONSE_INTERNAL_ERROR);
				return RequestError;
			}
		}
	}

	std::string stateDept;
	::Ice::Long timeOffset;
	::Ice::Float currentSpeed;

	_reqScale = getRequestHeader(NGOD_HEADER_SCALE);
	if (false == _reqScale.empty())
	{	
		float f_speed = (float) atof(_reqScale.c_str());		
		try
		{
			HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(PlayHandler, "to perform setSpeedEx() on stream: [%s]"), _pContext->streamFullID.c_str());
			_streamPrx->setSpeedEx(f_speed, timeOffset, currentSpeed);
			HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PlayHandler, "perform setSpeedEx() on stream: [%s] successfully, timeOffset(%lld), currentSpeed(%f)"), _pContext->streamFullID.c_str(), timeOffset, currentSpeed);
		}
		catch(::TianShanIce::BaseException& ex)
		{
			HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(PlayHandler, "perform setSpeedEx() on stream: [%s] caught [%s]:[%s]"), _pContext->streamFullID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (Ice::Exception& ex)
		{
			HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(PlayHandler, "perform setSpeedEx() on stream: [%s] caught [%s]"), _pContext->streamFullID.c_str(), ex.ice_name().c_str());
		}
	}
	
	try
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(PlayHandler, "to perform playEx() on stream: [%s]"), _pContext->streamFullID.c_str());
		_streamPrx->playEx(timeOffset, currentSpeed);
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PlayHandler, "perform playEx() on stream: [%s] successfully, timeOffset(%lld), currentSpeed(%f)"), _pContext->streamFullID.c_str(), timeOffset, currentSpeed);
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform playEx() on stream: [%s] caught [%s]:[%s]", _pContext->streamFullID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	catch(const ::Ice::Exception& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform playEx() on stream: [%s] caught [%s]", _pContext->streamFullID.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	snprintf(szBuf, sizeof(szBuf) - 1, "%lld.%03I64d-", timeOffset / 1000, timeOffset % 1000);
	_retRange = "npt=";
	_retRange += szBuf;

	snprintf(szBuf, sizeof(szBuf) - 1, "%f", currentSpeed);
	_retScale = szBuf;
	
	_pResponse->setHeader(NGOD_HEADER_SCALE, _retScale.c_str());
	_pResponse->setHeader(NGOD_HEADER_RANGE, _retRange.c_str());
	
	responseOK();

	return RequestProcessed;
}

