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
// Name  : DBSAdiMan.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-11-21
// Desc  : DBSync Add-in Manager Definition
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/DBSync/DBSAdiMan.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// ===========================================================================

#ifndef _DBSADIMAN_H_
#define	_DBSADIMAN_H_

#pragma warning (disable: 4786)
#include "DBSAdi_def.h"
#include "seachange.h"
#include <vector>
#include <string>

#define EVID_INIT			1
#define EVID_UNINIT			2
#define EVID_SYNCBEGIN		3
#define EVID_SYNCEND		4
#define EVID_TRGGMD			5
#define EVID_TRGGMDD		6
#define EVID_TRGGHRCHY		7
#define EVID_TRGGCA			8
#define EVID_TRGGSTAT		9

#define MAX_EVID_NUM		10			

class DBSAdiMan
{
public:
	DBSAdiMan();
	virtual ~DBSAdiMan();
	
	void setPluginPath(wchar_t* pluginPath);

	bool isValid() { return _enabled; }

public:
	void Init(DA_dbsyncInfo* pDbsInfo, DA_itvInfo* pItvInfo);
	void Uninit();
	void SyncBegin();
	void SyncEnd();
	void TrggMd(DA_entryDb*	pEntryBlock, DWORD dwMdNumber, DA_metaDb* pFirstMdBlock);
	void TrggMdd(DA_mddListDb* pMddList);
	void TrggHrchy(DWORD dwOperation, DA_hierarchyDb* pHierarchyBlock);
	void TrggCa(DA_entryDb* pAsset);
	void TrggStat(DA_entryDb* pEntryBlock, DA_stateDb* pStateBlock);

private:
	wchar_t									_szPluginPath[MAX_PATH];
	HINSTANCE								_hPluginLib; 
	bool									_enabled;
	bool									_procValidList[MAX_EVID_NUM];
	static	std::vector<std::string>		_procNameVector;
};

#endif