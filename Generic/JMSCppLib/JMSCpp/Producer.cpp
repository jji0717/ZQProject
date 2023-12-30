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
// Ident : $Id:  Producer.cpp,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : implements class Producer
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JMSCppLib/JMSCpp/Producer.cpp $
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
// 6     06-11-30 15:41 Shuai.chen
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
#include "Producer.h"
#include "header\jms.h"

using namespace ZQ::JMSCpp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//bool Producer::send(Message *message)
//{
//	ASSERT(NULL!=_producer && message!=NULL);
//		
//	int retCode;
//	retCode = JmsProducerSend((JmsProducer *)_producer, NULL,(JmsMessage *)message->_message,(JMS64I) 0);
//	return retCode!=JMS_NO_ERROR? false:true;
//}
bool Producer::send(Message* ms,ProducerOptions *op)
{
	//ASSERT(NULL!=ms&&NULL!=op&&NULL!=_producer);
	ASSERT(NULL!=_producer&&NULL!=ms);
	if (NULL == _producer || NULL == ms)
	{
		return false;
	}

	int retCode=0;
	if(op!=NULL)
	{
		JmsProducerOptions tmpOp;
		tmpOp.deliveryMode = op->deliveryMode;
		
		if(op->flags&PO_DESTINATION)
			tmpOp.destination = (JmsDestination*)(op->destination->_destination);
		else 
			tmpOp.destination=NULL;
		tmpOp.flags = op->flags;
		tmpOp.priority = op->priority;
		tmpOp.timeToLive = op->timeToLive;

		try
		{
			retCode=JmsProducerSend((JmsProducer *)_producer, &tmpOp, (JmsMessage *)ms->_message,(JMS64I) 0);
		}
		catch (...) 
		{
			return false;
		}
	}
	else
	{
		try
		{
			retCode=JmsProducerSend((JmsProducer *)_producer, NULL,(JmsMessage *)ms->_message,(JMS64I) 0);
		}
		catch (...) 
		{
			return false;
		}
	}

	return retCode!=JMS_NO_ERROR? false:true;
}
Producer::~Producer()
{
	if(NULL!=_producer)
	{
		try
		{
			JmsProducerClose((JmsProducer *)_producer, 0);
		}
		catch (...) { }

		_producer=NULL;
	}
}
void Producer::close()
{
	if(NULL!=_producer)
	{
		try
		{
			JmsProducerClose((JmsProducer *)_producer, 0);
		}
		catch (...) { }
		
		_producer=NULL;
	}
}