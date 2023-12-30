// pcapsrc.cpp: implementation of the pcapsrc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pcapsrc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PCapSrc::PCapSrc(int ifNum) :
	_ifNum(ifNum)
{
	_proto = IPPROTO_UDP;
}

PCapSrc::~PCapSrc()
{

}

bool PCapSrc::open(const char* srcName)
{
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int i = 0;
	char packet_filter[] = "ip and (udp or tcp)";
	struct bpf_program fcode;
	u_long netmask;

	if (!parseSrcName(srcName, &_filterAddr))
		return false;

	char devname[MAX_PATH];

	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, 
		&alldevs, errbuf) == -1) {

		fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
		return false;
	}

	for(d=alldevs; d && i != _ifNum; d=d->next, i ++);

	strcpy(devname, d->name);

	if(d->addresses != NULL) {
		/* Retrieve the mask of the first address of the interface */
		netmask = ((struct sockaddr_in *)
			(d->addresses->netmask))->sin_addr.s_addr;
	} else {
		/*	
			If the interface is without addresses we suppose to be 
			in a C class network 
		*/

		netmask=0xffffff; 
	}

	pcap_freealldevs(alldevs);

	_pcapHandle= pcap_open(devname,	65536, 
		PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf);	

	if ( _pcapHandle == NULL) {

		fprintf(stderr,"\nUnable to open the adapter. "
			"%s is not supported by WinPcap\n");
		return false;
	}

	if (pcap_compile(_pcapHandle, &fcode, packet_filter, 1, netmask) <0 )	{

		fprintf(stderr, "\nUnable to compile the packet filter. "
			"Check the syntax.\n");
		return false;
	}
	
	//set the filter
	if (pcap_setfilter(_pcapHandle, &fcode) < 0) {

		fprintf(stderr,"\nError setting the filter.\n");
		return false;
	}

	return true;
}

extern bool verbose_option;

size_t PCapSrc::recv(void* buf, size_t len)
{
	pcap_pkthdr* pkt_header;
	const u_char*  pkt_data;
	int r = 0;

	// char packet[0xff90]; // 64k
	// size_t recvLen;
	tcphdr* tcpHdr;
	udphdr* udpHdr;

	while (true) {
		r = pcap_next_ex(_pcapHandle, &pkt_header, &pkt_data);
		
		if (r == 0)
			continue;

		if (r < 0)
			return -1;

		const u_char* packet = pkt_data + 14;

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

bool PCapSrc::close()
{
	pcap_close(_pcapHandle);
	return true;
}

static const char* network_reg_path = "SYSTEM\\CurrentControlSet\\"
	"Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}";

std::string PCapSrc::getConnectionName(const char* ifName)
{
	const char* it = ifName;
	while (it && *it != '{')
		it ++;

	std::string ifGuid(it);
	char regPath[MAX_PATH];
	sprintf(regPath, "%s\\%s\\Connection", 
		network_reg_path, ifGuid.c_str());

	HKEY key;
	LONG r = RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath, 0, KEY_READ, &key);
	if (r !=  ERROR_SUCCESS) {
		return std::string();
	}

	DWORD type;
	BYTE connName[MAX_PATH];
	DWORD nameLen = sizeof(connName) - 3;

	connName[0] = '[';
	r = RegQueryValueExA(key, "Name", NULL, &type, &connName[1], &nameLen);
	RegCloseKey(key);
	if (r !=  ERROR_SUCCESS) {
		return std::string();
	}

	strcat((char* )connName, "] ");	
	return (char *)connName;	
}

bool PCapSrc::listItems(std::vector<std::string>& items)
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int i=0;

	char errbuf[PCAP_ERRBUF_SIZE];

	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, 
		&alldevs, errbuf) == -1) {

		fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
		return false;
	}
	
	std::string ifName;
	std::string connName;
	for(d=alldevs; d; d=d->next)
	{
		connName = getConnectionName(d->name);
		ifName = connName;
		ifName += d->name;
		ifName += " [";
		if (d->description)
			ifName += d->description;
		else
			ifName += "No description available";
		ifName += "]";
		items.push_back(ifName);
	}

	pcap_freealldevs(alldevs);

	return true;
}

void PCapSrc::printIPHeader(iphdr* ipHdr)
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
