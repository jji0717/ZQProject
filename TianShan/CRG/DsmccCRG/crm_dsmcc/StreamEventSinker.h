#ifndef _tianshan_ngod_stream_event_sinker_header_file_h__
#define _tianshan_ngod_stream_event_sinker_header_file_h__

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <TsStreamer.h>
#include <Sentinel.h>
#include <NativeThread.h>

namespace  ZQ {
namespace CLIENTREQUEST {

class DSMCC_Environment;

class StreamEventSinker : public EventChannel::Sentinel::ExternalControlData , public ZQ::common::NativeThread
{
public:
	StreamEventSinker( DSMCC_Environment& env );
	virtual ~StreamEventSinker(void);

public:

	bool		start( const std::string& endpoint );
	
	void		stop( );

	bool		addEventHandler( TianShanIce::Streamer::StreamEventSinkPtr handler );
	bool		addEventHandler( TianShanIce::Streamer::PlaylistEventSinkPtr handler );
	bool		addEventHandler( TianShanIce::Events::GenericEventSinkPtr handler , const std::string& topicName );

	bool		subscribe( );

protected:
	int				run();
	virtual void	reportBadConnection();	
	virtual void	onConnectionEstablished();

	void			unsubscribe( );

private:
	
	DSMCC_Environment&				mEnv;

	std::string						mEndpoint;

	struct Subscriber 
	{
		Ice::ObjectPrx			handler;
		std::string				topicName;
		IceStorm::TopicPrx		topic;
	};

	std::vector< Subscriber >		mSubscriber;
	IceStorm::TopicManagerPrx		mTopicManager;

	bool							mbQuit;
	ZQ::common::Semaphore			mSem;

	EventChannel::Sentinel*			mSentinel;
};

}}

#endif//_tianshan_ngod_stream_event_sinker_header_file_h__

