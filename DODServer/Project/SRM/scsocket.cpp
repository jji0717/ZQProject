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

// simin add
// To use this, make certain that your STDAFX.H has the following:
////
//#include <afxsock.h>		// MFC socket extensions
//#include <atlbase.h>
//#include <afxoledb.h>
//#include <atlplus.h>
////
//2004/11/12 Add ReceiveData to receive any data without waiting for wanted length

// ===========================================================================

#include "stdafx.h"
#include "scsocket.h"

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

INT32 CSCSocket::Bind(const CString tcsIP, const WORD wPort) const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

	if(wPort == 0)
		return SCS_BAD_PARAMS;
	
	struct sockaddr_in sock_addr;

	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	if(tcsIP == _T(""))
	{
		sock_addr.sin_addr.s_addr = INADDR_ANY;
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

CString CSCSocket::GetLocalIP() const
{
	if(m_hSocket == INVALID_SOCKET)
		return _T("");
	
	struct sockaddr_in sock_addr;
	int name_len = sizeof(sock_addr);

	if(getsockname( m_hSocket, (SOCKADDR*)&sock_addr, &name_len) == SOCKET_ERROR )
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


CString CSCSocket::GetLocalHost() const
{
	if(m_hSocket == INVALID_SOCKET)
		return _T("");
	
	char str_host[512] = {0x00,};

	if(gethostname(str_host, 512) == SOCKET_ERROR)
		return _T("");

	return CString(str_host);

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

INT32 CSCTCPSocket::Connect(const CString tcsRemote, const WORD wPort) const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

	if(tcsRemote == _T("") || wPort == 0)
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
}

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
			int Ret=::WSAGetLastError();
			Clog( LOG_DEBUG, "Send Socket Error Code:%d", Ret);
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
			int Ret=::WSAGetLastError();
			Clog( LOG_DEBUG, "Receive Socket Error Code:%d", Ret);
			return SCS_FAILED;
        }

        left -= result;
		iLength += result;
    }

    return SCS_SUCCESS;
}

//2004/11/12 Add receive any data without waiting for length
INT32 CSCTCPSocket::ReceiveData(CHAR* pBuffer, INT32& iLength) const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

		//TRACE("\n ReceiveData гнгнгнгнгнгнгнгнгнгн--length=%d\n",iLength);
        iLength = recv(m_hSocket, pBuffer, iLength, 0);

        if(SOCKET_ERROR == iLength)
        {
			//TRACE("\n -----------Failed\n",iLength);
			int Ret=::WSAGetLastError();
			Clog( LOG_DEBUG, "Receive Socket Error Code:%d", Ret);
            return SCS_FAILED;
        }

    return SCS_SUCCESS;
}


INT32 CSCUDPSocket::Create()
{
	if((m_hSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
        return SCS_FAILED;
	}

	return SCS_SUCCESS;	
}

INT32 CSCUDPSocket::Send(CHAR* pBuffer, INT32& iLength, const ULONG ulIP, const WORD wPort) const
{
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

	struct sockaddr_in sock_addr;
	sock_addr.sin_family =      AF_INET;
    sock_addr.sin_addr.s_addr = ulIP;
    sock_addr.sin_port =        htons(wPort);

    INT32 result = 0;
    INT32 left = iLength;
	iLength = 0;

    while(left)
    {
        result = sendto(m_hSocket, pBuffer+iLength, left, 0, (struct sockaddr*)&sock_addr, sizeof(sock_addr));

        if(SOCKET_ERROR == result)
        {
            return SCS_FAILED;
        }

        left -= result;
		iLength += result;
    }

    return SCS_SUCCESS;

}


INT32 CSCUDPSocket::Send(CHAR* pBuffer, INT32& iLength, const CString csHost, const WORD wPort) const
{
	static TCHAR buffered_host[256] = { 0x00,
	};
	static ULONG buffered_ip = 0;


	if(csHost.Compare(buffered_host)!=0)
	{
		_stprintf(buffered_host, _T("%s"), (LPCTSTR)csHost);
		buffered_ip = inet_addr(buffered_host);
	}
	
	return Send(pBuffer, iLength, buffered_ip, wPort);
}


INT32 CSCUDPSocket::Receive(CHAR* pBuffer, INT32& iLength, CString& csHost, WORD& wPort) const
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
			csHost = _T("");
			wPort = 0;
            return SCS_FAILED;
        }

        left -= result;
		iLength += result;
    }

	TCHAR buffer[20] = { 0x00,
	};
	
	_stprintf(buffer, "%d.%d.%d.%d",(int)((BYTE*)&sock_addr.sin_addr.s_addr)[0],
		(int)((BYTE*)&sock_addr.sin_addr.s_addr)[1],
		(int)((BYTE*)&sock_addr.sin_addr.s_addr)[2],
		(int)((BYTE*)&sock_addr.sin_addr.s_addr)[3]);

	csHost = buffer;
	wPort = sock_addr.sin_port;
    return SCS_SUCCESS;

}

/*INT32 CSCMulticastSocket::Join(const CString tcsIP, const WORD wPort)
{
	#define MULTICAST_TTL	(0x40)
	
	if(m_hSocket == INVALID_SOCKET)
		return SCS_FAILED;

	// Join the multicast group
    struct ip_mreq mreq;
	int result;

    mreq.imr_multiaddr.s_addr = inet_addr(tcsIP);
    mreq.imr_interface.s_addr = INADDR_ANY;	

    result = setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
    if (result == SOCKET_ERROR) 
    {
        return SCS_FAILED;
    } 

    unsigned long ttl = MULTICAST_TTL;

    //* Set IP TTL to traverse up to multiple routers 
    result = setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl));
    if (result == SOCKET_ERROR) 
    {    
        return SCS_FAILED;
    }

    //* Disable loopback 
    BOOL fFlag = FALSE;
    result = setsockopt(m_hSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&fFlag, sizeof(fFlag));

    if (result == SOCKET_ERROR) 
    {
        return SCS_FAILED;
    }
	
	m_ulIP = mreq.imr_multiaddr.s_addr;
	m_wPort = wPort;
	
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
    mreq.imr_interface.s_addr = INADDR_ANY;	

    result = setsockopt(m_hSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
    if (result == SOCKET_ERROR) 
    {
        return SCS_FAILED;
    } 
	
	return SCS_SUCCESS;
}*/

INT32 CSCMulticastSocket::Send(CHAR* pBuffer, INT32& iLength) const
{
	if(m_ulIP == 0 || m_wPort == 0)
		return SCS_FAILED;
	return CSCUDPSocket::Send(pBuffer, iLength, m_ulIP, m_wPort);
}

INT32 CSCMulticastSocket::Receive(CHAR* pBuffer, INT32& iLength) const
{
	CString ip;
	WORD port;
	return CSCUDPSocket::Receive(pBuffer, iLength, ip, port);
}
