#include "ModConfig.h"
#include "ModService.h"

// ZQMODApplication::ModConfigLoader g_ModCfg;
// ZQ::common::ConfigLoader* configLoader = &g_ModCfg;

namespace ZQMODApplication
{

// ModConfigLoader::ModConfigLoader()
// {
// }
// 
// ModConfigLoader::~ModConfigLoader()
// {
// }
// 
// ZQ::common::ConfigLoader::PConfigSchema ModConfigLoader::getSchema()
// {
// 	return NULL; // 不依靠Config Loader读取配置信息
// }

SmartPreference::SmartPreference(ZQ::common::XMLPreferenceEx*& p) : _p(p)
{
}

SmartPreference::~SmartPreference()
{
	if (NULL != _p)
		_p->free();
	_p = NULL;
}

bool SmartPreference::getIntProp(const char* name, int& value)
{
	bool bRet = _p->getIntProp(name, value);
	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));
	_p->getPreferenceName(szBuf);
	if (bRet)
		glog(ZQ::common::Log::L_INFO, CLOGFMT(Configure, "%s.%s = %d"), szBuf, name, value);
	return bRet;
}

bool SmartPreference::getStrProp(const char* name, char* buff, const int buffSize)
{
	bool bRet = _p->getStrProp(name, buff, buffSize);
	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));
	_p->getPreferenceName(szBuf);
	if (bRet)
		glog(ZQ::common::Log::L_INFO, CLOGFMT(Configure, "%s.%s = %s"), szBuf, name, buff);
	return bRet;
}

} // namespace ZQMODApplication

