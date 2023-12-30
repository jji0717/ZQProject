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
// Ident : $Id:  MapMessage.cpp,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : implements class MapMessage
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JMSCppLib/JMSCpp/MapMessage.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 5     08-01-16 11:12 Ken.qian
// add codes to catch unknow exception
// 
// 4     07-08-23 10:51 Hongquan.zhang
// 
// 3     05-08-26 10:58 Jianjun.li
// 
// 2     05-07-28 16:58 Jianjun.li
// 
// 1     05-07-28 10:44 Jianjun.li
// 
// 1     05-07-27 18:48 Jianjun.li
// 
// 3     05-07-27 18:12 Jianjun.li
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
#include "MapMessage.h"
#include "header\jms.h"

using namespace ZQ::JMSCpp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MapMessage::MapMessage()
{
	m_pEnumeration = NULL;
}
MapMessage::~MapMessage()
{
	if(m_pEnumeration)
	{
		try
		{
			JmsEnumerationDestroy(m_pEnumeration,0);
		}
		catch (...) { }
	}
}

bool MapMessage::getBoolean(char *key,int *value)
{
   ASSERT(_message!=NULL);
	JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;

   try
   {
	   retCode = JmsMapMessageGetBoolean((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::getByte(char *key,unsigned char *value)
{
   ASSERT(_message!=NULL);
	JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;

   try
   {
	   retCode = JmsMapMessageGetByte((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::getShort(char *key,short *value)
{
	ASSERT(_message!=NULL);
   JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;

   try
   {
	   retCode = JmsMapMessageGetShort((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::getInt(char *key,int *value)
{
	ASSERT(_message!=NULL);
   JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;

   try
   {
	   retCode = JmsMapMessageGetInt((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? true:false;
}

bool MapMessage::getLong(char *key,__int64 *value)
{
	ASSERT(_message!=NULL);
   JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;

   try
   {
	   retCode = JmsMapMessageGetLong((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::getFloat(char *key,float *value)
{
   ASSERT(_message!=NULL);
   JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;
   
   try
   {
	   retCode = JmsMapMessageGetFloat((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::getDouble(char *key,double *value)
{
   ASSERT(_message!=NULL);
	JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;
   
   try
   {
	   retCode = JmsMapMessageGetDouble((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }
   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::getString(char *key,char *value,int size)
{
   ASSERT(_message!=NULL);
	JmsString jstring,jsMy;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;
   jsMy.stringType = CSTRING;
   jsMy.uniOrC.string = value;
   jsMy.allocatedSize = size;
 
   try
   {
	   retCode = JmsMapMessageGetString((JmsMapMessage *)_message,&jstring,&jsMy,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::setBoolean(char *key,int value)
{
   ASSERT(_message!=NULL);
	JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;
   
   
   try
   {
	   retCode = JmsMapMessageSetBoolean((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::setByte(char *key,unsigned char value)
{
   ASSERT(_message!=NULL);
	JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;
   
   try
   {
	   retCode = JmsMapMessageSetByte((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::setShort(char *key,short value)
{
   ASSERT(_message!=NULL);
	JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;

   try
   {
	   retCode = JmsMapMessageSetShort((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::setInt(char *key,int value)
{
   ASSERT(_message!=NULL);
	JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;
   
   try
   {
	   retCode = JmsMapMessageSetInt((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::setLong(char *key,__int64 value)
{
   ASSERT(_message!=NULL);
	JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;
   
   try
   {
	   retCode = JmsMapMessageSetLong((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::setFloat(char *key,float value)
{
	ASSERT(_message!=NULL);
   JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;
   
   try
   {
	   retCode = JmsMapMessageSetFloat((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::setDouble(char *key,double value)
{
	ASSERT(_message!=NULL);
   JmsString jstring;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;
   
   try
   {
	   retCode = JmsMapMessageSetDouble((JmsMapMessage *)_message,&jstring,value,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}

bool MapMessage::setString(char *key,char *value)
{
   ASSERT(_message!=NULL);
   JmsString jstring,jsMy;
   int retCode;

   jstring.stringType = CSTRING;
   jstring.uniOrC.string = key;
   
   jsMy.stringType = CSTRING;
   jsMy.uniOrC.string = value;
   
   try
   {
	   retCode = JmsMapMessageSetString((JmsMapMessage *)_message,&jstring,&jsMy,0);
   }
   catch (...) 
   {
	   return false;
   }

   return retCode!=JMS_NO_ERROR? false:true;
}
const char* MapMessage::getFirstKey(char* buf,int size)
{
	ASSERT(_message!=NULL);
		
	JmsString strKey ;
	JmsString* pStrkey=&strKey;
	strKey.allocatedSize = size;
	strKey.stringType =CSTRING;	
	strKey.uniOrC.string=buf;
	if (m_pEnumeration!=NULL) 
	{
		try
		{
			JmsEnumerationDestroy(m_pEnumeration,0);
		}
		catch (...) { }
	}

	try
	{
		//get the Enumeration
		if(!(JmsMapMessageGetNames((JmsMapMessage *)_message,&m_pEnumeration,0)==JMS_NO_ERROR && m_pEnumeration))	
		{//No enumeration is found
			return NULL;
		}
	}
	catch (...) 
	{
		return NULL;
	}
	
	int bMore = 0;
	try
	{
		if(JmsEnumerationHasMoreElements(m_pEnumeration,&bMore,0)==JMS_NO_ERROR && bMore)	
		{		
			if(JmsEnumerationNextElement(m_pEnumeration,(void**)&pStrkey,0)==JMS_NO_ERROR)
			{
				return strKey.uniOrC.string;
			}
			else
			{
				return NULL;
			}
		}
		else
		{//no more keys
			return NULL;
		}
	}
	catch (...) 
	{
		return NULL;
	}
}
const char* MapMessage::getNextKey(char* buf,int size)
{
	ASSERT(_message!=NULL);
	JmsString strKey ;
	JmsString* pStrkey=&strKey;
	strKey.allocatedSize = size;
	strKey.stringType =CSTRING;	
	strKey.uniOrC.string=buf;
	int bMore = 0;

	try
	{
		if(JmsEnumerationHasMoreElements(m_pEnumeration,&bMore,0)==JMS_NO_ERROR && bMore)	
		{		
			if(JmsEnumerationNextElement(m_pEnumeration,(void**)&pStrkey,0)==JMS_NO_ERROR)
			{
				return pStrkey->uniOrC.string;
			}
			else
			{
				return NULL;
			}
		}
		else
		{//no more keys
			return NULL;
		}
	}
	catch (...) 
	{
		return NULL;
	}
}
