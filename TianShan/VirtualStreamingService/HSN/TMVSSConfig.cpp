#include "TMVSSConfig.h"

namespace ZQTianShan{

namespace VSS{

namespace TM{

TMVSSConfig::TMVSSConfig(const char *filepath):
_logFile(ZQ::common::Log::L_DEBUG),
_crashDump(ZQ::common::Config::Loader< CrashDump >("")),
_iceTrace(ZQ::common::Config::Loader< IceTrace >("")),
_iceStorm(ZQ::common::Config::Loader< IceStorm >("")),
_iceProperties(ZQ::common::Config::Loader< IceProperties >("")),
_dataBase(ZQ::common::Config::Loader< Database >("")),
_publishedLogs(ZQ::common::Config::Loader< PublishedLogs >("")),
_cLogFile(ZQ::common::Config::Loader< LogFile >("")),
_rtspProp(ZQ::common::Config::Loader< RTSPProp >("")),
_bind(ZQ::common::Config::Loader< stBind >("")),
_tmvssIceLog(ZQ::common::Config::Loader< TMVSSIceLog >("")),
_localInfo(ZQ::common::Config::Loader< LocalInfo >("")),
_soapServerInfo(ZQ::common::Config::Loader< SoapServerInfo >("")),
_privateData(ZQ::common::Config::Loader< PrivateData >("")),
_resourceMap(ZQ::common::Config::Loader< ResourceMap>(""))
{
	if (filepath != NULL)
		_strFilePath = filepath;
}

TMVSSConfig::~TMVSSConfig()
{
}

void TMVSSConfig::ConfigLoader()
{
	//get default configuration
	_crashDump.setLogger(&_logFile);
    if(!_crashDump.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <CrashDump>"));
    }

	_iceTrace.setLogger(&_logFile);
    if(!_iceTrace.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <IceTrace>"));
    }

	_iceStorm.setLogger(&_logFile);
	if(!_iceStorm.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <IceStorm>"));
	}
	
	_iceProperties.setLogger(&_logFile);
    if(!_iceProperties.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <IceProperties>"));
    }

	_dataBase.setLogger(&_logFile);
    if(!_dataBase.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <DataBase>"));
    }
	
	_publishedLogs.setLogger(&_logFile);
	if(!_publishedLogs.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <PublishedLogs>"));
	}

	//get NSS configuration
	_cLogFile.setLogger(&_logFile);
	if(!_cLogFile.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <LogFile>"));
	}

	_bind.setLogger(&_logFile);
    if(!_bind.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <Bind>"));
    }

	_rtspProp.setLogger(&_logFile);
	if(!_rtspProp.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <RtspProp>"));
	}

	_tmvssIceLog.setLogger(&_logFile);
	if(!_tmvssIceLog.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <Icelog>"));
	}

	_localInfo.setLogger(&_logFile);
	if(!_localInfo.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <LocalInfo>"));
	}
	
	_soapServerInfo.setLogger(&_logFile);
    if(!_soapServerInfo.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <SoapServerInfo>"));
    }

	_privateData.setLogger(&_logFile);
	if(!_privateData.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <PrivateData>"));
	}

	_resourceMap.setLogger(&_logFile);
	if(!_resourceMap.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(TMVSSConfig,"fail parse <ResourceMap>"));
	}
}

}//namespace TM

}//namespace NSS

}//namespace ZQTianShan