#ifndef _MCUSS_SERVICE_H
#define _MCUSS_SERVICE_H

#include "ZQ_common_conf.h"
#include "MCUSSEnv.h"
#include "MCUSSCfg.h"
#include "FileLog.h"
//#include "Ice/Ice.h"
#include "SsServiceImpl.h"
#include "snmp/SubAgent.hpp"

#ifdef  ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#else
#include <ZQDaemon.h>
#endif

namespace ZQTianShan{
	namespace MCUSS{
		class MCUSSService 
			:public ZQ::common::BaseZQServiceApplication
		{
		public:
			MCUSSService(void);
			~MCUSSService(void);
		protected:
			HRESULT OnInit();
			HRESULT OnStart();
			HRESULT OnStop();
			HRESULT OnUnInit();	

			bool	InitializeIceRunTime( );
			void	UninitializeIceRunTime( );
			bool	initializeCrashDumpLocation( );
			bool	initializeLogger( );
			void	uninitializeLogger( );
			bool	initializeServiceParameter( );
			std::string	getLoggerFolder( ) const;
		private:
			Ice::CommunicatorPtr				mIc;
			ZQ::common::FileLog					mIceFileLogger;
			ZQ::common::FileLog					mSessionLogger;
			ZQTianShan::MCUSS::MCUSSEnv*			mpMCUSSEnv;
			ZQ::StreamService::SsServiceImpl*	mpServiceImpl;
			ZQ::Snmp::Subagent*                 nssSnmpAgent;

			//ZQTianShan::ContentStore::NGODCSEnv	mNGODCSEnv;
			ZQADAPTER_DECLTYPE					mainAdapter;
			ZQ::common::NativeThreadPool		mStreamServiceThreadpool;
			//ZQ::common::NativeThreadPool		mContentStorethreadpool;
			ZQ::common::NativeThreadPool		_thrdPoolRTSP;
		};

		class nssSessionCount: public ZQ::Snmp::IVariable
		{
		public:
			nssSessionCount(ZQ::StreamService::SsServiceImpl* serviceInstance)
				:_serviceInstance(serviceInstance){};

			~nssSessionCount()
			{
				_serviceInstance = NULL;
			}

			virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
			{
				int sessionCount = _serviceInstance->sessionCount();
				return smivalFrom(val, sessionCount, desiredType);
			}

			virtual bool set(const ZQ::Snmp::SmiValue& val)
			{
				return true;// read only, not set
			}

			virtual bool validate(const ZQ::Snmp::SmiValue& val) const
			{
				return true;
			}

		private:
			ZQ::StreamService::SsServiceImpl* _serviceInstance;
		};


	}//namespace MCUSS
}//namespace ZQTianShan


#endif