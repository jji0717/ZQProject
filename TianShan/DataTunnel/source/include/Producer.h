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
// Ident : $Id:  BytesMessage.h,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : define class Producer
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataTunnel/source/include/Producer.h $
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
// 4     05-07-27 18:12 Jianjun.li
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
#ifndef __Producer_h__
#define __Producer_h__

#define NULL 0
typedef void * PD_HANDLE;

#include "Message.h"
#include "Destination.h"
#include "header\jms.h"

namespace ZQ{
namespace JMSCpp{

typedef struct {
  /*!
    flags tells which options have been set.  Valid flags are defined using the
    PO_... defines
    */
  int           flags;
  /*!
    The destination to send this message to if PO_DESTINATION is set in flags
    */
  Destination*	destination;
  /*!
    The delivery mode to use with this message if PO_DELIVERY_MODE is set in flags
    */
  int              deliveryMode;
  /*!
    The priority that should be used with this message if PO_PRIORITY is set in flags
    */
  int              priority;
  /*!
    The time to live for this message if PO_TIMETOLIVE has been set in flags
    */
  __int64           timeToLive;
} ProducerOptions;

#define PO_NOMESSAGEID   (1 << 0)
/*!
  Indicates that no timestamp need be generated for this message
  */
#define PO_NOTIMESTAMP   (1 << 1)
/*!
  Indicates that the destination field of the JmsProducerOptions structure has been set
  */
#define PO_DESTINATION   (1 << 2)
/*!
  Indicates that the deliveryMode field of the JmsProducerOptions structure has been set
  */
#define PO_DELIVERY_MODE (1 << 3)
/*!
  Indicates that the priority field of the JmsProducerOptions structure has been set
  */
#define PO_PRIORITY      (1 << 4)
/*!
  Indicates that the timeToLive field of the JmsProducerOptions structure has been set
  */
#define PO_TIMETOLIVE    (1 << 5)


	// -----------------------------
// class Producer
// -----------------------------
/// The counterpart of the java interface javax.jms.MessageProducer
 
class Producer  
{
public:
   Producer() : _producer(NULL) {};
   ~Producer();

   /// Sends a message to the destination.
   /// @return true on success,false on failure
   /// @param ms the message to be sent
   /// @param op [optional],to set the send option of each message
   bool send(Message *ms,ProducerOptions *op=NULL);

//   /// Send a message to the destination with option
//   bool	send(Message* ms,ProducerOptions *op=NULL);

   /// close producer explicitly
   /// @return void
   /// @param void
   void close();
public:

   /// A void pointer that represents a java Producer object.
   /// Some other classes in the lib need to access it directly.
   /// It is made public to avoid using too much friend classes
   /// So avoid accessing it directly in your program.
   PD_HANDLE _producer;
};
}
}
#endif //__Producer_h__