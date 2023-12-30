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
// Ident : $Id:  Connection.cpp,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : implements class Connection
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JMSCppLib/JMSCpp/Connection.cpp $
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
#include "Connection.h"
#include "header\jms.h"
#include <string>

using namespace ZQ::JMSCpp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool Connection::createSession(Session &se)
{
	ASSERT(_connection!=NULL);
	if (NULL == _connection)
	{
		return false;
	}
	
	JmsSession *jmsSession=NULL;
	int retCode;
	try
	{
		retCode = JmsConnectionCreateSession((JmsConnection *)_connection, 0,AUTO_ACKNOWLEDGE, &jmsSession, 0);
	}
	catch (...)
	{
		return false;
	}

    if(JMS_NO_ERROR!=retCode)
		return false;
    se._session = jmsSession;
	return true;
}

bool Connection::getClientID(char *identifier,int size)
{
 	ASSERT(_connection!=NULL);
	if (NULL == _connection)
	{
		return false;
	}

	int retCode;
	JmsString jstring;
	
	jstring.stringType = CSTRING;
	jstring.uniOrC.string = (char *)identifier;
	jstring.allocatedSize = size;
	try
	{
		retCode = JmsConnectionGetClientId((JmsConnection *)_connection,&jstring ,0);
	}
	catch (...) 
	{
		return false;
	}

    return retCode!=JMS_NO_ERROR? false:true;
}

bool Connection::setClientID(char *identifier)
{
 	ASSERT(_connection!=NULL);
	if (NULL == _connection)
	{
		return false;
	}

	int retCode;
	JmsString jstring;
	
	jstring.stringType = CSTRING;
	jstring.uniOrC.string = (char *)identifier;
	
	try
	{
		retCode = JmsConnectionSetClientId((JmsConnection *)_connection,&jstring,0);
	}
	catch(...)
	{
		return false;
	}
	
    return retCode!=JMS_NO_ERROR? false:true;
}

bool Connection::start()
{
	ASSERT(_connection!=NULL);
	if (NULL == _connection)
	{
		return false;
	}

	int retCode;
	try
	{
		retCode = JmsConnectionStart((JmsConnection *)_connection, (JMS64I)0);
	}
	catch(...)
	{
		return false;
	}
	return retCode!=JMS_NO_ERROR? false:true;
}

bool Connection::stop()
{
	ASSERT(_connection!=NULL);
	if (NULL == _connection)
	{
		return false;
	}
		
	int retCode;
	try
	{
		retCode = JmsConnectionStop((JmsConnection *)_connection, (JMS64I)0);
	}
	catch(...)
	{
		return false;
	}
	
	return retCode!=JMS_NO_ERROR? false:true;
}

void Connection::close()
{
	if(NULL!=_connection)
	{
		stop();
		try
		{
			JmsConnectionClose((JmsConnection *)_connection, 0);	
		}
		catch(...) {}
		
		_connection=NULL;
	}
}

Connection::~Connection()
{
	if(NULL!=_connection)
	{
		try
		{
			JmsConnectionClose((JmsConnection *)_connection, 0);
		}
		catch(...) {}
		
		_connection=NULL;
	}
}

void Connection::SetConnectionCallback(ConnExceptionCallBack pCallBack,void* lpData)
{
	ASSERT(NULL!=_connection);
	
	if (NULL == _connection)
	{
		return;
	}

	_pCallBack=pCallBack;
	_lpData=lpData;
}
void Connection::SetConnectionCallback2(ConnExceptionCallBack2 pCallBack,void* lpData)
{
	ASSERT(NULL!=_connection);

	if (NULL == _connection)
	{
		return;
	}

	_pCallBack2=pCallBack;
	_lpData=lpData;
}
void Connection::fireConnCallback(int ErrType,const std::string& strErr)
{
	if (NULL == _connection)
	{
		return;
	}

	if(_pCallBack)
	{
		try
		{
			_pCallBack(ErrType,_lpData);
		}
		catch(...)
		{
		}
	}
	if(_pCallBack2)
	{
		try
		{
			_pCallBack2(ErrType,strErr.c_str(),_lpData);
		}
		catch(...)
		{
		}
	}
}