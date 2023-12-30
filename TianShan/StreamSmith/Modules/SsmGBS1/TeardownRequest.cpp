#include "TeardownRequest.h"

extern ZQ::common::Log* s1log;

namespace TianShanS1
{
	FixupTeardown::FixupTeardown(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: FixupRequest(env, pSite, pReq, pResponse)
	{
	}

	FixupTeardown::~FixupTeardown()
	{
	}

	bool FixupTeardown::process()
	{
		std::string require = getRequestHeader(HeaderRequire);
		if(require.find("com.comcast.ngod.s1") != std::string::npos)// NGOD spec
		{
			if(getRequestHeader(HeaderSession).empty())
			{
				std::string urlStr;
				memset(_szBuf, 0, sizeof(_szBuf));
				urlStr = _pRequest->getUri(_szBuf, sizeof(_szBuf) - 1);
				if (!urlStr.empty() && urlStr != "*" && urlStr.find_last_of('/') != std::string::npos)	// sample : rtsp://10.50.19.61:554/716195834
				{
					std::string sessionId = urlStr.substr(urlStr.find_last_of('/')+1, urlStr.size());
					_pRequestWriter->setHeader(HeaderSession, (char *)sessionId.c_str());
				}
			}
			if(!getRequestHeader(HeaderReason).empty())
			{
				_pRequestWriter->setHeader(HeaderXReason, (char *)getRequestHeader(HeaderReason).c_str());
			}
		}
		return true;
	}

	HandleTeardown::HandleTeardown(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: ContentHandler(env, pSite, pReq, pResponse)
	{
	}

	HandleTeardown::~HandleTeardown()
	{
	}

	bool HandleTeardown::process()
	{
		Ice::Identity ident;
		ident.name = _session;
		ident.category = ServantType;
		if (false == openSessionCtx(ident))
		{
			composeResponse(_statusCode);
			//composeErrorResponse();
			return false;
		}

		if (1 == _cltSessCtx.requestType) // SeaChange
			_pRequestWriter->setHeader(HeaderFormatType, SeaChangeFormat);
		else 
		{
			_pRequestWriter->setHeader(HeaderFormatType, TianShanFormat);
			_bNeedModifyResponse = false;
		}

		// DO: destroy rtsp client session
		if (_pSite->destroyClientSession(_session.c_str()))
			SSMLOG(InfoLevel, HandlerFmt(HandleTeardown, "rtspproxy session destroyed"));

		// DO: destroy weiwoo session
		try
		{
			SSMLOG(DebugLevel, HandlerFmt(HandleTeardown, "destroy weiwoo session..."));
			Ice::Context iceCtx;
			iceCtx["caller"] = getRequestHeader(HeaderXReason);
			iceCtx["caller_type"] = "client_request_destroy";
			getWeiwooPrx();
			// throw TianShanIce::ServerError
			// according to TsSRM.ICE, version: 34
			_srvrSessPrx->destroy(iceCtx);
			SSMLOG(InfoLevel, HandlerFmt(HandleTeardown, "weiwoo session destroyed"));
			_statusCode = 200;
		}
		catch (TianShanIce::BaseException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[TeardownHandle:0300] %s caught by weiwoo session(%s).destroy", 
				ex.ice_name().c_str(), ex.message.c_str(), _cltSessCtx.srvrSessPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleTeardown, "%s"), _szBuf);
		}
		catch( const Ice::ObjectNotExistException& )
		{
			_statusCode = 200;			
		}
		catch( const Ice::SocketException& ex)
		{
			_statusCode = 503;
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[TeardownHandle:0301] %s caught by weiwoo session(%s).destroy", 
				ex.ice_name().c_str(), ex.ice_name().c_str(), _cltSessCtx.srvrSessPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleTeardown, "%s"), _szBuf);
		}
		catch( const Ice::TimeoutException& ex)
		{
			_statusCode = 503;
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[TeardownHandle:0302] %s caught by weiwoo session(%s).destroy", 
				ex.ice_name().c_str(), ex.ice_name().c_str(), _cltSessCtx.srvrSessPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleTeardown, "%s"), _szBuf);
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[TeardownHandle:0303] caught by weiwoo session(%s).destroy", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessPrxID.c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandleTeardown, "%s"), _szBuf);
		}

		// DO: remove session context
		removeSessionCtx(_cltSessCtx.ident);

		_env._pWatchDog->unwatch(_cltSessCtx.ident.name);

		
		composeResponse(_statusCode);

		return true;
	}

	TeardownResponse::TeardownResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		: FixupResponse(env, pSite, pReq, pResponse)
	{
	}

	TeardownResponse::~TeardownResponse()
	{
	}

	bool TeardownResponse::process()
	{
		if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), SeaChangeFormat))
		{
			_pResponse->setHeader(HeaderSeaChangeNotice, getResponseHeader(HeaderTianShanNotice).c_str());
			_pResponse->setHeader(HeaderTianShanNotice, ""); // remove "TianShan-Notice"
		}
		else if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), NGODFormat))
		{
			if(!getRequestHeader(HeaderOnDemandSessionId).empty())
			{
				_pResponse->setHeader(HeaderOnDemandSessionId, (char *)getRequestHeader(HeaderOnDemandSessionId).c_str());
			}

			if(!getRequestHeader(HeaderClientSessionId).empty())
			{
				_pResponse->setHeader(HeaderClientSessionId, (char *)getRequestHeader(HeaderClientSessionId).c_str());
			}
		}
		
		return true;
	}
	
} // namespace TianShanS1

