
#include <ZQ_common_conf.h>
#include <stdio.h>
#include "C2StreamerEnv.h"
#include "C2SessionManager.h"
#include "C2Session.h"
#include "C2StreamerService.h"
#include <assert.h>

namespace C2Streamer
{

C2SessionManager::C2SessionManager( C2StreamerEnv& env ,C2Service& svc)
:mEnv(env),
mSvc(svc),
mbRunning(true),
mAssetAttrs(5000) // configuration required
{
}

C2SessionManager::~C2SessionManager(void)
{
}

C2SessionPtr C2SessionManager::createSession( const std::string& sessionId )
{
	if(!mbRunning)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2SessionManager,"createSession() service is stopping, no session is allowed to create"));
		return NULL;
	}
	std::string sessId;
	if( sessionId.empty())
		sessId = mEnv.generateSessionId();
	else
		sessId = sessionId;
	
	C2SessionPtr pSess = new C2Session( mEnv , mSvc , sessId );
	assert( pSess != 0 );
	{
		ZQ::common::MutexGuard gd(mMutex);
		mSessions.insert( SessMap::value_type( sessId , pSess ) );
	}
	return pSess;
}

void C2SessionManager::removeSession( const std::string& sessionId )
{
	ZQ::common::MutexGuard gd(mMutex);
	mSessions.erase(sessionId);
}

void C2SessionManager::stop( )
{
	mbRunning = false;
	return ;
// 	SessMap sessions;
// 	{
// 		ZQ::common::MutexGuard gd(mMutex);
// 		sessions = mSessions;
// 	}
// 	SessMap::iterator it = sessions.begin();
// 	for( ; it != sessions.end() ; it++ )
// 	{
// 		it->second->destroy();
// 	}
}

void C2SessionManager::destroySession(const std::string &sessionId)
{
	C2SessionPtr pSess = NULL;
	{
		ZQ::common::MutexGuard gd(mMutex);
		SessMap::iterator it = mSessions.find( sessionId);
		if( it != mSessions.end())
		{
			pSess = it->second;
			mSessions.erase(it);
		}
	}
	if( pSess )
	{
		pSess->destroy();
	}
}

C2SessionPtr C2SessionManager::findSession( const std::string& sessionId ) const
{
	ZQ::common::MutexGuard gd(mMutex);
	SessMap::const_iterator it = mSessions.find( sessionId );
	if( it != mSessions.end() )
		return it->second;
	else
		return NULL;
}

int C2SessionManager::run( )
{
	///FIXME
	///TODO: not implemented yet
	return 0;
}

static const char* file_extnames[] = {"FF","FR","INDEX","0X"};

AssetAttribute::Ptr	C2SessionManager::getAssetAttribute( const std::string& fn ) {
	AssetAttribute::Ptr attr = NULL;
	std::string filename = fn;
	std::string::size_type pos = filename.find_last_of('/');
	if(pos != std::string::npos) {
		filename = filename.substr(pos+1);
	}
	std::string::size_type posExtName = filename.find_last_of('.');
	if ( posExtName != std::string::npos ) {
		std::string extname = filename.substr(posExtName+1);
		bool bFound = false;
		for( size_t i = 0 ; i < sizeof(file_extnames)/sizeof(file_extnames[0]); i++ ) {
			if( strncasecmp( file_extnames[i], extname.c_str(), strlen(file_extnames[i])) ==0 ) {
				bFound = true;
				break;
			}
		}
		filename = filename.substr(0,posExtName);
	}
	filename += ".index";

	ZQ::common::MutexGuard gd(mMutex);
	bool bReqNew = false;
	AssetAttributeMap::iterator it = mAssetAttrs.find( filename );
	if( it == mAssetAttrs.end() ) {
		bReqNew = true;
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(C2SessionManager,"getAssetAttribute() not cached attribute for [%s], create one by using filename[%s]"),
				fn.c_str(), filename.c_str() );
	} else {
		attr = it->second;
		if( attr->expired() ) {
			mAssetAttrs.erase(filename);
			bReqNew = true;
			MLOG(ZQ::common::Log::L_INFO, CLOGFMT(C2SessionManager,"attribute for[%s] is expired, create one by using filename[%s]"),
					fn.c_str(), filename.c_str());
		}
	}
	if(bReqNew ) {
		attr = new AssetAttribute(mEnv,filename);
		mSvc.getCacheCenter().queryAssetAttributes(filename, attr);
	}
	mAssetAttrs[filename] = attr;
	return attr;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///C2ClientManager
C2ClientManager::C2ClientManager( C2StreamerEnv& env )
:mEnv(env)
{
}

C2ClientManager::~C2ClientManager()
{
}

bool C2ClientManager::registerSession( const std::string& clientIp , const std::string& sessId , int64 requestBW )
{	
	ZQ::common::MutexGuard gd(mMutex);	
	ClientMap::iterator it = mClients.find(clientIp);
	if( it == mClients.end() )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2ClientManager,"registerSession() failed to find client[%s] : session[%s] requestBW["FMT64"]"),
			clientIp.c_str() , sessId.c_str() , requestBW );
		return false;
	}

	ClientAttr& cliAttr = it->second;

	// 0 ingress capacity is allowed 
	if( ( cliAttr.capcacity >0 ) &&
		   	( cliAttr.capcacity < cliAttr.usedBW + requestBW ) )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2ClientManager,"registerSession() not enough bandwidth for session[%s] requestBW["FMT64"] , client[%s] capacity["FMT64"]"),
			sessId.c_str() , requestBW , clientIp.c_str() , cliAttr.capcacity );
		return false;
	}
	
	cliAttr.usedBW += requestBW;
	
	sessionAttr sessAttr;
	sessAttr.sessionId	= sessId;
	sessAttr.usedBW		= requestBW;
	cliAttr.sessInfos.insert( SESSIONATTRS::value_type( sessId , sessAttr ) );
	int32 sessCount = (int32)cliAttr.sessInfos.size();

	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(C2ClientManager,"register new session[%s] requestBW["FMT64"]: client[%s] capacity["FMT64"] usedBW["FMT64"] count[%d]"),
		sessId.c_str() , requestBW , clientIp.c_str() , cliAttr.capcacity , cliAttr.usedBW , sessCount );

	return true;
}

void C2ClientManager::unregisterSession( const std::string& clientIp , const std::string& sessId )
{
	ZQ::common::MutexGuard gd(mMutex);
	ClientMap::iterator it = mClients.find(clientIp);
	if( it == mClients.end() )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2ClientManager,"unregisterSession() failed to find client[%s] for session[%s]"),clientIp.c_str() , sessId.c_str());
		return;
	}

	ClientAttr& cliAttr = it->second;
	SESSIONATTRS::iterator itSess = cliAttr.sessInfos.find( sessId );
	if( itSess == cliAttr.sessInfos.end() )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2ClientManager,"unregisterSession() failed to find session[%s] with client[%s]"), sessId.c_str() , clientIp.c_str() );
		return;
	}
	cliAttr.usedBW	-= itSess->second.usedBW;
	cliAttr.usedBW	= MAX(cliAttr.usedBW , 0 );
	cliAttr.sessInfos.erase(sessId);

	int32 sessCount = (int32)cliAttr.sessInfos.size();

	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(C2ClientManager,"unregistered session[%s], client[%s]: capacity["FMT64"] usedBW["FMT64"] count[%d]"),
		sessId.c_str() , clientIp.c_str() , cliAttr.capcacity , cliAttr.usedBW , sessCount );

}

void C2ClientManager::updateClient( const std::string& clientIp , int64 capacity )
{
	bool bChanged = false;
	{
		ZQ::common::MutexGuard gd(mMutex);
		ClientMap::iterator it = mClients.find( clientIp );
		if( it != mClients.end() )
		{
			ClientAttr& attr = it->second;
			bChanged = attr.capcacity != capacity;
			attr.capcacity = capacity;
		}
		else
		{
			bChanged = true;
			ClientAttr attr;
			attr.capcacity	= capacity;
			attr.clientIp	= clientIp;
			mClients.insert( ClientMap::value_type( clientIp , attr ) );
		}
	}
	if( bChanged)
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(C2ClientManager,"updateClient() client[%s] capacity["FMT64"]"),
			clientIp.c_str() , capacity);
}

bool C2ClientManager::getClientAttr( const std::string& clientIp , ClientAttr& attr ) const
{
	ZQ::common::MutexGuard gd(mMutex);
	ClientMap::const_iterator it = mClients.find(clientIp);
	if( it == mClients.end())
	{
		return false;
	}
	else
	{
		attr = it->second;
		return true;
	}
}

void C2ClientManager::getClientsAttr( ClientMap& attrs ) const
{
	ZQ::common::MutexGuard gd(mMutex);
	attrs = mClients;
}

}//namespace C2Streamer

