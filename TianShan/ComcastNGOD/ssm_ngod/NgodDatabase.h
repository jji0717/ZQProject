#ifndef _tianshan_ngod_database_header_file_h__
#define _tianshan_ngod_database_header_file_h__

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>


#include "./Ice/StreamIdx.h"
#include "./Ice/GroupIdx.h"
#include "./Ice/ngod.h"
#include "./Ice/Streamer2Sess.h"


namespace NGOD
{

class NgodEnv;

typedef std::vector<NGOD::NgodSessionPrx>	NgodSessionPrxS;

class NgodDatabase
{
public:
	NgodDatabase( NgodEnv& env );
	virtual ~NgodDatabase(void);

public:

	bool						openDatabase( const std::string& strDbPath  , int32 evictorSize );

	void						closeDatabase( );

	bool						addSession( const std::string& sessionId , NgodSessionPtr obj );

	void						removeSession( const std::string& sessionId );
	
	NgodSessionPrx				openSession( const std::string& sessionId );
	NgodSessionPrx				openSessionByStream( const std::string& sessionId );
	NgodSessionPrxS				openSessionByGroup( const std::string& groupId );
	NgodSessionPrxS				openAllSessions( );
	NgodSessionPrxS				openSessionByStreamer( const std::string& streamerNetId );

protected:

	void						generateDbConfig( const std::string& dbPath );

	void						updateDbEnvConfig( const std::string& env, const std::string& key, const std::string& value );
	void						updateDbFileConfig( const std::string& env, const std::string& file ,const std::string& key, const std::string& value );

private:

	NgodEnv&					mEnv;
	Freeze::EvictorPtr			mSessionEvictor;	
	NGOD::StreamIdxPtr			mStreamIdx;
	NGOD::GroupIdxPtr			mGroupIdx;
	NGOD::Streamer2SessPtr		mStreamer2SessIdx;
};

}

#endif//_tianshan_ngod_database_header_file_h__
