#ifndef JMSPROCTHREAD_H
#define JMSPROCTHREAD_H

#define BACKUPLEN 50
#define IP_LENGTH 20

#include "afx.h"
#include <queue>
#include "Locks.h"
#include "NativeThread.h"
#include "JMSCpp/jmshead.h"
#include "cfgpkg.h"
#include "ManualSyncDef.h"
#include "XMLProc.h"

class JmsProcThread : public ZQ::common::NativeThread
{
	typedef std::queue<std::string> STRQUEUE;
public:
	JmsProcThread(ZQ::common::Log * pLog);
	~JmsProcThread();

public:
	bool      init();
	int       run();
	void      final();

	void      StopThread();
	void      SetConStart() { SetEvent(m_hConStart); };
	void      SetConBroken() { SetEvent(m_hConBroken); m_bCreateProducer = false; m_bConBroken = true;};

private:
	/************************************************************************/
	/*        Jms Function                                                  */
	/************************************************************************/
	bool      InitJMS(); 
	bool      StartJMS();
	bool      SetMessageProperty();
	void	  UnInitJms();
	void      SetReplyAndProducer();

	static void connectionMonitor(int errType,VOID* lpData);

	/************************************************************************/
	/*        Configuration Function                                        */
	/************************************************************************/
	void      GetHostName();
	void	  GetConfiguration();
	void	  GetConfiguration(wchar_t* app_name, std::string& buf);
	void	  GetConfiguration(wchar_t* app_name, DWORD& buf);
	void      Unicode2Ansi(wchar_t* wch, char* ch);
	
	/************************************************************************/
	/*        XML Proc Function                                             */
	/************************************************************************/
	void      SendStartXMLFile(_LAM_to_DBSync* ltd);
	void      GenerateStartXMLFile(char* xml, _LAM_to_DBSync* ltd);

	void      SendCompleteXMLFile(_DBSync_to_LAM* dtl);
	void      GenerateCompleteXMLFile(char* xml, _DBSync_to_LAM* dbSync_to_Lam);

	bool      ParseXMLFile(char* xml, _LAM_to_DBSync* lam_to_DBSync);

	bool      DumpBufQueue();

private:
	char      m_hostname[IP_LENGTH];
	HANDLE	  m_hCfgHandle;
	HANDLE    m_hConStart;
	HANDLE    m_hStop;
	HANDLE    m_hConBroken;
	bool      m_bConBroken;
	bool      m_bCreateProducer;
	bool      m_bInitialize;
	bool      m_bContinued;
	XMLProc	  m_XmlProc;

	DWORD                           m_ServerPort;
	std::string						m_ServerIP;
	std::string						m_NamingContext;
	std::string						m_ConnectionFactory;
	std::string						m_SendDestinationName;
	std::string						m_RecvDestinationName;
	STRQUEUE                        m_SendBufQueue;

	ZQ::common::Log*                m_Log;

	// JMS Variables
	ZQ::JMSCpp::Context*            m_pJmsContext;
	ZQ::JMSCpp::ConnectionFactory*  m_JmsCNFactory;
	ZQ::JMSCpp::Connection*         m_JmsConnection;
	ZQ::JMSCpp::Session*            m_JmsSession;
	ZQ::JMSCpp::Destination			m_JmsSendDestination;
	ZQ::JMSCpp::Destination			m_JmsRecvDestination;
	ZQ::JMSCpp::Producer			m_JmsProducer;
	ZQ::JMSCpp::Consumer			m_JmsConsumer;
	ZQ::JMSCpp::TextMessage			m_JmsTxtMessage;

	Mutex m_TaskMutex;
};

#endif