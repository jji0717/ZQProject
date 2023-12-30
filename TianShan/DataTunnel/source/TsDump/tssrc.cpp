// MCastTsSrc.cpp: implementation of the MCastTsSrc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tssrc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MCastTsSrc::MCastTsSrc()
{

}

MCastTsSrc::~MCastTsSrc()
{
	if (_sock != INVALID_SOCKET)
		close();
}

bool MCastTsSrc::open(const char* srcName)
{
	struct sockaddr_in local,remote;
    SOCKET sockm;

	if (!parseSrcName(srcName, &_addr))
		return false;

	if((_sock=WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0,
		WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF|
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("socket failed with:%d\n",WSAGetLastError());
		return false;
	}
	//将sock绑定到本机某端口上。
	local.sin_family = AF_INET;
	local.sin_port = htons(0);//_addr.sin_port;
	local.sin_addr.s_addr = INADDR_ANY;
	if( bind(_sock,(struct sockaddr*)&local,sizeof(local)) == SOCKET_ERROR )
	{
		printf( "bind failed with:%d \n",WSAGetLastError());
		closesocket(_sock);
		return false;
	}
	//加入多播组
	remote.sin_family = AF_INET;
	remote.sin_port = _addr.sin_port;
	remote.sin_addr.s_addr = _addr.sin_addr.s_addr;

	/* Winsock2.0*/
	if(( sockm = WSAJoinLeaf(_sock,(SOCKADDR*)&remote,sizeof(remote),NULL,NULL,NULL,NULL, JL_BOTH)) == INVALID_SOCKET)
	{
		printf("WSAJoinLeaf() failed:%d\n",WSAGetLastError());
		closesocket(_sock);
		return false;
	}

	return true;
}

size_t MCastTsSrc::recv(void* buf, size_t len)
{
	sockaddr_in addr;
	int size = sizeof(addr);
	return ::recvfrom(_sock, (char* )buf, len, 0, 
		(sockaddr* )&addr, &size);
}

bool MCastTsSrc::close()
{
	ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = _addr.sin_addr.s_addr;
	mreq.imr_interface.s_addr = INADDR_ANY;
	int r = setsockopt(_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                        (char* )&mreq, sizeof(mreq));
	return SocketTsSrc::close();
}

//////////////////////////////////////////////////////////////////////////

FileTsSrc::FileTsSrc()
{

}

FileTsSrc::~FileTsSrc()
{
	if (_fp != NULL)
		close();
}

// TSSource
bool FileTsSrc::open(const char* srcName)
{
	_fp = fopen(srcName, "rb");
	if (_fp == NULL)
		return false;
	return true;
}

size_t FileTsSrc::recv(void* buf, size_t len)
{
	size_t r = fread(buf, 1, len, _fp);
	if (r == 0)
		r = -1;
	return r;
}

bool FileTsSrc::close()
{
	if (_fp) {
		fclose(_fp);
		_fp = NULL;
		return true;
	} else
		return false;	
}

//////////////////////////////////////////////////////////////////////////

TcpTsSrc::TcpTsSrc()
{

}

TcpTsSrc::~TcpTsSrc()
{

}

// TSSource
bool TcpTsSrc::open(const char* srcName)
{
	sockaddr_in addr;
	if (!parseSrcName(srcName, &addr))
		return false;

	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		return false;

	int one = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one));
	
	if (bind(sock, (sockaddr* )&addr, sizeof(addr)) == SOCKET_ERROR) {
		closesocket(sock);
		return false;
	}

	listen(sock, 5);
	int len = sizeof(addr);
	_sock = accept(sock, (sockaddr* )&addr, &len);
	closesocket(sock);

	if (_sock == INVALID_SOCKET) {
		return false;
	}

	return true;
}

size_t TcpTsSrc::recv(void* buf, size_t len)
{
	return ::recv(_sock, (char* )buf, len, 0);
}

bool TcpTsSrc::close()
{
	return SocketTsSrc::close();	
}

//////////////////////////////////////////////////////////////////////////


UdpTsSrc::UdpTsSrc()
{

}

UdpTsSrc::~UdpTsSrc()
{

}

// TSSource
bool UdpTsSrc::open(const char* srcName)
{
	if (!parseSrcName(srcName, &_addr))
		return false;
	
	_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (_sock == INVALID_SOCKET)
		return false;

	/*
	LINGER linger;
	linger.l_onoff = 1;
	linger.l_linger = 5;
	setsockopt(_sock, SOL_SOCKET, SO_LINGER, 
		(char* )&linger, sizeof(linger));
	*/
	
	int one = 1;
	setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one));

	if (bind(_sock, (sockaddr* )&_addr, sizeof(_addr)) == SOCKET_ERROR) {
		closesocket(_sock);
		return false;
	}
	
	return true;

}

size_t UdpTsSrc::recv(void* buf, size_t len)
{
	sockaddr_in addr;
	int size = sizeof(addr);
	return ::recvfrom(_sock, (char* )buf, len, 0, 
		(sockaddr* )&addr, &size);

}
