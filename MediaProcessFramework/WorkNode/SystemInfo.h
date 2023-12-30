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

#include <conio.h>
#include <stdio.h>

#pragma warning(disable : 4786)
#include <vector>
#include <string>
#include <map>
using namespace std;


#include "MPFCommon.h"

MPF_SYSTEMINFO_NAMESPACE_BEGIN

//cpu info******************************

class SystemInfo
{
	//////////////////////////////////////////////////////////////////////////
	// General
public:
	static void					Init();


	//////////////////////////////////////////////////////////////////////////
	// for CPU
public:
	static int					CPUGetUsage();
	static string				CPUGetFixInfo();

private:
	static LARGE_INTEGER        liOldIdleTime;
	static LARGE_INTEGER        liOldSystemTime;

	//////////////////////////////////////////////////////////////////////////
	// for Memory
public:
	static long					MemoryGetAvailable();
	static long					MemoryGetTotal();


	//////////////////////////////////////////////////////////////////////////
	// for Network
public:
	static void					NetworkSetTrafficType(int trafficType);		
	static DWORD				NetworkGetInterfaceTotalTraffic(int index);	
	static BOOL					NetworkGetInterfaceName(string &InterfaceName, int index);
	static int					NetworkGetInterfacesCount();
	static double				NetworkGetTraffic(int interfaceNumber);
	static DWORD				NetworkGetInterfaceBandwidth(int index);

private:
	static BOOL					NetworkGetInterfaces();
	static double				lasttraffic;
	static vector<string>		Interfaces;		
	static vector< long>		Bandwidths;	
	static map<string,long>		TotalTraffics;
	static int					CurrentInterface;
	static int					CurrentTrafficType;
	
	//////////////////////////////////////////////////////////////////////////
	
};

MPF_SYSTEMINFO_NAMESPACE_END

#endif
