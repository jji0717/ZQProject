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
// Ident : $Id:  Destination.cpp,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : implements class Destination
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JMSCppLib/JMSCpp/Destination.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 4     08-01-16 11:12 Ken.qian
// add codes to catch unknow exception
// 
// 3     05-12-22 11:20 Hongquan.zhang
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
#include "Destination.h"
#include "header\jms.h"

using namespace ZQ::JMSCpp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//Destination::
Destination::~Destination()
{
	if(NULL!=_destination)
	{
		try
		{
			JmsDestinationDestroy((JmsDestination *)_destination, 0);
		}
		catch(...) {}

		_destination=NULL;
	}
}
void Destination::destroy()
{
	if(NULL!=_destination)
	{
		try
		{
			JmsDestinationDestroy((JmsDestination *)_destination, 0);
		}
		catch (...) {}
		
		_destination=NULL;
	}
}