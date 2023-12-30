// MsgManager.h: interface for the CMsgManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MSGMANAGER_H__0A5F912F_1D76_4FA4_8B40_33EAA1966ABB__INCLUDED_)
#define AFX_MSGMANAGER_H__0A5F912F_1D76_4FA4_8B40_33EAA1966ABB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "JMSCpp/header/Jms.h"
#include "JMSCpp/jmshead.h"
#include "../../doc/DBSAdi_def.h"
//#include "ini.h"
#include <vector>
#include <string>
#pragma warning(disable:4786)


#define	CFG_FROM_REGISTRY	1


typedef	std::string				JMSMsg;
typedef	std::vector<JMSMsg>		MsgStack;


class CMsgManager  
{
public:
	CMsgManager();
	virtual ~CMsgManager();

public:
	//Monitor connection status
	static	VOID	ConnectionMonitor(int errType,VOID* lpData);
	//The thread for send message
	static DWORD	WINAPI	SendMessageThread(LPVOID lpData);
	//Send the message using JMS
	VOID		SendJmsMessage();
	//push message into stack
	VOID		PushMessage(JMSMsg& strMsg);
	//pop up message from stack
	JMSMsg		PopMessage();
	//check if the stack has message
	BOOL		HasMessage();
	//Set thread status to running
	VOID		SetThreadStatus(BOOL bRunning){m_bThreadRunning=bRunning;}
	//Get thread status 
	BOOL		GetThreadStatus(){return m_bThreadRunning;}
	//check if initialize is OK
	BOOL		IsInitOK(){return m_bInitializeOK;}
	//Check if connection is ok
	BOOL		GetConnectionStatus(){return m_bConnectionOK;}
	//Set Connection Status
	VOID		SetConnectionStatus(BOOL bOK){m_bConnectionOK=bOK;}
	//Connect to host
	BOOL		ConnetToHost();
	//Get Reconnect time
	DWORD		GetReconnectTime(){return m_dwMillCountReConnect;}
	//Get Send message interval time
	DWORD		GetSendMsgIntervalTime(){return m_dwMillIntervalSendMsg; }
	//Get shut down wait time
	DWORD		GetShutDownWaitTime(){return m_dwShutDownWaitTime;}
public:

#ifdef TEST
	JMSMsg		GenerateTestMsg();
	DWORD		m_dwCount;
#endif
	
	JMSMsg		GenerateInitialize(DA_dbsyncInfo*	pDbsInfo,
									DA_itvInfo*		pItvInfo );
	JMSMsg		GenerateUninitialize();
	JMSMsg		GenerateTriggerState(DA_entryDb*	pEntryBlock,
									DA_stateDb*		pStateBlock);
	JMSMsg		GenerateSyncBegin();
	JMSMsg		GenerateSyncEnd();
	JMSMsg		GenerateTriggerMd(	DA_entryDb*			pEntryBlock,
									DWORD				dwMdNumber,
									DA_metaDb*			pFirstMdBlock);
private:
	BOOL		CreateMessageProperty();
public:
		///Send message thread
	HANDLE							m_hSendMessage;

private:
	JMSMsg		GenerateMsgHeader( );
	VOID		GenerateMsgFooter(JMSMsg& str);
	VOID		PutTabbledChar(std::string& str,int n);
	VOID		BreakeLine(std::string& str);
	VOID		CreateBodyHeader(JMSMsg& str,int ID);
	VOID		CreatreBodyFooter(JMSMsg& str);
protected:
	BOOL		GetConfiguration();	
	BOOL		InitializeJMS();
	VOID		UnitializeJMS();
private:	
	//JMS Message header
	JMSMsg							m_strMessageHeader;	
	//These two var must be initialized in function DBSA_Initialize
	DA_dbsyncInfo					m_DbsInfo;
	DA_itvInfo						m_ItvInfo;
	//stack for reserve message
	MsgStack						m_vMsgStack;

	ZQ::JMSCpp::Context				*m_pJmsContext;
	ZQ::JMSCpp::ConnectionFactory	m_JmsCNFactory;
	ZQ::JMSCpp::Connection			m_JmsConnection;
	ZQ::JMSCpp::Producer			m_JmsProducer;
	ZQ::JMSCpp::Destination			m_JmsDestination;
	ZQ::JMSCpp::Session				m_JmsSession;
	ZQ::JMSCpp::TextMessage			m_JmsTxtMessage;
	ZQ::JMSCpp::Consumer			m_JmsConsumer;

	std::string						m_strServerAddress;
	std::string						m_strServerIP;
	DWORD                           m_dwServerPort;
	std::string						m_strNamingContext;
	std::string						m_strDestinationName;	
	std::string						m_strLogPath;
	std::string						m_strConnectionFactory;
	std::string						m_strIniFilePath;

	DWORD							m_dwLogBufferSize;
	DWORD							m_dwLogFileSize;
	DWORD							m_dwLogWriteTimeOut;
	DWORD							m_dwSendMsgCycleCount;
	DWORD							m_dwMillCountReConnect;
	DWORD							m_dwMillIntervalSendMsg;
	DWORD							m_dwShutDownWaitTime;
	//Add to support duration time
	DWORD							m_KeepAliveTime;
	

#if CFG_FROM_REGISTRY
	HANDLE							m_hCfgHandle;
#endif


	CRITICAL_SECTION				m_Section;
	///Is Send thread running???
	BOOL							m_bThreadRunning;
	//Is initialize OK
	BOOL							m_bInitializeOK;
	//Is Connection OK
	BOOL							m_bConnectionOK;

	
	std::string						m_strVersion;
	std::string						m_strAddInIP;
};

#endif // !defined(AFX_MSGMANAGER_H__0A5F912F_1D76_4FA4_8B40_33EAA1966ABB__INCLUDED_)
