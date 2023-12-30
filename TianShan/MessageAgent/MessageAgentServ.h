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
// Ident : $Id: MessageAgentServ.cpp $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : Service definition
//
// Revision History: 
// ---------------------------------------------------------------------------

// ===========================================================================

#ifndef   _MESSAGEANGENTSERV_H
#define   _MESSAGEANGENTSERV_H

#include "BaseSchangeServiceApplication.h"
#include "EventChannel.h"

#include "CSProvisionEventHelper.h"
#include "ProvisionEventI.h"

#include "JmsPublisher.h"

class MessageAgentServ : public ZQ::common::BaseSchangeServiceApplication
{	
	class AgentItem
	{
	public:
		std::string _agentName;
		std::string _jmsTopic;
		
		typedef std::vector<TianShanIce::MessageAgent::TypeKeyValue> JmsHeaderProperties;
		JmsHeaderProperties _properties;
	};

public:
	MessageAgentServ();
	virtual ~MessageAgentServ();

protected:
	
	HRESULT OnInit(void);
	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnUnInit(void);
	
	bool isHealth(void);
	void exitProcess(void);

private:
	/// Load configuration files
	bool LoadAgentConfig();

	/// Initialize/Uninitialize the JMS context and connection
	bool InitJMSContext();
	void UninitJMSContext();

	/// Initialize the IceStorm Subscriber
	bool InitPubSubers();

	/// Subscribe provision StateChange event
	bool PubSubProvStateChangeEvt();

	/// Subscribe provision Progress event
	bool PubSubProvProgressEvt();

public:
	/// Notify the publisher threads that the connection is lost
	void NotifyJmsPubsConnLost();

	/// since the JMS connection lost callback will always triggered.
	/// this function is to let it only processed once
	bool HasConnEvtNotified() { return _connLostNotified; };

protected:
	TianShanIce::Events::EventChannelImpl::Ptr _eventChannel;

	/// IceStorm configuration
	std::string             _tsTopicManagerEndpoint;
	std::string             _tsSubscribeEndpoint;

	/// Safestore path
	std::string             _safestorePath;

	/// JMS configuration
	std::string             _jmsServerIP;
	DWORD                   _jmsServerPort;
	DWORD                   _jmsReconnInterval;
	std::string             _namingContext;
	std::string             _topicConfigFile;
	std::string             _connectionFactory;

	/// Agent configuration set
	typedef std::map<std::string, AgentItem> AGENTMAP;
	AGENTMAP	            _agents;

	/// Ice run time variables
	Ice::CommunicatorPtr	_communicator;
	Ice::ObjectAdapterPtr   _adapter;

	/// JMS context definition
	ZQ::JMSCpp::Context*            _pJmsContext;
	ZQ::JMSCpp::ConnectionFactory	_jmsConnFactory;
	ZQ::JMSCpp::Connection			_jmsConnection;
	ZQ::JMSCpp::Session  			_jmsSession;
	ZQ::JMSCpp::Destination			_jmsDestination;

	/// Provision Helper objects
	TianShanIce::MessageAgent::JMSPublisher*                _provStateChangePublisher;
	TianShanIce::MessageAgent::ProvStateChangeHelper*       _provStateChangeHelper;

	TianShanIce::MessageAgent::JMSPublisher*                _provProgressPublisher;
	TianShanIce::MessageAgent::ProvProgressHelper*          _provProgressHelper;

	DWORD _lastReconnTime;

	bool  _connLostNotified;
};


#endif    // _MESSAGEANGENTSERV_H