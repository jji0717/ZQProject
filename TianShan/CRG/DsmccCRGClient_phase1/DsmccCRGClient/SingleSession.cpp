#include "SingleSession.h"
#include "DsmccMsg.h"
#include "DsmccClientCfg.h"
#include "lsc_parser.h"
#include "lsc_protocol.h"
#include "InetAddr.h"
#include "TimeUtil.h"
#include <FileLog.h>
#include "DsmccSocket.h"
#include "DsmccThread.h"
//#include <ctime>
#ifdef ZQ_OS_MSWIN
	#include <Windows.h>
#else
	#include <unistd.h>
#endif

using namespace ZQ::DSMCC;
extern ZQ::DsmccCRGClient::DsmccCRGClientConfig _gConfig;
extern ZQ::common::FileLog  *_gLog;

SingleSession::SingleSession(void)
			:m_dsmccClient(NULL)
			,m_pthread(NULL)
			,m_lscpClient(NULL)
			,m_nStreamHandle(0)
			,m_bInit(false)
			,m_bffbegin(false)
			,_stepNumber(0)
			,_responseCode(parse_error)
			,_currentStatus("success")
			,_errResponseCode(session_ok)
{
}

SingleSession::~SingleSession(void)
{
}

bool SingleSession::Init()
{

	ZQ::common::tpport_t port;
 	
//	m_tcpClient = new ZQ::common::TCPClient();
//	m_udpClient = new ZQ::common::UDPSocket();
	int i = 0;
	while(i < _gConfig.dsmccserver.size())
	{
		port = atoi(_gConfig.dsmccserver[i].port.c_str());
		if (0 ==_gConfig.dsmccserver[i].protocol.compare("dsmcc"))
		{
			if (0 == _gConfig.dsmccserver[i].type.compare("tcp"))
			{
				m_dsmccClient = new DsmccClientSocket(true);
			}
			else if (0 == _gConfig.dsmccserver[i].type.compare("udp"))
			{
				m_dsmccClient = new DsmccClientSocket(false);
			}
			if (!m_dsmccClient->Init(_gConfig.dsmccserver[i].ip,_gConfig.dsmccserver[i].port))
			{
				_errorRequestName = "connect DsmccServer " + _gConfig.dsmccserver[i].ip + ":" + _gConfig.dsmccserver[i].port;
				_errResponseCode = connect_error;
				_currentStatus = "failed";
				_errStepNumber = -1;
				return false;
			}
			(*_gLog)(ZQ::common::Log::L_INFO,CLOGFMT(SingleSession,"dcmcc client Init sucess,ip = %s,port = %s"),_gConfig.dsmccserver[i].ip.c_str(),_gConfig.dsmccserver[i].port.c_str());

		}
		else if (0 == _gConfig.dsmccserver[i].protocol.compare("lscp"))
		{
			if (0 == _gConfig.dsmccserver[i].type.compare("tcp"))
			{
				m_lscpClient = new DsmccClientSocket(true);
			}
			else if (0 == _gConfig.dsmccserver[i].type.compare("udp"))
			{
				m_lscpClient = new DsmccClientSocket(false);
			}

			if (!m_lscpClient->Init(_gConfig.dsmccserver[i].ip,_gConfig.dsmccserver[i].port))
			{
				_errorRequestName = "connect LscpServer " + _gConfig.dsmccserver[i].ip + ":" + _gConfig.dsmccserver[i].port;
				_errResponseCode = connect_error;
				_currentStatus = "failed";
				_errStepNumber = -1;
				return false;
			}
			(*_gLog)(ZQ::common::Log::L_INFO,CLOGFMT(SingleSession,"lscp client Init sucess,ip = %s,port = %s"),_gConfig.dsmccserver[i].ip.c_str(),_gConfig.dsmccserver[i].port.c_str());
		}
		i++;
	}
 	
	m_pthread = new ClientThread(m_dsmccClient);

	m_bInit = true;
	return true;
}

bool SingleSession::UnInit()
{
 
	if (m_pthread != NULL)
	{
		delete m_pthread;
		m_pthread = NULL;
	}

	if (m_dsmccClient != NULL)
	{
		m_dsmccClient->UnInit();
		delete m_dsmccClient;
		m_dsmccClient = NULL;
	}

	if (m_lscpClient != NULL)
	{
		delete m_lscpClient;
		m_lscpClient = NULL;
	}

	m_bInit = false;
	return true;
}

bool SingleSession::Start()
{
	if (!m_bInit)
	{
		return false;
	}
	bool bRet = true;

	if (_gConfig.request[0].operation.compare("SETUP") != 0)
	{
		_errorRequestName = " ";
		_errResponseCode = step_error;
		_errStepNumber = -1;
		(*_gLog)(ZQ::common::Log::L_ERROR,CLOGFMT(SingleSession,"The first request must be [SETUP]."));
		return false;
	}

	int i = 0;
	while(i < _gConfig.request.size())
	{
		_stepNumber = i;
		if (!bRet && (1 == _gConfig.request[i].skip) && _gConfig.request[i].operation.compare("SETUP") != 0)
		{
			printf("step[%-2d] command[%-8s] status[skipped] \n", i+1, _gConfig.request[i].operation.c_str());
			i++;
			continue;
		}

		bool bCurrent = true;
		int  currentResponseCode = 0;

		if (0 == _gConfig.request[i].operation.compare("SETUP"))
		{
			if (!Setup())
			{
				_currentStatus = "failed";
				currentResponseCode = _responseCode;
				bCurrent = false;
				_errorRequestName = _gConfig.request[i].operation;
				_errResponseCode = _responseCode;
				_errStepNumber = i + 1;
				return false;
			}			
		}
		else if (0 == _gConfig.request[i].operation.compare("PLAY"))
		{
			if (!Play())
			{
				currentResponseCode = play_error;
				bCurrent = false;
			}
		}
		else if (0 == _gConfig.request[i].operation.compare("PAUSE"))
		{
			if (!Pause())
			{
				bCurrent = false;
				currentResponseCode = pause_error;
			}
		}
		else if (0 == _gConfig.request[i].operation.compare("STATUS"))
		{
			if (!Status())
			{
				bCurrent = false;
				currentResponseCode = status_error;
			}
		}
		else if (0 == _gConfig.request[i].operation.compare("RESUME"))
		{
			if (!Resume())
			{
				bCurrent = false;
				currentResponseCode = resume_error;
			}
		}
		else if (0 == _gConfig.request[i].operation.compare("RELEASE"))
		{
			if (!Release())
			{
				bCurrent = false;
				currentResponseCode = _responseCode;
			}
			if(1 == _gConfig.request[i].skip)
			{
				bCurrent = true;
				currentResponseCode = _responseCode = 0;
			}
		}
		#ifdef ZQ_OS_MSWIN
			Sleep(_gConfig.request[i].waitTime);
		#else
			usleep(_gConfig.request[i].waitTime * 1000);
		#endif
		
		_errorRequestName = _gConfig.request[i].operation;
		if(bCurrent == false)
		{
			_currentStatus = "failed";
			bRet = false;
			_errResponseCode = currentResponseCode;
		}
		_errStepNumber = i + 1;
		printf("step[%-2d] request[%-8s] status[%-8s] responseCode[%-5d]\n", i+1, _gConfig.request[i].operation.c_str(),_currentStatus.c_str(),currentResponseCode);
		(*_gLog)(ZQ::common::Log::L_INFO,CLOGFMT(SingleSession,"step[%d] command[%s] status[%s] responseCode[%d]"),i+1,_gConfig.request[i].operation.c_str(),_currentStatus.c_str(),currentResponseCode);
		i++;
	}
	
	return bRet;
}

std::string SingleSession::GetSessionId()
{
	if (m_strSessionId.size() == 0)
	{
// 		if (m_bffbegin)
// 		{
// 			CreateSessionId();
// 		}
// 		else
		CreateRandom(20);
//			CreateSessionId();
	}
	return m_strSessionId;
}
void SingleSession::CreateSessionId()
{
/*	char  _decimalserialn[11];
	memset(_decimalserialn,0x00,sizeof(_decimalserialn));
	unsigned long handlepid=GetCurrentProcessId(); 
	unsigned long handletid = GetCurrentThreadId();
	time_t lt;
	lt = ZQ::common::TimeUtil::now();

	uint8 uff=0xff;
	BYTE* pbyte=(BYTE*)_decimalserialn;
	if(m_bffbegin)
	{
		memcpy(pbyte,&uff,1);//0xFF  创建session序列 第一个字节是否为FF标志
		pbyte+=1;
		memcpy(pbyte,&handlepid,1);
		pbyte+=1;
	}
	else
	{
		memcpy(pbyte,&handlepid,2);
		pbyte+=2;
	}
	memcpy(pbyte,&handletid,2);
	pbyte+=2;
	memcpy(pbyte,&lt,4);
	srand((unsigned int)(ZQ::common::TimeUtil::now()));
	int loc_rand=rand();//random number
	pbyte+=4;
	memcpy(pbyte,&loc_rand,2);
	HexToString(m_strSessionId,(BYTE*)_decimalserialn,10);
*/
}

bool SingleSession::HexToString(std::string& outputString ,const unsigned char* buf, unsigned short int len)
{
	if(NULL == buf || len < 1)
		return false;
	char temp[4096] = "";
	for(unsigned short int i = 0 ; i < len; i++)
	{
		//	itoa(*(buf + i), temp + i * 2, 16);
		sprintf(temp + i * 2, "%02x",*(buf + i));
	}	
	outputString = (temp);
	return true;
}

std::string SingleSession::CreateRandom(int strLen)
{

	char* pstr="1234567890abcdef";

	int len = strlen(pstr);

	srand(ZQ::common::TimeUtil::now()+ rand());  
//	srand(time(NULL));
	for(int i = 0; i  < strLen; i++) 
	{ 
		m_strSessionId.push_back(pstr[rand()%len]);
	}
	if (m_bffbegin)
	{
		m_strSessionId[0] = 'f';
		m_strSessionId[1] = 'f';
	}
	else
	{
		while(m_strSessionId[0] == 'f' && m_strSessionId[1] == 'f')
		{
			m_strSessionId[0] = pstr[rand()%len];
			m_strSessionId[1] = pstr[rand()%len];
		}
	}

	return m_strSessionId;
}

bool SingleSession::Setup()
{
//	CreateSessionId();
	uint8 strBuf[1024];
	memset(strBuf, '0', 1024);
	ZQ::DSMCC::DsmccMsg::HardHeader tempheader;
	ZQ::DSMCC::StringMap tempmeta;


	tempheader.protocolDiscriminator = (uint8)atoi(_gConfig.commonheader.protocolDiscriminator.c_str());
	tempheader.dsmccType = (uint8)atoi(_gConfig.commonheader.dsmccType.c_str());
	tempheader.messageId = (uint16)atoi(_gConfig.request[_stepNumber].messageId.c_str());		//setup
	tempheader.reserved1 = (uint8)atoi(_gConfig.commonheader.reserved.c_str());
	tempheader.transactionId = (uint32)atoi(_gConfig.commonheader.transactionId.c_str());

 	tempmeta[CRMetaData_SessionId] =  GetSessionId();
	std::cout <<"sessionId:"<< m_strSessionId << std::endl;

	tempmeta[CRMetaData_CSSRclientId] = _gConfig.request[_stepNumber].header["clientId"];
	tempmeta[CRMetaData_CSSRserverId] = _gConfig.request[_stepNumber].header["serverId"];
	tempmeta[CRMetaData_CSSRreserved]= _gConfig.request[_stepNumber].header["reserved"];


	tempmeta[CRMetaData_assetId] = _gConfig.request[_stepNumber].appdata["AssetId"];
	tempmeta[CRMetaData_nodeGroupId]= _gConfig.request[_stepNumber].appdata["nodeGroupId"];
	tempmeta[CRMetaData_billingId] = _gConfig.request[_stepNumber].appdata["billingId"];
	tempmeta[CRMetaData_purchaseTime] = _gConfig.request[_stepNumber].appdata["purchaseTime"];
	tempmeta[CRMetaData_remainingPlayTime] = _gConfig.request[_stepNumber].appdata["remainingPlayTime"];
	tempmeta[CRMetaData_errorCode] = _gConfig.request[_stepNumber].appdata["errorCode"];

	tempmeta[CRMetaData_homeId] = _gConfig.request[_stepNumber].appdata["homeId"];
	tempmeta[CRMetaData_purchaseId] = _gConfig.request[_stepNumber].appdata["purchaseId"];

	tempmeta[CRMetaData_smartCardId] = "2";
	tempmeta[CRMetaData_analogCopyPurchase] = _gConfig.request[_stepNumber].appdata["analogCopyPurchase"];
	tempmeta[CRMetaData_packageId] = _gConfig.request[_stepNumber].appdata["packageId"];

	tempmeta[CRMetaData_protocolDiscriminator] = _gConfig.commonheader.protocolDiscriminator;
	tempmeta[CRMetaData_dsmccType] = _gConfig.commonheader.dsmccType;
 	tempmeta[CRMetaData_messageId] = _gConfig.request[_stepNumber].messageId;
	tempmeta[CRMetaData_reserved1] = _gConfig.commonheader.reserved;
	tempmeta[CRMetaData_transactionId] = _gConfig.commonheader.transactionId;


	ZQ::DSMCC::DsmccMsg::Ptr setupReqPtr = new ZQ::DSMCC::ClientSessionSetupRequest(tempheader);

	setupReqPtr->readMetaData(tempmeta);

	int msglen = setupReqPtr->toMessage(strBuf,1024);

	(*_gLog)(ZQ::common::Log::L_INFO,CLOGFMT(SingleSession,"Setup Request SessionId = %s"),m_strSessionId.c_str());
	m_pthread->start();

 	if (!m_dsmccClient->SendBinary((char*)strBuf,msglen))
 	{
		_responseCode = send_error;
 		return false;
 	}
	
// 	if (!m_TcpClient->SendDataSync((char*)strBuf,msglen))
// 	{
// 		return false;
// 	}
	
	int i = 0;
	int count = _gConfig.messageTimeout/1000;
	if(count < 5)
		count = 5;
	while (i< count)
	{
		if (m_dsmccClient->getSetupStatus())
		{
			m_nStreamHandle = m_dsmccClient->getStreamHandle();
			return true;
		}
		else
		{
			_responseCode = m_dsmccClient->getResponseCode();
			if (_responseCode != parse_error && _responseCode != RsnOK)
			{
				return false;
			}
		}

		#ifdef ZQ_OS_MSWIN
				Sleep(1000);
		#else
				sleep(1);
		#endif
		i++;
	}
	_responseCode = setup_error;
	(*_gLog)(ZQ::common::Log::L_INFO,CLOGFMT(SingleSession,"Setup Request time out. time = %ds"),count);
	return false;
}


bool SingleSession::TestSetupMessage(uint8* buf, size_t maxLen)
{
	size_t processedLength = 0;

	ZQ::DSMCC::DsmccMsg::Ptr pMsg = ZQ::DSMCC::DsmccMsg::parseMessage(buf, maxLen, processedLength);

	if(pMsg)
	{
		ZQ::DSMCC::StringMap metadatas;
		uint16 size = pMsg->toMetaData(metadatas);

		uint8 toBuf[1024] ="";
		int length =  pMsg->toMessage(toBuf, 1024);

		pMsg = ZQ::DSMCC::DsmccMsg::parseMessage(toBuf, length, processedLength);

		uint8 totoBuf[1024] ="";
		length =  pMsg->toMessage(totoBuf, 1024);

		for(int i = 0 ; i < length; i++)
		{
			if(totoBuf[i] != toBuf[i])
			{
				//printf("diff [%d]\n", i) ;
				return false;
			}
		}
	}
	else
		return false;
	return true;
}




bool SingleSession::Play()
{

	lsc::PlayMessage_t  lscPlayMsg;
	lscPlayMsg.header.version = (uint8_t)atoi(_gConfig.request[_stepNumber].header["version"].c_str());
	lscPlayMsg.header.transactionId = (uint8_t)atoi(_gConfig.request[_stepNumber].header["transactionId"].c_str());	
	lscPlayMsg.header.opCode = (uint8_t)atoi(_gConfig.request[_stepNumber].messageId.c_str());
	lscPlayMsg.header.statusCode = (uint8_t)atoi(_gConfig.request[_stepNumber].header["statusCode"].c_str());
	lscPlayMsg.header.streamHandle = (uint32_t)m_nStreamHandle;


	lscPlayMsg.data.startNpt = (uint32_t)atoi(_gConfig.request[_stepNumber].appdata["StartNpt"].c_str());
	lscPlayMsg.data.stopNpt = (uint32_t)atoi(_gConfig.request[_stepNumber].appdata["StopNPT"].c_str());
	lscPlayMsg.data.numerator= (int16_t)atoi(_gConfig.request[_stepNumber].appdata["Numerator"].c_str());
	lscPlayMsg.data.denominator= (uint16_t)atoi(_gConfig.request[_stepNumber].appdata["Denominator"].c_str());
	
	lscPlayMsg.hton();
	uint8 buf[1024];
	int len = sizeof(lscPlayMsg);
	memcpy(buf, &lscPlayMsg, sizeof(lscPlayMsg));
// 	if (!TestPlayMessage(buf,len))
// 	{
// 		return false;
// 	}

	(*_gLog)(ZQ::common::Log::L_DEBUG,CLOGFMT(SingleSession,"Play Request ..."));

	return m_lscpClient->SyncSendData((char*)buf,len);
}

bool SingleSession::TestPlayMessage(uint8* buf, int maxLen)
{

	void *tempbuf = buf;
	lsc::lscMessage * lastMsg = NULL;
	lsc::lscMessage * tempMsg = lsc::ParseMessage(tempbuf,maxLen,lastMsg);
	lsc::StringMap lscmetadata;
	if(tempMsg)
	{
		std::cout <<"------"<< tempMsg->getMsgSize() <<std::endl;
		uint16 size = tempMsg->toMetaData(lscmetadata);
		lsc::OperationCode opCode = tempMsg->GetLscMessageOpCode();

		if(opCode == lsc::LSC_PLAY)
		{
			lsc::LSCMESSAGE msg  = tempMsg->GetLscMessageContent();

			msg.play.ntoh();
			printf("------------------------------------testLscpPlay---------------------------------\n");
			printf("opCode: %d\n",msg.play.header.opCode);
			printf("statusCode: %d\n",msg.play.header.statusCode);
			printf("streamHandle: %ul\n",msg.play.header.streamHandle);
			printf("transactionId: %d\n",msg.play.header.transactionId);
			printf("version: %d\n",msg.play.header.version);

			printf("startNpt: %lu\n",msg.play.data.startNpt);
			printf("stopNpt: %d\n",msg.play.data.stopNpt);
			printf("numerator: %d\n",msg.play.data.numerator);
			printf("denominator: %d\n",msg.play.data.denominator);

		}

	}
	return true;
}

bool SingleSession::Pause()
{

	lsc::PauseMessage_t  lscPauseMsg;

	lscPauseMsg.header.version = (uint8_t)atoi(_gConfig.request[_stepNumber].header["version"].c_str());
	lscPauseMsg.header.transactionId = (uint8_t)atoi(_gConfig.request[_stepNumber].header["transactionId"].c_str());	
	lscPauseMsg.header.opCode = (uint8_t)atoi(_gConfig.request[_stepNumber].messageId.c_str());
	lscPauseMsg.header.statusCode = (uint8_t)atoi(_gConfig.request[_stepNumber].header["statusCode"].c_str());
	lscPauseMsg.header.streamHandle = m_nStreamHandle;

	lscPauseMsg.stopNpt = (uint32_t)atoi(_gConfig.request[2].appdata["StopNPT"].c_str());

	lscPauseMsg.hton();

	uint8 buf[1024];
	int len = sizeof(lscPauseMsg);
	memcpy(buf, &lscPauseMsg, sizeof(lscPauseMsg));

	(*_gLog)(ZQ::common::Log::L_DEBUG,CLOGFMT(SingleSession,"Pause Request ..."));

	return m_lscpClient->SyncSendData((char*)buf,len);
}

bool SingleSession::Status()
{

	lsc::StatusMessage_t  lscStatusMsg;

	lscStatusMsg.header.version = (uint8_t)atoi(_gConfig.request[_stepNumber].header["version"].c_str());
	lscStatusMsg.header.transactionId = (uint8_t)atoi(_gConfig.request[_stepNumber].header["transactionId"].c_str());	
	lscStatusMsg.header.opCode = (uint8_t)atoi(_gConfig.request[_stepNumber].messageId.c_str());
	lscStatusMsg.header.statusCode = (uint8_t)atoi(_gConfig.request[_stepNumber].header["statusCode"].c_str());
	lscStatusMsg.header.streamHandle = m_nStreamHandle;

	lscStatusMsg.header.hton();

	uint8 buf[1024];
	int len = sizeof(lscStatusMsg);
	memcpy(buf, &lscStatusMsg, sizeof(lscStatusMsg));

	(*_gLog)(ZQ::common::Log::L_INFO,CLOGFMT(SingleSession,"Status Request StreamHandle = %u"),m_nStreamHandle);

	return m_lscpClient->SyncSendData((char*)buf,len);
}

bool SingleSession::Resume()
{

	lsc::ResumeMessage_t lscResumeMsg;
	lscResumeMsg.header.version = (uint8_t)atoi(_gConfig.request[_stepNumber].header["version"].c_str());
	lscResumeMsg.header.transactionId = (uint8_t)atoi(_gConfig.request[_stepNumber].header["transactionId"].c_str());	
	lscResumeMsg.header.opCode = (uint8_t)atoi(_gConfig.request[_stepNumber].messageId.c_str());
	lscResumeMsg.header.statusCode = (uint8_t)atoi(_gConfig.request[_stepNumber].header["statusCode"].c_str());
	lscResumeMsg.header.streamHandle = m_nStreamHandle;

	lscResumeMsg.startNpt = (uint32_t)atoi(_gConfig.request[_stepNumber].appdata["StartNpt"].c_str());
	lscResumeMsg.numerator = (int16_t)atoi(_gConfig.request[_stepNumber].appdata["Numerator"].c_str());
	lscResumeMsg.denominator = (uint16_t)atoi(_gConfig.request[_stepNumber].appdata["Denominator"].c_str());

	lscResumeMsg.hton();

	uint8 buf[1024];
	int len = sizeof(lscResumeMsg);
	memcpy(buf, &lscResumeMsg, sizeof(lscResumeMsg));

	(*_gLog)(ZQ::common::Log::L_DEBUG,CLOGFMT(SingleSession,"Resume Request ..."),m_nStreamHandle);

	return m_lscpClient->SyncSendData((char*)buf,len);
}

bool SingleSession::Release()
{

	uint8 strBuf[1024];
	memset(strBuf, '0', 1024);
	ZQ::DSMCC::DsmccMsg::HardHeader tempheader;
	ZQ::DSMCC::StringMap tempmeta;

	tempheader.protocolDiscriminator = (uint8)atoi(_gConfig.commonheader.protocolDiscriminator.c_str());
	tempheader.dsmccType = (uint8)atoi(_gConfig.commonheader.dsmccType.c_str());
	tempheader.messageId = (uint16)atoi(_gConfig.request[_stepNumber].messageId.c_str());		//setup
	tempheader.reserved1 = (uint8)atoi(_gConfig.commonheader.reserved.c_str());
	tempheader.transactionId = (uint32)atoi(_gConfig.commonheader.transactionId.c_str());


	tempmeta[CRMetaData_protocolDiscriminator] = _gConfig.commonheader.protocolDiscriminator;
	tempmeta[CRMetaData_dsmccType] = _gConfig.commonheader.dsmccType;
	tempmeta[CRMetaData_messageId] = _gConfig.request[_stepNumber].messageId;
	tempmeta[CRMetaData_reserved1] = _gConfig.commonheader.reserved;
	tempmeta[CRMetaData_transactionId] = _gConfig.commonheader.transactionId;


	tempmeta[CRMetaData_SessionId] = m_strSessionId;

	tempmeta[CRMetaData_CSRreason] = _gConfig.request[_stepNumber].appdata["reason"];

	ZQ::DSMCC::DsmccMsg::Ptr releaseReqPtr = new ZQ::DSMCC::ClientSessionReleaseRequest(tempheader);

	releaseReqPtr->readMetaData(tempmeta);
	int msglen = releaseReqPtr->toMessage(strBuf,1024);

// 	if (!TestSetupMessage(strBuf,msglen))
// 	{
// 		return false;
// 	}

	(*_gLog)(ZQ::common::Log::L_INFO,CLOGFMT(SingleSession,"Release Request SessionId = %s"),m_strSessionId.c_str());
	if (!m_dsmccClient->SendBinary((char*)strBuf,msglen))
	{
		_responseCode = send_error;
		return false;
	}

// 	if (!m_TcpClient->SyncSendRelease((char*)strBuf,msglen))
// 	{
// 		return false;
// 	}

	int i = 0;
	int count = _gConfig.messageTimeout/1000;
	if(count < 5)
		count = 5;
	while (i< count)
	{
		if (m_dsmccClient->getReleaseStatus())
		{
			return true;
		}
		else
		{
			_responseCode = m_dsmccClient->getResponseCode();
			if (_responseCode != parse_error && _responseCode != RsnOK)
			{
				return false;
			}
		}
		#ifdef ZQ_OS_MSWIN
				Sleep(1000);
		#else
				sleep(1);
		#endif
		i++;
	}
	_responseCode = release_error;
	(*_gLog)(ZQ::common::Log::L_INFO,CLOGFMT(SingleSession,"Release Request time out. time = %ds"),count);
	return false;
}


int SingleSession::toStreamHandle(const uint8* pbuf,int len)
{
	size_t processedLength = 0;

	ZQ::DSMCC::DsmccMsg::Ptr pMsg = ZQ::DSMCC::DsmccMsg::parseMessage(pbuf, len, processedLength);

	if(pMsg)
	{

 		ZQ::DSMCC::StringMap metadatas;
 		pMsg->toMetaData(metadatas);


		ZQ::DSMCC::DsmccMsg::HardHeader tmpHeader;
		tmpHeader.messageId = pMsg->getMessageId();

		if(tmpHeader.messageId == MsgID_ProceedingIndication)
			return 1;
		else if(tmpHeader.messageId == MsgID_SetupConfirm)
		{
			StringMap::const_iterator itorMd;
			itorMd = metadatas.find(CRMetaData_StreamHandelId);
			if(itorMd == metadatas.end() || itorMd->second.empty())
			{
				return -1;
			}
			sscanf(itorMd->second.c_str(),"%u", &m_nStreamHandle);
		}
		else
			return -1;
	}
	else
		return -1;
	return 0;
}

std::string SingleSession::getErrorMessage(int errorcode)
{
	switch(errorcode)
	{
	case session_ok:
		return "session ok";
	case setup_error:
		return "setup request time out";
	case play_error:
		return "play request error";
	case pause_error:
		return "pause request error";
	case status_error:
		return "status request error";
	case resume_error:
		return "resume request error";
	case release_error:
		return "release request time out";
	case connect_error:
		return "tcp connect error";
	case send_error:
		return "send error";
	case step_error:
		return "first request must be SETUP";
	case  parse_error:
		return "parse response error";
	case MotoReserved:
		return "MotoReserved";
	case RspNeNoCalls:
		return "RspNeNoCalls";
	case RspNeNoSession:
		return "RspNeNoSession";
	case RspSeNOCalls:
		return "RspSeNOCalls";
	case RspSeNoService:
		return "RspSeNoService";
	case RspSeNoSession:
		return "RspSeNoSession";
	case RspNeNoResource:
		return "RspNeNoResource";
	case RspSeNoResource:
		return "RspSeNoResource";
	case RspNeProcError:
		return "RspNeProcError";
	case RspSeProcError:
		return "RspSeProcError";
	case RspSeNoResponse:
		return "RspSeNoResponse";
	case RspUserAccountNotExist:							/// From 0x8001 - 8fff defined by 3rd servers
		return "RspUserAccountNotExist";
	case RspMovieNotExist:
		return "RspMovieNotExist";
	case RspNoEnoughMoney:
		return "RspNoEnoughMoney";
	case RspExceptionError:
		return "RspExceptionError";
	case RspProgramInfoError:
		return "RspProgramInfoError";
	case RspProgramError:
		return "RspProgramError";
	case RspProgramUpdateError:
		return "RspProgramUpdateError";
	case RspSystemCongestion:
		return "RspSystemCongestion";
	case RspBillingUnavailable:
		return "RspBillingUnavailable";
	case RspAssetNotFound:
		return "RspAssetNotFound";
	default:
		return "Undefined error code";
	}
}

void SingleSession::GetErrorInfo(std::string& status,std::string&commandName, int& responseCode, std::string& errorMessage, int& stepNum)
{
	commandName = _errorRequestName;
	responseCode = _errResponseCode;
	stepNum = _errStepNumber;
	status = _currentStatus;
	errorMessage = getErrorMessage(_errResponseCode);
}
