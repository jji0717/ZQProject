#include "DsmccSocket.h"
#include "DsmccMsg.h"
//#include "DsmccDefine.h"
#include <iostream>
#include <FileLog.h>
#include "SingleSession.h"

extern ZQ::common::FileLog  *_gLog;
using namespace ZQ::DSMCC;
DsmccClientSocket::DsmccClientSocket(bool istcp)
		:m_socket(0)
		,m_istcp(istcp)
		,ReleaseOk(false)
		,SetupOk(false)
		,m_responsecode(parse_error)
{
	if (m_istcp)
	{
		m_socket = socket(AF_INET,SOCK_STREAM,0);  
	}
	else
	{
		m_socket = socket(AF_INET,SOCK_DGRAM,0);  
	}
	
	memset(&peer_addr,0,sizeof(peer_addr));
	if (m_socket == INVALID_SOCKET) {  
		throw "INVALID_SOCKET s_";  
	}  
}

DsmccClientSocket::~DsmccClientSocket()
{
#ifdef ZQ_OS_MSWIN
	closesocket(m_socket);
#else
	close(m_socket);
#endif
}

bool DsmccClientSocket::Init(std::string& ip,std::string& port)
{
#ifdef ZQ_OS_MSWIN
	peer_addr.sin_family=AF_INET;
	peer_addr.sin_port=htons(atoi(port.c_str()));
	peer_addr.sin_addr.S_un.S_addr=inet_addr(ip.c_str());
#else
	peer_addr.sin_family=AF_INET;
	peer_addr.sin_port=htons(atoi(port.c_str()));
	peer_addr.sin_addr.s_addr=inet_addr(ip.c_str());
#endif

	if (m_istcp)
	{
		if (connect(m_socket, (sockaddr *)&peer_addr, sizeof(sockaddr))) {  
			(*_gLog)(ZQ::common::Log::L_ERROR,CLOGFMT(DsmccClientSocket,"Tcp client connect error [Code:%d],ip = %s,port = %s"),connect_error,ip.c_str(),port.c_str());
			_Resplock.enter();
			m_responsecode = connect_error;
			_Resplock.leave();
			return false;
		} 
	}

	return true;
}

bool DsmccClientSocket::UnInit()
{
	#ifdef ZQ_OS_MSWIN
		closesocket(m_socket);
	#else
		close(m_socket);
	#endif
	ReleaseOk = false;
	SetupOk = false;
	return true;
}

int DsmccClientSocket::SendBinary(char* pdata,unsigned long slen)
{
	if (m_istcp)
	{
		return send(m_socket,pdata,slen,0);
	}
	else
	{
		return sendto(m_socket, pdata, slen, 0, (struct sockaddr *)&peer_addr,sizeof(peer_addr));
	}
}

int DsmccClientSocket::RecvBinary(char* pdata,unsigned long slen)
{
// 	u_long arg = 0; 
// 	ioctlsocket(m_socket, FIONREAD, &arg);
	return recv(m_socket, pdata, slen, 0); 
}

bool DsmccClientSocket::SyncSendRelease(char* pdata,unsigned long slen)
{
	struct timeval timeout_insobj;
	timeout_insobj.tv_sec = 15;
	timeout_insobj.tv_usec = 0;
	int retnum = 0;
	char  recvdatabuf[1024]={0};

	if(SendBinary(pdata,slen) == -1)
	{
		return false;
	}

	fd_set  fdR; 
	FD_ZERO(&fdR);
	FD_SET(m_socket,&fdR);

	int   blflag=-1,blflagsuccess2=-1;

	if (select(m_socket+1,&fdR,NULL,NULL,&timeout_insobj) <= 0)
	{
		(*_gLog)(ZQ::common::Log::L_ERROR,CLOGFMT(DsmccClientSocket,"Tcp client select error"));
		return false;
	}
	if (FD_ISSET(m_socket,&fdR))
	{
		retnum = RecvBinary(recvdatabuf,1024);
		if (retnum <= 0)
		{
			return false;
		}
		size_t processedLength = 0;

		ZQ::DSMCC::DsmccMsg::Ptr pMsg = ZQ::DSMCC::DsmccMsg::parseMessage((uint8*)pdata, slen, processedLength);

		if(pMsg)
		{

			ZQ::DSMCC::StringMap metadatas;
			pMsg->toMetaData(metadatas);


			ZQ::DSMCC::DsmccMsg::HardHeader tmpHeader;
			tmpHeader.messageId = pMsg->getMessageId();

			if(tmpHeader.messageId == MsgID_ReleaseConfirm)
			{
				_Relelock.enter();
				ReleaseOk = true;
				_Relelock.leave();
				return true;
			}
		}
	}
	else
		return false;

}

bool DsmccClientSocket::SyncSendData(char* pdata,unsigned long slen)
{
	if (SendBinary(pdata,slen) < 0)
	{
		return false;
	}
	struct timeval timeout_insobj;
	timeout_insobj.tv_sec = 5;
	timeout_insobj.tv_usec = 0;
	fd_set  fdR; 
	FD_ZERO(&fdR);
	FD_SET(m_socket,&fdR);

	if (select(m_socket+1,&fdR,NULL,NULL,&timeout_insobj) <= 0)
	{
		return false;
	}
	char recvbuf[1024] = {0};
	int recvlen = RecvBinary(recvbuf,1024);
	if (recvlen == -1)
	{
		return false;
	}
	return true;
}

bool DsmccClientSocket::SendDataSync(char* pdata,unsigned long slen)
{
	struct timeval timeout_insobj;
	timeout_insobj.tv_sec = 15;
	timeout_insobj.tv_usec = 0;
	int retnum = 0;
	char  recvdatabuf[1024]={0};

	unsigned short int  uiflag=0x8240;

	if(SendBinary(pdata,slen) == -1)
	{
		return false;
	}
	
	fd_set  fdR; 
	FD_ZERO(&fdR);
	FD_SET(m_socket,&fdR);

	int   blflag=-1,blflagsuccess2=-1;

	if (select(m_socket+1,&fdR,NULL,NULL,&timeout_insobj) <= 0)
	{
		return false;
	}
	
	if (FD_ISSET(m_socket,&fdR))
	{
		retnum = RecvBinary(recvdatabuf,1024);
		if (retnum <= 0)
		{
			return false;
		}
		blflag=memcmp(&uiflag,recvdatabuf+2,2);
		if (blflag != 0)
		{
			return false;
		}
	}

	if (select(m_socket+1,&fdR,NULL,NULL,&timeout_insobj) <= 0)
	{
		return false;
	}

	if (FD_ISSET(m_socket,&fdR))
	{
		memset(recvdatabuf,0x00,sizeof(recvdatabuf));
		retnum = RecvBinary(recvdatabuf,1024);
		if (retnum <= 0)
		{
			return false;
		}
		

		unsigned short int  uconfirm=0x1140,uconfirmsuccess=0x0000;
		blflag=memcmp(&uconfirm,recvdatabuf+2,2);
		blflagsuccess2=memcmp(&uconfirmsuccess,recvdatabuf+22,2);

		if(0 == blflag && 0 == blflagsuccess2)
		{
		//	toStreamHandle(recvdatabuf,retnum);
			return true;
		}
		else
		{	
			return false;
		}
	}
}

bool DsmccClientSocket::getReleaseStatus()
{
	ZQ::common::MutexGuard gd(_Relelock);
	return ReleaseOk;
}

bool DsmccClientSocket::getSetupStatus()
{
	ZQ::common::MutexGuard gd(_Setuplock);
	return SetupOk;
}

uint32 DsmccClientSocket::getStreamHandle()
{
	return m_StreamHandle;
}

int  DsmccClientSocket::getResponseCode()
{
	ZQ::common::MutexGuard gd(_Resplock);
	return m_responsecode;
}

bool DsmccClientSocket::toStreamHandle(const char* pbuf,int nlen)
{
	char original_StreamHandle[5];
	memset(original_StreamHandle,0x00,sizeof(original_StreamHandle));
	memcpy(original_StreamHandle,pbuf+nlen-4,4);
	sscanf(original_StreamHandle,"%u", &m_StreamHandle);
	SetupOk = true;

	printf("original_StreamHandle:%u\n",m_StreamHandle);
	for(int i = 0; i < sizeof(original_StreamHandle); i++)
		printf("%0x\t ", original_StreamHandle[i]);
	printf("\n \n");

	std::cout << "setup success" <<std::endl;
	return true;
}

bool DsmccClientSocket::OnRecvData(const char* pdata,int slen)
{
// 	if (slen < 0)
// 	{
// 		return false;
// 	}
// 	unsigned short int  uiflag=0x8240;
// 	unsigned short int  uconfirm=0x1140,uconfirmsuccess=0x0000;
// 	int   blflag=-1,blflagsuccess2=-1;
// 
// 	blflag=memcmp(&uiflag,pdata+2,2);
// 	if (blflag != 0)
// 	{
// 		blflag=memcmp(&uconfirm,pdata+2,2);
// 		blflagsuccess2=memcmp(&uconfirmsuccess,pdata+22,2);
// 		if(0 == blflag && 0 == blflagsuccess2)
// 		{
// 			toStreamHandle(pdata,slen);
// 			return true;
// 		}
// 		return false;
// 	}
// 	else
// 	{	
// 		return true;
// 	}

	(*_gLog)(ZQ::common::Log::L_DEBUG,CLOGFMT(DsmccClientSocket,"received len = %d"),slen);
	size_t processedLength = 0;

	ZQ::DSMCC::DsmccMsg::Ptr pMsg = ZQ::DSMCC::DsmccMsg::parseMessage((uint8*)pdata, slen, processedLength);

	if(pMsg)
	{

		ZQ::DSMCC::StringMap metadatas;
		StringMap::const_iterator itMd;
		pMsg->toMetaData(metadatas);

		ZQ::DSMCC::DsmccMsg::HardHeader tmpHeader;
		tmpHeader.messageId = pMsg->getMessageId();
		if(tmpHeader.messageId == MsgID_ReleaseConfirm)
		{
			(*_gLog)(ZQ::common::Log::L_INFO,CLOGFMT(DsmccClientSocket,"Release confirm"));
			itMd = metadatas.find(CRMetaData_CSRreason);
			if(itMd == metadatas.end() || itMd->second.empty())
			{
				return false;
			}
			_Resplock.enter();
			m_responsecode = atoi(itMd->second.c_str());
			_Resplock.leave();
			if ( m_responsecode != RsnOK && m_responsecode != RspSeNoSession)
			{
				return false;
			}
			_Relelock.enter();
			ReleaseOk = true;
			_Relelock.leave();
			return true;
		}
		if(tmpHeader.messageId == MsgID_ProceedingIndication)
		{
			(*_gLog)(ZQ::common::Log::L_INFO,CLOGFMT(DsmccClientSocket,"Setup message is processing"));
			return true;
		}
		else if(tmpHeader.messageId == MsgID_SetupConfirm)
		{
			itMd = metadatas.find(CRMetaData_CSSCresponse);
			if(itMd == metadatas.end() || itMd->second.empty())
			{
				return false;
			}
			_Resplock.enter();
			m_responsecode = (ZQ::DSMCC::ResponseCode)atoi(itMd->second.c_str());
			_Resplock.leave();
			if ( m_responsecode != RsnOK)
			{
				return false;
			}

			ZQ::DSMCC::StringMap::const_iterator itorMd;
			itorMd = metadatas.find(CRMetaData_StreamHandelId);
			if(itorMd == metadatas.end() || itorMd->second.empty())
			{
				return false;
			}
			sscanf(itorMd->second.c_str(),"%u", &m_StreamHandle);
			(*_gLog)(ZQ::common::Log::L_INFO,CLOGFMT(DsmccClientSocket,"m_StreamHandle = %u"),m_StreamHandle);
			//printf("StreamHandle = %u \n",m_StreamHandle);
			_Setuplock.enter();
			SetupOk = true;
			_Setuplock.leave();
		}
		else
		{
			if (SetupOk&&(slen==12))
			{
				ReleaseOk = true;
				m_responsecode = RsnOK;
				return true;
			}
			m_responsecode = parse_error;
			(*_gLog)(ZQ::common::Log::L_ERROR,CLOGFMT(DsmccClientSocket," Message parsing failure "));
			return false;
		}
	}
	else
	{
		m_responsecode = parse_error;
		(*_gLog)(ZQ::common::Log::L_ERROR,CLOGFMT(DsmccClientSocket," Message parsing failure "));
		return false;
	}
	return true;
}
