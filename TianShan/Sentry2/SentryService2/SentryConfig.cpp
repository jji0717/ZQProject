
#include "SentryConfig.h"
void TimeServerCfg::structure(ZQ::common::Config::Holder<TimeServerCfg> &holder)
{
	holder.addDetail("", "address", &TimeServerCfg::timeServerAddress, "",  ZQ::common::Config::optReadOnly);
	holder.addDetail("", "port", &TimeServerCfg::timeServerPort, "123",  ZQ::common::Config::optReadOnly);
}

void NTPClientCfg::structure(ZQ::common::Config::Holder<NTPClientCfg>& holder)
{
	holder.addDetail("", "enabled", &NTPClientCfg::ntpClientEnabled, "1",  ZQ::common::Config::optReadOnly);
	holder.addDetail("", "adjustTimeout", &NTPClientCfg::ntpClientAdjustTimeout, "",  ZQ::common::Config::optReadOnly);
	holder.addDetail("", "syncInterval", &NTPClientCfg::ntpClientSyncInterval, "",  ZQ::common::Config::optReadOnly);
	holder.addDetail("", "timeMaxOffset", &NTPClientCfg::timeMaxOffset, "",  ZQ::common::Config::optReadOnly);
	holder.addDetail("TimeServer", &NTPClientCfg::readTimeServer, &NTPClientCfg::registerTimeServer);
}

void NTPClientCfg::readTimeServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
{
	TimeServerHolder timeServer("address");
	timeServer.read(node, hPP);
	timeServerDatas.push_back(timeServer);
}

void NTPClientCfg::registerTimeServer(const std::string &full_path)
{
	std::vector<TimeServerHolder>::iterator iter = timeServerDatas.begin();
	for (; iter != timeServerDatas.end(); iter++)
	{
		iter->snmpRegister(full_path);
	}
}
