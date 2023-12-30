// Sniffer.h: interface for the Sniffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SNIFFER_H__3E5FDB29_9B7D_448B_89D8_B2D14CBE920F__INCLUDED_)
#define AFX_SNIFFER_H__3E5FDB29_9B7D_448B_89D8_B2D14CBE920F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "tsdump.h"
#include "tcpip.h"

#define CAPTRUE_PORT		27777
#define SIO_RCVALL			_WSAIOW(IOC_VENDOR,1)

class Sniffer : public TSSource
{
public:
	Sniffer(int ifNum = 0);
	virtual ~Sniffer();

	virtual bool open(const char* srcName);
	virtual size_t recv(void* buf, size_t len);
	virtual bool close();	
	
	virtual bool listItems(std::vector<std::string>& items);

protected:
	bool parseSrcName(const char* srcName, sockaddr_in* addr)
	{
		char ipaddr[MAX_PATH];
		u_short port;

		if (srcName == NULL)
			return false;
		
		bool result = false;
		const char* c = srcName;

		switch (*c) {
		case 't':
			_proto = IPPROTO_TCP;
			c = ++ srcName;
			break;

		case 'u':
			_proto = IPPROTO_UDP;
			c = ++ srcName;
			break;
			
		case 'a':
			_proto = 0;
			c = ++ srcName;
			break;
		}

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
			result = true;
		}

		addr->sin_family = AF_INET;
		addr->sin_addr.s_addr = inet_addr(ipaddr);
		addr->sin_port = htons(port);

		return result;
	}

	void printIPHeader(iphdr* hdr);

protected:
	std::string		_filter;
	SOCKET			_sCap;
	sockaddr_in		_filterAddr;

	char			_snifferBuf[TS_PACKAGE_SIZE * 20];
	size_t			_snifferPos;
	int				_ifNum;
	u_char			_proto;
};

#endif // !defined(AFX_SNIFFER_H__3E5FDB29_9B7D_448B_89D8_B2D14CBE920F__INCLUDED_)
