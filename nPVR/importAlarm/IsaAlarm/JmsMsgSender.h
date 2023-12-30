// JmsMsgSender.h: interface for the JmsMsgSender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JMSMSGSENDER_H__F20E5F34_48ED_414B_AC7F_8E34FE267BDD__INCLUDED_)
#define AFX_JMSMSGSENDER_H__F20E5F34_48ED_414B_AC7F_8E34FE267BDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <deque>
#include "KeyDefine.h"
#include "BaseMessageReceiver.h"
//include jms healper file

#include "JMSCpp/header/Jms.h"
#include "JMSCpp/jmshead.h"

typedef void* HANDLE;

class JmsMsgSender : public BaseMessageReceiver 
{
public:
	JmsMsgSender(int iChannelID);
	virtual ~JmsMsgSender();
public:
	virtual bool init(InitInfo& initInfo, const char* szSessionName);
	virtual void close();

	virtual void OnMessage(int nMessageID, MessageFields* pMessage);

	virtual void requireFields(std::vector<std::string>& fields);

	static const char* getTypeInfo()
	{
		return KD_KV_RECEIVERTYPE_JMSSENDER;
	}
public:
	//monitor connection status
	static	void	ConnectionMonitor(int errType,void* lpData);
	//set connection status
	void			SetConnectionStatus(bool bOK){m_bConnectionOK=bOK;}	
protected:
	//create message property from ini file
	bool		CreateMessageProperty();
	// Initialize JMS environment
	bool		InitializeJMS();
	// connect to server 
	bool		ConnectToServer();
	// send message
	bool		SendJMSmessage();
	//uninitialize jms
	void		UnInitializeJMS();
	//send message
	bool		InternalSendMessage(std::string	strMsg);
private:

	
	bool			HasNoneSendMessageInFile();

	int				GetLineLogContent(char* buf, char** pline);	

	bool			DeleteLogContent(long pos,bool bFront);
	bool			AddLogContent(std::deque<std::string>& vContent,bool bFront);
	bool			CopyFileContent(HANDLE hSrc,long posSrc,HANDLE hDst,long posDst);
	//if there is any none send message
	bool			HasNoneSendMessage();

	//push message into none send message queue
	void			PushNoneSendMessage(std::string&	strMsg,bool bHead);
	
	//pop none send message from queue
	std::string		PopNoneSendMessage();
	//This vector is used to record the message which 
	//hasn't been sent due to some bad condition
	std::deque<std::string>			m_vNoneSendMessageHeader;
	std::deque<std::string>			m_vNoneSendMessageTail;
private:
	ZQ::JMSCpp::Context					*m_pJmsContext;
	ZQ::JMSCpp::ConnectionFactory		m_JmsCNFactory;
	ZQ::JMSCpp::Connection				m_JmsConnection;
	ZQ::JMSCpp::Producer				m_JmsProducer;
	ZQ::JMSCpp::Destination				m_JmsDestination;
	ZQ::JMSCpp::Session					m_JmsSession;
	ZQ::JMSCpp::TextMessage				m_JmsTxtMessage;
	ZQ::JMSCpp::ProducerOptions						m_MsgOption;
	

	std::string						m_strServerAddress;
	std::string						m_strNamingContext;
	std::string						m_strDestinationName;		
	std::string						m_strConnectionFactory;
	
	std::string						m_strMsgStoreFile;
	//re-connect count
	int								m_iReconnectCount;
	//How many millisecond pause before re-connect
	int								m_iReconnectInterval;
	//
	int								m_iNoneSendMsgFlushCount;

//	DWORD							m_dwSendMsgCycleCount;
//	DWORD							m_dwMillCountReConnect;
//	DWORD							m_dwMillIntervalSendMsg;
//	DWORD							m_dwShutDownWaitTime;
//	DWORD							m_KeepAliveTime;
	
	bool							m_bConnectionOK;
	bool							m_bJmsInitializeOK;
private:
	static	const char*				_requiredFields[];
	static  int						_nRequiredField;
	std::string						m_strIniFilePath;
};

#endif // !defined(AFX_JMSMSGSENDER_H__F20E5F34_48ED_414B_AC7F_8E34FE267BDD__INCLUDED_)
