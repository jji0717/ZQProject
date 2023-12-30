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
// $Log: /ZQProjs/ScheduledTV_old/IssStreamCtrl/StreamData.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 17    05-07-12 18:49 Bernie.zhao
// 
// 16    05-06-27 19:51 Ken.qian
// 
// 15    05-01-19 22:02 Ken.qian
// 
// 14    05-01-19 16:26 Ken.qian
// 
// 13    04-12-31 11:47 Bernie.zhao
// added file lock
// 
// 12    04-12-16 14:56 Bernie.zhao
// 
// 11    04-12-03 10:07 Ken.qian
// 
// 10    04-12-02 20:32 Ken.qian
// 
// 9     04-12-01 17:38 Ken.qian
// 
// 8     04-12-01 11:02 Ken.qian
// 
// 7     04-11-24 14:21 Ken.qian
// 
// 6     04-10-21 17:51 Ken.qian
// 
// 5     04-10-20 10:31 Ken.qian
// 
// 4     04-10-15 11:38 Ken.qian
// 
// 3     04-10-14 18:43 Ken.qian
// 
// 2     04-10-14 18:08 Ken.qian
// 
// 1     04-10-11 17:04 Ken.qian
// 
// Revision 1.1  2004/10/07 11:00:00 Ken Qian
//   definition and implemention
//
// ===========================================================================
#include "Log.h"
using namespace ZQ::common;


#include <tchar.h>
#include "StreamData.h"
#include "stv_ssctrl.h"

///////////////////////////////////////////////////////////////////////////
////////			CStreamData implementation					///////////
///////////////////////////////////////////////////////////////////////////

CStreamData::CStreamData(DWORD dwPurchaseID, STREAMID sid, DWORD dwAssetID,
						DWORD dwPreAEUID/* =0 */, DWORD dwNewAEUID/* =0 */)
{
	m_dwPurchaseID = dwPurchaseID;
	memcpy(&m_sid, &sid, sizeof(m_sid));
	//m_sid = sid;
	m_dwAssetID = dwAssetID;

	m_dwPreAEUID = dwPreAEUID;
	m_dwNewAEUID = dwNewAEUID;

	m_bHasSch = false;
	m_dwXitionSeqNum = -1;
}

CStreamData::CStreamData(const CStreamData& rhs)
{
	m_dwPurchaseID = rhs.m_dwPurchaseID;
	memcpy(&m_sid, &rhs.m_sid, sizeof(m_sid));
	//m_sid = rhs.m_sid;
	m_dwAssetID = rhs.m_dwAssetID;
	m_dwPreAEUID = rhs.m_dwPreAEUID;
	m_dwNewAEUID = rhs.m_dwNewAEUID;

	m_bHasSch = false;
	m_dwXitionSeqNum = -1;
}

CStreamData::~CStreamData()
{

}

const CStreamData& CStreamData::operator=(const CStreamData& rhs)
{
	if(this == &rhs)
		return *this;

	m_dwPurchaseID = rhs.m_dwPurchaseID;
	//memcpy(&m_sid, &rhs.m_sid, sizeof(m_sid));
	m_sid = rhs.m_sid;
	m_dwAssetID = rhs.m_dwAssetID;
	m_dwPreAEUID = rhs.m_dwPreAEUID;
	m_dwNewAEUID = rhs.m_dwNewAEUID;

	m_bHasSch = false;
	m_dwXitionSeqNum = -1;
	return *this;
}

BOOL CStreamData::ReadFromXML(ZQ::common::IPreference* itemIpref)
{
	if(itemIpref == NULL)
		return FALSE;

	char chValue[16];

	// get purchase id
	itemIpref->get(XML_STRMDATA_ITEM_PURCHASEID, chValue);
	m_dwPurchaseID = atol(chValue);

	// get stream id
	itemIpref->get(XML_STRMDATA_ITEM_SID, chValue);
	m_sid.dwStreamIdNumber = atol(chValue);

	// get asset id
	itemIpref->get(XML_STRMDATA_ITEM_ASSET, chValue);
	m_dwAssetID = atol(chValue);

	// get PreAEUID
	itemIpref->get(XML_STRMDATA_ITEM_PreAEUID, chValue);
	m_dwPreAEUID = atol(chValue);

	// get NewAEUID
	itemIpref->get(XML_STRMDATA_ITEM_NewAEUID, chValue);
	m_dwNewAEUID = atol(chValue);

	return TRUE;
}

BOOL CStreamData::WriteToXML(ZQ::common::IPreference* itemIpref)
{
	if(itemIpref == NULL)
		return FALSE;

	char chValue[64];
	// set purchase id
	ltoa(m_dwPurchaseID, chValue, 10);
	itemIpref->set(XML_STRMDATA_ITEM_PURCHASEID, chValue);

	// set stream id
	ltoa(m_sid.dwStreamIdNumber, chValue, 10);
	itemIpref->set(XML_STRMDATA_ITEM_SID, chValue);

	// set asset id
	ltoa(m_dwAssetID, chValue, 10);
	itemIpref->set(XML_STRMDATA_ITEM_ASSET, chValue);

	// set stream preAEUID
	ltoa(m_dwPreAEUID, chValue, 10);
	itemIpref->set(XML_STRMDATA_ITEM_PreAEUID, chValue);

	// set stream NewAEUID
	ltoa(m_dwNewAEUID, chValue, 10);
	itemIpref->set(XML_STRMDATA_ITEM_NewAEUID, chValue);

	return TRUE;
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
				m_bHasSch = true;
				return true;
			}
		}
		return false;
	}
	else
	{
		return true;
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
	// initialize com interface
	m_comInit = new ZQ::common::ComInitializer();

	m_streamDoc = new ZQ::common::XMLPrefDoc (*m_comInit);
}

CStreamDataManager::~CStreamDataManager()
{
	FreeXmlDoc();
}

BOOL CStreamDataManager::LoadStreamDoc(const wchar_t* wchFileName)
{
	wcscpy(m_wszStreamDocFileName, wchFileName);

	// convert to ANSI char
	char chFileName[MAX_PATH];
	size_t dwCount = MAX_PATH;
	wcstombs(chFileName, m_wszStreamDocFileName, dwCount);

	BOOL bXmlDoc = FALSE;
	BOOL bCreate = TRUE;
	ZQ::common::IPreference* rootIpref;

	WIN32_FIND_DATA findData;
	memset(&findData, 0x0, sizeof(WIN32_FIND_DATA));
	HANDLE hFind = FindFirstFile((LPCTSTR)m_wszStreamDocFileName, &findData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		// open exist xml file
		try
		{
			bXmlDoc = m_streamDoc->open(chFileName);
		}
		catch(...)
		{
			bXmlDoc = FALSE;
		}

		if(bXmlDoc)
		{
			bCreate = FALSE;
		}
		else
		{	// open failed then delete the bad file
			if(!DeleteFile((LPCTSTR)m_wszStreamDocFileName))
			{
				FreeXmlDoc();
				return FALSE;
			}
			bCreate = TRUE;
		}
	}

	if(bCreate)
	{
		// open xml file with creation
		bXmlDoc = m_streamDoc->open(chFileName, XMLDOC_CREATE);

		rootIpref = m_streamDoc->newElement(XML_STRMDATA_COLLECTION);
		m_streamDoc->set_root(rootIpref);

		rootIpref->free();
	}
	else
	{
		Resotre();
	}

	return TRUE;
}

BOOL CStreamDataManager::DeleteStreamDoc(void)
{
	if(m_streamDoc == NULL)
		return FALSE;
	return DeleteFile((LPCTSTR)m_wszStreamDocFileName);
}

BOOL CStreamDataManager::Resotre(void)
{
	if(m_streamDoc == NULL)
		return FALSE;

	ZQ::common::IPreference* rootIpref = m_streamDoc->root();
	ZQ::common::IPreference* itemIpref = NULL;

	itemIpref = rootIpref->firstChild();
	while(itemIpref != NULL)
	{
		CStreamData streamData;

		if(streamData.ReadFromXML(itemIpref))
		{
			// add StreamData to List
			m_lstStreamDatas.push_back(streamData);	
		}

		itemIpref->free();
		itemIpref = rootIpref->nextChild(); 
	}

	rootIpref->free();
	return TRUE;
}

BOOL CStreamDataManager::UpdateToFile(void)
{
	if(m_streamDoc == NULL)
		return FALSE;

	// convert to ANSI char
	char chFileName[MAX_PATH];
	size_t dwCount = MAX_PATH;
	wcstombs(chFileName, m_wszStreamDocFileName, dwCount);

	bool bret;
	m_xmlLock.enter();
	try
	{
		bret=m_streamDoc->save(chFileName);
	}
	catch (ZQ::common::Exception excep) {
		glog(Log::L_DEBUG, "CStreamDataManager::UpdateToFile failed, with error: %s", excep.getString());
	}
	m_xmlLock.leave();
	return bret;
}

void CStreamDataManager::FreeXmlDoc(void)
{
	if(m_streamDoc != NULL)
		delete m_streamDoc;
	m_streamDoc = NULL;
	
	// uninitialize com interface
	if(m_comInit != NULL)
		delete m_comInit;
	m_comInit = NULL;
}

ZQ::common::IPreference* CStreamDataManager::FindItemIprefByPchID(CStreamData& streamData)
{
	if(m_streamDoc == NULL)
		return NULL;

	// change the xml 
	DWORD dwPurchaseID;
	char chValue[64];

	ZQ::common::IPreference* rootIpref = m_streamDoc->root();
	ZQ::common::IPreference* itemIpref = NULL;

	itemIpref = rootIpref->firstChild(); 
	while(itemIpref != NULL)
	{
		// get purchase id
		itemIpref->get(XML_STRMDATA_ITEM_PURCHASEID, chValue);
		dwPurchaseID = atol(chValue);

		if(dwPurchaseID == streamData.GetPurchaseID())
		{
			rootIpref->free();
			return itemIpref;
		}
		else
		{
			itemIpref->free();
		}
		itemIpref = rootIpref->nextChild();
	}
	rootIpref->free();
	return NULL;
}

BOOL CStreamDataManager::AddStreamDataToXML(CStreamData& streamData)
{
	if(m_streamDoc == NULL)
		return FALSE;

	ZQ::common::IPreference* rootIpref = m_streamDoc->root();
	ZQ::common::IPreference* itemIpref = NULL;

	// add a status node
	itemIpref =m_streamDoc->newElement(XML_STRMDATA_ITEM);
	streamData.WriteToXML(itemIpref);
	rootIpref->addNextChild(itemIpref);

	itemIpref->free();
	rootIpref->free();

	return TRUE;
}

BOOL CStreamDataManager::RemoveStreamDataFromXML(CStreamData& streamData)
{
	if(m_streamDoc == NULL)
		return FALSE;

	ZQ::common::IPreference* rootIpref = m_streamDoc->root();
	ZQ::common::IPreference* itemIpref = FindItemIprefByPchID(streamData);
	if(itemIpref != NULL)
	{
		rootIpref->removeChild(itemIpref);

		rootIpref->free();
		itemIpref->free();

		return TRUE;
	}
	
	rootIpref->free();
	return FALSE;
}

BOOL CStreamDataManager::Add(CStreamData& streamData)
{
	list<CStreamData>::iterator desItr = QueryByPurchaseID(streamData.GetPurchaseID());		
	if(desItr != NULL)
	{	
		((list<CStreamData>::reference)(*desItr)) = streamData;
		glog(Log::L_DEBUG, _T("CStreamDataManager::ADD(REPALCE) with PurchaseID=%d, CurStreamDataCount=%d"), 
			streamData.GetPurchaseID(), m_lstStreamDatas.size());

		// update the xml value
		ZQ::common::IPreference* itemIpref = FindItemIprefByPchID(streamData);
		if(itemIpref != NULL)
		{
			streamData.WriteToXML(itemIpref);
			itemIpref->free();
		}
	}
	else
	{
		m_lstStreamDatas.push_back(streamData);	
		glog(Log::L_DEBUG, _T("CStreamDataManager::ADD(NEW) with PurchaseID=%d, CurStreamDataCount=%d"), 
			streamData.GetPurchaseID(), m_lstStreamDatas.size());

		// add data to xml
		AddStreamDataToXML(streamData);
	}
	// save the xml to file
	UpdateToFile();

	return TRUE;
}

BOOL CStreamDataManager::Remove(const STREAMID& sid)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);
	if(desItr != NULL)
	{
		// remove from xml
		RemoveStreamDataFromXML((list<CStreamData>::reference)(*desItr));

		glog(Log::L_DEBUG, _T("CStreamDataManager::Remove with PurchaseID=%d, CurStreamDataCount=%d"), 
			desItr->GetPurchaseID(), m_lstStreamDatas.size()-1);
		m_lstStreamDatas.erase(desItr);

		// save the xml to file
		UpdateToFile();

		return TRUE;
	}
	return FALSE;
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

	if(desItr != NULL)
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
		bool bPreExisted = desItr->ExistedAEID(dwPreAEUID);
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

		// update the xml value
		ZQ::common::IPreference* itemIpref = FindItemIprefByPchID((list<CStreamData>::reference)(*desItr));
		if(itemIpref != NULL)
		{
			desItr->WriteToXML(itemIpref);
			itemIpref->free();

			// save the xml to file
			UpdateToFile();
		}

		return nRet;
	}
	return NOTITYPE_NONE;
}


BOOL CStreamDataManager::GenSSStreamNotification(const STREAMID& sid, DWORD errCode, 
									DWORD errOp, SSStreamNotification& errNoti)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);

	if(desItr != NULL)
	{
		errNoti.dwPurchaseID = desItr->GetPurchaseID();
		errNoti.dwErrorCode = errCode;
		errNoti.dwOperation = errOp;

		return TRUE;
	}
	return FALSE;
}

DWORD CStreamDataManager::GetPurchaseID(const STREAMID& sid)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);
	if(desItr != NULL)
	{
		return desItr->GetPurchaseID();
	}
	return 0;
}

/// for save asset element id 
BOOL CStreamDataManager::AddAEID(const STREAMID& sid, DWORD dwAEID)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);
	if(desItr != NULL)
	{
		desItr->AddAEID(dwAEID);
		return TRUE;
	}
	return FALSE;
}

void CStreamDataManager::ClearAEIDs(const STREAMID& sid)
{
	list<CStreamData>::iterator desItr = QueryBySID(sid);
	if(desItr != NULL)
	{
		desItr->ClearAEIDs();
	}
}


list<CStreamData>::iterator CStreamDataManager::QueryBySID(const STREAMID& sid)
{
	list<CStreamData>::iterator itr = m_lstStreamDatas.begin();
	
	while(itr != m_lstStreamDatas.end())
	{
		if(itr->GetStreamID()->dwStreamIdNumber == sid.dwStreamIdNumber)
			return itr;
		*itr++;
	}
	return NULL;
}

list<CStreamData>::iterator CStreamDataManager::QueryByPurchaseID(DWORD dwPurchaseID)
{
	list<CStreamData>::iterator itr = m_lstStreamDatas.begin();
	
	while(itr != m_lstStreamDatas.end())
	{
		if(itr->GetPurchaseID() == dwPurchaseID)
			return itr;
		*itr++;
	}
	return NULL;
}
