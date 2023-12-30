#include "./GetParamHandler.h"

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

GetParamHandler::GetParamHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
{
	_method = "GET_PARAMETER";
	_inoutMap[MAP_KEY_METHOD] = _method;
#ifdef _DEBUG
	cout<<"construct GET_PARAMETER handler"<<endl;
#endif
}

GetParamHandler::~GetParamHandler()
{
#ifdef _DEBUG
	cout<<"deconstruct GET_PARAMETER handler"<<endl;
#endif
}

RequestProcessResult GetParamHandler::process()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(GetParamHandler, "start processing"));

	std::vector<std::string> reqParams;
	std::vector<std::string> streamParams;
	std::vector<std::string> appParams;
	std::map<std::string, std::string> outMap;
	std::string returnContent;
	returnContent.reserve(512);

	if (false == _canProcess)
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(GetParamHandler, "failed to process the request because [%s]"), szBuf);
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(GetParamHandler, "failed to process the request because [%s]"), szBuf);
		return RequestError;
	}
	if (_ngodConfig._MessageFmt.rtspNptUsage <= 0)
	{
		if (!handshake(_requireProtocol))
		{
			return RequestError;
		}
	}
	

	std::string onDemandID;
	onDemandID = getRequestHeader(NGOD_HEADER_ONDEMANDSESSIONID);
	_pResponse->setHeader(NGOD_HEADER_ONDEMANDSESSIONID, onDemandID.c_str());	

	if (_session != _ssmNGODr2c1._globalSession)
	{
		if (false == getContext())
		{
			//responseError(RESPONSE_INTERNAL_ERROR);
			responseError(RESPONSE_SESSION_NOTFOUND);
			return RequestError;
		}
		
		if (false == renewSession())
		{
			responseError(RESPONSE_INTERNAL_ERROR);
			return RequestError;
		}

		if( !checkStreamer() )
		{
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(GetParamHandler, "streamer is unavailable [%s:%s]"), 
				_context.sopname.c_str() , _context.streamNetId.c_str() );
			responseError(RESPONSE_SERVICE_UNAVAILABLE);
			return RequestError;
		}

		//update C1ConnectionId
		::std::string connId = getRequestHeader("SYS#ConnID");
		if (false == updateContextProp(C1CONNID, connId))
		{
			responseError(RESPONSE_INTERNAL_ERROR);
			return RequestError;
		}
	}


	getContentBody();
	ZQ::StringOperation::splitStr(_requestBody, " \n\r\t", reqParams);
	
	if (reqParams.size() == 0)
	{
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(GetParamHandler, "no parameter is specified, treated as a PING"));
		responseOK();
		return RequestProcessed;
	}
	
	std::string presentation_state, connection_timeout, session_list;
	for (int i = 0; i < (int)reqParams.size(); i ++)
	{
		if (stricmp(reqParams[i].c_str(), "scale") == 0 || stricmp(reqParams[i].c_str(), "position") == 0)
			streamParams.push_back(reqParams[i]);
		else if (stricmp(reqParams[i].c_str(), "presentation_state") == 0)
			presentation_state = reqParams[i];
		else if (stricmp(reqParams[i].c_str(), "connection_timeout") == 0)
			connection_timeout = reqParams[i];
		else if (stricmp(reqParams[i].c_str(), "session_list") == 0)
			session_list = reqParams[i];
		else 
			appParams.push_back(reqParams[i]);
	}

	if (streamParams.size() > 0)
	{
		std::string range, scale;
		_inoutMap[MAP_KEY_STREAMFULLID] = _context.streamFullID;
		_inoutMap[MAP_KEY_SOPNAME]		= _context.sopname;
		_inoutMap[MAP_KEY_STREMAERNETID]= _context.streamNetId;
		if (true == getStream() && true == getPositionAndScale(range, scale))
		{
		for (int tcur = 0; tcur < (int)streamParams.size(); tcur ++)
		{
			if (stricmp(streamParams[tcur].c_str(), "scale") == 0)
				outMap[streamParams[tcur]] = scale;
			else if (stricmp(streamParams[tcur].c_str(), "position") == 0)
			{
				// add by zjm for bug 10363
				size_t nPosition = range.find("-");
				if (nPosition != std::string::npos)
				{
					outMap[streamParams[tcur]] = range.substr(0, nPosition);
				}
				else
				{
					outMap[streamParams[tcur]] = range;
				}
				//outMap[streamParams[tcur]] = range;
			}
		}
		}
	}

	if (false == presentation_state.empty())
	{
		TianShanIce::Streamer::StreamState strmState;
		std::string stateDept;
		if (true == getStream() && true == getStreamState(strmState, stateDept))
			outMap[presentation_state] = stateDept;
	}

	if (false == connection_timeout.empty())
	{
		char buff_tmp[20];
		buff_tmp[sizeof(buff_tmp) - 1] = '\0';
		snprintf(buff_tmp, sizeof(buff_tmp) - 1, "%d", _ngodConfig._rtspSession._timeout);
		outMap[connection_timeout] = buff_tmp;
	}

	std::map<std::string, std::string>::iterator retMapItor;
	for (retMapItor = outMap.begin(); retMapItor != outMap.end(); retMapItor ++)
	{
		returnContent += retMapItor->first;
		returnContent += ": ";
		returnContent += retMapItor->second;
		returnContent += "\r\n";
	}

	// because the session list will be a big block, so do some optimization
	if (false == session_list.empty())
	{
		std::string sessionGroup;
		sessionGroup = getRequestHeader(NGOD_HEADER_SESSIONGROUP);
		std::vector<Ice::Identity> idents;
		try
		{
			idents = _ssmNGODr2c1._pGroupIdx->find(sessionGroup);
			int nSessionCount = idents.size();
			HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(GetParamHandler, "found [%d] sessions of SessionGroup[%s] "), 
				nSessionCount, sessionGroup.c_str());

			if (nSessionCount)
			{
				// clientsession length + ondemandsession lenght + 2(':' and ' ')
				unsigned int nNewSize = returnContent.size() + (unsigned int) nSessionCount * (_context.ident.name.size() + _context.onDemandID.size() + 2);
				if (nNewSize>returnContent.capacity())
					returnContent.reserve(nNewSize);
			}
			
			returnContent += session_list + ": ";  // + retMapItor->second + "\r\n";
			for (int tcur = 0; tcur < nSessionCount; tcur ++)
			{			
				NGODr2c1::ctxData NewContext;
				NGODr2c1::ContextPrx pNewContextPrx = NULL;
				bool ret = getContextByIdentity(NewContext, pNewContextPrx, idents[tcur]);
				if (true == ret)
				{
					returnContent += NewContext.ident.name;
					returnContent += ":";
					returnContent += NewContext.onDemandID;
					returnContent += " ";
				}
			}
			returnContent += "\r\n";
		}
		catch (const Freeze::DatabaseException& ex)
		{
			HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(GetParamHandler, "inquiry session_list caught exception[%s]: %s"), ex.ice_name().c_str(), ex.message.c_str());
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(GetParamHandler, "inquiry session_list caught exception[%s]: %s"), ex.ice_name().c_str(), ex.message.c_str());
			responseError(RESPONSE_INTERNAL_ERROR);
			return RequestError;
		}
		catch (const Ice::Exception& ex)
		{
			HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(GetParamHandler, "inquiry session_list caught exception[%s]"), ex.ice_name().c_str());
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(GetParamHandler, "inquiry session_list caught exception[%s]"), ex.ice_name().c_str());
			responseError(RESPONSE_INTERNAL_ERROR);
			return RequestError;
		}
	}

	
	_pResponse->setHeader(NGOD_HEADER_CONTENTTYPE, "text/parameters");
	_pResponse->printf_postheader(returnContent.c_str());
	responseOK();

	return RequestProcessed;
}

