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
// Ident : $Id:  Requestor.h,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : define class Requestor
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataTunnel/source/include/Requestor.h $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 1     09-03-09 10:34 Li.huang
// 
// 1     08-12-08 11:11 Li.huang
// 
// 1     07-03-13 18:27 Li.huang
// 
// 1     07-01-03 10:39 Li.huang
// 
// 3     06-12-06 16:55 Ken.qian
// 
// 3     05-07-29 21:14 Build
// Fixed comment for doxygen automation
// 
// 2     05-07-28 17:07 Jianjun.li
// 
// 1     05-07-28 10:42 Jianjun.li
// 
// 1     05-07-27 18:48 Jianjun.li
// 
// 7     05-07-27 18:12 Jianjun.li
// 
// 6     05-07-27 16:12 Jianjun.li
// 
// 5     05-07-27 16:10 Jianjun.li
// 
// 4     05-07-27 15:27 Jianjun.li
// 
// 3     05-07-26 19:17 Jianjun.li
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
#ifndef __REQUESTOR_H__
#define __REQUESTOR_H__

#include "Message.h"
#include "Destination.h"
#include "Session.h"
#include "Producer.h"
#include "Consumer.h"

namespace ZQ{
namespace JMSCpp{

// -----------------------------
// class Requestor
// -----------------------------
/// This class provides the way to send message synchronizly
/// It creates a TemporaryQueue for the responses and provides
/// a request method that sends the request message and waits for 
/// its reply.
 
class Requestor  
{
public:

	/// Constructor to create a Requestor object according to Destination object
	/// @param sess the Queue Session the queue belongs to
	/// @param des the queue to perform the request/reply call on
	Requestor(Session *sess,Destination *des);

	/// Sends a request and waits for a reply. 
	/// The temporary queue is used for the JMSReplyTo destination,
	/// and only one reply per request is expected.
	/// @return true on success,false on failure
	/// @param mes the message to send 
	/// @param response returns the reply message
	/// @param timeout The timeout in milliseconds to wait for a reply 
	/// @param timeToLive the TimeToLive of message keeped in JMS server in milliseconds, 0 means no limitation
	bool request(Message *mes, Message &response, __int64 timeout, __int64 timeToLive=0);
	//~Requestor();
private:
	/// used to send request message
    Producer _pro;
	/// used to receive reply message
	Consumer _con;

	/// temporaty queue to get reply message
    Destination _tempDestination;
};
}
}
#endif // __REQUESTOR_H__
