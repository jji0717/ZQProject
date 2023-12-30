
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
// Ident : $Id: FtpMover.cpp,v 1.5 2004/07/29 05:13:27 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : impl the Main service class FtpConnMgr
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/FtpMover/FtpMover.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 9     06-08-04 13:30 Ken.qian
// move from comextra to common
// 
// 8     05-04-07 20:03 Kaliven.lee
// 
// 7     05-04-07 20:03 Kaliven.lee
// 
// 6     05-04-07 18:32 Kaliven.lee
// change because of the zqthread class change
// 
// 5     05-04-04 16:58 Kaliven.lee
// support the path string end without "\"
// 
// 4     05-03-24 17:02 Kaliven.lee
// 
// 2     05-02-04 11:36 Kaliven.lee
// add head
// 
// 
//
//////////////////////////////////////////////////////////////////////
// FtpMover.cpp: implementation of the FtpMover class.
//
//////////////////////////////////////////////////////////////////////

#include "FtpMover.h"
#include "FtpMoverConf.h"
#include "ZQResource.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define FTPMOVER_MANPKG_WORKQUEUE	_T("WorkQueue")
#define FTPMOVER_MANPKG_FTPCONN		_T("FtpConn")

using ZQ::common::BaseSchangeServiceApplication;
using ZQ::common::Log;
ZQ::common::Log* pGlog = NULL;
FtpMover server;
ZQ::common::BaseSchangeServiceApplication* Application = &server;

using ZQ::common::IPreference ;
WORKQUEUE gWorkQueue;
ZQ::common::Mutex g_WorkQueueLock;
TASKLIST gTasks;
FILELOCKLIST gFileLocks;


wchar_t	gwcsServer[64] =_T("192.168.80.236");
wchar_t	gwcsPassword[64] = _T("AM");
wchar_t	gwcsUserName[64] = _T("AM");
DWORD gdwPort = 80;

wchar_t FtpMover::m_wcsGuardDir[MAX_PATH] ;
wchar_t FtpMover::m_wcsTempDir[MAX_PATH] ;
wchar_t FtpMover::m_wcsConfigFile[MAX_PATH] = _T("FtpMover.xml");
DWORD FtpMover::m_dwScanDirInterval = 2000;
DWORD FtpMover::m_dwScanWQInterval = 2000;
DWORD FtpMover::m_dwMaxConnCount = 1;

FtpConnMgr* FtpMover::m_pFtpConMgr;
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;
FtpMover::FtpMover():BaseSchangeServiceApplication()
{

}

FtpMover::~FtpMover()
{

}
HRESULT FtpMover::OnInit(void)
{
	BaseSchangeServiceApplication::OnInit();
	pGlog = m_pReporter;
	glog(Log::L_NOTICE,_T("***************FTP Mover %S***************"),ZQ_PRODUCT_VER_STR1);
	
	DWORD dwSize = sizeof(m_wcsConfigFile);
	getConfigValue(_T("ConfigFileName"),m_wcsConfigFile,m_wcsConfigFile,&dwSize,true,true);
	// processing the path 

	ReadConfigFile();
	m_pFtpConMgr = new FtpConnMgr(m_dwMaxConnCount,m_dwScanWQInterval);

	m_pScanner = new FtpMoverScanner(m_dwScanDirInterval);
	
	DWORD dwErr = 0;
	
	m_pFtpConMgr->Initialize();
	manageVar(FTPMOVER_MANPKG_WORKQUEUE,MAN_COMPLEX,(DWORD)MgmtWorkQueue,true,&dwErr);
	return S_OK;
}
HRESULT FtpMover::OnStart(void)
{
	BaseSchangeServiceApplication::OnStart();
	m_pFtpConMgr->start();
	m_pScanner->start();	
	
	return S_OK;
}
HRESULT FtpMover::OnStop(void)
{
	BaseSchangeServiceApplication::OnStop();
	m_pFtpConMgr->Stop();
	m_pScanner->Stop();	
	return S_OK;
}
HRESULT FtpMover::OnUnInit(void)
{
	BaseSchangeServiceApplication::OnUnInit();
	if(m_pFtpConMgr)
		delete m_pFtpConMgr;
	if(m_pScanner)
		delete m_pScanner;
	if(gFileLocks.size()!=0)
	{
		FILELOCKLIST::iterator iter;
		for (iter = gFileLocks.begin();iter != gFileLocks.end(); iter++)
		{
			gFileLocks.erase(iter);
			iter = gFileLocks.begin();
		}
	}
	if(gTasks.size()!=0 )
	{
		TASK* pTask;
		TASKLIST::iterator taskIter;
		for (;gTasks.size()!= 0;)
		{
			taskIter = gTasks.begin();
			pTask = *taskIter;
			if(pTask->FtpServerList.size() != 0)
			{
				FTPSERVER *pServer;
				SERVERLIST::iterator iter;
				for(;pTask->FtpServerList.size()!=0;)
				{
					iter = pTask->FtpServerList.begin();
					pServer = *iter;
					delete pServer;
					pTask->FtpServerList.erase(iter);
				}
				
			}
			delete pTask;
			gTasks.erase(taskIter);
		}
	}
	return S_OK;
}
bool FtpMover::isHealth(void)
{
//	if((!m_pScanner->isRunning())||(!m_pFtpConMgr->isRunning()))
//	{
//		logEvent(_T("Thread run abnormally.Exit the service and let Service shell restart it."),Log::L_NOTICE);
//		HANDLE hStop = ::OpenEvent(EVENT_ALL_ACCESS,FALSE,_T("SERVICEAPP_Stop"));
//		::SetEvent(hStop);		
//	}
	return true;
}
UINT FtpMover::MgmtWorkQueue(WCHAR *pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength)
{
	WORD wCommand;
	swscanf(pwszCmd, L"%c\t", &wCommand);

	// Dispatch based on requested operation
	if (wCommand == MAN_FREE)
	{
		// Free the allocated response buffer
		if (*ppwszBuffer != NULL)
		{
			delete [] *ppwszBuffer;
			*ppwszBuffer = NULL;
		}
		return MAN_SUCCESS;
	}
	else if (wCommand != MAN_READ_VARS)
	{
		return MAN_BAD_PARAM;
	}
	WCHAR *pszTmp = wcschr(pwszCmd, '\t');
	WCHAR *pszWhich = pszTmp + 1;	

	if (0 == wcsncmp(pszWhich, FTPMOVER_MANPKG_WORKQUEUE, wcslen(FTPMOVER_MANPKG_WORKQUEUE)))
	{
		wchar_t wsOutPut[4096];
		wchar_t *tmp = wsOutPut;

		DWORD dwVarCount = 0;
		DWORD dwColumnCount = 9;
		DWORD dwTaskCount = gWorkQueue.size(); 

		tmp += wsprintf(tmp,L"%d\t%d\n",dwVarCount,dwColumnCount);
		std::vector<TRANSFERTASK> Dis_Tasks;
		DWORD i =0;
		{
			g_WorkQueueLock.enter();
			for( i = 0; i < dwTaskCount; i ++)
			{
				TRANSFERTASK task;
				task.dwPort = gWorkQueue.at(i)->dwPort;
				task.state = gWorkQueue.at(i)->state;
				wcscpy(task.wcsDestineServer , gWorkQueue.at(i)->wcsDestineServer);
				wcscpy(task.wcsFileName , gWorkQueue.at(i)->wcsFileName);
				wcscpy(task.wcsPassword, gWorkQueue.at(i)->wcsPassword);
				wcscpy(task.wcsRemoteFileName,gWorkQueue.at(i)->wcsRemoteFileName);
				wcscpy(task.wcsUserName,gWorkQueue.at(i)->wcsUserName);
				task.percentage = gWorkQueue.at(i)->percentage;
				task.dwFtpConnID = gWorkQueue.at(i)->dwFtpConnID;
				Dis_Tasks.push_back(task);
			}			
			g_WorkQueueLock.leave();
		}
		TRANSFERTASK task;

		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_INT, dwTaskCount, L"ID");
	
		for( i = 0; i < dwTaskCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>					
			tmp += wsprintf(tmp, L"%d\n",i+1);			
		}

		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTaskCount, L"ServerAddress");
	
		for( i = 0; i < dwTaskCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>	
			task = Dis_Tasks.at(i);
			tmp += wsprintf(tmp, L"%s\n",task.wcsDestineServer);			
		}
		
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTaskCount, L"Port");
	
		for( i = 0; i < dwTaskCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>	
			task = Dis_Tasks.at(i);
			tmp += wsprintf(tmp, L"%d\n",task.dwPort);			
		}
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTaskCount, L"UserName");
	
		for( i = 0; i < dwTaskCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>	
			task = Dis_Tasks.at(i);
			tmp += wsprintf(tmp, L"%s\n",task.wcsUserName);			
		}
/*
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTaskCount, L"Password");
	
		for( i = 0; i < dwTaskCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>	
			task = Dis_Tasks.at(i);
			tmp += wsprintf(tmp, L"%s\n",task.wcsPassword);			
		}
		*/
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTaskCount, L"SourceFile");
	
		for( i = 0; i < dwTaskCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>	
			task = Dis_Tasks.at(i);
			tmp += wsprintf(tmp, L"%s\n",task.wcsFileName);			
		}
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTaskCount, L"DestineFile");
	
		for( i = 0; i < dwTaskCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>	
			task = Dis_Tasks.at(i);
			tmp += wsprintf(tmp, L"%s\n",task.wcsRemoteFileName);			
		}
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTaskCount, L"ConnectionID");
	
		for( i = 0; i < dwTaskCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>	
			task = Dis_Tasks.at(i);
			tmp += wsprintf(tmp, L"%d\n",task.dwFtpConnID);			
		}		

		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_INT, dwTaskCount, L"Percentage");
	
		for( i = 0; i < dwTaskCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>	
			task = Dis_Tasks.at(i);
			tmp += swprintf(tmp, L"%.2f%%\n",task.percentage);			
		}
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTaskCount, L"State");
		for( i = 0; i < dwTaskCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>	
			task = Dis_Tasks.at(i);
			switch(task.state)
			{
				case Ready:
					tmp += wsprintf(tmp, L"%s\n",_T("Ready"));	
				break;
				case 	Processing:
					tmp += wsprintf(tmp, L"%s\n",_T("Processing"));	
				break;
				case	Finished:
					tmp += wsprintf(tmp, L"%s\n",_T("Finished"));	
				break;
				case	Failed:
					tmp += wsprintf(tmp, L"%s\n",_T("Failed"));	
				break;
				default:
					break;
			}
			
		}
		*pdwLength = wcslen(wsOutPut);
		wchar_t* pOutputBuf = new wchar_t[*pdwLength+1];
		wcscpy(pOutputBuf, wsOutPut);
		*ppwszBuffer = pOutputBuf;
	}
	return MAN_SUCCESS;
}
UINT FtpMover::MgmtFtpConn(WCHAR *pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength)
{
	WORD wCommand;
	swscanf(pwszCmd, L"%c\t", &wCommand);

	// Dispatch based on requested operation
	if (wCommand == MAN_FREE)
	{
		// Free the allocated response buffer
		if (*ppwszBuffer != NULL)
		{
			delete [] *ppwszBuffer;
			*ppwszBuffer = NULL;
		}
		return MAN_SUCCESS;
	}
	else if (wCommand != MAN_READ_VARS)
	{
		return MAN_BAD_PARAM;
	}
	WCHAR *pszTmp = wcschr(pwszCmd, '\t');
	WCHAR *pszWhich = pszTmp + 1;	

	if (0 == wcsncmp(pszWhich, FTPMOVER_MANPKG_FTPCONN, wcslen(FTPMOVER_MANPKG_FTPCONN)))
	{
		wchar_t wsOutPut[4096];
		wchar_t *tmp = wsOutPut;

		DWORD dwVarCount = 0;
		DWORD dwColumnCount = 3;


		tmp += wsprintf(tmp,L"%d\t%d\n",dwVarCount,dwColumnCount);
		
		std::vector<DISFTPCONNINFO> Dis_Conns;
		DWORD i =0;
		{
			m_pFtpConMgr->m_ConnLock.enter();
			FtpConn* pFtpConn  = NULL;
			for( i = 0; i < m_dwMaxConnCount; i ++)
			{
				pFtpConn = m_pFtpConMgr->ConnectionList.at(i);
				DISFTPCONNINFO DisConnInfo;
				DisConnInfo.dwId = pFtpConn->GetID();
				if(pFtpConn->m_pTask)
					wcscpy(DisConnInfo.wcsFileName,pFtpConn->m_pTask->wcsFileName);
				else
					wcscpy(DisConnInfo.wcsFileName,_T("")); 
				DisConnInfo.states =  pFtpConn->GetCurState();
				Dis_Conns.push_back(DisConnInfo);
			}
			m_pFtpConMgr->m_ConnLock.leave();
		}
		DISFTPCONNINFO DisConnInfo;

		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_INT, dwColumnCount, L"ID");
	
		for( i = 0; i < m_dwMaxConnCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>	
			DisConnInfo =Dis_Conns.at(i) ;
			tmp += wsprintf(tmp, L"%d\n",DisConnInfo.dwId);			
		}

		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwColumnCount, L"TransferFile");
	
		for( i = 0; i < m_dwMaxConnCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>	
			DisConnInfo =Dis_Conns.at(i) ;
			tmp += wsprintf(tmp, L"%s\n",DisConnInfo.wcsFileName);			
		}
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwColumnCount, L"Status");
	
		for( i = 0; i < m_dwMaxConnCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>	
			DisConnInfo =Dis_Conns.at(i) ;
			CString csStr;
			switch(DisConnInfo.states)
			{
			case FTPCONN_STATE_NOMORAL:
				csStr = _T("NORMAL");
				break;
			case FTPCONN_STATE_TRANSFERRING:
				csStr = _T("TRANSFERRING");
				break;
			case FTPCONN_STATE_READY:
				csStr = _T("READY");
				break;
			case FTPCONN_STATE_TRANSFERRED:
				csStr = _T("TRANSFERRED");
				break;
			case FTPCONN_STATE_READFAILED:
				csStr = _T("READFAILED");
				break;
			case FTPCONN_STATE_WRITEFAILED:
				csStr = _T("WRITEFAILED");
				break;
			case FTPCONN_STATE_CONNECTFAILED:
				csStr = _T("CONNECTFAILED");
				break;

			default:
				break;
				
			}
			tmp += wsprintf(tmp, L"%s\n",csStr);			
		}

	
		*pdwLength = wcslen(wsOutPut);
		wchar_t* pOutputBuf = new wchar_t[*pdwLength+1];
		wcscpy(pOutputBuf, wsOutPut);
		*ppwszBuffer = pOutputBuf;
	}
	return MAN_SUCCESS;
}

HRESULT FtpMover::ReadConfigFile(void)
{
	ZQ::common::ComInitializer ComInit;
	ZQ::common::XMLPrefDoc* pXmlDoc = new ZQ::common::XMLPrefDoc(ComInit);
	char filename[MAX_PATH];
	wcstombs(filename,m_wcsConfigFile,sizeof(m_wcsConfigFile));
	bool isOpen;
	try{
		isOpen = pXmlDoc->open(filename);
	}catch(ZQ::common::Exception e)
	{
		
		glog(Log::L_ERROR,_T("Error to open the configfile %s:%S"),m_wcsConfigFile,e.getString());		
		delete pXmlDoc;
		return 1;
	}
	glog(Log::L_DEBUG,_T("Success to open the config file %s.start to read the configuration."),m_wcsConfigFile);		
	
	char ScanDirInterval[32];
	char ScanWQInterval[32];
	char MaxConnections[32];
	IPreference* root = pXmlDoc->root();
	//read setting node
	try{
	
		ZQ::common::IPreference* Setting= root->firstChild("Common");
		ZQ::common::IPreference* child = Setting->firstChild("ScanDirInterval");
		child->gettext(ScanDirInterval);
		m_dwScanDirInterval = atol(ScanDirInterval);
		DWORD dwError= 0;
		manageVar(_T("ScanDirInterval"),MAN_INT,(DWORD)&m_dwScanDirInterval,true,&dwError);
		child->free();

		child = Setting->firstChild("ScanWQInterval");
		child->gettext(ScanWQInterval);
		m_dwScanDirInterval = atol(ScanWQInterval);		
		manageVar(_T("ScanWQInterval"),MAN_INT,(DWORD)&m_dwScanDirInterval,true,&dwError);
		child->free();

		child = Setting->firstChild("MaxConnections");
		child->gettext(MaxConnections);
		m_dwMaxConnCount = atol(MaxConnections);
		manageVar(_T("MaxConnections"),MAN_INT,(DWORD)&m_dwMaxConnCount,true,&dwError);
		
		child->free();
		Setting->free();
	}catch(ZQ::common::Exception e)
	{
		glog(Log::L_DEBUG,_T("Failed to read the setting node:%S"),e.getString());				
	}
	//read task node
	try{
	
		
		char GuardDir[MAX_PATH];
		char TempDir[MAX_PATH];
		char address[32];
		char Port[32];
		char username[64];
		char password[64];
		ZQ::common::IPreference* pTask = root->firstChild("Task");
		while(pTask!=NULL)
		{
			TASK *task = new TASK;			
			ZQ::common::IPreference* child = pTask->firstChild("GuardDirectory");
			child->gettext(GuardDir);
			child->free();
			CString str = GuardDir;
			if(str.Right(1) != _T("\\"))
			{
				strcat(GuardDir,"\\");
			}
			mbstowcs(task->wcsGuardDir,GuardDir,sizeof(GuardDir));
			child = pTask->firstChild("TempDirectory");			
			child->gettext(TempDir);
			str = TempDir;
			if(str.Right(1) != _T("\\"))
			{
				strcat(TempDir,"\\");
			}
			mbstowcs(task->wcsTempDir,TempDir,sizeof(TempDir));
			child->free();
			child = pTask->firstChild("FtpServer");
			ZQ::common::IPreference* serverChild;
			
			while(child!=NULL)
			{
				FTPSERVER* pFtpServer = new FTPSERVER;

				serverChild = child->firstChild("Address");
				serverChild->gettext(address);
				mbstowcs(pFtpServer->wcsServer,address,sizeof(address));
				serverChild->free();

				serverChild = child->firstChild("User");
				serverChild->gettext(username);
				mbstowcs(pFtpServer->wcsUserName,username,sizeof(username));
				serverChild->free();
				
				serverChild = child->firstChild("Password");
				serverChild->gettext(password);
				mbstowcs(pFtpServer->wcsPassword,password,sizeof(password));
				serverChild->free();

				serverChild = child->firstChild("Port");
				serverChild->gettext(Port);
				pFtpServer->dwPort = atol(Port);
				serverChild->free();

				task->FtpServerList.push_back(pFtpServer);
				child = pTask->nextChild();
			}
			pTask->free();
			gTasks.push_back(task);
			pTask = root->nextChild();
		}
	}catch(ZQ::common::Exception e)
	{
		glog(Log::L_DEBUG,_T("Failed to read FtpServer node:%S"),e.getString());
		return 1;
	}
	root->free();
	delete pXmlDoc;
	return S_OK;
}
