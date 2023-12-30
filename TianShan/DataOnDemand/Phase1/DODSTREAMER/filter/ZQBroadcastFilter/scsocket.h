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

#if !defined(AFX_SCSOCKET_H__13F24F9B_B041_4863_A323_B1789900F0E5__INCLUDED_)
#define AFX_SCSOCKET_H__13F24F9B_B041_4863_A323_B1789900F0E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define SCS_SUCCESS           (0x00)
#define SCS_FAILED            (0x01)
#define SCS_WSASTARTUP_FAILED (0x02)
#define SCS_BAD_PARAMS        (0x03)
#define SCS_NOT_SUPPORT       (0x04)



#include <winsock2.h>
//#include <winsock.h>
#include <WS2tcpip.h>
//define IP 
typedef struct{
	char	cAddr[30];
	WORD	wPort;
	TCHAR	szLocalIP[30];
}struIP;

// global function for writing log to Windows Event Logger, from Rose.LU
extern void LogMyEvent(int errorLevel,int errorcode,LPCTSTR errorStr);
/////////////////////////////////////////////////////////////////

// CSCSocket
class CSCSocket  
{
public:
	/// Ctor
	/// Throw SCS_WSASTARTUP_FAILED if WSAStartup failed.
	CSCSocket();

	/// Dtor
	virtual ~CSCSocket();

	/// Create socket, pure virtual method, sub class must implement it.
	/// @return SCS_NOT_SUPPORT.
	virtual INT32 Create() = 0;

	/// Close socket.
	/// @return SCS_FAILED if failed, otherwise return SCS_SUCCESS.
	INT32 Close() const;

	/// Bind to an IP address.
	/// @param tcsIP, bind IP address.
	/// @param wPort, bind IP port.
	/// @return SCS_FAILED if failed, return SCS_BAD_PARAMS if parameters
	/// are invalid, otherwise return SCS_SUCCESS.
	INT32 Bind(const char *tcsIP, const WORD wPort) const;

	/// Set send timeout time.
	/// @param iMilliSeconds milliseconds of timeout time.
	/// @return SCS_FAILED if failed, return SCS_BAD_PARAMS if parameters
	/// are invalid, otherwise return SCS_SUCCESS.
	INT32 SetSendTimeout(const INT32 iMilliSeconds) const;

	/// Set receive timeout time.
	/// @param iMilliSeconds milliseconds of timeout time.
	/// @return SCS_FAILED if failed, return SCS_BAD_PARAMS if parameters
	/// are invalid, otherwise return SCS_SUCCESS.
	INT32 SetRecvTimeout(const INT32 iMilliSeconds) const;

	/// Get socket local IP address.
	/// @return socket local IP address.
	WORD  GetLocalIP(char * buffer);

	/// Get socket local IP port.
	/// @return socket local IP port.
	WORD    GetLocalPort() const;

	/// Get socket local hostname.
	/// @return socket local hostname.
	WORD GetLocalHost(char * buffer);

protected:
	SOCKET m_hSocket;
public:
	TCHAR  m_szIPWatched[20];
	DWORD  m_tmLastUpdate;
};


// CSCTCPSocket, implements TCP protocol.
class CSCTCPSocket : public CSCSocket
{
public:	
	/// Ctor
	CSCTCPSocket() : CSCSocket()
	{
	}

	/// Ctor
	CSCTCPSocket(SOCKET socket) : CSCSocket()
	{		
		m_hSocket = socket;
	}

	/// Create tcp socket
	/// @return SCS_FAILED if failed, otherwise return SCS_SUCCESS.
	virtual INT32 Create();

	/// Listen for connections.
	/// @param iQueueConnections how many connections supported.
	/// @return SCS_FAILED if failed, return SCS_BAD_PARAMS if parameters
	/// are invalid, otherwise return SCS_SUCCESS.
	INT32 Listen(const INT32 iQueuedConnections) const;

	/// Accept a connection
	/// @return NULL if failed to accept a connection, otherwise return a new
	/// created CSCTCPSocket pointer.
	CSCTCPSocket* Accept() const;

	/// Connects the socket to a remote site.
	/// @param tcsRemote remote IP address.
	/// @param wPort remote IP port.
	/// @return SCS_FAILED if failed, return SCS_BAD_PARAMS if parameters
	/// are invalid, otherwise return SCS_SUCCESS.	
	INT32 Connect(CHAR* tcsRemote, const WORD wPort) const;

	/// Allows calling window to receive notifications (non-blocking socket).
	/// @param hWnd, calling window HWND.
	/// @param wMsg, message to receive.
	/// @param ulEvent, event to receive.
	/// @return SCS_FAILED if failed, return SCS_BAD_PARAMS if parameters
	/// are invalid, otherwise return SCS_SUCCESS.	
	INT32 AsyncSelect(HWND hWnd,DWORD wMsg, LONG ulEvent);

	/// Send data.
	/// @param pBuffer bytes need to send.
	/// @param ilength how many bytes need to send and have sent.
	/// @return SCS_FAILED if failed, otherwise return SCS_SUCCESS.
	INT32 Send(CHAR* pBuffer, INT32& iLength) const;

	/// Receives data
	/// @param pBuffer bytes need to receive.
	/// @param ilength how many bytes need to receive and have received.
	/// @return SCS_FAILED if failed, otherwise return SCS_SUCCESS.
	INT32 Receive(CHAR* pBuffer, INT32& iLength) const;

	/// Receives data
	/// @param [out] pBuffer bytes need to receive.
	/// @param [in,out] ilength how many bytes have received, please guarantee iLength <= the length of pBuffer.
	/// @return SCS_FAILED if failed, otherwise return SCS_SUCCESS.
	INT32 ReceiveData(CHAR* pBuffer, INT32& iLength) const;
/*
	/// Get remote side IP.
	/// @return remote IP address.
	virtual CString GetRemoteIP() const;

	/// Get remote side Port number.
	/// @return remote IP port.
	virtual WORD    GetRemotePort() const;

	/// Get remote host name.
	/// @return remote host name.
	virtual CString GetRemoteHost() const;*/

};
class CSCUDPSocket : public CSCSocket
{
public:
	/// Create udp socket
	/// @return SCS_FAILED if failed, otherwise return SCS_SUCCESS.
	virtual INT32 Create();
	INT32 BindAndOtherPara(const char *tcsIP, const WORD wPort) const;
	/// @param pBuffer bytes need to send.
	/// @param ilength how many bytes need to send and have sent.
	/// @param ulIP remote IP address.
	/// @param wPort remtoe IP port.
	/// @return SCS_FAILED if failed, otherwise return SCS_SUCCESS.
	INT32 Send(CHAR* pBuffer, INT32& iLength, const ULONG ulIP,const WORD wPort) const;

	/// @param pBuffer bytes need to send.
	/// @param ilength how many bytes need to send and have sent.
	/// @param ulIP remote IP address.
	/// @param wPort remtoe IP port.
	/// @return SCS_FAILED if failed, otherwise return SCS_SUCCESS.
	INT32 Send(CHAR* pBuffer, INT32& iLength, const char * csHost, const WORD wPort) const;

	/// Receives data
	/// @param pBuffer bytes need to receive.
	/// @param ilength how many bytes need to receive and have received.
	/// @param csHost remote host IP address have received.
	/// @param wPort remote host IP port have received.
	/// @return SCS_FAILED if failed, otherwise return SCS_SUCCESS.
	INT32 Receive(CHAR* pBuffer, INT32& iLength, char * csHost, WORD& wPort) const;

protected:
	unsigned int		m_maxSendSize;
};


class CSCMulticastSocket : public CSCUDPSocket
{
public:

	/// Ctor
	CSCMulticastSocket() : CSCUDPSocket()
	{
		m_ulIP = 0;
		m_wPort = 0;
	}

	virtual INT32 Create();

	/// Join a multicast group.
	/// @param tcsIP multicast IP address.
	/// @param wPort multicast IP port.
	/// @return SCS_FAILED if failed, otherwise return SCS_SUCCESS.
	//INT32 Join(const char * tcsIP, WORD wPort, WORD wSourcePort);
	INT32 Join(const char * tcsIP, const WORD wPort, WORD wSourcePort,const char * tsourceIP);

	/// Leave a multicast group.
	/// @return SCS_FAILED if failed, otherwise return SCS_SUCCESS.
	INT32 Leave();

	/// Send data.
	/// @param pBuffer bytes need to send.
	/// @param ilength how many bytes need to send and have sent.
	/// @return SCS_FAILED if failed, otherwise return SCS_SUCCESS.
	INT32 Send(CHAR* pBuffer, INT32& iLength) const;

	/// Receives data
	/// @param pBuffer bytes need to receive.
	/// @param ilength how many bytes need to receive and have received.
	/// @return SCS_FAILED if failed, otherwise return SCS_SUCCESS.
//	INT32 Receive(CHAR* pBuffer, INT32& iLength) const;
private:
	ULONG m_ulIP;
	WORD m_wPort;
	char m_csourceip[16];
	WORD m_wSourcePort;
};
#endif // !defined(AFX_SCSOCKET_H__13F24F9B_B041_4863_A323_B1789900F0E5__INCLUDED_)
