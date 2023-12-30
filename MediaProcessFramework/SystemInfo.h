// ===========================================================================
// Copyright (c) 2005 by
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
// Ident : $Id: SystemInfo.h,v 1.0 2005/05/08 16:34:35 Gu Exp $
// Branch: $Name:  $
// Author: Hongye Gu
// Desc  : get cpu and memory information
//
// Revision History: 
// ---------------------------------------------------------------------------

#ifndef __SYSTEMINFO_H__
#define __SYSTEMINFO_H__



#pragma warning(disable : 4786)
#include <map>
#include <string>

#include "MPFCommon.h"


MPF_SYSTEMINFO_NAMESPACE_BEGIN

#define MAX_OS_VERION_STRLEN	256
#define MAX_INTERFACES			32

using namespace std;
//cpu info******************************

class SystemInfo
{
	//////////////////////////////////////////////////////////////////////////
	// General
public:
	static void					Init();
	static void					UnInit();


	//////////////////////////////////////////////////////////////////////////
	// for CPU
public:
	static int					CPUGetUsage();
	static string				CPUGetFixInfo();

private:
	static LARGE_INTEGER        liOldIdleTime;
	static LARGE_INTEGER        liOldSystemTime;


	//////////////////////////////////////////////////////////////////////////
	// for OS version
public:
	static const char*			OSGetVersion();

private:
	static char					strOsVersion[MAX_OS_VERION_STRLEN];

	//////////////////////////////////////////////////////////////////////////
	// for Memory
public:
	static long					MemoryGetAvailable();
	static long					MemoryGetTotal();


	//////////////////////////////////////////////////////////////////////////
	// for Network
	enum TrafficType
	{
		AllTraffic		= 388,//total traffic
		IncomingTraffic	= 264,//in traffic
		OutGoingTraffic	= 506 //out traffic
	};

public:
	static void					NetworkSetTrafficType(TrafficType trafficType);		
	static int					NetworkGetInterfaceIndexByIP(const char* ipaddr);
	static int					NetworkGetInterfaceIndexByName(const char* itname);
	static BOOL					NetworkGetInterfaceName(string &InterfaceName, int index);
	static int					NetworkGetInterfacesCount();
	static DWORD				NetworkGetInterfaceBandwidth(int index);
	static double				NetworkGetInterfaceTraffic(int interfaceNumber);

private:
	static BOOL					NetworkGetInterfaces();
	static map<DWORD, string>	Interfaces;		
	static map<DWORD, DWORD>	Bandwidths;	
	static DWORD				InBytes;
	static DWORD				OutBytes;
	static DWORD				TotalBytes;
	static time_t				ElapseTime;
	static TrafficType			CurrentTrafficType;
	
	//////////////////////////////////////////////////////////////////////////
	
};

MPF_SYSTEMINFO_NAMESPACE_END

#endif
