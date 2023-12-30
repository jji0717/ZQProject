// FileName : A3DllMain.cpp
// Author   : Zheng Junming
// Date     : 2009-05
// Desc     : entry point of A3 Server

#include "A3Environment.h"
#include "A3MsgHandler.h"
#include "ZQ_common_conf.h"

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
#endif

bool bModuled = false;
CRG::Plugin::A3Server::Environment env;

extern "C"
{
	__EXPORT bool CRM_Entry_Init(CRG::ICRMManager* mgr)
	{
		if (false == bModuled)
		{
			bModuled = true;
			std::string strCfgFolder = mgr->getConfigFolder();
			std::string strLogFolder = mgr->getLogFolder();
			if (env.doInit(strLogFolder, strCfgFolder))
			{
				mgr->registerContentHandler(".*/GetVolumeInfo", env._A3MsgHandler);
				mgr->registerContentHandler(".*/GetContentInfo", env._A3MsgHandler);
				mgr->registerContentHandler(".*/TransferContent", env._A3MsgHandler);
				mgr->registerContentHandler(".*/CancelTransfer", env._A3MsgHandler);
				mgr->registerContentHandler(".*/ExposeContent", env._A3MsgHandler);
				mgr->registerContentHandler(".*/GetContentChecksum", env._A3MsgHandler);
				mgr->registerContentHandler(".*/DeleteContent", env._A3MsgHandler);
				mgr->registerContentHandler(".*/GetTransferStatus", env._A3MsgHandler);
				return true;
			}
			else
			{
				env.doUninit();
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
			mgr->unregisterContentHandler(".*/GetVolumeInfo", env._A3MsgHandler);
			mgr->unregisterContentHandler(".*/GetContentInfo", env._A3MsgHandler);
			mgr->unregisterContentHandler(".*/TransferContent",env._A3MsgHandler);
			mgr->unregisterContentHandler(".*/CancelTransfer", env._A3MsgHandler);
			mgr->unregisterContentHandler(".*/ExposeContent", env._A3MsgHandler);
			mgr->unregisterContentHandler(".*/GetContentChecksum", env._A3MsgHandler);
			mgr->unregisterContentHandler(".*/DeleteContent", env._A3MsgHandler);
			mgr->unregisterContentHandler(".*/GetTransferStatus", env._A3MsgHandler);
			env.doUninit();
		}
	}
};

