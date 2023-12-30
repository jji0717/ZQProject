// File Name: SocketAPI.h
// Description: define socket interface
// Date: 2008-12

#ifndef __SOCKETAPI_H__
#define __SOCKETAPI_H__

#include "ZQ_common_conf.h"

#define MSG_TYPE 0

namespace SocketAPI
{
	class SocketInterface
	{
	public:
		SocketInterface();
		~SocketInterface();
	public:
#ifdef ZQ_OS_MSWIN
		//@Function	get the error description of the specified error code.
		//@Param	char** lpMsgBuf:	second pointer to retrive the error description.
		//@Param	uint32 dwErrorcode:	specified error code.
		//@Return	true if successful to retrive error description, else return false.
		//@Usage    char* lpMsgBuf, getLastErrorDes(&lpMsgBuf);
		//@Remark	You must call ReleaseMessage to realse the memory allocated by this function.
		static bool getLastErrorDes(char** lpMsgBuf, uint32 dwErrorCode);

		//@Function	release the error description gotton by GetLastErrorDes.
		//@Param	char* lMsgBuf:	error description gotton by GetLastErrorDes.
		//@Return	NULL.
		static void releaseMessage(char* lpMsgBuf);

#else
		static std::string getLastErrorDes(uint32 nerrnum);
		
		//return errno
		int WSAGetLastError();
#endif
		//@Function	change ip address(192.168.80.91) to 4 uint8(192 168 80 91)
		//@Return	true if successful to change, else return false.
		static bool IP2B4(char* pIP, uint8* b1, uint8* b2, uint8* b3, uint8* b4);

		//@Function	load socket dynamic library.
		//@Return	true if successful init, else return false.
		static bool initSocket(uint32& dwError);

		//@Function	unload socket dynamic library.
		//@Return	void
		static void uninitSocket();

		//@Function: create socket.
		//@Param:    std::string& localAddress, bind with this address
		//@Param:    uint32& dwError, error code.
		//@Param:    short sPort, bind with this port
		//@Return:   return true if successs, false if fail
		bool createSocket(uint32& dwError, const std::string strLoaclAddress = "", const short sPort = 0);

		//@Function: close socket
		//@Return:   void
		void closeSocket();

		//@Function: disables sends or receives on a socket
		//@Return:   void
		void shutdownSocket();

		//@Function: socket connect server.
		//@Param:    string strServer, server's IP address
		//@Param:    short sPort, server's IP port
		//@Param:    uint32& dwError, error code.
		//@Return:   return true if successs, false if fail
		bool connectServer(const std::string& strServer, const short sPort, uint32& dwError);

		//@Function	get socket ip address and its port.
		//@Param	specified socket handle.
		//@Param	char* pAddress: string to retrive socket ip address.
		//@Param	int&  pPort: pointer to retrive socket port.
		//@Param	uint32& dwError: pointer to retrive error code.
		//@Retrun	true if successful to retrive, else return false.
	    bool getSocketName(char* pAddress, int& pPort, uint32& dwError);

		//@Function: server socket listen for connection
		//@param:   int nBacklog, server socket can accept max connection
		//@Param:   uint32& dwError, error code.
		//@Return:  true if successful to listen, else return false
		bool listenRequest(int nBacklog, uint32& dwError);

		//@Function:accept client socket connection request
		//@Param:   uint32& dwError, error code
		//@Return:  return socket if successful,else return INVALID_SOCKET.
	    bool acceptSocket(SocketInterface& socketAccept, uint32& dwError);

	    //@Function	check if there are some data in the socket to read.
		//@Param	uint32 dwTimeout: time out in mill seconds.
		//@Return	It returns SOCKET_ERROR if error occurs;
		//			return 1 if some data are in the socket; 
		//			return 0 if time out.
		int checkRead(uint32 dwTimeout, uint32& dwError);

		//@Function	check if spcified socket can write data.
		//@Param	uint32 dwTimeout: time out in mill seconds.
		//@Return	It returns SOCKET_ERROR if error occurs;
		//			return 1 if this socket can write; 
		//			return 0 if time out.
		int checkWrite(uint32 dwTimeout, uint32& dwError);

		//@Function	send specified length of data to peer socket until complete data is received, error, or timeout
		//@Param	char* pBuffer: data buffer to retrive data.
		//@Param	int nLen: bytes number to retrive.
		//@Return	0  successful to receive complete data from peer socket.
		//			1  timeout.
		//			2  peer socket has gracefully closed.
		//		   -1  error occurs while sending these data.
	    int completeSend(char* pBuffer, int nLen, uint32 dwTimeout, uint32& pdwError);

		//@Function receive datas from peer socket
		//@Param	char* pBuffer: data buffer to retrive data.
		//@Param	int nLen: bytes number to retrive.
		//@Return	>=1  successful to receive complete data from peer socket.
		//			0  peer socket has gracefully closed.
		//		    -1  error occurs while sending these data.
		int receiveDatas(char* pBuffer, int nLen, uint32 dwTimeout, uint32& pdwError);

	private:
#ifdef ZQ_OS_MSWIN
		SOCKET _hSocket;
#else
		int _hSocket;
#endif
	};
}

#endif

