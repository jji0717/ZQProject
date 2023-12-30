#include "SmsControl.h"
#include "SystemUtils.h"
#ifdef ZQ_OS_LINUX
extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h> 
#include <errno.h>
#include <string.h>
}
#endif

using namespace ZQ::common;
using namespace nsSMS;

#define SMS_MSG_LIVETIME	60//short message live time,unit is minute
//send message interval,unit is second
#define	SMS_SENDMIX_INTERVAL	5
#define	SMS_SENDDEF_INTERVAL	10
#define SMS_SENDMAX_INTERVAL	120
#define SMSCTLLOG if(_pLog != NULL) (*_pLog)

SmsControl::SmsControl(void)
:_pLog(NULL),_bQuit(false), _bMsgUnicode(true),_nSendInterval(10)
{
#ifdef ZQ_OS_MSWIN
	_hCom = NULL;
	_hStop = NULL;
	_hMsg = NULL;
#else
	_hCom = 0;
#endif
	memset(_chSmsc,0,sizeof(_chSmsc));
	_nSmLT = SMS_MSG_LIVETIME;
	memset(_chPort,0,sizeof(_chPort));
}

SmsControl::~SmsControl(void)
{
	close();
}

void SmsControl::SetPort(const char* pPort)
{
	if(pPort != NULL && strlen(pPort) > 3)
	{
		memset(_chPort,0,sizeof(_chPort));
		strcpy(_chPort,pPort);
	}
	else
		SMSCTLLOG(Log::L_ERROR,"SmsControl::SetPort() the serial port [%s] is invalid",pPort);

	SMSCTLLOG(Log::L_DEBUG,"SmsControl::SetPort() set the serial port '%s'",pPort);
}

void SmsControl::SetCenterNum(const char* pCenterNum)
{
	if(pCenterNum ==NULL || strlen(pCenterNum)<11)
	{
		SMSCTLLOG(Log::L_WARNING,"SmsControl::SetCenterNum() short message center number is invalidation");
		return;
	}
	
	if(*pCenterNum == '+')
		pCenterNum++;
	if(*pCenterNum == '8' && *(pCenterNum+1) == '6')
	{
		memset(_chSmsc,0,sizeof(_chSmsc));
		strcpy(_chSmsc,pCenterNum);
	}
	else
	{
		memset(_chSmsc,0,sizeof(_chSmsc));
		sprintf(_chSmsc,"86%s",pCenterNum);
	}
	
	SMSCTLLOG(Log::L_DEBUG,"SmsControl::SetCenterNum() set short message center number '%s'",pCenterNum);
}

void SmsControl::SetMsgLiveTime(int nLT)
{
	if(nLT <= 5)
		_nSmLT = 5;
	else if(nLT >= 24*60)
		_nSmLT = 24*60;
	else
		_nSmLT = nLT;

	SMSCTLLOG(Log::L_DEBUG,"SmsControl::SetMsgLiveTime() set message life time of live '%d'",_nSmLT);
}

void SmsControl::SetMsgUnicodeType(bool bUnicode)
{
	_bMsgUnicode = bUnicode;
	if(_bMsgUnicode)
		SMSCTLLOG(Log::L_DEBUG,"SmsControl::SetMsgUnicodeType() set message type is unicode");
	else
		SMSCTLLOG(Log::L_DEBUG,"SmsControl::SetMsgUnicodeType() set message type is single character");;
}

void SmsControl::SetSendInterval(int nInterval )
{
	if(nInterval <= SMS_SENDMIX_INTERVAL)
		_nSendInterval = SMS_SENDDEF_INTERVAL;
	else if(nInterval >= SMS_SENDMAX_INTERVAL)
		_nSendInterval = SMS_SENDMAX_INTERVAL;
	else
		_nSendInterval = nInterval;

	SMSCTLLOG(Log::L_DEBUG,"SmsControl::SetSendInterval() set send message interval '%d'",_nSendInterval);
}

bool SmsControl::init(void)
{

	if(strlen(_chPort) < 3)//the port is NULL or invalidation
		strcpy(_chPort,"COM1");

	//open serial port
	if(!_hCom)
	{
		bool bR = OpenCom(_chPort);
		if(!bR)
		{
			SMSCTLLOG(Log::L_ERROR,"SmsControl::init() open serial port '%s' failed",_chPort);
			return false;
		}
	}
	//init serial port
	if(!InitCom())
	{
		SMSCTLLOG(Log::L_ERROR,"SmsControl::init() init serial port '%s' failed",_chPort);
		return false;
	}

#ifdef ZQ_OS_MSWIN
	if(_hStop == NULL)
		_hStop = CreateEvent(NULL,false,false,NULL);
	if(_hMsg == NULL)
		_hMsg = CreateSemaphore(NULL,0,2000,NULL);
#else
	sem_init(&_hMsg, 0, 0);
#endif

	SMSCTLLOG(Log::L_INFO,"SmsControl::init() successful");
	return true;
}

void SmsControl::close(void)
{
	_bQuit = true;

#ifdef ZQ_OS_MSWIN
	if(_hStop)
	{
		SetEvent(_hStop);
		CloseHandle(_hStop);
		_hStop = NULL;
	}
	if(_hMsg)
	{
		CloseHandle(_hMsg);
		_hMsg = NULL;
	}
#else
	try
	{
		sem_destroy(&_hMsg);
	}
	catch(...){}
#endif

	CloseCom();
}

int SmsControl::run(void)
{
//	SM_BUFF buf = {0};
	SM_PARAM parBuf[256];
	memset(parBuf, 0, sizeof(parBuf));

	int64 dwSendL = 0;
	int64 dwSendN = 0;

#ifdef ZQ_OS_MSWIN
	HANDLE hHandles[2];
	hHandles[0] = _hStop;
	hHandles[1] = _hMsg;
	DWORD dwRet = 0;

	while(!_bQuit)
	{
		dwRet = WaitForMultipleObjects(2,hHandles,false,INFINITE);
		if (dwRet == WAIT_OBJECT_0)          //stop
		{
			SMSCTLLOG(Log::L_DEBUG,"SmsControl::run() wait a stop event");
			break;
		}
		else if (dwRet == WAIT_OBJECT_0 + 1) //have message
		{
			;
		}
		else                                  //error
		{
			break;
		}
#else
	int rt = 0;
	while(!_bQuit)
	{
		rt = sem_wait(&_hMsg);
		if(rt == 0)
		{
			;//have a message
		}
		else if(errno == EINTR)
			continue;
		else
		{
			if(_bQuit)
				SMSCTLLOG(Log::L_DEBUG,"SmsControl::run() will stop");
			break;
		}
			
#endif

		while(_msgQue.size())
		{
			if(_bQuit)
				break;

			SMSINFO msgInfo;
			{
				ZQ::common::MutexGuard mg(_lock);
				msgInfo = _msgQue.front();
				_msgQue.pop_front();				
			}

			if(!msgInfo.strTel.length() || !msgInfo.strMsg.length())
				continue;
		
			//send long message
			int nMsgC = 0;
			int nOff = 0;
			int nIndex = 0;
			if(_bMsgUnicode)
			{
				nOff = 70;
				nMsgC = (int)msgInfo.strMsg.length()/71 + 1;
			}
			else
			{
				nOff = 160;
				nMsgC = (int)msgInfo.strMsg.length()/161 + 1;
			}
			
			do{
				SM_PARAM SmParam;
				memset(&SmParam, 0, sizeof(SM_PARAM));
				// 填充短消息结构
				std::string strTelN = "86";
				strTelN += msgInfo.strTel;

				strcpy(SmParam.SCA, _chSmsc);
				strcpy(SmParam.TPA, strTelN.c_str());
				strcpy(SmParam.TP_UD, msgInfo.strMsg.substr(nIndex*nOff,nOff).c_str());
				SmParam.TP_PID = 0;
				if(_bMsgUnicode)
					SmParam.TP_DCS = GSM_UCS2;
				else
					SmParam.TP_DCS = GSM_7BIT;
				
				dwSendN = SYS::getTickCount();
				if((dwSendN > dwSendL) && (dwSendN - dwSendL) < _nSendInterval*1000)
					SYS::sleep(_nSendInterval*1000 + dwSendL - dwSendN);
				else if(dwSendN <= dwSendL )
					SYS::sleep(_nSendInterval*1000);
				else
					;
				dwSendL = dwSendN;

				int nS = SendMessage(&SmParam);
				if(nS <= 0)//not successful
				{
					SMSCTLLOG(Log::L_ERROR,"SmsControl::run() send message '%s' to '%s'failed",msgInfo.strMsg.c_str(),msgInfo.strTel.c_str());
				}
				else
					SMSCTLLOG(Log::L_DEBUG,"SmsControl::run() send message '%s' to '%s' successful",msgInfo.strMsg.c_str(),msgInfo.strTel.c_str());

				nIndex++;

			}while(--nMsgC);

		}

	}
	
	SMSCTLLOG(Log::L_DEBUG,"SmsControl::run() this thread quit");

	return 0;
}

void SmsControl::SetLog(ZQ::common::Log* pLog)
{
	_pLog = pLog;
}

void SmsControl::AddMsg(const char* pTel, const char* pMsg)
{
	if(pTel == NULL || strlen(pTel)!=11 || pMsg == NULL || strlen(pMsg)<1)
	{
		SMSCTLLOG(Log::L_ERROR,"SmsControl::AddMsg() the telephone number or message is invalidation");
		return;
	}
	{
		SMSINFO msgInfo;

		msgInfo.strTel = pTel;
		msgInfo.strMsg = pMsg;
		MutexGuard MG(_lock);
		_msgQue.push_back(msgInfo);
	}
#ifdef ZQ_OS_MSWIN
	LONG nPre;
	ReleaseSemaphore(_hMsg, 1, &nPre);
#else
	sem_post(&_hMsg);
#endif
}

bool SmsControl::InitCom()
{
	char ans[128] = {0};
	if( !_hCom )
	{
		bool br = OpenCom(_chPort);
		if(br == false)
		{
			SMSCTLLOG(Log::L_ERROR,"SmsControl::InitCom() can not open com");
			return false;
		}
	}
#ifdef ZQ_OS_MSWIN
	PurgeComm(_hCom,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
#else

#endif

	WriteCom((void*)"AT\r", 3);
	ReadCom(ans, 128);
	if (strstr(ans, "OK") == NULL)  
	{
		SMSCTLLOG(Log::L_ERROR,"SmsControl::InitCom() send AT command  not ok answen '%s'",ans);
		return false;
	}

	// ECHO OFF
	memset(ans,0,sizeof(ans));
	WriteCom((void*)"ATE0\r", 5);
	ReadCom(ans, 128);
	if(strstr(ans,"OK") == NULL)
	{
		SMSCTLLOG(Log::L_ERROR,"SmsControl::InitCom() send ATE0 command not answen");
		return false;
	}

	// PDU模式
	memset(ans,0,sizeof(ans));
	WriteCom((void*)"AT+CMGF=0\r", 10);
	ReadCom(ans, 128);
	if(strstr(ans,"OK") == NULL)
	{
		SMSCTLLOG(Log::L_ERROR,"SmsControl::InitCom() send AT+CMGF command not answen");
		return false;
	}

	return true;
}

#ifdef ZQ_OS_MSWIN
bool SmsControl::OpenCom(const char* pPort, BAUDRATE bRate, PARITY par, BYTESIZE bSize, STOPBIT sBit)
{
	DCB dcb;		// 串口控制块
	if(_hCom != NULL)
	{
		CloseHandle(_hCom);
		_hCom = NULL;
	}

	_hCom = CreateFile(pPort,
			GENERIC_READ | GENERIC_WRITE,
			0,				
			NULL,		
			OPEN_EXISTING,	
			0,				
			NULL);
	
	if(_hCom == INVALID_HANDLE_VALUE) 
		return false;

	if( !GetCommState(_hCom, &dcb) )// 取DCB
		return false;
	
	int nBaudRate = CBR_9600;
	switch(bRate)//baudrate
	{
	case BR4800:
		nBaudRate = CBR_4800;
		break;
	case BR9600:
		nBaudRate = CBR_9600;
		break;
	case BR19200:
		nBaudRate = CBR_19200;
		break;
	default:
		break;
	}
	
	int nParity = NOPARITY;
	switch(par)
	{
	case PARNO:
		nParity = NOPARITY;
		break;
	case PAREVEN:
		nParity = EVENPARITY;
		break;
	case PARODD:
		nParity = ODDPARITY;
		break;
	default:
		break;
	}
	
	int nByteSize = 8;
	switch(bSize)
	{
	case BS7:
		nByteSize = 7;
		break;
	case BS8:
		nByteSize = 8;
		break;
	default:
		break;
	}

	int nStopBits = ONESTOPBIT;
	switch(sBit)
	{
	case SB1:
		nStopBits = ONESTOPBIT;
		break;
	case SB2:
		nStopBits = TWOSTOPBITS;
		break;
	default:
		break;
	}	

	dcb.BaudRate = nBaudRate;
	dcb.ByteSize = nByteSize;
	dcb.Parity = nParity;
	dcb.StopBits = nStopBits;

	if(SetCommState(_hCom, &dcb) == FALSE)// 设置DCB
	{
		CloseHandle(_hCom);
		_hCom = NULL;
		return false;
	}

	if(SetupComm(_hCom, 4096, 1024) == FALSE)// 设置输入输出缓冲区大小
	{
		CloseHandle(_hCom);
		_hCom = NULL;
		return false;
	}
	COMMTIMEOUTS timeouts;

	timeouts.ReadIntervalTimeout = 300;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 300;
	timeouts.WriteTotalTimeoutMultiplier = 1;

	if(SetCommTimeouts(_hCom, &timeouts) == FALSE)	// 设置超时
	{
		CloseHandle(_hCom);
		_hCom = NULL;
		return false;
	}

	return true;
}

void SmsControl::CloseCom()
{
	if(_hCom)
	{
		SMSCTLLOG(Log::L_DEBUG,"SmsControl::CloseCom() close com");
		WriteCom((void*) "ATH\r", 4);
		CloseHandle(_hCom);
		_hCom = NULL;
	}
}


int SmsControl::WriteCom(void* pData, int nLength)
{
	int nTryC = 3;
	int nHS = 0;
	DWORD dwNumWrite = 0;
	do
	{
		dwNumWrite = 0;
		WriteFile(_hCom, (char*)pData+nHS, (DWORD)nLength, &dwNumWrite, NULL);
		nHS += dwNumWrite;
		if(dwNumWrite <= 0)
		{
			nTryC --;
			Sleep(10);
		}
	}while(nHS < nLength && nTryC);

	std::string str((char*)pData,nLength);
	str[nLength] ='\0';
	if(nTryC == 0)
	{
		SMSCTLLOG(Log::L_ERROR,"SmsControl::WriteCom() write content '%s' failed",str.c_str());
		return 0;
	}

	
	SMSCTLLOG(Log::L_DEBUG, "wrote: %s", str.c_str());

	return nLength;
}

int SmsControl::ReadCom(void* pData, int nLength)
{
	DWORD dwNumRead = 0;

	ReadFile(_hCom, pData, (DWORD)nLength, &dwNumRead, NULL);
	char str[256];
	memcpy(str, pData, dwNumRead); str[dwNumRead] ='\0';
	SMSCTLLOG(Log::L_DEBUG, "read: %s", str);
	
	return (int)dwNumRead;
}

#else
bool SmsControl::OpenCom(const char* pPort, BAUDRATE bRate, PARITY par, BYTESIZE bSize, STOPBIT sBit)
{
	if(pPort == NULL || strlen(pPort) <4)
		return false;

	if(!_hCom)
	{
		::close(_hCom);
		_hCom = 0;
	}	
	
	int ndigit =  atoi(pPort + 3) -1;
	char chindex[10] = {0};
	sprintf(chindex,"%d",ndigit);
	std::string strname = "/dev/ttyS";
	strname += chindex;

	_hCom = open(strname.c_str(), O_RDWR);
	if(_hCom == -1)
	{
		SMSCTLLOG(Log::L_ERROR,"open serial port [%s] failed error code[%d] string[%s]",
			strname.c_str(),errno, strerror(errno));
		return false; 	
	}

	struct termios opts; 
	if( tcgetattr( _hCom,&opts) != 0) 
	{ 
		::close(_hCom);
		_hCom = 0;
		return false;  
	}
	tcflush(_hCom, TCIOFLUSH);

	int nbaudrate = B9600;
	switch(bRate)
	{
	case BR4800:
		nbaudrate = B4800;
		break;
	case BR9600:
		nbaudrate = B9600;
		break;
	case BR19200:
		nbaudrate = B19200;
		break;
	default:
		break;
	}
	cfsetispeed(&opts, nbaudrate);
	cfsetospeed(&opts, nbaudrate);

	switch(par)
	{
	case PARNO:
		opts.c_cflag &= ~PARENB;
		opts.c_iflag &= ~INPCK;
		break;
	case PAREVEN:
		opts.c_cflag |= PARENB;
		opts.c_cflag &= ~PARODD;
		opts.c_iflag |= INPCK;
		break;
	case PARODD:
		opts.c_cflag |= ( PARODD | PARENB );
		opts.c_iflag |= INPCK;
		break;
	default:
		break;
	}
	
	switch(bSize)
	{
	case BS7:
		opts.c_cflag &= ~CSIZE;
		opts.c_cflag |= CS7;
		break;
	case BS8:
		opts.c_cflag &= ~CSIZE;
		opts.c_cflag |= CS8;
		break;
	default:
		break;
	}

	switch(sBit)
	{
	case SB1:
		opts.c_cflag &= ~CSTOPB;
		break;
	case SB2:
		opts.c_cflag |= CSTOPB;
		break;
	default:
		break;
	}	

	opts.c_cc[VTIME] = 150; /* time out 15 seconds*/   
	opts.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	if (tcsetattr(_hCom,TCSANOW,&opts) != 0)   
	{ 
		::close(_hCom);
		_hCom = 0;
		return false;  
	} 

	return true;
}

void SmsControl::CloseCom()
{
	if(_hCom)
	{
		SMSCTLLOG(Log::L_DEBUG,"SmsControl::CloseCom() close com");
		WriteCom((void*) "ATH\r", 4);
		::close(_hCom);
		_hCom = 0;
	}
}


int SmsControl::WriteCom(void* pData, int nLength)
{
	int nTryC = 3;
	int nHS = 0;
	int writeNum = 0;
	do
	{
		writeNum = 0;
		writeNum = write(_hCom, (void*)((char*)pData+nHS), nLength-nHS);
		nHS += writeNum;
		if(writeNum <= 0)
		{
			nTryC --;
			usleep(10000);
		}
	}while(nHS < nLength && nTryC);

	std::string str((char*)pData,nLength);
	if(nTryC == 0)
	{
		SMSCTLLOG(Log::L_ERROR,"SmsControl::WriteCom() write content '%s' failed, error code[%d] string[%s]",
			str.c_str(),errno,strerror(errno));
		return 0;
	}
	
	SMSCTLLOG(Log::L_DEBUG, "wrote: [%s]", str.c_str());

	return nLength;
}

int SmsControl::ReadCom(void* pData, int nLength)
{
	int readNum = 0;

	readNum = read(_hCom, pData, nLength);
	if(readNum <= 0)
	{
		return 0;
	}
	char str[256];
	memcpy(str, pData, readNum); 
	str[readNum] ='\0';
	SMSCTLLOG(Log::L_DEBUG, "read: [%s]", str);
	
	return readNum;
}
#endif

int SmsControl::SendMessage(nsSMS::SM_PARAM* pSrc)
{
	int nPduLength = 0;
	int nSmscL = 0;
	unsigned char chSmscLength[2] = {0};
	int nLength = 0;	
	char cmd[16] = {0};		
	char pdu[1024] = {0};
	char ans[128] = {0};		

	nPduLength = _smsCom.EncodePdu(pSrc, _nSmLT, pdu);	// 根据PDU参数，编码PDU串
	strcat(pdu, "\x01a");		// 以Ctrl-Z结束

	_smsCom.String2Bytes(pdu, 2, chSmscLength);
	nSmscL = chSmscLength[0] + 1;

	// 命令中的长度，不包括SMSC信息长度，以数据字节计
	sprintf(cmd, "AT+CMGS=%d\r", nPduLength / 2 - nSmscL);

	WriteCom(cmd, (int)strlen(cmd));

	nLength = ReadCom(ans, 128);

	// 根据能否找到"\r\n> "决定成功与否
	if(nLength == 4 && strncmp(ans, "\r\n> ", 4) == 0)
	{
		int ns = WriteCom(pdu, (int)strlen(pdu));
		if(ns > 0)//get response
		{
			memset(ans,0,sizeof(ans));
			nLength = ReadCom(ans, 128);
			if(nLength && std::string(ans).find("OK",0) != std::string::npos)//OK
			{
				;
//				SMSCTLLOG(Log::L_DEBUG,"SendMessage() send message '%s' OK",pSrc->TP_UD);
			}
			else
				return 0;
		}
		return ns;
	}

	return 0;
}

int SmsControl::ReadMessageList()
{
	return WriteCom((void*)"AT+CMGL\r", 8);
}

int SmsControl::DeleteMessage(int index)
{
	char cmd[16];
	sprintf(cmd, "AT+CMGD=%d\r", index);

	return WriteCom(cmd, (int)strlen(cmd));
}

int SmsControl::GetResponse(nsSMS::SM_BUFF* pBuff)
{
	int nLength;
	int nState;

	// 从串口读数据，追加到缓冲区尾部
	nLength = ReadCom(&pBuff->data[pBuff->len], 128);	
	pBuff->len += nLength;

	// 确定GSM MODEM的应答状态
	nState = GSM_WAIT;
	if ((nLength > 0) && (pBuff->len >= 4))
	{
		if (strncmp(&pBuff->data[pBuff->len - 4], "OK\r\n", 4) == 0)  
			nState = GSM_OK;
		else if (strstr(pBuff->data, "+CMS ERROR") != NULL) 
			nState = GSM_ERR;
	}

	return nState;
}

int SmsControl::ParseMessageList(nsSMS::SM_PARAM* pMsg, nsSMS::SM_BUFF* pBuff)
{
	int nMsg = 0;	
	char* ptr = pBuff->data;


	// 循环读取每一条短消息, 以"+CMGL:"开头
	while((ptr = strstr(ptr, "+CMGL:")) != NULL)
	{
		ptr += 6;		// 跳过"+CMGL:", 定位到序号
		sscanf(ptr, "%d", &pMsg->index);	// 读取序号

		ptr = strstr(ptr, "\r\n");	// 找下一行
		if (ptr != NULL)
		{
			ptr += 2;			
			_smsCom.DecodePdu(ptr, pMsg);	// PDU串解码

			pMsg++;
			nMsg++;
		}
	}

	return nMsg;
}


int SmsControl::SendMessage(const char* pNum, const char* chContent)
{
	if(pNum == NULL || strlen(pNum) !=11 || chContent == NULL)
	{
		SMSCTLLOG(Log::L_ERROR,"SmsControl::SendMessage() the telephone number or message is invalidation");
		return 0;
	}

	SM_PARAM SmParam;
	memset(&SmParam, 0, sizeof(SM_PARAM));
	
	std::string strContent = chContent;
	char chDN[20] = {0};
	sprintf(chDN,"86%s",pNum);


	// 填充短消息结构
	strcpy(SmParam.SCA, _chSmsc);
	strcpy(SmParam.TPA, chDN);
	strcpy(SmParam.TP_UD, strContent.c_str());
	SmParam.TP_PID = 0;
	if(_bMsgUnicode)
		SmParam.TP_DCS = GSM_UCS2;
	else
		SmParam.TP_DCS = GSM_7BIT;
	
	int nS = SendMessage(&SmParam);
	SMSCTLLOG(Log::L_DEBUG,"SendMessage() send message number '%d'",nS);
	return nS;
}

