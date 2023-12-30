#include "crm_dsmcc.h"

lscp_PauseHandler::lscp_PauseHandler(DSMCC_Environment& env,RequestPtr request,TianShanIce::ClientRequest::SessionPrx sess)
:RequestHandler(env,request,sess,"PAUSE")
{
	_method= "PAUSE" ;
#ifdef _DEBUG
	std::cout<< " construct PAUSE  handler" <<std::endl;
#endif

};

lscp_PauseHandler::~lscp_PauseHandler()
{
#ifdef _DEBUG
	std::cout<<" deconstruct  PAUSE handler"<<std::endl;
#endif

};

void  lscp_PauseHandler::composeErrorResponse(uint32 resultCode)
{
	hlog(ZQ::common::Log::L_INFO,HLOGFMT("entry lscp_PauseHandler::composeErrorResponse()"));

	MAPSET(Properties, loc_resposedata, CRMetaData_LscStreamHandle,  pa_header_LStreamHandle);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscTransactionId, pa_header_LTransactionId);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscStatusCode,    pa_header_LStatusCode);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscVersion,       pa_header_LVersion);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscOpCode,        "129"); //0x81

	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseCurrentNpt,  "6720");//x01a 0x40
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseNumerator,   "1");
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseDenominator, "1");
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseMode,        "1");//?

	RequestHandler::postResponse(COMMAND_PAUSE_RESPONSE, resultCode);
}

void  lscp_PauseHandler::initMdMember_data()
{
	lsc::StringMap metadatas = _req->getMessage()->getProperties(); 
	try
	{
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscStreamHandle, "0", pa_header_LStreamHandle);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscTransactionId,"1", pa_header_LTransactionId);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscStatusCode,   "0", pa_header_LStatusCode);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscVersion,      "1", pa_header_LVersion);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscOpCode,       "0", pa_header_LOpCode);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_PauseStopNpt,    "1", pa_md_PauseStopNpt);
	}
	catch (...)
	{
		hlog(ZQ::common::Log::L_ERROR, CLOGFMT(lscp_PauseHandler,"[PAUSE]LscpPasueRequest: failed to get pause request parameter caught unknown exception(%d)"), SYS::getLastErr());
	}

	std::string strTempData;
	lsc::StringMap::const_iterator itorMd;
	for (itorMd = metadatas.begin(); itorMd != metadatas.end(); itorMd++)
		strTempData+=itorMd->first + "=" + itorMd->second+ " "; 

	hlog(ZQ::common::Log::L_INFO,CLOGFMT(lscp_PauseHandler,"[PAUSE]LscpPasueRequest:%s"),strTempData.c_str());
}

ProcessResult  lscp_PauseHandler::doFixupRequest()
{
	//hlog(ZQ::common::Log::L_INFO,CLOGFMT(lscp_PauseHandler,"doFixupRequest()"));
	return RESULT_PHASE_DONE ;
}

ProcessResult lscp_PauseHandler::doContentHandler()
{
	//int64 lStart = ZQ::common::now();
	initMdMember_data();

	try
	{	
		uint32  tmSessionID;
		sscanf(pa_header_LStreamHandle.c_str(),"%u",&tmSessionID);
		_clientSessId = mEnv.mGateway->streamHandle2SessionId(tmSessionID);
		//open a gateway session by session id	
		if (_clientSessId.empty())
		{
			hlog(ZQ::common::Log::L_ERROR, CLOGFMT(lscp_PauseHandler, "failed to find sessionId by streamHandle[%s]"), pa_header_LStreamHandle.c_str());
			composeErrorResponse(lscErr_NoSession); // composeErrorResponse(8);
			return RESULT_PROCESSED;
		}

		_clientSession = mEnv.mGateway->openSession(_clientSessId);
		if (!_clientSession)
		{
			//Log Warning
			hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to open client session proxy by sessionId[%s]"),_clientSessId.c_str());
			composeErrorResponse(lscErr_NoSession); // composeErrorResponse(8);
			return RESULT_PROCESSED;
		}

		_clientSession->updateTimer(_CRMDmsccConfig.heartbeatInterval);//80000000

		if ( NULL == openAttachedServerSession())
		{
			hlog(ZQ::common::Log::L_ERROR,HLOGFMT("failed to get server session object"));
			composeErrorResponse(lscErr_NoSession); // composeErrorResponse(8);
			return RESULT_PROCESSED  ;
		}

		_svrSess->renew(_CRMDmsccConfig.heartbeatInterval);

		if (NULL == openAttachedStream())
		{
			hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to get stream object"));
			composeErrorResponse(lscErr_ServerErr); // composeErrorResponse(8);
			return RESULT_PROCESSED  ;
		}

		TianShanIce::StrValues expectValues;
		expectValues.push_back("CURRENTPOS");
		expectValues.push_back("TOTALPOS");
		expectValues.push_back("SPEED");
		expectValues.push_back("STATE");
		expectValues.push_back("USERCTRLNUM");

		_streaminfo = _stream->pauseEx(expectValues);

		hlog(ZQ::common::Log::L_INFO, HLOGFMT("pause stream successfully"));
	}
	catch(const TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to pause stream caught exception[%s] %s"), ex.ice_name().c_str(), ex.message.c_str());
		composeErrorResponse(lscErr_InvalidMethod); //composeErrorResponse(8); //0x0008返回码待定
		return RESULT_PROCESSED ;
	}
	catch(const Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to pause stream caught exception[%s]"), ex.ice_name().c_str());
		composeErrorResponse(lscErr_InvalidMethod); //composeErrorResponse(8); //0x0008返回码待定
		return RESULT_PROCESSED ;
	}
	catch(...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to pause stream caught unkown exception(%d)"), SYS::getLastErr());
		composeErrorResponse(lscErr_InvalidMethod); //composeErrorResponse(8); //0x0008返回码待定
		return RESULT_PROCESSED;
	}

	char locbuf_lcurrentNpt[20]={0};
	std::string  md_res_Numerator,md_res_Denominator;
	TianShanIce::Streamer::StreamInfo _streaminfo1;
	try
	{	
		Ice::Float lscale = ZQTianShan::Util::getSpeedFromStreamInfo(_streaminfo);
		Ice::Long  lcurrentNpt = ZQTianShan::Util::getStreamTimeOffset( _streaminfo );

		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("stream scale[%2f] currentNpt[%ld]"),lscale,lcurrentNpt);

		sprintf(locbuf_lcurrentNpt,"%d",lcurrentNpt);

		lscp_PlayHandler::toFenshu(lscale,md_res_Numerator,md_res_Denominator);
	}
	catch (TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to get stream parameter speed and offset info caught exception[%s]"),ex.ice_name().c_str());
		composeErrorResponse(lscErr_ServerErr); //composeErrorResponse(8); //0x0008返回码待定
		return RESULT_PROCESSED;
	}
	catch (...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to get stream parameter speed and offset info caught unknown excepiotn(%d)"),  SYS::getLastErr());
		composeErrorResponse(lscErr_ServerErr); //composeErrorResponse(8); //0x0008返回码待定
		return RESULT_PROCESSED;
	}

	MAPSET(Properties, loc_resposedata, CRMetaData_LscStreamHandle,      pa_header_LStreamHandle);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscTransactionId,     pa_header_LTransactionId);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscStatusCode,        pa_header_LStatusCode);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscVersion,           pa_header_LVersion);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscOpCode,            "129"); //0x81

	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseCurrentNpt,   locbuf_lcurrentNpt);
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseNumerator,    md_res_Numerator);
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseDenominator,  md_res_Denominator);
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseMode,         "1"); //?

	mEnv.mStreamingInfoCache.updateInfo(_clientSessId,true);

	// send successful response
	RequestHandler::postResponse(COMMAND_PAUSE_RESPONSE, lscErr_OK);

	return RESULT_PROCESSED ;
}

ProcessResult lscp_PauseHandler::doFixupRespone()
{
	//hlog(ZQ::common::Log::L_INFO,CLOGFMT(lscp_PauseHandler,"doFixupRespone()"));
	return RESULT_PROCESSED ;
}