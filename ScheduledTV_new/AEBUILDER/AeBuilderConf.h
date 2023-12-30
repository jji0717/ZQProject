

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
// Ident : $Id: AeBuilderConf.cpp
// Author: Kaliven Lee
// Desc  : Configuration for AeBuilder
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_new/AEBUILDER/AeBuilderConf.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 6     04-09-29 12:04 Kaliven.lee
// #pragma once
// 
// 5     04-09-29 9:40 Kaliven.lee
// compatible with filler
// 
// 4     04-09-27 13:45 Kaliven.lee
// add compatiable for filler
// 
// 3     04-09-23 15:29 Kaliven.lee
// modify out put structure
// 
// 2     04-09-23 12:22 Kaliven.lee
// get the inform once
// 
// 1     04-09-22 18:50 Kaliven.lee
// File create
#pragma once
#include "XMLPreference.h"
#include "ids.h"
#include "ids_interfaces.h"


#include <vector>
#include <map>
#ifndef MAX_SQL_TNAME
	#define MAX_SQL_TNAME 31
#endif

#define IDS_DEFAULT_WAIT_TIME_OUT	2000

#ifndef MAX_GUID_LENGTH
#	define MAX_GUID_LENGTH  32
#endif
/// define of Asset Element 
typedef struct tag_ASSETELEMNT{
	DWORD dwAEUID;					/// UID of Asset Element
	DWORD dwPlayTime;				/// second
	DWORD dwBitRate;				/// BitRate of Asset Element
}ASSETELEMENT,*PASSETELEMENT;
typedef std::vector<ASSETELEMENT>	ASSETELEMENTS,*PASSETELEMENTS;		//list of Asset element

/// define of Asset
/// when input only AssetUID need to be set
#ifndef ASSET
typedef struct tag_ASSET{
	DWORD	dwAssetUID;				/// UID of Asset 
	DWORD	dwPlayTime;				//	the sum of the playtime of all the Asset element.
	DWORD	dwWeight;				///	weight compatible with filler
	ASSETELEMENTS AssetElements ;	/// point of Element list of this Asset 
}ASSET,*PASSET;
#endif
//////////////////////////////////////////////////////////////////////////

typedef std::vector<ASSET>			ASSETS,*PASSETS;	//list of Asset

//////////////////////////////////////////////////////////////////////////

