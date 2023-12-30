// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
// ===========================================================================

#include "stdafx.h"
#include "scsocket.h"
#include <stdio.h>

#define MAXUDPSENDSIZE (188*345)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CSCSocket::CSCSocket()
{
	WSADATA wsad;						// Structure initialsed by WSAStartup
	WORD version = MAKEWORD(2,2);	// Version number requested

	INT32 result = WSAStartup(version, &wsad);	// Initialize Winsock
	
	if(0 != result) // WSAStartup failed
	{
        throw SCS_WSASTARTUP_FAILED;
	}

	m_hSocket = INVALID_SOCKET;
	
}


CSCSocket::~CSCSocket()
{
	if(m_hSocket != INVALID_SOCKET)
	{
		Close();
	}
	m_hSocket = INVALID_SOCKET;
	WSACleanup();
}

INT32 CSCSocket::Create()
{
	return SCS_NOT_SUPPORT;
}

INT32 CSCSocket::Close() const
{
	if(m_hSocket != INVALID_SOCKET)
	{	
		if(closesocket(m_hSocket) == SOCKET_ERROR)
		{
			return SCS_FAILED;
		}
	}
	return SCS_SUCCESS;
}

INT32 CSCSocket::Bind(const char * tcsIP, const WORD wPort) const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

	if(wPort == 0)
		return SCS_BAD_PARAMS;
	
	struct sockaddr_in sock_addr;

	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	
// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月11日 15:42:06
	if(strlen(tcsIP)==0)
	{
		sock_addr.sin_addr.s_addr =  htonl(INADDR_ANY);	
	}
	else
	{
		sock_addr.sin_addr.s_addr = inet_addr(tcsIP);
	}
	sock_addr.sin_port = htons(wPort);

	if(bind( m_hSocket, (SOCKADDR*)&sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
	{
		return SCS_FAILED;
	}

	return SCS_SUCCESS;
}


INT32 CSCSocket::SetSendTimeout(const INT32 iMilliSeconds) const
{
	if(iMilliSeconds < 0)
	{
		return SCS_BAD_PARAMS;
	}
	
	if(setsockopt( m_hSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&iMilliSeconds, sizeof(iMilliSeconds)) == SOCKET_ERROR)
	{
		return SCS_FAILED;
	}

	return SCS_SUCCESS;
}

INT32 CSCSocket::SetRecvTimeout(const INT32 iMilliSeconds) const
{
	if(iMilliSeconds < 0)
	{
		return SCS_BAD_PARAMS;
	}
	
	if(setsockopt( m_hSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&iMilliSeconds, sizeof(iMilliSeconds)) == SOCKET_ERROR)
	{
		return SCS_FAILED;
	}

	return SCS_SUCCESS;
}

WORD  CSCSocket::GetLocalIP(char * buffer)
{	
	if(m_hSocket == INVALID_SOCKET)
		return 0;
	
	struct sockaddr_in sock_addr;
	int name_len = sizeof(sock_addr);

	if(getsockname( m_hSocket, (SOCKADDR*)&sock_addr, &name_len) == SOCKET_ERROR )
	{
		return 0;
	}
	//TCHAR buffer[20] = { 0x00,
	//};
	
	sprintf(buffer, "%d.%d.%d.%d",(int)((BYTE*)&sock_addr.sin_addr.s_addr)[0],
		(int)((BYTE*)&sock_addr.sin_addr.s_addr)[1],
		(int)((BYTE*)&sock_addr.sin_addr.s_addr)[2],
		(int)((BYTE*)&sock_addr.sin_addr.s_addr)[3]);
	
	return 0;
}

WORD CSCSocket::GetLocalPort() const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;
	
	struct sockaddr_in sock_addr;
	int name_len = sizeof(sock_addr);

	if(getsockname(m_hSocket, (SOCKADDR*)&sock_addr, &name_len) == SOCKET_ERROR )
	{
		return 0;
	}
	
	return sock_addr.sin_port;
}


WORD CSCSocket::GetLocalHost(char * buffer) 
{
	if(m_hSocket == INVALID_SOCKET)
		return 0;
	
	//char str_host[512] = {0x00,};
// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月11日 16:08:52
	if(gethostname(buffer, 512) == SOCKET_ERROR)
		return 0;

	return 0;

}

extern unsigned int max_udpmsg_size;

INT32 CSCUDPSocket::Create()
{
	if((m_hSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		DWORD errorCode=WSAGetLastError();
		char strLog[MAX_PATH];
		sprintf(strLog,"UDP:Create to FAILED:errorcode: %d",errorCode);
		LogMyEvent(1,1,strLog);
        return SCS_FAILED;
	}

	/*
	int len = sizeof(m_maxSendSize);
	if (getsockopt(m_hSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char* )&m_maxSendSize, 
		&len) == 0) {

		m_maxSendSize = (m_maxSendSize / 188) * 188;

	} else 
		m_maxSendSize = MAXUDPSENDSIZE;	
	*/

	m_maxSendSize = (max_udpmsg_size / 188) * 188;;

	glog(ISvcLog::L_DEBUG, "%s:\tmaxSendSize = %d", __FUNCTION__, m_maxSendSize);

	return SCS_SUCCESS;	
}

INT32 CSCUDPSocket::BindAndOtherPara(const char *tcsIP, const WORD wPort) const
{
	int result;

	sockaddr_in M_localAddr;

	M_localAddr.sin_family=AF_INET;

	M_localAddr.sin_port = 0;

	// M_localAddr.sin_addr.S_un.S_addr = 0;
	M_localAddr.sin_addr.S_un.S_addr=inet_addr(tcsIP);
	//M_localAddr.sin_addr.
	/* m_ulIP=inet_addr(tcsIP);
	m_wPort = wPort;

	//  mreq.imr_multiaddr.s_addr = inet_addr(tcsIP);

	result=bind(m_hSocket,(PSOCKADDR)&(M_localAddr),sizeof(M_localAddr)); 

	unsigned long iMcastTTL = MULTICAST_TTL;

	result=WSAIoctl(m_hSocket,SIO_MULTICAST_SCOPE,&iMcastTTL,sizeof(iMcastTTL),NULL,0,&tempdata,NULL,NULL); 


	int bLoopback=FALSE;
	result=WSAIoctl(m_hSocket,SIO_MULTIPOINT_LOOPBACK,&bLoopback,sizeof(bLoopback),NULL,0,&tempdata,NULL,NULL); 

	*/
	struct ip_mreq mreq;

	int sock_reuse=1;
	result = setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&sock_reuse, sizeof(sock_reuse));
	if ( result == SOCKET_ERROR) 
	{
		int dd=WSAGetLastError();
		char strLog[MAX_PATH];
		sprintf(strLog,"setsockopt SOL_SOCKET, SO_REUSEADDR join error - %d",dd);
		LogMyEvent(1,1,strLog);
		return INVALID_SOCKET;
	}

	result=bind(m_hSocket,(PSOCKADDR)&(M_localAddr),sizeof(M_localAddr)); 
	if (0 != result)
	{
		int dd=WSAGetLastError();
		char strLog[MAX_PATH];
		sprintf(strLog,"bind SOL_SOCKET, PSOCKADDR)&(M_localAddr), error - %d",dd);
		LogMyEvent(1,1,strLog);
		return INVALID_SOCKET;
	}

	M_localAddr.sin_port=htons(wPort);

	// Join the multicast group

	//mreq.imr_multiaddr.s_addr = inet_addr(tcsIP);
	//mreq.imr_interface.s_addr = inet_addr(tsourceIP);	

	//result = setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
	//if (result == SOCKET_ERROR) 
	//{
	//	int dd=WSAGetLastError();
	//	char strLog[MAX_PATH];
	//	sprintf(strLog,"setsockopt IPPROTO_IP, IP_ADD_MEMBERSHIP join error - %d",dd);
	//	LogMyEvent(1,1,strLog);
	//	return SCS_FAILED;
	//} 
	////WSANOTINITIALISED       WSAENOPROTOOPT
	//unsigned long ttl = MULTICAST_TTL;

	//// Set IP TTL to traverse up to multiple routers /
	//result = setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl));
	//if (result == SOCKET_ERROR) 
	//{    
	//	int dd=WSAGetLastError();
	//	char strLog[MAX_PATH];
	//	sprintf(strLog,"setsockopt IPPROTO_IP, IP_MULTICAST_TTL join error - %d",dd);
	//	LogMyEvent(1,1,strLog);
	//	return SCS_FAILED;
	//}

	//  Disable loopback 
	//BOOL fFlag = FALSE;
	//result = setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&fFlag, sizeof(fFlag));

	//if (result == SOCKET_ERROR) 
	//{
	//	int dd=WSAGetLastError();
	//	char strLog[MAX_PATH];
	//	sprintf(strLog,"setsockopt  IPPROTO_IP, IP_MULTICAST_LOOP error - %d",dd);
	//	LogMyEvent(1,1,strLog);
	//	return SCS_FAILED;
	//}
	return SCS_SUCCESS;
}
INT32 CSCUDPSocket::Send(CHAR* pBuffer, INT32& iLength, const ULONG ulIP, const WORD wPort) const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

	struct sockaddr_in sock_addr;
	sock_addr.sin_family =      AF_INET;
    sock_addr.sin_addr.s_addr = ulIP;
    sock_addr.sin_port =     htons(wPort);

    INT32 result = 0;
    INT32 left = iLength;
	
	INT32 maxsendsize=m_maxSendSize;

	if(iLength>m_maxSendSize)
		left=m_maxSendSize;

	int leftdata=iLength;
	iLength = 0;


    while(left)
    {
        result = sendto(m_hSocket, pBuffer+iLength, left, 0, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
		 
        if(SOCKET_ERROR == result)
        {
			DWORD errorCode=WSAGetLastError();
			char strLog[MAX_PATH];
			sprintf(strLog,"UDP:SendData to server FAILED:errorcode: %d",errorCode);
			LogMyEvent(1,1,strLog);
            return SCS_FAILED;
        }

		leftdata -= result;
		if(leftdata >m_maxSendSize)
			left=m_maxSendSize;
		else
			left=leftdata;
		iLength += result;
    }

    return SCS_SUCCESS;

}


INT32 CSCUDPSocket::Send(CHAR* pBuffer, INT32& iLength, const char* csHost, const WORD wPort) const
{
	static TCHAR buffered_host[256] = { 0x00,
	};
	static ULONG buffered_ip = 0;


	if(strcmp(csHost,buffered_host)!=0)
	{
		sprintf(buffered_host, "%s", csHost);
		buffered_ip = inet_addr(buffered_host);
	}
	
	return Send(pBuffer, iLength, buffered_ip, wPort);
}


INT32 CSCUDPSocket::Receive(CHAR* pBuffer, INT32& iLength, char* csHost, WORD& wPort) const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

	struct sockaddr_in sock_addr = { 0x00, };
	int name_len = sizeof(sock_addr);

    INT32 result = 0;
    INT32 left = iLength;
	iLength = 0;

    while(left)
    {
        result = recvfrom(m_hSocket, pBuffer+iLength, left, 0, (struct sockaddr*)&sock_addr, &name_len);

        if(SOCKET_ERROR == result)
        {
			csHost[0] ='\0' ;
			wPort = 0;
            return SCS_FAILED;
        }

        left -= result;
		iLength += result;
    }

	TCHAR buffer[20] = { 0x00,
	};
	
	sprintf(buffer, "%d.%d.%d.%d",(int)((BYTE*)&sock_addr.sin_addr.s_addr)[0],
		(int)((BYTE*)&sock_addr.sin_addr.s_addr)[1],
		(int)((BYTE*)&sock_addr.sin_addr.s_addr)[2],
		(int)((BYTE*)&sock_addr.sin_addr.s_addr)[3]);

	csHost = buffer;
	wPort = sock_addr.sin_port;
    return SCS_SUCCESS;

}


INT32 CSCTCPSocket::Create()
{
	if((m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
        return SCS_FAILED;
	}

	return SCS_SUCCESS;	
}

INT32 CSCTCPSocket::Listen(const INT32 iQueuedConnections) const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

	if( iQueuedConnections == 0 )
		return SCS_BAD_PARAMS;

	if(listen( m_hSocket, iQueuedConnections) == SOCKET_ERROR)
	{
        return SCS_FAILED;
	}

	return SCS_SUCCESS;
}

CSCTCPSocket* CSCTCPSocket::Accept() const
{
	if(m_hSocket == INVALID_SOCKET)
		return NULL;

	struct sockaddr_in sock_addr;
	int len = sizeof(sock_addr);
	SOCKET socket = INVALID_SOCKET;

	memset(&sock_addr, 0, sizeof(sock_addr));
	socket = accept(m_hSocket, (SOCKADDR*)&sock_addr, &len);

	if(INVALID_SOCKET == socket)
	{
		return NULL;
	}
	return new CSCTCPSocket(socket);
}

INT32 CSCTCPSocket::Connect(CHAR* tcsRemote, const WORD wPort) const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

	if(strlen(tcsRemote)==0 || wPort == 0)
		return SCS_BAD_PARAMS;

	hostent *host_ent = NULL;
	long ip_address = 0;
	struct sockaddr_in sock_addr;

	host_ent = gethostbyname(tcsRemote);

	if(host_ent!= NULL)
	{
		ip_address = ((in_addr*)host_ent->h_addr)->s_addr;
		sock_addr.sin_addr.s_addr = ip_address;
	}
	else
	{
		sock_addr.sin_addr.s_addr = inet_addr(tcsRemote);
	}

	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(wPort);

	if(connect(m_hSocket, (SOCKADDR*)&sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
	{
        return SCS_FAILED;
	}

	return SCS_SUCCESS;
}

INT32 CSCTCPSocket::AsyncSelect(HWND hWnd,DWORD wMsg, LONG ulEvent)
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

	if( !IsWindow( hWnd ) || wMsg == 0 || ulEvent == 0 )
        return SCS_BAD_PARAMS;

	if( WSAAsyncSelect( m_hSocket, hWnd, wMsg, ulEvent ) == SOCKET_ERROR )
	{
        return SCS_FAILED;
	}
	return SCS_SUCCESS;
}
/*
CString CSCTCPSocket::GetRemoteIP() const
{
	if(m_hSocket == INVALID_SOCKET)
		return _T("");

	struct sockaddr_in sock_addr;
	int name_len = sizeof(sock_addr);

	if(getpeername(m_hSocket, (SOCKADDR*)&sock_addr, &name_len) == SOCKET_ERROR)
	{
		return _T("");
	}

	TCHAR buffer[20] = { 0x00,
	};
	
	_stprintf(buffer, "%d.%d.%d.%d",(int)((BYTE*)&sock_addr.sin_addr.s_addr)[0],
		(int)((BYTE*)&sock_addr.sin_addr.s_addr)[1],
		(int)((BYTE*)&sock_addr.sin_addr.s_addr)[2],
		(int)((BYTE*)&sock_addr.sin_addr.s_addr)[3]);


	return CString(buffer);
}
WORD CSCTCPSocket::GetRemotePort() const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

	struct sockaddr_in sock_addr;
	int name_len = sizeof(sock_addr);

	if(getpeername(m_hSocket, (SOCKADDR*)&sock_addr, &name_len) == SOCKET_ERROR)
	{
		return 0;
	}

	return sock_addr.sin_port;
}

CString CSCTCPSocket::GetRemoteHost() const
{
	if(m_hSocket == INVALID_SOCKET)
		return _T("");

	hostent* host_ent = NULL;
	struct sockaddr_in sock_addr;
	int name_len = sizeof( sock_addr );

	if(getpeername(m_hSocket, (SOCKADDR*)&sock_addr, &name_len) == SOCKET_ERROR)
	{
		return _T("");
	}

	host_ent = gethostbyaddr( (char*)&sock_addr.sin_addr.s_addr, 4 ,PF_INET );

	if( host_ent != NULL )
	{
		return CString(host_ent->h_name);
	}

	return _T("");
	}*/


INT32 CSCTCPSocket::Send(CHAR* pBuffer, INT32& iLength) const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

    INT32 result = 0;
    INT32 left = iLength;
	iLength = 0;

    while(left)
    {
        result = send(m_hSocket, pBuffer+iLength, left, 0);

        if(SOCKET_ERROR == result)
        {
            return SCS_FAILED;
        }

        left -= result;
		iLength += result;
    }

    return SCS_SUCCESS;
}

INT32 CSCTCPSocket::Receive(CHAR* pBuffer, INT32& iLength) const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

    INT32 result = 0;
    INT32 left = iLength;
	iLength = 0;

    while(left)
    {
        result = recv(m_hSocket, pBuffer+iLength, left, 0);

        if(SOCKET_ERROR == result)
        {
            return SCS_FAILED;
        }

        left -= result;
		iLength += result;
    }

    return SCS_SUCCESS;
}

INT32 CSCTCPSocket::ReceiveData(CHAR* pBuffer, INT32& iLength) const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

        iLength = recv(m_hSocket, pBuffer, iLength, 0);

        if(SOCKET_ERROR == iLength)
        {
            return SCS_FAILED;
        }

    return SCS_SUCCESS;
}


INT32 CSCMulticastSocket::Create()
{

//	m_hSocket=WSASocket(AF_INET,SOCK_DGRAM,IPPROTO_UDP,NULL,0,
//WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF); 
//	strcpy(m_csourceip,"192.8.8.8");

    if((m_hSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
        return SCS_FAILED;
	}
	
	if(m_hSocket == INVALID_SOCKET)
	{
		return SCS_FAILED;
	}

	return SCS_SUCCESS;	
}
INT32 CSCMulticastSocket::Join(const char * tcsIP, const WORD wPort, WORD wSourcePort,const char * tsourceIP)
//INT32 CSCMulticastSocket::Join(const char * tcsIP, const WORD wPort, WORD wSourcePort)
{
	#define MULTICAST_TTL	(0x40)
	
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

	int result;
//	DWORD tempdata;


 sockaddr_in M_localAddr;

 M_localAddr.sin_family=AF_INET;

 M_localAddr.sin_port=wPort;

// M_localAddr.sin_addr.S_un.S_addr = 0;
 M_localAddr.sin_addr.S_un.S_addr=inet_addr(tsourceIP);
//M_localAddr.sin_addr.
/* m_ulIP=inet_addr(tcsIP);
 	m_wPort = wPort;

 //  mreq.imr_multiaddr.s_addr = inet_addr(tcsIP);

result=bind(m_hSocket,(PSOCKADDR)&(M_localAddr),sizeof(M_localAddr)); 

  unsigned long iMcastTTL = MULTICAST_TTL;
  
  result=WSAIoctl(m_hSocket,SIO_MULTICAST_SCOPE,&iMcastTTL,sizeof(iMcastTTL),NULL,0,&tempdata,NULL,NULL); 


  int bLoopback=FALSE;
  result=WSAIoctl(m_hSocket,SIO_MULTIPOINT_LOOPBACK,&bLoopback,sizeof(bLoopback),NULL,0,&tempdata,NULL,NULL); 

*/

	result=bind(m_hSocket,(PSOCKADDR)&(M_localAddr),sizeof(M_localAddr)); 
	if (0 != result)
	{
		int dd=WSAGetLastError();
		char strLog[MAX_PATH];
		sprintf(strLog,"bind SOL_SOCKET, PSOCKADDR)&(M_localAddr), error - %d",dd);
		LogMyEvent(1,1,strLog);
		return INVALID_SOCKET;
	}

		// Join the multicast group
    struct ip_mreq mreq;

	int sock_reuse=1;
	result = setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&sock_reuse, sizeof(sock_reuse));
	if ( result == SOCKET_ERROR) 
	{
		int dd=WSAGetLastError();
		char strLog[MAX_PATH];
		sprintf(strLog,"setsockopt SOL_SOCKET, SO_REUSEADDR join error - %d",dd);
		LogMyEvent(1,1,strLog);
		return INVALID_SOCKET;
	}

    mreq.imr_multiaddr.s_addr = inet_addr(tcsIP);
    mreq.imr_interface.s_addr = inet_addr(tsourceIP);	

    result = setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
    if (result == SOCKET_ERROR) 
    {
		int dd=WSAGetLastError();
		char strLog[MAX_PATH];
		sprintf(strLog,"setsockopt IPPROTO_IP, IP_ADD_MEMBERSHIP join error - %d",dd);
		LogMyEvent(1,1,strLog);
        return SCS_FAILED;
    } 
//WSANOTINITIALISED       WSAENOPROTOOPT
    unsigned long ttl = MULTICAST_TTL;

    // Set IP TTL to traverse up to multiple routers /
    result = setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl));
    if (result == SOCKET_ERROR) 
    {    
		int dd=WSAGetLastError();
		char strLog[MAX_PATH];
		sprintf(strLog,"setsockopt IPPROTO_IP, IP_MULTICAST_TTL join error - %d",dd);
		LogMyEvent(1,1,strLog);
        return SCS_FAILED;
    }

   //  Disable loopback 
    BOOL fFlag = FALSE;
    result = setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&fFlag, sizeof(fFlag));

    if (result == SOCKET_ERROR) 
    {
		int dd=WSAGetLastError();
		char strLog[MAX_PATH];
		sprintf(strLog,"setsockopt  IPPROTO_IP, IP_MULTICAST_LOOP join error - %d",dd);
		LogMyEvent(1,1,strLog);
        return SCS_FAILED;
    }
	
	m_ulIP = mreq.imr_multiaddr.s_addr;
	m_wPort = wPort;
	m_wSourcePort=wSourcePort;

	return SCS_SUCCESS;
}

INT32 CSCMulticastSocket::Leave()
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

	if(m_ulIP == 0 || m_wPort == 0)
		return SCS_FAILED;

	// Leave the multicast group
    struct ip_mreq mreq;
	int result;

    mreq.imr_multiaddr.s_addr = m_ulIP;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);	

    result = setsockopt(m_hSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
    if (result == SOCKET_ERROR) 
    {
        return SCS_FAILED;
    } 
	
	return SCS_SUCCESS;
}

INT32 CSCMulticastSocket::Send(CHAR* pBuffer, INT32& iLength) const
{

	struct sockaddr_in sock_addr;
	sock_addr.sin_family =      AF_INET;
    sock_addr.sin_addr.s_addr = m_ulIP;
    sock_addr.sin_port =     htons(m_wPort);


	INT32 result = 0;
	INT32 left = iLength;

	INT32 maxsendsize=MAXUDPSENDSIZE;

	if(iLength>MAXUDPSENDSIZE)
		left=MAXUDPSENDSIZE;

	int leftdata=iLength;
	iLength = 0;


	while(left)
	{
		result = sendto(m_hSocket, pBuffer+iLength, left, 0,  (struct sockaddr*)&sock_addr, sizeof(sock_addr));
		
		if(SOCKET_ERROR == result)
		{
			int dd=WSAGetLastError();
			return SCS_FAILED;
		}

		leftdata -= result;
		if(leftdata >MAXUDPSENDSIZE)
			left=MAXUDPSENDSIZE;
		else
			left=leftdata;
		iLength += result;
	}
//	if(m_ulIP == 0 || m_wPort == 0)
//		return SCS_FAILED;
//	return CSCUDPSocket::Send(pBuffer, iLength, m_ulIP, m_wPort);

	return SCS_SUCCESS;

}

