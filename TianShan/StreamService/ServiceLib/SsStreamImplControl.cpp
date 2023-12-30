#include <stdlib.h>
#include <math.h>
#include <TianShanDefines.h>
#include <TianShanIceHelper.h>
#include "SsStreamImpl.h"
#include "SsServiceImpl.h"

#ifdef ZQ_OS_MSWIN
#include "memoryDebug.h"
#endif

namespace ZQ
{
namespace StreamService
{

#if defined ZQ_OS_MSWIN
	#define	PLFMT(x,y) 	CLOGFMT(x, "SsPlaylist[%s]\t"##y),ident.name.c_str()
#else
	#define	PLFMT(x,y) 	CLOGFMT(x, "SsPlaylist[%s] "y),ident.name.c_str()
#endif	

inline bool isFFScale( float scale )
{
	return scale > 1.0001f;
}

inline bool isREWScale( float scale )
{
	return scale < -0.0001f;
}

inline bool IsItemPlaytimesCountZero(const SsStreamImpl::constIter& iter) 
{
	return (iter->itemPlayTimesCount == 0);
}

void SsStreamImpl::updateLocalRecord(  const StreamParams& paras , const TianShanIce::Streamer::StreamState& newState )
{
	//update local record 
	// only scale and state are updated
	if ( paras.mask & MASK_SCALE )
	{
		//update scale
		if( fabs( paras.scale - speed ) > 0.01f && env->getConfig().iEnableSGScaleChangeEvent >= 1 )
		{
			fireScaleChangedMsg( speed , paras.scale ,TianShanIce::Properties());			
		}
		ENVLOG(ZQ::common::Log::L_INFO,	PLFMT(updateLocalRecord,"update scale from[%f] to [%f]"), speed , paras.scale );
		speed		=	paras.scale;
	}
	

	if( newState != playlistState )
	{	
		if ( env->getConfig().iEnableSGStateChangeEvent >= 1 )
		{
			fireStateChangedMsg( playlistState , newState , TianShanIce::Properties() );
		}
		ENVLOG(ZQ::common::Log::L_INFO,	PLFMT(updateLocalRecord,"update state from [%s] to [%s]"),
			ZQTianShan::Util::dumpTianShanStreamState(playlistState),
			ZQTianShan::Util::dumpTianShanStreamState( newState ));		
		playlistState	= newState;
	}
	
}

bool SsStreamImpl::constructStreamInfo( const StreamParams& para ,
										const int32& expectMask , 
										iter itEffect,
										TianShanIce::Streamer::StreamInfo& info )
{	
	if( expectMask & VALUE_WANTED_ITEM_NPT )
	{//want item time offset
		if( para.mask & MASK_TIMEOFFSET )
		{
			switch( env->getConfig().iSupportPlaylist )
			{
			case LIB_SUPPORT_NORMAL_PLAYLIST:
				{
					Ice::Long timeoffset = para.timeoffset;
					convertResultToItemWide( itEffect,  timeoffset );
					mInstanceStateAfterAction.timeOffset = timeoffset;
					ZQTianShan::Util::updateItemTimeOffsetToStreamInfo( info, timeoffset );
				}
				break;
			case LIB_SUPPORT_AUTO_PLAYLIST:
			case LIB_SUPPORT_NORMAL_STREAM:
				{					
					Ice::Long tmpTimeOffset = para.timeoffset;
					convertResultToItemWide( tmpTimeOffset );
					mInstanceStateAfterAction.timeOffset = tmpTimeOffset;
					ZQTianShan::Util::updateItemTimeOffsetToStreamInfo( info, tmpTimeOffset );
				}
				break;
			default:
				{
					assert(false);
				}
				break;
			}		
		}
		else
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0, PLFMT(constructStreamInfo,"TIMEOFFSET is required but not returned from porting layer"));
		}
	}

	if( expectMask & VALUE_WANTED_ITEM_DURATION )
	{
		if( para.mask & MASK_CONTENT_DURATION )
		{
			switch( env->getConfig().iSupportPlaylist )
			{
			case LIB_SUPPORT_NORMAL_PLAYLIST:
				{
					ZQTianShan::Util::updateItemTotalDurationToStreamInfo( info , para.duration );
				}
				break;
			case LIB_SUPPORT_AUTO_PLAYLIST:
			case LIB_SUPPORT_NORMAL_STREAM:
				{
//					Ice::Short from = SEEK_FROM_BEGIN;
					Ice::Long tmpOffset = para.duration;
					convertResultToItemWide( tmpOffset );
					ZQTianShan::Util::updateItemTotalDurationToStreamInfo( info , tmpOffset );
				}
				break;
			default:
				{
					assert(false);
				}
				break;
			}
		}
		else
		{
			if( getItemAttribute( itEffect ) )
			{
				ZQTianShan::Util::updateItemTotalDurationToStreamInfo(info, itEffect->itemTotalDuration);
			}
			else
			{
				ZQTianShan::Util::updateStreamTotalDurationToStreamInfo( info , 0 );
			}
		}
	}
	
	if( expectMask & VALUE_WANTED_STREAM_NPT )
	{
		if( para.mask & MASK_TIMEOFFSET )
		{
			switch( env->getConfig().iSupportPlaylist )
			{
			case LIB_SUPPORT_NORMAL_PLAYLIST:
				{
					Ice::Long tmpOffset = para.timeoffset;
					convertResultToStreamWide( itEffect ,tmpOffset );
					mInstanceStateAfterAction.timeOffset = tmpOffset;
					ZQTianShan::Util::updateStreamTimeOffsetToStreamInfo( info ,tmpOffset );
				}
				break;
			case LIB_SUPPORT_AUTO_PLAYLIST:
			case LIB_SUPPORT_NORMAL_STREAM:
				{
					mInstanceStateAfterAction.timeOffset = para.timeoffset;
					ZQTianShan::Util::updateStreamTimeOffsetToStreamInfo( info , para.timeoffset );
				}
				break;
			default:
				{
					assert(false);
				}
				break;
			}
		}
		else
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
				PLFMT(constructStreamInfo,"TIMEOFFSET is required but not returned from porting layer"));
		}	
	}

	if( expectMask & VALUE_WANTED_STREAM_DURATION )
	{
		if( para.mask & MASK_CONTENT_DURATION )
		{
			switch( env->getConfig().iSupportPlaylist )
			{
			case LIB_SUPPORT_NORMAL_PLAYLIST:
				{
					bool bOK = true;
					Ice::Long lTotal = 0 ;
					iter it = iterFirstItem();
					for ( ; it != itEffect ; it ++ )
					{
						if (!getItemAttribute(it))
						{
							bOK = false;
							break;
						}
						lTotal += it->itemTotalDuration;
					}
					lTotal += para.duration;
					for( ; it != iterEndItem() ; it ++ )
					{
						if (!getItemAttribute(it))
						{
							bOK = false;
							break;
						}
						lTotal += it->itemTotalDuration;
					}
					if(bOK)
						ZQTianShan::Util::updateStreamTotalDurationToStreamInfo( info , lTotal );
					else
						ZQTianShan::Util::updateStreamTotalDurationToStreamInfo( info , 0 );

				}
				break;
			case LIB_SUPPORT_AUTO_PLAYLIST:
			case LIB_SUPPORT_NORMAL_STREAM:
				{
					ZQTianShan::Util::updateStreamTotalDurationToStreamInfo( info , para.duration );
				}
				break;
			default:
				{
					assert( false );
				}
				break;
			}
		}
		else
		{
			bool bOK = true;
			Ice::Long lTotal = 0 ;
			iter it = iterFirstItem();
			for ( ; it != iterEndItem() ; it ++ )
			{
				if (!getItemAttribute(it))
				{
					bOK = false;
					break;
				}
				lTotal += it->itemRealDuration;
			}
			if ( bOK )
			{
				ZQTianShan::Util::updateStreamTotalDurationToStreamInfo( info , lTotal );
			}
			else
			{
				ZQTianShan::Util::updateStreamTotalDurationToStreamInfo( info , 0 );
			}
		}		
		
	}

	if(  expectMask & VALUE_WANTED_STREAM_SPEED )
	{
		if( para.mask & MASK_SCALE )
		{		
			ZQTianShan::Util::updateSpeedToStreamInfo( info , para.scale );
		}
		else
		{//using current recorded scale
			ENVLOG(ZQ::common::Log::L_WARNING, PLFMT(constructStreamInfo,"scale required but not returned from porting layer, use recorded [%f] scale instead"),
				speed );
			ZQTianShan::Util::updateSpeedToStreamInfo( info , speed );
		}
	}
	
	if( expectMask & VALUE_WANTED_STREAM_STATE )
	{		
		info.state	=	playlistState;
	}

	if( expectMask & VALUE_WANTED_USER_CTRLNUM )
	{
		if( iterValid( itEffect ) )
		{
			ZQTianShan::Util::updateUserCtrlNumToStreamInfo( info , itEffect->userCtrlNum );
		}
		else
		{
			ZQTianShan::Util::updateUserCtrlNumToStreamInfo( info , TianShanIce::Streamer::InvalidCtrlNum );
		}
	}

	return true;
}


bool SsStreamImpl::canChangeScaleInNormalPlaylistMode( const Ice::Float& newSpeed )
{
	if( playlistState == TianShanIce::Streamer::stsPause || playlistState == TianShanIce::Streamer::stsStreaming )
	{
		if( playlistState == TianShanIce::Streamer::stsPause )
			return true;

		if( !iterValid(iterCurrentItem()) )
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
				PLFMT(canChangeScaleInNormalPlaylistMode,"current item is invalid"));
		}
		iter it				= iterCurrentItem( ) ;
		Ice::Long lPassed	= 0;
		Ice::Long lLeft		= getSessTimeLeft( &lPassed );
		

		if( bEnableEOT && //enable EOT
			(getEOTSize() > 0)  && //EOTSize > 0 
			(iterCurrentItem() == iterLastItem()) && //
			(newSpeed > 1.0001f) ) //
		{
			Ice::Long predictLeft = static_cast<Ice::Long>(lLeft * speed / newSpeed);
			if( predictLeft < getEOTSize() )
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG ,"StreamService",0, PLFMT(canChangeScaleInNormalPlaylistMode,
					"item[%s] userCtrlNum[%d] is last item , EOTSize[%d] current distance [%ld][%ld],"
					" new Speed[%f] , reject change scale request"),
					it->setupInfo.contentName.c_str() ,
					it->userCtrlNum,
					getEOTSize(),
					lLeft,
					predictLeft,
					newSpeed);
				return false;
			}
		}

		if ( env->getConfig().iPreloadTimeInMS <= 0 )
		{
			ENVLOG(ZQ::common::Log::L_INFO,PLFMT(canChangeScaleInNormalPlaylistMode,"preLoadTime [%d] , allow to change scale"),
				env->getConfig().iPreloadTimeInMS);
			return true;
		}	

		Ice::Long lTo = lLeft;
		if ( isReverseStreaming() )
		{//
			if( ( (iterCurrentItem() == iterFirstItem()) && (newSpeed < -0.001f ) )	|| 
				( ( iterCurrentItem() == iterLastItem() ) &&( newSpeed > 0.001f ) ) )
			{//first item and new speed < 0
				//can change speed
				lTo = env->getConfig().iPreloadTimeInMS + 1;
			}
			else
			{
				if( newSpeed < -0.001f )
				{//reverse streaming
					lTo = lLeft;					
				}
				else if( newSpeed > 0.001f)
				{
					lTo	= lPassed;					
				}
				else
				{//
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0, PLFMT(canChangeScaleInNormalPlaylistMode,"can't change speed"));
				}
			}
		}
		else
		{
			if( ( (iterCurrentItem() == iterFirstItem()) && (newSpeed < -0.001f ) ) || 
				( ( iterCurrentItem() == iterLastItem() ) && ( newSpeed > 0.001f ) ) )
			{
				//can change speed
				lTo = env->getConfig().iPreloadTimeInMS + 1;
			}
			else
			{
				if( newSpeed < -0.001f )
				{
					lTo = lPassed;
				}
				else if( newSpeed > 0.001f)
				{
					lTo = lLeft;
				}
			}
		}
		if( lTo < env->getConfig().iPreloadTimeInMS )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,LOGCATAGORY,0,
				PLFMT(canChangeScaleInNormalPlaylistMode,"can't change scale because current distance is [%ld]"),
				lTo	);
		}
		return true;
	}
	else
	{
		return true;
	}
	return false;
}

bool SsStreamImpl::canChangeScale( const Ice::Float& newSpeed )
{
	if( fabs( newSpeed - 0.001 ) < 0.001 )
	{
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(canChangeScale,"new Scale [%f] , just accept it as No Change Scale"),
			newSpeed);
		return true;
	}

	switch ( env->getConfig().iSupportPlaylist )
	{
	case LIB_SUPPORT_NORMAL_PLAYLIST:
		{
			return canChangeScaleInNormalPlaylistMode( newSpeed );
		}
		break;
	case LIB_SUPPORT_AUTO_PLAYLIST:
		{
			return true;
		}
		break;
	case LIB_SUPPORT_NORMAL_STREAM:
		{
			return true;
		}
		break;
	default:
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
				PLFMT(canChangeScale,"config error , not supported mode[%d]"),
				env->getConfig().iSupportPlaylist );
		}
		break;

	}
	return false;
}

void SsStreamImpl::throwException( int32 errCode , const std::string& logPrefix)
{
	switch( errCode )
	{
	case ERR_RETURN_SERVER_ERROR:
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,
				LOGCATAGORY,0,PLFMT(throwException,"[%s] %s"),logPrefix.c_str(),"ERR_RETURN_SERVER_ERROR");
		}
		break;
	case ERR_RETURN_INVALID_PARAMATER:
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,
				LOGCATAGORY,0,PLFMT(throwException,"[%s] %s"),logPrefix.c_str(),"ERR_RETURN_INVALID_PARAMATER");
		}
		break;
	case ERR_RETURN_INVALID_STATE_OF_ART:
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,
				LOGCATAGORY,0,PLFMT(throwException,"[%s] %s"),logPrefix.c_str(),"ERR_RETURN_INVALID_STATE_OF_ART");
		}
		break;
	case ERR_RETURN_NOT_SUPPORT:
		{
			ZQTianShan::_IceThrow<TianShanIce::NotSupported>(ENVLOG,
				LOGCATAGORY,0,PLFMT(throwException,"[%s] %s"),logPrefix.c_str(),"ERR_RETURN_NOT_SUPPORT");
		}
		break;
	case ERR_RETURN_NOT_IMPLEMENT:
		{
			ZQTianShan::_IceThrow<TianShanIce::NotImplemented>(ENVLOG,
				LOGCATAGORY,0,PLFMT(throwException,"[%s] %s"),logPrefix.c_str(),"ERR_RETURN_NOT_IMPLEMENT");
		}
		break;
	case ERR_RETURN_OBJECT_NOT_FOUND:
		{//FIXME: which exception should be thrown out ?
			ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(throwException,"got ERR_RETURN_OBJECT_NOT_FOUND from portal, clean up current session"));
			DestroyPLRequest* pRequest = new DestroyPLRequest(env->getMainThreadPool() , env , ident );
			assert( pRequest );
			pRequest->start();
			throw new Ice::ObjectNotExistException("Object not exist", ERR_RETURN_OBJECT_NOT_FOUND);
		}
		break;
	case ERR_RETURN_ASSET_NOTFOUND:
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,
				LOGCATAGORY,TianShanIce::errcodeAssetNotfound,PLFMT(throwException,"[%s] %s"),logPrefix.c_str(),"ERR_RETURN_ASSET_NOTFOUND");
		}
		break;
	default:
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,
				LOGCATAGORY,0,PLFMT(throwException,"[%s] caught unknown exception"),
				logPrefix.c_str());
		}
	}
}

void SsStreamImpl::destroy(const ::Ice::Current& c) 
{
	::TianShanIce::Properties feedback; // simply throw away feedback returned from destory2()
	stampStateChangeReq = ZQ::common::TimeUtil::now();
	 destroy2(feedback, c);
	ENVLOG(ZQ::common::Log::L_INFO, PLFMT(destroy,"destroy leave stampStateChangeReq[%lu]"),stampStateChangeReq);
}

void SsStreamImpl::destroy2(::TianShanIce::Properties& feedback, const ::Ice::Current& c)
{
	std::string caller;
	std::string destroyReason;
	ZQTianShan::Util::getPropertyDataWithDefault(c.ctx, "caller", "", caller);
	ZQTianShan::Util::getPropertyDataWithDefault(c.ctx, "reason", "", destroyReason);
	
	Lock sync(*this);
	if( !bAlive )
	{
		return;
	}
	updateContextProperty( "user.destroy.caller", caller);
	updateContextProperty( "user.destroy.reason", destroyReason);

	ENVLOG(ZQ::common::Log::L_INFO, PLFMT(destroy,"enter destroy, called by [%s], reason [%s]"),
		caller.c_str(), destroyReason.c_str() );	

	try
	{
		SsServiceImpl::releaseStreamResource( serviceImpl , *this , streamerReplicaId, feedback);

		switch (env->getConfig().iSupportPlaylist)
		{
		case LIB_SUPPORT_NORMAL_PLAYLIST:
			{
				//destroy next item if available
				if( isItemStreaming(iterNextItem()) )
				{
					ENVLOG(ZQ::common::Log::L_INFO, PLFMT(destroy,"session[%s] belongs to [%s][%d] is running , destroy it"),
						iterNextItem()->sessionId.c_str() ,
						iterNextItem()->setupInfo.contentName.c_str(),
						iterNextItem()->userCtrlNum);
					doDestroySession( iterNextItem());
				}

				if( isItemStreaming(iterCurrentItem()))
				{
					ENVLOG(ZQ::common::Log::L_INFO,	PLFMT(destroy,"session[%s] belongs to [%s][%d] is running , destroy it"),
						iterCurrentItem()->sessionId.c_str() ,
						iterCurrentItem()->setupInfo.contentName.c_str(),
						iterCurrentItem()->userCtrlNum);
					doDestroySession( iterCurrentItem() );
				}
			}
			break;

		default:
			{
				std::string streamSessId = getStreamSessId(  iterEndItem() );
				if( !streamSessId.empty() )
				{
					ENVLOG(ZQ::common::Log::L_INFO,PLFMT(destroy,"destroy running session[%s]"),
						streamSessId.c_str());
					doDestroySession( streamSessId.c_str( ) );
				}
			}
			break;
		}

		//clear all running timer request
		env->getSceduler().cancelSchedule( ident.name , TianShanIce::Streamer::TIMERRENEWTICKET );		
		env->getSceduler().cancelSchedule( ident.name , TianShanIce::Streamer::TIMERPLAYLISTDESTROY );
		env->getSceduler().cancelSchedule( ident.name , TianShanIce::Streamer::TIMERDPC );

		firePlaylistDestroyMsg( iterCurrentItem() , 0 , "" , TianShanIce::Properties() );		

		bAlive = false;

		//remove item record from db
		serviceImpl.removeServantFromDB( ident , property );

		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(destroy,"leave destroy"));

	}
	catch( const TianShanIce::BaseException& ex )
	{
		playlistState = TianShanIce::Streamer::stsStop;
		setTimer( env->getConfig().iPlaylistTimeout);
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(destroy,"caught exception while destroying playlist:%s"),ex.message.c_str() );
		ex.ice_throw();
	}
	catch( const Ice::Exception& ex )
	{
		playlistState = TianShanIce::Streamer::stsStop;
		setTimer( env->getConfig().iPlaylistTimeout);
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(destroy,"caught ice exception:%s during destroying playlist"),ex.ice_name().c_str() );
		ex.ice_throw();
	}
}

bool SsStreamImpl::play(const ::Ice::Current& c)
{
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(play,"enter play"));
	
	TianShanIce::StrValues expectProps;
	expectProps.clear();
	if( playlistState != TianShanIce::Streamer::stsStreaming ||
		playlistState != TianShanIce::Streamer::stsPause )
	{
		if( iterValid( iterCurrentItem() ) )
		{
//			Ice::Long criticalStartTime = iterCurrentItem()->setupInfo.criticalStart;
//			time_t* t = NULL;
#pragma message(__MSGLOC__"TODO: How to check the critical start time ?")
		}
	}
	
	playEx( 0.0f , 0, 0, expectProps , c);

	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(play,"leave play"));
	return true;
}

bool SsStreamImpl::setSpeed(::Ice::Float newSpeed, const ::Ice::Current& c )
{
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(setSpeed,"enter with speed[%f]"),newSpeed );
	TianShanIce::StrValues expectProps;
	expectProps.clear();
	(void)playEx(newSpeed,0,0, expectProps,c);
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(setSpeed,"leave setspeed"));
	return true;
}

bool SsStreamImpl::pause(const ::Ice::Current& c )
{
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(pause,"enter pause"));
	if (!checkRestriction(iterCurrentItem(), TianShanIce::Streamer::PLISFlagNoPause))
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,PLFMT(pause,"reject pause command with restriction[%s]"),ZQTianShan::Util::convertItemFlagToStr(iterCurrentItem()->setupInfo.flags).c_str());
	}
	stampStateChangeReq = ZQ::common::TimeUtil::now();
	TianShanIce::StrValues	expectProps;
	expectProps.clear();
	(void)pauseEx(expectProps,c);
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(pause,"leave pause stampStateChangeReq[%lu]"),stampStateChangeReq);
	return true;
}

bool SsStreamImpl::resume(const ::Ice::Current& c )
{
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(resume,"enter resume"));
	TianShanIce::StrValues	expectProps;
	(void)playEx(0.0f,0,0,expectProps,c);
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(resume,"leave resume"));
	return true;
}
bool SsStreamImpl::skipToItem(::Ice::Int userCtrlNum, bool bPlay, const ::Ice::Current& c )
{
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(skipToItem,"enter skipToItem with userCtrlNum [%d] "), userCtrlNum );
	TianShanIce::StrValues expectProps;
	
	(void)playItem(userCtrlNum , 0, 1, 0.0f , expectProps , c );

	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(skipToItem,"leave skipToItem"));
	return true;
}

TianShanIce::Streamer::StreamState SsStreamImpl::getCurrentState(const ::Ice::Current& ) const 
{
	Lock sync(*this);
	ENVLOG(ZQ::common::Log::L_DEBUG,
		PLFMT(getCurrentState,"current state is [%s]"),
		ZQTianShan::Util::convertStreamStateToString(playlistState) );	
	return playlistState;
}

TianShanIce::SRM::SessionPrx SsStreamImpl::getSession(const ::Ice::Current& )
{
	Lock sync(*this);	
	return srmSession;
}

Ice::Long SsStreamImpl::seekStream(::Ice::Long offset, ::Ice::Int startPos , const ::Ice::Current& c ) 
{
	ENVLOG(ZQ::common::Log::L_DEBUG,
		PLFMT(seekStream,"enter seek steram with offset[%ld] startPos[%d]"),
		offset,startPos);
	if (iterCurrentItem()->setupInfo.flags & TianShanIce::Streamer::PLISFlagNoSeek)	
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,PLFMT(seekStream,"offset[%lld], flags[%s], SEEK is not allowed"),offset, ZQTianShan::Util::convertItemFlagToStr(iterCurrentItem()->setupInfo.flags).c_str());
	}
	TianShanIce::StrValues expectProps;
	ZQTianShan::Util::updateStreamInfoValue(expectProps,VALUE_WANTED_STREAM_NPT);
	TianShanIce::Streamer::StreamInfo info =  playEx(0.0f,offset,static_cast<Ice::Short>(startPos),expectProps , c );
	
	Ice::Long	newOffset = ZQTianShan::Util::getStreamTimeOffset(info);
	
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(seekStream,"leave Seek stream with return offset[%ld]"),newOffset);
	return newOffset;
}
bool SsStreamImpl::seekToPosition(::Ice::Int userCtrlNum, 
								  ::Ice::Int offset, 
								  ::Ice::Int startPos,
								  const ::Ice::Current& c )
{
	ENVLOG(ZQ::common::Log::L_DEBUG,
		PLFMT(seekToPosition,"enter seekToPosition with userCtrlNum[%d] offset[%ld] startPos[%d]"),
		userCtrlNum , offset , startPos	);
	TianShanIce::StrValues expectProps;
	(void)playItem(userCtrlNum , offset , static_cast<Ice::Short>(startPos) , 0.0f , expectProps , c );
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(seekToPosition,"leave seekToPosition"));
	return true;
}

Ice::Long	SsStreamImpl::getSessTimeLeft( Ice::Long* nptPassed )
{//this is only available for Playlist service

	Ice::Long tLeft = 0;
	
	//calculate the target time
	if( ( playlistState == TianShanIce::Streamer::stsStreaming ) && iterValid( iterCurrentItem()) && isItemStreaming(iterCurrentItem()) )
	{
#pragma message(__MSGLOC__"TODO: do not forget the critical start item")
		StreamParams paras;
		paras.mask = MASK_TIMEOFFSET | MASK_SCALE;
		std::string streamId = getStreamSessId( iterCurrentItem() );

		int32 ret = SsServiceImpl::doGetStreamAttr( serviceImpl , *this, streamId ,iterCurrentItem()->setupInfo , paras );
		if( ret != ERR_RETURN_SUCCESS )
		{
			//throwException( ret , "SsServiceImpl::doGetStreamAttr" );
			ENVLOG(ZQ::common::Log::L_WARNING, PLFMT(getSessTimeLeft,"can't get stream sess[%s]'s attr , return 0 as durationLeft"),
				streamId.c_str() );
			return 0;
		}
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(getSessTimeLeft,"get item [%s] [%d] streamId[%s] attribute: {%s}"),
			iterCurrentItem()->setupInfo.contentName.c_str() ,
			iterCurrentItem()->userCtrlNum,
			streamId.c_str(),
			dumpStreamParas( paras).c_str() );

		if( !( paras.mask & MASK_SCALE ) )
		{//use local record if scale is not returned by porting layer
			paras.scale = speed;
		}
		bool bReverseStreaming = paras.scale < -0.01f;
		
		if( ! ( paras.mask & MASK_TIMEOFFSET ) )
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
				PLFMT(getSessTimeLeft,"time offset is not returned from porting layer"));
		}

		if( bReverseStreaming )
		{
			tLeft = paras.timeoffset;
			if( nptPassed )
			{
				if( !getItemAttribute (iterCurrentItem()) )
				{
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
						PLFMT(getSessTimeLeft,"can't get item play duration"));
				}
				*nptPassed = iterCurrentItem()->itemRealDuration - paras.timeoffset;				
			}
		}
		else
		{
			if( !getItemAttribute (iterCurrentItem()) )
			{
				ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
					PLFMT(getSessTimeLeft,"can't get item play duration"));
			}
			tLeft = iterCurrentItem()->itemRealDuration - paras.timeoffset;
			if( nptPassed )
			{
				*nptPassed = paras.timeoffset;
			}			
		}		

		
		if( paras.mask & MASK_SCALE  && (fabs( paras.scale -0.00f ) >0.01f) )
		{
			tLeft = (Ice::Long)((double)(tLeft) / fabs(paras.scale) );
			if( nptPassed )
			{
				*nptPassed = (Ice::Long)((double)*nptPassed / fabs(paras.scale) );
			}
		}
		else if( fabs(speed -0.00f ) > 0.01f )
		{
			tLeft = (Ice::Long)((double)tLeft / fabs(speed) );
			if( nptPassed )
			{
				*nptPassed = (Ice::Long)((double)*nptPassed / fabs(speed) );
			}
		}
	
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(getSessTimeLeft,"get duration left[%ld] according to item realPlayDuration[%ld] ,item[%s][%d] inTimeOffset[%ld] outTimeOffset[%ld]"),
			tLeft ,
			iterCurrentItem()->itemRealDuration,
			iterCurrentItem()->setupInfo.contentName.c_str(),
			iterCurrentItem()->userCtrlNum,
			iterCurrentItem()->setupInfo.inTimeOffset,
			iterCurrentItem()->setupInfo.outTimeOffset);
		return tLeft;	
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_DEBUG, PLFMT(getSessTimeLeft,"current item is not being streamed"));
		return env->getConfig().iPlaylistTimeout;
	}
}

bool SsStreamImpl::restorePortalSession( iter it , TianShanIce::Streamer::StreamState& state , float& scale )
{
	std::string sessId = getStreamSessId( it );
	if( sessId.empty() )
		return false;

	serviceImpl.registerStreamId( sessId , ident.name );//register current portal session

	try
	{
		StreamParams paras;
		paras.mask = MASK_SCALE_WANTED | MASK_STREAM_STATE_WANTED | MASK_SESSION_RESTORE;
		int32 ret  = ERR_RETURN_SUCCESS;
		if( iterValid( it ) )
		{
			ret = SsServiceImpl::doGetStreamAttr( serviceImpl , *this , sessId , it->setupInfo , paras );
		}
		else
		{
			TianShanIce::Streamer::PlaylistItemSetupInfo dummyInfo;
			ret = SsServiceImpl::doGetStreamAttr( serviceImpl , *this , sessId , dummyInfo , paras );
		}

		if( ret != ERR_RETURN_SUCCESS )
		{
			ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SsStreamImpl,"restorePortalSession() portal indicate that the session[%s] is not available"), 
				sessId.c_str() );
			return false;
		}
		state = paras.streamState;
		scale = paras.scale;
		ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(SsStreamImpl,"restorePortalSession() got session[%s], scale[%f] state[%d]"),
			sessId.c_str() , scale , state );
		return true;
	}
	catch( const TianShanIce::BaseException& ex)
	{
		ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SsStreamImpl,"restorePortalSession() caught %s"),ex.message.c_str() );
		return false;
	}
	catch( const Ice::Exception& ex)
	{
		ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SsStreamImpl,"restorePortalSession() caught %s"),ex.ice_name().c_str() );
		return false;
	}
}

void SsStreamImpl::onRestore( const ::Ice::Current& )
{
	serviceImpl.getRenewTicketCenter().doRegister(ident, pathTicket );//register current session into ticket renew center
	
	TianShanIce::Streamer::StreamState state = playlistState;
	float scale  = speed ;
	
	std::string sess[2];
	std::string effectiveSess;
	size_t count = 0;

	if( env->getConfig().iSupportPlaylist == LIB_SUPPORT_NORMAL_PLAYLIST )
	{
		if( iterValid( iterCurrentItem() ) )
		{
			effectiveSess = iterCurrentItem()->sessionId;
			if(! restorePortalSession(iterCurrentItem() , state , scale ))
			{
				sess[count++] = iterCurrentItem()->sessionId;
			}
		}
		if( iterValid( iterNextItem() ) )
		{
			effectiveSess = iterNextItem()->sessionId;
			if(! restorePortalSession(iterNextItem() , state , scale ))
			{
				sess[count++] = iterNextItem()->sessionId;
			}
		}
				
	}
	else
	{
		effectiveSess = getStreamSessId( iterEndItem() );
		if( !restorePortalSession( iterEndItem() , state , scale ) )
		{
			sess[count++] = effectiveSess;
		}	
	}

	if( !effectiveSess.empty() )
	{
		//if we get a effective session 
		if( env->getConfig().iEnableSGScaleChangeEvent >= 1)
		{
			StreamParams paras;
			paras.mask = MASK_SCALE;
			paras.scale = scale;
			
			serviceImpl.OnStreamEvent( SsServiceImpl::seScaleChanged , effectiveSess , paras , TianShanIce::Properties() );
		}

		if( env->getConfig().iEnableSGStateChangeEvent >= 1)
		{
			StreamParams paras;
			paras.streamState = state;
			paras.mask = MASK_STATE;
			serviceImpl.OnStreamEvent( SsServiceImpl::seStateChanged , effectiveSess , paras , TianShanIce::Properties() );
		}
	}
	if( count > 0 )
	{//issue session gone event
		for( size_t i = 0 ; i < count ; i ++ )
		{
			StreamParams paras;
			serviceImpl.OnStreamEvent( SsServiceImpl::seGone , sess[i] , paras , TianShanIce::Properties() );
		}
	}
	updateTimer();
}

bool	SsStreamImpl::primeNextItem( )
{
	if( LIB_SUPPORT_NORMAL_STREAM == env->getConfig().iSupportPlaylist || 
			LIB_SUPPORT_AUTO_PLAYLIST == env->getConfig().iSupportPlaylist  )
	{
		return true;//do not prime next item 
	}
	if (iterValid(iterNextItem()))
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(primeNextItem,"enter primenext, curitem[%s],nextitem[%s]"),iterCurrentItem()->setupInfo.contentName.c_str(), iterNextItem()->setupInfo.contentName.c_str());
	else
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(primeNextItem,"enter primenext, curitem[%s],nextitem[NULL]"),iterCurrentItem()->setupInfo.contentName.c_str());
	if (isReverseStreaming())
	{
		if (iterNextItem() >= iterCurrentItem() || iterNextItem() <= iterBeginItem())
		{
			setNextItem(iterCurrentItem()-1);
			ENVLOG(ZQ::common::Log::L_INFO,PLFMT(primeNextItem,"reversestreaming setnextitem"));
		}
	}
	else
	{
		if ( iterNextItem() <= iterCurrentItem() || iterNextItem() >= iterEndItem())
		{
			setNextItem(iterCurrentItem()+1);
			ENVLOG(ZQ::common::Log::L_INFO,PLFMT(primeNextItem,"normalstreaming setnextitem"));
		}
	}
	if( !iterValid(iterNextItem()) )
	{
		//updateTimer();
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(primeNextItem,"no next item is valid, return ok"));
		return true;
	}
	if( isItemStreaming( iterNextItem() ) )
	{
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(primeNextItem,"next item[%s][%d][%s] is running, return ok"),
			iterNextItem()->setupInfo.contentName.c_str(),
			iterNextItem()->userCtrlNum,
			iterNextItem()->sessionId.c_str());
		return true;
	}
	Ice::Long diff = (mNextWakeup -IceUtil::Time::now()).toMilliSeconds();

	if( diff > env->getConfig().iPreloadTimeInMS )
	{
		updateTimer();			
		return true;
	}

	//load next item
	{		
		TianShanIce::StrValues expectProps;
		iter tmpNextIt = iterNextItem();
		if (iterValid( iterNextItem() ) && !isItemStreaming(iterNextItem()) )
		{
			if (isReverseStreaming()) //reverse direction
			{
				for (;tmpNextIt > iterBeginItem() && tmpNextIt->sessionId.empty();tmpNextIt--)
				{
					if ( isREWScale(speed) && !checkRestriction(tmpNextIt,TianShanIce::Streamer::PLISFlagNoRew)  )
					{
						ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(primeNextItem,"skip next due to restriction[%s]"),ZQTianShan::Util::convertItemFlagToStr( tmpNextIt->setupInfo.flags ).c_str());
						continue;
					}
					if ( isREWScale(speed) && !checkRestriction(tmpNextIt, TianShanIce::Streamer::PLISFlagSkipAtRew))
					{
						ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(primeNextItem,"skip next due to restriction[%s]"),ZQTianShan::Util::convertItemFlagToStr( tmpNextIt->setupInfo.flags ).c_str());
						continue;
					}
					if (IsItemPlaytimesCountZero(tmpNextIt))
					{
						ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(primeNextItem,"skip item for playtimes = 0"));
						continue;
					}

					StreamParams paras;
					try
					{
						std::string streamId;
						iter itTo = doLoadItem( tmpNextIt , 0 , 0.0f , paras, streamId );				
						itTo->sessionState	= TianShanIce::Streamer::stsStreaming;				
						updateStreamSessId( itTo , streamId );
						setNextItem(itTo);				
					}
					catch(const TianShanIce::BaseException& ex)
					{
						ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(primeNextItem,"caught exception[%s] when load next item"),
								ex.message.c_str());
					}
				}
			}
			else  //normal direction
			{
				for (;tmpNextIt< iterEndItem() && tmpNextIt->sessionId.empty();)
				{
					if ( isFFScale(speed) && !checkRestriction(tmpNextIt,TianShanIce::Streamer::PLISFlagSkipAtFF) )
					{
						ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(primeNextItem,"ads can't play in FF mode,skip flags[%s]"),ZQTianShan::Util::convertItemFlagToStr(tmpNextIt->setupInfo.flags).c_str());
						tmpNextIt+= 1;
						continue;
					}
					StreamParams paras;
					try
					{
						std::string streamId;
						iter itTo = doLoadItem( tmpNextIt , 0 , 0.0f , paras, streamId );
						itTo->sessionState  = TianShanIce::Streamer::stsStreaming;
						updateStreamSessId( itTo , streamId );
						setNextItem(itTo);
						if (iterValid(iterNextItem()))
							ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(primeNextItem,"normal direction after load curritem[%s],nextitem[%s]"),iterCurrentItem()->setupInfo.contentName.c_str(),iterNextItem()->setupInfo.contentName.c_str());
						else
							ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(primeNextItem,"normal direction after load curritem[%s],nextitem[NULL]"),iterCurrentItem()->setupInfo.contentName.c_str());
						break;
					}
					catch(const TianShanIce::BaseException& ex)
					{
						ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(primeNextItem,"caught exception[%s] when load next item"),ex.message.c_str());
					}
				}
			}
//			updateTimer();
		}
	}

	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(primeNextItem,"leave primeNextItem"));
	return true;
}
bool SsStreamImpl::doDestroySession( const std::string& streamId )
{
	if( streamId.empty())
		return true;

	int32 ret = SsServiceImpl::doDestroy( serviceImpl ,*this, streamId );
	if( ret != ERR_RETURN_SUCCESS && ret != ERR_RETURN_OBJECT_NOT_FOUND )
	{
		throwException( ret , "SsServiceImpl::doDestroy" );
		return false;
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_INFO, PLFMT(doDestroySession,"session [%s] is destroyed"),
			streamId.c_str());
		return true;
	}
}

bool SsStreamImpl::doDestroySession( iter it )
{
	if( !iterValid(it) )
	{
		return true;
	}
	std::string streamId = getStreamSessId(it);
	if( !streamId.empty() )
	{
		if( doDestroySession( streamId ) )
		{
			it->sessionId = "";
			it->sessionState = TianShanIce::Streamer::stsStop;		
		}
	}
	return true;
}

bool SsStreamImpl::doPauseSession( iter it , const int32& expectMask , TianShanIce::Streamer::StreamInfo& info )
{
	Lock sync(*this);

	if ( isItemStreaming(it))
	{
		if ( it->setupInfo.flags & TianShanIce::Streamer::PLISFlagNoPause )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
				PLFMT(doPauseSession,"flags[%s], PAUSE is not allowed"),
				ZQTianShan::Util::convertItemFlagToStr( it->setupInfo.flags ).c_str());
		}
	}
	switch ( playlistState )
	{
	case TianShanIce::Streamer::stsSetup:
	case TianShanIce::Streamer::stsStop:
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,LOGCATAGORY,0,
				PLFMT(doPauseSession,"current is in [%s] do not allowed to pause"),
				ZQTianShan::Util::dumpTianShanStreamState(playlistState));
		}
		break;
	case TianShanIce::Streamer::stsPause:
		{
			ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doPauseSession,"current is in [%s], return ok"),
				ZQTianShan::Util::dumpTianShanStreamState(playlistState));
			
			std::string		streamPort	= getStreamPort();
			std::string		streamId	= getStreamSessId(it);
			StreamParams	paras;
			paras.mask	=	getExpectParamMask( expectMask );
			int32 ret = ERR_RETURN_SUCCESS;
			if( iterValid(it))
			{
				ret = SsServiceImpl::doGetStreamAttr( serviceImpl , 
															*this,
															streamId,
															it->setupInfo,
															paras );
			}
			else
			{
				ret = SsServiceImpl::doGetStreamAttr( serviceImpl , 
															*this,
															streamId,
															TianShanIce::Streamer::PlaylistItemSetupInfo(),
															paras );
			}
			if( ret != ERR_RETURN_SUCCESS )
			{
				throwException( ret , "SsServiceImpl::doPause" );
			}
			updateLocalRecord( paras , TianShanIce::Streamer::stsPause );
			constructStreamInfo( paras, expectMask , iterCurrentItem() , info );			
			return true;
		}
		break;
	case TianShanIce::Streamer::stsStreaming:
		{
			//pause the stream
			std::string		streamPort	= getStreamPort();
			std::string		streamId	= getStreamSessId(it);
			StreamParams	paras;
			paras.mask	=	getExpectParamMask( expectMask );
			int32 ret = SsServiceImpl::doPause( serviceImpl , *this , streamId , paras );
			if( ret != ERR_RETURN_SUCCESS )
			{
				throwException( ret , "SsServiceImpl::doPause" );
			}			
			updateLocalRecord( paras , TianShanIce::Streamer::stsPause );
			constructStreamInfo( paras , expectMask , it , info );
			primeNextItem();
			return true;
		}
		break;
	}

	return false;
}

SsStreamImpl::iter	SsStreamImpl::doLoadItem(iter itTo, 
											 Ice::Long timeoffset , 
											 Ice::Float scale ,											 
											 StreamParams& paras,
											 std::string& streamId )
{
	if( !iterValid(itTo) )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
			PLFMT(doLoadItem,"invalid item") );
	}
	if( isItemStreaming(iterCurrentItem()))
	{
		if ( timeoffset != 0 && !checkRestriction(iterCurrentItem(), TianShanIce::Streamer::PLISFlagNoSeek) )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
				PLFMT(doLoadItem,"flags[%s], SEEK is not allowed"),
				ZQTianShan::Util::convertItemFlagToStr( iterCurrentItem()->setupInfo.flags ).c_str());
		}
		if ( isFFScale(scale) && !checkRestriction(itTo, TianShanIce::Streamer::PLISFlagNoFF) )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
				PLFMT(doLoadItem,"new scale[%f] and flags[%s], FF is not allowed"),
				scale , ZQTianShan::Util::convertItemFlagToStr(itTo->setupInfo.flags).c_str());
		}

		if( isREWScale(scale) && !checkRestriction(itTo, TianShanIce::Streamer::PLISFlagNoRew))
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
				PLFMT(doLoadItem,"new scale[%f] and flags[%s], REW is not allowed"),
				scale , ZQTianShan::Util::convertItemFlagToStr(itTo->setupInfo.flags).c_str());
		}
	}

	bool	bForward	=	false;
	//detect new direction
	if( fabs( scale - 0.00f ) < 0.01f )
	{
		if( bFirstPlay )
		{
			bForward = true;
			scale = 1.0f;			
		}
		else
		{
			bForward = speed > 0.01f;
		}
	}
	else
	{
		bForward = scale > 0.01f;
	}

	std::string		streamPort = getStreamPort();	

	int32 ret = ERR_RETURN_SUCCESS;
	do
	{	
		ENVLOG(ZQ::common::Log::L_INFO, PLFMT(doLoadItem,"loading item [%s][%d] with timeoffset[%ld] sclae[%f]"),
			itTo->setupInfo.contentName.c_str(), itTo->userCtrlNum,
			timeoffset,	scale );

		paras.inTimeOffset = itTo->setupInfo.inTimeOffset;
		paras.outTimeOffset = itTo->setupInfo.outTimeOffset;
		ret = SsServiceImpl::doLoad(	serviceImpl, *this, itTo->setupInfo,
										timeoffset , scale , paras , streamId ) ;
		if( ret != ERR_RETURN_SUCCESS )
		{
			itTo	+= bForward ? 1 : -1;
			Ice::Short from	=  bForward ? 1 : 2;
			if( iterValid(itTo))
				convertPositionToItemWide( itTo , timeoffset , from , bForward );
			timeoffset = 0;
			continue;
		}
		if( streamId.empty() )
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
				PLFMT(doLoadItem,"doLoad return success , but none valid stream id is returned"));
		}
		ret = SsServiceImpl::doPlay( serviceImpl , *this , streamId , timeoffset , scale ,paras );
		if( ret != ERR_RETURN_SUCCESS )
		{
			itTo += bForward ? 1 : -1;
			Ice::Short from = bForward ? 1 : 2;
			if( iterValid(itTo))
				convertPositionToItemWide( itTo , timeoffset , from ,bForward );
			timeoffset = 0;
			continue;
		}						
	}while( ret != ERR_RETURN_SUCCESS  && iterValid(itTo) );

	if( !iterValid(itTo))
	{
		throwException(ERR_RETURN_SERVER_ERROR,"SsServiceImpl::doLoad");
	}
	else
	{
		if( bFirstPlay )
		{
			bFirstPlay = false;			
		}
		serviceImpl.registerStreamId( streamId , ident.name );
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doLoadItem,"loaded item [%s][%d], sessionId[%s]"),
			itTo->setupInfo.contentName.c_str(), itTo->userCtrlNum, streamId.c_str());
	}
	return itTo;
}
SsStreamImpl::iter	SsStreamImpl::doChangeScale( iter itTo , Ice::Float scale , StreamParams& paras )
{
	if( !iterValid(itTo) )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
			PLFMT(doLoadItem,"invalid item") );
	}
	
	if ( isFFScale(scale) && !checkRestriction(itTo,TianShanIce::Streamer::PLISFlagNoFF))
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
			PLFMT(doChangeScale,"new scale[%f] and flags[%s], FF is not allowed"),
			scale , ZQTianShan::Util::convertItemFlagToStr(itTo->setupInfo.flags).c_str());
	}
	if( isREWScale(scale) && !checkRestriction(itTo, TianShanIce::Streamer::PLISFlagNoRew))
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
			PLFMT(doChangeScale,"new scale[%f] and flags[%s], REW is not allowed"),
			scale , ZQTianShan::Util::convertItemFlagToStr(itTo->setupInfo.flags).c_str());
	}
	std::string streamPort = getStreamPort();
	stampSpeedChangeReq = ZQ::common::TimeUtil::now();
	if( canChangeScale(scale) )
	{		
		ENVLOG(ZQ::common::Log::L_DEBUG,
			PLFMT(doLoadItem,"change scale with item [%s][%d] with sclae[%f], stampSpeedChangeReq[%lu]"),
			itTo->setupInfo.contentName.c_str(),
			itTo->userCtrlNum,
			scale,
			stampSpeedChangeReq);

		int32 ret = SsServiceImpl::doChangeScale( serviceImpl , 
											*this,											
											itTo->sessionId,
											scale,
											paras );
		if( ret != ERR_RETURN_SUCCESS )
		{
			throwException( ret , "SsServiceImpl::doChangeScale" );
		}
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doChangeScale,"change [%s][%d]'s scale to[%f]"),
			itTo->setupInfo.contentName.c_str(),
			itTo->userCtrlNum,
			paras.scale );
		return itTo;
	}
	else
	{
		//an error has been threw out
		itTo = iterEndItem();
	}
	return itTo;
}

SsStreamImpl::iter SsStreamImpl::doRepositionItem( iter itTo ,Ice::Long timeoffset , 
													 Ice::Float scale , StreamParams& paras )
{
	if( !iterValid(itTo) )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
			PLFMT(doLoadItem,"invalid item") );
	}
	bool bChangeParameter = false;
	if ( timeoffset != 0 && !checkRestriction(iterCurrentItem(),TianShanIce::Streamer::PLISFlagNoSeek))
	{
		bChangeParameter = true;
		timeoffset = 0;
	}
	if ( isFFScale(scale) && !checkRestriction(iterCurrentItem(),TianShanIce::Streamer::PLISFlagNoFF)||
		isREWScale(scale) && !checkRestriction(iterCurrentItem(),TianShanIce::Streamer::PLISFlagNoRew))
	{
		bChangeParameter = true;
		scale = 1.0;
	}
	if (bChangeParameter)
	{
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doReposition,"adjust timeoffset[%ld],speed[%lf] dut to restriction[%s]"),timeoffset,scale,ZQTianShan::Util::convertItemFlagToStr(iterCurrentItem()->setupInfo.flags).c_str() );
		
	}
	if ( fabs(scale - speed) > 0.01f ) //speed change
	{
		stampSpeedChangeReq = ZQ::common::TimeUtil::now();
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doReposition,"stampSpeedChangeReq[%lu]"),stampSpeedChangeReq );
	}
	std::string streamPort = getStreamPort();
	ENVLOG(ZQ::common::Log::L_DEBUG,
		PLFMT(doLoadItem,"reposition item [%s][%d] with timeoffset[%ld] sclae[%f]"),
		itTo->setupInfo.contentName.c_str(),
		itTo->userCtrlNum,
		timeoffset,scale );

	int32 ret = SsServiceImpl::doReposition( serviceImpl , *this , itTo->sessionId ,timeoffset , scale , paras );
	if( ret )
	{
		throwException( ret ,"SsServiceImpl::doReposition");
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_INFO,
			PLFMT(doRepositionItem,"reposition item[%s][%d] to timeoffset[%ld] scale[%f]"),
			itTo->setupInfo.contentName.c_str(),
			itTo->userCtrlNum,
			paras.timeoffset,
			paras.scale);
	}
	return itTo;

}

SsStreamImpl::iter SsStreamImpl::doPlaySession(	iter it, 
									Ice::Short originalFrom, 
									Ice::Long originalTimeOffset , 
									Ice::Float scale ,
									const int32& expectMask ,
									TianShanIce::Streamer::StreamInfo& info )
{
	Lock sync(*this);

	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(doPlaySession,"enter with from[%d] timeoffset[%ld] scale[%f] expectMask[%x] status[%s]"),
		originalFrom , originalTimeOffset , scale , expectMask ,
		iterValid(it) ? "ITEMWIDE":"STREAMWIDE");
	
	
	///first I need to know the input parameters is bound to StreamWide or ItemWide 
	/// So I can decide how to convert these parameter
	bool bClientItemWide = iterValid( it );

	StreamParams		paras;
	paras.mask	=		getExpectParamMask( expectMask );
	
	std::string			streamId ;
	std::string			streamPort = getStreamPort();

	///detect the target streaming direction forward or backward
	bool				bForward	=	false;	
	if( fabs( scale - 0.00f ) < 0.01f )
	{
		bForward = speed > 0.01f;
	}
	else
	{
		bForward = scale > 0.01f;
	}

	Ice::Short	from		= originalFrom;
	Ice::Long	timeoffset	= originalTimeOffset;

	switch( env->getConfig().iSupportPlaylist )
	{
	case LIB_SUPPORT_NORMAL_PLAYLIST:
		{
			iter itTo = iterEndItem();
			if( bClientItemWide )
			{
				itTo = convertPositionToItemWide( it, timeoffset , from , bForward );
			}
			else
			{
				itTo = convertPositionToItemWide(timeoffset,from ,bForward );
			}
			if( !iterValid( itTo ) )
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,LOGCATAGORY,0,
					PLFMT( doPlaySession , "bad input parameter") );
			}
			switch( playlistState )
			{
			case TianShanIce::Streamer::stsSetup:
			case TianShanIce::Streamer::stsStop:
				{
					itTo = doLoadItem( itTo, timeoffset, scale, paras, streamId );
					if( iterValid(itTo) )
					{						
						updateStreamSessId( itTo , streamId );
						itTo->sessionState		= TianShanIce::Streamer::stsStreaming;				
						updateLocalRecord( paras , TianShanIce::Streamer::stsStreaming );						
						constructStreamInfo( paras , expectMask , itTo , info );
						setCurrentItem( itTo );
						setNextItem( isReverseStreaming() ? itTo -1 : itTo + 1 );
						if (iterValid(iterNextItem()))
							ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(doPlaySession,"stsStop curritem[%s],nextitem[%s]"),iterCurrentItem()->setupInfo.contentName.c_str(), iterNextItem()->setupInfo.contentName.c_str());
						else
							ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(doPlaySession,"stsStop curritem[%s],nextitem[NULL]"),iterCurrentItem()->setupInfo.contentName.c_str());

						primeNextItem();
						return itTo;
					}
					else
					{
#pragma message(__MSGLOC__"TODO: should I send out a end-of-stream event??")
						ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
							PLFMT(doPlaySession,"can't load any item"));
					}
				}
				break;
			case TianShanIce::Streamer::stsPause:
				{
					if( !iterValid( iterCurrentItem() ) )
					{
						ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,LOGCATAGORY,0,
							PLFMT(doPlaySession,"no item is in pause state"));
					}
					if( originalFrom == 0 && originalTimeOffset == 0 && fabs(scale-0.00f) < 0.01f )
					{//just resume the item session
						int32 ret = SsServiceImpl::doResume( serviceImpl, *this , iterCurrentItem()->sessionId , paras );
						if( ret != ERR_RETURN_SUCCESS )
						{
							throwException( ret , "SsServiceImpl::doResume");
						}
						iterCurrentItem()->sessionState	= TianShanIce::Streamer::stsStreaming;
						updateLocalRecord( paras , TianShanIce::Streamer::stsStreaming );
						constructStreamInfo( paras , expectMask , iterCurrentItem() , info );
						primeNextItem();
						return iterCurrentItem();
					}				
				}
				//DO NOT BREAK HERE because
			case TianShanIce::Streamer::stsStreaming:
				{					
					int32 ret = ERR_RETURN_SUCCESS;
					std::string		sessionCurrent;
					std::string		sessionNext;
					
					iter tmpIterCurrent = iterCurrentItem();
					iter tmpIterNext	= iterNextItem();

					if( iterValid( iterCurrentItem() ) )	sessionCurrent	= iterCurrentItem()->sessionId;
					if ( iterValid( iterNextItem() ) )		sessionNext		= iterNextItem()->sessionId;

					if( !isItemStreaming( itTo ) )
					{
						itTo = doLoadItem( itTo , timeoffset , scale , paras, streamId );
						if( iterValid(itTo) )
						{
							if(!sessionNext.empty())  doDestroySession( sessionNext );
							if(!sessionCurrent.empty())  doDestroySession( sessionCurrent);							
							
							updateStreamSessId( itTo, streamId );
							itTo->sessionState		= TianShanIce::Streamer::stsStreaming;					
							updateLocalRecord( paras , TianShanIce::Streamer::stsStreaming );
							constructStreamInfo( paras , expectMask , itTo , info );
							setCurrentItem( itTo );
							setNextItem( isReverseStreaming() ? itTo -1 : itTo + 1 );
							if (iterValid(iterNextItem()))
								ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(doPlaySession,"stsStreaming curritem[%s],nextitem[%s]"),iterCurrentItem()->setupInfo.contentName.c_str(), iterNextItem()->setupInfo.contentName.c_str());
							else
								ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(doPlaySession,"stsStreaming curritem[%s],nextitem[NULL]"),iterCurrentItem()->setupInfo.contentName.c_str());

							primeNextItem();
							return itTo;
						}
						else
						{
							ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
								PLFMT(doPlaySession,"can't load any item"));
						}
					}
					//in streaming state
					else if( originalFrom == 0 && originalTimeOffset == 0 )
					{//do not reposition
						if( fabs( scale -0.00f) < 0.01f )
						{//do not change scale
							//just get information
							if( !iterValid(iterCurrentItem()))
							{
								ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,LOGCATAGORY,0,
									PLFMT(doPlaySession,"Current item is not valid"));
							}
							if( isItemStreaming(iterCurrentItem()))
							{
								ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,LOGCATAGORY,0,
									PLFMT(doPlaySession,"curren item is not running"));
							}
							paras.mask |= MASK_STATE;
							ret = SsServiceImpl::doGetStreamAttr( serviceImpl, *this,iterCurrentItem()->sessionId,iterCurrentItem()->setupInfo , paras );
							if( ret != ERR_RETURN_SUCCESS )
							{
								throwException( ret , "SsServiceImpl::doGetStreamAttr");
							}					
							if( paras.mask & MASK_STATE )
								updateLocalRecord( paras , paras.streamState );
							else
								updateLocalRecord( paras , TianShanIce::Streamer::stsStreaming );

							constructStreamInfo( paras , expectMask , itTo , info );
							return iterCurrentItem();
						}
						else
						{//change scale
							if( canChangeScale( scale ) )
							{						
#pragma message(__MSGLOC__"TODO: must be noticed that here need to be changed")
								itTo = doChangeScale( itTo, scale, paras );								
								if( speed * scale < 0.1f )
								{//new direction is not the same as current direction , just destroy next item session
									if( tmpIterNext != itTo )
									{
										doDestroySession( tmpIterNext );
									}
									else
									{
										if(!sessionNext.empty()) doDestroySession(sessionNext) ;//destroy next session if it exist
									}
								}
								updateLocalRecord( paras , playlistState );
								constructStreamInfo( paras , expectMask , itTo , info );
								primeNextItem();
								return it;
							}
							else
							{//an error has already been threw out
							}
						}
					}
					else
					{//reposition				
						if( itTo == iterCurrentItem() )
						{	
							ENVLOG(ZQ::common::Log::L_DEBUG,
								PLFMT(doLoadItem,"reposition item [%s][%d] with timeoffset[%ld] sclae[%f]"),
								itTo->setupInfo.contentName.c_str(),
								itTo->userCtrlNum,
								timeoffset,scale );
							itTo = doRepositionItem( itTo , timeoffset , scale , paras)	;
							updateLocalRecord( paras , TianShanIce::Streamer::stsStreaming );
							constructStreamInfo( paras , expectMask , itTo , info );
							primeNextItem();
							return itTo;
						}
						else
						{
							itTo = doLoadItem( itTo , timeoffset , scale , paras, streamId );				
							//unload current session and next session
							if( tmpIterNext != itTo )
							{
								doDestroySession( tmpIterNext);
							}
							else
							{
								if(!sessionNext.empty())	doDestroySession( sessionNext );
							}
							if( tmpIterCurrent != itTo )
							{
								doDestroySession(tmpIterCurrent);
							}
							else
							{
								if(!sessionCurrent.empty()) doDestroySession( sessionCurrent );
							}

							updateStreamSessId( itTo , streamId );
							itTo->sessionState	= TianShanIce::Streamer::stsStreaming;
							updateLocalRecord( paras , TianShanIce::Streamer::stsStreaming );								
							constructStreamInfo( paras , expectMask , itTo , info );
							setCurrentItem( itTo );
							setNextItem( isReverseStreaming() ? itTo -1 : itTo + 1 );
							if (iterValid(iterNextItem()))
								ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(doPlaySession,"InStreaming unload current curritem[%s],nextitem[%s]"),iterCurrentItem()->setupInfo.contentName.c_str(), iterNextItem()->setupInfo.contentName.c_str());
							else
								ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(doPlaySession,"InStreaming unload current curritem[NULL],nextitem[%s]"),iterCurrentItem()->setupInfo.contentName.c_str());

							primeNextItem();
							return itTo;
						}
					}
				}
				break;
			default:
				{
					ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
						PLFMT(doPlaySession,"unknown playlist state"));
				}
				break;
			}
		
		}
		break;
	default:
		{
			streamId = getStreamSessId( iterEndItem() );//get streamId 
			if(bClientItemWide)
			{
				convertPositionToStreamWide(it,timeoffset,from);
			}
			else
			{
				convertPositionToStreamWide(timeoffset,from);
			}
			switch ( playlistState )
			{
			case TianShanIce::Streamer::stsSetup:
			case TianShanIce::Streamer::stsStop:
				{					
					TianShanIce::Streamer::PlaylistItemSetupInfo dummyInfo;
					int32 ret = SsServiceImpl::doLoad( serviceImpl , *this,
														dummyInfo , timeoffset , scale , 														
														paras , streamId );
					if( ret != ERR_RETURN_SUCCESS )
					{
						throwException( ret , "SsServiceImpl::doLoad");
					}
					//dummy item do not has inTimeOffset and outTimeOffset
					ret = SsServiceImpl::doPlay( serviceImpl , *this,streamId , 
												timeoffset , scale , paras );
					if( ret != ERR_RETURN_SUCCESS )
					{
						throwException( ret , "SsServiceImpl::doPlay");
					}
					//record stream session id into playlist attributes
					updateStreamSessId( iterEndItem() , streamId );
					//register session id into service instance so StreamService can find the the playlist through the session id
					serviceImpl.registerStreamId( streamId , ident.name );
					updateLocalRecord(paras , TianShanIce::Streamer::stsStreaming );
					constructStreamInfo( paras , expectMask , iterEndItem() , info );
					return iterEndItem();
				}
				break;
			case TianShanIce::Streamer::stsPause:
				{
					if( from == 0 && timeoffset == 0 && fabs(scale-0.00f)<0.01f )
					{
						int32 ret =  SsServiceImpl::doResume( serviceImpl , *this, streamId , paras );
						if( ret != ERR_RETURN_SUCCESS )
						{
							throwException( ret ,"SsServiceImpl::doResume" );
						}				
						updateLocalRecord(paras , TianShanIce::Streamer::stsStreaming );
						constructStreamInfo( paras , expectMask , iterEndItem() , info );
						return iterEndItem();
					}
				}
				//DO NOT BREAK HERE
			case TianShanIce::Streamer::stsStreaming:
				{
					//get stream id from private data
					streamId = getStreamSessId( iterEndItem());
					int32 ret = ERR_RETURN_SUCCESS;
					if( ( from == 0 && timeoffset == 0 ) )
					{
						if( fabs(scale-0.00f)<0.01f )
						{
							paras.mask |= MASK_STATE;
							ret = SsServiceImpl::doGetStreamAttr( serviceImpl, *this, streamId, iterCurrentItem()->setupInfo , paras );
						}
						else
						{
							ret = SsServiceImpl::doChangeScale( serviceImpl, *this,	streamId, scale, paras );
						}
					}
					else
					{
						paras.mask |= MASK_TIMEOFFSET;
						// inTimeOffset and outTimeOffset is not set in streamMode
						ret = SsServiceImpl::doPlay(serviceImpl , *this, streamId , timeoffset , scale , paras );							
					}

					if( ret != ERR_RETURN_SUCCESS )
					{
						throwException( ret ,"SsServiceImpl::doPlay" );
					}
					updateLocalRecord( paras , TianShanIce::Streamer::stsStreaming );
					constructStreamInfo( paras , expectMask , iterEndItem() , info );
					return iterEndItem();
				}
				break;
			default:
				{
					assert(false);
				}
				break;
			}
		}
	}
	return iterEndItem();
}

TianShanIce::Streamer::StreamInfo SsStreamImpl::playEx(::Ice::Float speed, 
													   ::Ice::Long offset, 
													   ::Ice::Short from,
													   const ::TianShanIce::StrValues& expectedProps, 
													   const ::Ice::Current& )
{
	TianShanIce::Streamer::StreamInfo info;

	
//	bool	bRet		=	false;
	int32	expectMask	=	getExpectParamMask( expectedProps );
	
	Lock sync(*this);
	if( itemCount() <= 0 )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,LOGCATAGORY,0,PLFMT(playEx,"no item is playlist"));
	}
	iter it = iterEndItem();
	
	stampStateChangeReq = ZQ::common::TimeUtil::now();
	recordInstanceState( mInstanceStateBeforeAction );

	doPlaySession( iterEndItem() , from , offset , speed , expectMask , info );	
	
	recordInstanceState( mInstanceStateAfterAction );
	analyzeInstanceState( );
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(playEx,"leave playEx stampStateChangeReq[%lu]"),stampStateChangeReq);

	return info;

}

TianShanIce::Streamer::StreamInfo SsStreamImpl::pauseEx(const ::TianShanIce::StrValues& expectedProps, const ::Ice::Current& )
{	
	TianShanIce::Streamer::StreamInfo info;
	int32	expectMask	=	getExpectParamMask( expectedProps );

	Lock sync(*this);
	
	if( !bAlive )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,LOGCATAGORY,0,PLFMT(playEx,"playlist is destroyed"));
	}

	switch ( env->getConfig().iSupportPlaylist )
	{
	case LIB_SUPPORT_NORMAL_PLAYLIST:
		{
			doPauseSession( iterCurrentItem() , expectMask , info );			
		}
		break;
	case LIB_SUPPORT_AUTO_PLAYLIST:
	case LIB_SUPPORT_NORMAL_STREAM:
		{
			doPauseSession( iterEndItem() , expectMask , info );			
		}
		break;
	default:
		{
			assert(false);
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
				PLFMT(pauseEx,"not supported service mode"));
		}
	}	

	return info;	
}

TianShanIce::Streamer::StreamInfo SsStreamImpl::playItem(::Ice::Int userCtrlNum ,
														 ::Ice::Int offset , 
														 ::Ice::Short from, 
														 ::Ice::Float scale, 
														 const ::TianShanIce::StrValues& expectedProps, 
														 const ::Ice::Current&  )
{
	TianShanIce::Streamer::StreamInfo info;
	iter it = findItemWithUserCtrlNum(userCtrlNum);
	if( !iterValid( it ) )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
			PLFMT(playItem,"can't find item with ctrl num [%d]"), userCtrlNum );
	}
	
	int32	expectMask	=	getExpectParamMask( expectedProps );

	recordInstanceState(mInstanceStateBeforeAction);
	
	doPlaySession( it , from , offset , scale , expectMask , info );

	recordInstanceState(mInstanceStateAfterAction);
	analyzeInstanceState( );

	return info;
}

::TianShanIce::Streamer::PlaylistItemSetupInfoS SsStreamImpl::getPlaylistItems(const ::Ice::Current& /*= ::Ice::Current()*/) const {
	Lock sync(*this);
	TianShanIce::Streamer::PlaylistItemSetupInfoS items;
	items.reserve(itemCount());
	for( constIter it = iterFirstItem(); it != iterEndItem(); it ++ ) {
		items.push_back(it->setupInfo);
	}
	return items;
}
bool SsStreamImpl::clearPending(bool , const ::Ice::Current& )
{
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(clearPending,"enter clearPending"));
	{
		Lock sync(*this);
		if( ! iterValid( iterCurrentItem()) )
		{
			ENVLOG(ZQ::common::Log::L_INFO,PLFMT(clearPending,"leave clearPending"));
			return true;
		}		
		iter it = iterNextItem();
		Ice::Int curCtrlNum = iterCurrentItem()->userCtrlNum ;
		if( isReverseStreaming() )
		{//
			if ( it == iterBeginItem() )
			{
				return true;
			}
			items.erase( iterBeginItem() , it - 1 );
		}
		else
		{	
			if ( it == iterEndItem() )
			{
				return true;
			}
			items.erase( it + 1, iterEndItem() );
		}		
		setCurrentItem( curCtrlNum );
		setNextItem( isReverseStreaming() ? ( iterCurrentItem() -1) : ( iterCurrentItem() + 1 ) );
		if (iterValid(iterNextItem()))
			ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(clearPending,"curritem[%s],nextitem[%s]"),iterCurrentItem()->setupInfo.contentName.c_str(), iterNextItem()->setupInfo.contentName.c_str());
		else
			ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(clearPending,"curritem[%s],nextitem[NULL]"),iterCurrentItem()->setupInfo.contentName.c_str());

	}
	return true;
}

Ice::Int SsStreamImpl::flushExpired(const ::Ice::Current& ) 
{
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(clearPending,"enter flushExpired"));
	
	{
		Lock sync(*this);
		if(  !iterValid( iterCurrentItem( ) ) )
		{
#pragma message(__MSGLOC__"TODO: not implement yet")

			//�Ѿ������ˣ�����
		}

		iter it = iterNextItem();
		Ice::Int curCtrlNum = iterCurrentItem()->userCtrlNum ;
		if( isReverseStreaming() )
		{//�����ܵ�ʱ��
			if ( it == iterBeginItem() )
			{
				return true;
			}
			items.erase( iterBeginItem() , it - 1 );
		}
		else
		{	
			if ( it == iterEndItem() )
			{
				return true;
			}
			items.erase( it + 1, iterEndItem() );
		}		
		setCurrentItem( curCtrlNum );
		setNextItem( isReverseStreaming() ? (iterCurrentItem() -1) : (iterCurrentItem() + 1) );
		if (iterValid(iterNextItem()))
			ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(flushExpired,"curritem[%s],nextitem[%s]"),iterCurrentItem()->setupInfo.contentName.c_str(), iterNextItem()->setupInfo.contentName.c_str());
		else
			ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(flushExpired,"curritem[%s],nextitem[NULL]"),iterCurrentItem()->setupInfo.contentName.c_str());
	}
	return TianShanIce::Streamer::InvalidCtrlNum;
}

void SsStreamImpl::erase(::Ice::Int userCtrlNum, const ::Ice::Current& ) 
{	
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(erase,"enter erase with userCtrlNum[%d]"),userCtrlNum);
		
//	bool bRet = false;
	try
	{
		Lock sync(*this);
		constIter it = findItemWithUserCtrlNum(userCtrlNum);		
		if( !iterValid( it ) )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
				PLFMT(erase,"invalid userCtrlNum[%d]"),userCtrlNum);
		}
		Ice::Int ctrlNum = TianShanIce::Streamer::InvalidCtrlNum;
		if( it == iterCurrentItem()  )
		{
			if (  iterValid( iterNextItem() )  )
				ctrlNum = iterNextItem()->userCtrlNum;
		}
		else
		{
			if( iterValid(iterCurrentItem()) )
				ctrlNum = iterCurrentItem()->userCtrlNum;
		}

		if( isItemStreaming( it ) )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,LOGCATAGORY,0,
				PLFMT(erase,"invalid state, item [%d] is running"),userCtrlNum );
		}				
		setCurrentItem(ctrlNum);		
		if (iterValid(iterNextItem()))
			ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(erase,"curritem[%s],nextitem[%s]"),iterCurrentItem()->setupInfo.contentName.c_str(), iterNextItem()->setupInfo.contentName.c_str());
		else
			ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(erase,"curritem[%s],nextitem[NULL]"),iterCurrentItem()->setupInfo.contentName.c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
			PLFMT(erase,"caught unknown expcetion when call erase from porting layer"));
	}
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(erase,"leave erase"));

}

Ice::Int SsStreamImpl::current(const ::Ice::Current& ) const
{
	Lock sync(*this);
	if(itemCount() <= 0 )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,LOGCATAGORY,0,
			PLFMT(current,"no items in current playlist"));
	}
	return iterCurrentItem()->userCtrlNum;
}

Ice::Int SsStreamImpl::insert(::Ice::Int userCtrlNum, 
							const ::TianShanIce::Streamer::PlaylistItemSetupInfo& info,
							Ice::Int where,
							const ::Ice::Current& )
{
	ZQTianShan::Util::TimeSpan sw;sw.start();
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(insert,"insert item [%d] after [%d] with info [%s]"),
		userCtrlNum,where,
		ZQTianShan::Util::dumpPlaylistItemSetupInfo(info).c_str() );

	Lock sync(*this);

	if( itemCount() >= env->getConfig().iMaxPlaylistItemCount ) 
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
			PLFMT(insert,"playlist items exceeded limitation [%d]"), (int)itemCount() );
	}

	if( iterValid(findItemWithUserCtrlNum(userCtrlNum)))
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,
			PLFMT(insert,"duplicate item ctrl num [%d] "),userCtrlNum );
	}
	
	
	if( itemCount() >= 2 )
	{
		if( isItemStreaming(iterNextItem()) && isItemStreaming(iterCurrentItem()) )
		{
			if( isReverseStreaming() )
			{
				if ( where == iterNextItem()->userCtrlNum )
				{
					ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,LOGCATAGORY,0,
						PLFMT(insert,"current stream is reverse steraming , and inserted position [%d] is located between next and current running item "),
						where);
				}
			}
			else
			{
				if ( where == iterCurrentItem()->userCtrlNum )
				{
					ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(ENVLOG,LOGCATAGORY,0,
						PLFMT(insert,"current stream is reverse steraming , and inserted position [%d] is located between next and current running item "),
						where);
				}
			}
		}
	}

	Ice::Int	curCtrlNum	= TianShanIce::Streamer::InvalidCtrlNum;
	Ice::Int	nextCtrlNum	= TianShanIce::Streamer::InvalidCtrlNum;
	if( iterValid( iterCurrentItem() ))
	{
		curCtrlNum	= iterCurrentItem()->userCtrlNum;
	}
	if( iterValid( iterNextItem()) )
	{
		nextCtrlNum	= iterNextItem()->userCtrlNum;
	}

	TianShanIce::Streamer::ItemSessionInfo newItem;
	clearItemSessionInfo( newItem );
	newItem.setupInfo		=	info;
	newItem.sessionState	=	TianShanIce::Streamer::stsSetup;
	newItem.userCtrlNum		=	userCtrlNum;
	int64 playtimes = (info.flags & TianShanIce::Streamer::PLISFlagPlayTimes) >> 4;
	if ( playtimes >=1 && playtimes <=13)
	{
		newItem.itemPlayTimesCount = playtimes;
		ENVLOG(ZQ::common::Log::L_INFO, PLFMT(SsStreamCommitRequest,"insert item, Advertisement playtimes:[%d],flags[%s]"),playtimes,ZQTianShan::Util::convertItemFlagToStr(info.flags).c_str() );
	}
	else {
		newItem.itemPlayTimesCount = -1;
	}
//	items.insert(itWhere , newItem );	
	
		
	int32 ret = SsServiceImpl::doValidateItem( serviceImpl , *this , newItem.setupInfo );	
	
	
	
	if( ret != ERR_RETURN_SUCCESS )
	{
// 		deleteItem( userCtrlNum );
// 		setCurrentItem( curCtrlNum );		
// 		
// 		setNextItem(  isReverseStreaming() ? iterCurrentItem() - 1 : iterCurrentItem() + 1 );

		throwException( ret , "insert");
	}

	addItemToList( newItem , where ) ;

	{
		//set current item
		if ( curCtrlNum == TianShanIce::Streamer::InvalidCtrlNum  )
		{
			setCurrentItem( userCtrlNum );
		}
		else
		{
			setCurrentItem( curCtrlNum );
		}

		setNextItem(  isReverseStreaming() ? iterCurrentItem() - 1 : iterCurrentItem() + 1 );
		if (iterValid(iterNextItem()))
			ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(insert,"curritem[%s],nextitem[%s]"),iterCurrentItem()->setupInfo.contentName.c_str(), iterNextItem()->setupInfo.contentName.c_str());
		else
			ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(insert,"curritem[%s],nextitem[NULL]"),iterCurrentItem()->setupInfo.contentName.c_str());
	}

	if ( playlistState == TianShanIce::Streamer::stsStreaming )
	{
		primeNextItem();
	}
	ENVLOG(ZQ::common::Log::L_INFO, PLFMT(SsStreamCommitRequest,"insert item, cost[%ld/0]"),
		sw.stop());
	return userCtrlNum;
}

bool SsStreamImpl::getInfo(::Ice::Int mask, ::TianShanIce::ValueMap& outValue,
						   const ::Ice::Current& ) 
{
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(SsStreamImpl,"getInfo() mask[%x]"),mask);
	Lock sync(*this);
	switch( mask )
	{
	case TianShanIce::Streamer::infoDVBCRESOURCE:
		{
			ZQTianShan::_IceThrow<TianShanIce::NotSupported>(ENVLOG,LOGCATAGORY,0,
				PLFMT(getInfo,"mask infoDVBCRESOURCE is not supported now"));
		}
		break;
	case TianShanIce::Streamer::infoPLAYPOSITION:
		{
			
			switch ( env->getConfig().iSupportPlaylist)
			{
			case LIB_SUPPORT_NORMAL_PLAYLIST:
				{
					bool bGetContentDuraion = false;
					bool bRunning = isItemStreaming( iterCurrentItem());
					iter itTo = bRunning ? iterCurrentItem() : iterFirstItem();
					std::string streamId = getStreamSessId(itTo);
					StreamParams	paras;
					if(bRunning)
					{
						paras.mask	=	MASK_TIMEOFFSET | MASK_SCALE | MASK_STATE;
					}
					else
					{
						bGetContentDuraion = true;
						paras.mask |= MASK_CONTENT_DURATION | MASK_CONTENT_STATE;
					}
					
					if( !contentIsInService(itTo) )
					{
						paras.mask |= MASK_CONTENT_DURATION | MASK_CONTENT_STATE;
						bGetContentDuraion = true;
					}
					int32 ret = SsServiceImpl::doGetStreamAttr(serviceImpl , *this , streamId , itTo->setupInfo ,  paras );
					if( ret != ERR_RETURN_SUCCESS)
					{
						if(!getItemAttribute( itTo ) )
						{
							ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(getInfo,"can't get current item's play duration, set it to 0"));
							paras.duration = 0;
						}
						else
						{
							paras.duration = itTo->itemRealDuration;
						}
					}
					else if( bGetContentDuraion )
					{
						if( paras.mask & MASK_CONTENT_STATE )
						{
							const std::string contentStateKey = "StreamService.ContentInService";
							ZQTianShan::Util::updateValueMapData( itTo->setupInfo.privateData , 
								contentStateKey,
								paras.contentState == TianShanIce::Storage::csInService ? 1 : 0 );
						}

						if( !( paras.mask & MASK_CONTENT_DURATION ))
						{
							try
							{
								if(!getItemAttribute( itTo ) )
								{
									ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(getInfo,"can't get current item's play duration, set it to 0"));
									paras.duration = 0;
								}
							}
							catch( const TianShanIce::BaseException& )
							{
								ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(getInfo,"can't get current item's play duration, set it to 0"));
								paras.duration = 0;
							}
						}						
						else
						{
							itTo->itemTotalDuration	=	paras.duration;
							if( itTo->setupInfo.outTimeOffset != 0 )
							{
								itTo->itemRealDuration	=	( itTo->itemTotalDuration > itTo->setupInfo.outTimeOffset ? itTo->setupInfo.outTimeOffset : itTo->itemTotalDuration )
																		- itTo->setupInfo.inTimeOffset ;						
							}
							else
							{
								itTo->itemRealDuration	=	itTo->itemTotalDuration - itTo->setupInfo.inTimeOffset;
							}
							itTo->itemInTimeOffset		=	itTo->setupInfo.inTimeOffset;
							itTo->itemOutTimeOffset		=	itTo->itemInTimeOffset + itTo->itemRealDuration;
							
							paras.duration = itTo->itemRealDuration;

						}
					}
					
					size_t itemIdx = itTo - iterBeginItem();
					ZQTianShan::Util::updateValueMapData( outValue , "index" , (Ice::Int)itemIdx);
					Ice::Int itemNpt = static_cast<Ice::Int>(paras.timeoffset - itTo->itemInTimeOffset );
					itemNpt = itemNpt < 0 ? 0 : itemNpt;
					ZQTianShan::Util::updateValueMapData( outValue , "itemOffset" , itemNpt );
					ZQTianShan::Util::updateValueMapData( outValue , "ctrlnumber" , itTo->userCtrlNum );
					ZQTianShan::Util::updateValueMapData( outValue , "playposition" , itemNpt );
					ZQTianShan::Util::updateValueMapData( outValue , "totalplaytime" , (Ice::Int)paras.duration );
					std::ostringstream ossScale;
					ossScale<<paras.scale;
					ZQTianShan::Util::updateValueMapData( outValue , "scale" , ossScale.str() );

				}
				break;
			default:
				{
					std::string streamId = getStreamSessId(iterEndItem());
					StreamParams paras;
					paras.mask = MASK_CONTENT_DURATION | MASK_SCALE | MASK_STATE | MASK_TIMEOFFSET ;
					TianShanIce::Streamer::PlaylistItemSetupInfo dummyInfo;
					int32 ret = SsServiceImpl::doGetStreamAttr( serviceImpl , *this , streamId , dummyInfo ,  paras );
					if( ret != ERR_RETURN_SUCCESS )
					{
						throwException(ret,"SsServiceImpl::doGetStreamAttr");
					}
					else if( !(paras.mask & MASK_CONTENT_DURATION) )
					{
						bool bOk = true;
						paras.duration = 0;
						iter it = iterFirstItem();
						for ( ; it!= iterEndItem() ; it ++ )
						{
							if(!getItemAttribute(it))
							{
								bOk = false;
							}
							else
							{
								paras.duration += it->itemRealDuration;
							}
						}
						if(!bOk)
							paras.duration = 0;
					}
					ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(getInfo,"got timeoffset[%ld] scale[%f] from portal layer"),
						paras.timeoffset , paras.scale );
					Ice::Short from = 1;
					Ice::Long  timeOffset = paras.timeoffset;
					iter itTo = convertPositionToItemWide(timeOffset , from ,(speed > 0.1f) );
					paras.timeoffset = timeOffset;
					size_t itemIdx = itTo - iterBeginItem();
					ZQTianShan::Util::updateValueMapData( outValue , "index" , (Ice::Int)itemIdx);
					Ice::Int itemNpt = static_cast<Ice::Int>(paras.timeoffset - itTo->itemInTimeOffset );
					itemNpt = itemNpt < 0 ? 0 : itemNpt;
					ZQTianShan::Util::updateValueMapData( outValue , "itemOffset" , itemNpt );
					ZQTianShan::Util::updateValueMapData( outValue , "ctrlnumber" , itTo->userCtrlNum );
					ZQTianShan::Util::updateValueMapData( outValue , "playposition" , itemNpt );
					ZQTianShan::Util::updateValueMapData( outValue , "totalplaytime" , (Ice::Int)paras.duration );
					std::ostringstream ossScale;
					ossScale<<paras.scale;
					ZQTianShan::Util::updateValueMapData( outValue , "scale" , ossScale.str() );
					ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(getInfo,"return timeoffset[%ld] scale[%f] to client"), itemNpt , paras.scale );

				}
				break;
			}

		}
		break;
	case TianShanIce::Streamer::infoSTREAMNPTPOS:
		{
			switch ( env->getConfig().iSupportPlaylist)
			{
			case LIB_SUPPORT_NORMAL_PLAYLIST:
				{
					bool bGetContentDuraion = false;
					bool bRunning = isItemStreaming( iterCurrentItem());
					iter itTo = bRunning ? iterCurrentItem() : iterFirstItem();					
					std::string streamId = getStreamSessId(itTo);
					StreamParams	paras;
					if(bRunning)
					{
						paras.mask	=	MASK_TIMEOFFSET | MASK_SCALE | MASK_STATE;
					}
					else
					{
						bGetContentDuraion = true;
						paras.mask |= MASK_CONTENT_DURATION | MASK_CONTENT_STATE;
					}					
					if( !contentIsInService(itTo) )
					{
						paras.mask |= MASK_CONTENT_DURATION | MASK_CONTENT_STATE;
						bGetContentDuraion = true;
					}
					int32 ret = SsServiceImpl::doGetStreamAttr(serviceImpl , *this , streamId , itTo->setupInfo ,  paras );
					if( ret != ERR_RETURN_SUCCESS)
					{
#pragma message(__MSGLOC__"TODO: should throw exception here ?")
						throwException(ret , "SsServiceImpl::doGetStreamAttr");
					}
					else if( bGetContentDuraion )
					{
						if( paras.mask & MASK_CONTENT_STATE )
						{
							const std::string contentStateKey = "StreamService.ContentInService";
							ZQTianShan::Util::updateValueMapData( itTo->setupInfo.privateData , 
								contentStateKey,
								paras.contentState == TianShanIce::Storage::csInService ? 1 : 0 );
						}
						if( !( paras.mask & MASK_CONTENT_DURATION ))
						{
							try
							{
								if(!getItemAttribute( itTo ) )
								{
									ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(getInfo,"can't get current item's play duration, set it to 0"));
									paras.duration = 0;
								}
							}
							catch( const TianShanIce::BaseException& )
							{
								ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(getInfo,"can't get current item's play duration, set it to 0"));
								paras.duration = 0;
							}
						}						
					}

					{
						paras.timeoffset -= itTo->itemInTimeOffset;
						paras.timeoffset = paras.timeoffset < 0 ? 0 : paras.timeoffset;
						bool bOk = true;
						paras.duration = 0;
						iter it = iterFirstItem();
						for ( ; it!= iterEndItem() ; it ++ )
						{
							if(!getItemAttribute(it))
							{
								bOk = false;
							}
							else
							{
								if( it < itTo )
								{
									paras.timeoffset += it->itemRealDuration;
								}
								paras.duration += it->itemRealDuration;
							}
						}
						if(!bOk)
							paras.duration = 0;
					}
					
					size_t itemIdx = itTo - iterBeginItem();
					ZQTianShan::Util::updateValueMapData( outValue , "index" , (Ice::Int)itemIdx);
					Ice::Int itemNpt = static_cast<Ice::Int>(paras.timeoffset);
					itemNpt = itemNpt < 0 ? 0 : itemNpt;
					ZQTianShan::Util::updateValueMapData( outValue , "itemOffset" , itemNpt );					
					ZQTianShan::Util::updateValueMapData( outValue , "playposition" , itemNpt );
					ZQTianShan::Util::updateValueMapData( outValue , "totalplaytime" , (Ice::Int)paras.duration );
					std::ostringstream ossScale;
					ossScale<<paras.scale;
					ZQTianShan::Util::updateValueMapData( outValue , "scale" , ossScale.str() );
					ZQTianShan::Util::updateValueMapData( outValue , "playlistid" , ident.name );

				}
				break;
			default:
				{
					std::string streamId = getStreamSessId(iterEndItem());
					StreamParams paras;
					paras.mask = MASK_CONTENT_DURATION | MASK_SCALE | MASK_STATE | MASK_TIMEOFFSET ;
					TianShanIce::Streamer::PlaylistItemSetupInfo dummyInfo;
					int32 ret = SsServiceImpl::doGetStreamAttr( serviceImpl , *this , streamId , dummyInfo ,  paras );
					if( ret != ERR_RETURN_SUCCESS )
					{
						throwException(ret,"SsServiceImpl::doGetStreamAttr");
					}
					else if( !(paras.mask & MASK_CONTENT_DURATION) )
					{
						paras.duration = 0;
					}
					ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(getInfo,"SsServiceImpl::doGetStreamAttr() got timeoffset[%ld] scale[%f] from portal layer"),
						paras.timeoffset , paras.scale);
					Ice::Short from = 1;
					Ice::Long  timeOffset = paras.timeoffset;					
					//iter itTo = convertPositionToItemWide(timeOffset , from , (speed > 0.1f ));//only used to get the itemIdx
					//paras.timeoffset = timeOffset;
					//size_t itemIdx = itTo - iterBeginItem();
					size_t itemIdx = 0; //do not calculate which item is being streamed
					ZQTianShan::Util::updateValueMapData( outValue , "index" , (Ice::Int)itemIdx);
					Ice::Int itemNpt = static_cast<Ice::Int>(paras.timeoffset) ;
					itemNpt = itemNpt < 0 ? 0 : itemNpt;
					ZQTianShan::Util::updateValueMapData( outValue , "itemOffset" , itemNpt );					
					ZQTianShan::Util::updateValueMapData( outValue , "playposition" , itemNpt );
					ZQTianShan::Util::updateValueMapData( outValue , "totalplaytime" , (Ice::Int)paras.duration );
					std::ostringstream ossScale;
					ossScale<<paras.scale;
					ZQTianShan::Util::updateValueMapData( outValue , "scale" , ossScale.str() );
					ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(getInfo,"SsServiceImpl::doGetStreamAttr() return timeoffset[%ld] scale[%f] to client"),
						paras.timeoffset , paras.scale);

				}
				break;
			}

		}
		
		break;
	case TianShanIce::Streamer::infoSTREAMSOURCE:
		{
			std::string		sourceIp;
			Ice::Int		sourcePort = 0;
			ZQTianShan::Util::getPropertyDataWithDefault( property , STREAMINGRESOURCE_STREAMING_SOUCREIP , "" , sourceIp );
			ZQTianShan::Util::getPropertyDataWithDefault( property , STREAMINGRESOURCE_STREAMING_SOURCEPORT , 0 , sourcePort );
			
			ZQTianShan::Util::updateValueMapData( outValue , "StreamingSourceIp" , sourceIp );
			ZQTianShan::Util::updateValueMapData( outValue , "StreamingSourcePort" , sourcePort );

		}
		break;
	default:
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(ENVLOG,LOGCATAGORY,0,PLFMT(getInfo,"invalid mask"));
		}
		break;
	}
	return true;
}

void SsStreamImpl::onSessionStateChanged(::TianShanIce::Streamer::StreamState state, 
										  const std::string& sessionId, 
										 ::Ice::Long opTime, 
										 const TianShanIce::Properties& props,
										 const ::Ice::Current&) 
{
	Lock sync(*this);
	lastOperationTime = opTime;
	
	if( (env->getConfig().iSupportPlaylist != LIB_SUPPORT_NORMAL_PLAYLIST) && env->getConfig().iPassSGStateChangeEvent )
	{
		fireStateChangedMsg( playlistState , state ,props );
	}
	else if( playlistState != state  )
	{
		fireStateChangedMsg( playlistState , state ,props );
		ENVLOG(ZQ::common::Log::L_INFO,	PLFMT(onSessionStateChanged,"session state changed from [%s] to [%s]"),
			ZQTianShan::Util::dumpTianShanStreamState( playlistState ),
			ZQTianShan::Util::dumpTianShanStreamState( state ) );
		primeNextItem();
	}
	playlistState = state;	
}

void SsStreamImpl::onSessionSpeedChanged(::Ice::Float newSpeed,
										  const std::string& sessionId, 
										 ::Ice::Long opTime, 
										 const TianShanIce::Properties& props,
										 const ::Ice::Current& ) 
{
	Lock sync(*this);

	//WHAT IS THIS
// 	lastOperationTime = opTime;
// 	if( opTime < lastOperationTime )
// 	{
// 		return;
// 	}

	if( (env->getConfig().iSupportPlaylist != LIB_SUPPORT_NORMAL_PLAYLIST) && env->getConfig().iPassSGScaleChangeEvent  )
	{
		fireScaleChangedMsg( speed , newSpeed ,props );
	}
	else if( fabs( speed - newSpeed ) > 0.01f )
	{
		ENVLOG(ZQ::common::Log::L_INFO,
			PLFMT(onSessionSpeedChanged,"Speed changed from [%f] to [%f]"),
			speed , newSpeed);
		fireScaleChangedMsg( speed , newSpeed ,props );
		primeNextItem();
	}	
	speed = newSpeed;	
}

void SsStreamImpl::onPlaylistDone( const TianShanIce::Properties& props )
{
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(onPlaylistDone,"playlist is done"));	
	if ( env->getConfig().iEnableSGStateChangeEvent >= 1 ) {
		fireStateChangedMsg( playlistState , TianShanIce::Streamer::stsStop ,props );
	}
	firePlaylistDoneMsg( props );
	playlistState = TianShanIce::Streamer::stsStop;
	//update timer
	setTimer( env->getConfig().iPlaylistTimeout , TianShanIce::Streamer::TIMERDPC);

	//reposition file pointer so we can play the stream again
	setCurrentItem( iterFirstItem() );
	if( itemCount() >= 1 )
		setNextItem( iterFirstItem() + 1 );
	else
		setNextItem( iterEndItem() );
	speed = 1.0f;
	
	bFirstPlay = true;
#pragma message(__MSGLOC__"TODO: not implement yet")
}

void SsStreamImpl::onSessionProgress(const ::std::string& sessId, ::Ice::Long curTimeOff, ::Ice::Long totalDuration, 
									 const TianShanIce::Properties& props, const ::Ice::Current&) 
{
	if( !iterValid( iterCurrentItem() ) )
		return;
	if( getStreamSessId( iterCurrentItem()) == sessId )
	{
		fireProgressMsg( iterCurrentItem() , curTimeOff , totalDuration ,props );
	}
}

void SsStreamImpl::onSessionExpired( const std::string& sessionId,
									::Ice::Long opTime,
									const TianShanIce::Properties& props,
									const ::Ice::Current& ) 
{
	serviceImpl.unregisterStreamId( sessionId );
	Lock sync(*this);
	lastOperationTime = opTime;

	//log session expired
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(onSessionExpired,"session [%s] is expired") ,  sessionId.c_str() );
	
	switch ( env->getConfig().iSupportPlaylist )
	{
	case LIB_SUPPORT_NORMAL_PLAYLIST:
		{
			if( !iterValid(iterCurrentItem()) )
			{
				ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(onSessionExpired,"current session is not valid"));
				return ;
			}
			if ( iterCurrentItem()->sessionId != sessionId )
			{
				ENVLOG(ZQ::common::Log::L_INFO,PLFMT(onSessionExpired , "session[%s] is not current running session item [%s],filename[%s]"),
								sessionId.c_str() ,
								iterCurrentItem()->sessionId.c_str(),
								iterCurrentItem()->setupInfo.contentName.c_str());
				return;
			}	
			//set current item 's state to Stop			
			iterCurrentItem()->sessionState = TianShanIce::Streamer::stsStop;
			iterCurrentItem()->sessionId	= "";

			//move the cursor to next
			iter tmpCurIt = iterCurrentItem(), tmpNextIt = iterNextItem();
			if (isReverseStreaming())
			{
				tmpCurIt = (tmpNextIt < tmpCurIt && tmpNextIt > iterBeginItem())? tmpNextIt : (tmpCurIt-1);
				tmpNextIt = tmpCurIt > iterBeginItem() ? (tmpCurIt-1) : iterBeginItem();
			}
			else
			{
				tmpCurIt = (tmpNextIt > tmpCurIt && tmpNextIt < iterEndItem())? tmpNextIt : (tmpCurIt+1);
				tmpNextIt = (tmpCurIt < iterEndItem())? (tmpCurIt+1) : iterEndItem();
			}



			if ( !iterValid(iterNextItem()) )
			{
				fireItemSteppedMsg( iterCurrentItem() ,iterNextItem() );
				onPlaylistDone( props );				
			}
			else
			{
				if( isItemStreaming(tmpCurIt) )
				{
					ENVLOG(ZQ::common::Log::L_INFO,PLFMT(onSessionExpired,"CurIt[%s],NextIt[%s],speed[%f]"),tmpCurIt->setupInfo.contentName.c_str(),tmpNextIt->setupInfo.contentName.c_str(),speed);
					iter itTmp = tmpCurIt;
					if ( isFFScale(speed) && !checkRestriction(tmpCurIt, TianShanIce::Streamer::PLISFlagNoFF) )
					{
						ENVLOG(ZQ::common::Log::L_INFO,PLFMT(onSessionExpired,"change scale to[1.0] due to restriction[%s]"),ZQTianShan::Util::convertItemFlagToStr(tmpCurIt->setupInfo.flags).c_str());
						StreamParams paras;
						doChangeScale(tmpCurIt,1.0,paras);
					}
					bool bRewRestriction = false;
					while ( iterValid(tmpCurIt) && (isREWScale(speed)) && !checkRestriction(tmpCurIt,TianShanIce::Streamer::PLISFlagNoRew))
					{
						ENVLOG(ZQ::common::Log::L_INFO,PLFMT(onSessionExpired,"skip to next item due to restriction[%s]"),ZQTianShan::Util::convertItemFlagToStr(tmpCurIt->setupInfo.flags).c_str());
					    tmpCurIt = (tmpNextIt < tmpCurIt && tmpNextIt > iterBeginItem()) ? tmpNextIt : (tmpCurIt-1);	
						tmpNextIt = tmpCurIt > iterBeginItem() ? (tmpCurIt-1) : iterBeginItem();
						bRewRestriction = true;
					}
					if (bRewRestriction)
					{
						doDestroySession(itTmp);
						if (iterValid(tmpCurIt))
						{
							::Ice::Current cur;
							seekStream(0,SEEK_FROM_END,cur);
						}
					}
					//fireItemSteppedMsg( iterCurrentItem() ,iterNextItem() );
					fireItemSteppedMsg( iterCurrentItem() ,tmpCurIt );
					//setCurrentItem( iterNextItem() );					
					setCurrentItem( tmpCurIt );					
				
				//	setNextItem( isReverseStreaming() ? (iterCurrentItem() -1) : (iterCurrentItem() + 1) );
					setNextItem( isReverseStreaming() ? (tmpCurIt-1) : (tmpCurIt+1));
					if (iterValid(iterNextItem()))
						ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(onSessionExpired,"curritem[%s],nextitem[%s]"),iterCurrentItem()->setupInfo.contentName.c_str(), iterNextItem()->setupInfo.contentName.c_str());
					else
						ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(onSessionExpired,"curritem[%s],nextitem[NULL]"),iterCurrentItem()->setupInfo.contentName.c_str());
					//need refresh the timer
					primeNextItem();
				}
				else
				{
					TianShanIce::Streamer::StreamInfo dummyInfo;
					iter itTmp = tmpCurIt;
					//current item is not loaded yet 
					ENVLOG(ZQ::common::Log::L_INFO,PLFMT(onSessionExpired,"CurIt[%s],NextIt[%s],speed[%f]"),tmpCurIt->setupInfo.contentName.c_str(),tmpNextIt->setupInfo.contentName.c_str(),speed);
					while ( iterValid(tmpCurIt) && (!isItemStreaming(tmpCurIt)))
					{
						if (isFFScale(speed) && !checkRestriction(tmpCurIt,TianShanIce::Streamer::PLISFlagNoFF))
						{
							speed = 1.0f;
							ENVLOG(ZQ::common::Log::L_INFO,PLFMT(onSessionExpired,"adjust speed[1.0] due to restriction[%s]"),ZQTianShan::Util::convertItemFlagToStr(tmpCurIt->setupInfo.flags).c_str());
						}
						bool bRewRestriction = false;
						while ( iterValid(tmpCurIt) && (isREWScale(speed) && (!checkRestriction(tmpCurIt, TianShanIce::Streamer::PLISFlagNoRew) || !checkRestriction(tmpCurIt, TianShanIce::Streamer::PLISFlagSkipAtRew)))) 
						{
							ENVLOG(ZQ::common::Log::L_INFO,PLFMT(onSessionExpired,"skip to next item due to REW restriction[%s]"),ZQTianShan::Util::convertItemFlagToStr(tmpCurIt->setupInfo.flags).c_str());
							tmpCurIt = ( tmpNextIt < tmpCurIt && tmpNextIt > iterBeginItem()) ? tmpNextIt : (tmpCurIt-1);
							tmpNextIt = tmpCurIt > iterBeginItem() ? (tmpCurIt-1) : iterBeginItem();
							bRewRestriction = true;
						}
						while ( iterValid(tmpCurIt) && ( isFFScale(speed) && !checkRestriction(tmpCurIt,TianShanIce::Streamer::PLISFlagSkipAtFF)))
						{
							ENVLOG(ZQ::common::Log::L_INFO,PLFMT(onSessionExpired,"skip to next item due to FF mode restriction[%s]"),ZQTianShan::Util::convertItemFlagToStr(tmpCurIt->setupInfo.flags).c_str());
							tmpCurIt = (tmpNextIt > tmpCurIt && tmpNextIt < iterEndItem()? tmpNextIt : (tmpCurIt+1));
							tmpNextIt= tmpCurIt < iterEndItem() ? (tmpCurIt+1) : iterEndItem();
							bRewRestriction = true;
						}
						if (bRewRestriction)
						{
							doDestroySession(itTmp);
						}
						if (!iterValid(tmpCurIt))
						{
							break;  //???????
						}

						try
						{
							StreamParams	paras;
							std::string		streamId;						
							iter itTo = doLoadItem( tmpCurIt , 0, 0.0f , paras , streamId );
							fireItemSteppedMsg( iterCurrentItem() ,itTo );

							updateStreamSessId(itTo,streamId);
							itTo->sessionState	= TianShanIce::Streamer::stsStreaming;
							setCurrentItem(itTo);
							setNextItem( isReverseStreaming() ? itTo -1 : itTo + 1 );
							if (iterValid(iterNextItem()))
								ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(onSessionExpired,"fireItemSteppedMsg curritem[%s],nextitem[%s]"),iterCurrentItem()->setupInfo.contentName.c_str(), iterNextItem()->setupInfo.contentName.c_str());
							else
								ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(onSessionExpired,"fireItemSteppedMsg curritem[%s],nextitem[NULL]"),iterCurrentItem()->setupInfo.contentName.c_str());
							primeNextItem();
							if (iterValid(itTo))
							{
								break;
							}
						}
						catch( TianShanIce::BaseException& )
						{
							ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(onSessionExpired,"failed to load next item "));
							onPlaylistDone( );
						}
						if ( isReverseStreaming())
						{
							tmpCurIt = (tmpNextIt< tmpCurIt && tmpNextIt > iterBeginItem()) ? tmpNextIt:(tmpCurIt-1);
							tmpNextIt = tmpCurIt> iterBeginItem() ? (tmpCurIt-1):iterBeginItem();
						}
						else
						{
							tmpCurIt = (tmpNextIt > tmpCurIt && tmpNextIt < iterEndItem())? tmpNextIt :(tmpCurIt+1);
							tmpNextIt = (tmpCurIt < iterEndItem())? (tmpCurIt+1):iterEndItem();
						}

					}
				}				
			}
		}
		break;
	case LIB_SUPPORT_AUTO_PLAYLIST:
		{
			//just step the list
			stepList( iterCurrentItem() , iterNextItem() );
		}
		break;
	case LIB_SUPPORT_NORMAL_STREAM:
		{//Take this as playlist done
			onPlaylistDone(props);
		}
		break;
	default:
		{
			
		}
		break;
	}
}

void SsStreamImpl::onTimerDPC( )
{

#if defined _DEBUG || defined DEBUG
	Ice::Time now = IceUtil::Time::now()
	Ice::Long diff = (now - mNextWakeup).toMilliSeconds();
	ENVLOG(ZQ::common::Log::L_INFO,	PLFMT(onTimerDPC,"target time [%ld] current[%ld] diff[%ld]"), mNextWakeup.toMilliSeconds(), now.toMilliSeconds(), diff );
#endif

	switch ( playlistState )
	{
	case TianShanIce::Streamer::stsSetup:
		{
			if ( env->getConfig().iSupportPlaylist != LIB_SUPPORT_NORMAL_PLAYLIST )
			{//do nothing if not in NORMAL PLAYLIST mode but update timer again
				//updateTimer();
				return;
			}
			ENVLOG(ZQ::common::Log::L_INFO, PLFMT(onTimerDPC,"current state[%s] , destroy current playlist"),
				ZQTianShan::Util::dumpTianShanStreamState(playlistState) );
			Ice::Current c;
			c.ctx["caller"] = "timeout on setup stage";
			destroy(c);
		}
		break;
	case TianShanIce::Streamer::stsStreaming:
		{
			if ( env->getConfig().iSupportPlaylist != LIB_SUPPORT_NORMAL_PLAYLIST )
			{//do nothing if not in NORMAL PLAYLIST mode but update timer again
				//updateTimer();
				return;
			}
			ENVLOG(ZQ::common::Log::L_INFO, PLFMT(onTimerDPC,"current state[%s] , prime next item"),
				ZQTianShan::Util::dumpTianShanStreamState(playlistState) );
			time_t t = 0;
			iter it = findNextCriticalStartItem(t);
			if( iterValid(it) && t <= 0 )
			{
				ENVLOG(ZQ::common::Log::L_INFO,PLFMT(onTimerDPC,"skip to item[%s] [%d]"),
					it->setupInfo.contentName.c_str(),
					it->userCtrlNum);
				skipToItem( it->userCtrlNum , true );
			}
			if( bEnableEOT &&  iterValid(iterCurrentItem()) && (iterCurrentItem() == iterLastItem() ) && (!isReverseStreaming() ) )
			{
				Ice::Long lLeft = getSessTimeLeft();
				if( lLeft <= getEOTSize() )
				{
					StreamParams para;
					ENVLOG(ZQ::common::Log::L_INFO,PLFMT(onTimerDPC,"last item and in EOT area , change speed to 1.0"));
					doChangeScale( iterCurrentItem() , 1.0 , para );
				}
			}
			primeNextItem();
		}
		break;
	case TianShanIce::Streamer::stsPause:
		{
			if ( env->getConfig().iSupportPlaylist != LIB_SUPPORT_NORMAL_PLAYLIST )
			{//do nothing if not in NORMAL PLAYLIST mode but update timer again
				updateTimer();
				return;
			}
			if( env->getConfig().iExpireOnPause >= 1 )
			{
				ENVLOG(ZQ::common::Log::L_INFO, PLFMT(onTimerDPC,"current state[%s] , and ExpiredOnPause[%d] destroy current playlist"),
												ZQTianShan::Util::dumpTianShanStreamState(playlistState) ,
												env->getConfig().iExpireOnPause );
				
				Ice::Current c;
				c.ctx["caller"] = "timeout on pause stage";
				destroy(c);
			}
			else
			{
				ENVLOG(ZQ::common::Log::L_INFO, PLFMT(onTimerDPC,"current state[%s] , and ExpiredOnPause[%d] , update timer[%d]"),
												ZQTianShan::Util::dumpTianShanStreamState(playlistState) ,
												env->getConfig().iExpireOnPause ,
												env->getConfig().iPlaylistTimeout );
				updateTimer();
			}			
		}
		break;
	case TianShanIce::Streamer::stsStop:
		{
			ENVLOG(ZQ::common::Log::L_INFO, PLFMT(onTimerDPC,"current state[%s] , destroy current playlist"),
				ZQTianShan::Util::dumpTianShanStreamState(playlistState) );
			Ice::Current c;
			c.ctx["caller"] = "timeout on stop stage";
			destroy(c);
		}
		break;
	default:
		{
			ENVLOG(ZQ::common::Log::L_CRIT,PLFMT(onTimerDPC,"Invalid playliState[%d]") , playlistState );
		}
		break;
	}	
}

void SsStreamImpl::onTimer(::TianShanIce::Streamer::Timertype type, const ::Ice::Current& ) 
{
	Lock sync(*this);
	switch ( type )
	{
	case TianShanIce::Streamer::TIMERNULL:
		{
			ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(onTimer,"invalid timer type"));
		}
		break;	
	case TianShanIce::Streamer::TIMERPLAYLISTDESTROY:
		{

		}
		break;
	case TianShanIce::Streamer::TIMERRENEWTICKET:
		{

		}
		break;
	case TianShanIce::Streamer::TIMERDPC:
		{
			onTimerDPC( );
		}
		break;
	default:
		{
			ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(onTimer,"invalid timer type"));
		}
		break;
	}
}

bool SsStreamImpl::stepList(  iter itFrom , iter itTo )
{
	Ice::Int		fromCtrlNum = iterValid(itFrom) ? itFrom->userCtrlNum : TianShanIce::Streamer::InvalidCtrlNum;
	std::string		fromItemName= iterValid(itFrom)	? itFrom->setupInfo.contentName : "";

	Ice::Int		toCtrlNum	= iterValid( itTo ) ? itTo->userCtrlNum : TianShanIce::Streamer::InvalidCtrlNum;
	std::string		toItemName	= iterValid( itTo ) ? itTo->setupInfo.contentName : "";
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(stepList,"setp from [%d][%s] to [%d][%s]"),
			fromCtrlNum , fromItemName.c_str(),
			toCtrlNum , toItemName.c_str() );
	if ( iterValid(itTo) )
	{				
		
		//after invoked onPlaylistDone
		//reset all playlist information		
		if( itemCount() > 0 )
		{
			currentItemCtrlNum	=	iterFirstItem()->userCtrlNum;
			currentItemCtrlNum	=	currentItemCtrlNum;
			currentIter			=	findItemWithUserCtrlNum(currentItemCtrlNum);
			bCurIterInited		=	true;
			nextIter			=	currentIter + ( isReverseStreaming() ? -1 : 1 );
			bNextIterInited		=	true;			
		}
		playlistState		=	TianShanIce::Streamer::stsStop;
#pragma message(__MSGLOC__"TODO: update timer so I can kill playlist")
	}
	else
	{
		currentItemCtrlNum	=	toCtrlNum;
		currentIter			=	itTo;
		bCurIterInited		=	true;
		nextIter			=	currentIter + ( isReverseStreaming() ? -1 : 1 );
		bNextIterInited		=	true;
		fireItemSteppedMsg( itFrom , itTo );
#pragma message(__MSGLOC__"TODO: update timer so I can load the next item")
	}

	return true;
}

bool SsStreamImpl::deleteItem( Ice::Int userCtrlNum )
{
	iter it = findItemWithUserCtrlNum(userCtrlNum);
	if( iterValid(it) )
	{
		Ice::Int curCtrlNum = currentItemCtrlNum;
		if( currentItemCtrlNum == userCtrlNum )
		{
			curCtrlNum = iterNextItem()->userCtrlNum;			
		}
		items.erase(it);
		currentIter			= findItemWithUserCtrlNum(curCtrlNum);
		bCurIterInited		= true;
		nextIter			= currentIter + ( isReverseStreaming() ? -1 : 1 );
		bNextIterInited		= true;
		assert(iterValid(currentIter));
	}
	return true;
}

bool SsStreamImpl::addItemToList( const TianShanIce::Streamer::ItemSessionInfo& newItem , Ice::Int where )
{
	
	iter itWhere = findItemWithUserCtrlNum(where);
	if( !iterValid( itWhere ) ) { itWhere = iterEndItem(); }
	items.insert(itWhere,newItem);
	if( itemCount() == 1 )
	{
		currentItemCtrlNum	=	newItem.userCtrlNum;
		assert( iterValid( findItemWithUserCtrlNum(currentItemCtrlNum) ) );
	}	
	{
		currentIter		= findItemWithUserCtrlNum( currentItemCtrlNum );
		bCurIterInited	= true;
		nextIter		= currentIter + ( isReverseStreaming() ? -1 : 1 );
		bNextIterInited = true;
	}

	return true;
}

void SsStreamImpl::clearItemSessionInfo( TianShanIce::Streamer::ItemSessionInfo& info )
{
	info.setupInfo.privateData.clear( );
	info.setupInfo.contentName			= "";
	info.setupInfo.criticalStart		= 0;
	info.setupInfo.flags				= 0;
	info.setupInfo.inTimeOffset			= 0;
	info.setupInfo.outTimeOffset		= 0;
	info.setupInfo.spliceIn				= false;
	info.setupInfo.spliceOut			= 0;
	info.itemInTimeOffset				= 0;
	info.itemOutTimeOffset				= 0;
	info.itemProfiles.clear();
	info.itemRealDuration				= 0;
	info.itemTotalDuration				= 0;
	info.loadedCount					= 0;
	info.muxBitrate						= 0;
	info.privateData.clear();
	info.runningOffset					= 0;
	info.sessFlag						= 0;
	info.sessionId						= "";
	info.sessionState					= TianShanIce::Streamer::stsSetup;
	info.timeLaunched					= 0;
	info.timeLoaded						= 0;
	info.timeUnloaded					= 0;
	info.userCtrlNum					= TianShanIce::Streamer::InvalidCtrlNum;
}
void	SsStreamImpl::updateTimer()
{
	Ice::Long tLeft = env->getConfig().iPlaylistTimeout;
	switch ( playlistState )
	{
	case TianShanIce::Streamer::stsSetup:
		{

		}
		break;
	case TianShanIce::Streamer::stsStreaming:
		{
			tLeft = getSessTimeLeft();
			if( iterValid( iterCurrentItem()) && (iterCurrentItem() == iterLastItem()) )
			{
				if( bEnableEOT && (speed > 1.01f) && (!isReverseStreaming()) && ( getEOTSize() > 0) )
				{
					tLeft -= getEOTSize();
					tLeft = tLeft < 0 ? 0 : tLeft;
					ENVLOG(ZQ::common::Log::L_INFO,PLFMT(getSessTimeLeft,"adjust duration left to [%ld] because EOTSize [%d]"),
						tLeft,getEOTSize() );
				}
			}
			else
			{
				tLeft = tLeft - getPreloadTime() ;
			}
			tLeft = tLeft < 10 ? 10 : tLeft;
		}
		break;
	case TianShanIce::Streamer::stsPause:
		{

		}
		break;
	case TianShanIce::Streamer::stsStop:
		{

		}
		break;
	default:
		{

		}
		break;
	}
	setTimer(tLeft);
}
void	SsStreamImpl::setTimer( Ice::Long t , TianShanIce::Streamer::Timertype type )
{
	if(!bAlive)
	{
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(setTimer,"session is destroyed ,refuse to set timer"));
		return;
	}
	switch( type )
	{
	case TianShanIce::Streamer::TIMERDPC:
		{
			if( playlistState == TianShanIce::Streamer::stsStreaming )
			{//only use critical start time in streamig state
				time_t timeout;
				iter it = findNextCriticalStartItem(timeout);
				if( iterValid(it) && (timeout < t) )
				{
					t = timeout;
				}
			}
#if defined _DEBUG || defined DEBUG
			ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(setTimer,"updeat timer with ident[%s/%s]"),
				ident.category.c_str() , ident.name.c_str());
#endif
			DPCScheduleTask* p = new DPCScheduleTask( ident , env);
			IceUtil::Time expired = IceUtil::Time::milliSeconds(t) + IceUtil::Time::now();			
			env->getSceduler().cancelSchedule( ident.name , TianShanIce::Streamer::TIMERDPC );
			env->getSceduler().scheduleAt( ident.name , TianShanIce::Streamer::TIMERDPC , p , expired );
			//ZQTianShan::Util::updatePropertyData( property,"TARGETTIME", expired.toMilliSeconds());
			mNextWakeup = expired;
			ENVLOG(ZQ::common::Log::L_INFO,PLFMT(setTimer,"update timer [%ld] [%ld]"), t , expired.toMilliSeconds() );
		}
		break;
	default:
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,LOGCATAGORY,0,
				PLFMT(setTimer,"unkown timer type"));
		}
	}
}

bool SsStreamImpl::checkRestriction(constIter itTarget, const long mask)
{
	if ( !iterValid(itTarget) ) return true;
	bool bNoRestriction = true;
	unsigned long flag = itTarget->setupInfo.flags;
	if ( mask & TianShanIce::Streamer::PLISFlagNoPause)
	{
		if (flag & TianShanIce::Streamer::PLISFlagNoPause)
			bNoRestriction = false;
	}
	if ( mask & TianShanIce::Streamer::PLISFlagNoFF)
	{
		if (flag & TianShanIce::Streamer::PLISFlagNoFF)
			bNoRestriction = false;
	}
	if ( mask & TianShanIce::Streamer::PLISFlagNoRew)
	{
		if (flag & TianShanIce::Streamer::PLISFlagNoRew)
			bNoRestriction = false;
	}
	if ( mask & TianShanIce::Streamer::PLISFlagNoSeek )
	{
		if (flag & TianShanIce::Streamer::PLISFlagNoSeek)
			bNoRestriction = false;
	}
	if ( mask & TianShanIce::Streamer::PLISFlagSkipAtFF)
	{
		if (flag & TianShanIce::Streamer::PLISFlagSkipAtFF)
			bNoRestriction = false;
	}
	if ( mask & TianShanIce::Streamer::PLISFlagSkipAtRew)
	{
		if (flag & TianShanIce::Streamer::PLISFlagSkipAtRew)
			bNoRestriction = false;
	}
	return bNoRestriction;
}
}}//namespace ZQ::StreamService

