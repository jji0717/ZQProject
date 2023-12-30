#include "./RequestHandler.h"
#include <strHelper.h>


// add by zjm to support versioning
const char* RequestHandler::NgodVerStrs[5]= {"com.comcast.ngod.r2", "com.comcast.ngod.r2.decimal_npts", 
"com.comcast.ngod.c1","com.comcast.ngod.c1.decimal_npts", NULL};

RequestHandler::RequestHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq)
	: _ssmNGODr2c1(ssm), _pSite(pSite), _pRequest(pReq), 
	_pResponse(NULL),  mStreamPrx(NULL),
	 _postResponse(true), 
	 _pContextPrx(NULL), _canProcess(true), _requireProtocol(NgodVerCode_UNKNOWN)
{
	timeStarted = ZQTianShan::now();
	_returnType = RETURN_SYNC; //set default to sync
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

bool RequestHandler::handshake(int& requireCode, int from , int to, bool bRequired)
{
	requireCode = NgodVerCode_UNKNOWN;
	if (_ngodConfig._protocolVersioning.enableVersioning > 0)
	{
		std::string strSupportedProtocols = NgodVerStrs[from];
		for (int i = from + 1; i < to; i++)
		{
			strSupportedProtocols += ",";
			strSupportedProtocols += NgodVerStrs[i];
		}
		//_pResponse->setHeader(NGOD_HEADER_SUPPORTED, strSupportedProtocols.c_str());
		std::string strRequire = getRequestHeader(NGOD_HEADER_REQUIRE);
		if (!strRequire.empty())
		{
			std::vector<std::string> requires;
			ZQ::common::stringHelper::SplitString(strRequire, requires);
			std::vector<std::string>::iterator strIter = requires.begin();
			int i;
			for (; strIter != requires.end(); strIter++)
			{
				for (i = from; i < to; i++)
				{
					if (*strIter == NgodVerStrs[i])
					{
						if (requireCode == NgodVerCode_UNKNOWN || requireCode < i)
						{
							if (!(requireCode == NgodVerCode_R2_DecNpt && i == NgodVerCode_C1))
							{
								requireCode = i;
							}
						}
						break;
					}
				}
				if ( i >= to)
				{
					_pResponse->setHeader(NGOD_HEADER_UNSUPPORTED, strRequire.c_str());
					_pResponse->setHeader(NGOD_HEADER_SUPPORTED, strSupportedProtocols.c_str());
					responseError(RESPONSE_UNSUPPORTED_OPTIONS); // "551 Option Not Supported"
					HANDLERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RequestHandler, "handShake() : The require header[%s] is unsupported"), strRequire.c_str());
					return false;
				}
			}
			if (requireCode == NgodVerCode_C1 || requireCode == NgodVerCode_C1_DecNpt)
			{
				strSupportedProtocols = "com.comcast.ngod.c1,";
				strSupportedProtocols += "com.comcast.ngod.c1.decimal_npts";
				
			}
			else
			{
				strSupportedProtocols = "com.comcast.ngod.r2,";
				strSupportedProtocols += "com.comcast.ngod.r2.decimal_npts";
			}
			_pResponse->setHeader(NGOD_HEADER_SUPPORTED, strSupportedProtocols.c_str());
			return true;


			//int i = from;
			//for (; i < to; i++)
			//{
			//	if (strRequire == NgodVerStrs[i])
			//	{
			//		requireCode = i;
			//		HANDLERLOG(ZQ::common::Log::L_INFO, CLOGFMT(RequestHandler, "handShake() : Success to hand shake. Require[%s]"), strRequire.c_str());
			//		return true;
			//	}
			//}
			//_pResponse->setHeader(NGOD_HEADER_UNSUPPORTED, strRequire.c_str());
			//responseError(RESPONSE_UNSUPPORTED_OPTIONS); // "551 Option Not Supported"
			//HANDLERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RequestHandler, "handShake() : The require header[%s] is unsupported"), strRequire.c_str());
			//return false;
		}
		else
		{
			if (bRequired)
			{
				_pResponse->setHeader(NGOD_HEADER_SUPPORTED, strSupportedProtocols.c_str());
				_pResponse->setHeader(NGOD_HEADER_UNSUPPORTED, "Require header is empty");
				responseError(RESPONSE_UNSUPPORTED_OPTIONS); // "551 Option Not Supported"
				HANDLERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RequestHandler, "handShake() : The require header required is empty"));
				return false;
			}
			return true;
		}
	}
	else
	{
		HANDLERLOG(ZQ::common::Log::L_INFO, CLOGFMT(RequestHandler, "handShake() : Hand Shake is disabled"));
		return true;
	}
}

bool RequestHandler::getRemoteAddr(std::string &strClientIP, uint16 &port)
{
	IClientRequest::RemoteInfo localInfo;
	char localBuffer[256];
	localInfo.size = sizeof(localInfo);
	localInfo.ipaddr = localBuffer;
	localInfo.addrlen = sizeof(localBuffer);
	if(_pRequest->getRemoteInfo(localInfo))
	{
		strClientIP = localInfo.ipaddr;
		port = localInfo.port;
		return true;
	}
	else
	{
		strClientIP = "";
		port = 0;
		return false;
	}
}

bool RequestHandler::getServerAddr(std::string& strServerIP, uint16& port)
{
	IClientRequest::LocalInfo localInfo;
	char localBuffer[256];
	localInfo.size = sizeof(localInfo);
	localInfo.ipaddr = localBuffer;
	localInfo.addrlen = sizeof(localBuffer);
	if(_pRequest->getLocalInfo(localInfo))
	{
		strServerIP = localInfo.ipaddr;
		port = localInfo.port;
		return true;
	}
	else
	{
		strServerIP = "";
		port = 0;
		return false;
	}
}



std::string RequestHandler::getRequestType()
{
	return _method;
}

std::string RequestHandler::getSession()
{
	return _session;
}

std::string RequestHandler::getSequence()
{
	return _sequence;
}

IServerResponse* RequestHandler::getResponse( )
{
	_pResponse = _pRequest->getResponse();
	return _pResponse;
}

Ice::Long	RequestHandler::getStartTime( )
{
	return timeStarted;
}
RequestHandler::ReturnType	RequestHandler::getReturnType( )
{
	return _returnType;
}
void RequestHandler::setReturnType( ReturnType type )
{
	_returnType = type;
}
std::string RequestHandler::getRequestHeader(const char* pHeader, int logLevel)
{
	if (NULL == pHeader || 0 == strlen(pHeader))
		return "";

	memset(szBuf, 0, sizeof(szBuf));
	
	szBufLen = sizeof(szBuf) - 1;
	const char* pRet = NULL;
	pRet = _pRequest->getHeader(pHeader, szBuf, &szBufLen);

	std::string ret = (NULL != pRet) ? pRet : "";

	ZQ::StringOperation::trimAll(ret);

	HANDLERLOG(logLevel, HANDLERLOGFMT(RequestHandler, "get header [%s: %s]"), pHeader, ret.c_str());

	return ret;
}

void RequestHandler::getContentBody(int logLevel)
{
	std::string contentSize;
	contentSize = getRequestHeader(NGOD_HEADER_CONTENTLENGTH);
	
	unsigned char* pContentBody = new unsigned char[atoi(contentSize.c_str()) + 1];
	uint32 contentLen;

	memset(pContentBody, 0, atoi(contentSize.c_str()) + 1);
	contentLen = atoi(contentSize.c_str()) + 1;
	
	const char* pRet = NULL;
	pRet = _pRequest->getContent(pContentBody, &contentLen);

	_requestBody = (NULL != pRet) ? pRet : "";

	delete []pContentBody;

	HANDLERLOG(logLevel, HANDLERLOGFMT(RequestHandler, "get content body: [%s]"), _requestBody.c_str());
}


int RequestHandler::preprocess()
{
	memset(szBuf, 0, sizeof(szBuf));
	szBufLen = sizeof(szBuf) - 1;

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



	_sequence = getRequestHeader(NGOD_HEADER_SEQ);
	_inoutMap[MAP_KEY_SEQUENCE] = _sequence;

	_session = getRequestHeader(NGOD_HEADER_SESSION, ZQ::common::Log::L_INFO);
	_inoutMap[MAP_KEY_SESSION] = _session;

	_userAgent = getRequestHeader(NGOD_HEADER_USERAGENT);
	_inoutMap[MAP_KEY_USERAGENT] = _userAgent;

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
	_xreason = getRequestHeader(NGOD_HEADER_XREASON, ZQ::common::Log::L_INFO);
	_inoutMap[MAP_KEY_XREASON] = _xreason;

	return 0;// success
}
void RequestHandler::setResponseString( const char* responseString , const char* noticeString /*= NULL */)
{
	if( responseString && responseString[0] != NULL )
	{
		_strResponse = responseString;
	}
	else
	{
		_strResponse = "";
	}
	if( noticeString && noticeString[0] != NULL )
	{
		_strNotice = noticeString;
	}
	else
	{
		_strNotice = "";
	}
}

int RequestHandler::responseError(const char* statusCodeLine, const char* szNotice)
{
	if( _strResponse.empty() )
		setResponseString( statusCodeLine , szNotice );

	if (NULL == _pResponse)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "Response Object is null");
		HANDLERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RequestHandler, "%s"), szBuf);
		return 1;
	}
	if( _strResponse.empty() )
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR,CLOGFMT( RequestHandler ,"No response status line"));
	}

	#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
	/*HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "state(%s)"), _strResponse.c_str());
	_pResponse->printf_preheader( _strResponse.c_str() );*/
	HANDLEREVENTLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RequestHandler, "state(%s)"), _strResponse.c_str());
	_pResponse->printf_preheader( _strResponse.c_str() );
	if (_ssmNGODr2c1._globalSession != _session)
		_pResponse->setHeader(NGOD_HEADER_SESSION, _session.c_str());
	_pResponse->setHeader(NGOD_HEADER_MTHDCODE, _method.c_str());
	_pResponse->setHeader(NGOD_HEADER_SERVER, _ssmNGODr2c1._serverHeader.c_str());

	if (_strNotice.empty() )
	{
		//_pResponse->setHeader(NGOD_HEADER_NOTICE, szBuf);
		std::string notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR, 
			NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING);
		_pResponse->setHeader(NGOD_HEADER_NOTICE, notice_str.c_str());
		_strNotice = notice_str;
	}
	else 
		_pResponse->setHeader(NGOD_HEADER_NOTICE, _strNotice.c_str() ) ;

	if (true == _postResponse)
	{
		if ( _strNotice.empty() )
		{
// 			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "[ResponseError] %s"), szBuf);
// 			HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "[ResponseError] %s"), szBuf);
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
			HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		}

		else
		{
// 			HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "[ResponseError] %s"), _strNotice.c_str() );
// 		    HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "[ResponseError] %s"), _strNotice.c_str() );
			HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _strNotice.c_str() );
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _strNotice.c_str() );
		}
		_pResponse->post();
	}
	
	return 0;
}

int RequestHandler::responseOK(  )
{	
	if (NULL == _pResponse)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "Response Object is null");
		HANDLERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RequestHandler, "%s"), szBuf);
		return 1;
	}
// 	if( _strResponse.empty() )
// 	{
// 		HANDLERLOG(ZQ::common::Log::L_ERROR,CLOGFMT( RequestHandler ,"No response status line"));
// 	}

	#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
	/*HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "state(%s)"), RESPONSE_OK);
	_pResponse->printf_preheader(RESPONSE_OK);*/
	HANDLEREVENTLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RequestHandler, "state(%s)"), RESPONSE_OK);
	_pResponse->printf_preheader(RESPONSE_OK);
	if (_ssmNGODr2c1._globalSession != _session)
		_pResponse->setHeader(NGOD_HEADER_SESSION, _session.c_str());
	_pResponse->setHeader(NGOD_HEADER_MTHDCODE, _method.c_str());
	_pResponse->setHeader(NGOD_HEADER_SERVER, _ssmNGODr2c1._serverHeader.c_str());

	if (true == _postResponse)
	{
		#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "to response OK"));
		_pResponse->post();
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RequestHandler, "response OK has been sent out"));
	}
	
	return 0;
}

bool RequestHandler::getContext()
{
	if ( NULL != _pContextPrx)
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "context and context proxy already gained"));
		return true;
	}

	bool bRet = _ssmNGODr2c1.openContext(_session, _context, _pContextPrx, _inoutMap);

	if (false == bRet)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "%s", _inoutMap[MAP_KEY_LASTERROR].c_str());
		setResponseString( RESPONSE_SESSION_NOTFOUND ,szBuf );
	}

	return bRet;
}

bool RequestHandler::getContextByIdentity(::Ice::Identity& ident)
{
	if ( NULL != _pContextPrx)
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "context and context proxy already gained"));
		return true;
	}

	bool bRet = _ssmNGODr2c1.openContext(ident.name, _context, _pContextPrx, _inoutMap);

	if (false == bRet)
		snprintf(szBuf, sizeof(szBuf) - 1, "%s", _inoutMap[MAP_KEY_LASTERROR].c_str());

	return bRet;
}

bool RequestHandler::getContextByIdentity(NGODr2c1::ctxData& context
						, NGODr2c1::ContextPrx& pContextPrx, ::Ice::Identity& ident)
{
	bool bRet = _ssmNGODr2c1.openContext(ident.name, context, pContextPrx, _inoutMap);

	if (false == bRet)
		snprintf(szBuf, sizeof(szBuf) - 1, "%s", _inoutMap[MAP_KEY_LASTERROR].c_str());

	return bRet;
}

bool RequestHandler::getStream()
{
	if (NULL != mStreamPrx)
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "stream proxy already gained"));
		return true;
	}
	HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "to gain stream: [%s]")
			, _context.streamFullID.c_str());
	try
	{
		mStreamPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(_pContextPrx->getStream());
	}
	catch (Ice::Exception& ex)	
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform uncheckedCast to stream: [%s] caught [%s]"
			,_context.streamFullID.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);		
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);	
	}	
	if (NULL == mStreamPrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "result stream proxy: [%s] is null"
			,_context.streamFullID.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
	}
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RequestHandler, "stream: [%s] gained successfully")
			, _context.streamFullID.c_str());
		
	return (NULL != mStreamPrx);
}



void RequestHandler::changePostFlag(bool bFlag)
{
	_postResponse = bFlag;
}



bool RequestHandler::getStreamState(TianShanIce::Streamer::StreamState& streamState, std::string& stateDept)
{
	_inoutMap[MAP_KEY_STREAMFULLID] = _context.streamFullID;
	_inoutMap[MAP_KEY_SOPNAME]		= _context.sopname;
	_inoutMap[MAP_KEY_STREMAERNETID]= _context.streamNetId;

	bool bRet = _ssmNGODr2c1.getStreamState(streamState, mStreamPrx, _inoutMap);

	if (true == bRet)
		stateDept = _inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION];
	else 
		snprintf(szBuf, sizeof(szBuf) - 1, "%s", _inoutMap[MAP_KEY_LASTERROR].c_str());

	return bRet;
}

bool RequestHandler::getPositionAndScale(std::string& range, std::string& scale)
{
	_inoutMap[MAP_KEY_STREAMFULLID] = _context.streamFullID;

	bool bRet = _ssmNGODr2c1.getPositionAndScale(mStreamPrx, _inoutMap, _requireProtocol);

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
	bool bRet = _ssmNGODr2c1.addContext(_context, _inoutMap);

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
bool RequestHandler::updateContextProp(const std::string& key, const std::string& val)
{
	bool bRet = true;
	if ( NULL == _pContextPrx)
	{
		bRet = getContext();
	}

	if (bRet)
		_pContextPrx->updateCtxProp(key, val);
	else
		return false;

	return bRet;
}
bool RequestHandler::updateContextProp(::Ice::Identity& ident, const std::string& key, const std::string& val)
{
	bool bRet = true;
	if ( NULL == _pContextPrx)
	{
		bRet = getContextByIdentity(ident);
	}

	if (bRet)
		_pContextPrx->updateCtxProp(key, val);
	else
		return false;

	return bRet;
}
void RequestHandler::destroyStream()
{
#pragma message(__MSGLOC__"TODO:must free the used bandwidth")
	if ( !mStreamPrx  ) 
	{
		try
		{
			mStreamPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(_ssmNGODr2c1._pCommunicator->stringToProxy(_context.streamFullID));
		}		
		catch (...) 
		{
			mStreamPrx = NULL;
		}
	}
	if (mStreamPrx) 
	{
		try
		{
			mStreamPrx->destroy();
			mStreamPrx = NULL;
		}
		catch (...) 
		{
		}
	
	}
}

bool RequestHandler::renewSession(Ice::Long ttl)
{
	if (ttl == -1)
		ttl = (Ice::Long) (_ngodConfig._rtspSession._timeout) * 1000;

	// DO: renew client session
	try
	{
		_pContextPrx->renew(ttl);
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "ClientSession(%s) renewed(%lldms)")
			, _context.ident.name.c_str(), ttl);
	}
	catch( const Ice::ObjectNotExistException& ex )
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "renewClientSession(%s) caught(%s)"
			, _context.ident.name.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		setResponseString( RESPONSE_SESSION_NOTFOUND,szBuf);
	}
	catch (const Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "renewClientSession(%s) caught(%s)"
			, _context.ident.name.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		setResponseString( RESPONSE_INTERNAL_ERROR,szBuf);
		return false;
	}
		
	_ssmNGODr2c1._pSessionWatchDog->watchSession(_context.ident, (long) ttl);

	// DO: renew weiwoo session in a background thread
//	pRenewCmd->start();

	return true;
}

bool RequestHandler::checkStreamer( )
{
	return _ssmNGODr2c1.mResManager.isStreamerReplicaAvail( _context.streamNetId );
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

