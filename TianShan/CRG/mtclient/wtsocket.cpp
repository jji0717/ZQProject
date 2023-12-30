#include "wtsocket.h"
#include "wtlog.h"

#include "../DsmccCRG/DsmccDefine.h"

int Socket::nofSockets_= 0;  
extern ZQ::DSMCC::ProtocolType protocol_type;

Socket::Socket() : s_(0) 
{  
	// UDP: use SOCK_DGRAM instead of SOCK_STREAM  
	s_ = socket(AF_INET,SOCK_STREAM,0);  
	s2_ = socket(AF_INET,SOCK_STREAM,0);
	if (s_ == INVALID_SOCKET) {  
		throw "INVALID_SOCKET s_";  
	}  
	if (s2_ == INVALID_SOCKET) 
		throw "INVALID_SOCKET s2_";  
}  

Socket::~Socket() 
{ 
	Close();  
	(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(Socket,"Socket beginning to de-construction OK"));
}  

void Socket::Close() 
{  
	closesocket(s_);  
	closesocket(s2_);
}  

int  Socket::SendBinary(char* pdata,ULONG slen,bool fs1,bool fs2)
{
	if (1 == fs1)
	{
		return send(s_,pdata,slen,0);
	}
	if (1 == fs2)
	{
		return send(s2_,pdata,slen,0);
	}
	 return true;
}
int  Socket::RecvBinary(char* pdata,ULONG slen,bool fs1,bool fs2)
{
	u_long arg = 0;  
	if (1 == fs1)
	{ 
		ioctlsocket(s_, FIONREAD, &arg) ;//!= 0)  
		return recv(s_, pdata, slen, 0); 
	}
	if (1 == fs2) 
	{
		ioctlsocket(s2_, FIONREAD, &arg) ;//!= 0)  
		return recv(s2_, pdata, slen, 0);  
	}
	return true;
}

bool SocketClient::hexdump(void *ptr, int buflen,int bflag)
{
	unsigned char *buf = (unsigned char*)ptr;
	char outbuffer[260]={0},cobuf[256*2]={0};
	unsigned char buffer[16*2]={0};
	int i, j;
	for (i=0; i<buflen; i+=16) 
	{
		sprintf(outbuffer, "%8.8lx: ", i);
		for (j=0; j<16; j++)
		{
			if (i+j < buflen)
				sprintf(outbuffer+10+3*j, "%2.2X ",buf[i+j]);
			if (isprint(buf[i+j]))
				buffer[j]=buf[i+j];
			else
				buffer[j] = '.';
		}
		sprintf(cobuf,"%-60s %*.*s", outbuffer, j, j, buffer);
		//global->logtoformat("%s",cobuf);
	}
	return true;	
}

bool SocketClient::toStreamHandle(const char* pbuf,ssize_t& nlen)
{
	memset(original_StreamHandle,0x00,sizeof(original_StreamHandle));
	memcpy(original_StreamHandle,pbuf+nlen-4,4);
	return false;
}

SocketClient::SocketClient() : Socket() {  
	QueryPerformanceFrequency(&m_nFreq);
}  

bool SocketClient::selectstatus(int& iStatus,const char* pmethod,std::string serialNumber)
{
	if(0 == iStatus)
	{
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"%s  receive data timeout by sessionID[%s]"),pmethod,serialNumber.c_str());
		return false;
	}
	if(0 > iStatus)
	{
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"%s on error [%s] occured when close receive data"),pmethod,strerror(errno));
		return false;
	}
	return true;
}
bool SocketClient::inisocket(std::string& sipport,bool bflags1,bool bflags2)
{
	std::string error;  
	size_t pos1=sipport.find(":",0);
	peerip=sipport.substr(0,pos1);
	peerport=sipport.substr(pos1+1,sipport.length());

	peer_addr.sin_family=AF_INET;
	peer_addr.sin_port=htons(atoi(peerport.c_str()));
	peer_addr.sin_addr.S_un.S_addr=inet_addr(peerip.c_str());
	if (1 == bflags1)
	{
		if (::connect(s_, (sockaddr *) &peer_addr, sizeof(sockaddr))) {  
			error = strerror(WSAGetLastError());  
			throw error;  
		}  
	}
	if (1 == bflags2)
	{
		if (::connect(s2_, (sockaddr *) &peer_addr, sizeof(sockaddr))) {  
			error = strerror(WSAGetLastError());  
			throw error;  
		}  
	}
	
	return true;
}
bool SocketClient::send_dataAsyn(std::string serialNumber ,std::string& stripport,char* pdata,unsigned long slen,const char* pmethod)
{
	LARGE_INTEGER m_nBeginTime1,nEndTime1;
	char logsub[50]={0},recvBuffer[RECVDATAMAX]={0};
	sprintf(logsub,"mtclient.%s",pmethod);
	//(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"%s : begging to send data"),logsub);
	struct timeval timeout_insobj;
	timeout_insobj.tv_sec=g_ltimeout/1000;
	timeout_insobj.tv_usec=0;
	QueryPerformanceCounter(&m_nBeginTime1); 
	sendNumber = SendBinary(pdata,slen,0,1);

	if (0 >  sendNumber)
	{
		(*PLOG)(ZQ::common::Log::L_ERROR,CCLOGFMT(SocketClient,"%s send data  failed:%s"),pmethod,strerror(errno));
		return false;
	}
	else
	{
		char tpbuffer[256]={0};
		sprintf(tpbuffer,"%s: %s-->>%s",pmethod,serialNumber.c_str(),stripport.c_str());
		PLOG->hexDump(ZQ::common::Log::L_INFO,pdata,slen,tpbuffer);
	}
	fd_set  fdR; 
	FD_ZERO(&fdR);
	FD_SET(s2_,&fdR);
	(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"%s waiting for receiving data. wait-time  %d sec"),pmethod,g_ltimeout/1000);
	int iStatus = select(s2_+1,&fdR,NULL,NULL,&timeout_insobj);
	if(0 == iStatus)
	{
		(*PLOG)(ZQ::common::Log::L_WARNING,CCLOGFMT(SocketClient,"%s timeout when  receive data of the SessionID[%s]"),pmethod,serialNumber.c_str());
		return false;
	}
	if(0 > iStatus)
	{
		(*PLOG)(ZQ::common::Log::L_ERROR,CCLOGFMT(SocketClient,"%s on error [%s] occured when  receive data of the SessionID[%s] "),pmethod,strerror(errno),serialNumber.c_str());
		return false;
	}
	if (FD_ISSET(s2_,&fdR)) //if (ssl_insobj.Readable(&s2_))
	{
		recvNumber=RecvBinary(recvBuffer,RECVDATAMAX,0,1);
		QueryPerformanceCounter(&nEndTime1);
	}
	else
	{
		QueryPerformanceCounter(&nEndTime1);
		float time_use=(double)(nEndTime1.QuadPart-m_nBeginTime1.QuadPart)*1000/m_nFreq.QuadPart ;
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"%s receive data failed. Time-consuming[%.3fms failed]"),pmethod,time_use);
		return false;
	}
#ifdef  _KWG_DEBUG
	//cout<<".....m_nBeginTime="<<m_nBeginTime.QuadPart<<endl;
	//cout<<".....nEndTime="<<nEndTime.QuadPart<<endl;
	//cout<<"m_nFreq="<<m_nFreq.QuadPart<<endl;
#endif
	float time_use=(double)(nEndTime1.QuadPart-m_nBeginTime1.QuadPart)*1000/m_nFreq.QuadPart ;
	char tpbuffer[256]={0};
	sprintf(tpbuffer,"%s: %s:9999<<--%s",pmethod,g_locIP.c_str(),serialNumber.c_str());
	PLOG->hexDump(ZQ::common::Log::L_INFO,recvBuffer,recvNumber,tpbuffer);
	(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"%s senddata-len:%d; recvdata-len:%d; Time-consuming[%.3fms success] "),pmethod,sendNumber,recvNumber,time_use);
	return true;
}
bool SocketClient::send_dataSyn(std::string serialNumber ,std::string& stripport,char* pdata,ULONG slen,const char* pmethod,bool bfsetup,bool bfclose)
{
	LARGE_INTEGER m_nBeginTime1,nEndTime1;
	struct sockaddr_in resoursein;
	int address_size;float time_use1=0,time_use2=0;
	char  recvdatabuf[RECVDATAMAX]={0};ssize_t retNumber,retNumber1,retNumber2;

	address_size=sizeof(resoursein);
	unsigned short int  uiflag=0x8240;
	struct timeval timeout_insobj;
	timeout_insobj.tv_sec=g_ltimeout/1000;
	timeout_insobj.tv_usec=0;

	QueryPerformanceCounter(&m_nBeginTime1); 
	retNumber = SendBinary(pdata,slen,1,0);
	if (sendNumber == -1) { 
		(*PLOG)(ZQ::common::Log::L_WARNING,CCLOGFMT(SocketClient,"%s : send data failed [%s],so and programe return false!"),pmethod,strerror(errno));
		return false;
	}
	else
	{	
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"%s: send data hexdump  :slen-len:%d "),pmethod,slen);
		char tpbuffer[256]={0};
		sprintf(tpbuffer,"%s: %s-->>%s",pmethod,serialNumber.c_str(),stripport.c_str());
		PLOG->hexDump(ZQ::common::Log::L_INFO,pdata,slen,tpbuffer);
	}
	fd_set  fdR; 
	FD_ZERO(&fdR);
	FD_SET(s_,&fdR);
	if (1 == bfsetup)
	{
		int   blflag=-1,blflagsuccess2=-1;
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"%s, 1 waiting for receiving data. if wait-time great than %d sec,the program exit!"),pmethod,g_ltimeout/1000);
		int iStatus;
		if (protocol_type == ZQ::DSMCC::Protocol_MOTO)
		{
			iStatus = select(s_+1,&fdR,NULL,NULL,&timeout_insobj);
			if(!selectstatus(iStatus,pmethod,serialNumber))
				return false;
			if (FD_ISSET(s_,&fdR))
			{
				retNumber1 = RecvBinary(recvdatabuf,RECVDATAMAX,1,0);
				QueryPerformanceCounter(&nEndTime1);
				if(-1 == retNumber )
				{      
					(*PLOG)(ZQ::common::Log::L_ERROR,CCLOGFMT(SocketClient,"%s the first receive data failed [: %s],and this thread exited!"),pmethod,strerror(errno));
					return false;
				}
				else
				{
					char tpbuffer[256]={0};
					sprintf(tpbuffer,"first %s:9999<<-- %s",g_locIP.c_str(),serialNumber.c_str());
					PLOG->hexDump(ZQ::common::Log::L_INFO,recvdatabuf,retNumber1,tpbuffer);
				}


				time_use1=(double)(nEndTime1.QuadPart-m_nBeginTime1.QuadPart)*1000/m_nFreq.QuadPart ;
				blflag=memcmp(&uiflag,recvdatabuf+2,2);
				if(0 == blflag)
					(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"%s,The first received data len: %d . Time-consuming[%.3fms success]"),pmethod,retNumber1,time_use1);
				else
				{
					(*PLOG)(ZQ::common::Log::L_ERROR,CCLOGFMT(SocketClient,"%s,the first recvdata failed ! and return false "),pmethod);
					return false;
				}
			}

			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"%s,2 waiting for receiving data. if wait-time great than %d sec,and this thread exited!"),pmethod,g_ltimeout/1000);
		}
		iStatus = select(s_+1,&fdR,NULL,NULL,&timeout_insobj);
		if(!selectstatus(iStatus,pmethod,serialNumber))
			return false;
		if (FD_ISSET(s_,&fdR))
		{
			memset(recvdatabuf,0x00,sizeof(recvdatabuf));
			retNumber2 = RecvBinary(recvdatabuf,RECVDATAMAX,1,0);
			QueryPerformanceCounter(&nEndTime1);
			if(-1 == retNumber )
			{
				(*PLOG)(ZQ::common::Log::L_ERROR,CCLOGFMT(SocketClient,"%s,the second recvdata failed [%s],and the program  exited!"),pmethod,strerror(errno));
				return false;
			}
			else
			{
				char tpbuffer[256]={0};
				sprintf(tpbuffer,"second %s:9999<<--%s",g_locIP.c_str(),serialNumber.c_str());
				PLOG->hexDump(ZQ::common::Log::L_INFO,recvdatabuf,retNumber2,tpbuffer);
			}
			time_use2 = (double)(nEndTime1.QuadPart-m_nBeginTime1.QuadPart)*1000/m_nFreq.QuadPart ;
			unsigned short int  uconfirm=0x1140,uconfirmsuccess=0x0000;
			blflag=memcmp(&uconfirm,recvdatabuf+2,2);
			blflagsuccess2=memcmp(&uconfirmsuccess,recvdatabuf+22,2);
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"9999--%d"),(int)blflagsuccess2);
			if(0 == blflag && 0 == blflagsuccess2)
			{
				toStreamHandle(recvdatabuf,retNumber2);
				(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"%s,senddata-len:%d ;recvdata1-len:%d ;recvdata2-len:%d ; setup Time-consuming[%.3fms success]"),pmethod,retNumber,retNumber1,retNumber2,time_use2);
				return true;
			}
			else
			{	
				(*PLOG)(ZQ::common::Log::L_WARNING,CCLOGFMT(SocketClient,"%s,the second recvdata:%d and print 10 bytes"),pmethod,retNumber2);
				PLOG->hexDump(ZQ::common::Log::L_INFO,recvdatabuf,10);
				(*PLOG)(ZQ::common::Log::L_ERROR,CCLOGFMT(SocketClient,"%s, Time-consuming[%.3f ms failed]"),pmethod,time_use2);
				return false;
			}
		}
	}
	if (1 == bfclose)
	{
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"%s waiting for receiving data. wait-time  %d sec"),pmethod,g_ltimeout/1000);
		int iStatus = select(s_+1,&fdR,NULL,NULL,&timeout_insobj);
		if(!selectstatus(iStatus,pmethod,serialNumber))
			return false;
		if (FD_ISSET(s_,&fdR))
		{
			retNumber1 = RecvBinary(recvdatabuf,RECVDATAMAX,1,0);
			QueryPerformanceCounter(&nEndTime1);
			if(-1 == retNumber )
			{      
				time_use1=(double)(nEndTime1.QuadPart-m_nBeginTime1.QuadPart)*1000/m_nFreq.QuadPart ;
				(*PLOG)(ZQ::common::Log::L_ERROR,CCLOGFMT(SocketClient,"%s received data . Time-consuming[%.3fms failed]"),pmethod,time_use1);
				return false;
			}
			else
			{
				char tpbuffer[256]={0};
				sprintf(tpbuffer,"%s: second %s:9999<<--%s",pmethod,g_locIP.c_str(),serialNumber.c_str());
				PLOG->hexDump(ZQ::common::Log::L_INFO,recvdatabuf,retNumber1,tpbuffer);
			}
			time_use1=(double)(nEndTime1.QuadPart-m_nBeginTime1.QuadPart)*1000/m_nFreq.QuadPart ;
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(SocketClient,"%s received data len: %d.Time-consuming[%.3f ms success]"),pmethod,retNumber1,time_use1);
			return true;
		}
	}
	return false;
}

