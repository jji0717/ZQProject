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
// Ident : $Id: AppSiteData.cpp,v 1.1 2005/07/14 11:00:00 Ken Qian $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : impl the application site configuation data & management 
//
// Revision History: 
// ---------------------------------------------------------------------------
// 
// 
// Revision 1.1  2005/07/14 11:00:00 Ken Qian
//   definition and implemention
//
// ===========================================================================
#include "AppSite.h"
using namespace ZQ::common;


#include <tchar.h>


///////////////////////////////////////////////////////////////////////////
////////			CAppSiteData implementation					///////////
///////////////////////////////////////////////////////////////////////////

CAppSiteData::CAppSiteData(string strName, string strID, string strWSDL)
{
	m_strName = strName;
	m_strID   = strID;
	m_strWSDL = strWSDL;
}

CAppSiteData::CAppSiteData(const CAppSiteData& rhs)
{
	m_strName = rhs.m_strName;
	m_strID   = rhs.m_strID;
	m_strWSDL = rhs.m_strWSDL;
}

CAppSiteData::~CAppSiteData()
{

}

const CAppSiteData& CAppSiteData::operator=(const CAppSiteData& rhs)
{
	if(this == &rhs)
		return *this;

	m_strName = rhs.m_strName;
	m_strID   = rhs.m_strID;
	m_strWSDL = rhs.m_strWSDL;

	return *this;
}

BOOL CAppSiteData::ReadFromXML(ZQ::common::PrefGuard& itemPrefG)
{
	if(!itemPrefG.valid())
		return FALSE;

	char chValue[256];

	// get Name
	itemPrefG.pref()->get(XML_APP_SITE_NAME, chValue);
	m_strName = chValue;

	// get ID
	itemPrefG.pref()->get(XML_APP_SITE_ID, chValue);
	m_strID = chValue;

	// get WSDL
	itemPrefG.pref()->get(XML_APP_SITE_WSDL, chValue);
	m_strWSDL = chValue;

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////
////////	   CAppSiteDataManager implementation           ///////////
///////////////////////////////////////////////////////////////////////////

CAppSiteDataManager::CAppSiteDataManager()
{
	m_dwPrefixLength = 2; // 2 as default

	// initialize com interface
	m_comInit = new ZQ::common::ComInitializer();

	m_xmlDoc = new ZQ::common::XMLPrefDoc (*m_comInit);
}

CAppSiteDataManager::~CAppSiteDataManager()
{
	FreeXmlDoc();
}

bool CAppSiteDataManager::ReadConfiguration(const wchar_t* wchFileName)
{
	// convert to ANSI char
	char chFileName[MAX_PATH];
	size_t dwCount = MAX_PATH;
	wcstombs(chFileName, wchFileName, dwCount);

	BOOL bResult = FALSE;

	WIN32_FIND_DATA findData;
	memset(&findData, 0x0, sizeof(WIN32_FIND_DATA));
	HANDLE hFind = FindFirstFile((LPCTSTR)wchFileName, &findData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		// open exist xml file
		try
		{
			bResult = m_xmlDoc->open(chFileName);
		}
		catch(...)
		{
			bResult = false;
		}
	}
	if(!bResult)
		return false;

	ZQ::common::PrefGuard rootPrefG = m_xmlDoc->root();
	ZQ::common::PrefGuard itemPrefG;

	if(!rootPrefG.valid())
		return false;
	
	// get ID
	char chValue[256];
	rootPrefG.pref()->get(XML_APPLICATION_PREFIX, chValue);
	m_dwPrefixLength = atol(chValue);
	
	itemPrefG.pref(rootPrefG.pref()->firstChild());
	while(itemPrefG.valid())
	{
		CAppSiteData AppSiteData;

		if(AppSiteData.ReadFromXML(itemPrefG))
		{
			// add AppSiteData to List
			m_lstAppSiteDatas.push_back(AppSiteData);	
		}

		itemPrefG.pref(rootPrefG.pref()->nextChild());
	}

	return true;
}

void CAppSiteDataManager::FreeXmlDoc(void)
{
	if(m_xmlDoc != NULL)
		delete m_xmlDoc;
	m_xmlDoc = NULL;
	
	// uninitialize com interface
	if(m_comInit != NULL)
		delete m_comInit;
	m_comInit = NULL;
}

CAppSiteData* CAppSiteDataManager::GetSiteDataByID(string strID)
{
	list<CAppSiteData>::iterator itr = m_lstAppSiteDatas.begin();
	
	while(itr != m_lstAppSiteDatas.end())
	{
		if( itr->GetID().compare(strID) == 0 )
		{
			return &((list<CAppSiteData>::reference)(*itr));
		}
		itr++;
	}
	return NULL;
}

