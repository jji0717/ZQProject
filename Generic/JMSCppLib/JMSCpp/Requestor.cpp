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
// Ident : $Id:  Requestor.cpp,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : implements class Requestor
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JMSCppLib/JMSCpp/Requestor.cpp $
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
// 5     06-12-06 18:24 Ken.qian
// 
// 4     05-08-26 10:58 Jianjun.li
// 
// 3     05-08-11 16:22 Jianjun.li
// 
// 2     05-07-28 16:58 Jianjun.li
// 
// 1     05-07-28 10:44 Jianjun.li
// 
// 1     05-07-27 18:48 Jianjun.li
// 
// 5     05-07-27 18:12 Jianjun.li
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
#include <memory.h>

#include "Requestor.h"
#include "header\jms.h"

using namespace ZQ::JMSCpp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Requestor::Requestor(Session *ses,Destination *des)
{
	ASSERT(ses!=NULL&&des!=NULL);
	
	JmsDestination *jmsDes = NULL;

	ses->createProducer(des,_pro);
	try
	{
		JmsSessionCreateTemporaryQueue((JmsSession *)ses->_session,&jmsDes,0);
	}
	catch (...) 
	{
		jmsDes = NULL;

		return;
	}
	
	_tempDestination._destination = jmsDes;
	ses->createConsumer(&_tempDestination,_con);
}

bool Requestor::request(Message *req, Message &response, __int64 timeout, __int64 timeToLive)
{
    ASSERT(req!=NULL);

	if(NULL == _pro._producer || NULL == _tempDestination._destination)
		return false;
	
	ProducerOptions* pOption = NULL;

	ProducerOptions	op;
	memset(&op, 0x0, sizeof(ProducerOptions));

	// set the jms message TimeToLive
	if(timeToLive > 0)
	{
		op.flags = PO_TIMETOLIVE;
		op.timeToLive = timeToLive;   
		
		pOption = &op;
	}

	int retCode;
	req->setReplyTo(&_tempDestination);
	retCode =_pro.send(req, pOption);
	if(!retCode)
		return false;
	retCode =_con.receive(timeout,response);
    if(!retCode)
		return false;
	return true;
}

/*
Requestor::~Requestor()
{
  //  JmsDestinationDestroy((JmsDestination *)_tempDestination._destination,0);
}
*/