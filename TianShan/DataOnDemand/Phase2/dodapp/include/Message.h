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
// Ident : $Id:  Message.h,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : define class Message
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase2/dodapp/include/Message.h $
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
// 7     05-07-27 18:12 Jianjun.li
// 
// 6     05-07-27 16:16 Jianjun.li
// 
// 5     05-07-27 16:13 Jianjun.li
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
#ifndef __Message_h__
#define __Message_h__

#define NULL 0
typedef void * MS_HANDLE;
#include "Destination.h"

namespace ZQ{
namespace JMSCpp{

// -----------------------------
// class Message
// -----------------------------
/// Message is the root of three other message type.It provides 
/// some methods to set and get message header and properties.
class Message  
{
public:
   Message() : _message(NULL) {};
   ~Message();

   /// Gets the subclass of the given message handle 
   /// @return true on success,false onfailure
   /// @param type return message type:
   /// 0: this message is a TextMessage; 
   /// 1: this message is a ByetsMessage; 
   /// 3: this message is a MapMessage;
   bool getSubClass(int *type);
   
   /// Gets the message ID. The JMSMessageID header field contains
   /// a value that uniquely identifies each message sent by a provider. 
   /// When a message is sent, JMSMessageID can be ignored. 
   /// When the send method returns, it contains a provider-assigned value.
   /// @return true on success,false onfailure
   /// @param messageID May not be NULL. On success will contain the message identifier of the message 
   /// @param size the length of the buffer pointed by messageID
   bool getMessageID(char *messageID,int size);
   
   /// Gets the message timestamp. 
   /// @return true on success,false onfailure
   /// @param time returns the timeStamp
   bool getTimeStamp(__int64 *time);
   
   /// Gets the correlation ID for the message. 
   /// @return true on success,false on failure
   /// @param id May not be NULL. On success will contain the correlation id of the message
   /// @param size the buffer length of id;
   bool getCorrelationID(char *id,int size);
   
   /// Gets the Destination object to which a reply to this message should be sent.
   /// @return true on success,false onfailure
   /// @param des returns the destination object pointer;
   bool getReplyTo(Destination &des);
   
   /// Gets the Destination object for this message.
   /// @return true on success,false on failure
   /// @param des returns the Destination object pointer;
   bool getDestination(Destination &des);
   
   /// Gets the delivery mode of this message handle 
   /// @return true on success,false onfailure
   /// @param mode May not be NULL. On success, *mode will have one of the following values:
   /// JMS_NON_PERSISTENT  JMS_PERSISTENT 
   bool getDeliveryMode(int *mode);
 
   ///  Gets the message's expiration value.
   /// @return true on success,false onfailure
   /// @param expiration May not be NULL. On success, will contain the expiration time of the message 
   bool getExpiration(__int64 *expiration);
   
   /// Gets the message priority level. 
   /// @return true on success,false onfailure
   /// @param priority May not be NULL. On success, *priority will contain the priority of the message
   bool getPriority(int *priority);  
   
   /// Sets the correlation ID for the message.
   /// @return true on success,false on failure
   /// @param value the message ID of a message being referred to 
   bool setCorrelationID(char *value);  

   /// Sets the Destination object to which a reply to this message should be sent. 
   /// @return true on success,false on failure
   /// @param des Destination to which to send a response to this message
   bool setReplyTo(Destination *des);

   /// Sets the Destination object for this message.
   /// JMS providers set this field when a message is sent. 
   /// This method can be used to change the value for a message 
   /// that has been received. 
   /// @return true on success,false on failure
   /// @param des the destination for this message 
   bool setDestination(Destination *des); 

   /// Gets the value of the byte property with the specified name.
   /// @return true on success,false on failure
   /// @param key the name of the byte property
   /// @param value the byte property value for the specified name 
   bool getByteProperty(char *key,unsigned char *value); 
   
   /// Returns the value of the long property with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the long property
   /// @param value the long property value for the specified name 
   bool getLongProperty(char *key,__int64 *value); 
  
   /// Returns the value of the int property with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the int property
   /// @param value the int property value for the specified name 
   bool getIntProperty(char *key,int *value); 
   
   /// Returns the value of the string property with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the String property
   /// @param value the string property value for the specified name 
   /// @param size the buffer length of *value
   bool getStringProperty(char*key,char *value,int size);
   
   /// Returns the value of the double property with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the double property
   /// @param value the double property value for the specified name 
   bool getDoubleProperty(char*key,double *value);
   
   /// Returns the value of the float property with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the float property
   /// @param value the float property value for the specified name 
   bool getFloatProperty(char *key,float *value);
   
   /// Returns the value of the short property with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the short property
   /// @param value the short property value for the specified name 
   bool getShortProperty(char* key,short *value);
   
   /// Returns the value of the bool property with the specified name. 
   /// @return true on success,false on failure
   /// @param key the name of the bool property
   /// @param value the bool property value for the specified name 
   bool getBoolProperty(char* key,int *value);
   
   /// Sets a byte property value with the specified name into the message. 
   /// @return true on success,false on failure
   /// @param key the name of the byte property
   /// @param value the byte property value to set 
   bool setByteProperty(char *key,unsigned char value);
   
   /// Sets a long property value with the specified name into the message. 
   /// @return true on success,false on failure
   /// @param key the name of the long property
   /// @param value the long property value to set 
   bool setLongProperty(char *key,__int64 value);
   
   /// Sets a int property value with the specified name into the message. 
   /// @return true on success,false on failure
   /// @param key the name of the int property
   /// @param value the int property value to set 
   bool setIntProperty(char *key,int value);
   
   /// Sets a string property value with the specified name into the message. 
   /// @return true on success,false on failure
   /// @param key the name of the string property
   /// @param value the string property value to set 
   bool setStringProperty(char* key,char* value);
   
   /// Sets a double property value with the specified name into the message. 
   /// @return true on success,false on failure
   /// @param key the name of the double property
   /// @param value the double property value to set 
   bool setDoubleProperty(char* key,double value);
   
   /// Sets a float property value with the specified name into the message. 
   /// @return true on success,false on failure
   /// @param key the name of the float property
   /// @param value the float property value to set 
   bool setFloatProperty(char *key,float value);
   
   /// Sets a short property value with the specified name into the message. 
   /// @return true on success,false on failure
   /// @param key the name of the short property
   /// @param value the short property value to set 
   bool setShortProperty(char* key,short value);
   
   /// Sets a boolean property value with the specified name into the message. 
   /// @return true on success,false on failure
   /// @param key the name of the boolean property
   /// @param value the boolean property value to set 
   bool setBoolProperty(char* key,bool value);

   /// Destroy message Explicitly
   /// @return void
   /// @param void
   void	DestroyMessage();
public:
   /// A void pointer that represents a java Message object.
   /// Some other classes in the lib need to access it directly.
   /// It is made public to avoid using too much friend classes
   /// So avoid accessing it directly in your program.
   MS_HANDLE  _message;
};
}
}
#endif // __Message_h__