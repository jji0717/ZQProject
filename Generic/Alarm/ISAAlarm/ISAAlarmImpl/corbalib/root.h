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
// Name  : root.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 3/9/2005
// Desc  : TAO\CORBA root poa 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/ISAAlarm/ISAAlarmImpl/corbalib/root.h $
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

#ifndef _TAO_CORBA_ROOT_H_
#define _TAO_CORBA_ROOT_H_

#include "base.h"
#include <tchar.h>
#include <tao\portableserver\portableserverc.h>


/// -----------------------------
/// TaoRootPOA
/// -----------------------------
/// Desc : TAO\CORBA root poa 
/// @template - 
/// -----------------------------
class TaoRootPOA
{
private:
	PortableServer::POA_var	m_Poa;
	PortableServer::POAManager_var	m_PoaManager;
	bool	m_bPosLoad;
	bool	m_bPosRunning;

	TaoServiceBase*	m_tsb;
public:
	/// constructor
	/// @param tsb - ORB object
	/// @param strName - Root poa name
	/// @return - 
	TaoRootPOA(TaoServiceBase* tsb, const char* strName = "RootPOA");

	/// destructor
	/// @param  - 
	/// @return - 
	~TaoRootPOA();


	/// judge if the root poa loaded
	/// @param  - 
	/// @return - return true if loaded
	bool IsLoaded()
	{ return m_bPosLoad; }

	/// judge if the root poa running
	/// @param  - 
	/// @return - return true if running
	bool IsRunning()
	{ return m_bPosRunning; }


	/// set the orb object
	/// @param tsb - ORB object
	/// @return - return true if ok
	bool SetServiceBase(TaoServiceBase* tsb);

	/// initialize root poa object
	/// @param strName - root poa name
	/// @return - return true if ok
	bool Init(const char* strName);

	/// destroy root poa
	/// @param  - 
	/// @return - 
	void Destroy(void);

	/// run root poa object
	/// @param  - 
	/// @return - return true if ok
	bool Run(void);
};


#endif//_TAO_CORBA_ROOT_H_
