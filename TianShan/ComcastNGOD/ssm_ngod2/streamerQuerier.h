
#ifndef _ZQ_NGOD2_STREAMER_QURIER_HEADER_FILE_H__
#define	_ZQ_NGOD2_STREAMER_QURIER_HEADER_FILE_H__

#include <NativeThread.h>
#include <locks.h>
#include <list>
#include <string>
#include <TianShanIce.h>
#include <streamsmithadmin.h>
#include "PenaltyManager.h"

class NGODEnv;

class streamerReplicaSink : public ZQ::common::NativeThread, public TianShanIce::ReplicaSubscriber
{
public:
	streamerReplicaSink(  NGODEnv&	env ,PenaltyManager* pPenaltyManager);
	~streamerReplicaSink( );
	void		stop( );

	bool		init(void);
	int			run(void);
	void		final(void);

	void		updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr&, const ::TianShanIce::Replicas&, const ::Ice::Current& = ::Ice::Current());

private:

	NGODEnv&		_env;
	int32			_waitDelta;
	HANDLE			_hEvent;
	bool			_bQuit;
	int64			_nextWakeup;
	PenaltyManager* _pPenaltyManager;
};

class streamerQuerier : public ZQ::common::NativeThread 
{
public:
	streamerQuerier( NGODEnv&	env );
	~streamerQuerier();

public:
	void		stop( );

	void		pushStreamer( const std::string& sopName , const std::string& strNetId , const std::string& strEndpoint );


protected:

	typedef struct _tagStreamerInfo 
	{
		std::string			sopName;
		std::string			netId;
		std::string			endpoint;
		_tagStreamerInfo( )
		{
			sopName = "";
			netId = "";
			endpoint = "";
		}
		_tagStreamerInfo( const _tagStreamerInfo& info )
		{
			sopName		= info.sopName;
			netId		= info.netId;
			endpoint	= info.endpoint;
		}

		bool operator ==( const _tagStreamerInfo& info)
		{
			return	info.endpoint	== endpoint &&
					info.netId		== netId	&&
					info.sopName	== sopName;
		}
	}StreamerInfo;


	int			run(void);	

	void		updateStreamer(	const StreamerInfo& info ,  TianShanIce::Streamer::StreamSmithAdminPrx streamerPrx );
	
	void		findAndEraseStreamerInfo( const StreamerInfo& info );

private:

	NGODEnv&		_env;
	HANDLE			_hEvent;
	bool			_bQuit;

	typedef std::list< StreamerInfo >	StreamerList;
	StreamerList				_list;
	ZQ::common::Mutex			_listLocker;
};

#endif //_ZQ_NGOD2_STREAMER_QURIER_HEADER_FILE_H__

