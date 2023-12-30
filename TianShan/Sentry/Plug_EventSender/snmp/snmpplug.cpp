// snmpplug.cpp : Defines the entry point for the DLL application.
//

#include "SnmpSender.h"
#include <ZQ_common_conf.h>

using namespace std;
using namespace ZQ::common;

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

SnmpSender* pSender = NULL;

ZQ::common::Log* plog = NULL;
Config::Loader< SnmpSenderInfo>* pSnmpSenderCfg = NULL;

IMsgSender* g_pIMsgSender = NULL;

void OnSnmpMessage(const MSGSTRUCT& msgStruct, const MessageIdentity& mid, void* ctx)
{
	if(pSender == NULL)
		return;

 	pSender->AddMessage(msgStruct, mid, ctx);
}

bool init(const char* pCfgPath,const char* pType)
{
	if(pCfgPath == NULL || pType == NULL)
		return false;

	if((stricmp(pType,"snmp") != 0))
		return false;

	if(pSender == NULL)
	{	
		pSender = new SnmpSender(5);
		if(pSender)
		{		
			if(!pSender->GetParFromFile(pCfgPath))
			{
				if(plog != NULL)
				{
					LOG(Log::L_ERROR,"SNMP sender get configuration error");
					delete plog;
					plog = NULL;
				}
				if(pSnmpSenderCfg != NULL)
				{
					delete pSnmpSenderCfg;
					pSnmpSenderCfg = NULL;
				}
				delete pSender;
				pSender = NULL;
				return false;		
			}
			if(!pSender->init())
			{
				if(plog != NULL)
				{
					LOG(Log::L_ERROR,"SNMP sender init error");
					delete plog;
					plog = NULL;
				}
				if(pSnmpSenderCfg != NULL)
				{
					delete pSnmpSenderCfg;
					pSnmpSenderCfg = NULL;
				}
				
				delete pSender;
				pSender = NULL;
				return false;
			}

			if(plog != NULL)
				LOG(Log::L_INFO,"SNMP sender init successful");
		}
		else
			return false;
	}
	return true;
}

void Exit()
{
	if(pSender != NULL)
	{
		pSender->Close();
		delete pSender;
		pSender = NULL;
		LOG(Log::L_INFO,"SNMP Sender plug exit");		
		return;
		
	}
	if(plog != NULL)
			LOG(Log::L_WARNING,"SNMP sender plug has exit,can not exit again");
}

extern "C"
{

__EXPORT bool InitModuleEntry( IMsgSender* pISender, const char* pType, const char* pText)
{
	if(pISender == NULL || pText == NULL)
		return false;

	if(init(pText,pType))
	{
		if(pISender->regist((OnNewMessage)OnSnmpMessage,pType))
		{
			LOG(Log::L_INFO,"PlugIn register SNMP sender successful");

			if(g_pIMsgSender == NULL)
				g_pIMsgSender = pISender;

			return true;
		}
		LOG(Log::L_ERROR,"SNMP sender regist failed");
		Exit();//release plug 
	}

	return false;
}

__EXPORT void UninitModuleEntry( IMsgSender* pISender )
{
	if(pISender != NULL)
		pISender->unregist((OnNewMessage)OnSnmpMessage,"SNMP");
		

	Exit();
	g_pIMsgSender = NULL;

	if(pSnmpSenderCfg != NULL)
	{
		delete pSnmpSenderCfg;
		pSnmpSenderCfg = NULL;
	}
	if(plog != NULL)
	{
		LOG(Log::L_INFO,"SNMP sender plug unregister");
		delete plog;
		plog = NULL;
	}
}

}//extern "c"

