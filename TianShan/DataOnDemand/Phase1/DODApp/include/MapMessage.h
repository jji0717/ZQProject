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
// Ident : $Id:  MapMessage.h,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : define class MapMessage
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase1/DODApp/include/MapMessage.h $
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

#ifndef __MapMessage_h__
#define __MapMessage_h__

#include "Message.h"

namespace ZQ{
namespace JMSCpp{

// -----------------------------
// class MapMessage
// -----------------------------
/// A MapMessage object is used to send a set of name-value pairs.

class MapMessage : public Message  
{
public:
   
   /// Returns the boolean value with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the boolean 
   /// @param value the boolean value with the specified name
   bool getBoolean(char *key,int *value);

   /// Returns the byte value with the specified name.
   /// @return true on success,false on failure
   /// @param key the name of the byte 
   /// @param value the byte value with the specified name
   bool getByte(char *key,unsigned char *value);
   
   /// Returns the short value with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the short 
   /// @param value the short value with the specified name 
   bool getShort(char *key,short *value);
   
   /// Returns the int value with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the int 
   /// @param value the int value with the specified name 
   bool getInt(char *key,int *value);
   
   /// Returns the long value with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the long 
   /// @param value the long value with the specified name 
   bool getLong(char *key,__int64 *value);
   
   /// Returns the float value with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the float 
   /// @param value the float value with the specified name 
   bool getFloat(char *key,float *value);
   
   /// Returns the double value with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the double 
   /// @param value the double value with the specified name 
   bool getDouble(char *key,double *value);
   
   /// Returns the string value with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the string 
   /// @param value the string value with the specified name 
   /// @param size the buffer length of value
   bool getString(char *key,char *value,int size);

   /// Sets a boolean value with the specified name into the Map.
   /// @return true on success,false on failure
   /// @param key the name of the boolean 
   /// @param value the boolean value to set in the Map  
   bool setBoolean(char *key,int value);
   
   /// Sets a Byte value with the specified name into the Map.
   /// @return true on success,false on failure
   /// @param key the name of the byte 
   /// @param value the byte value to set in the Map
   bool setByte(char *key,unsigned char value);
   
   /// Sets a short value with the specified name into the Map.
   /// @return true on success,false on failure
   /// @param key the name of the short 
   /// @param value the short value to set in the Map
   bool setShort(char *key,short value);
   
   /// Sets a int value with the specified name into the Map.
   /// @return true on success,false on failure
   /// @param key the name of the int 
   /// @param value the int value to set in the Map
   bool setInt(char *key,int value);
   
   /// Sets a Long value with the specified name into the Map.
   /// @return true on success,false on failure
   /// @param key the name of the Long 
   /// @param value the long value to set in the Map
   bool setLong(char *key,__int64 value);
   
   /// Sets a float value with the specified name into the Map.
   /// @return true on success,false on failure
   /// @param key the name of the float 
   /// @param value the float value to set in the Map
   bool setFloat(char *key,float value);
   
   /// Sets a double value with the specified name into the Map.
   /// @return true on success,false on failure
   /// @param key the name of the double 
   /// @param value the double value to set in the Map 
   bool setDouble(char *key,double value);
   
   /// Sets a string value with the specified name into the Map.
   /// @return true on success,false on failure
   /// @param key the name of the string 
   /// @param value the string value to set in the Map
   bool setString(char *key,char *value);

};
}
}
#endif // __MapMessage_h__