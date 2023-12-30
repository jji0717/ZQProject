

#ifndef _WINPCAP_MUTLCAP_ 
#define _WINPCAP_MUTLCAP_ 

#include "pcap.h"
#include <string>
#include "locks.h"


typedef struct ip_address{
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
}ip_address;

/* IPv4 header */
typedef struct ip_header{
    u_char  ver_ihl;        // Version (4 bits) + Internet header length (4 bits)
    u_char  tos;            // Type of service 
    u_short tlen;           // Total length 
    u_short identification; // Identification
    u_short flags_fo;       // Flags (3 bits) + Fragment offset (13 bits)
    u_char  ttl;            // Time to live
    u_char  proto;          // Protocol
    u_short crc;            // Header checksum
    ip_address  saddr;      // Source address
    ip_address  daddr;      // Destination address
    u_int   op_pad;         // Option + Padding
}ip_header;


/* UDP header*/
typedef struct udp_header{
    u_short sport;          // Source port
    u_short dport;          // Destination port
    u_short len;            // Datagram length
    u_short crc;            // Checksum
}udp_header;


typedef struct _WPCAPBUF
{
	PVOID   buffContext;
	BYTE*	pBuffer;
	DWORD	dwLength;
}WPCAPBUF, *PWPCAPBUF;


class MCastCapture {
	
public:
	
	MCastCapture();
	~MCastCapture();
	
public:
	
	bool open(const std::string& localIP, const std::string& MCastIP, int port);
	
	enum CAPRET{
		CAP_OK,
		CAP_TIMEOUT,
		CAP_ERROR
	};

	CAPRET capture(int timeOutMs=10000);
	
	void stop() { _stopCap = true; };
	
	void close();
	
	inline std::string getLastError() const {
		return _lastError;
	}
public:
	static bool initReceiver(std::string& localIP, std::string& errstr);
	static void uninitReceiver();
protected:	
	
	virtual PWPCAPBUF acquireOutputBuffer() = 0;
	virtual void releaseOutputBuffer(PWPCAPBUF wpcapBuffer, DWORD dataLen) = 0;
	
	inline void setLastError(const std::string& error) {
		_lastError.assign(error);
	}
	
protected:
	static pcap_if_t* _allDevs;
	static pcap_if_t* _bindDev;
	static bpf_u_int32 _netmask;
	static std::string  _localIP;
	static ZQ::common::Mutex	_lock;
	
	pcap_t* _handle;
	SOCKET  _socket;
	
	std::string _lastError;
	
private:
	bool        _stopCap;
};

#endif