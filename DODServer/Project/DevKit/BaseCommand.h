#pragma once

class CBaseCommand
{
public:
	CBaseCommand();
	~CBaseCommand();

public:

	//the importancest founction .
	//waitting received message in promised time
	//return -1,if waitting time is bigger than m_nOverTimeLong.
	int Execute();

public:

	//identify each message 
	int m_nCommandID;
	HANDLE m_HWaitEvent;
	int m_nOverTimeLong;
	int m_nSessionID;
	int m_nPortID;
	int m_nChannelID;
	int m_nRetuenCommand;
	int m_nSend1,m_nSend2,m_nSend3;
	CString m_strSend1,m_strSend2,m_strSend3;
	int m_nReturn1,m_nReturn2,m_nReturn3;
	CString m_strReturn1,m_strReturn2,m_strReturn3;
/*
private:
	virtual int paremeterWrap(){return 0;}
	virtual int paremeterUnWrap(){return 0;}
	virtual int ReceiveReturn(){return 0;}*/

// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê7ÔÂ11ÈÕ 10:09:02

	BOOL m_bStop;
};

/*
class CConcreteCom: public CBaseCommand
{
public:
	int	m_nReturn;
private:
	int paremeterWrap();
	int paremeterUnWrap();
	int ReceiveReturn();

};*/