
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


#ifndef _ZQTianShan_MCS_CFG_H_
#define _ZQTianShan_MCS_CFG_H_

#include "ConfigHelper.h"

struct MonitoredLog
{
	std::string name;
	std::string syntax;
	std::string syntaxKey;
	std::string logType;
	static void structure(ZQ::common::Config::Holder<MonitoredLog> &holder)
	{
		holder.addDetail("", "path", &MonitoredLog::name);
		holder.addDetail("", "syntax", &MonitoredLog::syntax);
		holder.addDetail("", "key", &MonitoredLog::syntaxKey);
		holder.addDetail("", "type", &MonitoredLog::logType);
	}
};

struct AdvertiseMethod
{
	std::string method;
	static void structure(ZQ::common::Config::Holder<AdvertiseMethod>& holder)
	{
	holder.addDetail("","name",&AdvertiseMethod::method);
	}
};

struct D4MessageConf
{
	std::string		listener;
	int32			enableD4;
	int32           advInterval;
	std::string		strA3Interface;
	std::string     strStreamZone;
	std::string		strRouteAddr;
	std::vector<AdvertiseMethod>  AdMethod;
	std::string     strVolumeId;
	int32			portId;
	std::string     serverName;
	std::string     serverIp;

	ZQ::common::Config::Holder<AdvertiseMethod> hoderAdMethods;
	void readAdvertiseMethod( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
	{
	hoderAdMethods.read(node , hPP);
	AdMethod.push_back(hoderAdMethods);
	}
		void registerNothing(const std::string&){}
	static void structure( ZQ::common::Config::Holder<D4MessageConf>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","enabled",&D4MessageConf::enableD4,"0",optReadOnly);
		holder.addDetail("","listener",&D4MessageConf::listener ,"" ,optReadOnly);
		holder.addDetail("","A3Interface",&D4MessageConf::strA3Interface,"",optReadOnly);
		holder.addDetail("","advInterval",&D4MessageConf::advInterval,"30000",optReadOnly);
		holder.addDetail("","zone",&D4MessageConf::strStreamZone,"ZQ",optReadOnly);
		holder.addDetail("","routeAddr",&D4MessageConf::strRouteAddr,"",optReadOnly);
		holder.addDetail("","volumeId",&D4MessageConf::strVolumeId,"",optReadOnly);
		holder.addDetail("","portId",&D4MessageConf::portId,"",optReadOnly);
		holder.addDetail("","serverIp",&D4MessageConf::serverIp,"",optReadOnly);
		holder.addDetail("","serverName",&D4MessageConf::serverName,"",optReadOnly);

		holder.addDetail("AdvertisedMethod",&D4MessageConf::readAdvertiseMethod,&D4MessageConf::registerNothing);
	}

};

struct UrlExpression
{
	std::string expression;
	static void structure(ZQ::common::Config::Holder<UrlExpression>& holder)
	{
		holder.addDetail("", "expression", &UrlExpression::expression , "");
	}
};

struct HashedFolder
{
	std::string folderName;
	static void structure(ZQ::common::Config::Holder<HashedFolder>& holder)
	{
		holder.addDetail("","folderName",&HashedFolder::folderName , "");
	}
};
struct Rule
{
	std::string contentname;

	static void structure(ZQ::common::Config::Holder<Rule > &holder)
	{
		holder.addDetail("", "contentName", &Rule::contentname, NULL,ZQ::common::Config::optReadOnly);
	}
};
struct NoTrickSpeeds
{
	int32 enable;
	std::vector<std::string>expressionList;
	static void structure(ZQ::common::Config::Holder<NoTrickSpeeds > &holder)
	{
		holder.addDetail("", "enable", &NoTrickSpeeds::enable, "0",ZQ::common::Config::optReadOnly);
		holder.addDetail("Rule",&NoTrickSpeeds::readRule, &NoTrickSpeeds::registerNothing);
	}
	NoTrickSpeeds()
	{
		enable = 0;
	};
	void readRule(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ZQ::common::Config::Holder<Rule> ruleholder("");
		ruleholder.read(node, hPP);
		expressionList.push_back(ruleholder.contentname);
	}
	void registerNothing(const std::string&){}
};
struct ClusterCSConfig
{
	std::string netId;
	std::string rootPath;
	int32 dispatchSize;
	int32 dispatchMax;
	int32 workerThreadSize;
	int32 contentEvictorSize;
	int32 volumeEvictorSize;

	std::string exportLocatorInterface;

	int32 contentSavePeriod;
	int32 contentSaveSizeTrigger;

	int32 isCacheMode;
    int32 cacheLevel;

    int32 enalbeInServiceCheck;

    // free space monitor
    int32 warningFreeSpacePercent;
    int32 stepFreeSpacePercent;

	int32 defaultProvisionBW;
	std::string			strTrickSpeeds;
	std::vector<float>  trickSpeedCollection;

	int32					timeoutIdleProvisioning;
	int32					timeoutNotProvisioned;

	std::string clusterEndpoint;

	std::string			csStrReplicaGroupId;
	std::string			csStrReplicaId;
	int32				csIReplicaPriority;
	int32				csReplicaTimeout;		//in milisecond


	/* <IceProperties> */
	typedef std::map<std::string, std::string> ICEProperties;
	ICEProperties iceProp;

	/* <Bind> */
	std::string cpcEndPoint;
	int registerInterval; // milli seconds
	std::string strDefaultIndexType;

	std::vector<MonitoredLog> monitoredLogs;
	D4MessageConf _d4messsage;
	HashedFolder   hashFolder;
	UrlExpression  urlExpr;

	NoTrickSpeeds  noTrickSpeeds;
	static void structure(ZQ::common::Config::Holder<ClusterCSConfig>& holder);

	void readICEProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerICEProp(const std::string& path) ;
	void readD4Message( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP );
	void readMonitoredLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP);
	void readHashFolder(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP);
	void readUrlExpression(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP);
	void readNoTrickSpeeds(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerNothing(const std::string&){}

};

typedef std::map<std::string, ZQ::common::Config::Holder<ClusterCSConfig> > ConfigMap;

struct ConfigGroup {

	std::string dumpPath;
	int32 dumpEnabled;

	int32 iceTraceEnabled;
	int32 iceTraceLevel;
	int32 iceTraceSize;
	int32 iceTraceFileCount;

	std::string dbPath;

	ZQ::common::Config::Holder<ClusterCSConfig>		mccsConfig;

	//////////////////////////////////////////////////////////////////////////
	//
	//	set contentstore netId, after this the mccsConfig is asigned to 
	//  right configugration
	bool setContentStoreNetId(const std::string& netId = "");

protected:
	static void structure(ZQ::common::Config::Holder<ConfigGroup>& holder);

	void readConfigGroup(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) ;

	void registerConfigGroup(const std::string& full_path);

private:
	ConfigMap configMap;
};

extern ZQ::common::Config::Loader<ConfigGroup>				configGroup;

#endif

