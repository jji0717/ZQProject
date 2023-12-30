#include "PlayRequest.h"
#include <TianShanIceHelper.h>

extern ZQ::common::Log* s1log;

namespace LiveChannel
{

#define HandlerFmtEx(_C, _X) CLOGFMT(_C, "Req(%s)Sess(%s)Seq(%s)Mtd(%s:%s) " _X), _pHandler->_reqIdent.c_str(), _pHandler->_session.c_str(), _pHandler->_sequence.c_str(), _pHandler->_phase.c_str(), _pHandler->_method.c_str()

	FixupPlay::FixupPlay(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: FixupRequest(env, pSite, pReq, pResponse)
	{
	}

	FixupPlay::~FixupPlay()
	{
	}

	bool FixupPlay::process()
	{
		std::string require = getRequestHeader(HeaderRequire);
		if(require.find("com.comcast.ngod.c1") != std::string::npos && getRequestHeader(HeaderSession).empty())// s1 spec
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
		return true;
	}

	HandlePlay::HandlePlay(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: ContentHandler(env, pSite, pReq, pResponse)
	{
	}

	HandlePlay::~HandlePlay()
	{
	}

	bool HandlePlay::process()
	{	
		Ice::Identity ident;
		ident.name = _session;
		ident.category = ServantType;
		if (false == openSessionCtx(ident))
		{
			//composeErrorResponse();
			composeResponse(_statusCode);
			return false;
		}

		if (1 == _cltSessCtx.requestType) // SeaChange
			_pRequestWriter->setHeader(HeaderFormatType, SeaChangeFormat);
		else 
		{
			_pRequestWriter->setHeader(HeaderFormatType, TianShanFormat);
			_bNeedModifyResponse = false;
		}

		float fScale = 0.0f;

		String::removeChar(_requestScale = getRequestHeader(HeaderScale), ' ');
		if (false == _requestScale.empty())
		{			
			fScale = static_cast<float>( atof(_requestScale.c_str()) );
		}

		if (1.0 != fScale)
		{
			int errCode = 461;
			composeResponse( errCode , "Not supported, Speed is not equal to 1");//Not supported
			return false;
		}

		_statusCode = 200;
		composeResponse(_statusCode);
		return true;
	}

	PlayResponse::PlayResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		: FixupResponse(env, pSite, pReq, pResponse)
	{
	}

	PlayResponse::~PlayResponse()
	{
	}

	bool PlayResponse::process()
	{
		if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), SeaChangeFormat))
		{
			_pResponse->setHeader(HeaderSeaChangeNotice, getResponseHeader(HeaderTianShanNotice).c_str());
			_pResponse->setHeader(HeaderTianShanNotice, ""); // remove "TianShan-Notice"
		}
		//postResponse(); // need not modify response, post directly
		return true;
	}

} // end namespace LiveChannel

