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
// Ident : $Id:  Message.cpp,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : implements class Message
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JMSCppLib/JMSCpp/Message.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 6     08-01-16 11:12 Ken.qian
// add codes to catch unknow exception
// 
// 5     05-12-22 11:20 Hongquan.zhang
// 
// 4     05-08-26 10:58 Jianjun.li
// 
// 3     05-08-25 20:02 Jianjun.li
// 
// 2     05-07-28 16:58 Jianjun.li
// 
// 1     05-07-28 10:44 Jianjun.li
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
#include "Message.h"
#include "header\jms.h"

using namespace ZQ::JMSCpp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Message::~Message()
{
    if(NULL!=_message)
	{
		try
		{
			JmsMessageDestroy((JmsMessage *)_message, 0);
		}
		catch (...) {}

		_message=NULL;
	}
}
void Message::DestroyMessage()
{
	if(NULL!=_message)
	{
		try
		{
			JmsMessageDestroy((JmsMessage *)_message, 0);
		}
		catch (...) {}

		_message=NULL;
	}
}
bool Message::getSubClass(int *messageType)
{	
	ASSERT(_message!=NULL);
	int retCode;
	
	try
	{
		retCode = JmsMessageGetSubclass((JmsMessage *)_message,messageType,0);
	}
	catch (...) 
	{
		return false;
	}

    return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getMessageID(char *messageID,int size)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;

    jstring.stringType = CSTRING;
    jstring.allocatedSize = size;
    jstring.uniOrC.string = messageID;

	try
	{
		retCode = JmsMessageGetMessageId((JmsMessage *)_message,&jstring,(JMS64I)0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getTimeStamp(__int64 *time)
{
	ASSERT(_message!=NULL);
	int retCode;
	
	try
	{
		retCode = JmsMessageGetTimestamp((JmsMessage *)_message,time,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getCorrelationID(char *id,int size)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;

    jstring.stringType = CSTRING;
    jstring.allocatedSize = size;
    jstring.uniOrC.string = id;

	try
	{
		retCode = JmsMessageGetCorrelationId((JmsMessage *)_message,&jstring,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getReplyTo(Destination &des)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsDestination *destination=NULL;

	try
	{
		retCode = JmsMessageGetReplyTo((JmsMessage *)_message,&destination,0);
	}
	catch (...) 
	{
		return false;
	}

	des._destination=destination;
	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getDeliveryMode(int *mode)
{
	ASSERT(_message!=NULL);
	int retCode;

	try
	{
		retCode = JmsMessageGetDeliveryMode((JmsMessage *)_message,mode,0);  
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getExpiration(__int64 *expiration)
{
	ASSERT(_message!=NULL);
	int retCode;

	try
	{
		retCode = JmsMessageGetExpiration((JmsMessage *)_message,expiration,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getPriority(int *priority)
{
	ASSERT(_message!=NULL);
	int retCode;

	try
	{
		retCode = JmsMessageGetPriority((JmsMessage *)_message,priority,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::setCorrelationID(char *value)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;

    jstring.stringType = CSTRING;
    jstring.uniOrC.string = value;

	try
	{
		retCode = JmsMessageSetCorrelationId((JmsMessage *)_message,&jstring,0); 
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::setReplyTo(Destination *des)
{
	ASSERT(_message!=NULL);
	int retCode;
	
	try
	{
		retCode = JmsMessageSetReplyTo((JmsMessage *)_message,(JmsDestination *)des->_destination,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::setDestination(Destination *des)
{
	ASSERT(_message!=NULL);
	int retCode;

	try
	{
		retCode = JmsMessageSetDestination((JmsMessage *)_message,(JmsDestination *)des->_destination,0); 
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}
/////get or set property
bool Message::getByteProperty(char *key,unsigned char *value)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;

	try
	{
		retCode = JmsMessageGetByteProperty((JmsMessage *)_message,&jstring,value,0); 
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getLongProperty(char *key,__int64 *value)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;

	try
	{
		retCode = JmsMessageGetLongProperty((JmsMessage *)_message,&jstring,value,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getIntProperty(char *key,int *value)
{
	ASSERT(_message!=NULL);
	int retCode;
    JmsString jstring;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;

	try
	{
		retCode = JmsMessageGetIntProperty((JmsMessage *)_message,&jstring,value,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getStringProperty(char* key,char *value,int size)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring,jsMy;

	jstring.stringType = CSTRING;
    jsMy.stringType = CSTRING;
	jstring.uniOrC.string =key;
	jsMy.uniOrC.string =value;
	jsMy.allocatedSize = size;

	try
	{
		retCode = JmsMessageGetStringProperty((JmsMessage *)_message,&jstring,&jsMy,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getDoubleProperty(char* key,double *value)
{
	ASSERT(_message!=NULL);
	int retCode;
    JmsString jstring;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;

	try
	{
		retCode = JmsMessageGetDoubleProperty((JmsMessage *)_message,&jstring,value,0); 
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getFloatProperty(char *key,float *value)
{
	ASSERT(_message!=NULL);
	int retCode;
    JmsString jstring;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;

	try
	{
		retCode = JmsMessageGetFloatProperty((JmsMessage *)_message,&jstring,value,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getShortProperty(char* key,short *value)
{
    ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;

	try
	{
		retCode = JmsMessageGetShortProperty((JmsMessage *)_message,&jstring,value,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::getBoolProperty(char* key,int *value)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;

	try
	{
		retCode = JmsMessageGetBooleanProperty((JmsMessage *)_message,&jstring,value,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}
//set property for application
bool Message::setByteProperty(char *key,unsigned char value)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;

	try
	{
		retCode = JmsMessageSetByteProperty((JmsMessage *)_message,&jstring,value,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::setLongProperty(char *key,__int64 value)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;

	try
	{
		retCode = JmsMessageSetLongProperty((JmsMessage *)_message,&jstring,value,0); 
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::setIntProperty(char *key,int value)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;

	try
	{
		retCode = JmsMessageSetIntProperty((JmsMessage *)_message,&jstring,value,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::setStringProperty(char* key,char* value)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring,jsMy;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;
	jsMy.stringType = CSTRING;
	jsMy.uniOrC.string =value;

	try
	{
		retCode = JmsMessageSetStringProperty((JmsMessage *)_message,&jstring,&jsMy,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::setDoubleProperty(char* key,double Value)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;

	try
	{
		retCode = JmsMessageSetDoubleProperty((JmsMessage *)_message,&jstring,Value,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::setFloatProperty(char *key,float Value)
{
	ASSERT(_message!=NULL);
	int retCode;
    JmsString jstring;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;

	try
	{
		retCode = JmsMessageSetFloatProperty((JmsMessage *)_message,&jstring,Value,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::setShortProperty(char* key,short Value)
{
	ASSERT(_message!=NULL);
	int retCode;
    JmsString jstring;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;

	try
	{
		retCode = JmsMessageSetShortProperty((JmsMessage *)_message,&jstring,Value,0);
	}
	catch (...) 
	{
		return false;
	}

	return retCode!=JMS_NO_ERROR? false:true;
}

bool Message::setBoolProperty(char* key,bool value)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;
	jstring.stringType = CSTRING;
	jstring.uniOrC.string =key;
	
	try
	{
	    retCode = JmsMessageSetBooleanProperty((JmsMessage *)_message,&jstring,value,0);
	}
	catch (...) 
	{
		return false;
	}
	
	return retCode!=JMS_NO_ERROR? false:true;
}
