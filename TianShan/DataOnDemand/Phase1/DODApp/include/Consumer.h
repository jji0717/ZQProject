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
// Ident : $Id:  Consumer.h,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : define class Consumer
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase1/DODApp/include/Consumer.h $
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
// 3     05-07-29 21:14 Build
// Fixed comment for doxygen automation
// 
// 2     05-07-28 16:58 Jianjun.li
// 
// 1     05-07-28 10:42 Jianjun.li
// 
// 1     05-07-27 18:48 Jianjun.li
// 
// 6     05-07-27 18:12 Jianjun.li
// 
// 5     05-07-27 16:16 Jianjun.li
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
#ifndef __Consumer_h__
#define __Consumer_h__

#include "Listener.h"
#define NULL 0
typedef void * CM_HANDLE;

namespace ZQ{
namespace JMSCpp{

// -----------------------------
// class Consumer
// -----------------------------
/// A client uses a Message Consumer object to receive messages from a destination. 
/// A Message Consumer object is created by passing a Destination object to a
/// message-consumer creation method supplied by a session.

class Consumer  
{
public:
   Consumer() : _consumer(NULL) {};
   ~Consumer();

   /// Receives the next message that arrives within the specified timeout interval. 
   /// This call blocks until a message arrives, 
   /// the timeout expires, or this message consumer is closed. 
   /// A timeout of zero never expires, and the call blocks indefinitely.
   /// @returns true on success,false on failure
   /// @param time the timeout value (in milliseconds)
   /// @param ms the next message produced for this message consumer, 
   /// or null if the timeout expires or this message consumer is concurrently closed
   bool receive(__int64 time,Message &ms);
  
   /// Sets the message consumer's Message Listener.
   /// @returns true on success,false on failure
   /// @param lis the listener to which the messages are to be delivered 
   bool setMessageListener(Listener *lis);

	/// close consumer explicitly
	/// return void
	/// param void
	void close();
public:

   /// A void pointer that represents a java Consumer object.
   /// Some other classes in the lib need to access it directly.
   /// It is made public to avoid using too much friend classes
   /// So avoid accessing it directly in your program.
   CM_HANDLE _consumer;
};
}
}
#endif // __Consumer_h__
