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
// Adsuki  : Wrap a JMS topic message's processing, including safestore, queue, and sending
//
// Revision History: 
// ---------------------------------------------------------------------------

// ===========================================================================
#include "JMSPublisher.h"
#include <iomanip>
#include "time.h"


#pragma comment(lib,"jmsc.lib")

#ifdef _DEBUG
#	pragma comment(lib,"jmscpp_d.lib")
#else
#	pragma comment(lib,"jmscpp.lib")
#endif


namespace TianShanIce
{

namespace common
{

int JMSPublisher::_pubInstance = 0;

////////////////////////////////////////////////////////////////////////
//           Implementation of JMSPublisher                           //
////////////////////////////////////////////////////////////////////////

JMSPublisher::JMSPublisher(
		const char* jmsServerIP, 
		unsigned long jmsServerPort, 
		unsigned long jmsReconnInterval,
		const char* jmsNamingContext, 
		const char* jmsConnFactory):
_jmsServerIP(jmsServerIP), 
_jmsServerPort(jmsServerPort), 
_jmsReconnInterval(jmsReconnInterval),
_supportSafestore(false), 
_safestoreFilename(""),
_namingContext(jmsNamingContext), 
_connectionFactory(jmsConnFactory),
_messages(NULL), 
_continue(true), 
_logger(NULL), 
_destination(""), 
_connected(false),
_pJmsContext(NULL), 
_pJmsConnFactory(NULL), 
_pJmsConnection(NULL), 
_pJmsSession(NULL),
_pubMgr(NULL) {

	_cxtMode = MODE_BYMYSELF;

	_hStop = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);
	_hInited = CreateEvent(NULL, false, false, NULL);	
}

JMSPublisher::JMSPublisher(
		JMSPublisherManager& pubMgr, 
		ZQ::JMSCpp::Connection& conn, 
		ZQ::JMSCpp::Session& session, 
		const char* desname):
_jmsServerIP(""), 
_jmsServerPort(0), 
_supportSafestore(false), 
_safestoreFilename(""),
_namingContext(""), 
_connectionFactory(""),
_messages(NULL), 
_continue(true), 
_logger(NULL), 
_destination(desname), 
_connected(false),
_pJmsContext(NULL), 
_pJmsConnFactory(NULL), 
_pJmsConnection(&conn), 
_pJmsSession(&session),
_pubMgr(&pubMgr) {

	_cxtMode = MODE_BYMANAGER;

	_hStop = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);
	_hInited = CreateEvent(NULL, false, false, NULL);
}

JMSPublisher::~JMSPublisher() {
	// release resource
	if(_messages != NULL) {
		delete _messages;
		_messages = NULL;
	}

	// close the handle
	if(_hStop != NULL) {
		CloseHandle(_hStop);
		_hStop = NULL;
	}
	if(_hNotify != NULL) {
		CloseHandle(_hNotify);
		_hNotify = NULL;
	}
	
	if(_hInited != NULL) {
		CloseHandle(_hInited);
		_hInited = NULL;
	}

	try	{
		if(_ic != NULL) {
			_ic->destroy();
			_ic = NULL;
		}
	}
	catch(const Ice::Exception& ex) {
		writeLog(ZQ::common::Log::L_ERROR, "Publisher's Ice communication destroy met exception with error: %s", 
						ex.ice_name().c_str());
	}
	catch(...) {
		writeLog(ZQ::common::Log::L_ERROR, "Publisher's Ice communication destroy met unknown exception");
	}
}

/// set the JMS header with Type, Key, Value

void JMSPublisher::setMsgHeader(const char* type, const char* key, const char* value) {
	TypeKeyValue header(type, key, value);

	_msgHeaders.push_back(header);
}

bool JMSPublisher::initializeSafeStore() {

	char sfPath[MAX_PATH] = {0};
	char sfName[MAX_PATH] = {0};;

	if("" == _safestoreFilename)
		return false;

	strcpy(sfPath, _safestoreFilename.c_str());

	char* p = strrchr(sfPath, FNSEPC);
	// find the last FNSEPC
	if(p == NULL)
	{
		writeLog(ZQ::common::Log::L_ERROR, "Invalid safestore file name %s", _safestoreFilename.c_str());
		return false;
	}
		
	// is the last char is FNSEPC?
	if(FNSEPC == _safestoreFilename[_safestoreFilename.size()-1])
	{
		writeLog(ZQ::common::Log::L_ERROR, "Invalid safestore file name %s", _safestoreFilename.c_str());
		return false;
	}

	// get the path and name
	strcpy(sfName, p+1);
	*p = '\0';

	// create the directory if it does not existed
	::CreateDirectoryA(sfPath, NULL);


	if(!_supportSafestore) {
		DeleteFileA(_safestoreFilename.c_str());
		char file[100];
		sprintf(file, "%s\\__catalog", sfPath);
		DeleteFileA(file);
		ZeroMemory(file, 100);
		sprintf(file, "%s\\log.0000000001", sfPath);
		DeleteFileA(file);
	}

	try {
		int i =0;
		_ic = Ice::initialize(i, NULL);
		
		_conn = Freeze::createConnection(_ic, sfPath);

		_messages = new Messages(_conn, sfName);

		if(_supportSafestore) {
			writeLog(ZQ::common::Log::L_INFO, 
					LOGFMT("(%d) messages of (%s) loaded"), 
					_messages->size(), _destination.c_str());
		}
	}
	catch(const Ice::Exception& ex)	{
		/* to avoid nested exception */
		delete _messages;
		_messages = NULL;

		writeLog(ZQ::common::Log::L_ERROR, 
					LOGFMT("failed to initialize safestore (%s\\%s): (%s)"), 
					sfPath, sfName, ex.ice_name().c_str());
		
		return false;
	}
	catch(...) {
		writeLog(ZQ::common::Log::L_ERROR, 
					LOGFMT("failed to initialize safestore (%s\\%s) with unknown error"), 
					sfPath, sfName);
		
		return false;
	}

 	return true;
}

bool JMSPublisher::initializeJMS() {

//	writeLog(ZQ::common::Log::L_INFO, LOGFMT("(%s) initializeJMS() enter"), _destination.c_str());

	//
	// uninitialize first
	//
	uninitializeJMS();

	
	if(MODE_BYMYSELF == _cxtMode)
	{
		//
		// Initialize JMS context
		//

		// create jms context
		writeLog(ZQ::common::Log::L_INFO, "Initialize JMS Context... , tid=0X%08X", GetCurrentThreadId()); 

		char jmsURL[256] = {0};
		sprintf(jmsURL, "%s:%d", _jmsServerIP.c_str(), _jmsServerPort);

		_pJmsContext = new ZQ::JMSCpp::Context(jmsURL, _namingContext.c_str());
		if(NULL == _pJmsContext || NULL == _pJmsContext->_context)
		{
			writeLog(ZQ::common::Log::L_ERROR, "Initialize JMS Context failed to %s:%d, naming context %s with error code %d",
						_jmsServerIP.c_str(), _jmsServerPort, _namingContext.c_str(), ZQ::JMSCpp::getLastJmsError());

			return false;
		}

		// create connection factory
		writeLog(ZQ::common::Log::L_INFO, "Create JMS connection factory... , tid=0X%08X", GetCurrentThreadId());
		_pJmsConnFactory = new ZQ::JMSCpp::ConnectionFactory();
		if(!_pJmsContext->createConnectionFactory(_connectionFactory.c_str(), *_pJmsConnFactory)
				|| NULL == _pJmsConnFactory->_connectionFactory)
		{		
			writeLog(ZQ::common::Log::L_ERROR, "Create Connection Factory failed on %s with error code %d",
						_connectionFactory.c_str(), ZQ::JMSCpp::getLastJmsError());

			return false;
		}

		// create connection
		writeLog(ZQ::common::Log::L_INFO, "creating JMS connection, tid=0X%08X", GetCurrentThreadId());
		
		_pJmsConnection = new ZQ::JMSCpp::Connection();
		if(!_pJmsConnFactory->createConnection(*_pJmsConnection) || 
			NULL == _pJmsConnection->_connection) {
			
			writeLog(ZQ::common::Log::L_ERROR, "Create Connection failed with error code %d", ZQ::JMSCpp::getLastJmsError());
			return false;
		}
		
		_pJmsConnection->SetConnectionCallback2(JMSPublisher::connectionMonitor, this);

		// create session
		writeLog(ZQ::common::Log::L_INFO, "creating JMS connection session, tid=0X%08X", GetCurrentThreadId());
		
		_pJmsSession = new ZQ::JMSCpp::Session();
		if(!_pJmsConnection->createSession(*_pJmsSession) || 
			NULL == _pJmsSession->_session) {

			writeLog(ZQ::common::Log::L_ERROR, "Create Session failed with error code %d", ZQ::JMSCpp::getLastJmsError());
			return false;
		}	

		// create destination for Topic/Queue
		writeLog(ZQ::common::Log::L_INFO, "creating JMS destination for (%s), tid=0X%08X", _destination.c_str(), GetCurrentThreadId());
		if(!_pJmsContext->createDestination(_destination.c_str(), _jmsDestination) || 
			NULL == _jmsDestination._destination) {

			writeLog(ZQ::common::Log::L_ERROR, "Create destination %s failed with error code %d", _destination.c_str(), ZQ::JMSCpp::getLastJmsError());
			return false;
		}

	}
	// Attention: The destination must be create in the same thread with Context,
	// otherwise it can not created successfully.

	// create producer
	writeLog(ZQ::common::Log::L_INFO, LOGFMT("(%s) creating JMS producer, tid=0X%08X"), _destination.c_str(), GetCurrentThreadId());
	if(!_pJmsSession->createProducer(&_jmsDestination, _jmsProducer) || 
		NULL == _jmsProducer._producer)	{

		writeLog(ZQ::common::Log::L_INFO, 
					LOGFMT("failed to create producer: (%d)"), 
					ZQ::JMSCpp::getLastJmsError());

		return false;
	}

	writeLog(ZQ::common::Log::L_INFO, LOGFMT("(%s) creating JMS text message, tid=0X%08X"), _destination.c_str(), GetCurrentThreadId());
	if(!_pJmsSession->textMessageCreate("", _jmsTxtMessage) || 
		NULL == _jmsTxtMessage._message) {

		writeLog(ZQ::common::Log::L_INFO, 
					LOGFMT("failed to create text message: (%d)"), 
					ZQ::JMSCpp::getLastJmsError());

		return false;
	}
	
	writeLog(ZQ::common::Log::L_INFO, 
				LOGFMT("(%s) creating JMS header properites"), 
				_destination.c_str());

	if(!createMsgHeaderProperites()) {
		writeLog(ZQ::common::Log::L_INFO, 
				LOGFMT("failed to create JMS header property: (%d)"), 
				ZQ::JMSCpp::getLastJmsError());

		return false;
	}
	_connected = true;

	return true;
}

bool JMSPublisher::uninitializeJMS()
{
	_jmsTxtMessage.DestroyMessage();
	_jmsProducer.close();

	if(MODE_BYMYSELF == _cxtMode)
	{
		_jmsDestination.destroy();

		if(_pJmsSession != NULL)
		{
			_pJmsSession->close();
			delete _pJmsSession;
		}

		if(_pJmsConnection != NULL)
		{
			_pJmsConnection->close();
			delete _pJmsConnection;
		}

		if(_pJmsConnFactory != NULL)
		{
			_pJmsConnFactory->Destroy();
			delete _pJmsConnFactory;
		}

		if(_pJmsContext != NULL)
		{
			delete _pJmsContext;
			_pJmsContext=NULL;
		}

		_pJmsSession = NULL;
		_pJmsConnection = NULL;
		_pJmsConnFactory = NULL;
		_pJmsContext = NULL;
	}
		
	return true;
}

bool JMSPublisher::createMsgHeaderProperites()
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
void JMSPublisher::connectionMonitor(int errType, const char* errMsg, VOID* lpData)
{
	// this event will always triggered. only Notify once.
	// Only re-connect JMS server when sending failed.
	
	JMSPublisher* pub = (JMSPublisher*)lpData;
	
	pub->setConnBroken(errType, errMsg);
}

/// Notify the connection broken
void JMSPublisher::setConnBroken(int errType, const char* errMsg, bool flag) 
{ 
	// this maybe called by PublishManager, if yes, no exactly errorCode and errMsg are available
	if(flag)
	{
		writeLog(ZQ::common::Log::L_ERROR, "JMSPublisher: JMS connection broken with errMsg: %s, errType:%d", 
			errMsg, errType);
	}

	_connected = false; 
	SetEvent(_hNotify); 
}

/// Add Message to the queue of Publisher, the JMS message is text typed
void JMSPublisher::publishMsg(const char* message)
{
	if(NULL == _messages) {
		return ;
	}

	if(!_connected && !_supportSafestore) {
		// in case of connection broken & no safestore for the message, 
		// just discard the message
		
		writeLog(ZQ::common::Log::L_DEBUG, "message (%s) discard", message);

		return;
	}

	TianShanIce::common::Message jmsMsg;
	jmsMsg.content = message;

	{
		ZQ::common::MutexGuard guard(_msgMutex);
	
		static short seq = 1;
		static time_t last = 0;

		std::ostringstream os;

		time_t now = time(0);

		os << now;
		if(now == last) {
			seq += 1;
		}
		else {
			last = now;
			seq = 1;
		}
		
		os << "-" << std::setfill('0') << std::setw(3) << seq;

		_messages->put(Messages::value_type(os.str(), jmsMsg));
	}

	if(_connected) {
		SetEvent(_hNotify);
	}
}

int JMSPublisher::run()	{
//	_connected = false;

	writeLog(ZQ::common::Log::L_INFO, LOGFMT("JMSPubliser(%s)::run() enter"), _destination.c_str());
	bool bRet = true;
	
	//
	// initialize the safestore
	//
	bRet = initializeSafeStore();
	if(!bRet) {
		_connected = false;

		SetEvent(_hInited);

		return 1;
	}

	//
	// initialize the JMS context
	//
	while(true) {
		bRet = initializeJMS();

		if(!bRet) {

			_connected = false;
			writeLog(ZQ::common::Log::L_ERROR, "failed to initialize JMS, retry in 5 seconds");
	
			int ret = WaitForSingleObject(_hStop, 5000);
			if (ret == WAIT_OBJECT_0) {
				SetEvent(_hInited);
				return 1;
			}
			
			continue;			
		}
		writeLog(ZQ::common::Log::L_INFO, 
					LOGFMT("publisher (%s) initialized successfully"), _destination.c_str());
		break;
	}

	// connection is created, start succeed
	_connected = true;

	SetEvent(_hInited);

	// trigger the message sending, if there is any message
	if(_messages->size() > 0) {
		SetEvent(_hNotify);
	}
	
	//
	//	entering message sending 
	//
	_continue = true;
	HANDLE handles[2] = { _hStop, _hNotify };

	unsigned long timeout = INFINITE;
	int retryCount = 0;

	while(_continue)
	{
		DWORD dwWaitStatus = WaitForMultipleObjects(2, handles, false, timeout);
		switch(dwWaitStatus)
		{
		case WAIT_OBJECT_0:
			_continue = false;
			break;

		case WAIT_OBJECT_0 + 1:
		case WAIT_TIMEOUT:
			/*
			*	1. the publisher is disconnected.
			*	2. JMS manager available and connected.
			*	ignore BYMYSELF here
			*/
			if(!_connected                  && 
			   ((MODE_BYMYSELF == _cxtMode) || 
			   (MODE_BYMANAGER == _cxtMode  && _pubMgr != NULL && _pubMgr->isConnected()))) {

				writeLog(ZQ::common::Log::L_INFO, LOGFMT("JMSPublisher (%s) reconnecting"), _destination.c_str());
				
				if(!initializeJMS()) {

					writeLog(ZQ::common::Log::L_ERROR, 
							LOGFMT("(%s) failed to reconnect, retry in (%d) seconds"), 
							_destination.c_str(), _jmsReconnInterval);

					timeout = _jmsReconnInterval * 1000;

					break;
				}

				timeout = INFINITE;

				writeLog(ZQ::common::Log::L_INFO, LOGFMT("(%s) connection recreated"), _destination.c_str());
				_connected = true;
				
				Sleep(2000);

				SetEvent(_hNotify);
			}
			/*
			*	the publisher is ready while manager not.
			*/
			else if(_connected                 && 
				    MODE_BYMANAGER == _cxtMode && 
				    _pubMgr != NULL            && 
					!_pubMgr->isConnected()) {

				// the manager JMS context is NOT initialized ready
				// skip and wait for manager's notification
				writeLog(ZQ::common::Log::L_ERROR, 
						LOGFMT("(%s) not ready, wait for notification"), 
						_destination.c_str());
				break;
			}

			//
			// send jms message
			//
			retryCount = 0;

			while(_connected && retryCount < MAX_JMS_SEND_RETRY_COUNT) {

				// sent successfully
				if(processMessage()) {
					break;
				}

				// send fail, retry
				retryCount ++;
				
				writeLog(ZQ::common::Log::L_WARNING, 
						LOGFMT("sent JMS message failed [%d / %d]"), 
						retryCount, MAX_JMS_SEND_RETRY_COUNT);

				Sleep(50);
			}

			if(_connected && (MAX_JMS_SEND_RETRY_COUNT == retryCount) ) {
				
				writeLog(ZQ::common::Log::L_INFO, 
						LOGFMT("Jms message sent fail after trying %d times, trigger JMSPublishManager to reconnect JMS server"), 
						MAX_JMS_SEND_RETRY_COUNT);
				// Still fail to send jms message after trying, need to actively reconnect JMS server
				if(_pubMgr) {

					// The condition is to avoid PublishManager trigger connection broken at the same time.
					// And, besides there, only publishManager set the flag when it received callback
					_pubMgr->setConnBroken(0, "Jms message sent fail after trying, need to reconnect JMS Server");
					_connected = false;
				}
				else
				{
					setConnBroken(0, "Jms message sent fail after trying, need to reconnect JMS Server");
				}

			}
			else  {

				ZQ::common::MutexGuard guard(_msgMutex);

				if(_messages->size() > 0) {
					SetEvent(_hNotify);
				}
			}
			break;

		default:
			break;
		}
	}

	writeLog(ZQ::common::Log::L_INFO, LOGFMT("JMSPubliser(%s)::run() leave"), _destination.c_str());
	return 0;
}

bool JMSPublisher::processMessage() {
	ZQ::common::MutexGuard guard(_msgMutex);
	
	if(_messages->size() > 0) {

		Messages::iterator iter = _messages->begin();

		std::string msgcontent = iter->second.content;

		ZQ::JMSCpp::ProducerOptions	pOption;
		ZeroMemory(&pOption,sizeof(ZQ::JMSCpp::ProducerOptions));
		pOption.flags = PO_TIMETOLIVE;
		pOption.timeToLive = 0;      // ms, 0 - no restriction

		// fullfill the message to jms object
		_jmsTxtMessage.setText((char*)msgcontent.c_str());

		if(!_jmsProducer.send(&_jmsTxtMessage, &pOption)) {
			writeLog(ZQ::common::Log::L_INFO, 
					LOGFMT("failed to send JMS message [%s]"), 
					msgcontent.c_str());

			return false;
		}
		else {
			writeLog(ZQ::common::Log::L_INFO, LOGFMT("JMS message sent [%s]"), msgcontent.c_str());

			_messages->erase(iter);

			return true;
		}
	}
	return true;
}

void JMSPublisher::final()
{
	uninitializeJMS();

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

	// delete the temp safestore file
	if(!_supportSafestore)
	{
		DeleteFileA(_safestoreFilename.c_str());
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
		writeLog(ZQ::common::Log::L_ERROR,  "Start JMS Publisher thread failed");

		return false;
	}

	//
	// wait for the initialization completion in the thread
	//
	WaitForSingleObject(_hInited, INFINITE);

	return _connected;
}

void JMSPublisher::stop()
{
	_continue = false;
	SetEvent(_hStop);

	waitHandle(1000);
}

void JMSPublisher::writeLog(ZQ::common::Log::loglevel_t level, char* fmt, ...)
{
	if(_logger == NULL)
		return;

	char logMsg[1024*2];
	memset(logMsg, 0x00, 1024*2*sizeof(char));

	va_list args;
	va_start(args, fmt);
	vsprintf(logMsg, fmt, args);
	va_end(args);
	
	char logMsgWID[1024*2];
//	_snprintf(logMsgWID, 510, "0x%08X, %s", GetCurrentThreadId(), logMsg);
	_snprintf(logMsgWID, 1024*2, "%s", logMsg);
	(*_logger)(level, logMsgWID);
}


////////////////////////////////////////////////////////////////////////
//           Implementation of JMSPublisherManager                    //
////////////////////////////////////////////////////////////////////////
JMSPublisherManager::JMSPublisherManager(
		const char* jmsServerIP, 
		unsigned long jmsServerPort, 
		unsigned long jmsReconnInterval, 
		const char* jmsNamingContext, 
		const char* jmsConnFactory):
_jmsServerIP(jmsServerIP), 
_jmsServerPort(jmsServerPort), 
_jmsReconnInterval(jmsReconnInterval),
_namingContext(jmsNamingContext), 
_connectionFactory(jmsConnFactory), 
_pJmsContext(NULL), 
_logger(NULL),
_continue(true), 
_connected(false) {

	_hStop = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);
	_hInited = CreateEvent(NULL, false, false, NULL);	
}

JMSPublisherManager::~JMSPublisherManager()
{
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
	
	if(_hInited != NULL)
	{
		CloseHandle(_hInited);
		_hInited = NULL;
	}
}

bool JMSPublisherManager::createPublisher(const char* desName) {

	PUBMAP::iterator it = _publishers.find(desName);
	if(it != _publishers.end()) {
		writeLog(ZQ::common::Log::L_ERROR, "JMSPublisher (%s) already created", desName);
		return false;
	}

	// create publisher
	JMSPublisher* pub = new JMSPublisher(*this, _jmsConnection, _jmsSession, desName);

	std::string destName = desName;
	_publishers.insert(PUBMAP::value_type(destName, pub));

//	writeLog(ZQ::common::Log::L_INFO, "JMSPublisher %s created successfully", desName);

	return true;
}

bool JMSPublisherManager::setSafeStore(const char* desName, const char* safestoreFilename, bool support) {
	PUBMAP::iterator it = _publishers.find(desName);
	if(_publishers.end() == it) {
		writeLog(ZQ::common::Log::L_ERROR, LOGFMT("JMSPublisher (%s) not exist"), desName);
		return false;
	}

	it->second->setSafeStore(safestoreFilename, support);
	writeLog(ZQ::common::Log::L_INFO, LOGFMT("safestore of (%s) set to (%s)"), desName, safestoreFilename);

	return true;
}


bool JMSPublisherManager::setMsgHeader(const char* desName, const char* type, const char* key, const char* value) {

	PUBMAP::iterator it = _publishers.find(desName);
	if(_publishers.end() == it) {
		writeLog(ZQ::common::Log::L_ERROR, "JMSPublisher (%s) not exist", desName);
		return false;
	}

	it->second->setMsgHeader(type, key, value);
	writeLog(ZQ::common::Log::L_INFO, "(%s) add header [Type (%s) Key (%s) Value (%s)]", 
				desName, type, key, value);

	return true;
}


bool JMSPublisherManager::publishMsg(const char* desName, const char* msg) {
	PUBMAP::iterator it = _publishers.find(desName);
	if(_publishers.end() == it)	{
		writeLog(ZQ::common::Log::L_ERROR, LOGFMT("JMSPublisher (%s) does not exist"), desName);
		return false;
	}

	it->second->publishMsg(msg);

	return true;
}

bool JMSPublisherManager::start() {
	//
	// invoke base start() routine
	//
	bool bRet = NativeThread::start();
	if(!bRet) {
		writeLog(ZQ::common::Log::L_ERROR,  LOGFMT("failed to start JMSPublisherManager"));

		return false;
	}

	//
	// wait for the initialization completion in the thread
	//
	WaitForSingleObject(_hInited, INFINITE);

	return _connected;
}

void JMSPublisherManager::stop()
{
	// stop all the publishers
	stopPubs();

	// stop myself
	_continue = false;
	SetEvent(_hStop);
	
	// must wait, coz the JMS object is initialized on its thread, 
	// and the this object's destruct also try to uninitialize jms object, 
	// but there is no lock.
	waitHandle(5000);
}

int JMSPublisherManager::run()
{
//	_connected = false;

	writeLog(ZQ::common::Log::L_INFO, "JMSPublisherManager::run() enter");
	bool bRet = true;
	
	//
	// initialize the JMS context
	//
	while(true) {
		bRet = initializeJMS();
		if(!bRet) {
			_connected = false;
			
			writeLog(ZQ::common::Log::L_ERROR, 
					LOGFMT("failed to initialize JMS, retry in 5 seconds"));
	
			int ret = WaitForSingleObject(_hStop, 5000);
			if (ret == WAIT_OBJECT_0) {
				SetEvent(_hInited);
				return 1;
			}
			
			continue;			
		}
		writeLog(ZQ::common::Log::L_INFO, 
				LOGFMT("JMSPublisherManager initialized successfully"));

		_connected = true;

		break;
	}

	//
	// start all the publishers
	//
	bRet = startPubs();
	if(!bRet) {

		_connected = false;

		SetEvent(_hInited);
		return 1;
	}
	

	SetEvent(_hInited);

//	writeLog(ZQ::common::Log::L_INFO, "JMS publishers have been started");

	//
	//	entering manager checking thread
	//
	_continue = true;
	HANDLE handles[2] = { _hStop, _hNotify };

	unsigned long timeout = INFINITE;

	while(_continue)
	{
		DWORD dwWaitStatus = WaitForMultipleObjects(2, handles, false, timeout);
		switch(dwWaitStatus){

		case WAIT_OBJECT_0:
			_continue = false;
			break;
		
		case WAIT_OBJECT_0 + 1:
		case WAIT_TIMEOUT:
			// if the connection is lost, do not send the message
			if(!_connected) {

				writeLog(ZQ::common::Log::L_INFO, "connection broken, notify publishers");

				// notify the publishers that network is broken
				notifyNetbrkPubs();
				Sleep(500);
				
				writeLog(ZQ::common::Log::L_INFO, "reinitializing JMS");

				// the JMS publisher init failed, keep to do initialization next loop
				if(!initializeJMS()) {
					writeLog(ZQ::common::Log::L_ERROR, 
							LOGFMT("failed to reinitialize, retry in (%d) seconds"), _jmsReconnInterval);

					_connected = false;

					timeout = _jmsReconnInterval * 1000;

					break;
				}

				// the connection is ready
				DWORD timeout = INFINITE;

				_connected = true;

				// notify the publishers that the manager have accomplished the JMS context has been initialized
				// here I invoke notifyNetbrkPubs(), coz it has SetEvent to publishers and now the Manager's _connected = true
				writeLog(ZQ::common::Log::L_INFO, "connection recreated, notify publishers");
				notifyNetbrkPubs();
			}
			break;

		default:
			break;
		}
	}

	writeLog(ZQ::common::Log::L_INFO, "JMSPublisherManager::run() leave");
	return 0;
}

void JMSPublisherManager::final()
{
	writeLog(ZQ::common::Log::L_INFO, "JMSPublisherManager::final() enter");

	uninitializeJMS();

	bool bRet = true;

	PUBMAP::iterator it = _publishers.begin();
	for(; it != _publishers.end(); it++)
	{
		JMSPublisher* pub = (JMSPublisher*)it->second;
		delete pub;

		it->second = NULL;
	}
	_publishers.clear();

	writeLog(ZQ::common::Log::L_INFO, "JMSPublisherManager::final() leave");
}


bool JMSPublisherManager::initializeJMS() {

//	writeLog(ZQ::common::Log::L_INFO, "JMSPublisherManger::initializeJMS() enter");

	//
	// uninitialize first
	//
	uninitializeJMS();

	
	//
	// Initialize JMS context
	//

	// create jms context
	writeLog(ZQ::common::Log::L_INFO, 
			LOGFMT("creating JMS Context (%s), tid=0X%08X"), _namingContext.c_str(), GetCurrentThreadId()); 

	char jmsURL[256] = {0};
	sprintf(jmsURL, "%s:%d", _jmsServerIP.c_str(), _jmsServerPort);

	_pJmsContext = new ZQ::JMSCpp::Context(jmsURL, _namingContext.c_str());
	if(NULL == _pJmsContext || NULL == _pJmsContext->_context) {

		writeLog(ZQ::common::Log::L_ERROR, 
				LOGFMT("failed to initialize JMS Context (%s): (%d)"),
					_namingContext.c_str(), ZQ::JMSCpp::getLastJmsError());

		return false;
	}

	writeLog(ZQ::common::Log::L_INFO, 
				LOGFMT("creating JMS connection factory (%s), tid=0X%08X"),
				_connectionFactory.c_str(), GetCurrentThreadId());

	if(!_pJmsContext->createConnectionFactory(_connectionFactory.c_str(), _jmsConnFactory)
			|| NULL == _jmsConnFactory._connectionFactory) {
		
		writeLog(ZQ::common::Log::L_ERROR, 
				LOGFMT("failed to create connection factory (%s): (%d)"),
				_connectionFactory.c_str(), 
				ZQ::JMSCpp::getLastJmsError());

		return false;
	}

	writeLog(ZQ::common::Log::L_INFO, "creating JMS connection, tid=0X%08X", GetCurrentThreadId()); 
	if(!_jmsConnFactory.createConnection(_jmsConnection) || 
		NULL == _jmsConnection._connection) {

		writeLog(ZQ::common::Log::L_ERROR, 
				LOGFMT("failed to create connection: (%d)"), 
				ZQ::JMSCpp::getLastJmsError());

		return false;
	}

	// set connection callback
	_jmsConnection.SetConnectionCallback2(JMSPublisherManager::connectionMonitor, this);
	
	// start the connection
	if(!_jmsConnection.start()) {
		writeLog(ZQ::common::Log::L_ERROR, 
				LOGFMT("failed to start connection: (%d)"), 
				ZQ::JMSCpp::getLastJmsError());

		return false;
	}

	// create session
	writeLog(ZQ::common::Log::L_INFO, "creating JMS session"); 
	if(!_jmsConnection.createSession(_jmsSession) || 
		NULL == _jmsSession._session) {
		writeLog(ZQ::common::Log::L_ERROR, 
				LOGFMT("failed to create session: (%d)"), 
				ZQ::JMSCpp::getLastJmsError());

		return false;
	}

	PUBMAP::iterator it = _publishers.begin();
	for(; it != _publishers.end(); it++) {
		
		// create destination for Topic/Queue
		writeLog(ZQ::common::Log::L_INFO, 
				LOGFMT("creating JMS destination (%s)"), 
				it->first.c_str());

		if(!_pJmsContext->createDestination(it->first.c_str(), it->second->_jmsDestination) || 
			NULL == it->second->_jmsDestination._destination) {

			writeLog(ZQ::common::Log::L_ERROR, 
					LOGFMT("failed to create destination (%s): (%d)"), 
					it->first.c_str(), 
					ZQ::JMSCpp::getLastJmsError());

			return false;
		}
	}


	//
	// set connection is ready flag
	// 
	_connected = true;

//	writeLog(ZQ::common::Log::L_INFO, "JMSPublisherManager::initializeJMS() leave");
	
	return true;
}

bool JMSPublisherManager::uninitializeJMS() {
//	writeLog(ZQ::common::Log::L_INFO, LOGFMT("JMSPublisherManager::uninitializeJMS() enter"));

	// destroy all the publisher's destination
	PUBMAP::iterator it = _publishers.begin();
	for(; it != _publishers.end(); it++) {		
		it->second->_jmsDestination.destroy();
	}

	_jmsSession.close();
	_jmsConnection.close();
	_jmsConnFactory.Destroy();

	if(_pJmsContext != NULL)
	{
		delete _pJmsContext;
		_pJmsContext=NULL;
	}
		
//	writeLog(ZQ::common::Log::L_INFO, LOGFMT("JMSPublisherManager::uninitializeJMS() leave"));
	return true;
}

bool JMSPublisherManager::startPubs() {

	bool bRet = true;

	PUBMAP::iterator it = _publishers.begin();
	for(; it != _publishers.end(); it++) {
		it->second->setLogger(_logger);

		writeLog(ZQ::common::Log::L_INFO, 
				LOGFMT("starting JMS Publisher (%s)"), 
				it->first.c_str());

		bRet = it->second->start();
		if(!bRet) {
			return false;
		}
	}

	return true;
}

bool JMSPublisherManager::stopPubs() {
	PUBMAP::iterator it = _publishers.begin();
	for(; it != _publishers.end(); it++) {
		
		writeLog(ZQ::common::Log::L_INFO, 
				LOGFMT("stopping JMS Publisher (%s)"), 
				it->first.c_str());

		it->second->stop();
	}
	
	return true;
}

bool JMSPublisherManager::notifyNetbrkPubs()
{
	PUBMAP::iterator it = _publishers.begin();
	for(; it != _publishers.end(); it++) {
		it->second->setConnBroken(0, "", false);
	}

	return true;
}

void JMSPublisherManager::setConnBroken(int errType, const char* errMsg) 
{ 
	writeLog(ZQ::common::Log::L_ERROR, "JMSPublisherManager: JMS connection broken with errMsg: %s, errType:%d", 
		errMsg, errType);

	_connected = false; 
	SetEvent(_hNotify); 
};

void JMSPublisherManager::connectionMonitor(int errType, const char* errMsg, VOID* lpData) {
	// this event will always triggered. only Notify once.
	// Only re-connect JMS server when sending failed.	

	JMSPublisherManager* pubMgr = (JMSPublisherManager*)lpData;
	
	pubMgr->setConnBroken(errType, errMsg);
}


void JMSPublisherManager::writeLog(ZQ::common::Log::loglevel_t level, char* fmt, ...)
{
	if(_logger == NULL)
		return;

	char logMsg[1024*2];
	memset(logMsg, 0x00, 1024*2*sizeof(char));

	va_list args;
	va_start(args, fmt);
	vsprintf(logMsg, fmt, args);
	va_end(args);
	
	char logMsgWID[1024*2];
//	_snprintf(logMsgWID, 510, "0x%08X, %s", GetCurrentThreadId(), logMsg);
	_snprintf(logMsgWID, 1024*2, "%s", logMsg);
	(*_logger)(level, logMsgWID);
}






}}

