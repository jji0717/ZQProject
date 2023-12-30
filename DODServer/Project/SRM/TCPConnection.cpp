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
#include"TCPConnection.h"

//CConnection* CConnection::connection=NULL;
CTCPConnection::CTCPConnection(CSCCommonSender* pSender,CSCCommonReceiver* pRecerver/*,CParse *pParse*/)
{
   m_pSender=pSender;
   m_pRecerver=pRecerver;
   m_IsConnect=true;
   InitializeCriticalSection(&m_lock);
   IPFrom=0;
   // connection=this;
   //m_ConectNotify=Notify;
   // m_pParse=pParse;
}
CTCPConnection::~CTCPConnection(void )
{  
	::DeleteCriticalSection(&m_lock);
}
/*CConnection::SetConnection(CSCCommonSender* pSender,CSCCommonReceiver* pRecerver)
{
  m_pSender=pSender;
  m_pRecerver=pRecerver;
}*/
void CTCPConnection::SetState(bool state)
{
   Lock();
   m_IsConnect=state;
   UnLock();
  
}
bool CTCPConnection::GetState()
{   
	
    bool  IsConnect;
	this->Lock();
	IsConnect=m_IsConnect;
	this->UnLock();
    return IsConnect;
	   

}
void CTCPConnection::Start()
{   
	try
	{
	Lock();
    if(m_pSender)
    {
	  m_pSender->Start();
	}
	if(m_pRecerver)
	{
	  m_pRecerver->Start();
	}
	UnLock();
	}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in  CTCPConnection::Start") );

   }

}
void CTCPConnection::SendData(char* data,int length)

{  
	try
	{
	this->Lock();
   // Clog(LOG_DEBUG,"begin connection send data");
    char * pcData = CSCMemoryBlock::AllocBlock(length+1);	
	memcpy( pcData, data, length );
	pcData[length] = MSG_END_FLAG;
	CSCMemoryBlock * mBlock = new CSCMemoryBlock( pcData, length+1 );
	CSCMemoryBlockPtr pFred( mBlock );	
    if( m_IsConnect)
	{
	 if(m_pSender)
	 { 
		//Clog(LOG_DEBUG,"push the data to send buffer begin");
		m_pSender->Push(pFred);
       // Clog(LOG_DEBUG,"push the data to send buffer end");
        //printf("error end\n");
	 }
	}
	this->UnLock();
	}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in  CTCPConnection::SendData") );

   }

}
void CTCPConnection::Close()
{
	try
	{
  //Clog( LOG_DEBUG, "CTCPConnection::Close break1" );
  this->Lock();
 // Clog( LOG_DEBUG, "CTCPConnection::Close break2" );
  if(m_pSender)
  {
      //Clog( LOG_DEBUG, "CTCPConnection::Close break3" );
	  m_pSender->Stop();
	  delete  m_pSender;
      m_pSender=NULL;
     // Clog( LOG_DEBUG, "CTCPConnection::Close break4" );
  }
  if(m_pRecerver)
  {    
	 CSCTCPSocket* pSocket;
	 pSocket=m_pRecerver->GetSocket();
     pSocket->Close();
     if( pSocket )
	 {
	    delete pSocket;
	    pSocket = NULL;
   	  }
      //m_pRecerver->Stop();
	  //Sleep(500);
	  delete  m_pRecerver;
      m_pRecerver=NULL;
  }
  this->UnLock();
	}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in  CTCPConnection::Close") );

   }
}
/*void CConnection::Notify()
{
  //connection->SetState(false);
  //connection->Close();
}*/



