#ifndef __HSN_H__
#define __HSN_H__

#include "TMVSSCfgLoader.h"
#include "BaseZQServiceApplication.h"
#include "TMVSSEnv.h"
#include "TMVSStreamServiceImpl.h"
#include "IceLog.h"
//#include "ContentImpl.h"
#include <direct.h>

namespace ZQTianShan {

namespace VSS {

namespace TM{

class HSN : public ZQ::common::BaseZQServiceApplication 
{
public:

	HSN ();
	virtual ~HSN ();

	//public:

	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

	TMVSSEnv						*_pEnv;
	TMVStreamServiceImplPtr			 _cps;

	::std::string					m_strProgramRootPath;
	::std::string					_strPluginFolder;
	::std::string					_strLogFolder;

	Ice::CommunicatorPtr			_ic;
	ZQADAPTER_DECLTYPE				_csAdapter;

	::TianShanIce::common::IceLogIPtr	_iceLog;

	::ZQ::common::FileLog			_iceFileLog;
	::ZQ::common::FileLog			_fileLog;

	//for contentstore
	::ZQTianShan::ContentStore::ContentStoreImpl::Ptr _contentStore;
	//::ZQ::common::FileLog			_csIceFileLog;
	::ZQ::common::FileLog			_csFileLog;
	::ZQ::common::FileLog			_csEventFileLog;
};

}//namespace TM

}//namespace VSS

}//namespace ZQTianShan

#endif __HSN_H__