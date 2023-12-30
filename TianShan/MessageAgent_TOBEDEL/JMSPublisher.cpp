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
#include "MessageAgentServ.h"
#include "JMSPublisher.h"
#include "time.h"


#pragma comment(lib,"jmsc.lib")

#ifdef _DEBUG
#	pragma comment(lib,"jmscpp_d.lib")
#else
#	pragma comment(lib,"jmscpp.lib")
#endif


namespace TianShanIce
{

namespace MessageAgent
{

JMSPublisher::JMSPublisher(ZQ::JMSCpp::Session& session, ZQ::JMSCpp::Destination& destination, 
						   bool supportSafestore, std::string safestorePath, std::string safestoreName)
:_pJmsSession(&session), _pJmsDestination(&destination),
_supportSafestore(supportSafestore), _safestorePath(safestorePath), _safestoreName(safestoreName),
_messages(NULL), _continue(true), _initializeOK(false), _jmsConnStat(JMS_READY)
{
	_hStop = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);
	_hStarted = CreateEvent(NULL, false, false, NULL);	
	
	_jmsConnStat = JMS_READY;
}

JMSPublisher::~JMSPublisher()
{
	// release resource
	if(_messages != NULL)
	{
		delete _messages;
		_messages = NULL;
	}

	// close the handle
	if(_hStop != NULL)
	{
		CloseHandle(_hStop);
		_hStop = NULL;
	}
	if(_hNotify != NULL)
	{
		CloseHandle(_hNotify);
		_hNotify = NULL;
	}
	
	if(_hStarted != NULL)
	{
		CloseHandle(_hStarted);
		_hStarted = NULL;
	}

	try
	{
		if(_ic != NULL)
		{
			_ic->destroy();
		}
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, "Publisher's Ice communication destroy met exception with error: %s", 
						ex.ice_name().c_str());
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, "Publisher's Ice communication destroy met unknown exception");
	}

	_ic = NULL;
}

bool JMSPublisher::NotifyJMSContextReset()
{
	// the connection is ok, do NOT care it.
	if(_jmsConnStat != JMS_LOST)
	{
		return false;
	}

	// set the status to CONNETING, let the jms initialization completed in the thread
	_jmsConnStat = JMS_CONNECTING;

	SetEvent(_hNotify);

	//
	// wait for the initialization completion in the thread
	//
	WaitForSingleObject(_hStarted, INFINITE);

	return _initializeOK;
}

bool JMSPublisher::InitializeSafeStore()
{
	//
	// Initial the DB path
	//

	// generate default path
	if(_safestorePath == "")
	{
		char path[MAX_PATH];
		if (::GetModuleFileNameA(NULL, path, MAX_PATH-1)>0)
		{
			char* p = strrchr(path, FNSEPC);
			if (NULL !=p)
			{
				*p='\0';
				p = strrchr(path, FNSEPC);
				if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
					*p='\0';
			}

			strcat(path, FNSEPS "safestore" FNSEPS);
			_safestorePath = path;
		}
	}
	// create the directory if it does not existed
	::CreateDirectoryA(_safestorePath.c_str(), NULL);


	// generate default name
	if(_safestoreName=="")
	{
		// generate the rand number
		srand( (unsigned)time(NULL) );
		int number = rand();
		char szNumber[20];
		sprintf(szNumber, "%d", number);

		// set the name
		_safestoreName = "JMSMessages" + std::string(szNumber);
	}
	else if(!_supportSafestore)
	{
		std::string dbFullPath;
		// the file full path
		if(_safestorePath[_safestorePath.size()-1] != '\\' || _safestorePath[_safestorePath.size()-1] != '/')
		{
			dbFullPath = _safestorePath + "/" + _safestoreName;
		}
		else
		{
			dbFullPath = _safestorePath + _safestoreName;
		}
		
		// delete the file, coz there is no need to do safestore
		DeleteFileA(dbFullPath.c_str());
	}

	//
	// Initialize the safestore DB
	//
	try
	{
		int i =0;
		_ic = Ice::initialize(i, NULL);
		
		_conn = Freeze::createConnection(_ic, _safestorePath);

		_messages = new Messages(_conn, _safestoreName/*, _supportSafestore*/);

		if(_supportSafestore)
		{
			glog(ZQ::common::Log::L_INFO, "There are %d messages loaded from safestore", _messages->size());
		}
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_INFO, "Initialize provision safestore met exception with error: %s", 
					ex.ice_name().c_str());
		
		return false;
	}

	return true;
}

bool JMSPublisher::InitializeJMSPub()
{
	glog(ZQ::common::Log::L_INFO, "JMSPublisher::InitializeJMSPub() enter");

	// uninitialize first
	UninitializeJMSPub();

	// create producer
	glog(ZQ::common::Log::L_INFO, "Create JMS producer...");
	if(!_pJmsSession->createProducer(_pJmsDestination, _jmsProducer) || NULL == _jmsProducer._producer)
	{
		glog(ZQ::common::Log::L_INFO, "Create Producer failed with error code %d", ZQ::JMSCpp::getLastJmsError());

		return false;
	}

	// create text message
	glog(ZQ::common::Log::L_INFO, "Create JMS text message...");
	if(!_pJmsSession->textMessageCreate("", _jmsTxtMessage) || NULL == _jmsTxtMessage._message)
	{
		glog(ZQ::common::Log::L_INFO, "Create Text Message failed with error code %d", ZQ::JMSCpp::getLastJmsError());

		return false;
	}
	
	// Set the Message Header
	glog(ZQ::common::Log::L_INFO, "Create JMS message header properites...");
	if(!CreateMsgHeaderProperites())
	{
		glog(ZQ::common::Log::L_INFO, "Create JMS message header property failed with error code %d", ZQ::JMSCpp::getLastJmsError());
		
		return false;
	}

	glog(ZQ::common::Log::L_INFO, "JMSPublisher::InitializeJMSPub() leave");

	return true;
}

bool JMSPublisher::UninitializeJMSPub()
{
	_jmsTxtMessage.DestroyMessage();
	_jmsProducer.close();
	return true;
}

bool JMSPublisher::CreateMsgHeaderProperites()
{
	bool bRet = true;

	for(DWORD i=0; i<_msgHeaders.size(); i++)
	{
		std::string type = _msgHeaders[i]._type;
		std::string key = _msgHeaders[i]._key;
		std::string value = _msgHeaders[i]._value;

		if(0==stricmp("int",type.c_str()))
		{	
			int	nValue = atoi(value.c_str());
			
			bRet = _jmsTxtMessage.setIntProperty((char*)key.c_str(), nValue);
		}
		else if(0==stricmp("long",type.c_str()))
		{
			long lValue = atol(value.c_str());
			bRet = _jmsTxtMessage.setLongProperty((char*)key.c_str(), lValue);
		}
		else if(0==stricmp("double",type.c_str()))
		{
			double dValue = atof(value.c_str());
			bRet = _jmsTxtMessage.setDoubleProperty((char*)key.c_str(), dValue);
		}
		else if(0==stricmp("float",type.c_str()))
		{
			float fValue= (float)atof(value.c_str());
			bRet = _jmsTxtMessage.setFloatProperty((char*)key.c_str(),fValue);
		}
		else if(0==stricmp("bool",key.c_str()))
		{
			bool bValue = true;
			if(value == "false" || value == "0")
			{
				bValue = false;
			}
			bRet = _jmsTxtMessage.setBoolProperty((char*)key.c_str(),bValue);
		}
		else if(0==stricmp("byte",type.c_str()))
		{
			unsigned char byValue = (unsigned char)atoi(value.c_str());
			bRet = _jmsTxtMessage.setByteProperty((char*)key.c_str(),byValue);
		}
		else if(0==stricmp("short",type.c_str()))
		{
			short sValue = (short)atoi(value.c_str());
			bRet = _jmsTxtMessage.setShortProperty((char*)key.c_str(),sValue);
		}
		else if(0==stricmp("string",type.c_str()))
		{
			bRet = _jmsTxtMessage.setStringProperty((char*)key.c_str(),(char*)value.c_str());
		}
		else
		{
			bRet = false;
		}

		if(!bRet)
		{
			return false;
		}
	}
	return true;
}

void JMSPublisher::ConnectionMonitor(int errType,VOID* lpData)
{
	// this event will always triggered. only Notify once.
	// Only re-connect JMS server when sending failed.
	MessageAgentServ* msgAgent = (MessageAgentServ*) lpData;

	if(!msgAgent->HasConnEvtNotified())
	{
		glog(ZQ::common::Log::L_ERROR, "JMS connection is broken which detected in the callback");		
		msgAgent->NotifyJmsPubsConnLost();
	}
}

/// set the JMS header with Type, Key, Value
void JMSPublisher::SetMsgHeader(std::string type, std::string key, std::string value)
{
	TypeKeyValue header(type, key, value);

	_msgHeaders.push_back(header);
}

/// Add Message to the queue of Publisher, the JMS message is text typed
void JMSPublisher::SendJMSMessage(std::string message)
{
	if(!JmsConnectionIsReady() && !_supportSafestore)
	{
		// in case of connection broken & no safestore for the message, 
		// just discard the message
		//glog(ZQ::common::Log::L_DEBUG, "JMS connection is lost, discard message %s", message.c_str());
		return ;
	}

	TianShanIce::MessageAgent::Message jmsMsg;
	jmsMsg.content = message;

	{
		ZQ::common::MutexGuard guard(_msgMutex);
		// put the message object to the map
		std::string msgid = IceUtil::generateUUID();
		_messages->put(Messages::value_type(msgid, jmsMsg));
	}

	// trigger thread to process the message if the connection is ready
	if(JmsConnectionIsReady())
	{
		SetEvent(_hNotify);
	}
}

int JMSPublisher::run()
{
	_jmsConnStat = JMS_READY;

	glog(ZQ::common::Log::L_INFO, "JMSPubliser::run() enter");
	bool bRet = true;
	//
	// initialize the safestore
	//
	bRet = InitializeSafeStore();
	if(!bRet)
	{
		_initializeOK = false;

		SetEvent(_hStarted);

		return 1;
	}

	//
	// initialize the JMSCPP
	//
	bRet = InitializeJMSPub();
	if(!bRet)
	{
		// start failed
		_initializeOK = false;

		SetEvent(_hStarted);

		return 1;
	}

	// start succeed
	_initializeOK = true;

	SetEvent(_hStarted);

	// trigger the message sending, if there is any message
	if(_messages->size() > 0)
	{
		SetEvent(_hNotify);
	}
	//
	//	entering message sending 
	//
	_continue = true;
	HANDLE handles[2] = { _hStop, _hNotify };

	while(_continue)
	{
		DWORD dwWaitStatus = WaitForMultipleObjects(2, handles, false, INFINITE);
		switch(dwWaitStatus)
		{
		case WAIT_OBJECT_0:
			_continue = false;
			break;
		case WAIT_OBJECT_0 + 1:
			// if the connection is lost, do not send the message
			if(JMS_LOST == _jmsConnStat )
			{
				break;
			}
			// checking current status, if the jms context has been re-initialized, 
			// re-initialize the JMS publisher
			if(JMS_CONNECTING == _jmsConnStat )
			{
				// the JMS publisher init failed, set the status LOST, 
				// let MessageAgentServ to re-create the JMS context
				if(!InitializeJMSPub())
				{
					_jmsConnStat = JMS_LOST;

					_initializeOK = false;
					SetEvent(_hStarted);

					break;
				}

				// the connection is ready, can be send message right now
				_initializeOK = true;
				SetEvent(_hStarted);

				_jmsConnStat = JMS_READY;
			}
			//
			// send jms message
			//
			if(!SendingMessage())
			{ // sending met problem, re-connect the JMS server
				//glog(ZQ::common::Log::L_ERROR, "JMS client's connection to JMS server is broken now, will try to reconnect...");

				glog(ZQ::common::Log::L_ERROR, "JMS client's connection to JMS server is broken now. will re-connect");
				
				// sending message failed, set the status to lost 
				_jmsConnStat = JMS_LOST;
			}
			else
			{
				ZQ::common::MutexGuard guard(_msgMutex);

				// if there is additional message, trigger next sending operation
				if(_messages->size() > 0)
				{
					SetEvent(_hNotify);
				}

			}
			break;
		default:
			break;
		}
	}

	glog(ZQ::common::Log::L_INFO, "JMSPubliser::run() leave");
	return 0;
}

bool JMSPublisher::SendingMessage()
{
	ZQ::common::MutexGuard guard(_msgMutex);
	
	if(_messages->size() > 0)
	{
		Messages::iterator iter = _messages->begin();

		std::string msgcontent = iter->second.content;

		ZQ::JMSCpp::ProducerOptions	pOption;
		ZeroMemory(&pOption,sizeof(ZQ::JMSCpp::ProducerOptions));
		pOption.flags = PO_TIMETOLIVE;
		pOption.timeToLive = 0;

		// fullfill the message to jms object
		_jmsTxtMessage.setText((char*)msgcontent.c_str());

		if(!_jmsProducer.send(&_jmsTxtMessage, &pOption))
		{// send failed
			glog(ZQ::common::Log::L_INFO, "JMS message send failed. msg content is %s", msgcontent.c_str());

			return false;
		}
		else // message is sent successfully
		{
			glog(ZQ::common::Log::L_DEBUG, "JMS message send successfully. msg content is %s", msgcontent.c_str());

			// remove the sent message from the queue
			_messages->erase(iter);

			return true;
		}
	}
	return true;
}

void JMSPublisher::final()
{
	UninitializeJMSPub();

	// release Ice object
	if(_conn != NULL)
	{
		try
		{
			_conn->close();
		}
		catch(...)
		{
		}
		_conn = NULL;
	}

	if(_ic != NULL)
	{
		try
		{
			_ic->destroy();
		}
		catch(...)
		{
		}
		_ic = NULL;
	}
}

bool JMSPublisher::start()
{
	//
	// invoke base start() routine
	//
	bool bRet = NativeThread::start();
	if(!bRet)
	{
		glog(ZQ::common::Log::L_ERROR,  "Start JMS Publisher thread failed");

		return false;
	}

	//
	// wait for the initialization completion in the thread
	//
	WaitForSingleObject(_hStarted, INFINITE);

	return _initializeOK;
}

void JMSPublisher::stop()
{
	_continue = false;
	SetEvent(_hStop);
}

}
}

