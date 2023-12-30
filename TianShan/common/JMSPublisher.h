// ===========================================================================
// Copyright (c) 2006 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: CSProvisionEventHelper.cpp $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : Wrap a JMS topic message's processing, including safestore, queue, and sending
//
// Revision History: 
// ---------------------------------------------------------------------------

// ===========================================================================
#ifndef   _JMSPUBLISHER_H_
#define   _JMSPUBLISHER_H_

#include "NativeThread.h"
#include "JMSCpp/header/Jms.h"
#include "JMSCpp/jmshead.h"
#include "Log.h"
#include "Locks.h"

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

#include "MessageData.h"

namespace TianShanIce
{
namespace common
{

#define DEF_JMS_RECONN_INTERVAL   5     // the interval between retrying to connect JMS Server in case net broken
	                                    // In seconds
#define MAX_JMS_SEND_RETRY_COUNT  5     // the max retry times if jms message send fail

class JMSPublisherManager;

//////////////////////////////////////////////////////////////////////////
// class definition for JMS header property                             //
//////////////////////////////////////////////////////////////////////////
class TypeKeyValue
{
public:
	TypeKeyValue(std::string type, std::string key, std::string value) { _type = type; _key = key; _value = value; };
	TypeKeyValue(const TypeKeyValue& rhs) 
		{
			if(this == &rhs)
				return ;
			this->_type = rhs._type;
			this->_key = rhs._key;
			this->_value = rhs._value;
		};
	
	virtual ~TypeKeyValue() {};

	std::string _type;
	std::string _key;
	std::string _value;
};

///////////////////////////////////////////////////////////////////////////////
//  JMSPublisher could ONLY be used in case of your application only have    //
//  ONE topic/queue. That means only one JMSPublisher object could be crated //
//  in a process. For multiple JMSPublisher objects, the second will failed  //
//  initialize JMS context. This result is coming from the testing, that JMS //
//  C API by Webloigic only support one context in a process.                //
//  If in your application, there are multiple destination(topic/queue),     //
//  Please use JMSPublisherManager class                                     //
///////////////////////////////////////////////////////////////////////////////

class JMSPublisher : public ZQ::common::NativeThread
{	
	friend class JMSPublisherManager;
public:
	JMSPublisher(const char* jmsServerIP, unsigned long jmsServerPort, unsigned long jmsReconnInterval = DEF_JMS_RECONN_INTERVAL, 
				 const char* jmsNamingContext="org.jnp.interfaces.NamingContextFactory", 
				 const char* jmsConnFactory="ConnectionFactory");


	JMSPublisher(JMSPublisherManager& pubMgr, 
				 ZQ::JMSCpp::Connection& conn, ZQ::JMSCpp::Session& session, 
				 const char* desname);
	
	virtual ~JMSPublisher();

public:
	/// enable and specified the safestore file name
	void setSafeStore(const char* safestoreFilename, bool support=true) { 
		_supportSafestore = support; 
		
		_safestoreFilename = safestoreFilename; 
		
		if(support) {
			_pubInstance++; 
		}
	};

	/// set the jms destination, could be Topic or Queue
	void setDestination(const char* destination) { _destination = destination; };

	/// set log pointer
	void setLogger(ZQ::common::Log* logger) { _logger = logger; };

	/// set the JMS header with Type, Key, Value
	void setMsgHeader(const char* type, const char* key, const char* value);


	/// start the thread
	bool start();

	/// stop the jms publisher
	void stop();

	/// Add Message to the queue of Publisher, the JMS message is text typed
	void publishMsg(const char* message);

	/// Notify the connection broken
	void setConnBroken(int errType, const char* errMsg, bool flag = true);

protected:
	// thread operation
	virtual int run();

	virtual void final();

protected:
	/// initialize safestore
	bool initializeSafeStore();

	/// Initialize / Uninitialize the JMS context, connection, topic ...
	bool initializeJMS();
	bool uninitializeJMS();

	bool createMsgHeaderProperites();

	/// the callback function of JMSConnection
	static void connectionMonitor(int errCode, const char* errMsg, VOID* lpData);

	/// the function to sending the message to jms server
	bool processMessage();

	void writeLog(ZQ::common::Log::loglevel_t level, char* fmt, ...);

private:
	typedef enum { MODE_BYMYSELF = 0, MODE_BYMANAGER } CONTEXTMODE;

	CONTEXTMODE       _cxtMode;

	/// Handle for control the thread
	HANDLE             _hStop;
	HANDLE             _hNotify;
	HANDLE             _hInited;
	
	bool               _continue;	
	bool               _connected;

	/// flag & variables for message safestore
	bool               _supportSafestore;
	std::string        _safestoreFilename;
	static int         _pubInstance;

	/// container for the message
	ZQ::common::Mutex	_msgMutex;
	Messages*			_messages;

	/// Ice related definition
	Ice::CommunicatorPtr	_ic;
	Freeze::ConnectionPtr	_conn;

	/// JMS configuration
	std::string             _jmsServerIP;
	unsigned long           _jmsServerPort;
	std::string             _namingContext;
	std::string             _connectionFactory;
	std::string             _destination;

	unsigned long           _jmsReconnInterval;

	/// container for header
	std::vector<TypeKeyValue>       _msgHeaders;

	/// JMS context definition
	ZQ::JMSCpp::Context*            _pJmsContext;
	ZQ::JMSCpp::ConnectionFactory*	_pJmsConnFactory;
	ZQ::JMSCpp::Connection*			_pJmsConnection;
	ZQ::JMSCpp::Session*		    _pJmsSession;
	ZQ::JMSCpp::Destination	        _jmsDestination;
	ZQ::JMSCpp::Producer			_jmsProducer;
	ZQ::JMSCpp::TextMessage			_jmsTxtMessage;

	ZQ::common::Log*		_logger;
	JMSPublisherManager*    _pubMgr;	
};	

///////////////////////////////////////////////////////////////////////////////
// JMSPublisherManager is designed for supporting multiple                   //
// destination(topic/queue) in a process. The reason is from testing, the    //
// Weblogic JMS C API only could create one JMS context in a process.        //
///////////////////////////////////////////////////////////////////////////////
class JMSPublisherManager : public ZQ::common::NativeThread
{
	friend class JMSPublisher;
public:
	JMSPublisherManager(const char* jmsServerIP, unsigned long jmsServerPort, unsigned long jmsReconnInterval = DEF_JMS_RECONN_INTERVAL,
				 const char* jmsNamingContext="org.jnp.interfaces.NamingContextFactory", 
				 const char* jmsConnFactory="ConnectionFactory");
	~JMSPublisherManager();

public:
	/// set log pointer
	void setLogger(ZQ::common::Log* logger) { _logger = logger; };

	/// Initialize the destinations, destination is the key 
	bool createPublisher(const char* desName);
	
	/// enable and specified the safestore file name
	bool setSafeStore(const char* desName, const char* safestoreFilename, bool support=true);

	/// set header properties
	bool setMsgHeader(const char* desName, const char* type, const char* key, const char* value);

	/// publish the specified destination's msg
	bool publishMsg(const char* desName, const char* msg);

	/// Notify the connection broken
	void setConnBroken(int errType, const char* errMsg);

	/// get to know whether jms is initialized
	bool isConnected() { return _connected; };
public:
	/// start the thread
	bool start();

	/// stop the jms publisher
	void stop();
	
protected:
	// thread operation
	virtual int run();

	virtual void final();

protected:
	// initialize JMS context
	bool initializeJMS();
	bool uninitializeJMS();

	// start/stop publishers who managed
	bool startPubs();
	bool stopPubs();

	bool notifyNetbrkPubs();

	/// the callback function of JMSConnection
	static void connectionMonitor(int errCode, const char* errMsg, VOID* lpData);

	void writeLog(ZQ::common::Log::loglevel_t level, char* fmt, ...);

protected:
	HANDLE							_hStop;
	HANDLE							_hNotify;
	HANDLE							_hInited;

	typedef std::map<std::string, JMSPublisher*> PUBMAP;

	PUBMAP    _publishers;

	ZQ::JMSCpp::Context*            _pJmsContext;
	ZQ::JMSCpp::ConnectionFactory	_jmsConnFactory;
	ZQ::JMSCpp::Connection			_jmsConnection;
	ZQ::JMSCpp::Session				_jmsSession;
	ZQ::JMSCpp::Destination			_jmsDestination;

	std::string                     _jmsServerIP;
	unsigned long					_jmsServerPort;
	std::string                     _namingContext;
	std::string                     _connectionFactory;

	unsigned long					_jmsReconnInterval;

	bool                            _connected;
	bool                            _continue;
	ZQ::common::Log*				_logger;
};

}
}


#endif    // _JMSPUBLISHER_H_