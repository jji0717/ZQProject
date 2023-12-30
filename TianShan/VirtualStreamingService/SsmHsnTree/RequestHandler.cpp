#include "RequestHandler.h"

namespace HSNTree
{

	//////////////////////////////////////////////////////////////////////////
	// class RequestHandler
	//////////////////////////////////////////////////////////////////////////

	RequestHandler::RequestHandler(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		: _env(env)
		, _pSite(pSite)
		, _pRequest(pReq)
		, _pResponse(pResponse)
		, _bNeedModifyResponse(true)
		, _bProcessSuccessfully(true)
		, _usedTime(0)
	{
		_startTime = ZQTianShan::now();
		_returnType = RETURN_SYNC;
		_usedTime = ZQTianShan::now();
		_ok_statusline = ResponseOK;
		_error_statusline= ResponseInternalError;
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
		glog(NoticeLevel, HandlerFmt(RequestHandler, "request processed (%lldms, %s)"), _usedTime, rstStr.c_str());
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
			glog(logLevel, HandlerFmt(RequestHandler, "getRequestHeader(%s: %s)"), pHeaderStr, retHeader.c_str());
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
		glog(DebugLevel, HandlerFmt(RequestHandler, "getContentBody(), (%s)"), _reqContent.c_str());
	}

	std::string RequestHandler::getUrl()
	{
		_szBuf[sizeof(_szBuf) - 1] = '\0';
		const char* pUrl = _pRequest->getUri(_szBuf, sizeof(_szBuf) - 1);
		std::string strUrl = "rtsp://";
		strUrl += NULL != pUrl ? pUrl : "";
		return strUrl;
	}

	void RequestHandler::composeErrorResponse(const char* errorInfo)
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


	//////////////////////////////////////////////////////////////////////////
	// class FixupRequest
	//////////////////////////////////////////////////////////////////////////

	FixupRequest::FixupRequest(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: RequestHandler(env, pSite, pReq, pResponse)
	{
		_phase = "FixupRequest";
		_pRequestWriter = dynamic_cast<IClientRequestWriter*>(_pRequest);		
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
		_pRequestWriter = dynamic_cast<IClientRequestWriter*>(_pRequest);		
	}

	ContentHandler::~ContentHandler()
	{
	}

	bool ContentHandler::renewSession()
	{
		__int64 rn_value = (__int64) _tsConfig._rtspSession._timeout * 1000 + 60000;
		try
		{
			_srvrSessPrx->renew(rn_value);
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0300] caught in session(%s).renew", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
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
			_srvrSessPrx = TianShanIce::SRM::SessionPrx::uncheckedCast(_env._pCommunicator->stringToProxy(_cltSessCtx.srvrSessPrxID));
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0400] caught in getWeiwooPrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessPrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		return true;
	}
	
	bool ContentHandler::getStreamPrx()
	{
		if (NULL != _streamPrx)
			return true;
		try
		{
			_streamPrx = TianShanIce::Streamer::StreamPrx::uncheckedCast(_env._pCommunicator->stringToProxy(_cltSessCtx.streamPrxID));
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0500] caught in getStreamPrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
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
			_purchasePrx = TianShanIce::Application::PurchasePrx::uncheckedCast(_env._pCommunicator->stringToProxy(_cltSessCtx.purchasePrxID));
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0600] caught in getPurchasePrx(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
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
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		catch(const ::Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0700] caught in getStreamState(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
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
			TianShanIce::Streamer::PlaylistPrx playlist = TianShanIce::Streamer::PlaylistPrx::checkedCast(_streamPrx);
			bInfoSucc = playlist->getInfo(TianShanIce::Streamer::infoSTREAMNPTPOS, vMap); // 这里必须是infoStreamNptPos，因为我们要取的值是相对与整个playlist头部的offset
		}
		catch(const ::Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0800] caught in stream(%s).getInfo()", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		if (!bInfoSucc)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "stream(%s).getInfo failed[ContentHandler:0801]", 
				_cltSessCtx.streamPrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		if (vMap.end() != vMap.find("scale") && vMap["scale"].type == TianShanIce::vtStrings && vMap["scale"].strs.size() > 0)
			scale = vMap["scale"].strs[0];
		if (vMap.end() != vMap.find("playposition") && vMap["playposition"].type == TianShanIce::vtInts && vMap["playposition"].ints.size() > 0)
			iCurrentPos = vMap["playposition"].ints[0];
		if (vMap.end() != vMap.find("totalplaytime") && vMap["totalplaytime"].type == TianShanIce::vtInts && vMap["totalplaytime"].ints.size() > 0)
			iTotalPos = vMap["totalplaytime"].ints[0];		
		glog(InfoLevel, HandlerFmt(ContentHandler, "scale [%s], npt [%d.%03d-%d.%03d]"), scale.c_str(), iCurrentPos / 1000, iCurrentPos % 1000, iTotalPos / 1000, iTotalPos % 1000);
		return true;
	}

	bool ContentHandler::getPlaylistPlayInfo(std::string& scale, Ice::Int& ctrlNum, Ice::Int& offset)
	{
		TianShanIce::ValueMap vMap;
		bool bInfoSucc = false;
		try
		{
			TianShanIce::Streamer::PlaylistPrx playlist = TianShanIce::Streamer::PlaylistPrx::checkedCast(_streamPrx);
			bInfoSucc = playlist->getInfo(TianShanIce::Streamer::infoPLAYPOSITION, vMap);
		}
		catch(const ::Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:0900] caught in stream(%s).getInfo()", 
				ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		if (!bInfoSucc)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "stream(%s).getInfo() failed[ContentHandler:0901]", 
				_cltSessCtx.streamPrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		if (vMap.end() != vMap.find("scale") && vMap["scale"].type == TianShanIce::vtStrings && vMap["scale"].strs.size() > 0)
			scale = vMap["scale"].strs[0];
		if (vMap.end() != vMap.find("ctrlnumber") && vMap["ctrlnumber"].type == TianShanIce::vtInts && vMap["ctrlnumber"].ints.size() > 0)
			ctrlNum = vMap["ctrlnumber"].ints[0];
		if (vMap.end() != vMap.find("playposition") && vMap["playposition"].type == TianShanIce::vtInts && vMap["playposition"].ints.size() > 0)
			offset = vMap["playposition"].ints[0];		
		glog(InfoLevel, HandlerFmt(ContentHandler, "scale [%s], ctrlnum [%d], offset[%d]"), scale.c_str(), ctrlNum, offset);
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
		catch (const ::TianShanIce::InvalidParameter& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught in UtcTime2PlayInfo(%s)"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_error_statusline = ResponseParameterNotUnderstood;
			return false;
		}
		catch (const ::TianShanIce::NotSupported& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught in UtcTime2PlayInfo(%s)"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_error_statusline = ResponseParameterNotUnderstood;
			return false;
		}
		catch(const ::TianShanIce::ServerError& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught in UtcTime2PlayInfo(%s)"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		catch(const ::Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:1000] caught in UtcTime2PlayInfo(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		if (outMap.end() != outMap.find("UserCtrlNum") && TianShanIce::vtInts == outMap["UserCtrlNum"].type && outMap["UserCtrlNum"].ints.size() > 0)
			ctrlNum = outMap["UserCtrlNum"].ints[0];
		if (outMap.end() != outMap.find("Offset") && TianShanIce::vtInts == outMap["Offset"].type && outMap["Offset"].ints.size() > 0)
			offset = outMap["Offset"].ints[0];
		if (outMap.end() != outMap.find("StartPos") && TianShanIce::vtInts == outMap["StartPos"].type && outMap["StartPos"].ints.size() > 0)
			startPos = outMap["StartPos"].ints[0];

		glog(InfoLevel, HandlerFmt(ContentHandler, "utctime [%s], ctrlnum [%d], offset [%d], startpos [%d]"), 
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
		catch (const ::TianShanIce::InvalidParameter& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught in PlayInfo2UtcTime(%s)"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_error_statusline = ResponseParameterNotUnderstood;
			return false;
		}
		catch(const ::TianShanIce::NotSupported& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught in PlayInfo2UtcTime(%s)"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		catch(const ::TianShanIce::ServerError& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught in PlayInfo2UtcTime(%s)"
				, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		catch(const ::Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[ContentHandler:1100] caught in PlayInfo2UtcTime(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
			glog(ErrorLevel, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}

		if (outMap.end() != outMap.find("BcastPos") && TianShanIce::vtStrings == outMap["BcastPos"].type && outMap["BcastPos"].strs.size() > 0)
			utcTime = outMap["BcastPos"].strs[0];

		glog(InfoLevel, HandlerFmt(ContentHandler, "utctime [%s], ctrlnum [%d], offset [%d]"), utcTime.c_str(), ctrlNum, offset);

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
		glog(DebugLevel, HandlerFmt(ContentHandler, "open session context..."));
		try
		{
			_cltSessPrx = SessionContextPrx::checkedCast(_env._pAdapter->createProxy(ident));
			_cltSessCtx = _cltSessPrx->getSessionData();
		}
		catch (Ice::ObjectNotExistException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1200] caught in openSessionCtx()", ex.ice_name().c_str());
			glog(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			_error_statusline = ResponseSessionNotFound;
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1201] caught in openSessionCtx()", ex.ice_name().c_str());
			glog(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		glog(InfoLevel, HandlerFmt(ContentHandler, "session context opened"));
		return true;
	}
	
	bool ContentHandler::saveSessionCtx(SessionContextImplPtr pSessionContext)
	{	
		glog(DebugLevel, HandlerFmt(ContentHandler, "save session context..."));
		try
		{
			ZQ::common::MutexGuard lk(_env._lockEvictor);
			_env._pContextEvictor->add(pSessionContext, pSessionContext->ident);
		}
		catch (Freeze::DatabaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1300] %s caught in saveSessionCtx()", 
				ex.ice_name().c_str(), ex.message.c_str());
			glog(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1301] caught in saveSessionCtx()", ex.ice_name().c_str());
			glog(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}	
		glog(InfoLevel, HandlerFmt(ContentHandler, "session context saved"));
		return true;
	}
	
	bool ContentHandler::removeSessionCtx(const Ice::Identity& ident)
	{	
		glog(DebugLevel, HandlerFmt(ContentHandler, "remove session context..."));
		try
		{
			ZQ::common::MutexGuard lk(_env._lockEvictor);
			_env._pContextEvictor->remove(ident);
		}
		catch (Freeze::DatabaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1400] %s caught in removeSessionCtx()", 
				ex.ice_name().c_str(), ex.message.c_str());
			glog(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[RequestHandle:1401] caught in removeSessionCtx()", ex.ice_name().c_str());
			glog(ZQ::common::Log::L_ERROR, HandlerFmt(ContentHandler, "%s"), _szBuf);
			return false;
		}
		glog(InfoLevel, HandlerFmt(ContentHandler, "session context removed"));
		return true;
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

} // end namespace HSNTree

