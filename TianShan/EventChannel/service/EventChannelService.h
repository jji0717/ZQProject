#ifndef __TianShan_EventChannel_Service_H__
#define __TianShan_EventChannel_Service_H__

#ifdef ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#else
#include "ZQDaemon.h"
#endif
#include "../EmbeddedIceStorm.h"
#include "../Sentinel.h"
#include <IceLog.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

class EventIceLogI : public TianShanIce::common::IceLogI
{
public:
	EventIceLogI(ZQ::common::Log* pLog):IceLogI(pLog),_pLog(pLog){};
private:
	virtual void writeLog(ZQ::common::Log::loglevel_t level, const std::string& msg);
	virtual void writeGlog(ZQ::common::Log::loglevel_t level, std::string s);
    ZQ::common::Log* _pLog;
};
void EventIceLogI::writeLog(ZQ::common::Log::loglevel_t level, const std::string& msg)
{
	std::string s = msg;
	for (int pos = s.find_first_of("\r\n"); std::string::npos != pos; pos = s.find_first_of("\r\n", pos+1))
		s.replace(pos, 1, ".");
#if ICE_INT_VERSION / 100 != 306
    (*_pLog)(level, CLOGFMT(ICE, "%s"), s.c_str());
#else
    (*_pLog)(level, CLOGFMT(ICE36, "%s"), s.c_str());
#endif
	writeGlog(level,s);
}
class EventChannelService : public ZQ::common::BaseZQServiceApplication
{
public:
    EventChannelService();
    virtual ~EventChannelService();
    virtual HRESULT OnInit(void);
    virtual HRESULT OnStop(void);
    virtual HRESULT OnStart(void);
    virtual HRESULT OnUnInit(void);		
    // virtual void OnSnmpSet(const char*);
    void restartIceStorm();
	virtual void doEnumSnmpExports();

	uint32 getLogLevel_Ice();
	void   setLogLevel_Ice(const uint32& newLevel);

private:	
    EmbeddedIceStorm *_pIceStorm;
    ZQ::common::FileLog *_pIceTraceLog;
    EventChannel::Sentinel *_pSentinel;
    bool _bInService;
};
#endif
