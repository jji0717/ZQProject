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
// Ident : $Id:  Consumer.cpp,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : implements class Consumer
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JMSCppLib/JMSCpp/Consumer.cpp $
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
// 5     06-11-30 15:41 Shuai.chen
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
#include "Consumer.h"
#include "header\jms.h"

using namespace ZQ::JMSCpp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
static void
MessageListener(JmsMessage *message, void *argument, JMS64I flags)
{
	if(NULL!=argument)
	{
      Listener *listener = (Listener *)argument;
	  Message ms;
	  ms._message = message;
      listener->onMessage(&ms);
	  ms._message = NULL;
	}

	return ;
}
bool Consumer::receive(__int64 time,Message &ms)
{  
	ASSERT(_consumer!=NULL);
	if (NULL == _consumer)
	{
		return false;
	}

	int retCode;
	JmsMessage *jsMessage = NULL;

	try
	{
		retCode = JmsConsumerReceive((JmsConsumer *)_consumer,time,&jsMessage,0); 
	}
	catch(...)
	{
		return false;
	}
	
    ms._message = jsMessage;
	return retCode!=JMS_NO_ERROR? false:true;
}

bool Consumer::setMessageListener(Listener *listener)
{
	ASSERT(_consumer!=NULL);
	if (NULL == _consumer)
	{
		return false;
	}
   
	int retCode;
	try
	{
		retCode = JmsConsumerSetMessageListener((JmsConsumer *)_consumer,MessageListener, listener, 0); 
	}
	catch(...)
	{
		return false;
	}
	return retCode!=JMS_NO_ERROR? false:true;
}

Consumer::~Consumer()
{
   if(NULL!=_consumer)
   {
	   try
	   {
		   JmsConsumerClose((JmsConsumer *)_consumer, 0);
	   }
	   catch(...) {}

	   _consumer=NULL;
   }
}
void Consumer::close()
{
   if(NULL!=_consumer)
   {
	   try
	   {
		   JmsConsumerClose((JmsConsumer *)_consumer, 0);
	   }
	   catch (...) {};
	   
	   _consumer=NULL;
   }
}