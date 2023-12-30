// ============================================================================================
// Copyright (c) 2006, 2007 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Ken Qian
// Desc  : Implement the RFTLib Filter
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/ContentProcess/MCastIOSource.h 1     10-11-12 15:58 Admin $
// $Log: /ZQProjs/Generic/ContentProcess/MCastIOSource.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 6     08-01-16 15:22 Fei.huang
// add: adjust session time 
// 
// 5     07-11-28 10:54 Fei.huang
// add bitrate limit 
// 
// 4     07-11-16 19:16 Ken.qian
// change getTotalStuff()
// 
// 3     07-11-15 18:55 Ken.qian
// checkin after pass unittest with utility
// 
// 2     07-11-15 15:16 Ken.qian
// check in after passing unit test
// 
// 1     07-11-14 21:09 Ken.qian
// Initial check in based on Fei Huang's codes

#ifndef __MCASTIOSOURCE_H__
#define __MCASTIOSOURCE_H__

#include "pcap.h"
#include "GraphFilter.h"


/* 4 bytes IP address */
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

	bool open(const std::string localIP, const std::string& MCastIP, int port);

	bool capture(int timeOutMs=10000);

	void stop() { _stopCap = true; };

	void close();

	inline std::string getLastError() const {
		return _lastError;
	}
protected:
	static bool initReceiver(std::string& localIP, std::string& errstr);
	static void uninitReceiver();


	virtual PWPCAPBUF acquireOutputBuffer() = 0;
	virtual void releaseOutputBuffer(PWPCAPBUF wpcapBuffer, DWORD dataLen) = 0;
	virtual bool checkBitrate(u_int lastTimer) = 0;

	inline void setLastError(const std::string& error) {
		_lastError.assign(error);
	}

protected:
	static pcap_if_t* _allDevs;
	static pcap_if_t* _bindDev;
	static bpf_u_int32 _netmask;
	static std::string  _localIP;
	
	pcap_t* _handle;
	SOCKET  _socket;
	
	std::string _lastError;
	
private:
	bool        _stopCap;
};


namespace ZQ { 
namespace Content { 
namespace Process {

class MCastIOSource : public SourceFilter, protected MCastCapture {
	friend class Graph;


public:

	MCastIOSource(
			Graph& graph, 
			u_int timeout, 
			u_int bitrateFluctuate,
			const std::string& proto="udp");

	virtual ~MCastIOSource();

public:
	/*
		Following two functions must be invoked one time to a process
	*/
	static bool initMCastReceiver(std::string& localIP, std::string& errstr);
	static void uninitMCastReceiver();

	void notifyTimeChanged(); 

public:

	virtual bool receive(Filter*, ZQ::Content::BufferData*) {
		return true;
	}

	
	virtual bool begin();

	virtual bool pause();

	virtual bool abort();

	virtual void stop();

	virtual void quit();

	virtual void endOfStream(void);

	virtual __int64 getTotalStuff() {
		return _totalBytes;
	}
	
	virtual __int64 getProcessedStuff() {
		return _processedBytes;
	}

public:

	int run();
	
	inline void setReadSize(u_int size) {
		_readSize = size;
	}


private:
	virtual PWPCAPBUF acquireOutputBuffer();
	virtual void releaseOutputBuffer(PWPCAPBUF wpcapBuffer, DWORD dataLen);
	virtual bool checkBitrate(u_int lastTimer);
	
private:
	
	HANDLE _hNotify;
	HANDLE _hQuit;
	HANDLE _timeChanged;

	bool _quit;

	u_int _abortedBy;
	u_int _readSize;
	u_int _maxbps;
	u_int _timeout;
		
	bool  _endofStream;

private:
	WPCAPBUF  _wpcapBuffers[2];
	int       _inUseBufferNo;

	DWORD     _buffCount;
	__int64   _processedBytes;
	__int64   _totalBytes;

	bool	  _started; /* capture started or not */

	u_int _bitrateFluctuate;
};

}}}


#endif

