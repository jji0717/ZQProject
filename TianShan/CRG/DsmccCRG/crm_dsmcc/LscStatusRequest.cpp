#include "crm_dsmcc.h"
/* static uint32 NPT =609;
	NPT +=1000;
	if(NPT >= 1687000 -1000)
	{
	composeErrorResponse("16");
	return RESULT_PROCESSED;
	}
	char strTemp[65]="";
	sprintf(strTemp, "%u",NPT);
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseCurrentNpt ,strTemp ));
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseNumerator,"1" ));
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseDenominator,"1" ));*/

lscp_StatusHandler::lscp_StatusHandler(DSMCC_Environment& env,RequestPtr request,TianShanIce::ClientRequest::SessionPrx sess)
:RequestHandler(env,request,sess,"LSC_PING")
{
	_method = "LSC_PING" ;
#ifdef _DEBUG
	std::cout<< " construct LSC_PING handler "<<std::endl ;
#endif
}

lscp_StatusHandler::~lscp_StatusHandler()
{
#ifdef _DEBUG
	std::cout<<" deconstruct LSC_PING handler "<<std::endl ;
#endif
};

void  lscp_StatusHandler::composeErrorResponse(uint32 resultCode)
{
	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("entry lscp_StatusHandler::composeErrorResponse()"));

	MAPSET(Properties, loc_resposedata, CRMetaData_LscStreamHandle,   status_header_LStreamHandle);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscTransactionId,  status_header_LTransactionId);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscStatusCode,     status_header_LStatusCode);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscVersion,        status_header_LVersion);

	if("3" == status_header_LOpCode)
	{
		MAPSET(Properties, loc_resposedata, CRMetaData_LscOpCode,    "131");
		status_header_LOpCode="131";
	}

	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseCurrentNpt, "0");
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseNumerator,  "1");
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseDenominator,"1");
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseMode,       "3");

	// send the successful response.
	RequestHandler::postResponse(COMMAND_STATUS_RESPONSE, 0);
}


void  lscp_StatusHandler::initMdMember_data()
{
	lsc::StringMap metadatas = _req->getMessage()->getProperties() ; 
	try
	{
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscStreamHandle,"0",status_header_LStreamHandle);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscTransactionId,"6",status_header_LTransactionId);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscStatusCode,"0",status_header_LStatusCode);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscVersion,"1",status_header_LVersion);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscOpCode,"0",status_header_LOpCode);

	}
	catch (...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("[LSC_PING]LscpStatusRequest: failed to get status request parameter caught unknown exception(%d)"),  SYS::getLastErr());
	}

	std::string strTempData;
	lsc::StringMap::const_iterator itorMd;
	for(itorMd = metadatas.begin(); itorMd != metadatas.end(); itorMd++)
		strTempData+=itorMd->first + "=" + itorMd->second+ " "; 

	hlog(ZQ::common::Log::L_INFO, HLOGFMT("[LSC_PING]LscpStatusRequest:%s"),strTempData.c_str());
}

//
ProcessResult  lscp_StatusHandler::doFixupRequest()
{
	//hlog(ZQ::common::Log::L_INFO,CLOGFMT(lscp_StatusHandler,"doFixupRequest()"));
	return RESULT_PROCESSED;
}

ProcessResult lscp_StatusHandler::getStreamingInfo( bool& bcontinue )
{
	bcontinue = false;
	try
	{
		uint32  tmSessionID;
		sscanf(status_header_LStreamHandle.c_str(),"%u",&tmSessionID) ;
		_clientSessId = mEnv.mGateway->streamHandle2SessionId(tmSessionID);

		if(_clientSessId.empty())
		{
			hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to find sessionid by streamHandle[%s]"),status_header_LStreamHandle.c_str());
			composeErrorResponse(lscErr_NoSession); // composeErrorResponse(8);
			return RESULT_PROCESSED;
		}
		
		int64 npt = 0;
		float speed = 0.0f;
		if( mEnv.mStreamingInfoCache.getInfo(_clientSessId,npt,speed))
		{
			bcontinue = true;
			mbInfoFromCache = true;
			status_SSInfo.npt = (Ice::Int)npt;
			char szScale[16];
			sprintf(szScale,"%f",speed);
			status_SSInfo.scale = szScale;				 
			return RESULT_DONE;
		}

		//open a gateway session by session id	
		_clientSession = mEnv.mGateway->openSession(_clientSessId) ;
		if (!_clientSession)
		{
			//Log Warning
			hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to open client session proxy by sessionId[%s]"),_clientSessId.c_str());
			composeErrorResponse(lscErr_NoSession); // composeErrorResponse(8);
			return RESULT_DONE;
		}

		_clientSession->updateTimer(_CRMDmsccConfig.heartbeatInterval);

		if( NULL == openAttachedServerSession())
		{
			hlog(ZQ::common::Log::L_ERROR,HLOGFMT("failed to get server session object")) ;
			composeErrorResponse(lscErr_NoSession); // composeErrorResponse(8);
			return RESULT_PROCESSED;
		}
		_svrSess->renew(_CRMDmsccConfig.heartbeatInterval);
		hlog(ZQ::common::Log::L_DEBUG,HLOGFMT("lscp_StatusHandler invoke renew() by para[%ld] "),_CRMDmsccConfig.heartbeatInterval);

		if (NULL == openAttachedStream())
		{
			hlog(ZQ::common::Log::L_ERROR,HLOGFMT("failed to get stream object")) ;
			composeErrorResponse(lscErr_ServerErr); // composeErrorResponse(8);
			return RESULT_PROCESSED;
		}
	}
	catch( const Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to get stream caught exception[%s]"), ex.ice_name().c_str());
		composeErrorResponse(lscErr_ServerErr); // composeErrorResponse(8);
		return RESULT_DONE;
	}
	catch( ... )
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to get stream caught unknown exception"));
		composeErrorResponse(lscErr_ServerErr); // composeErrorResponse(8);
		return RESULT_DONE;
	}
	//lscp_StatusHandler::StreamSessionInfo  ssInfo ;
	hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("start to get stream parameter"));
	try
	{
		TianShanIce::ValueMap  valMap ;
		TianShanIce::Streamer::PlaylistPrx pl = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(_stream);
		pl->getInfo( ::TianShanIce::Streamer::infoSTREAMNPTPOS,valMap) ;

		ZQTianShan::Util::getValueMapDataWithDefault( valMap , "playposition" , 0 , status_SSInfo.npt );
		ZQTianShan::Util::getValueMapDataWithDefault( valMap , "totalplaytime" , 0, status_SSInfo.playTime );
		ZQTianShan::Util::getValueMapDataWithDefault( valMap , "scale", "" ,status_SSInfo.scale );
		ZQTianShan::Util::getValueMapDataWithDefault( valMap , "itemOffset", -1 , status_SSInfo.assetNpt );
		ZQTianShan::Util::getValueMapDataWithDefault( valMap , "index" , 0 , status_SSInfo.assetIndex  );

		hlog(ZQ::common::Log::L_INFO, HLOGFMT("stream parameter: npt[%d] playtime[%d] scale[%s] assetNpt[%d] assetIndex[%d] "),
			status_SSInfo.npt , status_SSInfo.playTime , status_SSInfo.scale.c_str() , status_SSInfo.assetNpt , status_SSInfo.assetIndex );
	}
	catch( TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR,HLOGFMT("failed to get stream instance caught exception %s"),ex.message.c_str()) ;
		composeErrorResponse(lscErr_ServerErr); // composeErrorResponse(8);
		return RESULT_DONE ;
	}
	catch( const Ice::ObjectNotExistException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR,HLOGFMT("failed to get stream instance caught exception %s"),ex.ice_name().c_str() ) ;
		composeErrorResponse(lscErr_ServerErr); // composeErrorResponse(8);
		return RESULT_DONE ;
	}
	catch( const Ice::TimeoutException & ex)
	{
		hlog(ZQ::common::Log::L_ERROR,HLOGFMT("failed to get stream instance caught exception %s"),ex.ice_name().c_str() ) ;
		composeErrorResponse(lscErr_ServerErr); // composeErrorResponse(8);
		return RESULT_DONE ;
	}
	catch( const Ice::SocketException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR,HLOGFMT("failed to get stream instance caught exception %s"),ex.ice_name().c_str() ) ;
		composeErrorResponse(lscErr_ServerErr); // composeErrorResponse(8);
		return RESULT_DONE ;
	}
	catch( const Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR,HLOGFMT("failed to get stream instance caught exception %s"),ex.ice_name().c_str()) ;
		composeErrorResponse(lscErr_ServerErr); // composeErrorResponse(8);
		return RESULT_DONE ;
	}

	bcontinue = true;
	return RESULT_DONE;
}
//
ProcessResult lscp_StatusHandler::doContentHandler()
{
	//int64 lStart = ZQ::common::now();
	initMdMember_data() ;

	mbInfoFromCache = false;
	ProcessResult pr;
	bool bContinue = false;
	if( ( pr = getStreamingInfo(bContinue) ) != RESULT_DONE )
	{
		if(!bContinue)
			return pr;
	}

	float lfscale = 1.1;
	char locbuf_lcurrentNpt[20]={0};
	sprintf(locbuf_lcurrentNpt,"%d",status_SSInfo.npt);
/*	
	std::istringstream liscale(status_SSInfo.scale);
	if ( == ""  || liscale == 0 )
		lfscale=1.1;//default 
	else
		liscale>>lfscale ;
*/

	if(!status_SSInfo.scale.empty() && status_SSInfo.scale != "0")
	{
		sscanf(status_SSInfo.scale.c_str(), "%f",&lfscale);
	}
	//LSC_Response
	std::string  md_res_Numerator,md_res_Denominator ;

	lscp_PlayHandler::toFenshu(lfscale,md_res_Numerator,md_res_Denominator) ;

	//
	MAPSET(Properties, loc_resposedata, CRMetaData_LscStreamHandle,     status_header_LStreamHandle);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscTransactionId,    status_header_LTransactionId);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscStatusCode,       status_header_LStatusCode);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscVersion,          status_header_LVersion);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscOpCode,           "131");

	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseCurrentNpt,  locbuf_lcurrentNpt);
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseNumerator,   md_res_Numerator);
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseDenominator, md_res_Denominator);
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseMode,        "3");

	if( !mbInfoFromCache)
		mEnv.mStreamingInfoCache.updateInfo(_clientSessId,status_SSInfo.npt,lfscale);

	// send successful response
	RequestHandler::postResponse(COMMAND_STATUS_RESPONSE, lscErr_OK);

	return RESULT_PROCESSED;
}

//
ProcessResult  lscp_StatusHandler::doFixupRespone()
{
	//hlog(ZQ::common::Log::L_INFO,CLOGFMT(lscp_StatusHandler,"doFixupRespone()"));
	return RESULT_PROCESSED ;
}
