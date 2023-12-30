#include "TMVSSEnv.h"

namespace ZQTianShan {
namespace VSS{
namespace TM{

TMVSSEnv::TMVSSEnv(ZQ::common::FileLog& filelog, 
			   ZQ::common::NativeThreadPool& threadPool, 
			   std::string &strServerPath,
			   uint16 &uServerPort,
			   std::string &strNotifyServer,
			   uint16 &usNotifyPort,
			   Ice::CommunicatorPtr& communicator,
			   const char* iceStormEndpoint,
			   const char* endpoint /* = DEFAULT_ENDPOINT_TMVSS */, 
			   const char* databasePath /* = NULL */, 
			   const char* runtimeDBFolder /* = NULL */)
:_thpool(threadPool)
,_communicator(communicator)
,_adapter(NULL)
,_logFile(filelog)
,_tmvssEventSinkI(filelog, iceStormEndpoint)
,_strServerPath(strServerPath)
,_uServerPort(uServerPort)
,_strNotifyServer(strNotifyServer)
,_usNotifyPort(usNotifyPort)
{
	_urlStr.setHost(_strServerPath.c_str());
	_urlStr.setPort(_uServerPort);
	_urlStr.setProtocol("http");

	_urlStrNotify.setHost(_strNotifyServer.c_str());
	_urlStrNotify.setPort(usNotifyPort);
	_urlStrNotify.setProtocol("http");

	_endpoint = (endpoint && strlen(endpoint)>0) ? endpoint : DEFAULT_ENDPOINT_TMVSS;

	_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSEnv, "open adapter %s at %s"), ADAPTER_NAME_TMVSS, _endpoint.c_str());
	try
	{
		//initialize adapter
		_adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_TMVSS, _endpoint.c_str(), glog);
		 _factory = new TMVSSFactory(*this);

		 _adapter->activate();

		 openDB(databasePath,runtimeDBFolder);
	}
	catch (::TianShanIce::ServerError& ex)
	{
		_logFile(ZQ::common::Log::L_ERROR,CLOGFMT(TMVSSEnv,"Create adapter failed and open db with endpoint=%s and exception is %s"),endpoint,ex.ice_name().c_str());
		throw ex;
	}
	catch(Ice::Exception& ex)
	{
		_logFile(ZQ::common::Log::L_ERROR,CLOGFMT(TMVSSEnv,"Create adapter and open db failed with endpoint=%s and exception is %s"),endpoint,ex.ice_name().c_str());
		throw ex;
	}
	
	_pTMVSSSoapServer = new TMVSSServer(_urlStrNotify.generate(), _logFile);
	_pTMVSSSoapServer->start();

	_tmvssEventSinkI.setAdapter(_adapter);
	_tmvssEventSinkI.start();
}

TMVSSEnv::~TMVSSEnv()
{
	//::Ice::Identity ident = _adapter->getCommunicator()->stringToIdentity(DBFILENAME_TMVSession);
	//_adapter->remove(ident);
	//_contentStore = NULL;
	closeDB();
	delete _pTMVSSSoapServer;
}

#define TMVSSDataSubDir "TMVSS"
bool TMVSSEnv::openDB(const char* databasePath /* = NULL */,const char* dbRuntimePath/* =NULL */)
{
	closeDB();

	if (NULL == databasePath || strlen(databasePath) <1)
		_dbPath = _programRootPath + "data" FNSEPS;
	else 
		_dbPath = databasePath;

	if (FNSEPC != _dbPath[_dbPath.length()-1])
		_dbPath += FNSEPS;

	if ( NULL == dbRuntimePath || strlen(dbRuntimePath)<1 ) 
	{
		_dbRuntimePath = _dbPath;
	}
	else
	{
		_dbRuntimePath = dbRuntimePath;
	}
	if (FNSEPC != _dbRuntimePath[_dbRuntimePath.length()-1])
		_dbRuntimePath += FNSEPS;
	try 
	{	
		
		// open the Indexes
#define INSTANCE_INDEX(_IDX) _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSEnv, "create index: " #_IDX)); \
	_idx##_IDX = new ::TianShanIce::Streamer::TMVStreamServer::##_IDX(INDEXFILENAME(_IDX))

		_logFile(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSEnv, "opening runtime database at path: %s"), _dbRuntimePath.c_str());

		//_idxSessionIdx = new ::TianShanIce::Streamer::NGODStreamServer::SessionIdx("SessionIdx");
		
		::CreateDirectory((_dbPath + TMVSSDataSubDir FNSEPS).c_str(), NULL);
		::CreateDirectory((_dbRuntimePath + TMVSSDataSubDir FNSEPS).c_str(), NULL);

		INSTANCE_INDEX(SessionIdx);
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxSessionIdx);
			
			_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSEnv, "create evictor %s with index %s"), DBFILENAME_TMVSession, "SessionIdx");

#if ICE_INT_VERSION / 100 >= 303
			_eTMVStream = ::Freeze::createBackgroundSaveEvictor(_adapter, _dbRuntimePath + TMVSSDataSubDir FNSEPS, DBFILENAME_TMVSession, 0, indices);
#else
			_eTMVStream = Freeze::createEvictor(_adapter, _dbRuntimePath + TMVSSDataSubDir FNSEPS, DBFILENAME_TMVSession, 0, indices);
#endif

			_eTMVStream->setSize(100);
			_adapter->addServantLocator(_eTMVStream, DBFILENAME_TMVSession);	
		}

		return true;
	}
	catch(const Ice::Exception& ex)
	{
		//printf("%d\n", ex.ice_name().c_str());
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_logFile,"TMVSSEnv",1001,CLOGFMT(TMVSSEnv, "openDB() caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_logFile,"TMVSSEnv",1002, CLOGFMT(TMVSSEnv, "openDB() caught unkown exception"));
	}

	return true;
}

void TMVSSEnv::closeDB(void)
{
	_eTMVStream = NULL;
	_idxSessionIdx = NULL;
	_factory = NULL;
}

}//namespace TM
}//namespace VSS
}//namespace ZQTianShan