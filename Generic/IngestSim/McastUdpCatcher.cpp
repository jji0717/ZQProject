// ===========================================================================
// Copyright (c) 2004 by
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
// Name  : McastUdpCatcher.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2006-3-15
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/IngestSim/McastUdpCatcher.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 3     06-04-06 17:21 Bernie.zhao
// fixed bug of local file write
// 
// 2     06-03-16 17:09 Bernie.zhao
// use WinPcap to capture multicast
// ===========================================================================

// McastUdpCatcher.cpp: implementation of the McastUdpCatcher class.
//
//////////////////////////////////////////////////////////////////////

#include "McastUdpCatcher.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

McastUdpCatcher::McastUdpCatcher()
{
	_allDevs	= NULL;
	_bindDev	= NULL;
	_bindNetMask= 0xFFFFFF;	// by default C-class network
	_adHandle	= NULL;
	ClearErr();
}

McastUdpCatcher::~McastUdpCatcher()
{
	Close();
	ClearErr();
}

int	McastUdpCatcher::Bind(const char* adapterIp)
{
	pcap_if_t *d;

	if(adapterIp==NULL)
	{
		ClearErr();
		sprintf(_lastErrBuff, "Invalid bind IP.");
		return -1;
	}

	
	// Retrieve the interfaces list 
	ClearErr();
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &_allDevs, _lastErrBuff) == -1)
	{
		Close();
		return -1;
	}
	
	
	// search each adapter
	bool bContinue = true;
	for(d = _allDevs; d && bContinue; d = d->next)
	{
		pcap_addr_t *a;
		char ip6str[128];
		
		// search for the binding IP addresses
		for(a=d->addresses; a && bContinue; a=a->next) 
		{
			switch(a->addr->sa_family)
			{
			case AF_INET:
				if (a->addr)
				{
					if(0 == strcmp(adapterIp, _iptos(((struct sockaddr_in *)a->addr)->sin_addr.s_addr) ) )
					{
						_bindDev = d;
						_bindNetMask = ((struct sockaddr_in *)(a->netmask))->sin_addr.S_un.S_addr;
						bContinue = false;
					}
				}
				break;
				
			case AF_INET6:
				if (a->addr)
				{
					if(0 == strcmp(adapterIp, _ip6tos(a->addr, ip6str, sizeof(ip6str)) ) )
					{
						_bindDev = d;
						bContinue = false;
					}
				}
				break;
				
			default:
				break;
			}
		}	// end search addresses

	}	// end search adapters

	if(_bindDev == NULL)
	{	
		// can not find the address
		Close();
		ClearErr();
		sprintf(_lastErrBuff, "Can not find this address in all adapter devices.");
		return -1;
	}
	
	return 0;
}

int	McastUdpCatcher::Open(const char* mcastIp, int mcastPort)
{
	if(mcastIp==NULL || mcastPort<=0)
	{
		ClearErr();
		sprintf(_lastErrBuff, "Invalid parameter.");
		return -1;
	}

	// validate binded device
	if(_bindDev == NULL)
	{
		ClearErr();
		sprintf(_lastErrBuff, "Function \"Bind()\" should be called before \"Open()\"");
		return -1;
	}


	// open adapter device
	if ( (_adHandle= pcap_open(	_bindDev->name,  // name of the device
								PACKSIZE,     // portion of the packet to capture. 
								// 65536 grants that the whole packet will be captured on all the MACs.
								PCAP_OPENFLAG_PROMISCUOUS,         // promiscuous mode
								1000,      // read timeout
								NULL,      // remote authentication
								_lastErrBuff     // error buffer
								) ) == NULL)
    {
		// failed to open adapter
        Close();
        return -1;
    }


	// Check the link layer. We support only Ethernet for simplicity.
    if(pcap_datalink(_adHandle) != DLT_EN10MB)
    {
        sprintf(_lastErrBuff,"This program works only on Ethernet networks.");
        Close();
        return -1;
    }

	
	//compile the filter
	char	filterStr[256] = {0};
	sprintf(filterStr, MCASTUDP_FILSTR, mcastIp, mcastPort);
    if (pcap_compile(_adHandle, &_fCode, filterStr, 1, _bindNetMask) <0 )
    {
        sprintf(_lastErrBuff,"Unable to compile the packet filter \"%s\"", filterStr);
        Close();
        return -1;
    }
	//set the filter
    if (pcap_setfilter(_adHandle, &_fCode)<0)
    {
        fprintf(stderr,"Error setting the filter.");
        Close();
        return -1;
    }

	// At this point, we don't need any more the device list. Free it
	pcap_freealldevs(_allDevs);
	_allDevs = NULL;

	return 0;
}

int	McastUdpCatcher::Recv(char* buff, u_int* length)
{
	if(buff==NULL || *length==0)
	{
		ClearErr();
		sprintf(_lastErrBuff, "Invalid parameter.");
		return -1;
	}
	
	// validate binded device
	if(_adHandle == NULL)
	{
		ClearErr();
		sprintf(_lastErrBuff, "Function \"Open()\" should be called before \"Recv()\"");
		return -1;
	}

	// get package
	u_char				*pPackbuff;
	struct pcap_pkthdr	*pHeader;
	int res = pcap_next_ex(_adHandle, &pHeader, (const u_char**)&pPackbuff);

	if(res == 0)
	{
		// timeout elapsed
		return 1;
	}
	else if(res < 0)
	{
		// error occurs
		sprintf(_lastErrBuff, "Error reading packets: %s", pcap_geterr(_adHandle));
		return -1;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// decode the packet to get real data
	ip_header	*ih;
    udp_header	*uh;
    u_int		ip_len;
	u_char*		pData;
	u_int		dataLen;	

	// length of ethernet header
	ih = (ip_header *) (pPackbuff + 14); 
	
    // retrieve the position of the UDP header
    ip_len = (ih->ver_ihl & 0xf) * 4;
    uh = (udp_header *) ((u_char*)ih + ip_len);
	
    // UDP data start position
	pData = (u_char *)uh + sizeof(udp_header);
	dataLen = pHeader->caplen - (pData - pPackbuff);

	// copy to user buffer
	if(*length < dataLen+1)
		*length = *length;
	else
		*length = dataLen;

	memcpy(buff, (const char*)pData, *length);

	return 0;
}

int McastUdpCatcher::Close()
{
	if(_allDevs)
	{
		pcap_freealldevs(_allDevs);
		_allDevs = NULL;
		_bindDev = NULL;
	}

	if(_adHandle)
	{
		pcap_close(_adHandle);
		_adHandle = NULL;
	}
	
	return 0;
}

int	McastUdpCatcher::GetLastErr(char* buffer, int* length)
{
	if(buffer == NULL || *length<=0)
	{
		ClearErr();
		sprintf(_lastErrBuff, "Invalid parameter.");
		return -1;
	}

	if(*length < PCAP_ERRBUF_SIZE+1)
		*length = *length;
	else
		*length = PCAP_ERRBUF_SIZE;

	strncpy(buffer, _lastErrBuff, *length);
	
	return 0;
}

void McastUdpCatcher::ClearErr()
{
	ZeroMemory(_lastErrBuff, sizeof(_lastErrBuff));
}

//////////////////////////////////////////////////////////////////////////
// utility functions

#define IPTOSBUFFERS    12
char* McastUdpCatcher::_iptos(u_long in)
{
    static char output[IPTOSBUFFERS][3*4+3+1];
    static short which;
    u_char *p;
	
    p = (u_char *)&in;
    which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
    sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
    return output[which];
}

char* McastUdpCatcher::_ip6tos(struct sockaddr *sockaddr, char *address, int addrlen)
{
    socklen_t sockaddrlen;
	
#ifdef WIN32
    sockaddrlen = sizeof(struct sockaddr_in6);
#else
    sockaddrlen = sizeof(struct sockaddr_storage);
#endif
	
	
    if(getnameinfo(sockaddr, 
        sockaddrlen, 
        address, 
        addrlen, 
        NULL, 
        0, 
        NI_NUMERICHOST) != 0) address = NULL;
	
    return address;
}
