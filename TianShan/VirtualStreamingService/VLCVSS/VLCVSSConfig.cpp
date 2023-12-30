#include "VLCVSSConfig.h"

namespace ZQTianShan{

namespace VSS{

namespace VLC{

VLCVSSConfig::VLCVSSConfig(const char *filepath):
_logFile(ZQ::common::Log::L_DEBUG),
_crashDump(ZQ::common::Config::Loader< CrashDump >("")),
_iceTrace(ZQ::common::Config::Loader< IceTrace >("")),
_iceStorm(ZQ::common::Config::Loader< IceStorm >("")),
_iceProperties(ZQ::common::Config::Loader< IceProperties >("")),
_dataBase(ZQ::common::Config::Loader< Database >("")),
_publishedLogs(ZQ::common::Config::Loader< PublishedLogs >("")),
_cLogFile(ZQ::common::Config::Loader< LogFile >("")),
_bind(ZQ::common::Config::Loader< stBind >("")),
_telnetProp(ZQ::common::Config::Loader< TelnetProp >("")),
_streamServiceProp(ZQ::common::Config::Loader< StreamServiceProp >("")),
_VLCServiceProp(ZQ::common::Config::Loader< VLCServiceProp >("")),
_VLCVSSIceLog(ZQ::common::Config::Loader< VLCVSSIceLog >("")),
_telnetServerInfo(ZQ::common::Config::Loader< TelnetServerInfo >(""))
,_volumnInfo(ZQ::common::Config::Loader< VolumnInfo >(""))
,_storeInfo(ZQ::common::Config::Loader< StoreInfo >(""))
{
	if (filepath != NULL)
		_strFilePath = filepath;
}

VLCVSSConfig::~VLCVSSConfig()
{
}

void VLCVSSConfig::ConfigLoader()
{
	//get default configuration
	_crashDump.setLogger(&_logFile);
    if(!_crashDump.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <CrashDump>"));
    }

	_iceTrace.setLogger(&_logFile);
    if(!_iceTrace.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <IceTrace>"));
    }

	_iceStorm.setLogger(&_logFile);
	if(!_iceStorm.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <IceStorm>"));
	}
	
	_iceProperties.setLogger(&_logFile);
    if(!_iceProperties.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <IceProperties>"));
    }

	_dataBase.setLogger(&_logFile);
    if(!_dataBase.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <DataBase>"));
    }
	
	_publishedLogs.setLogger(&_logFile);
	if(!_publishedLogs.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <PublishedLogs>"));
	}

	//get NSS configuration
	_cLogFile.setLogger(&_logFile);
	if(!_cLogFile.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <LogFile>"));
	}

	_bind.setLogger(&_logFile);
    if(!_bind.load(_strFilePath.c_str()))
    {
        _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <Bind>"));
    }

	_telnetProp.setLogger(&_logFile);
	if(!_telnetProp.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <TelnetProp>"));
	}

	_streamServiceProp.setLogger(&_logFile);
	if (!_streamServiceProp.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <StreamServiceProp>"));
	}

	_VLCServiceProp.setLogger(&_logFile);
	if (!_VLCServiceProp.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <VLCServiceProp>"));
	}

	_VLCVSSIceLog.setLogger(&_logFile);
	if(!_VLCVSSIceLog.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <Icelog>"));
	}

	_telnetServerInfo.setLogger(&_logFile);
	if(!_telnetServerInfo.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <SoapServerInfo>"));
	}
	
	_volumnInfo.setLogger(&_logFile);
	if(!_volumnInfo.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <VolumnInfo>"));
	}

	_storeInfo.setLogger(&_logFile);
	if(!_storeInfo.load(_strFilePath.c_str()))
	{
		_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCVSSConfig,"fail parse <StoreInfo>"));
	}
}

}//namespace VLC

}//namespace VSS

}//namespace ZQTianShan