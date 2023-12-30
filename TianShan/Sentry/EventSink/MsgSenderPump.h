// MsgSenderPump.h: interface for the MsgSenderPump class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGSENDERPUMP_H__AB4992EE_A638_4F65_87FC_B19120F62FB9__INCLUDED_)
#define AFX_MSGSENDERPUMP_H__AB4992EE_A638_4F65_87FC_B19120F62FB9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "MsgSenderInterface.h"
#include "EventSinkCfg.h"
#include <vector>
#include <Locks.h>
#include <Log.h>

typedef struct MsgHandle
{
	std::string strType;
	OnNewMessage handle;
	MsgHandle():handle(NULL){};
}MSGHANDLE;

class MsgSenderPump : public IMsgSender  
{
public:
    MsgSenderPump(ZQ::common::Log& log);
	virtual ~MsgSenderPump();
	virtual bool regist(const OnNewMessage& pMsg ,const char* type );
	virtual void unregist( const OnNewMessage& pMsg , const char* type);
    /// acknowledge the sent message
    virtual void ack(const MessageIdentity& mid, void* ctx);

	bool init(const EventSinkConf* pSinkCfg);
	void uninit();

    std::vector<MSGHANDLE> query();
private:
#ifdef ZQ_OS_MSWIN
	bool AddModuleHandle(HMODULE& handle);
#else
	bool AddModuleHandle(void* handle);
#endif
private:
    ZQ::common::Log& _log;
	ZQ::common::Mutex			_lock;
	std::vector<MSGHANDLE>	_vecPMsg;
#ifdef ZQ_OS_MSWIN
	std::vector<HMODULE>		_vecHDll;
#else
	std::vector<void*>		_vecHDll;
#endif

};

#endif // !defined(AFX_MSGSENDERPUMP_H__AB4992EE_A638_4F65_87FC_B19120F62FB9__INCLUDED_)

