
#include <ZQ_common_conf.h>
#include <assert.h>
#include <FileSystemOp.h>
#include <TianShanIceHelper.h>
#include "NgodEnv.h"
#include "NgodDatabase.h"
#include "NgodConfig.h"

#ifdef ZQ_OS_MSWIN
#include <io.h>
#endif

namespace NGOD
{


NgodDatabase::NgodDatabase(NgodEnv& env )
:mEnv(env)
{
}

NgodDatabase::~NgodDatabase(void)
{
}

const std::string SessionCategory="NgodSession";

void NgodDatabase::generateDbConfig( const std::string& dbPath )
{
#define MAX_CONTENTS       (100*1000)
#define DEFAULT_CACHE_SIZE (160*1024*1024)
	::std::string dbConfFile = ZQTianShan::Util::fsConcatPath( dbPath, "DB_CONFIG" );

	if ( -1 == ::access(dbConfFile.c_str(), 0))
	{
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NgodDatabase, "initializing %s"), dbConfFile.c_str());
		FILE* f = ::fopen(dbConfFile.c_str(), "w+");
		if (NULL != f)
		{
			::fprintf(f, "set_lk_max_locks %d\n",   MAX_CONTENTS);
			::fprintf(f, "set_lk_max_objects %d\n", MAX_CONTENTS);
			::fprintf(f, "set_lk_max_lockers %d\n", MAX_CONTENTS);
			::fprintf(f, "set_cachesize 0 %d 0\n",	DEFAULT_CACHE_SIZE);
			::fclose(f);
		}
	}
}

void NgodDatabase::updateDbEnvConfig( const std::string& env, const std::string& key, const std::string& value )
{
	static std::string prefix = "Freeze.DbEnv.";

	std::string strProp = prefix+env+"."+key;
	Ice::PropertiesPtr props = mEnv.getCommunicator()->getProperties();

	props->setProperty( strProp , value );
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodDatabase,"updateDbEnvConfig() key[%s] value[%s]"),strProp.c_str() , value.c_str() );
}

void NgodDatabase::updateDbFileConfig( const std::string& env, const std::string& file ,const std::string& key, const std::string& value )
{
	static std::string prefix = "Freeze.Evictor.";

	std::string strProp = prefix+env+"."+file+"."+key;
	Ice::PropertiesPtr props = mEnv.getCommunicator()->getProperties();

	props->setProperty( strProp , value );
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodDatabase,"updateDbFileConfig() key[%s] value[%s]"),strProp.c_str() , value.c_str() );
}

bool NgodDatabase::openDatabase( const std::string& strDbPath , int32 evictorSize )
{
	std::string dbPath = ZQTianShan::Util::fsConcatPath(strDbPath,"ngod");
	if( !FS::createDirectory(dbPath, true) )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodDatabase,"failed to open folder [%s]"), dbPath.c_str() );
		return false;
	}

	//create stream index and group index
	mStreamIdx	= new NGOD::StreamIdx("StreamIdx");
	assert( mStreamIdx != NULL );
	mGroupIdx		= new NGOD::GroupIdx("GroupIdx");
	assert( mGroupIdx != NULL );
	mStreamer2SessIdx	= new NGOD::Streamer2Sess("Streamer2Sess");
	assert(mStreamer2SessIdx != NULL );

	std::vector<Freeze::IndexPtr> index;
	index.push_back(mStreamIdx);
	index.push_back(mGroupIdx);
	index.push_back(mStreamer2SessIdx);

	generateDbConfig( dbPath );

	updateDbEnvConfig(dbPath,"DbPrivate","0");
	updateDbEnvConfig(dbPath ,"DbRecoverFatal",ngodConfig.database.fatalRecover );
	updateDbEnvConfig(dbPath,"CheckpointPeriod",ngodConfig.database.checkpointPeriod);
	updateDbFileConfig(dbPath,SessionCategory,"SaveSizeTrigger",ngodConfig.database.saveSizeTrigger);
	updateDbFileConfig(dbPath,SessionCategory,"SavePeriod",ngodConfig.database.savePeriod);

	try
	{
#if ICE_INT_VERSION / 100 >= 303		
		mSessionEvictor = Freeze::createBackgroundSaveEvictor( mEnv.getObjAdapter(), dbPath, SessionCategory , 0, index);
#else//ICE_INT_VERSION
		mSessionEvictor = Freeze::createEvictor( mEnv.getObjAdapter(), dbPath, SessionCategory, 0, index);
#endif//ICE_INT_VERSION

		mSessionEvictor->setSize( evictorSize );

		mEnv.getObjAdapter()->addServantLocator( mSessionEvictor, SessionCategory);
	}
	catch( const Ice::Exception& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodDatabase,"failed to open database at [%s] due to [%s]"),
			dbPath.c_str() , ex.ice_name().c_str() );
		return false;
	}

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodDatabase,"database opened at [%s] and evictor size[%d]"),
		dbPath.c_str() , evictorSize );
	return true;
}

void NgodDatabase::closeDatabase( )
{
	mStreamIdx	= NULL;
	mGroupIdx		= NULL;
	mSessionEvictor	= NULL;
	mStreamer2SessIdx = NULL;
}

bool NgodDatabase::addSession( const std::string& sessionId , NgodSessionPtr obj )
{
	obj->mIdent.name			= sessionId;
	obj->mIdent.category		= SessionCategory;
	try
	{
		mSessionEvictor->add( obj , obj->mIdent );
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodDatabase,"failed to add object[%s] to database due to [%s]"),
			sessionId.c_str() , ex.ice_name().c_str() );
		return false;
	}
	return true;
}

void NgodDatabase::removeSession( const std::string& sessionId )
{
	Ice::Identity id;
	id.name		= sessionId;
	id.category	= SessionCategory;

	try
	{
		mSessionEvictor->remove( id );
	}
	catch( const Ice::Exception& )
	{
	}
}
NgodSessionPrxS NgodDatabase::openAllSessions( )
{
	NgodSessionPrxS prxs;
	::Freeze::EvictorIteratorPtr tItor = mSessionEvictor->getIterator("", 4096);
	while (tItor->hasNext())
	{
		NgodSessionPrx prx = openSession( tItor->next().name );
		if( prx )
		{
			prxs.push_back(prx);
		}
	}
	return prxs;
}

NgodSessionPrx NgodDatabase::openSession( const std::string& sessionId )
{
	Ice::Identity id;
	id.name			= sessionId;
	id.category		= SessionCategory;
	
	NgodSessionPrx prx  = NULL ;
	try
	{
		prx = NgodSessionPrx::uncheckedCast( mEnv.getObjAdapter()->createProxy(id) );
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodDatabase,"failed to open proxy of [%s] due to [%s]"),
			sessionId.c_str() , ex.ice_name().c_str() );
		return NULL;
	}

	return prx;
}

NgodSessionPrx NgodDatabase::openSessionByStream( const std::string& sessionId )
{
	std::vector<Ice::Identity> ids = mStreamIdx->findFirst( sessionId , 1);
	if( ids.size() <= 0 )
	{
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodDatabase,"failed to open session proxy through stream[%s]"),
			sessionId.c_str() );
		return NULL;
	}
	return openSession( ids[0].name );
}

NgodSessionPrxS NgodDatabase::openSessionByStreamer( const std::string& streamerNetId )
{
	NgodSessionPrxS prxs;

	std::vector<Ice::Identity> ids =  mStreamer2SessIdx->find(streamerNetId);
	
	std::vector<Ice::Identity>::const_iterator it = ids.begin();
	for( ; it != ids.end() ; it ++ )
	{
		NgodSessionPrx prx = openSession( it->name );
		if( prx )
		{
			prxs.push_back(prx);
		}
	}

	return prxs;
}

NgodSessionPrxS NgodDatabase::openSessionByGroup( const std::string& groupId  )
{
	std::vector<Ice::Identity> ids = mGroupIdx->find( groupId );
	NgodSessionPrxS prxs;
	
	std::vector<Ice::Identity>::const_iterator it = ids.begin();
	for( ; it != ids.end() ; it++ )
	{
		NgodSessionPrx prx = openSession( it->name );
		if( prx )
		{
			prxs.push_back(prx);
		}
	}
	return prxs;
}

}//namespace NGOD

