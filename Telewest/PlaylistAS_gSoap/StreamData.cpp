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
// Ident : $Id: StreamData.cpp,v 1.1 2004/10/07 11:00:00 Ken Qian $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : impl the data and data-management of the session between AS and SS
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/PlaylistAS_gSoap/StreamData.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 3     10-03-05 15:19 Build
// Code change to be compatible with VS2008
// 
// 2     05-07-29 16:16 Bernie.zhao
// 
// 1     05-06-28 11:38 Bernie.zhao
// 
// 2     05-03-24 16:50 Bernie.zhao
// added soap call when acceptrdyasset failed
// 
// 1     05-02-16 11:56 Bernie.zhao
// ===========================================================================
#include "Log.h"
using namespace ZQ::common;


#include <tchar.h>
#include "StreamData.h"
#include "stv_ssctrl.h"

///////////////////////////////////////////////////////////////////////////
////////			CStreamData implementation					///////////
///////////////////////////////////////////////////////////////////////////

CStreamData::CStreamData(DWORD dwHomeID, BoxMacAddr deviceID, DWORD dwPurchaseID, STREAMID sid, DWORD dwAssetID, DWORD dwPreAEUID/* =0 */, DWORD dwNewAEUID/* =0 */)
{
	m_dwHomeID = dwHomeID;
	m_DeviceID = deviceID;
	m_dwPurchaseID = dwPurchaseID;
	m_sid = sid;
	
	m_dwAssetID = dwAssetID;

	m_dwPreAEUID = dwPreAEUID;
	m_dwNewAEUID = dwNewAEUID;

	m_bHasSch = FALSE;
	m_dwXitionSeqNum = -1;

	m_bIsFirstPlay = true;
}

CStreamData::CStreamData(const CStreamData& rhs)
{
	m_dwHomeID = rhs.m_dwHomeID;
	m_DeviceID = rhs.m_DeviceID;
	m_dwPurchaseID = rhs.m_dwPurchaseID;
	m_sid = rhs.m_sid;
	
	m_dwAssetID = rhs.m_dwAssetID;
	m_dwPreAEUID = rhs.m_dwPreAEUID;
	m_dwNewAEUID = rhs.m_dwNewAEUID;

	m_bHasSch = FALSE;
	m_dwXitionSeqNum = -1;

	m_bIsFirstPlay = rhs.m_bIsFirstPlay;
}

CStreamData::~CStreamData()
{

}

const CStreamData& CStreamData::operator=(const CStreamData& rhs)
{
	if(this == &rhs)
		return *this;

	m_dwHomeID = rhs.m_dwHomeID;
	m_DeviceID = rhs.m_DeviceID;
	m_dwPurchaseID = rhs.m_dwPurchaseID;
	m_sid = rhs.m_sid;

	m_dwAssetID = rhs.m_dwAssetID;
	
	m_dwPreAEUID = rhs.m_dwPreAEUID;
	m_dwNewAEUID = rhs.m_dwNewAEUID;

	m_bIsFirstPlay = rhs.m_bIsFirstPlay;
	
	return *this;
}

// save Asset Element ID
void CStreamData::AddAEID(DWORD dwAEID)
{
	m_arrAEID.push_back(dwAEID);
}

// Does the specified Asset Element existed in the array.
BOOL CStreamData::ExistedAEID(DWORD dwAEID)
{
	if(!m_bHasSch)
	{
		for(int i=0; i<m_arrAEID.size(); i++)
		{
			if(m_arrAEID[i] == dwAEID)
			{
				m_bHasSch = TRUE;
				return TRUE;
			}
		}
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

void CStreamData::ClearAEIDs(void)
{
	m_arrAEID.clear();
}

///////////////////////////////////////////////////////////////////////////
////////		CStreamDataManager implementation				///////////
///////////////////////////////////////////////////////////////////////////

CStreamDataManager::CStreamDataManager()
{
	m_lstStreamDatas.clear();
}

CStreamDataManager::~CStreamDataManager()
{
	m_lstStreamDatas.clear();
}

BOOL CStreamDataManager::Add(CStreamData& streamData)
{
	PSTREAMID pTmpSid = streamData.GetStreamID();

	MutexGuard	tmpGd(m_lstLock);
	list<CStreamData>::iterator desItr = QueryBySID(*pTmpSid);		
	if(desItr != m_lstStreamDatas.end())
	{	
		((list<CStreamData>::reference)(*desItr)) = streamData;
		glog(Log::L_DEBUG, _T("CStreamDataManager::Add(REPALCE): with SID=%x, CurStreamDataCount=%d"), 
			streamData.GetStreamID()->dwStreamIdNumber, m_lstStreamDatas.size());
	}
	else
	{
		m_lstStreamDatas.push_back(streamData);	
		glog(Log::L_DEBUG, _T("CStreamDataManager::Add(NEW): with SID=%x, CurStreamDataCount=%d"), 
			streamData.GetStreamID()->dwStreamIdNumber, m_lstStreamDatas.size());
	}
	
	return TRUE;
}

BOOL CStreamDataManager::Remove(const STREAMID& sid)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);
	bool bRet = FALSE;

	MutexGuard	tmpGd(m_lstLock);
	if(desItr != m_lstStreamDatas.end())
	{
		glog(Log::L_DEBUG, _T("CStreamDataManager::Remove: with SID=%x, CurStreamDataCount=%d"), 
			desItr->GetStreamID()->dwStreamIdNumber, m_lstStreamDatas.size()-1);
		m_lstStreamDatas.erase(desItr);

		bRet = TRUE;
	}
		
	return bRet;
}

/* Return Value.
	 NOTITYPE_NONE : Error - no notification
	 NOTITYPE_PLAY : only play notification
	 NOTITYPE_PS   : stop and play notification
	 NOTITYPE_STOP : only stop notification
*/
int  CStreamDataManager::GenSSAENotification(const STREAMID& sid, DWORD dwPreAEUID, DWORD dwNewAEUID, 
					   DWORD dwXitionSeqMum, SSAENotification& stopNoti, SSAENotification& playNoti)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);

	if(desItr != m_lstStreamDatas.end())
	{
/*		
		// Current AE Transition Event's previous AEUID 
		// not equal to Last AE Transition Event's new AEUID.
		// there must be some AE Transition Event lost.(No CallBack);

		// if you dont want to process this exception, Just leave it alone.
		
		DWORD dwOldNewAEUID = desItr->GetNewAEUID();
		if( dwOldNewAEUID!= 0 && dwOldNewAEUID != dwPreAEUID)
		{
		}

*/	
		////////   Set SSNotication   //////////
		int nRet;
		BOOL bPreExisted = desItr->ExistedAEID(dwPreAEUID);
		if(!bPreExisted)
		{	
			// the first asset element, so just playing notification
			playNoti.dwPurchaseID = desItr->GetPurchaseID();
			playNoti.dwAeUID = dwNewAEUID;
			playNoti.wOperation = SAENO_PLAY;
			playNoti.dwStatus = SAENS_SUCCEED;

			// set the new AEUID
			desItr->SetPreAEUID(dwPreAEUID);
			desItr->SetNewAEUID(dwNewAEUID);

			nRet = NOTITYPE_PLAY;
		}
		else
		{
			if(desItr->GetXitionSeqNum() == dwXitionSeqMum)
			{	
				// the last asset element, so just stopping notification
				stopNoti.dwPurchaseID = desItr->GetPurchaseID();
				stopNoti.dwAeUID = dwPreAEUID;
				stopNoti.wOperation = SAENO_STOP;
				stopNoti.dwStatus = SAENS_SUCCEED;

				// set the new AEUID
				desItr->SetPreAEUID(dwPreAEUID);
				desItr->SetNewAEUID(dwNewAEUID);

				nRet = NOTITYPE_STOP;
			}
			else
			{
				// the middle asset element, stopping notification of the previous element
				// and playing notification of the next element.
				playNoti.dwPurchaseID = desItr->GetPurchaseID();
				playNoti.dwAeUID = dwNewAEUID;
				playNoti.wOperation = SAENO_PLAY;
				playNoti.dwStatus = SAENS_SUCCEED;

				stopNoti.dwPurchaseID = desItr->GetPurchaseID();
				stopNoti.dwAeUID = dwPreAEUID;
				stopNoti.wOperation = SAENO_STOP;
				stopNoti.dwStatus = SAENS_SUCCEED;

				// set the new AEUID
				desItr->SetPreAEUID(dwPreAEUID);
				desItr->SetNewAEUID(dwNewAEUID);

				nRet = NOTITYPE_PS;
			}
		}

		// set the current AETransition Seqence Number.
		desItr->SetXitionSeqNum(dwXitionSeqMum);

		return nRet;
	}
	return NOTITYPE_NONE;
}


BOOL CStreamDataManager::GenSSStreamNotification(const STREAMID& sid, DWORD errCode, 
									DWORD errOp, SSStreamNotification& errNoti)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);

	if(desItr != m_lstStreamDatas.end())
	{
		errNoti.dwPurchaseID = desItr->GetPurchaseID();
		errNoti.dwErrorCode = errCode;
		errNoti.dwOperation = errOp;

		return TRUE;
	}
	return FALSE;
}

DWORD CStreamDataManager::GetHomeID(const STREAMID& sid)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);
	if(desItr != m_lstStreamDatas.end())
	{
		return desItr->GetHomeID();
	}
	return 0;
}

BoxMacAddr CStreamDataManager::GetDeviceID(const STREAMID& sid)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);
	if(desItr != m_lstStreamDatas.end())
	{
		return desItr->GetDeviceID();
	}
	BoxMacAddr noneAddr;
	memset(&noneAddr, 0, sizeof(BoxMacAddr));
	return noneAddr;
}

DWORD CStreamDataManager::GetPurchaseID(const STREAMID& sid)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);
	if(desItr != m_lstStreamDatas.end())
	{
		return desItr->GetPurchaseID();
	}
	return 0;
}

bool CStreamDataManager::GetIsFirstPlay(const STREAMID& sid)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);
	if(desItr != m_lstStreamDatas.end())
	{
		return desItr->GetIsFirstPlay();
	}
	return false;
}

void CStreamDataManager::SetIsFirstPlay(const STREAMID& sid, BOOL value)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);
	if(desItr != m_lstStreamDatas.end())
	{
		desItr->SetIsFirstPlay(value);
	}
}

/// for save asset element id 
BOOL CStreamDataManager::AddAEID(const STREAMID& sid, DWORD dwAEID)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);
	if(desItr != m_lstStreamDatas.end())
	{
		desItr->AddAEID(dwAEID);
		return TRUE;
	}
	return FALSE;
}

void CStreamDataManager::ClearAEIDs(const STREAMID& sid)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);
	if(desItr != m_lstStreamDatas.end())
	{
		desItr->ClearAEIDs();
	}
}


list<CStreamData>::iterator CStreamDataManager::QueryBySID(const STREAMID& sid)
{
	list<CStreamData>::iterator itr;
	bool bMatch = FALSE;
	
	MutexGuard	tmpGd(m_lstLock);

	itr = m_lstStreamDatas.begin();
	while(itr != m_lstStreamDatas.end())
	{
		if(itr->GetStreamID()->dwStreamIdNumber == sid.dwStreamIdNumber) {
			bMatch = TRUE;
			break;
		}
		*itr++;
	}

	if(bMatch)
		return itr;
	else
		return m_lstStreamDatas.end();
}

