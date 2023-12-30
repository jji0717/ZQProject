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
// Name  : SystemInfo_def.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-5-11
// Desc  : common define for systemHardware class
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SystemInfo_def.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 3     05-06-14 19:43 Bernie.zhao
// 
// 2     05-06-07 18:08 Bernie.zhao
// fixed network interface problem
// 
// 1     05-06-01 1:49p Daniel.wang
// move from worknode
// 
// 2     05-05-11 13:48 Bernie.zhao
// got rid of hardcoded parts
// ===========================================================================
#ifndef __SYSTEMINFO_DEF_H__
#define __SYSTEMINFO_DEF_H__

#include "winperf.h"

MPF_SYSTEMINFO_NAMESPACE_BEGIN
//////////////////////////////////////////////////////////////////////////
// for CPU
//////////////////////////////////////////////////////////////////////////
#define SystemBasicInformation       0
#define SystemPerformanceInformation 2
#define SystemTimeInformation        3
	
#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))
	
typedef struct
{
	DWORD   dwUnknown1;
	ULONG   uKeMaximumIncrement;
	ULONG   uPageSize;
	ULONG   uMmNumberOfPhysicalPages;
	ULONG   uMmLowestPhysicalPage;
	ULONG   uMmHighestPhysicalPage;
	ULONG   uAllocationGranularity;
	PVOID   pLowestUserAddress;
	PVOID   pMmHighestUserAddress;
	ULONG   uKeActiveProcessors;
	BYTE    bKeNumberProcessors;
	BYTE    bUnknown2;
	WORD    wUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
	LARGE_INTEGER   liIdleTime;
	DWORD           dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
	LARGE_INTEGER liKeBootTime;
	LARGE_INTEGER liKeSystemTime;
	LARGE_INTEGER liExpTimeZoneBias;
	ULONG         uCurrentTimeZoneId;
	DWORD         dwReserved;
} SYSTEM_TIME_INFORMATION;

//////////////////////////////////////////////////////////////////////////
// for Memory
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// for Network
//////////////////////////////////////////////////////////////////////////
#define		PDH_NETWORK_INTERFACE		510
#define		PDH_CURRENT_BANDWIDTH		520


MPF_SYSTEMINFO_NAMESPACE_END

#endif