// MRTStreamService.h: interface for the MRTStreamService class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MRTSTREAMSMITH_H__69FFD043_BFCA_48F8_8FAC_3B414E227B07__INCLUDED_)
#define AFX_MRTSTREAMSMITH_H__69FFD043_BFCA_48F8_8FAC_3B414E227B07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ZQ_common_conf.h"
#include <TsStreamer.h>
#include "StreamSmithAdmin.h"
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <IceStorm/IceStorm.h>
#include <Locks.h>
#include "NativeThread.h"
#include "NativeThreadPool.h"
#include <map>
#include "SystemUtils.h"
#include <TianShanIceHelper.h>
#include "TianShanDefines.h"
#include "MRTClient.h"
#include "MRTDef.h"

extern int	pauseMax;
extern int pauseMin;
extern void tempPause();

class DBStore;
class FailOverFactory;
class MRTProxy;

//class MRTStream:public TianShanIce::Streamer::PlaylistEx ,public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class MRTStream:public TianShanIce::Streamer::PlaylistEx ,public ICEAbstractMutexRLock
{
public:
	MRTStream( Ice::ObjectAdapterPtr adapter, MRTProxy* service, int targettime = 1000);
	~MRTStream();		

	void  onTimer( const ::Ice::Current& = ::Ice::Current() );

	void allotPathTicket(const ::TianShanIce::Transport::PathTicketPrx&, const ::Ice::Current& = ::Ice::Current()) ;

	void destroy(const ::Ice::Current& = ::Ice::Current()) ;
	void destroy2(::TianShanIce::Properties& feedback, const ::Ice::Current& c);

	::std::string lastError(const ::Ice::Current& = ::Ice::Current()) const ;


	::Ice::Identity getIdent(const ::Ice::Current& = ::Ice::Current()) const;


	void setConditionalControl(const ::TianShanIce::Streamer::ConditionalControlMask&, const ::TianShanIce::Streamer::ConditionalControlPrx&, const ::Ice::Current& = ::Ice::Current());


	bool play(const ::Ice::Current& = ::Ice::Current()) ;


	bool setSpeed(::Ice::Float, const ::Ice::Current& = ::Ice::Current()) ;


	bool pause(const ::Ice::Current& = ::Ice::Current()) ;


	bool resume(const ::Ice::Current& = ::Ice::Current()) ;


	::TianShanIce::Streamer::StreamState getCurrentState(const ::Ice::Current& = ::Ice::Current()) const ;


	::TianShanIce::SRM::SessionPrx getSession(const ::Ice::Current& = ::Ice::Current()) ;


	void setMuxRate(::Ice::Int, ::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;


	bool allocDVBCResource(::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;

	::Ice::Long seekStream(::Ice::Long, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;

	::std::string getId(const ::Ice::Current& = ::Ice::Current()) const ;

	bool getInfo(::Ice::Int, ::TianShanIce::ValueMap&, const ::Ice::Current& = ::Ice::Current()) ;

	::Ice::Int insert(::Ice::Int, const ::TianShanIce::Streamer::PlaylistItemSetupInfo&, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;

	::Ice::Int pushBack(::Ice::Int, const ::TianShanIce::Streamer::PlaylistItemSetupInfo&, const ::Ice::Current& = ::Ice::Current()) ;

	::Ice::Int size(const ::Ice::Current& = ::Ice::Current()) const ;

	::Ice::Int left(const ::Ice::Current& = ::Ice::Current()) const ;

	bool empty(const ::Ice::Current& = ::Ice::Current()) const ;

	::Ice::Int current(const ::Ice::Current& = ::Ice::Current()) const ;    

	void erase(::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;

	::Ice::Int flushExpired(const ::Ice::Current& = ::Ice::Current()) ;

	bool clearPending(bool, const ::Ice::Current& = ::Ice::Current()) ;

	bool isCompleted(const ::Ice::Current& = ::Ice::Current()) ;

	::TianShanIce::IValues getSequence(const ::Ice::Current& = ::Ice::Current()) const ;

	::Ice::Int findItem(::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;

	bool distance(::Ice::Int, ::Ice::Int, ::Ice::Int&, const ::Ice::Current& = ::Ice::Current()) ;

	bool skipToItem(::Ice::Int, bool, const ::Ice::Current& = ::Ice::Current()) ;

	bool seekToPosition(::Ice::Int, ::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current());

	virtual void commit_async(const ::TianShanIce::Streamer::AMD_Stream_commitPtr& commitPtr, const ::Ice::Current& = ::Ice::Current())
	{
		commitPtr->ice_response();
	}

	::TianShanIce::Streamer::PlaylistAttr getAttr(const ::Ice::Current& = ::Ice::Current())
	{
		RLock sync(*this);
		return attr;
	}

	void updateAttr(const ::TianShanIce::Streamer::PlaylistAttr&, const ::Ice::Current& = ::Ice::Current())
	{
		WLock sync(*this);
	}    

	void attachSession(const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current& = ::Ice::Current())
	{
	}   

	void setDestination(const ::std::string&, ::Ice::Int, const ::Ice::Current& = ::Ice::Current())
	{
	}

	void setDestMac(const ::std::string&, const ::Ice::Current& = ::Ice::Current())
	{
	}    

	void setSourceStrmPort(::Ice::Int, const ::Ice::Current& = ::Ice::Current())
	{
	}
	void enableEoT(bool, const ::Ice::Current& = ::Ice::Current())
	{
	}

	::TianShanIce::Streamer::StreamInfo makedummyinfo() const
	{
		::TianShanIce::Streamer::StreamInfo info;
		info.ident = ident;
		info.props["CURRENTPOS"] = "9527";
		info.props["ITEM_CURRENTPOS"] = "9527";
		info.props["TOTALPOS"] = "65536";
		info.props["ITEM_TOTALPOS"] = "65536";
		info.props["SPEED"] = "1.000";
		info.props["USERCTRLNUM"] = "1";
		info.state = TianShanIce::Streamer::stsStreaming;
		return info;
	}

	void playEx(::Ice::Long& i, ::Ice::Float& j, const ::Ice::Current& = ::Ice::Current()) 
	{
		i = 0;
		j = 0.0;
	}

	::TianShanIce::Streamer::StreamInfo playEx(::Ice::Float, ::Ice::Long, ::Ice::Short, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current())
	{
		::TianShanIce::Streamer::StreamInfo streamInfo = makedummyinfo();
		tempPause();
		return streamInfo;
	}

	::TianShanIce::Streamer::StreamInfo pauseEx(const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current())
	{
		::TianShanIce::Streamer::StreamInfo streamInfo = makedummyinfo();
		tempPause();
		return streamInfo;
	}

	void setSpeedEx(::Ice::Float i, ::Ice::Long& j, ::Ice::Float& k, const ::Ice::Current& = ::Ice::Current()) 
	{
		tempPause();
		j=0;
		k=0.0;
	}  
	virtual void setPID(::Ice::Int, const ::Ice::Current& = ::Ice::Current()) 
	{
	}

	::TianShanIce::Streamer::StreamInfo playItem(::Ice::Int UserCtrlNum, ::Ice::Int timeOffset, ::Ice::Short from, ::Ice::Float newSpeed, const ::TianShanIce::StrValues& expectedProps, const ::Ice::Current& c)
	{
		::TianShanIce::Streamer::StreamInfo streamInfo;
		return streamInfo;
	}

	virtual ::TianShanIce::Streamer::PlaylistItemSetupInfoS getPlaylistItems(const ::Ice::Current& = ::Ice::Current()) const
	{
		::TianShanIce::Streamer::PlaylistItemSetupInfoS playIterSetupInfos;
		return playIterSetupInfos;
	}

	::TianShanIce::Properties getProperties(const ::Ice::Current& c) const
	{
		RLock rLock(*this);
		::TianShanIce::Properties props;
		return props;
	}

	void setProperties(const ::TianShanIce::Properties& prop, const ::Ice::Current& c)
	{
		WLock wLock(*this);
	}

	::TianShanIce::SRM::ResourceMap getResources(const ::Ice::Current& /* = ::Ice::Current */) const
	{
		::TianShanIce::SRM::ResourceMap resourceMap;
		resourceMap.clear();
		return resourceMap;
	}
	virtual void play_async(const ::TianShanIce::Streamer::AMD_Stream_playPtr&, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void seekStream_async(const ::TianShanIce::Streamer::AMD_Stream_seekStreamPtr&, ::Ice::Long, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) ;
	virtual void playEx_async(const ::TianShanIce::Streamer::AMD_Stream_playExPtr&, ::Ice::Float, ::Ice::Long, ::Ice::Short, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current());
	virtual void skipToItem_async(const ::TianShanIce::Streamer::AMD_Playlist_skipToItemPtr&, ::Ice::Int, bool, const ::Ice::Current& = ::Ice::Current());
	virtual void seekToPosition_async(const ::TianShanIce::Streamer::AMD_Playlist_seekToPositionPtr&, ::Ice::Int, ::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current());
	virtual void playItem_async(const ::TianShanIce::Streamer::AMD_Playlist_playItemPtr&, ::Ice::Int, ::Ice::Int, ::Ice::Short, ::Ice::Float, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current());
protected:
	void		sendSpeedChange( );
	void		sendStateChange( );
public:
	void	SetID( const std::string& strID )
	{
		_strUID = strID;
	}
	void SetTargetTime(int& time)
	{
		m_targetTime = time;
	}
private:
	bool setupMRTStream(std::string& errorMsg);

private:
	std::string	_strUID;
	Ice::ObjectAdapterPtr				m_Adapter;
	MRTProxy*						m_Service;
	float								m_lastSpeed;
	TianShanIce::Streamer::StreamState	m_lastState;
	int									m_targetTime;
};


#define PURCHASE_CATAGORY	"STREAMINDEX"
class FailOverFactory:public Ice::ObjectFactory
{
public:
	FailOverFactory(Ice::ObjectAdapterPtr adapter,MRTProxy* service)
		: m_Adapter(adapter),
		m_Service( service)
	{;}
	~FailOverFactory(){;}
public:
	Ice::ObjectPtr create(const std::string& strID)
	{
		if( strID == TianShanIce::Streamer::Stream::ice_staticId ())
			return new MRTStream(m_Adapter,m_Service);
		else if (strID == TianShanIce::Streamer::Playlist::ice_staticId())
			return new MRTStream(m_Adapter,m_Service);
		else if (strID == TianShanIce::Streamer::PlaylistEx::ice_staticId()) 
			return new MRTStream(m_Adapter,m_Service);
		else
			return NULL;
	}
	void destroy()
	{

	}
private:
	Ice::ObjectAdapterPtr				m_Adapter;
	MRTProxy*							m_Service;
};
typedef IceUtil::Handle<FailOverFactory>	FailOverFactoryPtr;
class DBStore
{
public:
	DBStore(Ice::ObjectAdapterPtr& objAdapter);
	~DBStore();
public:
	bool				Init(const std::string& dbPath);
	bool				UpdateStreamInstance(TianShanIce::Streamer::StreamPtr& streamPtr,Ice::Identity& id);
	bool				ClearStreamInstance(const Ice::Identity& id);
	bool				ClearAll();
private:
	::Ice::ObjectAdapterPtr					m_pAdpter;
	::Freeze::EvictorPtr					m_DBEvictor;
	::Ice::CommunicatorPtr					m_pCommunicatorPtr;
	::Freeze::ConnectionPtr					m_pConnectionPtr;
	ZQ::common::Mutex						m_dbLocks;	
};

class TimerWatch : public ZQ::common::NativeThread 
{
public:
	struct TimerInfo {
		long long target;
		std::string id;
		bool operator<( const TimerInfo& b ) const { 
			if( target < b.target )
				return true;
			else if (target == b.target )
				return id < b.id;
			else
				return false;
		}
	};
	TimerWatch( MRTProxy* service ):mMRTProxy(service),mbRunning(false){}
	virtual ~TimerWatch(){}

	bool	start( ) ;
	void	stop( ) ;
	void	watch( const std::string& id, long long target) ;
	void	unwatch( const std::string& id );
protected:

	TianShanIce::Streamer::PlaylistExPrx	openStream( const std::string& id);
	int										run( );
private:

	MRTProxy*		mMRTProxy;
	std::set<TimerInfo>	mTimerInfos;
	std::map<std::string,TimerInfo> mInstance2Infos;
	ZQ::common::Mutex	mLocker;
	ZQ::common::Semaphore	mSem;
	bool				mbRunning;
	long long			mNextWakeup;
};

class MRTProxy : public TianShanIce::Streamer::StreamSmithAdmin,
	//public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>,
	public ICEAbstractMutexRLock,
	public ZQ::common::NativeThread
{
public:
	MRTProxy(Ice::ObjectAdapterPtr& Adapter,const std::string& dbPath ,const std::string& netId,const std::vector<std::string>& streamers, const StreamNetIDToMRTEndpoints& mrtEndpintInfos, ZQ::common::Log& log,int target = 1000 );
	~MRTProxy();

	virtual ::TianShanIce::Streamer::StreamPrx createStream(const ::TianShanIce::Transport::PathTicketPrx&, const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Streamer::StreamerDescriptors listStreamers(const ::Ice::Current& = ::Ice::Current());    

	virtual ::std::string getNetId(const ::Ice::Current& = ::Ice::Current()) const 
	{
		RLock sync(*this);
		return mNetId;
	};

	virtual ::std::string getAdminUri(const ::Ice::Current& = ::Ice::Current()) 
	{
		WLock sync(*this);
		return "";
	}

	virtual ::TianShanIce::State getState(const ::Ice::Current& = ::Ice::Current()) 
	{
		RLock sync(*this);
		::TianShanIce::State s = TianShanIce::stInService;
		return s;
	}
	virtual ::std::string ShowMemory(const ::Ice::Current& = ::Ice::Current()) 
	{
		return "";
	}
	virtual ::TianShanIce::Streamer::PlaylistIDs listPlaylists(const ::Ice::Current& = ::Ice::Current()) 
	{
		::TianShanIce::Streamer::PlaylistIDs ids;
		return ids;
	}

	virtual ::TianShanIce::Streamer::PlaylistPrx openPlaylist(const ::std::string& id, const ::TianShanIce::Streamer::SpigotBoards&, bool, const ::Ice::Current& = ::Ice::Current()) 
	{
		try {
			Ice::Identity ident;
			ident.name = id;
			ident.category = PURCHASE_CATAGORY;
			return ::TianShanIce::Streamer::PlaylistPrx::checkedCast( m_Adapter->createProxy(ident) );
		}
		catch( const Ice::ObjectNotExistException& ) {
			return NULL;
		}		
	}

	virtual ::TianShanIce::Streamer::StreamPrx createStreamByResource(const ::TianShanIce::SRM::ResourceMap&, const TianShanIce::Properties &, const ::Ice::Current& = ::Ice::Current());
	virtual void createStreamBatch_async(const ::TianShanIce::Streamer::AMD_StreamSmithAdmin_createStreamBatchPtr&, const ::TianShanIce::Streamer::StreamBatchRequest&, const ::Ice::Current& /* = ::Ice::Current */){}
	virtual ::TianShanIce::Streamer::StreamPrx openStream(const ::std::string&, const ::Ice::Current& = ::Ice::Current())
	{
		return NULL;
	}
	bool UpdateStreamInstance(TianShanIce::Streamer::StreamPtr& streamPtr,
		Ice::Identity& id,
		const TianShanIce::Transport::PathTicketPrx& ticket);

	bool ClearStreamInstance(const Ice::Identity& id);

	void queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdReplicaQuery, const ::std::string&, const ::std::string&, bool, const ::Ice::Current& = ::Ice::Current())
	{
		//collect replicas
		::TianShanIce::Replicas reps;

		std::vector<std::string>::const_iterator it = mStreamers.begin();
		for( ; it != mStreamers.end() ; it ++ )
		{
			::TianShanIce::Replica  rep;
			rep.replicaId				= *it;
			rep.groupId					= mNetId;
			rep.replicaState			= TianShanIce::stInService;

			std::string streamNetId = mNetId + "/" + *it;
			int penalty = geStreamPenalty(streamNetId);

			if(penalty > _maxPenalty)
				rep.replicaState			= TianShanIce::stOutOfService;	
			rep.category				= "streamer";
			reps.push_back(rep);
		}
		amdReplicaQuery->ice_response(reps);
	}


	int		run();

	bool	connectToEventChannel( const std::string& endpoint );

	Ice::ObjectPrx		getTopic( const std::string& topicName );

	void increaseStreamPenalty(const std::string streamNetId)
	{
		ZQ::common::MutexGuard gd(_lockStreamPenaltys);
		std::map<std::string, StreamPenaltyInfo>::iterator itor = _streamPenaltys.find(streamNetId);
		if(itor == _streamPenaltys.end())
		{
			StreamPenaltyInfo streamPenaltyInfo;
			streamPenaltyInfo.penalty = 1;
			streamPenaltyInfo.lastUpdateTimeStamp = ZQ::common::now();
			_streamPenaltys[streamNetId] = streamPenaltyInfo;
		}
		else
		{
			itor->second.penalty+= 1;
			if(itor->second.penalty > _maxPenalty)
				itor->second.penalty = _maxPenalty;
			itor->second.lastUpdateTimeStamp = ZQ::common::now();
		}
	}
	void decreaseStreamPenalty(const std::string streamNetId)
	{
		ZQ::common::MutexGuard gd(_lockStreamPenaltys);
		std::map<std::string, StreamPenaltyInfo>::iterator itor = _streamPenaltys.find(streamNetId);
		if(itor == _streamPenaltys.end())
		{
			StreamPenaltyInfo streamPenaltyInfo;
			streamPenaltyInfo.penalty = 0;
			streamPenaltyInfo.lastUpdateTimeStamp = ZQ::common::now();
			_streamPenaltys[streamNetId] = streamPenaltyInfo;
		}
		else 
		{
			itor->second.penalty = 0;
			itor->second.lastUpdateTimeStamp = ZQ::common::now();
		}
	}

	int geStreamPenalty(const std::string streamNetId)
	{
		int penalty = 0;
		ZQ::common::MutexGuard gd(_lockStreamPenaltys);
		std::map<std::string, StreamPenaltyInfo>::iterator itor = _streamPenaltys.find(streamNetId);
		if(itor!= _streamPenaltys.end())
		{		
			penalty = itor->second.penalty;

			if(penalty > 0)
			{
				int timeStamp = (int)(ZQ::common::now() - itor->second.lastUpdateTimeStamp);

				if(timeStamp > 0)
				{
                   penalty = penalty - timeStamp * 5 / 1000;
				}
			}
		}
		if(penalty < 0)
			penalty = 0;
		return  penalty;
	}

	void setStreamPenalty(int penalty, int maxpenalty)
	{
       _outOfServicePenalty = penalty;
	   _maxPenalty = maxpenalty;
	}
public:
/*	bool setupMRTStream(const std::string& streamID);
	bool getMRTStreamStatus(const std::string& streamID);
	bool teardownMRTStream(const std::string& streamID);
*/
	bool findMRTEnpoint(const std::string& streamNetID, MRTEndpointInfo& mrtInfo)
	{
		ZQ::common::MutexGuard gurad(_lockMrtEndpointInfos);
		bool bret = false;
		if(_mrtEndpintInfos.find(streamNetID) != _mrtEndpintInfos.end())
		{
			mrtInfo = _mrtEndpintInfos[streamNetID];
			bret = true;
		}
		return bret;
	}
	///setup MRT stream时， 根据StreamID，找MRTStream setup所需要信息: AssetName， destIp, destPort, bitrate, srmSessionId，streamNetId
	virtual bool findMRTSetupInfoByStreamId(const std::string& streamId, std::string& AssetName, std::string& destIp, int& destPort, int& bitrate, std::string& srmSessionId, std::string& streamNetId)
	{
		return false;
	}
	///根据StreamID，找到已经setup成功的 MRT SessionId;
	virtual bool findMRTSessionIdByStreamId(const std::string& streamId, std::string& mrtSessionId, std::string& srmSessionId, std::string& streamNetId)
	{
		return false;
	}

	virtual bool updateMRTSessionIdToStore(const std::string& streamId, const std::string& mrtSessionId, const std::string& streamNetId)
	{
		return false;
	}

	ZQ::common::Log& getLogger(){ return _log ;};
private:
	StreamNetIDToMRTEndpoints _mrtEndpintInfos;
	ZQ::common::Mutex _lockMrtEndpointInfos;
private:

	friend class MRTStream;

	TimerWatch											mWatchDog;

	TianShanIce::Streamer::PlaylistEventSinkPrx			mPlaylistEventPrx;
	TianShanIce::Streamer::StreamEventSinkPrx			mStreamEventPrx;
	TianShanIce::Streamer::StreamProgressSinkPrx		mProgressEventPrx;
	TianShanIce::Events::GenericEventSinkPrx			mPauseTimeoutEventPrx;
	TianShanIce::Events::GenericEventSinkPrx			mRepostionEventPrx;
	IceStorm::TopicManagerPrx							mTopicManagerPrx;

	std::string											mStrTopicProxy;

	std::string							mNetId;
	DBStore								_db;
	Ice::ObjectAdapterPtr&				m_Adapter;
	TianShanIce::Streamer::StreamPtr	_stream;
	IceUtil::Handle<FailOverFactory>	_factory;

	bool								_bQuit;

	//HANDLE								_hEvent;
	SYS::SingleObject				_sigleEvent;
	Ice::Long							_nextWakeup;

	typedef struct _tagTimerProperty 
	{
		Ice::Long								_nextWakeup;
		TianShanIce::Transport::PathTicketPrx	_ticketPrx;
	}TimerProperty;
	typedef std::map<std::string,TimerProperty>		TimerMap;
	TimerMap									_map;
	ZQ::common::Mutex							_mapMutex;

	std::vector<std::string>					mStreamers;

	typedef struct
	{
		int penalty;
		int64 lastUpdateTimeStamp;
	}StreamPenaltyInfo;
	std::map<std::string, StreamPenaltyInfo> _streamPenaltys;
	ZQ::common::Mutex _lockStreamPenaltys;

	int _outOfServicePenalty;
	int _maxPenalty;

	ZQ::common::Log& _log;

public:
	int								reportSpigot();
	void setTargetTime(int time) { targetTime = time; }

	//add by lxm for report spigot status
	std::string						listenerEndpoint;
	int								updateInterval;
	int                                            targetTime;
};

class RenewTicket : public ZQ::common::ThreadRequest
{
public:
	RenewTicket(TianShanIce::Transport::PathTicketPrx& prx,Ice::Int renewTime,ZQ::common::NativeThreadPool& pool)
		:ZQ::common::ThreadRequest(pool)
	{
		_prx = prx;
		_renewTime =renewTime;
	}
	~RenewTicket()
	{
	}
	int run()
	{
		try
		{
			if(_prx)
			{
				_prx->renew(_renewTime);
			}
		}
		catch (...) 
		{
		}
		return 1;
	}
	void final(int retcode /* =0 */, bool bCancelled /* =false */)
	{
		delete this;
	}
private:
	TianShanIce::Transport::PathTicketPrx	_prx;
	Ice::Int								_renewTime;
};


class ReplicaUpdater: public ZQ::common::NativeThread
{
public:
	ReplicaUpdater(MRTProxy& svc );
	virtual ~ReplicaUpdater( );

public:

	int		run( );

	void	stop( );

protected:

	int		report( );

private:
	MRTProxy&	mService;
	//HANDLE			mEvent;
	SYS::SingleObject	_replicaUpdaterEvent;
	bool			mbQuit;
};

#endif // !defined(AFX_MRTSTREAMSMITH_H__69FFD043_BFCA_48F8_8FAC_3B414E227B07__INCLUDED_)
