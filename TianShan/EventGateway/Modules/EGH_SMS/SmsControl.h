#pragma once

#include "NativeThread.h"
#include "SmsCommon.h"
#include "Log.h"
#include "Locks.h"
#include <string>
#include <deque>


class SmsControl : public ZQ::common::NativeThread
{
public:
	SmsControl(void);
	~SmsControl(void);

	//set sms attribute
	//set the serial port name,default is "COM1"
	void SetPort(const char* pPort = "COM1");
	//set short message center number
	void SetCenterNum(const char* pCenterNum);
	//set message live time,from 5  to 24*60 minute,default is 60 minute 
	void SetMsgLiveTime(int nLT = 60);
	//set message code type UNICODE ou ASCII,set bUnicode true is UNICODE code type,else ASCII
	void SetMsgUnicodeType(bool bUnicode = true);
	//set send interval,unit is second default is 10 second
	void SetSendInterval(int nInterval = 10);

	void SetLog(ZQ::common::Log* pLog);

	bool init(void);
	void close(void);

	//add message to send
	void AddMsg(const char* pTel, const char* pMsg);
	
	typedef struct smsInfo
	{
		std::string	strTel;
		std::string	strMsg;
	}SMSINFO;
	
	typedef enum baudrate
	{
		BR4800,
		BR9600,
		BR19200
	}BAUDRATE;

	typedef enum parity
	{
		PARNO,
		PAREVEN,
		PARODD
	}PARITY;

	typedef enum bytesize
	{
		BS7,
		BS8
	}BYTESIZE;	
	
	typedef enum stopbit
	{
		SB1,
		SB2
	}STOPBIT;

private:
		
	bool OpenCom(const char* pPort, BAUDRATE bRate=BR9600, PARITY par=PARNO, BYTESIZE bSize=BS8, STOPBIT sBit=SB1);
	bool InitCom();
	void CloseCom();

	int SendMessage(nsSMS::SM_PARAM* pSrc);
	int ReadMessageList();
	int DeleteMessage(int index);
	int GetResponse(nsSMS::SM_BUFF* pBuff);
	int ParseMessageList(nsSMS::SM_PARAM* pMsg, nsSMS::SM_BUFF* pBuff);
	
	int SendMessage(const char* pNum, const char* chContent);
	
private:
	int WriteCom(void* pData, int nLength);
	int ReadCom(void* pData, int nLength);

	
protected:

	virtual int run(void);


private:
	ZQ::common::Log*			_pLog;
	bool						_bQuit;

	std::deque<SMSINFO>			_msgQue;
	ZQ::common::Mutex			_lock;

	nsSMS::SmsCommon			_smsCom;

#ifdef ZQ_OS_MSWIN
	HANDLE						_hCom;

	HANDLE						_hStop;
	HANDLE						_hMsg;
#else
	int							_hCom;
	sem_t						_hMsg;	
#endif

	char						_chSmsc[20];//短信中心号码
	int							_nSmLT;//短信有效期，单位分钟,defualt value is 60 minute
	char						_chPort[10];//serial port name com1 or com2 ...

	bool						_bMsgUnicode;//编码方式，true is 16 bit 编码，false 8 bit,default is true
	int							_nSendInterval;

};

