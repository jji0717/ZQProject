// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: NoiseSim.cpp,v 1.1 2005-07-06 15:30:26 Ouyang Exp $
// Branch: $Name:  $
// Author: Lin Ouyang
// Desc  : the implementation of class McastUdpSocket
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/IngestSim/McastUdpSocket.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 3     05-10-20 14:54 Jie.zhang
// 
// 2     05-08-31 16:48 Mei.zhang
// 
// 1     05-08-26 17:04 Lin.ouyang
// init version
// 
// 1     05-07-06 19:55 Lin.ouyang
// add multicast udp support
//
// Revision 1.1  2005-07-06 15:30:26  Ouyang
// initial created
// ===========================================================================


/// @file "McastUdpSocket.cpp"
/// @brief the implementation of class McastUdpSocket
/// @author (Lorenzo) Lin Ouyang
/// @date 2005-7-6 19:00
/// @version 0.1.0
//
// McastUdpSocket.cpp: the implementation of class McastUdpSocket.
//
//////////////////////////////////////////////////////////////////////

// McastUdpSocket.cpp: implementation of the McastUdpSocket class.
//
//////////////////////////////////////////////////////////////////////

#include <winsock2.h>
#include <Ws2tcpip.h>

#include "McastUdpSocket.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

McastUdpSocket::McastUdpSocket()
{
	InitSocket();
	m_addrMulticast.sin_addr.s_addr = ADDR_ANY;
}

McastUdpSocket::~McastUdpSocket()
{
	struct ip_mreq group;
	memset(&group, 0, sizeof(group));
	
	group.imr_multiaddr.S_un.S_addr = inet_addr(getLocalAddr().c_str());
	group.imr_interface.s_addr = htonl(INADDR_ANY);
	
	setsockopt(m_iSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char *)&group, sizeof(group));
}

bool McastUdpSocket::Bind(string strIP, int iPort)
{
	m_addrLocal.sin_family = m_iDomain;
	setLocalAddr(strIP);
	setLocalPort(iPort);

	return Bind();
}

bool McastUdpSocket::setMulticastAddr(const char* szAddr)
{
	m_addrMulticast.sin_addr.s_addr = inet_addr(szAddr);

	return (m_addrMulticast.sin_addr.s_addr!=0);
}

bool McastUdpSocket::Bind()
{
	int iRet;

	// set reuse
	int optval;
	
	optval = 1;
//	iRet = setsockopt(m_iSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
//	if(iRet == SOCKET_ERROR)
//		return false;

	// set bind address
	sockaddr_in addrBind;
	addrBind.sin_family = AF_INET;
	addrBind.sin_addr.S_un.S_addr = m_addrLocal.sin_addr.S_un.S_addr;
	addrBind.sin_port = htons(getLocalPort());
	iRet = bind(m_iSocket, (sockaddr *)&addrBind, sizeof(sockaddr));
	if(iRet == SOCKET_ERROR)
	{
		printf("bind address %s, port %d fail with code 0x%08x\n", inet_ntoa(m_addrLocal.sin_addr),
			getLocalPort(), WSAGetLastError());

		return false;
	}
	else
	{
		cout<<"bind address "<< inet_ntoa(m_addrLocal.sin_addr)<<", port "<<getLocalPort()<<" success\n";
	}
	///////////////////////////////////////////
	{

	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	socklen_t len = sizeof(addr);

	getsockname(m_iSocket, (struct sockaddr *)&addr, &len);

	char* psin_addr = (char*) &(addr.sin_addr);
	int   sin_len = sizeof(addr.sin_addr);

	setsockopt(m_iSocket, IPPROTO_IP, IP_MULTICAST_IF, psin_addr, sin_len);
	}
	///////////////////////////////////////////

	// set multicast interface and group
	struct ip_mreq group;
	memset(&group, 0, sizeof(group));

	// set group to join
//	group.imr_multiaddr.S_un.S_addr = m_addrMulticast.sin_addr.S_un.S_addr;
	group.imr_multiaddr = m_addrMulticast.sin_addr;

	struct sockaddr name;
	int namelen = sizeof(struct sockaddr);
	memset(&name, 0, sizeof(struct sockaddr));
	if(getsockname(m_iSocket, &name, &namelen)==SOCKET_ERROR)
	{
		cout<<"getsockname error\n";
	}

	// set send interface address
	memcpy(&group.imr_interface, &(((struct sockaddr_in*)&name)->sin_addr), sizeof(group.imr_interface)); //htonl(INADDR_ANY);

	if (SOCKET_ERROR == setsockopt(m_iSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&group, sizeof(group))) 
	{
		cout<<"join group "<<inet_ntoa(m_addrMulticast.sin_addr)<<" fail\n";
		return false;
	}
	else
	{
		cout<<"join group "<<inet_ntoa(m_addrMulticast.sin_addr)<<" success\n";
	}

	return isActive();
}
