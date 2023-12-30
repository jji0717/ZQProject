/*****************************************************************************
File Name:     SessionManager.cpp
Author:        haiping.wan
Security:      SEACHANGE SHANGHAI
Description:   implements class CSessionManager
Function Inventory: 
Modification Log:
When           Version        Who						What
------------------------------------------------------------------------------
2005/04/22     1.0            haiping.wan					Created
*******************************************************************************/

#include "StdAfx.h"
#include"SessionManager.h"
CSessionManager::CSessionManager(CSRManager* pOwner)
{
   m_pOwner=pOwner;
   m_pResourceManager=NULL;
   InitializeCriticalSection(&m_sessionLock);
   InitializeCriticalSection(&m_timeLock);
   m_hEventCheck=::CreateEvent(NULL,false,false,NULL);

}
void CSessionManager::Destroy(void)
{

    Close();
	Lock();
    while(m_SessionList.size())
   {
	   std::list<CDODSession>::iterator it = m_SessionList.begin();
	   m_SessionList.erase(it);
   }
   UnLock();

   DeleteCriticalSection(&m_sessionLock);
   DeleteCriticalSection(&m_timeLock);
}

CSessionManager::~CSessionManager(void)
{
  /* Close();
   DeleteCriticalSection(&m_sessionLock);
   DeleteCriticalSection(&m_timeLock);*/

}
void CSessionManager::SetResourceManger(CResourceManager*  pManager)
{
   m_pResourceManager=pManager;
}
CTCPConnection* CSessionManager::GetConnection(int SesionID)
{
	//std::list<Session>::iterator it;
	try
	{
	CTCPConnection* Connection;
    Lock();
	std::list<CDODSession>::iterator it;
	for(it=m_SessionList.begin();it!=m_SessionList.end();it++)
	{
		if(SesionID==it->GetSessionID())
		{
            Connection = it->GetClientConnction();
            this->UnLock();
			return Connection;
		}
	}
	this->UnLock();
	
    return NULL;
	}
 catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in  CSessionManager::GetConnection") );

   }
   return NULL;
}
CDSAResource* CSessionManager::GetDSResource(int SesionID)

{
	try
	{
    //std::list<Session>::iterator it;
	CDSAResource* Resource;
	this->Lock();
    std::list<CDODSession>::iterator it;
	for(it=m_SessionList.begin();it!=m_SessionList.end();it++)
	{
		if(SesionID==it->GetSessionID())
		{
            Resource=it->GetDSAConnection();
            this->UnLock();
			return Resource;
		}
	}
   this->UnLock();
   return NULL;
	}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in  CSessionManager::GetDSResource") );

   }
   return NULL;

}
bool CSessionManager::DeleteSession(int SesionID)
{
	try
	{
    //std::list<Session>::iterator it;
	 this->Lock();
     std::list<CDODSession>::iterator it;
	 for(it=m_SessionList.begin();it!=m_SessionList.end();it++)
	 {
		if(SesionID==it->GetSessionID())
		{
		   m_SessionList.erase(it);
           this->UnLock();
		   return true;;
		}
	 }
	 this->UnLock();
   }
   catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in  CSessionManager::DeleteSession") );

   }
   return false;
}


bool CSessionManager::DeleteSession(CDSAResource* pDSAConnection)
{
try
{    
	 if(pDSAConnection==NULL)
		 return false;
     this->Lock();
     std::list<CDODSession>::iterator it;
     int size=m_SessionList.size();
	 BOOL Flag=TRUE;;
     for(int i=0;i<size;i++)
	  {
		if(Flag)
		{
             Flag=FALSE;
             for(it=m_SessionList.begin();it!=m_SessionList.end();it++)
	         {
				if(it->GetDSAConnection()==pDSAConnection)
		        {
		            m_SessionList.erase(it);
                    Flag=TRUE;
				    break;
		        }
	          }
          }
		  else
		  {
		     break;
		  }
	   }
 this->UnLock();
}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in   CSessionManager::DeleteSession(CDSAResource* pDSAConnection)") );

   }
 return true;
}


void CSessionManager::Lock()
{
    EnterCriticalSection(&m_sessionLock);
}
void CSessionManager::LockTime()
{
	EnterCriticalSection(&m_timeLock);
}
void CSessionManager::UnLock()
{
    LeaveCriticalSection(&m_sessionLock);
}
void CSessionManager::UnlockTime()
{
    LeaveCriticalSection(&m_timeLock);
}
DWORD CSessionManager::GenerateSessionID()
{  
	try
	{
    DWORD ID;
	LockTime();
   	SYSTEMTIME time;
	GetLocalTime(&time);
	int day=time.wDay;
	int hour=time.wHour;
	int minute=time.wMinute;
	int second=time.wSecond;
	int  milliseconds=time.wMilliseconds;
    ID=(milliseconds-1)/100+second*10+minute*1000+hour*100000+day*10000000;
	Sleep(120);
    UnlockTime();
    return ID;
	}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CSessionManager::GenerateSessionID") );

   }
   return -1;

 }
void CSessionManager::AddASession(CDODSession Session)
{
	try
	{
   this->Lock();
   m_SessionList.push_back(Session);
   this->UnLock();
	}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CSessionManager::AddASession") );

   }

}
void CSessionManager::StartCheck()
{
	//m_hThreadCheck=CreateThread(NULL,0,EntryCheck,this,0,NULL);
}
DWORD WINAPI CSessionManager::EntryCheck(LPVOID pParam)
{
   ((CSessionManager*)pParam)->ThreadProc();
   return 1;
}
void CSessionManager::ThreadProc()
{
	try
	{
   for(;;)
   {
	   if(WaitForSingleObject(m_hEventCheck,1000*60*10)!=WAIT_OBJECT_0)
	   {
              DWORD CurrentTime;
              SYSTEMTIME time;
	          GetLocalTime(&time);
	          int day=time.wDay;
	          int hour=time.wHour;
	          int minute=time.wMinute;
	          int second=time.wSecond;
	          int  milliseconds=time.wMilliseconds;
              CurrentTime=(milliseconds-1)/100+second*10+minute*1000+hour*100000+day*10000000;
			  this->Lock();
              std::list<CDODSession>::iterator it;
              ///////////////////////////////////////////////////
			  int size=m_SessionList.size();
			  BOOL Flag=TRUE;;
              for(int i=0;i<size;i++)
			  {
				  if(Flag)
				  {
                    Flag=FALSE;
                    for(it=m_SessionList.begin();it!=m_SessionList.end();it++)
	                {
		              if(CurrentTime-it->GetSessionID()>=100000*2)
		              {
		                m_SessionList.erase(it);
                        Flag=TRUE;
						break;
		              }
	                }
                  }
				  else
				  {
					  break;
				  }
			   }
			  //////////////////////////////////////////////////
            /*  for(it=m_SessionList.begin();it!=m_SessionList.end();it++)
	          {
		         if(CurrentTime-it->GetSessionID()>=100000*2)
		         {
		            m_SessionList.erase(it);
		         }
	          }*/
	        this->UnLock();
	   }
	   else
	   {
		   break;
	   }
   }
	}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in  CSessionManager::ThreadProc") );

   }

}
void CSessionManager::Close()
{
	try
	{
	SetEvent(m_hEventCheck);
	WaitForSingleObject(m_hThreadCheck,INFINITE);
	}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in  CSessionManager::Close") );

   }

}

