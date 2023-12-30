// CRM_C3dServer.cpp : Defines the entry point for the DLL application.

#include "ZQ_common_conf.h"
#include "C3dServerCommon.h"
#include "C3dServerMsgHandler.h"
#include "C3dServerEnv.h"
#include  "ZQResource.h"

bool bModuled = false;
CRM::C3dServer::C3dServerEnv _env;

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	return TRUE;
}
#endif

extern "C"
{
	__EXPORT bool CRM_Entry_Init(CRG::ICRMManager* mgr)
	{
		if (false == bModuled && mgr)
		{
			std::string strCfgFolder = mgr->getConfigFolder();
			std::string strLogFolder = mgr->getLogFolder();
			_env.setCRMmanager(mgr);
			if (_env.doInit(strLogFolder, strCfgFolder))
			{
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(CRM_3DSERVER, "Load CRM_3dServer module Version: %d.%d.%d.%d"), ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_PRODUCT_VER_PATCH, ZQ_PRODUCT_VER_BUILD);

				mgr->registerContentHandler(URL_CONTENT, _env._3dServerMsgHandler);
				mgr->registerContentHandler(URL_CONTENTS, _env._3dServerMsgHandler);
				bModuled = true;
				return true;
			}
			else
			{
				_env.doUninit();
				return false;
			}
		}
		return true;
	}

	__EXPORT void CRM_Entry_Uninit(CRG::ICRMManager* mgr)
	{
		if (true == bModuled)
		{
			bModuled = false;
			mgr->unregisterContentHandler(URL_CONTENT, _env._3dServerMsgHandler);
			mgr->unregisterContentHandler(URL_CONTENTS, _env._3dServerMsgHandler);
			_env.doUninit();
		}
	}
}
