// ConnChecker.cpp: implementation of the ConnChecker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ConnChecker.h"
#include "DSInterface.h"
#include "LocalDB.h"
#include "DBSyncServ.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define DEF_IDS_MAX_INIT_TIMES          10

using ZQ::common::Log;
extern CDSInterface g_ds;
extern LocalDB g_localDB;	//	Global local DB operation object
extern ZQ::common::Log* gpDbSyncLog;
extern DBSyncServ g_server;


ConnChecker::ConnChecker(DWORD TimeOut):ZQ::common::NativeThread(),
m_bQuit(false),
m_dwTimeOut(TimeOut)
{
	m_EventStop = ::CreateEvent(NULL,false,false,NULL);
	m_hNofity = ::CreateEvent(NULL,false,false,NULL);

	m_isBroken = false;
}

ConnChecker::~ConnChecker()
{
	if(isRunning())
	{
		StopChecker();
		waitHandle(100);
	}
	
	::CloseHandle(m_EventStop);
}
int ConnChecker::run(void)
{
	HANDLE handles[2] = { m_EventStop, m_hNofity};

	while(!m_bQuit)
	{
		DWORD rtn = ::WaitForMultipleObjects(2, handles, FALSE, m_dwTimeOut);
		if(m_bQuit)
			return 0;
		switch(rtn) {
		case WAIT_OBJECT_0:
			m_bQuit = true;
			break;
		case WAIT_OBJECT_0+1:
		case WAIT_TIMEOUT:
			Check();
			break;
		default:
			break;
		}
	}	
	return 0;
}

void ConnChecker::TriggerCheck()
{
	if(m_hNofity != NULL)
	{
		SetEvent(m_hNofity);
	}
}

void ConnChecker::Check(void)
{
	if(g_localDB.m_bIsRetrying)
		return;
	
	(*gpDbSyncLog)(Log::L_INFO,_T("Connection Checker start to check the connection"));

	//
	// do the check
	//
	RTNDEF rtn = g_ds.CheckConn();
	
	if(ITV_SUCCESS == rtn)
	{
		(*gpDbSyncLog)(Log::L_INFO,_T("The IDS Connection is ok"));	
		return;
	}
	
	//
	// connection is lost, re-initialize IDS context
	//
	(*gpDbSyncLog)(Log::L_WARNING,_T("Connection is lost.try to reestablish the connection"));	

	m_isBroken = true;

	g_ds.Uninitialize();

	bool bRet = g_ds.Initialize();
	if(bRet)
	{
		(*gpDbSyncLog)(Log::L_NOTICE,_T("IDS Connection is re-create successfully"));	

		m_isBroken = false;

		//
		// initial a full sync 
		//
		g_server.InitialFullSync();
	}
}

void ConnChecker::StopChecker(void)
{
	m_bQuit = true;
	::SetEvent(m_EventStop);
}

bool ConnChecker::isBroken()
{	
	return m_isBroken;
}