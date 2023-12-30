// ===========================================================================
// Copyright (c) 1997, 1998 by
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
//
// Ident : $Id: Socket.cpp,v 1.1 2005-05-17 15:30:26 Ouyang Exp $
// Branch: $Name:  $
// Author: Lin Ouyang
// Desc  : the implementation of Socket class
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/NoiseSim/Socket.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 2     05-05-19 12:09 Lin.ouyang
// by: lorenzo(lin ouyang)
// add comment for doxgen
// 
// 1     05-05-17 15:48 Lin.ouyang
// init version
// 
// Revision 1.1  2005-05-17 15:30:26  Ouyang
// initial created
// ===========================================================================


/// @file "Socket.cpp"
/// @brief the implementation file for Socket class
/// @author (Lorenzo) Lin Ouyang
/// @date 2005-5-19 9:52
/// @version 0.1.0

//
// Socket.cpp: implementation of the Socket class.
//
//////////////////////////////////////////////////////////////////////

#include "Socket.h"


// -----------------------------
// class init_WSA
// -----------------------------
init_WSA::init_WSA()
{
	
};

init_WSA::~init_WSA() 
{ 
	WSACleanup(); 
} 

bool init_WSA::init()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int iError;
	
	wVersionRequested = MAKEWORD( 2, 2 );
	
	iError = WSAStartup( wVersionRequested, &wsaData );
	if ( iError != 0 ) 
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return false;
	}
	
	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */
	
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ) 
	{
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		WSACleanup( );
		return false; 
	}
	
	/* The WinSock DLL is acceptable. Proceed. */
	return true;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Socket::Socket()
{
	m_iSocket = INVALID_SOCKET;			// Socket
	InitSocket();
}

Socket::Socket(int iDomain, int iSockType, int iProtocol)
{
	InitSocket();
	setDomain(iDomain);
	setSockType(iSockType);
	setProtocol(iProtocol);
}

Socket::~Socket()
{
	if(isActive() == true)
		Close();
}

int Socket::getSocket() const
{
	return m_iSocket;
}

void Socket::setDomain(int iDomain)
{
	m_iDomain = iDomain;
	m_addrLocal.sin_family = m_iDomain;
	m_addrRemote.sin_family = m_iDomain;
}

void Socket::setSockType(int iSockType)
{
	m_iSockType = iSockType;
}

void Socket::setProtocol(int iProtocol)
{
	m_iProtocol = iProtocol;
}

void Socket::InitSocket()
{
	memset(&m_addrLocal, 0, sizeof(m_addrLocal));
	memset(&m_addrRemote, 0, sizeof(m_addrRemote));
	setDomain(AF_INET);				// Address Family
	setSockType(SOCK_STREAM);			// Socket Type
	setProtocol(0);					// Protocol
}

bool Socket::isActive()
{
	if(m_iSocket == INVALID_SOCKET)
		return false;
	else
		return true;
}

bool Socket::SocketCreate(int iDomain, int iSockType, int iProtocol)
{
	setDomain(iDomain);
	setSockType(iSockType);
	setProtocol(iProtocol);

	return SocketCreate();
}

bool Socket::SocketCreate()
{
	m_iSocket = socket(m_iDomain, m_iSockType, m_iProtocol);

	return isActive();
}


bool Socket::Bind(string strIP, int iPort)
{
	m_addrLocal.sin_family = m_iDomain;
	setLocalAddr(strIP);
	setLocalPort(iPort);

	return Bind();
}

bool Socket::Listen(int iBacklog)
{
	int iRet;

	iRet = listen(m_iSocket, iBacklog);
	if(iRet == SOCKET_ERROR)
		return false;
	else
		return true;
}

Socket Socket::Accept()
{
	int iSocketNew, iFromlen;
	Socket sockNew = *this;

	iFromlen = sizeof(m_addrRemote);
	iSocketNew = accept(m_iSocket, (sockaddr *)&m_addrRemote, &iFromlen);
	sockNew.setSocket(iSocketNew);

	return sockNew;
}

void Socket::PrintSocket()
{
	cout << "Socket infomation" << endl;
	cout << "Socket         : " << m_iSocket << endl;
	cout << "Socket domain  : " << m_iDomain << endl;
	cout << "Socket type    : " << m_iSockType << endl;
	cout << "Socket protocol: " << m_iProtocol << endl;
	cout << "Local address  : " << getLocalAddr() << endl;
	cout << "Local port     : " << getLocalPort() << endl;
	cout << "Remote address : " << getRemoteAddr() << endl;
	cout << "Remote port    : " << getRemotePort() << endl;
}

void Socket::setSocket(int iSocket)
{
	m_iSocket = iSocket;
}

string Socket::getLocalAddr()
{
	return inet_ntoa(m_addrLocal.sin_addr);
}

bool Socket::setLocalAddr(string strIP)
{
	m_addrLocal.sin_addr.s_addr = inet_addr(strIP.c_str());
	return true;
}

bool Socket::setLocalPort(int iPort)
{
	m_addrLocal.sin_port = htons(iPort);
	return true;
}

int Socket::getLocalPort()
{
	return ntohs(m_addrLocal.sin_port);
}

string Socket::getRemoteAddr()
{
	return inet_ntoa(m_addrRemote.sin_addr);
}

bool Socket::setRemoteAddr(string strIP)
{
	m_addrRemote.sin_addr.s_addr = inet_addr(strIP.c_str());
	return true;
}

bool Socket::setRemotePort(int iPort)
{
	m_addrRemote.sin_port = htons(iPort);
	return true;
}

int Socket::getRemotePort()
{
	return ntohs(m_addrRemote.sin_port);
}

bool Socket::Close()
{
	if(closesocket(m_iSocket) == SOCKET_ERROR)
		return false;
	else
		return true;
}

bool Socket::Connect()
{
	int iRet;

	iRet = connect(m_iSocket, (sockaddr *)&m_addrRemote, sizeof(m_addrRemote));
	if(iRet == SOCKET_ERROR)
		return false;
	else
		return true;
}

bool Socket::Connect(string strIP, int iPort)
{
	setRemoteAddr(strIP);
	setRemotePort(iPort);
	return Connect();
}

// get local address for a connected socket
bool Socket::GetSockname()
{
	int iRet, iNamelen;
	
	iNamelen = sizeof(m_addrLocal);
	iRet = getsockname(m_iSocket, (sockaddr *)&m_addrLocal, &iNamelen);
	if(iRet == SOCKET_ERROR)
		return false;
	else
		return true;
}

int Socket::Recv(char *szBuff, int iLen, int iFlags)
{
	return recv(m_iSocket, szBuff, iLen, iFlags);
}

int Socket::RecvFrom(char *szBuff, int iLen, int iFlags)
{
	int iFromlen;

	iFromlen = sizeof(m_addrRemote);
	return recvfrom(m_iSocket, szBuff, iLen, iFlags, (sockaddr *)&m_addrRemote, &iFromlen);
}

int Socket::Send(char *szBuff, int iLen, int iFlags)
{
	return send(m_iSocket, szBuff, iLen, iFlags);
}

int Socket::SendTo(char *szBuff, int iLen, int iFlags)
{
	int iTolen;

	iTolen = sizeof(m_addrRemote);
	return sendto(m_iSocket, szBuff, iLen, iFlags, (sockaddr *)&m_addrRemote, iTolen);
}

int Socket::SendTo(char *szBuff, int iLen, string strIP, int iPort, int iFlags)
{
	setRemoteAddr(strIP);
	setRemotePort(iPort);
	return SendTo(szBuff, iLen, iFlags);
}

bool Socket::Bind()
{
	int iRet;

	iRet = bind(m_iSocket, (sockaddr *)&m_addrLocal, sizeof(m_addrLocal));
	if(iRet == SOCKET_ERROR)
		return false;
	else
		return true;
}
