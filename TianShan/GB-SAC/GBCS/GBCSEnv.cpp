#include "GBCSEnv.h"
#include "TianShanIceHelper.h"


namespace ZQTianShan {
	namespace ContentStore {

void GBCSEnv::setThreadPool( ::ZQ::common::NativeThreadPool* pThreadPool )
{
	_threadPool = pThreadPool;
}

void GBCSEnv::setConfig( ZQTianShan::GBCS::GBCSBaseConfig::GBCSHolder* pBaseCfg )
{
	_pGBCSBaseConfig = pBaseCfg;
}

void GBCSEnv::setDataPath( const char* szDataPath )
{
	if (szDataPath)
		_strDataPath = szDataPath;
}

int GBCSEnv::setGBCSLog(ZQ::common::FileLog* gbcsLog)
{
	if (NULL == gbcsLog)
		return false;

	_GBCSLogger = gbcsLog;
	return true;
}

void GBCSEnv::setLogPath( const char* szLogPath )
{
	if (szLogPath)
		_strLogPath = szLogPath;
}

void GBCSEnv::setIceAdapter( ZQADAPTER_DECLTYPE& adapter )
{
	_pAdapter = &adapter;
}

bool GBCSEnv::initEnv()
{
	if (!_threadPool || !_pAdapter)
	{
		//
		return false;
	}

	(*_GBCSLogger)(ZQ::common::Log::L_INFO, CLOGFMT(NGODCS, "==================================== initializing ContentStore ===================================="));

	std::string strEvtLogFile = ZQTianShan::Util::fsConcatPath( _strLogPath , _strServiceName) + "_events.log";
	try
	{
		_GBCSEventLogger.open(strEvtLogFile.c_str(), ZQ::common::Log::L_DEBUG, 5, 1024*1024*20);
	}
	catch (ZQ::common::FileLogException& ex)
	{
		(*_GBCSLogger)(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "failed to open NGODCS event log file %s with error %s"), strEvtLogFile.c_str(), ex.what());
		return false;
	}

	std::string strDatabasePath;
	strDatabasePath = ZQTianShan::Util::fsConcatPath( _strDataPath , _strServiceName);
	_store = new ::ZQTianShan::ContentStore::ContentStoreImpl(*_GBCSLogger, _GBCSEventLogger, *_threadPool, *_pAdapter, strDatabasePath.c_str());
	if(! _store->initializeContentStore())
	{
		(*_GBCSLogger)(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "failed to initialize ContentStore"));
		return false;
	}

	std::string volName;
	std::string volPath;
	try
	{
		for (size_t i = 0; i <_pGBCSBaseConfig->_videoServer.vols.size(); i++)
		{
			volName = _pGBCSBaseConfig->_videoServer.vols[i].mount;
			volPath = _pGBCSBaseConfig->_videoServer.vols[i].targetName;
			if (volPath.empty())
				volPath = volName;

			volPath += FNSEPS;
			bool bDefaultVolume = (_pGBCSBaseConfig->_videoServer.vols[i].defaultVal != 0);

			_store->mountStoreVolume(volName.c_str(), volPath.c_str(), bDefaultVolume);
		}
	}
	catch(...)
	{
		(*_GBCSLogger)(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "mount store volume[%s] path[%s] caught an exception"), volName.c_str(),volPath.c_str());
		return false;
	}

	return true;
}

void GBCSEnv::uninitEnv()
{
	_store->unInitializeContentStore();
	_GBCSEventLogger.flush();
}

GBCSEnv::GBCSEnv()
{
	_threadPool = NULL;
	_pAdapter = NULL;
	_GBCSLogger = NULL;
}

GBCSEnv::~GBCSEnv()
{

}

void GBCSEnv::setServiceName( const char* szServiceName )
{
	_strServiceName = szServiceName;
}


}}