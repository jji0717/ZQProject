#include "GetParamRequest.h"

extern ZQ::common::Log* s1log;

namespace TianShanS1
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

		if (false == getWeiwooPrx())
		{
			composeErrorResponse();
			return false;
		}
		int statusCode;
		if (false == renewSession(statusCode))
		{
			if (404 == statusCode || 503 == statusCode)
			{
				composeErrorResponse();
				return false;
			}
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

		STRINGVECTOR queryStreams, queryApps;
		bool bQueryState = false;
		for (paramsItor = params.begin(); paramsItor != params.end(); paramsItor ++)
		{
			if (0 == stricmp(paramsItor->c_str(), "scale") || 0 == stricmp(paramsItor->c_str(), "position"))
				queryStreams.push_back(*paramsItor);
			else if (0 == stricmp(paramsItor->c_str(), "presentation_state"))
				bQueryState = true;
			else 
				queryApps.push_back(*paramsItor);
		}

		STRINGMAP resultMap;
		if (0 != queryStreams.size())
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

		if (0 != queryApps.size())
		{
			if (false == getPurchasePrx())
			{
				composeErrorResponse();
				return false;
			}

			TianShanIce::ValueMap inMap, outMap;
			try
			{
				// throws InvalidParameter, NotSupported, ServerError;
				// according to TsApplication.ICE, version: 17
				_purchasePrx->getParameters(queryApps, inMap, outMap);
			}
			catch (TianShanIce::InvalidParameter& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by purchase(%s).getParameters", 
					ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleGetParam, "%s"), _szBuf);
				_error_statusline = ResponseParameterNotUnderstood;
				composeErrorResponse();
				return false;
			}
			catch (TianShanIce::NotSupported& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by purchase(%s).getParameters", 
					ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleGetParam, "%s"), _szBuf);
				_error_statusline = ResponseParameterNotUnderstood;
				composeErrorResponse();
				return false;
			}
			catch (TianShanIce::ServerError& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by purchase(%s).getParameters"
					, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleGetParam, "%s"), _szBuf);
				composeErrorResponse();
				return false;
			}
			catch (Ice::Exception& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[GetParameter:0300] caught by purchase(%s).getParameters", 
					ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleGetParam, "%s"), _szBuf);
				composeErrorResponse();
				return false;
			}

			for (::TianShanIce::ValueMap::iterator outMapItor = outMap.begin(); outMapItor != outMap.end(); outMapItor ++)
			{
				for (int i = 0; i < (int) queryApps.size(); i ++)
				{
					if (0 == stricmp(queryApps[i].c_str(), outMapItor->first.c_str()))
					{
					switch (outMapItor->second.type)
					{
					case TianShanIce::vtStrings: 
						{
							if (outMapItor->second.strs.size() > 0)
								resultMap[outMapItor->first] = outMapItor->second.strs[0];
						}
						break;
					case TianShanIce::vtLongs: 
						{
							if (outMapItor->second.lints.size() > 0)
							{
								snprintf(_szBuf, sizeof(_szBuf) - 1, FMT64, outMapItor->second.lints[0]);
								resultMap[outMapItor->first] = _szBuf;
							}
						}
						break;
					case TianShanIce::vtInts: 
						{
							if (outMapItor->second.ints.size() > 0)
							{
								snprintf(_szBuf, sizeof(_szBuf) - 1, "%d", outMapItor->second.ints[0]);
								resultMap[outMapItor->first] = _szBuf;
							}
						}
						break;
					case TianShanIce::vtFloats: 
						{
							if (outMapItor->second.floats.size() > 0)
							{
								snprintf(_szBuf, sizeof(_szBuf) - 1, "%f", outMapItor->second.floats[0]);
								resultMap[outMapItor->first] = _szBuf;
							}
						}
						break;
					default: break;
					}
					break;
					}
				}
			}
		}
		
		for (STRINGMAP_ITOR resultMapItor = resultMap.begin(); resultMapItor != resultMap.end(); resultMapItor ++)
		{
			_returnContentBody += resultMapItor->first + ": " + resultMapItor->second + "\r\n";
		}

		_pResponse->setHeader("HeaderContentType", "text/parameters");

		_pResponse->setHeader("GlobalSession", _cltSessCtx.props["GlobalSession"].c_str());

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

} // namespace TianShanS1

