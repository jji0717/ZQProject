#include "crm_dsmcc.h"


clientses_ReleaseRHandler::clientses_ReleaseRHandler(DSMCC_Environment& env,RequestPtr request,TianShanIce::ClientRequest::SessionPrx sess)
:RequestHandler(env,request,sess,"TEARDOWN")
{	
	_method = "TEARDOWN";
#ifdef _DEBUG
	std::cout<<"construct TEARDOWN handler"<<std::endl;
#endif

}
clientses_ReleaseRHandler::~clientses_ReleaseRHandler()
{
#ifdef _DEBUG
	std::cout<<"deconstruct TEARDOWN handler"<<std::endl;
#endif

}

void  clientses_ReleaseRHandler::client_SessionDestroy()
{
	
	try
	{
		mEnv.mStreamingInfoCache.deleteInfo(_clientSessId);
		_clientSession->destroy();
		hlog(ZQ::common::Log::L_INFO, HLOGFMT("destory session[%s] successfully"), _clientSessId.c_str());
	}
	catch(const TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to destory session[%s] caugth exception [%s][%s][%d][%s]"),_clientSessId.c_str(),
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to destory session[%s] caugth exception [%s]"),_clientSessId.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to destroy  session caught unknown exception(%d)"),  SYS::getLastErr());
	}
}
//
void clientses_ReleaseRHandler::composeErrorResponse(uint32 resultCode)
{
	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("entry clientses_ReleaseRHandler::composeErrorResponse()")) ;

	char temp[20] = {0}; 
	itoa(ZQ::DSMCC::MsgID_ReleaseConfirm, temp, 10);
	MAPSET(Properties, loc_resposedata, CRMetaData_messageId,     temp);
	MAPSET(Properties, loc_resposedata, CRMetaData_SessionId,     loc_crmetadatasessionId);
	MAPSET(Properties, loc_resposedata, CRMetaData_transactionId, loc_txnId);
	MAPSET(Properties, loc_resposedata, CRMetaData_reserved1,     _reserved1);

	if (resultCode >0)
	{
		snprintf(temp, sizeof(temp)-2, "%d", resultCode);
		MAPSET(Properties, loc_resposedata, CRMetaData_CSRreason, temp);
	}

	RequestHandler::postResponse(COMMAND_DESTROY_RESPONSE, resultCode);
}

//
ProcessResult clientses_ReleaseRHandler::doFixupRequest()
{
	//hlog(ZQ::common::Log::L_INFO,CLOGFMT(clientses_ReleaseRHandler,"doFixupRequest()"));
	return   RESULT_DONE ;
}

//
ProcessResult clientses_ReleaseRHandler::doContentHandler()
{
	int64 lStart = ZQ::common::now();

	ZQ::DSMCC::StringMap metadatas = _req->getMessage()->getProperties(); 
	ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_transactionId,"0",loc_txnId);
	ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_SessionId,"-1",_clientSessId);
	ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_reserved1,"256",_reserved1);

#ifdef  _KWG_DEBUG
	_clientSessId="123456789" ;
#endif
	if ("-1" == loc_crmetadatasessionId)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("missing sessionId in Session release request"));
		return RESULT_PROCESSED;
	}

	_clientSession = mEnv.mGateway->openSession(_clientSessId);
	if (!_clientSession)
	{//Log Warning
		hlog(ZQ::common::Log::L_ERROR,HLOGFMT("failed to open client session proxy by sessionId[%s]"),_clientSessId.c_str());//md_header_LStreamHandle.c_str()
		composeErrorResponse(ZQ::DSMCC::RspNeNoSession);//0x0005
		return RESULT_DONE;
	}
	_clientSession->updateTimer(_CRMDmsccConfig.heartbeatInterval);

	_connId = _req->getConnectionId();
//	postCSRI();  //error, only receive one setup message.????????????????????????????????

	//_svrSess =_clientSession->getWeiwooSession();
	//if (_svrSess == NULL)
	//{   //0x0008 Indicates that the server rejected the request because the requested service could not be provided.
	//	hlog(ZQ::common::Log::L_WARNING,HLOGFMT("failed to get weiwoo session object"));
	//	composeErrorResponse(8);//0x0008
	//	client_SessionDestroy();
	//	return RESULT_DONE ;
	//}
	
    try
	{
		_svrSess =_clientSession->getWeiwooSession();
		hlog(ZQ::common::Log::L_DEBUG,HLOGFMT( "destroy weiwo session"));
		_svrSess->destroy();
	}
	catch (TianShanIce::ServerError& ex)
	{
		hlog(ZQ::common::Log::L_ERROR,HLOGFMT("failed to destory weiwoo session caught exception[%s, %s]"), ex.ice_name().c_str(), ex.message.c_str());
		composeErrorResponse(ZQ::DSMCC::RspSeNoService); //the parameter is temporal
		client_SessionDestroy();
		return  RESULT_PHASE_DONE;
	}
	catch( const TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR,HLOGFMT("caught [%s] while destroy weiwoo session[%s]"),
			_clientSessId.c_str(), ex.message.c_str(), mEnv._communicator->proxyToString(_svrSess).c_str());
		composeErrorResponse(ZQ::DSMCC::RspSeNoService);//the parameter is temporal
		client_SessionDestroy();
		return  RESULT_PHASE_DONE;
	}
	catch( const Ice::ObjectNotExistException&)
	{

	}
	catch(Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR,HLOGFMT("failed to destory weiwoo session caught exception[%s]"), ex.ice_name().c_str());
		composeErrorResponse(ZQ::DSMCC::RspSeNoService);//the parameter is temporal
		client_SessionDestroy();
		return  RESULT_PHASE_DONE;
	}
	catch( const std::exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR,HLOGFMT("ession[%s] caught [%s] while destroy weiwoo session[%s]"),
			_clientSessId.c_str(), ex.what(), mEnv._communicator->proxyToString(_svrSess).c_str());
		composeErrorResponse(ZQ::DSMCC::RspSeNoService);//the parameter is temporal
		client_SessionDestroy();
		return  RESULT_PHASE_DONE;
	}
	catch( ... )
	{
		hlog(ZQ::common::Log::L_ERROR,HLOGFMT("session[%s] caught [%s] while destroy weiwoo session[%s]"),
			_clientSessId.c_str(), "unknown exception", mEnv._communicator->proxyToString(_svrSess).c_str());
		composeErrorResponse(ZQ::DSMCC::RspSeNoService);//the parameter is temporal
		client_SessionDestroy();
		return  RESULT_PHASE_DONE;
	}


	char temp[32] ={0}; 
	//sprintf(temp,"%d",ZQ::DSMCC::MsgID_ReleaseResponse);
	sprintf(temp,"%d",ZQ::DSMCC::MsgID_ReleaseConfirm);
	MAPSET(Properties, loc_resposedata, CRMetaData_messageId,     temp);
	MAPSET(Properties, loc_resposedata, CRMetaData_transactionId, loc_txnId);
	MAPSET(Properties, loc_resposedata, CRMetaData_reserved1,     _reserved1);
	MAPSET(Properties, loc_resposedata, CRMetaData_SessionId,     _clientSessId);
	MAPSET(Properties, loc_resposedata, CRMetaData_CSRreason,     "16");//0x0010
    
	// send the successful response.
	RequestHandler::postResponse(COMMAND_DESTROY_RESPONSE, ZQ::DSMCC::RsnOK);

	client_SessionDestroy();
	hlog(ZQ::common::Log::L_INFO, HLOGFMT("clientSess[%s] cleaned, took %dms"), _clientSessId.c_str(), (int)(ZQ::common::now() - _startTime));//lStart

	return RESULT_DONE;
}

//
ProcessResult clientses_ReleaseRHandler::doFixupRespone()
{
	//response    TODO
	//hlog(ZQ::common::Log::L_INFO,CLOGFMT(clientses_ReleaseRHandler,"doFixupRespone()"));
	return RESULT_DONE;
}
void clientses_ReleaseRHandler::postCSRI()
{
	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("post SessionRelaseIndication message to STB"));
	::TianShanIce::Properties csriParams;
	char buf[64];
	snprintf(buf, sizeof(buf)-2, "%d", ZQ::DSMCC::MsgID_ReleaseIndication);
	MAPSET(TianShanIce::Properties, csriParams, CRMetaData_messageId,  buf);
	MAPSET(TianShanIce::Properties, csriParams, CRMetaData_transactionId,  loc_txnId);
	MAPSET(TianShanIce::Properties, csriParams, CRMetaData_reserved1,  _reserved1);
	MAPSET(TianShanIce::Properties, csriParams, CRMetaData_SessionId, _clientSessId);
	MAPSET(TianShanIce::Properties, csriParams, CRMetaData_CSRreason, "0");

	if (NULL == mEnv.mGateway || NULL == _clientSession)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to post SessionRelaseIndication message to STB, NULL server or session"));
		return;
	}

	WritableMessagePtr msg;

	ServerRequestPtr csri = mEnv.mGateway->createServerRequest(_connId, _clientSession);
	if (NULL != csri)
		msg = csri->getMessage();

	if (NULL == csri || NULL == msg)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to post SessionRelaseIndication message to STB, NULL ServerRequest or message"));
		return;
	}

	std::string strTempData;
	TianShanIce::Properties::iterator itorMd;
	for(itorMd = csriParams.begin(); itorMd != csriParams.end(); itorMd++)
		strTempData+=itorMd->first + "=" + itorMd->second+ " "; 
	//hlog(ZQ::common::Log::L_INFO,HLOGFMT("SessionRelaseIndication[RESPONSE]: [%s] [%s]") , itorMd->first.c_str(),itorMd->second.c_str());
	hlog(ZQ::common::Log::L_INFO,HLOGFMT("SessionReleaseConfirm[SuccessResponse]: %s"),strTempData.c_str());

	std::string peerIp,peerPort;
	_req->getPeerInfo(peerIp,peerPort);
	csri->updatePeerInfo(peerIp,peerPort);

	msg->setProperties(csriParams);
	msg->setCommand(COMMAND_RELEASE_INDICATION);
	csri->complete();
	if (_clientSession)
		_clientSession->updateTimer(_CRMDmsccConfig.heartbeatInterval);

	hlog(ZQ::common::Log::L_INFO, HLOGFMT("post SessionRelaseIndication message to STB successfully"));
}


