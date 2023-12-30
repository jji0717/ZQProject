/*****************************************************************************
File Name:     ClientManager.cpp
Author:        haiping.wan
Security:      SEACHANGE SHANGHAIpConnection
Description:   implements class CClientManager
Function Inventory: 
Modification Log:
When           Version        Who						What
------------------------------------------------------------------------------
2005/04/22     1.0            haiping.wan					Created
*******************************************************************************/

#include "StdAfx.h"
#include"ClientManager.h"
#include"ListenManager.h"
#include"Markup.h"

CClientManager::CClientManager(CSRManager* pOwner):
m_pListenManager(this)
{
   m_pOwner=pOwner;
   //m_pListenManager=new CListenManager(this);
   m_hEvent=::CreateEvent(NULL,false,false,NULL);
   ::InitializeCriticalSection(&m_lock);
}

void CClientManager::Destroy(void)
{
	try
	{
		printf("CClientManager 1");
		this->CloseThread();
		printf("CClientManager 2");
		m_pListenManager.Stop();
		printf("CClientManager 3");

		while(m_pConnectionList.size())
		{
			std::list< CTCPConnection* >::iterator it = m_pConnectionList.begin();
			//close socket detete the recveive and send thread
			(*it)->Close();
			delete (*it);
			m_pConnectionList.erase(it);
		}
		//delete m_pListenManager;
		::DeleteCriticalSection(&m_lock);
		if(m_hEvent)
		{
			CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CClientManager:: ~CClientManager") );
	}
}

CClientManager:: ~CClientManager(void)
{
	/*try
	{
	this->CloseThread();
   m_pListenManager->Stop();
   delete m_pListenManager;
   while(m_pConnectionList.size())
   {
	   std::list< CTCPConnection* >::iterator it = m_pConnectionList.begin();
	   //close socket detete the recveive and send thread
       (*it)->Close();
       delete (*it);
	   m_pConnectionList.erase(it);
   }
   ::DeleteCriticalSection(&m_lock);
    if(m_hEvent)
    {
	    CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}
	}
 catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CClientManager:: ~CClientManager") );
   }*/

 }
void CClientManager::SetHost(char*IP,int Port)
{
	m_pListenManager.SetHost(IP,Port);
}
void CClientManager::StartListen()
{
    m_pListenManager.Start();
}
void CClientManager::SetState(CSCTCPSocket* pSocket,bool state)
{
	try
	{
		this->Lock();
		std::list< CTCPConnection* >::iterator it;
		for(it=m_pConnectionList.begin();it!=m_pConnectionList.end();it++)
		{
			if((*it)->GetReceive())
			{
				if((*it)->GetReceive()->GetSocket()==pSocket)
				{
					(*it)->SetState(state);
				}
			}
		}
		this->UnLock();
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CClientManager::SetState") );
	}

}
void CClientManager::CloseConnection(CSCTCPSocket* pSocket)
{
	try
	{
		//Clog( LOG_DEBUG, "CloseConnection break1" );
		this->Lock();
		//Clog( LOG_DEBUG, "CloseConnection break2" );
		std::list< CTCPConnection* >::iterator it;
		for(it=m_pConnectionList.begin();it!=m_pConnectionList.end();it++)
		{
			if((*it)->GetReceive())
			{
				if((*it)->GetReceive()->GetSocket()==pSocket)
				{
					// Clog( LOG_DEBUG, "CloseConnection break3" );
					(*it)->Close();
					//Clog( LOG_DEBUG, "CloseConnection break4" );
				}
			}
		}
		this->UnLock();
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CClientManager::CloseConnection") );
	}
}
void CClientManager::AddConnection(CTCPConnection* pConnection)
{ 
	try
	{
		// Clog( LOG_DEBUG, "AddConnection break1" );
		this->Lock();
		// Clog( LOG_DEBUG, "AddConnection break2" );
		std::list<CTCPConnection*>::iterator it;
		for(it=m_pConnectionList.begin();it!=m_pConnectionList.end();it++)
		{
			if((*it)->GetIP()==pConnection->GetIP())
			{
				// Clog( LOG_DEBUG, "AddConnection break3" );
				if(!(*it)->GetState())
				{
					//Clog( LOG_DEBUG, "AddConnection break4" );
					(*it)->SetIP(pConnection->GetIP());
					(*it)->SetReceive(pConnection->GetReceive());
					(*it)->SetSender(pConnection->GetSender());
					(*it)->SetState(pConnection->GetState());
					CClientParse* tempParse;
					tempParse=reinterpret_cast<CClientParse*>((*it)->GetReceive()->GetParse());
					tempParse->SetConnection(*it);
					//tempParse=dynamic_cast< CClientParse*>((*it)->GetReceive()->GetParse());
					//tempParse->SetConnection(*it);
					delete pConnection;
					(*it)->Start();
					this->UnLock();
					return ;
				}
			}
		}
		m_pConnectionList.push_back(pConnection);
		pConnection->Start();
		this->UnLock();
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CClientManager::AddConnection") );
	}
}

void CClientManager::Lock()
{
	EnterCriticalSection(&m_lock);
}
void CClientManager::UnLock()
{
	LeaveCriticalSection(&m_lock);
}
void CClientManager::SendHeartBeat()
{
	m_hThread=CreateThread(NULL,0,ThreadEntry,this,0,NULL);
}
DWORD  CClientManager::ThreadEntry(LPVOID pParam)
{
  ((CClientManager*)pParam)->ThreadProc();
   return 1;
}

void CClientManager::ThreadProc()
{ 
	try
	{
		CMarkup tmpXmlDOM;
		tmpXmlDOM.AddElem( _T("Message") );
		tmpXmlDOM.IntoElem();
		tmpXmlDOM.AddElem( _T("MessageHeader") );
		tmpXmlDOM.AddAttrib( _T("MessageCode"), _T("0") );  
		tmpXmlDOM.AddAttrib( _T("SessionID"),"0" );  
		tmpXmlDOM.AddAttrib( _T("MessageTime"), "0" ); 
		tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 
		tmpXmlDOM.AddAttrib( _T("CommandID"),"0" );
		CString sTmp = tmpXmlDOM.GetDoc();
		int length = sTmp.GetLength();
		//char * pcData = CSCMemoryBlock::AllocBlock(length+1);	
		char pcData[1000];
		strncpy( pcData, sTmp.GetBuffer(0), length );
		//pcData[length] = MSG_END_FLAG;
		/* if(Send(pcData,length+1)>0)
		{
		printf("successfully send\n");
		}*/
		for(;;)
		{
			if(WaitForSingleObject(m_hEvent,1000*5)!=WAIT_OBJECT_0)
		 {
			 Lock();
			 std::list<CTCPConnection*>::iterator it;
			 for(it=m_pConnectionList.begin();it!=m_pConnectionList.end();it++)
			 {
				 (*it)->SendData(pcData,length);
			 }
			 UnLock();           
		 }
			else
		 {
			 break;
		 }
		}
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CClientManager::ThreadProc") );
	}
}

void CClientManager::CloseThread()
{
	try
	{
		SetEvent(m_hEvent);
		WaitForSingleObject(m_hThread,INFINITE);
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CClientManager::CloseThread") );
	}
}
