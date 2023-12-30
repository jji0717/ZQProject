
#ifndef _zq_weiwoo_path_manager_streamer_replica_updater_h__
#define _zq_weiwoo_path_manager_streamer_replica_updater_h__


#include <NativeThread.h>
#include <TianShanIce.h>
#include <Locks.h>
#include <list>
#include <map>
#include <string>



namespace ZQTianShan {
namespace AccreditedPath {

class	PathSvcEnv;

class AliveStreamerCollector : public TianShanIce::ReplicaSubscriber , public ZQ::common::NativeThread
{

public:
	typedef IceUtil::Handle< AliveStreamerCollector > Ptr;

	AliveStreamerCollector( PathSvcEnv& environment, bool enableReplicaCheck, int32 updateTimeout);
	virtual ~AliveStreamerCollector( );

public:	
	
	// bool				getAllStreamerReplica( bool bFirst = false );

	bool				start();
	bool				stop( );

	typedef struct _StreamerAttr 
	{
		::std::string		netId;
		::std::string		type;
		::std::string		desc;
		::std::string		ifep;
		Ice::Long			lastUpdate;
		bool				bEnable;
	} StreamerAttr;
	typedef std::map< std::string , StreamerAttr > StreamerAttrMap;

	bool  lookupAliveStreamer(const std::string& streamerNetId, StreamerAttr& streamAttr);
	size_t getAliveStreamers(StreamerAttrMap& streamerMap, bool bCleanExpired=false);

protected: // impl of NativeThread
	virtual int			run(  );

protected: // impl of ReplicaSubscriber
	void				updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr&, const ::TianShanIce::Replicas&, const ::Ice::Current& = ::Ice::Current());

protected:

//	uint32				queryAllStreamer( );

	void				cleanExpired();

//	uint32				updateReplicaInfo( );

//	void				updateStreamerStatusToDb( const std::string& netId , bool bEnable );

private:
	
	bool					_bEnableReplicCheck;
	uint32					_replicaUpdateTimeout; // in msec

	typedef std::list< TianShanIce::Replicas > ReplicaList;

	bool                _bQuit;

	PathSvcEnv&			_env;

	ReplicaList			_replicaInfoAwaitUpdate;
	StreamerAttrMap		_streamAttrMap;
	ZQ::common::Mutex	_lock;
	ZQ::common::Cond	_cond;


	// uint32				mWaitDelta;
	
	///	int32				mCurAction;
	///	 1	---> for expiration check
	///  2  ---> for entire query
	///  3 ---> for update replica information
	/// 0 --> for quit
	//enum {
	//	sscStage_Quit            =0,
	//	sscStage_ExpirationCheck =1,
	//	sscStage_Query,
	//	sscStage_UpdateReplica,
	//} _stage;

};

}}

#endif//_zq_weiwoo_path_manager_streamer_replica_updater_h__
