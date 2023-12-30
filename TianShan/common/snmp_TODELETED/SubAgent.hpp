#ifndef _SNMP_SUBAGENT_HPP
#define _SNMP_SUBAGENT_HPP

#include <ZQ_common_conf.h>
#include <NativeThread.h>
#include "ZQSnmpMgmt.hpp"
#include <Log.h>

namespace ZQ{
namespace Snmp{

typedef GuardedCompositeObject Module;

//
// subagent that process the message communication
//
class Subagent: public ZQ::common::NativeThread
{
public:
    Subagent(uint32 serviceId, uint32 moduleId, uint32 serviceInstanceId = 0);
    ~Subagent();
    bool addObject(const Oid& subid, ManagedPtr obj);
    void setLogger(ZQ::common::Log* pLog);
    virtual int run();
    void stop();

	timeout_t setTimeout(timeout_t timeOut){ return selectTimeout_ = timeOut;}
    Module& module();

private:
	int refreshBasePort(void);

#ifdef ZQ_OS_MSWIN
    bool processMessage(const void *pRequestMsg, int len, std::string& responseMsg);
#else
    bool processMessage(const u_char *pRequestMsg, int len, std::string& responseMsg);
#endif

private:
    ZQ::common::Log* pLog_;
    Oid root_;
    std::string pipeName_;
    CompositeObject rootObj_;
    Module mod_;

	uint32 serviceId_;
	uint32 serviceInstanceId_;
	uint32 moduleId_;
	uint32 snmpUdpBasePort_;
	timeout_t selectTimeout_;
    bool quit_;
};

}} // namespace ZQ::Snmp

#endif   //_SNMP_SUBAGENT_HPP