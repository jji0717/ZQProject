// ===========================================================================
// Copyright (c) 2004 by
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
// Name  : name.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 3/9/2005
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/ISAAlarm/ISAAlarmImpl/corbalib/name.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 1     05-07-11 3:34p Daniel.wang
// 
// 1     5/14/05 2:57p Daniel.wang
// 
// 1     05-03-29 21:28 Daniel.wang
// 
// 2     05-03-22 14:06 Daniel.wang
// ===========================================================================

#include "name.h"


TaoNamingService::TaoNamingService(TaoServiceBase* tsb, const char* strName)
{
	SetServiceBase(tsb);
	Init(strName);
}

TaoNamingService::~TaoNamingService()
{
	Destroy();
}


bool TaoNamingService::SetServiceBase(TaoServiceBase* tsb)
{
	m_tsb = tsb;
	return (NULL != m_tsb);
}

bool TaoNamingService::Init(const char* strName)
{
	if (NULL == m_tsb)
		return false;

	m_NamingContext = NARROW_VAR(CosNaming::NamingContext, m_tsb->ResolveObject(strName));
	m_bNamingLoaded = true;
	return true;
}

void TaoNamingService::Destroy(void)
{
	if (m_bNamingLoaded)
		m_bNamingLoaded = false;
}

CORBA::Object_var TaoNamingService::ResolveObject(const char* str)
{
	if (!m_bNamingLoaded)
		return NULL;

	CosNaming::Name cname(1);
	cname.length(1);
	cname[0].id = CORBA::string_dup(str);

	return m_NamingContext->resolve(cname);
}