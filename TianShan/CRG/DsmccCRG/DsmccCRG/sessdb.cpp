
#include <ZQ_common_conf.h>
#include <FileSystemOp.h>
#include <TianShanIceHelper.h>
#include <gatewayconfig.h>
#include "environment.h"
#include "sessdb.h"

#ifdef ZQ_OS_MSWIN
#	include <io.h>
#endif

#if ICE_INT_VERSION / 100 >= 303
#define CREATEEVICTORAPI Freeze::createBackgroundSaveEvictor
#else
#define CREATEEVICTORAPI Freeze::createEvictor
#endif//ICE_INT_VERSION / 100 >= 303

namespace ZQ { namespace CLIENTREQUEST{

static std::string SESSION_CATEGORY = "GATEWAYSESSION";
static std::string DB_DIRETORY = "gatewaysession";

extern Config::GateWayConfig gwConfig;

SessionDatabase::SessionDatabase( Environment& env )
:mEnv(env)
{
}

SessionDatabase::~SessionDatabase(void)
{
	closeDB();	
}

void touchDbConfigFile( const std::string& dbpath )
{
	//NOT IMPLEMENT
#define MAX_CONTENTS 100000
#define DEFAULT_CACHE_SIZE (160*1024*1024)
	::std::string dbConfFile = ZQTianShan::Util::fsConcatPath( dbpath , "DB_CONFIG" );
	if ( -1 == ::access(dbConfFile.c_str(), 0) )
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionDatabase, "initializing %s"), dbConfFile.c_str());
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

void SessionDatabase::updateIceProperty(Ice::PropertiesPtr iceProperty , 
									  const std::string& key ,
									  const std::string& value )
{
	assert(iceProperty);
	iceProperty->setProperty( key , value );
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SessionDatabase,"updateIceDbProperty() [%100s] = [%s]"), key.c_str() , value.c_str() );	
}

#define FREEZEPROPENV(x,y)	std::string("Freeze.DbEnv.")+x+y
#define FREEZEPROPEVTSTREAM(x,y) std::string("Freeze.Evictor.")+x+"."+SESSION_CATEGORY+y

bool SessionDatabase::openDB(const std::string &dbpath, bool enableClientIdIndex, ZQADAPTER_DECLTYPE objadapter)
{
	closeDB();
	mAdapter = objadapter;

	std::string dbTopPath = ZQTianShan::Util::fsConcatPath(dbpath,DB_DIRETORY);
	if(!FS::createDirectory(dbTopPath,true))
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SessionDatabase,"openDB() failed to open directory [%s]"),dbTopPath.c_str() );
		return false;
	}

	touchDbConfigFile(dbTopPath);	

	Ice::PropertiesPtr iceProperty = mEnv.getIc()->getProperties();
	updateIceProperty( iceProperty, FREEZEPROPENV(dbTopPath,".CheckpointPeriod") ,gwConfig.perfTune.checkpointPeriod );
	updateIceProperty( iceProperty, FREEZEPROPENV(dbTopPath,".DbRecoverFatal" ) , gwConfig.perfTune.dbRecoverFatal );
	updateIceProperty( iceProperty, FREEZEPROPEVTSTREAM(dbTopPath,".SavePeriod"), gwConfig.perfTune.savePeriod );	
	updateIceProperty( iceProperty, FREEZEPROPEVTSTREAM(dbTopPath,".SaveSizeTrigger"),gwConfig.perfTune.saveSizeTrigger);
	updateIceProperty( iceProperty, FREEZEPROPENV(dbTopPath,".DbPrivate") , "0");
	updateIceProperty( iceProperty, FREEZEPROPENV(dbTopPath,".OldLogsAutoDelete" ) , "1" );

	try
	{
		std::vector<Freeze::IndexPtr> indices;
		if(enableClientIdIndex)
		{
			mClientIdx = new TianShanIce::ClientRequest::ClientIdx("ClientIndex");
			indices.push_back(mClientIdx);
		}
		mEvictor = CREATEEVICTORAPI( mAdapter , dbTopPath , SESSION_CATEGORY , 0 , indices );
		mAdapter->addServantLocator( mEvictor , SESSION_CATEGORY);
		
		mEvictor->setSize( gwConfig.perfTune.evictorSize );
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SessionDatabase,"openDB() db opened on [%s] with evictor size [%d]"),
			dbTopPath.c_str() , gwConfig.perfTune.evictorSize );

	}
	catch( const Ice::Exception& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SessionDatabase,"openDB() failed to open database on [%s] due to [%s]"),
			dbTopPath.c_str() , ex.ice_name().c_str() );
		return false;
	}

	return true;
}
void SessionDatabase::closeDB()
{
	if( mAdapter)
	{
		mAdapter = NULL;
	}
	if( !mEvictor )
		return;
	mClientIdx	= NULL;
	mEvictor	= NULL;
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(SessionDatabase,"closeDB() database closed"));
}

std::vector<std::string> SessionDatabase::loadAllSessionIds( )
{
	std::vector<std::string> ids;
	try
	{
		Freeze::EvictorIteratorPtr iter = mEvictor->getIterator("",2048);
		while( iter->hasNext())
		{
			ids.push_back(iter->next().name);
		}
	}
	catch( const Ice::Exception& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SessionDatabase,"loadAllSessionIds() caught [%s] while loading session identities"),
			ex.ice_name().c_str() );
	}
	return ids;
}

TianShanIce::ClientRequest::SessionPrx	SessionDatabase::openSession( const std::string& sessId )
{
	Ice::Identity id;
	id.name = sessId;
	id.category = SESSION_CATEGORY;
	try
	{
		return TianShanIce::ClientRequest::SessionPrx::checkedCast( mAdapter->createProxy(id) );
	}
	catch( const Ice::Exception& )
	{
		return NULL;
	}
	return NULL;
}

std::vector<TianShanIce::ClientRequest::SessionPrx> SessionDatabase::findSessionByClient( const std::string& clientId )
{
	std::vector<TianShanIce::ClientRequest::SessionPrx> sessions;
	if( !mClientIdx )
		return sessions;
	try
	{
		std::vector<Ice::Identity> ids = mClientIdx->find( clientId );
		std::vector<Ice::Identity>::const_iterator it = ids.begin();
		for( ; it != ids.end() ; it++ )
		{
			TianShanIce::ClientRequest::SessionPrx sess = openSession( it->name );
			if( sess )
			{
				sessions.push_back( sess );
			}
		}
	}
	catch(const Ice::Exception& )
	{
	}
	return sessions;
}

bool	SessionDatabase::addSession( TianShanIce::ClientRequest::SessionPtr sess )
{
	Ice::Identity id;
	
	id.name			= sess->sessionId;
	id.category		= SESSION_CATEGORY;
	
	try
	{
		mEvictor->add( sess , id );
	}
	catch( const Ice::Exception& )
	{
		return false;
	}
	return true;
}

void	SessionDatabase::removeSession( const std::string& sessId )
{
	Ice::Identity id;

	id.name			= sessId;
	id.category		= SESSION_CATEGORY;

	try
	{
		mEvictor->remove( id );
	}
	catch( const Ice::Exception& )
	{
	}
}


}}//namespace ZQ::DSMCC