#include "TeardownRequest.h"

namespace HSNTree
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
			composeErrorResponse();
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
			glog(InfoLevel, HandlerFmt(HandleTeardown, "rtspproxy session destroied"));

		// DO: destroy weiwoo session
		try
		{
			glog(DebugLevel, HandlerFmt(HandleTeardown, "destroy weiwoo session..."));
			Ice::Context iceCtx;
			iceCtx["caller"] = getRequestHeader(HeaderXReason);
			iceCtx["caller_type"] = "client_request_destroy";
			getWeiwooPrx();
			// throw TianShanIce::ServerError
			// according to TsSRM.ICE, version: 34
			_srvrSessPrx->destroy(iceCtx);
			glog(InfoLevel, HandlerFmt(HandleTeardown, "weiwoo session destroied"));
		}
		catch (TianShanIce::ServerError& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[TeardownHandle:0300] %s caught by weiwoo session(%s).destroy", 
				ex.ice_name().c_str(), ex.message.c_str(), _cltSessCtx.srvrSessPrxID.c_str());
			glog(ErrorLevel, HandlerFmt(HandleTeardown, "%s"), _szBuf);
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[TeardownHandle:0301] caught by weiwoo session(%s).destroy", 
				ex.ice_name().c_str(), _cltSessCtx.srvrSessPrxID.c_str());
			glog(ErrorLevel, HandlerFmt(HandleTeardown, "%s"), _szBuf);
		}

		try
		{
			_cltSessPrx->sessionTeardown();
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[TeardownHandle:0302] caught by sessionTeardown(%s)", 
				ex.ice_name().c_str(), _cltSessCtx.ident.name.c_str());
			glog(ErrorLevel, HandlerFmt(HandleTeardown, "%s"), _szBuf);
		}
		catch (...)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "unknown exception caught by sessionTeardown(%s)", _cltSessCtx.ident.name.c_str());
			glog(ErrorLevel, HandlerFmt(HandleTeardown, "%s"), _szBuf);
		}

		// DO: remove session context
		removeSessionCtx(_cltSessCtx.ident);

		_env._pWatchDog->unwatch(_cltSessCtx.ident.name);

		composeRightResponse();

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
		
		return true;
	}
	
} // namespace HSNTree

