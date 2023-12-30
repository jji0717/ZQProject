#include "./SrvrLoadCfg.h"
ZQ::common::Config::Loader<SrvrLoad::SrvrLoadCfg> g_SrvrLoadCfg("ServerLoad.xml");
ZQ::common::Config::ILoader *configLoader = &g_SrvrLoadCfg;
/*
SrvrLoad::SrvrLoadCfg g_SrvrLoadCfg;
ZQ::common::ConfigLoader* configLoader = &g_SrvrLoadCfg;

namespace SrvrLoad
{

SrvrLoadCfg::SrvrLoadCfg()
{
}

SrvrLoadCfg::~SrvrLoadCfg()
{
}

ZQ::common::ConfigLoader::PConfigSchema SrvrLoadCfg::getSchema()
{
	return NULL; // 不依靠Config Loader读取配置信息
}

} // namespace SrvrLoad
*/
