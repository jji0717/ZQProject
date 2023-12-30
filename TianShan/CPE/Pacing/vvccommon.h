// ===========================================================================
// Copyright (c) 2010 by
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
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================

#ifndef ZQTS_CPEPLG_PACINGVVCCOMMON_H
#define ZQTS_CPEPLG_PACINGVVCCOMMON_H


#include "ZQ_common_conf.h"
#include "CTFBaseTypes.h"
#include "vvc.h"

#pragma pack(push, ctf_types, 1)
typedef struct _CTF_TLV
{
	VVC_TAG tag;
	UINT16 len;
} CTF_TLV;

// extract a word from an array of unsigned chars
#define WRD(p)			( ((uint16_t)(*((p)+1))<<8) | (*(p)) )

// extract a long from an array of unsigned chars
#define DWD(p)			( ((((uint32_t)WRD((p)+2)))<<16) | (WRD((p))) )

// extract a long long from an array of unsigned char
#define BY5(p)			( ((((uint64_t)*((p)+4)))<<32) | (DWD((p))) )

// locally defined structures

typedef struct _CTF_TIME
{
	int year;
	int month;
	int dayOfWeek;
	int day;
	int hour;
	int minute;
	int second;
	int milliseconds;
} CTF_TIME;

#endif

