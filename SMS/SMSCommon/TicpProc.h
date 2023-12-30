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

	// @param[in] mode:		   ������־(PLAY,CHAT,REGISTER)
	// @param[in] phoneNumber: �ֻ�����
	// @param[in] sequence:	   ���к�
	// @param[in] sendTime:	   �û����ŵķ���ʱ��
	// @param[in] content:	   �ֻ����ݻ�����
	// return value :		   ��������ֵ
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


	// ���ܺͷ��͸� TICP ������
	// @param[in] xmlfile		: Ҫ���͵�xml
	// @param[in] returnXmlfile : ���ܵ���xml
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