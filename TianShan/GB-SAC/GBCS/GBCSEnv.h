#ifndef __ZQTianShanNGODEnv_H__
#define __ZQTianShanNGODEnv_H__


#include "FileLog.h"
#include "ConfigHelper.h"
#include "GBCSConfig.h"
#include "GBCSCfgLoader.h"
#include "ContentImpl.h"


namespace ZQTianShan {
	namespace ContentStore {

class GBCSEnv
{
public:
	GBCSEnv();
	~GBCSEnv();

	void setServiceName(const char* szServiceName);
	void setLogPath(const char* szLogPath);
	int  setGBCSLog(ZQ::common::FileLog	* gbcsLog);
	void setDataPath(const char* szDataPath);
	void setConfig(ZQTianShan::GBCS::GBCSBaseConfig::GBCSHolder* pBaseCfg);
	void setThreadPool(::ZQ::common::NativeThreadPool*	pThreadPool);
	void setIceAdapter(ZQADAPTER_DECLTYPE& adapter);
	bool initEnv();
	void uninitEnv();
public:

	::ZQTianShan::ContentStore::ContentStoreImpl::Ptr	_store;
	ZQ::common::FileLog	*	_GBCSLogger;
	ZQ::common::FileLog		_GBCSEventLogger;
	::ZQ::common::NativeThreadPool*		_threadPool;
	std::string			_strServiceName;
	std::string			_strLogPath;
	std::string			_strDataPath;
	ZQADAPTER_DECLTYPE*		_pAdapter;
	ZQTianShan::GBCS::GBCSBaseConfig::GBCSHolder *	_pGBCSBaseConfig;
};

}}

#endif
