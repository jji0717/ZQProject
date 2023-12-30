// File Name : CRGSessionManager.cpp

#include "CRGSessionManager.h"

#include "GBssConfig.h"

#include "CRGSessionImpl.h"

#include "stroprt.h"

#include "Environment.h"

#include "SelectionResourceManager.h"

// ZQ Common
#include "FileSystemOp.h"

// 
#ifdef ZQ_OS_LINUX
#include <unistd.h>
#else
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


namespace GBss
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
		if (id == SsmGBss::CRGSession::ice_staticId())
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
	::Freeze::EvictorIteratorPtr tItor = _pContextEvtr->getIterator("", _GBssConfig._rtspSession._cacheSize);
	IStreamSmithSite& site = _env.getStreamSmithSite();
	while (!_bQuit && tItor->hasNext())
	{
		Ice::Identity ident = tItor->next();
		TianShanIce::Properties sessionContext;
		SsmGBss::CRGSessionPrx sessionProxy = getSessionContext(ident.name, sessionContext);
		if (NULL == sessionProxy)
		{
			continue;
		}
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
		NgodResourceManager::StreamerResourcePara streamerResource;
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

bool CRGSessoionManager::addSession(const SsmGBss::CRGSessionPtr& sessionPtr, std::string &strLastError)
{
	char szBuf[2048];  // error message buffer
	szBuf[sizeof(szBuf) - 1] = '\0';

	::Ice::ObjectPrx basePrx = NULL;
	try
	{
		basePrx = _pContextEvtr->add(sessionPtr, sessionPtr->ident);
	}
	catch (Freeze::DatabaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "caught [%s]:[%s] when added session[%s] into evictor", ex.ice_name().c_str(), ex.message.c_str(), sessionPtr->ident.name.c_str());
		strLastError = strLastError;
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "%s"), szBuf);
		return false;
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "caught [%s] when added session[%s] into evictor ", ex.ice_name().c_str(), sessionPtr->ident.name.c_str());
		strLastError = szBuf;
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "%s"), szBuf);
		return false;
	}

	if (NULL == basePrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "failed to added session[%s] into evictor", sessionPtr->ident.name.c_str());
		strLastError = szBuf;
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "%s"), szBuf);
		return false;
	}
	_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(CRGSessoionManager, "successed to add session[%s] into evictor"), sessionPtr->ident.name.c_str());

	return true;
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
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "caught [%s] when remove session[%s] context", ex.ice_name().c_str(), sessId.c_str());
		strLastError = szBuf;
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "%s"), szBuf);
		return false;
	}
	catch (...)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "caught unknown exception when remove session[%s] context", sessId.c_str());
		strLastError = szBuf;
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "%s"), szBuf);
		return false;
	}

	// decrease streamer statistic 
	if (bUpdateStreamer && sessionPtr)
	{
		NgodResourceManager::StreamerResourcePara streamerResource;
		streamerResource.identifier = sessionPtr->streamerSource;
		streamerResource.requestBW = atol(sessionPtr->metadata[SESSION_META_DATA_USED_BANDWIDTH].c_str());
		streamerResource.bNeedImportChannel = !sessionPtr->metadata[SESSION_META_DATA_IMPORT_CHANNEL_NAME].empty();
		streamerResource.method = "destroy";
		streamerResource.cseq = "";
		streamerResource.sessionId = sessionPtr->ident.name;
		_env.getResourceManager().releaseResource(streamerResource, sessionPtr->streamerNetId);
	}
	return true;
}

bool CRGSessoionManager::openDB(std::string& databasePath, int32 evictorSize)
{
	closeDB();

	if (databasePath.empty())
	{
		_dbPath = _programRootPath + "data" FNSEPS ;
	}
	else
	{
		_dbPath = databasePath;
	}
	
	if (FNSEPC != _dbPath[_dbPath.length()-1])
	{
		_dbPath += FNSEPS;
	}

	_dbPath += "ssm_GBss";

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
		{
			return false;
		}
		_pCommunicator->addObjectFactory(_pFactory, SsmGBss::CRGSession::ice_staticId());
	}
	catch (Ice::Exception& ex)
	{
		_fileLog(ZQ::common::Log::L_EMERG, CLOGFMT(CRGSessoionManager, "catch [%s] when added object factory into ICE Communicator"), ex.ice_name().c_str());
		return false;
	}

	_pStreamIdx = new (std::nothrow) SsmGBss::StreamIdx(INDEXFILENAME(StreamIdx));

	std::vector<Freeze::IndexPtr> indexs;
	indexs.push_back(_pStreamIdx);

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


SsmGBss::CRGSessionPrx CRGSessoionManager::findSession(const std::string& sessId, 
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

	SsmGBss::CRGSessionPrx sessionPrx;
	try
	{
		sessionPrx = SsmGBss::CRGSessionPrx::checkedCast(_pEventAdapter->createProxy(ident));
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
	_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(CRGSessoionManager, "successed to open session[%s] context"), sessId.c_str());
	return sessionPrx;
}


bool CRGSessoionManager::pingStreamOfSession(const std::string& sessId, TianShanIce::Properties& metaData)
{
	int statusCode;
	std::string strLastError;
	SsmGBss::CRGSessionPrx sessionPrx = findSession(sessId, strLastError, statusCode);
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

SsmGBss::CRGSessionPrx CRGSessoionManager::getSessionContext(const std::string& sessId, TianShanIce::Properties& sessionContext)
{
	int statusCode;
	std::string strLastError;
	SsmGBss::CRGSessionPrx sessionProxy = findSession(sessId, strLastError, statusCode);
	if (NULL == sessionProxy)
	{
		return NULL;
	}
	try
	{
		sessionContext = sessionProxy->getMetaData();
		_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(CRGSessoionManager, "successed to get session[%s] context"), sessId.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CRGSessoionManager, "get session[%s] context caught[%s]"), sessId.c_str(), ex.ice_name().c_str());
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
	::Freeze::EvictorIteratorPtr tItor = _pContextEvtr->getIterator("", _GBssConfig._rtspSession._cacheSize);
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
	::Freeze::EvictorIteratorPtr tItor = _pContextEvtr->getIterator("", _GBssConfig._rtspSession._cacheSize);
	while (tItor->hasNext())
	{
		Ice::Identity ident = tItor->next();
		sessionList += ident.name;
		sessionList += " ";
	}
}

std::vector<Ice::Identity> CRGSessoionManager::findStreams(const ::std::string&uid, Ice::Int index) const
{
	::std::vector<Ice::Identity> idents;
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

} // end GBss
