#ifndef __ZQTianShan_TMVSSEnv_H__
#define __ZQTianShan_TMVSSEnv_H__

#include <TianShanDefines.h>
#include <sstream>

//Ice include header
#include "SessionIdx.h"
#include "TMVSSIce.h"
#include "TMVSSFactory.h"

//config file header
#include "TMVSSConfig.h"

//Event sink header
#include "TMVSSEventSinkI.h"

//include soap header
#include "TMVSSServer.h"

//tianshan common include header
#include "locks.h"
#include "FileLog.h"
#include "ConfigHelper.h"
#include "Guid.h"
#include "NativeThreadPool.h"

//content store include header
#include "ContentImpl.h"

#ifdef _DEBUG
#  define UNATTEND_TIMEOUT			(20*1000) // 20 sec
#  define DEFAULT_SCH_ERROR_WIN		(60000) // 1 min
#  define MAX_START_DELAY			(60*1000) //1 min
#  define STOP_REMAIN_TIMEOUT		(5*1000) // 5 sec
#  define MIN_PROGRESS_INTERVAL		(10*1000) // 10 sec
#else
#  define UNATTEND_TIMEOUT			(48*60*60*1000) // 48 hours
#  define DEFAULT_SCH_ERROR_WIN		(5000) // 5 sec
#  define MAX_START_DELAY			(5*60*1000) // 5 min
#  define STOP_REMAIN_TIMEOUT		(60*1000) // 1 min
#  define MIN_PROGRESS_INTERVAL		(30*1000) // 30 sec
#endif // _DEBUG

#define MAX_IDLE (60*60*1000) // 1hour
#define DEFAULT_IDLE (5* 60*1000) // 5sec

namespace ZQTianShan{
namespace VSS{
namespace TM{

#define NSSDECLARE_DICT(_DICT)	_DICT##Ptr _p##_DICT; ZQ::common::Mutex _lock##_DICT
#define NSSDECLARE_CONTAINER(_OBJ)	Freeze::EvictorPtr _e##_OBJ; ZQ::common::Mutex _lock##_OBJ
#define DECLARE_TMVSS_INDEX(_IDX)	TianShanIce::Streamer::TMVStreamServer::##_IDX##Ptr _idx##_IDX
#define NSSDECLARE_AMICB(_CL, _API)	TianShanIce::Streamer::TMVStreamServer::AMI_##_CL##_##_API##Ptr _amiCB##_API

class TMVSSEnv
{
public:
	//constructor
	TMVSSEnv(::ZQ::common::FileLog& filelog, 
		   ::ZQ::common::NativeThreadPool& threadPool,
		   std::string &strServerPath,
		   uint16 &uServerPort,
		   std::string &strNotifyServer,
		   uint16 &usNotifyPort,
		   ::Ice::CommunicatorPtr& communicator,
		   const char* iceStormEndpoint,
		   const char* endpoint = DEFAULT_ENDPOINT_TMVSS,
		   const char* databasePath = NULL,
		   const char* runtimeDBFolder = NULL);
	virtual ~TMVSSEnv();

	// configurations
	::std::string					_dbPath;	
	::std::string					_endpoint;
	::std::string					_dbRuntimePath;
	::std::string					_programRootPath;

	TMVSSFactory::Ptr				_factory;
	Freeze::EvictorPtr				_eTMVStream;
	::ZQ::common::FileLog&			_logFile;

	::Ice::CommunicatorPtr			_communicator;
	ZQADAPTER_DECLTYPE				_adapter;

	::ZQ::common::NativeThreadPool& _thpool;
	
	//DECLARE_INDEX(SessionIdx);
	::TianShanIce::Streamer::TMVStreamServer::SessionIdxPtr	_idxSessionIdx;
	//TMVSoapClientSessionMap				_soapSessionMap;
	TMVSSServer							*_pTMVSSSoapServer;

	TMVSSEventSinkI					_tmvssEventSinkI;
	ZQ::common::URLStr	_urlStr;
	ZQ::common::URLStr	_urlStrNotify;
	std::string _strServerPath;
	uint16 _uServerPort;
	std::string _strNotifyServer;
	uint16 _usNotifyPort;
	
protected:
	bool openDB(const char* databasePath = NULL,const char* dbRuntimePath=NULL);
	void closeDB(void);
};

#define DBFILENAME_TMVSession			"TMVSS"

#define INDEXFILENAME(_IDX)			#_IDX"Idx"

#define TMVSSIdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::Streamer::TMVStreamServer::##_CLASS##Prx::uncheckedCast((_ENV)._adapter->createProxy(_ID))

#define TMVSSFindIdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::Streamer::TMVStreamServer::##_CLASS##Prx::uncheckedCast(&(_ENV)._adapter->find(_ID))

#define envlog			(_env._logFile)

}//namespace TM
}//namespace NSS
}//namespace ZQTianShan

#endif __ZQTianShan_TMVSSEnv_H__
