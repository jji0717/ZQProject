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
// Ident : $Id: DSCallBack.h,v 1.9 2006/05/19 07:24:54 Ken Qian $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : Define the encapsulated class for IDS callback data
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/DBSync/DSCallBack.h $
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
// 2     07-03-07 15:01 Ken.qian
// 
//
// Revision 1.0  2006/05/19 16:20:20  Ken Qian
// Initial codes
//
// ===========================================================================

//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DSCALLBACK_H__BF07D458_EA94_40A6_9097_76519E6AE0DB__INCLUDED_)
#define AFX_DSCALLBACK_H__BF07D458_EA94_40A6_9097_76519E6AE0DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ids_def.h"
// base class for IDS CallBack data
class DSCallBackBase  
{
public:
	DSCallBackBase(int nCallbackType, DWORD dwTriggerUID);
	virtual ~DSCallBackBase();

public:
	enum DSCBTYPE{SITEMD, APPMD, FOLDERMD, ASSETMD, AELEMENTMD, CLIPMD, 
				  ASSETSTATE, AELEMENTSTATE, FOLDER, CASSET, MDDESCRIPTOR, 
				  MANUALLYSYNC, FULLSYNC };

	virtual DWORD Process(void) = 0;
protected:
	int			m_nCallBackType;
	DWORD		m_dwTriggerUID;
};

// class for IDS Metadata Change callback
class DSCallBackMatadata : public DSCallBackBase
{
public:
	DSCallBackMatadata(DSCBTYPE callbackType, DWORD dwTriggerUID, DWORD dwUID, DWORD dwNumMd, METADATA* pMd);
	virtual ~DSCallBackMatadata();

public:
	DWORD Process(void);

private:
	DWORD		m_dwUID;
	DWORD		m_dwNumMd;
	METADATA*	m_pOutMd;
};

// class for IDS State Change (Asset & Element) callback
class DSCallBackState : public DSCallBackBase
{
public:
	DSCallBackState(DSCBTYPE callbackType, DWORD dwTriggerUID, DWORD dwUID, WORD wEntryType, 
					WORD wState, WORD wOp, IDSUIDUPDATESTAMP stamp);
	virtual ~DSCallBackState();

public:
	DWORD Process(void);

private:
	DWORD		m_dwUID;
	WORD		m_wEntryType;		
	WORD		m_wState;
	WORD		m_wOperation;
	IDSUIDUPDATESTAMP m_updateStamp;
};

// class for Folder Change callback
class DSCallBackFolder : public DSCallBackBase
{
public:
	DSCallBackFolder(DSCBTYPE callbackType, DWORD dwTriggerUID, ENTRY* pFolderEntry, WORD wOp);
	virtual ~DSCallBackFolder();

public:
	DWORD Process(void);

private:
	ENTRY*		m_pFolderEntry;
	WORD		m_wOperation;
};


// class for IDS Complex Asset Change callback
class DSCallBackCAsset : public DSCallBackBase
{
public:
	DSCallBackCAsset(DSCBTYPE callbackType, DWORD dwTriggerUID, DWORD dwUID, DWORD dwNumMd, 
					COMPLEXASSET* pCAsset, IDSUIDUPDATESTAMP stamp);
	virtual ~DSCallBackCAsset();

public:
	DWORD Process(void);

	bool expired();
private:
	DWORD				m_dwUID;
	DWORD				m_dwNumMd;
	COMPLEXASSET*		m_pOutCAsset;
	IDSUIDUPDATESTAMP	m_updateStamp;

	DWORD               m_genTime;
};

// class for IDS Metadata Descriptor Change callback
class DSCallBackMDDescriptor : public DSCallBackBase
{
public:
	DSCallBackMDDescriptor(DSCBTYPE callbackType, DWORD dwTriggerUID, WORD wEntryType, WORD wOp, METADATADESC* pMdd);
	virtual ~DSCallBackMDDescriptor();

public:
	DWORD Process(void);

private:
	WORD				m_wEntryType;
	WORD				m_wOperation;
	METADATADESC*		m_pMddescriptor;
};

class FullSyncCallback  :  public DSCallBackBase
{
public:
	FullSyncCallback(DWORD dwTriggerUID);
	virtual ~FullSyncCallback();

public:
	DWORD Process(void);
	
};

#endif // !defined(AFX_DSCALLBACK_H__BF07D458_EA94_40A6_9097_76519E6AE0DB__INCLUDED_)
