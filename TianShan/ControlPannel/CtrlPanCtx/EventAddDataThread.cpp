// CEventAddDataThread.cpp: implementation of the EventAddDataThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EventAddDataThread.h"
#include "EventsDialog.h"
#define INITICESTORM  1

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEventsDialog * CEventAddDataThread::m_pDlg = NULL;
int CALLBACK  CEventAddDataThread::OnStreamEvent(const string & strCategory, const int &iLevel, const  string & strCurTime, const string & strMessage);

CEventAddDataThread::CEventAddDataThread()
{
	m_bExit = false;
	m_pDlg = NULL;
	memset(m_strDllName,0,sizeof(m_strDllName));
}
CEventAddDataThread::CEventAddDataThread(CEventsDialog *pEventDlg)
{
	m_bExit = false;
	m_pDlg = pEventDlg;
	memset(m_strDllName,0,sizeof(m_strDllName));
}

CEventAddDataThread::~CEventAddDataThread()
{
	m_bExit = true;
	if ( m_pDlg)
	{
		m_pDlg = NULL;
	}
}

void CEventAddDataThread::stop()
{
	m_bExit = true;
}

void CEventAddDataThread::final(void)
{
	delete(this);
}

bool CEventAddDataThread::init(void)
{
	TCHAR szTabName[XMLDATA_LEN]={0};
	
	// Get Event DSO Dll's name 
#if defined _UNICODE || defined UNICODE
	MultiByteToWideChar(
				 CP_ACP,         // code page
				 0,              // character-type options
				 gXMLFileData.strEventsDso.c_str(),  // address of string to map
				 strlen(gXMLFileData.strEventsDso.c_str()),      // number of bytes in string
				 szTabName,       // address of wide-character buffer
				 XMLDATA_LEN);             // size of buffer);

#else
	sprintf(szTabName,"%s",gXMLFileData.strEventsDso.c_str());
#endif
	if  ( _tcsstr(szTabName,_T("%") )!= NULL )
	{
			TCHAR szTemp[XMLDATA_LEN]={0};
			ExpandEnvironmentStrings(szTabName, szTemp, XMLDATA_LEN);
			memset(szTabName,0,sizeof(szTabName));
			_stprintf(szTabName,_T("%s"),szTemp);
	}
	_stprintf(m_strDllName,_T("%s"),szTabName);
/*
#ifdef INITICESTORM
	typedef void (CALLBACK *InitIceStormProc)();
	InitIceStormProc hInitIceStormProc = NULL;
	HINSTANCE hLib;
	hLib = LoadLibrary(m_strDllName);
	if ( hLib)
	{
		hInitIceStormProc = (InitIceStormProc)GetProcAddress(hLib,"InitIceStorm");
		if ( hInitIceStormProc)
		{
			(*hInitIceStormProc)();
		}
		FreeLibrary(hLib);
	}
#endif
	*/
	return true;
}

int CEventAddDataThread::run()
{
	// Get Event from dll,then add datas to UI Desktop
	typedef int (CALLBACK *GetEventProc)( const EVENTATTRISDATA & attribeData,RegEvent_Proc pFun);
	GetEventProc hEventProc = NULL;
	// modify hLib to global var
//	HINSTANCE hLib;
//	hLib = LoadLibrary(m_strDllName);
//	if ( hLib)
	if ( !m_hLib )
	{
		m_hLib = LoadLibrary(m_strDllName);
	}
	if ( m_hLib )
	{
	//	hEventProc = (GetEventProc)GetProcAddress(hLib,gXMLFileData.strEventsFeeder.c_str()); // for the test
		hEventProc = (GetEventProc)GetProcAddress(m_hLib,gXMLFileData.strEventsFeeder.c_str()); // for the test
		if ( hEventProc)
		{
			//注：因为这里使用了回调的功能，因而线程不需要循环操作。add by  dony 20070327
	//		while(true)
			{
				if ( m_bExit)
				{
					/* // modify hLib to global var
					if ( hLib)
					{
						FreeLibrary(hLib);
					}
					*/
					return 0;
				}
				int iReturn;
				EVENTATTRISDATA EventAttrisData;
				EventAttrisData.strIceStormEndPoint = gXMLFileData.strIceStormEndPoint;
				iReturn=(*hEventProc)(EventAttrisData,OnStreamEvent);
			}
		}
//		FreeLibrary(hLib); // modify hLib to global var
	}
	return 0;
}

int CALLBACK  CEventAddDataThread::OnStreamEvent(const string & strCategory, const int &iLevel, const  string & strCurTime, const string & strMessage)
{
	int iReturn = 0;
	if ( m_pDlg)
	{
		iReturn = m_pDlg->AddListCtrlData(strCategory,iLevel,strCurTime,strMessage);
	}
	return iReturn;
}