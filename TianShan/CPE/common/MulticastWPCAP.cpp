// ===========================================================================
// Copyright (c) 2008 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================

#include "MulticastWPCAP.h"
#include <pcap.h>
#include "ZQ_common_conf.h"

#ifndef ZQ_OS_MSWIN
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#endif

#define PACKET_SIZE           65536
#define READ_TIMEOUT          1000  /* ms */

#ifdef ZQ_OS_MSWIN
#pragma comment(lib, "wpcap.lib")
#endif

MulticastWPCAP::MulticastWPCAP()
{
	_allDevs = NULL;
	_bindDev = NULL;
	_nTimeout = READ_TIMEOUT;
	_nMinBytesToCopy = 512*1024;
	_nKernelBufferBytes = 256*1024*1024;

}

bool MulticastWPCAP::init()
{
	if (!openDevice())
		return false;

	if (!openCapture())
	{
		return false;
	}

	return true;
}

bool MulticastWPCAP::openDevice()
{
	if (_allDevs)
	{
		return true;
	}

	// if local ip not set
	if(_strLocalIp.empty()) 
	{
		char host[101];
		gethostname(host, 100);

		hostent* entry = gethostbyname(host);
		in_addr addr;

		addr.s_addr = *(u_int*)(entry->h_addr);
		_strLocalIp = inet_ntoa(addr);
	}

	// initialize winpcap to bind local ip
	char buf[PCAP_ERRBUF_SIZE];
#ifdef ZQ_OS_MSWIN
	if(pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &_allDevs, buf) == (-1)) 
#else
	if(pcap_findalldevs(&_allDevs, buf) == (-1)) 
#endif
	{
		setLastError(buf);
		return false;
	}

	unsigned long nLocalIP = inet_addr(_strLocalIp.c_str());
	// get the local binded device
	for(pcap_if_t* dev = _allDevs; dev != NULL && !_bindDev; dev = dev->next)
	{
		pcap_addr_t* addr;
		for(addr = dev->addresses; addr; addr = addr->next) 
		{
			if(addr->addr->sa_family == AF_INET) 
			{
#ifdef ZQ_OS_MSWIN
				if( nLocalIP == ((struct sockaddr_in*)addr->addr)->sin_addr.S_un.S_addr) 
#else
				if( nLocalIP == ((struct sockaddr_in*)addr->addr)->sin_addr.s_addr) 
#endif
				{
					_bindDev = dev;
#ifdef ZQ_OS_MSWIN
					_netmask = ((struct sockaddr_in*)(addr->netmask))->sin_addr.S_un.S_addr;
#else
					_netmask = ((struct sockaddr_in*)(addr->netmask))->sin_addr.s_addr;
#endif

					break;
				}
			}
		} /* end for (addresses) */
	} /* end for (devices) */

	if(!_bindDev) 
	{
		sprintf(buf, "could not find adapter for ip %s", _strLocalIp.c_str());
		setLastError(buf);

		return false;
	}

	return true;
}

bool MulticastWPCAP::openCapture()
{
	if (!_bindDev)
		return false;

	char buf[PCAP_ERRBUF_SIZE];
#ifdef ZQ_OS_MSWIN
	if((_handle = pcap_open(
		_bindDev->name, 
		PACKET_SIZE, 
		PCAP_OPENFLAG_PROMISCUOUS,
		_nTimeout, 
		NULL, 
		buf)) == NULL) 
	{
		setLastError(buf);
		return false;
	}
#else
	if ((_handle = pcap_create(_bindDev->name, buf)) == NULL)
	{
		setLastError(buf);
		return false;
	}

	pcap_set_snaplen(_handle, PACKET_SIZE);
	pcap_set_promisc(_handle, 1);
	pcap_set_timeout(_handle, _nTimeout);
	if (pcap_set_buffer_size(_handle, _nKernelBufferBytes) < 0)
	{
		setLastError(pcap_geterr(_handle));
		return false;
	}

	if (pcap_activate(_handle) < 0)
	{
		setLastError(pcap_geterr(_handle));
		return false;
	}
#endif

	if(pcap_datalink(_handle) != DLT_EN10MB) 
	{
		setLastError("only support Ethernet network");
		return false;
	}

	char szFilter[128];
	sprintf(szFilter, "ip multicast");

	struct bpf_program code;
	if(pcap_compile(_handle, &code, szFilter, 1, _netmask) < 0) 
	{
		setLastError(pcap_geterr(_handle));
		return false;
	}

	if(pcap_setfilter(_handle, &code) < 0) 
	{
		setLastError(pcap_geterr(_handle));
		return false;
	}

#ifdef ZQ_OS_MSWIN
	if (pcap_setbuff(_handle, _nKernelBufferBytes))
	{
		setLastError(pcap_geterr(_handle));
		return false;
	}

	//int  pcap_setmintocopy (pcap_t *p, int size) Set the minumum amount of data received by the kernel in a single call. 
	if (pcap_setmintocopy(_handle, _nMinBytesToCopy))
	{
		setLastError(pcap_geterr(_handle));
		return false;
	}
#endif

	return true;
}

void MulticastWPCAP::closeCapture()
{
	if(NULL != _handle)	
	{
		pcap_close(_handle);
		_handle = 0;
	}
}

void MulticastWPCAP::closeDevice()
{
	if (_allDevs)
	{
		pcap_freealldevs(_allDevs);
		_allDevs = 0;
		_bindDev = 0;
	}
}

void MulticastWPCAP::setLocalIP( const std::string& strLocalIp )
{
	_strLocalIp = strLocalIp;
}

void MulticastWPCAP::close()
{
	closeCapture();

	closeDevice();
}

void MulticastWPCAP::setTimeout( int nTimeoutInMilliseconds )
{
	_nTimeout = nTimeoutInMilliseconds;
}

void MulticastWPCAP::setMinBytesToCopy( int nBytes /*= 512*1024*/ )
{
	_nMinBytesToCopy = nBytes;
}

void MulticastWPCAP::setKernelBufferBytes( int nBytes /*= 256*1024*1024*/ )
{
	_nKernelBufferBytes = nBytes;
}
