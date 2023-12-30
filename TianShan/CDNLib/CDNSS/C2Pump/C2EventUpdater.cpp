
#include <sstream>
#ifndef ZQ_CDN_UMG
#include "C2StreamerEnv.h"
#else
#include "HttpC2Streamer.h"
#define MLOG	(*getEnvironment()->getLogger())
#endif
#include "C2EventUpdater.h"

namespace C2Streamer
{
#ifndef ZQ_CDN_UMG
C2EventUpdater::C2EventUpdater( C2StreamerEnv& env )
:mEnv(env),
#else
C2EventUpdater::C2EventUpdater()
:
#endif
mbQuit(false),
mEventMask(METHOD_NULL),
mEventSinker(NULL)
{
}
C2EventUpdater::~C2EventUpdater()
{
}

std::string dumpC2Event( C2EventPtr event )
{
	std::ostringstream oss;
	switch( event->eventmethod )
	{
	case METHOD_SESS_UPDATE:
		{
			TransferStateUpdateEventPtr s = TransferStateUpdateEventPtr::dynamicCast(event);
			assert( s != NULL );
			oss<<"C2Event: SESS_UPDATE sess[" << s->transferId << "] state["<<convertSessionStateToString(s->sessionState)<<"]";
		}
		break;
	case METHOD_UDP_EVENT:
		{
			C2UdpSessionEventPtr s = C2UdpSessionEventPtr::dynamicCast(event);
			assert(s != NULL);
			oss <<"C2Event: udpSession[" <<s->subSessId <<"] ";
		}
		break;
	default:
		oss<<"unknown";
		break;
	}
	return oss.str();
}

void C2EventUpdater::post( C2EventPtr event )
{
	if(!(event->eventmethod & mEventMask))
	{
		return;
	}
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(C2EventUpdater,"%s"),dumpC2Event(event).c_str());
	{
		ZQ::common::MutexGuard gd(mMutex);
		//update event list
		mC2Events.push_back( event );
	}
	mSema.post();	
}
bool C2EventUpdater::start(  )
{
	mbQuit = false;
    return ZQ::common::NativeThread::start();
}

void C2EventUpdater::setSinker(C2EventSinkerPtr sinker, int mask )
{
	mEventSinker 	= sinker;
	mEventMask		= mask;
}

void C2EventUpdater::stop()
{
	mbQuit = true;
	mSema.post();
	waitHandle( 100 * 1000 );
}

int C2EventUpdater::run()
{
	C2EventPtr e = NULL;
	while(!mbQuit)
	{
		{			
			mSema.timedWait( 1000 * 1000 );
		}
		while(true)
		{			
			if(mbQuit ) break;			
			{
				ZQ::common::MutexGuard gd(mMutex);
				if(mC2Events.empty())
					break;

				e = mC2Events.front();
				mC2Events.pop_front();
			}
			if( e && mEventSinker)
			{
				try			
				{
					mEventSinker->publish(e);
				}
				catch( ... )
				{
					MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(C2EventUpdater,"run() caught unkown exception while push event"));
				}
			}
		}
		e = NULL;
	}
	return 1;
}

	
}//namespace C2Streamer
