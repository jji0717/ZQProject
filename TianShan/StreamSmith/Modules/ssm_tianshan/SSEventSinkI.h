#ifndef _ZQ_STREAMSMITH_EVENTSINK_H__
#define	_ZQ_STREAMSMITH_EVENTSINK_H__


#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <TsStreamer.h>
#include <TsEvents.h>

#include <StreamSmithModuleEx.h>

#include <NativeThread.h>

namespace ZQ
{
namespace StreamSmith
{

class EventSinkEventChannel:public ZQ::common::NativeThread
{
public:
	EventSinkEventChannel(IPlaylistManager* pManager,ZQADAPTER_DECLTYPE  adpater);
	~EventSinkEventChannel();
	void	registerEventSink();
	void	setTopicProxyString(std::string strTopicProxy){mStrTopicProxy=strTopicProxy;}
	bool	dispatchEvent(DWORD eventType,ZQ::common::Variant& params);
	int		run();
	void	NotifyBadConnection();
protected:

	bool				connectToEventChannel();
	Ice::ObjectPrx		getTopic( const std::string& topicName );
	std::string			getPlaylistProxyString( const std::string& playlistId );

	bool			sendSessionProgressEvent( ZQ::common::Variant& params );
	bool			sendItemStepEvent( ZQ::common::Variant& params );
	bool			sendEndOfStreamEvent( ZQ::common::Variant& params );
	bool			sendBeginOfStreamEvent( ZQ::common::Variant& params );
	bool			sendScaleChangeEvent( ZQ::common::Variant& params );
	bool			sendStateChangeEvent( ZQ::common::Variant& params );
	bool			sendPlaylistDestroyEvent( ZQ::common::Variant& params );
	bool			sendPauseTimeoutEvent( ZQ::common::Variant& params );
	bool			sendRepositionEvent( ZQ::common::Variant& params );
	
private:
	std::string					mStrTopicProxy;
	bool						mbNetworkOK;
	bool						mbQuit;
	IPlaylistManager*			mpPlManager;
	ZQADAPTER_DECLTYPE			mobjAdapter;
	
	TianShanIce::Streamer::PlaylistEventSinkPrx			mPlaylistEventPrx;
	TianShanIce::Streamer::StreamEventSinkPrx			mStreamEventPrx;
	TianShanIce::Streamer::StreamProgressSinkPrx		mProgressEventPrx;
	TianShanIce::Events::GenericEventSinkPrx			mPauseTimeoutEventPrx;
	TianShanIce::Events::GenericEventSinkPrx			mRepostionEventPrx;


	IceStorm::TopicManagerPrx							mTopicManagerPrx;
	HANDLE												mhBadConn;
};

}}//namespace


#endif//_ZQ_STREAMSMITH_EVENTSINK_H__