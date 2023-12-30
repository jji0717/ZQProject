// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Server.h"
#include"winsock2.h"
#include"Markup.h"
#include"scqueue.h"
#define MSG_END_FLAG  127

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
INT32 Send(SOCKET sock, CHAR* pBuffer, int iLength) 
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

DWORD WINAPI StartReceive(LPVOID para)
{   
	bool tosend=false;
    SOCKET sock=*(SOCKET*)para;
	printf("start to receive\n");
	char buf[1000];
    int iLength=1000;
	for(;;)
	{
     int recvlen=recv(sock, buf, iLength, 0);
	 if( recvlen>0)
	 {
	    printf("received\n");
		printf(buf);
	    ///////////////////////////////////////
        for( int i=0; i<recvlen; i++ )
	    {			
	     if( buf[i] == MSG_END_FLAG )
		 {	
            tosend=true;
            printf("Succssfully received\n");
		    CHAR* data = CSCMemoryBlock::AllocBlock(i);
			memcpy( data, buf, i );

			CSCMemoryBlock * block = new CSCMemoryBlock( data, i );
			CSCMemoryBlockPtr pFredPtr( block );
            //modifyed  by whp
			//parser->Parse( pFredPtr);
            /////////////////////////////////////////////////
			}
           //
		 }
		if(tosend)
		{
          if(Send(sock,buf,recvlen)>0)
		  {
			  printf("successfully Send \n");
		  }
		}
	   }
	   Sleep(1000);
	}
	///////////////////////////////////////
   	return 1;
}
// The one and only application object
CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	//////////////////////////////////////////////
	WSADATA wsaData;
   if(::WSAStartup(0x202,&wsaData)!=0)
	{
		printf("\nError in Startup session.\n");
		WSACleanup();
		return -1;
	}
	unsigned int Port;
	//cin>>Port;
    SOCKET sock;
	sock=socket(AF_INET,SOCK_STREAM,0);
	unsigned long cmd;
	cmd=TRUE;
	ioctlsocket(sock,FIONBIO,&cmd);
	SOCKADDR_IN addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(8888);
	addr.sin_addr.s_addr=INADDR_ANY;
	bind(sock,(LPSOCKADDR)(&addr),sizeof(addr));
	listen(sock,5);
    for(;;)
	{
       TIMEVAL time;
	   time.tv_usec=1;
	   time.tv_sec=1;
       FD_SET readset;
	   FD_ZERO(&readset);
       FD_SET(sock, &readset);
	   int result;
       result=select(1,&readset,NULL,NULL,&time);
	   if(result!=0)
	   {
	     SOCKET accptsock= accept(sock,NULL,0);
		 if(accptsock!=NULL)
		 {
		   printf("Successfully accept a socket\n");
		   CreateThread(NULL,0,StartReceive,(LPVOID)&accptsock,0,NULL);
		 }
	   }
	}
	Sleep(INFINITE);
	return nRetCode;
}
