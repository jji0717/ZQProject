#include <TianShanIceHelper.h>
#include "./RequestHandler.h"

extern ZQ::common::Log* s1log;

namespace TianShanS1
{

	//////////////////////////////////////////////////////////////////////////
	// class RequestHandler
	//////////////////////////////////////////////////////////////////////////

	RequestHandler::RequestHandler(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		: _env(env)
		, _pSite(pSite)
		, _pResponse(pResponse)
		, _pRequest(pReq)
		, _bNeedModifyResponse(true)
		, _bProcessSuccessfully(true)
		, _usedTime(0)
	{
		_startTime = ZQTianShan::now();
		_returnType = RETURN_SYNC;
		_usedTime = ZQTianShan::now();
		/*_ok_statusline = ResponseOK;
		_error_statusline= ResponseInternalError;*/
		_szBuf[sizeof(_szBuf) - 1] = '\0';
		snprintf(_szBuf, sizeof(_szBuf) - 1, "%p", _pRequest);
		_reqIdent = _szBuf;		
		_session = getRequestHeader(HeaderSession);		
		_sequence = getRequestHeader(HeaderSequence);		
		switch (_pRequest->getVerb())
		{
		case RTSP_MTHD_SET_PARAMETER: _method="SET_PARAMETER"; break;
		case RTSP_MTHD_SETUP: _method="SETUP"; break;
		case RTSP_MTHD_PLAY: _method="PLAY"; break;
		case RTSP_MTHD_PAUSE: _method="PAUSE"; break;
		case RTSP_MTHD_TEARDOWN: _method="TEARDOWN"; break;
		case RTSP_MTHD_GET_PARAMETER: _method="GET_PARAMETER"; break;
		case RTSP_MTHD_OPTIONS: _method="OPTIONS"; break;
		case RTSP_MTHD_PING: _method="PING"; break;
		case RTSP_MTHD_DESCRIBE: _method="DESCRIBE"; break;
		default: break;
		}
		if (true == _env._serverHeader.empty())
			_env._serverHeader = getRequestHeader(HeaderServer) + "; " + ZQ_PRODUCT_NAME_SHORT;
	}

	RequestHandler::~RequestHandler()
	{
		_usedTime = ZQTianShan::now() - _usedTime;
		std::string rstStr;
		if (true == _bProcessSuccessfully)
			rstStr = "success";
		else 
			rstStr = "fail";
		SSMLOG(NoticeLevel, HandlerFmt(RequestHandler, "request processed (%lldms, %s)"), _usedTime, rstStr.c_str());
	}

	Ice::Long RequestHandler::getStartTime( ) const
	{
		return _startTime;
	}

	RequestHandler::ReturnType RequestHandler::getReturnType() const
	{
		return _returnType;
	}
	
	std::string RequestHandler::dumpTSStreamInfo( const ::TianShanIce::Streamer::StreamInfo& info ) const
	{
		std::string strRet = "StreamInfo:";
		TianShanIce::Properties::const_iterator it = info.props.begin();
		
		for( ; it != info.props.end() ; it ++ )
		{
			strRet = strRet + "   [" + it->first + "]:";
			strRet = strRet + "[" + it->second + "]";
		}
		return strRet;
	}

	void RequestHandler::setReturnType( ReturnType type )
	{
		_returnType = type;
	}

	std::string RequestHandler::getRequestHeader(const char* pHeaderStr, int logLevel)
	{
		uint16 szBufLen = sizeof(_szBuf) - 1;
		const char* pRetStr = NULL;
		pRetStr = _pRequest->getHeader(pHeaderStr, _szBuf, &szBufLen);
		std::string retHeader;
		retHeader = (NULL != pRetStr) ? pRetStr : "";
		if (logLevel != -1)
			SSMLOG(logLevel, HandlerFmt(RequestHandler, "getRequestHeader(%s: %s)"), pHeaderStr, retHeader.c_str());
		return retHeader;
	}

	std::string RequestHandler::getResponseHeader(const char* pHeaderStr)
	{
		uint16 szBufLen = sizeof(_szBuf) - 1;
		const char* pRetStr = NULL;
		memset(_szBuf, 0, sizeof(_szBuf));
		pRetStr = _pResponse->getHeader(pHeaderStr, _szBuf, &szBufLen);
		std::string retHeader;
		retHeader = (NULL != pRetStr) ? pRetStr : "";
		return retHeader;
	}
	
	void RequestHandler::setResponseHeader(const char* pHeaderStr, const char* strValue)
	{
		if (NULL == strValue)
			return;
		std::string tmpStr = String::nLeftStr(strValue, _tsConfig._response._maxFieldLen);
		::std::string::size_type getLen = tmpStr.size();
		if (_tsConfig._response._maxFieldLen == (int) getLen)
		{
			tmpStr[getLen - 3] = '.';
			tmpStr[getLen - 2] = '.';
			tmpStr[getLen - 1] = '.';
		}
		_pResponse->setHeader(pHeaderStr, tmpStr.c_str());
	}

	void RequestHandler::getContentBody()
	{
		_reqContent = "";
		uint32 cntLen = atoi(getRequestHeader(HeaderContentLength).c_str());
		if (0 == cntLen)
			return;
		cntLen ++;
		unsigned char* cntBuff = new unsigned char[cntLen];
		memset(cntBuff, 0, cntLen);
		const char* pRetStr = _pRequest->getContent(cntBuff, &cntLen);
		_reqContent = (NULL != pRetStr) ? pRetStr : "";
		delete []cntBuff;
		SSMLOG(DebugLevel, HandlerFmt(RequestHandler, "getContentBody(), (%s)"), _reqContent.c_str());
	}

	std::string RequestHandler::getUrl()
	{
		_szBuf[sizeof(_szBuf) - 1] = '\0';
		const char* pUrl = _pRequest->getUri(_szBuf, sizeof(_szBuf) - 1);
		std::string strUrl = "rtsp://";
		strUrl += NULL != pUrl ? pUrl : "";
		return strUrl;
	}

/*	void RequestHandler::composeErrorResponse(const char* errorInfo)
	{
		_pResponse->printf_preheader(_error_statusline.c_str());
		if (!_session.empty())
			_pResponse->setHeader(HeaderSession, _session.c_str());
		_pResponse->setHeader(HeaderMethodCode, _method.c_str());
		_pResponse->setHeader(HeaderServer, _env._serverHeader.c_str());
		const char* resError = NULL;
		if (NULL == errorInfo || 0 == strlen(errorInfo))
			setResponseHeader(HeaderTianShanNotice, _szBuf);
		else 
			setResponseHeader(HeaderTianShanNotice, errorInfo);
		_bProcessSuccessfully = false;
	}

	void RequestHandler::composeRightResponse()
	{
		_pResponse->printf_preheader(_ok_statusline.c_str());
		if (!_session.empty())
			_pResponse->setHeader(HeaderSession, _session.c_str());
		_pResponse->setHeader(HeaderMethodCode, _method.c_str());
		_pResponse->setHeader(HeaderServer, _env._serverHeader.c_str());
	}

*/
	void RequestHandler::composeResponse(int statusCode, const char* errorInfo)
	{
		std::string statusMsg; 
		switch (statusCode)
		{
		// "RTSP/1.0 200 OK"
		case 200: statusMsg = "OK"; break;

		// "RTSP/1.0 400 Bad Request"
		case 400: statusMsg = "Bad Request"; break;

        // "RTSP/1.0 401 Unauthorized"
		case 401: statusMsg = "Unauthorized"; break;

		// "RTSP/1.0 403 Forbidden"
		case 403: statusMsg = "Forbidden"; break;

        // "RTSP/1.0 404 Not Found"
		case 404: statusMsg = "Not Found"; break; 

		// "RTSP/1.0 405 Method Not Allowed"
		case 405: statusMsg = "Method Not Allowed"; break;

		// "RTSP/1.0 406 Not Acceptable"
		case 406: statusMsg = "Not Acceptable"; break; 

		// "RTSP/1.0 451 Parameter Not Understood"
		case 451: statusMsg = "Parameter Not Understood"; break; 

		// "RTSP/1.0 452 Conference Not Found"
		case 452: statusMsg = "Conference Not Found"; break;

		// "RTSP/1.0 453 Not Enough Bandwidth"
		case 453: statusMsg = "Not Enough Bandwidth"; break; 

		// "RTSP/1.0 454 Session Not Found"
		case 454: statusMsg = "Session Not Found"; break; 

		// "RTSP/1.0 455 Method Not Valid in This State"
		case 455: statusMsg = "Method Not Valid in This State"; break; 

		// "RTSP/1.0 457 Invalid Range"
		case 457: statusMsg = "Invalid Range"; break;

		// "RTSP/1.0 461 Unsupported Transport"
		case 461: statusMsg = "Unsupported Transport"; break; 


		// "RTSP/1.0 500 Internal Server Error"
		case 500: statusMsg = "Internal Server Error"; break; 
		
		// "RTSP/1.0 501 Not Implemented"
		case 501: statusMsg = "Not Implemented"; break; 

		// "RTSP/1.0 503 Service Unavailable"
		case 503: statusMsg = "Service Unavailable"; break; 


		// "RTSP/1.0 610 Unexpect Client Error"
		case 610: statusMsg = "Unexpect Client Error"; break; 

		// "RTSP/1.0 620 Unexpect Server Error" 
		case 620: statusMsg = "Unexpect Server Error"; break;
		
		// RTSP/1.0 676 Qam Name Not Found
		case 676: statusMsg = "Qam Name Not Found"; break;

#ifdef STATUS_CODE_WITH_NGOD_EXT
		case 770: statusMsg = "ServerSetupFailed No Response";					break;
		case 771: statusMsg = "ServerSetupFailed AssetNotFound";				break;
		case 772: statusMsg = "ServerSetupFailed SOPNotAvailable";				break;
		case 773: statusMsg = "ServerSetupFailed UnknownSOPGroup";				break;
		case 774: statusMsg = "ServerSetupFailed UnknownSOPNames";				break;
		case 775: statusMsg = "ServerSetupFailed InsufficientVolumeBandwidth";	break;
		case 776: statusMsg = "ServerSetupFailed InsufficientNetworkBandwidth";	break;
		case 777: statusMsg = "ServerSetupFailed InvalidRequest";				break;
#endif // STATUS_CODE_WITH_NGOD_EXT

		default:
			statusCode =500;
			statusMsg = "Internal Server Error";
			break;

		}
		char buf[128];
		snprintf(buf, sizeof(buf)-2, "RTSP/1.0 %d %s", statusCode, statusMsg.c_str());
		_pResponse->printf_preheader(buf);;

		if (!_session.empty())
			_pResponse->setHeader(HeaderSession, _session.c_str());
		_pResponse->setHeader(HeaderMethodCode, _method.c_str());
		_pResponse->setHeader(HeaderServer, _env._serverHeader.c_str());

		if (statusCode >= 200 && statusCode < 300)
		{
			return;
		}

		if (NULL == errorInfo || 0 == strlen(errorInfo))
			setResponseHeader(HeaderTianShanNotice, _szBuf);
		else 
			setResponseHeader(HeaderTianShanNotice, errorInfo);
		_bProcessSuccessfully = false;
	}

	//////////////////////////////////////////////////////////////////////////
	// class FixupRequest
	//////////////////////////////////////////////////////////////////////////

	FixupRequest::FixupRequest(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: RequestHandler(env, pSite, pReq, pResponse)
	{
		assert( _pRequest != 0);

		_phase = "FixupRequest";
		_pRequestWriter = pReq;
		assert( _pRequestWriter != 0 );
	}

	FixupRequest::~FixupRequest()
	{
	}

	
	//////////////////////////////////////////////////////////////////////////
	// class ContentHandler
	//////////////////////////////////////////////////////////////////////////	

	ContentHandler::ContentHandler(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: RequestHandler(env, pSite, pReq, pResponse)		
		, _cltSessPrx(NULL)
		, _srvrSessPrx(NULL)
		, _streamPrx(NULL)
		, _purchasePrx(NULL)
	{
		_phase = "ContentHandler";
		_pRequestWriter = pReq;		
	}

	ContentHandler::~ContentHandler()
	{
	}
	void ContentHandler::getLastScaleIndex( int& direction , int& index , int* lastDirection , int* lastIndex)
	{
		std::string strDirection	= getProperty( LAST_SCALE_STATUS_DIRECTION );
		std::string strIndex		= getProperty( LAST_SCALE_STATUS_INDEX );

		direction	= 1;
		index		= -1;
		if( !strDirection.empty() ) direction = atoi( strDirection.c_str() );
		if( !strIndex.empty())		index = atoi( strIndex.c_str() );
		if( lastDirection )
		{
			*lastDirection = 1;
			std::string	strLastDirection = getProperty( LAST_SCALE_STATUS_DIRECTION_BK );
			if( !strLastDirection.empty()) *lastDirection = atoi( strLastDirection.c_str() );
		}
		if( lastIndex)
		{
			*lastIndex =-1;
			std::string strLastIndex = getProperty( LAST_SCALE_STATUS_INDEX_BK );
			if( !strLastIndex.empty()) *lastIndex = atoi( strLastIndex.c_str() );
		}
	}

	void ContentHandler::updateLastScaleIndex( int direction , int index , int lastDirection , int lastIndex )
	{
		std::ostringstream oss;
		oss.str(""); oss << direction;
		updateProperty( LAST_SCALE_STATUS_DIRECTION , oss.str() );

		oss.str(""); oss << index;
		updateProperty( LAST_SCALE_STATUS_INDEX , oss.str() );

		oss.str(""); oss << lastDirection;
		updateProperty( LAST_SCALE_STATUS_DIRECTION_BK , oss.str() );

		oss.str("") ; oss << lastIndex;
		updateProperty( LAST_SCALE_STATUS_INDEX_BK , oss.str() );
	}
	bool ContentHandler::renewSession()
	{
		int64 rn_value = (int64) _tsConfig._rtspSession._timeout * 1000 + 60000;
		try
		{
			_srvrSessPrx->renew(rn_value);
		}
		catch( const Ice::ObjectNotExistException& ex)
		{
			_srvrSessPrx = NULL;			
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0300] caught in session(%s).renew", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 404;
			clearSessionResource( "" , "" , false );
			return false;
		}	
		catch( const Ice::SocketException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0300] caught in session(%s).renew", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(WarningLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			//_statusCode = 503;			
		}
		catch (const Ice::TimeoutException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0300] caught in session(%s).renew", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(WarningLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			//_statusCode = 503;			
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0300] caught in session(%s).renew", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			SSMLOG(WarningLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			//return false;
		}
		_env._pWatchDog->watch(_session, rn_value - 60000);
		return true;
	}

	bool ContentHandler::getWeiwooPrx()
	{
		if (NULL != _srvrSessPrx)
			return true;
		try
		{
			_srvrSessPrx = TianShanIce::SRM::SessionPrx::checkedCast(_env._pCommunicator->stringToProxy(_cltSessCtx.srvrSessPrxID));
		}
		catch( const Ice::ObjectNotExistException& ex)
		{
			_srvrSessPrx = NULL;
			clearSessionResource( "" , "" , false );
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0500] caught in getStreamPrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 404;
			return false;
		}
		catch( const Ice::SocketException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0400] caught in getWeiwooPrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessPrxID.c_str());
			SSMLOG(WarningLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			//_statusCode = 503;			
		}
		catch (const Ice::TimeoutException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0400] caught in getWeiwooPrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessPrxID.c_str());
			SSMLOG(WarningLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			//_statusCode = 503;			
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0400] caught in getWeiwooPrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessPrxID.c_str());
			SSMLOG(WarningLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			
		}
		return true;
	}
	
	bool ContentHandler::getStreamPrx()
	{
		if (NULL != _streamPrx)
			return true;
		try
		{
			_streamPrx = TianShanIce::Streamer::StreamPrx::checkedCast(_env._pCommunicator->stringToProxy(_cltSessCtx.streamPrxID));
		}
		catch( const Ice::ObjectNotExistException& ex)
		{
			clearSessionResource();
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0500] caught in getStreamPrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 404;
			return false;
		}
		catch( const Ice::SocketException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0500] caught in getStreamPrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 503;
			return false;
		}
		catch (const Ice::TimeoutException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0500] caught in getStreamPrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 503;
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0500] caught in getStreamPrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		return true;
	}

	bool ContentHandler::getPurchasePrx()
	{
		if (NULL != _purchasePrx)
			return true;
		try
		{
			_purchasePrx = TianShanIce::Application::PurchasePrx::checkedCast(_env._pCommunicator->stringToProxy(_cltSessCtx.purchasePrxID));
		}
		catch( const Ice::ObjectNotExistException& ex)
		{
			clearSessionResource();
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0500] caught in getStreamPrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 404;
			return false;
		}
		catch (const Ice::TimeoutException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0600] caught in getPurchasePrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 503;
			return false;
		}
		catch( const Ice::SocketException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0600] caught in getPurchasePrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 503;
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0600] caught in getPurchasePrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		return true;
	}

	bool ContentHandler::getStreamState(TianShanIce::Streamer::StreamState& strmState, std::string& statDescription)
	{
		try
		{
			// throw ServerError according to TsStreamer.ICE, version 42
			strmState = _streamPrx->getCurrentState();
		}
		catch(const ::TianShanIce::ServerError& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught in getStreamState(%s)"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		catch( const Ice::ObjectNotExistException& ex)
		{
			clearSessionResource();
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0700] caught in getStreamState(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 404;
			return false;
		}
		catch( const Ice::SocketException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0700] caught in getStreamState(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 503;
			return false;
		}
		catch (const Ice::TimeoutException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0700] caught in getStreamState(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 503;
			return false;
		}
		catch(const ::Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0700] caught in getStreamState(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		switch(strmState)
		{
		case TianShanIce::Streamer::stsSetup: statDescription = "init"; break;
		case TianShanIce::Streamer::stsStreaming: statDescription = "play"; break;
		case TianShanIce::Streamer::stsPause: statDescription = "pause"; break;
		case TianShanIce::Streamer::stsStop: statDescription = "stop"; break;
		default: statDescription = "unknown"; break;
		}
		return true;
	}
	
	bool ContentHandler::getStreamPlayInfo(std::string& scale, Ice::Int& iCurrentPos, Ice::Int& iTotalPos)
	{
		TianShanIce::ValueMap vMap;
		bool bInfoSucc = false;
		try
		{
			TianShanIce::Streamer::PlaylistPrx playlist = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(_streamPrx);
			bInfoSucc = playlist->getInfo(TianShanIce::Streamer::infoSTREAMNPTPOS, vMap); // 这里必须是infoStreamNptPos，因为我们要取的值是相对与整个playlist头部的offset
		}
		catch( const TianShanIce::ServerError& ex)
		{
			processServerErrorWithRtspErrCode(ex);
			return false;
		}
		catch( const Ice::ObjectNotExistException& ex)
		{
			clearSessionResource();
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0500] caught in getStreamPlayInfo(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 404;
			return false;
		}
		catch (const Ice::TimeoutException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0800] caught in stream(%s).getInfo()", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 503;
			return false;
		}
		catch( const Ice::SocketException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0800] caught in stream(%s).getInfo()", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 503;
			return false;
		}
		catch(const ::Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0800] caught in stream(%s).getInfo()", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		if (!bInfoSucc)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "stream(%s).getInfo failed[ContentHandler:0801]", 
				_cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		if (vMap.end() != vMap.find("scale") && vMap["scale"].type == TianShanIce::vtStrings && vMap["scale"].strs.size() > 0)
			scale = vMap["scale"].strs[0];
		if (vMap.end() != vMap.find("playposition") && vMap["playposition"].type == TianShanIce::vtInts && vMap["playposition"].ints.size() > 0)
			iCurrentPos = vMap["playposition"].ints[0];
		if (vMap.end() != vMap.find("totalplaytime") && vMap["totalplaytime"].type == TianShanIce::vtInts && vMap["totalplaytime"].ints.size() > 0)
			iTotalPos = vMap["totalplaytime"].ints[0];		
		SSMLOG(InfoLevel, HandlerFmt(ContentHandler, "scale [%s], npt [%d.%03d-%d.%03d]"), scale.c_str(), iCurrentPos / 1000, iCurrentPos % 1000, iTotalPos / 1000, iTotalPos % 1000);
		return true;
	}

	bool ContentHandler::processServerErrorWithRtspErrCode( const TianShanIce::ServerError& ex )
	{
		if (!ex.ice_name().empty()) // the default error code per exception should be 500 Internal Server Error
			_statusCode= 500;

		if( ex.category.compare("RtspProxying") == 0 )
		{
			switch ( ex.errorCode )
			{
			case 404:
			case 454:
				{
					clearSessionResource();
					_statusCode = 404;					
				}
				break;
			case 403:
				_statusCode = 455;
				break;

			case 408:
				{
					_statusCode = 503;
				}
				break;
			default:
				{
					_statusCode = ex.errorCode;					
					
				}
				break;
			}
//ssm_tianshan_s1.8.log:04-26 21:23:13.766 [   ERROR ] [ContentHandler/615     | 0000190C]  Req(19A8CC60)Sess(1787947579)Seq(32)Mtd(ContentHandler:PLAY) caught[doAction(PLAY) OnDemandSess[736d5f98-cae5-4f61-a3c0-e183913de9c9] timeout at RTSP messaging] while execute command on stream[NSS/736d5f98-cae5-4f61-a3c0-e183913de9c9 -t:tcp -h sss6_ss_cl -p 20800 -t 10000]
			snprintf(_szBuf, sizeof(_szBuf) - 1, "serverError: (%d) %s at strm[%s]", ex.errorCode, ex.message.c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
		}
		else
		{
			switch( ex.errorCode )
			{
			case	StrmCtrlErr_NoSession :
				{
					clearSessionResource();
					_statusCode = 404;
				
				}
				break;

			case StrmCtrlErr_Connection:				
			case StrmCtrlErr_AsynCall:				
			case StrmCtrlErr_Timeout:				
			case StrmCtrlErr_Response:
				_statusCode= 503;
			default:				
				break;
			}
//			snprintf(_szBuf, sizeof(_szBuf) - 1, "caught[%s] while execute command on stream[%s]",ex.message.c_str(), _cltSessCtx.streamPrxID.c_str());
			snprintf(_szBuf, sizeof(_szBuf) - 1, "serverError: (%d) %s at strm[%s]", ex.errorCode, ex.message.c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
		}
		return true;
	}

	bool ContentHandler::getPlaylistPlayInfo(std::string& scale, Ice::Int& ctrlNum, Ice::Int& offset)
	{
		TianShanIce::ValueMap vMap;
		bool bInfoSucc = false;
		try
		{
			TianShanIce::Streamer::PlaylistPrx playlist = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(_streamPrx);
			bInfoSucc = playlist->getInfo(TianShanIce::Streamer::infoPLAYPOSITION, vMap);
		}
		catch( const TianShanIce::ServerError& ex)
		{
			processServerErrorWithRtspErrCode(ex);
			return false;
		}
		catch( const Ice::ObjectNotExistException& ex)
		{
			clearSessionResource();
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0500] caught in getInfo(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 404;
			return false;
		}
		catch( const Ice::SocketException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0900] caught in stream(%s).getInfo()", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 503;
			return false;
		}
		catch( const Ice::TimeoutException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0900] caught in stream(%s).getInfo()", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 503;
			return false;
		}
		catch(const ::Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0900] caught in stream(%s).getInfo()", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		if (!bInfoSucc)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "stream(%s).getInfo() failed[ContentHandler:0901]", 
				_cltSessCtx.streamPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		if (vMap.end() != vMap.find("scale") && vMap["scale"].type == TianShanIce::vtStrings && vMap["scale"].strs.size() > 0)
			scale = vMap["scale"].strs[0];
		if (vMap.end() != vMap.find("ctrlnumber") && vMap["ctrlnumber"].type == TianShanIce::vtInts && vMap["ctrlnumber"].ints.size() > 0)
			ctrlNum = vMap["ctrlnumber"].ints[0];
		if (vMap.end() != vMap.find("playposition") && vMap["playposition"].type == TianShanIce::vtInts && vMap["playposition"].ints.size() > 0)
			offset = vMap["playposition"].ints[0];		
		SSMLOG(InfoLevel, HandlerFmt(ContentHandler, "scale [%s], ctrlnum [%d], offset[%d]"), scale.c_str(), ctrlNum, offset);
		return true;
	}
	
	bool ContentHandler::UtcTime2PlayInfo(const std::string& utcTime, ::Ice::Int& ctrlNum, ::Ice::Int& offset, ::Ice::Int& startPos)
	{
		STRINGVECTOR params;
		params.push_back("UserCtrlNum");
		params.push_back("Offset");
		params.push_back("StartPos");
		
		::TianShanIce::ValueMap inMap, outMap;
		TianShanIce::Variant vrtUtcTime;
		vrtUtcTime.bRange = false;
		vrtUtcTime.type = TianShanIce::vtStrings;
		vrtUtcTime.strs.clear();
		vrtUtcTime.strs.push_back(utcTime);
		inMap["Position.clock"] = vrtUtcTime;

		try
		{
			// throw InvalidParameter, NotSupported, ServerError;
			// according to TsApplication.ICE, version 17
			_purchasePrx->getParameters(params, inMap, outMap);
		}
		catch( const Ice::SocketException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s caught in UtcTime2PlayInfo(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			//_error_statusline = ResponseParameterNotUnderstood;
			_statusCode = 503;
			return false;
		}
		catch( const Ice::TimeoutException& ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s caught in UtcTime2PlayInfo(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			//_error_statusline = ResponseParameterNotUnderstood;
			_statusCode = 503;
			return false;
		}
		catch (const ::TianShanIce::InvalidParameter& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught in UtcTime2PlayInfo(%s)"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			//_error_statusline = ResponseParameterNotUnderstood;
			_statusCode = 451;
			return false;
		}
		catch (const ::TianShanIce::NotSupported& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught in UtcTime2PlayInfo(%s)"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			//_error_statusline = ResponseParameterNotUnderstood;
			_statusCode = 451;
			return false;
		}
		catch(const ::TianShanIce::ServerError& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught in UtcTime2PlayInfo(%s)"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		catch(const ::Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:1000] caught in UtcTime2PlayInfo(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		if (outMap.end() != outMap.find("UserCtrlNum") && TianShanIce::vtInts == outMap["UserCtrlNum"].type && outMap["UserCtrlNum"].ints.size() > 0)
			ctrlNum = outMap["UserCtrlNum"].ints[0];
		if (outMap.end() != outMap.find("Offset") && TianShanIce::vtInts == outMap["Offset"].type && outMap["Offset"].ints.size() > 0)
			offset = outMap["Offset"].ints[0];
		if (outMap.end() != outMap.find("StartPos") && TianShanIce::vtInts == outMap["StartPos"].type && outMap["StartPos"].ints.size() > 0)
			startPos = outMap["StartPos"].ints[0];

		SSMLOG(InfoLevel, HandlerFmt(ContentHandler, "utctime [%s], ctrlnum [%d], offset [%d], startpos [%d]"), 
			utcTime.c_str(), ctrlNum, offset, startPos);
		return true;
	}

	bool ContentHandler::PlayInfo2UtcTime(const ::Ice::Int& ctrlNum, const ::Ice::Int& offset, std::string& utcTime)
	{
		STRINGVECTOR params;
		params.push_back("BcastPos");
		::TianShanIce::ValueMap inMap, outMap;
		::TianShanIce::Variant vrtUserCtrlNum, vrtOffset;
		vrtUserCtrlNum.type = TianShanIce::vtInts;
		vrtUserCtrlNum.bRange = false;
		vrtUserCtrlNum.ints.clear();
		vrtUserCtrlNum.ints.push_back(ctrlNum);
		vrtOffset.type = TianShanIce::vtInts;
		vrtOffset.bRange = false;
		vrtOffset.ints.clear();
		vrtOffset.ints.push_back(offset);
		inMap["UserCtrlNum"] = vrtUserCtrlNum;
		inMap["Offset"] = vrtOffset;

		try
		{
			// throw InvalidParameter, NotSupported, ServerError;
			// according to TsApplication.ICE, version 17
			_purchasePrx->getParameters(params, inMap, outMap);
		}
		catch( const Ice::TimeoutException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s caught in PlayInfo2UtcTime(%s)"
				, ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			//_error_statusline = ResponseParameterNotUnderstood;
			_statusCode = 503;
			return false;
		}
		catch( const Ice::SocketException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s caught in PlayInfo2UtcTime(%s)"
				, ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			//_error_statusline = ResponseParameterNotUnderstood;
			_statusCode = 503;
			return false;
		}
		catch (const ::TianShanIce::InvalidParameter& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught in PlayInfo2UtcTime(%s)"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			//_error_statusline = ResponseParameterNotUnderstood;
			_statusCode = 451;
			return false;
		}
		catch(const ::TianShanIce::NotSupported& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught in PlayInfo2UtcTime(%s)"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		catch(const ::TianShanIce::ServerError& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught in PlayInfo2UtcTime(%s)"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		catch(const ::Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:1100] caught in PlayInfo2UtcTime(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}

		if (outMap.end() != outMap.find("BcastPos") && TianShanIce::vtStrings == outMap["BcastPos"].type && outMap["BcastPos"].strs.size() > 0)
			utcTime = outMap["BcastPos"].strs[0];

		SSMLOG(InfoLevel, HandlerFmt(ContentHandler, "utctime [%s], ctrlnum [%d], offset [%d]"), utcTime.c_str(), ctrlNum, offset);

		return true;
	}
	
	void ContentHandler::updateProperty( const std::string& key , const std::string& value )
	{
		_cltSessPrx->updateProperty(key,value);
	}

	std::string ContentHandler::getProperty( const std::string& key ) const
	{
		return _cltSessPrx->getProperty(key);
	}

	
	// session context manager
	bool ContentHandler::openSessionCtx(const Ice::Identity& ident)
	{
		SSMLOG(DebugLevel, HandlerFmt(ContentHandler, "open session context..."));
		try
		{
			_cltSessPrx = SessionContextPrx::checkedCast(_env._pAdapter->createProxy(ident));
			_cltSessCtx = _cltSessPrx->getSessionData();
		}
		catch (Ice::ObjectNotExistException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1200] caught in openSessionCtx()", ex.ice_name().c_str());
			SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			//_error_statusline = ResponseSessionNotFound;
			_statusCode = 454;
			return false;
		}
		catch( const Ice::SocketException &ex )
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1201] caught in openSessionCtx()", ex.ice_name().c_str());
			SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 503;
			return false;
		}
		catch (const Ice::TimeoutException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1201] caught in openSessionCtx()", ex.ice_name().c_str());
			SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_statusCode = 503;
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1201] caught in openSessionCtx()", ex.ice_name().c_str());
			SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		SSMLOG(InfoLevel, HandlerFmt(ContentHandler, "session context opened"));
		return true;
	}
	
	bool ContentHandler::saveSessionCtx(SessionContextImplPtr pSessionContext)
	{	
		SSMLOG(DebugLevel, HandlerFmt(ContentHandler, "save session context..."));
		try
		{
			ZQ::common::MutexGuard lk(_env._lockEvictor);
			_env._pContextEvictor->add(pSessionContext, pSessionContext->ident);
		}
		catch (Freeze::DatabaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1300] %s caught in saveSessionCtx()", 
				ex.ice_name().c_str(), ex.message.c_str());
			SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1301] caught in saveSessionCtx()", ex.ice_name().c_str());
			SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}	
		SSMLOG(InfoLevel, HandlerFmt(ContentHandler, "session context saved"));
		return true;
	}
	
	bool ContentHandler::removeSessionCtx(const Ice::Identity& ident)
	{	
		SSMLOG(DebugLevel, HandlerFmt(ContentHandler, "remove session context..."));
		try
		{
			ZQ::common::MutexGuard lk(_env._lockEvictor);
			_env._pContextEvictor->remove(ident);
		}
		catch (Freeze::DatabaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1400] %s caught in removeSessionCtx()", 
				ex.ice_name().c_str(), ex.message.c_str());
			SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1401] caught in removeSessionCtx()", ex.ice_name().c_str());
			SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		SSMLOG(InfoLevel, HandlerFmt(ContentHandler, "session context removed"));
		return true;
	}

	SessionContextImplPtr ContentHandler::convertSessionData(SessionData& sessionData)
	{
		SessionContextImplPtr pSessionContext = new SessionContextImpl(_env);
		pSessionContext->ident			=	sessionData.ident;
		pSessionContext->streamID		=	sessionData.streamID;
		pSessionContext->streamPrxID	=	sessionData.streamPrxID;
		pSessionContext->purchasePrxID	=	sessionData.purchasePrxID;
		pSessionContext->srvrSessID		=	sessionData.srvrSessID;
		pSessionContext->srvrSessPrxID	=	sessionData.srvrSessPrxID;
		pSessionContext->rangePrefix	=	sessionData.rangePrefix;
		pSessionContext->url			=	sessionData.url;
		pSessionContext->requestType	=	sessionData.requestType;
		pSessionContext->announceSeq	=	sessionData.announceSeq;
		ZQTianShan::Util::mergeProperty( pSessionContext->props, sessionData.props);

		return pSessionContext;
	}

	void ContentHandler::clearSessionResource( const std::string& reason, const std::string& type ,bool bClearWeiwooSess )
	{
		// DO: destroy rtsp client session
		if (_pSite->destroyClientSession(_session.c_str()))
			SSMLOG(InfoLevel, HandlerFmt(HandleTeardown, "rtspproxy session destroied"));

		// DO: destroy weiwoo session
		if( bClearWeiwooSess )
		{
			try
			{
				SSMLOG(DebugLevel, HandlerFmt(HandleTeardown, "destroy weiwoo session..."));
				Ice::Context iceCtx;
				iceCtx["caller"] = reason;
				iceCtx["caller_type"] = type;
				getWeiwooPrx();
				// throw TianShanIce::ServerError
				// according to TsSRM.ICE, version: 34
				_srvrSessPrx->destroy(iceCtx);
				SSMLOG(InfoLevel, HandlerFmt(HandleTeardown, "weiwoo session destroied"));
			}
			catch (TianShanIce::ServerError& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:0300] %s caught in weiwoo session(%s).destroy", 
					ex.ice_name().c_str(), ex.message.c_str(), _cltSessCtx.srvrSessPrxID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleTeardown, "%s"), _szBuf);
			}
			catch (Ice::Exception& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:0301] caught in weiwoo session(%s).destroy", 
					ex.ice_name().c_str(), _cltSessCtx.srvrSessPrxID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleTeardown, "%s"), _szBuf);
			}
		}

		// DO: remove session context
		removeSessionCtx(_cltSessCtx.ident);

		_env._pWatchDog->unwatch(_cltSessCtx.ident.name);

	}

	void ContentHandler::composeResponse(int statusCode, const char* errorInfo)
	{
		RequestHandler::composeResponse(statusCode, errorInfo);

		if (SPEC_NGOD_S1 == _cltSessCtx.requestType)	// NGOD spec
		{
			std::string clientSessID;
			ZQTianShan::Util::getPropertyDataWithDefault( _cltSessCtx.props , HeaderClientSessionId , "" , clientSessID);
			if (!clientSessID.empty())
			{
				_pResponse->setHeader(HeaderClientSessionId, clientSessID.c_str());
			}
			_pResponse->setHeader(HeaderOnDemandSessionId, _cltSessCtx.streamID.c_str());
		}
	}
	std::string ContentHandler::ParamLine2Str(const TianShanIce::Properties& eventParams) const
	{
		std::string line;
		for (TianShanIce::Properties::const_iterator it = eventParams.begin(); it != eventParams.end(); it++)
			line += it->first + "=" + it->second + "|";
		if (line.empty())
			return "";
		return line.substr(0, line.length() -1);
	}
	
	//////////////////////////////////////////////////////////////////////////
	// class FixupResponse
	//////////////////////////////////////////////////////////////////////////	

	FixupResponse::FixupResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		: RequestHandler(env, pSite, pReq, pResponse)
	{
		_phase = "FixupResponse";
	}

	FixupResponse::~FixupResponse()
	{
	}

} // end namespace TianShanS1

