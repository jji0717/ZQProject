// File Name : GetParameterHandler.cpp

#include "GetParameterHandler.h"

#include "Environment.h"

#include "CRGSessionManager.h"

#include "stroprt.h"

#include "RtspRelevant.h"


namespace GBss
{

GetParameterHandler::GetParameterHandler(ZQ::common::Log& fileLog, Environment& env, 
										 IStreamSmithSite* pSite, IClientRequestWriter* pReq)
:RequestHandler(fileLog, env, pSite, pReq)
{
	_method = "GET_PARAMETER";
}

GetParameterHandler::~GetParameterHandler()
{

}

RequestProcessResult GetParameterHandler::doContentHandler()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(GetParameterHandler, "start processing"));
	_response->setHeader(HeaderContentType, "text/parameters");

	// get content 
	getContentBody();
	std::vector<std::string> reqParams;
	ZQ::StringOperation::splitStr(_requestBody, " :\n\r\t", reqParams);

	std::string range("");
	std::string scale("");
	bool bGetPositionAndScale = false;
	std::string strResponseContent;
	if (_session != _env.getGlobalSession())
	{
		// renew session 
		if (!renewSession())
		{
			_response->setHeader(HeaderContentLength, "0");
			return RequestError;
		}

		for (size_t i = 0; i < reqParams.size(); i++)
		{
			if (0 == stricmp(reqParams[i].c_str(), "position"))
			{
				// get Position
				if (!bGetPositionAndScale)
				{
					if (!getPositionAndScale(range, scale))
					{
						return RequestError;
					}
					bGetPositionAndScale = true;
				}
				strResponseContent += reqParams[i] + ": ";
				//if (std::string::npos == range.find('='))
				//	strResponseContent += "npt=";
				strResponseContent += range;
				strResponseContent += CRLF;
				continue;
			}
			if (0 == stricmp(reqParams[i].c_str(), "Scale"))
			{
				// get Scale
				// get Position
				if (!bGetPositionAndScale)
				{
					if (!getPositionAndScale(range, scale))
					{
						return RequestError;
					}
					bGetPositionAndScale = true;
				}
				strResponseContent += reqParams[i] + ": ";
				strResponseContent += scale;
				strResponseContent += CRLF;
				continue;
				continue;
			}
			if (0 == stricmp(reqParams[i].c_str(), "stream_state") || 0 == stricmp(reqParams[i].c_str(), "presentation_state"))
			{
				// get stream state
				TianShanIce::Streamer::StreamState state;
				if (!getPlaylistState(state))
				{
					return RequestError;
				}
				strResponseContent += reqParams[i] + ": ";
				strResponseContent += getStateString(state);
				strResponseContent += CRLF;
				continue;
			}
			if (0 == stricmp(reqParams[i].c_str(), "StreamSource"))
			{
				// get stream source
				std::string sourceIP;
				int sourcePort;
				if( !getStreamSourceAddressInfo( sourceIP, sourcePort ))
					return RequestError;
				std::stringstream oss;
				oss << sourceIP << " " << sourcePort;
				strResponseContent += reqParams[i] + ": ";
				strResponseContent += oss.str();
				strResponseContent += CRLF;
				continue;
			}

			_statusCode = 451;
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s", reqParams[i].c_str());
			return RequestError;
		}
	}
	else
	{
		std::string srmId;

		for (size_t i = 0; i < reqParams.size(); i++)
		{
			if (0 == stricmp(reqParams[i].c_str(), "session_count"))
			{
				//_env.getSessionManager().
				int64 count = _env.getSessionManager().getTotalSessionsNum();
				char buf[126];
				sprintf(buf, "%lld", count);
				strResponseContent += "session_count: ";
				strResponseContent += buf;
				strResponseContent += CRLF;
				continue;
			}
			if (0 == stricmp(reqParams[i].c_str(), "session_list"))
			{
				std::string sessionList;
				_env.getSessionManager().getSessionlist(sessionList);
				strResponseContent += "session_list: ";
				strResponseContent += sessionList;
				strResponseContent += CRLF;
				continue;
			}

			if (0 == stricmp(reqParams[i].c_str(), "SRM_ID"))
			{
				if (++i < reqParams.size())
				{
					srmId = reqParams[i];
//					std::string strConnID = getRequestHeader("SYS#ConnID");
//					_env.addIntoSRMMap(reqParams[i], strConnID);
					continue;
				}
				else
				{
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s", "no SRM_ID found");
					_statusCode = 400;
					return RequestError;
				}
			}

			_statusCode = 451;
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%s", reqParams[i].c_str());
			return RequestError;
		}

		std::string strConnID = getRequestHeader("SYS#ConnID");
		if (srmId.empty())
		{
			srmId = ".default";
			HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(GetParameterHandler, "set default SRM conn[%s]"), strConnID.c_str());
		}
		_env.addIntoSRMMap(".default", strConnID);
	}

	_statusCode = 200;
	_response->printf_postheader(strResponseContent.c_str());
	return RequestProcessed;
}

} // end GBss
