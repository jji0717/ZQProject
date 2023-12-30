#ifndef __ZQTianShan_GBVSSEnv_H__
#define __ZQTianShan_GBVSSEnv_H__

#include <TianShanDefines.h>
#include <SsEnvironment.h>
#include "GBSession.h"

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

namespace ZQ{ namespace StreamService{

class GBVSSEnv : public SsEnvironment
{
public:
    //constructor
    GBVSSEnv(ZQ::common::Log& mainLog, ZQ::common::Log& sessLog, 
		ZQ::common::NativeThreadPool& rtspThpool, ZQ::common::NativeThreadPool& thrdPoolSvr);
    ~GBVSSEnv();

public:
    void init();
    void uninit();

    void start();

    void setNetId(std::string netId) { strNetId = netId; };

public:
    ZQ::common::FileLog SessHisLogger;	
	ZQ::common::NativeThreadPool& thrdPoolRTSP;
private:
    SyncWatchDog* syncWatchDog;
    std::string strNetId;
	
};

}}

#endif //__ZQTianShan_GBVSSEnv_H__
