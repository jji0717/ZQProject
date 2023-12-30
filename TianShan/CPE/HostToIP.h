#ifndef _HOST_TO_IP
#define _HOST_TO_IP

#include <ZQ_common_conf.h>
#include <string>

#ifdef ZQ_OS_LINUX
extern "C"
{
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}
#endif

class HostToIP
{
public:
	static bool translateHostIP(const char *dn_or_ip, std::string& strIP)
	{
		struct   hostent   *thisHost;   
		struct   in_addr   in;     
		char   *ptr;   
#ifdef ZQ_OS_MSWIN
		

		WORD   wVersionRequested;   
		WSADATA   wsaData;   
		int   err;   
		wVersionRequested   =   MAKEWORD(2,0);   
		err   =   WSAStartup(wVersionRequested,&wsaData);   
		if   (err   !=   0 )   
		{
			return false;
		} 

		bool bRet = false;
		do
		{
			if   ( LOBYTE(wsaData.wVersion)   !=   2   ||   
				HIBYTE(wsaData.wVersion)   !=   0   )   
				break;
			if(!(thisHost=gethostbyname(dn_or_ip)))   
				break;
			memset((void   *)&in,sizeof(in),0);   
			in.s_addr=*((unsigned   long   *)thisHost->h_addr_list[0]);   
			if(!(ptr=inet_ntoa(in))) 
				break;
			strIP = ptr;

			bRet = true;
		}while(0);

		WSACleanup();

		return bRet;
#else
	if(!(thisHost = gethostbyname(dn_or_ip)))
		return false;
	memset((void   *)&in,sizeof(in),0);   
	in.s_addr=*((unsigned   long   *)thisHost->h_addr_list[0]);   
	if(!(ptr=inet_ntoa(in))) 
		return false;
	strIP = ptr;
	return true;
#endif
	}	
};

#endif
