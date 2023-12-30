#include "crm_dsmcc.h"

lscp_PlayHandler::lscp_PlayHandler(DSMCC_Environment& env,RequestPtr request,TianShanIce::ClientRequest::SessionPrx sess)
:RequestHandler(env,request,sess,"PLAY-OR-RESUME") // "PLAY"
{
	_method = "PLAY-OR-RESUME" ;
#ifdef _DEBUG
	std::cout<<"construct PALY NAD RESUME  handler"<<std::endl;
#endif

}

lscp_PlayHandler::~lscp_PlayHandler()
{
#ifdef _DEBUG
	std::cout<<"deconstruct PALY NAD RESUME  handler"<<std::endl;
#endif

}
//
long int lscp_PlayHandler::max_gyue(long int x,long int y)
{
	long int  r,temp;
	if(x<y)
	{temp=x;x=y;y=temp;}
	r=x%y;
	while(r!=0)
	{
		x=y;
		y=r;
		r=x%y;
	}
	return y;
}

//
void lscp_PlayHandler::composeErrorResponse(uint32 resultCode)
{
	MAPSET(Properties, loc_resposedata, CRMetaData_LscStreamHandle,   md_header_LStreamHandle);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscTransactionId,  md_header_LTransactionId);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscStatusCode,     md_header_LStatusCode);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscVersion,        md_header_LVersion);

	if("6" == md_header_LOpCode) //play opcode "0x06"  play-reply opcode "0x86"
	{
		MAPSET(Properties, loc_resposedata, CRMetaData_LscOpCode, "134");
		md_header_LOpCode="134";
	}
	else //Return the op-code of Resume   resume opcode "0x02"  resume-reply opcode  "0x82"
	{	
		MAPSET(Properties, loc_resposedata, CRMetaData_LscOpCode,  "130");
		md_header_LOpCode="130";
	}
	
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseCurrentNpt,  "0");
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseNumerator,   "1");
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseDenominator, "1");
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseMode,        "3");//  ?????????

	RequestHandler::postResponse(COMMAND_PLAY_RESPONSE, resultCode);
}

#define MAX_DENOMINATOR (1<<6)
static void toFenShi(float f, int& numerator,  int& denominator)
{
	denominator = MAX_DENOMINATOR;
	f += 1.0/MAX_DENOMINATOR/2; // ajust for round 
	numerator = (int) (f * denominator);

	while (denominator>1 && 0 == numerator %2)
	{
		denominator >>=1;
		numerator >>=1;
	}
}

void  lscp_PlayHandler::toFenshu(float& flPar,std::string &Numerator ,std::string& Denominator )
{
	try
	{
		char buf[20]={0};
		int n, d;
		toFenShi(flPar, n, d);
		if (n!=0 && abs(n)<abs(d))
			n=d=1;

		sprintf(buf,"%d",n) ;
		Numerator =buf ;
		sprintf(buf,"%d", d) ;
		Denominator = buf ;
	}
	catch (...)
	{
		Numerator =1;
		Denominator =1;	
	}
}

void lscp_PlayHandler::initMdMember_data()
{
	lsc::StringMap metadatas = _req->getMessage()->getProperties() ; 

	try
	{
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_PlayStartNpt,"0",md_PlayStartNpt);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_PlayStopNpt,"0",md_PlayStopNpt);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_PlayNumerator,"0.0",md_PlayNumerator);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_PlayDenominator,"1",md_PlayDenominator);

		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscStreamHandle,"0",md_header_LStreamHandle);	
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscTransactionId,"1",md_header_LTransactionId);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscStatusCode,"1",md_header_LStatusCode);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscVersion,"1",md_header_LVersion);
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscOpCode,"0",md_header_LOpCode);
	}
	catch (...)
	{
		hlog(ZQ::common::Log::L_ERROR, CLOGFMT(lscp_PlayHandler,"[PLAY-OR-RESUME]LscpPlayRequest: failed to get play request parameter caught unknown exception(%d)"), SYS::getLastErr());
	}

	std::string strTempData;
	lsc::StringMap::const_iterator itorMd;
	for(itorMd = metadatas.begin(); itorMd != metadatas.end(); itorMd++)
		strTempData+=itorMd->first + "=" + itorMd->second+ " "; 

	hlog(ZQ::common::Log::L_INFO,CLOGFMT(lscp_PlayHandler,"[PLAY-OR-RESUME]LscpPlayRequest:%s"),strTempData.c_str());
}

ProcessResult  lscp_PlayHandler::doFixupRequest()
{
	 //hlog(ZQ::common::Log::L_INFO,CLOGFMT(lscp_PlayHandler,"doFixupRequest()"));
	 return RESULT_PROCESSED ;
}
bool lscp_PlayHandler::fixupScale( float& requestScale )
{

	if (_CRMDmsccConfig._fixedSpeedSet.speedSetEnable <= 0)
	{
		hlog(ZQ::common::Log::L_DEBUG, CLOGFMT(lscp_PlayHandler,"fixupScale()  skip FixedSpeedSet per disabled"));
		return true;
	}
	//暂停消息则不作处理
	if ( fabs(requestScale - 0.0f) < 0.0001f )
	{
		hlog(ZQ::common::Log::L_DEBUG, CLOGFMT(lscp_PlayHandler, "fixupScale() ignore requested scale[%f]"), requestScale);
		return true;
	}
	//正常1.0x播放不做处理
	if ( fabs( requestScale - 1.0f ) < 0.0001f )
	{
		_clientSession->setProperty(SESS_PROP_LSC_LASTSPEEDDIR, "1");
		_clientSession->setProperty(SESS_PROP_LSC_LASTSPEEDIDX, "-1");
		hlog(ZQ::common::Log::L_DEBUG, CLOGFMT(lscp_PlayHandler,"fixupScale()  ignore requested scale[%f]"), requestScale);
		return true;
	}

	//获取播放的direction
	int requestDirection = 1 ;
	if(  requestScale > 0.0f )
	{
		requestDirection = 1;
	}else
	{
		requestDirection = -1;
	}

	//根据direction获取对应的scale序列
	std::vector<float> scaleSet = (requestDirection > 0) ? _CRMDmsccConfig.forwardSpeeds :_CRMDmsccConfig.backwardSpeeds;
	if( scaleSet.size() <= 0 )
	{
		hlog(ZQ::common::Log::L_ERROR, CLOGFMT(lscp_PlayHandler,"fixupScale() ignore due to empty FixedSpeedSet of direction[%d]"), requestDirection);
		return false;
	}

	float scaleOld = requestScale;
	//根据配置项mode的值去处理相应的逻辑，具体可以参见bug19043
	//	1, 根据scale list 进行loop
	//	2, 根据scale list 和 inputList进行map
	if ( 1 == _CRMDmsccConfig._fixedSpeedSet.speedSetMode || 0 == _CRMDmsccConfig._fixedSpeedSet.speedSetMode )
	{
		int lastDirection = 1 , index = -1;
		std::string propertyStr;
		propertyStr = _clientSession->getProperty(SESS_PROP_LSC_LASTSPEEDIDX);
		if ( ! propertyStr.empty() )
			index = atoi(propertyStr.c_str());

		propertyStr.clear();
		propertyStr = _clientSession->getProperty(SESS_PROP_LSC_LASTSPEEDDIR);
		if ( ! propertyStr.empty() )
			lastDirection = atoi(propertyStr.c_str());

		if ( 1 == _CRMDmsccConfig._fixedSpeedSet.speedSetMode )//EnableFixedSpeedLoop
		{
			index = (int)( lastDirection * requestDirection ) < 0 ? 0 : ( (++index) % scaleSet.size() );
		}
		else 
		{
			index = ( lastDirection * requestDirection ) < 0 ? 0 : (++index) ;
			if( index >= (int)scaleSet.size() )
			{
				//reach the fix speed set end
				hlog(ZQ::common::Log::L_WARNING, CLOGFMT(lscp_PlayHandler,"fixupScale() reached the last fixed speed, direction[%d]"), requestDirection);
				return false;
			}		
		}
		// save the index and direction into session ctx
		char strIndex[32]=""; sprintf(strIndex,"%d",index);
		_clientSession->setProperty(SESS_PROP_LSC_LASTSPEEDIDX, strIndex);
		char strDirection[32]=""; sprintf(strDirection,"%d",requestDirection);
		_clientSession->setProperty(SESS_PROP_LSC_LASTSPEEDDIR, strDirection);
		requestScale = scaleSet[index];
	}
	else if ( 2 == _CRMDmsccConfig._fixedSpeedSet.speedSetMode )
	{
		//根据direction获取对应的map序列
		std::vector<float> inputSpeedList = (requestDirection > 0) ? _CRMDmsccConfig.inputFFs :_CRMDmsccConfig.inputREWs;
		if( inputSpeedList.size() <= 0 )
		{
			hlog(ZQ::common::Log::L_ERROR, CLOGFMT(lscp_PlayHandler,"fixupScale() ignore due to empty inputSpeedList of direction[%d]"), requestDirection);
			return false;
		}

		int i = 0;
		for (i = 0; i < inputSpeedList.size() ; i ++)
		{
			if ( abs(requestScale) <= (abs( inputSpeedList[i] ) + 0.01f ) )
				break;
		}
		if ( i >= scaleSet.size())
			i = scaleSet.size() - 1;
		requestScale = scaleSet[i];
	}
	hlog(ZQ::common::Log::L_INFO, CLOGFMT(lscp_PlayHandler,"fixupScale()  requested scale[%f] has been fixed up to scale[%f]"), scaleOld, requestScale);
	return true;

}
ProcessResult  lscp_PlayHandler::doContentHandler()
{
	//int64 lStart = ZQ::common::now();
	initMdMember_data() ;
	uint32  tmSessionID;
	sscanf(md_header_LStreamHandle.c_str(),"%u",&tmSessionID) ;
	_clientSessId = mEnv.mGateway->streamHandle2SessionId(tmSessionID);

	if(_clientSessId.empty())
	{
		hlog(ZQ::common::Log::L_ERROR, CLOGFMT(lscp_PlayHandler,"failed to find sessionid by calling streamHandle2SessionId[%s]"),md_header_LStreamHandle.c_str());
		composeErrorResponse(lscErr_NoSession); // composeErrorResponse(8);
		return RESULT_PROCESSED;
	}
	try
	{	//open a gateway session by session id	
		_clientSession = mEnv.mGateway->openSession(_clientSessId) ;
		if (!_clientSession)
		{	//Log Warning
			hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to open client session proxy by sessionId[%s]"),_clientSessId.c_str());
			composeErrorResponse(lscErr_NoSession); // composeErrorResponse(8);
			return RESULT_PROCESSED  ;
		}
		_clientSession->updateTimer(_CRMDmsccConfig.heartbeatInterval);//80000000

		if( NULL == openAttachedServerSession())
		{
			hlog(ZQ::common::Log::L_ERROR,HLOGFMT("failed to get server session object")) ;
			composeErrorResponse(lscErr_NoSession); // composeErrorResponse(8);
			return RESULT_PROCESSED;
		}
		_svrSess->renew(_CRMDmsccConfig.heartbeatInterval);

		std::string peerIp, peerPort;
		_req->getPeerInfo(peerIp,peerPort);
		_clientSession->updateTimer(_CRMDmsccConfig.heartbeatInterval);//80000000
		_clientSession->setProperty("lscp_peer_ip",peerIp);
		_clientSession->setProperty("lscp_peer_port",peerPort);
		int64 connId = _req->getConnectionId();
		char strConnId[32];sprintf(strConnId,"%lld",connId);
		_clientSession->setProperty(SESS_PROP_LSC_CONNID,strConnId);

		if(NULL == openAttachedStream() )
		{
			hlog(ZQ::common::Log::L_ERROR,HLOGFMT("failed to get stream object")) ;
			composeErrorResponse(lscErr_NoSession); // composeErrorResponse(8);
			return RESULT_PROCESSED  ;
		}
	}
	catch( const Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to get stream caught exception[%s]"), ex.ice_name().c_str());
		composeErrorResponse(lscErr_ServerErr); // composeErrorResponse(8);
		return RESULT_DONE ;
	}
	catch( ... )
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to get stream caught unknown exception"));
		composeErrorResponse(lscErr_ServerErr); // composeErrorResponse(8);
		return RESULT_DONE;
	}

	//
	TianShanIce::StrValues expectValues;
	expectValues.push_back("CURRENTPOS");
	expectValues.push_back("TOTALPOS");
	expectValues.push_back("SPEED");
	expectValues.push_back("STATE");
	expectValues.push_back("USERCTRLNUM");

	if(md_PlayDenominator.empty() )
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("invailed input play Denominator parampter"));
		composeErrorResponse(lscErr_BadRequest); // composeErrorResponse(8);
		return RESULT_PROCESSED;
	}
	if(0 == atol(md_PlayDenominator.c_str())) 
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("invailed input play Denominator = 0"));
		composeErrorResponse(lscErr_BadRequest); // composeErrorResponse(8);
		return RESULT_PROCESSED;
	}
	float locspeed ;
	if(0 == atol(md_PlayNumerator.c_str()))
		locspeed = 0.0 ;
	else
		locspeed = atof(md_PlayNumerator.c_str()) / atol(md_PlayDenominator.c_str()) ;
	if(!fixupScale(locspeed) )
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to fixSpeed ."));
		composeErrorResponse(lscErr_BadRequest);
		return RESULT_PROCESSED;
	}
	try
	{  
		uint64 startNpt = 0;
		sscanf(md_PlayStartNpt.c_str(), "%lld", &startNpt);
		Ice::Short from = 1;
		if(startNpt == 0x80000000)
		{
			startNpt = 0;
			from = 0;
		}
		hlog(ZQ::common::Log::L_INFO, HLOGFMT("start to play stream with speed[%2f] startNpt[%lld] from[%d]]"),locspeed, startNpt, from);

		_streaminfo = _stream->playEx(locspeed,(Ice::Long)startNpt,from,expectValues);

		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("play stream successful"));
	}
	catch( const TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to play stream caught exception[%s] %s"), ex.ice_name().c_str(), ex.message.c_str());
		composeErrorResponse(lscErr_InvalidMethod); //0x0008返回码待定
		return RESULT_DONE ;
	}
	catch( const Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to play stream caught exception[%s]"), ex.ice_name().c_str());
		composeErrorResponse(lscErr_InvalidMethod); //0x0008返回码待定
		return RESULT_DONE ;
	}
	catch( ... )
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to play stream caught unkonwn exception(%s)"),  SYS::getLastErr());
		composeErrorResponse(lscErr_InvalidMethod); //0x0008返回码待定
		return RESULT_DONE;
	}

	char locbuf_lcurrentNpt[20]={0};
	Ice::Long lcurrentNpt = 0;
	Ice::Float lscale = 0;
	try
	{	
		lscale = ZQTianShan::Util::getSpeedFromStreamInfo(_streaminfo);
		lcurrentNpt = ZQTianShan::Util::getStreamTimeOffset( _streaminfo);

		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("stream scale[%2f] currentNpt[%ld]"),lscale,lcurrentNpt);

		sprintf(locbuf_lcurrentNpt,"%d",lcurrentNpt);

		toFenshu(lscale,md_res_Numerator,md_res_Denominator);
	}
	catch(TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to get stream parameter speed and offset info caught exception[%s]"),ex.ice_name().c_str());
		composeErrorResponse(lscErr_ServerErr); //0x0008返回码待定
		return RESULT_DONE;
	}
	catch(...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("failed to get stream parameter speed and offset info caught unknown excepiotn(%d)"),  SYS::getLastErr());
		composeErrorResponse(lscErr_ServerErr); //0x0008返回码待定
		return RESULT_DONE;
	}

	MAPSET(Properties, loc_resposedata, CRMetaData_LscStreamHandle,   md_header_LStreamHandle);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscTransactionId,  md_header_LTransactionId);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscStatusCode,     md_header_LStatusCode);
	MAPSET(Properties, loc_resposedata, CRMetaData_LscVersion,        md_header_LVersion);

	GATEWAYCOMMAND cmdResp = COMMAND_PLAY_RESPONSE;
	
	if("6" == md_header_LOpCode)
	{	
		md_header_LOpCode="134";
		cmdResp =COMMAND_PLAY_RESPONSE;
	}
	else  //Return the op-code of Resume 
	{	
		md_header_LOpCode = "130";
		cmdResp = COMMAND_RESUME_RESPONSE;
	}
	
	MAPSET(Properties, loc_resposedata, CRMetaData_LscOpCode,          md_header_LOpCode);
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseCurrentNpt, locbuf_lcurrentNpt);
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseNumerator,  md_res_Numerator);
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseDenominator,md_res_Denominator);
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseMode,       "3");

	mEnv.mStreamingInfoCache.updateInfo(_clientSessId,lcurrentNpt,locspeed);

	// send the successful response.
	RequestHandler::postResponse(cmdResp, lscErr_OK);

	return RESULT_DONE;
}

ProcessResult  lscp_PlayHandler::doFixupRespone()
{
	//hlog(ZQ::common::Log::L_INFO,CLOGFMT(lscp_PlayHandler,"doFixupRespone()"));
	return RESULT_PROCESSED ;
}
