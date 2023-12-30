

#include <math.h>
#include <TianShanIceHelper.h>
#include "SsStreamImpl.h"
#include "EventSender.h"
#include "SsServiceImpl.h"

#ifdef ZQ_OS_MSWIN
#include "memoryDebug.h"
#endif

#if defined ZQ_OS_MSWIN
	#define	PLFMT(x,y) 	"%s/%08X/SsPlaylist[%16s]\t"##y,ident.name.c_str(),GetCurrentThreadId(),#x	
#else 
	#define	PLFMT(x,y) 	"%s/%08X/SsPlaylist[%16s] "y,ident.name.c_str(),pthread_self(),#x
#endif

namespace ZQ
{
namespace StreamService
{

void SsStreamImpl::fillEventData( EventData& data , Ice::Int seq , Ice::Int type )
{
	data.playlistId				=	ident;
	TianShanIce::Streamer::PlaylistExPrx plPrx = UCKGETOBJECT(TianShanIce::Streamer::PlaylistExPrx,ident);
	data.playlistProxyString	=	env->getCommunicator()->proxyToString(plPrx);
	data.eventSeq				=	seq;
	data.eventType				=	type;
}

void SsStreamImpl::fireItemSteppedMsg( constIter itFrom , constIter itTo , Ice::Int curItemOffset ,const TianShanIce::Properties& props )
{//because every time this method invoked , SsStreamImpl must be locked 
	// so no new lock is needed
	if( itFrom == itTo )
		return;
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(fireItemSteppedMsg, "firing ItemStepped from [%s][%u] to [%s][%d]: %s"),
		iterValid(itFrom) ? itFrom->setupInfo.contentName.c_str() :"",
		iterValid(itFrom) ? itFrom->userCtrlNum : TianShanIce::Streamer::InvalidCtrlNum ,
		iterValid(itTo) ? itTo->setupInfo.contentName.c_str() :"",
		iterValid(itTo) ? itTo->userCtrlNum : TianShanIce::Streamer::InvalidCtrlNum ,
		ZQTianShan::Util::dumpStringMap(props).c_str());

	Ice::Int	itemSteppedSeq = 0 ;
	ZQTianShan::Util::getPropertyDataWithDefault( property , EVENT_ITEMSTEP_EVENT_SEQ , -1 , itemSteppedSeq );
	++itemSteppedSeq;
	ZQTianShan::Util::updatePropertyData(property , EVENT_ITEMSTEP_EVENT_SEQ , itemSteppedSeq );
	
	EventData	data;	

	data.eventProp = props;
	fillEventData( data , itemSteppedSeq , ICE_EVENT_TYPE_ITEMSETPPED);
	
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_EVENT_SEQ , itemSteppedSeq );
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_ITEM_PREV_CTRLNUM , (iterValid(itFrom) ? itFrom->userCtrlNum : TianShanIce::Streamer::InvalidCtrlNum) );
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_ITEM_CUR_CTRLNUM , iterValid(itTo) ? itTo->userCtrlNum : TianShanIce::Streamer::InvalidCtrlNum );
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_PREV_ITEM_NAME , iterValid(itFrom) ? itFrom->setupInfo.contentName : "" );
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_NEXT_ITEM_NAME , iterValid(itTo) ? itTo->setupInfo.contentName : "" );
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_TIME_STAMP_UTC , ZQTianShan::Util::getNowOfUTC() );

	//if( iterValid(itFrom))
	{
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_PREV_ITEM_PID , getPID(itFrom) );
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_PREV_ITEM_PAID , getPAID(itFrom) );
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_PREV_STREAMSOURCE , getLastURL(itFrom));
	}
	//if( iterValid(itTo))
	{
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_CUR_ITEM_PID , getPID( itTo ));
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_CUR_ITEM_PAID , getPAID(itTo) ); 
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_CUR_STREAMSOURCE , getLastURL(itTo));
	}	
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_CLSUTERID , env->getConfig().iClusterId );

	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_ITEMSTEP_ITEM_CUR_TIMEOFFSET , curItemOffset );
	
#pragma message(__MSGLOC__"TODO: how to generate a cluster ID ?")
	serviceImpl.getEventSender().postEvent(data);
}

void SsStreamImpl::firePlaylistDoneMsg( const TianShanIce::Properties& props  ) 
{
	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(firePlaylistDoneMsg,"firing StreamCompleted: %s"),
		ZQTianShan::Util::dumpStringMap(props).c_str());

	Ice::Int	itemStreamCompletedSeq = 0 ;
	ZQTianShan::Util::getPropertyDataWithDefault( property , EVENT_STREAM_COMPLETE_EVENT_SEQ , -1 , itemStreamCompletedSeq );
	++itemStreamCompletedSeq;
	ZQTianShan::Util::updatePropertyData( property , EVENT_STREAM_COMPLETE_EVENT_SEQ , itemStreamCompletedSeq );

	EventData data;	

	data.eventProp = props;
	ZQTianShan::Util::updateValueMapData( data.eventData , EVENT_STREAM_COMPLETE_EVENT_SEQ , itemStreamCompletedSeq );

#pragma message(__MSGLOC__"TODO: get the sender and send out the event")
	if( isReverseStreaming() )
	{		
		fillEventData( data , itemStreamCompletedSeq , ICE_EVENT_TYPE_BEGINOFSTREAM);
	}
	else
	{
		//get total time duration
		Ice::Long lTotal = calculateDuration( iterFirstItem() , iterLastItem() );
		std::ostringstream oss;oss<<lTotal;
		ZQTianShan::Util::updateValueMapData( data.eventData ,EVENT_STREAM_COMPLETE_TIMEOFFSET , oss.str() );
		fillEventData( data , itemStreamCompletedSeq , ICE_EVENT_TYPE_ENDOFSTREAM);		
	}
	serviceImpl.getEventSender().postEvent(data);
}

void SsStreamImpl::fireScaleChangedMsg( const Ice::Float& scaleOld , const Ice::Float& scaleNew ,const TianShanIce::Properties& props  )
{
	if( fabs(scaleNew - scaleOld ) < 0.001)
	{
		if( !(	(env->getConfig().iSupportPlaylist != LIB_SUPPORT_NORMAL_PLAYLIST) && 
				env->getConfig().iPassSGScaleChangeEvent )  )
		return;
	}
	ENVLOG(ZQ::common::Log::L_INFO,	PLFMT(fireScaleChangedMsg,"firing ScaleChanged from [%f]to [%f]: %s"),
		scaleOld , scaleNew , ZQTianShan::Util::dumpStringMap(props).c_str());

	Ice::Int	itemScaleChangedSeq = 0 ;
	ZQTianShan::Util::getPropertyDataWithDefault( property , EVENT_SPEEDCHANGE_EVENT_SEQ , -1 , itemScaleChangedSeq );
	++itemScaleChangedSeq;
	ZQTianShan::Util::updatePropertyData( property , EVENT_SPEEDCHANGE_EVENT_SEQ , itemScaleChangedSeq );

	EventData data;
	
	data.eventProp = props;
	fillEventData(data,itemScaleChangedSeq,ICE_EVENT_TYPE_SPEEDCHANGE);
	
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_SPEEDCHANGE_PREV_SPEED , scaleOld );
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_SPEEDCHANGE_CUR_SPEED , scaleNew );

	serviceImpl.getEventSender().postEvent(data);

}
void SsStreamImpl::fireStateChangedMsg( const TianShanIce::Streamer::StreamState& stateOld , 
								   const TianShanIce::Streamer::StreamState& stateNew,
								   const TianShanIce::Properties& props )
{
	
	if( (stateOld == stateNew ))
	{
		if( !(	(env->getConfig().iSupportPlaylist != LIB_SUPPORT_NORMAL_PLAYLIST) && 
					env->getConfig().iPassSGStateChangeEvent ) )
		return;
	}
	ENVLOG(ZQ::common::Log::L_INFO, PLFMT(fireStateChangedMsg, "firing StateChanged from [%s] to [%s]: %s"),
		ZQTianShan::Util::dumpTianShanStreamState( stateOld ),
		ZQTianShan::Util::dumpTianShanStreamState( stateNew ),
		ZQTianShan::Util::dumpStringMap(props).c_str() );
	Ice::Int	itemStateChangedSeq = 0 ;
	ZQTianShan::Util::getPropertyDataWithDefault( property , EVENT_STATECHANGE_EVENTSEQ , -1 , itemStateChangedSeq );
	++itemStateChangedSeq;
	ZQTianShan::Util::updatePropertyData(property , EVENT_STATECHANGE_EVENTSEQ , itemStateChangedSeq );

	EventData data;

	data.eventProp = props;
	fillEventData(data , itemStateChangedSeq , ICE_EVENT_TYPE_STATECHANGE );
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_STATECHANGE_PREV_STATE , static_cast<Ice::Int>(stateOld));
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_STATECHANGE_CUR_STATE , static_cast<Ice::Int>(stateNew));
	serviceImpl.getEventSender().postEvent(data);
}

void	SsStreamImpl::fireProgressMsg( constIter it , const Ice::Long& curTimeOffset , const Ice::Long& totalDuration ,const TianShanIce::Properties& props )
{
	if( !iterValid(it))
		return;

#ifdef _DEBUG
	ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(fireProgressMsg, "ctrlNum[%d] current[%lld] total[%lld]"), it->userCtrlNum, curTimeOffset , totalDuration );
#endif

	EventData data;

	data.eventProp = props;
	Ice::Int	itemProgressSeq = 0 ;
	ZQTianShan::Util::getPropertyDataWithDefault( property , EVENT_SPEEDCHANGE_EVENT_SEQ , -1 , itemProgressSeq );
	++itemProgressSeq;
	fillEventData(data, itemProgressSeq , ICE_EVENT_TYPE_PROGRESS );
	ZQTianShan::Util::updatePropertyData( property , EVENT_SPEEDCHANGE_EVENT_SEQ , itemProgressSeq );

	ZQTianShan::Util::updateValueMapData( data.eventData , EVENT_PROGRESS_ITEM_EVENT_SEQ , itemProgressSeq );
	ZQTianShan::Util::updateValueMapData( data.eventData , EVENT_PROGRESS_ITEM_NAME , it->setupInfo.contentName );
	ZQTianShan::Util::updateValueMapData( data.eventData , EVENT_PROGRESS_ITEM_CTRL_NUM , it->userCtrlNum  );
	ZQTianShan::Util::updateValueMapData( data.eventData , EVENT_PROGRESS_ITEM_STEP_TOTAL , size() );
	ZQTianShan::Util::updateValueMapData( data.eventData , EVENT_PROGRESS_ITEM_STEP_CURRENT , static_cast<Ice::Int>( it - iterFirstItem() ) );
	ZQTianShan::Util::updateValueMapData( data.eventData , EVENT_PROGRESS_ITEM_PROGRESS_TOTAL , static_cast<Ice::Int>(totalDuration));
	ZQTianShan::Util::updateValueMapData( data.eventData , EVENT_PROGRESS_ITEM_PROGRESS_DONE , static_cast<Ice::Int>(curTimeOffset) );
		
	serviceImpl.getEventSender().postEvent(data);	
}
void	SsStreamImpl::firePlaylistDestroyMsg( constIter it , const Ice::Int& code , const std::string& reason ,const TianShanIce::Properties& props  )
{

	ENVLOG(ZQ::common::Log::L_INFO,PLFMT(firePlaylistDestroyMsg,"firing PlaylistDestroyed: %s"),
		ZQTianShan::Util::dumpStringMap(props).c_str());
	EventData data;
	
	data.eventProp = props;
	fillEventData(data, 0 , ICE_EVENT_TYPE_PLAYLISTDESTROY );
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_PLDESTROY_EXITCODE , code );
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_PLDESTROY_EXIT_REASON , reason );
	ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_PLDESTROY_CLUSTERID , env->getConfig().iClusterId );
	if( iterValid(it) && isItemStreaming(it) )
	{
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_PLDESTROY_EXITONPLAYING , static_cast<Ice::Int>(1) );
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_PLDESTROY_EXIT_PID , getPID(it));
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_PLDESTROY_EXIT_PAID , getPAID(it));
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_PLDESTROY_EXIT_STREAMINGSOURCE , getLastURL(it));
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_PLDESTROY_EVENT_TIMESTAMP , ZQTianShan::Util::getNowOfUTC());
	}
	else
	{
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_PLDESTROY_EXITONPLAYING , static_cast<Ice::Int>(0) );		
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_PLDESTROY_EXIT_PID , std::string(""));
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_PLDESTROY_EXIT_PAID , std::string(""));
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_PLDESTROY_EXIT_STREAMINGSOURCE , std::string(""));
		ZQTianShan::Util::updateValueMapData(data.eventData , EVENT_PLDESTROY_EVENT_TIMESTAMP , ZQTianShan::Util::getNowOfUTC());
	}
#pragma message(__MSGLOC__"TODO: how to generate a cluster ID ?")
	serviceImpl.getEventSender().postEvent(data);	
}


}
}

