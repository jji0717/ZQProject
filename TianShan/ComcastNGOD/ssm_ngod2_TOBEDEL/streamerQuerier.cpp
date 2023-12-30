
#include "NGODEnv.h"
#include "streamerQuerier.h"
#include <HelperClass.h>
#include <TianShanIceHelper.h>

#define QUERYLOG _env._fileLog

streamerQuerier::streamerQuerier(  NGODEnv&	env  )
:_env(env)
{
	_bQuit	= false;
	_hEvent = CreateEvent( NULL , FALSE , FALSE , NULL );
}

streamerQuerier::~streamerQuerier(void)
{
	stop( );
}

void streamerQuerier::stop( )
{
	if( _bQuit == false )
	{
		_bQuit = true;
		if( _hEvent != NULL )
		{
			SetEvent( _hEvent );
		}
		waitHandle( 2000 );
		CloseHandle( _hEvent );
		_hEvent = NULL;
	}
}
void	streamerQuerier::findAndEraseStreamerInfo( const streamerQuerier::StreamerInfo& info )
{
	StreamerList::iterator it = _list.begin( );
	for ( ; it != _list.end() ; it ++)
	{
		if ( *it == info )
		{
			_list.erase(it);
			return;
		}
	}
}

void streamerQuerier::pushStreamer( const std::string& sopName ,
									const std::string& strNetId , 
									const std::string& strEndpoint )
{
	QUERYLOG(ZQ::common::Log::L_INFO , 
		CLOGFMT(streamerQuerier, "push streamer[%s] with endpoint[%s] into streamerQuerier"),
		strNetId.c_str() ,
		strEndpoint.c_str() );
	StreamerInfo info;
	info.endpoint	= strEndpoint;
	info.netId		= strNetId;
	info.sopName	= sopName;
	{
		ZQ::common::MutexGuard gd(_listLocker);
		findAndEraseStreamerInfo(info);
		_list.push_back(info);		
	}
	SetEvent( _hEvent );
}

int streamerQuerier::run( )
{
	while ( !_bQuit )
	{
		WaitForSingleObject( _hEvent , _sopConfig._sopRestrict._streamerQueryInterval );
		if(_bQuit)	break;

		StreamerList tmplist;
		{
			ZQ::common::MutexGuard gd(_listLocker);
			tmplist = _list;
		}
		while ( 1 )
		{
			StreamerInfo info;
			{
				if ( tmplist.size() <= 0 )break;
				info = tmplist.front();
				tmplist.pop_front();
			}
			TianShanIce::Streamer::StreamSmithAdminPrx streamerPrx = NULL;
			if( info.netId.empty() || info.endpoint.empty() )
				continue;
			try
			{
				QUERYLOG(ZQ::common::Log::L_INFO , CLOGFMT(streamerQuerier , "try to connect to streamer[%s] with endpoint[%s]"),
					info.netId.c_str() ,
					info.endpoint.c_str( ) );
				streamerPrx	= TianShanIce::Streamer::StreamSmithAdminPrx::checkedCast( _env._pCommunicator->stringToProxy( info.endpoint ) );
				if( streamerPrx )
				{
					updateStreamer( info , streamerPrx );
					QUERYLOG(ZQ::common::Log::L_INFO , 
						CLOGFMT(streamerQuerier,"connect to streamer[%s] with endpoint[%s] OK"),
						info.netId.c_str() ,
						info.endpoint.c_str() );
				}
				else
				{
					QUERYLOG(ZQ::common::Log::L_ERROR , 
						CLOGFMT(streamerQuerier,"Can't connect to streamer[%s] with endpoint[%s]"),
						info.netId.c_str() ,
						info.endpoint.c_str() );
					findAndEraseStreamerInfo(info);
					{
						ZQ::common::MutexGuard gd(_listLocker);
						_list.push_back(info);
					}					
				}
			}
			catch ( const  Ice::Exception& ex )
			{
				QUERYLOG(ZQ::common::Log::L_ERROR , 
					CLOGFMT(streamerQuerier,"Can't connect to streamer[%s] with endpoint[%s] with Ice Exception:[%s]"),
					info.netId.c_str() ,
					info.endpoint.c_str() ,
					ex.ice_name().c_str() );
				findAndEraseStreamerInfo(info);
				{
					ZQ::common::MutexGuard gd(_listLocker);
					_list.push_back(info);
				}					
			}
		}
		
	}
	return 1;
}

void	streamerQuerier::updateStreamer( const StreamerInfo& info ,  
										TianShanIce::Streamer::StreamSmithAdminPrx streamerPrx )
{
	{
		ZQ::common::MutexGuard gd(_env._lockSopMap);

		std::map< std::string , NGOD2::SOPRestriction::SopHolder >& sops = _sopConfig._sopRestrict._sopDatas;
		std::map< std::string , NGOD2::SOPRestriction::SopHolder >::iterator itSop = sops.find(info.sopName);
		if ( itSop != sops.end() )
		{
			NGOD2::SOPRestriction::SopHolder& sopData = itSop->second;			
			std::vector<NGOD2::Sop::StreamerHolder>& streamers = sopData._streamerDatas;				
			std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = streamers.begin ();		
			for ( ; itStreamer != streamers.end() ; itStreamer ++ )
			{
				if ( itStreamer->_netId == info.netId )
				{
					QUERYLOG(ZQ::common::Log::L_INFO ,
						CLOGFMT(streamerQuerier,"update streamer proxy to streamer[%s] with endpoint[%s]"),
						info.netId.c_str() ,
						info.endpoint.c_str() );
					itStreamer->_streamServicePrx	= streamerPrx;
					break;
				}
			}
		}
	}
	{
		ZQ::common::MutexGuard gd( _listLocker );
		findAndEraseStreamerInfo(info);
	}
}

//////////////////////////////////////////////////////////////////////////
#define SINKLOG _env._fileLog
streamerReplicaSink::streamerReplicaSink( NGODEnv& env ,PenaltyManager* pPenaltyManager)
:_env(env)
{
	_bQuit = false;
	_hEvent = NULL;
	_waitDelta	=	10 * 1000;
	_pPenaltyManager = pPenaltyManager;
}
streamerReplicaSink::~streamerReplicaSink( )
{

}
bool streamerReplicaSink::init()
{
	if( _hEvent )
	{
		CloseHandle( _hEvent );
		_hEvent = NULL;
	}
	_hEvent	=	CreateEvent( NULL, FALSE , FALSE ,NULL	);
	return _hEvent != NULL;
}

void streamerReplicaSink::final()
{
	if( _hEvent )
	{
		CloseHandle(_hEvent);
		_hEvent = NULL;
		_bQuit = true;
	}
}

void streamerReplicaSink::stop()
{
	if( !_bQuit )
	{
		_bQuit = true;
		SetEvent( _hEvent );
		waitHandle(5000);
	}
}

int streamerReplicaSink::run( )
{
	return 1;
	//Leave Resource manager to check the status of streamer replica

	if( _sopConfig._sopRestrict._replicaUpdateEnable < 1 )
	{
		SINKLOG(ZQ::common::Log::L_WARNING,
			CLOGFMT(streamerReplicaSink,"disable check streamer replica info, quit replica update timeout check routine"));
		return 1;
	}
	SINKLOG(ZQ::common::Log::L_INFO,CLOGFMT(streamerReplicaSink,"Start sink streamer replica report"));
	
	int64 cur = ZQTianShan::now();
	int32 updateInterval = _sopConfig._sopRestrict._replicaUpdateInterval  * 1000;
	_nextWakeup = cur +	_waitDelta;

	while( !_bQuit )
	{
		WaitForSingleObject( _hEvent , _waitDelta );
		
		_waitDelta = 60 * 60 * 1000; //set wait delta time to 1 hour
		cur = ZQTianShan::now( );

		if( _bQuit ) break;
		//calculate waitDelta
		{
			ZQ::common::MutexGuard gd(_env._lockSopMap);
			std::map< std::string , NGOD2::SOPRestriction::SopHolder >& sops = _sopConfig._sopRestrict._sopDatas;
			std::map< std::string , NGOD2::SOPRestriction::SopHolder >::iterator itSop = sops.begin();

			for( ; itSop != sops.end() ; itSop ++ )
			{
				NGOD2::SOPRestriction::SopHolder& sopData = itSop->second;			
				std::vector<NGOD2::Sop::StreamerHolder>& streamers = sopData._streamerDatas;				
				std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = streamers.begin ();		
				for ( ; itStreamer != streamers.end() ; itStreamer ++ )
				{
					if(  itStreamer->_bReplicaStatus &&( itStreamer->_lastReplicaUpdate != 0 ) && 
						( ( itStreamer->_lastReplicaUpdate + updateInterval ) < cur ) )
					{
						//this streamer is out-of-date
						itStreamer->_bReplicaStatus = false;
						SINKLOG(ZQ::common::Log::L_ERROR,
							CLOGFMT(streamerReplicaSink,"streamer [%s] is not updated since [%lld],disble it"),
							itStreamer->_netId.c_str() , itStreamer->_lastReplicaUpdate	);
					}
					else
					{
						if(  itStreamer->_lastReplicaUpdate == 0 )
						{
							_waitDelta = 10 * 1000;
						}
						else
						{
							int64 diff = ( itStreamer->_lastReplicaUpdate + updateInterval ) - cur;
							_waitDelta = static_cast<int32>( _waitDelta > diff ? diff : _waitDelta);
						}
					}
				}
				_waitDelta = _waitDelta < 10 ? 10: _waitDelta;
			}
		}

	}
	SINKLOG(ZQ::common::Log::L_INFO,CLOGFMT(streamerReplicaSink,"End sink streamer replica report"));
	return 1;
}

void streamerReplicaSink::updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& callback, 
										  const ::TianShanIce::Replicas& reps, 
										  const ::Ice::Current&)
{
	if( _sopConfig._sopRestrict._replicaUpdateEnable < 1 )
	{
		QUERYLOG(ZQ::common::Log::L_WARNING,CLOGFMT(updateReplica,"disable check streamer replica info, do not modify replica status"));
		try
		{
			callback->ice_response( _sopConfig._sopRestrict._replicaUpdateInterval );
		}
		catch(...)
		{

		}
		return;
	}
	extern NGODEnv ssmNGOD;
	ssmNGOD.mResManager.updateReplica( reps );

// 	typedef std::map< std::string, NGOD2::PassThruStreaming::ImportChannelHolder> ImportChannelMap;
// 	ImportChannelMap& importChannels = _ngodConfig._passThruStreaming._importChannelDatas;
// 
// 	bool bUseReportBandwidth = _sopConfig._sopRestrict._enableReportedImportChannelBandWidth >= 1;
// 
// 	TianShanIce::Replicas::const_iterator itRep = reps.begin( );
// 
// 	
// 
// 	for( ; itRep != reps.end() ; itRep ++ )
// 	{
// 		if( stricmp( itRep->category.c_str() ,"BandwidthUsage" ) == 0 )
// 		{
// 			const TianShanIce::Properties& replicaProps = itRep->props;
// 			const std::string& groupId = itRep->groupId;
// 			TianShanIce::Properties::const_iterator itMaxBandWidth = replicaProps.find("UsedReplicaImportBandwidth");
// 			if( bUseReportBandwidth && ( itMaxBandWidth != replicaProps.end() ) && (!itMaxBandWidth->second.empty()) )
// 			{
// 				int64 usedImportBandwidth = 0;
// 				int64 totalImportBandwidth = 0;
// 				int32 runningImportSessCount = 0;
// 				
// 				//sscanf( itMaxBandWidth->second.c_str(),"%lld",&usedImportBandwidth );
// 				ZQTianShan::Util::getPropertyDataWithDefault( replicaProps, "UsedReplicaImportBandwidth", 0, usedImportBandwidth);
// 				ZQTianShan::Util::getPropertyDataWithDefault( replicaProps, "TotalReplicaImportBandwidth",0, totalImportBandwidth);
// 				ZQTianShan::Util::getPropertyDataWithDefault( replicaProps, "runningImportSessionCount",  0, runningImportSessCount);
// 
// 				ImportChannelMap::iterator itChannel = importChannels.find( groupId  );
// 				if( itChannel == importChannels.end() )
// 				{
// 					NGOD2::PassThruStreaming::ImportChannelHolder holder;							
// 					holder._name				=	groupId;
// 					holder._bandwidth			=	static_cast<int32>(usedImportBandwidth/1000);
// 					holder._maxBandwidth		=	totalImportBandwidth;					
// 					holder._reportUsedBandwidth =	usedImportBandwidth;
// 					holder._reportTotalBandwidth=	totalImportBandwidth;
// 					holder._runningImportSessCount=	runningImportSessCount;
// 					holder._usedBandwidth		=	usedImportBandwidth;
// 					holder._maxImport			=	0x7FFFFFFF;
// 					holder._usedImport			=	0;
// 					holder._bConfiged			=	false;
// 					importChannels.insert(ImportChannelMap::value_type( groupId , holder ) );
// 					QUERYLOG(ZQ::common::Log::L_INFO,
// 						CLOGFMT(updateReplica,"get new replica import channel:name[%s] usedBandwidth[%lld] totalBandwidth[%lld] runningSess[%d] "),
// 						groupId.c_str(), usedImportBandwidth, totalImportBandwidth, runningImportSessCount 	);
// 				}
// 				else
// 				{
// 					if( (usedImportBandwidth != itChannel->second._reportUsedBandwidth) ||
// 						(runningImportSessCount != itChannel->second._runningImportSessCount) ||
// 						(totalImportBandwidth != itChannel->second._reportTotalBandwidth ) )
// 					{
// 						QUERYLOG(ZQ::common::Log::L_INFO,
// 							CLOGFMT(updateReplica,"update replica import channel:name[%s] usedBandwidth[%lld] totalBandwidth[%lld] runningSess[%d] "),
// 							groupId.c_str(), 
// 							usedImportBandwidth,
// 							totalImportBandwidth,
// 							runningImportSessCount);
// 						itChannel->second._reportUsedBandwidth		= usedImportBandwidth;
// 						itChannel->second._reportTotalBandwidth		= totalImportBandwidth;
// 						itChannel->second._runningImportSessCount	= runningImportSessCount;
// 					}
// 				}
// 			}
// 		}
// 		else if( stricmp( itRep->category.c_str() ,"streamer" ) == 0 )
// 		{
// 			QUERYLOG(ZQ::common::Log::L_INFO,
// 				CLOGFMT(updateReplica,"replica information update: category[%s] group[%s] replicaId[%s] status[%s]"),
// 				itRep->category.c_str(),
// 				itRep->groupId.c_str(), 
// 				itRep->replicaId.c_str() ,
// 				( itRep->replicaState == TianShanIce::stInService ) ? "enabled":"disabled" );
// 
// 			{
// 				ZQ::common::MutexGuard gd(_env._lockSopMap);
// 				std::map< std::string , NGOD2::SOPRestriction::SopHolder >& sops = _sopConfig._sopRestrict._sopDatas;
// 				std::map< std::string , NGOD2::SOPRestriction::SopHolder >::iterator itSop = sops.begin();
// 
// 				for( ; itSop != sops.end() ; itSop ++ )
// 				{
// 					NGOD2::SOPRestriction::SopHolder& sopData = itSop->second;			
// 					std::vector<NGOD2::Sop::StreamerHolder>& streamers = sopData._streamerDatas;				
// 					std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = streamers.begin ();		
// 					for ( ; itStreamer != streamers.end() ; itStreamer ++ )
// 					{
// 						//how to construct a net id from replica information
// 						std::string	strReplicaInformation = itRep->groupId+ "/" + itRep->replicaId;						
// #pragma message(__MSGLOC__"TODO: enable replica report with Bandwidth?")
// 
// 						if ( stricmp( itStreamer->_netId.c_str(), strReplicaInformation.c_str() ) == 0 )
// 						{
// 							bool bLastStatus	= itStreamer->_bReplicaStatus;
// 							itStreamer->_bReplicaStatus	= ( itRep->replicaState == TianShanIce::stInService );
// 							itStreamer->_lastReplicaUpdate = ZQTianShan::now();
// 							if( itStreamer->_bReplicaStatus != bLastStatus )
// 							{
// 								QUERYLOG(ZQ::common::Log::L_INFO,CLOGFMT(updateReplica,
// 									"update sop[%s] streamer[%s]'s status to [%s]"),
// 									itSop->first.c_str(),
// 									itStreamer->_netId.c_str(),
// 									( itStreamer->_bReplicaStatus ? "Enabled" : "disabled" ) );						
// 							}
// 							if( _pPenaltyManager && itStreamer->_bReplicaStatus )//comes a replica information and the specified replica is UP
// 							{								
// 								_pPenaltyManager->decreasePenalty( itStreamer->_netId );
// 							}
// 						}
// 					}
// 				}
// 			}
// 
// 		}
// 		
// 	}
	
	//we must return a timeout value to reporter so that it can report the status again
#pragma message(__MSGLOC__"TODO: not implement yet")
	try
	{
		callback->ice_response( _sopConfig._sopRestrict._replicaUpdateInterval );
	}
	catch(...)
	{

	}
}

