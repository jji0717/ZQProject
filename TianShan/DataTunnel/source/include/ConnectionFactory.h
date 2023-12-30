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
// Ident : $Id:  ConnectionFactory.h,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : define ConnectionFactory
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataTunnel/source/include/ConnectionFactory.h $
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
// 2     05-07-28 16:58 Jianjun.li
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
#ifndef __ConnectionFactory_h__
#define __ConnectionFactory_h__

#include "Connection.h"
#define NULL 0
typedef void * CF_HANDLE;

namespace ZQ{
namespace JMSCpp{
// -----------------------------
// class ConnectionFactory
// -----------------------------
/// A ConnectionFactory object encapsulates a set of connection 
/// configuration parameters that has been defined by an administrator. 
/// A client uses it to create a connection with a JMS provider.

class ConnectionFactory  
{
public:
   ConnectionFactory() : _connectionFactory(NULL) {};

   /// Creates a connection with the default user identity.
   /// The connection is created in stopped mode.
   /// No messages will be delivered until the Connection.
   /// start method is explicitly called.
   /// @return true on success,false on failure
   /// @param conn returns the created Connection object
   bool createConnection(Connection &conn);

	/// destroy connection factory explicitly
	/// @return void   
	/// @param void
	void Destroy();

   ~ConnectionFactory();
public:
   
   /// A void pointer that represents a java ConnectionFactory object.
   /// Some other classes in the lib need to access it directly.
   /// It is made public to avoid using too much friend classes
   /// So avoid accessing it directly in your program.
   CF_HANDLE _connectionFactory;
};
}
}
#endif // __ConnectionFactory_h__
