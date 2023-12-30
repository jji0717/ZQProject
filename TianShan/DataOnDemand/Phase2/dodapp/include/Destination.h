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
// Ident : $Id:  Destination.h,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : define class Destination
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase2/dodapp/include/Destination.h $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
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
#ifndef __Destination_h__
#define __Destination_h__

#define NULL 0
typedef void * DN_HANDLE;

namespace ZQ{
namespace JMSCpp{

// -----------------------------
// class Destination
// -----------------------------
/// A Destination object encapsulates a provider-specific address.
/// It represents a topic or queue 

class Destination  
{
public:
   Destination() : _destination(NULL) {};
   ~Destination();

   ///Destroy desination explicity
   /// @return void
   /// @param void
   void destroy();
   
public:
   
   /// A void pointer that represents a java Destination object.
   /// Some other classes in the lib need to access it directly.
   /// It is made public to avoid using too much friend classes
   /// So avoid accessing it directly in your program.
   DN_HANDLE _destination;
};
}
}
#endif // __Destination_h__
