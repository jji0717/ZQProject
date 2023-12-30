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
//
// Ident : $Id:  $
// Branch: $Name:  $
// Author: Bernie Zhao (Tianbin Zhao)
// Desc  : rtsp sock definition
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspSock.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 4     04-12-06 20:55 Bernie.zhao
// socket reset improved
// 
// 3     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// 2     04-09-14 17:35 Bernie.zhao
// 
// ===========================================================================
#pragma once

#ifdef WIN32
#  include <winsock2.h>
typedef int socklen_t;    //Make compatible with UNIX
#else
typedef int SOCKET;       //Make compatible with MSVC.
#endif

#ifdef	INVALID_SOCKET
#define SOCK_INVALID INVALID_SOCKET
#else
#define SOCK_INVALID -1
#endif

#ifdef	SOCKET_ERROR
#define SOCK_ERROR SOCKET_ERROR
#else
#define SOCK_ERROR -1
#endif

#ifndef INADDR_NONE
#define INADDR_NONE 0xFFFFFFFF
#endif

#define SOCK_IPADDRLEN 32       //max string length of an IP address
#define SOCK_PORTLEN   16       //max string length of a port number

#include "Socket.h"
#include "Locks.h"

/// \class RtspSock  the socket encapsulated class for RTSP transfer
class RtspSock :
	public ZQ::common::Socket
{
public:
	RtspSock(void);
	RtspSock(SOCKET fd);
	~RtspSock(void);
protected:
	ZQ::common::Mutex* _mutx;

	struct sockaddr _sAddr;
	int				_nSec;
	
private:
	/// private method, connect to a specified address with timeout
	int connectAddr(struct sockaddr * saddr, int nsec);

public:
	//////////////////////////////////////////////////////////////////////////
	// methods for opening and closing connection

	/// get socket descriptor
	///@return the socket
	SOCKET getSock();

	/// open a connection for a client to a specified server
	///@return the socket
	///@param hostname	is the server hostname of ip address
	///@param portnum		is the server connection port number
	///@param nsec		is the timeout of the connection
	SOCKET openClient(const char* hostname, const char* portnum, int nsec);

	/// bind ip and port to current socket
	///@return	1 on success and 0 on failure
	///@param bindip		is the ip that been binded to the socket
	///@param bindport	is the port number been binded to the socket, if 0, OS decides it
	bool bindToAddr(const char* bindip, short bindport = 0);

	/// shutdown current socket and create a new socket object
	///@return	1 on success and 0 on failure
	bool resetSock();

	/// close a connection socket
	void closeSock(void);

	//////////////////////////////////////////////////////////////////////////
	// methods for transfering data

	/// receive bytes from socket
	///@return number of bytes received on success and  -1 on failure
	///@param buff		is the buffer that data put into
	///@param nbytes		is the max size of the buffer
	int recvN(char* buff, int nbytes);

	/// receive a line from socket until '\n' is reached
	///@return number of bytes received on success and  -1 on failure
	///@param buff		is the buffer that data put into
	///@param nbytes		is the max size of the buffer
	int recvLine(char* buff , int nbytes);

	/// send bytes in buffer to socket
	///@return number of bytes sent on success and -1 on failure
	///@param buff		is the buffer that contains the data
	///@param nbytes		is the size of data in buffer
	///@param flag		is the way that socket send data
	int sendN(const char* buff, int nbytes, int flag = 0);

	/// send a line to socket
	///@return number of bytes sent on success and -1 on failure
	///@param buff		is the buffer that contains the data
	///@param nbytes		is the size of data in buffer
	///@param flag		is the way that socket send data
	int sendLine(const char* buff, int nbytes, int flag = 0);
};
