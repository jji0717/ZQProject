// JMS_DBSyncAddIn_TEST.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "iostream.h"
#include "afxwin.h"
#include "../../doc/DBSAdi_def.h"
#include <tchar.h>

char*	WChartoAnsi(wchar_t* pwChar,char* pBuf,int bufLen)
{
	if(!pwChar)	return NULL;

	wcstombs(pBuf,pwChar,bufLen);
	return pBuf;
}
wchar_t* AnsitoWChar(char* pAnsi,wchar_t* pWbuf,int wBuflen)
{
	if(!pAnsi)	return NULL;

	int iCOunt=strlen(pAnsi)+1;
	//mbstowcs(pWbuf,pAnsi,iCOunt);
	MultiByteToWideChar(CP_ACP,0,pAnsi,wBuflen,pWbuf,wBuflen);
	
	return pWbuf;
}


int main(int argc, char* argv[])
{
#define		BUFSIZE 1024
#define		WBUFSIZE 2048
//	char		szBuf[BUFSIZE];
	wchar_t		wszBuf[WBUFSIZE];
#ifdef _DEBUG

	HMODULE	hMod=LoadLibrary("JMS_DBSyncAddIn_d.dll");
#else

	HMODULE	hMod=LoadLibrary("JMS_DBSyncAddIn.dll");

	
#endif
	if(!hMod)
	{
		cout<<"Can't load library JMS_DBSyncAddIn.dll"<<endl;
		return -1;
	}
	DBSAProto_Init			JmsInit;
	DBSAProto_Uninit		JmsUninit;
	DBSAProto_SyncBein		JmsBegin;
	DBSAProto_SyncEnd		JmsEnd;
	DBSAProto_TrggStat		JmsTriggerState;
	
	JmsInit=(DBSAProto_Init)GetProcAddress(hMod,"DBSA_Initialize");
	JmsUninit=(DBSAProto_Uninit)GetProcAddress(hMod,"DBSA_Uninitialize");
	JmsBegin=(DBSAProto_SyncBein)GetProcAddress(hMod,"DBSA_SyncBegin");
	JmsEnd=(DBSAProto_SyncEnd)GetProcAddress(hMod,"DBSA_SyncEnd");
	JmsTriggerState=(DBSAProto_TrggStat)GetProcAddress(hMod,"DBSA_TriggerState");
	if(!(JmsInit&&JmsUninit&&JmsBegin&&JmsEnd&&JmsTriggerState))
	{
		cout<<"Can't get function address"<<endl;
		return -1;
	}
	DA_dbsyncInfo	syncInfo;
	DA_itvInfo		itvInfo;
	
	syncInfo._dwInstanceID=1;
	syncInfo._dwSupportNav=1;
	syncInfo._dwTwThreshold=10;
	wcscpy(syncInfo._szIPAddr,AnsitoWChar("192.168.80.123",wszBuf,WBUFSIZE));
	wcscpy(syncInfo._szSyncDir,AnsitoWChar("\\ÕÅºêÈ¨\\",wszBuf,WBUFSIZE));
	wcscpy(syncInfo._szVersion,AnsitoWChar("1.2.3.4",wszBuf,WBUFSIZE));
	
	wcscpy(itvInfo._szIPAddr,AnsitoWChar("22.33.44.55",wszBuf,WBUFSIZE));
	wcscpy(itvInfo._szVersion,AnsitoWChar("4.3.2.1",wszBuf,WBUFSIZE));

	JmsInit(&syncInfo,&itvInfo);

	JmsBegin();

	////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	DA_entryDb	entryDb;
	DA_stateDb	StateDb;

	entryDb._dwEntryType=10;
	entryDb._dwEntryUID=1234354;
	wcscpy(entryDb._szLocalEntryUID,AnsitoWChar("it's nothing",wszBuf,WBUFSIZE));

	StateDb._dwEntryState=23;


	cout<<"start sync"<<endl;
	
//	for(DWORD i=0;i<10;i++)
	DWORD i=0;
	while(1)
	{	
		i++;
		entryDb._dwEntryType=i%2;
		entryDb._dwEntryUID=i*6%123456789;		
		StateDb._dwEntryState=i%2+1;
		Sleep(4000);
		JmsTriggerState(&entryDb,&StateDb);
	}
	cout<<"end sync"<<endl;
	
	cin>>i;
	JmsEnd();
	JmsUninit();
	FreeLibrary(hMod);
	return 1;		 
}

