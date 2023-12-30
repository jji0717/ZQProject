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
namespace MessageAgent
{

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

	
class JMSPublisher : public ZQ::common::NativeThread
{
	typedef enum {JMS_LOST, JMS_CONNECTING, JMS_READY } JMS_CONN_STAT;
	
public:
	JMSPublisher(ZQ::JMSCpp::Session& session, ZQ::JMSCpp::Destination& destination, 
				 bool supportSafestore=true, std::string safestorePath="", std::string safestoreName="");
	
	virtual ~JMSPublisher();

public:
	// this function is used to reset the context when the connection lost
	bool NotifyJMSContextReset();

	/// set the JMS header with Type, Key, Value
	void SetMsgHeader(std::string type, std::string key, std::string value);

	/// Add Message to the queue of Publisher, the JMS message is text typed
	void SendJMSMessage(std::string message);

	/// set / get current connection status
	void NotifyJmsConnectionLost() { _jmsConnStat = JMS_LOST; };
	bool JmsConnectionIsReady() 
	{ 
		//return JMS_READY == _jmsConnStat; 
		if(JMS_READY == _jmsConnStat)
			return true;
		return false;
	};

protected:
	// thread operation
	virtual int run();

	virtual void final();

public:
	/// start the thread
	bool start();

	/// stop the jms publisher
	void stop();

protected:
	/// Initialize safestore
	bool InitializeSafeStore();

	/// Initialize / Uninitialize the JMS context, connection, topic ...
	bool InitializeJMSPub();
	bool UninitializeJMSPub();

	bool CreateMsgHeaderProperites();

	/// the callback function of JMSConnection
	static void ConnectionMonitor(int errType,VOID* lpData);

private:
	/// the function to sending the message to jms server
	bool SendingMessage();

private:
	/// Handle for control the thread
	HANDLE             _hStop;
	HANDLE             _hNotify;
	HANDLE             _hStarted;
	bool               _continue;
	bool               _initializeOK;

	/// flag & variables for message safestore
	bool               _supportSafestore;
	std::string        _safestorePath;
	std::string        _safestoreName;

	/// Ice related definition
	Ice::CommunicatorPtr	_ic;
	Freeze::ConnectionPtr	_conn;

	/// container for header
	std::vector<TypeKeyValue> _msgHeaders;

	/// container for the message
	ZQ::common::Mutex      _msgMutex;
	Messages*              _messages;

	/// JMSCPP related definition
	ZQ::JMSCpp::Session*			_pJmsSession;
	ZQ::JMSCpp::Destination*		_pJmsDestination;
	ZQ::JMSCpp::Producer			_jmsProducer;
	ZQ::JMSCpp::TextMessage			_jmsTxtMessage;

	/// flag to see whether the jms connection is ok now
	JMS_CONN_STAT                   _jmsConnStat;
};	

}
}


#endif    // _JMSPUBLISHER_H_