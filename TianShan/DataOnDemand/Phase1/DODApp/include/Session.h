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
// Ident : $Id:  Session.h,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : define class Session
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase1/DODApp/include/Session.h $
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
// 2     05-07-28 16:58 Jianjun.li
// 
// 1     05-07-28 10:42 Jianjun.li
// 
// 1     05-07-27 18:48 Jianjun.li
// 
// 5     05-07-27 18:12 Jianjun.li
// 
// 4     05-07-27 16:09 Jianjun.li
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
#ifndef __Session_h__
#define __Session_h__

#define NULL 0
typedef void * SS_HANDLE;

#include "Destination.h"
#include "Producer.h"
#include "Consumer.h"

namespace ZQ{
namespace JMSCpp{
// -----------------------------
// class Session
// -----------------------------
/// A Session object is a single-threaded context for
/// producing and consuming messages.
/// should be initialized by Connection::createSession method

class Session  
{
public:
   Session() : _session(NULL) {};
   ~Session();

   /// Creates a durable subscriber to the specified topic.
   /// @return true on success,false on failure
   /// @param des the non-temporary Topic to subscribe to
   /// @param name the name used to identify this subscription 
   /// @param selector The selector to use when receiving messages from this topic. If NULL, all messages from the destination are selected 
   /// @param cons returns the created Consumer object
   bool createDurableSubscriber(Destination *des,char *name,char *selector,Consumer &cons);
   
   /// Unsubscribes a durable subscription that has been created by a client. 
   /// @return true on success,false on failure
   /// @param name the name used to identify this subscription 
   bool unSubscribe(char *name);

   /// Creates a MessageProducer to send messages to the specified destination. 
   /// @return true on success,false on failure
   /// @param des the Destination pointer to access
   /// @param pd returns the created Producer object
   bool  createProducer(Destination *des,Producer &pd);
   
   /// Creates a MessageConsumer for the specified destination.
   /// @return true on success,false on failure
   /// @param des the Destination pointer to access
   /// @param cs returns the created Consumer object
   bool  createConsumer(Destination *des,Consumer &cs);
   
   /// Creates an initialized TextMessage object. 
   /// A TextMessage object is used to send a message containing a String.
   /// @return true on success,false on failure
   /// @param text the string of the TextMessage
   /// @param ms returns the Message object created
   bool  textMessageCreate(char *text,Message &ms);

   /// Creates a MapMessage object.
   /// A MapMessage object is used to send a self-defining set of name-value pairs
   /// @return true on success,false on failure
   /// @param ms returns the initialized Message object
   bool  mapMessageCreate(Message &ms);

   /// Creates a BytesMessage object. A BytesMessage object is used to send 
   /// a message containing a stream of uninterpreted bytes.
   /// @return true on success,false on failure
   /// @param ms returns the initialized Message object
   bool  bytesMessageCreate(Message &ms);

   /// Close session explicitly 
   /// @return void 
   /// @param void
   void close();

public:
  
   /// A void pointer that represents a java Session object.
   /// Some other classes in the lib need to access it directly.
   /// It is made public to avoid using too much friend classes
   /// So avoid accessing it directly in your program.
   SS_HANDLE _session;
};
}
}
#endif // __Session_h__