#ifndef __ZQ_TianShan_StreamService_SsEnv_h__
#define __ZQ_TianShan_StreamService_SsEnv_h__

#include <FileLog.h>
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <NativeThreadPool.h>
#include "Scheduler.h"
#include <TsStorage.h>
#include <TianShanDefines.h>


namespace ZQ 
{ 
namespace StreamService
{

/*
Normal playlist就是指server本身支持多个item在同一个channel上面创建session，但是同时呢只有一个session是可以处在streaming状态的，
但是在playlist范围内，server本身不去自动load下一个需要播出的item，这个需要lib来支持
*/
const		Ice::Int			LIB_SUPPORT_NORMAL_PLAYLIST	=	1;

/*
Normal playlist就是指server本身支持多个item在同一个channel上面创建session，但是同时呢只有一个session是可以处在streaming状态的.
在这种模式下的server，lib只需要传入playlist item，server会在适当的时候自动的播放item，无需干预
*/
const		Ice::Int			LIB_SUPPORT_AUTO_PLAYLIST	=	2;

/*
normal stream模式下面，整个操作都是在stream范围的，所有的Item操作都需要转换成为stream操作
*/
const		Ice::Int			LIB_SUPPORT_NORMAL_STREAM	=	3;


class StreamServiceConfig
{
public:
	StreamServiceConfig()
	{
		///set default value to -1 milliseconds so that StreamService will not load next item automatically when current item is ending
		iPreloadTimeInMS	=	-1; 

		///set default value to 0 milliseconds which means do not protect EOT of the stream
		iEOTSize			=	30000;
		
		
		///set playlist timeout interval to -1 milliseconds so that the playlist is not dead until client destroy it
		iPlaylistTimeout	=	30*60*1000;

		//expired on pause if set to 1
		iExpireOnPause		=	0;

		///default 30min for renew ticket
		iRenewTicketInterval=	30* 60 *1000;

		iSupportPlaylist	=	LIB_SUPPORT_NORMAL_PLAYLIST;

		iClusterId			=	0;

		iUseMemoryDB		=	0;

		iEnableSGStateChangeEvent = 1;

		iEnableSGScaleChangeEvent = 1;
		
		iPassSGStateChangeEvent	= 0;

		iPassSGScaleChangeEvent = 0;

		iEventDiscardThrehold	= 100 * 1000;

		iEventDetainMaxInterval	= 60 * 1000;
		
		iMaxPendingRequestSize	= 100;

		iMaxPlaylistItemCount	 = 20;

		iPerRequestedInterval   = 1000;
	}

	~StreamServiceConfig()
	{
	}

public:
	/// preload time in milliseconds
	/// this indicate how many milliseconds before current session is end ,
	/// StreamSmith will load the next item
	/// if this value  < 0 , StreamSmith will load next item immediately after current item is loaded
	/// if this value  == 0  , StreamSmith will load next item when current item session is gone
	/// if this value >0 , StreamSmith will load next item iPreloadTimeInMS ms  before current item session is gone
	Ice::Int		iPreloadTimeInMS;


	/// end of tail protected area size in milliseconds
	/// if session is running into this area , it will be force to normal speed , (reverse streaming is not limited)
	/// actually this config is used in PWE  to prevent stream run to the end of stream but the uploading is in processing
	Ice::Int		iEOTSize;

	///playlist timeout interval in milliseconds
	Ice::Int		iPlaylistTimeout;

	Ice::Int		iExpireOnPause;

	///current service support playlist or not
	///if support playlist , Service lib will calculate a time interval to load the next item
	/// iSupportPlaylist > 0  ? "support" :"not support"
	Ice::Int		iSupportPlaylist;


	Ice::Int		iRenewTicketInterval;

	///service endpoint
	std::string		serviceEndpoint;

	Ice::Int		iClusterId;

	Ice::Int		iUseMemoryDB;
	//set to 1 if you want to enable service library self generated state changed event
	//default is 1
	Ice::Int		iEnableSGStateChangeEvent;

	//set to 1 if you want to enable service library self generated scale changed event
	//default is 1
	Ice::Int		iEnableSGScaleChangeEvent;
	
	
	///default is 0
	Ice::Int		iPassSGStateChangeEvent;

	///default is 0
	Ice::Int		iPassSGScaleChangeEvent;

	//
	Ice::Int		iEventDiscardThrehold;

	Ice::Int		iEventDetainMaxInterval;
	
	Ice::Int		iMaxPendingRequestSize;

	Ice::Int		iMaxPlaylistItemCount;

	Ice::Int        iPerRequestedInterval;
};




class SsEnvironment
{
public:
	SsEnvironment( ZQ::common::Log& mainLog, ZQ::common::Log& sessLog , ZQ::common::NativeThreadPool& pool );
	virtual ~SsEnvironment(  );

	bool				init( );

	void				uninit( );

public:

	inline Ice::CommunicatorPtr		getCommunicator( ) 
	{
		return iceCommunicator;
	}
	
	//inline Ice::ObjectAdapterPtr	getMainAdapter( ) 
	ZQADAPTER_DECLTYPE				getMainAdapter( ) 
	{
		return mainAdapter;
	}	

	inline ZQ::common::Log&			getMainLogger( ) 
	{
		return mainLogger;
	}
	inline ZQ::common::Log&			getSessionScanLogger( ) 
	{
		return sessLogger;
	}
	inline ZQ::common::NativeThreadPool& getMainThreadPool( ) 
	{
		return mainThreadPool;
	}
	inline ZQ::common::NativeThreadPool& getRenewTicketThreadPool( ) 
	{
		return mRenewTicketThreadpool;
	}

	inline StreamServiceConfig&		getConfig( )
	{
		return streamsmithConfig;
	}
	inline Scheduler&				getSceduler( )
	{
		return mainScheduler;
	}
	inline ZQ::common::Cond&		getReplicaCond( )
	{
		return mStreamerReplicaReportCond;
	}

public:
	StreamServiceConfig							streamsmithConfig;
	Ice::CommunicatorPtr						iceCommunicator;
	//Ice::ObjectAdapterPtr						mainAdapter;
	ZQADAPTER_DECLTYPE							mainAdapter;

protected:	
	
	ZQ::common::Log&							mainLogger;
	ZQ::common::Log&							sessLogger;	
	ZQ::common::NativeThreadPool&				mainThreadPool;
	ZQ::common::NativeThreadPool				mRenewTicketThreadpool;
	Scheduler									mainScheduler;
	ZQ::common::Cond							mStreamerReplicaReportCond;

};

extern SsEnvironment*	gSsEnvironment;

inline SsEnvironment*	getSsEnv( )
{
	return	gSsEnvironment;
}

///

#define LOGCATAGORY	"StreamService"

#define ENVLOG	( (env->getMainLogger()) )
#define SESSLOG	( (env->getSessionScanLogger()) )

#define GETOBJECT(type , id) type::checkedCast(env->getMainAdapter()->createProxy(id))
#define UCKGETOBJECT(type , id) type::uncheckedCast(env->getMainAdapter()->createProxy(id))



const char*	const	EVICTOR_NAME_STREAM		= "STREAM";

const char* const	STREAMSERVICEDBFOLDER	= "Streams";

//define event sink type 
const Ice::Int	ICE_EVENT_TYPE_PROGRESS			= 1000 ;
const Ice::Int	ICE_EVENT_TYPE_ITEMSETPPED		= 2000 ;
const Ice::Int	ICE_EVENT_TYPE_ENDOFSTREAM		= 2100 ;
const Ice::Int	ICE_EVENT_TYPE_BEGINOFSTREAM	= 2200;
const Ice::Int	ICE_EVENT_TYPE_SPEEDCHANGE		= 3000;
const Ice::Int	ICE_EVENT_TYPE_STATECHANGE		= 4000;
const Ice::Int	ICE_EVENT_TYPE_PLAYLISTDESTROY	= 5000;
const Ice::Int	ICE_EVENT_TYPE_GENERICEVENT		= 6000;


//define parameter name for each event type

//for ICE_EVENT_TYPE_PROGRESS
const char*	const	EVENT_PROGRESS_ITEM_PROGRESS_DONE		= "EventProgress-ItemProgressCurrent" ;
const char* const	EVENT_PROGRESS_ITEM_PROGRESS_TOTAL		= "EventProgress-ItemProgressTotal" ;
const char* const	EVENT_PROGRESS_ITEM_STEP_CURRENT		= "EventProgress-ItemStepCurrent" ;
const char* const	EVENT_PROGRESS_ITEM_STEP_TOTAL			= "EventProgress-ItemStepTotal" ;
const char* const	EVENT_PROGRESS_ITEM_CTRL_NUM			= "EventProgress-ItemCtrlNum";
const char* const	EVENT_PROGRESS_ITEM_NAME				= "EventProgress-ItemName";
const char* const	EVENT_PROGRESS_ITEM_EVENT_SEQ			= "EventProgress-ItemEventSequence";

//for ICE_EVENT_TYPE_ITEMSETPPED
const char*	const	EVENT_ITEMSTEP_PREV_ITEM_NAME			= "EventStep-ItemPreviousName";
const char* const	EVENT_ITEMSTEP_NEXT_ITEM_NAME			= "EventStep-ItemNextName";
const char* const	EVENT_ITEMSTEP_TIME_STAMP_UTC			= "EventStampUTC";
const char* const	EVENT_ITEMSTEP_PREV_ITEM_PID			= "previousProviderId";
const char* const	EVENT_ITEMSTEP_PREV_ITEM_PAID			= "previousProviderAssetId";
const char* const	EVENT_ITEMSTEP_CUR_ITEM_PID				= "currentProviderId";
const char* const	EVENT_ITEMSTEP_CUR_ITEM_PAID			= "currentProviderAssetId";
const char* const	EVENT_ITEMSTEP_PREV_STREAMSOURCE		= "PrevItemStreamingSource";
const char* const	EVENT_ITEMSTEP_CUR_STREAMSOURCE			= "CurItemStreamingSource";
const char* const	EVENT_ITEMSTEP_ITEM_PREV_CTRLNUM		= "EventStep-ItemPreviousCtrlNum";
const char* const	EVENT_ITEMSTEP_ITEM_CUR_CTRLNUM			= "EventStep-ItemCurrentCtrlNum";
const char* const	EVENT_ITEMSTEP_ITEM_CUR_TIMEOFFSET		= "currentTimeOffset";
const char* const	EVENT_ITEMSTEP_CLSUTERID				= "mediaclusterId";
const char* const	EVENT_ITEMSTEP_EVENT_SEQ				= "EventSequence";
const char* const   EVENT_ITEMSTEP_EVENT_CURFLAG            = "StreamingCurrentFlag";
const char* const   EVENT_ITEMSTEP_EVENT_PREVFLAG           = "StreamingPreviousFlag";
const char* const   EVENT_ITEMSTEP_EVENT_CURPLAYTIME        = "StreamingCurrentPlayTime";
const char* const   EVENT_ITEMSTEP_EVENT_PREVPLAYTIME       = "StreamingPreviousPlayTime";
const char* const   EVENT_ITEMSTEP_EVENT_PREREQUESTED       = "perRequested";
//for ICE_EVENT_TYPE_ENDOFSTREAM and ICE_EVENT_TYPE_BEGINOFSTREAM
const char* const	EVENT_STREAM_COMPLETE_EVENT_SEQ			= "EventSequence";
const char* const	EVENT_STREAM_COMPLETE_TIMEOFFSET		= "StreamingTotalDuration";
const char* const   EVENT_STREAM_VIDEOTOTALDURATION         = "StreamingTotalVideoDuration";
const char* const   EVENT_STREAM_PREV_PLYPOSPRIMARY         = "StreamPrevItemNptPrimary";
const char* const   EVENT_STREAM_NPT_POS                    = "streamNptPosition";

//for ICE_EVENT_TYPE_SPEEDCHANGE
const char* const	EVENT_SPEEDCHANGE_PREV_SPEED			= "prevSpeed";
const char* const	EVENT_SPEEDCHANGE_CUR_SPEED				= "scale";
const char* const	EVENT_SPEEDCHANGE_CUR_ITEM_NAME			= "EventSpeedChange_CurrentItemName";
//const char* const	EVENT_SPEEDCHANGE_CUR_CTRLNUM			= "userCtrlNumber";
const char* const	EVENT_SPEEDCHANGE_EVENT_SEQ				= "EventSequence";
const char* const   EVENT_SPEEDCHANGE_EVENT_PREREQUESTED    = "perRequested";
//for ICE_EVENT_TYPE_STATECHANGE
const char* const	EVENT_STATECHANGE_PREV_STATE			= "prevState";
const char* const	EVENT_STATECHANGE_CUR_STATE				= "currentState";
const char* const	EVENT_STATECHANGE_CUR_ITEMANEM			= "EventStateChange_CurrentItemName";
const char* const	EVENT_STATECHANGE_CUR_CTRLNUM			= "userCtrlNumber";
const char* const	EVENT_STATECHANGE_EVENTSEQ				= "EventSequence";
const char* const   EVENT_STATECHANGE_EVENT_PREREQUESTED    = "perRequested";

//for ICE_EVENT_TYPE_PLAYLISTDESTROY
const char* const	EVENT_PLDESTROY_CLUSTERID				= "EventPlDestroyed_ClusterId";
const char* const	EVENT_PLDESTROY_EXITONPLAYING			= "EventPlDestroyed_ExitOnPlaying";
const char* const	EVENT_PLDESTROY_EXITCODE				= "EventPlDestroyed_ExitCode";
const char* const	EVENT_PLDESTROY_EVENT_SEQ				= "EventPlDestroyed_EventSeq";
const char* const	EVENT_PLDESTROY_EXIT_REASON				= "ExitReason";
const char* const	EVENT_PLDESTROY_EXIT_PID				= "EventPlDestroyed_ExitPid";
const char* const	EVENT_PLDESTROY_EXIT_PAID				= "EventPlDestroyed_ExitPAid";
const char* const	EVENT_PLDESTROY_EXIT_STREAMINGSOURCE	= "EventPlDestroyed_StreamingSource";
const char* const	EVENT_PLDESTROY_EVENT_TIMESTAMP			= "EventPlDestroyed_EventTimeStamp";


}}//namespace ZQ::StreamService

#endif // __ZQ_TianShan_StreamService_SsEnv_h__
