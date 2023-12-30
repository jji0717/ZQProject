#include <assert.h>

#include "GBsvcConfig.h"
#include "IOCPThreadPool.h"
#include "SocketClientContext.h"
#include "HttpClientContext.h"
#include "TCPSocket.h"


extern ZQ::common::Config::Loader<ZQTianShan::GBServerNS::GBServerConfig > gConfig;

namespace ZQTianShan {	  
namespace GBServerNS { 

int ReleaseResource::run()
{
	if (NULL != _dataKey)
	{
		::shutdown(_dataKey->_clientSocket, SD_BOTH);
		::closesocket(_dataKey->_clientSocket);
		delete _dataKey;
		_dataKey = NULL;
	}

	return 0;
}


int  ProcessIncoming::run()
{ 
	ClientSocketContext* dataKey = (ClientSocketContext*)_dataKey;
	assert( dataKey != NULL );
	
	HttpContext   httpClientContext(_releasePool, _log); 	
	if(httpClientContext.obtainHttpContent(dataKey->_buf, dataKey->_bufUsed))
	{
		httpClientContext.parseHttpContent();
	}

	httpClientContext.reponseHttpClient(dataKey);

	return 0;
}


int AssociateWithIOCP::run()
{
	if (NULL == _pContext)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(AssociateWithIOCP,"associateWithIOCP failed to CreateIoCompletionPort, errorCode[NULL]"));
	    return 0;
	}

	ClientSocketContext *pContext = _pContext;
	HANDLE hTemp = CreateIoCompletionPort((HANDLE)pContext->_clientSocket, pContext->_completePort, (DWORD)pContext, 0);
	if (NULL == hTemp)
	{
		(new ReleaseResource(_releasePool, pContext))->start();
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(AssociateWithIOCP,"associateWithIOCP failed to CreateIoCompletionPort, errorCode[%u]"), WSAGetLastError());
		return false;
	}

	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	WSABUF * p_wbuf = &(pContext->_wsaBuf);
	OVERLAPPED *p_ol = &(pContext->_overlapped);
	pContext->_comPortType = RECV;
	int nBytesRecv = WSARecv(pContext->_clientSocket, p_wbuf, 1, &dwBytes, &dwFlags, p_ol, NULL );

	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError()))
	{
		int a =  WSAGetLastError();
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(AssociateWithIOCP,"associateWithIOCP failed to WSARecv client[%s], errorCode[%u]"), pContext->_clientAddr.sa_data, a);
		(new ReleaseResource(_releasePool, pContext))->start();
		return false;
	}

	return true;
}

int FeedBack::run()
{
	using namespace ZQ::common;

#if 0
	timeout_t timeout = 10000;
	int serverPort = 4444;
	InetAddress    feedbackAddress("192.168.81.108");
	TCPClient*     pTcpClient = new TCPClient;
	HttpFeedBackContext feedBack(_contentInf, _ngbCmdCode);
	boost::shared_ptr< TCPClient > tcpClientGuard(pTcpClient);

	std::string feedBackStr = feedBack.preHttpFeedBack();
	pTcpClient->setPeer(feedbackAddress, serverPort);
	pTcpClient->setCompletion(true);
	pTcpClient->connect();
	if(!pTcpClient->isPending(Socket::pendingOutput, timeout))
	{
		//pTcpClient->disconnect();
		return 0;					 
	}

	pTcpClient->send((void*)feedBackStr.c_str(), feedBackStr.size());
	if(!pTcpClient->isPending(Socket::pendingInput, timeout))
	{
		//pTcpClient->disconnect();
		return 0;					 
	}

	const int recvSize = 2048;
	char recvBuf[recvSize] = {0};
	pTcpClient->receive((void *)recvBuf, recvSize);
	//pTcpClient->disconnect();
#else
	struct sockaddr_in serverAddress = {0};
	SOCKET so = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(INVALID_SOCKET == so)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(FeedBack,"create socket failed, error[%u], ngbcmd[%s]"), GetLastError(), _contentInf._opCodeInXml.c_str());
		return 0;
	}

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(gConfig._gbSvcBase._feedBackPort);
	serverAddress.sin_addr.s_addr = inet_addr(gConfig._gbSvcBase._feedBackIP.c_str()); 
	if (SOCKET_ERROR == ::connect(so, reinterpret_cast<const struct sockaddr *>(&serverAddress), sizeof(serverAddress))) 
	{
		::closesocket(so);
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(FeedBack,"connect failed, error[%u], ngbcmd[%s]"), GetLastError(), _contentInf._opCodeInXml.c_str());
		return false; 
	}

	unsigned long mode = 1;//no blcok
	::ioctlsocket(so, FIONBIO, &mode);

	int status;
	struct timeval tv;
	struct timeval *tvp = &tv;
	fd_set grp;

	tv.tv_usec = 2000;
	FD_ZERO(&grp);
	FD_SET(so, &grp);
	status = select((int)so + 1, NULL, &grp, NULL, tvp);
	if(status < 1 || !FD_ISSET(so, &grp))
	{
		::closesocket(so);
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(FeedBack,"select before send failed, error[%u], ngbcmd[%s]"), GetLastError(), _contentInf._opCodeInXml.c_str());
		return false;
	}

	HttpFeedBackContext feedBack(_contentInf, _ngbCmdCode);
	std::string feedBackStr = feedBack.preHttpFeedBack();
	int nBytesSent = send(so, feedBackStr.c_str(), feedBackStr.length(), 0);
	if (SOCKET_ERROR == nBytesSent) 
	{
		::closesocket(so);
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(FeedBack,"send failed, error[%u], ngbcmd[%s]"), GetLastError(), _contentInf._opCodeInXml.c_str());
		return false; 
	}
	
	FD_ZERO(&grp);
	FD_SET(so, &grp);
	status = select((int)so + 1, &grp, NULL, NULL, tvp);
	if(status < 1 || !FD_ISSET(so, &grp))
	{
		::closesocket(so);
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(FeedBack,"select before recv failed, error[%u], ngbcmd[%s]"), GetLastError(), _contentInf._opCodeInXml.c_str());
		return false;
	}

	const int bufLen = 2048;
	char buf[bufLen] = {0};
	int ret = ::recv(so, (char *)buf, bufLen, 0);
	if (SOCKET_ERROR == ret) 
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(FeedBack,"recv failed, error[%u], ngbcmd[%s]"), GetLastError(), _contentInf._opCodeInXml.c_str());
	else
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(FeedBack,"recv succeed, Buf[%s]"), buf);

	::shutdown(so, SD_BOTH);
	::closesocket(so);
#endif//0
	return 0;
}

bool ConnectDispatch::init(void)
{
	_running = true;
	return true;
}

int  ConnectDispatch::run(void) 
{
	DWORD					dwOpByte		= 0;
	LONG_PTR				completionKey	= NULL;
	LPOVERLAPPED			overLapped		= NULL;
	ClientSocketContext *   dataKey			= NULL;
	while(_running)
	{
		completionKey = NULL;
		overLapped = NULL;
		dataKey = NULL;
		dwOpByte = 0;

		bool bRet = false;
		bRet =  GetQueuedCompletionStatus(_hIOCompletionPort, &dwOpByte, (PULONG_PTR) &completionKey, &overLapped, INFINITE/*60 * 1000 */);
		if (IOCP_EXIT_CODE == completionKey)
		{
		    _running = false;
			continue;
		}

		if(bRet)		
		{
			assert( completionKey != NULL && overLapped != NULL );
			dataKey = (ClientSocketContext*)completionKey;
			dataKey->_bufUsed = dwOpByte;
			if(0 != dwOpByte && RECV == dataKey->_comPortType)//just care of RECV
			{
				ProcessIncoming * handleProc = new ProcessIncoming(_processPool, _releasePool, dataKey, _log);
				handleProc->start();
			}
			else //if (WREATE == dataKey->_comPortType)	 
			{    //dwOpByte == 0 means client closed
				ReleaseResource * handleRelease = new ReleaseResource(_releasePool, dataKey);
				handleRelease->start();
			}
		}
	}
	return 0;
}

}//GBServerNS
}//	ZQTianShan