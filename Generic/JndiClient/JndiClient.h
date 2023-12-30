// ===========================================================================
// Copyright (c) 2004 by
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
// Ident : $Id: JndiClient.java, hui.shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JndiClient/JndiClient.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 8     10-02-24 17:10 Hui.shao
// supported message properties
// 
// 7     10-02-22 20:30 Hui.shao
// 
// 6     10-02-22 19:55 Hui.shao
// supported setProducerOptions; value types of MapMessage covered Integer
// and Boolean; todo:OnMapMessage() need more works
// 
// 5     10-02-22 15:11 Hui.shao
// sinks the event of connection-establish and connection-lost
// 
// 4     10-02-21 18:01 Hui.shao
// added the support of map messages
// 
// 3     10-02-09 14:14 Fei.huang
// + merge to linux
// 
// 2     10-02-04 16:52 Hui.shao
// 
// 1     10-02-03 16:00 Hui.shao
// ===========================================================================

#ifndef __ZQ_JndiClient_H__
#define __ZQ_JndiClient_H__

#include "ZQ_common_conf.h"
#include "Log.h"
#include "Exception.h"

#include <string>
#include <map>

namespace ZQ {
namespace JndiClient {

#ifdef JndiClient_EXPORTS
#  define JndiClient_API __EXPORT
#else
#  define JndiClient_API __DLLRTL
#endif

class JndiClient_API ClientContext;
class JndiClient_API JmsSession;

#define JNDICTXPROP_INITIAL_CONTEXT_FACTORY        "java.naming.factory.initial"
#define JNDICTXPROP_STATE_FACTORIES                "java.naming.factory.state"

#define DEFAULT_CONTEXT_FACTORY                    "org.jnp.interfaces.NamingContextFactory"
#define DEFAULT_STATE_FACTORY                      "org.jboss.naming:org.jnp.interfaces"
#define MAX_MSG_LENGTH   (4096)

#define JMSMSG_PROP_TimeStamp                      "SYS.timestamp"  // jlong
#define JMSMSG_PROP_DeliveryMode                   "SYS.deliveryMode" // jint value as JmsSession::DeliveryMode
#define JMSMSG_PROP_Priority                       "SYS.priority"   // jint 0 ~9
#define JMSMSG_PROP_Expiration                     "SYS.expiration" // jlong

// -----------------------------
// class JndiException
// -----------------------------
/// A sub-hierarchy of Exception Jndi related.
class JndiException : public ZQ::common::Exception
{
public:
	JndiException(const std::string &what_arg) throw();
	virtual ~JndiException() throw();
};

// -----------------------------
// class ClientContext
// -----------------------------
/// the client context to a destination JMS service provider
class ClientContext
{
	friend class JmsSession;
public:
	typedef ::std::map< std::string, std::string> Properties;

	/// constructor
	///@param serverUrl to specify the location of JMS service provider
	///@param contextFactory to specify the context factory of JMS service provider
	///@param stateFactory to specify the state factory of JMS service provider
	///@throw JndiException if failed to initialize the context
	///@note initJVM() must be called piror to the initialization of ClientContext
	ClientContext(const ::std::string& serverUrl, const ZQ::common::Log::loglevel_t javaTraceLevel=ZQ::common::Log::L_WARNING, const Properties& properties=Properties()) throw(JndiException);

	/// destructor
	virtual ~ClientContext();

	/// initialize the java virtual machine, this is a per-process invocation as a step of environment preparation
	///@param log  the logger that this JVM can access to during its lifetime, piror to uninitJVM() is called
	///@param classpath to specify the java classpath
	///@param jvmsofile file path name to specify the jre jvm.dll
	static bool initJVM(ZQ::common::Log& log, const char* classpath, const char* jvmsofile="JVM");

	/// uninitialize the java virtual machine.
	///@note it is unsafe to uninitJVM after its refered Logger is gone
	static void uninitJVM();

protected:

//	ZQ::common::Log& _log;

private:
	void* _nestedCls;
	void* _nestedObj;
	std::string _instanceId;
	Properties _props;
	ZQ::common::Log::loglevel_t  _javaTraceLevel;
};

// -----------------------------
// class JmsSession
// -----------------------------
/// the basic session bound to a destination JMS topic or queue
class JmsSession
{
public:

	typedef enum _DestType
	{
		DT_Queue, //< the client session is to a JMS queue
		DT_Topic, //< the client session is to a JMS topic
	} DestType;

	typedef enum _DeliveryMode
	{
		DM_NonPersisent =1,
		DM_Persisent =2,
	} DeliveryMode;

	/// constructor
	///@param context context the the ClientContext
	///@param destType to specify the type of JMS destination
	///@param destName the name of JMS destination to connect to
	///@param asProducer true if the session wishes to send the messages to JMS destination
	///@param asConsumer true if the session wishes to listen the messages from JMS destination
	JmsSession(ClientContext& context, DestType destType, const std::string& destName, bool asProducer =true, bool asConsumer=false) 
		throw(JndiException);

	/// destructor
	virtual ~JmsSession();

public:

	static void setProperty(ClientContext::Properties& props, const char* key, const char* value);
	static void setIntegerProperty(ClientContext::Properties& props, const char* key, long value);
	static void setBooleanProperty(ClientContext::Properties& props, const char* key, bool value);
	static void setProperty(ClientContext::Properties& props, const ::std::string& key, const ::std::string& value);

	bool setProducerOptions(int priority, long messageTTL, DeliveryMode deliveryMode=DM_Persisent, bool disableMessageID =false, bool disableMessageTimestamp =false);

	typedef ClientContext::Properties MapMessage;

	/// send a text message to the destination JMS queue or topic, valid only when asProducer=true in the constructor
	///@param message the text message to send
	///@return true if the sending gets succeeded
	///@note for those messages that failed to send, it is up to the caller to consider whether the retries are necessary
	bool sendTextMessage(const std::string& message, const ClientContext::Properties& msgProps=ClientContext::Properties());

	/// send a map message to the destination JMS queue or topic, valid only when asProducer=true in the constructor
	///@param message the map message to send
	///@return true if the sending gets succeeded
	///@note for those messages that failed to send, it is up to the caller to consider whether the retries are necessary
	bool sendMapMessage(const MapMessage& message, const ClientContext::Properties& msgProps=ClientContext::Properties());

	/// To sink the event of connection-established
	///@param message the text message about the establishing
	///@note if the message handling in the derived classes is complex and require long duration, it is required to have
	///      additional queue in the derived classes and respond this call quickly
	virtual void OnConnected(const std::string& notice) {}

	/// To sink the event of connection-lost
	///@param message the text message about the connection lost
	///@note if the message handling in the derived classes is complex and require long duration, it is required to have
	///      additional queue in the derived classes and respond this call quickly
	virtual void OnConnectionLost(const std::string& notice) {}

	/// To sink text messages from the destination JMS queue or topic, only valid if asConsumer=true when initialization
	///@param message the text message received
	///@note if the message handling in the derived classes is complex and require long duration, it is required to have
	///      additional queue in the derived classes and respond this call quickly
	virtual void OnTextMessage(const std::string& message, const ClientContext::Properties& msgProps) {}

	/// To sink map messages from the destination JMS queue or topic, only valid if asConsumer=true when initialization
	///@param message the map message received
	///@note if the message handling in the derived classes is complex and require long duration, it is required to have
	///      additional queue in the derived classes and respond this call quickly
	virtual void OnMapMessage(const MapMessage & message, const ClientContext::Properties& msgProps) {}

protected:
	ClientContext& _context;
	::std::string  _destDescription;

private:
	void* _nestedCls;
	void* _nestedObj;
	std::string _instanceId;
};

}}
#endif //__ZQ_JndiClient_H__
