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

		if (false == getWeiwooPrx() || false == renewSession())
		{
			//composeErrorResponse();
			composeResponse(_statusCode);
			return false;
		}

		getContentBody();

		STRINGVECTOR params;
		STRINGVECTOR_ITOR paramsItor;
		String::splitStr(_reqContent, " \r\n", params);

		if (0 == params.size())
		{
			//composeRightResponse();
			_statusCode = 200;
			composeResponse(_statusCode);
			return true;
		}

		STRINGVECTOR queryStreams, queryApps;
		bool bQueryState = false; std::string state_paramname;
		for (paramsItor = params.begin(); paramsItor != params.end(); paramsItor ++)
		{
			if (0 == stricmp(paramsItor->c_str(), "scale") || 0 == stricmp(paramsItor->c_str(), "position"))
				queryStreams.push_back(*paramsItor);
			else if (0 == stricmp(paramsItor->c_str(), "presentation_state") || 0 == stricmp(paramsItor->c_str(), "stream_state"))
			{	bQueryState = true; state_paramname = *paramsItor; }
			else 
				queryApps.push_back(*paramsItor);
		}

		//search cache if session was existed :ticket12629-->bug 17998
		//only cache parameters of session that has been requested
		STRINGMAP resultMap;
		bool bCached = true; //session has been cached with all the parameter wished
		Environment::SessStatus sessStatus(0,0,0,0,"","","");
		
		if (_env.getSessStatus(_session, sessStatus))
		{
			do 
			{
				if (ZQ::common::now() - sessStatus._stampFirst > _env._cacheInterval)
				{
					_env.removeCachedSess(_session);// the cached status got expired, clean it
					break;
				}

				if (queryStreams.size() >= 0)
				{
					for(int i = 0; i < (int)queryStreams.size(); i++)
					{
						if(0 == stricmp(queryStreams[i].c_str(),"scale"))
						{
							if (sessStatus._scale.empty())
							{
								bCached = false;
								break;
							}

							resultMap[queryStreams[i]] = sessStatus._scale;
							continue;
						}

						if(0 == stricmp(queryStreams[i].c_str(), "position"))
						{
							if(0 == sessStatus._npt || sessStatus._scale.empty())//cannot calculate the npt
							{
								bCached = false;
								break;
							}

							int64 currentPos = sessStatus._npt + (int64) (ZQ::common::now() - sessStatus._stampLast)* atof(sessStatus._scale.c_str());
							char buf[256];
							memset(buf,0,sizeof(buf));
							int curPosInt   = (int) (currentPos/1000);
							int curPosMod   = (int) (currentPos%1000);
							int totalPosInt = (int) (sessStatus._totalPos/1000);
							int totalPosMod = (int) (sessStatus._totalPos%1000);
							if (sessStatus._totalPos > 0)
								snprintf(buf, sizeof(buf) - 1, "%d.%03d-%d.%03d", curPosInt, curPosMod, totalPosInt, totalPosMod);
							//it does`t work
							//snprintf(buf, sizeof(buf) - 1, "%d.%03d-%d.%03d",currentPos/1000,currentPos%1000,sessStatus->_totalPos/1000,sessStatus->_totalPos%1000);
							else 
								snprintf(buf, sizeof(buf) - 1, "%d.%03d-", curPosInt, curPosMod);
							resultMap[queryStreams[i]] = buf;
							sessStatus._npt = currentPos;
							sessStatus._stampLast = ZQ::common::TimeUtil::now();
						}
					} // end of for
				}

				if(bQueryState && bCached)
				{
					if(sessStatus._state_paraName.empty() || sessStatus._state.empty())
					{
						bCached = false;
						break;
					}

					resultMap[sessStatus._state_paraName] = sessStatus._state;
				}

				if(!bCached) // there are some expected parameters not cached, to forward the request to the video server
					break;

				// when reach here, all wished parameters are presented in the cache
				std::string paramlogstr;
				for (STRINGMAP_ITOR resultMapItor = resultMap.begin(); resultMapItor != resultMap.end(); resultMapItor ++)
				{
					_returnContentBody += resultMapItor->first + ": " + resultMapItor->second + "\r\n";
					paramlogstr += resultMapItor->first + "[" + resultMapItor->second + "] ";
				}

				_pResponse->printf_postheader(_returnContentBody.c_str());
				_statusCode = 200;
				composeResponse(_statusCode);
				SSMLOG(ZQ::common::Log::L_DEBUG, HandlerFmt(HandleGetParam, "response via calculation: %s"), paramlogstr.c_str());

				return true;

			} while (0);
		}
		
		// it will run here in 2 cases:
		// case 1.session not cached
		// case 2.session has been cached,but not include the parameters of request
		if (0 != queryStreams.size())
		{
			std::string scale;
			Ice::Int iCurrentPos = 0, iTotalPos = 0;
			if (false == getStreamPrx() || false == getStreamPlayInfo(scale, iCurrentPos, iTotalPos))
			{
				//composeErrorResponse();
				composeResponse(_statusCode);
				return false;
			}
			for (int i = 0; i < (int) queryStreams.size(); i ++)
			{
				if (0 == stricmp(queryStreams[i].c_str(), "scale"))
				{
					TianShanIce::Streamer::StreamState strmState;
					std::string statDescription;
					resultMap[queryStreams[i]] = scale; // 这里是为了使请求的与返回的大小写一致
					sessStatus._scale = scale;
					if (true == getStreamPrx() && true == getStreamState(strmState, statDescription))
					{
						if (_tsConfig.scale0Paused != 0 && 0 == statDescription.compare("pause")) 
						{
							resultMap[queryStreams[i]] = "0";
							sessStatus._scale = "0";
						}
						SSMLOG(DebugLevel, HandlerFmt(HandleGetParam, "scale0AtPaused:%d current status:%s"), _tsConfig.scale0Paused,statDescription.c_str());
					}
				}
				else if (0 == stricmp(queryStreams[i].c_str(), "position"))
				{
					if (iTotalPos > 0)
						snprintf(_szBuf, sizeof(_szBuf) - 1, "%d.%03d-%d.%03d", 
							iCurrentPos / 1000, iCurrentPos % 1000, iTotalPos / 1000, iTotalPos % 1000);
					else 
						snprintf(_szBuf, sizeof(_szBuf) - 1, "%d.%03d-", iCurrentPos / 1000, iCurrentPos % 1000);

					resultMap[queryStreams[i]] = _szBuf;
					sessStatus._npt = iCurrentPos;
					sessStatus._totalPos = iTotalPos;
				}
			}
		}

		if (true == bQueryState)
		{
			TianShanIce::Streamer::StreamState strmState;
			std::string statDescription;
				
			if (true == getStreamPrx() && true == getStreamState(strmState, statDescription))
			{
				if (1 == _cltSessCtx.requestType && 0 == statDescription.compare("play")) // ticket#9185
					statDescription = "playing";
				resultMap[state_paramname] = statDescription;
				sessStatus._state_paraName = state_paramname;
				sessStatus._state = statDescription;
			}
			else 
			{
				//composeErrorResponse();
				composeResponse(_statusCode);
				return false;
			}
		}

		if (0 != queryApps.size())
		{
			if (false == getPurchasePrx())
			{
				//composeErrorResponse();
				composeResponse(_statusCode);
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
				//_error_statusline = ResponseParameterNotUnderstood;
				//composeErrorResponse();
				_statusCode = 451;
				composeResponse(_statusCode);
				return false;
			}
			catch (TianShanIce::NotSupported& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by purchase(%s).getParameters", 
					ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleGetParam, "%s"), _szBuf);
				/*_error_statusline = ResponseParameterNotUnderstood;
				composeErrorResponse();*/
				_statusCode = 451;
				composeResponse(_statusCode);
				return false;
			}
			catch (TianShanIce::ServerError& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by purchase(%s).getParameters"
					, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.purchasePrxID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleGetParam, "%s"), _szBuf);
				//composeErrorResponse();
				_statusCode = 500;
				composeResponse(_statusCode);
				return false;
			}
			catch( const Ice::SocketException& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[GetParameter:0300] caught by purchase(%s).getParameters", 
					ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleGetParam, "%s"), _szBuf);
				//composeErrorResponse();
				_statusCode = 503;
				composeResponse(_statusCode);
				return false;
			}
			catch (const Ice::TimeoutException& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[GetParameter:0300] caught by purchase(%s).getParameters", 
					ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleGetParam, "%s"), _szBuf);
				//composeErrorResponse();
				_statusCode = 503;
				composeResponse(_statusCode);
				return false;
			}
			catch (Ice::Exception& ex)
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[GetParameter:0300] caught by purchase(%s).getParameters", 
					ex.ice_name().c_str(), _cltSessCtx.purchasePrxID.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandleGetParam, "%s"), _szBuf);
				//composeErrorResponse();
				composeResponse(_statusCode);
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
		
		std::string paramlogstr;
		for (STRINGMAP_ITOR resultMapItor = resultMap.begin(); resultMapItor != resultMap.end(); resultMapItor ++)
		{
			_returnContentBody += resultMapItor->first + ": " + resultMapItor->second + "\r\n";
			paramlogstr += resultMapItor->first + "[" + resultMapItor->second + "] ";
		}

		_pResponse->printf_postheader(_returnContentBody.c_str());
		_statusCode = 200;
		composeResponse(_statusCode);

		//add session status to cache
		sessStatus._stampLast = ZQ::common::now();
		sessStatus._stampFirst = ZQ::common::now();

		if(!_session.empty())
			_env.addCacheSess(_session,sessStatus);
		//composeRightResponse();

		SSMLOG(ZQ::common::Log::L_DEBUG, HandlerFmt(HandleGetParam, "response via inquiry: %s"), paramlogstr.c_str());
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

