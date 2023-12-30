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
// Ident : $Id:  TextMessage.cpp,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : implements class TextMessage, a message type deprived from Message
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JMSCppLib/JMSCpp/TextMessage.cpp $
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
// 4     05-08-26 10:58 Jianjun.li
// 
// 3     05-08-25 17:15 Jianjun.li
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
#include "TextMessage.h"
#include "header\jms.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::JMSCpp;
bool TextMessage::getText(char *text,int size)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;

    jstring.stringType = CSTRING;
    jstring.allocatedSize = size;
    jstring.uniOrC.string = text;
	
	try
	{
		retCode = JmsTextMessageGetText((JmsTextMessage *)_message,&jstring,(JMS64I) 0);
	}
	catch (...) 
	{
		return false;
	}

    return retCode!=JMS_NO_ERROR? false:true;
}

bool TextMessage::setText(char *text)
{
	ASSERT(_message!=NULL);
	int retCode;
	JmsString jstring;

    jstring.stringType = CSTRING;
    jstring.uniOrC.string = text;

	try
	{
		retCode = JmsTextMessageSetText((JmsTextMessage *)_message,&jstring,(JMS64I) 0);
	}
	catch (...) 
	{
		return false;
	}
	
    return retCode!=JMS_NO_ERROR? false:true;
}