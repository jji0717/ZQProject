
#define WIN32_LEAN_AND_MEAN

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include "StreamerReplicaUpdater.h"
#include "PathSvcEnv.h"
#include <TianShanIceHelper.h>

#ifndef MAP_SET
#  define MAP_SET(_MAPTYPE, _MAP, _KEY, _VAL) if (_MAP.end() ==_MAP.find(_KEY)) _MAP.insert(_MAPTYPE::value_type(_KEY, _VAL)); else _MAP[_KEY] = _VAL
#endif // MAP_SET

namespace ZQTianShan {
namespace AccreditedPath {

AliveStreamerCollector::AliveStreamerCollector(PathSvcEnv& environment, bool enableReplicaCheck, int32 updateTimeout)
:_env(environment), _bQuit(false), _bEnableReplicCheck(enableReplicaCheck), _replicaUpdateTimeout(updateTimeout)
{
//	mWaitDelta	=	30 * 1000;
}

AliveStreamerCollector::~AliveStreamerCollector()
{
	stop();
}

bool AliveStreamerCollector::stop( )
{
	{
		ZQ::common::MutexGuard gd(_lock);
		_bQuit = true;
	}

	_cond.signal();
	waitHandle(5000);
	return true;
}

#if 0
bool AliveStreamerCollector::getAllStreamerReplica( bool bFirst )
{
	//get all registered streamer from db
	StreamerAttrMap		streamers;
	{
		ZQ::common::MutexGuard lockMap(_lock);
		//clear all records in memory and reload from db
		mMap.clear( );
		{
			ZQ::common::MutexGuard gd(_env._lockStreamerDict);
			TianShanIce::Transport::StreamerDict::const_iterator itStreamer = _env._pStreamerDict->begin();
			for( ; itStreamer != _env._pStreamerDict->end() ; itStreamer ++ )
			{
				StreamerAttr attr;
				attr.desc	=	itStreamer->second.desc;
				attr.ifep	=	itStreamer->second.ifep;
				attr.netId	=	itStreamer->second.netId;
				attr.type	=	itStreamer->second.type;
				attr.bEnable=	false;
				attr.lastUpdate = ZQTianShan::now( );
				mMap[attr.netId] = attr;
			}

			mCurAction = 2;
			if(bFirst)
				streamers = mMap;
		}
	}
	
	if(bFirst)
	{
		StreamerAttrMap::const_iterator it = streamers.begin( );
		for( ; it != streamers.end() ; it ++ )
		{
			//set it to true if enable replica subscriber
			//set it to false if disable replica subscriber
			updateStreamerStatusToDb( it->first , _bEnableReplicCheck < 1  );
		}	
	}	

	_cond.signal();	

	return true;
}
#endif

void AliveStreamerCollector::updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& callback,
												 const ::TianShanIce::Replicas& reps,
												 const ::Ice::Current& )
{
	{
		ZQ::common::MutexGuard gd(_lock);
		_replicaInfoAwaitUpdate.push_front(reps);
//		mCurAction = 3;
	}

	_cond.signal( );
	callback->ice_response( _replicaUpdateTimeout / 1000 );
}

bool AliveStreamerCollector::start()
{
	//query all streamer's replica information
	if( !ZQ::common::NativeThread::start( ) )
		return false;
	
	ZQ::common::delay(10);//context switch for a while

	// getAllStreamerReplica( true );

	return true;
}

int AliveStreamerCollector::run( )
{
	if (!_bEnableReplicCheck)
	{
		envlog(ZQ::common::Log::L_WARNING, CLOGFMT(AliveStreamerCollector, "replica check disabled, quit thread"));
		return 1;
	}

	uint32	nextSleep = _replicaUpdateTimeout;

	while(!_bQuit)
	{
		{
			// the old way of sleep
			ZQ::common::MutexGuard gd(_lock);
			_cond.wait(_lock, nextSleep );
		}

		int64 stampNow = ZQ::common::now();
		nextSleep = _replicaUpdateTimeout;

		::TianShanIce::Replicas reps;
		{
			ZQ::common::MutexGuard g(_lock);
			if(_bQuit || _replicaInfoAwaitUpdate.size() <= 0 )
				continue;

			reps = _replicaInfoAwaitUpdate.front();
			_replicaInfoAwaitUpdate.pop_front();
		}

		for (TianShanIce::Replicas::iterator itRep = reps.begin(); !_bQuit && itRep != reps.end() ; itRep ++ )
		{
			StreamerAttr sa;
			sa.netId = itRep->groupId + "/" + itRep->replicaId;
			//sa.type  = 
			//	sa.desc =
			//	sa.ifep = 
			sa.lastUpdate = ZQTianShan::now();
			sa.bEnable	  =	(itRep->replicaState == TianShanIce::stInService);

			ZQ::common::MutexGuard g(_lock);
			MAP_SET(StreamerAttrMap, _streamAttrMap, sa.netId, sa);
			nextSleep = 10;
		}

		static int64 stampLastClean =0;
		if (!_bQuit && stampNow > stampLastClean +_replicaUpdateTimeout)
		{
			cleanExpired();
			stampLastClean = stampNow;
		}
	}

	return 0;
}

bool AliveStreamerCollector::lookupAliveStreamer(const std::string& streamerNetId, StreamerAttr& streamAttr)
{
	ZQ::common::MutexGuard gd(_lock);
	StreamerAttrMap::iterator it = _streamAttrMap.find(streamerNetId);
	if (_streamAttrMap.end() == it)
		return false;
	
	streamAttr = it->second;
	return true;
}

size_t AliveStreamerCollector::getAliveStreamers(StreamerAttrMap& streamerMap, bool bCleanExpired)
{
	streamerMap.clear();

	if (bCleanExpired)
		cleanExpired();

	ZQ::common::MutexGuard gd(_lock);
	streamerMap = _streamAttrMap;
	return streamerMap.size();
}

void AliveStreamerCollector::cleanExpired()
{
	Ice::Long	stampExp = ZQTianShan::now() - _replicaUpdateTimeout *2;
	ZQ::common::MutexGuard gd(_lock);
	
	for (StreamerAttrMap::iterator it = _streamAttrMap.begin(); it != _streamAttrMap.end() ; it++ )
	{
		if( it->second.lastUpdate <= stampExp )
		{
			//timeout
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(AliveStreamerCollector, "cleanExpired() streamer[%s] endpoint[%s] expired, exclude it from alive streamers"),
				it->first.c_str(), it->second.ifep.c_str() );

			_streamAttrMap.erase(it);
			// updateStreamerStatusToDb( it->first , false );
			continue;
		}
	}
}

#if 0
uint32 AliveStreamerCollector::updateReplicaInfo()
{
	while ( true )
	{
		::TianShanIce::Replicas reps;
		{
		ZQ::common::MutexGuard g(_lock);
		if( _replicaInfoAwaitUpdate.size() <= 0 )
			break;

		reps = _replicaInfoAwaitUpdate.front();
		_replicaInfoAwaitUpdate.pop_front();

		TianShanIce::Replicas::const_iterator itRep = reps.begin();
		for( ; itRep != reps.end() ; itRep ++ )
		{
			StreamerAttr sa;
			sa.netId = itRep->groupId + "/" + itRep->replicaId;
			//sa.type  = 
			//	sa.desc =
			//	sa.ifep = 
			sa.lastUpdate = ZQTianShan::now();
			sa.bEnable	  =	(itRep->replicaState == TianShanIce::stInService);

			ZQ::common::MutexGuard g(_lock);
			MAP_SET(StreamerAttrMap, _streamAttrMap, sa.netId, sa);
		}
	}

	return cleanExpired();
}


uint32 AliveStreamerCollector::queryAllStreamer( )
{
	StreamerAttrMap streamers;
	{
		ZQ::common::MutexGuard gd(_lock);
		streamers = mMap;
	}
	
	StreamerAttrMap::iterator it = streamers.begin();
	for( ; it != streamers.end() ; it ++ )
	{
		TianShanIce::ReplicaQueryPrx prx = NULL;
		try
		{
			prx = TianShanIce::ReplicaQueryPrx::checkedCast(_env._communicator->stringToProxy(it->second.ifep) );
			if(!prx)
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(AliveStreamerCollector, "queryAllStreamer() can't connect to streamer[%s] with endpoint[%s]"),
					it->first.c_str() ,it->second.ifep.c_str() );
				continue;
			}
		}
		catch( const Ice::Exception& ex )
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(AliveStreamerCollector, "queryAllStreamer() can't connect to streamer[%s] with endpoint[%s] and exception[%s]"),
				it->first.c_str() ,it->second.ifep.c_str() ,ex.ice_name().c_str() );
			continue;
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(AliveStreamerCollector, "queryAllStreamer() can't connect to streamer[%s] with endpoint[%s] and caught unknown exception"),
				it->first.c_str() ,it->second.ifep.c_str() );
			continue;
		}
		
		try
		{
			if(!prx)
			{
				envlog(ZQ::common::Log::L_ERROR,CLOGFMT(AliveStreamerCollector, "queryAllStreamer() can't connect to streamer[%s] with endpoint[%s]"),
					it->first.c_str() ,it->second.ifep.c_str() );
				continue;
			}
			
			std::string groupId = it->second.netId ;
			std::string::size_type pos = groupId.find("/");
			if( pos != std::string::npos )
			{
				groupId = groupId.substr( 0 , pos );
			}			

			TianShanIce::Replicas reps = prx->queryReplicas("Streamer", groupId, true);
			TianShanIce::Replicas::const_iterator itRep = reps.begin();
			for( ; itRep != reps.end() ; itRep ++ )
			{
				std::string repNetId = itRep->groupId + "/" + itRep->replicaId;
				StreamerAttrMap::iterator itStreamer = streamers.find( repNetId );
				if( itStreamer == streamers.end() )
					continue;

				bool	bLastStatus			=	itStreamer->second.bEnable;
				//update streamer replica information
				itStreamer->second.bEnable	=	itRep->replicaState == TianShanIce::stInService;
				itStreamer->second.lastUpdate = ZQTianShan::now();
				
				if( itStreamer->second.bEnable != bLastStatus )
				{
					envlog(ZQ::common::Log::L_INFO,CLOGFMT(AliveStreamerCollector, "queryAllStreamer() update streamer[%s] endpoint[%s]'s status to [%s]"),
						itStreamer->second.netId.c_str( ), itStreamer->second.ifep.c_str() ,
						itStreamer->second.bEnable ?"Enable" :"Disable");

					updateStreamerStatusToDb( itStreamer->second.netId  , itStreamer->second.bEnable );
				}					
			}
		}
		catch( const Ice::Exception& ex)
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(AliveStreamerCollector, "queryAllStreamer() failed to get replica information for streamer[%s] with endpoint[%s] and exception[%s]"),
				it->first.c_str() , it->second.ifep.c_str() ,ex.ice_name().c_str() );
		}
		catch(...)
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(AliveStreamerCollector, "queryAllStreamer() failed to get replica information for streamer[%s] with endpoint[%s] and caught unknown exception"),
				it->first.c_str() , it->second.ifep.c_str() );
		}
	}

	return _replicaUpdateTimeout;
}

void AliveStreamerCollector::updateStreamerStatusToDb( const std::string& netId , bool bEnable )
{
	//get the streamer's information
	ZQ::common::MutexGuard gd( _env._lockStreamerDict );
	StreamerAttrMap::iterator it = mMap.find(netId);
	if(mMap.end()== it)
		return;

	it->second.bEnable = bEnable;
	{
		ZQ::common::MutexGuard lockStreamerDict(_env._lockStreamerDict);
		TianShanIce::Transport::StreamerDict::iterator itStreamer = _env._pStreamerDict->find(netId);
		if (_env._pStreamerDict->end() == itStreamer)
			return;

		TianShanIce::Transport::Streamer dbStreamer = itStreamer->second;
		
		//dbStreamer.privateData[REPLICA_STATUS()] = bEnable ? "Enable" : "Disable";
		Ice::Int streamerEnable = bEnable ? 1 : 0;
		ZQTianShan::Util::updateValueMapData(dbStreamer.privateData, REPLICA_STATUS(), streamerEnable );
		try
		{
			// already locked above: ZQ::common::MutexGuard g(_env._lockStreamerDict);
			Freeze::TransactionHolder txHolder(_env._conn);
			_env._pStreamerDict->put( TianShanIce::Transport::StreamerDict::value_type( dbStreamer.netId , dbStreamer) );
			txHolder.commit();
		}
		catch(...){}
	}
}
#endif //0

}}
