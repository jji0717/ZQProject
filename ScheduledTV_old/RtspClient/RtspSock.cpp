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
// Desc  : rtsp sock implementation
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_old/RtspClient/RtspSock.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 11    04-12-06 20:55 Bernie.zhao
// socket reset improved
// 
// 10    04-11-30 10:24 Bernie.zhao
// Nov30, after resolving ISSStreamTerminate problem
// 
// 9     04-10-24 19:04 Bernie.zhao
// end of 2004/Oct/24
// 
// 8     04-10-22 16:38 Bernie.zhao
// 
// 7     04-10-22 10:04 Ken.qian
// 
// 6     04-10-22 10:03 Bernie.zhao
// 
// 5     04-10-21 15:28 Bernie.zhao
// 
// 4     04-09-20 18:29 Bernie.zhao
// fixed time delay problem??
// 
// 3     04-09-14 17:30 Bernie.zhao
// 
// 2     04-09-14 15:30 Bernie.zhao
// block fixed??
// 
// ===========================================================================
//#include "StdAfx.h"
#include ".\rtspsock.h"
#include "RtspHeaders.h"

RtspSock::RtspSock(void) : Socket(AF_INET, SOCK_STREAM, 0)
{
	_mutx = new ZQ::common::Mutex;
}

RtspSock::RtspSock(SOCKET fd) : Socket(fd)
{
	_mutx = new ZQ::common::Mutex;
	_state = Socket::AVAILABLE;
}

RtspSock::~RtspSock(void)
{
	if(_so!=INVALID_SOCKET)
		endSocket();
	if(_mutx)
		delete _mutx;
}

SOCKET RtspSock::getSock()
{
	return _so;
}

int RtspSock::connectAddr(struct sockaddr * saddr, int nsec)
{
	int flagconnected = 0;
	fd_set writefds, exceptfds;
	struct timeval tv;
	long sockopt;
	socklen_t len;

	bool fblock = false;
	setCompletion(fblock);

	if (connect(_so, saddr,sizeof(struct sockaddr)) != SOCK_ERROR) {
		flagconnected = 1;
	} else {
		int lasterr = GetLastError();
#ifdef _DEBUG
		printf("Maybe socket connect error: %d\n", lasterr);
#endif
#ifdef WIN32
		if (lasterr == WSAEWOULDBLOCK) {
#else
		if (lasterr == EINPROGRESS) {
#endif //WIN32
#ifdef _DEBUG
			printf("connect on non-blocking socket %d got error code %d\n",_so, lasterr);
#endif
			
			//connect succeeded as a non-blocking call
			//now select to wait for the connection to be completed
			FD_ZERO(&writefds);
			FD_SET(_so,&writefds);
			FD_ZERO(&exceptfds);
			FD_SET(_so,&exceptfds);
			tv.tv_sec = nsec/1000;
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
			else {
#ifdef _DEBUG
				printf("Can not wait for non-blocking socket %d to connect successfully", _so);
#endif
			}
		}
	}
	

	//set socket to non-blocking
	if (flagconnected != 0) {
		bool cpl = false;
		setCompletion(cpl);
		_state = Socket::CONNECTED;
	}

	return(flagconnected);
}

SOCKET RtspSock::openClient(const char* hostname, const char* portnum, int nsec)
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
			closeSock();
			return(SOCK_INVALID);
		}
		addr = *((unsigned long *)hostp->h_addr);
	} else {
		addr = hostb;  //use this if you enter an IP addr
	}

	saddr.sa_family = AF_INET;
	(*((struct sockaddr_in*)&saddr)).sin_addr.s_addr = addr;
	(*((struct sockaddr_in*)&saddr)).sin_port = htons(port);

	_sAddr	= saddr;
	_nSec	= nsec;
	if (connectAddr(&saddr,nsec) <= 0)  {
		closeSock();
		return(SOCK_INVALID);
	}

	/*u_long nonblock = 1000;

	if(ioctlsocket(_so,FIONBIO,&nonblock) == SOCKET_ERROR ) {
		closeSock();
		return(SOCK_INVALID);
	}*/
	return(_so);
}

bool RtspSock::bindToAddr(const char* bindip, short bindport /*=0*/)
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
		closeSock();
		return false;
	}

	return true;
}

bool RtspSock::resetSock()
{
	endSocket();
	
	setSocket();

	_so = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(_so == INVALID_SOCKET)
	{
		error(errCreateFailed);
		return FALSE;
	}
	_state = AVAILABLE;
	
	if (connectAddr(&_sAddr,_nSec) <= 0)  {
		closeSock();
		return FALSE;
	}
	return TRUE;
}

void RtspSock::closeSock(void)
{
	endSocket();
}

int RtspSock::recvN(char* buff, int nbytes)
{
	int nleft, nread;
	bool	firstthrough=true;

	if (buff == NULL || nbytes == 0)
		return (-1);
	*buff = '\0';    //initialize to an "" string

	nleft = nbytes;
	//while (nleft > 0)  {
 //       nread = recv(_so, buff, nbytes, 0);
	//	if (nread < 0)  {
	//		if(firstthrough)
	//			return nread; // error, return < 0
	//		else
	//			break;	// had received data last times
	//	} else if (nread == 0) {
	//		break;    // EOF
	//	}
	//	nleft -= nread;
	//	buff += nread;
	//	firstthrough=false;
	//}
	_mutx->enter();
	nread = recv(_so, buff, nbytes, 0);
	_mutx->leave();
	if( nread <= 0)
		return nread; // error, return <= 0
    nleft -= nread;

	return (nbytes - nleft);    // return >= 0
}

int RtspSock::recvLine(char* buff , int nbytes)
{
	int i, rc;
	char c;

	if (buff == NULL || nbytes == 0)
		return(-1);
	*buff = '\0';    //initialize to an "" string

	for (i = 0;  i < (nbytes - 1);  i++)  {
		if ((rc = recv(_so, &c, 1, 0)) == 1)  {
			buff[i] = c;
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

	if (i >= (nbytes - 1))
		buff[i] = '\0';
	else
		buff[i+1] = '\0';

	return(i);
}

int RtspSock::sendN(const char* buff, int nbytes, int flag /*=0*/)
{
	int nleft;
	int nwritten;
	char *offsetptr;

	if (buff == NULL)
		return (-1);

	struct timeval tv={0,1};
	fd_set	sockset;
	FD_ZERO( &sockset);
	FD_SET( _so, &sockset);
	
//#ifdef _DEBUG
//	printf("begin select before send\n");
//#endif
	int selret = ::select(0, NULL, &sockset, NULL, &tv);
//#ifdef _DEBUG
//	printf("end select before send:  %d\n",selret);
//#endif
	if( selret > 0 ) {	// something notified
		// can send data, send
		offsetptr = (char *)buff;
		nleft = (int)nbytes;
		while (nleft > 0) {
			_mutx->enter();
//#ifdef _DEBUG
//			printf("begin sending...\n");
//#endif
			if (flag == 0)
				nwritten = send(_so,offsetptr,nleft,0);
			else
				nwritten = send(_so,offsetptr,nleft,MSG_OOB);
			if (nwritten <= 0)
				return(nwritten);    // error
			nleft -= nwritten;
			offsetptr += nwritten;
//#ifdef _DEBUG
//			printf("end sending!\n");
//#endif
		}
		_mutx->leave();
		return (int) (nbytes - nleft);
		//break;
	}

	else if(selret == 0) {
		return -1;
		//break;
		
	}
	else
	{
		int status = GetLastError();
#ifdef _DEBUG
		printf("select() error %d\n", status);
#endif
		return 0;
	}

	
}

int RtspSock::sendLine(const char* buff, int nbytes, int flag /*=0*/)
{
	char *offsetptr, *line;
	int retval;

	if (buff == NULL)
		return(-1);

	if ((line = new char[strlen(buff)+2]) == NULL)
		return false;
	strcpy(line,buff);

	//make sure the line ends with '\n'
	if(*(line+strlen(line)-1) != '\n') {
		*(line+strlen(line)+1) = '\0';
		*(line+strlen(line)) = '\n';
	}            

	//terminate the line after any '\n'
	offsetptr = strchr(line,'\n');
	*(offsetptr+1) = '\0';

	retval = sendN(line,(int)strlen(line),flag);

	delete[] line;
	return retval;
}
