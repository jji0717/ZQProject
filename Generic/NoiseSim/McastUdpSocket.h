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
// Desc  : the head file of class McastUdpSocket
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/NoiseSim/McastUdpSocket.h $
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


/// @file "McastUdpSocket.h"
/// @brief the head file of class McastUdpSocket
/// @author (Lorenzo) Lin Ouyang
/// @date 2005-7-6 19:00
/// @version 0.1.0
//
// McastUdpSocket.h: the head file of class McastUdpSocket.
//
//////////////////////////////////////////////////////////////////////

// McastUdpSocket.h: interface for the McastUdpSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MCASTUDPSOCKET_H__D6C35009_1BF1_4D72_826B_7E9E73493999__INCLUDED_)
#define AFX_MCASTUDPSOCKET_H__D6C35009_1BF1_4D72_826B_7E9E73493999__INCLUDED_


#include "Socket.h"

/// @class McastUdpSocket "McastUdpSocket.h"
/// @brief Multicast UDP class
///
/// derived from class Socket
class McastUdpSocket : public Socket  
{
public:
	/// default constructor
	McastUdpSocket();
	/// default destructor
	virtual ~McastUdpSocket();

	/// bind local address, set multicast send interface address and joined group
	/// @param string strIP[in] joined group
	/// @param int iPort[in] joined port
	/// @return ture if success, otherwise return false
	virtual bool Bind(string strIP, int iPort);
	/// bind local address, set multicast send interface address and joined group
	/// @return ture if success, otherwise return false
	virtual bool Bind();
};

#endif // !defined(AFX_MCASTUDPSOCKET_H__D6C35009_1BF1_4D72_826B_7E9E73493999__INCLUDED_)
