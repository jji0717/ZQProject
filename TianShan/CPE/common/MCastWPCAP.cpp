#include  "MCastWPCAP.h"
#include <sstream>
#include "urlstr.h"


namespace {
	const unsigned BITRATE_THRESHOLD = 2*1024*1024; /* in bytes */
}

#define  MIN_FILE_SIZE_FOR_TIMEOUT    1024*1024   // 1M


#define PACKET_SIZE           65536
#define ETHERNET_HEADER_SIZE  14
#define READ_TIMEOUT          1000  /* ms */



#pragma comment(lib, "wpcap.lib")

pcap_if_t* MCastCapture::_allDevs = NULL;
pcap_if_t* MCastCapture::_bindDev = NULL;
bpf_u_int32 MCastCapture::_netmask = 0;
std::string  MCastCapture::_localIP = "";
ZQ::common::Mutex	MCastCapture::_lock;

MCastCapture::MCastCapture():
_socket(INVALID_SOCKET),
_handle(NULL),
_stopCap(false) {
}


MCastCapture::~MCastCapture() {
	close();
}

bool MCastCapture::initReceiver(std::string& localIP, std::string& errstr)
{
	_localIP = localIP;

	WSADATA WSAData;

	// starts use of WS2_32.DLL
	if (WSAStartup (MAKEWORD(1,1), &WSAData) != 0) {

		std::string lastError;
	//	ZQ::Content::Process::getSystemErrorText(lastError);

	//	errstr = "MCastIOSource: WSAStartup failed with error " + lastError;

		return false;
	}

	if(_localIP.empty()) {
		char host[101];
		gethostname(host, 100);

		hostent* entry = gethostbyname(host);
		in_addr addr;

		addr.s_addr = *(u_int*)(entry->h_addr);
		_localIP = inet_ntoa(addr);

		localIP = _localIP;
	}

	// initialize winpcap to bind local ip
	char buf[PCAP_ERRBUF_SIZE];
	if(pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &_allDevs, buf) == (-1)) {

		errstr = buf;
		
		uninitReceiver();
		return false;
	}

	// get the local binded device
	for(pcap_if_t* dev = _allDevs; dev != NULL; dev = dev->next) {

		pcap_addr_t* addr;
		for(addr = dev->addresses; addr; addr = addr->next) {

			if(addr->addr->sa_family == AF_INET) {
				
				char* interfaceIP = inet_ntoa(((struct sockaddr_in*)addr->addr)->sin_addr);
				
				if(localIP == interfaceIP) {
					_bindDev = dev;
					_netmask = ((struct sockaddr_in*)(addr->netmask))->sin_addr.S_un.S_addr;
	
					break;
				}
			}
		} /* end for (addresses) */
	} /* end for (devices) */

	if(!_bindDev) {
		errstr = "can't find a interface to bind";
		return false;
	}

	return true;	
}

void MCastCapture::uninitReceiver()
{
	if(_allDevs) {
		pcap_freealldevs(_allDevs);
		_allDevs = 0;
		_bindDev = 0;
	}

	// terminates use of WS2_32.DLL
	WSACleanup();
}

bool MCastCapture::open(const std::string& localIP, const std::string& MCastIP, int port) 
{
	ZQ::common::Guard<ZQ::common::Mutex> opt (_lock);

	if(MCastIP.empty() || port <= 0) {
		setLastError("invalid IP address or port number");

		return false;
	}

	if(!_bindDev) {
		setLastError("no interface binded");

		return false;
	}

	std::ostringstream msg;
	// create socket handle for join multicast
	if ((_socket = socket (AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		msg <<  "failed to create socket: (" << WSAGetLastError() << ")\n";
		setLastError(msg.str());
		msg.str("");

		return false;
	}

	// bind socket to local IP
	SOCKADDR_IN local_sin;
	local_sin.sin_family = AF_INET;
	local_sin.sin_port = htons(INADDR_ANY);  
	local_sin.sin_addr.s_addr = inet_addr(localIP.c_str());

	if (::bind (_socket, 
            (struct sockaddr*)&local_sin, 
            sizeof (local_sin)) == SOCKET_ERROR) {
		msg << "failed to bind socket: (" << WSAGetLastError() << ")";
		setLastError(msg.str());
		msg.str("");

		close();
		
		return false;
	}
	
	// join the multicast
	struct ip_mreq mreq; 
	mreq.imr_multiaddr.s_addr = inet_addr (MCastIP.c_str());
	mreq.imr_interface.s_addr = INADDR_ANY;

	if (setsockopt (_socket, 
                  IPPROTO_IP, 
                  IP_ADD_MEMBERSHIP, 
                  (const char*)&mreq, 
                  sizeof (mreq)) == SOCKET_ERROR) {
		msg << "setsockopt failed: (" << WSAGetLastError() << ")";
		setLastError(msg.str());

        close();
		
		return false;
	}

	// open winpcap handle
	char buf[PCAP_ERRBUF_SIZE];
	if((_handle = pcap_open(
					_bindDev->name, 
					PACKET_SIZE, 
					PCAP_OPENFLAG_PROMISCUOUS,
					READ_TIMEOUT, 
					NULL, 
					buf)) == NULL) {
		
		setLastError(buf);
		
		close();

		return false;
	}

    if(pcap_datalink(_handle) != DLT_EN10MB) {
		setLastError("only support Ethernet network.");
        
		close();
        
		return false;
    }

	char szFilter[128];
	sprintf(szFilter, "ip multicast and dst host %s and dst port %d", MCastIP.c_str(), port);

	struct bpf_program code;
	if(pcap_compile(_handle, &code, szFilter, 1, _netmask) < 0) {
		setLastError(pcap_geterr(_handle));
		
		close();

		return false;
	}

	if(pcap_setfilter(_handle, &code) < 0) {
		setLastError(pcap_geterr(_handle));

		close();
		
		return false;
	}
	
	// set the flag
	_stopCap = false;

	return true;
}

void MCastCapture::close() {

	if(NULL != _handle)	{
		pcap_close(_handle);
		_handle = 0;
	}

	if(_socket != INVALID_SOCKET) {
		closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
}

MCastCapture::CAPRET MCastCapture::capture(int timeOutMs) 
{
	CAPRET nCapRet = CAP_OK;

	if(NULL == _handle) 
	{
		setLastError("invalid handle");
		return CAP_ERROR;
	}

	struct pcap_pkthdr* header;
	const u_char* data;

	PWPCAPBUF   curWAPBUF;
	DWORD       recdBytes = 0;


	DWORD       cpySize = 0;
	DWORD       leftSize = 0;

	DWORD       lastPackTKCount = GetTickCount();

	/*
	*  get WPCAPBUF buffer for output
	*/
	curWAPBUF = acquireOutputBuffer();
	if (!curWAPBUF)
	{
		setLastError("failed to allocate memory");
		return CAP_ERROR;
	}	

	u_int lastTimer = GetTickCount();
	while(!_stopCap) {

		/*
		*   check timeout 
		*/
		if( (GetTickCount() - lastPackTKCount) > (DWORD)timeOutMs ) 
		{
			setLastError("capture data timeout");
			nCapRet = CAP_TIMEOUT;
			break;
		}

		/*
		*   capture packet
		*/
		int res = pcap_next_ex(_handle, &header, &data);

		if(res == 0) {
			continue;
		}
		else if(res == (-1)) {
			setLastError(pcap_geterr(_handle));
			nCapRet = CAP_ERROR;
			break;
		}

		/*
		*	decode data from packet
		*/

		/* position of ip header */
		ip_header* ih = (ip_header*)(data + ETHERNET_HEADER_SIZE);

		/* position of udp header */
		udp_header* uh = (udp_header*)((u_char*)(ih) + ((ih->ver_ihl & 0xf) * 4));
		
		/* position of data */
		u_char* packet = (u_char*)(uh) + sizeof(udp_header);
		u_int len = header->caplen - (packet - data);
				
		// no packet was captured, continue to capture
		if(len == 0) {
			continue;
		}
		
		//
		// check after receive this buffer, whether exceed the buffer size
		//
		if(curWAPBUF->dwLength > (recdBytes + len) ) {
			// curWAPBUF does not full yet 
			memcpy(curWAPBUF->pBuffer+recdBytes, packet, len);
			recdBytes += len;
		}
		else 
		{
			cpySize = curWAPBUF->dwLength - recdBytes;
			leftSize = len - cpySize;

			// make the buffer full
			memcpy(curWAPBUF->pBuffer+recdBytes, packet, cpySize);
			recdBytes = curWAPBUF->dwLength;
			
			// release the full buffer
			releaseOutputBuffer(curWAPBUF, recdBytes);
			
			// acquire new buffer
			curWAPBUF = acquireOutputBuffer();
			if (!curWAPBUF)
			{
				setLastError("failed to allocate memory");
				nCapRet = CAP_ERROR;
				break;
			}	

			// copy the left buffer
			recdBytes = 0;
			if(leftSize > 0) {
				memcpy(curWAPBUF->pBuffer, packet+cpySize, leftSize);
				recdBytes = leftSize;
			}
		}

		// remember the last capture time
		lastPackTKCount = GetTickCount();
	}

	// release the buffer
	if (curWAPBUF)
	{
		releaseOutputBuffer(curWAPBUF, recdBytes);
	}

	return nCapRet;
}