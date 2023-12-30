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
// Ident : $Id:  Context.h,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : define class Context
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase1/DODApp/include/Context.h $
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
#ifndef __Context_h__
#define __Context_h__

#include "ConnectionFactory.h"
#include "Destination.h"
#define NULL 0
typedef void * CT_HANDLE;

namespace ZQ{
namespace JMSCpp{

#define LOWEST_EXCEPTION_SUBCLASS 2
/*!
  Will be set if a throwable handle is of type java.lang.Exception
  */
#define JAVA_EXCEPTION (1 << 0)
/*!
  Will be set if an exception handle is of type javax.jms.JMSException
  */
#define EXCEPTION (1 << 1)
/*!
  Will be set if a JMS exception handle is of type javax.jms.IllegalStateException
  */
#define ILLEGAL_STATE (1 << (LOWEST_EXCEPTION_SUBCLASS + 0))
/*!
  Will be set if a JMS exception handle is of type javax.jms.IllegalStateException
  */
#define INVALID_CLIENT_ID (1 << (LOWEST_EXCEPTION_SUBCLASS + 1))
/*!
  Will be set if a JMS exception handle is of type javax.jms.InvalidDestinationException
  */
#define INVALID_DESTINATION (1 << (LOWEST_EXCEPTION_SUBCLASS + 2))
/*!
  Will be set if a JMS exception handle is of type javax.jms.InvalidSelectorException
  */
#define INVALID_SELECTOR (1 << (LOWEST_EXCEPTION_SUBCLASS + 3))
/*!
  Will be set if a JMS exception handle is of type javax.jms.JMSSecurityException
  */
#define SECURITY (1 << (LOWEST_EXCEPTION_SUBCLASS + 4))
/*!
  Will be set if a JMS exception handle is of type javax.jms.MessageEOFException
  */
#define MESSAGE_EOF (1 << (LOWEST_EXCEPTION_SUBCLASS + 5))
/*!
  Will be set if a JMS exception handle is of type javax.jms.MessageFormatException
  */
#define MESSAGE_FORMAT (1 << (LOWEST_EXCEPTION_SUBCLASS + 6))
/*!
  Will be set if a JMS exception handle is of type javax.jms.MessageNotReadableException
  */
#define MESSAGE_NOT_READABLE (1 << (LOWEST_EXCEPTION_SUBCLASS + 7))
/*!
  Will be set if a JMS exception handle is of type javax.jms.MessageNotWriteableException
  */
#define MESSAGE_NOT_WRITEABLE (1 << (LOWEST_EXCEPTION_SUBCLASS + 8))
/*!
  Will be set if a JMS exception handle is of type javax.jms.ResourceAllocationException
  */
#define RESOURCE_ALLOCATION (1 << (LOWEST_EXCEPTION_SUBCLASS + 9))
/*!
  Will be set if a JMS exception handle is of type javax.jms.TransactionInProgressException
  */
#define TRANSACTION_IN_PROGRESS (1 << (LOWEST_EXCEPTION_SUBCLASS + 10))
/*!
  Will be set if a JMS exception handle is of type javax.jms.TransactionRolledBackException
  */
#define TRANSACTION_ROLLED_BACK (1 << (LOWEST_EXCEPTION_SUBCLASS + 11))


/// global error function
/// @return 
/// @parameter void
int		getLastJmsError();


// -----------------------------
// class Context
// -----------------------------
/// This class performs naming operations.It can get access to the JMS provider 

class Context  
{
public:

   /// Constructor to create a Context object
   /// @param pcURL The URI of the JMS provider instance. 
   /// @param pcNamingContextFactory The name of the jndi factory to use in order to get the initial context.
   Context(const char *pcURL,const char *pcNamingContextFactory);

   /// Create a ConnectionFactory object with the given name
   /// @return true o success ,false on failure
   /// @param factory Contains the JNDI name of the connection factory to create. May not be NULL.
   /// @param connFac Returns the ConnectionFactory object created
   bool createConnectionFactory(const char *factory,ConnectionFactory &connFac);
   
   /// Create a Destination object with the given name
   /// @return true on success, false on failure
   /// @param name The JNDI name of the destination to create. May not be NULL 
   /// @param des Returns  the Destination object created
   bool createDestination(const char *name,Destination &des);

	/// Destroy context explicitly
	/// @return void
	/// @param void
	void destroy();


   ~Context();
public:

   /// A void pointer that represents a java Context object.
   /// Some other classes in the lib need to access it directly.
   /// It is made public to avoid using too much friend classes
   /// So avoid accessing it directly in your program.
   CT_HANDLE _context;
};
}
}
#endif // __Context_h__
