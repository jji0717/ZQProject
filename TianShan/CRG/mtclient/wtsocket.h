#ifndef __WTSOCKET_DEFINE__H_
#define  __WTSOCKET_DEFINE__H_
#include <WinSock2.h>  
#include <string>  
#include <stdio.h>
#include <errno.h>
#include  <string.h>
#include <string>
#include <iostream>
#include "TianShanDefines.h"
#include <TianShanIceHelper.h>
#include <TianShanIce.h>
#include <exception>
#include <iostream>
#include <string>
#include <stdarg.h>

using namespace std;

extern unsigned long g_ltimeout;
extern char* g_pmv_resourse;


extern std::string g_sessionID;
extern std::string	g_locIP;
extern ZQ::common::Log  *PLOG;
typedef  long ssize_t ;

#define  RECVDATAMAX   2048

enum TypeSocket {BlockingSocket, NonBlockingSocket};  

// --base class
class Socket 
{  
public:  
	virtual ~Socket();  
	void   Close();  
	int   SendBinary(char* pdata,ULONG slen,bool fs1,bool fs2);
	int   RecvBinary(char* pdata,ULONG slen,bool fs1,bool fs2);
protected:  
	friend class SocketServer;  
	friend class SocketSelect;  // --friend class 
	Socket();  
	SOCKET s_;  
	SOCKET s2_;
	int* refCounter_;  
private:  
	static int  nofSockets_;  
};  

// --client class
class SocketClient : public Socket {  
public:  
	SocketClient();
	bool selectstatus(int& iStatus,const char* pmethod,std::string serialNumber);
	bool inisocket(std::string& sipport,bool bflags1,bool bflags2);
	bool toStreamHandle(const char* pbuf,ssize_t& nlen);
	bool send_dataSyn(std::string serialNumber ,std::string& stripport,char* pdata,ULONG,const char* pmethod,bool bfsetup,bool bfclose);
	bool send_dataAsyn(std::string serialNumber ,std::string& stripport,char* pdata,unsigned long slen,const char* pmethod);
public:
	char original_StreamHandle[5];
	bool hexdump(void *ptr, int buflen,int bflag=2);//0 send falg  ;1 receive flag 2 local flag 
	unsigned int _StreamHandle ;
private:
	LARGE_INTEGER m_nFreq;
	LARGE_INTEGER m_nBeginTime;  
	LARGE_INTEGER nEndTime;
	int err; 
	WORD wVersionRequested;  
	WSADATA wsaData;  

	size_t  sendNumber;
	size_t  recvNumber;
	struct sockaddr_in peer_addr;  
	std::string peerip;
	std::string peerport;
};  

#endif //__WTSOCKET_DEFINE__H_