/*****************************************************************************
File Name:     Connection.h
Author:        Haiping.Wan
Security:      SEACHANGE SHANGHAI
Description:   define class CConnection
Function Inventory: 
Modification Log:
When           Version        Who						What
---------------------------------------------------------------------
2005/04/21     1.0            Haiping.Wan					Created
*******************************************************************************/

#include "StdAfx.h"
#include"Connection.h"
CConnection::CConnection(CSCCommonSender* pSender,CSCCommonReceiver* pRecerver/*,CParse *pParse*/)
{
   m_pSender=pSender;
   m_pRecerver=pRecerver;
  // m_pParse=pParse;
}

CConnection::~CConnection(void )
{  
	/*if(m_pSender)
	{
	  m_pSender->Stop();
      delete  m_pSender;
      m_pSender=NULL;
	}
	if(m_pRecerver)
	{
	  m_pRecerver->Stop();
      delete m_pRecerver;
      m_pRecerver=NULL;
	}*/
}
/*CConnection::SetConnection(CSCCommonSender* pSender,CSCCommonReceiver* pRecerver)
{
  m_pSender=pSender;
  m_pRecerver=pRecerver;
}*/
