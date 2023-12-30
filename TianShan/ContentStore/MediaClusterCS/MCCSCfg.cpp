
// ===========================================================================
// Copyright (c) 2006 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: CPEImpl.cpp $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
//
// ===========================================================================

#include "MCCSCfg.h"
#include "TianShanDefines.h"


using namespace ZQ::common;

void ClusterCSConfig::structure(Config::Holder<ClusterCSConfig>& holder) 
{
	holder.addDetail("", "netId", &ClusterCSConfig::netId, 0, Config::optReadOnly);
	holder.addDetail("", "rootPath", &ClusterCSConfig::rootPath, "", Config::optReadOnly);
	holder.addDetail("", "cacheMode", &ClusterCSConfig::isCacheMode, "0", Config::optReadOnly);
	holder.addDetail("", "cacheLevel", &ClusterCSConfig::cacheLevel, "10", Config::optReadOnly);
	holder.addDetail("", "threads", &ClusterCSConfig::workerThreadSize, "10", Config::optReadOnly);
	holder.addDetail("", "enableInServiceValidate", &ClusterCSConfig::enalbeInServiceCheck, "0", Config::optReadOnly);
    holder.addDetail("", "warningFreeSpacePercent", &ClusterCSConfig::warningFreeSpacePercent, "0");
    holder.addDetail("", "stepFreeSpacePercent", &ClusterCSConfig::stepFreeSpacePercent, "0");
        
    // c2 locator interface to export content via c2http://, the value should be in the format of <ip>:<port>
    holder.addDetail("", "exportedLocator", &ClusterCSConfig::exportLocatorInterface, "", Config::optReadOnly);

	holder.addDetail("Bind", "endpoint", &ClusterCSConfig::clusterEndpoint, DEFAULT_ENDPOINT_ContentStore, Config::optReadOnly);
	holder.addDetail("Bind", "dispatchSize", &ClusterCSConfig::dispatchSize, "5", Config::optReadOnly);
	holder.addDetail("Bind", "dispatchMax", &ClusterCSConfig::dispatchMax, "20", Config::optReadOnly);
	holder.addDetail("Provision", "defaultBandwidth", &ClusterCSConfig::defaultProvisionBW, "3750000", Config::optReadOnly);
	holder.addDetail("Provision", "trickSpeeds", &ClusterCSConfig::strTrickSpeeds, "7.5", Config::optReadOnly);

	holder.addDetail("Provision", "timeoutIdleProvisioning", &ClusterCSConfig::timeoutIdleProvisioning, "172800000", Config::optReadOnly);
	holder.addDetail("Provision", "timeoutNotProvisioned", &ClusterCSConfig::timeoutNotProvisioned, "600000", Config::optReadOnly);        
	holder.addDetail("Provision/NoTrickSpeeds",&ClusterCSConfig::readNoTrickSpeeds ,&ClusterCSConfig::registerNothing, Config::Range(0, 1));

	holder.addDetail("DatabaseCache", "volumeSize", &ClusterCSConfig::volumeEvictorSize, "20", Config::optReadOnly);
	holder.addDetail("DatabaseCache", "contentSize", &ClusterCSConfig::contentEvictorSize, "500", Config::optReadOnly);
	holder.addDetail("DatabaseCache", "contentSavePeriod", &ClusterCSConfig::contentSavePeriod, "4000", Config::optReadOnly);
	holder.addDetail("DatabaseCache", "contentSaveSizeTrigger", &ClusterCSConfig::contentSaveSizeTrigger, "150", Config::optReadOnly);

	/* IceProperties */
	holder.addDetail("IceProperties/prop", &ClusterCSConfig::readICEProp, &ClusterCSConfig::registerICEProp);

	holder.addDetail("PublishedLogs/Log", &ClusterCSConfig::readMonitoredLog, &ClusterCSConfig::registerNothing);

	holder.addDetail("Replica",	"replicaGroupId", &ClusterCSConfig::csStrReplicaGroupId, "");
	holder.addDetail("Replica", "replicaId", &ClusterCSConfig::csStrReplicaId, "");
	holder.addDetail("Replica", "replicaPriority", &ClusterCSConfig::csIReplicaPriority, "0");
	holder.addDetail("Replica", "timeout", &ClusterCSConfig::csReplicaTimeout, "60000");


	/* CPC items */
	holder.addDetail("CPC/Bind", "endpoint", &ClusterCSConfig::cpcEndPoint, 0, Config::optReadOnly);
	holder.addDetail("CPC/Sessions", "registerInterval", &ClusterCSConfig::registerInterval, "15000", Config::optReadOnly);
	holder.addDetail("CPC/Sessions", "DefaultIndexType", &ClusterCSConfig::strDefaultIndexType,"");

	//HashFolder items
	holder.addDetail("HashedFolder",  &ClusterCSConfig::readHashFolder,    &ClusterCSConfig::registerNothing);
	holder.addDetail("ExportURL", &ClusterCSConfig::readUrlExpression, &ClusterCSConfig::registerNothing);

	holder.addDetail("D4Speaker",&ClusterCSConfig::readD4Message,&ClusterCSConfig::registerNothing);
}

void ClusterCSConfig::readD4Message( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
{
	using namespace ZQ::common::Config;
	Holder<D4MessageConf> lmHolder;
	lmHolder.read(node, hPP);
	_d4messsage = lmHolder;
}

void ClusterCSConfig::readICEProp(XMLUtil::XmlNode node, const Preprocessor* hPP) 
{
	Config::Holder<Config::NVPair> propHolder;
	propHolder.read(node, hPP);
	iceProp[propHolder.name] = propHolder.value;
}

void ClusterCSConfig::registerICEProp(const std::string& path) 
{
}

void ClusterCSConfig::readMonitoredLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
{
	using namespace ZQ::common::Config;
	Holder<MonitoredLog> lmHolder;
	lmHolder.read(node, hPP);
	monitoredLogs.push_back(lmHolder);
}

void ClusterCSConfig::readHashFolder(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
{
	using namespace ZQ::common::Config;
	Holder<HashedFolder> hashFolderHolder;
	hashFolderHolder.read(node, hPP);
	hashFolder = hashFolderHolder;
}

void ClusterCSConfig::readUrlExpression(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
{
	using namespace ZQ::common::Config;
	Holder<UrlExpression> urlExprHolder;
	urlExprHolder.read(node, hPP);
	urlExpr = urlExprHolder;
}
void  ClusterCSConfig::readNoTrickSpeeds (ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<NoTrickSpeeds> noTrickSpeedsholder("");
	noTrickSpeedsholder.read(node, hPP);
	noTrickSpeeds = noTrickSpeedsholder;
}
////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void ConfigGroup::structure(Config::Holder<ConfigGroup> &holder) 
{		
	/* default */
	holder.addDetail("default/CrashDump", "path", &ConfigGroup::dumpPath, 0, Config::optReadOnly);
	holder.addDetail("default/CrashDump", "enabled", &ConfigGroup::dumpEnabled, "1", Config::optReadOnly);

	holder.addDetail("default/IceTrace", "enabled", &ConfigGroup::iceTraceEnabled, "1", Config::optReadOnly);
	holder.addDetail("default/IceTrace", "level", &ConfigGroup::iceTraceLevel, "7", Config::optReadWrite);
	holder.addDetail("default/IceTrace", "size", &ConfigGroup::iceTraceSize, "10240000", Config::optReadOnly);
	holder.addDetail("default/IceTrace", "fileCount", &ConfigGroup::iceTraceFileCount, "1", Config::optReadWrite);

	holder.addDetail("default/Database", "path", &ConfigGroup::dbPath, 0, Config::optReadOnly);

	holder.addDetail("ContentStore", &ConfigGroup::readConfigGroup, &ConfigGroup::registerConfigGroup);
};

void ConfigGroup::readConfigGroup(XMLUtil::XmlNode node, const Preprocessor* hPP) 
{
	Config::Holder<ClusterCSConfig> configGroupHolder;
	configGroupHolder.read(node, hPP);

	ConfigMap::iterator iter = configMap.find(configGroupHolder.netId);
	if(iter != configMap.end()) {
		throwf<CfgException>(EXFMT(CfgException, "Duplicate ContentStore instanceId: (%s)"), configGroupHolder.netId.c_str());
	}

	configMap[configGroupHolder.netId] = configGroupHolder;
}

void ConfigGroup::registerConfigGroup(const std::string& full_path) 
{
	for(ConfigMap::iterator it = configMap.begin(); it != configMap.end(); ++it) {
		(it->second).snmpRegister(full_path);
	}
	
}

bool ConfigGroup::setContentStoreNetId(const std::string& netId)
{  
    if (!configMap.size())
		return false;
	ConfigMap::iterator iter = configMap.begin();

	/* this is the only instance */
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
