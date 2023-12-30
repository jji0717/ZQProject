#include "crm_dsmcc.h"
#include "DsmccDefine.h"
#include "urlstr.h"
#include "TianShanIce.h"
#define ParamRead(_PARAMS, _PARAM_NAME, _strVal, _bMust, _default) \
	{ _strVal = _default; \
	TianShanIce::Properties::iterator itParam = _PARAMS.find(_PARAM_NAME); \
	if (_PARAMS.end() == itParam) { \
	if (_bMust) hlog(ZQ::common::Log::L_ERROR, HLOGFMT(#_PARAMS "has no param[%s]"), _PARAM_NAME); } \
	else _strVal = itParam->second; }

clientses_SetupRHandler::clientses_SetupRHandler(DSMCC_Environment& env, RequestPtr request,TianShanIce::ClientRequest::SessionPrx sess)
: RequestHandler(env, request, NULL, "SETUP"), _dsmccErrorCode(ZQ::DSMCC::RsnOK), _bSuccess(false)
{
	try {
		_method ="SETUP" ;
		if (_req)
			_requestParams = _req->getMessage()->getProperties();

		std::string strTempData;
		lsc::StringMap::const_iterator itorMd;
		for(itorMd = _requestParams.begin(); itorMd != _requestParams.end(); itorMd++)
			strTempData+=itorMd->first + "=" + itorMd->second+ " "; 

		hlog(ZQ::common::Log::L_INFO, CLOGFMT(SetupRHandler, "SetupRequest: %s"),strTempData.c_str());
	}
	catch(...)	{ }

	if (_requestParams.size()<=0)
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("no input parameters"));
	//read the crmPRO
	std::string proType = "";
	ParamRead(_requestParams, CRMetaData_ProtocolType,	  proType,false,"");
	if (proType.empty())
	{
		proType = "0";
	}
	if ("0" == proType)
	{
		_crmProtocolType = ZQ::DSMCC::Protocol_MOTO;
	}
	else if ("1" == proType)
	{
		_crmProtocolType = ZQ::DSMCC::Protocol_Tangberg;
	}

	ParamRead(_requestParams, CRMetaData_SessionId,     _clientSessId, true, "");
	if (ZQ::DSMCC::Protocol_MOTO == _crmProtocolType)
		ParamRead(_requestParams, CRMetaData_assetId,       _assetId,      true, "");
	
	ParamRead(_requestParams, CRMetaData_nodeGroupId,   _nodeGroupId,  true, "");
	ParamRead(_requestParams, CRMetaData_transactionId, _txnId,        true, "");
	ParamRead(_requestParams, CRMetaData_CSSRserverId,  _serverId,     true, "");

	ParamRead(_requestParams, CRMetaData_reserved1, _reserved1, true, "");

	///dsmcc HardHeader
	char buf[64];
	snprintf(buf, sizeof(buf)-2, "%d", ZQ::DSMCC::MsgID_SetupConfirm);
	MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_messageId,  buf);
	MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_reserved1,  _reserved1);
	MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_transactionId, _txnId);

   ///dsmcc setupconfirm
	MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_SessionId, _clientSessId);
	MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_CSSCserverId,  _serverId);

	//add for tangberg
   ///dsmcc serviceGateWay service protocolId protocolVersion
	if (ZQ::DSMCC::Protocol_Tangberg == _crmProtocolType)
	{
		ParamRead(_requestParams, CRMetaData_assetIdPayLoad,       _assetIdPayLoad,      true, "");

		std::string serviceGateWay, service, protocolId, protocolVersion;
		ParamRead(_requestParams, CRMetaData_ServiceGateWay,   serviceGateWay,  true, "");
		ParamRead(_requestParams, CRMetaData_Service, service,        true, "");
		ParamRead(_requestParams, CRMetaData_AppPDprotocolId,  protocolId,     true, "");
		ParamRead(_requestParams, CRMetaData_AppPDprotocolVersion,  protocolVersion,     true, "");

		MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_ServiceGateWay,  serviceGateWay);
		MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_Service,  service);
		MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_AppPDprotocolId, protocolId);
		MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_AppPDprotocolVersion, protocolVersion);
	}

	_stupidCodeForHenNan = -1;
}
clientses_SetupRHandler::~clientses_SetupRHandler()
{
//#ifdef _DEBUG
//	std::cout<< "deconstruct SETUP handler" <<std::endl ;
//#endif
	if(_bSuccess ==  false)
	{
		try
		{
			std::string sessId;
			//destroy weiwoo session
			try
			{	
				if (_svrSess)
				{
					sessId = _svrSess->getId();
					hlog(ZQ::common::Log::L_INFO,CLOGFMT(clientses_SetupRHandler,"setup failed, trying to destroy server session[%s]"), sessId.c_str() );
					_svrSess->destroy();
				}
			}
			catch( const TianShanIce::BaseException& ex)
			{
				hlog(ZQ::common::Log::L_ERROR,CLOGFMT(clientses_SetupRHandler,"setup failed,) session[%s] caught [%s] while destroy weiwoo session"),
					sessId.c_str(), ex.message.c_str());
			}
			catch( const Ice::ObjectNotExistException& ex)
			{
				hlog(ZQ::common::Log::L_ERROR,CLOGFMT(clientses_SetupRHandler,"setup failed, session[%s] caught [%s] while destroy weiwoo session"),
					sessId.c_str(), ex.ice_name().c_str());
			}
			catch( const Ice::Exception& ex)
			{
				hlog(ZQ::common::Log::L_ERROR,CLOGFMT(clientses_SetupRHandler,"setup failed, session[%s] caught [%s] while destroy weiwoo session"),
					sessId.c_str(), ex.ice_name().c_str());
			}
			catch( const std::exception& ex)
			{
				hlog(ZQ::common::Log::L_ERROR,CLOGFMT(clientses_SetupRHandler,"setup failed, session[%s] caught [%s] while destroy weiwoo session]"),
					sessId.c_str(), ex.what());
			}
			catch(...)
			{
				hlog(ZQ::common::Log::L_ERROR,CLOGFMT(clientses_SetupRHandler,"setup failed, session[%s] caught [%s] while destroy weiwoo session"),
					sessId.c_str(), "unknown exception");
			}

			if(_clientSession)
			{
				std::string clientSessId = _clientSession->getSessId();
				hlog(ZQ::common::Log::L_INFO,CLOGFMT(clientses_SetupRHandler,"setup failed, trying to destroy client session[%s]"), clientSessId.c_str() );
				_clientSession->destroy();
			}
		}
		catch (...)
		{

		}
	}
}
ProcessResult clientses_SetupRHandler::doFixupRequest()
{
	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("clientses_SetupRHandler::doFixupRequest()"));
	return RESULT_PROCESSED;
}

ProcessResult clientses_SetupRHandler::doFixupRespone()
{
	hlog(ZQ::common::Log::L_INFO,HLOGFMT("clientses_SetupRHandler::doFixupRespone()"));
	return RESULT_PROCESSED;
}

void clientses_SetupRHandler::addPrivateData(const std::string& key, TianShanIce::Variant& var)
{
	// every key should add a "ClientRequest#" prefix in order to distinguish the private data
	// added by plug-in from other components. andy says
	std::string keyTmp = std::string("ClientRequest#") + key;
	_privateData[keyTmp] = var;
	if (TianShanIce::vtInts == var.type && var.ints.size() > 0)
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("addPrivateData(%s, %d)"), keyTmp.c_str(), var.ints[0]);
	else if (TianShanIce::vtLongs == var.type && var.lints.size() > 0) 
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("addPrivateData(%s, %lld)"), keyTmp.c_str(), var.lints[0]);
	else if (TianShanIce::vtStrings == var.type && var.strs.size() > 0)
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("addPrivateData(%s, %s)"), keyTmp.c_str(), var.strs[0].c_str());
}

void clientses_SetupRHandler::addPrivateData(const std::string& key, const std::string& val)
{
	if (key.empty() || val.empty())
		return;

	TianShanIce::Variant vrtAppData;
	vrtAppData.bRange = false;
	vrtAppData.type = TianShanIce::vtStrings;
	vrtAppData.strs.clear();
	vrtAppData.strs.push_back(val);
	addPrivateData(key, vrtAppData);
}

#define AddPD(_key, _val) addPrivateData(#_key, _val)
void clientses_SetupRHandler::getSessionMgr(TianShanIce::SRM::SessionManagerPrx& sessMgrPrx, std::string& sessMgrEp)
{
	std::string smEndpoint = _CRMDmsccConfig.sessionMgrEndpoint;
	int32 nodeGroupId = atoi(_nodeGroupId.c_str());

	for (NodeGroups::iterator itor = _CRMDmsccConfig.nodegroups.begin(); itor != _CRMDmsccConfig.nodegroups.end(); itor++)
	{
		if(itor->rangeStart <= nodeGroupId && itor->rangeStop >= nodeGroupId)
		{
			smEndpoint = itor->SMEndpoint;
			break;
		}
	}

	// ticket#18298 DONOT narrow to a single persistent SM proxy per each group
	// typedef std::map<std::string, TianShanIce::SRM::SessionManagerPrx>SMEndPoint2Proxys;
	// static SMEndPoint2Proxys smEndpoint2Proxys;

	try 
	{
		//static ZQ::common::Mutex sessMgrLck;
		//ZQ::common::MutexGuard g(sessMgrLck);
		//SMEndPoint2Proxys::iterator itorSM = smEndpoint2Proxys.find(smEndpoint);
		//
		//if (smEndpoint2Proxys.end() != itorSM)
		//{
		//	sessMgrPrx = itorSM->second;
		//}

		//if (!sessMgrPrx)
		//{
			hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("connecting to SessionMgr[%s]"), smEndpoint.c_str());
			sessMgrPrx = TianShanIce::SRM::SessionManagerPrx::checkedCast(mEnv._communicator->stringToProxy(smEndpoint));
		//  MAPSET(SMEndPoint2Proxys, smEndpoint2Proxys, smEndpoint, sessMgrPrx);
		//}

		//TODO:	if( _env._iceOverrideTimeout > 0 )
		//  	sessMgrPrx = TianShanIce::SRM::SessionPrx::uncheckedCast(sessMgrPrx->ice_timeout( _env._iceOverrideTimeout ));

		sessMgrEp = mEnv._communicator->proxyToString(sessMgrPrx);
	}
	catch (TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to connect to SessionMgr[%s], excp[%s]%s(%d): %s"), 
			smEndpoint.c_str(), ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
	}
	catch (Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to connect to SessionMgr[%s], excp[%s]"), smEndpoint.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to connect to SessionMgr[%s] caught exception"), smEndpoint.c_str());
	}
}

bool clientses_SetupRHandler::newServerSession()
{
	::TianShanIce::SRM::Resource sessRC;
	sessRC.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
	sessRC.status = TianShanIce::SRM::rsRequested;
	::TianShanIce::Variant vrtUrl;		
	vrtUrl.bRange = false;
	vrtUrl.type = TianShanIce::vtStrings;
	vrtUrl.strs.clear();
	vrtUrl.strs.push_back(_url);
	sessRC.resourceData["uri"] = vrtUrl;

	TianShanIce::SRM::SessionManagerPrx sessMgrPrx = NULL;
	std::string sessMgrEp = "";
	getSessionMgr(sessMgrPrx, sessMgrEp);

/*	static TianShanIce::SRM::SessionManagerPrx sessMgrPrx;
	static std::string sessMgrEp;

	try 
	{
		static ZQ::common::Mutex sessMgrLck;
		ZQ::common::MutexGuard g(sessMgrLck);
		if (!sessMgrPrx)
		{
			hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("connecting to SessionMgr[%s]"),_CRMDmsccConfig.sessionMgrEndpoint.c_str());
			sessMgrPrx = TianShanIce::SRM::SessionManagerPrx::checkedCast(mEnv._communicator->stringToProxy(_CRMDmsccConfig.sessionMgrEndpoint));
			sessMgrEp = mEnv._communicator->proxyToString(sessMgrPrx);
		}
	}
	catch (TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to connect to SessionMgr[%s], excp[%s]%s(%d): %s"), 
			_CRMDmsccConfig.sessionMgrEndpoint.c_str(), ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
	}
	catch (Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to connect to SessionMgr[%s], excp[%s]"), 
			_CRMDmsccConfig.sessionMgrEndpoint.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to connect to SessionMgr[%s] caught exception"), 
			_CRMDmsccConfig.sessionMgrEndpoint.c_str());
	}
*/
	if (!sessMgrPrx)
	{
		_dsmccErrorCode = ZQ::DSMCC::RspSeNoService;
		return false;
	}

	try {
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("creating a new session on SessionMgr[%s]"), sessMgrEp.c_str());
		_svrSess = sessMgrPrx->createSession(sessRC);
		if (_svrSess)
		{
			_svrSessId = _svrSess->getId();
			_svrSess->renew(_CRMDmsccConfig.heartbeatInterval + 1000);
			std::string svrSessEp =  mEnv._communicator->proxyToString(_svrSess);
			hlog(ZQ::common::Log::L_INFO, HLOGFMT("ServerSession[%s] created"), svrSessEp.c_str());
		}
	}
	catch (TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to create session on SessionMgr[%s], excp[%s]%s(%d): %s"), 
			sessMgrEp.c_str(), ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
	}
	catch (Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to create session on SessionMgr[%s], excp[%s]"), 
			sessMgrEp.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to create session on SessionMgr[%s] caught exception"), 
			sessMgrEp.c_str());
	}

	if (!_svrSess)
		return false;

	try
	{	
		_clientSession->attachWeiwooSession(_svrSess);
	}
	catch(const TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to attachWeiwooSession SessionMgr[%s], excp[%s]%s(%d): %s"), 
			sessMgrEp.c_str(), ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to attachWeiwooSession on SessionMgr[%s], excp[%s]"), 
			sessMgrEp.c_str(), ex.ice_name().c_str());
	}	
	catch(...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to attachWeiwooSession on SessionMgr[%s] caught exception"), 
			sessMgrEp.c_str());
	}
	return true;
}

/*
#define MsgParamBegin() \
	::TianShanIce::Properties msgParams; \
	try { if (!_req) msgParams = _req->getMessage()->getProperties(); } \
	catch(...)	{ } \
	if (msgParams.size()<=0) { \
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("no input parameters")); \
		postResponse(ZQ::DSMCC::RspNeNoSession); return RESULT_DONE; } \
	::TianShanIce::Properties::iterator itParam; \
	{   std::string paramstr; \
		for (itParam= msgParams.begin(); itParam != msgParams.end(); itParam++)	paramstr += itParam->first + "[" + itParam->second + "] "; \
		if (paramstr.length() > 1024) paramstr = paramstr.substr(0, 1010) + "... >>omitted"; \
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("request parameters: %s"), paramstr.c_str()); }
*/

ProcessResult clientses_SetupRHandler::doContentHandler()
{
	// step1. read the parameters from CSSR
#ifdef  _KWG_DEBUG
		_clientSessId="123456789" ;
#endif

	// section 1.1. the required data from the request
	if (_clientSessId.empty() || (_assetId.empty() && _assetIdPayLoad.empty()) || _nodeGroupId.empty() || _txnId.empty())
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("illegal param: assetId[%s] assetPayload[%s]nodeGroupId[%s]  txnId[%s]"), _assetId.c_str(), _assetIdPayLoad.c_str(),_nodeGroupId.c_str(), _txnId.c_str());
		postResponse(ZQ::DSMCC::RspNeNoSession);
		return RESULT_DONE;
	}

	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("SETUP param: assetId[%s] nodeGroupId[%s] txnId[%s]"), _assetId.c_str(), _nodeGroupId.c_str(), _txnId.c_str());
	AddPD(clientSessionId, _clientSessId);
	addPrivateData("node-group-id", _nodeGroupId);
	addPrivateData("assetUID",      _assetId);

	// section 1.2. the optional application data
	std::string appval,clientID;
	ParamRead(_requestParams, CRMetaData_CSSRclientId,	  clientID,false,"");
	if( ZQ::DSMCC::Protocol_MOTO == _crmProtocolType)
	{
		ParamRead(_requestParams, CRMetaData_billingId,         appval, false, ""); addPrivateData("billing-id",        appval);
		ParamRead(_requestParams, CRMetaData_purchaseTime,      appval, false, ""); addPrivateData("purchase-time",     appval);
		ParamRead(_requestParams, CRMetaData_remainingPlayTime, appval, false, ""); addPrivateData("remaining-play-time",appval);
		ParamRead(_requestParams, CRMetaData_errorCode,         appval, false, ""); addPrivateData("errorCode",        appval);
		ParamRead(_requestParams, CRMetaData_homeId,            appval, false, ""); addPrivateData("home-id",           appval);
		ParamRead(_requestParams, CRMetaData_purchaseId,        appval, false, ""); addPrivateData("purchase-id",       appval);
		ParamRead(_requestParams, CRMetaData_analogCopyPurchase,appval, false, ""); addPrivateData("analogCopyPurchase", appval);
		if (0 == appval.compare("255"))
			addPrivateData("skip-ads",std::string("1"));

		ParamRead(_requestParams, CRMetaData_smartCardId,       appval, false, "0");
		std::string smartcardID = appval;
		if (appval.length() <= 1 && clientID.length()>=19)
		{
			smartcardID =  clientID.substr(2,17);
		}
		addPrivateData("smartcard-id",  smartcardID);
	}
	ParamRead(_requestParams, CRMetaData_packageId,         appval, false, ""); addPrivateData("packageId",         appval);

	// step 2 create the client session by CRMetaData_SessionId and clientId
	std::string macAddressId="0c0c0c0c0c0c";
	if (_clientSessId.length() > 6*2)
		macAddressId = _clientSessId.substr(0, 6*2);
	addPrivateData("mac-address", macAddressId);
	addPrivateData("device-id", macAddressId);

	// NOTE: if you pass a sessId which has already been existed in gateway, null is returned
	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("creating session[%s] for client[%s]"), _clientSessId.c_str(), macAddressId.c_str());
	_clientSession = mEnv.mGateway->createSession(_clientSessId, macAddressId);
	if (!_clientSession)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to create new session[%s] client[%s]"), _clientSessId.c_str(), macAddressId.c_str());
		postResponse(ZQ::DSMCC::RspSeNoService);
		return RESULT_DONE;
	}
	_connId = _req->getConnectionId();
	_clientSession->updateTimer(_CRMDmsccConfig.heartbeatInterval + 300000);

	//add the protocolType to clientSession
	char strProType[8] = "";
	snprintf(strProType, 8, "%d", _crmProtocolType);
	_clientSession->setProperty(CRMetaData_ProtocolType, strProType);
	if (ZQ::DSMCC::Protocol_MOTO == _crmProtocolType)
		postCSPI();

	std::string streamHandle, lscIp, lscPort;
	bool lscUdpMode =true;
	::TianShanIce::Properties clientSessParams = _clientSession->getProperties();
	{
		std::string lscEndpoint;
		ParamRead(clientSessParams, SESS_PROP_STREAMHANDLE, streamHandle, true, "");
		ParamRead(clientSessParams, SESS_PROP_LSC_UDP,      lscEndpoint,  false, "");

		if (lscEndpoint.empty() || !_CRMDmsccConfig.lscUdp)
		{
			lscUdpMode  =false;
			ParamRead(clientSessParams, SESS_PROP_LSC_TCP,  lscEndpoint,  false, "");
		}

		if (lscEndpoint.empty())
		{
			hlog(ZQ::common::Log::L_ERROR, HLOGFMT("neither TCP nor UDP LSC server available"));
			postResponse(ZQ::DSMCC::RspNeNoResource);
			return RESULT_DONE;
		}

		size_t pos = lscEndpoint.find(';');
		if (std::string::npos != pos && pos>0)
			lscEndpoint = lscEndpoint.substr(0, pos);

		pos = lscEndpoint.find(':');
		if (std::string::npos != pos && pos>0)
		{
			lscIp = lscEndpoint.substr(0, pos);
			lscPort = lscEndpoint.substr(pos+1);
		}
		else
		{
			lscIp = lscEndpoint;
			lscPort = "3586"; // the default port
		}
	}
	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("StrmHandle[%s] %s[%s:%s]"), streamHandle.c_str(), (lscUdpMode?"UDP":"TCP"), lscIp.c_str(), lscPort.c_str());

	// step 3 - determine the URL, appId, IP connection
	_url = _CRMDmsccConfig.appURLPattern;
  	size_t pos =0;
  	for ( pos= _url.find("${Asset}", pos); std::string::npos != pos; pos= _url.find("${Asset}", pos))
 		_url.replace(pos, sizeof("${Asset}")-1, _assetId);
	pos = 0;
 	for ( pos= _url.find("${purchaseToken}", pos); std::string::npos != pos; pos= _url.find("${purchaseToken}", pos))
 		_url.replace(pos, sizeof("${purchaseToken}")-1, _assetIdPayLoad);

	pos = 0;
	for(pos = _url.find("${ServerID}", pos); std::string::npos != pos; pos= _url.find("${ServerID}", pos))
		_url.replace(pos, sizeof("${ServerID}")-1, _serverId);
	std::string appId;
	{
		ZQ::common::URLStr urlStr(_url.c_str(), true);
		const char* pAppID = urlStr.getPath();
		appId = (NULL != pAppID && strlen(pAppID)>0) ? pAppID : "60010000";
	}

	AddPD(OriginalUrl,   _url);
	AddPD(application-id, appId);

    //add for Tangberg
	AddPD(purchaseToken, _assetIdPayLoad);
	AddPD(serverID, _serverId);
	AddPD(qam_name, _nodeGroupId);
	AddPD(client, macAddressId);
	{
		std::string peerIp, peerPort, localIp, localPort;
		_req->getPeerInfo(peerIp, peerPort);
		_req->getLocalInfo(localIp, localPort);

		char buf[64];
		snprintf(buf, sizeof(buf)-2, "%lld", _connId);
		AddPD(clientConn,    buf);
		_clientSession->setProperty(SESS_PROP_DSMCC_CONNID, buf);
		_clientSession->setProperty("dsmcc_peer_ip",peerIp);
		_clientSession->setProperty("dsmcc_peer_port",peerPort);
		
		if (!peerPort.empty())
			peerPort = std::string(":") + peerPort;
		snprintf(buf, sizeof(buf)-2, "%s%s", peerIp.c_str(), peerPort.c_str());
		AddPD(clientAddress, buf);

		if (!localPort.empty())
			localPort = std::string(":") + localPort;
		snprintf(buf, sizeof(buf)-2, "%s%s", localIp.c_str(), localPort.c_str());
		AddPD(localAddress, buf);

		hlog(ZQ::common::Log::L_INFO, HLOGFMT("client peer[%s:%s] => local[%s:%s]: url[%s] appId[%s]"),
			peerIp.c_str(), peerPort.c_str(), localIp.c_str(), localPort.c_str(), _url.c_str(), appId.c_str());
	}

	// step 4 - creating the server session
	if (!clientses_SetupRHandler::newServerSession() || !_svrSess)
	{
		if (ZQ::DSMCC::RsnOK == _dsmccErrorCode)
			_dsmccErrorCode = ZQ::DSMCC::RspSeNoService;

		postResponse(ZQ::DSMCC::RspSeNoService);
		return RESULT_DONE;
	}

	// no resource needed for QAM mode that DSMCC only supports

	// step 4.2 flush the private datas to weiwoo session
	try 
	{
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("flushing private data to server session[%s]"), _svrSessId.c_str());
		_dsmccErrorCode = ZQ::DSMCC::RspSeNoService;

		_svrSess->setPrivateData2(_privateData);
		_dsmccErrorCode = ZQ::DSMCC::RsnOK;
	}
	catch (TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to flush private data to server session[%s], excp[%s] %s(%d): %s"), _svrSessId.c_str(),
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
	}
	catch (Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to flush private data to server session[%s], excp[%s]"), _svrSessId.c_str(),
			ex.ice_name().c_str());
	}
	catch (...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to flush private data to server session[%s], exception"), _svrSessId.c_str());
	}

	if (ZQ::DSMCC::RsnOK != _dsmccErrorCode)
	{
		postResponse(_dsmccErrorCode);
		return RESULT_DONE;
	}

	// step 5 - call to provision server session
	try
	{
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("calling Session[%s] provision()"), _svrSessId.c_str());
		_dsmccErrorCode = ZQ::DSMCC::RspSeNoService;

		Ice::Long timeUsed = ZQTianShan::now();
		_svrSess->provision();
		timeUsed = ZQTianShan::now() - timeUsed;
		hlog(ZQ::common::Log::L_INFO, HLOGFMT("Session[%s] provision successfully, used [%lld]ms"), _svrSessId.c_str(), timeUsed);
		_dsmccErrorCode = ZQ::DSMCC::RsnOK;
	}
	catch (TianShanIce::SRM::InvalidResource& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] provision failed, excp[%s] %s(%d): %s"), _svrSessId.c_str(), 
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
		_dsmccErrorCode = ZQ::DSMCC::RspNeNoResource;
		_stupidCodeForHenNan = 453;//Not Enough Bandwidth
	}
	catch (TianShanIce::InvalidParameter& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] provision failed, excp[%s] %s(%d): %s"), _svrSessId.c_str(), 
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
		_dsmccErrorCode = ZQ::DSMCC::RspNeNoResource;
		_stupidCodeForHenNan = 451; //Invalid Parameter
	}
	catch(TianShanIce::ServerError& ex)//for nss  ,via thowing an exception 
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] provision failed, excp[%s] %s(%d): %s"), _svrSessId.c_str(), 
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());

		if(ex.errorCode>= 400 && ex.errorCode<=800)
		{
			_stupidCodeForHenNan = ex.errorCode;
		}

		if (("RtspProxying" == ex.category &&  ex.errorCode == 771)  || ("ModPurchaseImpl" == ex.category &&  ex.errorCode == 310) )
			_dsmccErrorCode = ZQ::DSMCC::RspAssetNotFound; //0x9100;
		else
			_dsmccErrorCode = ZQ::DSMCC::RspNeNoResource;
	}
	catch (TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] provision failed, excp[%s] %s(%d): %s"), _svrSessId.c_str(), 
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
		_dsmccErrorCode = ZQ::DSMCC::RspNeNoResource;
		_stupidCodeForHenNan = 453;//Not Enough Bandwidth
	}
	catch (Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] provision failed, excp[%s]"), _svrSessId.c_str(), 
			ex.ice_name().c_str());
		_dsmccErrorCode = ZQ::DSMCC::RspNeNoResource;
		_stupidCodeForHenNan = 453;//Not Enough Bandwidth
	}
	catch (...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] provision failed, caught excp"), _svrSessId.c_str());
		_dsmccErrorCode = ZQ::DSMCC::RspNeNoResource;
		_stupidCodeForHenNan = 453;//Not Enough Bandwidth
	}

	if (ZQ::DSMCC::RsnOK != _dsmccErrorCode)
	{
		postResponse(_dsmccErrorCode);
		return RESULT_DONE;
	}

	// step 6 - calling Session::serve()
	try
	{
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("calling Session[%s] serve()"), _svrSessId.c_str());
		_dsmccErrorCode = ZQ::DSMCC::RspSeNoService;

		Ice::Long timeUsed = ZQTianShan::now();
		_svrSess->serve();
		_svrSess->renew(_CRMDmsccConfig.heartbeatInterval + 1000);
		_clientSession->updateTimer(_CRMDmsccConfig.heartbeatInterval + 1000);
		hlog(ZQ::common::Log::L_INFO, HLOGFMT("Session[%s] serve() successfully, used [%lld]ms"), _svrSessId.c_str(), ZQTianShan::now()-timeUsed);
		_dsmccErrorCode = ZQ::DSMCC::RsnOK;
	}
	catch (TianShanIce::SRM::InvalidResource& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] serve failed, excp[%s] %s(%d): %s"), _svrSessId.c_str(), 
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
		_dsmccErrorCode = ZQ::DSMCC::RspNeNoResource;
       _stupidCodeForHenNan = 453;//Not Enough Bandwidth
	}
	catch (TianShanIce::InvalidParameter& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] serve failed, excp[%s] %s(%d): %s"), _svrSessId.c_str(), 
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
		_dsmccErrorCode = ZQ::DSMCC::RspNeNoResource;
		_stupidCodeForHenNan = 451; //Invalid Parameter
	}
	catch(TianShanIce::ServerError& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] serve failed, excp[%s] %s(%d): %s"), _svrSessId.c_str(), 
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
		if(ex.errorCode>= 400 && ex.errorCode<=800)
		{
			_stupidCodeForHenNan = ex.errorCode;
		}
		if (("RtspProxying" == ex.category &&  ex.errorCode == 771)  || ("ModPurchaseImpl" == ex.category &&  ex.errorCode == 310) )
		{
			if (ZQ::DSMCC::Protocol_MOTO == _crmProtocolType)
			{
				_dsmccErrorCode = ZQ::DSMCC::RspAssetNotFound; //0x9100;
			}
			else if (ZQ::DSMCC::Protocol_Tangberg == _crmProtocolType)
			{
				//this errorcode for Tangberg mean AssetId not found  0x0020
				_dsmccErrorCode = ZQ::DSMCC::RspSeNoResource;//0x0020
			}
		}
		else if("AccreditedPaths" == ex.category &&  (ex.errorCode == 1172 || ex.errorCode == 1180))
		{
			if (ZQ::DSMCC::Protocol_MOTO == _crmProtocolType)
			{
				_dsmccErrorCode = ZQ::DSMCC::RspNeNoResource;// 0x0019
			}
			else if (ZQ::DSMCC::Protocol_Tangberg == _crmProtocolType)
			{
				//this errorcode for Tangberg mean serviceGroup not found 0x0019
				_dsmccErrorCode = ZQ::DSMCC::RspNeNoResource; //0x0019
			}
		}
		else
		{
			if (ZQ::DSMCC::Protocol_MOTO == _crmProtocolType)
			{
				_dsmccErrorCode = ZQ::DSMCC::RspNeNoResource;//0x0019
			}
			else if (ZQ::DSMCC::Protocol_Tangberg == _crmProtocolType)
			{
				_dsmccErrorCode = ZQ::DSMCC::RSpSeNoBW; //0x0012
			}
		}
	}
	catch (TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] serve failed, excp[%s] %s(%d): %s"), _svrSessId.c_str(), 
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
		_dsmccErrorCode = ZQ::DSMCC::RspNeNoResource;
		_stupidCodeForHenNan = 453;//Not Enough Bandwidth
		
	}
	catch (Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] serve failed, excp[%s]"), _svrSessId.c_str(), 
			ex.ice_name().c_str());
	}
	catch (...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] serve failed, caught excp"), _svrSessId.c_str());
	}

	if (ZQ::DSMCC::RsnOK != _dsmccErrorCode)
	{
		postResponse(_dsmccErrorCode);
		return RESULT_DONE;
	}

	// step 7 - attaching the resolved Stream
	TianShanIce::Properties streamParams;
	TianShanIce::ValueMap privateData;
	try
	{
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("linking stream of Session[%s]"), _svrSessId.c_str());
		_dsmccErrorCode = ZQ::DSMCC::RspSeNoService;

		_stream = _svrSess->getStream();
		_streamId = _stream->getIdent().name;
		_clientSession->attachStreamSession(_stream, _streamId);

		streamParams = _stream->getProperties();

		privateData = _svrSess->getPrivateData();

		hlog(ZQ::common::Log::L_INFO, HLOGFMT("Session[%s] stream[%s] linked"), _svrSessId.c_str(), _streamId.c_str());
		_dsmccErrorCode = ZQ::DSMCC::RsnOK;

	}
	catch (TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] link stream failed, excp[%s] %s(%d): %s"), _svrSessId.c_str(),
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
	}
	catch (Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] link stream failed, excp[%s]"), _svrSessId.c_str(),
			ex.ice_name().c_str());
	}
	catch (...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] link stream failed, excp[%s]"), _svrSessId.c_str());
	}

	if (ZQ::DSMCC::RsnOK != _dsmccErrorCode)
	{
		postResponse(_dsmccErrorCode);
		return RESULT_DONE;
	}

	/// step 8 - preparing the response parameters
	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("Session[%s] preparing response parameters"), _svrSessId.c_str());

	// get server session's resource
	TianShanIce::SRM::ResourceMap rsMap;
	try
	{
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("Session[%s] reading resources"), _svrSessId.c_str());
		_dsmccErrorCode = ZQ::DSMCC::RspSeNoService;

		rsMap = _svrSess->getReources();
		_dsmccErrorCode = ZQ::DSMCC::RsnOK;
	}
	catch(const TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] reading resource failed, excp[%s] %s(%d): %s"), _svrSessId.c_str()
			, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
	}
	catch(Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] reading resource failed, excp[%s]"), _svrSessId.c_str()
			, ex.ice_name().c_str());
	}
	catch(...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("Session[%s] reading resource failed, caught excp"), _svrSessId.c_str());
	}

	if (ZQ::DSMCC::RsnOK != _dsmccErrorCode)
	{
		try {
			_svrSess->destroy();
			mEnv.mStreamingInfoCache.deleteInfo(_clientSessId);
			_clientSession->destroy();
			_svrSess = NULL;
			_clientSession = NULL;
			hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("Session[%s] and local context withdrawn"), _svrSessId.c_str());
		}
		catch (...) {}

		postResponse(_dsmccErrorCode);
		return RESULT_DONE;
	}

	int64 appliedBW  = 0;
	ZQTianShan::Util::getResourceDataWithDefault(rsMap,TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth",0,appliedBW);
    
	// about _dsmccResources
	static struct { uint16 dsmccId; ::TianShanIce::SRM::ResourceType tsRt; } wishedResouceList[] = {
		{ZQ::DSMCC::RsrcType_MPEG_PROGRAM,          TianShanIce::SRM::rtMpegProgram}, 
		{ZQ::DSMCC::RsrcType_TS_DOWNSTREAM_BW,      TianShanIce::SRM::rtTsDownstreamBandwidth}, 
		{ZQ::DSMCC::RsrcType_SA_ETHERNET_INTERFACE, TianShanIce::SRM::rtEthernetInterface}, 
		{ZQ::DSMCC::RsrcType_PHYSICAL_CHANNEL,      TianShanIce::SRM::rtPhysicalChannel}, 
		{ZQ::DSMCC::RsrcType_SA_ATSC_MODULATION_MODE, TianShanIce::SRM::rtAtscModulationMode}, 
		{ZQ::DSMCC::RsrcType_SA_HEAD_END_ID,        TianShanIce::SRM::rtHeadendId}, 
		{0xffff, (::TianShanIce::SRM::ResourceType)0}, 
	};

	for (int i=0; 0xffff != wishedResouceList[i].dsmccId && 0 != wishedResouceList[i].tsRt; i++)
	{
		TianShanIce::SRM::ResourceMap::iterator itRes = rsMap.find(wishedResouceList[i].tsRt);
		if (rsMap.end() == itRes)
			continue;

		ZQ::DSMCC::DsmccResource dsmccRes;
		dsmccRes.resource = itRes->second;
		if(_CRMDmsccConfig.convertFreq && wishedResouceList[i].tsRt == TianShanIce::SRM::rtPhysicalChannel)
		{
			TianShanIce::ValueMap::iterator itorVM;
			itorVM = dsmccRes.resource.resourceData.find("channelId");
			if(itorVM != dsmccRes.resource.resourceData.end() && itorVM->second.type == TianShanIce::vtInts && itorVM->second.bRange == false && !(itorVM->second.ints.empty()))
				itorVM->second.ints[0] = itorVM->second.ints[0] * 1000;        
		}

		char buf[10];
		sprintf(buf, "%d", wishedResouceList[i].dsmccId);
		dsmccRes.resCommonHeader.insert(TianShanIce::Properties::value_type(CRMetaData_RESresDescriptorType, buf));
		_dsmccResources.push_back(dsmccRes);
	}

	// about _confirmParams
	if(privateData.end()!= privateData.find(SYS_PROP(primaryStart)) )
	{
		TianShanIce::Variant& varPrimaryStart =  privateData[SYS_PROP(primaryStart)];
		if(varPrimaryStart.lints.size() > 0)
		{  
			char strTemp[65]={0};
			sprintf(strTemp, "%lld", varPrimaryStart.lints[0]);
			hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("got primaryStart[%s]"),strTemp);
			// add to _confirmParams
			MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_analogCopyPurchase, std::string(strTemp));
		}
	}

//	if (streamParams.find("sys.fullPlayTime") != streamParams.end())
	if(privateData.find(SYS_PROP(fullPlayTime)) != privateData.end())
	{
//		std::string& val = streamParams["sys.primaryItemNPT"];
//		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("Session[%s] got primaryItemNPT[%s]"), _svrSessId.c_str(), val.c_str());
		TianShanIce::Variant& varRemainingPlayTime =  privateData[SYS_PROP(fullPlayTime)];
		if(varRemainingPlayTime.lints.size() > 0)
		{
            char strTemp[65];
			sprintf(strTemp, "%lld", varRemainingPlayTime.lints[0]);
			// add to _confirmParams
			MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_remainingPlayTimeF, std::string(strTemp));
			MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_remainingPlayTime, std::string(strTemp));
			_clientSession->setProperty(SYS_PROP(fullPlayTime), std::string(strTemp));
			hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("got fullPlayTime[%s]"),strTemp);
		}
		else
		{
			MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_remainingPlayTimeF, "0");
			MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_remainingPlayTime, "0");
		}
	}

	{
		if (_CRMDmsccConfig.heartbeatInterval < 1000*20)
			_CRMDmsccConfig.heartbeatInterval = 1000*20;

		char buf[20];
		// ticket#11384, some stupic STB take exact heartbeatInterval to issue a heartbeat so that the sever could accidently miss it in this measurement
		// so give a bit shorter as the interval to the STB
		snprintf(buf, sizeof(buf)-2, "%d", (_CRMDmsccConfig.heartbeatInterval - _CRMDmsccConfig.optionalInterval)/1000 );
		MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_heartbeat, buf);
		hlog(ZQ::common::Log::L_INFO, HLOGFMT("_CRMDmsccConfig.heartbeatInterval[%d] - _CRMDmsccConfig.optionalInterval[%d])/1000=%s"), _CRMDmsccConfig.heartbeatInterval , _CRMDmsccConfig.optionalInterval,buf);
	}
//	MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_remainingPlayTimeF, "1200000");
	MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_StreamHandelId, streamHandle);
	MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_IPaddress,      lscIp);
	MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_Port,           lscPort);
	MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_lscpIpProtocol, lscUdpMode?"1":"0");

#pragma message("TODO: get offerID and purchaseID, add in confirmParams map")
///////////modify  by tangberg
	if (ZQ::DSMCC::Protocol_Tangberg == _crmProtocolType)
	{
		MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_offeringId,      _assetIdPayLoad);
		MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_purchaseId,      "0"); 
		MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_responseType,    "5");
	}

	std::string strTempData;
	TianShanIce::Properties::iterator itorMd;
	for(itorMd = _confirmParams.begin(); itorMd != _confirmParams.end(); itorMd++)
		strTempData+=itorMd->first + "=" + itorMd->second+ " "; 
	//hlog(ZQ::common::Log::L_INFO,HLOGFMT("dsmcc_session_setup[RESPONSE]: [%s] [%s]") , itorMd->first.c_str(),itorMd->second.c_str());
	hlog(ZQ::common::Log::L_INFO,HLOGFMT("dsmcc_session_setup[RESPONSE]: %s"),strTempData.c_str());

	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("Session[%s] issuing SetupConfirm"), _svrSessId.c_str());
	
	if(postResponse(_dsmccErrorCode))
		_bSuccess = true;

	mEnv.mStreamingInfoCache.updateInfo(_clientSessId,appliedBW);

	hlog(ZQ::common::Log::L_INFO, HLOGFMT("Session[%s] SETUP completed, response issued"), _svrSessId.c_str());
	hlog(ZQ::common::Log::L_INFO, HLOGFMT("request processed (%dms,success)"), (int)(ZQ::common::now() - _startTime));

	return RESULT_DONE;
}

static void cbPrintLine(const char* line, void* pCtx)
{
	if (NULL == pCtx || NULL ==line)
		return;

	ZQ::common::Log* pLog = (ZQ::common::Log*) pCtx;
	(*pLog)(ZQ::common::Log::L_DEBUG, line);
}

bool clientses_SetupRHandler::postResponse(int16 dsmccErr)
{
	if(_stupidCodeForHenNan > 0 && ZQ::DSMCC::RsnOK != dsmccErr && _crmProtocolType == ZQ::DSMCC::Protocol_Tangberg)
	{
		MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_responseType,    "1");

		char buf[64];
		snprintf(buf, sizeof(buf) -2, "%d", _stupidCodeForHenNan);
		MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_StupidCodeForHeNan,    buf);
	}

	_dsmccErrorCode = dsmccErr;

	ResponsePtr pResp = _req->getResponse();
	if (!pResp)
		return false;

	WritableMessagePtr pOutMsg = pResp->getMessage();
	if (!pOutMsg)
		return false;

	char buf[64];
	snprintf(buf, sizeof(buf) -2, "%d", _dsmccErrorCode);
	MAPSET(TianShanIce::Properties, _confirmParams, CRMetaData_CSSCresponse, buf);
	pOutMsg->setProperties(_confirmParams);
	pOutMsg->setCommand(COMMAND_SETUP_RESPONSE);
	if (ZQ::DSMCC::RsnOK != _dsmccErrorCode)
	{
		hlog(ZQ::common::Log::L_INFO, HLOGFMT("Session[%s] setup failed, took %dmsec. sending CSSC with error[%d]"), _svrSessId.c_str(), (int)(ZQ::common::now() - _startTime), _dsmccErrorCode);
		pResp->complete(_dsmccErrorCode);
		return true;
	}

// #pragma message(__MSGLOC__"TODO: fillin the succ SetupConfirm parameters here")
	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("Session[%s] preparing CSSC"), _svrSessId.c_str());
	std::string tmpStr;
	for (ZQ::DSMCC::DsmccResources::iterator itRes = _dsmccResources.begin(); itRes <_dsmccResources.end(); itRes++)
	{
		tmpStr = _clientSessId.empty()?"0":_clientSessId;
		for (TianShanIce::Properties::iterator it = itRes->resCommonHeader.begin(); it != itRes->resCommonHeader.end(); it++)
			tmpStr += it->first + "[" + it->second + "] ";
		ZQTianShan::dumpResource(itRes->resource,  tmpStr.c_str(), cbPrintLine, (void*) &hlog);
	}
	for (TianShanIce::Properties::iterator it = _confirmParams.begin(); it !=_confirmParams.end(); it++)
		tmpStr += it->first + "[" + it->second + "] ";

	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("Session[%s] CSSC params:"), _svrSessId.c_str(), tmpStr.c_str());
	pOutMsg->setResources(_dsmccResources);

	pOutMsg->setProperties(_confirmParams);

	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("Session[%s] sending CSSC"), _svrSessId.c_str());
	pResp->complete(_dsmccErrorCode);
	return true;
}

void clientses_SetupRHandler::postCSPI()
{
	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("posting SessionProceedingIndication message to client"));

	::TianShanIce::Properties cspiParams;

	char buf[64];
	snprintf(buf, sizeof(buf)-2, "%d", ZQ::DSMCC::MsgID_ProceedingIndication);
	MAPSET(TianShanIce::Properties, cspiParams, CRMetaData_messageId,  buf);

	MAPSET(TianShanIce::Properties, cspiParams, CRMetaData_SessionId, _clientSessId);
	MAPSET(TianShanIce::Properties, cspiParams, CRMetaData_CSPIreason, "21");

	if (NULL == mEnv.mGateway || NULL == _clientSession)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to post SessionProceedingIndication message to client, NULL server or session"));
		return;
	}

	WritableMessagePtr msg;
	ServerRequestPtr cspi = mEnv.mGateway->createServerRequest(_connId, _clientSession);
	if (NULL != cspi)
		msg = cspi->getMessage();

	if (NULL == cspi || NULL == msg)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to post SessionProceedingIndication message to client, NULL ServerRequest or message"));
		return;
	}

	std::string strTempData;
	TianShanIce::Properties::iterator itorMd;
	for(itorMd = cspiParams.begin(); itorMd != cspiParams.end(); itorMd++)
		strTempData+=itorMd->first + "=" + itorMd->second+ " "; 
	//hlog(ZQ::common::Log::L_INFO,HLOGFMT("dsmcc_CSPI[RESPONSE]: [%s] [%s]") , itorMd->first.c_str(),itorMd->second.c_str());
	hlog(ZQ::common::Log::L_INFO,HLOGFMT("dsmcc_CSPI[RESPONSE]: %s"),strTempData.c_str());

	std::string peerIp,peerPort;
	_req->getPeerInfo(peerIp,peerPort);
	cspi->updatePeerInfo(peerIp,peerPort);

	
	msg->setProperties(cspiParams);
	msg->setCommand(COMMAND_PROCEEDING_INDICATION);
	cspi->complete();
	if (_clientSession)
		_clientSession->updateTimer(_CRMDmsccConfig.heartbeatInterval);

	hlog(ZQ::common::Log::L_INFO, HLOGFMT("posted SessionProceedingIndication message to client"));
}


// -----------------------------
// class clientses_InProgressHandler
// -----------------------------
clientses_InProgressRHandler::clientses_InProgressRHandler(DSMCC_Environment& env, RequestPtr request, TianShanIce::ClientRequest::SessionPrx sess)
:  RequestHandler(env, request, sess, "CSIP")
{
	_method ="CSIP" ;
#ifdef _DEBUG
//	std::cout<<" construct CSIP handler" <<std::endl ;
#endif
}
clientses_InProgressRHandler::~clientses_InProgressRHandler()
{
#ifdef _DEBUG
//	std::cout<<" deconstruct CSIP handler" <<std::endl ;
#endif
};

ProcessResult clientses_InProgressRHandler::doContentHandler() 
{
	try {
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("renewing server session"));

		ZQ::DSMCC::StringMap metadatas = _req->getMessage()->getProperties(); 
		std::string sessionCount = "0";
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_CSPRsessionCount,"0",sessionCount);
		uint sesCount = atoi(sessionCount.c_str());
		if(sesCount > 0)
		{
			std::string strKey = CSPRSessionID_ +std::string("0");
			ZQTianShan::Util::getPropertyDataWithDefault(metadatas,strKey,"-1",_clientSessId);
		}
		//
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("opening clientSession[%s]"), _clientSessId.c_str());
		_clientSession = mEnv.mGateway->openSession(_clientSessId);
		if (!_clientSession)
		{
			//Log Warning
			hlog(ZQ::common::Log::L_ERROR,  HLOGFMT("failed to open clientSession Proxy [SessionId:%s]"),_clientSessId.c_str());//md_header_LStreamHandle.c_str()
			return RESULT_DONE;
		}

		TianShanIce::SRM::SessionPrx svrSess = openAttachedServerSession();
		if (!svrSess)
		{
			hlog(ZQ::common::Log::L_WARNING, HLOGFMT("failed to address the attached server session"));
			return RESULT_DONE;
		}

		_clientSession->updateTimer(_CRMDmsccConfig.heartbeatInterval + 1000);
		svrSess->renew(_CRMDmsccConfig.heartbeatInterval + 1000);
		hlog(ZQ::common::Log::L_INFO, HLOGFMT("renewed server session for heartbeat[%d]msec"), _CRMDmsccConfig.heartbeatInterval);
	}
	catch (TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to renew server session, excp[%s] %s(%d): %s"),
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
	}
	catch (Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to renew server session, excp[%s]"), ex.ice_name().c_str());
	}
	catch (...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to renew server session, caught excp"));
	}
	return RESULT_DONE ;
}
