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
// Name  : name.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 3/9/2005
// Desc  : TAO\CORBA naming service
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/ISAAlarm/ISAAlarmImpl/corbalib/name.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 1     05-07-11 3:34p Daniel.wang
// 
// 3     05-06-29 12:38 Yan.zheng
// 
// 2     5/14/05 4:39p Hui.shao
// merged to ZQ dev environment
// 
// 1     5/14/05 2:57p Daniel.wang
// 
// 2     5/14/05 12:29p Hui.shao
// 
// 1     05-03-29 21:28 Daniel.wang
// 
// 2     05-03-22 14:06 Daniel.wang
// ===========================================================================

#ifndef _TAO_CORBA_NAME_H_
#define _TAO_CORBA_NAME_H_

#include "base.h"
#include <tchar.h>
#include <ORBSVCS/ORBSVCS/CosNamingC.h>



/// -----------------------------
/// TaoNamingService
/// -----------------------------
/// Desc : TAO\CORBA naming service
/// @template - 
/// -----------------------------
class TaoNamingService
{
private:
	CosNaming::NamingContext_var	m_NamingContext;
	bool	m_bNamingLoaded;

	TaoServiceBase*	m_tsb;
public:

	/// constructor
	/// @param tsb - ORB object
	/// @param strName - naming service name
	/// @return - 
	TaoNamingService(TaoServiceBase* tsb, const char* strName = "NameService");


	/// destructor
	/// @param  - 
	/// @return - 
	~TaoNamingService();


	/// judge the naming service if loaded
	/// @param  - 
	/// @return - return true if loaded
	bool IsLoaded(void)
	{ return m_bNamingLoaded; }


	/// set the ORB object
	/// @param tsb - ORB object
	/// @return - return true if ok
	bool SetServiceBase(TaoServiceBase* tsb);


	/// initialize naming serivce
	/// @param strName - naming service name
	/// @return - return true if ok
	bool Init(const char* strName);

	/// destroy naming service
	/// @param  - 
	/// @return - 
	void Destroy(void);


	/// resolve TAO\CORBA object from naming service 
	/// @param str - TAO\CORBA object name
	/// @return - return object
	CORBA::Object_var ResolveObject(const char* str);


	/// bind a name into naming service
	/// @template _INTYPE - the type of object in_type
	/// @param str - object name
	/// @param in - object in_type
	/// @return - return true if ok
	template <typename _INTYPE>
	bool Bind(const char* str, _INTYPE in)
	{
		if (!m_bNamingLoaded)
			return false;

		CosNaming::Name cname(1);
		cname.length(1);
		cname[0].id = CORBA::string_dup(str);

		m_NamingContext->bind(cname, in);
		return true;
	}

	/// rebind a name into naming service
	/// @template _INTYPE - the type of object in_type
	/// @param str - object name
	/// @param in - object in_type
	/// @return - return true if ok
	template <typename _INTYPE>
	bool ReBind(const char* str, _INTYPE in)
	{
		if (!m_bNamingLoaded)
			return false;

		CosNaming::Name cname(1);
		cname.length(1);
		cname[0].id = CORBA::string_dup(str);

		m_NamingContext->rebind(cname, in);
		return true;
	}
};


#endif//_TAO_CORBA_NAME_H_
