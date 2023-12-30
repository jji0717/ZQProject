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
// Name  : DBSAdi_def.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-11-8
// Desc  : This is the basic definition for the DBSync Add-in
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/DBSADI/doc/DBSAdi_def.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 6     07-02-25 16:38 Ken.qian
// ===========================================================================

#ifndef		_DBSADI_DEF_H_
#define		_DBSADI_DEF_H_

#include <stdlib.h>

#define DBSA_MAXMDNAME				256
#define DBSA_MAXMDVALUE				512
#define DBSA_MAXMDDUSER				24
#define DBSA_MAXLUID				24

// entry type defines
#define DBSA_ENTRY_ASSET			1
#define DBSA_ENTRY_ELEMENT			2
#define DBSA_ENTRY_FOLDER			3
#define DBSA_ENTRY_APPLICATION		4
#define DBSA_ENTRY_SITE				5
#define DBSA_ENTRY_WQ_ITEM			6
#define DBSA_ENTRY_CLIP				7
#define DBSA_ENTRY_NUM_OBJECTS		8
#define DBSA_ENTRY_PACKAGE			11

// operation defines
#define DBSA_ADD					1
#define DBSA_REPLACE				2
#define DBSA_DELETE					3
#define DBSA_PUT					4	// Replace metadata value if one exists; insert if one doesn't

// entry state defines
#define DBSA_STATE_ACTIVE			1
#define DBSA_STATE_INACTIVE			2

/// basic information for DBSync service
typedef struct _DA_dbsyncInfo 
{
	/// ip address of the server where DBSync service runs
	wchar_t		_szIPAddr[16];

	/// version of DBSync service
	wchar_t		_szVersion[16];

	/// the root of DBSync synchronizing directory in ITV AM 
	/// "\" or "\SeaChange Movies On Demand" means DBSync synchronizing all directories
	wchar_t		_szSyncDir[MAX_PATH];

	/// the instance id of DBSync
	DWORD		_dwInstanceID;

	/// whether DBSync service support Navigation Service (NSSync)
	/// 1 if yes, otherwise 0
	/// this option affect whether DBSync writes WorkQueue in NAV DB
	DWORD		_dwSupportNav;

	/// the time interval within which DBSync ignores when time window meta-data changes, in seconds
	/// Currently, if "ActivateTime", "DeactivateTime" or "DeleteTime" change is less than this value,
	/// DBSync will not write WorkQueue in NAV DB for NSSync, but still synchronize to LAM
	DWORD		_dwTwThreshold;

} DA_dbsyncInfo;


/// basic information for ITV DS
typedef struct _DA_itvInfo
{
	/// ip address of ITV DS server
	wchar_t		_szIPAddr[16];

	/// version of ITV DS API
	wchar_t		_szVersion[16];
} DA_itvInfo;


/// data block for entry meta-data change
typedef struct _DA_metaDb
{
	/// name of metadata
	wchar_t		_szMdName[DBSA_MAXMDNAME];

	/// value of metadata
	wchar_t		_szMdValue[DBSA_MAXMDVALUE];

	/// metadata UID in LAM
	DWORD		_dwMdUID;
	
	/// application UID; zero if this metadata is system/application-independent
	DWORD		_dwAppUID;

	/// operation types
	DWORD		_dwOp;
} DA_metaDb;


/// data block for meta-data description
typedef struct _DA_mddListDb
{
	/// name of metadata
	wchar_t		_szMdName[DBSA_MAXMDNAME];

	/// metadata UID in LAM
	DWORD		_dwMdUID;

	/// application UID; zero if this metadata is system/application-independent
	DWORD		_dwAppUID;

} DA_mddListDb;


/// data block for entry basic information
typedef struct _DA_entryDb
{
	/// entry type, refer to DBSA_ENTRY_XXX macros
	DWORD		_dwEntryType;

	/// entry UID, in ITV system
	DWORD		_dwEntryUID;

	/// LocalEntryUID in LAM
	wchar_t		_szLocalEntryUID[DBSA_MAXLUID];

	/// ProviderID
	wchar_t     _szProviderID[DBSA_MAXMDVALUE];

	/// ProviderAssetID
	wchar_t     _szProviderAssetID[DBSA_MAXMDVALUE];
	
} DA_entryDb;


/// data block for hierarchy entry
typedef struct _DA_hierarchyDb
{
	/// entry type, refer to DBSA_ENTRY_XXX macros
	DWORD		_dwEntryType;

	/// entry hierarchy UID, in ITV system
	DWORD		_dwHierarchyUID;

	/// localHierarchyUID in LAM
	wchar_t		_szLocalHUID[DBSA_MAXLUID];
	
} DA_hierarchyDb;


/// data block for state change information
typedef struct _DA_stateDb
{
	/// entry state, refer to DBSA_STATE_XXX macros
	DWORD		_dwEntryState;
	
} DA_stateDb;


//////////////////////////////////////////////////////////////////////////
// Prototype for DBSA API
//////////////////////////////////////////////////////////////////////////

#ifdef NATIVE_DLL_IMPL
	#define DBSACALLBACK	__declspec(dllexport) void
#else
	#define	DBSACALLBACK	void
#endif


#if defined(__cplusplus)
extern "C" {                                                // C++ to C linkage
#endif

// ------------------------------------------------
/// This API will get called when Add-in is loaded.  
/// This usually happens when DBSync service starts.
///@param[in]	dbsInfo		information about DBSync service
///@param[in]	itvInfo		information about ITV DS service
DBSACALLBACK 
DBSA_Initialize(	
		DA_dbsyncInfo*		pDbsInfo,
		DA_itvInfo*			pItvInfo
		);

#define	DBSACBNAME_INIT			"DBSA_Initialize"
typedef DBSACALLBACK			(*DBSAProto_Init)(DA_dbsyncInfo*, DA_itvInfo*);


// ------------------------------------------------
/// This API will get called when Add-in is released.  
/// This usually happens when DBSync service stops.
DBSACALLBACK 
DBSA_Uninitialize(
		);

#define DBSACBNAME_UNINIT		"DBSA_Uninitialize"
typedef DBSACALLBACK			(*DBSAProto_Uninit)();


// ------------------------------------------------
/// This API will get called when full synchronization begins.  
/// The full synchronization is to query all data from DS database, usually when DBSync starting or DS connection is lost.
DBSACALLBACK 
DBSA_SyncBegin(
		);

#define DBSACBNAME_SYNCBEGIN	"DBSA_SyncBegin"
typedef DBSACALLBACK			(*DBSAProto_SyncBein)();


// ------------------------------------------------
/// This API will get called when full synchronization is finished.
DBSACALLBACK 
DBSA_SyncEnd(
		);

#define DBSACBNAME_SYNCEND		"DBSA_SyncEnd"
typedef DBSACALLBACK			(*DBSAProto_SyncEnd)();


// ------------------------------------------------
/// This API will get called when metadata is added, deleted or updated.  
/// For example, this API will be called when operator changed 'Genre' metadata of an asset on ITV AM.
DBSACALLBACK 
DBSA_TriggerMd(
		DA_entryDb*			pEntryBlock,
		DWORD				dwMdNumber,
		DA_metaDb*			pFirstMdBlock
		);

#define DBSACBNAME_TRGGMD		"DBSA_TriggerMd"
typedef DBSACALLBACK			(*DBSAProto_TrggMd)(DA_entryDb*, DWORD, DA_metaDb*);


// ------------------------------------------------
/// This API will gGet called when metadata descriptor changes.  
/// For example, this API will be called when operator added a new descriptor of id '3' and value '$1.99' to the metadata 'price'.
DBSACALLBACK 
DBSA_TriggerMdd(
		DA_mddListDb*		pMddList
		);

#define	DBSACBNAME_TRGGMDD		"DBSA_TriggerMdd"
typedef DBSACALLBACK			(*DBSAProto_TrggMdd)(DA_mddListDb*);


// ------------------------------------------------
/// This API will get called when DS hierarchy structure changed.  
/// For example, this API will be called when operator link a new asset under a folder.
DBSACALLBACK 
DBSA_TriggerHierarchy(
		DWORD				dwOperation,
		DA_hierarchyDb*		pHierarchyBlock
		);

#define DBSACBNAME_TRGGHRCHY	"DBSA_TriggerHierarchy"
typedef DBSACALLBACK			(*DBSAProto_TrggHrchy)(DWORD, DA_hierarchyDb*);


// ------------------------------------------------
/// This API will get called when Complex Asset structure changed.  
/// For example, this API will be called when operator add a new asset element under an asset.
DBSACALLBACK 
DBSA_TriggerCa(
		DA_entryDb*			pAsset
		);

#define DBSACBNAME_TRGGCA		"DBSA_TriggerCa"
typedef DBSACALLBACK			(*DBSAProto_TrggCa)(DA_entryDb*);


// ------------------------------------------------
/// This API will get called when object state changed.  
/// For example, this API will be called when an inactive asset became 'active' state.
///@param[in]	entryBlock	data block containing entry basic information
///@param[in]	stateBlock	data block containing state change information
DBSACALLBACK 
DBSA_TriggerState(
		DA_entryDb*			pEntryBlock,
		DA_stateDb*			pStateBlock
		);

#define DBSACBNAME_TRGGSTAT		"DBSA_TriggerState"
typedef DBSACALLBACK			(*DBSAProto_TrggStat)(DA_entryDb*, DA_stateDb*);


#if defined(__cplusplus)
} // End extern "C" bracket
#endif

#endif	// #ifndef		_DBSADI_DEF_H_