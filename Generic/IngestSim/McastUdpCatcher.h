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
// Name  : McastUdpCatcher.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2006-3-15
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/IngestSim/McastUdpCatcher.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// ===========================================================================

// McastUdpCatcher.h: interface for the McastUdpCatcher class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MCASTUDPCATCHER_H__B4FD939C_8F96_4712_98F3_300F1D6F1FEF__INCLUDED_)
#define AFX_MCASTUDPCATCHER_H__B4FD939C_8F96_4712_98F3_300F1D6F1FEF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef	USE_WPCAP

#ifndef WPCAP
#	error "Please include WinPcap SDK in your project"
#endif	//WPCAP

#endif	//USE_WPCAP

#include "Socket.h"
#include "pcap.h"

// link WinPcap library
#pragma comment(lib, "wpcap.lib")


#define		MCASTUDP_FILSTR		"udp and dst host %s and dst port %d"
#define		PACKSIZE			65535


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


class McastUdpCatcher  
{
public:
	McastUdpCatcher();
	virtual ~McastUdpCatcher();

	/// bind the catch to specified adapter
	///@param[in]		adapterIp		the IP address of the adapter need to bind	
	///@return			0 for success, else failure
	int		Bind(const char* adapterIp);

	/// open a catcher to listening to a multicast stream
	///@param[in]		mcastIp			the multicast IP address you want to listen to
	///@param[in]		mcastPort		the multicast port you want to listen to
	///@return			0 for success, else failure
	int		Open(const char* mcastIp, int mcastPort);

	/// receive packages from the destination
	///@param[out]		buff			pointer to the buffer to stored the received data
	///@param[in,out]	length			pointer to the length of the buffer, in bytes, including '\0'
	///@return			0 for success, -1 if error occurs, 1 if no data is found during timeout period
	int		Recv(char* buff, u_int* length);

	/// close the catcher
	///@return			0 for success, else failure
	int		Close();

	/// get last error description.  If succeeded, return 1, and the
	/// buffer will be filled with the error description, until the length
	/// is reached.
	///@param[out]		buffer			pointer to a buffer to stored the error string
	///@param[in,out]	length			length of the buffer, in bytes
	///@return			0 for success, else failure
	int		GetLastErr(char* buffer, int* length);

private:
	void	ClearErr();

private:
	static char*	_iptos(u_long in);
	static char*	_ip6tos(struct sockaddr *sockaddr, char *address, int addrlen);
	
private:
	pcap_if_t	*_allDevs;
	pcap_if_t	*_bindDev;
	bpf_u_int32	_bindNetMask;
	pcap_t		*_adHandle;
	struct bpf_program _fCode;
	char		_lastErrBuff[PCAP_ERRBUF_SIZE+1];

};

#endif // !defined(AFX_MCASTUDPCATCHER_H__B4FD939C_8F96_4712_98F3_300F1D6F1FEF__INCLUDED_)
