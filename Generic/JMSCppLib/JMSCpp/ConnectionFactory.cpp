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
// Ident : $Id:  ConnectionFactory.cpp,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : implements class ConnectionFactory
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JMSCppLib/JMSCpp/ConnectionFactory.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 8     08-01-16 11:12 Ken.qian
// add codes to catch unknow exception
// 
// 7     08-01-04 14:28 Ken.qian
// change callback routine prototype
// 
// 6     06-11-30 15:40 Shuai.chen
// 
// 5     06-11-30 12:18 Hongquan.zhang
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
#include "ConnectionFactory.h"
#include "Context.h"
#include "header\jms.h"
#include <string>

using namespace ZQ::JMSCpp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//typedef void (*ExceptionCallBack) (JmsException *, void *, JMS64I);
//ExceptionCallBack	pGlobalCallBack;
void	ConnectionExceptionCallback(JmsException *JException, void *argument, JMS64I flag)
{
	ASSERT(NULL!=argument);

	if (NULL == argument)
	{
		return;
	}
	
	Connection* pConn=(Connection*)argument;

	// get error string
	JmsString	errMsg;
	char szMsg[1024];
	errMsg.allocatedSize=1023;
	errMsg.stringType=CSTRING;
	errMsg.uniOrC.string=szMsg;
	JmsThrowableGetMessage(JException,&errMsg,0);
	
	// get string type error code
	JmsString	errCode;
	char szCode[1024];
	errCode.allocatedSize=1023;
	errCode.stringType=CSTRING;
	errCode.uniOrC.string=szCode;
	JmsExceptionGetErrorCode(JException,&errCode,0);
	
	// get the full message
	std::string strErrMsg=std::string(szMsg) + std::string("[") + std::string(szCode) + std::string("]");
	
	pConn->fireConnCallback(getLastJmsError(),strErrMsg);
}

bool ConnectionFactory::createConnection(Connection &conn)
{
	ASSERT(_connectionFactory!=NULL);

	if (NULL == _connectionFactory)
	{
		return false;
	}
		
	JmsConnection *jmsConnection = NULL;
	int retCode;
	
	try
	{
		retCode = JmsConnectionFactoryCreateConnection((JmsConnectionFactory *)_connectionFactory,&jmsConnection, 0);
	}
	catch(...)
	{
		return false;
	}
	
    if(retCode!=JMS_NO_ERROR)
  	  return false;
	conn._connection = jmsConnection;

	try
	{
		JmsConnectionSetExceptionListener((JmsConnection*)conn._connection,ConnectionExceptionCallback,&conn,0);
	}
	catch (...) 
	{
		return false;
	}

	return true;
}

ConnectionFactory::~ConnectionFactory()
{
	if(NULL!=_connectionFactory)
	{
		try
		{
			JmsConnectionFactoryDestroy((JmsConnectionFactory *)_connectionFactory, 0);
		}
		catch (...) {}

		_connectionFactory=NULL;
	}
}
void ConnectionFactory::Destroy()
{
	if(NULL!=_connectionFactory)
	{
		try
		{
			JmsConnectionFactoryDestroy((JmsConnectionFactory *)_connectionFactory, 0);
		}
		catch(...) {}

		_connectionFactory=NULL;
	}
}