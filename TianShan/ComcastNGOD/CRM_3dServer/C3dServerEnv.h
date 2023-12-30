#ifndef __CRG_C3DSERVER__HANDLER_H__
#define __CRG_C3DSERVER__HANDLER_H__

#include "TianShanDefines.h"
#include "FileLog.h"
#include "SystemUtils.h"
#include "IceLog.h"
#include "CRMInterface.h"
#include "CdmiClientBase.h"
#include "CPE.h"

namespace CRM
{
	namespace C3dServer
	{
		class C3dServerMsgHandler;
		class C3dServerEnv
		{
		public:
			C3dServerEnv(void);
			~C3dServerEnv(void);

		public:
			bool doInit(const std::string& strLogPath, const std::string& strCfgPath);

			/// this method must be called no matter if doInit() success or fail
			void doUninit();
			void setCRMmanager(CRG::ICRMManager* mgr){_mgr = mgr;};

			TianShanIce::ContentProvision::ContentProvisionServicePrx getCPEProxy();
		public:
			C3dServerMsgHandler* _3dServerMsgHandler;
			Ice::CommunicatorPtr _communicator;

			CdmiClientBase* _pCdmiClient;
			ZQ::common::FileLog _log;

		private: 
			ZQ::common::Log* _pIceLog ;
			TianShanIce::common::IceLogIPtr	_iceLog ;

			CRG::ICRMManager* _mgr;

			TianShanIce::ContentProvision::ContentProvisionServicePrx _cpePrx;
			ZQ::common::NativeThreadPool* _cdmiClientPool;
		private:
			bool loadConfig(const std::string& strCfgPath);
			void initWithConfig(Ice::PropertiesPtr proper );
		};
#define envlog (_env._log)
	}///end namespace A3Server
}///end namespace CRM
#endif /// enddefine __CRG_3DSERVER__HANDLER_H__
