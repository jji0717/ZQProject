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
//
// Ident : $Id: CMSRetriever.cpp,v 1.8 2004/08/09 10:08:56 wli Exp $
// Branch: $Name:  $
// Author: kaliven lee
// Desc  : define srvload service
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/CMSRetiever/CMSRetriever.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 7     06-11-08 15:20 Ken.qian
// 
// 6     06-08-04 13:57 Ken.qian
// Move from comextra to common
// 
// 5     06-03-29 15:48 Bernie.zhao
// fixed handle leak in XML parser, as well as logging stop issue
// 
// 4     05-04-07 19:48 Kaliven.lee
// 
// 3     05-04-06 12:28 Kaliven.lee
// version change
// 
// 2     05-03-24 17:36 Kaliven.lee
// 
// 1     05-03-10 11:12 Kaliven.lee
// file create
#include "ZQResource.h"
#include "CMSRetriever.h"
#include "comutil.h"

#define DEFAULTLOADFILENAME	L"c:\\LoadFile\\srvrlist.xml"
#define	DEFAULTDSN			L"multiverse"
#define	DEFAULTUSER			L"multiverse"
#define	DEFAULTPWD			L"multiverse"
#define	DEFAULTDB			L"multiverse"
#define DEFCFGFILE          L""


#define XML_APP_SITE_NAME               L"Name"
#define XML_APP_SITE_DSN                L"DSN"
#define XML_APP_SITE_DATABASE           L"Database"
#define XML_APP_SITE_USERNAME           L"UserName"
#define XML_APP_SITE_PASSWD             L"Password"

using ZQ::common::BaseSchangeServiceApplication;
using ZQ::common::Log;

CMSRetriever Server;
ZQ::common::BaseSchangeServiceApplication * Application = &Server;

DWORD  gdwServiceType = 0;
DWORD gdwServiceInstance = 1;


CMSRetriever::CMSRetriever()
{
	m_bStarted = false;

	wcscpy(m_LoadFileName, L"");
	wcscpy(m_LAMDBDSN, DEFAULTDSN);
	wcscpy(m_LAMDBUSERNAME, DEFAULTUSER);
	wcscpy(m_LAMDBUSERNAME, DEFAULTUSER);
	wcscpy(m_LAMDBNAME, DEFAULTDB);
	wcscpy(m_szAppSitesCfg, DEFCFGFILE);

	m_nIndex = 0;
}
CMSRetriever::~CMSRetriever()
{
}
HRESULT CMSRetriever::OnInit(void)
{
	BaseSchangeServiceApplication::OnInit();
	ZQ::common::pGlog = m_pReporter;
	glog(Log::L_NOTICE, "**********************SrvLoad Service %s launched**********************************",ZQ_PRODUCT_VER_STR1);
	DWORD dwSize = MAXFILELEN*2;
	getConfigValue(L"LOADFILENAME", m_LoadFileName,m_LoadFileName,&dwSize,true,true);
	dwSize = MAXDNSLENGTH*2;
	getConfigValue(L"LAMDBDSN",m_LAMDBDSN,m_LAMDBDSN,&dwSize,true,true);
	dwSize = MAXUSERNAMELENGTH*2;
	getConfigValue(L"LAMDBUSERNAME",m_LAMDBUSERNAME,m_LAMDBUSERNAME,&dwSize,true,true);
	dwSize = MAXPASSWORDLENGTH*2;
	getConfigValue(L"LAMDBPASSWORD",m_LAMDBPASSWORD,m_LAMDBPASSWORD,&dwSize,true,true);	
	dwSize = MAXDBNAMELENGTH*2;
	getConfigValue(L"LAMDBNAME",m_LAMDBNAME,m_LAMDBNAME,&dwSize,true,true);	
	getConfigValue(L"READINTERVAL",&m_dwWorkTimeOut,0,true,true);

	dwSize = MAX_PATH;
	getConfigValue(L"AppSitesConfigurationFile", m_szAppSitesCfg, m_szAppSitesCfg, &dwSize,true,true);	
	
	// Does this MOD plugin use Application configuration file.
	if(wcscmp(m_szAppSitesCfg, L"") == 0)
	{
		CString strConn(L"");
		strConn.Format(L"DSN=%s;UID=%s;PWD=%s;DATABASE:%s",
						m_LAMDBDSN, m_LAMDBUSERNAME, m_LAMDBPASSWORD, m_LAMDBNAME);	
		m_arrConnStrs.Add(strConn);
		m_arrSites.Add(m_LAMDBDSN);

		glog(ZQ::common::Log::L_DEBUG, L"Application Site: DSN=%s Database=% UserName=%s Password=%s", 
					m_LAMDBDSN, m_LAMDBNAME, m_LAMDBUSERNAME, m_LAMDBPASSWORD);
	}
	else
	{
		glog(ZQ::common::Log::L_DEBUG, L"SrvLoad supports multiple WFES");

		bool bLoaded = LoadConfigFile();
		if(!bLoaded)
		{
			glog(ZQ::common::Log::L_ERROR, L"Load WFES configuration file %s failed", m_szAppSitesCfg);
			return S_FALSE;
		}
		if(m_arrConnStrs.GetUpperBound() == -1)
		{
			glog(ZQ::common::Log::L_ERROR, L"There is no valid WFES configuration in %s ", m_szAppSitesCfg);
			return S_FALSE;
		}
	}

	char * LoadFile = _com_util::ConvertBSTRToString(m_LoadFileName);
	m_pWorker = new RetrieveWorker(LoadFile,m_dwWorkTimeOut);
	delete[] LoadFile;
	return S_OK;
}
HRESULT CMSRetriever::OnStop(void)
{
	m_pWorker->stopWorker();
	m_pWorker->waitHandle(500);

	
	BaseSchangeServiceApplication::OnStop();
	m_bStarted =false;
	return S_OK;
}
HRESULT CMSRetriever::OnStart(void)
{
	BaseSchangeServiceApplication::OnStart();
	m_pWorker->start();
	m_bStarted =true;
	return S_OK;
}
HRESULT CMSRetriever::OnUnInit(void)
{
	if(m_pWorker)
		delete m_pWorker;

	m_arrSites.RemoveAll();
	m_arrConnStrs.RemoveAll();

	BaseSchangeServiceApplication::OnUnInit();
	return S_OK;
}


bool CMSRetriever::isHealth(void)
{
	if((!m_pWorker->isRunning())&&m_bStarted)
	{
		m_pWorker->stopWorker();
		m_pWorker->waitHandle(500);
		delete m_pWorker;
		char* LoadFile =_com_util::ConvertBSTRToString(m_LoadFileName);
		m_pWorker = new RetrieveWorker(LoadFile,m_dwWorkTimeOut);
		delete[] LoadFile;
		m_pWorker->start();
	}
	return true;
}
void CMSRetriever::Stop(void)
{
	stopService();
}

bool CMSRetriever::LoadConfigFile()
{
	// initialize com interface
	ZQ::common::ComInitializer comInit;

	ZQ::common::XMLPrefDoc* xmlDoc = new ZQ::common::XMLPrefDoc(comInit);

	char chFileName[MAX_PATH];
	size_t dwCount = MAX_PATH;
	wcstombs(chFileName, m_szAppSitesCfg, dwCount);

	// open exist xml file
	bool bResult = true;
	try
	{
		bResult = xmlDoc->open(chFileName);
	}
	catch(...)
	{
		bResult = false;
	}
	if(!bResult)
	{
		delete xmlDoc;
		return false;
	}
	
	// read the xml file
	ZQ::common::PrefGuard rootPrefG = xmlDoc->root();
	ZQ::common::PrefGuard itemPrefG;

	if(!rootPrefG.valid())
	{
		delete xmlDoc;
		return false;
	}
	
	itemPrefG.pref(rootPrefG.pref()->firstChild());
	while(itemPrefG.valid())
	{
		// get Name
		wchar_t wszSiteName[256] = {0};
		itemPrefG.pref()->getUnicode(XML_APP_SITE_NAME, wszSiteName);

		// get DSN
		wchar_t wszDSN[256] = {0};
		itemPrefG.pref()->getUnicode(XML_APP_SITE_DSN, wszDSN);

		// get Database
		wchar_t wszDatabase[256] = {0};
		itemPrefG.pref()->getUnicode(XML_APP_SITE_DATABASE, wszDatabase);

		// get UserName
		wchar_t wszUserName[256] = {0};
		itemPrefG.pref()->getUnicode(XML_APP_SITE_USERNAME, wszUserName);

		// get Password
		wchar_t wszPassword[256] = {0};
		itemPrefG.pref()->getUnicode(XML_APP_SITE_PASSWD, wszPassword);

		CString strConn(L"");
		strConn.Format(L"DSN=%s;UID=%s;PWD=%s;DATABASE:%s",
						wszDSN, wszUserName, wszPassword, wszDatabase);	
		m_arrConnStrs.Add(strConn);
		m_arrSites.Add(wszSiteName);

		glog(ZQ::common::Log::L_DEBUG, L"Application Site: %s - DSN=%s Database=% UserName=%s Password=%s", 
					wszSiteName, wszDSN, wszDatabase, wszUserName, wszPassword);

		itemPrefG.pref(rootPrefG.pref()->nextChild());
	}

	xmlDoc->close();

	delete xmlDoc;
	return true;

}

bool CMSRetriever::GetFirstConnString(CString& strSiteName, CString& strconn)
{
	if(-1 == m_arrConnStrs.GetUpperBound())
		return false;
	
	m_nIndex = 0;
	strconn = m_arrConnStrs[m_nIndex];
	strSiteName = m_arrSites[m_nIndex];

	return true;
}

bool CMSRetriever::GetNextConnString(CString& strSiteName, CString& strconn)
{
	m_nIndex++;
	if(m_nIndex > m_arrConnStrs.GetUpperBound())
	{
		return false;
	}
	strconn = m_arrConnStrs[m_nIndex];
	strSiteName = m_arrSites[m_nIndex];

	return true;
}
