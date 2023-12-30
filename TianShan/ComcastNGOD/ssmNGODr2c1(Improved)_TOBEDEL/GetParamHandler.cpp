#include "./GetParamHandler.h"

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

GetParamHandler::GetParamHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
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
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(GetParamHandler, "start to be processed"));

	if (false == _canProcess)
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(GetParamHandler, "we can't process the request because of [%s]"), szBuf);
		return RequestError;
	}

	if (_session != _ssmNGODr2c1._globalSession)
	{
		if (false == getContext())
		{
			responseError(RESPONSE_INTERNAL_ERROR);
			return RequestError;
		}
		
		if (false == renewSession())
		{
			responseError(RESPONSE_INTERNAL_ERROR);
			return RequestError;
		}
	}

	getContentBody();
	ZQ::StringOperation::splitStr(_requestBody, " \n\r\t", _reqParams);
	
	if (_reqParams.size() == 0)
	{
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(GetParamHandler, "Just a heartbeat!"));
		responseOK();
		return RequestProcessed;
	}
	
	std::string presentation_state, connection_timeout, session_list;
	for (int i = 0; i < (int)_reqParams.size(); i ++)
	{
		if (stricmp(_reqParams[i].c_str(), "scale") == 0 || stricmp(_reqParams[i].c_str(), "position") == 0)
			_streamParams.push_back(_reqParams[i]);
		else if (stricmp(_reqParams[i].c_str(), "presentation_state") == 0)
			presentation_state = _reqParams[i];
		else if (stricmp(_reqParams[i].c_str(), "connection_timeout") == 0)
			connection_timeout = _reqParams[i];
		else if (stricmp(_reqParams[i].c_str(), "session_list") == 0)
			session_list = _reqParams[i];
		else 
			_appParams.push_back(_reqParams[i]);
	}

	if (_streamParams.size() > 0)
	{
		std::string range, scale;
		_inoutMap[MAP_KEY_STREAMFULLID] = _pContext->streamFullID;
		if (true == getStream() && true == getPositionAndScale(range, scale))
		{
		for (int tcur = 0; tcur < (int)_streamParams.size(); tcur ++)
		{
			if (stricmp(_streamParams[tcur].c_str(), "scale") == 0)
				_outMap[_streamParams[tcur]] = scale;
			else if (stricmp(_streamParams[tcur].c_str(), "position") == 0)
				_outMap[_streamParams[tcur]] = range;
		}
		}
	}

	if (false == presentation_state.empty())
	{
		TianShanIce::Streamer::StreamState strmState;
		std::string stateDept;
		if (true == getStream() && true == getStreamState(strmState, stateDept))
			_outMap[presentation_state] = stateDept;
	}

	if (_appParams.size() > 0)
	{
		// DO: get purchase proxy
		if (true == getPurchase())
		{
		}
	}

	if (false == connection_timeout.empty())
	{
		char buff_tmp[20];
		buff_tmp[sizeof(buff_tmp) - 1] = '\0';
		snprintf(buff_tmp, sizeof(buff_tmp) - 1, "%d", _ssmNGODr2c1._config._timeoutInterval);
		_outMap[connection_timeout] = buff_tmp;
	}

	if (false == session_list.empty())
	{
		std::string pair_str;
		std::string sessionGroup;
		sessionGroup = getRequestHeader(NGOD_HEADER_SESSIONGROUP);
		std::vector<Ice::Identity> idents;
		idents = _ssmNGODr2c1._pGroupIdx->find(sessionGroup);
		for (int tcur = 0; tcur < (int)idents.size(); tcur ++)
		{
			NGODr2c1::ContextImplPtr pNewContext = new NGODr2c1::ContextImpl(_ssmNGODr2c1);
			NGODr2c1::ContextPrx pNewContextPrx = NULL;
			bool ret = getContextByIdentity(pNewContext, pNewContextPrx, idents[tcur]);
			if (true == ret)
				pair_str += pNewContext->ident.name + ":" + pNewContext->onDemandID + " ";
		}		
		_outMap[session_list] = pair_str;
	}

	std::map<std::string, std::string>::iterator retMapItor;
	for (retMapItor = _outMap.begin(); retMapItor != _outMap.end(); retMapItor ++)
	{
		_returnContent += retMapItor->first + ": " + retMapItor->second + "\r\n";
	}
	
	_pResponse->setHeader(NGOD_HEADER_CONTENTTYPE, "text/parameters");
	_pResponse->printf_postheader(_returnContent.c_str());
	responseOK();

	return RequestProcessed;
}

