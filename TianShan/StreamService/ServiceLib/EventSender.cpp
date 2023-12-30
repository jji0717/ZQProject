
#include "TianShanIceHelper.h"
#include "SsEnvironment.h"
#include "EventSender.h"
#include <TimeUtil.h>


namespace ZQ
{
namespace StreamService
{

IceEventSender::IceEventSender( SsEnvironment* environment )
	:env(environment)
{
	bConnected = false;
}
IceEventSender::~IceEventSender( )
{
	stopSender( );
}
bool IceEventSender::stopSender( )
{
	return true;
}

bool IceEventSender::startSender( const std::string& eventChannelEndPoint )
{
	strEventChannelEndPoint = eventChannelEndPoint;

	if(  !eventChannelEndPoint.empty( )  &&  eventChannelEndPoint.find(":") == std::string::npos )	
	{
		std::string tempEventChannelEndPoint = SERVICE_NAME_TopicManager ;
		strEventChannelEndPoint = tempEventChannelEndPoint + ":" + eventChannelEndPoint;
	}

	ENVLOG( ZQ::common::Log::L_INFO , 
		CLOGFMT( IceEventSender, "start event sender with Event Channel endpoint[%s] "),
		strEventChannelEndPoint.c_str() );
	connectEventChannel( );
	//what ever connect to event channel ok or false , just return OK
	//If we connect to Event Channel failed , just reconnect it in the future
	return true;
}

bool IceEventSender::connectEventChannel( )
{
	try
	{
		if( strEventChannelEndPoint.empty() )
		{
#pragma message(__MSGLOC__"TODO: if event channel endpoint is empty, all event can be discarded")
			bConnected = true;
			return true;
		}
		ENVLOG(ZQ::common::Log::L_DEBUG , CLOGFMT( IceEventSender, "connect to Event Channel : [%s]"), strEventChannelEndPoint.c_str() );
		Ice::CommunicatorPtr Ic = getSsEnv()->getCommunicator();
		if( !Ic )
		{
			ENVLOG(ZQ::common::Log::L_ERROR , CLOGFMT( IceEventSender , "no available Ice communicator , can't connect to Event Channel " ));
			return false;
		}

		topicManagerPrx = IceStorm::TopicManagerPrx::checkedCast( Ic->stringToProxy( strEventChannelEndPoint ) );
		if( !topicManagerPrx )
		{
			ENVLOG(ZQ::common::Log::L_ERROR , CLOGFMT( IceEventSender , "can't get topic manager through Event Channel [%s]" ),
				strEventChannelEndPoint.c_str() );
			return false;
		}

		IceStorm::TopicPrx			playlistTopic;
		IceStorm::TopicPrx			streamTopic;
		IceStorm::TopicPrx			progressTopic;
		IceStorm::TopicPrx			genericTopic;

		//get topic from Event Channel
		try
		{
			playlistTopic = topicManagerPrx->retrieve( TianShanIce::Streamer::TopicOfPlaylist );
		}
		catch( const IceStorm::NoSuchTopic& )
		{
			playlistTopic = topicManagerPrx->create( TianShanIce::Streamer::TopicOfPlaylist );
		}
		//playlistTopic->getPublisher()->ice_oneway( );

		playlistEventPrx = TianShanIce::Streamer::PlaylistEventSinkPrx::uncheckedCast( playlistTopic->getPublisher() );

		try
		{
			streamTopic = topicManagerPrx->retrieve( TianShanIce::Streamer::TopicOfStream );
		}
		catch (const IceStorm::NoSuchTopic& ) 
		{			
			streamTopic = topicManagerPrx->create( TianShanIce::Streamer::TopicOfStream );
		}
		//streamTopic->getPublisher()->ice_oneway();

		streamEventPrx = TianShanIce::Streamer::StreamEventSinkPrx::uncheckedCast( streamTopic->getPublisher() );

		try
		{
			progressTopic = topicManagerPrx->retrieve( TianShanIce::Streamer::TopicOfStreamProgress );
		}
		catch (const IceStorm::NoSuchTopic& ) 
		{		
			progressTopic = topicManagerPrx->create(TianShanIce::Streamer::TopicOfStreamProgress);
		}
		//progressTopic->getPublisher()->ice_oneway();

		progressEventPrx = TianShanIce::Streamer::StreamProgressSinkPrx::uncheckedCast( progressTopic->getPublisher() );		

		try
		{
			genericTopic = topicManagerPrx->retrieve( TianShanIce::Events::TopicOfGenericEvent );
		}
		catch (const IceStorm::NoSuchTopic& ) 
		{		
			progressTopic = topicManagerPrx->create( TianShanIce::Events::TopicOfGenericEvent );
		}
		//genericTopic->getPublisher()->ice_oneway();

		genericEventPrx	= TianShanIce::Events::GenericEventSinkPrx::uncheckedCast( genericTopic->getPublisher());


		ENVLOG(ZQ::common::Log::L_INFO , CLOGFMT( IceEventSender ,"connect to Event Channel OK :[%s] " ), strEventChannelEndPoint.c_str()  );

		bConnected = true;
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR , 
			CLOGFMT( IceEventSender , "caught ice exception [%s] when connect to Event Channel [%s]"),
			ex.ice_name().c_str() ,
			strEventChannelEndPoint.c_str() ) ;
		return false;
	}
	return true;
}

bool IceEventSender::sendEvent( const EventData& data )
{
	if( strEventChannelEndPoint.empty() )
	{//no Event Channel endpoint , just discard the event
		return true;
	}

	if( !bConnected  && !connectEventChannel() )
	{
		return false;
	}

	try
	{
		switch( data.eventType )
		{
		case ICE_EVENT_TYPE_PROGRESS:
			{
				return sendProgressEvent( data );
			}
			break;
		case ICE_EVENT_TYPE_ITEMSETPPED:
			{
				return sendItemStepEvent( data );
			}
			break;
		case ICE_EVENT_TYPE_ENDOFSTREAM:
			{
				return sendEndOfStream( data );
			}
			break;
		case ICE_EVENT_TYPE_BEGINOFSTREAM:
			{
				return sendBeginOfStream( data );
			}
			break;
		case ICE_EVENT_TYPE_SPEEDCHANGE:
			{
				return sendSpeedChange(data);
			}
			break;
		case ICE_EVENT_TYPE_STATECHANGE:
			{
				return sendStateChange( data );
			}
			break;
		case ICE_EVENT_TYPE_PLAYLISTDESTROY:
			{
				return sendPlaylistDestroy( data );
			}
			break;
		case ICE_EVENT_TYPE_GENERICEVENT:
			{
				return sendGenericEvent( data );
			}
			break;
		default:
			{
				//do nothing
			}
			break;
		}
	}
	catch( const TianShanIce::InvalidStateOfArt& )
	{//connection lost
		bConnected = false;
		connectEventChannel( );
		return false;
	}

	return true;
}

bool IceEventSender::sendGenericEvent( const EventData& data )
{
	try
	{
		genericEventPrx->post( data.catagory , data.eventId , data.eventName, data.stampUTC , data.sourceNetId , data.eventProp );
	}
	catch( const Ice::Exception& )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>( "StreamSmith" , 0 , "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just return true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT , CLOGFMT(IceEventSender,"caught unknow exception when sent generic event" ));
		return true;
	}	
	
	return true;
}

#define EVENTCSEQ "EventSeq"

bool IceEventSender::sendPlaylistDestroy( const EventData &data )
{
	try
	{
		Ice::Int			exitCode;	
		std::string			exitReason;
		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PLDESTROY_EXITCODE , exitCode );	
		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PLDESTROY_EXIT_REASON , exitReason );

		TianShanIce::Properties eventProperty;
		if(!translatePlaylistDestroyProperty( data , eventProperty ) )
		{//invalid event data
			return true;
		}
		ZQTianShan::Util::mergeProperty(eventProperty,data.eventProp);

		char szSeq[16];
		sprintf( szSeq, "%d" , data.eventSeq );

		Ice::Context ctx;
		ctx[EVENTCSEQ] = szSeq ;
		streamEventPrx->OnExit(	data.playlistProxyString , 
			data.playlistId.name , 
			exitCode , 
			exitReason ,
			ctx );

		streamEventPrx->OnExit2(data.playlistProxyString , 
			data.playlistId.name , 
			exitCode , 
			exitReason ,
			eventProperty , 
			ctx );
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR , 
			CLOGFMT( IceEventSender , "caught TianShanIce::InvalidParameter exception when send PlaylistDestroy event : %s" ),
			ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}
	catch( const Ice::Exception& )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>( "StreamSmith" , 0 , "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just resutn true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT , 
			CLOGFMT(IceEventSender,"caught unknow exception when sent PlaylistDestroy event" ));
		return true;
	}	
	return true;
}

bool IceEventSender::sendStateChange( const EventData& data )
{
	try
	{		
		Ice::Int		prevState,curState;
		std::string     perRequested;

		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_STATECHANGE_PREV_STATE , prevState );
		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_STATECHANGE_CUR_STATE ,curState );		

		TianShanIce::Streamer::StreamState streamerStatePrev, streamerStateCur ;
		streamerStatePrev = static_cast<TianShanIce::Streamer::StreamState>(prevState);
		streamerStateCur = static_cast<TianShanIce::Streamer::StreamState>(curState);

		TianShanIce::Properties eventProperty;
		if (!translateStateChangeProperty(data,eventProperty))
		{
			return false;
		}
		ZQTianShan::Util::mergeProperty(eventProperty,data.eventProp);
		char szSeq[16];
		sprintf( szSeq , "%d" , data.eventSeq );

		Ice::Context ctx;
		ctx[EVENTCSEQ] = szSeq ;
		streamEventPrx->OnStateChanged( data.playlistProxyString ,
			data.playlistId.name , 
			streamerStatePrev ,
			streamerStateCur , 
			eventProperty,
			ctx );
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR ,CLOGFMT( IceEventSender , "caught TianShanIce::InvalidParameter exception when send StateChange event : %s" ),
			ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}		
	catch( const Ice::Exception& )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>( "StreamSmith" , 0 , "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just resutn true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT , CLOGFMT(IceEventSender,"caught unknow exception when sent StateChange event" ));
		return true;
	}
	
	return true;
}

bool IceEventSender::sendSpeedChange( const EventData& data )
{
	try
	{
		Ice::Float		prevSpeed , curSpeed;

		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_SPEEDCHANGE_PREV_SPEED , prevSpeed );
		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_SPEEDCHANGE_CUR_SPEED , curSpeed );

		char szSeq[16];
		sprintf( szSeq , "%d" , data.eventSeq );

		TianShanIce::Properties eventProperty;
		if(!translateSpeedChangeProperty(data,eventProperty))
		{
			return false;
		}
		ZQTianShan::Util::mergeProperty(eventProperty,data.eventProp);
		Ice::Context ctx;
		ctx[EVENTCSEQ] = szSeq ;
		streamEventPrx->OnSpeedChanged( data.playlistProxyString , 
			data.playlistId.name , 
			prevSpeed , 
			curSpeed ,
			data.eventProp ,
			ctx );
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR , 
			CLOGFMT( IceEventSender , "caught TianShanIce::InvalidParameter exception when send SpeedChange event : %s" ),
			ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}
	catch( const Ice::Exception&  )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>( "StreamSmith" , 0 , "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just resutn true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT , 
			CLOGFMT(IceEventSender,"caught unknow exception when sent SpeedChange event" ));
		return true;
	}	
	return true;
}

bool IceEventSender::sendBeginOfStream( const EventData& data )
{
	try
	{
		char szSeq[16];
		sprintf( szSeq , "%d", data.eventSeq );
		Ice::Context ctx;					
		ctx[EVENTCSEQ] = szSeq ;
		TianShanIce::Properties eventproperty;
		if (!translateBeginOfStreamProperty(data,eventproperty))
		{
			return false;
		}
		ZQTianShan::Util::mergeProperty(eventproperty,data.eventProp);
		streamEventPrx->OnBeginningOfStream( data.playlistProxyString , data.playlistId.name , data.eventProp , ctx);
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR , 
			CLOGFMT( IceEventSender , "caught TianShanIce::InvalidParameter exception when send BeginOfStream event : %s" ),
			ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}
	catch( const Ice::Exception&  )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>( "StreamSmith" , 0 , "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just resutn true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT , 
			CLOGFMT(IceEventSender,"caught unknow exception when sent BeginOfStream event" ));
		return true;
	}		

	return true;
}

bool IceEventSender::sendEndOfStream( const EventData& data )
{
	try
	{
		char szSeq[16];
		sprintf( szSeq , "%d", data.eventSeq );

		Ice::Context ctx;					
		ctx[EVENTCSEQ] = szSeq ;

		std::string strEndTimeOffset ;
		ZQTianShan::Util::getValueMapDataWithDefault( data.eventData , EVENT_STREAM_COMPLETE_TIMEOFFSET , "" , strEndTimeOffset );
		if( !strEndTimeOffset.empty() )
		{
			ctx["EndTimeOffset"] = strEndTimeOffset;
		}
		TianShanIce::Properties eventproperty;
		if (!translateEndOfStreamProperty(data, eventproperty))
		{
			return false;
		}
		ZQTianShan::Util::mergeProperty(eventproperty,data.eventProp);
		streamEventPrx->OnEndOfStream( data.playlistProxyString , data.playlistId.name , data.eventProp ,  ctx );
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR , 
			CLOGFMT( IceEventSender , "caught TianShanIce::InvalidParameter exception when send EndOfStream event : %s" ),
			ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}
	catch( const Ice::Exception& )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>( "StreamSmith" , 0 , "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just return true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT , 
			CLOGFMT(IceEventSender,"caught unknow exception when sent EndOfStream event" ));
		return true;
	}		
	
	return true;
}

bool IceEventSender::sendItemStepEvent( const EventData& data )
{
	try
	{
		Ice::Int		prevCtrlNum , nextCtrlNum;

		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_ITEM_PREV_CTRLNUM , prevCtrlNum );
		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_ITEM_CUR_CTRLNUM , nextCtrlNum );

		TianShanIce::Properties eventProperty;
		if(!translateItemStepProperty(data,eventProperty))
		{
			return false;
		}
		ZQTianShan::Util::mergeProperty(eventProperty,data.eventProp);

		char szSeq[256];
		sprintf(szSeq, "%d", data.eventSeq );


		Ice::Context ctx;
		ctx[EVENTCSEQ] = szSeq ;
		playlistEventPrx->OnItemStepped(	data.playlistProxyString , 
			data.playlistId.name,
			nextCtrlNum,
			prevCtrlNum,
			eventProperty,
			ctx );
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR , 
			CLOGFMT( IceEventSender , "caught TianShanIce::InvalidParameter exception when send ItemStep event : %s" ),
			ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}
	catch( const Ice::Exception& )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>( "StreamSmith" , 0 , "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just resutn true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT , 
			CLOGFMT(IceEventSender,"caught unknow exception when sent ItemStep event" ));
		return true;
	}

	return true;
}

bool IceEventSender::sendProgressEvent( const EventData &data )
{
	try
	{
		Ice::Int		itemDone, itemTotal, curStep ,totalStep;
		Ice::Int		ctrlNum;
		std::string		itemName;
		Ice::Int		eventSeq;

		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PROGRESS_ITEM_PROGRESS_DONE , itemDone );
		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PROGRESS_ITEM_PROGRESS_TOTAL , itemTotal );
		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PROGRESS_ITEM_STEP_CURRENT , curStep );
		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PROGRESS_ITEM_STEP_TOTAL , totalStep );
		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PROGRESS_ITEM_CTRL_NUM , ctrlNum );
		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PROGRESS_ITEM_NAME , itemName );
		ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PROGRESS_ITEM_EVENT_SEQ , eventSeq );

		char	szComment[1024];
		sprintf( szComment, "CtrlNum=%d&ItemName=%s", ctrlNum , itemName.c_str() );

		char szSeq[16];
		sprintf(szSeq,"%d",eventSeq);


		Ice::Context ctx = data.eventProp;
		ctx[EVENTCSEQ] = szSeq ; 
		progressEventPrx->OnProgress(	data.playlistProxyString , 
			data.playlistId.name ,
			itemDone , itemTotal , 
			curStep , totalStep ,
			szComment ,
			ctx );
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR , CLOGFMT( IceEventSender , "caught TianShanIce::InvalidParameter exception when send Progress event : %s" ),
			ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}
	catch( const Ice::Exception&  )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>( "StreamSmith" , 0 , "connection lost");				
		return false;
	}
	catch( ... )
	{//unknown exception, just return true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT , 
			CLOGFMT(IceEventSender,"caught unknow exception when sent progress event" ));
		return true;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SsEventManager
SsEventManager::SsEventManager( IceEventSender& sender , SsEnvironment* environment )
	:eventSender(sender),
	env(environment)
{
	eventLst.clear( );
	bQuit = true;
}

SsEventManager::~SsEventManager( )
{
	stopSender();
}

void SsEventManager::startSender( const std::string& strEventChannelEndPoint )
{
	eventSender.startSender( strEventChannelEndPoint );
	start();
}

void SsEventManager::stopSender( )
{
	if( !bQuit )
	{
		bQuit = true;
		eventSem.post();
		waitHandle(-1);
	}
}

void	SsEventManager::postEvent(  EventData& eventData  )
{
	{
		eventData.lastAccessTime	= IceUtil::Time::now();		
		IceUtil::Mutex::Lock gd(mutex);
		eventLst.push_back( eventData );
	}
	eventSem.post();
}

#if defined ZQ_OS_MSWIN
	#pragma warning(disable:4996)
#endif

const char* convertEventTypeToString( Ice::Int type )
{
	switch( type )
	{
	case ICE_EVENT_TYPE_PROGRESS:
		return "ICE_EVENT_TYPE_PROGRESS";

	case ICE_EVENT_TYPE_ITEMSETPPED:
		return "ICE_EVENT_TYPE_ITEMSETPPED";

	case ICE_EVENT_TYPE_ENDOFSTREAM:
		return "ICE_EVENT_TYPE_ENDOFSTREAM";

	case ICE_EVENT_TYPE_BEGINOFSTREAM:
		return "ICE_EVENT_TYPE_BEGINOFSTREAM";

	case ICE_EVENT_TYPE_SPEEDCHANGE:
		return "ICE_EVENT_TYPE_SPEEDCHANGE";

	case ICE_EVENT_TYPE_STATECHANGE:
		return "ICE_EVENT_TYPE_STATECHANGE";

	case ICE_EVENT_TYPE_PLAYLISTDESTROY:
		return "ICE_EVENT_TYPE_PLAYLISTDESTROY";

	default :
		return "Unknown Event";
	}
}
const char* printDetailOfEventData( const EventData& data )
{
	static char szLocalBuffer[1024];
	szLocalBuffer[ sizeof(szLocalBuffer)-1 ] = 0;
	snprintf( szLocalBuffer , sizeof(szLocalBuffer)-1 , "EVENT[%s] PlayList[%s]" ,
		convertEventTypeToString( data.eventType ), 
		data.playlistId.name.c_str() );	
	return szLocalBuffer;
}

bool SsEventManager::isEventIgnorable( const EventData& data ) const
{
	return (data.eventType <= ICE_EVENT_TYPE_PROGRESS);	
}

int SsEventManager::run( )
{
	bQuit = false;

	Ice::Int sleepInterval = 1000;
	Ice::Int maxSleepInterval = env->getConfig().iEventDetainMaxInterval;
	if( maxSleepInterval < 1000 )
		maxSleepInterval = 1000;

	while(!bQuit)
	{
		{			
			eventSem.wait();
			if( bQuit )
			{//quit the thread
				break;
			}
		}

		do
		{
			EventData eventData;
			{
				IceUtil::Mutex::Lock gd(mutex);
				if( eventLst.size() <= 0 )
				{
					break;
				}
				if( eventLst.size() > 100 )
				{
					ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(SsEventManager,"current event count [%u]"), eventLst.size()	);
				}
				eventData = eventLst.front();
				eventLst.pop_front();
			}
			IceUtil::Time now = IceUtil::Time::now();

			if( ( now - eventData.lastAccessTime ).toMilliSeconds() > env->getConfig().iEventDiscardThrehold )
			{
				ENVLOG(ZQ::common::Log::L_WARNING ,	CLOGFMT( SsEventManager , "event [%s] is expired ,discard it" ) , 
					printDetailOfEventData( eventData ) );
				continue;
			}

			if( !eventSender.sendEvent(eventData) )
			{
				if( !isEventIgnorable(eventData))
				{
					ENVLOG(ZQ::common::Log::L_WARNING , CLOGFMT( SsEventManager, "can't send event [%s] , sleep for a while and re-try"),
						printDetailOfEventData( eventData ) );
					{

						IceUtil::Mutex::Lock gd(mutex);
						eventLst.push_front( eventData );
					}
					//sleep for a while					
					ZQ::common::delay(sleepInterval);
					sleepInterval = sleepInterval << 1;
					if(sleepInterval > maxSleepInterval)
						sleepInterval = maxSleepInterval;
				}
			}
			else
			{
				sleepInterval = 1000;
				if ( !isEventIgnorable(eventData) )
				{
					ENVLOG( ZQ::common::Log::L_DEBUG, CLOGFMT(SsEventManager , "sent event [%s] successfully") , 
						printDetailOfEventData( eventData ) );
				}
			}
		}
		while( true );
	}

	bQuit = true;
	return 1;
}

bool IceEventSender::translateProgressProperty( const EventData& data , TianShanIce::Properties& properties )
{
	return true;
}

bool IceEventSender::translateItemStepProperty( const EventData& data , TianShanIce::Properties& properties  )
{
	std::string		strPrevItemName, strNextItemName;
	std::string		strEventTimeStamp;
	std::string		strPreviousPID, strPreviousPAID;
	std::string		strNextPID, strNextPAID;
	std::string		strPreviousStreamingSource;
	std::string		strNextStreamingSource;
	std::string		strCompleteOffset, strTotalVideoDuration;
	Ice::Long       prevItemFlag;
	Ice::Long       currItemFlag;
	Ice::Long       currPlayTime;
	Ice::Long       prevPlayTime;
	Ice::Long       prevNptPrimary;

	Ice::Int		curItemTimeOffset;
	Ice::Int		clusterId;
	Ice::Int		eventSeq;

	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_PREV_ITEM_NAME , strPrevItemName );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_NEXT_ITEM_NAME , strNextItemName );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_TIME_STAMP_UTC , strEventTimeStamp );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_PREV_ITEM_PID , strPreviousPID );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_PREV_ITEM_PAID , strPreviousPAID );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_CUR_ITEM_PID , strNextPID );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_CUR_ITEM_PAID , strNextPAID );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_PREV_STREAMSOURCE , strPreviousStreamingSource );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_CUR_STREAMSOURCE , strNextStreamingSource );

	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_ITEM_CUR_TIMEOFFSET,curItemTimeOffset );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_EVENT_SEQ , eventSeq );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_CLSUTERID , clusterId );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_EVENT_CURFLAG ,currItemFlag );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_EVENT_PREVFLAG,prevItemFlag);
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_EVENT_CURPLAYTIME,currPlayTime);
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_EVENT_PREVPLAYTIME,prevPlayTime);
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_ITEMSTEP_EVENT_PREVPLAYTIME,prevPlayTime);
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_STREAM_COMPLETE_TIMEOFFSET,strCompleteOffset);
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_STREAM_VIDEOTOTALDURATION,strTotalVideoDuration);
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_STREAM_PREV_PLYPOSPRIMARY,prevNptPrimary);

	TianShanIce::Properties&	eventProperty = properties;

	eventProperty.insert( TianShanIce::Properties::value_type("prevItemName",strPrevItemName));
	eventProperty.insert( TianShanIce::Properties::value_type("currentItemName",strNextItemName));
	eventProperty.insert( TianShanIce::Properties::value_type("stampUTC",strEventTimeStamp));
	eventProperty.insert( TianShanIce::Properties::value_type("prevProviderId",strPreviousPID));
	eventProperty.insert( TianShanIce::Properties::value_type("prevProviderAssetId",strPreviousPAID));
	eventProperty.insert( TianShanIce::Properties::value_type("currentProviderId",strNextPID));
	eventProperty.insert( TianShanIce::Properties::value_type("currentProviderAssetId",strNextPAID));
	eventProperty.insert( TianShanIce::Properties::value_type("StreamingTotalDuration",strCompleteOffset));
	eventProperty.insert( TianShanIce::Properties::value_type("StreamingTotalVideoDuration",strTotalVideoDuration));

	if( strPreviousStreamingSource.empty() )
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT( IceEventSender , "playlist[%s] set prevStreamingSource to 0"),
			data.playlistId.name.c_str() );
		eventProperty.insert( TianShanIce::Properties::value_type("prevStreamingSource","0"));
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT( IceEventSender , "playlist[%s] set prevStreamingSource to 1"),
			data.playlistId.name.c_str() );
		eventProperty.insert( TianShanIce::Properties::value_type("prevStreamingSource","1"));
	}

	if( strNextStreamingSource.empty() )
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT( IceEventSender , "playlist[%s] set curStreamingSource to 0"),
			data.playlistId.name.c_str() );			
		eventProperty.insert( TianShanIce::Properties::value_type("curStreamingSource","0"));
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT( IceEventSender , "playlist[%s] set curStreamingSource to 1"),
			data.playlistId.name.c_str() );			
		eventProperty.insert( TianShanIce::Properties::value_type("curStreamingSource","1"));		
	}
	char szBuf[256];

	sprintf(szBuf,"%u",clusterId);
	eventProperty.insert( TianShanIce::Properties::value_type("clusterId",std::string(szBuf) ) );

	sprintf(szBuf,"%u",curItemTimeOffset);				
	eventProperty.insert( TianShanIce::Properties::value_type("currentItemTimeOffset",std::string(szBuf) ) );

	char szSeq[256];
	sprintf(szSeq, "%d", data.eventSeq );
	eventProperty.insert( TianShanIce::Properties::value_type(EVENTCSEQ,szSeq));

	char szFlag[256];
	sprintf(szFlag, "%lu",currItemFlag);
	eventProperty.insert( TianShanIce::Properties::value_type("StreamingCurrentFlag",std::string(szFlag)));
	sprintf(szFlag, "%lu", prevItemFlag);
	eventProperty.insert( TianShanIce::Properties::value_type("StreamingPreviousFlag",std::string(szFlag)));
	char szCurrPlayTime[128];
	sprintf(szCurrPlayTime,"%lu", currPlayTime);
	eventProperty.insert(TianShanIce::Properties::value_type("StreamingCurrentPlayTime",std::string(szCurrPlayTime)));
	char szPrevPlayTime[128];
	sprintf(szPrevPlayTime,"%lu",prevPlayTime);
	eventProperty.insert(TianShanIce::Properties::value_type("StreamingPreviousPlayTime",std::string(szPrevPlayTime)));
	char szPrevNpt[128];
	sprintf(szPrevNpt,"%lu",prevNptPrimary);
	eventProperty.insert( TianShanIce::Properties::value_type("StreamPrevItemNptPrimary",szPrevNpt));
	return true;
}

bool IceEventSender::translateEndOfStreamProperty( const EventData& data , TianShanIce::Properties& properties )
{
	Ice::Int  curItemTimeOffset;
	Ice::Float  currentSpeed;
	Ice::Int   userCtlNum;
	std::string  strCompleteTimeOffset;
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_SPEEDCHANGE_CUR_SPEED,currentSpeed);
	ZQTianShan::Util::getValueMapData(data.eventData,EVENT_ITEMSTEP_ITEM_CUR_TIMEOFFSET,curItemTimeOffset);
	ZQTianShan::Util::getValueMapData(data.eventData,"ItemIndexOnEndOfStream",userCtlNum);
	ZQTianShan::Util::getValueMapData(data.eventData,EVENT_STREAM_COMPLETE_TIMEOFFSET,strCompleteTimeOffset);
	TianShanIce::Properties& eventProperty = properties;
	eventProperty.insert(TianShanIce::Properties::value_type("TimeOffsetOnEndOfStream",strCompleteTimeOffset));
	char szScale[128];
	sprintf(szScale,"%lu",currentSpeed);
	eventProperty.insert(TianShanIce::Properties::value_type(EVENT_SPEEDCHANGE_CUR_SPEED,std::string(szScale)));
	char szTimeOffset[128];
	sprintf(szTimeOffset,"%lu",curItemTimeOffset);
	eventProperty.insert(TianShanIce::Properties::value_type("itemNptPosition",std::string(szTimeOffset)));
	char szCtlNum[128];
	sprintf(szCtlNum,"%d",userCtlNum);
	eventProperty.insert(TianShanIce::Properties::value_type("ItemIndexOnEndOfStream",std::string(szCtlNum)));
	return true;
}

bool IceEventSender::translateBeginOfStreamProperty( const EventData& data , TianShanIce::Properties& properties )
{
	Ice::Float currentSpeed;
	Ice::Int  curItemTimeOffset;
	Ice::Int   userCtlNum;
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_SPEEDCHANGE_CUR_SPEED,currentSpeed);
	ZQTianShan::Util::getValueMapData(data.eventData,EVENT_ITEMSTEP_ITEM_CUR_TIMEOFFSET,curItemTimeOffset);
	ZQTianShan::Util::getValueMapData(data.eventData,"ItemIndexOnEndOfStream",userCtlNum);
	TianShanIce::Properties& eventProperty = properties;
	char szScale[128];
	sprintf(szScale,"%lu",currentSpeed);
	eventProperty.insert(TianShanIce::Properties::value_type(EVENT_SPEEDCHANGE_CUR_SPEED,std::string(szScale)));
	char szTimeOffset[128];
	sprintf(szTimeOffset,"%lu",curItemTimeOffset);
	eventProperty.insert(TianShanIce::Properties::value_type("itemNptPosition",std::string(szTimeOffset)));
	char szCtlNum[128];
	sprintf(szCtlNum,"%d",userCtlNum);
	eventProperty.insert(TianShanIce::Properties::value_type("ItemIndexOnEndOfStream",std::string(szCtlNum)));
	return true;
}

bool IceEventSender::translateSpeedChangeProperty( const EventData& data , TianShanIce::Properties& properties )
{
	std::string     perRequested;
	std::string     strCompleteTimeOffset;
	std::string     strTotalVideoDuration;
	Ice::Float      scaleNew;
	Ice::Long       PrevNptPrimary;
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_SPEEDCHANGE_EVENT_PREREQUESTED,perRequested);
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_STREAM_COMPLETE_TIMEOFFSET,strCompleteTimeOffset);
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_STREAM_VIDEOTOTALDURATION,strTotalVideoDuration);
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_STREAM_PREV_PLYPOSPRIMARY,PrevNptPrimary);
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_SPEEDCHANGE_CUR_SPEED,scaleNew);
	TianShanIce::Properties& eventProperty = properties;
	eventProperty.insert(TianShanIce::Properties::value_type("perRequested",perRequested));
	eventProperty.insert(TianShanIce::Properties::value_type("StreamingTotalDuration",strCompleteTimeOffset));
	eventProperty.insert(TianShanIce::Properties::value_type("StreamingTotalVideoDuration",strTotalVideoDuration));
	char szScale[128];
	sprintf(szScale,"%lu",scaleNew);
	eventProperty.insert(TianShanIce::Properties::value_type(EVENT_SPEEDCHANGE_CUR_SPEED,std::string(szScale)));
	char szNpt[128];
	sprintf(szNpt,"%lu",PrevNptPrimary);
	eventProperty.insert(TianShanIce::Properties::value_type("StreamPrevItemNptPrimary",szNpt));
	return true;
}

bool IceEventSender::translateStateChangeProperty( const EventData& data , TianShanIce::Properties& properties )
{
	std::string     preRequested;
	std::string     strCompleteTimeOffset;
	std::string     strTotalVideoDuration;
	Ice::Long       PrevNptPrimary;
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_STATECHANGE_EVENT_PREREQUESTED,preRequested);
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_STREAM_COMPLETE_TIMEOFFSET,strCompleteTimeOffset);
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_STREAM_VIDEOTOTALDURATION,strTotalVideoDuration);
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_STREAM_PREV_PLYPOSPRIMARY,PrevNptPrimary);
	TianShanIce::Properties& eventProperty = properties;
	eventProperty.insert(TianShanIce::Properties::value_type("perRequested",preRequested));
	eventProperty.insert(TianShanIce::Properties::value_type("StreamingTotalDuration",strCompleteTimeOffset));
	eventProperty.insert(TianShanIce::Properties::value_type("StreamingTotalVideoDuration",strTotalVideoDuration));
	char szNpt[128];
	sprintf(szNpt,"%lu",PrevNptPrimary);
	eventProperty.insert(TianShanIce::Properties::value_type("StreamPrevItemNptPrimary",szNpt));
	return true;
}

bool IceEventSender::translatePlaylistDestroyProperty( const EventData& data , TianShanIce::Properties& properties )
{

	std::string			exitPID , exitPAID;
	std::string			streamingSource;
	std::string			eventTimeStamp;

	Ice::Int			clusterId;
	Ice::Int			exitOnPlaying;


	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PLDESTROY_CLUSTERID , clusterId );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PLDESTROY_EXITONPLAYING , exitOnPlaying );

	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PLDESTROY_EXIT_PID , exitPID );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PLDESTROY_EXIT_PAID , exitPAID );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PLDESTROY_EXIT_STREAMINGSOURCE , streamingSource );
	ZQTianShan::Util::getValueMapData( data.eventData , EVENT_PLDESTROY_EVENT_TIMESTAMP , eventTimeStamp );


	TianShanIce::Properties	eventProperty;
	properties.clear();

	properties.insert( TianShanIce::Properties::value_type("providerId",exitPID));
	properties.insert( TianShanIce::Properties::value_type("providerAssetId",exitPAID));
	if(streamingSource.empty() )
	{
		properties.insert( TianShanIce::Properties::value_type("streamingSource","0"));
	}
	else
	{
		properties.insert( TianShanIce::Properties::value_type("streamingSource","1"));
	}
	properties.insert( TianShanIce::Properties::value_type("exitWhilePlaying",exitOnPlaying ? "1" : "0"));

	char szBuf[256];
	sprintf(szBuf,"%u",clusterId);
	properties.insert( TianShanIce::Properties::value_type("clusterId",std::string(szBuf) ) );

	//stampUTC 
	properties.insert( TianShanIce::Properties::value_type("stampUTC",eventTimeStamp ) );

	return true;
}


}}//namespace ZQ::StreamSmith
