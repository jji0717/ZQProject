// File Name : CRGSessionManager.cpp

#include "CRGSessionManager.h"

#include "OpenVBOConfig.h"

#include "CRGSessionImpl.h"

#include "stroprt.h"

#include "Environment.h"

#include "SelectionResourceManager.h"

// ZQ Common
#include "FileSystemOp.h"

//
#ifdef ZQ_OS_MSWIN
#include <io.h>
#endif

#define INDEXFILENAME(_X) #_X "Idx"

#ifndef MAX_SESSIONS
#  define MAX_SESSIONS		             (20000) //20K
#endif 

#ifndef SESSION_CACHESIZE
#  define SESSION_CACHESIZE	         (16*1024*1000) //16MB
#endif

// session context type
#define SERVANT_TYPE "CRGSessionContext"


namespace EventISVODI5
{

class CRGSessionFactory : public Ice::ObjectFactory
{
public:
	CRGSessionFactory(Environment& env, ZQ::common::Log& fileLog) : _env(env), _fileLog(fileLog)
	{

	}

	virtual ~CRGSessionFactory()
	{

	}

public:
	virtual Ice::ObjectPtr create(const std::string& id)
	{
		if (id == SsmOpenVBO::CRGSession::ice_staticId())
		{
			return new (std::nothrow) CRGSessionImpl(_fileLog, _env);
		}
		return NULL;
	}

	virtual void destroy()
	{

	}

	typedef IceUtil::Handle<CRGSessionFactory> Ptr;

private:
	Environment& _env;
	ZQ::common::Log& _fileLog;
};

//--------------------------------------------------------------------

CRGSessoionManager::CRGSessoionManager(Environment&env, ZQ::common::Log& fileLog, 
									   ::Ice::CommunicatorPtr& pCommunicator, 
									   ZQADAPTER_DECLTYPE& pEventAdapter)
:_env(env), _fileLog(fileLog), _pCommunicator(pCommunicator), _pEventAdapter(pEventAdapter),
_pContextEvtr(NULL), _pFactory(NULL), _pStreamIdx(NULL), _bQuit(false)
{
	_programRootPath = ZQTianShan::getProgramRoot();
	_programRootPath += FNSEPS;
}

CRGSessoionManager::~CRGSessoionManager()
{

}

bool CRGSessoionManager::init()
{
	return true;
}

int CRGSessoionManager::run()
{
	/// update sessions in storage, including update streamer and import channel statistic
	std::string strLastError;
	TianShanIce::Properties metaDatas;
	::Freeze::EvictorIteratorPtr tItor = _pContextEvtr->getIterator("", _openVBOConfig._rtspSession._cacheSize);
	IStreamSmithSite& site = _env.getStreamSmithSite();
	while (!_bQuit && tItor->hasNext())
	{
		Ice::Identity ident = tItor->next();
		TianShanIce::Properties sessionContext;
		SsmOpenVBO::CRGSessionPrx sessionProxy = getSessionContext(ident.name, sessionContext);
		if (NULL == sessionProxy)
			continue;

		if (!pingStreamOfSession(ident.name, metaDatas))
		{
			try
			{
				// destroy stream instance
				sessionProxy->destroy();
			}
			catch (const Ice::Exception&)
			{
			}

			removeSession(ident.name, strLastError, false);
			site.destroyClientSession(ident.name.c_str());
			continue;
		}

		StreamerResourcePara streamerResource;
		streamerResource.identifier = sessionContext[SESSION_META_DATA_STREAMER_SOURCE];
		streamerResource.requestBW = atol(sessionContext[SESSION_META_DATA_USED_BANDWIDTH].c_str());
		streamerResource.bNeedImportChannel = sessionContext[SESSION_META_DATA_IMPORT_CHANNEL_NAME].empty();
		streamerResource.method = "init";
		streamerResource.cseq = "";
		streamerResource.sessionId = ident.name;
		_env.getResourceManager().allocateResource(streamerResource, sessionContext[SESSION_META_DATA_STREAMER_NET_ID]);
		_env.watchSession(ident.name);
		if (NULL != _env.getStreamSmithSite().createClientSession(ident.name.c_str(), sessionContext[SESSION_META_DATA_REQUEST_URL].c_str()))
		{
			_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(CRGSessoionManager, "create client session: [%s] when initialize plug-in"), ident.name.c_str());
		}

	} // end while

	_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(CRGSessoionManager,  "exit sessions checked"));
	return 1;
}

void CRGSessoionManager::final()
{
	_bQuit = true;
}

void CRGSessoionManager::stop()
{
	if (!_bQuit)
	{
		_bQuit = true;
		waitHandle(5000);
	}
}

bool CRGSessoionManager::addSession(const SsmOpenVBO::CRGSessionPtr& sessionPtr, std::string &strLastError)
{
	char szBuf[2048];  // error message buffer
	szBuf[sizeof(szBuf) - 1] = '\0';

	::Ice::ObjectPrx basePrx = NULL;
	try
	{
		basePrx = _pContextEvtr->add(sessionPtr, sessionPtr->ident);
		if (basePrx)
		{
			_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(CRGSessoionManager, "session[%s] added into DB"), sessionPtr->ident.name.c_str());
			return true;
		}
	}
	catch (Freeze::DatabaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "caught %s: %s", ex.ice_name().c_str(), ex.message.c_str());
		strLastError = strLastError;
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "caught %s", ex.ice_name().c_str());
		strLastError = szBuf;
	}
	catch (...)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "caught exception");
		strLastError = szBuf;
	}

	_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "sess[%s] failed to add into DB %s"), sessionPtr->ident.name.c_str(), strLastError.c_str());
	return false;
}

bool CRGSessoionManager::removeSession(const std::string &sessId, std::string& strLastError, bool bUpdateStreamer)
{
	if (sessId.empty())
	{
		strLastError = "Session identifier is empty";
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "%s"), strLastError.c_str());
		return false;
	}

	Ice::Identity ident;
	ident.category = SERVANT_TYPE;
	ident.name = sessId;

	char szBuf[2048];  // error message buffer
	szBuf[sizeof(szBuf) - 1] = '\0';
	CRGSessionImplPtr sessionPtr = NULL;

	try
	{
		Ice::ObjectPtr objectPtr = _pContextEvtr->remove(ident);
		sessionPtr = CRGSessionImplPtr::dynamicCast(objectPtr);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "caught [%s]:%s when remove session[%s] context", ex.ice_name().c_str(), ex.message.c_str(), sessId.c_str());
		strLastError = szBuf;
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "%s"), szBuf);
	}
	catch (const Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "caught [%s] when remove session[%s] context", ex.ice_name().c_str(), sessId.c_str());
		strLastError = szBuf;
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "%s"), szBuf);
	}
	catch (...)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "caught exception when remove session[%s] context", sessId.c_str());
		strLastError = szBuf;
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "%s"), szBuf);
	}

	// decrease streamer statistic 
	return removeSession(sessionPtr, strLastError, bUpdateStreamer);
}

bool CRGSessoionManager::removeSession(CRGSessionImplPtr sessImpl, std::string& strLastError, bool bUpdateStreamer)
{
	if (!sessImpl)
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "NULL sessImpl: %s"), strLastError.c_str());
		return false;
	}

	// decrease streamer statistic 
	if (bUpdateStreamer && sessImpl)
	{
		StreamerResourcePara streamerResource;
		streamerResource.identifier = sessImpl->streamerSource;
		streamerResource.requestBW = atol(sessImpl->metadata[SESSION_META_DATA_USED_BANDWIDTH].c_str());
		streamerResource.bNeedImportChannel = !sessImpl->metadata[SESSION_META_DATA_IMPORT_CHANNEL_NAME].empty();
		streamerResource.method = "destroy";
		streamerResource.cseq = "";
		streamerResource.sessionId = sessImpl->ident.name;
		_env.getResourceManager().releaseResource(streamerResource, sessImpl->streamerNetId);
	}

	return true;
}


void CRGSessoionManager::updateDbEnvConfig( const std::string& env, const std::string& key, const std::string& value )
{
    static std::string prefix = "Freeze.DbEnv.";

    std::string strProp = prefix+env+"."+key;
    Ice::PropertiesPtr props = _pCommunicator->getProperties();

    props->setProperty( strProp , value );
    _fileLog(ZQ::common::Log::L_INFO,CLOGFMT(CRGSessoionManager,"updateDbEnvConfig() key[%s] value[%s]"),strProp.c_str() , value.c_str() );
}

void CRGSessoionManager::updateDbFileConfig( const std::string& env, const std::string& file ,const std::string& key, const std::string& value )
{
    static std::string prefix = "Freeze.Evictor.";

    std::string strProp = prefix+env+"."+file+"."+key;
    Ice::PropertiesPtr props = _pCommunicator->getProperties();

    props->setProperty( strProp , value );
    _fileLog(ZQ::common::Log::L_INFO,CLOGFMT(CRGSessoionManager,"updateDbFileConfig() key[%s] value[%s]"),strProp.c_str() , value.c_str() );
}

bool CRGSessoionManager::openDB(std::string& databasePath, int32 evictorSize)
{
	closeDB();

	if (databasePath.empty())
		_dbPath = _programRootPath + "data" FNSEPS ;
	else
		_dbPath = databasePath;
	
	if (FNSEPC != _dbPath[_dbPath.length()-1])
		_dbPath += FNSEPS;

	_dbPath += "ssm_OpenVBO";

	std::string path = _dbPath + "CRGSess" +FNSEPS;
	FS::createDirectory(path, true);

	::std::string dbConfFile = _dbPath + "Contents" FNSEPS "DB_CONFIG";
	if ( -1 == access(dbConfFile.c_str(), 0))
	{
		_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(CRGSessoionManager, "initializing %s"), dbConfFile.c_str());
		FILE* f = ::fopen(dbConfFile.c_str(), "w+");
		if (NULL != f)
		{
			::fprintf(f, "set_lk_max_locks %d\n",   MAX_SESSIONS);
			::fprintf(f, "set_lk_max_objects %d\n", MAX_SESSIONS);
			::fprintf(f, "set_lk_max_lockers %d\n", MAX_SESSIONS);
			::fprintf(f, "set_cachesize 0 %d 0\n",  SESSION_CACHESIZE);
			::fclose(f);
		}
	}

	try
	{
		_pFactory = new (std::nothrow) CRGSessionFactory(_env, _fileLog);
		if (NULL == _pFactory)
			return false;

		_pCommunicator->addObjectFactory(_pFactory, SsmOpenVBO::CRGSession::ice_staticId());
	}
	catch (Ice::Exception& ex)
	{
		_fileLog(ZQ::common::Log::L_EMERG, CLOGFMT(CRGSessoionManager, "catch [%s] when added object factory into ICE Communicator"), ex.ice_name().c_str());
		return false;
	}

	_pStreamIdx = new (std::nothrow) SsmOpenVBO::StreamIdx(INDEXFILENAME(StreamIdx));

	std::vector<Freeze::IndexPtr> indexs;
	indexs.push_back(_pStreamIdx);

    updateDbEnvConfig(_dbPath + "CRGSess","DbPrivate","0");
    updateDbEnvConfig(_dbPath + "CRGSess" ,"DbRecoverFatal",_openVBOConfig._database._fatalRecover );
    updateDbEnvConfig(_dbPath + "CRGSess","CheckpointPeriod",_openVBOConfig._database._checkpointPeriod);
    updateDbFileConfig(_dbPath + "CRGSess","CRGSessions","SaveSizeTrigger",_openVBOConfig._database._saveSizeTrigger);
    updateDbFileConfig(_dbPath + "CRGSess","CRGSessions","SavePeriod",_openVBOConfig._database._savePeriod);

	try
	{
#if ICE_INT_VERSION / 100 >= 303		
		_pContextEvtr = Freeze::createBackgroundSaveEvictor(_pEventAdapter, databasePath.c_str(), "CRGSessions", 0, indexs);
#else
		_pContextEvtr = Freeze::createEvictor(_pEventAdapter, (_dbPath + "CRGSess").c_str(), "CRGSessions", 0, indexs);
#endif
		_pContextEvtr->setSize(evictorSize);
		_pEventAdapter->addServantLocator(_pContextEvtr, SERVANT_TYPE);
		_fileLog(ZQ::common::Log::L_NOTICE, CLOGFMT(CRGSessoionManager, "successed to created DB CRGSessions"));
	}
	catch (Ice::Exception& ex)
	{
		_fileLog(ZQ::common::Log::L_EMERG, CLOGFMT(CRGSessoionManager, "create freeze evictor, caught [%s]"), ex.ice_name().c_str());
		return false;
	}

    // TODO need to rework	
	// updateSessions(evictorSize); // update sessions context in storage
	return true;
}


SsmOpenVBO::CRGSessionPrx CRGSessoionManager::findSession(const std::string& sessId, 
														  std::string& strLastError, int& statusCode)
{
	if (sessId.empty())
	{
		strLastError = "Session identifier is empty";
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "%s"), strLastError.c_str());
		return NULL;
	}

	char szBuf[2048];  // error message buffer
	szBuf[sizeof(szBuf) - 1] = '\0';

	Ice::Identity ident;
	ident.category = SERVANT_TYPE;
	ident.name = sessId;

	SsmOpenVBO::CRGSessionPrx sessionPrx;
	try
	{
		sessionPrx = SsmOpenVBO::CRGSessionPrx::checkedCast(_pEventAdapter->createProxy(ident));
	}
	catch (const Ice::ObjectNotExistException& ex)
	{
		statusCode = 454;
		snprintf(szBuf, sizeof(szBuf) - 1, "caught [%s] when got session[%s] context proxy", ex.ice_name().c_str(), sessId.c_str());
		strLastError = szBuf;
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "%s"), szBuf);
		return NULL;
	}
	catch (const Ice::Exception& ex)
	{
		statusCode = 500;
		snprintf(szBuf, sizeof(szBuf) - 1, "caught [%s] when got session[%s] context proxy", ex.ice_name().c_str(), sessId.c_str());
		strLastError = szBuf;
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "%s"), szBuf);
		return NULL;
	}

	if (NULL == sessionPrx)
	{
		statusCode = 454;
		snprintf(szBuf, sizeof(szBuf) - 1, "session[%s] context proxy is NULL", sessId.c_str());
		strLastError = szBuf;
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "%s"), szBuf);
		return NULL;
	}

	_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(CRGSessoionManager, "findSession() successed to open session[%s] context"), sessId.c_str());
	return sessionPrx;
}


bool CRGSessoionManager::pingStreamOfSession(const std::string& sessId, TianShanIce::Properties& metaData)
{
	int statusCode;
	std::string strLastError;
	SsmOpenVBO::CRGSessionPrx sessionPrx = findSession(sessId, strLastError, statusCode);
	if (!sessionPrx)
	{
		return false;
	}

	metaData.clear();
	try
	{
		metaData = sessionPrx->getMetaData();
	}
	catch (const Ice::Exception& ex)
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "cauth [%s] when got session properties"), ex.ice_name().c_str());
		return false;
	}

	TianShanIce::Streamer::StreamPrx stream;
	try
	{
		std::string streamerNetID;
		std::string streamID;
		stream = sessionPrx->getStream(streamerNetID, streamID);
		if (NULL == stream)
		{
			_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "failed  to get stream proxy"));
			return false;
		}
	}
	catch (const Ice::Exception& ex)
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "cauth [%s] when got stream proxy"), ex.ice_name().c_str());
		return false;
	}

	try
	{
		stream->ice_ping();
	}
	catch (const Ice::Exception& ex)
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "cauth [%s] when ping stream"), ex.ice_name().c_str());
		return false;
	}
	return true;
}

SsmOpenVBO::CRGSessionPrx CRGSessoionManager::getSessionContext(const std::string& sessId, TianShanIce::Properties& sessionContext)
{
	int statusCode;
	std::string strLastError;
	SsmOpenVBO::CRGSessionPrx sessionProxy = findSession(sessId, strLastError, statusCode);
	if (NULL == sessionProxy)
		return NULL;

	try
	{
		sessionContext = sessionProxy->getMetaData();
		_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(CRGSessoionManager, "getSessionContext() got session[%s] context"), sessId.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "getSessionContext() session[%s] context caught[%s]"), sessId.c_str(), ex.ice_name().c_str());
		return NULL;
	}

	return sessionProxy;

}

void CRGSessoionManager::closeDB()
{
	_pContextEvtr =NULL;
}

int64 CRGSessoionManager::getTotalSessionsNum()
{
	int64 sessionsNum = 0;
	::Freeze::EvictorIteratorPtr tItor = _pContextEvtr->getIterator("", _openVBOConfig._rtspSession._cacheSize);
	while (tItor->hasNext())
	{
		tItor->next();
		sessionsNum++;
	}
	return sessionsNum;
}

void CRGSessoionManager::getSessionlist(std::string& sessionList)
{
	sessionList = "";
	::Freeze::EvictorIteratorPtr tItor = _pContextEvtr->getIterator("", _openVBOConfig._rtspSession._cacheSize);
	while (tItor->hasNext())
	{
		Ice::Identity ident = tItor->next();
		sessionList += ident.name;
		sessionList += " ";
	}
}

::std::vector< Ice::Identity > CRGSessoionManager::findStreams(const ::std::string&uid, Ice::Int index) const
{
	::std::vector< Ice::Identity > idents;
	try
	{
		idents = _pStreamIdx->findFirst(uid, index);
	}
	catch (const Ice::Exception&)
	{
		idents.clear();
	}
	return  idents;
}

::Ice::Identity CRGSessoionManager::getIdentity(const std::string &sessId)
{
	Ice::Identity ident;
	ident.category = SERVANT_TYPE;
	ident.name = sessId;
	return ident;
}

} // end EventISVODI5
