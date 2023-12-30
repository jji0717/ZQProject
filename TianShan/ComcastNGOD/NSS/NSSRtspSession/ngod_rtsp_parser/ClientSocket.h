/*
 * =====================================================================================
 * 
 *       Filename:  ClientSocket.h
 * 
 *    Description:   
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

#ifndef  MYSOCKET_FILE_HEADER_INC
#define  MYSOCKET_FILE_HEADER_INC

//socket header file
#include <winsock2.h>
#include <errno.h>

#include "Common.h"

#ifndef WIN32
typedef int SOCKET;
#endif

typedef enum {NONBLOCK, BLOCK}RECVMODEL;

typedef enum {TCPSOCKET, UDPSOCKET} SOCKET_TYPE;

const int UDPDefaultSize = 1500;
const int SNDBUFDefaultSize = 1024*1024*2;
const int RCVBUFDefaultSize = 1024*1024*2;

//initialize socket environment under Windows OS
bool InitialSocket();

//clean up all socket environment under Windows OS
bool CleanupSocket();

//create socket and close socket(release resource)
//on success,return the socket number, otherwise return -1, if type wrong will return -2;
SOCKET CreateSocket(SOCKET_TYPE type);

//close the specified socket, on success return TRUE, otherwise return FALSE;
bool CloseSocket(SOCKET SockID);

//operation of TCP socket
//connect to remote address's specified port, on success return TRUE, otherwise return FALSE; the usTimeout is the time out value set in connection, we will abort the connecting process after usTimeout seconds
bool bConnection(string  strIP,  int16 sPort, SOCKET tcpSocketID, int16 sTimeOut);

//bind to the specified strBindIP with the port of usPort, on success return TRUE, otherwise return FALSE;
bool Bind(string strRemoteAddr, int16 sRemotePort, SOCKET SocketID);

//recv from the specified SocketID and write to the pBuffer, on success return the bytes received, otherwise return -1;
int32 sRecv(char *pBuffer, int32 sBufferMaxSize, SOCKET SocketID, SOCKET_TYPE type, RECVMODEL model);

//operation of TCP socket
//Send to the specified tcpSocketID from the pBuffer with the length of usBufferSize, on success, return the send length, otherwise return 0(timeout) or -1;
int16 sTCPSend(const char *pBuffer, int16 sBufferSize, SOCKET tcpSocketID);

//operation of UDP socket
//send to the specified remote address' port,on success return the SendBuffer Size, otherwise return 0(timeout) or -1(error);
int16 sUDPSend(string strRemoteAddr, int16 sRemotePort, const char *pBuffer, int16 sBufferSize, SOCKET udpSocketID);

//set socket option
//set the socket to the non block model or block model
bool SetSocketNonblock(SOCKET tcpSocketID);
bool SetSocketBlock(SOCKET tcpSocketID);

//set the port to reuse
bool SetReusePort(SOCKET SocketID);

//set the SND_BUF and RCV_BUF
bool SetSNDBUFSize(SOCKET SocketID, int iSize);
bool SetRCVBUFSize(SOCKET SocketID, int iSize);
#endif   /* ----- #ifndef MYSOCKET_FILE_HEADER_INC  ----- */
