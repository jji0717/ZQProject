#include <new>
#include "VLCVSSEnv.h"

namespace ZQTianShan {
namespace VSS{
namespace VLC{

VLCVSSEnv::VLCVSSEnv(ZQ::common::FileLog& filelog, 
			   ZQ::common::NativeThreadPool& threadPool, 
			   std::string &strServerPath,
			   uint16 &uServerPort,
			   std::string &strPwd,
			   uint16 &uTelnetPoolSize,
			   Ice::CommunicatorPtr& communicator,
			   const char* iceStormEndpoint,
			   const std::string szServiceName,
			   int32 synInterval,
			   const char* endpoint /* = DEFAULT_ENDPOINT_VLCVSS */, 
			   const char* databasePath /* = NULL */, 
			   const char* runtimeDBFolder /* = NULL */)
:_thpool(threadPool)
,_communicator(communicator)
,_adapter(NULL)
,_logFile(filelog)
,_strServerPath(strServerPath)
,_uServerPort(uServerPort)
,_vlcTelnetSessionPool(filelog, strServerPath, uServerPort, strPwd, uTelnetPoolSize)
,_VLCVSSEventSink(filelog, iceStormEndpoint)
,_telnetParser(&filelog)
,_eventSequence(0)
{
	_urlStr.setHost(_strServerPath.c_str());
	_urlStr.setPort(_uServerPort);
	//ZQ::common::InetAddress addr;
	//addr.setAddress(strServerPath.c_str());
	//_telnet.setPeer(addr, uServerPort);
	//_telnet.setPWD(strPwd.c_str());

	_endpoint = (endpoint && strlen(endpoint)>0) ? endpoint : DEFAULT_ENDPOINT_VLCVSS;

	_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSEnv, "open adapter %s at %s"), ADAPTER_NAME_VLCVSS, _endpoint.c_str());
	try
	{
		//initialize adapter
		_adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_VLCVSS, _endpoint.c_str(), glog);

		 _factory = new VLCVSSFactory(*this);

		 _adapter->activate();

		 openDB(databasePath,runtimeDBFolder);
	}
	catch (::TianShanIce::ServerError& ex)
	{
		_logFile(ZQ::common::Log::L_ERROR,CLOGFMT(VLCVSSEnv,"Create adapter failed and open db with endpoint=%s and exception is %s"),endpoint,ex.ice_name().c_str());
		throw ex;
	}
	catch(Ice::Exception& ex)
	{
		_logFile(ZQ::common::Log::L_ERROR,CLOGFMT(VLCVSSEnv,"Create adapter and open db failed with endpoint=%s and exception is %s"),endpoint,ex.ice_name().c_str());
		throw ex;
	}

	//if (_telnet.connectToServer(5))
	//{
	//	_logFile(ZQ::common::Log::L_INFO,CLOGFMT(VLCVSSEnv,"success to connect to VLC telnet server(%s:%d) with password(%s)"), strServerPath.c_str(), uServerPort, strPwd.c_str());
	//}
	//else
	//{
	//	_logFile(ZQ::common::Log::L_ERROR,CLOGFMT(VLCVSSEnv,"fail to connect to VLC telnet server(%s:%d) with password(%s)"), strServerPath.c_str(), uServerPort, strPwd.c_str());
	//}

	_contentStoreEndpoint = endpoint;
	if (_contentStoreEndpoint.find(":") == std::string::npos)
		_contentStoreEndpoint = std::string("ContentStore:") +  _contentStoreEndpoint;
	
	_VLCVSSEventSink.setAdapter(_adapter);
	_VLCVSSEventSink.start();

	// add by zjm
	
	_controlVLCService = std::auto_ptr<ControlVLCService>(new(std::nothrow)ControlVLCService(_logFile, 
		szServiceName, _strServerPath, uServerPort, strPwd, synInterval));
	_controlVLCService->start();

	_streamWatchDog = std::auto_ptr<ZQTianShan::TimerWatchDog>(new (std::nothrow) ZQTianShan::TimerWatchDog(_logFile, _thpool, _adapter));
	_streamWatchDog->start();
}

VLCVSSEnv::~VLCVSSEnv()
{
	closeDB();
}

#define VLCVSSDataSubDir "VLCVSS"
bool VLCVSSEnv::openDB(const char* databasePath /* = NULL */,const char* dbRuntimePath/* =NULL */)
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
#define INSTANCE_INDEX(_IDX) _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSEnv, "create index: " #_IDX)); \
	_idx##_IDX = new ::TianShanIce::Streamer::VLCStreamServer::##_IDX(INDEXFILENAME(_IDX))

		_logFile(ZQ::common::Log::L_INFO, CLOGFMT(VLCVSSEnv, "opening runtime database at path: %s"), _dbRuntimePath.c_str());

		//_idxSessionIdx = new ::TianShanIce::Streamer::NGODStreamServer::SessionIdx("SessionIdx");
		
		::CreateDirectory((_dbPath + VLCVSSDataSubDir FNSEPS).c_str(), NULL);
		::CreateDirectory((_dbRuntimePath + VLCVSSDataSubDir FNSEPS).c_str(), NULL);

		INSTANCE_INDEX(SessionIdx);
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxSessionIdx);
			
			_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSEnv, "create evictor %s with index %s"), DBFILENAME_VLCSession, "SessionIdx");

#if ICE_INT_VERSION / 100 >= 303
			_eVLCStream = ::Freeze::createBackgroundSaveEvictor(_adapter, _dbRuntimePath + VLCVSSDataSubDir FNSEPS, DBFILENAME_VLCSession, 0, indices);
#else
			_eVLCStream = Freeze::createEvictor(_adapter, _dbRuntimePath + VLCVSSDataSubDir FNSEPS, DBFILENAME_VLCSession, 0, indices);
#endif

			_eVLCStream->setSize(100);
			_adapter->addServantLocator(_eVLCStream, DBFILENAME_VLCSession);	
		}

		return true;
	}
	catch(const Ice::Exception& ex)
	{
		//printf("%d\n", ex.ice_name().c_str());
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_logFile,"VLCVSSEnv",1001,CLOGFMT(VLCVSSEnv, "openDB() caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_logFile,"VLCVSSEnv",1002, CLOGFMT(VLCVSSEnv, "openDB() caught unkown exception"));
	}

	return true;
}

void VLCVSSEnv::closeDB(void)
{
	_eVLCStream = NULL;
	_idxSessionIdx = NULL;
	_factory = NULL;
	_contentStoreExPrx = NULL;
}

bool VLCVSSEnv::connectToContentStore()
{
	try
	{
		_contentStoreExPrx = ::TianShanIce::Storage::ContentStoreExPrx::checkedCast(_communicator->stringToProxy(_contentStoreEndpoint.c_str())); 
		return true;
	}
	catch(Ice::Exception& ex)
	{
		_logFile(ZQ::common::Log::L_ERROR,CLOGFMT(VLCVSSEnv,"connect to local contentstore(%s) catch ice exception %s"), _contentStoreEndpoint.c_str(), ex.ice_name().c_str());
		_contentStoreExPrx = NULL;
	}
	catch (...)
	{
		_logFile(ZQ::common::Log::L_ERROR,CLOGFMT(VLCVSSEnv,"connect to local contentstore(%s) catch unknown exception"), _contentStoreEndpoint.c_str());
		_contentStoreExPrx = NULL;
	}
	return false;
}
}//namespace VLC
}//namespace VSS
}//namespace ZQTianShan