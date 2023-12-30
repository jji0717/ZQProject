// Sniffer.cpp: implementation of the Sniffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Sniffer.h"

extern bool verbose_option;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Sniffer::Sniffer(int ifNum /* = 0 */)
{
	_ifNum = ifNum;
	_snifferPos = NULL;
	_sCap = INVALID_SOCKET;
	_proto = IPPROTO_UDP;
}

Sniffer::~Sniffer()
{

}

bool Sniffer::open(const char* srcName)
{
	SOCKADDR_IN addr;
	HOSTENT* pHostent = NULL;
	char szHostName[MAX_PATH];
	
	if (!parseSrcName(srcName, &_filterAddr))
		return false;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(CAPTRUE_PORT);
	if(gethostname(szHostName, MAX_PATH) != 0) {
		printf("gethostname() failed\n");
		closesocket(_sCap);
		return false;
	}

	pHostent = gethostbyname(szHostName);

	/*
	for (int i = 0; i <= _ifNum; i ++) {
		if (!pHostent->h_addr_list[i]) {
			printf("invalid interface number\n");
			return false;
		}
	}
	*/

	int i = 0;
	in_addr ifAddr;

	// printf("interface list: \n");
	while(pHostent->h_addr_list[i]) {
		/*
		memcpy(&ifAddr.s_addr, pHostent->h_addr_list[i], 
			pHostent->h_length);
		printf("if%d: %s\n", i, inet_ntoa(ifAddr));
		*/
		i ++;
	}

	// printf("\n");

	if (i == 0) {
		printf("can't find the nic\n");
		return false;
	}

	if (_ifNum < 0 || _ifNum >= i) {

		printf("invalid interface number\n");
		return false;
	}

	memcpy(&addr.sin_addr.s_addr, pHostent->h_addr_list[_ifNum], 
		pHostent->h_length);

	_filter = srcName;	

	_sCap = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (_sCap == INVALID_SOCKET) {
		assert(FALSE);
		return false;
	}

	int nTimeOut = 3000;
	if (setsockopt(_sCap, SOL_SOCKET,SO_RCVTIMEO, (const char*)&nTimeOut, 
		sizeof(nTimeOut)) == SOCKET_ERROR) {

		printf("setsockopt(SO_RCVTIMEO) failed\n");
		// assert(false);
		return false;
	}
	

	if (bind(_sCap,(PSOCKADDR )&addr, sizeof(addr)) == SOCKET_ERROR) {

		printf("bind failed\n");
		// assert(FALSE);
		return false;
	}
	
	DWORD dwOutBuff[100] ;
	DWORD dwInBuff= 1 ;
	DWORD dwRetLen = 0;
	
	if (WSAIoctl(_sCap, SIO_RCVALL, &dwInBuff, sizeof(dwInBuff), 
		&dwOutBuff, sizeof(dwOutBuff), &dwRetLen, 
		NULL, NULL) == SOCKET_ERROR) {

		printf("WSAIoctl(SIO_RCVALL) failed %x\n", WSAGetLastError());
		// assert(FALSE);
		return false;
	}
	
	return true;
}

size_t Sniffer::recv(void* buf, size_t len)
{
	char packet[0xff90]; // 64k
	size_t recvLen;
	tcphdr* tcpHdr;
	udphdr* udpHdr;

	while (true) {
		recvLen = (size_t )::recv(_sCap, (char* )packet, sizeof(packet), 0);
		if (recvLen <= 0) {
			printf("Sniffer::recv()\trecv() failed.");
			return -1;
		}

		iphdr* ipHdr = (iphdr* )packet;
		int ipHdrLen = (ipHdr->header_len & 0xf) * 4;

		if (_filterAddr.sin_addr.s_addr && 
			ipHdr->daddr != _filterAddr.sin_addr.s_addr) {

			continue;
		}

		if ((_proto == 0 || _proto == IPPROTO_UDP) && 
			ipHdr->proto == IPPROTO_UDP) {

			udpHdr = (udphdr* )(packet + ipHdrLen);

			if (_filterAddr.sin_port == 0 || 
				udpHdr->dest == _filterAddr.sin_port) {

				size_t udpLen = ntohs(udpHdr->len);
				udpLen -= sizeof(udphdr);
								
				if (udpLen > 0) {
					memcpy(buf, udpHdr + 1, udpLen);

					if (verbose_option)
						printIPHeader(ipHdr);

					return udpLen;
				}					
			}

		} else if ((_proto == 0 || _proto == IPPROTO_TCP) && 
			ipHdr->proto == IPPROTO_TCP) {

			tcpHdr = (tcphdr* )(packet + ipHdrLen);
			if (_filterAddr.sin_port == 0 || 
				tcpHdr->dest == _filterAddr.sin_port) {

				size_t tcpLen = ntohs(ipHdr->total_len) - 
					ipHdr->header_len * 4 - tcpHdr->doff * 4;
				if (tcpLen > 0) {
					memcpy(buf, (u_char* )tcpHdr + tcpHdr->doff * 4, 
						tcpLen);

					if (verbose_option)
						printIPHeader(ipHdr);

					return tcpLen;
				}
			}			
		}
	}

	return 0;
}

void Sniffer::printIPHeader(iphdr* ipHdr)
{
	int ipHdrLen = (ipHdr->header_len & 0xf) * 4;
	
	in_addr srcAddr, destAddr;
	char srcDesc[32];
	char destDesc[32];

	srcAddr.s_addr = ipHdr->saddr;
	destAddr.s_addr = ipHdr->daddr;
	strncpy(srcDesc, inet_ntoa(srcAddr), sizeof(srcDesc));
	strncpy(destDesc, inet_ntoa(destAddr), sizeof(destDesc));

	if (ipHdr->proto == IPPROTO_UDP) {

		udphdr* udpHdr = (udphdr* )((char* )ipHdr + ipHdrLen);

		printf("[SRC] UDP %s:%hu -> %s:%hu\n", 
			srcDesc, 
			ntohs(udpHdr->source), 
			destDesc, 
			ntohs(udpHdr->dest));

	} else if (ipHdr->proto == IPPROTO_TCP) {

		tcphdr* tcpHdr = (tcphdr* )((char* )ipHdr + ipHdrLen);

		printf("[SRC] TCP %s:%hu -> %s:%hu\n", 
			srcDesc, 
			ntohs(tcpHdr->source), 
			destDesc, 
			ntohs(tcpHdr->dest));

	} else if (ipHdr->proto == IPPROTO_ICMP) {

		printf("[SRC] ICMP %s -> %s\n", srcDesc, destDesc);

	} else {

		printf("[SRC] PROTO(%d) %s -> %s\n", ipHdr->proto, 
			srcDesc, destDesc);
	}
}

bool Sniffer::close()
{
	if (_sCap != INVALID_SOCKET) {
		closesocket(_sCap);
		_sCap = INVALID_SOCKET;
	}
	
	return true;
}

bool Sniffer::listItems(std::vector<std::string>& items)
{
	HOSTENT* pHostent = NULL;
	char szHostName[MAX_PATH];

	if(gethostname(szHostName, MAX_PATH) != 0) {
		printf("gethostname() failed\n");
		closesocket(_sCap);
		return false;
	}

	pHostent = gethostbyname(szHostName);

	int i = 0;
	in_addr ifAddr;

	while(pHostent->h_addr_list[i]) {
		memcpy(&ifAddr.s_addr, pHostent->h_addr_list[i], 
			pHostent->h_length);
		items.push_back(inet_ntoa(ifAddr));
		i ++;
	}

	return true;
}
