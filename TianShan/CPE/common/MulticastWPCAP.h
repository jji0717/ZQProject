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

#ifndef ZQTS_CPE_MULTICASTWINPCAP_H
#define ZQTS_CPE_MULTICASTWINPCAP_H

#include "pcap.h"
#include <string>

#define ETHERNET_HEADER_SIZE  14

class MulticastWPCAP
{
public:
	MulticastWPCAP();

	void setLocalIP(const std::string& strLocalIp);
	void setTimeout(int nTimeoutInMilliseconds);
	void setMinBytesToCopy(int nBytes = 512*1024);
	void setKernelBufferBytes(int nBytes = 256*1024*1024);

	bool init();

	inline int capture(const unsigned char*& pIPPacket)
	{
		struct pcap_pkthdr* header;
		const u_char* data;

		/*
		*   capture packet
		*/
		int res = pcap_next_ex(_handle, &header, &data);
		if(!res) 
		{
			return 0;
		}
		else if(res == (-1)) 
		{
			setLastError(pcap_geterr(_handle));
			return -1;
		}

		/* position of ip header */
		pIPPacket = data + ETHERNET_HEADER_SIZE;
		return header->caplen - ETHERNET_HEADER_SIZE;
	}

	void close();

	std::string getLastError(){return _strLastErr;}

protected:
	void setLastError(const std::string& strError)
	{
		_strLastErr = strError;
	}

	bool openDevice();

	bool openCapture();

	void closeCapture();

	void closeDevice();

protected:
	pcap_if*						_allDevs;
	pcap_if*						_bindDev;
	unsigned int					_netmask;
	std::string						_strLocalIp;
	pcap*							_handle;

	int								_nMinBytesToCopy;
	int								_nKernelBufferBytes;
	int								_nTimeout;
	std::string						_strLastErr;
};


#endif

