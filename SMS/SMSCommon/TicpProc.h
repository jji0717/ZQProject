#ifndef TICPPROC_H
#define TICPPROC_H

#include "NPVRSocket.h"
#include "SMSXmlProc.h"

#include "Log.h"
#include "ScLog.h"
using namespace ZQ::common;

#define    PLAY        1
#define    CHAT        2
#define    REGISTER    3
#define    NICKNAME    4

class TicpProc
{
public:
	TicpProc(char* ip, int port, DWORD timeout);
	~TicpProc();

	// @param[in] mode:		   操作标志(PLAY,CHAT,REGISTER)
	// @param[in] phoneNumber: 手机号码
	// @param[in] sequence:	   序列号
	// @param[in] sendTime:	   用户短信的发送时间
	// @param[in] content:	   手机内容或者是
	// return value :		   操作返回值
	int	TicpProcess(int		mode, 
					char*	phoneNumber, 
					int		sequenceID, 
					char*	sendTime, 
					char*	content);

private:
	/*************     play     *************/
	bool sendPlay();

	// identify XML file
	void generateIdentifyRequest(char* identifyXmlFile);

	// play XML file
	void generatePlayRequest(char* playXmlFile);
	
	
	/*************     chat     *************/
	bool sendChat();

	// chat XML file 
	void generateChatRequest(char* chatXmlFile);


	/*************   register   *************/
	bool sendRegister();

	// register XML file
	void generateRegisterRequest(char* regXmlFile);


	/*************   nick name   *************/
	bool sendNickName();

	// nick name XML file
	void generateNickNameRequest(char* nickNameXmlFile);

	
	// parse xml file
	bool ProcessRespXmlfile(char* Xmlfile);


	// 接受和发送给 TICP 服务器
	// @param[in] xmlfile		: 要发送的xml
	// @param[in] returnXmlfile : 接受到的xml
	bool SendAndRecv(char* xmlfile, char* returnXmlfile);

private:
	int		m_ret;
	int		m_mode;
	int		m_sequenceID;
	char	m_userID[128];
	char    m_sendTime[20];
	char	m_content[1000];
	char	m_phoneNumber[50];

	char	m_ip[20];
	int		m_port;
	DWORD	m_timeout;

	NPVRSocket* m_pTicpSocket;
	SMSXmlProc  m_pXmlProc;
};

#endif