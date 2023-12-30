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
// Desc  : define BytesMessage, a message type deprived from Message
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase2/dodapp/include/BytesMessage.h $
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
// 2     06-09-19 12:13 Li.huang
// test
// 3     05-07-29 21:30 Build
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
#ifndef __BytesMessage_h__
#define __BytesMessage_h__

#include "Message.h"

namespace ZQ{
namespace JMSCpp{

// -----------------------------
// class BytesMessage
// -----------------------------
/// A BytesMessage object is used to send a message containing a stream 
/// of uninterpreted bytes. It is deprived from class Message.
class BytesMessage:public Message
{
public:

   /// Get the total number of bytes in the message body
   /// @return true on success,false on failure
   /// @param length Must not be NULL,On success *length
   ///	will contain the body length of the bytes message 

   bool getBodyLength(__int64 *length);
   
   /// Read a bool value from the bytes message stream.
   /// @return true on success,false on failure
   /// @param value Must not be NULL,On success *value
   ///	will contain returned the boolean value 

   bool readBoolean(int *value);

   /// Read a byte value from the bytes message stream.
   /// @return true on success,false on failure
   /// @param value Must not be NULL,On success *value
   ///	will contain returned the Byte value 

   bool readByte(unsigned char *value);

   /// Read a byte value from the bytes message stream.
   /// @return true on success,false on failure
   /// @param value Must not be NULL,On success *value
   ///	will contain returned the short value 
   bool readShort(short *value);

   /// Read a byte value from the bytes message stream.
   /// @return true on success,false on failure
   /// @param value Must not be NULL,On success *value
   ///	will contain returned the unsigned short value 
   bool readUnsignedShort(unsigned short *value);

   /// read a byte value from the bytes message stream.
   /// @return true on success,false on failure
   /// @param value Must not be NULL,On success *value
   ///	will contain returned the int value 
   bool readInt(int *value);

   /// read a byte value from the bytes message stream.
   /// @return true on success,false on failure
   /// @param value Must not be NULL,On success *value
   ///	will contain returned the __int64 value 
   bool readLong(__int64 *value);

   /// read a byte value from the bytes message stream.
   /// @return true on success,false on failure
   /// @param value return the float value
   bool readFloat(float *value);

   /// read a byte value from the bytes message stream.
   /// @return true on success,false on failure
   /// @param value Must not be NULL,On success *value
   ///	will contain returned the double value 
   bool readDouble(double *value);

   /// read bytes array from the bytes message stream.
   /// @return true on success,false on failure
   /// @param bytes Must not be NULL, On success will 
   /// be filled in with *length bytes from the bytes message stream 
   /// @param length *length must contain the number of bytes available
   /// to be written in bytes. On success, *length will contain the 
   /// number of bytes written to bytes 
   bool readBytes(void *bytes,int *length);
  
   /// write a bool value into the message
   /// @return true on success,false on failure
   /// @param value the bool value to write
   bool writeBoolean(int value);

   /// Writes a unsigned char value to the bytes message stream
   /// @return true on success,false on failure
   /// @param value The byte value to write to the bytes message stream
   bool writeByte(unsigned char value);

   /// Writes a short value to the bytes message stream
   /// @return true on success,false on failure
   /// @param value The short value to write to the bytes message stream
   bool writeShort(short value);

   /// Writes a int value to the bytes message stream
   /// @return true on success,false on failure
   /// @param value The int value to write to the bytes message stream
   bool writeInt(int value);

   /// Writes a __int64 value to the bytes message stream
   /// @return true on success,false on failure
   /// @param value The __int64 value to write to the bytes message stream
   bool writeLong(__int64 value);

    /// Writes a float value to the bytes message stream
   /// @return true on success,false on failure
   /// @param value The float value to write to the bytes message stream
   bool writeFloat(float value);

    /// Writes a double value to the bytes message stream
   /// @return true on success,false on failure
   /// @param value The double value to write to the bytes message stream
   bool writeDouble(double value);

   /// Writes a byte array value to the bytes message stream 
   /// @return true on success,false on failure
   /// @param value The bytes to write to the bytes message stream 
   /// @param size The number of bytes to write 
   bool writeBytes(void *value,int size);
};
}
}
#endif // __BytesMessage_h__