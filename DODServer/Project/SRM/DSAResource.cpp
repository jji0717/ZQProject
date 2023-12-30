/*****************************************************************************
File Name:     DSAResource.cpp
Author:        haiping.wan
Security:      SEACHANGE SHANGHAI
Description:   implements class CDSAResource
Function Inventory: 
Modification Log:
When           Version        Who						What
------------------------------------------------------------------------------
2005/04/22     1.0            haiping.wan					Created
*******************************************************************************/
#include "StdAfx.h"
#include"DSAResource.h"
#include"ClientFactory.h"
CDSAResource::CDSAResource( CResourceManager *pOwner,char* IP,int Port):
//CDSAResource::CDSAResource(CResourceManager *pOwner)
m_pSingleConnect(this)
{
	try
	{
		//  m_pSingleConnect=new CClientFactory(this);
		m_pOwner=pOwner;
		m_pConnection=NULL;
		m_pSingleConnect.SetServer(IP,Port);
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CDSAResource::CDSAResource") );
	}
   //m_pParse=pParse;
}
CDSAResource::~CDSAResource(void)
{
	try
	{
		if(m_pConnection!=NULL)
		{
			m_pConnection->Close();
			delete m_pConnection;
		}
		//if(m_pSingleConnect!=0)
		//{
		m_pSingleConnect.Stop();
		// delete m_pSingleConnect;
		// }
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CDSAResource::~CDSAResource") );
	}
}
/*void CDSAResource::SetServer(char* IP,int Port)
{
  m_pSingleConnect->SetServer(IP,Port);
}*/

void CDSAResource::StartConnect()
{
    m_pSingleConnect.Start();
}

void CDSAResource::SetConnection(CTCPConnection* pConnection)
{
	try
	{
		if(m_pConnection!=NULL)
		{
			m_pConnection->Close();
			delete  m_pConnection;
		}
		m_pConnection=pConnection;
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CDSAResource::SetConnection(") );
	}
}

void CDSAResource::SetState(bool State)
{
	try
	{
		if(m_pConnection!=NULL)
		{
			m_pConnection->SetState(State);
		}
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CDSAResource::SetState") );
	}
}

CTCPConnection* CDSAResource::GetConnection()
{
   return m_pConnection;
}

void CDSAResource::CloseSocket()
{
	try
	{
		if(m_pConnection)
		{
			m_pConnection->Close();
			delete  m_pConnection;
			m_pConnection=NULL;
		}
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CDSAResource::CloseSocket") );
	}
 }

bool CDSAResource::GetState()
{
	try
	{
		if(m_pConnection!=NULL)
		{
			return  m_pConnection->GetState();
		}
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CDSAResource::GetState") );
	}
	return false;
}

void CDSAResource::RestartConnect()
{
	try
	{
		this->SetState(false);
		this->CloseSocket();
		m_pSingleConnect.Restart();
	}
	catch(...)
	{
		Clog( LOG_DEBUG, _T("catch Error in CDSAResource::RestartConnec") );
	}
}

