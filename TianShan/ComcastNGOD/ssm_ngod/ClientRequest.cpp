
#include <ZQ_common_conf.h>
#include <assert.h>
#include <strHelper.h>
#include <TsStreamer.h>
#include "NgodConfig.h"
#include "NgodHelper.h"
#include "NgodEnv.h"
#include "RtspHeaderDefines.h"
#include "NgodSessionManager.h"
#include "ClientRequest.h"
#include "ServerResponse.h"
#include "SelectionCommand.h"
#include <TianShanIceHelper.h>

#if defined ZQ_OS_MSWIN
#define	SESSFMT(x,y) 	"[%s]%s/%s/%s/%08X/REQUEST[%s]\t"##y, sessionId.c_str(), cseq.c_str(), verbstr.c_str(), ondemandId.c_str(), GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
#define	SESSFMT(x,y) 	"[%s]%s/%s/%s/%08X/REQUEST[%s]\t"y, sessionId.c_str(), cseq.c_str(), verbstr.c_str(), ondemandId.c_str(), pthread_self(),#x	
#endif	

#define EVENTLOGFMT2(_X, _T) CLOGFMT(_X, "Sess(%s)Seq(%s)Req(%p)Mtd(%s)ODSess(%s) " _T), sessionId.c_str(), cseq.c_str(), this, verbstr.c_str(), ondemandId.c_str()

namespace NGOD
{


///convert RTSP verb code defined in StreamSmithModule.h into RequestVerb
NGOD::RequestVerb convertVerbCode( REQUEST_VerbCode verb )
{
	switch ( verb )
	{
	case RTSP_MTHD_DESCRIBE:
		return NGOD::requestDESCRIBE;
	case RTSP_MTHD_PLAY:
		return NGOD::requestPLAY;
	case RTSP_MTHD_RECORD:
		return NGOD::requestRECORD;
	case RTSP_MTHD_SETUP:
		return NGOD::requestSETUP;
	case RTSP_MTHD_TEARDOWN:
		return NGOD::requestTEARDOWN;
	case RTSP_MTHD_PAUSE:
		return NGOD::requestPAUSE;
	case RTSP_MTHD_GET_PARAMETER:
		return NGOD::requestGETPARAMETER;
	case RTSP_MTHD_OPTIONS:
		return NGOD::requestOPTIONS;
	case RTSP_MTHD_SET_PARAMETER:
		return NGOD::requestSETPARAMETER;
	case RTSP_MTHD_RESPONSE:
		return NGOD::requestRESPONSE;
	default:
		return NGOD::requestUnknown;
	}
}

const char* verbCodeString( NGOD::RequestVerb verb )
{
	switch ( verb )
	{
	case NGOD::requestDESCRIBE:
		return "DESCRIBE";
	case NGOD::requestPLAY:
		return "PLAY";
	case NGOD::requestRECORD:
		return "RECORD";
	case NGOD::requestSETUP:
		return "SETUP";
	case NGOD::requestTEARDOWN:
		return "TEARDOWN";
	case NGOD::requestPAUSE:
		return "PAUSE";
	case NGOD::requestGETPARAMETER:
		return "GETPARAMETER";
	case NGOD::requestOPTIONS:
		return "OPTIONS";
	case NGOD::requestSETPARAMETER:
		return "SETPARAMETER";
	case NGOD::requestRESPONSE:
		return "RESPONSE";
	default:
		return "UNKNOWN";
	}
}

//////////////////////////////////////////////////////////////////////////
///NgodClientRequestI
NgodClientRequestI::NgodClientRequestI( NgodEnv& env, NgodSessionManager& manager, IClientRequest* request)
:mEnv(env),
mRequest(request),
mSessManager(manager),
mbNeedVerCode(true),
mbR2Request(true)
{
	assert( mRequest != NULL );
	mRequest->addRef();
	mResponse = mRequest->getResponse();
	assert( mResponse != NULL );
	mStartTime = ZQTianShan::now();
	mPrimaryItemNPT = 0.00f;
	mPrimaryItemEnd = 0.00f;
}

NgodClientRequestI::~NgodClientRequestI( void )
{
	if( mServerResponse )
	{
		ZQTianShan::Util::TimeSpan span; span.start();
		mServerResponse->post();
		if( span.stop() > 100 )
		{
			MLOG(ZQ::common::Log::L_WARNING, SESSFMT(NgodClientRequestI,"postResponse() time cost[%lld]"),
				span.span() );
		}
		mServerResponse = NULL;
	}
	mRequest->release();
}

#define initLocalBuffer() char szLocalBuffer[1024]; szLocalBuffer[sizeof(szLocalBuffer)-1]=0;uint16 bufSize = sizeof(szLocalBuffer)-1;
#define SafeInitBuffer(x) (x[sizeof(x)-1] = 0)
#define STRVALID(x) (x && x[0] != 0 )

#ifdef ZQ_OS_LINUX
	#ifndef stricmp
		#define	stricmp strcasecmp
	#endif

	#ifndef strnicmp
		#define strnicmp strncasecmp
	#endif
#endif

#define STRSWITCH() if(0){
#define STRCASE(x,y)	}else if( stricmp(x,y) == 0 ) {
#define STREND()	}

int32 NgodClientRequestI::getBodySize( ) const
{
	initLocalBuffer();
	const char* pSize= mRequest->getHeader( HeaderContentLength, szLocalBuffer, &bufSize);
	if( !STRVALID(pSize) )
		return 0;
	return atoi(pSize);
}

bool NgodClientRequestI::getContentBody( std::string& body ) const
{
	body = "";
	int32 bodySize =getBodySize();
	if( bodySize <= 0 )
		return true;
	++bodySize;
	char* p = new char[bodySize + 1];
	assert( p != NULL );
	p[bodySize] = 0;

	uint32 sz = bodySize;
	const char* pContent = mRequest->getContent( (unsigned char*)p, &sz );
	if(!STRVALID(pContent))
	{
		delete[] p;
		return false;
	}
	else
	{
		body = pContent;
		delete[] p;
		return true;
	}

}
std::string supportProtocol( bool bR2 )
{
	if( bR2)
	{
		return std::string("com.comcast.ngod.r2,com.comcast.ngod.r2.decimal_npts");
	}
	else
	{
		return std::string("com.comcast.ngod.c1,com.comcast.ngod.c1.decimal_npts");
	}
}

bool NgodClientRequestI::parseRequest( )
{
	initLocalBuffer();
	
	//get session id
	const char* pSessionId = mRequest->getHeader( HeaderSession, szLocalBuffer, &bufSize );
	if( STRVALID( pSessionId ) )
	{
		sessionId	= pSessionId;
	}

	//get cseq
	bufSize = sizeof(szLocalBuffer) - 1;
	const char* pCSeq = mRequest->getHeader( HeaderSequence, szLocalBuffer, &bufSize);
	if( STRVALID( pCSeq) )
	{
		cseq = pCSeq;
	}

	//get user agent
	bufSize = sizeof(szLocalBuffer) - 1;
	const char* pUserAgent = mRequest->getHeader( HeaderUserAgent, szLocalBuffer, &bufSize);
	if( STRVALID( pUserAgent) )
	{
		userAgent = pUserAgent;
	}

	//get OnDemandSessionId
	bufSize = sizeof(szLocalBuffer) - 1;
	const char* pOndemandId = mRequest->getHeader( HeaderOnDemandSessId, szLocalBuffer, &bufSize );
	if( STRVALID(pOndemandId) )
	{
		ondemandId = pOndemandId;
	}
	
	//get connection id
	bufSize	= sizeof(szLocalBuffer) - 1;
	const char* pConnectionId = mRequest->getHeader( "SYS#ConnID", szLocalBuffer, &bufSize );
	if( STRVALID(pConnectionId) )
	{
		connectionId = pConnectionId;
	}

	verb			= convertVerbCode( mRequest->getVerb() );
	verbstr			= verbCodeString( verb );

	//get server Ip
	bufSize	= sizeof(szLocalBuffer) - 1;
	const char* pServerIp = mRequest->getHeader( "SYS#LocalServerIP", szLocalBuffer, & bufSize );
	if( STRVALID(pServerIp) )
	{
		mRtspServerIp = pServerIp;
	}
	
	//get server port
	bufSize	= sizeof(szLocalBuffer) - 1;
	const char* pServerPort = mRequest->getHeader( "SYS#LocalServerPort", szLocalBuffer, & bufSize );
	if( STRVALID(pServerPort) )
	{
		mRtspServerPort = pServerPort;
	}
	
	bufSize	= sizeof(szLocalBuffer) - 1;
	const char* pProtocolVersion = mRequest->getHeader( HeaderRequire,szLocalBuffer,&bufSize );

	std::string protocolVersionStr;
	if( STRVALID(pProtocolVersion) )
	{
		protocolVersionStr = pProtocolVersion;
		protocolVerCode = parseProtocolVersionCode( pProtocolVersion );
	}
	else
	{
		protocolVerCode = NgodVerCode_UNKNOWN;
	}

	bufSize	= sizeof(szLocalBuffer) - 1;
	const char* pUrl = mRequest->getUri( szLocalBuffer, bufSize);
	if( STRVALID(pUrl))
	{
		originalUrl = pUrl;
	}

	//check if protocol version code is good
	if( ngodConfig.protocolVersioning.enableVersioning >= 1 && mbNeedVerCode )
	{
		// ticket#18521: fixup the version code per customer="henan"
		if (0 == ngodConfig.protocolVersioning.customer.compare("henan"))
		{
			switch(protocolVerCode)
			{
			case NgodVerCode_R2:
				protocolVerCode = NgodVerCode_R2_DecNpt;
				break;

			case NgodVerCode_C1:
				protocolVerCode = NgodVerCode_C1_DecNpt;
				break;
			}
		}

		if( protocolVerCode >= NgodVerCode_UNKNOWN )
		{			
			if( protocolVersionStr.empty() )
			{
				setHeader( HeaderSupported, supportProtocol( mbR2Request ) );
				setHeader( HeaderUnsupported, "Require header is empty" );
			}
			else
			{
				setHeader( HeaderUnsupported, protocolVersionStr );
			}
			mServerResponse->setLastErr( errorcodeOptionNotSupport, "invalid require header passed in");
			return false;
		}
	}

	return true;
}

void NgodClientRequestI::setStartline( const ::std::string& startLine, const ::Ice::Current& /*= ::Ice::Current()*/) 
{
	assert( mResponse != NULL );
	mResponse->printf_preheader( startLine.c_str() );
}
void NgodClientRequestI::setHeader( const ::std::string& key, const ::std::string& value, const ::Ice::Current& /*= ::Ice::Current()*/)
{
	assert( mResponse != NULL );
	mResponse->setHeader( key.c_str(), value.c_str() );
}
void NgodClientRequestI::setBody( const ::std::string& body, const ::Ice::Current& /*= ::Ice::Current()*/)
{
	assert( mResponse != NULL );
	mResponse->printf_postheader( body.c_str() );
}

bool NgodClientRequestI::post(const ::Ice::Current& /*= ::Ice::Current()*/) 
{
	assert( mResponse != NULL );
	return mResponse->post() > 0 ;
}
ServerResponsePtr NgodClientRequestI::getResponse( )
{
	return mServerResponse;
}

int64 NgodClientRequestI::getStartTime( ) const
{
	return mStartTime;
}

NgodSessionManager&	NgodClientRequestI::getSessionManager()
{
	return mSessManager;
}

bool NgodClientRequestI::process( )
{
	NGOD::NgodSessionPrx sess = mSessManager.openSession( sessionId )	;
	if( !sess )
	{
		mServerResponse->setLastErr( errorcodeSessNotFound,"failed to find session[%s]",sessionId.c_str() );
		return false;
	}
	try
	{
		return sess->processRequest(this);
	}
	catch( const Ice::ObjectNotExistException& )
	{
		mServerResponse->setLastErr( errorcodeSessNotFound,"failed to find session[%s]",sessionId.c_str() );
	}
	catch( const Ice::Exception& ex)
	{
		mServerResponse->setLastErr( errorcodeInternalError,"failed to process command for session[%s], due to[%s]",
			sessionId.c_str(), ex.ice_name().c_str() );
	}
	return false;
}

ZQ::common::Variant     NgodClientRequestI::getInfo( int32 infoType )
{
	return (mSessManager.getSite())->getInfo(infoType);
}

NgodProtoclVersionCode	parseProtocolVersionCode( const std::string& str )
{
	std::vector<std::string> requires;
	ZQ::common::stringHelper::SplitString( str, requires, ", ;", ", ;");

	NgodProtoclVersionCode verCode =  NgodVerCode_UNKNOWN ;

	std::vector<std::string>::const_iterator itRequire = requires.begin();
	for( ; itRequire != requires.end() ; itRequire++ )
	{
		STRSWITCH()
			STRCASE( itRequire->c_str(), "com.comcast.ngod.r2" )
				verCode = NgodVerCode_R2;
			STRCASE( itRequire->c_str(), "com.comcast.ngod.r2.decimal_npts" )
				verCode = NgodVerCode_R2_DecNpt;
			STRCASE( itRequire->c_str(), "com.comcast.ngod.c1");
				verCode = NgodVerCode_C1;
			STRCASE( itRequire->c_str(), "com.comcast.ngod.c1.decimal_npts" )
				verCode = NgodVerCode_C1_DecNpt;
		STREND()
	}
	return verCode;
}

int64 convertToNumberWithPrototcol( const std::string& str, NgodProtoclVersionCode code )
{
	if( str.empty() )
		return 0 ;

	int64 ret	= 0;
	double fTmp = 0.0f;
	switch ( code )
	{
	case NgodVerCode_C1:
	case NgodVerCode_R2:
		{
			sscanf( str.c_str(), "%llx", &ret );
		}
		break;

	case NgodVerCode_C1_DecNpt:
	case NgodVerCode_R2_DecNpt:
		{
			sscanf( str.c_str(), "%lf", &fTmp);
			ret = (int64)(fTmp * 1000);
		}
		break;
	default:
		ret = 0;
		break;
	}
	return ret;
}
//////////////////////////////////////////////////////////////////////////
///NgodRequestSetup
NgodRequestSetup::NgodRequestSetup( NgodEnv& env, NgodSessionManager& manager, IClientRequest* request )
:NgodClientRequestI(env,manager,request)
{
	mbR2Request = true;

	mServerResponse = new ServerResponseSetup( mEnv, *this );
	assert( mServerResponse );
}

NgodRequestSetup::~NgodRequestSetup( )
{
	
}

const SetupParam& NgodRequestSetup::getSetupParam( ) const
{
	return mSetupParam;
}

bool NgodRequestSetup::process( )
{	
	if( !parseRequest() )
	{
		mServerResponse->setLastErr( errorcodeBadRequest, "failed to parse setup request" );
		return false;
	}

	NGOD::NgodSessionPrx sess = mSessManager.creatSession( mSetupParam.ondemandSessId, mSetupParam.requestUri );
	
	if( !sess )
	{
		mServerResponse->setLastErr(503,"failed to create session");
		return false;
	}

	sessionId = sess->getSessionId();

	MLOG(ZQ::common::Log::L_INFO, SESSFMT(NgodRequestSetup,"setup with sop[%s] bw[%lld] volume[%s] sessgroup[%s] pokeSess[%s] "),
		mSetupParam.sopName.c_str(), mSetupParam.requestBW, mSetupParam.requestVolume.c_str(), 
		mSetupParam.sessionGroupName.c_str(), mSetupParam.pokeHoleSessId.c_str() );
	if(ngodConfig.publishLogs.enabled)
	{
		std::string strPAID = "";
		if (!mSetupParam.playlist.empty())
			strPAID = mSetupParam.playlist[0].paid;
		ELOG(ZQ::common::Log::L_INFO, EVENTLOGFMT2(NgodRequestSetup,"SessionHistoryLog ClientSessionId(%s)OnDemandSessionId(%s)SOP(%s)Destination(%s)Port(%d)PAID(%s)Bandwidth(%ld)"),
			mSetupParam.pokeHoleSessId.c_str(), mSetupParam.ondemandSessId.c_str(), mSetupParam.sopName.c_str(), mSetupParam.destIp.c_str(), mSetupParam.destPort, strPAID.c_str(), mSetupParam.requestBW);
	}

	std::vector<SetupParam::PlaylistItemInfo>::iterator it;
	bool primaryFound = false;
	if (ngodConfig.adsReplacment.enabled) // if all items are ads, take the last one as primary asset
	{
		for (it = mSetupParam.playlist.begin(); it != mSetupParam.playlist.end(); it++) 
		{
			////// merged V1.10 logic about ads replacement
			if (ngodConfig.adsReplacment.enabled)
			{				

				for(AdsReplacment::AdsProviderHolderS::iterator itAdPid =  ngodConfig.adsReplacment.providers.begin(); itAdPid != ngodConfig.adsReplacment.providers.end(); itAdPid++)
				{
					if (0 != stricmp(it->pid.c_str(), itAdPid->pid.c_str())) // non-ads
						continue;

					it->primaryAsset = false; // setting this to false will force to calcuate mPrimaryItemNPT/mPrimaryItemEnd at NgodRequestSetup::process()
					MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(NgodRequestSetup, "asset[%s#%s] known as an ads, set it to non-primary and appling restriction[%x] by [%x]"), it->pid.c_str(), it->paid.c_str(), it->restrictionFlag, ngodConfig.adsReplacment.adsRestricts);
					it->restrictionFlag |= ngodConfig.adsReplacment.adsRestricts;

					const char* p = strchr(it->range.c_str(), '-');

					if (NULL == p || atoi(p+1) <=0)
						MLOG(ZQ::common::Log::L_WARNING, SESSFMT(NgodRequestSetup, "Ads[%s#%s] without length: range[%s]"), it->pid.c_str(), it->paid.c_str(), it->range.c_str());
				}
			}
			
			// if (it->primaryAsset)
			//	break;

			 if (it->primaryAsset)
				primaryFound = true;

			if (!primaryFound && it == mSetupParam.playlist.end()-1) // if there are all ads in the list, take the last one as primaryAsset
				it->primaryAsset = true;
		} 
	}
	
	// determin primary start and end
	mPrimaryItemNPT = mPrimaryItemEnd =0.0f;
	std::vector <long> primaryNpts;
	long npt =0;
	int  cAds =0;
	for (it = mSetupParam.playlist.begin(); it != mSetupParam.playlist.end(); it++)
	{
		MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(NgodRequestSetup, "npt-scan PLI(%s,%s,%s,%c,%lld,%lld,%llx,%s)"), 
	           it->pid.c_str(), it->paid.c_str(), it->sid.c_str(), it->primaryAsset?'T':'F', 
			   it->cuein, it->cueout, it->restrictionFlag, it->range.c_str());
		if (it->primaryAsset)
			primaryNpts.push_back(npt);
//TODO:
		npt += (long)(it->cueout - it->cuein);
		if (it->primaryAsset)
			primaryNpts.push_back(npt);
		else cAds++;
	}
	
	if (primaryNpts.size() >1 && primaryNpts.size()%2 ==0)
	{
		mPrimaryItemNPT = (float) primaryNpts[0] / 1000;
		mPrimaryItemEnd = (float) primaryNpts[primaryNpts.size()-1] /1000;
	}
	
	if (mPrimaryItemEnd <= mPrimaryItemNPT)
		mPrimaryItemEnd =0.0f;

	if( errorcodeOK != sess->processRequest(this) )
	{
		mSessManager.destroySession( sessionId );
		return false;
	}

	MLOG(ZQ::common::Log::L_INFO, SESSFMT(NgodRequestSetup, "playlist created: size[%d] clips[%d:%d] primary[%.3f~%.3f]"),
		  mSetupParam.playlist.size(), primaryNpts.size(), cAds, mPrimaryItemNPT, mPrimaryItemEnd);

	return true;
}

void NgodRequestSetup::parseSessionGroup( )
{
	initLocalBuffer();
	const char* pSessionGroup =mRequest->getHeader( HeaderSessionGroup, szLocalBuffer, & bufSize);
	if( STRVALID(pSessionGroup) )
	{
		mSetupParam.sessionGroupName = pSessionGroup;
	}
}

void NgodRequestSetup::parseVolume()
{
	initLocalBuffer();
	const char* pVolume = mRequest->getHeader( HeaderVolume, szLocalBuffer, &bufSize );
	if( STRVALID(pVolume) && ( stricmp(pVolume,"library") != 0 ) )
	{	
		mSetupParam.requestVolume = pVolume;		
	}
}

void NgodRequestSetup::parseStartpoint( )
{
	initLocalBuffer();
	const char* pStartpoint = mRequest->getHeader( HeaderStartpoint, szLocalBuffer, &bufSize );
	if( !STRVALID(pStartpoint))
		return;

	std::vector<std::string> rets ;
	ZQ::common::stringHelper::SplitString( pStartpoint, rets, " "," ");
	if( rets.size() < 2 )
	{
		MLOG(ZQ::common::Log::L_WARNING, SESSFMT(NgodRequestSetup,"parseStartpoint() got startpoint but invalid format"));
		return;
	}

	mSetupParam.startPoint.index	= atoi( rets[0].c_str() );
	mSetupParam.startPoint.offset	= convertToNumberWithPrototcol(  rets[1], (NgodProtoclVersionCode)protocolVerCode );;
}

void NgodRequestSetup::parseTransport( )
{
	initLocalBuffer();
	const char* pTransport = mRequest->getHeader( HeaderTransport, szLocalBuffer, &bufSize );
	if( STRVALID(pTransport) )		
	{
		KVPairGroupWalker walker( pTransport, ";");
		KVPairGroupWalker::const_iterator it = walker.begin();
		for( ; it != walker.end() ; it ++ )
		{
			const std::string& value = it->value;
			STRSWITCH()
				STRCASE( it->key.c_str(), "client_port" )	
				mSetupParam.destPort	= value.empty() ? 0 : atoi( value.c_str() ) ;
			STRCASE( it->key.c_str(), "bandwidth" )
				sscanf( value.c_str(), "%lld", &mSetupParam.requestBW );
			STRCASE( it->key.c_str(), "sop_name" )
				mSetupParam.sopName	= value;
			STRCASE( it->key.c_str(), "sop_group" )
				mSetupParam.sopName	= value;
			STRCASE( it->key.c_str(), "destination" )
				mSetupParam.destIp	= value;
			STRCASE( it->key.c_str(), "client" )
				mSetupParam.destMac	= value;
			STRCASE( it->key.c_str(), "source" )
				mSetupParam.serverPort= value.empty() ? 0 : atoi( value.c_str() );
			STREND()
		}

		originalTransport = pTransport;
	}

	//get client Id
	{
		bufSize = sizeof(szLocalBuffer);
		const char* pClientId = mRequest->getHeader( HeaderClientSessId, szLocalBuffer, &bufSize );
		if( STRVALID(pClientId) )
		{
			mSetupParam.pokeHoleSessId = szLocalBuffer;
			if( mSetupParam.destMac.empty())		
			{
				std::string tmpDestMac = szLocalBuffer;
				std::size_t pos = tmpDestMac.find_first_of("/\\");
				if (std::string::npos != pos)
					mClientId.erase(pos);

				while (std::string::npos != (pos = tmpDestMac.find_first_of(" \t-:")))
					tmpDestMac.erase(pos, 1);

				if (tmpDestMac.length() >12)
					tmpDestMac.erase(12);
				mSetupParam.destMac = tmpDestMac;
			}	
		}			
	}
}

void NgodRequestSetup::parseEncryptionData( const std::string& str )
{
	std::string::size_type posColon = str.find(':');
	if( !POSOK(posColon) )
	{
		MLOG(ZQ::common::Log::L_WARNING, SESSFMT(parseEncryptionData,"invalid encryption data format, no ':' is found, skip"));
		return;
	}

	std::vector<std::string> tmpVec;
	ZQ::common::stringHelper::SplitString( str.substr( posColon + 1), tmpVec, " \t", " \t\r\n", "","");
	if( tmpVec.size() < 5 )
	{
		MLOG(ZQ::common::Log::L_WARNING, SESSFMT(parseEncryptionData,"invalid encryption format [%s], skip"), str.c_str() );
		return;
	}

	SetupParam::EncryptionDataKey key;
	SetupParam::EncryptionData data;
	key.pid		= tmpVec[0];
	key.paid	= tmpVec[1];

	data.iProgNum	= tmpVec[2].empty() ? 0 : atoi(tmpVec[2].c_str() ) ;
	data.iFrq1		= tmpVec[3].empty() ? 0 : atoi(tmpVec[3].c_str() ) ;
	int keyNum		= tmpVec[4].empty() ? 0 : atoi(tmpVec[4].c_str() ) ;	

	//Actually I don't understand the meaning of each parameter here, just follow the old implementation
	if ( keyNum > 0 && (int)tmpVec.size() >= (5 + keyNum * 2) )
	{
		for (int tCur = 0; tCur < keyNum; tCur ++)
		{
			int tv = atoi( tmpVec[5 + tCur * 2].c_str() );					
			data.vecKeyOffset.push_back (tv);

			std::string tmpStr = tmpVec[6 + tCur * 2];					
			data.vecKeys.push_back (tmpStr);
		}
	}

	mSetupParam.encryptData[ key ] = data;
}

void NgodRequestSetup::parsePlaylistItem( const std::string& str )
{
	std::string::size_type posColon = str.find(':');
	if( !POSOK(posColon) )
	{
		MLOG(ZQ::common::Log::L_WARNING, SESSFMT(parsePlaylistItem,"invalid playlist format, no ':' is found, skip"));
		return;
	}

	posColon ++;

	std::vector<std::string> tmpVec;	
	ZQ::common::stringHelper::SplitString(str.substr(posColon),tmpVec, " \t", " \t\r\n","","",4);//at least 3 parts
	if( tmpVec.size() < 2 )
	{
		MLOG( ZQ::common::Log::L_WARNING, SESSFMT(parsePlaylistItem,"invalid playlist format, parameter not enough [%s], skip "),str.c_str() );
		return;
	}

	SetupParam::PlaylistItemInfo info;
	info.pid		= tmpVec[0];
	info.paid		= tmpVec[1];
	info.cuein = info.cueout =0;
	info.restrictionFlag =0;

	int lastIdxEOA = -1;
	std::string lastDescEOA;

	if( tmpVec.size() >= 3 )
	{
		int itempMaxSize = (int)tmpVec.size();
		// itempMaxSize = min(itempMaxSize,4);

		for ( int i = 2 ; i< itempMaxSize ;i++ )
		{
			std::string strTemp = tmpVec[i];
			// transform(strTemp.begin(),strTemp.end(), strTemp.begin(), tolower); 
			// std::string::size_type trickPos = strTemp.find("tricks/");

			// step1. test if the string token is about trick restriction
			if ( 0 ==  strTemp.find("tricks/"))
			{
				size_t j =0;
				for(j= sizeof("tricks/") -1; j < strTemp.length(); j++ )
				{
					switch(strTemp[j])
					{
					case 'R':
						info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoRew;
						break;
					case 'F':
						info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoFF;
						break;
					case 'P':
						info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoPause;
						break;
					case 'S':
						info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoSeek;
						break;
					case 'K':
						info.restrictionFlag |= TianShanIce::Streamer::PLISFlagSkipAtFF;
						break;
					case 'W':
						info.restrictionFlag |= TianShanIce::Streamer::PLISFlagSkipAtRew;
						break;
					default:
						break;
					}
				}

				j = strTemp.length()-1;
				if(strTemp[j] > '0' && strTemp[j] <= '9')
				{
					int playtimes = strTemp[j] - '0';
					info.restrictionFlag |= (playtimes & 0x0f) << 4;
				}

				continue;
			}

			// step2. test if the string token is about range
			std::string::size_type posDash = strTemp.find('-');
			if (std::string::npos != posDash)
			{
				info.range = strTemp;
				info.cuein	= convertToNumberWithPrototcol( strTemp.substr( 0, posDash), (NgodProtoclVersionCode)protocolVerCode );
				info.cueout	= convertToNumberWithPrototcol( strTemp.substr( posDash + 1 ), (NgodProtoclVersionCode)protocolVerCode );
				continue;
			}

			if (0 ==  strTemp.find("ADStatus/"))
			{
				info.extAds = strTemp;
				continue;
			}
		}		
	}

	if (ngodConfig.adsReplacment.enabled)
	{
		NGOD::AdsReplacment::AdsProviderHolderS::iterator iter =  ngodConfig.adsReplacment.providers.begin();
		for(; iter != ngodConfig.adsReplacment.providers.end(); iter++)
		{
			if (!stricmp(info.pid.c_str(),iter->pid.c_str())) // ads
			{
				info.primaryAsset = false;

				if(info.cueout <= 0)
				{
					MLOG( ZQ::common::Log::L_ERROR, SESSFMT(PrepareItemInfoCommand, "Ads[%s#%s] without length"), info.pid.c_str(), info.paid.c_str());
				}

				if (0 == info.restrictionFlag)
				{
					if (strstr(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(), "F"))
						info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoFF;

					if (strstr(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(), "R"))
						info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoRew;

					if (strstr(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(), "P"))
						info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoPause;

					//fdj
					if (strstr(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(), "S"))
						info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoSeek;

					if (strstr(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(), "K"))
						info.restrictionFlag |= TianShanIce::Streamer::PLISFlagSkipAtFF;

					if (strstr(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(), "W"))
						info.restrictionFlag |= TianShanIce::Streamer::PLISFlagSkipAtRew;

					char playtimes = 0;
					sscanf(ngodConfig.adsReplacment.defaultTrickRestriction.c_str(),"%*[^0-9]%c",&playtimes);
					if( playtimes > '0' && playtimes <= '9')
						info.restrictionFlag |= ((int)(playtimes-'0'))<<4;

				}
// 				//if( ngodConfig.adsReplacment.playOnce)
// 				if(ngodConfig.adsReplacment.provideLeadingAdsPlaytime)//fdj
// 				{
// 					//info.restrictionFlag += TianShanIce::Streamer::PLISFlagOnce;
// 					info.restrictionFlag += TianShanIce::Streamer::PLISFlagPlayTimes;
// 					MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(PrepareItemInfoCommand,"Ads[%s#%s] playOnce -> playTimes"), info.pid.c_str(), info.paid.c_str() );
// 				}
			}
		}
	}

	info.sid	= mClientId;

	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(PrepareItemInfoCommand,"xplaylist[%s] to PLI(%s,%s,%s,%c,%lld,%lld,%llx,%s)"), str.c_str(), 
	           info.pid.c_str(), info.paid.c_str(), info.sid.c_str(), info.primaryAsset?'T':'F', 
			   info.cuein, info.cueout, info.restrictionFlag, info.range.c_str());

	mSetupParam.playlist.push_back( info );
}

void NgodRequestSetup::parseSDP( )
{
	std::string strBody;
	getContentBody( strBody );
	if(strBody.empty())
		return;

	KVPairGroupWalker walker( strBody );
	KVPairGroupWalker::const_iterator itSdp = walker.begin( );
	for( ; itSdp != walker.end() ; itSdp++ )
	{
		std::string key = itSdp->key;
		assert( !key.empty() );		
		std::string::value_type k = key.at( 0 );
		switch ( k )
		{
		case 'a':
			{
				std::string value = itSdp->value;
				transform(value.begin(), value.end(), value.begin(), tolower);
				if( POSOK( value.find("x-playlist-item") ) )
				{
					parsePlaylistItem( itSdp->value );
				}
				else if( POSOK( value.find("x-motorola-ecm") ) )
				{
					parseEncryptionData( itSdp->value );
				}
				else
				{
					//靠！不认识，直接无视之
				}
			}
			break;

		default:
			break;
		}
	}

	// fixup the playlist items to calculate the stupid offset to end-of-Ads
	int lastIdxOfSection =-1;
	std::string lastExtAds;

	for (int i=0; i < mSetupParam.playlist.size(); i++)
	{
		if (lastExtAds == mSetupParam.playlist[i].extAds)
			continue; // same section

		// just left an ADS-segment
		if (lastIdxOfSection >=0 && std::string::npos != lastExtAds.find("ADStatus/"))
		{
			int j=0;

			// sum up the durToEOA
			int durToEOA =0;
			for (j=i-1; j >= lastIdxOfSection && durToEOA >=0; j--)
			{
				int durItem = mSetupParam.playlist[j].cueout - mSetupParam.playlist[j].cuein;
				if (durItem <=0)
				{
					durToEOA = -1;
					break;
				}

				durToEOA += durItem;
			}

			if (durToEOA <=0)
				MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(parseSDP, "skip downcounting hint due to playtime-unknown segment %s[%d-%d]"), lastExtAds.c_str(), lastIdxOfSection, i);
			else
			{
				// set each offsetToEOA
				for (j=lastIdxOfSection; j <i; j++)
				{
					char buf[60];
					snprintf(buf, sizeof(buf)-2, ";TotalADItemDuration=%d.%03d", durToEOA/1000, durToEOA%1000);
					mSetupParam.playlist[j].extAds += buf;
					durToEOA -= mSetupParam.playlist[j].cueout - mSetupParam.playlist[j].cuein;
				}
			}
		}

		lastIdxOfSection = i;
		lastExtAds = mSetupParam.playlist[i].extAds;
	}

	// the rear is an ADS-segment
	if (lastIdxOfSection >=0 && lastIdxOfSection <= mSetupParam.playlist.size() -1  && std::string::npos != lastExtAds.find("ADStatus/"))
	{
		int j=0;

		// sum up the durToEOA
		int durToEOA =0;
		for (j=mSetupParam.playlist.size()-1; j >= lastIdxOfSection && durToEOA >=0; j--)
		{
			int durItem = mSetupParam.playlist[j].cueout - mSetupParam.playlist[j].cuein;
			if (durItem <=0)
			{
				durToEOA = -1;
				break;
			}

			durToEOA += durItem;
		}

		if (durToEOA <=0)
			MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(parseSDP, "skip downcounting hint due to playtime-unknown segment %s[%d-%d]"), lastExtAds.c_str(), lastIdxOfSection, mSetupParam.playlist.size()-1);
		else
		{
			// set each offsetToEOA
			for (j=lastIdxOfSection; j <mSetupParam.playlist.size(); j++)
			{
				char buf[60];
				snprintf(buf, sizeof(buf)-2, ";TotalADItemDuration=%d.%03d", durToEOA/1000, durToEOA%1000);
				mSetupParam.playlist[j].extAds += buf;
				durToEOA -= mSetupParam.playlist[j].cueout - mSetupParam.playlist[j].cuein;
			}
		}
	}

}

bool NgodRequestSetup::parseRequest( )
{
	if(!NgodClientRequestI::parseRequest())
		return false;

	parseSessionGroup( );
	parseVolume();
	parseStartpoint();
	parseTransport();
	parseSDP( );

	//TODO: check if the parameters we got are correct
	//TODO: create a session ?
	mSetupParam.ondemandSessId	= ondemandId;
	mSetupParam.connectionId	= connectionId;	

	return true;
}


//////////////////////////////////////////////////////////////////////////
///NgodRequestPlay
NgodRequestPlay::NgodRequestPlay( NgodEnv& env, NgodSessionManager& manager, IClientRequest* request )
:NgodClientRequestI( env, manager, request )
{
	mbR2Request = false;
	mServerResponse = new ServerResponsePlay( env, *this );
}

NgodRequestPlay::~NgodRequestPlay()
{
}

const PlayParam& NgodRequestPlay::getPlayParam( ) const
{
	return mPlayParam;
}

bool NgodRequestPlay::process( )
{
	if(!parseRequest())
	{
		mServerResponse->setLastErr( errorcodeBadRequest, "failed to parse play request" );
		return false;
	}	
	return NgodClientRequestI::process();
}

void NgodRequestPlay::parseScale( )
{
	initLocalBuffer( );
	const char* pScale = mRequest->getHeader( HeaderScale, szLocalBuffer, &bufSize);
	if(!STRVALID(pScale))
	{
		return;
	}
	mPlayParam.scale	= (float)atof(pScale);
	mPlayParam.reqScale = pScale;
}

void NgodRequestPlay::parseRange( )
{
	initLocalBuffer( );
	const char* pRange = mRequest->getHeader( HeaderRange, szLocalBuffer, & bufSize );
	if( !STRVALID(pRange) )
		return;

	//check if there is 'npt=' thing
	std::vector<std::string> range;
	ZQ::common::stringHelper::SplitString( pRange, range, "=","= \t");
	if( range.size() < 2 /*&& stricmp(range[0].c_str(), "now") == 0 */ )
	{
		mPlayParam.reqRange = pRange;
		return;//take fromNow as true and of course nptStart 
	}

	mPlayParam.reqRange = range[1];
	//left part should be 'npt'
	//I can ignore it because this is not TianShan spec
	if( strstr(range[1].c_str(), "now") == range[1].c_str() )
	{
		mPlayParam.nptStart = 0;
		mPlayParam.bFromNow = true;
	}
	else
	{
		mPlayParam.nptStart = (int64)(atof( range[1].c_str() ) * 1000.0f);
		mPlayParam.bFromNow = false;
	}
}

bool NgodRequestPlay::parseRequest( )
{
	if(!NgodClientRequestI::parseRequest())
	{		
		return false;
	}

	parseRange();
	parseScale();

	return true;
}

//////////////////////////////////////////////////////////////////////////
///NgodRequestPause
NgodRequestPause::NgodRequestPause( NgodEnv& env, NgodSessionManager& manager, IClientRequest* request )
:NgodClientRequestI( env, manager, request )
{
	mbR2Request = false;
	mServerResponse = new ServerResponsePause( env, *this );
}

NgodRequestPause::~NgodRequestPause()
{
}

bool NgodRequestPause::process( )
{
	if( !parseRequest() )
	{
		mServerResponse->setLastErr( errorcodeBadRequest, "failed to parse pause request" );
		return false;
	}

	return NgodClientRequestI::process();
}

bool NgodRequestPause::parseRequest( )
{
	if(!NgodClientRequestI::parseRequest())
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
///NgodRequestTeardown
NgodRequestTeardown::NgodRequestTeardown( NgodEnv& env, NgodSessionManager& manager, IClientRequest* request )
:NgodClientRequestI( env, manager, request )
{
	mbR2Request = true;
	mServerResponse = new ServerResponseTeardown( env, *this );
}

NgodRequestTeardown::~NgodRequestTeardown( )
{
}

const TeardownParam& NgodRequestTeardown::getTeardownParam( ) const
{
	return mTeardownParam;
}

bool NgodRequestTeardown::process( )
{
	if( !parseRequest() )
	{
		mServerResponse->setLastErr( errorcodeBadRequest, "failed to parse teardown request" );
		return false;
	}
	return NgodClientRequestI::process();
}

void NgodRequestTeardown::parseReason( )
{
	initLocalBuffer();
	const char* pReason = mRequest->getHeader( HeaderXReason, szLocalBuffer, &bufSize );
	if(!STRVALID(pReason))
		return;
	mTeardownParam.reason = pReason;
}

void NgodRequestTeardown::parseAddress()
{
	IClientRequest::RemoteInfo localInfo;
	char localBuffer[256];
	localInfo.size = sizeof(localInfo);
	localInfo.ipaddr = localBuffer;
	localInfo.addrlen = sizeof(localBuffer);
	if( mRequest->getRemoteInfo(localInfo))
	{
		mTeardownParam.clientIp = localInfo.ipaddr;	
	}

	if( mRequest->getLocalInfo(localInfo))
	{
		mTeardownParam.serverIp = localInfo.ipaddr;
	}
}

bool NgodRequestTeardown::parseRequest( )
{
	if( !NgodClientRequestI::parseRequest())
		return false;

	parseReason();
	parseAddress();
	return true;
}

//////////////////////////////////////////////////////////////////////////
///NgodRequestGetParameter
NgodRequestGetParameter::NgodRequestGetParameter( NgodEnv& env, NgodSessionManager& manager, IClientRequest* request )
:NgodClientRequestI( env, manager, request )
{
	mbR2Request = false;
	mServerResponse = new ServerResponseGetParameter( env, *this, manager);
}

NgodRequestGetParameter::~NgodRequestGetParameter()
{
}

const GetInfoParam& NgodRequestGetParameter::getParam( ) const
{
	return mParam;
}

std::string convertGetInfoParamMask( int mask )
{
	std::ostringstream	oss;
	if( mask & GetInfoParam::MASK_POSITION )
	{
		oss << "POSITION ";
	}

	if( mask & GetInfoParam::MASK_STATE )
	{
		oss << "STATE ";
	}

	if( mask & GetInfoParam::MASK_SCALE )
	{
		oss << "SCALE ";
	}

	if( mask & GetInfoParam::MASK_SESSIONLIST )
	{
		oss << "SESSIONLIST ";
	}

	if( mask & GetInfoParam::MASK_TIMEOUT )
	{
		oss << "TIMEOUT ";
	}

	return oss.str();
}

bool NgodRequestGetParameter::process( )
{
	if( !parseRequest() )
	{
		mServerResponse->setLastErr( errorcodeBadRequest, "failed to parse get_parameter request" );
		return false;
	}	
	
	ServerResponseGetParameterPtr::dynamicCast(mServerResponse)->setGetParamInfo(mParam);
	
	if( ( mParam.mask & GetInfoParam::MASK_SCALE ) ||
		( mParam.mask & GetInfoParam::MASK_STATE ) ||
		( mParam.mask & GetInfoParam::MASK_POSITION ) )
	{
		return NgodClientRequestI::process();
	}

	return true;
}

void NgodRequestGetParameter::parseSessionGroup( )
{
	initLocalBuffer();
	const char* pSessionGroup =mRequest->getHeader( HeaderSessionGroup, szLocalBuffer, & bufSize);
	if( STRVALID(pSessionGroup) )
	{
		mParam.sessionGroupName = pSessionGroup;
	}
}

void NgodRequestGetParameter::parseMask( )
{
	std::string strBody;
	getContentBody( strBody );
	if(strBody.empty())
		return;

	std::vector<std::string> rets;
	ZQ::common::stringHelper::SplitString( strBody, rets, " \r\n"," \r\n");

	std::vector<std::string>::const_iterator it = rets.begin();
	for( ; it != rets.end() ; it++ )
	{
		STRSWITCH()
			STRCASE(it->c_str(), "scale")
				mParam.mask		|= GetInfoParam::MASK_SCALE;
			STRCASE(it->c_str(), "presentation_state" )
				mParam.mask		|= GetInfoParam::MASK_STATE;
			STRCASE(it->c_str(), "position")
				mParam.mask		|= GetInfoParam::MASK_POSITION;
			STRCASE(it->c_str(), "connection_timeout")
				mParam.mask		|= GetInfoParam::MASK_TIMEOUT;
			STRCASE(it->c_str(), "session_list")
				mParam.mask		|= GetInfoParam::MASK_SESSIONLIST;
		STREND()
	}
}

bool NgodRequestGetParameter::parseRequest( )
{
	if( !NgodClientRequestI::parseRequest() )
		return false;
	
	parseMask();
	parseSessionGroup();

	return true;
}

//////////////////////////////////////////////////////////////////////////
///NgodRequestSetParam
NgodRequestSetParam::NgodRequestSetParam( NgodEnv& env, NgodSessionManager& manager, IClientRequest* request )
:NgodClientRequestI( env, manager, request )
,mMask(MASK_NONE)
{
	mbR2Request = false;
	mServerResponse = new ServerResponseSetParameter( env, *this );
}

NgodRequestSetParam::~NgodRequestSetParam()
{
}

void NgodRequestSetParam::parseRequire()
{
	initLocalBuffer();
	const char* pRequire = mRequest->getHeader( HeaderRequire, szLocalBuffer, &bufSize );
	if( !STRVALID(pRequire) )
		return;

	if( strstr( pRequire, "R2") != NULL || strstr( pRequire,"r2") != NULL )
	{
		mMask = MASK_R2;
	}
	else if( strstr(pRequire, "C1") != NULL || strstr(pRequire, "c1") != NULL )
	{
		mMask = MASK_C1;
	}
}

void NgodRequestSetParam::parseSessionGroup( const std::string& str )
{
	std::vector<std::string> groups;
	ZQ::common::stringHelper::SplitString( str, groups, " :\r\n"," :\r\n");
	if( groups.size() >= 2)
	{
		ZQ::common::stringHelper::SplitString( groups[1], sessionGroupsForR2, " "," ");
	}
}

void NgodRequestSetParam::parseSessionlist( const std::string& str )
{
	std::vector<std::string> lists;
	ZQ::common::stringHelper::SplitString( str, lists, " :"," :","","",1);
	if( lists.size() >= 2 )
	{
		ZQ::common::stringHelper::SplitString( lists[1], sessionListForC1, " ", " ");
	}
}

void NgodRequestSetParam::parseBody( )
{
	if( mMask == MASK_NONE )
		return;

	std::string strBody;
	getContentBody(strBody);
	if( strBody.empty() ) return;

	switch ( mMask)
	{
	case MASK_C1:
		{
			parseSessionlist( strBody );
		}
		break;

	case MASK_R2:
		{
			parseSessionGroup( strBody );
		}
		break;

	default:
		break;
	}
}

bool NgodRequestSetParam::parseRequest( )
{
	if(!NgodClientRequestI::parseRequest())
	{
		mServerResponse->setLastErr( errorcodeBadRequest, "failed to parse set_parameter request" );
		return false;
	}	

	parseRequire();
	parseBody( );

	return true;
}
void NgodRequestSetParam::updateSessionGroupInfo( const std::vector<std::string>& groups )
{
	std::vector<std::string>::const_iterator it = groups.begin();
	for( ; it != groups.end() ; it ++ )
	{
		mSessManager.updateR2ConnectionId( *it, connectionId );//update session group with current connection id
	}
}

void NgodRequestSetParam::updateSessionListInfo( const std::vector<std::string>&  sessList )
{
	std::vector<std::string>::const_iterator it = sessList.begin();
	while( it != sessList.end() )
	{
		it++;//skip OndemandSessionId
		if( it == sessList.end() )
			break;
		NGOD::NgodSessionPrx sessPrx = mSessManager.openSession( *it );
		if( sessPrx )
		{
			try
			{
				sessPrx->updateC1ConnectionId( connectionId );
			}
			catch( const Ice::Exception& )
			{//ignore the error
			}
		}

		it++;//step to next OnDemandId:session Id
	}
}

bool NgodRequestSetParam::process( )
{
	ServerResponseSetParameterPtr response = new ServerResponseSetParameter( mEnv, *this );
	if( !parseRequest())
	{
		mServerResponse->setLastErr( errorcodeBadRequest, "failed to parse set_parameter request" );
		return false;
	}	

	switch ( mMask )
	{
	case MASK_R2:
		{
			updateSessionGroupInfo( sessionGroupsForR2 );			
		}
		break;

	case MASK_C1:
		{
			updateSessionListInfo( sessionListForC1 );
		}
		break;

	default:
		break;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
///NgodRequestAnnounceResponse
NgodRequestAnnounceResponse::NgodRequestAnnounceResponse( NgodEnv& env, NgodSessionManager& manager, IClientRequest* request )
:NgodClientRequestI(env,manager,request),
mRetCode(200)
{
	mbNeedVerCode	= false;
	mbR2Request		= false;
	mServerResponse = new ServerResponseSetParameter( env, *this );
}

NgodRequestAnnounceResponse::~NgodRequestAnnounceResponse()
{
}

int NgodRequestAnnounceResponse::getRetCode( )
{
	return mRetCode;
}
bool NgodRequestAnnounceResponse::process( )
{
	return parseRequest();
}

bool NgodRequestAnnounceResponse::parseRequest( )
{
	if(!NgodClientRequestI::parseRequest())
		return false;

	//get response code
	initLocalBuffer();
	const char* pCode = mRequest->getUri( szLocalBuffer, bufSize );
	if( !STRVALID(pCode ))
		return false;

	mRetCode = atoi( pCode );
	return NgodClientRequestI::process();
}

}//namespace NGOD

