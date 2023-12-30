/*****************************************************************************
File Name:     ResourceManager.cpp
Author:        haiping.wan
Security:      SEACHANGE SHANGHAI
Description:   implements class CResourceManager
Function Inventory: 
Modification Log:
When           Version        Who						What
------------------------------------------------------------------------------
2005/04/22     1.0            haiping.wan					Created
*******************************************************************************/
#include "StdAfx.h"
#include"ResourceManager.h"
#include"Markup.h"
CResourceManager::CResourceManager(CSRManager* pOwner)
{
	InitializeCriticalSection(&m_ResourceLock);
	m_hEvent=::CreateEvent(NULL,false,false,NULL);
	m_pOwner=pOwner;
	m_CurResource=NULL;
	m_PreResource=NULL;
}
CResourceManager::~CResourceManager(void)
{
	/*CloseThread();
	while(m_DSAResourcePtrList.size())
	{
	std::list< CDSAResource* >::iterator it =m_DSAResourcePtrList.begin();
	delete (*it);
	m_DSAResourcePtrList.erase(it);
	}
	DeleteCriticalSection(&m_ResourceLock);*/

}
void CResourceManager::Destroy(void)
{
	CloseThread();
	while(m_DSAResourcePtrList.size())
	{
		std::list< CDSAResource* >::iterator it =m_DSAResourcePtrList.begin();
		delete (*it);
		m_DSAResourcePtrList.erase(it);
	}
	DeleteCriticalSection(&m_ResourceLock);
}

void CResourceManager::AddOneDSA(char* IPAddress,int port)
{
	try
	{
		strDSA  DSA;
		strcpy(DSA.IPAddress,IPAddress);
		DSA.IPPort=port;
		m_DSAAddrList.push_back(DSA);
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CResourceManager::AddOneDSA") );
	}
}
void CResourceManager::Create()
{
	try
	{
		std::list<strDSA>::iterator it;
		for(it=m_DSAAddrList.begin();it!=m_DSAAddrList.end();it++)
		{
			CDSAResource* pDSAResource=new CDSAResource(this,it->IPAddress,it->IPPort);
			//pDSAResource->SetServer(it->IPAddress,it->IPPort);
			pDSAResource->StartConnect();
			this->m_DSAResourcePtrList.push_back(pDSAResource);
		}
		std::list< CDSAResource* >::iterator Resourceit;
		if(m_DSAResourcePtrList.size()>0)
		{
			Resourceit=m_DSAResourcePtrList.begin();
			m_CurResource=*Resourceit;
		}
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CResourceManager::Create") );
	}
}

CDSAResource* CResourceManager::SelectAResource(void)
{
	try
	{
		std::list< CDSAResource* >::iterator it;
		CDSAResource* returnDSAResource;
		int listsize;
		int count=0;
		this->Lock();
		listsize=m_DSAResourcePtrList.size();
		if(m_PreResource==NULL)
		{
			for(it=m_DSAResourcePtrList.begin();it!=m_DSAResourcePtrList.end();it++)
			{
				returnDSAResource=*it;
				if(returnDSAResource->GetState())
				{
					m_PreResource=returnDSAResource;
					this->UnLock();
					return returnDSAResource;
				}
			}
			this->UnLock();
			return NULL;
		}
		///////////////////////////////////////
		else
		{
			for(it=m_DSAResourcePtrList.begin();it!=m_DSAResourcePtrList.end();it++)
			{  
				count=count+1;
				CDSAResource*  TempResource;
				TempResource=*it;
				if(m_PreResource==TempResource)
				{
					for(int i=count; i<listsize;i++)
					{
						it++;
						returnDSAResource=*(it);
						if(returnDSAResource->GetState())
						{
							m_PreResource=returnDSAResource;
							this->UnLock();
							return   returnDSAResource;
						}
					}
					it=m_DSAResourcePtrList.begin();
					for(int j=0;j<count; j++)
					{
						returnDSAResource=*(it);
						if(returnDSAResource->GetState())
						{
							m_PreResource=returnDSAResource;
							this->UnLock();
							return   returnDSAResource;
						}
						it++;
					}
					break;
				}
			}
			this->UnLock();
			return NULL;
		}
		/*this->Lock();
		std::list< CDSAResource* >::iterator it;
		CDSAResource* returnDSAResource;
		returnDSAResource=m_CurResource;
		int size=m_DSAResourcePtrList.size();
		int count=0;
		if(m_DSAResourcePtrList.size()<=0)
		{
		return NULL;
		}
		else
		{  
		for(it=m_DSAResourcePtrList.begin();it!=m_DSAResourcePtrList.end();it++)
		{  
		count=count+1;
		CDSAResource*  TempResource;
		TempResource=*it;
		if(m_CurResource==TempResource)
		{
		if(count==size)
		{
		m_CurResource= *(m_DSAResourcePtrList.begin());
		}
		else
		{
		it++;
		m_CurResource=*(it);
		}
		}
		}
		}
		this->UnLock();
		/*std::list< CDSAResource* >::iterator it;
		CDSAResource* returnDSAResource;
		it=m_DSAResourcePtrList.begin();
		returnDSAResource=*it;
		return returnDSAResource;*/
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CResourceManager::SelectAResource") );
	}
	return NULL;

}
void CResourceManager::notify(CSCTCPSocket* pSocket)
{
	try
	{
		CDSAResource * pResource=NULL;
		this->Lock();
		std::list< CDSAResource* >::iterator it;
		for(it=m_DSAResourcePtrList.begin();it!=m_DSAResourcePtrList.end();it++)
		{
			if((*it)->GetConnection()!=NULL)
			{
				if((*it)->GetConnection()->GetReceive()!=NULL)
				{
					if((*it)->GetConnection()->GetReceive()->GetSocket()==pSocket)
					{
						pResource=*it;
						(*it)->RestartConnect();
					}
				}
			}
		}
		this->UnLock();
		//added by whp 6.7
		if(pResource!=NULL)
		{
			if(m_pSessionManager!=NULL)
			{
				m_pSessionManager->DeleteSession(pResource);
			}
		}
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CResourceManager::notify") );
	}
}
DWORD  CResourceManager::ThreadEntry(LPVOID pParam)
{
	((CResourceManager*)pParam)->ThreadProc();
	return 1;
}

void CResourceManager::SendHeartBeat()
{
	m_hThread=CreateThread(NULL,0,ThreadEntry,this,0,NULL);
}

void CResourceManager::ThreadProc()
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
			 std::list< CDSAResource* >::iterator it;
			 for(it=m_DSAResourcePtrList.begin();it!=m_DSAResourcePtrList.end();it++)
			 {
				 if((*it)->GetConnection()!=NULL)
				 {
					 (*it)->GetConnection()->SendData(pcData,length);
				 }
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
		Clog( LOG_DEBUG, _T("catch Error in CResourceManager::ThreadProc") );
	}
}
void CResourceManager::CloseThread()
{
	try
	{
		SetEvent(m_hEvent);
		WaitForSingleObject(m_hThread,INFINITE);
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CResourceManager::CloseThread") );
	}
}

void CResourceManager::SetSessionManager( CSessionManager* pSessionManager)
{
	m_pSessionManager=pSessionManager;
}
