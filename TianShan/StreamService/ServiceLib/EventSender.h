#ifndef _zq_streamservice_event_sender_header_file_h__
#define _zq_streamservice_event_sender_header_file_h__

#include <string>
#include <list>
#include "SsEnvironment.h"
#include <Locks.h>
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <TsEvents.h>



namespace ZQ
{
namespace StreamService
{

typedef struct _EventData
{
	Ice::Int					eventType;
	Ice::Int					eventSeq;
	IceUtil::Time				lastAccessTime;
	TianShanIce::ValueMap		eventData;
	Ice::Identity				playlistId;
	std::string					playlistProxyString;
	//////////////////////////////////////////////////////////////////////////
	//for Generic event
	std::string					catagory;
	int							eventId;
	std::string					eventName;
	std::string					stampUTC;
	std::string					sourceNetId;
	TianShanIce::Properties		eventProp;
}EventData;

class IceEventSender 
{
public:
	IceEventSender( SsEnvironment* environment );
	virtual ~IceEventSender( );

public:

	bool				startSender( const std::string& strEventChannelEndpoint );

	bool				stopSender( );

	bool				sendEvent( const EventData& data );


protected:

	void				run( );

protected:

	bool				sendProgressEvent( const EventData& data );

	bool				sendItemStepEvent( const EventData& data );

	bool				sendEndOfStream( const EventData& data );

	bool				sendBeginOfStream( const EventData& data );

	bool				sendSpeedChange( const EventData& data );

	bool				sendStateChange( const EventData& data );

	bool				sendPlaylistDestroy( const EventData& data );

	bool				sendGenericEvent( const EventData& data );

protected:

	virtual bool		translateProgressProperty( const EventData& data , TianShanIce::Properties& properties );

	virtual	bool		translateItemStepProperty( const EventData& data , TianShanIce::Properties& properties  );

	virtual	bool		translateEndOfStreamProperty( const EventData& data , TianShanIce::Properties& properties );

	virtual bool		translateBeginOfStreamProperty( const EventData& data , TianShanIce::Properties& properties );

	virtual bool		translateSpeedChangeProperty( const EventData& data , TianShanIce::Properties& properties );

	virtual bool		translateStateChangeProperty( const EventData& data , TianShanIce::Properties& properties );

	virtual bool		translatePlaylistDestroyProperty( const EventData& data , TianShanIce::Properties& properties );

private:

	bool				connectEventChannel( );

private:

	std::string											strEventChannelEndPoint;
	IceStorm::TopicManagerPrx							topicManagerPrx;
	TianShanIce::Streamer::PlaylistEventSinkPrx			playlistEventPrx;
	TianShanIce::Streamer::StreamEventSinkPrx			streamEventPrx;
	TianShanIce::Streamer::StreamProgressSinkPrx		progressEventPrx;
	TianShanIce::Events::GenericEventSinkPrx			genericEventPrx;

	bool												bConnected;
	SsEnvironment*										env;
};

class SsEventManager : public ZQ::common::NativeThread
{
public:
	SsEventManager( IceEventSender& sender , SsEnvironment* environment );
	virtual ~SsEventManager(void);
public:
	///start thread and connect to event channel
	void		startSender( const std::string& strEventChannelEndPoint );

	///stop thread and disconnect to event channel
	void		stopSender( );


	void		postEvent( EventData& eventData );

protected:

	bool		isEventIgnorable( const EventData& data ) const;

	virtual int	run( );

private:

	bool			connectToEventChannel( );

private:

	ZQ::common::Semaphore								eventSem;
	IceUtil::Mutex										mutex;

	bool												bQuit;

	typedef std::list<EventData>						EventList;
	EventList											eventLst;

	IceEventSender&										eventSender;
	SsEnvironment*										env;
};


}}//namespace ZQ::StreamService

#endif//_zq_streamservice_event_sender_header_file_h__
