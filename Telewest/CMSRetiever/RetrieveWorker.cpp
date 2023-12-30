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
// Ident : $Id: RetrieveWorker.cpp,v 1.8 2004/08/09 10:08:56 wli Exp $
// Branch: $Name:  $
// Author: kaliven lee
// Desc  : define worker class
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/CMSRetiever/RetrieveWorker.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 15    09-04-22 17:40 Haoyuan.lu
// 
// 14    06-11-08 15:20 Ken.qian
// 
// 13    06-08-04 13:57 Ken.qian
// Move from comextra to common
// 
// 12    05-05-17 11:35 Kaliven.lee
// fix abnomal nodegroup sequence
// 
// 11    05-05-16 20:14 Kaliven.lee
// 
// 10    05-05-12 22:15 Kaliven.lee
// 
// 9     05-05-12 22:14 Kaliven.lee
// avoid access null memory
// 
// 8     05-04-28 11:36 Kaliven.lee
// 
// 7     05-04-27 17:46 Kaliven.lee
// 
// 6     05-04-15 11:15 Kaliven.lee
// 
// 5     05-04-06 11:54 Kaliven.lee
// 
// 4     05-04-05 20:47 Kaliven.lee
// 
// 3     05-04-05 19:20 Kaliven.lee
// 
// 2     05-03-24 17:04 Kaliven.lee
// 
// 1     05-03-10 11:12 Kaliven.lee
// file create
// RetrieveWorker.cpp: implementation of the RetrieveWorker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RetrieveWorker.h"
#include "CMSRetriever.h"


#define MAXTEXTLEN		1024

extern ZQ::common::Log* pGlog;
extern CMSRetriever Server;

using ZQ::common::Log;
using ZQ::common::XMLPrefDoc;
using ZQ::common::IPreference;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RetrieveWorker::RetrieveWorker(char* fileName,DWORD dwTimeOut):ZQ::common::NativeThread(),m_bQuit(false)
{
	
	std::string str = fileName;
	
	DWORD dwIndex = str.find(";");
	m_sFileName1 = str.substr(0,dwIndex);
	if(dwIndex == -1)
		m_sFileName2 = m_sFileName1;
	else
		m_sFileName2 = str.substr(dwIndex+1,str.length());	
	m_hEvent = ::CreateEvent(NULL,TRUE,FALSE,NULL);
	m_dwTimeOut = dwTimeOut;
}

RetrieveWorker::~RetrieveWorker()
{
	if(m_hEvent)
		::CloseHandle(m_hEvent);
	if(m_pXMLDoc)
		delete m_pXMLDoc;
	if(m_pComInit)
		delete m_pComInit;
}
int RetrieveWorker::run()
{
	m_pComInit = new ZQ::common::ComInitializer();
	m_pXMLDoc = new ZQ::common::XMLPrefDoc(*m_pComInit);
	
	CString strSiteName(L"");
	CString strDBConn(L"");
	bool bHasConnStr = false;

	while(!m_bQuit)
	{	
		DWORD dwRtn = WaitForSingleObject(m_hEvent,m_dwTimeOut);
		if(m_bQuit)
			return 0;
		switch(dwRtn)
		{
			case WAIT_TIMEOUT:
				// get the first connection string
				bHasConnStr = Server.GetFirstConnString(strSiteName, strDBConn);
				while(bHasConnStr)
				{
					TRY{				
						if(m_LocalDB.OpenEx(strDBConn,CDatabase::noOdbcDialog) == TRUE)
						{
							readLoadFile();
						}										
					}CATCH(CDBException, e)
					{
						TCHAR buf[256];
						e->GetErrorMessage(buf,sizeof(buf),NULL);
						glog(Log::L_ERROR, L"%s - %s", LPCTSTR(strSiteName), buf);					
					}
					END_TRY
						
					TRY{
						m_LocalDB.Close();				
					}CATCH(CDBException, e)
					{
						TCHAR buf[256];
						e->GetErrorMessage(buf,sizeof(buf),NULL);
						glog(Log::L_ERROR, L"%s - %s", LPCTSTR(strSiteName), buf);					
					}
					END_TRY

					// get next connection string
					bHasConnStr = Server.GetNextConnString(strSiteName, strDBConn);
				}
				break;
			case WAIT_OBJECT_0:				
				return 0;
				break;
			default:
				break;
		}
	};
	return 0;
}
void RetrieveWorker::readLoadFile(void)
{
	SQLTCHAR sql[10240];
	
	bool isOpen;
	try{
		isOpen = m_pXMLDoc->open(m_sFileName1.c_str());
	}catch(ZQ::common::Exception e)
	{
		glog(Log::L_ERROR,_T("Error to open the XML file %S in MDS1:%S"),m_sFileName1.c_str(),e.getString());		
	}

	try{
		isOpen = m_pXMLDoc->open(m_sFileName2.c_str());
	}catch(ZQ::common::Exception e)
	{
		
		glog(Log::L_ERROR,_T("Error to open the XML file %S in MDS2:%S"),m_sFileName1.c_str(),e.getString());
		return;
	}
	//start transaction
	TRY{
		m_LocalDB.BeginTrans();
	}CATCH(CDBException ,e)
	{
		TCHAR wcsMsg[256];
		e->GetErrorMessage(wcsMsg,sizeof(wcsMsg),NULL);
		glog(Log::L_ERROR,_T("Exception caught when beginthran:%s"),wcsMsg);
	//	CMSRetriever::m_LocalDB.Rollback();	
		m_pXMLDoc->close();
		return;
	}
	END_TRY
	TRY{
		glog(Log::L_DEBUG,_T("DELETE from ote_server"));
		m_LocalDB.ExecuteSQL(_T("DELETE from ote_server"));
		glog(Log::L_DEBUG,_T("DELETE from ote_Instance"));
		m_LocalDB.ExecuteSQL(_T("DELETE from ote_Instance"));
		glog(Log::L_DEBUG,_T("DELETE from ote_nodegroup"));
		m_LocalDB.ExecuteSQL(_T("DELETE from ote_nodegroup"));
	}CATCH(CDBException, e)
	{
		TCHAR buf[256];
		e->GetErrorMessage(buf,sizeof(buf),NULL);
		glog(Log::L_ERROR,_T("Exception caught when execute sql:%s"),buf);
		m_pXMLDoc->close();
		m_LocalDB.Rollback();
		return ;
	}
	END_TRY
	IPreference* root;
	try
	{
	
		char sAppType[MAXTEXTLEN] = "\0";
		char sNodeGroup[MAXTEXTLEN] ="\0";
		
		char sPGLevel[MAXTEXTLEN] = "\0";
		char sVersion[MAXTEXTLEN] = "\0";
		char sDateTime[MAXTEXTLEN] = "\0";
		char sInterval[MAXTEXTLEN] = "\0";

		root = m_pXMLDoc->root();
		readHead(root,sVersion,sDateTime,sInterval);
		readCMGroup(root,sAppType,sNodeGroup,sPGLevel);
		
		// write to the LAM DB
		TRY
		{
			swprintf(sql,_T("INSERT INTO ote_server (Version,Date_Time,interval_time,App_Type,PG_Level) VALUES ('%S','%S',%S,'%S',%S)"),
				sVersion,sDateTime,sInterval,sAppType,sPGLevel);
			glog(Log::L_DEBUG,_T("%s"),sql);
			m_LocalDB.ExecuteSQL(sql);
		}CATCH(CDBException ,e)
		{
			TCHAR wcsMsg[256];
			e->GetErrorMessage(wcsMsg,sizeof(wcsMsg),NULL);
			glog(Log::L_ERROR,_T("Exception caught when execute sql:%s"),wcsMsg);
		//	CMSRetriever::m_LocalDB.Rollback();	
			m_LocalDB.Rollback();
			m_pXMLDoc->close();
			root->free();
			return;
		}
		END_TRY
		
		
	
		if (readInstance(root))
		{
			m_LocalDB.CommitTrans();
		}
		else
		{
			m_LocalDB.Rollback();
			m_pXMLDoc->close();
			root->free();
			return;
		}
		m_pXMLDoc->close();
		root->free();
	}catch (...) {
			glog(Log::L_ERROR,_T("Exception caught when read Load File"));
			m_LocalDB.Rollback();
			m_pXMLDoc->close();
			root->free();
	}
	
}
bool RetrieveWorker::readInstance(ZQ::common::IPreference* parent)
{
	SQLTCHAR sql[1024];
	
	DWORD dwInstanceNum = 0;
	glog(Log::L_DEBUG,_T("Read CM Instance information from XML file"));
	ZQ::common::IPreference* CMGroup = parent->firstChild("CMGroup");	
	ZQ::common::IPreference* instance = CMGroup->firstChild("Instance");
	if(instance == NULL)
		return false;
	
	do{
		char sIP[16];
		char sPort[16];
		char sLoad[64];
		char sLifeTime[64];
		try{		

			ZQ::common::IPreference* child =  instance->firstChild("IPAddress");	
			child->gettext(sIP);
			child->free();

			child = instance->firstChild("Port");
			child->gettext(sPort);
			child->free();
			
			child = instance->firstChild("Load");
			child->gettext(sLoad);
			child->free();
			
			child = instance->firstChild("LifeTime");
			child->gettext(sLifeTime);
			child->free();
		}catch(...)
		{
			glog(Log::L_ERROR,_T("failed to read the information of the instance."));
			instance->free();
			CMGroup->free();
			return false;
		}
		// write a record to LAM DB
			
		dwInstanceNum++;
		
		// free and next instance.
		instance->free();
		
		swprintf(sql,_T("INSERT INTO OTE_INSTANCE (INSTANCE_NUMBER,IP_ADDRESS,PORT,SERVER_LOAD,LIFE_TIME) VALUES (%d,'%S',%S,%S,%S)"),
			dwInstanceNum,sIP,sPort,sLoad,sLifeTime);
		TRY
		{
			glog(Log::L_DEBUG,sql);
			m_LocalDB.ExecuteSQL(sql);
		}CATCH(CDBException,e)
		{
			TCHAR buf[256];
			e->GetErrorMessage(buf,sizeof(buf),NULL);
			glog(Log::L_ERROR,_T("Exception caught when Execute sql:%s"),buf);
		//	CMSRetriever::m_LocalDB.Rollback();
			CMGroup->free();
			return false;
		}
		END_TRY
		instance = CMGroup->nextChild();
	}while(instance);
	CMGroup->free();
	return true;
}

bool RetrieveWorker::readHead(ZQ::common::IPreference* parent,char* sVer,char* sTime,char* sInterval)
{
	glog(Log::L_DEBUG,_T("Read Head information from XML file"));	
	ZQ::common::IPreference* head = parent->firstChild("Header");
	if(!head)
		return false;
	try
	{
		ZQ::common::IPreference* child = head->firstChild("Version");
	
		child->gettext(sVer);
		child->free();

		child = head->firstChild("DateTime");
		child->gettext(sTime);
		child->free();
		
		child = head->firstChild("Interval");
		child->gettext(sInterval);	
		child->free();
	}catch(...)
	{
		glog(Log::L_ERROR,_T("failed to read the information of the header."));
	}
	head->free();
	return true;
}

bool  RetrieveWorker::readCMGroup(ZQ::common::IPreference* parent,char* sAppType,char* sNodeGroup,char* sPGLevel)
{
	SQLTCHAR sql[1024];
	glog(Log::L_DEBUG,_T("Read CM Group information from XML file"));
	ZQ::common::IPreference* CMGroup = parent->firstChild("CMGroup");
	ZQ::common::IPreference* child = CMGroup->firstChild("AppType");
	char buf[64];
	// read app type
	while(child) {
		try{
			memset(buf,0,64);
			child->gettext(buf);

			DWORD dwLen = strlen(sAppType);
			if(dwLen)
				strcat(sAppType,"_");
			strcat(sAppType,buf);
			
			child->free();
			child = CMGroup->nextChild();

		}catch(...)
		{
			glog(Log::L_INFO,_T("failed to read the information of the AppType."));
		}	
	};	
	//read node group
	child = CMGroup->firstChild("NodeGroup");
	int sequence_no = 0;
	while(child){
		sequence_no++;
		try{
			memset(buf,0,64);
			child->gettext(buf);
			try{
				wsprintf(sql,_T("INSERT INTO OTE_NODEGROUP (SEQUENCE_NO,NODEGROUP)VALUES(%d,'%S')"),sequence_no,buf);
				glog(Log::L_DEBUG,sql);
				m_LocalDB.ExecuteSQL(sql);
			}catch (CDBException e) {
				TCHAR buf[256];
				e.GetErrorMessage(buf,sizeof(buf),NULL);
				glog(Log::L_ERROR,_T("Exception caught when Execute sql:%s"),buf);
			}

			child->free();
			child = CMGroup->nextChild();
		}catch(...)
		{
			glog(Log::L_ERROR,_T("failed to read the information of the NodeGroup."));
		}
	};

	child = CMGroup->firstChild("PGLevel");
	if(child)
	{
		try
		{
			child->gettext(sPGLevel);
			child->free();
		}catch(...)
		{
			glog(Log::L_ERROR,_T("Failed to read the information of the PGLevel."));
		}
	}
	CMGroup->free();
	return true;
}
void RetrieveWorker::stopWorker(void)
{
	::SetEvent(m_hEvent);
	m_bQuit = true;
}
void RetrieveWorker::setLoadFileName(char* fileName)
{
	std::string str = fileName;
	DWORD dwIndex = str.find(";");
	m_sFileName1 = str.substr(0,dwIndex);
	m_sFileName2= str.substr(dwIndex+1,str.length());
}