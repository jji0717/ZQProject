#include "crm_dsmcc.h"
#include "DsmccDefine.h"
#include "urlstr.h"
#include "lsc_protocol.h"
#include "EventSink.h"



#define CurrentState_Key        "currentState"
#define PrevState_Key           "prevState"

#define CurrentSpeed_Key        "currentSpeed"
#define PrevSpeed_Key           "prevSpeed"

// -----------------------------
// class EventSink
// -----------------------------
#define envlog (_env.mMainLogger)

void EventSink::issueLscDone(const std::string& streamId, const TianShanIce::Properties& streamParams, bool bEOS) const
{
	const char* type = bEOS? "EndOfStream" : "BeginOfStream";
	const char* mode = "7";
	TianShanIce::Properties::const_iterator itor; 
	itor =  streamParams.find("sys.goneMode");
	if (bEOS)
	{
		// ticket#11287, stupid integration
		//if (streamParams.end() != streamParams.find("sys.goneMode") && streamParams["sys.goneMode"] == "T")
		if (streamParams.end() != itor &&  itor->second == "T")
			mode = "3";
	}
	else// ticket#11338
	{
		mode = "3";
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEvent, "issueLscDone(%s) %s"), streamId.c_str(), type);
	if (NULL == _env.mGateway)
		return;

	std::string clientSessId =_env.mGateway->streamSessId2ClientSessionId(streamId);
	TianShanIce::ClientRequest::SessionPrx clientSession = _env.mGateway->openSession(clientSessId);

	if (!clientSession)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEvent, "issueLscDone(%s) %s unkown stream, ignore"), streamId.c_str(), type);
		return;
	}

	TianShanIce::Properties clientSessionParams;
	try
	{
	   clientSessionParams = clientSession->getProperties();
	}
	catch (Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEvent, "issueLscDone(%s/%s) %s failed to get params caught exception(%s), ignore"), 
			streamId.c_str(),clientSessId.c_str(), type, ex.ice_name().c_str());
		return;
	}
	catch(...)
	{	
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEvent, "issueLscDone(%s/%s) %s failed to get params caught unknown exception, ignore"), 
			streamId.c_str(), clientSessId.c_str(), type);
		return;
	}
	int  dsmccType = ZQ::DSMCC::Protocol_MOTO;
	if (clientSessionParams.end() != clientSessionParams.find(CRMetaData_ProtocolType))
		dsmccType = atoi(clientSessionParams[CRMetaData_ProtocolType].c_str());

	std::string statusCode = "0";
/*	if(bEOS && dsmccType == ZQ::DSMCC::Protocol_Tangberg)
	{
		statusCode  = "40";
	}
*/
	std::string  fullplaytimeins;
	if (clientSessionParams.end() != clientSessionParams.find(SYS_PROP(fullPlayTime)))
		fullplaytimeins = clientSessionParams[SYS_PROP(fullPlayTime)];

	std::string streamHandle = "0";
	int64       connId =0;

	if (clientSessionParams.end() != clientSessionParams.find(SESS_PROP_STREAMHANDLE))
		streamHandle = clientSessionParams[SESS_PROP_STREAMHANDLE];

	if (clientSessionParams.end() != clientSessionParams.find(SESS_PROP_LSC_CONNID))
		sscanf(clientSessionParams[SESS_PROP_LSC_CONNID].c_str(), "%lld", &connId);

	std::string peerIp,peerPort;
	ZQTianShan::Util::getPropertyDataWithDefault(clientSessionParams, "lscp_peer_ip", "", peerIp);
	ZQTianShan::Util::getPropertyDataWithDefault(clientSessionParams, "lscp_peer_port", "", peerPort);

	char buf[60]="";
	TianShanIce::Properties lscParams;
	snprintf(buf, sizeof(buf)-2, "%d", lsc::LSC_DONE);

	MAPSET(TianShanIce::Properties, lscParams, CRMetaData_LscVersion,        "1");
	MAPSET(TianShanIce::Properties, lscParams, CRMetaData_LscOpCode,        buf);
	MAPSET(TianShanIce::Properties, lscParams, CRMetaData_LscStreamHandle,  streamHandle);
	MAPSET(TianShanIce::Properties, lscParams, CRMetaData_LscTransactionId, "0");
	MAPSET(TianShanIce::Properties, lscParams, CRMetaData_LscStatusCode,    statusCode);
	MAPSET(TianShanIce::Properties, lscParams, CRMetaData_ResponseMode,     mode);

	{
		std::string paramstr;
		for(TianShanIce::Properties::const_iterator itParam = streamParams.begin(); itParam != streamParams.end(); itParam++)
			paramstr += itParam->first + "=" + itParam->second + " ";

		envlog(ZQ::common::Log::L_INFO, CLOGFMT(StreamEvent, "issueLscDone(%s/%s) %s streamHandle[%s] stream params: %s"),
			streamId.c_str(), clientSessId.c_str(), type, streamHandle.c_str(), paramstr.c_str());
	}

	try
	{
		std::string currntNpt= bEOS ? fullplaytimeins : "0"; // enh#16403

		//LSC_Response
		std::string scaleNumerator, scaleDenominator;
		{
			TianShanIce::Properties::const_iterator itSP = streamParams.find("sys.npt");
			if (streamParams.end() != itSP)
			{
				currntNpt = itSP->second;
				if (currntNpt.size()>= 3)
				{	
					currntNpt.replace(currntNpt.end() -3, currntNpt.end(), 3, '0');
				}

				// enh#16403
				if (bEOS && abs( atol(currntNpt.c_str()) - atol(fullplaytimeins.c_str())) < _CRMDmsccConfig.eosNptErr)
					currntNpt=fullplaytimeins;
			}

			std::string strSpeed="1.1";
			itSP = streamParams.find("SPEED");
			if (streamParams.end()!= itSP)
				strSpeed = itSP->second;

			float f;
			sscanf(strSpeed.c_str() , "%f" , &f);
			lscp_PlayHandler::toFenshu(f, scaleNumerator, scaleDenominator);
		}

		if ('3' == *mode && streamParams.end() != streamParams.find("sys.includeNpt")) // ticket#11287, stupid integration
		{
			int64 npt = 0;
			float speed = 0.0f;
			if (_env.mStreamingInfoCache.getInfo(clientSessId,npt,speed))
			{
				char buf[20];
				snprintf(buf, sizeof(buf)-2, "%lld", npt);
				currntNpt = buf;
			}
		}
		
		MAPSET(TianShanIce::Properties, lscParams, CRMetaData_ResponseCurrentNpt, currntNpt);
		MAPSET(TianShanIce::Properties, lscParams, CRMetaData_ResponseNumerator,   scaleNumerator);
		MAPSET(TianShanIce::Properties, lscParams, CRMetaData_ResponseDenominator, scaleDenominator);
		
		int64 npt = 0 ;
		sscanf(currntNpt.c_str(),"%lld",&npt);
		

		_env.mStreamingInfoCache.updateInfo( clientSessId, npt, 0.0f );

		std::map<std::string,std::string>::iterator itorMd;
        std::string strTempData;
		for(itorMd = lscParams.begin(); itorMd != lscParams.end(); itorMd++)
		{
			strTempData+=itorMd->first + "=" + itorMd->second + " "; 
		}
		envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(StreamEvent, "issueLscDone(%s/%s): %s response params: %s") ,
			streamId.c_str(), clientSessId.c_str(), type, strTempData.c_str());

		ServerRequestPtr lscDone = _env.mGateway->createServerRequest(connId, clientSession);
		if (!lscDone)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEvent, "issueLscDone(%s/%s) %s streamHandle[%s] failed to create message via connId[%lld]"), 
				streamId.c_str(), clientSessId.c_str(), type, streamHandle.c_str(), connId);
			return;
		}

		lscDone->getMessage()->setProperties(lscParams);
		lscDone->getMessage()->setCommand(COMMAND_DONE_RESPONSE);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEvent, "issueLscDone(%s/%s) %s sending streamHandle[%s]"),
			streamId.c_str(), clientSessId.c_str(), type, streamHandle.c_str());
		lscDone->updatePeerInfo(peerIp, peerPort);
		lscDone->complete();
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(StreamEvent, "issueLscDone(%s/%s) %s sent streamHandle[%s]"),
			streamId.c_str(), clientSessId.c_str(), type, streamHandle.c_str());

	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEvent, "issueLscDone(%s/%s) %s failed to sent streamHandle[%s] done response"),
			streamId.c_str(), clientSessId.c_str(), type, streamHandle.c_str());
	}
}

// issue DSMCC ClientSessionReleaseIndication
void EventSink::issueCSRI(TianShanIce::ClientRequest::SessionPrx clientSession, int reasonCode)
{
	if (!clientSession)
		return;

	TianShanIce::Properties sessionParams = clientSession->getProperties();
	std::string sessionId = clientSession->getSessId();
	int64       connId =0;
	if (sessionParams.end() != sessionParams.find(SESS_PROP_DSMCC_CONNID))
		sscanf(sessionParams[SESS_PROP_DSMCC_CONNID].c_str(), "%lld", &connId);

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(issueCSRI, "issuing CSRI"));
	::TianShanIce::Properties csriParams;
	char buf[64];
	snprintf(buf, sizeof(buf)-2, "%d", ZQ::DSMCC::MsgID_ReleaseIndication);
	MAPSET(TianShanIce::Properties, csriParams, CRMetaData_messageId,  buf);

	MAPSET(TianShanIce::Properties, csriParams, CRMetaData_SessionId, sessionId);

	char strReason[65];
	memset(strReason, 0, sizeof(strReason));
	itoa(reasonCode, strReason, 10);
	MAPSET(TianShanIce::Properties, csriParams, CRMetaData_CSRreason, strReason); // timeout at server-side

	WritableMessagePtr msg;
	ServerRequestPtr csri = _env.mGateway->createServerRequest(connId, clientSession);
	if (NULL != csri)
		msg = csri->getMessage();

	if (NULL == csri || NULL == msg)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(issueCSRI, "issuing CSRI failed, NULL ServerRequest or message"));
		return;
	}
	std::string peerIp,peerPort;
	ZQTianShan::Util::getPropertyDataWithDefault(sessionParams,"dsmcc_peer_ip","",peerIp);
	ZQTianShan::Util::getPropertyDataWithDefault(sessionParams,"dsmcc_peer_port","",peerPort);

	msg->setProperties(csriParams);
	msg->setCommand( COMMAND_RELEASE_INDICATION );
	csri->updatePeerInfo(peerIp, peerPort);
	csri->complete();
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(issueCSRI, "CSPI issued"));

}
std::string getPropertiesString(const TianShanIce::Properties& props)
{
	std::string strProps;
	for(TianShanIce::Properties::const_iterator itor = props.begin(); itor != props.end(); itor++)
	{
		strProps += itor->first + std::string(":") + itor->second + std::string(";");
	}
	return strProps;
}
std::string  streamStateToString(TianShanIce::Streamer::StreamState state)
{
	std::string strState;
	switch(state)
	{
	case TianShanIce::Streamer::stsSetup:
		strState = "Setup";
		break;
	case TianShanIce::Streamer::stsStreaming:
		strState = "Streaming";
		break;
	case TianShanIce::Streamer::stsPause:
		strState = "Pause";
		break;
	case TianShanIce::Streamer::stsStop:
		strState = "Stop";
		break;
	default:
		strState = "Unknown";
	}
	return strState;
}
void EventSink::OnEndOfStream(const ::std::string& proxy, const ::std::string& playlistId, const TianShanIce::Properties& streamParams, const ::Ice::Current& ic) const
{
	try
	{
		issueLscDone(playlistId, streamParams, true);
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEvent, "OnEndOfStream(%s) caught exception"), proxy.c_str());
	}

}

void EventSink::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& playlistId, const TianShanIce::Properties& streamParams, const ::Ice::Current& ic) const
{
	try
	{
		issueLscDone(playlistId, streamParams, false);
	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEvent, "OnBeginningOfStream(%s) caught exception"), proxy.c_str());
	}
}

void EventSink::OnSpeedChanged(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const TianShanIce::Properties& props , const ::Ice::Current& ic) const
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEvent, "OnSpeedChanged() proxy[%s] playlistId[%s] prevSpeed[%f] currentSpeed[%f] props[%s]"),
		proxy.c_str(),playlistId.c_str(), prevSpeed, currentSpeed, getPropertiesString(props).c_str());


	if(props.find("user.LSCP-RespCode") == props.end())
		return;

	TianShanIce::Properties newProps = props;

	char buf[32];
	memset(buf, 0, sizeof(buf));
	sscanf(buf,  "%.2f", &currentSpeed);
	newProps[CurrentSpeed_Key] = std::string(buf);

	memset(buf, 0, sizeof(buf));
	sscanf(buf,  "%.2f", &prevSpeed);
	newProps[PrevSpeed_Key] = std::string(buf);

	return HeNanLscDone(playlistId, newProps, "OnSpeedChanged");
}

void EventSink::OnStateChanged(const ::std::string& proxy, const ::std::string& playlistId, ::TianShanIce::Streamer::StreamState prevState, ::TianShanIce::Streamer::StreamState currentState, const TianShanIce::Properties& props ,  const ::Ice::Current& ic) const 
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEvent, "OnStateChanged() proxy[%s] playlistId[%s] prevState[%d] currentState[%d] props[%s]"),
		proxy.c_str(),playlistId.c_str(), prevState, currentState, getPropertiesString(props).c_str());	

	if(props.find("user.LSCP-RespCode") == props.end())
		return;

	TianShanIce::Properties newProps = props;

	newProps[CurrentState_Key] = streamStateToString(currentState);
	newProps[PrevState_Key] = streamStateToString(prevState);
	return HeNanLscDone(playlistId, newProps, "OnStateChanged");

}
void EventSink::HeNanLscDone(const std::string& streamId, const TianShanIce::Properties& streamParams, const std::string& tag) const
{
	const char* mode = "7";
	TianShanIce::Properties::const_iterator itor; 

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEvent, "HeNanLscDone(%s) tag:%s"), streamId.c_str(), tag.c_str());
	if (NULL == _env.mGateway)
		return;

	std::string clientSessId =_env.mGateway->streamSessId2ClientSessionId(streamId);
	TianShanIce::ClientRequest::SessionPrx clientSession = _env.mGateway->openSession(clientSessId);

	if (!clientSession)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEvent, "HeNanLscDone(%s) unkown stream, ignore"), streamId.c_str());
		return;
	}

	TianShanIce::Properties clientSessionParams;
	try
	{
		clientSessionParams = clientSession->getProperties();
	}
	catch (Ice::Exception& ex)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEvent, "HeNanLscDone(%s/%s) failed to get params caught exception(%s), ignore"), 
			streamId.c_str(),clientSessId.c_str(), ex.ice_name().c_str());
		return;
	}
	catch(...)
	{	
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEvent, "HeNanLscDone(%s/%s)failed to get params caught unknown exception, ignore"), 
			streamId.c_str(), clientSessId.c_str());
		return;
	}

	int  dsmccType = ZQ::DSMCC::Protocol_MOTO;
	if (clientSessionParams.end() != clientSessionParams.find(CRMetaData_ProtocolType))
		dsmccType = atoi(clientSessionParams[CRMetaData_ProtocolType].c_str());

	if(dsmccType != ZQ::DSMCC::Protocol_Tangberg)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEvent, "HeNanLscDone(%s) ignore, dsmmcc type is not ZQ::DSMCC::Protocol_Tangberg"), streamId.c_str());
		return; 
	}

	std::string  fullplaytimeins = "0";
	if (clientSessionParams.end() != clientSessionParams.find(SYS_PROP(fullPlayTime)))
		fullplaytimeins = clientSessionParams[SYS_PROP(fullPlayTime)];

	std::string streamHandle = "0";
	int64       connId =0;

	if (clientSessionParams.end() != clientSessionParams.find(SESS_PROP_STREAMHANDLE))
		streamHandle = clientSessionParams[SESS_PROP_STREAMHANDLE];

	if (clientSessionParams.end() != clientSessionParams.find(SESS_PROP_LSC_CONNID))
		sscanf(clientSessionParams[SESS_PROP_LSC_CONNID].c_str(), "%lld", &connId);

	std::string peerIp,peerPort;
	ZQTianShan::Util::getPropertyDataWithDefault(clientSessionParams, "lscp_peer_ip", "", peerIp);
	ZQTianShan::Util::getPropertyDataWithDefault(clientSessionParams, "lscp_peer_port", "", peerPort);

	char buf[60]="";
	TianShanIce::Properties lscParams;
	snprintf(buf, sizeof(buf)-2, "%d", lsc::LSC_DONE);

	MAPSET(TianShanIce::Properties, lscParams, CRMetaData_LscVersion,        "1");
	MAPSET(TianShanIce::Properties, lscParams, CRMetaData_LscOpCode,        buf);
	MAPSET(TianShanIce::Properties, lscParams, CRMetaData_LscStreamHandle,  streamHandle);
	MAPSET(TianShanIce::Properties, lscParams, CRMetaData_LscTransactionId, "0");
	MAPSET(TianShanIce::Properties, lscParams, CRMetaData_LscStatusCode,    "0");
	MAPSET(TianShanIce::Properties, lscParams, CRMetaData_ResponseMode,     mode);

	{
		std::string paramstr;
		for(TianShanIce::Properties::const_iterator itParam = streamParams.begin(); itParam != streamParams.end(); itParam++)
			paramstr += itParam->first + "=" + itParam->second + " ";

		envlog(ZQ::common::Log::L_INFO, CLOGFMT(StreamEvent, "HeNanLscDone(%s/%s) streamHandle[%s] stream params: %s"),
			streamId.c_str(), clientSessId.c_str(), streamHandle.c_str(), paramstr.c_str());
	}

	try
	{
		std::string currntNpt= "0"; // enh#16403

		//LSC_Response
		std::string scaleNumerator, scaleDenominator;
		{
			TianShanIce::Properties::const_iterator itSP = streamParams.find("sys.npt");
			if (streamParams.end() != itSP)
			{
				currntNpt = itSP->second;
				if (currntNpt.size()>= 3)
				{	
					currntNpt.replace(currntNpt.end() -3, currntNpt.end(), 3, '0');
				}
			}

			std::string strSpeed="1.1";
			itSP = streamParams.find("SPEED");
			if (streamParams.end()!= itSP)
				strSpeed = itSP->second;

			float f;
			sscanf(strSpeed.c_str() , "%f" , &f);
			lscp_PlayHandler::toFenshu(f, scaleNumerator, scaleDenominator);
		}

		MAPSET(TianShanIce::Properties, lscParams, CRMetaData_ResponseCurrentNpt, currntNpt);
		MAPSET(TianShanIce::Properties, lscParams, CRMetaData_ResponseNumerator,   scaleNumerator);
		MAPSET(TianShanIce::Properties, lscParams, CRMetaData_ResponseDenominator, scaleDenominator);

		int64 npt = 0 ;
		sscanf(currntNpt.c_str(),"%lld",&npt);


		_env.mStreamingInfoCache.updateInfo( clientSessId, npt, 0.0f );

		std::map<std::string,std::string>::iterator itorMd;
		std::string strTempData;
		for(itorMd = lscParams.begin(); itorMd != lscParams.end(); itorMd++)
		{
			strTempData+=itorMd->first + "=" + itorMd->second + " "; 
		}
		envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(StreamEvent, "HeNanLscDone(%s/%s): response params: %s") ,
			streamId.c_str(), clientSessId.c_str(), strTempData.c_str());

		ServerRequestPtr lscDone = _env.mGateway->createServerRequest(connId, clientSession);
		if (!lscDone)
		{
			envlog(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEvent, "HeNanLscDone(%s/%s)streamHandle[%s] failed to create message via connId[%lld]"), 
				streamId.c_str(), clientSessId.c_str(), streamHandle.c_str(), connId);
			return;
		}

		lscDone->getMessage()->setProperties(lscParams);
		lscDone->getMessage()->setCommand(COMMAND_DONE_RESPONSE);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEvent, "HeNanLscDone(%s/%s) sending streamHandle[%s]"),
			streamId.c_str(), clientSessId.c_str(), streamHandle.c_str());
		lscDone->updatePeerInfo(peerIp, peerPort);
		lscDone->complete();
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(StreamEvent, "HeNanLscDone(%s/%s) sent streamHandle[%s]"),
			streamId.c_str(), clientSessId.c_str(), streamHandle.c_str());

	}
	catch (...)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEvent, "HeNanLscDone(%s/%s) failed to sent streamHandle[%s] done response"),
			streamId.c_str(), clientSessId.c_str(), streamHandle.c_str());
	}
}

void EventSink::OnExit(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic) const
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEvent, "OnExit(%s)"), proxy.c_str());
}

void EventSink::OnExit2(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int nExitCode, const ::std::string& sReason, const ::TianShanIce::Properties& props, const ::Ice::Current&) const
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEvent, "OnExit2(%s)"), proxy.c_str());
}
void PlaylistEventSinkI::ping(::Ice::Long timestamp, const ::Ice::Current& c)
{
}

void PlaylistEventSinkI::OnItemStepped(const ::std::string& proxy, const ::std::string& id, ::Ice::Int currentUserCtrlNum,
										  ::Ice::Int prevUserCtrlNum, const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& c) const
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(PlaylistEventSinkI, "OnItemStepped() proxy[%s] playlistId[%s] CurrentUserCtrlNum[%d] prevUserCtrlNum[%d]props[%s]"),
		proxy.c_str(), id.c_str(), currentUserCtrlNum, prevUserCtrlNum, getPropertiesString(ItemProps).c_str());	

	int errorCode	= 0;
	ZQTianShan::Util::getPropertyDataWithDefault( ItemProps, "ItemSkipErrorCode", 0 , errorCode );	

	int dsmccErrorCode = -1;
	switch(errorCode)
	{
	case 4400:
		dsmccErrorCode = 44;
		break;
	case 5401:
		dsmccErrorCode = 54;
		break;
	case 5502:
		dsmccErrorCode = 55;
		break;
	case 5602:
		dsmccErrorCode = 56;
		break;
	case 6004:
		dsmccErrorCode = 60;
		break;
	default:
		break;
	}

	if(dsmccErrorCode == -1)
		return;
    _env.sendCSIR(id, dsmccErrorCode);
}
///PauseTimeoutEventSinkI

void PauseTimeoutEventSinkI::post(const ::std::string& category, ::Ice::Int eventId,  const ::std::string& eventName,
								  const ::std::string& stampUTC,  const ::std::string& sourceNetId,  const ::TianShanIce::Properties& params,
								  const ::Ice::Current& ic  )
{
	std::string playlistId; 
	ZQTianShan::Util::getPropertyDataWithDefault(params, "streamSessionId", "",  playlistId);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(PauseTimeoutEventSinkI, "Event(%s) [%s]"), eventName.c_str(), playlistId.c_str());

	int dsmccErrorCode = 21;
	_env.sendCSIR(playlistId, dsmccErrorCode);
}