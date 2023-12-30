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
// Ident : $Id:  Session.cpp,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : implements class Session
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JMSCppLib/JMSCpp/Session.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 7     08-01-16 11:12 Ken.qian
// add codes to catch unknow exception
// 
// 6     06-11-30 15:42 Shuai.chen
// 
// 5     06-01-26 15:39 Hongquan.zhang
// 
// 4     05-12-22 11:20 Hongquan.zhang
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
#include "Session.h"
#include "header\jms.h"

using namespace ZQ::JMSCpp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Session::~Session()
{
	if(NULL!=_session)
	{
		try
		{
			JmsSessionClose((JmsSession *)_session, 0);
		}
		catch (...) { }

		_session=NULL;
	}
}
void Session::close()
{
	if(NULL!=_session)
	{
		try
		{
			JmsSessionClose((JmsSession *)_session, 0);
		}
		catch (...) { }
		
		_session=NULL;
	}
}

bool  Session::createDurableSubscriber(Destination *dn,char *name,char *selector,Consumer &cons)
{
	ASSERT(NULL!=_session && NULL != name);
	if (NULL == _session || NULL == name)
	{
		return false;
	}

	JmsString jsname,jselector;
	JmsConsumer *jmsConsumer = NULL;
    int retCode;
	
	JmsString*		pSelector=NULL;

    jsname.stringType = CSTRING;
	jsname.uniOrC.string = name;
	if(selector)
	{
		jselector.stringType = CSTRING;
		jselector.uniOrC.string = selector;		
		pSelector=&jselector;
	}

	try
	{
		retCode = JmsSessionCreateDurableSubscriber((JmsSession *)_session,(JmsDestination *) dn->_destination,
			&jsname,pSelector,1,&jmsConsumer,0);
	}
	catch (...) 
	{
		return false;
	}
	
    if(JMS_NO_ERROR!=retCode)
       return false;
    cons._consumer = jmsConsumer;
	return true;
}

bool Session::unSubscribe(char *name)
{
    ASSERT(NULL!=_session && NULL != name);
	if (NULL == _session || NULL == name)
	{
		return false;
	}

	int retCode;
	JmsString jstring;
	
	jstring.stringType = CSTRING;
	jstring.uniOrC.string = name;

	try
	{
		retCode = JmsSessionUnsubscribe((JmsSession *)_session,&jstring,0);
	}
	catch (...) 
	{
		return false;
	}

    if(JMS_NO_ERROR!=retCode)
       return false;
	return true;
}

bool  Session::createProducer(Destination *dn,Producer &pro)
{
	ASSERT(NULL!=_session && NULL != dn->_destination);
	if (NULL == _session || NULL == dn->_destination)
	{
		return false;
	}
		
	JmsProducer *jmsProducer = NULL;
	int retCode;

	try
	{
		retCode = JmsSessionCreateProducer((JmsSession *)_session,(JmsDestination *) dn->_destination,&jmsProducer, 0);
	}
	catch (...) 
	{
		return false;
	}
	
	if(JMS_NO_ERROR!=retCode)
       return false;
	pro._producer = jmsProducer;
	return true;
}

bool  Session::createConsumer(Destination *dn,Consumer &cons)
{
	ASSERT(NULL!=_session && NULL != dn->_destination);
	if (NULL == _session || NULL == dn->_destination)
	{
		return false;
	}

    JmsConsumer *jmsConsumer = NULL;
	int retCode;

	try
	{
		retCode = JmsSessionCreateConsumer((JmsSession *)_session,(JmsDestination *) dn->_destination,NULL,0,&jmsConsumer, 0);
	}
	catch (...) 
	{
		return false;
	}

	if(JMS_NO_ERROR!=retCode)
       return false;
    cons._consumer = jmsConsumer;
	return true;
}

bool  Session::textMessageCreate(char *text,Message &ms)
{	
	ASSERT(NULL!=_session);
	if (NULL == _session)
	{
		return false;
	}

	JmsString jstring;
	JmsTextMessage *jmsMessage = NULL;
    int retCode;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string = text;

	try
	{
		retCode = JmsSessionTextMessageCreate((JmsSession *)_session,&jstring, &jmsMessage, (JMS64I)0);
	}
	catch (...) 
	{
		return false;
	}

    if(JMS_NO_ERROR!=retCode)
       return false;
	ms._message = jmsMessage;
	return true;
}

bool  Session::mapMessageCreate(Message &ms)
{
	ASSERT(NULL!=_session);
	if (NULL == _session)
	{
		return false;
	}

	int retCode;
	JmsMapMessage *jmsMessage = NULL;
	
	try
	{
		retCode = JmsSessionMapMessageCreate((JmsSession *)_session,&jmsMessage,0);
	}
	catch (...) 
	{
		return false;
	}
	
    if(JMS_NO_ERROR!=retCode)
       return false;
    ms._message = jmsMessage;
	return true;
}

bool  Session::bytesMessageCreate(Message &ms)
{
	ASSERT(NULL!=_session);
	if (NULL == _session)
	{
		return false;
	}
	  
	int retCode;
    JmsBytesMessage *jmsMessage = NULL;

	try
	{
		retCode = JmsSessionBytesMessageCreate((JmsSession *)_session,&jmsMessage,0); 
	}
	catch (...) 
	{
		return false;
	}
	
	if(JMS_NO_ERROR!=retCode)
       return false;
    ms._message = jmsMessage;
	return true;
}
