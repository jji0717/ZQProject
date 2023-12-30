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
// Ident : $Id: FtpSock.h,v 1.3 2004/07/20 10:30:45 jshen Exp $
// Branch: $Name:  $
// Author: jshen
// Desc  : the socket class of the ftp
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/PT_FtpServer/FtpSock.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 3     09-06-26 17:45 Yixin.tian
// 
// 2     08-12-19 16:59 Yixin.tian
// merge for Linux OS
// 
// 1     08-02-15 12:38 Jie.zhang
// 
// 4     05-06-12 2:36 Jie.zhang
// 
// 3     05-04-11 19:03 Jie.zhang
// 
// 2     05-03-29 19:47 Jie.zhang
// 
// 1     04-12-03 13:56 Jie.zhang
// 
// 2     04-11-19 16:55 Jie.zhang
// Revision 1.3  2004/07/20 10:30:45  jshen
// no message
//
// Revision 1.2  2004/07/05 01:59:28  jshen
// add comments
//
// Revision 1.1  2004/06/07 09:19:43  jshen
// copied to production tree
//
// ===========================================================================

#ifndef __SOCK_H__
#define __SOCK_H__

#ifdef WIN32_MFC
#  include "StdAfx.h"
#endif // WIN32_MFC

#ifdef WIN32
#  include <winsock2.h>
typedef int socklen_t;    //Make compatible with UNIX
#else
	#ifndef SOCKET
		typedef int SOCKET;       //Make compatible with MSVC.
	#endif
#endif

#ifdef INVALID_SOCKET
#define SOCK_INVALID INVALID_SOCKET
#else
#define SOCK_INVALID -1
#endif

#ifdef SOCKET_ERROR
#define SOCK_ERROR SOCKET_ERROR
#else
#define SOCK_ERROR -1
#endif

#ifndef INADDR_NONE
#define INADDR_NONE 0xFFFFFFFF
#endif

#define SOCK_IPADDRLEN 32       //max string length of an IP address
#define SOCK_PORTLEN   16       //max string length of a port number
#define TIMEOUT_READDATA		1		// default read time out, in seconds

#include "Socket.h"
using ZQ::common::Socket;

class FtpSock : public ZQ::common::Socket
{
public:
	enum
	{
		E_SUCCESS = 0,
		E_BIND_PORT = 1,
		E_BIND_IP = 2,
		E_LISTEN = 3
	};

	FtpSock();
	FtpSock(SOCKET fd);
	virtual ~FtpSock();

	//Functions for opening and closing connections
	SOCKET OpenClient(const char *hostname, const char *portnum, int nsec);

	//	error code	E_SUCCESS, E_BIND_PORT, E_BIND_IP, E_LISTEN	
	SOCKET OpenServer(const unsigned short port, const char *bindip = NULL, int backlog = 5, uint32* pdwError = NULL);
	

	SOCKET BindServer(char *bindport, int maxportlen, char *bindip = NULL, short port = 0);
	int BindToAddr(char *bindip, short bindport);
	int ListenServer(int backlog = 5);
	SOCKET Accept(void);
	void Shutdown(int flag);
	void Close();
	static void Close(SOCKET fd);

	//Functions used to get address information
    void getAddrPort(char *ipaddr, int maxaddrlen, char *port, int maxportlen);
	void getPeerAddrPort(char *ipaddr, int maxaddrlen, char *port, int maxportlen);
	static void getPeerAddrPort(SOCKET sd, char *ipaddr, int maxaddrlen, char *port, int maxportlen);
	void GetAddrName(const char *ipaddr, char *name, int maxnamelen);
	int Addr2IP(const char *inputaddr, char *outputip, int maxiplen);
	int ValidateIP(const char *ipaddr);
	int GetMyIPAddress(char *outputip, int maxiplen);

	//Functions for transfering data
	int RecvN(char *ptr, int nbytes);
	int RecvLn(char *ptr, int nmax);
	int SendN(const char *ptr, size_t nbytes, int flagurgent = 0);
	int SendLn(const char *ptr, int flagurgent = 0);

	static int RecvN(SOCKET sd, char *ptr, int nbytes);
	static int RecvN_Timeout(SOCKET sd, char *ptr, int nbytes, int nTimeOut = TIMEOUT_READDATA);

	static int RecvLn(SOCKET sd, char *ptr, int nmax);
	static int SendN(SOCKET sd, const char *ptr, size_t nbytes, int flagurgent = 0);
	static int SendLn(SOCKET sd, const char *ptr, int flagurgent = 0);



	//Functions for setting socket options
	int SetKeepAlive(void);
	int SetTimeout(int timeoutms);
	int CheckStatus(int nsec, int nmsec = 0);
	static int CheckStatus(SOCKET sd, int nsec, int nmsec = 0);
	int SetRecvOOB();
	int SetAddrReuse();
	int SetTcpNoDelay(bool bEnale);

	//Functions used for network byte order conversions
	unsigned long HtoNl(unsigned long lval);
	unsigned short HtoNs(unsigned short sval);
	unsigned long NtoHl(unsigned long lval);
	unsigned short NtoHs(unsigned short sval);

private:
	int Connect(struct sockaddr *saddr, int nsec);
	int GetLastError();
};

#endif //__SOCK_H__
