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
// Ident : $Id:  BytesMessage.cpp,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : implements class BytesMessage
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JMSCppLib/JMSCpp/BytesMessage.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 3     05-08-26 10:58 Jianjun.li
// 
// 2     05-07-28 16:58 Jianjun.li
// 
// 1     05-07-28 10:44 Jianjun.li
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
#include "BytesMessage.h"
#include "header\jms.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::JMSCpp;
bool BytesMessage::getBodyLength(__int64 *length)
{
   ASSERT(_message!=NULL);
   int retCode;

   retCode = JmsBytesMessageGetBodyLength((JmsMapMessage *)_message,length,0);
   return retCode!=JMS_NO_ERROR? false:true;
   
}

bool BytesMessage::readBoolean(int *value)
{
   ASSERT(_message!=NULL);
   int retCode;
   retCode = JmsBytesMessageReadBoolean((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::readByte(unsigned char *value)
{
   ASSERT(_message!=NULL);
   int retCode;
   retCode = JmsBytesMessageReadByte((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::readShort(short *value)
{
   ASSERT(_message!=NULL);
   int retCode;
   retCode = JmsBytesMessageReadShort((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::readUnsignedShort(unsigned short *value)
{
   ASSERT(_message!=NULL);
	int retCode;
   retCode = JmsBytesMessageReadUnsignedShort((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::readInt(int *value)
{
   ASSERT(_message!=NULL);
	int retCode;
   retCode = JmsBytesMessageReadInt((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::readLong(__int64 *value)
{
   ASSERT(_message!=NULL);
	int retCode;
   retCode = JmsBytesMessageReadLong((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::readFloat(float *value)
{
   ASSERT(_message!=NULL);
	int retCode;
   retCode = JmsBytesMessageReadFloat((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::readDouble(double *value)
{
	ASSERT(_message!=NULL);
   int retCode;
   retCode = JmsBytesMessageReadDouble((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::readBytes(void *value,int *length)
{
   ASSERT(_message!=NULL);
	int retCode;
   retCode = JmsBytesMessageReadBytes((JmsBytesMessage *)_message,value,length,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::writeBoolean(int value)
{
   ASSERT(_message!=NULL);
	int retCode;
   retCode = JmsBytesMessageWriteBoolean((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::writeByte(unsigned char value)
{
   ASSERT(_message!=NULL);
	int retCode;
   retCode = JmsBytesMessageWriteByte((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::writeShort(short value)
{
   ASSERT(_message!=NULL);
	int retCode;
   retCode = JmsBytesMessageWriteShort((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::writeInt(int value)
{
   ASSERT(_message!=NULL);
	int retCode;
   retCode = JmsBytesMessageWriteInt((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::writeLong(__int64 value)
{
   ASSERT(_message!=NULL);
   int retCode;
   retCode = JmsBytesMessageWriteLong((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::writeFloat(float value)
{
   ASSERT(_message!=NULL);
	int retCode;
   retCode = JmsBytesMessageWriteFloat((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::writeDouble(double value)
{
   ASSERT(_message!=NULL);
	int retCode;
   retCode = JmsBytesMessageWriteDouble((JmsBytesMessage *)_message,value,0);
   return retCode!=JMS_NO_ERROR? false:true;
}

bool BytesMessage::writeBytes(void *value,int length)
{
	ASSERT(_message!=NULL);
   int retCode;
   retCode = JmsBytesMessageWriteBytes((JmsBytesMessage *)_message,value,length,0);
   return retCode!=JMS_NO_ERROR? false:true;
}
