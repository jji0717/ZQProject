/*
 * =====================================================================================
 *
 *       Filename:  ClientSocket.cpp
 *
 *    Description:  ClearSocket.h所定义函数的实现
 *        Version:  1.1
 *        Created:  March 18th, 2008
 *       Revision:  1.0
 *       Compiler:  vs.net 2005
 * 
 *         Author:  Xiaoming Li
 *        Company:  
 *
 * =====================================================================================
 */

#include "ClientSocket.h"

static const int g_iTCPSocketBufSize = 1025*1024*10;

bool InitialSocket()
{
#ifdef WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR)
		return false;
	else
		return true;
#else
	return true;
#endif
}

bool CleanupSocket()
{
#ifdef WIN32
	int ret = WSACleanup();
	if (0 == ret)
		return true;
	else
		return false;
#else
	return true;
#endif
}

SOCKET CreateSocket(SOCKET_TYPE type)
{
	SOCKET iSocketID = 0;
	
	if (type == TCPSOCKET)
		iSocketID = socket(AF_INET, SOCK_STREAM, 0);
	else if (type == UDPSOCKET)
		iSocketID = socket(AF_INET, SOCK_DGRAM, 0);
	else
	{
		cerr << "wrong socket type" << endl;
		return -2;
	}
	if(iSocketID == INVALID_SOCKET)
	{
		cerr << "create socket error:" << errno << endl;
		return iSocketID;
	}
	else
	{
		SetSNDBUFSize(iSocketID,g_iTCPSocketBufSize);
		SetRCVBUFSize(iSocketID,g_iTCPSocketBufSize);
		return iSocketID;
	}
}

//bool ClearSocket::CloseSocket(SOCKET SocketID)
bool CloseSocket(SOCKET SocketID)
{
	if (SocketID >= 0)
	{
		closesocket(SocketID);
		return true;
	}
	else
	{
		return false;
	}

}

//TCP connection function
bool bConnection(string strRemoteAddr, int16 sRemotePort, SOCKET tcpSocketID, int16 sTimeout)
{
	//set the default time out
	if (sTimeout < 0)
		sTimeout = 5;

	//initialize the structure
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, (size_t)sizeof(serverAddr));

/*used in vs.net 2005
	struct addrinfo aiHints;
	struct addrinfo *aiList = NULL;
	

	//--------------------------------
	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	memset(&aiHints, 0, sizeof(aiHints));
	aiHints.ai_family = AF_INET;
	aiHints.ai_socktype = SOCK_STREAM;
	aiHints.ai_protocol = IPPROTO_TCP;


	stringstream ss;
	ss << sRemotePort;
	string strRemotePort = ss.str();

	//--------------------------------
	// Call getaddrinfo(). If the call succeeds,
	// the aiList variable will hold a linked list
	// of addrinfo structures containing response
	// information about the host
	if ((rc = getaddrinfo(strRemoteAddr.c_str(), strRemotePort.c_str(), &aiHints, &aiList)) != 0)
	{
		cerr <<"getaddrinfo() failed.\n" << endl;
		return false;
	}
*******************************************************/

	hostent *pHosten = 	gethostbyname(strRemoteAddr.c_str());

	int rc;
	//set the value
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_addr.s_addr = inet_addr(strRemoteAddr.c_str());
	serverAddr.sin_port = htons(sRemotePort);

	//set the TCP socket to non-block
	SetSocketNonblock(tcpSocketID);

	//connect to server
	int serverLen = sizeof(serverAddr);

	//struct addrinfo *pList = aiList;
	//while (pList != NULL)

	if (pHosten != NULL)
	{
		//rc = connect(tcpSocketID, pList->ai_addr, serverLen);
		//pList = pList->ai_next;

		serverAddr.sin_addr.s_addr = *(u_long *) pHosten->h_addr_list[0];
		rc = connect(tcpSocketID, (struct sockaddr *)&serverAddr, serverLen);
		//fail to connect server
		if (rc == SOCKET_ERROR)
		{
			struct timeval timeout;
			fd_set fd;
			FD_ZERO(&fd);   
			FD_SET(tcpSocketID, &fd);
			timeout.tv_sec	= sTimeout;
			timeout.tv_usec	= 0;
			rc = select(0, NULL, &fd, NULL, &timeout);
			if (rc > 0)
				return true;
		}
		else if (rc == 0)
			return true;
	}
	return false;
}

//try to support join multi-cast team
bool Bind(string strBindIP, int16 sPort, SOCKET SocketID)
{
	//check the conditions
	if (sPort <= 0)
		return false;
	if (SocketID <= 0)
		return false;

	//try to bind
	char m_strBindIP[16];
	memset(m_strBindIP, 0, 16);

	//get the first bit
	int iFirstBit = 0;

	//bind to local IP(NULL IP address or illegal IP address will set to default IP)
	if (strBindIP.empty() || strBindIP.length() > 16)
		memcpy(m_strBindIP, "0.0.0.0", strlen("0.0.0.0"));
	else
		memcpy(m_strBindIP, strBindIP.data(), strBindIP.length());

	//get first bit of a IP address
	sscanf(m_strBindIP, "%d.", &iFirstBit);


	sockaddr_in udpLocalAddr;
	udpLocalAddr.sin_family = AF_INET;
	udpLocalAddr.sin_addr.s_addr = inet_addr(m_strBindIP);
	udpLocalAddr.sin_port = htons(sPort);

	//bind
	if(bind(SocketID, (struct sockaddr *)&udpLocalAddr, sizeof(udpLocalAddr)))
	{
		cerr << "bind call failure" << endl;
		return false;
	}
	else//multi cast
	{
		if (iFirstBit >= 224 && iFirstBit <= 239)
		{
			//TODO
		}
		return true;
	}
}

int32 sRecv(char *pBuffer, int32 sBufferMaxSize, SOCKET SocketID, SOCKET_TYPE type, RECVMODEL model)
{
	//check the initial conditions
	if (SocketID <= 0)
		return -2;
	if (type != TCPSOCKET && type != UDPSOCKET)
		return -2;
	if (model != NONBLOCK && model != BLOCK)
		return -2;
	if (sBufferMaxSize < 0)
	{
		printf("buffer size less than 0!\n");
		return -2;
	}

	int iRecvModel = 0;
	if (model == NONBLOCK)
		SetSocketNonblock(SocketID);
	else if (model == BLOCK)
		SetSocketBlock(SocketID);

	//initialize the sockaddr_in structure
	sockaddr_in fMsgAddr;
	memset(&fMsgAddr, 0, (size_t)sizeof(fMsgAddr));	
	int addrLen = sizeof(fMsgAddr);

	//clear all the data of the receive buffer
	//memset(pBuffer, 0, sBufferMaxSize);

	//call the select function
	int tmprc = 0;
	if (type == UDPSOCKET)
		tmprc = recvfrom(SocketID, (char *)pBuffer, (int)(sBufferMaxSize), iRecvModel, (sockaddr *)&fMsgAddr, &addrLen);
	else if (type == TCPSOCKET)
		tmprc = recv(SocketID, (char *)pBuffer,(int)(sBufferMaxSize), iRecvModel);

	if (tmprc == -1)
	{
		if (model == NONBLOCK)
		{
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK)
				return 0;
			else
				return -1;
		}
	}
	//else
	//	pBuffer[tmprc] = 0;
	return tmprc;
}

//UDP send function, use the select model
int16 sUDPSend(string strRemoteAddr, int16 sRemotePort, const char *pBuffer, int16 sBufferSize, SOCKET udpSocketID)
{
	//initialize the sockaddr_in structure
	sockaddr_in fMsgAddr;
	memset(&fMsgAddr, 0, (size_t)sizeof(fMsgAddr));	
	int addrLen = sizeof(fMsgAddr);

	//set the remote address and port to send the udp data
	fMsgAddr.sin_family = AF_INET;
	fMsgAddr.sin_addr.s_addr = inet_addr(strRemoteAddr.c_str());
	fMsgAddr.sin_port = htons(sRemotePort);

	struct timeval tmpTime;
	tmpTime.tv_sec = 0;
	tmpTime.tv_usec= 10;
	fd_set tmpFd;
	FD_ZERO(&tmpFd);
	FD_SET(udpSocketID,&tmpFd);

	//call the select function
	int tmprc = 0;
	if ((tmprc = select(0, NULL, &tmpFd, NULL, &tmpTime)) == 1)
		tmprc = sendto(udpSocketID, pBuffer, (int)sBufferSize, 0, (sockaddr *)&fMsgAddr, addrLen);
	else
		tmprc = 0;

	//return value
	return tmprc;
}

int16 sTCPSend(const char *pBuffer, int16 sBufferSize, SOCKET tcpSocketID)
{
	//set the fd_set and time value
	struct timeval tv = {0,10};
	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(tcpSocketID, &fd);
	int selectret = select(0, NULL, &fd, NULL, &tv);


	//detect the select result
	if(selectret < 0)
	{
		//select operation error
		return -1;
	}
	else if (selectret == 0)
	{
		//select operation timeout
		return 0;
	}
	else
	{
		// send to server
		int rc = send(tcpSocketID, pBuffer, sBufferSize, 0);
		if(rc == -1)
		{
			//error
			return -1;
		}
		else if (rc != sBufferSize)
		{
			//error
			return -1;
		}
		else
		{
			//success
			return rc;
		}
	}
}

//set the socket to non-block model
//bool ClearSocket::SetSocketNonblock(int &tcpSocketID)
bool SetSocketNonblock(SOCKET tcpSocketID)
{
	//-------------------------
	// Set the socket I/O mode: In this case FIONBIO
	// enables or disables the blocking mode for the 
	// socket based on the numerical value of iMode.
	// If iMode = 0, blocking is enabled; 
	// If iMode != 0, non-blocking mode is enabled.
	int iMode = 1;
	int ret = ioctlsocket(tcpSocketID, FIONBIO, (u_long FAR*) &iMode);
	if (0 == ret)
		return true;
	else
		return false;
}

//set the socket to block model
bool SetSocketBlock(SOCKET tcpSocketID)
{
	int iMode = 0;
	int ret = ioctlsocket(tcpSocketID, FIONBIO, (u_long FAR*) &iMode);
	if (0 == ret)
		return true;
	else
		return false;
}

//set the port of reuse
bool SetReusePort(SOCKET SocketID)
{
	int on = 1;
	int ret = setsockopt(SocketID, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(int));
	if (ret == 0)
		return true;
	else
		return false;
}

bool SetSNDBUFSize(SOCKET SocketID, int iSize)
{
	int ret = setsockopt(SocketID, SOL_SOCKET, SO_SNDBUF, (const char *)&iSize, sizeof(int));
	if (ret == 0)
		return true;
	else
		return false;
}

bool SetRCVBUFSize(SOCKET SocketID, int iSize)
{
	int ret = setsockopt(SocketID, SOL_SOCKET, SO_RCVBUF, (const char*)&iSize, sizeof(int));
	if (ret == 0)
		return true;
	else
		return false;
}