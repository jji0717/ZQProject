
#include <ZQ_common_conf.h>
#include <time.h>
#include <sstream>
#include <math.h>
#include <TianShanIceHelper.h>
#include "NgodConfig.h"
#include "NgodHelper.h"
#include "RtspHeaderDefines.h"
#include "NgodEnv.h"
#include "NgodSessionManager.h"
#include "ServerResponse.h"
#include "SelectionCommand.h"


#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	"[%s]%s/%s/%s/%08X/REQUEST[%s]\t"##y, mRequest.sessionId.c_str() ,  mRequest.cseq.c_str(), mRequest.verbstr.c_str(), mRequest.ondemandId.c_str() , GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	"[%s]%s/%s/%s/%08X/REQUEST[%s]\t"y, mRequest.sessionId.c_str() ,  mRequest.cseq.c_str(), mRequest.verbstr.c_str(),mRequest.ondemandId.c_str() , pthread_self(),#x	
#endif	
namespace NGOD
{

#define LINETERM "\r\n"

ServerResponse::ServerResponse( NgodEnv& env , NgodClientRequestI& request )
:mEnv(env),
mRequest(request),
mbSendResponse(true),
mErrorCode(200)//success
{
}

ServerResponse::~ServerResponse(void)
{
	
}

void ServerResponse::post()
{
	if( mbSendResponse )
	{
		composeResponseMessage();
		
		int64 requestSpan = ZQTianShan::now() - mRequest.getStartTime();	

		if( !mRequest.post() )
		{
			onResponseFailed();
		}
		int64 totalSpan = ZQTianShan::now() - mRequest.getStartTime();

		MLOG(ZQ::common::Log::L_INFO, SESSFMT(ServerResponse,"request response[%s], execution timespan[%lld/%lld]") , 
			mStartLine.c_str() ,requestSpan ,totalSpan);
		setNoResponse( true );
	}
}

void ServerResponse::onResponseFailed( )
{
	//do nothing
}

#define STATUSOK(x) ( x >=200 && x < 299 )

void ServerResponse::composeResponseMessage( )
{	
	mRequest.setHeader( HeaderSession, mRequest.sessionId );
	mRequest.setHeader( HeaderOnDemandSessId , mRequest.ondemandId );	
	mRequest.setHeader( HeaderServer , mEnv.moduleName() );	
	mRequest.setHeader( HeaderMethodCode, mRequest.verbstr );
	
	if( STATUSOK(mErrorCode) )
	{	
		mStartLine = RESPONSE_OK;
		mRequest.setStartline( mStartLine );
	}
	else
	{
		mStartLine = errorCodeTransformer(mErrorCode);
		mRequest.setStartline( mStartLine );
		if ( stricmp( mRequest.userAgent.c_str() , "TianShan") == 0 )
		{
			mRequest.setHeader( HeaderTianShanNotice, mErrorReason );
		}
	}
}

void ServerResponse::setNoResponse( bool bNoResponse )
{
	mbSendResponse = !bNoResponse;
}
int ServerResponse::getErrorCode( ) const
{
	return mErrorCode;
}
void ServerResponse::setLastErr( int errCode , const char* fmt , ... )
{
	char szLocalBuffer[1024];
	

	va_list args;

	va_start(args, fmt);
	int nCount = _vsnprintf( szLocalBuffer, sizeof(szLocalBuffer)-1, fmt, args );
	va_end(args);
	if(nCount == -1)
	{
		szLocalBuffer[ sizeof(szLocalBuffer) - 1 ] = 0;
	}
	else
	{
		szLocalBuffer[nCount] = '\0';
	}

	mErrorReason	= szLocalBuffer;
	mErrorCode		= errCode;
}


//////////////////////////////////////////////////////////////////////////
///ServerResponseSetup
ServerResponseSetup::ServerResponseSetup( NgodEnv& env , NgodClientRequestI& request )
:ServerResponse( env , request )
{
}

ServerResponseSetup::~ServerResponseSetup()
{
}
void ServerResponseSetup::setPara( const SetupResponsePara& para )
{
	mPara = para;
}

void ServerResponseSetup::onResponseFailed( )
{	
	MLOG(ZQ::common::Log::L_ERROR,SESSFMT(ServerResponseSetup,"failed to post response to client, destroy the session"));
	mRequest.getSessionManager().destroySession( mRequest.sessionId ) ;
}

void ServerResponseSetup::composeResponseMessage( )
{
	ServerResponse::composeResponseMessage();
	if( !STATUSOK( mErrorCode ) )
		return;
	
	std::string strTransport = mRequest.originalTransport;
	std::ostringstream ossTransExt;ossTransExt << ";source="<<mPara.streamerSourceIp<<";server_port="<<mPara.streamerSourcePort;
	strTransport += ossTransExt.str();
	mRequest.setHeader( HeaderTransport , strTransport );
	//
	// compose sdp
	//
	mRequest.setHeader( HeaderContentType , "application/sdp");

	//copy from ssm_ngod2->setupHandler

	std::string strUserAgent = mRequest.userAgent;
	std::transform(strUserAgent.begin(),strUserAgent.end(), strUserAgent.begin(), tolower);
	if (strUserAgent.find("tianshan") != std::string::npos && ngodConfig.adsReplacment.provideLeadingAdsPlaytime)
	{
		char temp[256];
		snprintf(temp, sizeof(temp), "%.3f", mRequest.mPrimaryItemNPT);
		std::stringstream oss;
		oss << "primaryItemNPT=" << temp;
		mRequest.setHeader(HeaderTianShanNoticeParam, oss.str().c_str());
	}
	

	std::ostringstream oss;
	oss <<  "v=0"LINETERM;
	oss << "o=- "	<< mRequest.sessionId << " " << getGMTString() << " IN IP4 " + mRequest.mRtspServerIp << LINETERM;
	oss << "s="LINETERM;
	oss << "c=IN IP4 " << mRequest.mRtspServerIp << LINETERM;
	oss << "t=0 0"LINETERM;

	//TODO: add support to lscp in the future
	if( stricmp( ngodConfig.response.streamCtrlProt.c_str() ,"lscp") == 0 )
	{
		ZQ::common::Variant var;
		var = mRequest.getInfo(IStreamSmithSite::INFORMATION_LSCP_PORT);
		char szLscpPort[256];
		szLscpPort[ sizeof(szLscpPort)-1 ] = 0;
		snprintf(szLscpPort,sizeof(szLscpPort)-1,"%d",(int32)var);
		oss << "a=control:lscp://"+ mRequest.mRtspServerIp << ":"<<szLscpPort << "/" << mRequest.sessionId << LINETERM;
	}
	else
		oss << "a=control:rtsp://"+ mRequest.mRtspServerIp << ":"<<mRequest.mRtspServerPort << "/" << mRequest.sessionId << LINETERM;

	mRequest.setBody( oss.str() );

}

//////////////////////////////////////////////////////////////////////////
///ServerResponsePlay
ServerResponsePlay::ServerResponsePlay( NgodEnv& env , NgodClientRequestI& request )
:ServerResponse( env , request )
{
}
ServerResponsePlay::~ServerResponsePlay()
{

}

void ServerResponsePlay::setRange( const std::string& rangeStart , const std::string& rangeEnd )
{
	std::string range = std::string("npt=") + rangeStart ;
	range += "-";
	if( !rangeEnd.empty() )
		range += rangeEnd;
	mRequest.setHeader( HeaderRange , range );
}

void ServerResponsePlay::setScale( const std::string& scale )
{
	mRequest.setHeader( HeaderScale , scale );
}

void ServerResponsePlay::composeResponseMessage( )
{
	ServerResponse::composeResponseMessage();
}


//////////////////////////////////////////////////////////////////////////
///ServerResponsePause
ServerResponsePause::ServerResponsePause( NgodEnv& env , NgodClientRequestI& request )
:ServerResponse(env,request)
{
}
ServerResponsePause::~ServerResponsePause()
{
}

void ServerResponsePause::composeResponseMessage( )
{
	ServerResponse::composeResponseMessage();
}
void ServerResponsePause::setRange( const std::string& rangeStart , const std::string& rangeEnd )
{
	std::string range = std::string("npt=") + rangeStart ;
	range += "-";
	if( !rangeEnd.empty() )
		range += rangeEnd;
	mRequest.setHeader( HeaderRange , range );
}
void ServerResponsePause::setScale( const std::string& scale )
{
	mRequest.setHeader( HeaderScale , scale );
}

ServerResponseGetParameter::ServerResponseGetParameter( NgodEnv& env , NgodClientRequestI& request , NgodSessionManager& manager)
:ServerResponse( env, request ),
mSessManager(manager)
{
}
ServerResponseGetParameter::~ServerResponseGetParameter()
{
}

void ServerResponseGetParameter::setInfo( const GetParamResponseInfo& info )
{
	mInfo = info;	
}

void ServerResponseGetParameter::getSessionlist( std::ostringstream& oss )
{	
	const std::string& sessionGroupName = mGetParaInfo.sessionGroupName;

	NgodDatabase& db = mSessManager.getDatabase();

	NgodSessionPrxS sessions = db.openSessionByGroup( sessionGroupName );
	oss << "session_list: ";
	
	NgodSessionPrxS::iterator it = sessions.begin();
	for( ; it != sessions.end() ; it ++ )
	{
		NgodSessionPrx prx = *it;
		try
		{
			std::string sessId = prx->getSessionId();
			std::string onDemandId = prx->getOndemandSessId();
			oss << sessId << ":" << onDemandId <<" ";
		}
		catch( const Ice::Exception&)
		{
		}
	}
}
void ServerResponseGetParameter::setGetParamInfo( const GetInfoParam& info )
{
	mGetParaInfo = info;
}
void ServerResponseGetParameter::composeResponseMessage( )
{
	ServerResponse::composeResponseMessage();

	if( !STATUSOK( mErrorCode ) )
		return;
	
	mRequest.setHeader( HeaderContentType, "text/parameters");

	int32 requestMask = mGetParaInfo.mask;


	std::ostringstream oss;

	if( requestMask & GetInfoParam::MASK_SCALE )
		oss << "scale: " << mInfo.scale <<LINETERM ;	

	if( requestMask & GetInfoParam::MASK_POSITION )
		oss << "position: " << mInfo.rangeStart << LINETERM;

	if( requestMask & GetInfoParam::MASK_STATE )
		oss << "presentation_state: "<< mInfo.state << LINETERM;
	
	if( requestMask & GetInfoParam::MASK_TIMEOUT )
		oss<<"connection_timeout: " << ngodConfig.rtspSession.timeout/1000 << LINETERM;

	if( requestMask & GetInfoParam::MASK_SESSIONLIST )
	{
		// for NSS to sync
		mRequest.setHeader( HeaderSessionGroup, mGetParaInfo.sessionGroupName );
		getSessionlist( oss );
	}

	mRequest.setBody( oss.str() );

}

//////////////////////////////////////////////////////////////////////////
///ServerResponseSetParameter
ServerResponseSetParameter::ServerResponseSetParameter(  NgodEnv& env , NgodClientRequestI& request )
:ServerResponse( env , request )
{
}

ServerResponseSetParameter::~ServerResponseSetParameter( )
{
}

void ServerResponseSetParameter::composeResponseMessage( )
{
	ServerResponse::composeResponseMessage();
	mRequest.setHeader(HeaderContentType, "text/parameters");
}


std::string quoteStr( const std::string& str )
{
	std::string ret = "\"";
	ret += str;
	ret += "\"";
	return ret;
}
std::string quoteStr( int32 i )
{
	char szBuf[16];
	sprintf( szBuf , "%d" , i );
	std::string ret = "\"";
	ret += szBuf;
	ret += "\"";
	return ret;
}

//////////////////////////////////////////////////////////////////////////
///ServerResponseTeardown
ServerResponseTeardown::ServerResponseTeardown( NgodEnv& env , NgodClientRequestI& request )
:ServerResponse( env , request )
{
}

ServerResponseTeardown::~ServerResponseTeardown( )
{
}

void ServerResponseTeardown::outputSessionEvents(  const NGOD::SessionEventRecords& records , std::ostringstream& oss )
{
	oss<< "<EventHistory>" << LINETERM;
	NGOD::SessionEventRecords::const_iterator it = records.begin();
	bool bEndEventAdded = false;
	for( ; it != records.end(); it++ )
	{
		switch( it->eventType )
		{
		case NGOD::UserEvent:
			outputUserEvent( *it ,oss );
			break;

		case NGOD::StartStreamEvent:
			outputStartStreamEvent( *it , oss );
			break;

		case NGOD::EndEvent:
			if (!bEndEventAdded)
				outputEndEvent( *it ,oss  );
			bEndEventAdded = true;
			break;

		case NGOD::RecoverableError:
			outputRecoverableErrorEvent( *it ,oss );
			break;

		case NGOD::Transition:
			outputTraisitionEvent( *it ,oss );
			break;

		default:
			break;
		}
	}

	oss<<"</EventHistory>";
}
void ServerResponseTeardown::outputTraisitionEvent( const NGOD::SessionEventRecord& event , std::ostringstream& oss )
{
	std::string newState,scale,reason,newResId,newNpt;
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "newState" , "" ,newState );
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "scale" , "" ,scale );
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "newStreamResourcesID" , "" , newResId);
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "newNPT" , "" , newNpt);
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "reason" , "" , reason);


	oss	<< "<Transition time="			<< quoteStr(event.eventTime)
		<< " NPT="						<< quoteStr(event.NPT)
		<< " streamResourcesID="		<< quoteStr(event.streamResourceID)
		<< " newState="					<< quoteStr(newState)
		<< " scale="					<< quoteStr(scale)
		<< " reason="					<< quoteStr(reason)
		<< " newNPT="					<< quoteStr(newNpt)
		<< " newStreamResourcesID="		<< quoteStr(newResId)
		<< " />" << LINETERM;
}
void ServerResponseTeardown::outputUserEvent( const NGOD::SessionEventRecord& event , std::ostringstream& oss )
{
	std::string newState;
	std::string scale;
	std::string reqState;
	std::string reqScale;
	std::string reqRange;
	std::string status;
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "scale", "",scale );
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "newState", "", newState );
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "reqState", "", reqState );
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "reqRange", "", reqRange );
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "reqScale", "", reqScale );
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "status", "", status );
	oss << "<UserEvent time="		<< quoteStr(event.eventTime)
		<< " NPT="					<< quoteStr(event.NPT)
		<< " streamResourcesID="	<< quoteStr(event.streamResourceID)
		<< " newState="				<< quoteStr(newState)
		<< " scale="				<< quoteStr(scale)
		<< " reqState="				<< quoteStr(reqState)
		<< " status="				<< quoteStr(status);
	if(reqRange != "")
		oss << " reqRange="		<< quoteStr(reqRange);
	if(reqScale != "")
		oss << " reqScale="		<< quoteStr(reqScale);
	oss << " />" << LINETERM;
}

void ServerResponseTeardown::outputStreamEvent( const NGOD::SessionEventRecord& event , std::ostringstream& oss )
{
	std::string newState;
	std::string scale;
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "scale", "",scale );
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "newState", "", newState );
	oss	<< "<StartStreamEvent time="	<< quoteStr(event.eventTime)
		<< " NPT="						<< quoteStr(event.NPT)
		<< " streamResourcesID="		<< quoteStr(event.streamResourceID)
		<< " newState="					<< quoteStr(newState)
		<< " scale="					<< quoteStr(scale)
		<< " />" << LINETERM;
}
void ServerResponseTeardown::outputStartStreamEvent( const NGOD::SessionEventRecord& event , std::ostringstream& oss )
{
	std::string newState,scale;
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "newState", "" , newState );
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "scale" , "1.000", scale );
	oss	<< "<StartStreamEvent time="	<< quoteStr(event.eventTime)
		<< " NPT="						<< quoteStr(event.NPT)
		<< " streamResourcesID="		<< quoteStr(event.streamResourceID)
		<< " newState="					<< quoteStr(newState)
		<< " scale="					<< quoteStr(scale)
		<< " />" << LINETERM;	
}

void ServerResponseTeardown::outputEndEvent( const NGOD::SessionEventRecord& event , std::ostringstream& oss )
{
	std::string reason;
	std::string errorcode;
	std::string errdesc;
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "reason" , "" , reason );
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "errorCode" , "", errorcode );
	
	std::string npt = event.NPT;
	if (fabs(atof(npt.c_str())) < 0.1f) //bug#15606
	{
//		std::string tmp;
//		ZQTianShan::Util::getPropertyDataWithDefault( mProps , "PlayListTotalSize", "", tmp);
		npt = "EOS";
	}
		
	oss	<< "<EndEvent time="       << quoteStr(event.eventTime)
		<< " NPT="                 << quoteStr(npt)
		<< " streamResourcesID="   << quoteStr(event.streamResourceID)
		<< " reason="              << quoteStr(reason);

	if ( !errorcode.empty() )
	{
		ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "errorDescription" ,"" , errdesc );
		oss << " code="             << quoteStr(errorcode)
			<< " description="      << quoteStr(errdesc);
	}

	oss	<< " />" << LINETERM;
}
void ServerResponseTeardown::outputRecoverableErrorEvent( const NGOD::SessionEventRecord& event , std::ostringstream& oss )
{
	std::string newState,scale,errocode,errodesc,newResId,newNpt;
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "newState" , "" ,newState );
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "scale" , "" ,scale );
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "errorCode" , "" ,errocode );
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "errorDescription" , "" , errodesc);
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "newStreamResourcesID" , "" , newResId);
	ZQTianShan::Util::getPropertyDataWithDefault( event.prop , "newNPT" , "" , newNpt);
		
	
	oss	<< "<RecoverableError time="	<< quoteStr(event.eventTime)
		<< " NPT="						<< quoteStr(event.NPT)
		<< " streamResourcesID="		<< quoteStr(event.streamResourceID)
		<< " newState="					<< quoteStr(newState)
		<< " scale="					<< quoteStr(scale)
		<< " errorCode="				<< quoteStr(errocode)
		<< " errorDescription="			<< quoteStr(errodesc)
		<< " newNPT="					<< quoteStr(newNpt)
		<< " newStreamResourcesID="		<< quoteStr(newResId)
		<< " />" << LINETERM;

}

void ServerResponseTeardown::outputBasicInfo( const TeardownResponseParameter& para , std::ostringstream& oss )
{
	oss	<< " componentName="	<< quoteStr( para.componentName )
		<< " ODSessionID="		<< quoteStr( para.odsessid )
		<< " setupDate="		<< quoteStr( para.setupDate )
		<< " ODRMIpAddr="		<< quoteStr( para.remoteIp )
		<< " SMIpAddr="			<< quoteStr( para.localIp )
		<< " resultCode="		<< quoteStr( para.resultCode )
		<< " teardownDate="		<< quoteStr( para.teardownDate );
	if ( !para.sessGroupName.empty() )
	{
		oss << " sessionGroup="     << quoteStr(para.sessGroupName);
	}
	oss << " >" << LINETERM;
	oss << "<PlayoutHistory time=" << quoteStr( getISOTimeString() ) << " >" << LINETERM;
}

void ServerResponseTeardown::outputplInfo( const NGOD::PlaylistItemSetupInfos& plInfos , const TeardownResponseParameter& para, std::ostringstream& oss )
{
	NGOD::PlaylistItemSetupInfos::iterator it = (const_cast<NGOD::PlaylistItemSetupInfos&>(plInfos)).begin();
	int32 ctrlNum = 0;
	for ( ; it != plInfos.end() ; it ++ )
	{
		std::string pid;
		std::string paid;
		ZQTianShan::Util::getValueMapDataWithDefault( it->privateData,"providerId","",pid );
		ZQTianShan::Util::getValueMapDataWithDefault( it->privateData,"providerAssetId","",paid );
		++ctrlNum;
		
		oss	<< "<StreamResources " 
			<< " ID="				<< quoteStr(it->privateData["currentCtrlNum"].ints[0])
			<< " SOP="				<< quoteStr(para.sopname)
			<< " filename="			<< quoteStr(it->contentName)
			<< " providerID="		<< quoteStr(pid)
			<< " assetID="			<< quoteStr(paid)
			<< " segmentNumber="	<< quoteStr(ctrlNum);
		if(it->privateData.find("range") != it->privateData.end() && it->privateData["range"].strs[0] != "")
		{
			oss << " range="			<< quoteStr(it->privateData["range"].strs[0]);
		}
		if(it->flags > 0)
		{
			std::string restriction = "";
			if( (it->flags & TianShanIce::Streamer::PLISFlagNoFF) == TianShanIce::Streamer::PLISFlagNoFF)
				restriction += "F";
			if( (it->flags & TianShanIce::Streamer::PLISFlagNoRew) == TianShanIce::Streamer::PLISFlagNoRew)
				restriction += "R";
			if( (it->flags & TianShanIce::Streamer::PLISFlagNoPause) == TianShanIce::Streamer::PLISFlagNoPause)
				restriction += "P";

			//fdj
			if((it->flags & TianShanIce::Streamer::PLISFlagNoSeek) == TianShanIce::Streamer::PLISFlagNoSeek)
				restriction += "S";
			if((it->flags & TianShanIce::Streamer::PLISFlagSkipAtFF) == TianShanIce::Streamer::PLISFlagSkipAtFF)
				restriction += "K";
			if((it->flags & TianShanIce::Streamer::PLISFlagSkipAtRew) == TianShanIce::Streamer::PLISFlagSkipAtRew)
				restriction += "W";
			long playtimes = (long) (it->flags & TianShanIce::Streamer::PLISFlagPlayTimes >> 4);
			if(playtimes >0 && playtimes < 10)
			{
				char Cplaytimes[2] = {0};
				sprintf(Cplaytimes,"%ld",playtimes);
				restriction += Cplaytimes;
			}
			oss << " tricks="			<< quoteStr(restriction);
		}
		oss	<< " />" << LINETERM;
	}
}

void ServerResponseTeardown::setSessionHistoryData(  const TeardownResponseParameter& para, 
													const NGOD::PlaylistItemSetupInfos& plInfos ,  
													const NGOD::SessionEventRecords& records  )
{
	std::ostringstream oss;
	
	oss << "<ResponseData>" << LINETERM
		<< "<ODRMSessionHistory>" << LINETERM;
	
	if( ngodConfig.sessionHistory.enableHistory >= 1 )	
	{
		oss << "<ODRMSession " ;

		outputBasicInfo( para , oss );

		outputplInfo( plInfos , para , oss );

		outputSessionEvents( records , oss );
		
		oss<<"</PlayoutHistory>"<<LINETERM;

		oss <<"</ODRMSession>";
	}

	oss << "</ODRMSessionHistory>"<< LINETERM
		<< "</ResponseData>"      << LINETERM;
	
	mRequest.setBody( oss.str() );	
	
	//record the information
	mPara = para;
}


void ServerResponseTeardown::composeResponseMessage( )
{
	ServerResponse::composeResponseMessage();

	if( !STATUSOK( mErrorCode ) )
		return;
	

	{//fill stop point 		
		std::ostringstream oss;
		oss<< mPara.assetIndex << " " << mPara.assetBasedNpt;		
		mRequest.setHeader( HeaderStopPoint , oss.str() );
	}

	char buf[64]="", *p=buf;
	snprintf(p, buf + sizeof(buf)-2 -p, "npt="); p+=strlen(p);
	char* finalNpt = p;
	snprintf(p, buf + sizeof(buf)-2 -p, "%d.%03d", (int32) (mPara.npt/1000), (int32) (mPara.npt%1000)); p+=strlen(p);
	mRequest.setHeader(HeaderFinalNPT, finalNpt); // NGOD-525, ComcastECR: Last Position, ticket#12844
	
	*p++= '-'; *p= 0x00;
	if (mPara.playTime > 0)
	{
		snprintf(p, buf + sizeof(buf)-2 -p, "%d.%03d", (int32) (mPara.playTime/1000), (int32) (mPara.playTime%1000)); p+=strlen(p);
	}

	if( ngodConfig.announce.includeTeardownRange >= 1 )
	{
		//sent out range in teardown response according to NGOD-472	
		mRequest.setHeader(HeaderRange, buf);
	}

	mRequest.setHeader( HeaderContentType, "text/xml");
}


}
