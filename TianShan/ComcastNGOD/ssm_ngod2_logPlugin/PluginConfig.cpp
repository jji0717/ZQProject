#include "PluginConfig.h"

PluginConfig::PluginConfig(const char *filepath)
:_myLog(ZQ::common::Config::Loader< myLog >(""))
,_dbPath(ZQ::common::Config::Loader< DBPath >(""))
,_timeOut(ZQ::common::Config::Loader< TimeOut >(""))
{
	if (filepath != NULL)
		_strFilePath = filepath;
}

PluginConfig::~PluginConfig()
{
}

void PluginConfig::ConfigLoader()
{
	if(!_myLog.load(_strFilePath.c_str()))
	{
		printf("fail parse <LOG>");
	}

	if(!_dbPath.load(_strFilePath.c_str()))
	{
		printf("fail parse <DBPath>");
	}

	if(!_timeOut.load(_strFilePath.c_str()))
	{
		printf("fail parse <TimeOut>");
	}
}