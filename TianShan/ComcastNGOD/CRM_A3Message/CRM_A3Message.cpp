// CRM_A3Message.cpp : Defines the entry point for the DLL application.
//

#include "ZQ_common_conf.h"
#include "A3MsgCommon.h"
#include "A3MessageHandler.h"
#include "A3MsgEnv.h"
#include  "ZQResource.h"
bool bModuled = false;
CRM::A3Message::A3MsgEnv _env;
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
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(CRM_A3MESSAGE, "Load CRM_A3Message module Version: %d.%d.%d.%d"), ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_PRODUCT_VER_PATCH, ZQ_PRODUCT_VER_BUILD);

				mgr->registerContentHandler(URL_GETVOLUMEINFO, _env._A3MsgHandler);
				mgr->registerContentHandler(URL_GETCONTENTINFO, _env._A3MsgHandler);
				mgr->registerContentHandler(URL_TRANSFERCONTENT, _env._A3MsgHandler);
				mgr->registerContentHandler(URL_CANCELTRANSFER, _env._A3MsgHandler);
				mgr->registerContentHandler(URL_EXPOSECONTENT, _env._A3MsgHandler);
				mgr->registerContentHandler(URL_GETCONTENTCHECKSUM, _env._A3MsgHandler);
				mgr->registerContentHandler(URL_DELETECONTENT, _env._A3MsgHandler);
				mgr->registerContentHandler(URL_GETTRANSFERSTATUS, _env._A3MsgHandler);		
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
			mgr->unregisterContentHandler(URL_GETVOLUMEINFO, _env._A3MsgHandler);
			mgr->unregisterContentHandler(URL_GETCONTENTINFO, _env._A3MsgHandler);
			mgr->unregisterContentHandler(URL_TRANSFERCONTENT, _env._A3MsgHandler);
			mgr->unregisterContentHandler(URL_CANCELTRANSFER, _env._A3MsgHandler);
			mgr->unregisterContentHandler(URL_EXPOSECONTENT, _env._A3MsgHandler);
			mgr->unregisterContentHandler(URL_GETCONTENTCHECKSUM, _env._A3MsgHandler);
			mgr->unregisterContentHandler(URL_DELETECONTENT, _env._A3MsgHandler);
			mgr->unregisterContentHandler(URL_GETTRANSFERSTATUS, _env._A3MsgHandler);
			_env.doUninit();
		}
	}
}
