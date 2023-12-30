// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: DSCallBack.h,v 1.9 2006/05/19 16:20:20 Ken Qian $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : Define the encapsulated class for IDS callback data
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/DBSync/DSCallBack.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 4     09-02-24 6:21 Ken.qian
// fix ACE-3033 - the updateAll() does not complete since the thread was
// exit caused by memory access violation
// 
// 3     07-04-12 17:35 Ken.qian
// Change DBSync auto-restart logic
// 
// 2     06-09-14 17:45 Ken.qian
// Fix bug 3755
// 
// 1     06-05-22 13:59 Ken.qian
// It is avaliable since DBSync3.6.0 to support taking callback while
// doing the full synchronization
// 
//
// Revision 1.0  2006/05/19 16:20:20  Ken Qian
// Initial codes
//
// ===========================================================================

#include "stdafx.h"
#include "DSCallBack.h"
#include "ids_interfaces.h"
#include "DSInterface.h"

using namespace ZQ::common;

extern CDSInterface g_ds;	//	Global object of class CDSInterface, implementing data synchronization
extern ZQ::common::Log * gpDbSyncLog;

extern DWORD g_dwFullSyncCount;

#define DISORDER_DSCALLBACK_KEPT_TIME   300000   // 5 minutes
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DSCallBackBase::DSCallBackBase(int nCallbackType, DWORD dwTriggerUID) 
: m_nCallBackType(nCallbackType), m_dwTriggerUID(dwTriggerUID)
{
}

DSCallBackBase::~DSCallBackBase()
{

}


DSCallBackMatadata::DSCallBackMatadata(DSCBTYPE callbackType, DWORD dwTriggerUID, DWORD dwUID, DWORD dwNumMd, METADATA* pMd)
: DSCallBackBase(callbackType, dwTriggerUID), m_dwUID(dwUID), m_dwNumMd(dwNumMd), m_pOutMd(pMd)
{
}

DSCallBackMatadata::~DSCallBackMatadata()
{
	if(m_pOutMd != NULL)
	{
		//	Free memory
		IdsFreeMd(m_pOutMd, m_dwNumMd);	
	}
	m_pOutMd = NULL;
}

DWORD DSCallBackMatadata::Process(void)
{
	switch(m_nCallBackType)
	{
	case SITEMD:
		g_ds.SiteMDCallBackProcess(m_dwTriggerUID, m_dwUID, m_dwNumMd, m_pOutMd);
		break;
	case APPMD:
		g_ds.ApplicationMDCallBackProcess(m_dwTriggerUID, m_dwUID, m_dwNumMd, m_pOutMd);
		break;
	case FOLDERMD:
		g_ds.FolderMDCallBackProcess(m_dwTriggerUID, m_dwUID, m_dwNumMd, m_pOutMd);
		break;
	case ASSETMD: 
		g_ds.AssetMDCallBackProcess(m_dwTriggerUID, m_dwUID, m_dwNumMd, m_pOutMd);
		break;
	case AELEMENTMD:
		g_ds.ElementMDCallBackProcess(m_dwTriggerUID, m_dwUID, m_dwNumMd, m_pOutMd);
		break;
	case CLIPMD:
		g_ds.ClipMDCallBackProcess(m_dwTriggerUID, m_dwUID, m_dwNumMd, m_pOutMd);
		break;
	default:
		break;
	}

	//m_pOutMd is released in the Process routine. Since I did not change original source too much
	m_pOutMd = NULL;
	return 0;
}


DSCallBackState::DSCallBackState(DSCBTYPE callbackType, DWORD dwTriggerUID, DWORD dwUID, WORD wEntryType, 
					WORD wState, WORD wOp, IDSUIDUPDATESTAMP stamp)
: DSCallBackBase(callbackType, dwTriggerUID), m_dwUID(dwUID), m_wEntryType(wEntryType), m_wState(wState), m_wOperation(wOp)
{
	memcpy(m_updateStamp, stamp, sizeof(IDSUIDUPDATESTAMP));
}

DSCallBackState::~DSCallBackState()
{
}


DWORD DSCallBackState::Process(void)
{
	switch(m_nCallBackType)
	{
	case ASSETSTATE:
		g_ds.AssetStateCallBackProcess(m_dwTriggerUID, m_wEntryType, m_dwUID, m_updateStamp, m_wState, m_wOperation);
		break;
	case AELEMENTSTATE:
		g_ds.ElementStateCallBackProcess(m_dwTriggerUID, m_wEntryType, m_dwUID, m_updateStamp, m_wState, m_wOperation);
		break;
	default:
		break;
	}
	return 0;
}


DSCallBackFolder::DSCallBackFolder(DSCBTYPE callbackType, DWORD dwTriggerUID, ENTRY* pFolderEntry, WORD wOp)
: DSCallBackBase(callbackType, dwTriggerUID), m_pFolderEntry(pFolderEntry), m_wOperation(wOp)
{
}

DSCallBackFolder::~DSCallBackFolder()
{
	if(m_pFolderEntry != NULL)
	{
		//	Free memory
		IdsFree(m_pFolderEntry);	
	}
	m_pFolderEntry = NULL;
}

DWORD DSCallBackFolder::Process(void)
{
	g_ds.FolderCallBackProcess(m_dwTriggerUID, m_pFolderEntry, m_wOperation);

	//m_pFolderEntry is released in the Process routine. Since I did not change original source too much
	m_pFolderEntry = NULL;

	return 0;
}


DSCallBackCAsset::DSCallBackCAsset(DSCBTYPE callbackType, DWORD dwTriggerUID, DWORD dwUID, DWORD dwNumMd, 
					COMPLEXASSET* pCAsset, IDSUIDUPDATESTAMP stamp)
: DSCallBackBase(callbackType, dwTriggerUID), m_dwUID(dwUID), m_dwNumMd(dwNumMd), m_pOutCAsset(pCAsset)
{
	memcpy(m_updateStamp, stamp, sizeof(IDSUIDUPDATESTAMP));

	m_genTime = GetTickCount();
}

DSCallBackCAsset::~DSCallBackCAsset()
{
	if(m_pOutCAsset != NULL)
	{
		//	Free memory
		IdsFree(m_pOutCAsset);	
	}
	m_pOutCAsset = NULL;
}


DWORD DSCallBackCAsset::Process(void)
{
	g_ds.CaCallBackProcess(m_dwTriggerUID, m_dwUID, m_updateStamp, m_dwNumMd, m_pOutCAsset);

	//m_pOutCAsset is released in the Process routine. Since I did not change original source too much
	m_pOutCAsset = NULL;
	return 0;
}

bool DSCallBackCAsset::expired()
{
	DWORD curTime = GetTickCount();

	if(curTime < m_genTime)
	{
		m_genTime = 0;
		return false;
	}

	if(curTime - m_genTime > DISORDER_DSCALLBACK_KEPT_TIME)  
	{
		return true;
	}
	return false;
}

DSCallBackMDDescriptor::DSCallBackMDDescriptor(DSCBTYPE callbackType, DWORD dwTriggerUID, WORD wEntryType, WORD wOp, METADATADESC* pMdd)
: DSCallBackBase(callbackType, dwTriggerUID), m_wEntryType(wEntryType), m_wOperation(wOp), m_pMddescriptor(pMdd)
{
}
	
DSCallBackMDDescriptor::~DSCallBackMDDescriptor()
{
	if(m_pMddescriptor != NULL)
	{
		IdsFreeMdDesc(m_pMddescriptor, 1);
	}
	m_pMddescriptor = NULL;
}

DWORD DSCallBackMDDescriptor::Process(void)
{
	g_ds.MDDCallBackProcess(m_dwTriggerUID, m_wEntryType, m_pMddescriptor, m_wOperation);

	//m_pMddescriptor is released in the Process routine. Since I did not change original source too much
	m_pMddescriptor = NULL;

	return 0;
}


FullSyncCallback::FullSyncCallback(DWORD dwTriggerUID)
: DSCallBackBase(FULLSYNC, dwTriggerUID)
{
}

FullSyncCallback::~FullSyncCallback()
{
}

DWORD FullSyncCallback::Process(void)
{
	DWORD dwStart = GetTickCount();

	// doing full syncing
	DWORD ret = g_ds.UpdateAll();

	DWORD timespan = (GetTickCount() - dwStart) / 1000;

	if(ret != 0)
	{
		(*gpDbSyncLog)(Log::L_ERROR, "UpdateAll() for No.%d full syncing failed, spent %d seconds", g_dwFullSyncCount, timespan);
	}
	else
	{
		(*gpDbSyncLog)(Log::L_NOTICE, "UpdateAll() for No.%d full syncing succeed, spent %d seconds", g_dwFullSyncCount, timespan);
	}
	return 0;
}

