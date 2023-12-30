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
// Desc  : Service implemenation
//
// Revision History: 
// ---------------------------------------------------------------------------

// ===========================================================================
#include "MessageAgentServ.h"
#include "XMLPreference.h"

#include "TsStorage.h"

#include "log.h"

#define DEF_TOPIC_MANAGER_ENDPOINT     L"TianShanEvents/TopicManager: default -p 10100"
#define DEF_SUBSCRIBER_ENDPOINT        L"tcp -p 20100"

#define JMS_NAMING_CONTENT             L"org.jnp.interfaces.NamingContextFactory"
#define JMS_CONNECTION_FACTORY         L"ConnectionFactory"

#define JMS_RECONNECTION_INTERVAL      5
//////////////////////////////////////////////////////////////////////////
//  Micro definition for XML configuration file Node 
//////////////////////////////////////////////////////////////////////////
#define MAX_NODE_LENGTH               128

#define NODE_AGENT                    "agent"
#define NODE_AGENT_NAME               "name"
#define NODE_AGNET_JMSTOPIC           "jmstopic"

#define NODE_JMS_PROPERTY             "jmsheaderpropery"
#define NODE_JMS_PROPERTY_TYPE        "type"
#define NODE_JMS_PROPERTY_KEY         "key"
#define NODE_JMS_PROPERTY_VALUE       "value"


//////////////////////////////////////////////////////////////////////////
// Micro definition for Agent Name, in lower case                       //
//////////////////////////////////////////////////////////////////////////
#define AGENT_NAME_PROVISION_STATECHANGE    "provisionstatechange"
#define AGENT_NAME_PROVISION_PROGRESS       "provisionprogress"

//////////////////////////////////////////////////////////////////////////
// Micro definition for Agent safestore if it is required               //
//////////////////////////////////////////////////////////////////////////
#define SAFES_PROV_STATECHANGE              "provstatechange"
#define SAFES_PROV_PROGRESS                 "provprogress"

DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

MessageAgentServ g_server;
ZQ::common::BaseSchangeServiceApplication *Application = &g_server;



MessageAgentServ::MessageAgentServ()
: _eventChannel(NULL), _pJmsContext(NULL), 
_provStateChangePublisher(NULL), _provStateChangeHelper(NULL),
_provProgressPublisher(NULL), _provProgressHelper(NULL),
_connLostNotified(false), _lastReconnTime(0)
{
}

MessageAgentServ::~MessageAgentServ()
{
}
	
HRESULT MessageAgentServ::OnInit(void)
{
	glog(ZQ::common::Log::L_INFO, "MessageAgentServ::OnInit() enter");

	BaseSchangeServiceApplication::OnInit();
	
	DWORD dwSize = 0;

	// get topic manager endpoint
	wchar_t wszTopicManagerEndpoint[256];
	dwSize = 256;	

	getConfigValue(L"TopicManagerEndpoints", wszTopicManagerEndpoint, DEF_TOPIC_MANAGER_ENDPOINT, &dwSize, TRUE);
	char szTopicManagerEndpoint[256];
	wcstombs(szTopicManagerEndpoint, wszTopicManagerEndpoint, 256);
	_tsTopicManagerEndpoint = szTopicManagerEndpoint;

	// get subscribe side endpoint to receive the message from IceStorm
	wchar_t wszSubscribeEndpoint[256];
	dwSize = 256;	

	getConfigValue(L"SubscriberEndpoints", wszSubscribeEndpoint, DEF_SUBSCRIBER_ENDPOINT, &dwSize, TRUE);
	char szSubscribeEndpoint[256];
	wcstombs(szSubscribeEndpoint, wszSubscribeEndpoint, 256);
	_tsSubscribeEndpoint = szSubscribeEndpoint;

	// get message safestore path
	wchar_t wszSafestorePath[256];
	dwSize = 256;	

	getConfigValue(L"SafestorePath", wszSafestorePath, L"", &dwSize, TRUE);
	char szSafestorePath[256];
	wcstombs(szSafestorePath, wszSafestorePath, 256);
	_safestorePath = szSafestorePath;
	
	// get JMS Server IP
	wchar_t wszJMSServerIP[256]={0};
	dwSize = 256;
	DWORD dwTmp;
	wchar_t wszComputerName[256]={0};
	GetComputerName(wszComputerName, &dwTmp);   // look local computer as the default jms server

	getConfigValue(L"JmsServerIP", wszJMSServerIP, wszComputerName, &dwSize, TRUE);
	if(wcscmp(wszJMSServerIP, L"") == 0)
	{
		glog(ZQ::common::Log::L_DEBUG, "JMSServerIP configuration MUST be specified");
	}
	char szJMSServerIP[256];
	wcstombs(szJMSServerIP, wszJMSServerIP, 256);
	_jmsServerIP = szJMSServerIP;

	// get the JmsServer port
	getConfigValue(L"JmsServerPort", &_jmsServerPort, 1199, true); 

	// get the JmsServer reconnection interval in seconds
	getConfigValue(L"JmsReconnInterval", &_jmsReconnInterval, JMS_RECONNECTION_INTERVAL, true); 

	// get naming context
	wchar_t wszNamingContext[256];
	dwSize = 256;	

	getConfigValue(L"JmsNamingContext", wszNamingContext, JMS_NAMING_CONTENT, &dwSize, TRUE);
	char szNamingContext[256];
	wcstombs(szNamingContext, wszNamingContext, 256);
	_namingContext = szNamingContext;

	// get connection factory
	wchar_t wszConnFactory[256];
	dwSize = 256;	

	getConfigValue(L"JmsConnectionFactory", wszConnFactory, JMS_CONNECTION_FACTORY, &dwSize, TRUE);
	char szConnFactory[256];
	wcstombs(szConnFactory, wszConnFactory, 256);
	_connectionFactory = szConnFactory;

	// get the jms topic configuration file
	wchar_t wszTopicCfgFile[256];
	dwSize = 256;	

	getConfigValue(L"AgentConfigFile", wszTopicCfgFile, L"", &dwSize, TRUE);
	char szTopicCfgFile[256];
	wcstombs(szTopicCfgFile, wszTopicCfgFile, 256);
	_topicConfigFile = szTopicCfgFile;

	// load configuration to get message Agent list
	if(!LoadAgentConfig())
	{
		return E_HANDLE;
	}
	return S_OK;
}

HRESULT MessageAgentServ::OnStart(void)
{
	glog(ZQ::common::Log::L_INFO, "MessageAgentServ::OnStart() enter");

	//
	// Initialize Ice environment
	//
	glog(ZQ::common::Log::L_INFO, "Ice communicator Initialize...");
	try
	{
		int argc = 0;
		_communicator = Ice::initialize(argc, NULL);
		_adapter = _communicator->createObjectAdapterWithEndpoints("MessageAgent.Subscriber", _tsSubscribeEndpoint.c_str());
		_adapter->activate();
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, "Initialize Ice runtime met exception with error: %s", 
						               ex.ice_name().c_str());
		return E_HANDLE;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, "Initialize Ice runtime met unknow exception");
		return E_HANDLE;
	}
	
	//
	// Initialize EventChannel
	//
	glog(ZQ::common::Log::L_INFO, "Initialize IceStorm event channel on endpointer %s", 
									_tsTopicManagerEndpoint.c_str());
	try
	{
		_eventChannel = new TianShanIce::Events::EventChannelImpl(_adapter, _tsTopicManagerEndpoint.c_str(), true);
	}
	catch(const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, "Initialize Event Channel on endpointer %s failed with error %s", 
					_tsTopicManagerEndpoint.c_str(), ex.message.c_str());
		return E_HANDLE;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, "Initialize Event Channel on endpointer %s failed with unknown exception", 
					_tsTopicManagerEndpoint.c_str());
		return E_HANDLE;
	}

	//
	// initialize the context
	//
	if(!InitJMSContext())
	{
		return E_HANDLE;
	}

	//
	// initialize JMS publisher and IceStorm subscriber
	//
	if(!InitPubSubers())
	{
		return E_HANDLE;
	}
	
	// set the last connection time
	_lastReconnTime = GetTickCount();

	glog(ZQ::common::Log::L_INFO, "MessageAgentServ::OnStart() leave");

	return S_OK;
}

HRESULT MessageAgentServ::OnStop(void)
{
	glog(ZQ::common::Log::L_INFO, "MessageAgentServ::OnStop() enter");
	
	//
	// stop publisher's thread
	//

	// stop provision StateChange publisher
	_provStateChangePublisher->stop();
	_provStateChangePublisher->waitHandle(1000);

	// stop provision Progress publisher
	_provProgressPublisher->stop();
	_provProgressPublisher->waitHandle(1000);

	BaseSchangeServiceApplication::OnStop();

	glog(ZQ::common::Log::L_INFO, "MessageAgentServ::OnStop() leave");

	return S_OK;
}

HRESULT MessageAgentServ::OnUnInit(void)
{
	glog(ZQ::common::Log::L_INFO, "MessageAgentServ::OnUnInit() enter");

	glog(ZQ::common::Log::L_DEBUG, "Release provision StateChange JMS publisher");

	// release provision statechange helper object and publisher
	if(_provStateChangeHelper != NULL)
	{
		delete _provStateChangeHelper;
		_provStateChangeHelper = NULL;
	}
	
	if(_provStateChangePublisher != NULL)
	{
		_provStateChangePublisher->stop();
		_provStateChangePublisher->waitHandle(1000);

		delete _provStateChangePublisher;
		_provStateChangePublisher = NULL;
	}
	
	glog(ZQ::common::Log::L_DEBUG, "Release provision Progress JMS publisher");

	// release provision provision helper object and publisher
	if(_provProgressHelper != NULL)
	{
		delete _provProgressHelper;
		_provProgressHelper = NULL;
	}

	if(_provProgressPublisher != NULL)
	{
		_provProgressPublisher->stop();
		_provProgressPublisher->waitHandle(1000);

		delete _provProgressPublisher;
		_provProgressPublisher = NULL;
	}

	// release JMS context related resource
	glog(ZQ::common::Log::L_DEBUG, "Release JMS context");
	UninitJMSContext();

	glog(ZQ::common::Log::L_DEBUG, "Release ICE communication objects");
	// free ICE object
	_eventChannel = NULL;

	_adapter = NULL;

	if(_communicator != NULL)
	{
		try
		{
			_communicator->destroy();
		}
		catch(...)
		{
			return S_OK;
		}
	}

	BaseSchangeServiceApplication::OnUnInit();

	glog(ZQ::common::Log::L_INFO, "MessageAgentServ::OnUnInit() leave");

	return S_OK;
}

bool MessageAgentServ::isHealth(void)
{
	if( (_provStateChangePublisher != NULL && !_provStateChangePublisher->JmsConnectionIsReady())
		|| (_provProgressPublisher != NULL && !_provProgressPublisher->JmsConnectionIsReady()) )
	{
		DWORD dwCurrent = GetTickCount();
		// if the exe is running more than 49.7, dwCurrent will less than _lastReconnTime. 
		// In this case, just do re-connection just now
		DWORD elapsedTime = dwCurrent > _lastReconnTime ? dwCurrent - _lastReconnTime : 2 * _jmsReconnInterval * 1000;
		
		if( elapsedTime < _jmsReconnInterval * 1000 )
		{
			return true; 
		}

		glog(ZQ::common::Log::L_INFO, "MessageAgent isHealth() found JMS connection lost, reconnect it now");

		// what ever which one is lost, notify another one.
		NotifyJmsPubsConnLost();

		//
		// re-initialize the context for the reconnection to jms server
		//
		if(!InitJMSContext())
		{
			_lastReconnTime = GetTickCount();

			return false;
		}
		bool bRet = false;
		//
		// Notify the publisher thread to reset JMS context
		//
		bRet = _provStateChangePublisher->NotifyJMSContextReset();
		if(!bRet)
		{
			return false;
		}
		bRet = _provProgressPublisher->NotifyJMSContextReset();
		if(!bRet)
		{
			return false;
		}
		
		glog(ZQ::common::Log::L_NOTICE, "JMS connection was recreated now");

		// remember the last reconnection time
		_lastReconnTime = GetTickCount();
	}
	else if(_connLostNotified) 
	{
		// reset the flag after the connection is ready
		_connLostNotified = false;
	}

	return true;
}

void MessageAgentServ::exitProcess(void)
{
}

bool MessageAgentServ::LoadAgentConfig()
{
	ZQ::common::ComInitializer comInier;
	ZQ::common::XMLPrefDoc xmlDoc(comInier);

	// open xml doc
	bool ret = xmlDoc.open(_topicConfigFile.c_str());
	if(!ret)
	{
		glog(ZQ::common::Log::L_ERROR, "Load agent's topic configuration file %s failed", _topicConfigFile.c_str());
		return false;
	}

	ZQ::common::PrefGuard root = xmlDoc.root();

	ZQ::common::PrefGuard pAgentPref;

	pAgentPref.pref(root.pref()->firstChild());
	while(pAgentPref.valid())
	{
		AgentItem aitem;

		// get node name 
		char szNodeName[MAX_NODE_LENGTH]={0};
		pAgentPref.pref()->name(szNodeName);
		strlwr(szNodeName);  // to lower case

		// if NOT an Agent Node, ignore it
		if(strcmp(szNodeName, NODE_AGENT) != 0)  
		{	
			pAgentPref.pref(root.pref()->nextChild());
			continue;
		}
		
		// get agent's xml node properties
		char szName[MAX_NODE_LENGTH]={0};
		pAgentPref.pref()->get(NODE_AGENT_NAME, szName);
		aitem._agentName = szName;

		// get JMSTopic
		char szTopic[MAX_NODE_LENGTH];
		pAgentPref.pref()->get(NODE_AGNET_JMSTOPIC, szTopic);
		if(strcmp(szTopic, "") == 0)
		{
			glog(ZQ::common::Log::L_ERROR, "Agent %s 's %s MUST be configured.", aitem._agentName.c_str(), NODE_AGNET_JMSTOPIC);
			return false;
		}
		aitem._jmsTopic = szTopic;

		// fetch jms head property
		ZQ::common::PrefGuard propertyPref;
		propertyPref.pref(pAgentPref.pref()->firstChild());
		while(propertyPref.valid())
		{
			// get node name 
			propertyPref.pref()->name(szNodeName);
			strlwr(szNodeName);  // to lower case

			// if NOT a valid JmsHeaderPropery Node, return false
			if(strcmp(szNodeName, NODE_JMS_PROPERTY) != 0)  
			{	
				glog(ZQ::common::Log::L_ERROR, "Invalid subnode under Agent %s", aitem._agentName.c_str());
				return false;
			}

			// get type
			char szType[MAX_NODE_LENGTH];
			propertyPref.pref()->get(NODE_JMS_PROPERTY_TYPE, szType);
			if(strcmp(szType, "") == 0)
			{
				glog(ZQ::common::Log::L_ERROR, "Type can NOT be empty under Agent %s 's subnode.", aitem._agentName.c_str());
				return false;
			}

			// get key
			char szKey[MAX_NODE_LENGTH];
			propertyPref.pref()->get(NODE_JMS_PROPERTY_KEY, szKey);
			if(strcmp(szKey, "") == 0)
			{
				glog(ZQ::common::Log::L_ERROR, "Key can NOT be empty under Agent %s 's subnode.", aitem._agentName.c_str());
				return false;
			}

			// get value
			char szValue[MAX_NODE_LENGTH];
			propertyPref.pref()->get(NODE_JMS_PROPERTY_VALUE, szValue);
			if(strcmp(szValue, "") == 0)
			{
				glog(ZQ::common::Log::L_ERROR, "Value can NOT be empty under Agent %s 's subnode.", aitem._agentName.c_str());
				return false;
			}
			
			// set the Type-Key-Value
			TianShanIce::MessageAgent::TypeKeyValue tkv(szType, szKey, szValue);
			
			aitem._properties.push_back(tkv);

			// turn to next node
			propertyPref.pref(pAgentPref.pref()->nextChild());
		}

		// put the agent to the agent map
		
		// set map key to be lower case
		strlwr(szName);  // to lower case

		_agents.insert(AGENTMAP::value_type(szName, aitem));

		glog(ZQ::common::Log::L_INFO, "MessageAgent support %s agent on JMSTopic %s", aitem._agentName.c_str(), aitem._jmsTopic.c_str());
		

		// turn to next loop
		pAgentPref.pref(root.pref()->nextChild());
	}
	
	if(_agents.size() == 0)
	{
		glog(ZQ::common::Log::L_ERROR, "Did not find any Message Agent list in the configuration file %s", _topicConfigFile.c_str());
		return false;
	}
	
	return true;
}

bool MessageAgentServ::InitJMSContext()
{
	// un-init first
	UninitJMSContext();

	glog(ZQ::common::Log::L_INFO, "Initialize JMS Context..."); 
	char jmsURL[256] = {0};
	sprintf(jmsURL, "%s:%d", _jmsServerIP.c_str(), _jmsServerPort);
	_pJmsContext = new ZQ::JMSCpp::Context(jmsURL, _namingContext.c_str());
	if(NULL == _pJmsContext || NULL == _pJmsContext->_context)
	{
		glog(ZQ::common::Log::L_ERROR, "Initialize JMS Context failed to server ip %s, naming context %s with error code %d",
					_jmsServerIP.c_str(), _namingContext.c_str(), ZQ::JMSCpp::getLastJmsError());

		return false;
	}

	// create connection factory
	glog(ZQ::common::Log::L_INFO, "Create JMS connection factory..."); 
	if(!_pJmsContext->createConnectionFactory(_connectionFactory.c_str(), _jmsConnFactory)
			|| NULL == _jmsConnFactory._connectionFactory)
	{		
		glog(ZQ::common::Log::L_ERROR, "Create Connection Factory failed on %s with error code %d",
					_connectionFactory.c_str(), ZQ::JMSCpp::getLastJmsError());

		return false;
	}

	// create connection
	glog(ZQ::common::Log::L_INFO, "Create JMS connection ..."); 
	if(!_jmsConnFactory.createConnection(_jmsConnection) || NULL == _jmsConnection._connection)
	{
		glog(ZQ::common::Log::L_ERROR, "Create Connection failed with error code %d", ZQ::JMSCpp::getLastJmsError());

		return false;
	}
	// set connection callback
	_jmsConnection.SetConnectionCallback(TianShanIce::MessageAgent::JMSPublisher::ConnectionMonitor, this);

	// create session
	glog(ZQ::common::Log::L_INFO, "Create JMS connection session ..."); 
	if(!_jmsConnection.createSession(_jmsSession) || NULL == _jmsSession._session)
	{
		glog(ZQ::common::Log::L_ERROR, "Create Session failed with error code %d", ZQ::JMSCpp::getLastJmsError());

		return false;
	}

	std::string agentName = "";
	//
	// find provision state change configuration from MAP
	//
	agentName = AGENT_NAME_PROVISION_STATECHANGE;
	AGENTMAP::iterator itState = _agents.find(agentName);
	if(itState == _agents.end())
	{
		glog(ZQ::common::Log::L_ERROR, "Did not find Agent %s configuration", agentName.c_str());
		
		return false;
	}

	//
	// create topic/destination for provision StateChange
	//
	glog(ZQ::common::Log::L_INFO, "Create JMS destination for topic: %s", itState->second._jmsTopic.c_str());
	if(!_pJmsContext->createDestination(itState->second._jmsTopic.c_str(), _jmsDestination) || NULL == _jmsDestination._destination)
	{
		glog(ZQ::common::Log::L_ERROR, "Create Topic(Destination) failed with error code %d", ZQ::JMSCpp::getLastJmsError());

		return false;
	}


	//
	// find provision progress configuration from MAP
	//
	agentName = AGENT_NAME_PROVISION_PROGRESS;
	AGENTMAP::iterator itProg = _agents.find(agentName);
	if(itProg == _agents.end())
	{
		glog(ZQ::common::Log::L_ERROR, "Did not find Agent %s configuration", agentName.c_str());
		
		return false;
	}

	//
	// create topic/destination for provision progress
	//
	glog(ZQ::common::Log::L_INFO, "Create JMS destination for topic: %s", itProg->second._jmsTopic.c_str());
	if(!_pJmsContext->createDestination(itProg->second._jmsTopic.c_str(), _jmsDestination) || NULL == _jmsDestination._destination)
	{
		glog(ZQ::common::Log::L_ERROR, "Create Topic(Destination) failed with error code %d", ZQ::JMSCpp::getLastJmsError());

		return false;
	}

	return true;
}

void MessageAgentServ::UninitJMSContext()
{
	_jmsDestination.destroy();
	_jmsSession.close();
	_jmsConnection.close();
	_jmsConnFactory.Destroy();
	if(_pJmsContext)
	{
		delete _pJmsContext;
		_pJmsContext=NULL;
	}
}

bool MessageAgentServ::InitPubSubers()
{
	bool bRet = true;
	//
	// Subscribe Provision StateChange topic
	// 
	bRet = PubSubProvStateChangeEvt();
	if(!bRet)
	{
		glog(ZQ::common::Log::L_ERROR, "Subscribe Provision StateChange event failed");
		return false;
	}

	//
	// Subscribe Provision Progress topic
	//
	bRet = PubSubProvProgressEvt();
	if(!bRet)
	{
		glog(ZQ::common::Log::L_ERROR, "Subscribe Provision Progress event failed");
		return false;
	}

	return true;
}

void MessageAgentServ::NotifyJmsPubsConnLost()
{
	_connLostNotified = true;

	_provStateChangePublisher->NotifyJmsConnectionLost();
	_provProgressPublisher->NotifyJmsConnectionLost();	
}

bool MessageAgentServ::PubSubProvStateChangeEvt()
{
	bool bRet = true;

	//
	// find provision state change configuration from MAP
	//
	std::string agentName = AGENT_NAME_PROVISION_STATECHANGE;
	AGENTMAP::iterator it = _agents.find(agentName);
	if(it == _agents.end())
	{
		glog(ZQ::common::Log::L_ERROR, "Did not find Agent %s configuration", agentName.c_str());
		
		return false;
	}

	//
	// create provision state change publisher
	//
	_provStateChangePublisher = new TianShanIce::MessageAgent::JMSPublisher(_jmsSession, _jmsDestination, 
									true, _safestorePath, SAFES_PROV_STATECHANGE);
	// set the publisher header properties
	for(int i=0; i<it->second._properties.size(); i++)
	{
		std::string type = it->second._properties[i]._type;
		std::string key = it->second._properties[i]._key;
		std::string value = it->second._properties[i]._value;

		glog(ZQ::common::Log::L_INFO, "Set JMS Header type=%s, key=%s, value=%s for agent %s", 
				type.c_str(), key.c_str(), value.c_str(), agentName.c_str());

		_provStateChangePublisher->SetMsgHeader(type, key, value);
	}
	// start the publisher thread
	glog(ZQ::common::Log::L_INFO, "Start StateChange event publisher thread...");
	bRet = _provStateChangePublisher->start();
	if(!bRet)
	{
		glog(ZQ::common::Log::L_ERROR, "Initialize JMS publisher for agent %s failed", 
				agentName.c_str());

		return false;
	}

	//
	// create provision state change helper object
	//
	_provStateChangeHelper = new TianShanIce::MessageAgent::ProvStateChangeHelper(*_provStateChangePublisher);

	//
	// subscribe the IceStorm Topic
	//
	TianShanIce::Storage::ProvisionStateChangeSinkPtr sinkptr = new TianShanIce::MessageAgent::ProvisionStateChangeSinkI(*_provStateChangeHelper);
	::TianShanIce::Properties qos;

	bRet = _eventChannel->sink(sinkptr, qos);
	if(!bRet)
	{
		glog(ZQ::common::Log::L_ERROR, "Failed to subscribe topic %s to Eventchannel", it->second._jmsTopic.c_str());
		return false;
	}

	return true;
}

bool MessageAgentServ::PubSubProvProgressEvt()
{
	bool bRet = true;

	//
	// find provision progress configuration from MAP
	//
	std::string agentName = AGENT_NAME_PROVISION_PROGRESS;
	AGENTMAP::iterator it = _agents.find(agentName);
	if(it == _agents.end())
	{
		glog(ZQ::common::Log::L_ERROR, "Did not find Agent %s configuration", agentName.c_str());
		
		return false;
	}

	//
	// create provision progress publisher, no safestore for progress message
	//
	_provProgressPublisher = new TianShanIce::MessageAgent::JMSPublisher(_jmsSession, _jmsDestination, 
									false, _safestorePath, SAFES_PROV_PROGRESS);
	// set the publisher header properties
	for(int i=0; i<it->second._properties.size(); i++)
	{
		std::string type = it->second._properties[i]._type;
		std::string key = it->second._properties[i]._key;
		std::string value = it->second._properties[i]._value;

		glog(ZQ::common::Log::L_INFO, "Set JMS Header type=%s, key=%s, value=%s for agent %s", 
				type.c_str(), key.c_str(), value.c_str(), agentName.c_str());

		_provProgressPublisher->SetMsgHeader(type, key, value);
	}
	// start the publisher thread
	glog(ZQ::common::Log::L_INFO, "Start StateChange event publisher thread...");
	bRet = _provProgressPublisher->start();
	if(!bRet)
	{
		glog(ZQ::common::Log::L_ERROR, "Initialize JMS publisher for agent %s failed", 
				agentName.c_str());

		return false;
	}

	//
	// create provision progress helper object
	//
	_provProgressHelper = new TianShanIce::MessageAgent::ProvProgressHelper(*_provProgressPublisher);

	//
	// subscribe the IceStorm Topic
	//
	TianShanIce::Storage::ProvisionProgressSinkPtr sink = new TianShanIce::MessageAgent::ProvisionProgressSinkI(*_provProgressHelper);
	::TianShanIce::Properties qos;

	bRet = _eventChannel->sink(sink, qos);
	if(!bRet)
	{
		glog(ZQ::common::Log::L_ERROR, "Failed to subscribe topic %s to EventChannel", it->second._jmsTopic.c_str());
		return false;
	}
	
	return true;
}

