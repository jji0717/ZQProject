// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Client.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#include<winsock2.h>
#include"Markup.h"
#include"scqueue.h"
#include"MessageMacro.h"
#define MSG_END_FLAG	127

SOCKET sock;
CWinApp theApp;
INT32 Send(CHAR* pBuffer, int iLength) 
{
	if(sock == INVALID_SOCKET)
		return -1;
    INT32 result = 0;
    INT32 left = iLength;
	iLength = 0;
    while(left)
    {
        result = send(sock, pBuffer+iLength, left, 0);
        if(SOCKET_ERROR == result)
        {
            return -2;
        }
        left -= result;
		iLength += result;
    }
    return 1;
}
CString GetCurrDateTime()
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	CString sTime;
	sTime.Format("%04d%02d%02d%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	return sTime;
}

DWORD WINAPI startClient(LPVOID proc)
{
	printf("start to receive\n");
	char buf[1000];
    int iLength=1000;
	for(;;)
	{
    int recvlen=recv(sock, buf, iLength, 0);
	if(recvlen>0)
	{
	 printf("have receive \n");
	 printf(buf);
     //////////////////////////////////////////////////////////
     for( int i=0; i<recvlen; i++ )
	 {			
	   if( buf[i] == MSG_END_FLAG )
		{	
            printf("have receive successfully\n");
		    CHAR* data = CSCMemoryBlock::AllocBlock(i);
			memcpy( data, buf, i );
			CSCMemoryBlock * block = new CSCMemoryBlock( data, i );
			CSCMemoryBlockPtr pFredPtr( block );
            //modifyed  by whp
			//parser->Parse( pFredPtr);
			//Parse Message get sesionID
            //////////////////////////////
             DWORD SessionID;
             if( block->GetSize() <= 0 )
	         return 0;
	         TCHAR sTmp[128] = {0};
	         CMarkup m_XmlDOM;
	         if(!m_XmlDOM.SetDoc(CString(block->GetBlock(), block->GetSize())))
	         {
		        //Clog( LOG_DEBUG, _T("Info - CDSAParse::Parse() - !Parse Message Error."));
		        return 0;
	         }
	         //TRACE( _T("%s \r\n"), CString(block->GetBlock(), block->GetSize()) );
	         TRACE( _T("%s \r\n"), m_XmlDOM.GetDoc() );
	         if(!m_XmlDOM.FindElem( MESSAGEFLAG ) )
		      return 0 ;
	         m_XmlDOM.IntoElem();
	         // Do parse stuff
	         if( !m_XmlDOM.FindElem( MESSAGEHEADER ) )
		     return 0;
	         UINT32 MessageCode = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGECODE ));
	         CString strTime = m_XmlDOM.GetAttrib( MESSAGETIME );
	         UINT32 BeReturn = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( MESSAGERETURN ));
	         SessionID= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( SESSION ));
	         //if( !m_XmlDOM.FindElem( MESSAGEBODY ) )
			 // {
		     //  return 0;
			 // }
			 //Constrct the message to send
			 ///////////////////////////////////////
			 //else
			 {
               CMarkup tmpXmlDOM;
               tmpXmlDOM.AddElem( _T("Message") );
	           tmpXmlDOM.IntoElem();
	           CString sessionid;
	           sessionid.Format("%d",SessionID);
               tmpXmlDOM.AddElem( _T("MessageHeader") );
	           tmpXmlDOM.AddAttrib( _T("MessageCode"), _T("2008") );  
               tmpXmlDOM.AddAttrib( _T("SessionID"),sessionid );  
	           tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
	           tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 
               CString sTmp = tmpXmlDOM.GetDoc();
	           int length = sTmp.GetLength();
	           //char * pcData = CSCMemoryBlock::AllocBlock(length+1);	
	           char pcData[1000];
	           strncpy( pcData, sTmp.GetBuffer(0), length );
	           pcData[length] = MSG_END_FLAG;
               if(Send(pcData,length+1)>0)
	           {
		         printf("successfully send\n");
	           }
		 	 }
	    	}
    	}
	}
	Sleep(1000);
	}
	///////////////////////////////////////////////////
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only application object
using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	WSADATA wsaDATA;
	if(::WSAStartup(0x202,&wsaDATA)!=0)
	{
		printf("error\n");
        WSACleanup();
		return -1;
	}
    SOCKADDR_IN addr;
	addr.sin_family=AF_INET;
	addr.sin_addr.S_un.S_addr=inet_addr("192.168.80.46");
	addr.sin_port=htons(4000);
    for(;;)
	{
      sock=socket(AF_INET,SOCK_STREAM,0);
      unsigned long cmd;
      cmd=1;
	  ioctlsocket(sock,FIONBIO,&cmd);
	  int n;
	  if(n=connect(sock,(LPSOCKADDR)(&addr),sizeof(addr))==0)
	  {
		printf("Successfully connected\n");
		break;
	  }
	  else 
	  {
        if(WSAGetLastError()==WSAEWOULDBLOCK)
		{
		  FD_SET writeset;
		  FD_ZERO(&writeset);
		  FD_SET(sock,&writeset);
		  TIMEVAL  time;
		  time.tv_sec=5;
		  time.tv_usec=0;
		  if(select(1,NULL,&writeset,NULL,&time)!=0)
		  {
             printf("Successfully connected\n");
			 break;
		  }
		  else
		  {
			  printf("over time\n");
          }
		}
		else
		{
            printf("error");
            break;
		}
	  }
    }
	///////////////////////////////////////////////////
	//Sleep(2000);
    CMarkup tmpXmlDOM;
    tmpXmlDOM.AddElem( _T("Message") );
	tmpXmlDOM.IntoElem();
	//CString sessionid;
	//sessionid.Format("%d",ID);
    tmpXmlDOM.AddElem( _T("MessageHeader") );
	tmpXmlDOM.AddAttrib( _T("MessageCode"), _T("2001") );  
    tmpXmlDOM.AddAttrib( _T("SessionID"),"11" );  
	tmpXmlDOM.AddAttrib( _T("MessageTime"), GetCurrDateTime() ); 
	tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 
    CString sTmp = tmpXmlDOM.GetDoc();
	int length = sTmp.GetLength();
	//char * pcData = CSCMemoryBlock::AllocBlock(length+1);	
	char pcData[1000];
	strncpy( pcData, sTmp.GetBuffer(0), length );
	pcData[length] = MSG_END_FLAG;
    if(Send(pcData,length+1)>0)
	{
		printf("successfully send\n");
	}
	//CSCMemoryBlock * mBlock = new CSCMemoryBlock( pcData, length+1 );
   	//CSCMemoryBlockPtr pFred( mBlock );
	//if(connection->m_pSender!=NULL)
	//{
	  // connection->m_pSender->Push(pFred);
    //}
	//delete pcData;
	///////////////////////////////////////////////////////
	CreateThread(NULL,0,startClient,NULL,0,NULL);
	Sleep(INFINITE);
	return 0;
}


