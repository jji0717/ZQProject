// File Name: SocketAPI.cpp
// Description: implement socket interface
// Date: 2008-12

#include <string>
#include "SocketAPI.h"

#ifdef ZQ_OS_MSWIN
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
extern "C"
{
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
}
#ifndef INVALID_SOCKET
	#define INVALID_SOCKET -1
#endif
#ifndef SOCKET_ERROR
	#define SOCKET_ERROR -1
#endif
#ifndef INADDR_NONE 
	#define INADDR_NONE -1
#endif
#endif

namespace SocketAPI
{
	SocketInterface::SocketInterface()
		:_hSocket(INVALID_SOCKET)

	{
	}

	SocketInterface::~SocketInterface()
	{
		closeSocket(); // to ensure socket be closed
	}

#ifdef ZQ_OS_MSWIN
	//@Function	get the error description of the specified error code.
	//@Param	char** lpMsgBuf:	second pointer to retrive the error description.
	//@Param	uint32 dwErrorcode:	specified error code.
	//@Return	True if successful to retrive error description, else return False.
	//@Remark	You must call ReleaseMessage to realse the memory allocated by this function.
	bool SocketInterface::getLastErrorDes(char** lpMsgBuf, uint32 dwErrorCode)
	{
		*lpMsgBuf = 0;
		if (!FormatMessageA( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwErrorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR) lpMsgBuf,
			0,
			NULL ))
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	//@Function	release the error description gotton by GetLastErrorDes.
	//@Param	char* lMsgBuf:	error description gotton by GetLastErrorDes.
	//@Return	NULL.
	void SocketInterface::releaseMessage(char* lpMsgBuf)
	{
		if( lpMsgBuf != 0)
		{
			LocalFree(lpMsgBuf);
			lpMsgBuf = 0;
		}
	}

	//@Function	init socket dynamic library.
	//@Return	true if successful init, else return false.
	bool SocketInterface::initSocket(uint32& dwError)
	{
		dwError = 0;
		WSAData wsaData;
		WORD wVersionRequested = MAKEWORD(2, 2);
		if( WSAStartup(wVersionRequested, &wsaData) != 0 )
		{
			dwError = WSAGetLastError();
			return false;
		}
		if (wsaData.wVersion != wVersionRequested)
		{
			dwError = WSAGetLastError();
			WSACleanup();
			return false;
		}
		return true;
	}

	//@Function	uninit socket dynamic library.
    //@Return	void
	void SocketInterface::uninitSocket()
	{
		WSACleanup();
	}

#else
	std::string SocketInterface::getLastErrorDes(uint32 nerrnum)
	{
		std::string strErr;
		char buf[256] = {0};
		sprintf(buf,"Error code [%u], ",nerrnum);
		strErr = buf;
		memset(buf,0,sizeof(buf));
		char* pbuf = strerror_r(nerrnum,buf,sizeof(buf));
		if(pbuf)
			strErr += pbuf;	
		return strErr;
	}

	//@Function	init socket dynamic library.
	//@Return	true if successful init, else return false.
	bool SocketInterface::initSocket(uint32& dwError)
	{
		return true;
	}

	void SocketInterface::uninitSocket()
	{
	}
	
	int SocketInterface::WSAGetLastError()
	{
		return errno;
	}
#endif

	//@Function: create socket.
	//@Param:    std::string& localAddress, bind with this address
	//@Param:    uint32& dwError, error code.
	//@Param:    short sPort, bind with this port
	//@Return:   return true if successs, false if fail
	bool SocketInterface::createSocket(uint32& dwError, const std::string strLocalAddress, const short sPort)
	{
		dwError = 0;
		_hSocket = socket(AF_INET, SOCK_STREAM, 0);  // IPPROTO_TCP 6, 0 server choice protocol
		if( INVALID_SOCKET == _hSocket )
		{
			dwError = WSAGetLastError();
			return false;
		}
		unsigned long uLong = inet_addr(strLocalAddress.c_str());
		if(INADDR_NONE == uLong)
		{
			uLong = htonl(INADDR_ANY);
		}
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = sPort;
#ifdef ZQ_OS_MSWIN
		addr.sin_addr.S_un.S_addr = uLong;
#else
		addr.sin_addr.s_addr = uLong;
#endif
		if(bind(_hSocket, (struct sockaddr*)&addr, sizeof(addr)) != 0)
		{
			dwError = WSAGetLastError();
			closeSocket();
			return false;
		}
		return true;
	}

	//@Function: close socket
	//@Return:   void
	void SocketInterface::closeSocket()
	{
		if (INVALID_SOCKET != _hSocket)
		{
#ifdef ZQ_OS_MSWIN
			closesocket(_hSocket);
#else
			close(_hSocket);
#endif
			_hSocket = INVALID_SOCKET;
		}
	}

	void SocketInterface::shutdownSocket()
	{
		if (INVALID_SOCKET != _hSocket)
		{
#ifdef ZQ_OS_MSWIN
			shutdown(_hSocket, SD_SEND);
#else
			shutdown(_hSocket, SHUT_WR);
#endif
		} 
	}
	//@Function: socket connect server.
	//@Param:    string strServer, server's IP address
	//@Param:    short sPort, server's IP port
	//@Param:    uint32& dwError, error code.
	//@Return:   return true if successs, false if fail
	bool SocketInterface::connectServer(const std::string& strServer, const short sPort, uint32& dwError)
	{
		int nCon = 0;
		dwError = 0;
		unsigned long uLong = inet_addr(strServer.c_str());
		if(INADDR_NONE == uLong)
		{
			dwError = WSAGetLastError();
			return false;
		}
		sockaddr_in addr;	
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(sPort);
#ifdef ZQ_OS_MSWIN
		addr.sin_addr.S_un.S_addr = uLong;
#else
		addr.sin_addr.s_addr = uLong;
#endif
		nCon = connect(_hSocket, (struct sockaddr*)&addr, sizeof(addr));
		if(nCon != 0)
		{
			dwError = WSAGetLastError();
			return false;
		}
		return true;
	}

	//@Function	check if there are some data in the socket to read.
	//@Param	uint32 dwTimeout: time out in mill seconds.
	//@Return	It returns SOCKET_ERROR if error occurs;
	//			return 1 if some data are in the socket; 
	//			return 0 if time out.
	int SocketInterface::checkRead(uint32 dwTimeout, uint32& dwError)
	{
#ifdef ZQ_OS_MSWIN
		dwError = 0;
		fd_set setCheck;
		timeval timeout;
		timeout.tv_sec = dwTimeout/1000; 
		timeout.tv_usec = dwTimeout - dwTimeout/1000*1000;
		FD_ZERO(&setCheck);
		FD_SET( _hSocket, &setCheck );
		int nRet = select(0, &setCheck, 0, 0, &timeout);
		if( SOCKET_ERROR == nRet)
		{
			dwError = WSAGetLastError();
		}
		return nRet;
#else
		struct pollfd pfd;
		pfd.fd = _hSocket;
		pfd.events = POLLIN;
		int iRet = poll(&pfd,1,dwTimeout);
		if(iRet == -1)
			dwError = WSAGetLastError();
		return iRet;
#endif
	}

	//@Function	check if spcified socket can write data.
	//@Param	uint32 dwTimeout: time out in mill seconds.
	//@Return	It returns SOCKET_ERROR if error occurs;
	//			return 1 if this socket can write; 
	//			return 0 if time out.
	int SocketInterface::checkWrite(uint32 dwTimeout, uint32& dwError)
	{
#ifdef ZQ_OS_MSWIN
		dwError = 0;
		fd_set setCheck;
		timeval timeout;
		timeout.tv_sec = dwTimeout/1000; 
		timeout.tv_usec = dwTimeout - dwTimeout/1000*1000;
		FD_ZERO(&setCheck);
		FD_SET( _hSocket, &setCheck );
		int nRet = select(0, 0, &setCheck, 0, &timeout);
		if(SOCKET_ERROR == nRet)
		{
			dwError = WSAGetLastError();
		}
		return nRet;
#else
		struct pollfd pfd;
        pfd.fd = _hSocket;
		pfd.events = POLLOUT;
		int iRet = poll(&pfd,1,dwTimeout);
		if(iRet == -1)
			dwError = WSAGetLastError();
		return iRet;
#endif
	}

	//@Function	send specified length of data to peer socket until complete data is received, error, or timeout
	//@Param	char* pBuffer: data buffer to retrive data.
	//@Param	int nLen: bytes number to retrive.
	//@Return	0  successful to send complete data to peer socket.
	//			1  timeout.
	//			2  peer socket has gracefully closed.
	//		   -1  error occurs while sending these data.
	int SocketInterface::completeSend(char* pBuffer, int nLen, uint32 dwTimeout, uint32& pdwError)
	{
		int			nStatus = 0;		// check read socket status.
		int			nPosition = 0;		// current position to store data.
		int			nRet;				// return value of recvfrom.
		uint32		dwTimePer = 1000;
		uint32		dwTimeUsed = 0;
		pdwError = 0;
		while(nPosition < nLen)
		{
			// check if socket can write.
			nStatus = SocketInterface::checkWrite(dwTimePer, pdwError);
			if(1 == nStatus) //if there are some data to write.
			{
				nRet = send( _hSocket, pBuffer + nPosition, nLen - nPosition, MSG_TYPE );
				if(0 == nRet ) //if peer socket has gracefully closed.
				{
					pdwError = WSAGetLastError();
					return 2;
				}
				else if(SOCKET_ERROR == nRet) //if error occurs while sending data to peer socket.
				{
					pdwError = WSAGetLastError();
					return -1;
				}
				else //successful to send data from peer socket.
				{
					nPosition += nRet;
					dwTimeUsed = 0;
				}
			}
			else if( SOCKET_ERROR == nStatus ) //if error occurs while check read socket.
			{
				pdwError = WSAGetLastError();
				return -1;
			}
			else // if time out
			{
				dwTimeUsed += dwTimePer;
				if( dwTimeUsed > dwTimeout )
				{
					return 1;
				}
			}
		}
		return 0;
	}

	//@Function receive datas from peer socket
	//@Param	char* pBuffer: data buffer to retrive data.
	//@Param	int nLen: bytes number to retrive.
	//@Return	>=1 total receive bytes, successful to receive complete data from peer socket.
	//          -3  time out
	//			-2  peer socket has gracefully closed.
	//		    -1  error occurs while sending these data.
	int SocketInterface::receiveDatas(char *pBuffer, int nLen, uint32 dwTimeout, uint32& pdwError)
	{
		pdwError = 0;
		int nLeft = nLen;
		int nReceive = 0;
		int nCheckRead = 0;
		char *pCurPos = pBuffer;
		while (nLeft > 0)
		{
			nCheckRead = SocketInterface::checkRead(dwTimeout, pdwError);
			if (nCheckRead > 0) // have some datas to read
			{
				nReceive = recv(_hSocket, pCurPos, nLeft, 0);
				if (nReceive <= 0)
				{
					pdwError = WSAGetLastError();
					break;
				}
				nLeft -= nReceive;
				pCurPos += nReceive;
			}
			else
			{
				if (SOCKET_ERROR == nCheckRead) // socket error
				{
					pdwError = WSAGetLastError();
				}
				break; // time out
			}
		}// end for while
		return (nLen - nLeft);    // return >= 0
	}


	//@Function	get socket ip address and its port.
	//@Param	char* pAddress: string to retrive socket ip address.
	//@Param	int&  pPort: pointer to retrive socket port.
	//@Param	uint32& dwError: pointer to retrive error code.
	//@Retrun	TRUE if successful to retrive, else return FALSE.
	bool SocketInterface::getSocketName(char* pAddress, int& pPort, uint32& dwError)
	{
		dwError = 0;
		struct sockaddr_in address;
		int nLen = sizeof(address);
#ifdef ZQ_OS_MSWIN
		int nRet = getsockname(_hSocket, (struct sockaddr*)(&address), &nLen);
#else
		int nRet = getsockname(_hSocket, (struct sockaddr*)(&address), (socklen_t*)&nLen);
#endif
		if(nRet != 0)
		{
			dwError = WSAGetLastError();
			return false;
		}
		pPort = address.sin_port;
		char* pTmp = inet_ntoa(address.sin_addr);
		if( NULL == pTmp )
		{
			pPort = 0;
			dwError = WSAGetLastError();
			return false;
		}
		pPort = ntohs(pPort);
		strcpy( pAddress, pTmp );
		return true;
	}

	//@Function	change ip address(192.168.80.91) to 4 BYTE(192 168 80 91)
	//@Return	TRUE if successful to change, else return FASLE.
	bool SocketInterface::IP2B4(char* pIP, uint8* b1,uint8* b2,uint8* b3,uint8* b4)
	{
		char* p1;
		char* p2;

		//192
		p2 = pIP;
		p1 = p2;
		while( *p2 != '.' && *p2 != '\0' )
		{
			p2++;
		}
		if( *p2 != '.' )
			return false;
		*p2 = '\0';
		*b1 = atoi( p1 );
		*p2 = '.';
		p2++;

		//168
		p1 = p2;
		while( *p2 != '.' && *p2 != '\0' )
		{
			p2++;
		}
		if( *p2!='.' )
			return false;
		*p2 = '\0';
		*b2 = atoi( p1 );
		*p2 = '.';
		p2++;

		//80
		p1 = p2;
		while( *p2 != '.' && *p2 != '\0' )
		{
			p2++;
		}
		if( *p2 != '.' )
			return false;
		*p2 = '\0';
		*b3 = atoi( p1 );
		*p2 = '.';
		p2++;

		//91
		*b4 = atoi(p2);
		return true;
	}


	//@Function: server socket listen for connection
	//@param:   int nBacklog, server socket can accept max connection
	//@Param:   uint32& dwError, error code.
	//@Return:  true if successful to listen, else return false
	bool SocketInterface::listenRequest(int nBacklog, uint32& dwError)
	{
		dwError = 0;
		int nRet = listen(_hSocket, nBacklog);
		if( nRet != 0 )
		{
			dwError = WSAGetLastError();
			return false;
		}
		return true;
	}

	//@Function:accept client socket connection request
	//@Param:   uint32& dwError, error code
	//@Return:  return socket if successful,else return INVALID_SOCKET.
	bool SocketInterface::acceptSocket(SocketInterface& socketAccept, uint32& dwError)
	{
	    
		uint32 dwTimeout = 4000;
		int nCheckRead = SocketInterface::checkRead(dwTimeout, dwError);
		if (nCheckRead <= 0) 
		{
			return false;
		}
		// accept socket connection
		dwError = 0;
		sockaddr addr;
		int nLen = sizeof(addr);
#ifdef ZQ_OS_MSWIN
		SOCKET sktTmp = accept(_hSocket, &addr, &nLen);
#else
		int sktTmp = accept(_hSocket, &addr, (socklen_t*)&nLen);
#endif
		if(INVALID_SOCKET == sktTmp)
		{
			dwError = WSAGetLastError();
			return false;
		}
		// set socket option
		linger  Linger;
		Linger.l_onoff = 1;
		Linger.l_linger = 0xFFFFFFFF;
		int  result = setsockopt(sktTmp, SOL_SOCKET, SO_LINGER, (char *)&Linger, sizeof(linger));
		if (SOCKET_ERROR == result)
		{
			dwError = WSAGetLastError();
			return false;
		}
		socketAccept._hSocket = sktTmp;
		return true;
	}




	// -----------------------------
	// class init_WSA
	// -----------------------------
	class init_WSA
	{
	public:
		init_WSA();
		~init_WSA();
	};

	init_WSA::init_WSA()
	{
		//-initialize OS socket resources!
		uint32 _dwError;
		if (!SocketInterface::initSocket(_dwError))
		{
			printf("Fail to load socket dynamic library. Error code : [%d]", _dwError);
			abort();
		}
	};

	init_WSA::~init_WSA() 
	{ 
		SocketInterface::uninitSocket(); 
	} 

	/// a static instance to init winsock
	static init_WSA init_wsa;

}
