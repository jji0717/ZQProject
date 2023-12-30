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
// Ident : $Id: FtpSock.cpp,v 1.3 2004/07/20 10:30:57 jshen Exp $
// Branch: $Name:  $
// Author: jshen
// Desc  : the socket class of the ftp
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/PT_FtpServer/FtpSock.cpp $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 2     08-12-19 16:59 Yixin.tian
// merge for Linux OS
// 
// 1     08-02-15 12:38 Jie.zhang
// 
// 2     07-12-19 15:31 Fei.huang
// 
// 1     07-08-16 10:12 Jie.zhang
// 
// 1     06-08-30 12:33 Jie.zhang
// 
// 2     06-07-18 17:09 Jie.zhang
// remove nameserver, direct connect to isacontent
// 
// 1     05-09-06 13:57 Jie.zhang
// 
// 3     05-08-08 12:23 Jie.zhang
// 
// 3     05-06-12 2:36 Jie.zhang
// 
// 2     05-03-29 19:47 Jie.zhang
// 
// 1     04-12-03 13:56 Jie.zhang
// 
// 3     04-11-22 14:27 Jie.zhang
// 
// 2     04-11-19 16:55 Jie.zhang
// Revision 1.3  2004/07/20 10:30:57  jshen
// no message
//
// Revision 1.2  2004/07/05 01:59:20  jshen
// add comments
//
// Revision 1.1  2004/06/07 09:19:43  jshen
// copied to production tree
//
// ===========================================================================

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#endif

#include "FtpSock.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FtpSock::FtpSock() : Socket(AF_INET, SOCK_STREAM, 0)
{
}

FtpSock::FtpSock(SOCKET fd) : Socket(fd)
{
}

FtpSock::~FtpSock()
{

}

//////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////




////////////////////////////////////////
// Functions for opening and closing
// connections.
////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// OpenClient() returns a handle to the connection to the host at
// "hostname" on port "portnum" (this is for the client).
//
// [in] hostname : The name of the host to connect to.
// [in] portnum  : The port number to connect on.
// [in] nsec     : Maximum number of seconds to wait.
//
// Return : On success the client socket desc is returned.
//          On failure SOCK_INVALID is returned.
//
SOCKET FtpSock::OpenClient(const char *hostname, const char *portnum, int nsec)
{
	struct hostent *hostp;
	unsigned short port;
	unsigned long addr;
	struct sockaddr saddr;
	unsigned long hostb;  //binary representation of the Internet address

	if (hostname == NULL || portnum == NULL)
		return(SOCK_INVALID);

	//Set the port to connect to --Port MUST be in Network Byte Order
	if (sscanf(portnum,"%hu",&port) != 1)
		return(SOCK_INVALID);

	//Set the IP address to connect to
	if ((hostb = inet_addr(hostname)) == INADDR_NONE) {
		if ((hostp = gethostbyname(hostname)) == 0) { //use this line if you want to enter host names
			Close();
			return(SOCK_INVALID);
		}
		addr = *((unsigned long *)hostp->h_addr);
	} else {
		addr = hostb;  //use this if you enter an IP addr
	}

	saddr.sa_family = AF_INET;
	(*((struct sockaddr_in*)&saddr)).sin_addr.s_addr = addr;
	(*((struct sockaddr_in*)&saddr)).sin_port = htons(port);

	if (Connect(&saddr,nsec) <= 0)  {
		Close();
		return(SOCK_INVALID);
	}

	return(_so);
}

//////////////////////////////////////////////////////////////////////
// OpenServer creates and binds a server to the port specified
// and the listens for a connection.  If a "bindip" is also specified
// the server will attempt to bind to this IP address.  Otherwise,
// the server will bind to all IPs.
//
// [in] port    : The port number to listen on.
// [in] bindip  : IP address to bind the server to (must be x.x.x.x)
// [in] backlog : the maximum length of the suspended queue
//
// Return : On success the listening socket desc is returned.
//          On failure SOCK_INVALID is returned.
//
SOCKET FtpSock::OpenServer(const unsigned short port, const char *bindip /*=NULL*/, int backlog /*=5*/, uint32* pdwError /* = NULL*/)
{
	struct sockaddr servaddr;
	unsigned long hostb;  //binary representation of the Internet address

	if (port <=0)
	{
#ifdef ZQ_OS_MSWIN
		if (pdwError)
			*pdwError = WSAEADDRINUSE;
#else
		if(pdwError)
			*pdwError = EADDRINUSE;
#endif
		return(SOCK_INVALID);
	}

	servaddr.sa_family = AF_INET;
	(*(struct sockaddr_in*)(&servaddr)).sin_port = htons(port);  //Port MUST be in Network Byte Order
	if (bindip == NULL) 
	{
		hostb = htonl(INADDR_ANY);  //Set to bind to all IPs
	} 
	else 
	{
		//Set the IP address to bind to
		if ((hostb = inet_addr(bindip)) == INADDR_NONE) 
		{
			printf("inet_addr INADDR_NONE\n");
			if (pdwError)
#ifdef ZQ_OS_MSWIN
				*pdwError = WSAEADDRINUSE;
#else
				*pdwError = EADDRINUSE;
#endif

			Close();
			return(SOCK_INVALID);
		}
	}

	(*(struct sockaddr_in*)(&servaddr)).sin_addr.s_addr = hostb;

	if (bind(_so,&servaddr,sizeof(struct sockaddr)) < 0)  
	{
		if (pdwError)
		{
#ifdef ZQ_OS_MSWIN
			*pdwError =	WSAGetLastError();
#else
			*pdwError = errno;
#endif
		}

		Close();
		return(SOCK_INVALID);
	}

	if (listen(_so, backlog) < 0)  
	{
		if (pdwError)
#ifdef ZQ_OS_MSWIN
			*pdwError = WSAGetLastError();
#else
			*pdwError = errno;
#endif

		Close();
		return(SOCK_INVALID);
	}

	if (pdwError)
		*pdwError = 0;

	return(_so);
}

//////////////////////////////////////////////////////////////////////
// Binds a server to "bindip":"port" and returns the port number that
// was binded to.  If "port" is 0, the port number will automatically
// be chosen by the OS.
//
// [out] bindport  : The port number that was binded to.
// [in] maxportlen : Max size of the bindport string.
// [in] bindip     : IP address to bind the server to (x.x.x.x)
// [in] port       : Port that should be binded to
//                   (if 0 the OS decides).
//
// Return : On success the socket desc that was binded to is returned.
//          On failure SOCK_INVALID is returned.
//
SOCKET FtpSock::BindServer(char *bindport, int maxportlen, char *bindip /*=NULL*/, short port /*=0*/)
{
	char bindaddr[SOCK_IPADDRLEN];

	//allows the port to be binded to again without a delay
	SetAddrReuse();

	//bind to the specified IP and port
	if (BindToAddr(bindip,port) == 0) {
		Close();
		return(SOCK_INVALID);
	}

	//Get the addr and port that was binded to
	if (bindport != NULL)
		getAddrPort(bindaddr,sizeof(bindaddr),bindport,maxportlen);

	return(_so);
}


//////////////////////////////////////////////////////////////////////
// Binds a socket desc to a specified IP and port.
//
// [in] bindip     : IP address to bind to (must be in x.x.x.x form)
// [in] port       : Port that should be binded to
//                   (if 0 the OS decides).
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FtpSock::BindToAddr(char *bindip, short bindport /*=0*/)
{
	struct sockaddr servaddr;
	unsigned long hostb;  //binary representation of the Internet address

	servaddr.sa_family = AF_INET;
	(*(struct sockaddr_in*)(&servaddr)).sin_port = htons(bindport); //if 0, OS will auto allocate
	if (bindip == NULL) {
		hostb = htonl(INADDR_ANY);  //Set to bind to all IPs
	} else {
		//Set the IP address to bind to
		if ((hostb = inet_addr(bindip)) == INADDR_NONE)
			return false;
	}
	(*(struct sockaddr_in*)(&servaddr)).sin_addr.s_addr = hostb;

	if (bind(_so,&servaddr,sizeof(struct sockaddr)) < 0) 
	{
		Close();
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Listens for a connection on the socket specified by "sd".
//
// [in] backlog : The number of backlogged connections to allow.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FtpSock::ListenServer(int backlog /*=5*/)
{

	if (listen(_so,backlog) < 0)
	{
		Close();
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Accept() takes the server handle (output of OpenServer()) and
// blocks until a client makes a connection.  At that point, it
// returns a handle to the connection to the client.
//
// [in] sd : The socket descriptor to accept the client connection on.
//
// Return : On success the socket desc for the client connection is
//          returned.  On failure SOCK_INVALID is returned.
//
SOCKET FtpSock::Accept()
{
	SOCKET clientsd;
	socklen_t addrlen;
	struct sockaddr cliaddr;

	addrlen = sizeof(struct sockaddr);
	if ((clientsd = accept(_so,&cliaddr,&addrlen)) < 0) {
		return(SOCK_INVALID);
	}

	return(clientsd);
}

//////////////////////////////////////////////////////////////////////
// Used to shutdown the socket (usually not necessary).
//
// [in] flag : How the socket should be shutdown.
//             0, further receives will be disallowed.
//             1, further sends will be disallowed.
//             2, further sends and receives will be disallowed.
//
// Return : VOID
//
void FtpSock::Shutdown(int flag)
{
	shutdown(_so,flag);
}

//////////////////////////////////////////////////////////////////////
// Used to close the socket.
//
// Return : VOID
//
#ifdef ZQ_OS_MSWIN
void FtpSock::Close()
{
	if (_so)
	{
		closesocket(_so);
		_so = NULL;
	}
}

void FtpSock::Close(SOCKET fd)
{
	closesocket(fd);
}

#else
void FtpSock::Close()
{
	if (_so)
	{
		close(_so);
		_so = -1;
	}
}

void FtpSock::Close(SOCKET fd)
{
	close(fd);
}

#endif
////////////////////////////////////////
// Functions used to get address
// information.
////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Returns the local address and port.
//
// [out] ipaddr    : IP addr of the socket.
// [in] maxaddrlen : Max size of the "ipaddr" string.
// [out] port      : Port number of the socket.
// [in] maxportlen : Max size of the "port" string.
//
// Return : VOID
//
void FtpSock::getAddrPort(char *ipaddr, int maxaddrlen, char *port, int maxportlen)
{
	unsigned short a1,a2,a3,a4;
	char *ptr;
	socklen_t addrlen;
	struct sockaddr saddr;
	unsigned short portus;

	if (ipaddr == NULL || maxaddrlen == 0 || port == NULL || maxportlen == 0)
		return;

	if (maxaddrlen < 16 || maxportlen < 6) {
		*ipaddr = '\0'; *port = '\0';
		return;
	}

	//get the local IP address and port
	addrlen = sizeof(struct sockaddr);
	if (getsockname(_so,&saddr,&addrlen) == 0) {
		ptr = (char *)&(*(struct sockaddr_in*)(&saddr)).sin_addr.s_addr;
		a1 = (unsigned short)*ptr & 0x00FF;
		a2 = (unsigned short)*(ptr+1) & 0x00FF;
		a3 = (unsigned short)*(ptr+2) & 0x00FF;
		a4 = (unsigned short)*(ptr+3) & 0x00FF;
		sprintf(ipaddr,"%hu.%hu.%hu.%hu",a1,a2,a3,a4);
		portus = ntohs((*(struct sockaddr_in*)(&saddr)).sin_port);
		sprintf(port,"%hu",portus);
	} else {
		*ipaddr = '\0';
		*port = '\0';
	}
}

//////////////////////////////////////////////////////////////////////
// Returns the peer's address and port.
//
// [out] ipaddr    : IP addr of the socket.
// [in] maxaddrlen : Max size of the "ipaddr" string.
// [out] port      : Port number of the socket.
// [in] maxportlen : Max size of the "port" string.
//
// Return : VOID
//
void FtpSock::getPeerAddrPort(char *ipaddr, int maxaddrlen, char *port, int maxportlen)
{
	unsigned short a1,a2,a3,a4;
	char *ptr;
	socklen_t addrlen;
	struct sockaddr saddr;
	unsigned short portus;

	if (ipaddr == NULL || maxaddrlen == 0 || port == NULL || maxportlen == 0)
		return;

	if (maxaddrlen < 16 || maxportlen < 6) {
		*ipaddr = '\0'; *port = '\0';
		return;
	}

	//get the peer's IP address and port
	addrlen = sizeof(struct sockaddr);
	if (getpeername(_so, &saddr, &addrlen) == 0) {
		ptr = (char *)&(*(struct sockaddr_in*)(&saddr)).sin_addr.s_addr;
		a1 = (unsigned short)*ptr & 0x00FF;
		a2 = (unsigned short)*(ptr+1) & 0x00FF;
		a3 = (unsigned short)*(ptr+2) & 0x00FF;
		a4 = (unsigned short)*(ptr+3) & 0x00FF;
		sprintf(ipaddr,"%hu.%hu.%hu.%hu",a1,a2,a3,a4);
		portus = ntohs((*(struct sockaddr_in*)(&saddr)).sin_port);
		sprintf(port,"%hu",portus);
	} else {
		*ipaddr = '\0';
		*port = '\0';
	}
}

//////////////////////////////////////////////////////////////////////
// Returns the peer's address and port.
//
// [in] sd         : The socket desc to get the addr and port for.
// [out] ipaddr    : IP addr of the socket.
// [in] maxaddrlen : Max size of the "ipaddr" string.
// [out] port      : Port number of the socket.
// [in] maxportlen : Max size of the "port" string.
//
// Return : VOID
//
void FtpSock::getPeerAddrPort(SOCKET sd, char *ipaddr, int maxaddrlen, char *port, int maxportlen)
{
	unsigned short a1,a2,a3,a4;
	char *ptr;
	socklen_t addrlen;
	struct sockaddr saddr;
	unsigned short portus;

	if (ipaddr == NULL || maxaddrlen == 0 || port == NULL || maxportlen == 0)
		return;

	if (maxaddrlen < 16 || maxportlen < 6) {
		*ipaddr = '\0'; *port = '\0';
		return;
	}

	//get the peer's IP address and port
	addrlen = sizeof(struct sockaddr);
	if (getpeername(sd, &saddr, &addrlen) == 0) {
		ptr = (char *)&(*(struct sockaddr_in*)(&saddr)).sin_addr.s_addr;
		a1 = (unsigned short)*ptr & 0x00FF;
		a2 = (unsigned short)*(ptr+1) & 0x00FF;
		a3 = (unsigned short)*(ptr+2) & 0x00FF;
		a4 = (unsigned short)*(ptr+3) & 0x00FF;
		sprintf(ipaddr,"%hu.%hu.%hu.%hu",a1,a2,a3,a4);
		portus = ntohs((*(struct sockaddr_in*)(&saddr)).sin_port);
		sprintf(port,"%hu",portus);
	} else {
		*ipaddr = '\0';
		*port = '\0';
	}
}

//////////////////////////////////////////////////////////////////////
// Retrieves the host name corresponding to the IP address "ipaddr".
// The first "maxnamelen" characters of the host name are stored
// in "name".  This function basically does a reverse DNS lookup.
//
// [in] ipaddr     : IP addr to get the name for.
// [out] name      : Name of the machine with the addr in "ipaddr".
// [in] maxnamelen : Max size of the "name" string.
//
// Return : VOID
//
void FtpSock::GetAddrName(const char *ipaddr, char *name, int maxnamelen)
{
	unsigned long addr;
	struct hostent *hp;

	if (ipaddr == NULL || name == NULL || maxnamelen == 0)
		return;

	addr = inet_addr(ipaddr);
	if ((hp = gethostbyaddr((char *)&addr,4,AF_INET)) != NULL) {
		strncpy(name,hp->h_name,maxnamelen-1);
		name[maxnamelen-1] = '\0';    //make sure the string is NULL terminated
	} else {
		*name = '\0';
	}
}

//////////////////////////////////////////////////////////////////////
// Returns the IP (dotted notation) for "inputaddr".  This function
// basically does a DNS lookup.
//
// [in] inputaddr : Addr to convert to an IP.
// [out] outputip : IP string in dotted notation (xxx.xxx.xxx.xxx).
// [in] maxiplen  : Max size of the "outputip" string.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FtpSock::Addr2IP(const char *inputaddr, char *outputip, int maxiplen)
{
	struct hostent *hp;
	struct in_addr in;
	char *ptr;

	if (inputaddr == NULL || outputip == NULL || maxiplen == 0)
		return false;

	if (inet_addr(inputaddr) == INADDR_NONE) {
		//inputaddr is not an IP
		*outputip = '\0';
		if ((hp = gethostbyname(inputaddr)) != NULL) {
			memcpy((void*)&in,(void*)hp->h_addr,sizeof(in));    //changed from in.S_un.S_addr
			if ((ptr = inet_ntoa(in)) != NULL) {
				strncpy(outputip,ptr,maxiplen-1);
				outputip[maxiplen-1] = '\0';
			}
		}
	} else {
		//inputaddr is an IP
		strncpy(outputip,inputaddr,maxiplen-1);
		outputip[maxiplen-1] = '\0';
	}

	if (*outputip == '\0')
		return false;
	else
		return true;
}

//////////////////////////////////////////////////////////////////////
// Validates a "." (dotted) notation IP address.
//
// [in] ipaddr : IP address to validate.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FtpSock::ValidateIP(const char *ipaddr)
{

	if (inet_addr(ipaddr) == INADDR_NONE)
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
// Gets the IP address of this machine (not 127.0.0.1)
//
// [out] outputip : IP string in dotted notation (xxx.xxx.xxx.xxx).
// [in] maxiplen  : Max size of the "outputip" string.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FtpSock::GetMyIPAddress(char *outputip, int maxiplen)
{
	struct hostent *hostp1;
	struct hostent *hostp;
	struct in_addr in;

	if (outputip == NULL || maxiplen == 0)
		return false;

	*outputip = '\0';

	hostp = gethostbyname("localhost");
	//look up the real name to get the real IP address
	if (hostp) {
		hostp1 = gethostbyname(hostp->h_name);  
		if (hostp1) {
			memcpy((void*)&in,(void*)(hostp1->h_addr_list[0]),sizeof(in));
			strncpy(outputip,inet_ntoa(in),maxiplen-1);
			outputip[maxiplen-1] = '\0';
			return true;
		}
	}

	return false;
}

////////////////////////////////////////
// Functions for transfering data.
////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Read "nbytes" bytes from a descriptor.
//
// [out] ptr   : Buffer to put the received data into.
// [in] nbytes : Max size of the "ptr" buffer.
//
// Return : On success the number of bytes read is returned.  On
//          failure -1 is returned.
//
int FtpSock::RecvN(char *ptr, int nbytes)
{
	int nleft, nread;

	if (ptr == NULL || nbytes == 0)
		return (-1);
	*ptr = '\0';    //initialize to an "" string

	nleft = nbytes;
	while (nleft > 0)  {
		nread = recv(_so, ptr, nleft, 0);
		if (nread < 0)  {
			return(nread);    // error, return < 0
		} else if (nread == 0) {
			break;    // EOF
		}
		nleft -= nread;
		ptr += nread;
	}

	return (nbytes - nleft);    // return >= 0
}

int FtpSock::RecvN(SOCKET sd, char *ptr, int nbytes)
{
	int nleft, nread;

	if (ptr == NULL || nbytes == 0)
		return (-1);
	*ptr = '\0';    //initialize to an "" string

	nleft = nbytes;
	while (nleft > 0)  {
		nread = recv(sd, ptr, nleft, 0);
		if (nread < 0)  {
			return(nread);    // error, return < 0
		} else if (nread == 0) {
			break;    // EOF
		}
		nleft -= nread;
		ptr += nread;
	}

	return (nbytes - nleft);    // return >= 0

}

//////////////////////////////////////////////////////////////////////
// Reads a line of data from a socket -- reads until a '\n' is found.
// Same syntax as fgets.
//
// [out] ptr : Buffer to put the received data into.
// [in] nmax : Max size of the "ptr" buffer.
//
// Return : On success the number of bytes read is returned.  On
//          failure -1 is returned.
//
int FtpSock::RecvLn(char *ptr, int nmax)
{
	int i, rc;
	char c;

	if (ptr == NULL || nmax == 0)
		return(-1);
	*ptr = '\0';    //initialize to an "" string

	for (i = 0;  i < (nmax - 1);  i++)  {
		if ((rc = recv(_so, &c, 1, 0)) == 1)  {
			ptr[i] = c;
			if (c == '\n')
				break;
		} else if (rc == 0) {
			if (i == 0)
				return false;  // EOF no data read
			else
				break;      // EOF some data read
		} else {
			return(-1);     // error
		}
	}

	if (i >= (nmax - 1))
		ptr[i] = '\0';
	else
		ptr[i+1] = '\0';

	return(i);
}

int FtpSock::RecvLn(SOCKET sd, char *ptr, int nmax)
{
	int i, rc;
	char c;

	if (ptr == NULL || nmax == 0)
		return(-1);
	*ptr = '\0';    //initialize to an "" string

	for (i = 0;  i < (nmax - 1);  i++)  {
		if ((rc = recv(sd, &c, 1, 0)) == 1)  {
			ptr[i] = c;
			if (c == '\n')
				break;
		} else if (rc == 0) {
			if (i == 0)
				return false;  // EOF no data read
			else
				break;      // EOF some data read
		} else {
			return(-1);     // error
		}
	}

	if (i >= (nmax - 1))
		ptr[i] = '\0';
	else
		ptr[i+1] = '\0';

	return(i);
}

//////////////////////////////////////////////////////////////////////
// Write "nbytes" bytes to a descriptor.  Use in place of write()
//
// [in] ptr        : Buffer containing the data to write.
// [in] nbytes     : Number of bytes to write.
// [in] flagurgent : If flagurgent != 0, the data will be sent as urgent.
//
// Return : On success the number of bytes written is returned.  On
//          failure -1 is returned.
//
int FtpSock::SendN(const char *ptr, size_t nbytes, int flagurgent /*=0*/)
{
	int nleft;
	int nwritten;
	char *offsetptr;

	if (ptr == NULL)
		return (-1);

	offsetptr = (char *)ptr;
	nleft = (int)nbytes;
	while (nleft > 0) {
		if (flagurgent == 0)
			nwritten = send(_so,offsetptr,nleft,0);
		else
			nwritten = send(_so,offsetptr,nleft,MSG_OOB);
		if (nwritten <= 0)
			return(nwritten);    // error
		nleft -= nwritten;
		offsetptr += nwritten;
	}

	return (int) (nbytes - nleft);
}


int FtpSock::SendN(SOCKET sd, const char *ptr, size_t nbytes, int flagurgent /*=0*/)
{
	int nleft;
	int nwritten;
	char *offsetptr;

	if (ptr == NULL)
		return (-1);

	offsetptr = (char *)ptr;
	nleft = (int)nbytes;
	while (nleft > 0) {
		if (flagurgent == 0)
			nwritten = send(sd,offsetptr,nleft,0);
		else
			nwritten = send(sd,offsetptr,nleft,MSG_OOB);
		if (nwritten <= 0)
			return(nwritten);    // error
		nleft -= nwritten;
		offsetptr += nwritten;
	}

	return (int) (nbytes - nleft);

}

//////////////////////////////////////////////////////////////////////
// Write until '\n' is found (inserts a '\n' if necessary).
//
// [in] ptr : Buffer containing the data to write.
//
// Return : On success the number of bytes written is returned.  On
//          failure -1 is returned.
//
// NOTE: "ptr" must be a NULL terminated string.
//
int FtpSock::SendLn(const char *ptr, int flagurgent /*=0*/)
{
	char *offsetptr, *line;
	int retval;

	if (ptr == NULL)
		return(-1);

	if ((line = new char[strlen(ptr)+2]) == NULL)
		return false;
	strcpy(line,ptr);

	//make sure the line ends with '\n'
	if(*(line+strlen(line)-1) != '\n') {
		*(line+strlen(line)+1) = '\0';
		*(line+strlen(line)) = '\n';
	}            

	//terminate the line after any '\n'
	offsetptr = strchr(line,'\n');
	*(offsetptr+1) = '\0';

	retval = SendN(line,strlen(line),flagurgent);

	delete[] line;
	return retval;
}

int FtpSock::SendLn(SOCKET sd, const char *ptr, int flagurgent /*=0*/)
{
	char *offsetptr, *line;
	int retval;

	if (ptr == NULL)
		return(-1);

	if ((line = new char[strlen(ptr)+2]) == NULL)
		return false;
	strcpy(line,ptr);

	//make sure the line ends with '\n'
	if(*(line+strlen(line)-1) != '\n') {
		*(line+strlen(line)+1) = '\0';
		*(line+strlen(line)) = '\n';
	}            

	//terminate the line after any '\n'
	offsetptr = strchr(line,'\n');
	*(offsetptr+1) = '\0';

	retval = SendN(sd,line,strlen(line),flagurgent);

	delete[] line;
	return retval;
}

////////////////////////////////////////
// Functions for setting socket options.
////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Sets the socket to S_KEEPALIVE -- so the connections won't be
// dropped.
//
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FtpSock::SetKeepAlive()
{
	int ret;
	int val = 1;

	ret = setsockopt(_so,SOL_SOCKET,SO_KEEPALIVE,(char *)&val,sizeof(int));

	if (ret == 0)
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////
// Sets the max time a socket will block when receiving.
//
// [in] timeoutms : Recv timeout in milliseconds (0 = infinite)
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FtpSock::SetTimeout(int timeoutms)
{
	int retval;

	retval = setsockopt(_so,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeoutms,sizeof(timeoutms));

	if (retval == 0)
		return true;  //setsockopt() successful
	else
		return false;  //setsockopt() failed
}

//////////////////////////////////////////////////////////////////////
// Checks to see if a socket will block.
//
// [in] nsec  : Maximum number of seconds to wait.
// [in] nmsec ; Maximum number of milliseconds to wait.
//
// Return : 1 if the socket will not block. 0 if the socket will
//          block. -1 on error.  If the timeout period is set to
//          0, 1 is returned.
//
int FtpSock::CheckStatus(int nsec, int nmsec /*=0*/)
{
	int retval = 0;

	fd_set sdset;
	struct timeval tv;

	if (nsec == 0 && nmsec == 0)
		return true;

	FD_ZERO(&sdset);    //initialize the sdset structure
	FD_SET(_so, &sdset);  //set the structure
	tv.tv_sec = nsec;   //set server to wait for nsec seconds
	tv.tv_usec = nmsec * 1000;

	retval = select((int)_so+1, &sdset, NULL, NULL, &tv);

	return retval;
}

//////////////////////////////////////////////////////////////////////
// Checks to see if a socket will block.
//
// [in] sd    : Socket desc to check.
// [in] nsec  : Maximum number of seconds to wait.
// [in] nmsec ; Maximum number of milliseconds to wait.
//
// Return : 1 if the socket will not block. 0 if the socket will
//          block. -1 on error.  If the timeout period is set to
//          0, 1 is returned.
//
int FtpSock::CheckStatus(SOCKET sd, int nsec, int nmsec /*=0*/)
{
	int retval = 0;

	fd_set sdset;
	struct timeval tv;

	if (nsec == 0 && nmsec == 0)
		return true;

	FD_ZERO(&sdset);    //initialize the sdset structure
	FD_SET(sd, &sdset);  //set the structure
	tv.tv_sec = nsec;   //set server to wait for nsec seconds
	tv.tv_usec = nmsec * 1000;

	retval = select((int)sd+1, &sdset, NULL, NULL, &tv);

	return retval;
}

//////////////////////////////////////////////////////////////////////
// Sets the socket to receive OOB data in the normal data stream.
//
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FtpSock::SetRecvOOB()
{
	int ret;
	int val = 1;

	ret = setsockopt(_so,SOL_SOCKET,SO_OOBINLINE,(char *)&val,sizeof(int));

	if (ret == 0)
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////
// Sets the socket to allow immediate reuse of an address/port.
//
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
int FtpSock::SetAddrReuse()
{
	int ret;
	int val = 1;

	ret = setsockopt(_so,SOL_SOCKET,SO_REUSEADDR,(char *)&val,sizeof(int));

	if (ret == 0)
		return true;
	else
		return false;
}

////////////////////////////////////////
// Functions used for network byte
// order conversions.
////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Converts an unsigned long from host to TCP/IP network byte order
// (which is big-endian).
//
// [in] lval : value to convert.
//
// Return : Converted value.
//
unsigned long FtpSock::HtoNl(unsigned long lval)
{

	return(htonl(lval));
}

//////////////////////////////////////////////////////////////////////
// Converts an unsigned short from host to TCP/IP network byte order.
//
// [in] sval : value to convert.
//
// Return : Converted value.
//
unsigned short FtpSock::HtoNs(unsigned short sval)
{

	return(htons(sval));
}

//////////////////////////////////////////////////////////////////////
// Converts an unsigned long from TCP/IP network byte order to host
// byte order.
//
// [in] lval : value to convert.
//
// Return : Converted value.
//
unsigned long FtpSock::NtoHl(unsigned long lval)
{

	return(ntohl(lval));
}

//////////////////////////////////////////////////////////////////////
// Converts an unsigned short from TCP/IP network byte order to host
// byte order.
//
// [in] sval : value to convert.
//
// Return : Converted value.
//
unsigned short FtpSock::NtoHs(unsigned short sval)
{

	return(ntohs(sval));
}


//////////////////////////////////////////////////////////////////////
// Private Methods
//////////////////////////////////////////////////////////////////////

//implements the standard connect() function with a timeout
int FtpSock::Connect(struct sockaddr *saddr, int nsec)
{
	int retval, flagconnected = 0;
	fd_set writefds, exceptfds;
	struct timeval tv;
	long sockopt;
	socklen_t len;

	//set socket to non-blocking
#ifdef WIN32
	unsigned long ularg;
	ularg = 1;
	retval = ioctlsocket(_so,FIONBIO,&ularg);
#else
	int fflags;
	fflags = fcntl(_so,F_GETFL);
	retval = fcntl(_so,F_SETFL,fflags|O_NONBLOCK);
#endif

	if (retval != SOCK_ERROR) {
		if (connect(_so, saddr,sizeof(struct sockaddr)) != SOCK_ERROR) {
			flagconnected = 1;
		} else {
#ifdef WIN32
			if (GetLastError() == WSAEWOULDBLOCK) {
#else
			if (GetLastError() == EINPROGRESS) {
#endif
				//connect succeeded as a non-blocking call
				//now select to wait for the connection to be completed
				FD_ZERO(&writefds);
				FD_SET(_so,&writefds);
				FD_ZERO(&exceptfds);
				FD_SET(_so,&exceptfds);
				tv.tv_sec = nsec;
				tv.tv_usec = 0;
				sockopt = 0;
				len = sizeof(sockopt);
				//Windows sets the exceptfds if a nonblocking connect fails.
				//retval = select(sd+1,NULL,&writefds,NULL,&tv);    //possible non-windows ver
				if (select((int)_so+1,NULL,&writefds,&exceptfds,&tv) > 0) {
					if (getsockopt(_so,SOL_SOCKET,SO_ERROR,(char *)&sockopt,&len) != SOCK_ERROR) {
						if (sockopt == 0 && FD_ISSET(_so,&writefds) != 0)
							flagconnected = 1;
					}
				}
			}
		}
	}

	if (flagconnected != 0) {
		//unset non-blocking
#ifdef WIN32
		ularg = 0;
		retval = ioctlsocket(_so,FIONBIO,&ularg);
#else
		retval = fcntl(_so,F_SETFL,fflags);
#endif

	}

	return(flagconnected);
}

//gets the error code of the last error
int FtpSock::GetLastError()
{
	int errval;

#ifdef WIN32
	errval = WSAGetLastError();
#else
	errval = errno;
#endif

	return (errval);
}

int FtpSock::RecvN_Timeout(SOCKET sd, char *ptr, int nbytes, int nTimeOut /* = TIMEOUT_READDATA second*/)
{
	int nleft, nread;

	if (ptr == NULL || nbytes == 0)
		return (-1);
	*ptr = '\0';    //initialize to an "" string

	nleft = nbytes;
	while (nleft > 0)  {
	    if (CheckStatus(sd, nTimeOut, 0) <= 0)
		{
			break;			
		}

		nread = recv(sd, ptr, nleft, 0);
		if (nread <= 0)  
		{
			break;
		}

		nleft -= nread;
		ptr += nread;
	}

	return (nbytes - nleft);    // return >= 0
}

int FtpSock::SetTcpNoDelay(bool bEnale)
{
	setNoDelay(bEnale);
	return 0;
}
