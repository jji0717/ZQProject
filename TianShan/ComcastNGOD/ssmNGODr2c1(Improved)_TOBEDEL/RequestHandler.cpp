#include "./RequestHandler.h"

RequestHandler::RequestHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq)
	: _ssmNGODr2c1(ssm), _pSite(pSite), _pRequest(pReq), _pResponse(NULL), _sessionPrx(NULL), _streamPrx(NULL), _purchasePrx(NULL), _postResponse(true), _pContext(NULL), _pContextPrx(NULL), _canProcess(true)
{
	try
	{
		bool bRet = preprocess();
		if (1 ==  bRet)// preprocess failed
			_canProcess = false;
	}
	catch (...)
	{
	}
}

RequestHandler::~RequestHandler()
{
}

std::string RequestHandler::getRequestType()
{
	return _method;
}

std::string RequestHandler::getSession()
{
	return _session;
}

std::string RequestHandler::getRequestHeader(const char* pHeader, int logLevel)
{	
	szBufLen = sizeof(szBuf) - 1;
	szBuf[szBufLen] = '\0';
	const char* pRet = _pRequest->getHeader(pHeader, szBuf, &szBufLen);

	std::string ret = (NULL != pRet) ? pRet : "";

	ZQ::StringOperation::trimAll(ret);

	HANDLERLOG(logLevel, HANDLERLOGFMT(RequestHandler, "get header [%s: %s]"), pHeader, ret.c_str());

	return ret;
}

void RequestHandler::getContentBody(int logLevel)
{
	uint32 contentLen = atoi(getRequestHeader(NGOD_HEADER_CONTENTLENGTH).c_str());
	contentLen ++; // ++ 为了分配一个缓冲区，其大小比content-body内容多一个字节
	unsigned char* pContentBody = new unsigned char[contentLen];
	pContentBody[contentLen - 1] = '\0';
	const char* pRet = _pRequest->getContent(pContentBody, &contentLen);

	_requestBody = (NULL != pRet) ? pRet : "";

	delete []pContentBody;

	HANDLERLOG(logLevel, HANDLERLOGFMT(RequestHandler, "get content body: [%s]"), _requestBody.c_str());
}

int RequestHandler::preprocess()
{
	if (NULL == _pSite)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "StreamSmithSite is null");
		HANDLERLOG(ZQ::common::Log::L_ALERT, CLOGFMT(RequestHandler, "%s"), szBuf);
		return 1;// error
	}

	if (NULL == _pRequest)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "Client Request is null");
		HANDLERLOG(ZQ::common::Log::L_ALERT, CLOGFMT(RequestHandler, "%s"), szBuf);
		return 1;// error
	}

	_pResponse = _pRequest->getResponse();
	if (NULL == _pResponse)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "Response Object is null");
		HANDLERLOG(ZQ::common::Log::L_ALERT, CLOGFMT(RequestHandler, "%s"), szBuf);
		return 1;// error
	}

	try
	{
		_pContext = new NGODr2c1::ContextImpl(_ssmNGODr2c1);
	}
	catch (bad_alloc)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "alloc memory for context object caught bad_alloc exception");
		HANDLERLOG(ZQ::common::Log::L_ALERT, CLOGFMT(RequestHandler, "%s"), szBuf);
		return 1;// error
	}
	catch (...)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "alloc memory for context object caught unexpect exception");
		HANDLERLOG(ZQ::common::Log::L_ALERT, CLOGFMT(RequestHandler, "%s"), szBuf);
		return 1;// error
	}

	if (NULL == _pContext)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "Context Object is null");
		HANDLERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RequestHandler, "%s"), szBuf);
		return 1;
	}

	_sequence = getRequestHeader(NGOD_HEADER_SEQ);
	_inoutMap[MAP_KEY_SEQUENCE] = _sequence;

	_session = getRequestHeader(NGOD_HEADER_SESSION, ZQ::common::Log::L_INFO);
	_inoutMap[MAP_KEY_SESSION] = _session;

	_userAgent = getRequestHeader(NGOD_HEADER_USERAGENT);
	_inoutMap[MAP_KEY_USERAGENT] = _userAgent;

	_xreason = getRequestHeader(NGOD_HEADER_XREASON, ZQ::common::Log::L_INFO);
	_inoutMap[MAP_KEY_XREASON] = _xreason;

	return 0;// success
}

int RequestHandler::responseError(const char* statusCodeLine, const char* szNotice)
{
	_pResponse->printf_preheader(statusCodeLine);

	if (_ssmNGODr2c1._globalSession != _session)
		_pResponse->setHeader(NGOD_HEADER_SESSION, _session.c_str());

	_pResponse->setHeader(NGOD_HEADER_MTHDCODE, _method.c_str());
	_pResponse->setHeader(NGOD_HEADER_SERVER, _ssmNGODr2c1._serverHeader.c_str());

	if (NULL == szNotice || 0 == strlen(szNotice))
		_pResponse->setHeader(NGOD_HEADER_NOTICE, szBuf);
	else 
		_pResponse->setHeader(NGOD_HEADER_NOTICE, szNotice);

	if (true == _postResponse)
		_pResponse->post();
	
	return 0;
}

int RequestHandler::responseOK()
{
	_pResponse->printf_preheader(RESPONSE_OK);

	if (_ssmNGODr2c1._globalSession != _session)
		_pResponse->setHeader(NGOD_HEADER_SESSION, _session.c_str());

	_pResponse->setHeader(NGOD_HEADER_MTHDCODE, _method.c_str());
	_pResponse->setHeader(NGOD_HEADER_SERVER, _ssmNGODr2c1._serverHeader.c_str());

	if (true == _postResponse)
		_pResponse->post();	

	return 0;
}

bool RequestHandler::getContext()
{
	if (NULL != _pContext && NULL != _pContextPrx)
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "context and context proxy already gained"));
		return true;
	}

	bool bRet = _ssmNGODr2c1.openContext(_session, _pContext, _pContextPrx, _inoutMap);

	if (false == bRet)
		snprintf(szBuf, sizeof(szBuf) - 1, "%s", _inoutMap[MAP_KEY_LASTERROR].c_str());

	return bRet;
}

bool RequestHandler::getContextByIdentity(::Ice::Identity& ident)
{
	if (NULL != _pContext && NULL != _pContextPrx)
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "context and context proxy already gained"));
		return true;
	}

	bool bRet = _ssmNGODr2c1.openContext(ident.name, _pContext, _pContextPrx, _inoutMap);

	if (false == bRet)
		snprintf(szBuf, sizeof(szBuf) - 1, "%s", _inoutMap[MAP_KEY_LASTERROR].c_str());

	return bRet;
}

bool RequestHandler::getContextByIdentity(NGODr2c1::ContextImplPtr& pContext
						, NGODr2c1::ContextPrx& pContextPrx, ::Ice::Identity& ident)
{
	bool bRet = _ssmNGODr2c1.openContext(ident.name, pContext, pContextPrx, _inoutMap);

	if (false == bRet)
		snprintf(szBuf, sizeof(szBuf) - 1, "%s", _inoutMap[MAP_KEY_LASTERROR].c_str());

	return bRet;
}

bool RequestHandler::getWeiwooSession()
{
	if (NULL != _sessionPrx)
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "weiwoo proxy already gained"));
		return true;
	}

	try
	{
		_sessionPrx = TianShanIce::SRM::SessionPrx::uncheckedCast(_ssmNGODr2c1._pCommunicator->stringToProxy(_pContext->weiwooFullID));
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "get weiwoo session(%s) caught: [%s]"
			, _pContext->weiwooFullID.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		return false;
	}

	if (NULL == _sessionPrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "result weiwoo session proxy: [%s] is null"
			, _pContext->weiwooFullID.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		return false;
	}

	return true;
}

bool RequestHandler::renewWeiwooSession(int timeout_value)
{
	if (-1 == timeout_value)
		timeout_value = 1000 * _ssmNGODr2c1._config._timeoutInterval + 60000;

	try
	{
		_sessionPrx->renew(timeout_value);
	}
	catch(const TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform renew() on weiwoo session: [%s] proxy caught [%s]:[%s]"
			, _pContext->weiwooFullID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		return false;
	}
	catch(const Ice::Exception& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform renew() on weiwoo session: [%s] proxy caught [%s]"
			, _pContext->weiwooFullID.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		return false;
	}

	return true;
}

bool RequestHandler::getStream()
{
	if (NULL != _streamPrx)
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "stream proxy already gained"));
		return true;
	}

	HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "to gain stream: [%s]")
		, _pContext->streamFullID.c_str());

	try
	{
		_streamPrx = ::TianShanIce::Streamer::StreamPrx::uncheckedCast(_ssmNGODr2c1._pCommunicator->stringToProxy(_pContext->streamFullID));
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform uncheckedCast to stream: [%s] caught [%s]"
			, _pContext->streamFullID.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		return false;
	}

	if (NULL == _streamPrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "result stream proxy: [%s] is null"
			, _pContext->streamFullID.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		return false;
	}

	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RequestHandler, "stream: [%s] gained successfully")
		, _pContext->streamFullID.c_str());

	return true;
}

bool RequestHandler::getPurchase()
{
	if (NULL != _purchasePrx)
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "purchase proxy already gained"));
		return true;
	}

	HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "do getPurchase()"));

	try
	{
		_purchasePrx = _sessionPrx->getPurchase();
	}
	catch (TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform getPurchase() on weiwoo session: [%s] caught [%s]:[%s]"
			, _pContext->weiwooFullID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		return false;
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform getPurchase() on weiwoo session: [%s] caught [%s]"
			, _pContext->weiwooFullID.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		return false;
	}

	if (NULL == _purchasePrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform getPurchase() on weiwoo session: [%s] successfully, but result is null"
			, _pContext->weiwooFullID.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		return false;
	}

	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RequestHandler, "purchase proxy gained via weiwoo session: [%s] successfully")
		, _pContext->weiwooFullID.c_str());

	return true;
}

void RequestHandler::changePostFlag(bool bFlag)
{
	_postResponse = bFlag;
}

void RequestHandler::addPrivateData(std::string key, TianShanIce::Variant& var)
{
	_pdMap[key] = var;
	if (TianShanIce::vtInts == var.type && var.ints.size() > 0)
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "Add key: [%s], value: [%d] to weiwoo session's private data"), key.c_str(), var.ints[0]);
	else if (TianShanIce::vtLongs == var.type && var.lints.size() > 0) 
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "Add key: [%s], value: [%lld] to weiwoo session's private data"), key.c_str(), var.lints[0]);
	else if (TianShanIce::vtStrings == var.type && var.strs.size() > 0)
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "Add key: [%s], value: [%s] to weiwoo session's private data"), key.c_str(), var.strs[0].c_str());
}

bool RequestHandler::flushPrivateData()
{
	HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "do flush weiwoo session: [%s] private data"), _pContext->weiwooFullID.c_str());

	if (NULL == _sessionPrx)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1, "weiwoo session proxy is null");
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		return false;
	}

	try 
	{
		_sessionPrx->setPrivateData2(_pdMap);
	}
	catch (TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1, "perform setPrivateData2() on weiwoo session: [%s] caught [%s]:[%s]", _pContext->weiwooFullID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		return false;
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1, "perform setPrivateData2() on weiwoo session: [%s] caught [%s]", _pContext->weiwooFullID.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		return false;
	}
	
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RequestHandler, "flush weiwoo session: [%s] private data successfully"), _pContext->weiwooFullID.c_str());

	return true;
}

bool RequestHandler::getStreamState(TianShanIce::Streamer::StreamState& streamState, std::string& stateDept)
{
	_inoutMap[MAP_KEY_STREAMFULLID] = _pContext->streamFullID;

	bool bRet = _ssmNGODr2c1.getStreamState(streamState, _streamPrx, _inoutMap);

	if (true == bRet)
		stateDept = _inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION];
	else 
		snprintf(szBuf, sizeof(szBuf) - 1, "%s", _inoutMap[MAP_KEY_LASTERROR].c_str());

	return bRet;
}

bool RequestHandler::getPositionAndScale(std::string& range, std::string& scale)
{
	_inoutMap[MAP_KEY_STREAMFULLID] = _pContext->streamFullID;

	bool bRet = _ssmNGODr2c1.getPositionAndScale(_streamPrx, _inoutMap);

	if (true == bRet)
	{
		range = _inoutMap[MAP_KEY_STREAMPOSITION];
		scale = _inoutMap[MAP_KEY_STREAMSCALE];
	}
	else 
		snprintf(szBuf, sizeof(szBuf) - 1, "%s", _inoutMap[MAP_KEY_LASTERROR].c_str());

	return bRet;
}

bool RequestHandler::addContext()
{
	bool bRet = _ssmNGODr2c1.addContext(_pContext, _inoutMap);

	if (false == bRet)
		snprintf(szBuf, sizeof(szBuf) -1, "%s", _inoutMap[MAP_KEY_LASTERROR].c_str());

	return bRet;
}

bool RequestHandler::removeContext(const std::string& reason)
{
	_inoutMap[MAP_KEY_REMOVECONTEXTREASON] = reason;

	bool bRet = _ssmNGODr2c1.removeContext("", _inoutMap);

	if (false == bRet)
		snprintf(szBuf, sizeof(szBuf) - 1, "%s", _inoutMap[MAP_KEY_LASTERROR].c_str());

	return bRet;
}

bool RequestHandler::destroyWeiwooSession(const std::string& reason)
{
	_inoutMap[MAP_KEY_WEIWOOSESSIONFULLID] = _pContext->weiwooFullID;
	_inoutMap[MAP_KEY_WEIWOOSESSIONDESTROYREASON] = reason;

	bool bRet = _ssmNGODr2c1.destroyWeiwooSession(_sessionPrx, _inoutMap);

	if (false == bRet)
		snprintf(szBuf, sizeof(szBuf) - 1, "%s", _inoutMap[MAP_KEY_LASTERROR].c_str());

	return bRet;
}

bool RequestHandler::renewSession(Ice::Long ttl)
{
	if (ttl == -1)
		ttl = _ssmNGODr2c1._config._timeoutInterval * 1000;

	SessionRenewCmd* pRenewCmd = new SessionRenewCmd(_ssmNGODr2c1, _pContext->ident.name.c_str(), _pContext->weiwooFullID
		, ZQTianShan::now() + ttl + 60000, _pContext->expiration);

	// DO: renew client session
	try
	{
		_pContextPrx->renew(ttl);
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "ClientSession(%s) renewed(%lldms)")
			, _pContext->ident.name.c_str(), ttl);
	}
	catch (const Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "renewClientSession(%s) caught(%s)"
			, _pContext->ident.name.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		return false;
	}
		
	_ssmNGODr2c1._pSessionWatchDog->watchSession(_pContext->ident, ttl);

	// DO: renew weiwoo session in a background thread
	pRenewCmd->start();

	return true;
}

SmartRequestHandler::SmartRequestHandler(RequestHandler*& pHandler) : _pHandler(pHandler)
{
}

SmartRequestHandler::~SmartRequestHandler()
{
	if (NULL != _pHandler)
		delete _pHandler;
	_pHandler = NULL;
}

