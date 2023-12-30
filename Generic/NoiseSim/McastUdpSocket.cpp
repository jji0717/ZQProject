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
// $Log: /ZQProjs/Generic/NoiseSim/McastUdpSocket.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
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

bool McastUdpSocket::Bind()
{
	int iRet;

	// set reuse
	int optval;
	
	optval = 1;
	iRet = setsockopt(m_iSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
	if(iRet == SOCKET_ERROR)
		return false;

	// set bind address
	sockaddr_in addrBind;
	addrBind.sin_family = AF_INET;
	addrBind.sin_addr.S_un.S_addr = INADDR_ANY;
	addrBind.sin_port = htons(getLocalPort());
	iRet = bind(m_iSocket, (sockaddr *)&addrBind, sizeof(sockaddr));
	if(iRet == SOCKET_ERROR)
		return false;

	// set multicast interface and group
	struct ip_mreq group;
	memset(&group, 0, sizeof(group));

	// set group to join
	group.imr_multiaddr.S_un.S_addr = m_addrLocal.sin_addr.S_un.S_addr;

	// set send interface address
	group.imr_interface.s_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == setsockopt(m_iSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&group, sizeof(group))) 
	{
		return false;
	}

	return isActive();
}
