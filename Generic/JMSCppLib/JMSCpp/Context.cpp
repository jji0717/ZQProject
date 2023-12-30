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
// Ident : $Id:  Context.cpp,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : implements class Context
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JMSCppLib/JMSCpp/Context.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 11    08-01-17 14:53 Ken.qian
// Correct the mistake change of getLastJmsError 
// 
// 10    08-01-17 14:52 Ken.qian
// 
// 9     08-01-16 11:12 Ken.qian
// add codes to catch unknow exception
// 
// 8     08-01-15 15:21 Jie.zhang
// add some try catch to catch some unknwon exception
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
#include "Context.h"
#include "header\jms.h"

using namespace ZQ::JMSCpp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Context::Context(const char *pcURL,const char *pcNamingContextFactory)
{
   JmsString jstring,jsMy;

   _context=NULL;
   jstring.stringType = CSTRING;
   jsMy.stringType=CSTRING;
   jstring.uniOrC.string =(char *)pcURL;
   jsMy.uniOrC.string=(char *)pcNamingContextFactory;

   try
   {
	  JmsContextCreate(&jstring, &jsMy, NULL, NULL,(JmsContext **) &_context, 0); 
   }
   catch(...)
   {
	   _context = NULL;
   };
}

Context::Context(const char *pcURL,const char *pcNamingContextFactory,char* userName,char* UserPwd)
{
  JmsString jstring,jsMy;
  JmsString	jsUserName,jsUserPWd;

   _context=NULL;
   jstring.stringType = CSTRING;
   jsMy.stringType=CSTRING;
   jstring.uniOrC.string =(char *)pcURL;
   jsMy.uniOrC.string=(char *)pcNamingContextFactory;

   jsUserName.stringType=CSTRING;
   jsUserName.uniOrC.string=(char*)userName;

   jsUserPWd.stringType=CSTRING;
   jsUserPWd.uniOrC.string=(char*)UserPwd;

   try
   {
	  JmsContextCreate(&jstring, &jsMy, &jsUserName, &jsUserPWd,(JmsContext **) &_context, 0);
   }
   catch(...)
   {
	   _context = NULL;
   };
}

bool Context::createConnectionFactory(const char *factory, ConnectionFactory & connFac)
{
	ASSERT(NULL!=_context && NULL!=factory);
	if (NULL == _context || NULL == factory)
	{
		return false;
	}
		
	JmsString jstring;
	JmsConnectionFactory *jmsFactory = NULL;
    int retCode;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string = (char *)factory;
	
	try
    {
		retCode = JmsContextCreateConnectionFactory((JmsContext *)_context, &jstring,&jmsFactory, 0);
	}
	catch(...)
	{
		return false;
	}

	if(JMS_NO_ERROR!=retCode)
		return false;
    connFac._connectionFactory = jmsFactory;
	return true;
}

bool Context::createDestination(const char *destination, Destination & des)
{
	ASSERT(NULL!=destination && NULL!=_context);
	if (NULL == destination || NULL == _context)
	{
		return false;
	}

	JmsString jstring;
	int retCode;
	JmsDestination *jmsDestination = NULL;

	jstring.stringType = CSTRING;
	jstring.uniOrC.string = (char *)destination;
	
	try
	{
		retCode = JmsDestinationCreate((JmsContext *)_context, &jstring, &jmsDestination, 0);
	}
	catch(...)
	{
		return false;
	}

    if(JMS_NO_ERROR!=retCode)
		return false;
	des._destination = jmsDestination;
	return true;
}

Context::~Context()
{
	if(NULL!=_context)
	{
		try
		{
			JmsContextDestroy((JmsContext *)_context, 0);
		}
		catch(...)
		{
		}
		_context=NULL;
	}
}
void Context::destroy()
{
	if(NULL!=_context)
	{
		try
		{
			JmsContextDestroy((JmsContext *)_context, 0);
		}
		catch(...)
		{
		}
		_context=NULL;
	}
}
int ZQ::JMSCpp::getLastJmsError()
{
	JavaThrowable	*JException=NULL;
	int				ErrType=0;

	try
	{
		// No JmsExceptionDestroy called
		JmsGetLastException(&JException,&ErrType,JMS_PEEK_ONLY);
	}
	catch(...){};

	return ErrType;
}
//char* ZQ::JMSCpp::GetLastJmsErrorDesc()
//{
//	JavaThrowable	*JException=NULL;
//	int				ErrType;
//	JmsString		ErrStr;
//	if(JmsGetLastException(&JException,&ErrType,JMS_PEEK_ONLY)!=JMS_NO_ERROR)
//		return NULL;
//	if(JmsExceptionGetErrorCode((JmsException*)JException,&ErrStr,0)!=JMS_NO_ERROR)
//		return NULL;
//	return NULL;
//}