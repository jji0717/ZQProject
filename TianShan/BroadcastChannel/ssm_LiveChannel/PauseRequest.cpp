#include "PauseRequest.h"

extern ZQ::common::Log* s1log;

namespace LiveChannel
{

#define HandlerFmtEx(_C, _X) CLOGFMT(_C, "Req(%s)Sess(%s)Seq(%s)Mtd(%s:%s) " _X), _pHandler->_reqIdent.c_str(), _pHandler->_session.c_str(), _pHandler->_sequence.c_str(), _pHandler->_phase.c_str(), _pHandler->_method.c_str()

	FixupPause::FixupPause(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: FixupRequest(env, pSite, pReq, pResponse)
	{
	}
	
	FixupPause::~FixupPause()
	{
	}

	bool FixupPause::process()
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

	HandlePause::HandlePause(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: ContentHandler(env, pSite, pReq, pResponse)
	{
	}

	HandlePause::~HandlePause()
	{
	}

	bool HandlePause::process()
	{
		//Do not support pause, so return directly
		int error = 455;
		composeResponse(error, "Not allowed to pause");
		return false;
	}

	PauseResponse::PauseResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		: FixupResponse(env, pSite, pReq, pResponse)
	{
	}

	PauseResponse::~PauseResponse()
	{
	}

	bool PauseResponse::process()
	{
		if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), SeaChangeFormat))
		{
			_pResponse->setHeader(HeaderSeaChangeNotice, getResponseHeader(HeaderTianShanNotice).c_str());
			_pResponse->setHeader(HeaderTianShanNotice, ""); // remove "TianShan-Notice"
		}
		//postResponse(); // need not modify response, post directly
		return true;
	}

} // namespace LiveChannel

