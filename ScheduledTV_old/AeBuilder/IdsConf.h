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
// Ident : $Id: AeBuidler.cpp
// Author: Kaliven Lee
// Desc  : IDS session manager configuration
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_old/AeBuilder/IdsConf.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 3     04-10-12 13:27 Kaliven.lee
// add CueIn CueOut
// 
// 2     04-10-08 10:46 Kaliven.lee
// 
#pragma once

#include "ids.h"
#include "ids_interfaces.h"

#include <stdlib.h>
#include <vector>
#include <map>

#ifndef MAX_SQL_TNAME
	#define MAX_SQL_TNAME 31
#endif

#define IDS_DEFAULT_WAIT_TIME_OUT	2000

#ifndef MAX_GUID_LENGTH
#	define MAX_GUID_LENGTH  32
#endif

#ifndef LIBEXTERN
#	ifdef _DEBUG 
#		define	LIBEXTERN "_d.lib"
#	else
#		define	LIBEXTERN ".lib"
#	endif
#endif

/// define of Asset Element 

typedef struct tag_ASSETELEMNT{
	DWORD dwAEUID;					/// UID of Asset Element
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
	DWORD	dwPlayTime;				//	the sum of the playtime of all the Asset element.
	DWORD	dwWeight;				///	weight compatible with filler
	ASSETELEMENTS AssetElements ;	/// point of Element list of this Asset 
}ASSET,*PASSET;

//////////////////////////////////////////////////////////////////////////

typedef std::vector<ASSET>			ASSETS,*PASSETS;	//list of Asset


#ifndef APPNAMES
	typedef std::vector<APPNAME>	APPNAMES;
#endif
//////////////////////////////////////////////////////////////////////////