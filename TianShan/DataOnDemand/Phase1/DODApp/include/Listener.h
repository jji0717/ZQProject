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
// Ident : $Id:  Listener.h,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : define class Listener
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase1/DODApp/include/Listener.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 1     08-12-08 11:10 Li.huang
// 
// 1     07-01-03 10:39 Li.huang
// 
// 2     06-09-19 12:15 Li.huang
// 
// 2     05-07-29 21:14 Build
// Fixed comment for doxygen automation
// 
// 1     05-07-28 10:42 Jianjun.li
// 
// 1     05-07-27 18:48 Jianjun.li
// 
// 4     05-07-27 18:12 Jianjun.li
// 
// 3     05-07-27 15:27 Jianjun.li
// 
// 2     05-07-26 15:10 Jianjun.li
//
// Revision 1.2 2005/07/26 10:08:12 li
// notation added
//
// Revision 1.1 2005/07/22 14:23:46 li 
// created
//
// ===========================================================================

#ifndef __Listener_h__
#define __Listener_h__

#include "Message.h"

namespace ZQ{
namespace JMSCpp{

// -----------------------------
// class Listener
// -----------------------------
/// A MessageListener object is used to receive asynchronously delivered messages. 
/// You should deprive a new class from it,and rewrite the onMessage() method
/// to process asynchronously received message.

class Listener  
{
public:

	/// A Listener object is used to receive asynchronously delivered messages.
    /// Each session must insure that it passes messages serially to the listener.
	/// This means that a listener assigned to one or more consumers of the same 
	///	session can assume that the onMessage method is not called with the next 
	/// message until the session has completed the last call. 
	/// @param mes The message received
	virtual void onMessage(Message *mes) = 0;
};
}
}
#endif // __Listener_h__