
#include "TianShanIceHelper.h"
#include "SsEnvironment.h"
#include "EventSender.h"
#include <TimeUtil.h>


namespace ZQ {
namespace StreamService {

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

	ENVLOG( ZQ::common::Log::L_INFO, 
		CLOGFMT( IceEventSender, "start event sender with Event Channel endpoint[%s] "),
		strEventChannelEndPoint.c_str() );
	connectEventChannel( );
	//what ever connect to event channel ok or false, just return OK
	//If we connect to Event Channel failed, just reconnect it in the future
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
		ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT( IceEventSender, "connect to Event Channel: [%s]"), strEventChannelEndPoint.c_str() );
		Ice::CommunicatorPtr Ic = getSsEnv()->getCommunicator();
		if( !Ic )
		{
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT( IceEventSender, "no available Ice communicator, can't connect to Event Channel " ));
			return false;
		}

		topicManagerPrx = IceStorm::TopicManagerPrx::checkedCast( Ic->stringToProxy( strEventChannelEndPoint ) );
		if( !topicManagerPrx )
		{
			ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT( IceEventSender, "can't get topic manager through Event Channel [%s]" ),
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

		ENVLOG(ZQ::common::Log::L_INFO, CLOGFMT( IceEventSender, "connect to Event Channel OK :[%s] " ), strEventChannelEndPoint.c_str()  );

		bConnected = true;
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, 
			CLOGFMT( IceEventSender, "caught ice exception [%s] when connect to Event Channel [%s]"),
			ex.ice_name().c_str(),
			strEventChannelEndPoint.c_str() ) ;
		return false;
	}

	return true;
}

bool IceEventSender::sendEvent( const EventData& data )
{
	if( strEventChannelEndPoint.empty() )
	{
		//no Event Channel endpoint, just discard the event
		return true;
	}

	if( !bConnected  && !connectEventChannel() )
		return false;

	try
	{
		switch( data.eventType )
		{
		case ICE_EVENT_TYPE_PROGRESS:
			return sendProgressEvent( data );

		case ICE_EVENT_TYPE_ITEMSETPPED:
			return sendItemStepEvent( data );

		case ICE_EVENT_TYPE_ENDOFSTREAM:
			return sendEndOfStream( data );

		case ICE_EVENT_TYPE_BEGINOFSTREAM:
			return sendBeginOfStream( data );

		case ICE_EVENT_TYPE_SPEEDCHANGE:
			return sendSpeedChange(data);

		case ICE_EVENT_TYPE_STATECHANGE:
			return sendStateChange( data );

		case ICE_EVENT_TYPE_PLAYLISTDESTROY:
			return sendPlaylistDestroy( data );

		case ICE_EVENT_TYPE_GENERICEVENT:
			return sendGenericEvent( data );

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
		genericEventPrx->post( data.catagory, data.eventId, data.eventName, data.stampUTC, data.sourceNetId, data.eventProp );
	}
	catch( const Ice::Exception& )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("StreamService", 0, "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just return true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT, CLOGFMT(IceEventSender, "caught exception when sending generic event" ));
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
		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PLDESTROY_EXITCODE, exitCode );	
		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PLDESTROY_EXIT_REASON, exitReason );

		TianShanIce::Properties eventProperty;
		if(!translatePlaylistDestroyProperty( data, eventProperty ) )
		{//invalid event data
			return true;
		}

		ZQTianShan::Util::mergeProperty(eventProperty,data.eventProp);

		char szSeq[16];
		sprintf( szSeq, "%d", data.eventSeq );

		Ice::Context ctx;
		ctx[EVENTCSEQ] = szSeq ;
		streamEventPrx->OnExit(	data.playlistProxyString, 
			data.playlistId.name, 
			exitCode, 
			exitReason,
			ctx );

		streamEventPrx->OnExit2(data.playlistProxyString, 
			data.playlistId.name, 
			exitCode, 
			exitReason,
			eventProperty, 
			ctx );
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, 
			CLOGFMT( IceEventSender, "caught InvalidParameter when sending PlaylistDestroy event: %s" ),
			ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}
	catch( const Ice::Exception& )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("StreamService", 0, "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just resutn true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT, 
			CLOGFMT(IceEventSender, "caught exception when sending PlaylistDestroy event" ));
		return true;
	}	
	return true;
}

bool IceEventSender::sendStateChange( const EventData& data )
{
	try
	{		
		Ice::Int		prevState,curState;

		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_STATECHANGE_PREV_STATE, prevState );
		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_STATECHANGE_CUR_STATE,curState );		

		TianShanIce::Streamer::StreamState streamerStatePrev, streamerStateCur ;
		streamerStatePrev = static_cast<TianShanIce::Streamer::StreamState>(prevState);
		streamerStateCur = static_cast<TianShanIce::Streamer::StreamState>(curState);

		char szSeq[16];
		sprintf( szSeq, "%d", data.eventSeq );

		Ice::Context ctx;
		ctx[EVENTCSEQ] = szSeq ;
		streamEventPrx->OnStateChanged( data.playlistProxyString, data.playlistId.name, 
			streamerStatePrev, streamerStateCur, data.eventProp, ctx );
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT( IceEventSender, "caught InvalidParameter when sending StateChange event: %s" ),
			ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}		
	catch( const Ice::Exception& )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("StreamService", 0, "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just resutn true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT, CLOGFMT(IceEventSender, "caught exception when sending StateChange event" ));
		return true;
	}
	
	return true;
}

bool IceEventSender::sendSpeedChange( const EventData& data )
{
	try
	{
		Ice::Float		prevSpeed, curSpeed;

		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_SPEEDCHANGE_PREV_SPEED, prevSpeed );
		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_SPEEDCHANGE_CUR_SPEED, curSpeed );

		char szSeq[16];
		sprintf( szSeq, "%d", data.eventSeq );

		Ice::Context ctx;
		ctx[EVENTCSEQ] = szSeq ;
		streamEventPrx->OnSpeedChanged( data.playlistProxyString, data.playlistId.name, 
			prevSpeed, curSpeed, data.eventProp, ctx );
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, 
			CLOGFMT( IceEventSender, "caught InvalidParameter when sending SpeedChange event: %s" ),
			ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}
	catch( const Ice::Exception&  )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("StreamService", 0, "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just resutn true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT, 
			CLOGFMT(IceEventSender, "caught exception when sending SpeedChange event" ));
		return true;
	}	

	return true;
}

bool IceEventSender::sendBeginOfStream( const EventData& data )
{
	try
	{
		char szSeq[16];
		sprintf( szSeq, "%d", data.eventSeq );
		Ice::Context ctx;					
		ctx[EVENTCSEQ] = szSeq ;
		streamEventPrx->OnBeginningOfStream( data.playlistProxyString, data.playlistId.name, data.eventProp, ctx);
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT( IceEventSender, "caught InvalidParameter when send BeginOfStream event: %s" ), ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}
	catch( const Ice::Exception&  )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("StreamService", 0, "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just resutn true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT, CLOGFMT(IceEventSender, "caught exception when sending BeginOfStream event" ));
		return true;
	}		

	return true;
}

bool IceEventSender::sendEndOfStream( const EventData& data )
{
	try
	{
		char szSeq[16];
		sprintf( szSeq, "%d", data.eventSeq );

		Ice::Context ctx;					
		ctx[EVENTCSEQ] = szSeq ;

		std::string strEndTimeOffset ;
		ZQTianShan::Util::getValueMapDataWithDefault( data.eventData, EVENT_STREAM_COMPLETE_TIMEOFFSET, "", strEndTimeOffset );
		if( !strEndTimeOffset.empty() )
		{
			ctx["EndTimeOffset"] = strEndTimeOffset;
		}

		streamEventPrx->OnEndOfStream( data.playlistProxyString, data.playlistId.name, data.eventProp,  ctx );
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT( IceEventSender, "caught InvalidParameter when sending EndOfStream event: %s" ),
			ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}
	catch( const Ice::Exception& )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("StreamService", 0, "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just return true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT, CLOGFMT(IceEventSender, "caught exception when sending EndOfStream event" ));
		return true;
	}		
	
	return true;
}

bool IceEventSender::sendItemStepEvent( const EventData& data )
{
	try
	{
		Ice::Int		prevCtrlNum, nextCtrlNum;

		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_ITEM_PREV_CTRLNUM, prevCtrlNum );
		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_ITEM_CUR_CTRLNUM, nextCtrlNum );

		TianShanIce::Properties eventProperty;
		if(!translateItemStepProperty(data,eventProperty))
			return false;

		ZQTianShan::Util::mergeProperty(eventProperty,data.eventProp);

		char szSeq[256];
		sprintf(szSeq, "%d", data.eventSeq );

		Ice::Context ctx;
		ctx[EVENTCSEQ] = szSeq ;
		playlistEventPrx->OnItemStepped(data.playlistProxyString, data.playlistId.name,
			nextCtrlNum, prevCtrlNum, eventProperty, ctx );
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT( IceEventSender, "caught InvalidParameter when sending ItemStepped event: %s" ),
			ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}
	catch( const Ice::Exception& )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("StreamService", 0, "connection lost");
		return false;
	}
	catch( ... )
	{//unknown exception, just resutn true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT, 
			CLOGFMT(IceEventSender, "caught exception when sending ItemStepped event" ));
		return true;
	}

	return true;
}

bool IceEventSender::sendProgressEvent( const EventData &data )
{
	try
	{
		Ice::Int		itemDone, itemTotal, curStep,totalStep;
		Ice::Int		ctrlNum;
		std::string		itemName;
		Ice::Int		eventSeq;

		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PROGRESS_ITEM_PROGRESS_DONE, itemDone );
		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PROGRESS_ITEM_PROGRESS_TOTAL, itemTotal );
		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PROGRESS_ITEM_STEP_CURRENT, curStep );
		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PROGRESS_ITEM_STEP_TOTAL, totalStep );
		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PROGRESS_ITEM_CTRL_NUM, ctrlNum );
		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PROGRESS_ITEM_NAME, itemName );
		ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PROGRESS_ITEM_EVENT_SEQ, eventSeq );

		char	szComment[1024];
		sprintf( szComment, "CtrlNum=%d&ItemName=%s", ctrlNum, itemName.c_str() );

		char szSeq[16];
		sprintf(szSeq, "%d",eventSeq);


		Ice::Context ctx = data.eventProp;
		ctx[EVENTCSEQ] = szSeq ; 
		progressEventPrx->OnProgress(data.playlistProxyString, data.playlistId.name,
			itemDone, itemTotal, curStep, totalStep, szComment,	ctx );
	}
	catch( const TianShanIce::InvalidParameter& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamService, "caught InvalidParameter when sending Progress event: %s" ),
			ex.message.c_str() );
		//return true so we don't need to resend this invalid event
		return true;
	}
	catch( const Ice::Exception&  )
	{//treat all ice exception as connection exception because we use one way
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>("StreamService", 0, "connection lost");				
		return false;
	}
	catch( ... )
	{//unknown exception, just return true so we don't need to resend this event again
		ENVLOG(ZQ::common::Log::L_CRIT, 
			CLOGFMT(IceEventSender, "caught exception when sending progress event" ));
		return true;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SsEventManager
SsEventManager::SsEventManager( IceEventSender& sender, SsEnvironment* environment )
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

	default:
		return "Unknown Event";
	}
}
const char* printDetailOfEventData( const EventData& data )
{
	static char szLocalBuffer[1024];
	szLocalBuffer[ sizeof(szLocalBuffer)-1 ] = 0;
	snprintf( szLocalBuffer, sizeof(szLocalBuffer)-1, "EVENT[%s] PlayList[%s]",
		convertEventTypeToString( data.eventType ), data.playlistId.name.c_str() );	
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
					break;

				if( eventLst.size() > 100 )
				{
					ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(SsEventManager, "current event count [%u]"), eventLst.size()	);
				}

				eventData = eventLst.front();
				eventLst.pop_front();
			}
			IceUtil::Time now = IceUtil::Time::now();

			if( ( now - eventData.lastAccessTime ).toMilliSeconds() > env->getConfig().iEventDiscardThrehold )
			{
				ENVLOG(ZQ::common::Log::L_WARNING,	CLOGFMT( SsEventManager, "event [%s] is expired,discard it" ), 
					printDetailOfEventData( eventData ) );
				continue;
			}

			if( !eventSender.sendEvent(eventData) )
			{
				if( !isEventIgnorable(eventData))
				{
					ENVLOG(ZQ::common::Log::L_WARNING, CLOGFMT( SsEventManager, "can't send event [%s], sleep for a while and re-try"),
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
					ENVLOG( ZQ::common::Log::L_DEBUG, CLOGFMT(SsEventManager, "sent event [%s] successfully"), 
						printDetailOfEventData( eventData ) );
				}
			}
		}
		while( true );
	}

	bQuit = true;
	return 1;
}

bool IceEventSender::translateProgressProperty( const EventData& data, TianShanIce::Properties& properties )
{
	return true;
}

typedef struct _PropMapping
{
	const char* vmkey, *prop;
} PropMapping;

const PropMapping StrPropMapping_ItemStepped[] = {
	{ EVENT_ITEMSTEP_PREV_ITEM_NAME,    "prevItemName" },
	{ EVENT_ITEMSTEP_PREV_ITEM_PID,     "prevProviderId"},
	{ EVENT_ITEMSTEP_PREV_ITEM_PAID,    "prevProviderAssetId"},
	{ EVENT_ITEMSTEP_PREV_STREAMSOURCE, "prevStreamingSource"},

	{ EVENT_ITEMSTEP_NEXT_ITEM_NAME,    "currentItemName"},
	{ EVENT_ITEMSTEP_CUR_ITEM_PID,      "currentProviderId"},
	{ EVENT_ITEMSTEP_CUR_ITEM_PAID,     "currentProviderAssetId"},
	{ EVENT_ITEMSTEP_CUR_STREAMSOURCE,  "curStreamingSource"},

	{ EVENT_ITEMSTEP_TIME_STAMP_UTC,    "stampUTC"},

	{NULL, NULL} };

const PropMapping IntPropMapping_ItemStepped[] = {
	{ EVENT_ITEMSTEP_EVENT_SEQ,          EVENTCSEQ},

	{ EVENT_ITEMSTEP_PREV_ITEM_DUR,        "prevItemDuration"},
	{ EVENT_ITEMSTEP_PREV_ITEM_FLAGS,      "prevItemFlags"},

	{ EVENT_ITEMSTEP_ITEM_CUR_TIMEOFFSET, "currentItemTimeOffset"},
	{ EVENT_ITEMSTEP_CUR_ITEM_DUR,        "currentItemDuration"},
	{ EVENT_ITEMSTEP_CUR_ITEM_FLAGS,      "currentItemFlags"},

	{NULL, NULL} };

bool IceEventSender::translateItemStepProperty( const EventData& data, TianShanIce::Properties& properties )
{
	size_t i =0;
	for (i =0; StrPropMapping_ItemStepped[i].vmkey; i++)
	{
		std::string	value;
		ZQTianShan::Util::getValueMapData(data.eventData, StrPropMapping_ItemStepped[i].vmkey, value);
		properties.insert(TianShanIce::Properties::value_type(StrPropMapping_ItemStepped[i].prop,  value));
	}

	for (i =0; IntPropMapping_ItemStepped[i].vmkey; i++)
	{
		Ice::Int value =0;
		if (!ZQTianShan::Util::getValueMapDataWithDefault(data.eventData, IntPropMapping_ItemStepped[i].vmkey, 0, value))
			continue;

		char str[256];
		snprintf(str, sizeof(str)-2, "%d", value);
		properties.insert(TianShanIce::Properties::value_type(IntPropMapping_ItemStepped[i].prop, str));
	}

	if (properties["prevStreamingSource"].empty())
	{
		ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT( IceEventSender, "playlist[%s] set prevStreamingSource to 0"),	data.playlistId.name.c_str() );
		properties["prevStreamingSource"] = "0";
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT( IceEventSender, "playlist[%s] set prevStreamingSource to 1"),	data.playlistId.name.c_str() );
		properties["prevStreamingSource"] = "1";
	}

	if (properties["curStreamingSource"].empty())
	{
		ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT( IceEventSender, "playlist[%s] set curStreamingSource to 0"), data.playlistId.name.c_str() );			
		properties["curStreamingSource"] = "0";
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT( IceEventSender, "playlist[%s] set curStreamingSource to 1"), data.playlistId.name.c_str() );			
		properties["curStreamingSource"] = "1";
	}

	return true;
}

/*
bool IceEventSender::translateItemStepProperty( const EventData& data, TianShanIce::Properties& properties  )
{
	std::string		strPrevItemName, strNextItemName;
	std::string		strEventTimeStamp;
	std::string		strPreviousPID, strPreviousPAID;
	std::string		strNextPID, strNextPAID;
	std::string		strPreviousStreamingSource;
	std::string		strNextStreamingSource;

	Ice::Int		curItemTimeOffset;
	Ice::Int		clusterId;
	Ice::Int		eventSeq;

	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_PREV_ITEM_NAME,     strPrevItemName );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_NEXT_ITEM_NAME,     strNextItemName );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_TIME_STAMP_UTC,     strEventTimeStamp );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_PREV_ITEM_PID,      strPreviousPID );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_PREV_ITEM_PAID,     strPreviousPAID );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_CUR_ITEM_PID,       strNextPID );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_CUR_ITEM_PAID,      strNextPAID );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_PREV_STREAMSOURCE,  strPreviousStreamingSource );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_CUR_STREAMSOURCE,   strNextStreamingSource );

	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_ITEM_CUR_TIMEOFFSET, curItemTimeOffset );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_EVENT_SEQ,          eventSeq );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_ITEMSTEP_CLSUTERID,          clusterId );

	TianShanIce::Properties& eventProperty = properties;

	eventProperty.insert(TianShanIce::Properties::value_type("prevItemName",           strPrevItemName));
	eventProperty.insert(TianShanIce::Properties::value_type("currentItemName",        strNextItemName));
	eventProperty.insert(TianShanIce::Properties::value_type("stampUTC",               strEventTimeStamp));
	eventProperty.insert(TianShanIce::Properties::value_type("prevProviderId",         strPreviousPID));
	eventProperty.insert(TianShanIce::Properties::value_type("prevProviderAssetId",    strPreviousPAID));
	eventProperty.insert(TianShanIce::Properties::value_type("currentProviderId",      strNextPID));
	eventProperty.insert(TianShanIce::Properties::value_type("currentProviderAssetId", strNextPAID));

	if( strPreviousStreamingSource.empty() )
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT( IceEventSender, "playlist[%s] set prevStreamingSource to 0"),
			data.playlistId.name.c_str() );
		eventProperty.insert(TianShanIce::Properties::value_type("prevStreamingSource", "0"));
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT( IceEventSender, "playlist[%s] set prevStreamingSource to 1"),
			data.playlistId.name.c_str() );
		eventProperty.insert(TianShanIce::Properties::value_type("prevStreamingSource", "1"));
	}

	if( strNextStreamingSource.empty() )
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT( IceEventSender, "playlist[%s] set curStreamingSource to 0"),
			data.playlistId.name.c_str() );			
		eventProperty.insert(TianShanIce::Properties::value_type("curStreamingSource", "0"));
	}
	else
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT( IceEventSender, "playlist[%s] set curStreamingSource to 1"),
			data.playlistId.name.c_str() );			
		eventProperty.insert(TianShanIce::Properties::value_type("curStreamingSource", "1"));		
	}

	char szBuf[256];

	sprintf(szBuf, "%u",clusterId);
	eventProperty.insert(TianShanIce::Properties::value_type("clusterId",std::string(szBuf) ) );

	sprintf(szBuf, "%u",curItemTimeOffset);				
	eventProperty.insert(TianShanIce::Properties::value_type("currentItemTimeOffset",std::string(szBuf) ) );

	char szSeq[256];
	sprintf(szSeq, "%d", data.eventSeq );
	eventProperty.insert(TianShanIce::Properties::value_type(EVENTCSEQ,szSeq));

	return true;
}
*/

bool IceEventSender::translateEndOfStreamProperty( const EventData& data, TianShanIce::Properties& properties )
{
	return true;
}

bool IceEventSender::translateBeginOfStreamProperty( const EventData& data, TianShanIce::Properties& properties )
{
	return true;
}

bool IceEventSender::translateSpeedChangeProperty( const EventData& data, TianShanIce::Properties& properties )
{
	return true;
}

bool IceEventSender::translateStateChangeProperty( const EventData& data, TianShanIce::Properties& properties )
{
	return true;
}

bool IceEventSender::translatePlaylistDestroyProperty( const EventData& data, TianShanIce::Properties& properties )
{
	std::string			exitPID, exitPAID;
	std::string			streamingSource;
	std::string			eventTimeStamp;

	Ice::Int			clusterId;
	Ice::Int			exitOnPlaying;

	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PLDESTROY_CLUSTERID, clusterId );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PLDESTROY_EXITONPLAYING, exitOnPlaying );

	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PLDESTROY_EXIT_PID, exitPID );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PLDESTROY_EXIT_PAID, exitPAID );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PLDESTROY_EXIT_STREAMINGSOURCE, streamingSource );
	ZQTianShan::Util::getValueMapData(data.eventData, EVENT_PLDESTROY_EVENT_TIMESTAMP, eventTimeStamp );


	TianShanIce::Properties	eventProperty;
	properties.clear();

	properties.insert( TianShanIce::Properties::value_type("providerId",exitPID));
	properties.insert( TianShanIce::Properties::value_type("providerAssetId",exitPAID));
	if(streamingSource.empty() )
	{
		properties.insert( TianShanIce::Properties::value_type("streamingSource", "0"));
	}
	else
	{
		properties.insert( TianShanIce::Properties::value_type("streamingSource", "1"));
	}

	properties.insert( TianShanIce::Properties::value_type("exitWhilePlaying", exitOnPlaying ? "1": "0"));

	char szBuf[256];
	sprintf(szBuf, "%u",clusterId);
	properties.insert( TianShanIce::Properties::value_type("clusterId", std::string(szBuf) ) );

	//stampUTC 
	properties.insert( TianShanIce::Properties::value_type("stampUTC", eventTimeStamp ) );

	return true;
}


}}//namespace ZQ::StreamService
