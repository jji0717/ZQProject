
#include <ZQ_common_conf.h>

#ifndef WITH_ICESTORM
	#define WITH_ICESTORM
#endif
#include <TianShanDefines.h>
#include "SiteAdminEnv.h"
#include "StreamEventSinker.h"



StreamEventSinker::StreamEventSinker( SiteAdminEnv& env )
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
	::Ice::ObjectPrx obj  = mEnv.getAdapter()->addWithUUID(handler);
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
	::Ice::ObjectPrx obj  = mEnv.getAdapter()->addWithUUID(handler);
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
	::Ice::ObjectPrx obj  = mEnv.getAdapter()->addWithUUID(handler);
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
	return true;
}

void StreamEventSinker::stop()
{
	mbQuit = true;
	if( mSentinel)
	{
		mSentinel->stop();
		delete mSentinel;
		mSentinel = NULL;
	}
	unsubscribe();
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
	{
		try
		{
			Ice::CommunicatorPtr ic = mEnv.getIc();
			mTopicManager = IceStorm::TopicManagerPrx::checkedCast( mEnv.getIc()->stringToProxy(mEndpoint) );
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
	return true;
}

