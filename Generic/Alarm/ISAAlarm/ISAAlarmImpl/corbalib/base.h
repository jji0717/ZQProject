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
// Name  : base.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 3/8/2005
// Desc  : TAO\CORBA ORB
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/ISAAlarm/ISAAlarmImpl/corbalib/base.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 2     05-07-14 14:37 Daniel.wang
// 
// 1     05-07-11 3:33p Daniel.wang
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

#ifndef _TAO_CORBA_BASE_H_
#define _TAO_CORBA_BASE_H_

#include <TAO/exception.h>
#include <TAO/orb.h>
#include <TAO/object.h>

#ifdef _DEBUG
#	pragma comment(lib, "TAO_CosEventd.lib")
#	pragma comment(lib, "TAO_CosNotificationd.lib")
#	pragma comment(lib, "TAO_CosNotification_Skeld.lib")
#	pragma comment(lib, "TAO_PortableServerd.lib")
#	pragma comment(lib, "TAO_CosEvent_Skeld.lib")
// #	pragma comment(lib, "TAO_ObjRefTemplated.lib")
#	pragma comment(lib, "TAO_CosNamingd.lib")
#	pragma comment(lib, "TAO_Svc_Utilsd.lib")
#	pragma comment(lib, "TAO_ETCLd.lib")
#	pragma comment(lib, "TAO_IORTabled.lib")
#	pragma comment(lib, "TAO_DynamicAnyd.lib")
#	pragma comment(lib, "TAOd.lib")
#	pragma comment(lib, "ACEd.lib")
#else//_DEBUG
#	pragma comment(lib, "TAO_CosEvent.lib")
#	pragma comment(lib, "TAO_CosNotification.lib")
#	pragma comment(lib, "TAO_CosNotification_Skel.lib")
#	pragma comment(lib, "TAO_PortableServer.lib")
#	pragma comment(lib, "TAO_CosEvent_Skel.lib")
// #	pragma comment(lib, "TAO_ObjRefTemplate.lib")
#	pragma comment(lib, "TAO_CosNaming.lib")
#	pragma comment(lib, "TAO_Svc_Utils.lib")
#	pragma comment(lib, "TAO_ETCL.lib")
#	pragma comment(lib, "TAO_IORTable.lib")
#	pragma comment(lib, "TAO_DynamicAny.lib")
#	pragma comment(lib, "TAO.lib")
#	pragma comment(lib, "ACE.lib")
#endif//_DEBUG

/// exception 
typedef CORBA::Exception TaoException;

#define SHOW_EXP(exception) exception._tao_print_exception(0)

#define NARROW_VAR(_FACTORY, data) _FACTORY::_narrow((data).in())


/// -----------------------------
/// TaoServiceBase
/// -----------------------------
/// Desc : TaoServiceBase is a necessery object for all corba application.
/// you must create a TaoServiceBase before all TAO classes
/// @template - 
/// -----------------------------
class TaoServiceBase
{
private:
	CORBA::ORB_var	m_Orb;
	bool			m_bOrbLoaded;
	bool			m_bOrbRunning;

public:
	/// constructor
	/// @param argc - TAO\CORBA application arguments count
	/// @param argv - TAO\CORBA application arguments
	/// @return -
	TaoServiceBase(int argc, char* argv[]);

	/// destructor
	/// @param  - 
	/// @return  - 
	~TaoServiceBase();

	/// judge ORB if loadeds
	/// @param  - 
	/// @return  - return true if loaded and false other wise
	bool IsLoaded()
	{ return m_bOrbLoaded; }

	/// judge ORB if running
	/// @param  - 
	/// @return  - return true if running and false other wise
	bool IsRunning()
	{ return m_bOrbRunning; }


	/// initialize ORB
	/// @param argc - TAO\CORBA application arguments count
	/// @param argv - TAO\CORBA application arguments
	/// @return  - return true if initialized
	bool Init(int argc, char* argv[]);

	/// destroy ORB
	/// @param  - 
	/// @return  - 
	void Destroy(void);

	/// shutdown ORB
	void Shutdown(void);

	/// run ORB
	/// @param  - 
	/// @return  - return true if run ok
	bool Run(void);


	/// load CORBA object from ORB
	/// @param str - object name
	/// @return  - return object instance
	CORBA::Object_var LoadObject(const char* str);

	/// resolve CORBA object from ORB
	/// @param str - object name
	/// @return  - return object instance
	CORBA::Object_var ResolveObject(const char* str);

	/// get ORB instance
	/// @param  - 
	/// @return - return OBR instance
	CORBA::ORB_var& GetORB(void)
	{ return m_Orb; }

};

#endif//_TAO_CORBA_BASE_H_
