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
// Name  : base.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 3/9/2005
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/ISAAlarm/ISAAlarmImpl/corbalib/base.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 1     05-07-11 3:33p Daniel.wang
// 
// 1     5/14/05 2:57p Daniel.wang
// 
// 1     05-03-29 21:28 Daniel.wang
// 
// 2     05-03-22 14:06 Daniel.wang
// ===========================================================================

#include "base.h"



TaoServiceBase::TaoServiceBase(int argc, char* argv[])
:m_bOrbLoaded(false), m_bOrbRunning(false)
{
	Init(argc, argv);
}

TaoServiceBase::~TaoServiceBase()
{
	Destroy();
}


bool TaoServiceBase::Init(int argc, char* argv[])
{
	m_Orb = CORBA::ORB_init(argc, argv, "");
	m_bOrbLoaded = true;

	return m_bOrbLoaded;
}

void TaoServiceBase::Destroy(void)
{
	m_bOrbRunning = false;

	if (m_bOrbLoaded)
		m_Orb->destroy();
}

void TaoServiceBase::Shutdown(void)
{
	m_bOrbRunning = false;

	if (m_bOrbLoaded)
		m_Orb->shutdown();
}

bool TaoServiceBase::Run(void)
{
	if (!m_bOrbLoaded)
		return false;

	m_Orb->run();
	m_bOrbRunning = true;

	return true;
}

CORBA::Object_var TaoServiceBase::LoadObject(const char* str)
{
	if (!m_bOrbLoaded)
		return NULL;

	return  m_Orb->string_to_object(str);
}

CORBA::Object_var TaoServiceBase::ResolveObject(const char* str)
{
	if (!m_bOrbLoaded)
		return NULL;

	return m_Orb->resolve_initial_references(str);
}
