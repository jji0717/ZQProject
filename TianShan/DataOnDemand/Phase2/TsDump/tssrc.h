// MCastTsSrc.h: interface for the MCastTsSrc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MCASTTSSRC_H__D7BC987C_9C92_4F16_8396_7DC49B6A3FE1__INCLUDED_)
#define AFX_MCASTTSSRC_H__D7BC987C_9C92_4F16_8396_7DC49B6A3FE1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "tsdump.h"

class SocketTsSrc: public TSSource
{
public:
	SocketTsSrc()
	{
		_sock = INVALID_SOCKET;
	}

	virtual ~SocketTsSrc()
	{
		if (_sock != INVALID_SOCKET)
			close();
	}

	// TSSource
	virtual bool close()
	{
		bool result = closesocket(_sock) != SOCKET_ERROR;
		_sock = INVALID_SOCKET;
		return result;
	}

protected:
	bool parseSrcName(const char* srcName, sockaddr_in* addr)
	{
		char ipaddr[MAX_PATH];
		u_short port;
		bool result = false;
		const char* c = srcName;
		while (*c) {
			if (*c == ':') {
				lstrcpyn(ipaddr, srcName, c - srcName + 1);
				break;
			}
			c ++;
		}

		if (*c == ':') {
			c ++;
			port = atoi(c);
			if (port != 0) {
				result = true;
			}
		}

		addr->sin_family = AF_INET;
		addr->sin_addr.s_addr = inet_addr(ipaddr);
		addr->sin_port = htons(port);

		return result;
	}
	
protected:
	SOCKET		_sock;
};

class MCastTsSrc : public SocketTsSrc
{
public:
	MCastTsSrc();
	virtual ~MCastTsSrc();

	// TSSource
	virtual bool open(const char* srcName);
	virtual size_t recv(void* buf, size_t len);
	virtual bool close();

protected:
	sockaddr_in		_addr;
};

class FileTsSrc: public TSSource {
public:
	FileTsSrc();
	virtual ~FileTsSrc();

	// TSSource
	virtual bool open(const char* srcName);
	virtual size_t recv(void* buf, size_t len);
	virtual bool close();

protected:
	FILE*	_fp;
};

class TcpTsSrc : public SocketTsSrc
{
public:
	TcpTsSrc();
	virtual ~TcpTsSrc();

	// TSSource
	virtual bool open(const char* srcName);
	virtual size_t recv(void* buf, size_t len);
	virtual bool close();

};

class UdpTsSrc : public SocketTsSrc
{
public:
	UdpTsSrc();
	virtual ~UdpTsSrc();

	// TSSource
	virtual bool open(const char* srcName);
	virtual size_t recv(void* buf, size_t len);

protected:
	sockaddr_in		_addr;
};

#endif // !defined(AFX_MCASTTSSRC_H__D7BC987C_9C92_4F16_8396_7DC49B6A3FE1__INCLUDED_)
