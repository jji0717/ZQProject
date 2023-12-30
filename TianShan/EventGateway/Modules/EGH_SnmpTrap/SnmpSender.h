// SnmpSender.h: interface for the SnmpSender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SNMPSENDER_H__E5240689_DDED_4DC3_8B9C_4D9035ED349F__INCLUDED_)
#define AFX_SNMPSENDER_H__E5240689_DDED_4DC3_8B9C_4D9035ED349F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <NativeThread.h>
#include <deque>
#include <string>
#include <Locks.h>
#include <ConfigLoader.h>
#include "SystemUtils.h"

#include <snmp_pp/snmp_pp.h>

//////////////////////////////////////
/// from MsgSender
typedef struct _tagMsgStructure
{
	int										id;				//Event ID
	std::string								category;		//Event Category
	std::string								timestamp;		//timestamp
	std::string								eventName;		//Event name such as 'SessionInService'
	std::string								sourceNetId;	//source net id,host name is recommended
	std::map<std::string,std::string>		property;	//Event properties
}MSGSTRUCT;
//////////////////////////////////////

class SnmpSender : public ZQ::common::NativeThread  
{
public:
	SnmpSender();
	virtual ~SnmpSender();

	virtual bool init(void);
	virtual int run(void);
	
	virtual bool GetParFromFile();
	virtual void AddMessage(const MSGSTRUCT& msgStruct);
	virtual void Close();

private:
	bool initSnmp();
	bool send(MSGSTRUCT& msg);
	bool SetTrapHeader(Pdu& pdu);
	bool ReadEventFromFile();
	bool SaveEventToFile(std::deque<MSGSTRUCT>& deq);
private:	
	
	ZQ::common::Mutex	_lock;	

	bool						_quit;
	Semaphore			        _hMsgSem;
	std::deque<MSGSTRUCT>		_msgQue;

	Snmp*						_pSnmp;
	time_t						_timeStamp;
	std::string					_strAgentIp;//agent address

	FILE*                       _hFile;
	uint32						_dwPos;
	int							_nDequeSize;//deque size if larger than this size save some record to file
	bool						_bConnectOk;
	std::string                 _strSaveName;
	//ZQ::common::ConfigLoader::VECKVMAP		_targetVM;//targets
};

#endif // !defined(AFX_SNMPSENDER_H__E5240689_DDED_4DC3_8B9C_4D9035ED349F__INCLUDED_)
