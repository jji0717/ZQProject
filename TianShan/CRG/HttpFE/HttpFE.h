#ifndef __ELOOP_HttpFE_H__
#define __ELOOP_HttpFE_H__

#include "CRGateway.h"
#ifdef ZQ_OS_MSWIN
#include "BaseZQServiceApplication.h"
#else
#include "ZQDaemon.h"
#endif

// #include "snmp/ZQSnmpMgmt.hpp"
// #include "snmp/SubAgent.hpp"

namespace ZQTianShan {

	namespace HttpFE {

		class HttpFEService : public ZQ::common::BaseZQServiceApplication 
		{
		public:

			HttpFEService ();
			virtual ~HttpFEService ();

			HRESULT OnStart(void);
			HRESULT OnStop(void);
			HRESULT OnInit(void);
			HRESULT OnUnInit(void);

			void doEnumSnmpExports(void);

			::std::string					m_strProgramRootPath;
			::std::string					_strPluginFolder;
			::std::string					_strLogFolder;

			//	::ZQ::common::FileLog			_fileLog;

			//CRGateway obj
			CRG::CRGateway*                _crg;
			//	 ZQ::Snmp::Subagent *           _HttpFESnmp;

		public:
			uint32 dummyGet(void) { return 1; }
			void snmp_resetStat(const uint32&);
		};

	}//namespace HttpFE

}//namespace ZQTianShan

#endif __ZQTIANSHAN_HttpFE_H__
