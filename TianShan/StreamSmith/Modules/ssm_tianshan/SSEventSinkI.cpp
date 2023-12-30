#include "public.h"
#include <ZQ_common_conf.h>
#include "SSEventSinkI.h"
//#include <Log.h>
#include <TianShanDefines.h>
#include <TianShanIceHelper.h>

namespace ZQ
{
namespace StreamSmith
{
#ifdef WITH_ICESTORM
using namespace ZQ::common;
EventSinkEventChannel::EventSinkEventChannel(IPlaylistManager* pManager,ZQADAPTER_DECLTYPE adpater)
						:mobjAdapter(adpater)
{
	mpPlManager = pManager;
	mbQuit = false;
	mbNetworkOK = false;
	mStrTopicProxy = SERVICE_NAME_TopicManager":"DEFAULT_ENDPOINT_TopicManager;	
	mhBadConn = CreateEvent(NULL,FALSE,FALSE,NULL);
}
EventSinkEventChannel::~EventSinkEventChannel()
{
	mbQuit = true;
	SetEvent(mhBadConn);
	Sleep(1);
	waitHandle( 10000 );
	CloseHandle( mhBadConn );	
}
bool	EventDispatchEventChannelI(DWORD eventType,ZQ::common::Variant& params,void* pExtraData)
{
	EventSinkEventChannel* pThis=(EventSinkEventChannel*)pExtraData;
	return pThis->dispatchEvent(eventType,params);
}
TianShanIce::Streamer::StreamState ConvertVstrmStateToTianshanIceState(ULONG plstate)
{
	TianShanIce::Streamer::StreamState ret=TianShanIce:: Streamer::stsStop;
	switch(plstate)
	{
	case IPlaylist::PLAYLIST_SETUP:
		{
			ret=TianShanIce::Streamer::stsSetup;
		}
		break;
	case IPlaylist::PLAYLIST_PLAY:
		{
			ret=TianShanIce::Streamer::stsStreaming;
		}
		break;
	case IPlaylist::PLAYLIST_PAUSE:
		{
			ret=TianShanIce::Streamer::stsPause;
		}
		break;
	case IPlaylist::PLAYLIST_STOP:
		{
			ret=TianShanIce::Streamer::stsStop;
		}
		break;
	default:
		break;
	}
	return ret;		 
}

std::string EventSinkEventChannel::getPlaylistProxyString( const std::string& playlistId )
{
	Ice::Identity	idTemp;
	idTemp.category = PL_CATALOG;
	idTemp.name		= playlistId;

	std::string strProxy;
	try
	{
		Ice::ObjectPrx prx = mobjAdapter->createProxy( idTemp );
		strProxy = mobjAdapter->getCommunicator()->proxyToString( prx );
	}
	catch( const Ice::Exception& )
	{
		strProxy = "";
	}
	return strProxy;
}

void addExtraProperties( ZQ::common::Variant& var , TianShanIce::Properties& props )
{
	if( !var.has(EventField_ExtraProperties)) return;
	
	ZQ::common::Variant varExtra = (ZQ::common::Variant&)var[EventField_ExtraProperties];
	int count = varExtra.size();
	for( int i = 0 ; i < count ; i++ )
	{
		std::string key = varExtra.key(i);
		std::string value = varExtra[key];
		props.insert( TianShanIce::Properties::value_type( key , value ) );
	}
}

#define EVENTCSEQ				"EventSeq"
#define EVENTSTREAMNPTPOS		"streamNptPosition"
#define EVENTITEMNPTPOS			"itemNptPosition"
#define EVENTSTREAMSCALE		"scale"
#define EVENTPERREQUESTED       "perRequested"  

bool EventSinkEventChannel::sendSessionProgressEvent(ZQ::common::Variant& params)
{
	std::string	strGuid		= (std::string)params[EventField_PlaylistGuid];	
	std::string	strProxy	= getPlaylistProxyString(strGuid);

	ULONGLONG	done,total;
	int			step,totalstep;				
	params.getQuadword(params,EventField_runningByteOffset,done);
	params.getQuadword(params,EventField_TotalbyteOffset,total);
	step					= (int)params[EventField_currentStep];
	totalstep				= (int)params[EventField_totalStep];

	int	ctrlNumber			= (int)params[EventField_UserCtrlNum];
	std::string	strItemName	= (std::string)params[EventField_ItemFileName];
	
	char	szbuf[1024];
	sprintf(szbuf,"CtrlNum=%d&ItemName=%s",ctrlNumber,strItemName.c_str());
	
	long	lSeq			= (long)params[EventField_EventCSEQ];
	char szSeq[16];
	sprintf(szSeq,"%d",lSeq);

	TianShanIce::Properties exProps;
	addExtraProperties( params , exProps );

	try
	{
		Ice::Context ctx;					
		ctx[EVENTCSEQ] = szSeq ; 
		mProgressEventPrx->OnProgress(	strProxy,strGuid, (Ice::Int)done,(Ice::Int)total, (Ice::Int)step,(Ice::Int)totalstep, szbuf, exProps, ctx );
	}
	catch (...) 
	{
		NotifyBadConnection();
		return false;
	}
	return true;
}

bool EventSinkEventChannel::sendItemStepEvent( ZQ::common::Variant& params )
{
	std::string	strProxy;

	std::string	strGuid						= (std::string)params[EventField_PlaylistGuid];
	std::string strPrevFileName				= (std::string)params[EventField_ItemFileName];;
	std::string strnextFileName				= (std::string)params[EventField_ItemOtherFileName];;
	std::string	strStampUTC					= (std::string)params[EventField_StampUTC];
	std::string	strPrevProviderId			= (std::string)params[EventField_prevProviderId];
	std::string strPrevProviderAssetId		= (std::string)params[EventField_prevProviderAssetId];
	std::string strCurProviderId			= (std::string)params[EventField_currentPoviderId];
	std::string strCurProviderAssetId		= (std::string)params[EventField_currentProviderAssetId];
	std::string strPrevStreamingSource		= (std::string)params[EventField_PrevStreamingSource];
	std::string strCurStreamingSource		= (std::string)params[EventField_CurStreamingSource];
	std::string strItemExitReason			= (std::string)params[EventField_ItemExitReason];

	ULONG prevItemFlag                       = (ULONG)params[EventField_PrevFlag];
	ULONG currItemFlag                       = (ULONG)params[EventField_CurrFlag];

	int prevNptPos							= (int)params[EventField_PreviousePlayPos];
	int prevNptPosPrimary                   = (int)params[EventField_PreviousePlayPosPrimary];
	int currentNptPos						= (int)params[EventField_CurrentPlayPos];
	long prevPlayTime                       = (long)params[EventField_PrevPlayTime];
	
	int64 totalDuration                      = (int64)params[EventField_TotalDuration];
	int64 totalVideoDuration                 = (int64)params[EventField_TotalVideoDuration];

	long currentPlayTime                    = (long)params[EventField_CurrPlayTime];



	char szPrevNptPos[128];
	char szPrevNptPosPrimary[128];
	char szTotalDuration[128];
	char szTotalVideoDuration[128];
	char szCurrentNptPos[128];
	char szPrevPlayTime[128];
	char szCurrPlayTime[128];


	sprintf(szPrevNptPos,"%d",prevNptPos);
	sprintf(szPrevNptPosPrimary, "%d", prevNptPosPrimary);
	sprintf( szTotalDuration, "%d", totalDuration);
	sprintf( szTotalVideoDuration, "%d", totalVideoDuration);
	sprintf(szCurrentNptPos,"%d",currentNptPos);
	sprintf(szPrevPlayTime, "%d", prevPlayTime);
	sprintf(szCurrPlayTime, "%d", currentPlayTime);



	std::string strItemSkipErrorFileName	= (std::string)params[EventField_AnnounceLastItemName];
	std::string strItemSkipErrorDesc		= (std::string)params[EventField_AnnounceLastErrDesc];
	int			iItemSkipErrorCode			= (int)params[EventField_AnnounceLastErrorCode];
	char		strItemSkipErrorCode[32]	= {0};
	sprintf(strItemSkipErrorCode,"%d",iItemSkipErrorCode);

	int			ctrlPrevItem		= (int)params[EventField_UserCtrlNum];
	int			ctrlNextItem		= (int)params[EventField_NextUserCtrlNum];

	int			timeoffsetCurItem	= (int)params[EventField_CurrentItemTimeOffset];
	long		clusterId			= (long)params[EventField_clusterId];

	ZQ::common::Variant varSpeedCurrent=(ZQ::common::Variant&)params[EventField_CurrentSpeed];
	float	currentSpeed=(float)((int)varSpeedCurrent[EventField_SpeedNumer])/(float)((int)varSpeedCurrent[EventField_SpeedDenom]);
	char	szCurrentSpeed[128];
	sprintf( szCurrentSpeed , "%f", currentSpeed );

	TianShanIce::Streamer::StreamState		CurrentState;				
	CurrentState=	ConvertVstrmStateToTianshanIceState((long)params[EventField_CurrentState]);
	char szCurrentState[128];
	sprintf(szCurrentState,"%d",CurrentState);


	TianShanIce::Properties	pro;
	ZQTianShan::Util::updatePropertyData( pro ,"prevItemName" , strPrevFileName );
	ZQTianShan::Util::updatePropertyData( pro ,"currentItemName" , strnextFileName );
	ZQTianShan::Util::updatePropertyData( pro ,"stampUTC" , strStampUTC );
	ZQTianShan::Util::updatePropertyData( pro ,"prevProviderId" , strPrevProviderId );
	ZQTianShan::Util::updatePropertyData( pro ,"prevProviderAssetId" , strPrevProviderAssetId );
	ZQTianShan::Util::updatePropertyData( pro ,"currentProviderId" , strCurProviderId );
	ZQTianShan::Util::updatePropertyData( pro ,"currentProviderAssetId" , strCurProviderAssetId );
	ZQTianShan::Util::updatePropertyData( pro , EventField_ItemExitReason, strItemExitReason);
	ZQTianShan::Util::updatePropertyData( pro ,"ItemSkipErrorFileName" , strItemSkipErrorFileName );
	ZQTianShan::Util::updatePropertyData( pro ,"ItemSkipErrorDescription" , strItemSkipErrorDesc );
	ZQTianShan::Util::updatePropertyData( pro ,"errorDetail" , strItemSkipErrorDesc );
	ZQTianShan::Util::updatePropertyData( pro ,"ItemSkipErrorCode" , strItemSkipErrorCode );

	ZQTianShan::Util::updatePropertyData( pro ,"oldStreamNPT" , szPrevNptPos );		
	ZQTianShan::Util::updatePropertyData( pro, EventField_PreviousePlayPosPrimary, szPrevNptPosPrimary); 
	ZQTianShan::Util::updatePropertyData( pro, EventField_TotalDuration, szTotalDuration);
	ZQTianShan::Util::updatePropertyData( pro, EventField_TotalVideoDuration, szTotalVideoDuration);

	ZQTianShan::Util::updatePropertyData( pro, EventField_PrevPlayTime, szPrevPlayTime);  
	ZQTianShan::Util::updatePropertyData( pro, EventField_CurrPlayTime, szCurrPlayTime); 
	ZQTianShan::Util::updatePropertyData( pro, EventField_PrevFlag, prevItemFlag); 
	ZQTianShan::Util::updatePropertyData( pro, EventField_CurrFlag, currItemFlag); 


	ZQTianShan::Util::updatePropertyData( pro ,"newStreamNPT" , szCurrentNptPos );
	ZQTianShan::Util::updatePropertyData( pro , EVENTSTREAMNPTPOS , szCurrentNptPos );

	int oldItemNpt			= (int)params[EventField_PrevItemTimeOffset];
	int newItemNpt			= (int)params[EventField_CurrentItemTimeOffset];
	ZQTianShan::Util::updatePropertyData( pro ,"oldItemNPT" , oldItemNpt );
	ZQTianShan::Util::updatePropertyData( pro ,"newItemNPT" , newItemNpt );
	ZQTianShan::Util::updatePropertyData( pro ,EVENTITEMNPTPOS , newItemNpt );

	ZQTianShan::Util::updatePropertyData( pro ,"newState" , szCurrentState );
	ZQTianShan::Util::updatePropertyData( pro ,"scale" , szCurrentSpeed );	

	ZQTianShan::Util::updatePropertyData( pro , "prevStreamingSource" , strPrevStreamingSource );
	ZQTianShan::Util::updatePropertyData( pro , "curStreamingSource" , strCurStreamingSource );
	
	ZQTianShan::Util::updatePropertyData( pro , "clusterId" , clusterId );

	ZQTianShan::Util::updatePropertyData( pro , "currentItemTimeOffset" , timeoffsetCurItem );

	long	lSeq = (long)params[EventField_EventCSEQ];
	ZQTianShan::Util::updatePropertyData( pro , EVENTCSEQ , lSeq );
	
	strProxy			= getPlaylistProxyString( strGuid );
		

	addExtraProperties( params , pro );

	try
	{
		Ice::Context ctx;
		char szSeq[64];
		sprintf(szSeq , "%d" , lSeq );
		ctx[EVENTCSEQ] = szSeq ; 
		mPlaylistEventPrx->OnItemStepped( strProxy , strGuid , ctrlNextItem , ctrlPrevItem , pro , ctx );
	}
	catch (...) 
	{
		NotifyBadConnection();
		return false;
	}

	return true;
}

bool EventSinkEventChannel::sendEndOfStreamEvent( ZQ::common::Variant& params )
{
	std::string	strGuid;
	std::string	strProxy;
	strGuid								= (std::string)params[EventField_PlaylistGuid];
	std::string strSessionExitReason		= (std::string)params[EventField_ItemExitReason];

	strProxy							= getPlaylistProxyString( strGuid );
	
	long	lSeq						= (long)params[EventField_EventCSEQ];
	char szSeq[16];
	sprintf(szSeq,"%d",lSeq);

	long	lEndTimeOffset = (long)params[EventField_TotalTimeOffset];
	char szEndTimeOffset[16];
	sprintf(szEndTimeOffset,"%d", lEndTimeOffset);

	TianShanIce::Properties exProps;

	ZQ::common::Variant varSpeedCurrent	= (ZQ::common::Variant&)params[EventField_CurrentSpeed];
	float currentSpeed					= (float)((int)varSpeedCurrent[EventField_SpeedNumer])/(float)((int)varSpeedCurrent[EventField_SpeedDenom]);
	int currentStreamPos				= (int)params[EventField_CurrentPlayPos];
	ZQTianShan::Util::updatePropertyData( exProps , EVENTSTREAMNPTPOS, currentStreamPos );
	ZQTianShan::Util::updatePropertyData( exProps , EVENTSTREAMSCALE ,currentSpeed );

	int newItemNpt						= (int)params[EventField_CurrentItemTimeOffset];
	ZQTianShan::Util::updatePropertyData( exProps ,EVENTITEMNPTPOS , newItemNpt );

	int iCtrlNum						= (int)params[EventField_UserCtrlNum];
	ZQTianShan::Util::updatePropertyData( exProps , "ItemIndexOnEndOfStream" , iCtrlNum );

	ZQTianShan::Util::updatePropertyData( exProps , "TimeOffsetOnEndOfStream" , std::string(szEndTimeOffset) );

	addExtraProperties( params , exProps );
	try
	{
		Ice::Context ctx;					
		ctx[EVENTCSEQ] = szSeq ;
		ctx["EndTimeOffset"]		= szEndTimeOffset;
		ctx["SessionExitReason"]	= strSessionExitReason;		
		mStreamEventPrx->OnEndOfStream( strProxy , strGuid , exProps, ctx);
	}
	catch (...) 
	{
		NotifyBadConnection();
		return false;
	}
	return true;
}

bool EventSinkEventChannel::sendBeginOfStreamEvent( ZQ::common::Variant& params )
{
	std::string	strGuid;
	std::string	strProxy;
	strGuid								= (std::string)params[EventField_PlaylistGuid];
	std::string strSessionExitReason	= (std::string)params[EventField_ItemExitReason];
	strProxy							= getPlaylistProxyString( strGuid );
	long	lSeq						= (long)params[EventField_EventCSEQ];
	char szSeq[16];
	sprintf(szSeq,"%d",lSeq);

	TianShanIce::Properties exProps;

	ZQ::common::Variant varSpeedCurrent	= (ZQ::common::Variant&)params[EventField_CurrentSpeed];
	float	currentSpeed				= (float)((int)varSpeedCurrent[EventField_SpeedNumer])/(float)((int)varSpeedCurrent[EventField_SpeedDenom]);
	int currentStreamPos				= (int)params[EventField_CurrentPlayPos];
	ZQTianShan::Util::updatePropertyData( exProps , EVENTSTREAMNPTPOS, currentStreamPos );
	ZQTianShan::Util::updatePropertyData( exProps , EVENTSTREAMSCALE ,currentSpeed );

	int newItemNpt						= (int)params[EventField_CurrentItemTimeOffset];
	ZQTianShan::Util::updatePropertyData( exProps ,EVENTITEMNPTPOS , newItemNpt );

	addExtraProperties( params , exProps );
	try
	{
		Ice::Context ctx;					
		ctx[EVENTCSEQ] = szSeq ;
		ctx["SessionExitReason"]	= strSessionExitReason;
		mStreamEventPrx->OnBeginningOfStream( strProxy , strGuid , exProps , ctx );
	}
	catch (...) 
	{
		NotifyBadConnection();
		return false;
	}
	return true;
}

bool EventSinkEventChannel::sendScaleChangeEvent( ZQ::common::Variant& params )
{
	std::string	strGuid;
	std::string	strProxy;
	strGuid								= (std::string)params[EventField_PlaylistGuid];
	strProxy							= getPlaylistProxyString( strGuid );
	
	ZQ::common::Variant	varSpeedPrev	= (ZQ::common::Variant&)params[EventField_PrevSpeed];
	float	prevSpeed					= (float)((int)varSpeedPrev[EventField_SpeedNumer])/(float)((int)varSpeedPrev[EventField_SpeedDenom]);
	ZQ::common::Variant varSpeedCurrent	= (ZQ::common::Variant&)params[EventField_CurrentSpeed];
	float	currentSpeed				= (float)((int)varSpeedCurrent[EventField_SpeedNumer])/(float)((int)varSpeedCurrent[EventField_SpeedDenom]);

	std::string strContentName			= (std::string)params[EventField_ItemFileName];
	std::string preRequest              = (std::string)params[EventField_perRequested];
	int prevNptPos						= (int)params[EventField_PreviousePlayPos];
	int prevNptPosPrimary               = (int)params[EventField_PreviousePlayPosPrimary];
	int64 totalDuration                 = (int64)params[EventField_TotalDuration];
	int64 totalVideoDuration            = (int64)params[EventField_TotalVideoDuration];
	int iCtrlNum						= (int)params[EventField_UserCtrlNum];

	long	lSeq						= (long)params[EventField_EventCSEQ];
	char szSeq[16];
	char szPrevNptPos[128];
	char szPrevNptPosPrimary[128];
	char szTotalDuration[128];
	char szTotalVideoDuration[128];
	sprintf(szSeq,"%d",lSeq);
	sprintf(szPrevNptPos,"%d",prevNptPos);
	sprintf(szPrevNptPosPrimary, "%d", prevNptPosPrimary);
	sprintf(szTotalDuration, "%d", totalDuration);
	sprintf(szTotalVideoDuration, "%d", totalVideoDuration);

	TianShanIce::Properties exProps;

	int currentStreamPos				= (int)params[EventField_CurrentPlayPos];
	ZQTianShan::Util::updatePropertyData(exProps, EVENTPERREQUESTED, preRequest);
	ZQTianShan::Util::updatePropertyData( exProps , EVENTSTREAMNPTPOS, currentStreamPos );
	ZQTianShan::Util::updatePropertyData( exProps , EVENTSTREAMSCALE ,currentSpeed );

	int newItemNpt = (int)params[EventField_CurrentItemTimeOffset];
	ZQTianShan::Util::updatePropertyData( exProps ,"oldStreamNPT" , szPrevNptPos );		
	ZQTianShan::Util::updatePropertyData( exProps, EventField_PreviousePlayPosPrimary, szPrevNptPosPrimary); 
	ZQTianShan::Util::updatePropertyData( exProps, EventField_TotalDuration, szTotalDuration);
	ZQTianShan::Util::updatePropertyData( exProps, EventField_TotalVideoDuration, szTotalVideoDuration);
	ZQTianShan::Util::updatePropertyData( exProps ,EVENTITEMNPTPOS , newItemNpt );

	addExtraProperties( params , exProps );
	try
	{
		Ice::Context ctx;					
		ctx[EVENTCSEQ] = szSeq ;
		mStreamEventPrx->OnSpeedChanged(strProxy, strGuid, prevSpeed, currentSpeed,	exProps, ctx );
	}
	catch (...) 
	{
		NotifyBadConnection();
		return false;
	}
	return true;
}

bool EventSinkEventChannel::sendStateChangeEvent( ZQ::common::Variant& params )
{
	std::string	strGuid;
	std::string	strProxy;
	strGuid								= (std::string)params[EventField_PlaylistGuid];				
	strProxy							= getPlaylistProxyString( strGuid );
	
	TianShanIce::Streamer::StreamState		PrevState;
	TianShanIce::Streamer::StreamState		CurrentState;
	PrevState							= ConvertVstrmStateToTianshanIceState((long)params[EventField_PrevState]);
	CurrentState						= ConvertVstrmStateToTianshanIceState((long)params[EventField_CurrentState]);
	std::string strContentName			= (std::string)params[EventField_ItemFileName];
	int iCtrlNum						= (int)params[EventField_UserCtrlNum];
	int prevNptPos							= (int)params[EventField_PreviousePlayPos];
	int prevNptPosPrimary                   = (int)params[EventField_PreviousePlayPosPrimary];
	int64 totalDuration                      = (int64)params[EventField_TotalDuration];
	int64 totalVideoDuration                 = (int64)params[EventField_TotalVideoDuration];
	long	lSeq						= (long)params[EventField_EventCSEQ];
	char szSeq[16];
	char szPrevNptPos[128];
	char szPrevNptPosPrimary[128];
	char szTotalDuration[128];
	char szTotalVideoDuration[128];

	sprintf(szSeq,"%d",lSeq);
	sprintf(szPrevNptPos,"%d",prevNptPos);
	sprintf(szPrevNptPosPrimary, "%d", prevNptPosPrimary);
	sprintf( szTotalDuration, "%d", totalDuration);
	sprintf( szTotalVideoDuration, "%d", totalVideoDuration);

	TianShanIce::Properties exProps;

	ZQ::common::Variant varSpeedCurrent	= (ZQ::common::Variant&)params[EventField_CurrentSpeed];
	float	currentSpeed				= (float)((int)varSpeedCurrent[EventField_SpeedNumer])/(float)((int)varSpeedCurrent[EventField_SpeedDenom]);
	int currentStreamPos				= (int)params[EventField_CurrentPlayPos];
	std::string preRequest              = (std::string)params[EventField_perRequested];
	ZQTianShan::Util::updatePropertyData( exProps, EVENTPERREQUESTED, preRequest);
	ZQTianShan::Util::updatePropertyData( exProps , EVENTSTREAMNPTPOS, currentStreamPos );
	ZQTianShan::Util::updatePropertyData( exProps , EVENTSTREAMSCALE ,currentSpeed );
	int newItemNpt						= (int)params[EventField_CurrentItemTimeOffset];
	ZQTianShan::Util::updatePropertyData( exProps ,EVENTITEMNPTPOS , newItemNpt );
	ZQTianShan::Util::updatePropertyData( exProps ,"oldStreamNPT" , szPrevNptPos );		
	ZQTianShan::Util::updatePropertyData( exProps, EventField_PreviousePlayPosPrimary, szPrevNptPosPrimary); 
	ZQTianShan::Util::updatePropertyData( exProps, EventField_TotalDuration, szTotalDuration);
	ZQTianShan::Util::updatePropertyData( exProps, EventField_TotalVideoDuration, szTotalVideoDuration);

	addExtraProperties( params , exProps );
	try
	{
		Ice::Context ctx;					
		ctx[EVENTCSEQ] = szSeq ;
		mStreamEventPrx->OnStateChanged( strProxy , strGuid , PrevState , CurrentState, exProps, ctx );
	}
	catch (...) 
	{
		NotifyBadConnection();
		return false;
	}
	return true;
}

bool EventSinkEventChannel::sendPlaylistDestroyEvent( ZQ::common::Variant& params )
{
	std::string strProxy ;

	std::string strGuid				= (std::string)params[EventField_PlaylistGuid];				
	std::string	strReason			= (std::string)params[EventField_ExitReason];
	std::string strProviderId		= (std::string)params[EventField_prevProviderId];
	std::string strProviderAssetId	= (std::string)params[EventField_prevProviderAssetId];
	std::string strStreamingSoure	= (std::string)params[EventField_PrevStreamingSource];
	std::string strTimestamp		= (std::string)params[EventField_StampUTC];
	std::string preRequest              = (std::string)params[EventField_perRequested];

	long		clusterId			= (long)params[EventField_clusterId];
	bool		bExitOnPlaying		= (bool)params[EventField_playlistExitStatus];

	strProxy						= getPlaylistProxyString( strGuid );
	
	int exitCode					= (int)params[EventField_ExitCode];
	long	lSeq					= (long)params[EventField_EventCSEQ];
	char szSeq[16];
	sprintf(szSeq,"%d",lSeq);

	TianShanIce::Properties	pro;

	ZQTianShan::Util::updatePropertyData( pro , EVENTPERREQUESTED, preRequest);
	ZQTianShan::Util::updatePropertyData( pro , "providerId" , strProviderId );
	ZQTianShan::Util::updatePropertyData( pro , "providerAssetId" , strProviderAssetId );
	ZQTianShan::Util::updatePropertyData( pro , "streamingSource" , strStreamingSoure.empty() ? "0" : "1" );
	ZQTianShan::Util::updatePropertyData( pro , "exitWhilePlaying" , bExitOnPlaying ? "1" : "0" );
	ZQTianShan::Util::updatePropertyData( pro , "clusterId" , clusterId );
	ZQTianShan::Util::updatePropertyData( pro , "stampUTC" , strTimestamp );

	addExtraProperties( params , pro );
	try
	{
		Ice::Context ctx;					
		ctx[EVENTCSEQ] = szSeq ;
		mStreamEventPrx->OnExit(strProxy , strGuid , exitCode , strReason ,ctx );
		mStreamEventPrx->OnExit2(strProxy , strGuid , exitCode , strReason ,pro , ctx );
	}
	catch (...) 
	{
		NotifyBadConnection();
		return false;
	}
	return true;
}

bool EventSinkEventChannel::sendPauseTimeoutEvent( ZQ::common::Variant& params )
{
	std::string strGuid				= (std::string)params[EventField_PlaylistGuid];				
	std::string strTimestamp		= (std::string)params[EventField_StampUTC];
	std::string strProxy			= getPlaylistProxyString( strGuid );
	std::string strSourceNetId		= (std::string)params[EventField_SourceNetId];
	
	int curTimeOffset				= (int)params[EventField_CurrentTimeOffset];
	int curCtrlNum					= (int)params[EventField_UserCtrlNum];
	
	TianShanIce::Properties props;
	ZQTianShan::Util::updatePropertyData( props , "newStreamNPT" , curTimeOffset );	
	ZQTianShan::Util::updatePropertyData( props , "stampUTC" , strTimestamp );
	ZQTianShan::Util::updatePropertyData( props , "streamSessionId" , strGuid );

	addExtraProperties( params , props );
	try
	{
		mPauseTimeoutEventPrx->post( TianShanIce::Events::TopicStreamPauseTimeoutEvent ,
										0 ,
										TianShanIce::Events::TopicStreamPauseTimeoutEvent ,
										strTimestamp , 
										strSourceNetId , 
										props );
	}
	catch (...) 
	{
		NotifyBadConnection();
		return false;
	}
	return true;
}

bool EventSinkEventChannel::sendRepositionEvent( ZQ::common::Variant& params )
{
	std::string strGuid				= (std::string)params[EventField_PlaylistGuid];				
	std::string strTimestamp		= (std::string)params[EventField_StampUTC];
	std::string strProxy			= getPlaylistProxyString( strGuid );
	std::string strSourceNetId		= (std::string)params[EventField_SourceNetId];

	int curTimeOffset				= (int)params[EventField_CurrentItemTimeOffset];
	int prevTimeOffset				= (int)params[EventField_PrevItemTimeOffset];
	int curCtrlNum					= (int)params[EventField_UserCtrlNum];

	ZQ::common::Variant varSpeedCurrent=(ZQ::common::Variant&)params[EventField_CurrentSpeed];
	float	currentSpeed=(float)((int)varSpeedCurrent[EventField_SpeedNumer])/(float)((int)varSpeedCurrent[EventField_SpeedDenom]);
	char	szCurrentSpeed[128];
	sprintf( szCurrentSpeed , "%f", currentSpeed );

	TianShanIce::Properties props;
	ZQTianShan::Util::updatePropertyData( props , EVENTSTREAMSCALE , currentSpeed );
	ZQTianShan::Util::updatePropertyData( props , "newItemNPT" , curTimeOffset );
	ZQTianShan::Util::updatePropertyData( props , "oldItemNPT" , prevTimeOffset );
	ZQTianShan::Util::updatePropertyData( props , "newItemIndex" , curCtrlNum );
	ZQTianShan::Util::updatePropertyData( props , "stampUTC" , strTimestamp );
	ZQTianShan::Util::updatePropertyData( props , "streamSessionId" , strGuid );
	
	addExtraProperties( params , props );
	try
	{
		mRepostionEventPrx->post( TianShanIce::Events::TopicStreamRepositionEvent ,
									0 ,
									TianShanIce::Events::TopicStreamRepositionEvent ,
									strTimestamp , 
									strSourceNetId , 
									props );
	}
	catch (...) 
	{
		NotifyBadConnection();
		return false;
	}
	return true;
}

bool EventSinkEventChannel::dispatchEvent(DWORD eventType,ZQ::common::Variant& params)
{		
	if(mStrTopicProxy.empty())	{ return true; }
	if( !mbNetworkOK ) { return false; }

	try
	{
		switch(eventType)
		{
		case E_PLAYLIST_INPROGRESS:
			{
				return sendSessionProgressEvent( params );
			}
			break;
		case E_PLAYLIST_ITEMDONE://this is for item stepped
			{
				return sendItemStepEvent( params );
			}
			break;
		case E_PLAYLIST_END:
			{
				return sendEndOfStreamEvent( params );
			}
			break;
		case E_PLAYLIST_BEGIN:
			{
				return sendBeginOfStreamEvent( params );
			}
			break;
		case E_PLAYLIST_SPEEDCHANGED:
			{
				return sendScaleChangeEvent( params );
			}
			break;
		case E_PLAYLIST_STATECHANGED:
			{
				return sendStateChangeEvent( params );
			}
			break;
		case E_PLAYLIST_DESTROYED:
			{
				return sendPlaylistDestroyEvent( params );
			}
			break;
		case E_PLAYLIST_PAUSETIMEOUT:
			{
				return sendPauseTimeoutEvent( params );
			}
			break;
		case E_PLAYLIST_REPOSITION:
			{
				return sendRepositionEvent( params );
			}
			break;
		default:
			break;
		}
	}
	catch (...) 
	{
		return false;
	}
	return true;
}
void EventSinkEventChannel::NotifyBadConnection()
{
	SUPERLOG(Log::L_WARNING,SPLUGIN("event channel is down , connect it again"));
	mbNetworkOK=false;
	SetEvent(mhBadConn);
}
int EventSinkEventChannel::run()
{
	do 
	{
		WaitForSingleObject( mhBadConn , 20000 );
		if(!mbQuit)
		{
			if(!mbNetworkOK)
			{
				if (!mStrTopicProxy.empty()) 
				{
					SUPERLOG(Log::L_DEBUG,SPLUGIN("no connection to event channel service,connect it now"));
					connectToEventChannel();
				}
			}
			else
			{
				try
				{
					mTopicManagerPrx->ice_ping();
				}
				catch (...) 
				{
					mbNetworkOK=false;
					SetEvent(mhBadConn);
				}
			}
		}
	} while(!mbQuit);

	return 1;
}

Ice::ObjectPrx EventSinkEventChannel::getTopic( const std::string& topicName )
{
	if( !mTopicManagerPrx )
		return NULL;

	IceStorm::TopicPrx			iceTopic = NULL;	
	Ice::ObjectPrx				iceObj = NULL;
	
	try
	{
		iceTopic = mTopicManagerPrx->retrieve( topicName );
	}
	catch (const IceStorm::NoSuchTopic& ) 
	{	
		try
		{
			iceTopic = mTopicManagerPrx->create( topicName );
		}
		catch( const Ice::Exception&)
		{
			iceTopic = NULL;
		}
	}
	if( !iceTopic )
		return NULL;
	try
	{
		iceObj = iceTopic->getPublisher();
		if(!iceObj->ice_isDatagram())
		{//set it to one way schema
			iceObj->ice_oneway();
		}
	}
	catch( const Ice::Exception&)
	{
		iceObj = NULL;
	}
	return iceObj;
}

bool EventSinkEventChannel::connectToEventChannel()
{
	try
	{
		if(mStrTopicProxy.empty())
		{
			mbNetworkOK = true;
			//SUPERLOG(Log::L_DEBUG,SPLUGIN("no event channel endpoint,return without connecting to event channel service"));
			return true;
		}
		SUPERLOG(Log::L_INFO,SPLUGIN("connect to event channel service with %s"),mStrTopicProxy.c_str());
		Ice::CommunicatorPtr ic=mobjAdapter->getCommunicator();
		Ice::ObjectPrx base=ic->stringToProxy(mStrTopicProxy);
		if(!base)
		{
			//glog(Log::L_ERROR,"can't find topic manager");
			return false;
		}
		mTopicManagerPrx = IceStorm::TopicManagerPrx::checkedCast( base );
		if(!mTopicManagerPrx)
		{
			//glog(Log::L_ERROR,"Can't get topic manager");
			return false;
		}

		Ice::ObjectPrx topic = NULL;
		topic = getTopic( TianShanIce::Streamer::TopicOfPlaylist );
		if( !topic ) return false;
		mPlaylistEventPrx	= TianShanIce::Streamer::PlaylistEventSinkPrx::uncheckedCast( topic );
		
		topic = getTopic( TianShanIce::Streamer::TopicOfStream );
		if(!topic ) return false;
		mStreamEventPrx		= TianShanIce::Streamer::StreamEventSinkPrx::uncheckedCast( topic );

		topic = getTopic( TianShanIce::Streamer::TopicOfStreamProgress );
		if(!topic) return false;
		mProgressEventPrx	= TianShanIce::Streamer::StreamProgressSinkPrx::uncheckedCast( topic );

		topic = getTopic( TianShanIce::Events::TopicStreamRepositionEvent );
		if(!topic) return false;
		mRepostionEventPrx = TianShanIce::Events::GenericEventSinkPrx::uncheckedCast( topic );

		topic = getTopic( TianShanIce::Events::TopicStreamPauseTimeoutEvent );
		if(! topic ) return false;
		mPauseTimeoutEventPrx = TianShanIce::Events::GenericEventSinkPrx::uncheckedCast( topic );

		SUPERLOG(Log::L_DEBUG,SPLUGIN("connect to event channel service %s successfully!"),mStrTopicProxy.c_str());
	}
	catch ( const Ice::Exception& ex) 
	{
		SUPERLOG(Log::L_ERROR,SPLUGIN("Catch %s when connect to event channel %s"),ex.ice_name().c_str(),mStrTopicProxy.c_str());
		return false;
	}
	catch (...) 
	{
		SUPERLOG(Log::L_ERROR,SPLUGIN("Connect to event channel service %s failed"), mStrTopicProxy.c_str() );
		return false;
	}
	mbNetworkOK = true;
	return true;
}
void EventSinkEventChannel::registerEventSink()
{
	connectToEventChannel();

	mpPlManager->registerEventSink( E_PLAYLIST_INPROGRESS|E_PLAYLIST_ITEMDONE|
									E_PLAYLIST_END|E_PLAYLIST_BEGIN|
									E_PLAYLIST_STATECHANGED|E_PLAYLIST_SPEEDCHANGED|
									E_PLAYLIST_SESSEXPIRED|E_PLAYLIST_DESTROYED	| 
									E_PLAYLIST_PAUSETIMEOUT |E_PLAYLIST_REPOSITION,									
									EventDispatchEventChannelI,this);
	start();
}
#endif//WITH_ICESTORM

}}//namespace

