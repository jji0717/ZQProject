#ifndef _CVSS_SERVICE_H_
#define _CVSS_SERVICE_H_

#include "ConfigHelper.h"
#include "BaseZQServiceApplication.h"
#include "CVSSEnv.h"
#include "CVSSImpl.h"
#include "IceLog.h"

#include "ContentImpl.h"
#include <direct.h>

namespace ZQTianShan {

namespace CVSS {

class CVSSvc : public ZQ::common::BaseZQServiceApplication 
{
public:

	CVSSvc ();
	virtual ~CVSSvc ();

	//public:

	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

	CVSSEnv							*_pCVSSEnv;
	CiscoVirtualStreamServiceImplPtr _cps;

	::std::string					m_strProgramRootPath;
	::std::string					_strPluginFolder;
	::std::string					_strLogFolder;

	Ice::CommunicatorPtr			_ic;

	::TianShanIce::common::IceLogIPtr	_iceLog;

	::ZQ::common::FileLog			_iceFileLog;
	::ZQ::common::FileLog			_fileLog;

	//for A3CS
	::ZQTianShan::ContentStore::ContentStoreImpl::Ptr _contentStore;
	//::ZQ::common::FileLog			_csIceFileLog;
	::ZQ::common::FileLog			_csFileLog;
	::ZQ::common::FileLog			_csEventFileLog;
	ZQADAPTER_DECLTYPE				_csAdapter;
};

	
}//namespace ZQTianShan

}//namespace CVSS

#endif _CVSS_SERVICE_H_