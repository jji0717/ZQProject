#ifndef SA_DSMCCSOCKET_H_
#define  SA_DSMCCSOCKET_H_

#include <Locks.h>
#include "ZQ_common_conf.h"
#ifdef ZQ_OS_MSWIN
    #include <WinSock2.h>
#else


    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/types.h>
    #include <stdlib.h>
#endif

#include <string>


class DsmccClientSocket
{
public:
	DsmccClientSocket(bool istcp);
	virtual ~DsmccClientSocket();
	void DsmccCloseSocket();
	bool Init(std::string& ip,std::string& port);
	bool UnInit();
	int SendBinary(char* pdata,unsigned long slen);
	int RecvBinary(char* pdata,unsigned long slen);
	bool SyncSendRelease(char* pdata,unsigned long slen);

	bool SendDataSync(char* pdata,unsigned long slen);
	bool OnRecvData(const char* pdata,int slen);
	bool getReleaseStatus();
	bool getSetupStatus();
	int  getResponseCode();
	uint32 getStreamHandle();
	bool SyncSendData(char* pdata,unsigned long slen);

	
private:
	int m_socket;
	struct sockaddr_in peer_addr; 
	ZQ::common::Mutex		_Resplock;
	ZQ::common::Mutex		_Relelock;
	ZQ::common::Mutex		_Setuplock;
	uint32 m_StreamHandle;
	int m_responsecode;
	bool ReleaseOk;
	bool SetupOk;
	bool m_istcp;
	bool toStreamHandle(const char* pbuf,int nlen);
};







#endif
