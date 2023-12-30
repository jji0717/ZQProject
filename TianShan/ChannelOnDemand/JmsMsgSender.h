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
#include <vector>
#include <Locks.h>
//include jms healper file
#include <JndiClient.h>

class JmsMsgSender : virtual public ZQ::JndiClient::JmsSession 
{
public:
	JmsMsgSender(::ZQ::JndiClient::ClientContext& context, const std::string& dstType, const std::string& destination);
	virtual ~JmsMsgSender();

	// connection status callback
	virtual void OnConnected(const std::string& notice);
	virtual void OnConnectionLost(const std::string& notice);

	struct InitInfo
	{
		std::string strClassPath;
		std::string strJavaHome;

		std::string strNamingCtx;
		std::string strSrvIpPort;
		std::string strDestName;
		std::string strConnFactory;
		std::string strSafestoreFile;
		std::string strMsgProperty;
		int		nReConnectCount;
		int		nReConnectInterval;
		int		nFlushToFileCount;	
		int		nKeepAliveTime;
		int     nLogLevel;
		InitInfo()
		{
			nReConnectCount = 0;
			nReConnectInterval = 0;
			nFlushToFileCount = 0;
			nKeepAliveTime = 0;
		}
	};
public:
	virtual bool init(InitInfo& initInfo);
	virtual void close();

	void SendMsg(const std::string& strMsg);

	// send all none-sent message, true for all none-sent message sent, false for stil exist none-sent message
	bool SendAllMsg(); 
protected:
     void buildMessageProperties(ZQ::JndiClient::ClientContext::Properties& props);
	// Initialize JMS environment
	bool		InitializeJMS();
	// connect to server 
	bool		ConnectToServer();
	//clear connection instance
	bool		ClearConnInstance();
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
	void			PushNoneSendMessage(const std::string&	strMsg,bool bHead);
	
	//pop none send message from queue
	std::string		PopNoneSendMessage();
	//This vector is used to record the message which 
	//hasn't been sent due to some bad condition
	std::deque<std::string>			m_vNoneSendMessageHeader;
	std::deque<std::string>			m_vNoneSendMessageTail;
private:
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
	
	long							m_lConnectionLostTime;
		
	bool							m_bConnectionOK;

	ZQ::common::Mutex				m_sendMutex;

	ZQ::JndiClient::ClientContext::Properties _msgProperties;

};

#endif // !defined(AFX_JMSMSGSENDER_H__F20E5F34_48ED_414B_AC7F_8E34FE267BDD__INCLUDED_)
