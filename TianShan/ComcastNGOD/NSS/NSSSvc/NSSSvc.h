#ifndef _NSS_SERVICE_H_
#define _NSS_SERVICE_H_


#include "ConfigHelper.h"
#include "BaseZQServiceApplication.h"
#include "NSSEnv.h"
#include "NSSImpl.h"
#include "IceLog.h"

//#include "A3ContentStoreImpl.h"
#include "ContentImpl.h"
#include <direct.h>

namespace ZQTianShan {

namespace NSS {

class NSSSvc : public ZQ::common::BaseZQServiceApplication 
{
public:
	
	NSSSvc ();
	virtual ~NSSSvc ();
	
//public:
	
	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

	NSSEnv							*_pNSSEnv;
	NGODStreamServiceImplPtr		_cps;
		
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

}//namespace NSS

#endif _NSS_SERVICE_H_