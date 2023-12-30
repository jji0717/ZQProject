#ifndef __ZQTianShan_VLCVSSEnv_H__
#define __ZQTianShan_VLCVSSEnv_H__

#include <memory>

#include <TianShanDefines.h>
#include <sstream>

//Ice include header
#include "SessionIdx.h"
#include "VLCVSS.h"
#include "VLCVSSFactory.h"

//config file header
#include "VLCVSSConfig.h"

//Event sink header
#include "VLCVSSEventSinkI.h"

//include soap header
#include "VLCTelnetSessionPool.h"

//include contentstore header
#include "ContentImpl.h"

//tianshan common include header
#include "locks.h"
#include "FileLog.h"
#include "ConfigHelper.h"
#include "Guid.h"
#include "NativeThreadPool.h"

// control vlc service (add by zjm)
#include "ControlVLCService.h"


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
namespace VLC{

#define NSSDECLARE_DICT(_DICT)	_DICT##Ptr _p##_DICT; ZQ::common::Mutex _lock##_DICT
#define NSSDECLARE_CONTAINER(_OBJ)	Freeze::EvictorPtr _e##_OBJ; ZQ::common::Mutex _lock##_OBJ
#define DECLARE_TMVSS_INDEX(_IDX)	TianShanIce::Streamer::VLCStreamServer::##_IDX##Ptr _idx##_IDX
#define NSSDECLARE_AMICB(_CL, _API)	TianShanIce::Streamer::VLCStreamServer::AMI_##_CL##_##_API##Ptr _amiCB##_API

class VLCVSSEnv
{
public:
	//constructor
	VLCVSSEnv(::ZQ::common::FileLog& filelog, 
		   ::ZQ::common::NativeThreadPool& threadPool,
		   std::string &strServerPath,
		   uint16 &uServerPort,
		   std::string &strPwd,
		   uint16 &uTelnetPoolSize,
		   ::Ice::CommunicatorPtr& communicator,
		   const char* iceStormEndpoint,
		   const std::string szServiceName,
		   int32 synInterval,
		   const char* endpoint = DEFAULT_ENDPOINT_VLCVSS,
		   const char* databasePath = NULL,
		   const char* runtimeDBFolder = NULL);
	virtual ~VLCVSSEnv();

	bool connectToContentStore();

protected:
	bool openDB(const char* databasePath = NULL,const char* dbRuntimePath=NULL);
	void closeDB(void);

public:
	// configurations
	::std::string					_dbPath;	
	::std::string					_endpoint;
	::std::string					_dbRuntimePath;
	::std::string					_programRootPath;

	//::ZQ::common::Telnet			_telnet;
	::ZQ::common::TelnetParser		_telnetParser;
	VLCTelnetSessionPool			_vlcTelnetSessionPool;

	VLCVSSFactory::Ptr				_factory;
	Freeze::EvictorPtr				_eVLCStream;
	::ZQ::common::FileLog&			_logFile;

	::Ice::CommunicatorPtr			_communicator;
	ZQADAPTER_DECLTYPE				_adapter;

	::ZQ::common::NativeThreadPool& _thpool;
	
	::TianShanIce::Streamer::VLCStreamServer::SessionIdxPtr	_idxSessionIdx;

	ZQ::common::URLStr	_urlStr;
	std::string _strServerPath;
	uint16 _uServerPort;
	
	VLCVSSEventSinkI				_VLCVSSEventSink;
	::TianShanIce::Storage::ContentStoreExPrx _contentStoreExPrx;
	std::string _contentStoreEndpoint;
	
	// add by zjm
	std::auto_ptr<ControlVLCService> _controlVLCService;
	std::auto_ptr<ZQTianShan::TimerWatchDog> _streamWatchDog;

	long _eventSequence;
	

};

#define DBFILENAME_VLCSession			"VLCVSS"

#define INDEXFILENAME(_IDX)			#_IDX"Idx"

#define VLCVSSIdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::Streamer::VLCStreamServer::##_CLASS##Prx::checkedCast((_ENV)._adapter->createProxy(_ID))

#define VLCVSSFindIdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::Streamer::VLCStreamServer::##_CLASS##Prx::uncheckedCast(&(_ENV)._adapter->find(_ID))

#define envlog			(_env._logFile)

}//namespace VLC
}//namespace VSS
}//namespace ZQTianShan

#endif __ZQTianShan_VLCVSSEnv_H__
