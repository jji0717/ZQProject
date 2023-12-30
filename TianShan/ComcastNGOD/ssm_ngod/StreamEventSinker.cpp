
#include <ZQ_common_conf.h>

#include <TimeUtil.h>
#include "NgodConfig.h"

#define WITH_ICESTORM
#include <TianShanDefines.h>
#include "NgodEnv.h"
#include "StreamEventSinker.h"



namespace NGOD
{

StreamEventSinker::StreamEventSinker( NgodEnv& env )
:mEnv(env),
mbQuit( true ),
mSentinel(NULL)
{
}

StreamEventSinker::~StreamEventSinker(void)
{
	stop();
}

bool StreamEventSinker::addEventHandler( TianShanIce::Streamer::StreamEventSinkPtr handler )
{
	::Ice::ObjectPrx obj  = mEnv.getObjAdapter()->addWithUUID(handler);
	if( !obj )
	{
		return false;
	}
	Subscriber s;
	s.handler	= obj;
	s.topic		= NULL;
	s.topicName = TianShanIce::Streamer::TopicOfStream;
	mSubscriber.push_back( s );
	return true;
}

bool StreamEventSinker::addEventHandler( TianShanIce::Streamer::PlaylistEventSinkPtr handler )
{
	::Ice::ObjectPrx obj  = mEnv.getObjAdapter()->addWithUUID(handler);
	if( !obj )
	{
		return false;
	}

	Subscriber s;
	s.handler	= obj;
	s.topic		= NULL;
	s.topicName = TianShanIce::Streamer::TopicOfPlaylist;
	mSubscriber.push_back( s );
	return true;
}

bool StreamEventSinker::addEventHandler( TianShanIce::Events::GenericEventSinkPtr handler , const std::string& topicName  )
{
	::Ice::ObjectPrx obj  = mEnv.getObjAdapter()->addWithUUID(handler);
	if( !obj )
	{
		return false;
	}

	Subscriber s;
	s.handler	= obj;
	s.topic		= NULL;
	s.topicName = topicName;
	mSubscriber.push_back( s );
	return true;
}

bool StreamEventSinker::start( const std::string& endpoint )
{
	if(endpoint.empty())
	{
		mEndpoint= "";
		return true;
	}

	if(endpoint.find(":") == std::string::npos )
	{
		mEndpoint = SERVICE_NAME_TopicManager":";
		mEndpoint = mEndpoint + endpoint;
	}
	else
	{
		mEndpoint = endpoint;
	}
	mbQuit = false;
	if( mSentinel )
	{
		mSentinel->stop();
		delete mSentinel;
	}
	mSentinel = new EventChannel::Sentinel(MLOG,mEndpoint,this);
	mSentinel->start();	
	return ZQ::common::NativeThread::start();
}

int StreamEventSinker::run()
{
	int64 interval = ngodConfig.announce.resubscribeAtIdle;
	if(interval < 10 * 60 * 1000 )
		interval = 10 * 60 * 1000;
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamEventSinker,"event receiving status checker is running, interval[%lld]"), interval);
	while(!mbQuit)
	{
		mSem.timedWait(1000);
		if(mbQuit)	break;
		int64 delta = ZQ::common::now() - mEnv.getLastEventRecvTime();
		if( delta > interval )
		{
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(StreamEventSinker,"no new event received for [%lld]ms , reconnect to event channel"),delta );
			try
			{
				subscribe();// force to reconnect to event channel
			}
			catch(...){}
		}
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamEventSinker,"event receiving status checker is stopped"));
	return 0;
}

void StreamEventSinker::stop()
{
	mbQuit = true;
	mSem.post();
	if( mSentinel)
	{
		mSentinel->stop();
		delete mSentinel;
		mSentinel = NULL;
	}
	unsubscribe();
	waitHandle(10000);
}

void StreamEventSinker::onConnectionEstablished()
{	
	subscribe();
}
void StreamEventSinker::reportBadConnection()
{
	MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(StreamEventSinker,"connection to eventChannel is down, try to re-connect"));
}

void StreamEventSinker::unsubscribe( )
{
	std::vector< Subscriber >::iterator it = mSubscriber.begin();
	for( ; it != mSubscriber.end() ; it ++ )
	{
		try
		{
			if( it->topic)
			{
				it->topic->unsubscribe(it->handler);
			}			
		}
		catch( const Ice::Exception&)
		{
			
		}
		it->topic = NULL;
	}
}

bool StreamEventSinker::subscribe( )
{
	static ZQ::common::Mutex locker;
	ZQ::common::MutexGuard gd(locker);//should get lock before connect to event channel

	{
		try
		{
			Ice::CommunicatorPtr ic = mEnv.getCommunicator();
			mTopicManager = IceStorm::TopicManagerPrx::checkedCast( STR2PROXY(mEndpoint) );
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamEventSinker,"connected to [%s] OK"), mEndpoint.c_str() );
		}
		catch( const Ice::Exception& ex )
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinker,"failed to connect to [%s] due to [%s]"), mEndpoint.c_str(),ex.ice_name().c_str() );
			return false;
		}
		if( mTopicManager == NULL )
			return false;
	}
	unsubscribe();
	std::vector< Subscriber >::iterator itSub = mSubscriber.begin();
	for( ; itSub != mSubscriber.end() ; itSub ++ )
	{
		try
		{
			itSub->topic = mTopicManager->retrieve( itSub->topicName );
		}
		catch( const IceStorm::NoSuchTopic& )
		{
			try
			{
				itSub->topic =  mTopicManager->create( itSub->topicName );
			}
			catch( const Ice::Exception& ex)
			{
				mTopicManager = NULL;
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(StreamEventSinker,"failed to create topic[%s] due to [%s]"),
					itSub->topicName.c_str() , ex.ice_name().c_str() );
				return false;
			}
		}
		catch( const Ice::Exception& ex )
		{
			mTopicManager = NULL;
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(StreamEventSinker,"failed to subscribe event due to [%s]"),ex.ice_name().c_str() );
			return false;
		}
		const ::TianShanIce::Properties qos;
#if ICE_INT_VERSION / 100 >= 303
		itSub->topic->subscribeAndGetPublisher(qos, itSub->handler );
#else
		itSub->topic->subscribe(qos, itSub->handler );
#endif
	}
	
	//update last event recv time
	mEnv.updateLastEventRecvTime( ZQ::common::now() );

	return true;
}

}//namespace NGOD
