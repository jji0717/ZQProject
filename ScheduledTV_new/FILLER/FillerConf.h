
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
// Ident : $Id: FillerConf.h
// Author: Kaliven Lee
// Desc  : Configuration for the Filler class. 
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_new/FILLER/FillerConf.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 7     04-10-18 14:17 Kaliven.lee
// and serial mode in barker system
// 
// 6     04-10-12 16:52 Bernie.zhao
// 
// 5     04-09-29 12:04 Kaliven.lee
// #pragma once
// 
// 4     04-09-29 11:49 Kaliven.lee
// output point list
// 
// 3     04-09-29 9:05 Kaliven.lee
// compatible with aebuilder
// 
// 2     04-09-23 17:13 Kaliven.lee
// delete unused define
// 
// 1     04-09-23 17:10 Kaliven.lee
// creat File
// 

// FillConf.h: Configuration for the Filler class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include <windows.h>
#pragma warning(disable: 4786)
#include <vector>
#include <map>

/// define of Asset Element 

typedef struct tag_ASSETELEMNT{
	DWORD dwAEUID;					/// UID of Asset Element
	DWORD dwNO;						/// reserved
	DWORD dwPlayTime;				/// second
	DWORD dwCueIn;
	DWORD dwCueOut;
	DWORD dwBitRate;				/// BitRate of Asset Element
}ASSETELEMENT,*PASSETELEMENT;

typedef std::vector<ASSETELEMENT>	ASSETELEMENTS,*PASSETELEMENTS;		//list of Asset element


/// define of Asset
/// when input only AssetUID need to be set

typedef struct tag_ASSET{
	DWORD	dwAssetUID;				/// UID of Asset 
	DWORD	dwNO;					/// reserved
	DWORD	dwPlayTime;				///	the sum of the playtime of all the Asset element.
	DWORD	dwWeight;				///	weight compatible with filler
	ASSETELEMENTS AssetElements ;	/// point of Element list of this Asset 
}ASSET,*PASSET;

//////////////////////////////////////////////////////////////////////////

typedef std::vector<ASSET>			ASSETS,*PASSETS;	//list of Asset

//////////////////////////////////////////////////////////////////////////

/// Enum type FILLTYPE 
typedef enum{
	FILLTYPE_RANDOM = 0 ,
	FILLTYPE_SERIAL = 100,
//	FILLTYPE_BARKER = 200
} FILLTYPE;

///////////////////////////////////////////////////

#ifndef DWORD 
	typedef unsigned long DWORD;
#endif

typedef std::vector<ASSET*> PASSETLIST;

////////////////////////////////////////////////////

