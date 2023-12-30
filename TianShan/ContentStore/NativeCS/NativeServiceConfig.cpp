#include "NativeServiceConfig.h"
#include "TianShanDefines.h"


Config::Loader<ConfigGroup> configGroup(CONFIGURATION_XML);

void NativeServiceConfig::structure(Config::Holder<NativeServiceConfig>& holder) {

	holder.addDetail("", "netId", &NativeServiceConfig::netId, 0, Config::optReadOnly);
	holder.addDetail("", "cacheMode", &NativeServiceConfig::isCacheMode, "0", Config::optReadOnly);
	holder.addDetail("", "cacheLevel", &NativeServiceConfig::cacheLevel, "10", Config::optReadOnly);
	holder.addDetail("", "threads", &NativeServiceConfig::workerThreadSize, "10", Config::optReadOnly);

	holder.addDetail("Bind", "endpoint", &NativeServiceConfig::clusterEndpoint, DEFAULT_ENDPOINT_ContentStore, Config::optReadOnly);
	holder.addDetail("Bind", "dispatchSize", &NativeServiceConfig::dispatchSize, "5", Config::optReadOnly);
	holder.addDetail("Bind", "dispatchMax", &NativeServiceConfig::dispatchMax, "20", Config::optReadOnly);
	holder.addDetail("Provision", "defaultBandwidth", &NativeServiceConfig::defaultProvisionBW, "3750000", Config::optReadOnly);
	holder.addDetail("Provision", "trickSpeeds", &NativeServiceConfig::strTrickSpeeds, "7.5", Config::optReadOnly);

	holder.addDetail("DatabaseCache", "volumeSize", &NativeServiceConfig::volumeEvictorSize, "20", Config::optReadOnly);
	holder.addDetail("DatabaseCache", "contentSize", &NativeServiceConfig::contentEvictorSize, "500", Config::optReadOnly);

	/* IceProperties */
	holder.addDetail("IceProperties/prop", &NativeServiceConfig::readICEProp, &NativeServiceConfig::registerICEProp);
	
	holder.addDetail("Volumes/volume", &NativeServiceConfig::readVolumes, &NativeServiceConfig::registerVolumes);

	/* CPC items */
	holder.addDetail("CPC/Bind", "endpoint", &NativeServiceConfig::cpcEndPoint, 0, Config::optReadOnly);
	holder.addDetail("CPC/Sessions", "registerInterval", &NativeServiceConfig::registerInterval, "15000", Config::optReadOnly);
}

void NativeServiceConfig::readVolumes(XMLUtil::XmlNode node, const Preprocessor* hPP) {
	Config::Holder<VolumeConfig> volumeHolder;
	volumeHolder.read(node, hPP);
	
	volumes.push_back(volumeHolder);
}

void NativeServiceConfig::registerVolumes(const std::string& path) {
}

void NativeServiceConfig::readICEProp(XMLUtil::XmlNode node, const Preprocessor* hPP) {
	Config::Holder<Config::NVPair> propHolder;
	propHolder.read(node, hPP);
	iceProp[propHolder.name] = propHolder.value;
}

void NativeServiceConfig::registerICEProp(const std::string& path) {
}

void ConfigGroup::structure(Config::Holder<ConfigGroup>& holder) {		

	holder.addDetail("default/IceTrace", "enabled", &ConfigGroup::iceTraceEnabled, "1", Config::optReadOnly);
	holder.addDetail("default/IceTrace", "level", &ConfigGroup::iceTraceLevel, "7", Config::optReadOnly);
	holder.addDetail("default/IceTrace", "size", &ConfigGroup::iceTraceSize, "10240000", Config::optReadOnly);

	holder.addDetail("default/Database", "path", &ConfigGroup::dbPath, 0, Config::optReadOnly);

	holder.addDetail("ContentStore", &ConfigGroup::readConfigGroup, &ConfigGroup::registerConfigGroup);
};

void ConfigGroup::readConfigGroup(XMLUtil::XmlNode node, const Preprocessor* hPP) {
	Config::Holder<NativeServiceConfig> configGroupHolder;
	configGroupHolder.read(node, hPP);

	ConfigMap::iterator iter = configMap.find(configGroupHolder.netId);
	if(iter != configMap.end()) {
		throwf<CfgException>(EXFMT(CfgException, "Duplicate ContentStore instanceId: (%s)"), configGroupHolder.netId.c_str());
	}

	configMap[configGroupHolder.netId] = configGroupHolder;
}

void ConfigGroup::registerConfigGroup(const std::string& full_path) {
	for(ConfigMap::iterator it = configMap.begin(); it != configMap.end(); ++it) {
		(it->second).snmpRegister(full_path);
	}
}

bool ConfigGroup::setContentStoreNetId(const std::string& netId) {
	ConfigMap::iterator iter = configMap.begin();
	if (!configMap.size())
		return false;

	if(!netId.empty()) 
	{
		iter = configMap.find(netId);
		if(iter == configMap.end()) 
		{		
			return false;
		}		
	}
	mccsConfig = iter->second;
	mccsConfig.snmpRegister("");
    
	
/* bug#16994, looks like istringstream behaves differently on windows and linux, replaced with str tokens
	//
	// do some initialization
	//
	std::istringstream iss(mccsConfig.strTrickSpeeds);
	float val = 0.0f;
	while(iss >> val) {
		mccsConfig.trickSpeedCollection.push_back(val);
	}
*/
	// parsing for the speed sequence
	TianShanIce::StrValues strtok;
	ZQTianShan::tokenize(strtok, mccsConfig.strTrickSpeeds.c_str(), " \t\n\r");
	for (TianShanIce::StrValues::iterator it = strtok.begin(); it < strtok.end(); it++)
	{
		if (it->empty())
			continue;
		
		float val = atof(it->c_str());
		mccsConfig.trickSpeedCollection.push_back(val);
	}

	return true;
}


