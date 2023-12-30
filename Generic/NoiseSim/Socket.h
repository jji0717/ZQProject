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
// Ident : $Id: Socket.h,v 1.1 2005-05-17 15:30:26 Ouyang Exp $
// Branch: $Name:  $
// Author: Lin Ouyang
// Desc  : define Socket class
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/NoiseSim/Socket.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 3     05-07-06 19:56 Lin.ouyang
// add multicast support
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
//
//////////////////////////////////////////////////////////////////////


/// @file "Socket.h"
/// @brief the header file for Socket class
/// @author (Lorenzo) Lin Ouyang
/// @date 2005-5-19 9:52
/// @version 0.1.0
#if !defined(AFX_Socket_H__AA6DCE3F_C731_46F4_ACD6_9E5028FE18FC__INCLUDED_)
#define AFX_Socket_H__AA6DCE3F_C731_46F4_ACD6_9E5028FE18FC__INCLUDED_


#include <Winsock2.h>
#include <string>
#include <iostream>

using namespace std;

//#define INVALID_SOCKET -1

/// @class Socket "Socket.h"
/// @brief Socket class
///
/// Encapsulate socket
class Socket  
{
public:
	/// @fn bool Bind()
	/// @brief Bind socket
	/// @return true/false
	/// @retval true Bind succeed
	/// @retval false Bind failur
	virtual bool Bind();
	
	/// @fn int SendTo(char *szBuff, int iLen, string strIP, int iPort, int iFlags = NULL)
	/// @brief Send message
	/// @param[in,out] szBuff null terminated C string
	/// @param[in] iLen length of szBuff
	/// @param[in] strIP Destination IP address, only support decimal dotted format now
	/// @param[in] iPort Destination port
	/// @param[in] iFlags Flags
	/// @return If no error occurs, returns the total number of bytes sent, 
	/// which can be less than the number indicated by len. 
	/// Otherwise, a value of SOCKET_ERROR is returned.
	/// SOCKET_ERROR is defined to -1
	virtual int SendTo(char *szBuff, int iLen, string strIP, int iPort, int iFlags = NULL);
	
	/// @fn int SendTo(char *szBuff, int iLen, int iFlags = NULL)
	/// @brief Send message
	/// @param[in,out] szBuff null terminated C string
	/// @param[in] iLen length of szBuff
	/// @param[in] iFlags Flags
	/// @return If no error occurs, returns the total number of bytes sent, 
	/// which can be less than the number indicated by len. 
	/// Otherwise, a value of SOCKET_ERROR is returned
	/// SOCKET_ERROR is defined to -1
	virtual int SendTo(char *szBuff, int iLen, int iFlags = NULL);
	
	/// send message
	virtual int Send(char *szBuff, int iLen, int iFlags = NULL);
	
	/// receive messages
	virtual int RecvFrom(char *szBuff, int iLen, int iFlags = NULL);
	
	/// receive message
	virtual int Recv(char *szBuff, int iLen, int iFlags = NULL);
	
	/// @brief get local address for a connected socket
	///
	/// The function retrieves the current name. 
	/// It is used on the bound or connected socket. 
	/// The local association is returned. 
	/// This call is especially useful when a connect call has been made 
	/// without doing a bind first; 
	/// the function provides the only way to determine the local association 
	/// that has been set by the system.
	/// The function does not always return information about the host address 
	/// when the socket has been bound to an unspecified address, 
	/// unless the socket has been connected with connect 
	/// or accept (for example, using ADDR_ANY). 
	/// A Windows Sockets application must not assume that the address will be specified unless the socket is connected. 
	/// The address that will be used for the socket is unknown 
	/// unless the socket is connected when used in a multihomed host. 
	/// If the socket is using a connectionless protocol, 
	/// the address may not be available until I/O occurs on the socket.
	///
	/// After this call, you can call getLocalAddr() to get the IP, 
	/// or call getLocalPort() to get the port
	/// @return true/false
	/// @sa getLocalAddr()
	/// @sa getLocalPort()
	virtual bool GetSockname();
	
	/// @brief Connect to remote machine
	///
	/// Connect to the remote machine according the given IP address and port
	/// @param[in] strIP remote address in decimal dotted format only
	/// @param[in] iPort remote port
	/// @return if succeed, return true, otherwise return false
	virtual bool Connect(string strIP, int iPort);
	
	/// Connect to remote machine
	virtual bool Connect();
	
	/// Close socket
	virtual bool Close();
	
	// operations for m_addrRemote
	/// @brief Get remote port
	/// @return return the remote port
	virtual int getRemotePort();
	
	/// Set remote port
	virtual bool setRemotePort(int iPort);
	
	/// @brief Set remote IP
	/// @param[in] strAddr remote IP in decimal dotted format
	virtual bool setRemoteAddr(string strAddr);
	
	/// Get remote address
	/// @return return remote IP in decimal dotted format
	virtual string getRemoteAddr();
	
	// operations for m_addrLocal
	/// Get local port
	virtual int getLocalPort();
	
	/// Set local port
	virtual bool setLocalPort(int iPort);
	
	/// Set local IP
	/// @param[in] strAddr local IP in decimal dotted format
	virtual bool setLocalAddr(string strAddr);
	
	/// Get local IP
	/// @return return local IP in decimal dotted format
	virtual string getLocalAddr();
	
	/// Set socket descriptor
	virtual void setSocket(int iSocket);
	
	/// Output socket message
	virtual void PrintSocket();
	
	/// @brief Accept connection
	/// @return return a new Socket
	virtual Socket Accept();
	
	/// Listen for connecting
	virtual bool Listen(int iBacklog = 5);
	
	/// @brief Bind to the specified IP and port
	/// @param[in] strIP local IP in decimal dotted format
	/// @param[in] iPort local port
	virtual bool Bind(string strIP, int iPort);
	
	/// Create socket
	virtual bool SocketCreate();
	
	/// Create Socket
	virtual bool SocketCreate(int iDomain, int iSockType, int iProtocol = 0);
	
	/// Whether the socket active/created
	virtual bool isActive();
	
	/// Init some member variable
	virtual void InitSocket();
	
	/// Set socket protocol
	virtual void setProtocol(int iProtocol);
	
	/// Set socket type, SOCK_RAW/SOCK_DGRAM/SOCK_STREAM
	virtual void setSockType(int iSockType);
	
	/// Set socket domain/address family
	virtual void setDomain(int iDomain = AF_INET);
	
	/// Get socket descriptor
	virtual int getSocket() const;
	
	/// Constructor
	Socket();
	
	/// Constructor
	Socket(int iDomain, int iSockType, int iProtocol);
	
	/// Destructor
	virtual ~Socket();

protected:
	int m_iSocket;				///< Socket
	int m_iDomain;				///< Address Family
	int m_iSockType;			///< Socket Type
	int m_iProtocol;			///< Protocol
	sockaddr_in m_addrLocal, m_addrRemote;	///< sockaddr_in variable
};

/// @class init_WSA Socket.h "Socket.h"
/// @brief Init socket under Windows platform
///
/// Only need to run once before socket, and it will auto cleanup after it's life time circle
class init_WSA
{
public:
	/// Constructor
	init_WSA();
	/// Destructor
	virtual ~init_WSA();
	/// Init socket
	bool init();
};


#endif // !defined(AFX_Socket_H__AA6DCE3F_C731_46F4_ACD6_9E5028FE18FC__INCLUDED_)
