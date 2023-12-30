// JMS_DBSyncAddIn.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#pragma warning(disable:4786)

#include "MsgManager.h"

#define		TEST_XML_TO_FILE		0

#if		TEST_XML_TO_FILE
void StoreMsg(std::string& msg)
{
	char	tag[]="\n\n\n\n\n\n new message is \n\n\n\n";
	FILE* pfile=fopen("d:\\log.txt","a+t");
	if(!pfile)	return;
	fwrite(tag,sizeof(tag),1,pfile);
	fwrite(msg.c_str(),msg.size(),1,pfile);	
	fclose(pfile);
}
#endif

CMsgManager*	pMan=NULL;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
	return TRUE;
}

#ifdef TEST
BOOL	Initialize()
{
	if(pMan)
	{
		delete	pMan;
		pMan=NULL;
	}
	pMan=new CMsgManager;
	if(!pMan)
		return FALSE;
	return pMan->IsInitOK();
}
VOID	UnInitialize()
{
	if(pMan)
	{
		delete pMan;
		pMan=NULL;
	}
}
VOID	PutMessage(char *p)
{
	//JMSMsg strMsg=pMan->GenerateTestMsg();
	JMSMsg	msg(p);
	pMan->PushMessage(msg);
}

#else

DBSACALLBACK DBSA_Initialize(DA_dbsyncInfo*	pDbsInfo,DA_itvInfo*	pItvInfo)
{
	if(pMan)
	{
		delete pMan;
		pMan=NULL;
	}
	pMan=new CMsgManager;
	
	JMSMsg	msg=pMan->GenerateInitialize(pDbsInfo,pItvInfo);
#if TEST_XML_TO_FILE
	StoreMsg(msg);
#endif
	if(!msg.empty())
		pMan->PushMessage(msg);
}
DBSACALLBACK DBSA_Uninitialize()
{
	JMSMsg	msg=pMan->GenerateUninitialize();
#if TEST_XML_TO_FILE
	StoreMsg(msg);
#endif
	if(!msg.empty())
		pMan->PushMessage(msg);
	if(pMan)
	{
		delete pMan;
		pMan=NULL;
	}
}
DBSACALLBACK DBSA_SyncBegin()
{
	JMSMsg msg=pMan->GenerateSyncBegin();
#if TEST_XML_TO_FILE
	StoreMsg(msg);
#endif
	if(!msg.empty())
		pMan->PushMessage(msg);
}

DBSACALLBACK DBSA_SyncEnd()
{
	JMSMsg msg=pMan->GenerateSyncEnd();
#if TEST_XML_TO_FILE
	StoreMsg(msg);
#endif
	if(!msg.empty())
		pMan->PushMessage(msg);
}
DBSACALLBACK DBSA_TriggerState(DA_entryDb*	pEntryBlock,DA_stateDb*	pStateBlock)
{
	JMSMsg msg=pMan->GenerateTriggerState(pEntryBlock,pStateBlock);
#if TEST_XML_TO_FILE
	StoreMsg(msg);
#endif
	if(!msg.empty())
		pMan->PushMessage(msg);
}

#endif



