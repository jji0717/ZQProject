// PenaltyManager.cpp: implementation of the PenaltyManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PenaltyManager.h"
#include "NGODEnv.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define PENALTYLOG _env._fileLog

PenaltyManager::PenaltyManager(NGODEnv& env)
:_env(env)
{
	_bQuit =true;
	_hEvent = CreateEvent ( NULL ,FALSE, FALSE, NULL );
}

PenaltyManager::~PenaltyManager()
{
	Stop ();
}
void	PenaltyManager::Stop( )
{
	if (_hEvent != NULL) 
	{
		_bQuit = TRUE;
		SetEvent (_hEvent);
		waitHandle (1000);
		CloseHandle (_hEvent);
		_hEvent = NULL;
	}
}
void	PenaltyManager::decreasePenalty( const std::string& streamerNetId )
{
	{
		ZQ::common::MutexGuard gd(_locker);
		_streamers.push_back( streamerNetId );
	}
	SetEvent (_hEvent);
}
int PenaltyManager::run ()
{
	_bQuit = false;
	bool bHasPenalty = false;
	bool bLastPenaltyState = false;
	while (TRUE) 
	{
		WaitForSingleObject (_hEvent , INFINITE );
		if (_bQuit ) {	break; }
		
		while( !_bQuit )
		{
			std::string streamerNetId;
			{				
				ZQ::common::MutexGuard gd( _locker );
				if(_streamers.size() <= 0 )
					break;
				streamerNetId = _streamers.front();
				_streamers.pop_front();
			}
			if( streamerNetId.empty())
				break;
			//find all streamer in sop group
			{
				ZQ::common::MutexGuard gd(_env._lockSopMap);
				std::map< std::string , NGOD2::SOPRestriction::SopHolder >& sops = _sopConfig._sopRestrict._sopDatas;
				std::map< std::string , NGOD2::SOPRestriction::SopHolder >::iterator itSop = sops.begin();
				for ( ; itSop != sops.end () ; itSop++ ) 
				{
					NGOD2::SOPRestriction::SopHolder& sopData = itSop->second;				
					std::string strSop = sopData._name;
					std::vector<NGOD2::Sop::StreamerHolder>& streamers = sopData._streamerDatas;				
					std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = streamers.begin ();
					for ( ; itStreamer != streamers.end () ; itStreamer ++ ) 
					{
						if ( itStreamer->_netId == streamerNetId &&  itStreamer->_penalty > 0 ) 
						{							
							itStreamer->_penalty --;
							PENALTYLOG(ZQ::common::Log::L_INFO, CLOGFMT(PenaltyManager, "sop[%s] netId[%s] endpoint[%s] decrease penalty by 1 to [%d]"),
								strSop.c_str (),
								itStreamer->_netId.c_str(),
								itStreamer->_serviceEndpoint.c_str(),
								itStreamer->_penalty);
							if ( itStreamer->_penalty < 0 ) 
							{
								itStreamer->_penalty = 0;
							}

							if ( itStreamer->_penalty <= 0 )
							{
								PENALTYLOG(ZQ::common::Log::L_DEBUG, "PenaltyManager streamer hasn't penalty sop[%s] netId[%s] endpoint[%s]", strSop.c_str (),itStreamer->_netId.c_str(), itStreamer->_serviceEndpoint.c_str());
							}
							else
							{
								PENALTYLOG(ZQ::common::Log::L_DEBUG, "PenaltyManager streamer has penalty sop[%s] netId[%s] endpoint[%s]", strSop.c_str (),itStreamer->_netId.c_str(), itStreamer->_serviceEndpoint.c_str());
							}
						}					
					}
				}
			}
		}		
		if (_bQuit ) {	break; }
	}
	return 1;
}
