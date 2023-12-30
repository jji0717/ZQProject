
/*
**	FILENAME			SingleConnect.cpp
**						
**	PURPOSE				The file is a implement file, used along with SingleConnect.h.
**						a class CSingleConnect is defined in the file, and is used to 
**						manager auto-connect process to server.
**						
**						
**	CREATION DATE		22-11-2004
**	LAST MODIFICATION	22--2004
**
**	AUTHOR				Leon.li (ZQ Interactive)
**
**
*/
#include "StdAfx.h"
#include "ClientFactory.h"
#include"SRManager.h"
CClientFactory* CClientFactory::s_ClientFactory = NULL;
//start thread
VOID CClientFactory::Start()
{
	try
	{
	Clog( LOG_DEBUG, _T("Enter Connect Thread Start.") );
	//m_iStatus = 0;
	if( m_bThreadExit )
	{
		ResetEvent(m_hEvent);
		ResetEvent(m_hShutdownEvent);
		m_dwThreadID = 104;
		m_Thread = CreateThread(NULL,0, ConnectThread,(LPVOID)this,0,&m_dwThreadID);
		Clog( LOG_DEBUG, _T("Create Connect Thread.") );
	}
	}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CClientFactory::Start()") );
   }

}
//stop thread
VOID CClientFactory::Stop()

{
	try
	{
	int iRtn = SCS_SUCCESS, iCount=0;
	Clog( LOG_DEBUG, _T("Enter Stop Connect Thread.") );
	//EndHeartBeatThread();
	if( !m_bThreadExit )
		SetEvent(m_hEvent);
        WaitForSingleObject( m_hShutdownEvent, INFINITE );

		/*if( WaitForSingleObject( m_hShutdownEvent, 4000 ) != WAIT_OBJECT_0 )
		{			
			if( !m_bThreadExit )
			{
				Clog( LOG_DEBUG, _T("Connect Thread Exit Call TerminateThread.") );
				TerminateThread( m_Thread, 0x12 );
			}
		}
	else
	Clog( LOG_DEBUG, _T("Connect Thread Exit Normally.") );*/
	Clog( LOG_DEBUG, "Close Socket." );
	//m_iStatus = 3;
	//m_Socket.Close();
	m_bConnected = false;
	Clog( LOG_DEBUG, "Socket Closed." );
	}
 catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CClientFactory::Stop()") );
   }


}
// when connection is lost, notify it
void CClientFactory::Notify(INT32 status,CSCTCPSocket* pSocket)
{
	try
	{
		//added by whp 6.4
    if(s_ClientFactory!=NULL)
	{
	   s_ClientFactory->m_pOwner->m_pOwner->notify(pSocket);
	}
   	
	CString logString="DisConnect from Server: ";
    Clog( LOG_DEBUG,logString );
	}


	/*s_SingleConnect->m_pOwner->SetState(false);
	s_SingleConnect->m_pOwner->CloseSocket();
    if( s_SingleConnect->m_bConnected )
	{
       s_SingleConnect->Start();
       s_SingleConnect->m_bConnected=false;
	}*/
    catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CClientFactory::Notify()") );
   }

   
}
//connection thread body
DWORD WINAPI CClientFactory::ConnectThread(LPVOID pParam)
{
	try
	{
	Clog( LOG_DEBUG, "Start ConnectThread." );
	CClientFactory* app = (CClientFactory*)pParam;
	HANDLE event = app->m_hEvent;
	CDSAResource* pOwner=app->m_pOwner;
    CSCTCPSocket* pSocket=new CSCTCPSocket();
    
	//app->m_Socket.Close();
	if(pSocket->Create() != SCS_SUCCESS)
	{
		Clog( LOG_DEBUG, "Create Socket Fail, ConnectThread Exit." );
		app->m_bThreadExit = true;
		SetEvent( app->m_hShutdownEvent );
		return 0x12;
	}
	app->m_bThreadExit = false;	
/////////////////////////////////////////////////////////////////////////////////////////////////
    DWORD  recvtimeout=1000*10;
    //if(setsockopt(pSocket->GetSocket(),SOL_SOCKET,SO_RCVTIMEO,(char*)&recvtimeout,sizeof(recvtimeout))!=0)
	//return 0;
    while(WaitForSingleObject(event, 1000 )!=WAIT_OBJECT_0)
	{	
            unsigned long cmd;
            cmd=1;
		    ioctlsocket(pSocket->GetSocket(),FIONBIO,&cmd);
		     
		    if(pSocket->Connect(app->m_IP, app->m_Port ) == SCS_SUCCESS)
	        {
               app->m_bConnected = TRUE;
			   CString strPort;//("%d",app->m_Port);
               //strPort.Format("%d",app->m_Port);

               char strBuf[MAX_PATH];
			   sprintf(strBuf,"%.0f",(FLOAT)app->m_Port);
               strPort=strBuf;

			   CString logString;
			   logString="Connect to IP: ";
               logString=logString+app->m_IP;
			   logString=logString+"Port:";
               logString=logString+strPort;
               logString=logString+"successfully";

               unsigned long cmd;
               cmd=0;
			   ioctlsocket(pSocket->GetSocket(),FIONBIO,&cmd);
 
			   Clog( LOG_DEBUG,logString );
			   break;
		    }
/////////////////////////////////////////////////////////////////
            else
			{
               FD_SET writeset;
		       FD_ZERO(&writeset);
			   FD_SET(pSocket->GetSocket(),&writeset);
		       TIMEVAL  time;
		       time.tv_sec=0;
		       time.tv_usec=100;
			   int ret=select(1,NULL,&writeset,NULL,&time);
		       if(ret>0)
		       {
                    app->m_bConnected = TRUE;
			        CString strPort;//("%d",app->m_Port);
					char strBuf[MAX_PATH];
					sprintf(strBuf,"%.0f",(FLOAT)app->m_Port);
                 	strPort=strBuf;
			        CString logString;
			        logString="Connect to IP: ";
                    logString=logString+app->m_IP;
			        logString=logString+"Port:";
                    logString=logString+strPort;
                    logString=logString+"successfully";
                    Clog( LOG_DEBUG,logString );

                    unsigned long cmd;
                    cmd=0;
			        ioctlsocket(pSocket->GetSocket(),FIONBIO,&cmd);
 			        break;
                                  
		       }
			   else
			   {
			        Sleep(1000*10);
                    CString strPort;//("%d",app->m_Port);

                    char strBuf[MAX_PATH];
					sprintf(strBuf,"%.0f",(FLOAT)app->m_Port);
                 	strPort=strBuf;

					
			        CString logString;
			        logString="Connect to IP: ";
                    logString=logString+app->m_IP;
			        logString=logString+" Port:";
                    logString=logString+strPort;
                    logString=logString+" Failed";
                    Clog( LOG_DEBUG,logString );

					if(ret=SOCKET_ERROR)
					{
						pSocket->Close();
						if(pSocket->Create() != SCS_SUCCESS)
	                    {
		                  Clog( LOG_DEBUG, "Create Socket Fail, ConnectThread Exit." );
		                  app->m_bThreadExit = true;
		                  SetEvent( app->m_hShutdownEvent );
		                  return 0x12;
	                    }
					}
			   }
	        }
/////////////////////////////////////////////////////////////
			/*else
			{
               FD_SET writeset;
		       FD_ZERO(&writeset);
			   FD_SET(pSocket->GetSocket(),&writeset);
		       TIMEVAL  time;
		       time.tv_sec=0;
		       time.tv_usec=100;
		       if(select(1,NULL,&writeset,NULL,&time)!=0)
		       {
                    app->m_bConnected = TRUE;
			        CString strPort("%d",app->m_Port);
			        CString logString;
			        logString="Connect to IP: ";
                    logString=logString+app->m_IP;
			        logString=logString+"Port:";
                    logString=logString+strPort;
                    logString=logString+"successfully";

                     unsigned long cmd;
                     cmd=0;
			         ioctlsocket(pSocket->GetSocket(),FIONBIO,&cmd);

 
			        Clog( LOG_DEBUG,logString );
			        break;
                                  
		       }
			}*/
/////////////////////////////////////////////////////
	}

	if(!app->m_bConnected)
	{
		Clog( LOG_DEBUG, _T("Connect Fail. Close Socket.") );
		pSocket->Close();
		app->m_bThreadExit = true;
		SetEvent( app->m_hShutdownEvent );
		return 0x12;
	}
	Clog( LOG_DEBUG, _T("Connect Thread Exit.") );
	CTCPConnection* pConnection=NULL;
	CDSAParse* pParse;
	pParse=new CDSAParse(pOwner);
	pParse->SetSessionManager(pOwner->m_pOwner->m_pOwner->m_Sessionmanager);
	//CSCCommonReceiver* pCSCCommonReceiver = new CSCCommonReceiver(pSocket/*&app->m_Socket*/, /*app->m_pServerParser*/pParse, app->Notify);
	//CSCCommonSender*  pCSCCommonSender = new CSCCommonSender(pSocket/*&app->m_Socket*/, NULL, app->Notify );//NULL);
     CSCCommonReceiver* pCSCCommonReceiver = new CSCCommonReceiver(pSocket/*&app->m_Socket*/, /*app->m_pServerParser*/pParse, CClientFactory::Notify);
	 CSCCommonSender*  pCSCCommonSender = new CSCCommonSender(pSocket/*&app->m_Socket*/, NULL, CClientFactory::Notify );//NULL);
    pConnection=new CTCPConnection(pCSCCommonSender,pCSCCommonReceiver);
	pOwner->SetConnection(pConnection);
    pConnection->Start();
    app->m_bThreadExit = true;
	SetEvent( app->m_hShutdownEvent );
	}
   catch(...)
   {
       Clog( LOG_DEBUG, _T("catch Error in CClientFactory::ConnectThread") );
   }
   //app->m_bThreadExit = true;
	return 0x12;
}

void CClientFactory::SetServer(char* IP,int Port)
{
	try
	{
       m_IP=IP;
       m_Port=Port;
	}
   catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CClientFactory::SetServer") );
   }
}
void CClientFactory::Restart()
{
    m_bConnected=false;
    //m_bThreadExit=true;
	this->Start();
}

