
#ifndef _tianshan_cdnss_c2streamer_event_updater_header_file_h__
#define _tianshan_cdnss_c2streamer_event_updater_header_file_h__

#include <list>
#include <string>
#include <Locks.h>
#include <NativeThread.h>

#include "C2StreamerLib.h"

namespace C2Streamer
{

#ifndef ZQ_CDN_UMG
class C2StreamerEnv;
#endif
class C2EventUpdater : public ZQ::common::NativeThread
{
public:
#ifndef ZQ_CDN_UMG
	C2EventUpdater( C2StreamerEnv& env );
#else
    C2EventUpdater();
#endif
	virtual ~C2EventUpdater();

public:
	
	void	setSinker(C2EventSinkerPtr sinker, int mask );
	
	void	post( C2EventPtr event );
	
	bool	start(  );
	
	void	stop();
protected:
	
	int		run();
		
private:
#ifndef ZQ_CDN_UMG
	C2StreamerEnv&							mEnv;
#endif
	typedef std::list<C2EventPtr>			EVENTS;
	EVENTS									mC2Events;
	bool									mbQuit;
	
	ZQ::common::Mutex						mMutex;
	ZQ::common::Semaphore					mSema;
	
	int										mEventMask;
#ifndef _C2_STREAMER_OVER_HTTP_	
	C2EventSinkerPtr						mEventSinker;
#else
	
#endif

};

}//namespace C2Streamer

#endif//_tianshan_cdnss_c2streamer_event_updater_header_file_h__
