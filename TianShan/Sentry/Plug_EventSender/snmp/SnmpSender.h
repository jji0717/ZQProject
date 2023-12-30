// SnmpSender.h: interface for the SnmpSender class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __SNMPPLUG_SNMPSENDER_H__
#define __SNMPPLUG_SNMPSENDER_H__


#include "SnmpSenderCfg.h"

#include <NativeThreadPool.h>
#include <string>
#include <Locks.h>
#include <MsgSenderInterface.h>

#include <snmp_pp/snmp_pp.h>

#define SNMP_DEF_POOLSIZE	5
class SnmpSender  
{
	friend class PublishCmd;
public:
	SnmpSender(int poolSize = SNMP_DEF_POOLSIZE);
	virtual ~SnmpSender();

	virtual bool init(void);
	virtual bool GetParFromFile(const char* pFileName);
	virtual void AddMessage(const MSGSTRUCT& msgStruct, const MessageIdentity& mid, void* ctx);
	virtual void Close();

private:
	bool initSnmp();
	bool send(MSGSTRUCT& msg);
	bool SetTrapHeader(Pdu& pdu);
	
private:	
	ZQ::common::NativeThreadPool	_thPool;

	Snmp*						_pSnmp;
	time_t						_timeStamp;
	std::string					_strAgentIp;//agent address

	bool						_bConnectOk;
	bool						_bQuit;
	SysLog*					_sysLog;
};

#define LOG (*plog)
extern  ZQ::common::Log* plog;
extern Config::Loader< SnmpSenderInfo>* pSnmpSenderCfg;

extern IMsgSender* g_pIMsgSender;

#endif //__SNMPPLUG_SNMPSENDER_H__
