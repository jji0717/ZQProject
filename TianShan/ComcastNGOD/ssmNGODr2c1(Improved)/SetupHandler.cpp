#include "./SetupHandler.h"

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

SetupHandler::SetupHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
{
	_method = "SETUP";
	_inoutMap[MAP_KEY_METHOD] = _method;
#ifdef _DEBUG
	cout<<"construct SETUP handler"<<endl;
#endif
}

SetupHandler::~SetupHandler()
{
#ifdef _DEBUG
	cout<<"deconstruct SETUP handler"<<endl;
#endif
}

RequestProcessResult SetupHandler::process()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetupHandler, "start to be processed"));

	if (false == _canProcess)
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "we can't process the request because [%s]"), szBuf);
		return RequestError;
	}
	
	// DO: get OnDemandSessionID
	std::string onDemandID;
	onDemandID = getRequestHeader(NGOD_HEADER_ONDEMANDSESSIONID);
	_pResponse->setHeader(NGOD_HEADER_ONDEMANDSESSIONID, onDemandID.c_str());

	std::string originalURI, strResourceURI;
	std::string rtspServerIP, rtspServerPort;
	rtspServerIP = getRequestHeader("SYS#LocalServerIP");
	rtspServerPort = getRequestHeader("SYS#LocalServerPort");
	originalURI = getRequestHeader(ORIGINAL_URI);
	strResourceURI = getRequestHeader(RESOURCE_URI);
	
	HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler, "to create rtspProxy client session with URL: [%s]"), strResourceURI.c_str());
	IClientSession* pSession = _pSite->createClientSession(NULL, strResourceURI.c_str());
	if (NULL == pSession || NULL == pSession->getSessionID())
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "create rtspProxy client session failed");
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	_session = pSession->getSessionID();
	_inoutMap[MAP_KEY_SESSION] = _session;
	_pRequest->setHeader(NGOD_HEADER_SESSION, (char*)_session.c_str());
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetupHandler, "rtspProxy client session created successfully"));	
	
	// DO: get SessionGroup
	std::string sessionGroup;
	sessionGroup = getRequestHeader(NGOD_HEADER_SESSIONGROUP);
	
	// DO: Get connectionID
	std::string connectionID;
	connectionID = getRequestHeader("SYS#ConnID");

	// DO: create a new session context
	_pContext->ident.name = _session;
	_pContext->ident.category = SERVANT_TYPE;
    _pContext->onDemandID = onDemandID;
    _pContext->normalURL = originalURI;
    _pContext->resourceURL = strResourceURI;
    _pContext->connectID = connectionID;
    _pContext->groupID = sessionGroup;
	_pContext->expiration = ZQTianShan::now() + _ssmNGODr2c1._config._timeoutInterval * 1000;
	_pContext->announceSeq = 1;
	
	// DO: Create Weiwoo Session
	::TianShanIce::SRM::Resource ResourceURI;
	ResourceURI.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
	ResourceURI.status = TianShanIce::SRM::rsRequested;
	::TianShanIce::Variant vResourceURI;
	vResourceURI.bRange = false;
	vResourceURI.type = TianShanIce::vtStrings;
	vResourceURI.strs.clear();
	vResourceURI.strs.push_back(strResourceURI);
	ResourceURI.resourceData["uri"] = vResourceURI;
	try
	{
		if(NULL == _ssmNGODr2c1._pSessionManager)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "session manager proxy is null");
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
			responseError(RESPONSE_INTERNAL_ERROR);
			return RequestError;
		}
		
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler, "to create weiwoo session"));
		_sessionPrx = _ssmNGODr2c1._pSessionManager->createSession(ResourceURI);
		_pContext->weiwooFullID = _ssmNGODr2c1._pCommunicator->proxyToString(_sessionPrx);
		HANDLERLOG(ZQ::common::Log::L_NOTICE, HANDLERLOGFMT(SetupHandler, "weiwoo session: [%s] created successfully"), _pContext->weiwooFullID.c_str());
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		snprintf(szBuf, sizeof(szBuf) - 1,"perform createSession() on SessionManager: [%s] caught [%s]:[%s]", _ssmNGODr2c1._config._sessionManager.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
		responseError(RESPONSE_BAD_REQUEST);
		return RequestError;
	}
	catch (const Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1,"perform createSession() on SessionManager: [%s] caught [%s]", _ssmNGODr2c1._config._sessionManager.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	_pdMap.clear();
	
	::TianShanIce::Variant vSessionGroup;
	vSessionGroup.type = ::TianShanIce::vtStrings;
	vSessionGroup.bRange = false;
	vSessionGroup.strs.clear();
	vSessionGroup.strs.push_back(_pContext->groupID);
	addPrivateData("NGOD.R2.SessionGroup", vSessionGroup);
	
	// DO: add originalURL to weiwoo session's private data
	::TianShanIce::Variant vOriginalURI;
	vOriginalURI.type = ::TianShanIce::vtStrings;
	vOriginalURI.bRange = false;
	vOriginalURI.strs.clear();
	vOriginalURI.strs.push_back(_pContext->normalURL);
	addPrivateData("NGOD.R2.url", vOriginalURI);
	
	// DO: add node-group-id to weiwoo session's private data
	::TianShanIce::Variant vGroupID;
	vGroupID.type = ::TianShanIce::vtStrings;
	vGroupID.bRange = false;
	vGroupID.strs.clear();
	vGroupID.strs.push_back(_ssmNGODr2c1._config._nodeGroupID);
	addPrivateData("node-group-id", vGroupID);

	std::string transportData = getRequestHeader(NGOD_HEADER_TRANSPORT);

	long lbandwidth;
	
	std::vector<std::string> Transports;
	ZQ::StringOperation::splitStr(transportData, ";", Transports);
	int Transports_size = Transports.size();
	for (int cur = 0; cur < Transports_size; cur++)
	{
		std::string left, right;
		left = ZQ::StringOperation::getLeftStr(Transports[cur],"=");
		right = ZQ::StringOperation::getRightStr(Transports[cur],"=");
		if (left.empty() || right.empty())
			continue;

		::TianShanIce::Variant var;
		var.type = ::TianShanIce::vtStrings;
		var.bRange = false;
		var.strs.clear();
		var.strs.push_back(right);
		std::string keystr("NGOD.R2.");
		keystr += left;
		addPrivateData(keystr, var);

		if (left == "client_port")
		{
			::TianShanIce::Variant var;
			var.type = ::TianShanIce::vtInts;
			var.bRange = false;
			var.ints.clear();
			var.ints.push_back(atoi(right.c_str()));
			addPrivateData("NGOD.R2.client_port", var);
		}
		else if (left == "bandwidth")
		{
			lbandwidth = atol(right.c_str());
			continue;
		}
		else if (left == "sop_name" || left == "sop_group")
		{
			ssmNGODr2c1::SopMap::const_iterator itSopRest = _ssmNGODr2c1._sopRestriction.find(right);
			if (_ssmNGODr2c1._sopRestriction.end() == itSopRest)
				continue;

			if (itSopRest->second.serviceGroup >0)
			{
				// DO: add node-group-id to weiwoo session's private data
				::TianShanIce::Variant vGroupID;
				vGroupID.type = ::TianShanIce::vtStrings;
				vGroupID.bRange = false;
				vGroupID.strs.clear();
				char buff[30];
				memset(buff, 0, sizeof(buff));
				snprintf(buff, sizeof(buff) - 1, "%d", itSopRest->second.serviceGroup);
				vGroupID.strs.push_back(buff);
				addPrivateData("node-group-id", vGroupID);
			}

			// DO: Add Resource(rtStreamer)
			try
			{
				::TianShanIce ::ValueMap vMap = itSopRest->second.streamers.resourceData;
				if (vMap["NetworkId"].strs.size()>0)
				{
					HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler, "addResourceEx() session[%s] restrict streamers per sop [%s]"), _pContext->weiwooFullID.c_str(), right.c_str());
					_sessionPrx->addResourceEx(::TianShanIce::SRM::rtStreamer, itSopRest->second.streamers);
					HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetupHandler, "addResourceEx() session[%s] restrict streamers per sop [%s] successfully"), _pContext->weiwooFullID.c_str(), right.c_str());
				}
			}
			catch(const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform addResourceEx() on weiwoo session: [%s] caught [%s]:[%s]", _pContext->weiwooFullID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
				HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
				responseError(RESPONSE_INTERNAL_ERROR);
				return RequestError;
			}
			catch(Ice::Exception& ex)
			{
				snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform addResource() on weiwoo session: [%s] caught [%s]", _pContext->weiwooFullID.c_str(), ex.ice_name().c_str());
				HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
				responseError(RESPONSE_INTERNAL_ERROR);
				return RequestError;
			}
		}
	}
	
	// DO: Add Resource, resource type is rtTsDownstreamBandwidth
	::TianShanIce::ValueMap BWResource;
	TianShanIce::Variant vrtBandWidth;
	vrtBandWidth.type = ::TianShanIce::vtLongs;
	vrtBandWidth.bRange = false;
	vrtBandWidth.lints.clear();
	vrtBandWidth.lints.push_back(lbandwidth);
	BWResource["bandwidth"] = vrtBandWidth;
	try
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler, "to perform addResource() on weiwoo session: [%s]"), _pContext->weiwooFullID.c_str());
		_sessionPrx->addResource(::TianShanIce::SRM::rtTsDownstreamBandwidth, TianShanIce::SRM::raMandatoryNonNegotiable, BWResource);
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetupHandler, "perform addResource() on weiwoo session: [%s] successfully"), _pContext->weiwooFullID.c_str());
	}
	catch(const TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform addResource() on weiwoo session: [%s] caught [%s]:[%s]", _pContext->weiwooFullID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	catch(Ice::Exception& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform addResource() on weiwoo session: [%s] caught [%s]", _pContext->weiwooFullID.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	
	bool bQam = false;
	if( strstr(szBuf,"MP2T") != NULL &&  strstr(szBuf,"DVB") != NULL && strstr(szBuf,"UDP") != NULL  )
	{
		bQam = true;
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler, "QAM mode"));
	}
	else 
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler, "IP mode"));

	//////////////////////////////////////////////////////////////////////////
	// DO: support encription
	getContentBody();
	
	const char* next_content = _requestBody.c_str();
	const char* pTemp = NULL;
	std::vector<std::string> assets;
	pTemp = strstr(next_content, "a=X-motorola-ecm:");
	while (pTemp != NULL)
	{
		pTemp += strlen("a=X-motorola-ecm:");
		next_content = pTemp;
		std::vector<std::string> temp_strs;
		ZQ::StringOperation::splitStr(pTemp, " \r\n\t", temp_strs);
		std::string providerID, assetID;
		if (temp_strs.size() >= 5 
			&& ZQ::StringOperation::isInt(temp_strs[2].c_str())
			&& ZQ::StringOperation::isInt(temp_strs[3].c_str())
			&& ZQ::StringOperation::isInt(temp_strs[4].c_str()))
		{
			providerID = temp_strs[0];
			assetID = temp_strs[1];
			
			std::string keyStr;
			keyStr = "MotoEcm.progNum.";
			keyStr += providerID + "#" + assetID;
			::TianShanIce::Variant var;
			var.type = ::TianShanIce::vtInts;
			var.bRange = false;
			var.ints.clear();
			int tmpInt = atoi(temp_strs[2].c_str());
			var.ints.push_back(tmpInt);
			addPrivateData(keyStr, var);
			
			keyStr = "MotoEcm.freq1.";
			keyStr += providerID + "#" + assetID;
			var.ints.clear();
			tmpInt = atoi(temp_strs[3].c_str());
			var.ints.push_back(tmpInt);
			addPrivateData(keyStr, var);
			
			int keyNum = atoi(temp_strs[4].c_str());
			if (keyNum > 0 && (int)temp_strs.size() >= 5 + keyNum * 2)
			{
				::TianShanIce::Variant intVar, strVar;
				intVar.type = ::TianShanIce::vtInts;
				intVar.bRange = false;
				intVar.ints.clear();
				
				strVar.type = ::TianShanIce::vtStrings;
				strVar.bRange = false;
				strVar.strs.clear();
				
				for (int tCur = 0; tCur < keyNum; tCur ++)
				{
					int tv = atoi(temp_strs[5 + tCur * 2].c_str());
					intVar.ints.push_back(tv);
					
					std::string tmpStr = temp_strs[6 + tCur * 2];
					strVar.strs.push_back(tmpStr);
				}
				
				keyStr = "MotoEcm.keyoffsets.";
				keyStr += providerID + "#" + assetID;
				addPrivateData(keyStr, intVar);
				
				keyStr = "MotoEcm.keys.";
				keyStr += providerID + "#" + assetID;
				addPrivateData(keyStr, strVar);
				
			}
			else 
				HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(Servant, "Content format error for encription"));
		}
		else 
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(Servant, "Content format error for encription"));
		
		pTemp = strstr(next_content, "a=X-motorola-ecm:");
	}
	
	// DO: flush private data
	if (false == flushPrivateData())
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	
	try
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler, "to perform provision() on weiwoo session: [%s]"), _pContext->weiwooFullID.c_str());
		_sessionPrx->provision();
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetupHandler, "perform provision() on weiwoo session: [%s] successfully"), _pContext->weiwooFullID.c_str());
	}
	catch(const TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform provision() on weiwoo session: [%s] caught [%s]:[%s]", _pContext->weiwooFullID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	catch(Ice::Exception& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform provision() on weiwoo session: [%s] caught [%s]", _pContext->weiwooFullID.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	
	// DO: execute Weiwoo Session's serve().
	try
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler, "to perform serve() on weiwoo session: [%s]"), _pContext->weiwooFullID.c_str());
		_sessionPrx->serve();
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetupHandler, "perform serve() on weiwoo session: [%s] successfully"), _pContext->weiwooFullID.c_str());
	}
	catch(const TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform serve() on weiwoo session: [%s] caught [%s]:[%s]", _pContext->weiwooFullID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	catch(Ice::Exception& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform serve() on weiwoo session: [%s] caught [%s]", _pContext->weiwooFullID.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	try
	{
		_streamPrx = _sessionPrx->getStream();
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform getStream() caught: [%s]", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	try
	{
		_pContext->streamFullID = _ssmNGODr2c1._pCommunicator->proxyToString(_streamPrx);
		_pContext->streamShortID = _streamPrx->getIdent().name;
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "change stream proxy to string caught [%s]", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	SessionRenewCmd* pRenewCmd = new SessionRenewCmd(_ssmNGODr2c1, _pContext->ident.name.c_str(), _pContext->weiwooFullID
		, ZQTianShan::now() + _ssmNGODr2c1._config._timeoutInterval * 1000 + 60000, 0);

	if (false == addContext())
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	_ssmNGODr2c1._pSessionWatchDog->watchSession(_pContext->ident, _ssmNGODr2c1._config._timeoutInterval * 1000);

	pRenewCmd->start();

	char stampUTP[20];
	memset(stampUTP, 0, 20);
	time_t tnow = time(NULL);
	__int64 seconds1900 = __int64(tnow) + 2208988800;
	snprintf(stampUTP, 19, "%lld", seconds1900);

	std::string retContent;
	retContent = "v=0\r\n";
	retContent += std::string("o=- ") + _session + " " + stampUTP + " IN IP4 " + rtspServerIP + "\r\n";
	retContent += "s=\r\n";
	retContent += "t=0 0\r\n";
	retContent += std::string("a=control:rtsp://") + rtspServerIP + ":" + rtspServerPort + "/" + _session + "\r\n";
	
	// DO: Response OK
	_pResponse->setHeader(NGOD_HEADER_TRANSPORT, transportData.c_str());
	_pResponse->setHeader(NGOD_HEADER_CONTENTTYPE, getRequestHeader(NGOD_HEADER_CONTENTTYPE).c_str());
	_pResponse->printf_postheader(retContent.c_str());
	responseOK();

	return RequestProcessed;
}

