
#include <TianShanDefines.h>
#include <TianShanIceHelper.h>
#include <assert.h>
#include "VstrmSessionScaner.h"
#include "StreamSmithConfig.h"

#include <memoryDebug.h>

namespace ZQ
{
namespace StreamService
{
char* vstrmSessionStateText[]=
{
	"Closed",
	"Loaded",
	"ReadToPlay",
	"Playing",
	"AboutToFastReverse",
	"FastReverse",
	"AboutToFastForward",
	"FastForward",
	"AboutToPause",
	"Paused",
	"Resuming"
	"Inactive",
	"",
	"",
	""
};

#ifndef VOD_STATE_ALL_EXCEPT_INIT
	#define VOD_STATE_ALL_EXCEPT_INIT	VOD_STATE_ALL_EXCEPT_IDLE
#endif

VstrmSessionScaner::VstrmSessionScaner(SsEnvironment* environment , VstrmStreamerManager& manager )
:env(environment),
vstrmManager(manager)
{
	mbQuit = false;
}

VstrmSessionScaner::~VstrmSessionScaner()
{
}

void VstrmSessionScaner::attachServiceInstance( SsServiceImpl* s )
{
	assert( s != NULL );
	ss = s;
}

void VstrmSessionScaner::checkVstrmPort( )
{
	int32 iCount = vstrmManager.getTotalVstrmPortCount();
	if( iCount > 0 )
	{
		oldSessionInfo.reserve(iCount);
		newSessionInfo.reserve(iCount);
		expiredSessionInfo.reserve(iCount);
	}
}

VSTATUS VstrmSessionScaner::vstrmFOR_EACH_SESSION_CB( HANDLE vstrmClassHandle, 
													   PVOID cbParm,
													   ESESSION_CHARACTERISTICS* sessionChars, 
													   ULONG sizeofSessionChars, 
													   ULONG currentSessionInstance, 
													   ULONG totalSessions )
{
	VstrmSessionScaner* pThis = reinterpret_cast<VstrmSessionScaner*>(cbParm);
	assert(pThis);
	if( !sessionChars->SessionCharacteristics.Hidden &&
		!sessionChars->SessionCharacteristics.DeadSession &&
		IS_VIDEO_PORT( sessionChars->DestPortType ) )
	{
		pThis->newSessionInfo.push_back( *sessionChars );
	}
	return VSTRM_SUCCESS;
}
VSTATUS vtrsmCount_CB ( HANDLE , PVOID , ULONG )
{
	return VSTRM_SUCCESS;
}
void VstrmSessionScaner::scanSessions( )
{
	newSessionInfo.clear();
	if( VSTRM_SUCCESS != VstrmClassForEachSession( vstrmManager.getVstrmHandle() , vtrsmCount_CB, vstrmFOR_EACH_SESSION_CB, this ) )
	{
		SESSLOG(ZQ::common::Log::L_CRIT,
			CLOGFMT(VstrmSessionScaner,"scanSessions() failed to call VstrmClassForEachSession() with error [%s]"),
			vstrmManager.getVstrmError(VstrmGetLastError()).c_str()	);
	}
}
bool lessCompare ( ESESSION_CHARACTERISTICS& elem1, ESESSION_CHARACTERISTICS& elem2 )
{
	return static_cast<bool>( elem1.SessionId < elem2.SessionId );
}

void VstrmSessionScaner::stop( )
{
	mbQuit = true;
	waitHandle(10000);
}

int	VstrmSessionScaner::run( )
{
	SESSLOG(ZQ::common::Log::L_INFO,"start scanning");

	Ice::Long timeStamp		= 0;
	Ice::Long queryStart	= 0;
	Ice::Long queryStop		= 0;
	int		scanInterval	= gStreamSmithConfig.lVstrmSessionScanInterval;	
	scanInterval = scanInterval < 33 ? 33 : scanInterval;
	scanInterval = scanInterval > 2000 ? 2000 : scanInterval;
	ULONG		progressSendoutInterval = gStreamSmithConfig.lProgressEventSendoutInterval;
	if( progressSendoutInterval < 2000)
		progressSendoutInterval = 2000;
		
	checkVstrmPort();

	while( !mbQuit )
	{
		queryStart = ZQTianShan::now();
		scanSessions( );
		timeStamp = ZQTianShan::now();
		size_t newInfoCount = newSessionInfo.size( );
		size_t oldInfoCount = oldSessionInfo.size( );
		expiredSessionInfo.clear( );		
		
		std::sort( newSessionInfo.begin() , newSessionInfo.end() , lessCompare );
		if( newInfoCount == 0 && oldInfoCount > 0 )
		{			
			expiredSessionInfo = oldSessionInfo;			
		}
		else
		{

			SESSIONS::iterator itOld = oldSessionInfo.begin( ); 
			SESSIONS::iterator itNew = newSessionInfo.begin( );

			while( itOld != oldSessionInfo.end()  &&  itNew != newSessionInfo.end() )
			{			
				if( itOld->SessionId < itNew->SessionId )
				{				
					//expired session
					expiredSessionInfo.push_back( *itOld );
					itOld ++;
				}			
				else if( itOld->SessionId == itNew->SessionId )
				{
					if( itOld->SessionCharacteristics.Speed.denominator != itNew->SessionCharacteristics.Speed.denominator || 
						itOld->SessionCharacteristics.Speed.numerator != itNew->SessionCharacteristics.Speed.numerator )
					{//SpeedChange
						SESSLOG(ZQ::common::Log::L_DEBUG,"Port[%04d] Session[%u] SpeedChange from [%d/%d] to [%d/%d] ",
							itOld->DestPortHandle , itOld->SessionId ,
							itOld->SessionCharacteristics.Speed.numerator,
							itOld->SessionCharacteristics.Speed.denominator,
							itNew->SessionCharacteristics.Speed.numerator,
							itNew->SessionCharacteristics.Speed.denominator );
						notifySpeedChange( itOld->SessionId , 
											itOld->SessionCharacteristics.Speed , 
											itNew->SessionCharacteristics.Speed,
											timeStamp );
						notifyProgress( itNew->SessionId , itNew->SessionCharacteristics.PlayoutTimeOffset , itNew->SessionCharacteristics.EndTimeOffset );
					}
					if( itOld->SessionCharacteristics.State != itNew->SessionCharacteristics.State )
					{//StateChange
						SESSLOG(ZQ::common::Log::L_DEBUG,"Port[%04d] Session[%u] StateChange from [%s] to [%s] ",
							itOld->DestPortHandle , itOld->SessionId ,
							vstrmSessionStateText[itOld->SessionCharacteristics.State],
							vstrmSessionStateText[itNew->SessionCharacteristics.State]);
						notifyStateChange( itOld->SessionId , itNew->SessionCharacteristics.State , timeStamp );
					}
					if( (itOld->SessionCharacteristics.PlayoutTimeOffset + progressSendoutInterval ) < itNew->SessionCharacteristics.PlayoutTimeOffset )
					{
						notifyProgress( itNew->SessionId , itNew->SessionCharacteristics.PlayoutTimeOffset , itNew->SessionCharacteristics.EndTimeOffset );
					}
					else
					{//trick , record old timeOffset to new information so we can check it next time
						itNew->SessionCharacteristics.PlayoutTimeOffset = itOld->SessionCharacteristics.PlayoutTimeOffset;
					}
					
					itOld ++;
					itNew ++;
				}
				else 
				{//new session
					SESSLOG(ZQ::common::Log::L_DEBUG,("Port[%04d] new session [%u] found"),
						itNew->DestPortHandle,
						itNew->SessionId );
					
					notifyNewSession( itNew->SessionId , timeStamp );
										
					notifyProgress( itNew->SessionId , itNew->SessionCharacteristics.PlayoutTimeOffset , itNew->SessionCharacteristics.EndTimeOffset );

					itNew ++;
					
				}
			}
			while ( itNew != newSessionInfo.end() )
			{//new session
				SESSLOG(ZQ::common::Log::L_DEBUG,("Port[%04d] new session [%u] found"),
					itNew->DestPortHandle,
					itNew->SessionId );
				notifyNewSession( itNew->SessionId , timeStamp );
				notifyProgress( itNew->SessionId , itNew->SessionCharacteristics.PlayoutTimeOffset , itNew->SessionCharacteristics.EndTimeOffset );
				itNew ++;
			}
			while ( itOld != oldSessionInfo.end() )
			{
				expiredSessionInfo.push_back(*itOld);
				itOld++;
			}
		}
		SESSIONS::const_iterator itExpired = expiredSessionInfo.begin();
		while ( itExpired != expiredSessionInfo.end() )
		{//expired session
			SESSLOG(ZQ::common::Log::L_DEBUG,("Port[%04d] expired session %u found "), 
				itExpired->DestPortHandle ,
				itExpired->SessionId );
			notifyExpiredSession( itExpired->SessionId , timeStamp );
			itExpired ++;
		}

		oldSessionInfo = newSessionInfo;
		

		queryStop = ZQTianShan::now();
		if( ( queryStop - queryStart ) < scanInterval )
		{
			ZQ::common::delay( scanInterval - static_cast<int>(queryStop - queryStart) );
		}
	}
	SESSLOG(ZQ::common::Log::L_INFO,("leave scanning"));
	return 1;
}


void VstrmSessionScaner::notifyStateChange( ULONG sessionId , UCHAR newVstrmState , Ice::Long timeStamp )
{
	TianShanIce::Properties props;
	StreamParams paras;
	paras.mask = MASK_STATE;
	paras.streamState = convertVstrmStateToTianShanStreamState( newVstrmState );
	if( paras.streamState == TianShanIce::Streamer::stsStreaming || 
		paras.streamState == TianShanIce::Streamer::stsPause )
	{
		std::ostringstream oss;
		oss<<sessionId;
		ss->OnStreamEvent( SsServiceImpl::seStateChanged , oss.str() , paras  ,  props );
	}
}
void VstrmSessionScaner::notifySpeedChange(   ULONG sessionId , 
											   SPEED_IND oldSpeed , 
											   SPEED_IND newSpeed ,  
											   Ice::Long timeStamp   )
{
	TianShanIce::Properties props;
	
	StreamParams paras;
	paras.mask		= MASK_SCALE;
	if( newSpeed.denominator == 0 )
	{
		paras.scale = 0.0f;
	}
	else
	{
		paras.scale		=  (float)newSpeed.numerator / (float)newSpeed.denominator;
	}
	
	std::ostringstream oss;
	oss<<sessionId;
	
	ss->OnStreamEvent( SsServiceImpl::seScaleChanged , oss.str() , paras  ,  props );

}

void VstrmSessionScaner::notifyNewSession( ULONG sessionId  ,  Ice::Long timeStamp )
{
	TianShanIce::Properties props;
	
	StreamParams paras;	
	
	std::ostringstream oss;
	oss<<sessionId;
	
	ss->OnStreamEvent( SsServiceImpl::seNew , oss.str() , paras  ,  props );
}

void VstrmSessionScaner::notifyExpiredSession(  ULONG sessionId ,  Ice::Long timeStamp  )
{
	TianShanIce::Properties props;
	
	StreamParams paras;	
	
	std::ostringstream oss;
	oss<<sessionId;
	
	ss->OnStreamEvent( SsServiceImpl::seGone , oss.str() , paras  ,  props );	
}

void VstrmSessionScaner::notifyProgress( ULONG sessionId , ULONG curOffset, ULONG totalDuration )
{
	TianShanIce::Properties props;
	
	StreamParams paras;	
	paras.mask				= MASK_TIMEOFFSET | MASK_CONTENT_DURATION;
	paras.timeoffset		= curOffset;
	paras.duration			= totalDuration;	

	std::ostringstream oss;
	oss<<sessionId;

	ss->OnStreamEvent( SsServiceImpl::seProgress , oss.str() , paras  ,  props );	

}


TianShanIce::Streamer::StreamState VstrmSessionScaner::convertVstrmStateToTianShanStreamState( UCHAR state )
{
	switch (  state )
	{
	case PLAYING:
		return TianShanIce::Streamer::stsStreaming;
		break;
	case PAUSED:
		return TianShanIce::Streamer::stsPause;
		break;
	default:
		//as invalid
		return TianShanIce::Streamer::stsStop;
		break;
	}
}

}}
