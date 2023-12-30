#include "NPVRSocket.h"

#include "Log.h"
#include "ScLog.h"
using namespace ZQ::common;

NPVRSocket::NPVRSocket()
{
	WSADATA wsadata;
	WORD version = MAKEWORD(2, 0);
	int ret = WSAStartup(version, &wsadata);
	if (ret != 0)
	{
		glog(Log::L_ERROR, "WSAStartup failed <%d>", ret);
		return;
	}
	
	if(m_socket != NULL)
	{
		closesocket(m_socket);
		m_socket = NULL;
	}

	if (m_hSocket != NULL)
	{
		closesocket(m_hSocket);
		m_hSocket = NULL;
	}
}

bool NPVRSocket::InitialSocket()
{
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == NULL)
	{
		int ret = WSAGetLastError();
		glog(Log::L_ERROR, "socket() <%d>", ret);
		return false;
	}
	return true;
}

NPVRSocket::~NPVRSocket()
{
	if (m_socket != NULL)
	{
		closesocket(m_socket);
		m_socket = NULL;
	}
	if (m_hSocket != NULL)
	{
		closesocket(m_hSocket);
		m_hSocket = NULL;
	}
}

// client function
bool NPVRSocket::Connect(int port, char* ip)
{
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.S_un.S_addr = inet_addr(ip);
	m_addr.sin_port = htons(port);

	int ret = connect(m_socket, (LPSOCKADDR)&m_addr, sizeof(m_addr));
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		glog(Log::L_DEBUG, "connect() <%d>", err);
		if (err == WSAEISCONN)
		{
			return true;
		}
		return false;
	}
	return true;
}

bool NPVRSocket::SendC(char* buf, int bufLen)
{
	int ret = send(m_socket, buf, bufLen, 0);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK)
		{
			glog(Log::L_ERROR, "SendC() <%d>", err);
		}
		return false;
	}
	return true;
}

bool NPVRSocket::RecvC(char* buf, int bufLen)
{
	int ret = recv(m_socket, buf, bufLen, 0);
	if (ret == 0)
	{
		glog(Log::L_DEBUG, "connection has been gracefully closed");
		return false;
	}
	else if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK)
		{
			glog(Log::L_ERROR, "RecvC() <%d>", err);
		}
		return false;
	}
	return true;
}

bool NPVRSocket::WSASelectC(WSAEVENT& hWSAEvent, long lNetworkEvents)
{
	if (m_socket == NULL)
	{
		m_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (m_socket == NULL)
		{
			glog(Log::L_ERROR, "socket() <%d>", WSAGetLastError());
			return false;
		}
	}
	
	int ret = WSAEventSelect(m_socket, hWSAEvent, lNetworkEvents);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		glog(Log::L_DEBUG, "WSASelect() <%d>", err);
		return false;
	}
	return true;
}

bool NPVRSocket::SelectC(DWORD timeout)
{
	struct fd_set   readfds;
	struct timeval  tmval;
	FD_ZERO(&readfds);
	FD_SET(m_socket, &readfds);

	tmval.tv_sec = (unsigned long)timeout /1000;
	tmval.tv_usec = (unsigned long)timeout %1000 * 1000;

	select(1, &readfds, NULL, NULL, &tmval);
	if (!FD_ISSET(m_socket, &readfds))
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK)
		{
			glog(Log::L_DEBUG, "Select() <%d>", err);
		}
		return false;
	}
	return true;
}

long NPVRSocket::GetNetworkEventC(WSAEVENT& hWSAEvent)
{
	WSANETWORKEVENTS wsaNetworkEvents;
	int ret = WSAEnumNetworkEvents(m_socket, hWSAEvent, &wsaNetworkEvents);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		glog(Log::L_DEBUG, "WSAEnumNetworkEvents()::SOCKET_ERROR <%d>", err);
		
		return SOCKET_ERROR;
	}

	//glog(Log::L_DEBUG, "WSANETWORKEVENTS.lNetworkEvents ÊÇ %d", wsaNetworkEvents.lNetworkEvents);

	// process FD_READ
	if (wsaNetworkEvents.lNetworkEvents & FD_READ)
	{
		if (wsaNetworkEvents.iErrorCode[FD_READ_BIT] != 0)
		{
			glog(Log::L_ERROR, "FD_READ failed with error %d\n", wsaNetworkEvents.iErrorCode[FD_READ_BIT]);
			return ERROR_READ;
		}
		//glog(Log::L_DEBUG, "FD_READ");
		//glog(Log::L_DEBUG, "WSANETWORKEVENTS.iErrorCode     is %d", wsaNetworkEvents.iErrorCode[FD_READ_BIT]);
		return FD_READ;
	}
	
	// process FD_CONNECT
	else if (wsaNetworkEvents.lNetworkEvents & FD_CONNECT)
	{
		if (wsaNetworkEvents.iErrorCode[FD_CONNECT_BIT] != 0)
		{
			glog(Log::L_ERROR, "FD_CONNECT failed with error %d", wsaNetworkEvents.iErrorCode[FD_CONNECT_BIT]);
			return ERROR_CONNECT;
		}
		
		glog(Log::L_DEBUG, "FD_CONNECT");
		return FD_CONNECT;
	}

	// process FD_CLOSE
	else if (wsaNetworkEvents.lNetworkEvents & FD_CLOSE)
	{
		if (wsaNetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
		{
			glog(Log::L_ERROR, "FD_CLOSE failed with error %d\n", wsaNetworkEvents.iErrorCode[FD_CLOSE_BIT]);
			return ERROR_CLOSE;
		}
		
		glog(Log::L_DEBUG, "FD_CLOSE");
		return FD_CLOSE;
	}

	else
	{
		//glog(Log::L_DEBUG, "WSANETWORKEVENTS.iErrorCode     is %d", wsaNetworkEvents.iErrorCode[wsaNetworkEvents.lNetworkEvents]);
		return wsaNetworkEvents.lNetworkEvents;
	}
}

bool NPVRSocket::CloseSocketC()
{
	int ret = closesocket(m_socket);
	if (ret == 0)
	{
		m_socket = NULL;
		return true;
	}
	else if (ret == WSAEWOULDBLOCK)
	{
		m_socket = NULL;
		return true;
	}
	int err = GetLastError();
	glog(Log::L_DEBUG, "CloseSocketC() <%d>", err);
	return false;
}

// server function ///////////////////////////////////////////////////////////////////
bool NPVRSocket::WSASelectS(WSAEVENT& hWSAEvent, long lNetworkEvents)
{
	int ret = WSAEventSelect(m_socket, hWSAEvent, lNetworkEvents);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		return false;
	}
	return true;
}

bool NPVRSocket::Listen(int port)
{
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	m_addr.sin_port = htons(port);

	int ret = bind(m_socket, (LPSOCKADDR)&m_addr, sizeof(m_addr));
	if (ret == SOCKET_ERROR)
	{
		int  err = WSAGetLastError();
		glog(Log::L_ERROR, "bind() <%d>", err);
		return false;
	}

	ret = listen(m_socket, 2);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		glog(Log::L_ERROR, "listen() <%d>", err);
		return false;
	}

	return true;
}

bool NPVRSocket::Accept()
{
	if (m_hSocket != NULL)
	{
		closesocket(m_hSocket);
		m_hSocket = NULL;
	}

	m_hSocket = accept(m_socket, NULL, NULL);
	if (m_hSocket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
		{
			glog(Log::L_ERROR, "accept() <%d>", err);
		}
		return false;
	}

	return true;
}

bool NPVRSocket::SendS(char* buf, int bufLen)
{
	int ret = send(m_hSocket, buf, bufLen, 0);
	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
		{
			glog(Log::L_ERROR, "SendS() <%d>", err);
		}
		return false;
	}
	return true;
}

bool NPVRSocket::RecvS(char* buf, int bufLen)
{
	int ret = recv(m_hSocket, buf, bufLen, 0);
	if (ret == 0)
	{
		glog(Log::L_DEBUG, "connection has been gracefully closed");
		return false;
	}
	else if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
		{
			glog(Log::L_ERROR, "RecvS() <%d>", err);	
		}
		return false;
	}
	return true;
}

bool NPVRSocket::CloseSocketS()
{	
	int ret = closesocket(m_hSocket);
	if (ret == 0)
	{
		m_hSocket = NULL;
		return true;
	}
	else if (ret == WSAEWOULDBLOCK)
	{
		m_hSocket = NULL;
		return true;
	}
	int err = GetLastError();
	glog(Log::L_ERROR, "CloseSocketS() <%d>", err);
	return false;
}