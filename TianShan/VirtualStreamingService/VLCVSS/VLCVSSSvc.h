#ifndef __VLCVSSSvc_H__
#define __VLCVSSSvc_H__

#include "VLCVSSCfgLoader.h"
#include "BaseZQServiceApplication.h"
#include "VLCVSSEnv.h"
#include "VLCStreamServiceImpl.h"
#include "IceLog.h"
//#include "ContentImpl.h"
#include <direct.h>

namespace ZQTianShan {

namespace VSS {

namespace VLC{

class VLCVSS : public ZQ::common::BaseZQServiceApplication 
{
public:

	VLCVSS ();
	virtual ~VLCVSS ();

	//public:

	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

	VLCVSSEnv						*_pVSSEnv;
	VLCStreamServiceImplPtr			 _cps;

	::std::string					m_strProgramRootPath;
	::std::string					_strPluginFolder;
	::std::string					_strLogFolder;

	Ice::CommunicatorPtr			_ic;
	ZQADAPTER_DECLTYPE				_csAdapter;

	::TianShanIce::common::IceLogIPtr	_iceLog;

	::ZQ::common::FileLog			_iceFileLog;
	::ZQ::common::FileLog			_fileLog;
	::ZQ::common::FileLog			_fileLog_CS;
	::ZQ::common::FileLog			_fileLog_CSEvent;

	//for contentstore
	::ZQTianShan::ContentStore::ContentStoreImpl::Ptr _contentStore;
	//::ZQ::common::FileLog			_csIceFileLog;
	::ZQ::common::FileLog			_csFileLog;
	::ZQ::common::FileLog			_csEventFileLog;
};

}//namespace VLC

}//namespace VSS

}//namespace ZQTianShan

#endif __VLCVSSSvc_H__