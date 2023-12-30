#include "GetParamRequest.h"

namespace HSNTree
{
	FixupGetParam::FixupGetParam(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: FixupRequest(env, pSite, pReq, pResponse)
	{
	}

	FixupGetParam::~FixupGetParam()
	{
	}

	bool FixupGetParam::process()
	{
		return true;
	}

	HandleGetParam::HandleGetParam(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: ContentHandler(env, pSite, pReq, pResponse)
	{
	}

	HandleGetParam::~HandleGetParam()
	{
	}

	bool HandleGetParam::process()
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

		if (false == getWeiwooPrx() || false == renewSession())
		{
			composeErrorResponse();
			return false;
		}

		getContentBody();

		STRINGVECTOR params;
		STRINGVECTOR_ITOR paramsItor;
		String::splitStr(_reqContent, " \r\n", params);

		if (0 == params.size())
		{
			composeRightResponse();
			return true;
		}

		STRINGVECTOR queryStreams;
		bool bQueryState = false;
		for (paramsItor = params.begin(); paramsItor != params.end(); paramsItor ++)
		{
			if (0 == stricmp(paramsItor->c_str(), "scale") || 0 == stricmp(paramsItor->c_str(), "position"))
				queryStreams.push_back(*paramsItor);
			else if (0 == stricmp(paramsItor->c_str(), "presentation_state"))
				bQueryState = true;
		}

		STRINGMAP resultMap;
/*		if (0 != queryStreams.size())
		{
			std::string scale;
			Ice::Int iCurrentPos = 0, iTotalPos = 0;
			if (false == getStreamPrx() || false == getStreamPlayInfo(scale, iCurrentPos, iTotalPos))
			{
				composeErrorResponse();
				return false;
			}
			for (int i = 0; i < (int) queryStreams.size(); i ++)
			{
				if (0 == stricmp(queryStreams[i].c_str(), "scale"))
					resultMap[queryStreams[i]] = scale; // 这里是为了使请求的与返回的大小写一致
				else if (0 == stricmp(queryStreams[i].c_str(), "position"))
				{
				if (iTotalPos != 0)
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%d.%03d-%d.%03d", 
						iCurrentPos / 1000, iCurrentPos % 1000, iTotalPos / 1000, iTotalPos % 1000);
				else 
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%d.%03d-", iCurrentPos / 1000, iCurrentPos % 1000);
				resultMap[queryStreams[i]] = _szBuf;
				}
			}
		}
*/

		if (true == bQueryState)
		{
			TianShanIce::Streamer::StreamState strmState;
			std::string statDescription;
			if (true == getStreamPrx() && true == getStreamState(strmState, statDescription))
				resultMap["presentation_state"] = statDescription;
			else 
			{
				composeErrorResponse();
				return false;
			}
		}

		for (STRINGMAP_ITOR resultMapItor = resultMap.begin(); resultMapItor != resultMap.end(); resultMapItor ++)
		{
			_returnContentBody += resultMapItor->first + ": " + resultMapItor->second + "\r\n";
		}

		_pResponse->printf_postheader(_returnContentBody.c_str());
		composeRightResponse();

		return true;
	}

	GetParamResponse::GetParamResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		: FixupResponse(env, pSite, pReq, pResponse)
	{
	}

	GetParamResponse::~GetParamResponse()
	{
	}

	bool GetParamResponse::process()
	{
		if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), SeaChangeFormat))
		{
			_pResponse->setHeader(HeaderSeaChangeNotice, getResponseHeader(HeaderTianShanNotice).c_str());
			_pResponse->setHeader(HeaderTianShanNotice, ""); // remove "TianShan-Notice"
		}

		return true;
	}

} // namespace HSNTree

