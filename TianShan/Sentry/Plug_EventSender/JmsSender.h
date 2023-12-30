// JmsSender.h: interface for the JmsSender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JMSSENDER_H__8A472A2F_E0BD_4FE1_8CFB_426F17EBBEBE__INCLUDED_)
#define AFX_JMSSENDER_H__8A472A2F_E0BD_4FE1_8CFB_426F17EBBEBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <NativeThread.h>
#include "BaseSender.h"
#include <deque>
#include <string>
#include <vector>
#include <Locks.h>
#include "SystemUtils.h"

#include "JndiClient.h"

// #include <JMSCpp/header/Jms.h>
// #include <JMSCpp/jmshead.h>

#include <ConfigLoader.h>

class JmsSender;

class SenderSession : public ZQ::JndiClient::JmsSession
{
public:
	SenderSession(JmsSender& connStatusReceiver, ZQ::JndiClient::ClientContext& context, ZQ::JndiClient::JmsSession::DestType destType, const std::string& destName )
		:ZQ::JndiClient::JmsSession(context,destType,destName,true),
		mConnectionStatusReceiver(connStatusReceiver)
	{
	}	
	virtual ~SenderSession(){}
protected:
	virtual void OnConnected(const std::string& notice);
	virtual void OnConnectionLost(const std::string& notice);	
private:
	JmsSender&		mConnectionStatusReceiver;
};

class JmsSender : public ZQ::common::NativeThread , public BaseSender 
{
public:
	JmsSender();
	virtual ~JmsSender();

	virtual bool init(void);
	virtual int run(void);
	
	virtual void Close();
	virtual void AddMessage(const MSGSTRUCT& msgStruct);
	virtual bool GetParFromFile(const char* pFileName);

	void	onConnected();
	void	onDisconnected();

protected:
	virtual void OnConnected(const std::string& notice) {}
	virtual void OnConnectionLost(const std::string& notice) {}

	bool InitializeJMS();
	void UnInitializeJMS();
	bool CreateMessageProperty( ::ZQ::JndiClient::JmsSession::MapMessage& mapMsg );	
	bool InternalSendMessage(const MSGSTRUCT& msgStruct);
	
#ifdef ZQ_OS_MSWIN
	static BOOL WINAPI HandlerRoutine(DWORD dwCtrlType);
#endif

	bool ReadEventFromFile();
	bool SaveEventToFile( std::deque<MSGSTRUCT>& deq );
	
private:
	ZQ::common::Mutex			_lock;	

	ZQ::common::Semaphore		mSemExit;
	SYS::SingleObject			mSemMsg;

	std::deque<MSGSTRUCT>		mMsgQue;
	std::string					_strCfgName;
	size_t						_dwPos;
	int							_nDequeSize;   //deque size if large this size save some record to file

	std::string					_strSaveName;	//failed send event save path
	FILE*						_hFile;

	ZQ::JndiClient::ClientContext*	_pJmsContext;	
	SenderSession*					_pJmsSenderSession;
	::ZQ::JndiClient::JmsSession::MapMessage	mMessageProperties;
// 
// 	ZQ::JMSCpp::Context					*_pJmsContext;
// 	ZQ::JMSCpp::ConnectionFactory		_JmsCNFactory;
// 	ZQ::JMSCpp::Connection				_JmsConnection;
// 	ZQ::JMSCpp::Producer				_JmsProducer;
// 	ZQ::JMSCpp::Destination				_JmsDestination;
// 	ZQ::JMSCpp::Session					_JmsSession;
// 	ZQ::JMSCpp::ProducerOptions			_MsgOption;

	std::string						_strServerAddress;
	std::string						_strNamingContext;
	std::string						_strDestinationName;		
	std::string						_strConnectionFactory;
	bool							_bConnectionOK;
	bool							_bJmsInitializeOK;
	int								_iNoneSendMsgFlushCount;
	int								_msgTTL;
	long							_lConnectionLostTime;
	bool _bQuit;
};


#endif // !defined(AFX_JMSSENDER_H__8A472A2F_E0BD_4FE1_8CFB_426F17EBBEBE__INCLUDED_)
