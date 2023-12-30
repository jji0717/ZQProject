#include "./ParseXMLData.h"
#include "./stroprt.h"
#include "./SrvrLoadEnv.h"
#include "Log.h"

#include <map>

namespace SrvrLoad
{

#define ParseFmt(_C, _X) CLOGFMT(_C, "(Sequence: %08I64d)" _X), _sequence

	
ParseXMLData::ParseXMLData(const std::string& filename, SrvrLoadEnv& env, ZQ::common::NativeThreadPool& pool, __int64 sequence, std::map<std::string, std::string>& properties)
	: _env(env), ZQ::common::ThreadRequest(pool), _pRoot(NULL), _fileName(filename), _sequence(sequence), _properties(properties)
{
}

ParseXMLData::~ParseXMLData()
{
	_xmlDoc.clear();
}

const db_datas& ParseXMLData::get_result() const
{
	return _dbDatas;
}

bool ParseXMLData::init(void)
{
	glog(InfoLevel, ParseFmt(ParseXMLData, "ParseXMLData::init()"));
	return true;
}

int ParseXMLData::run(void)
{
	glog(InfoLevel, ParseFmt(ParseXMLData, "ParseXMLData::run()"));
	bool bOpen = false;
	try
	{
		bOpen = _xmlDoc.open(_fileName.c_str());
	}
	catch (ZQ::common::XMLException& e)
	{
		glog(ErrorLevel, ParseFmt(ParseXMLData, "_xmlDoc.open(%s) failed, caught XMLException: %s"), _fileName.c_str(), e.getString());
		return 1;
	}
	catch (...)
	{
		glog(ErrorLevel, ParseFmt(ParseXMLData, "_xmlDoc.open(%s) failed, caught unexpect exception"), _fileName.c_str());
		return 1;
	}

	if (false == bOpen)
	{
		glog(ErrorLevel, ParseFmt(ParseXMLData, "_xmlDoc.open(%s) failed."), _fileName.c_str());
		return 1;
	}

	_pRoot = _xmlDoc.getRootPreference();
	SmartPreferenceEx smtRoot(_pRoot);
	if (NULL == _pRoot)
	{
		glog(ErrorLevel, ParseFmt(ParseXMLData, "get root failed"), _fileName.c_str());
		return 1;
	}

	ZQ::common::XMLPreferenceEx* pCMGroup = _pRoot->findSubPreference("CMGroup");
	SmartPreferenceEx smtCMGroup(pCMGroup);
	if (NULL == pCMGroup)
	{
		glog(ErrorLevel, ParseFmt(ParseXMLData, "get CMGroup failed"), _fileName.c_str());
		return 1;
	}

	std::map<std::string, std::string> prop_cmgroup = pCMGroup->getProperties();
	std::map<std::string, std::string>::const_iterator itor_prop = prop_cmgroup.find("CMGroupID");
	if (itor_prop != prop_cmgroup.end())
	{
		glog(DebugLevel, ParseFmt(ParseXMLData, "found CMGroupID in <CMGroup>, take %s as TianShan format"), _fileName.c_str());
//		parseTianShan();
		parseTianShan2();
	}
	else 
	{
		glog(DebugLevel, ParseFmt(ParseXMLData, "CMGroupID not found in <CMGroup>, take %s as Axiom format"), _fileName.c_str());
		parseAxiom();
	}

	_env.updateDB(_dbDatas, _sequence);

	return 0;
}

void ParseXMLData::final(int retcode, bool bCancelled)
{
	glog(InfoLevel, ParseFmt(ParseXMLData, "ParseXMLData::final()"));
	delete this;
}

/*
void ParseXMLData::parseTianShan()
{
	ote_servers_ts();
	ote_instances_ts();
	ote_groups_ts();
	ote_cm_apps_ts();
}
// */
void ParseXMLData::parseAxiom()
{
	ote_servers_axm();
	ote_instances_axm();
	ote_groups_axm();
	ote_cm_apps_axm();
}

void ParseXMLData::parseTianShan2()
{
	_dbDatas.erase_ote_servers();
	glog(DebugLevel, ParseFmt(ParseXMLData, "do ote_servers_ts()"));

	char szBuf[100];
	memset(szBuf, 0, sizeof(szBuf));
	std::string sVersion, sDataTime, sInterval;
	ZQ::common::XMLPreferenceEx* pHeader = _pRoot->findSubPreference("Header");
	SmartPreferenceEx smtHeader(pHeader);
	if (NULL != pHeader)
	{
		ZQ::common::XMLPreferenceEx* pVersion = pHeader->findSubPreference("Version");
		SmartPreferenceEx smtVersion(pVersion);
		if (NULL != pVersion)
		{
			if (true == pVersion->getPreferenceText(szBuf, sizeof(szBuf) - 1))
				sVersion = NULL != szBuf ? szBuf : "";
		}
		ZQ::common::XMLPreferenceEx* pDateTime = pHeader->findSubPreference("DateTime");
		SmartPreferenceEx smtDataTime(pDateTime);
		if (NULL != pDateTime)
		{
			if (true == pDateTime->getPreferenceText(szBuf, sizeof(szBuf) - 1))
				sDataTime = NULL != szBuf ? szBuf : ""; // format: YYYY-MM-DDThh:mm:ss
		}
		ZQ::common::XMLPreferenceEx* pInterval = pHeader->findSubPreference("Interval");
		SmartPreferenceEx smtInterval(pInterval);
		if (NULL != pInterval)
		{
			if (true == pInterval->getPreferenceText(szBuf, sizeof(szBuf) - 1))
				sInterval = NULL != szBuf ? szBuf : "";
		}
	}

	ZQ::common::XMLPreferenceEx* pCmGroup = _pRoot->firstChild("CMGroup");
	SmartPreferenceEx smtCmGroup(pCmGroup);
	while (NULL != pCmGroup)
	{
		parseTianShanCmGroup(sVersion, sDataTime, sInterval, pCmGroup);

		// search for next child
		if (true == _pRoot->hasNextChild())
		{
			ZQ::common::XMLPreferenceEx* pTemp = pCmGroup;
			SmartPreferenceEx smtTemp(pTemp);
			pCmGroup = _pRoot->nextChild();
		}
		else pCmGroup =NULL;
	}
}

void ParseXMLData::parseTianShanCmGroup(const std::string& sVersion, const std::string&sDataTime, const std::string&sInterval, ZQ::common::XMLPreferenceEx* pCmGroup)
{
	char szBuf[256];
	std::map<std::string, std::string> cmGroupProps = pCmGroup->getProperties();
	std::map<std::string, std::string>::iterator cmGroup_itor;

	ote_server_rec srvrRec;
	memset(&srvrRec, 0, sizeof(srvrRec));

	cmGroup_itor = cmGroupProps.find("CMGroupID");

	if (cmGroupProps.end() == cmGroup_itor || cmGroup_itor->second.empty())
	{
		glog(ErrorLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> no CMGroupID found or empty."));
		return;
	}

	srvrRec.cm_group_id = atoi(cmGroup_itor->second.c_str());
		
	// get server type
	cmGroup_itor = cmGroupProps.find("ServerType");
	if (cmGroupProps.end() != cmGroup_itor && 0 != strlen(cmGroup_itor->second.c_str()))
		srvrRec.server_type = atoi(cmGroup_itor->second.c_str());
	else 
	{
		srvrRec.bServerTypeEmpty = true;
		glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> no ServerType found or value empty."));
	}
	
	// get interval			
	if (false == sInterval.empty())
		srvrRec.interval_time = atoi(sInterval.c_str());
	else 
	{
		srvrRec.bIntervalTimeEmpty = true;
		glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <Header> no <Interval> found or content empty."));
	}
	
	snprintf(srvrRec.version, sizeof(srvrRec.version) - 1, "%s", sVersion.c_str());
	snprintf(srvrRec.data_time, sizeof(srvrRec.data_time) - 1, "%s", sDataTime.c_str());
	
	// get pg level
	ZQ::common::XMLPreferenceEx* pPGLevel = pCmGroup->findSubPreference("PGLevel");
	SmartPreferenceEx smtPGLevel(pPGLevel);
	if (NULL != pPGLevel)
	{
		std::string sPGLevel;
		if (true == pPGLevel->getPreferenceText(szBuf, sizeof(szBuf) - 1))
			sPGLevel = NULL != szBuf ? szBuf : "";
		if (false == sPGLevel.empty())
			srvrRec.pg_level = atoi(sPGLevel.c_str());
		else 
		{
			srvrRec.bPGLevelEmpty = true;
			glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> <PGLevel> content empty."));
		}
	}
	else 
	{
		srvrRec.bPGLevelEmpty = true;
		glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> no <PGLevel> found."));
	}
		
	glog(DebugLevel, ParseFmt(ParseXMLData, "(ote_server_record)"
		"cm_group_id: %d;"
		"server_type: %d;"
		"interval_time: %d;"
		"pg_level: %d;"
		"version: %s;"
		"data_time: %s")
		, srvrRec.cm_group_id
		, srvrRec.server_type
		, srvrRec.interval_time
		, srvrRec.pg_level
		, srvrRec.version
		, srvrRec.data_time);
	
	_dbDatas.add_ote_server_rec(srvrRec);
	
	ZQ::common::XMLPreferenceEx* pInstance = pCmGroup->firstChild("Instance");
	SmartPreferenceEx smtInstance(pInstance);
	while (NULL != pInstance)
	{
		parseTianShanInstance(srvrRec, pInstance);
		
		// search for next child
		if (pCmGroup->hasNextChild())
		{
			ZQ::common::XMLPreferenceEx* pTemp = pInstance;
			SmartPreferenceEx smtTemp(pTemp);
			pInstance = pCmGroup->nextChild();
		}
		else pInstance = NULL;
	}

	ZQ::common::XMLPreferenceEx* pNodeGroup = pCmGroup->firstChild("NodeGroup");
	SmartPreferenceEx smtNodeGroup(pNodeGroup);
	while (NULL != pNodeGroup)
	{
		parseTianShanNodeGroup(srvrRec, pNodeGroup);

		// search for next child
		if (pCmGroup->hasNextChild())
		{
			ZQ::common::XMLPreferenceEx* pTemp = pNodeGroup;
			SmartPreferenceEx smtTemp(pTemp);
			pNodeGroup = pCmGroup->nextChild();
		}
		else pNodeGroup = NULL;

	}

	ZQ::common::XMLPreferenceEx* pAppType = pCmGroup->firstChild("AppType");
	SmartPreferenceEx smtAppType(pAppType);
	while (NULL != pAppType)
	{
		parseTianShanApp(srvrRec, pAppType);
		
		// search for next child
		if (true == pCmGroup->hasNextChild())
		{
			ZQ::common::XMLPreferenceEx* pTemp = pAppType;
			SmartPreferenceEx smtTemp(pTemp);
			pAppType = pCmGroup->nextChild();
		}
		else pAppType = NULL;
	}
}

void ParseXMLData::parseTianShanInstance(ote_server_rec& cmGroupInfo, ZQ::common::XMLPreferenceEx* pInstance)
{
	char szBuf[256];
	std::map<std::string, std::string> instProps = pInstance->getProperties();
	
	ote_instance_rec instRec;
	memset(&instRec, 0, sizeof(instRec));
				
	instRec.cm_group_id = cmGroupInfo.cm_group_id;
				
	snprintf(instRec.session_protocal_type, sizeof(instRec.session_protocal_type) - 1, "%s", instProps["SessionProtocolType"].c_str());
				
	ZQ::common::XMLPreferenceEx* pIPAddress = pInstance->findSubPreference("IPAddress");
	SmartPreferenceEx smtIPAddress(pIPAddress);
	if (NULL != pIPAddress)
	{
		if (true == pIPAddress->getPreferenceText(szBuf, sizeof(szBuf) - 1))
			snprintf(instRec.ip_address, sizeof(instRec.ip_address) - 1, "%s", szBuf);
	}
				
	ZQ::common::XMLPreferenceEx* pHostName = pInstance->findSubPreference("HostName");
	SmartPreferenceEx smtHostName(pHostName);
	if (NULL != pHostName)
	{
		if (true == pHostName->getPreferenceText(szBuf, sizeof(szBuf) - 1))
			snprintf(instRec.host_name, sizeof(instRec.host_name) - 1, "%s", szBuf);
	}

	if (NULL == pIPAddress && NULL == pHostName)
	{
		glog(ErrorLevel, ParseFmt(ParseXMLData, "TianShan format, invalid <CMGroup::Instance> no IP and no Hosthame, skip"));
		return;
	}
				
	// Since the "port" in Axiom generated server load xml file does not represents
	// the RTSP or DSMCC port for streaming control, it is just an internal port.
	// So the "port" also is configurable, and if configured, ServerLoad Service
	// must use the configured value to update OTE database, instead of the one
	// read from server load xml. And this is effective for TianShan. referenced from disign document.
	ZQ::common::XMLPreferenceEx* pPort = pInstance->findSubPreference("Port");
	SmartPreferenceEx smtPort(pPort);
	if (NULL != pPort)
	{
		std::string sPort;
		if (true == pPort->getPreferenceText(szBuf, sizeof(szBuf) - 1))
			sPort = NULL != szBuf ? szBuf : "";
	
		if (false == sPort.empty())
			instRec.port = atoi(sPort.c_str());
		else 
		{
			instRec.bPortEmpty = true;
			glog(ErrorLevel, ParseFmt(ParseXMLData, "TianShan format, <Instance> <Port> content empty."));
		}
	}
	else 
	{
		instRec.bPortEmpty = true;
		glog(ErrorLevel, ParseFmt(ParseXMLData, "TianShan format, <Instance> no <Port> found."));
	}
				
	// if Port found in configuration<ServerLoad><WatchList><File Port="">
	// and it's value larger than 0, use it.
	std::map<std::string, std::string>::iterator port_itor = _properties.find("Port");
	if (_properties.end() != port_itor && atoi(port_itor->second.c_str()) > 0)
	{
		instRec.bPortEmpty = false;
		instRec.port = atoi(port_itor->second.c_str());
	}
				
	ZQ::common::XMLPreferenceEx* pLoad = pInstance->findSubPreference("Load");
	SmartPreferenceEx smtLoad(pLoad);
	if (NULL != pLoad)
	{
		std::string sLoad;
		if (true == pLoad->getPreferenceText(szBuf, sizeof(szBuf) - 1))
			sLoad = NULL != szBuf ? szBuf : "";
		if (false == sLoad.empty())
			instRec.server_load = atoi(sLoad.c_str());
		else 
		{
			instRec.bServerLoadEmpty = true;
			glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <Instance> <Load> content empty"));
		}
	}
	else 
	{
		instRec.bServerLoadEmpty = true;
		glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <Instance> no <Load> found"));
	}
				
	ZQ::common::XMLPreferenceEx* pLifeTime = pInstance->findSubPreference("LifeTime");
	SmartPreferenceEx smtLifeTime(pLifeTime);
	if (NULL != pLifeTime)
	{
		std::string sLifeTime;
		if (true == pLifeTime->getPreferenceText(szBuf, sizeof(szBuf) - 1))
			sLifeTime = NULL != szBuf ? szBuf : "";
		if (false == sLifeTime.empty())
			instRec.life_time = atoi(sLifeTime.c_str());
		else 
		{
			instRec.bLifeTimeEmpty = true;
			glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <Instance> <LifeTime> content empty"));
		}
	}
	else 
	{
		instRec.bLifeTimeEmpty = true;
		glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <Instance> no <LifeTime> found"));
	}

	if (instRec.bLifeTimeEmpty)
	{
		instRec.life_time = cmGroupInfo.interval_time;
		instRec.bLifeTimeEmpty = cmGroupInfo.bIntervalTimeEmpty;
		glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <Instance> has no <LifeTime>, use CMGroup's instead: %d"), instRec.life_time);
	}
				
	glog(DebugLevel, ParseFmt(ParseXMLData, "(ote_instance_record)"
		"cm_group_id: %d;"
		"host_name: %s;"
		"session_protocal_type: %s;"
		"ip_address: %s;"
		"port: %d;"
		"server_load: %d;"
		"life_time: %d"),
		instRec.cm_group_id,
		instRec.host_name
		, instRec.session_protocal_type
		, instRec.ip_address
		, instRec.port
		, instRec.server_load
		, instRec.life_time);
				
	_dbDatas.add_ote_instance_rec(instRec);
}

void ParseXMLData::parseTianShanNodeGroup(ote_server_rec& cmGroupInfo, ZQ::common::XMLPreferenceEx* pNodeGroup)
{
	char szBuf[256];
	ote_group_rec groupRec;
	memset(&groupRec, 0, sizeof(groupRec));
				
	groupRec.cm_group_id = cmGroupInfo.cm_group_id;
				
	std::string sNodeGroup;
	if (true == pNodeGroup->getPreferenceText(szBuf, sizeof(szBuf) - 1))
		sNodeGroup = NULL != szBuf ? szBuf : "";

	if (sNodeGroup.empty())
	{
		glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> <NodeGroup> content empty."));
		return;
	}

	sscanf(sNodeGroup.c_str(), "%I64d", &groupRec.node_group);
	glog(DebugLevel, ParseFmt(ParseXMLData, "(ote_group_record)"
						"cm_group_id: %d;"
						"node_group: %I64d")
						, groupRec.cm_group_id
						, groupRec.node_group);					
	_dbDatas.add_ote_group_rec(groupRec);
}

void ParseXMLData::parseTianShanApp(ote_server_rec& cmGroupInfo, ZQ::common::XMLPreferenceEx* pAppType)
{
	char szBuf[256];
	ote_cm_app_rec cmAppRec;
	memset(&cmAppRec, 0, sizeof(cmAppRec));
				
	cmAppRec.cm_group_id = cmGroupInfo.cm_group_id;
	
	if (pAppType->getPreferenceText(szBuf, sizeof(szBuf) - 1))
		snprintf(cmAppRec.app_type, sizeof(cmAppRec.app_type) - 1, "%s", szBuf);
				
	if (0 == strlen(cmAppRec.app_type))
	{
		glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> <AppType> content empty."));
		return;
	}

	glog(DebugLevel, ParseFmt(ParseXMLData, "(ote_cm_app_record)"
			"cm_group_id: %d;"
			"app_type: %s")
			, cmAppRec.cm_group_id
			, cmAppRec.app_type);
	
	_dbDatas.add_ote_cm_app_rec(cmAppRec);
}

/*

void ParseXMLData::ote_servers_ts()
{
	_dbDatas.erase_ote_servers();
	glog(DebugLevel, ParseFmt(ParseXMLData, "do ote_servers_ts()"));
	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));
	std::string sVersion, sDataTime, sInterval;
	ZQ::common::XMLPreferenceEx* pHeader = _pRoot->findSubPreference("Header");
	SmartPreferenceEx smtHeader(pHeader);
	if (NULL != pHeader)
	{
		ZQ::common::XMLPreferenceEx* pVersion = pHeader->findSubPreference("Version");
		SmartPreferenceEx smtVersion(pVersion);
		if (NULL != pVersion)
		{
			if (true == pVersion->getPreferenceText(szBuf, sizeof(szBuf) - 1))
				sVersion = NULL != szBuf ? szBuf : "";
		}
		ZQ::common::XMLPreferenceEx* pDateTime = pHeader->findSubPreference("DateTime");
		SmartPreferenceEx smtDataTime(pDateTime);
		if (NULL != pDateTime)
		{
			if (true == pDateTime->getPreferenceText(szBuf, sizeof(szBuf) - 1))
				sDataTime = NULL != szBuf ? szBuf : ""; // format: YYYY-MM-DDThh:mm:ss
		}
		ZQ::common::XMLPreferenceEx* pInterval = pHeader->findSubPreference("Interval");
		SmartPreferenceEx smtInterval(pInterval);
		if (NULL != pInterval)
		{
			if (true == pInterval->getPreferenceText(szBuf, sizeof(szBuf) - 1))
				sInterval = NULL != szBuf ? szBuf : "";
		}
	}

	ZQ::common::XMLPreferenceEx* pCmGroup = _pRoot->firstChild("CMGroup");
	SmartPreferenceEx smtCmGroup(pCmGroup);
	while (NULL != pCmGroup)
	{
		std::map<std::string, std::string> cmGroupProps = pCmGroup->getProperties();
		std::map<std::string, std::string>::iterator cmGroup_itor;

		ote_server_rec srvrRec;
		memset(&srvrRec, 0, sizeof(srvrRec));

		cmGroup_itor = cmGroupProps.find("CMGroupID");

		if (cmGroupProps.end() != cmGroup_itor && 0 != strlen(cmGroup_itor->second.c_str()))
		{
			srvrRec.cm_group_id = atoi(cmGroup_itor->second.c_str());

			// get server type
			cmGroup_itor = cmGroupProps.find("ServerType");
			if (cmGroupProps.end() != cmGroup_itor && 0 != strlen(cmGroup_itor->second.c_str()))
				srvrRec.server_type = atoi(cmGroup_itor->second.c_str());
			else 
			{
				srvrRec.bServerTypeEmpty = true;
				glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> no ServerType found or value empty."));
			}

			// get interval			
			if (false == sInterval.empty())
				srvrRec.interval_time = atoi(sInterval.c_str());
			else 
			{
				srvrRec.bIntervalTimeEmpty = true;
				glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <Header> no <Interval> found or content empty."));
			}

			snprintf(srvrRec.version, sizeof(srvrRec.version) - 1, "%s", sVersion.c_str());
			snprintf(srvrRec.data_time, sizeof(srvrRec.data_time) - 1, "%s", sDataTime.c_str());

			// get pg level
			ZQ::common::XMLPreferenceEx* pPGLevel = pCmGroup->findSubPreference("PGLevel");
			SmartPreferenceEx smtPGLevel(pPGLevel);
			if (NULL != pPGLevel)
			{
				std::string sPGLevel;
				if (true == pPGLevel->getPreferenceText(szBuf, sizeof(szBuf) - 1))
					sPGLevel = NULL != szBuf ? szBuf : "";
				if (false == sPGLevel.empty())
					srvrRec.pg_level = atoi(sPGLevel.c_str());
				else 
				{
					srvrRec.bPGLevelEmpty = true;
					glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> <PGLevel> content empty."));
				}
			}
			else 
			{
				srvrRec.bPGLevelEmpty = true;
				glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> no <PGLevel> found."));
			}

			glog(DebugLevel, ParseFmt(ParseXMLData, "(ote_server_record)"
				"cm_group_id: %d;"
				"server_type: %d;"
				"interval_time: %d;"
				"pg_level: %d;"
				"version: %s;"
				"data_time: %s")
				, srvrRec.cm_group_id
				, srvrRec.server_type
				, srvrRec.interval_time
				, srvrRec.pg_level
				, srvrRec.version
				, srvrRec.data_time);

			_dbDatas.add_ote_server_rec(srvrRec);
		}
		else 
			glog(ErrorLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> no CMGroupID found or empty."));
		
		// search for next child
		if (true == _pRoot->hasNextChild())
		{
			ZQ::common::XMLPreferenceEx* pTemp = pCmGroup;
			SmartPreferenceEx smtTemp(pTemp);
			pCmGroup = _pRoot->nextChild();
		}
		else break;
	}
}

void ParseXMLData::ote_instances_ts()
{
	_dbDatas.erase_ote_instances();
	glog(DebugLevel, ParseFmt(ParseXMLData, "do ote_instances_ts()"));
	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));
	ZQ::common::XMLPreferenceEx* pCmGroup = _pRoot->firstChild("CMGroup");
	SmartPreferenceEx smtCMGroup(pCmGroup);
	while (NULL != pCmGroup)
	{
		std::map<std::string, std::string> cmGroupProps = pCmGroup->getProperties();
		std::map<std::string, std::string>::iterator cmGroup_itor;

		cmGroup_itor = cmGroupProps.find("CMGroupID");
		if (cmGroupProps.end() != cmGroup_itor && 0 != strlen(cmGroup_itor->second.c_str()))
		{
			int iCMGroupID = atoi(cmGroupProps["CMGroupID"].c_str());

			ZQ::common::XMLPreferenceEx* pInstance = pCmGroup->firstChild("Instance");
			SmartPreferenceEx smtInstance(pInstance);
			while (NULL != pInstance)
			{
				std::map<std::string, std::string> instProps = pInstance->getProperties();

				ote_instance_rec instRec;
				memset(&instRec, 0, sizeof(instRec));

				instRec.cm_group_id = iCMGroupID;

				snprintf(instRec.session_protocal_type, sizeof(instRec.session_protocal_type) - 1, "%s", instProps["SessionProtocolType"].c_str());

				ZQ::common::XMLPreferenceEx* pIPAddress = pInstance->findSubPreference("IPAddress");
				SmartPreferenceEx smtIPAddress(pIPAddress);
				if (NULL != pIPAddress)
				{
					if (true == pIPAddress->getPreferenceText(szBuf, sizeof(szBuf) - 1))
						snprintf(instRec.ip_address, sizeof(instRec.ip_address) - 1, "%s", szBuf);
				}

				ZQ::common::XMLPreferenceEx* pHostName = pInstance->findSubPreference("HostName");
				SmartPreferenceEx smtHostName(pHostName);
				if (NULL != pHostName)
				{
					if (true == pHostName->getPreferenceText(szBuf, sizeof(szBuf) - 1))
						snprintf(instRec.host_name, sizeof(instRec.host_name) - 1, "%s", szBuf);
				}

				// Since the "port" in Axiom generated server load xml file does not represents
				// the RTSP or DSMCC port for streaming control, it is just an internal port.
				// So the "port" also is configurable, and if configured, ServerLoad Service
				// must use the configured value to update OTE database, instead of the one
				// read from server load xml. And this is effective for TianShan. referenced from disign document.
				ZQ::common::XMLPreferenceEx* pPort = pInstance->findSubPreference("Port");
				SmartPreferenceEx smtPort(pPort);
				if (NULL != pPort)
				{
					std::string sPort;
					if (true == pPort->getPreferenceText(szBuf, sizeof(szBuf) - 1))
						sPort = NULL != szBuf ? szBuf : "";
					if (false == sPort.empty())
						instRec.port = atoi(sPort.c_str());
					else 
					{
						instRec.bPortEmpty = true;
						glog(ErrorLevel, ParseFmt(ParseXMLData, "TianShan format, <Instance> <Port> content empty."));
					}
				}
				else 
				{
					instRec.bPortEmpty = true;
					glog(ErrorLevel, ParseFmt(ParseXMLData, "TianShan format, <Instance> no <Port> found."));
				}
				
				// if Port found in configuration<ServerLoad><WatchList><File Port="">
				// and it's value larger than 0, use it.
				std::map<std::string, std::string>::iterator port_itor = _properties.find("Port");
				if (_properties.end() != port_itor && atoi(port_itor->second.c_str()) > 0)
				{
					instRec.bPortEmpty = false;
					instRec.port = atoi(port_itor->second.c_str());
				}

				ZQ::common::XMLPreferenceEx* pLoad = pInstance->findSubPreference("Load");
				SmartPreferenceEx smtLoad(pLoad);
				if (NULL != pLoad)
				{
					std::string sLoad;
					if (true == pLoad->getPreferenceText(szBuf, sizeof(szBuf) - 1))
						sLoad = NULL != szBuf ? szBuf : "";
					if (false == sLoad.empty())
						instRec.server_load = atoi(sLoad.c_str());
					else 
					{
						instRec.bServerLoadEmpty = true;
						glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <Instance> <Load> content empty"));
					}
				}
				else 
				{
					instRec.bServerLoadEmpty = true;
					glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <Instance> no <Load> found"));
				}

				ZQ::common::XMLPreferenceEx* pLifeTime = pInstance->findSubPreference("LifeTime");
				SmartPreferenceEx smtLifeTime(pLifeTime);
				if (NULL != pLifeTime)
				{
					std::string sLifeTime;
					if (true == pLifeTime->getPreferenceText(szBuf, sizeof(szBuf) - 1))
						sLifeTime = NULL != szBuf ? szBuf : "";
					if (false == sLifeTime.empty())
						instRec.life_time = atoi(sLifeTime.c_str());
					else 
					{
						instRec.bLifeTimeEmpty = true;
						glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <Instance> <LifeTime> content empty"));
					}
				}
				else 
				{
					instRec.bLifeTimeEmpty = true;
					glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <Instance> no <LifeTime> found"));
				}

				glog(DebugLevel, ParseFmt(ParseXMLData, "(ote_instance_record)"
					"cm_group_id: %d;"
					"host_name: %s;"
					"session_protocal_type: %s;"
					"ip_address: %s;"
					"port: %d;"
					"server_load: %d;"
					"life_time: %d")
					, instRec.cm_group_id
					, instRec.host_name
					, instRec.session_protocal_type
					, instRec.ip_address
					, instRec.port
					, instRec.server_load
					, instRec.life_time);

				_dbDatas.add_ote_instance_rec(instRec);

				// search for next child
				if (true == pCmGroup->hasNextChild())
				{
					ZQ::common::XMLPreferenceEx* pTemp = pInstance;
					SmartPreferenceEx smtTemp(pTemp);
					pInstance = pCmGroup->nextChild();
				}
				else break;
			}
		}
		else 
			glog(ErrorLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> no CMGroupID found or value empty."));

		// search for next child
		if (true == _pRoot->hasNextChild())
		{
			ZQ::common::XMLPreferenceEx* pTemp = pCmGroup;
			SmartPreferenceEx smtTemp(pTemp);
			pCmGroup = _pRoot->nextChild();
		}
		else break;
	}
}

void ParseXMLData::ote_groups_ts()
{
	_dbDatas.erase_ote_groups();
	glog(DebugLevel, ParseFmt(ParseXMLData, "do ote_groups_ts()"));
	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));
	ZQ::common::XMLPreferenceEx* pCmGroup = _pRoot->firstChild("CMGroup");
	SmartPreferenceEx smtCmGroup(pCmGroup);
	while (NULL != pCmGroup)
	{
		std::map<std::string, std::string> cmGroupProps = pCmGroup->getProperties();
		std::map<std::string, std::string>::iterator cmGroup_itor;

		cmGroup_itor = cmGroupProps.find("CMGroupID");
		if (cmGroupProps.end() != cmGroup_itor && 0 != strlen(cmGroup_itor->second.c_str()))
		{
			ZQ::common::XMLPreferenceEx* pNodeGroup = pCmGroup->firstChild("NodeGroup");
			SmartPreferenceEx smtNodeGroup(pNodeGroup);
			while (NULL != pNodeGroup)
			{
				ote_group_rec groupRec;
				memset(&groupRec, 0, sizeof(groupRec));
				
				groupRec.cm_group_id = atoi(cmGroupProps["CMGroupID"].c_str());
				
				std::string sNodeGroup;
				if (true == pNodeGroup->getPreferenceText(szBuf, sizeof(szBuf) - 1))
					sNodeGroup = NULL != szBuf ? szBuf : "";

				if (false == sNodeGroup.empty())
				{
					sscanf(sNodeGroup.c_str(), "%I64d", &groupRec.node_group);
					glog(DebugLevel, ParseFmt(ParseXMLData, "(ote_group_record)"
						"cm_group_id: %d;"
						"node_group: %I64d")
						, groupRec.cm_group_id
						, groupRec.node_group);					
					_dbDatas.add_ote_group_rec(groupRec);
				}
				else 
					glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> <NodeGroup> content empty."));

				// search for next child
				if (true == pCmGroup->hasNextChild())
				{
					ZQ::common::XMLPreferenceEx* pTemp = pNodeGroup;
					SmartPreferenceEx smtTemp(pTemp);
					pNodeGroup = pCmGroup->nextChild();
				}
				else break;
			}
		}
		else 
			glog(ErrorLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> no CMGroupID found or value empty."));

		// search for next child
		if (true == _pRoot->hasNextChild())
		{
			ZQ::common::XMLPreferenceEx* pTemp = pCmGroup;
			SmartPreferenceEx smtTemp(pTemp);
			pCmGroup = _pRoot->nextChild();
		}
		else break;
	}
}

void ParseXMLData::ote_cm_apps_ts()
{
	_dbDatas.erase_ote_cm_apps();
	glog(DebugLevel, ParseFmt(ParseXMLData, "do ote_cm_apps_ts()"));
	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));
	ZQ::common::XMLPreferenceEx* pCmGroup = _pRoot->firstChild("CMGroup");
	SmartPreferenceEx smtCmGroup(pCmGroup);
	while (NULL != pCmGroup)
	{
		std::map<std::string, std::string> cmGroupProps = pCmGroup->getProperties();
		std::map<std::string, std::string>::iterator cmGroup_itor;

		cmGroup_itor = cmGroupProps.find("CMGroupID");
		if (cmGroupProps.end() != cmGroup_itor && 0 != strlen(cmGroup_itor->second.c_str()))
		{
			ZQ::common::XMLPreferenceEx* pAppType = pCmGroup->firstChild("AppType");
			SmartPreferenceEx smtAppType(pAppType);
			while (NULL != pAppType)
			{
				ote_cm_app_rec cmAppRec;
				memset(&cmAppRec, 0, sizeof(cmAppRec));

				cmAppRec.cm_group_id = atoi(cmGroupProps["CMGroupID"].c_str());

				if (true == pAppType->getPreferenceText(szBuf, sizeof(szBuf) - 1))
					snprintf(cmAppRec.app_type, sizeof(cmAppRec.app_type) - 1, "%s", szBuf);

				if (0 != strlen(cmAppRec.app_type))
				{
					glog(DebugLevel, ParseFmt(ParseXMLData, "(ote_cm_app_record)"
						"cm_group_id: %d;"
						"app_type: %s")
						, cmAppRec.cm_group_id
						, cmAppRec.app_type);
					_dbDatas.add_ote_cm_app_rec(cmAppRec);
				}
				else 
					glog(InfoLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> <AppType> content empty."));

				// search for next child
				if (true == pCmGroup->hasNextChild())
				{
					ZQ::common::XMLPreferenceEx* pTemp = pAppType;
					SmartPreferenceEx smtTemp(pTemp);
					pAppType = pCmGroup->nextChild();
				}
				else break;
			}
		}
		else 
			glog(ErrorLevel, ParseFmt(ParseXMLData, "TianShan format, <CMGroup> no CMGroupID found or value empty."));

		// search for next child
		if (true == _pRoot->hasNextChild())
		{
			ZQ::common::XMLPreferenceEx* pTemp = pCmGroup;
			SmartPreferenceEx smtTemp(pTemp);
			pCmGroup = _pRoot->nextChild();
		}
		else break;
	}
}
// */

void ParseXMLData::ote_servers_axm()
{
	glog(DebugLevel, ParseFmt(ParseXMLData, "do ote_servers_axm()"));
	
	_dbDatas.erase_ote_servers();
	
	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));
	
	std::string sVersion, sDataTime, sInterval;
	ZQ::common::XMLPreferenceEx* pHeader = _pRoot->findSubPreference("Header");
	SmartPreferenceEx smtHeader(pHeader);
	if (NULL != pHeader)
	{
		ZQ::common::XMLPreferenceEx* pVersion = pHeader->findSubPreference("Version");
		SmartPreferenceEx smtVersion(pVersion);
		if (NULL != pVersion)
		{
			if (true == pVersion->getPreferenceText(szBuf, sizeof(szBuf) - 1))
				sVersion = NULL != szBuf ? szBuf : "";
		}
		
		ZQ::common::XMLPreferenceEx* pDateTime = pHeader->findSubPreference("DateTime");
		SmartPreferenceEx smtDateTime(pDateTime);
		if (NULL != pDateTime)
		{
			if (true == pDateTime->getPreferenceText(szBuf, sizeof(szBuf) - 1))
				sDataTime = NULL != szBuf ? szBuf : ""; // format: YY.MM.DD.HH.MM.SS
			std::vector<std::string> strs;
			String::splitStr(sDataTime, ".", strs);
			if (strs.size() == 6) // format to: YYYY-MM-DDThh:mm:ss
			{
				if (strs[0].size() == 2) // if format: YY.MM.DD.HH.MM.SS not YYYY.MM.DD.HH.MM.SS
				{
					SYSTEMTIME time;
					GetLocalTime(&time);
					int century = time.wYear / 100;
					char yearBuff[5];
					snprintf(yearBuff, sizeof(yearBuff) - 1, "%02d%s", century, strs[0].c_str());
					strs[0] = yearBuff;
				}
				snprintf(szBuf, sizeof(szBuf) - 1, "%04d-%02d-%02dT%02d:%02d:%02d"
					, atoi(strs[0].c_str())
					, atoi(strs[1].c_str())
					, atoi(strs[2].c_str())
					, atoi(strs[3].c_str())
					, atoi(strs[4].c_str())
					, atoi(strs[5].c_str()));
				sDataTime = szBuf;
			}
			else 
			{
				glog(ErrorLevel, ParseFmt(ParseXMLData, "DataTime invalid(%s)"), sDataTime.c_str());
			}
		}
		
		ZQ::common::XMLPreferenceEx* pInterval = pHeader->findSubPreference("Interval");
		SmartPreferenceEx smtInterval(pInterval);
		if (NULL != pInterval)
		{
			if (true == pInterval->getPreferenceText(szBuf, sizeof(szBuf) - 1))
				sInterval = NULL != szBuf ? szBuf : "";
		}
	}

	// get StartCMGroupID, EndCMGroupID from configuration, they must be configurated for Axiom
	// generated xml files.
	int nCMGroupID = 0;
	int nStartCMGroupID = 0, nEndCMGroupID = 0;
	nStartCMGroupID = atoi(_properties["StartCMGroupID"].c_str());
	nEndCMGroupID = atoi(_properties["EndCMGroupID"].c_str());
	if (nStartCMGroupID < 1 || nStartCMGroupID > 10000 || nEndCMGroupID < 1 || nEndCMGroupID > 10000 || nStartCMGroupID > nEndCMGroupID)
	{// nStartCMGroupID, nEndCMGroupID must be in [1, 10000], and nEndCMGroupID must be larger than or equal to nStartCMGroupID.
		glog(ErrorLevel, ParseFmt(ParseXMLData, "nStartCMGroupID, nEndCMGroupID must be in [1, 10000], and nEndCMGroupID must be larger than or equal to nStartCMGroupID."));
		return;
	}
	nCMGroupID = nStartCMGroupID;

	ZQ::common::XMLPreferenceEx* pCmGroup = _pRoot->firstChild("CMGroup");
	SmartPreferenceEx smtCmGroup(pCmGroup);
	while (NULL != pCmGroup)
	{
		ote_server_rec srvrRec;
		memset(&srvrRec, 0, sizeof(srvrRec));

		srvrRec.cm_group_id = nCMGroupID ++;
		if (srvrRec.cm_group_id > nEndCMGroupID)
		{// if current file has a lot of <CMGroup> which is larger than reserved for it, exit the loop.
			glog(WarningLevel, ParseFmt(ParseXMLData, "CMGroupID range[%d, %d] is not enough for %s"), nStartCMGroupID, nEndCMGroupID, _fileName.c_str());
			break;
		}

		srvrRec.server_type = 1; // by default give it a default value 1 which is reserved for Axiom.

		if (false == sInterval.empty())
			srvrRec.interval_time = atoi(sInterval.c_str());
		else 
		{
			srvrRec.bIntervalTimeEmpty = true;
			glog(InfoLevel, ParseFmt(ParseXMLData, "Axiom format, <Header> no <Interval> found or content empty."));
		}

		snprintf(srvrRec.version, sizeof(srvrRec.version) - 1, "%s", sVersion.c_str());
		snprintf(srvrRec.data_time, sizeof(srvrRec.data_time) - 1, "%s", sDataTime.c_str());

		ZQ::common::XMLPreferenceEx* pPGLevel = pCmGroup->findSubPreference("PGLevel");
		SmartPreferenceEx smtPGLevel(pPGLevel);
		if (NULL != pPGLevel)
		{
			std::string sPGLevel;
			if (true == pPGLevel->getPreferenceText(szBuf, sizeof(szBuf) - 1))
				sPGLevel = NULL != szBuf ? szBuf : "";
			if (false == sPGLevel.empty())
				srvrRec.pg_level = atoi(sPGLevel.c_str());
			else 
			{
				srvrRec.bPGLevelEmpty = true;
				glog(InfoLevel, ParseFmt(ParseXMLData, "Axiom format, <CMGroup> <PGLevel> content empty."));
			}
		}
		else 
		{
			srvrRec.bPGLevelEmpty = true;
			glog(InfoLevel, ParseFmt(ParseXMLData, "Axiom format, <CMGroup> no <PGLevel> found."));
		}
// some of axiom generated xml file's PGLevel stores in
//		<CMGroup>
//			<PeerGroup>
//				<PGID>63</PGID>
//				<PGLevel>4</PGLevel>
//			</PeerGroup>
//		</CMGroup>
		if (true == srvrRec.bPGLevelEmpty) // if no PGLevel found, so if has PeerGroup.
		{
		ZQ::common::XMLPreferenceEx* pPeerGroup = pCmGroup->findSubPreference("PeerGroup");
		SmartPreferenceEx smtPeerGroup(pPeerGroup);
		if (NULL != pPeerGroup)
		{
			ZQ::common::XMLPreferenceEx* pPGLevel = pPeerGroup->findSubPreference("PGLevel");
			SmartPreferenceEx smtPGLevel(pPGLevel);
			if (NULL != pPGLevel)
			{
				std::string sPGLevel;
				if (true == pPGLevel->getPreferenceText(szBuf, sizeof(szBuf) - 1))
					sPGLevel = NULL != szBuf ? szBuf : "";
				if (false == sPGLevel.empty())
				{
					srvrRec.bPGLevelEmpty = false;
					srvrRec.pg_level = atoi(sPGLevel.c_str());
				}
			}
		}
		}
		
		glog(DebugLevel, ParseFmt(ParseXMLData, "(ote_server_record)"
		"cm_group_id: %d;"
		"server_type: %d;"
		"interval_time: %d;"
		"pg_level: %d;"
		"version: %s;"
		"data_time: %s")
		, srvrRec.cm_group_id
		, srvrRec.server_type
		, srvrRec.interval_time
		, srvrRec.pg_level
		, srvrRec.version
		, srvrRec.data_time);

		_dbDatas.add_ote_server_rec(srvrRec);
		
		// search for next child
		if (true == _pRoot->hasNextChild())
		{
			ZQ::common::XMLPreferenceEx* pTemp = pCmGroup;
			SmartPreferenceEx smtTemp(pTemp);
			pCmGroup = _pRoot->nextChild();
		}
		else break;
	}
}

void ParseXMLData::ote_instances_axm()
{
	glog(DebugLevel, ParseFmt(ParseXMLData, "do ote_instances_axm()"));

	_dbDatas.erase_ote_instances();

	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));

	// get StartCMGroupID, EndCMGroupID from configuration, they must be configurated for Axiom
	// generated xml files.
	int nCMGroupID = 0;
	int nStartCMGroupID = 0, nEndCMGroupID = 0;
	nStartCMGroupID = atoi(_properties["StartCMGroupID"].c_str());
	nEndCMGroupID = atoi(_properties["EndCMGroupID"].c_str());
	if (nStartCMGroupID < 1 || nStartCMGroupID > 10000 || nEndCMGroupID < 1 || nEndCMGroupID > 10000 || nStartCMGroupID > nEndCMGroupID)
	{// nStartCMGroupID, nEndCMGroupID must be in [1, 10000], and nEndCMGroupID must be larger than or equal to nStartCMGroupID.
		glog(ErrorLevel, ParseFmt(ParseXMLData, "nStartCMGroupID, nEndCMGroupID must be in [1, 10000], and nEndCMGroupID must be larger than or equal to nStartCMGroupID."));
		return;
	}
	nCMGroupID = nStartCMGroupID;

	ZQ::common::XMLPreferenceEx* pCmGroup = _pRoot->firstChild("CMGroup");
	SmartPreferenceEx smtCmGroup(pCmGroup);
	while (NULL != pCmGroup)
	{
		if (nCMGroupID > nEndCMGroupID)
		{// if current file has a lot of <CMGroup> which is larger than reserved for it, exit the loop.
			glog(WarningLevel, ParseFmt(ParseXMLData, "CMGroupID range[%d, %d] is not enough for %s"), nStartCMGroupID, nEndCMGroupID, _fileName.c_str());
			break;
		}

		ZQ::common::XMLPreferenceEx* pInstance = pCmGroup->firstChild("Instance");
		SmartPreferenceEx smtInstance(pInstance);
		while (NULL != pInstance)
		{
			ote_instance_rec instRec;
			memset(&instRec, 0, sizeof(instRec));

			instRec.cm_group_id = nCMGroupID;

			// get SessionProtocolType from configuration which has been stored in _properties
			snprintf(instRec.session_protocal_type, sizeof(instRec.session_protocal_type) - 1, "%s", _properties["SessionProtocolType"].c_str());

			ZQ::common::XMLPreferenceEx* pIPAddress = pInstance->findSubPreference("IPAddress");
			SmartPreferenceEx smtIPAddress(pIPAddress);
			if (NULL != pIPAddress)
			{
				if (true == pIPAddress->getPreferenceText(szBuf, sizeof(szBuf) - 1))
					snprintf(instRec.ip_address, sizeof(instRec.ip_address) - 1, "%s", szBuf);
			}

			// get HostName from configuration which has been stored in _properties
			snprintf(instRec.host_name, sizeof(instRec.host_name) - 1, "%s", _properties["HostName"].c_str());

			// Since the "port" in Axiom generated server load xml file does not represents
			// the RTSP or DSMCC port for streaming control, it is just an internal port.
			// So the "port" also is configurable, and if configured, ServerLoad Service
			// must use the configured value to update OTE database, instead of the one
			// read from server load xml. And this is effective for TianShan. referenced from disign document.
			ZQ::common::XMLPreferenceEx* pPort = pInstance->findSubPreference("Port");
			SmartPreferenceEx smtPort(pPort);
			if (NULL != pPort)
			{
				std::string sPort;
				if (true == pPort->getPreferenceText(szBuf, sizeof(szBuf) - 1))
					sPort = NULL != szBuf ? szBuf : "";
				if (false == sPort.empty())
					instRec.port = atoi(sPort.c_str());
				else 
				{
					instRec.bPortEmpty = true;
					glog(InfoLevel, ParseFmt(ParseXMLData, "Axiom format, <Instance> <Port> content empty."));
				}
			}
			else 
			{
				instRec.bPortEmpty = true;
				glog(InfoLevel, ParseFmt(ParseXMLData, "Axiom format, <Instance> no <Port> found."));
			}

			// if Port found in configuration<ServerLoad><WatchList><File Port="">
			// and it's value larger than 0, use it.
			std::map<std::string, std::string>::iterator port_itor = _properties.find("Port");
			if (_properties.end() != port_itor && atoi(port_itor->second.c_str()) > 0)
			{
				instRec.bPortEmpty = false;
				instRec.port = atoi(port_itor->second.c_str());
			}

			ZQ::common::XMLPreferenceEx* pLoad = pInstance->findSubPreference("Load");
			SmartPreferenceEx smtLoad(pLoad);
			if (NULL != pLoad)
			{
				std::string sLoad;
				if (true == pLoad->getPreferenceText(szBuf, sizeof(szBuf) - 1))
					sLoad = NULL != szBuf ? szBuf : "";
				if (false == sLoad.empty())
					instRec.server_load = atoi(sLoad.c_str());
				else 
				{
					instRec.bServerLoadEmpty = true;
					glog(InfoLevel, ParseFmt(ParseXMLData, "Axiom format, <Instance> <Load> content empty."));
				}
			}
			else 
			{
				instRec.bServerLoadEmpty = true;
				glog(InfoLevel, ParseFmt(ParseXMLData, "Axiom format, <Instance> no <Load> found."));
			}

			ZQ::common::XMLPreferenceEx* pLifeTime = pInstance->findSubPreference("LifeTime");
			SmartPreferenceEx smtLifeTime(pLifeTime);
			if (NULL != pLifeTime)
			{
				std::string sLifeTime;
				if (true == pLifeTime->getPreferenceText(szBuf, sizeof(szBuf) - 1))
					sLifeTime = NULL != szBuf ? szBuf : "";
				if (false == sLifeTime.empty())
					instRec.life_time = atoi(sLifeTime.c_str());
				else 
				{
					instRec.bLifeTimeEmpty = true;
					glog(InfoLevel, ParseFmt(ParseXMLData, "Axiom format, <Instance> <LifeTime> content empty."));
				}
			}
			else 
			{
				instRec.bLifeTimeEmpty = true;
				glog(InfoLevel, ParseFmt(ParseXMLData, "Axiom format, <Instance> no <LifeTime> found."));
			}

			glog(DebugLevel, ParseFmt(ParseXMLData, "(ote_instance_record)"
				"cm_group_id: %d;"
				"host_name: %s;"
				"session_protocal_type: %s;"
				"ip_address: %s;"
				"port: %d;"
				"server_load: %d;"
				"life_time: %d")
				, instRec.cm_group_id
				, instRec.host_name
				, instRec.session_protocal_type
				, instRec.ip_address
				, instRec.port
				, instRec.server_load
				, instRec.life_time);

			_dbDatas.add_ote_instance_rec(instRec);

			// search for next child
			if (true == pCmGroup->hasNextChild())
			{
			ZQ::common::XMLPreferenceEx* pTemp = pInstance;
			SmartPreferenceEx smtTemp(pTemp);
			pInstance = pCmGroup->nextChild();
			}
			else break;
		}
		nCMGroupID ++;

		// search for next child
		if (true == _pRoot->hasNextChild())
		{
			ZQ::common::XMLPreferenceEx* pTemp = pCmGroup;
			SmartPreferenceEx smtTemp(pTemp);
			pCmGroup = _pRoot->nextChild();
		}
		else break;
	}
}

void ParseXMLData::ote_groups_axm()
{
	glog(DebugLevel, ParseFmt(ParseXMLData, "do ote_groups_axm()"));

	_dbDatas.erase_ote_groups();

	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));

	// get StartCMGroupID, EndCMGroupID from configuration, they must be configurated for Axiom
	// generated xml files.
	int nCMGroupID = 0;
	int nStartCMGroupID = 0, nEndCMGroupID = 0;
	nStartCMGroupID = atoi(_properties["StartCMGroupID"].c_str());
	nEndCMGroupID = atoi(_properties["EndCMGroupID"].c_str());
	if (nStartCMGroupID < 1 || nStartCMGroupID > 10000 || nEndCMGroupID < 1 || nEndCMGroupID > 10000 || nStartCMGroupID > nEndCMGroupID)
	{// nStartCMGroupID, nEndCMGroupID must be in [1, 10000], and nEndCMGroupID must be larger than or equal to nStartCMGroupID.
		glog(ErrorLevel, ParseFmt(ParseXMLData, "nStartCMGroupID, nEndCMGroupID must be in [1, 10000], and nEndCMGroupID must be larger than or equal to nStartCMGroupID."));
		return;
	}
	nCMGroupID = nStartCMGroupID;

	ZQ::common::XMLPreferenceEx* pCmGroup = _pRoot->firstChild("CMGroup");
	SmartPreferenceEx smtCmGroup(pCmGroup);
	while (NULL != pCmGroup)
	{
		if (nCMGroupID > nEndCMGroupID)
		{// if current file has a lot of <CMGroup> which is larger than reserved for it, exit the loop.
			glog(WarningLevel, ParseFmt(ParseXMLData, "CMGroupID range[%d, %d] is not enough for %s"), nStartCMGroupID, nEndCMGroupID, _fileName.c_str());
			break;
		}

		ZQ::common::XMLPreferenceEx* pNodeGroup = pCmGroup->firstChild("NodeGroup");
		SmartPreferenceEx smtNodeGroup(pNodeGroup);
		while (NULL != pNodeGroup)
		{
			ote_group_rec groupRec;
			memset(&groupRec, 0, sizeof(groupRec));

			groupRec.cm_group_id = nCMGroupID;

			std::string sNodeGroup;
			if (true == pNodeGroup->getPreferenceText(szBuf, sizeof(szBuf) - 1))
			{
				sNodeGroup = NULL != szBuf ? szBuf : "";
				if (0 == stricmp(String::nLeftStr(sNodeGroup, 2).c_str(), "0x"))
					sNodeGroup = String::rightStr(sNodeGroup, 1);
			}
			
			if (false == sNodeGroup.empty())
			{
				sscanf(sNodeGroup.c_str(), "%I64x", &groupRec.node_group);
				glog(DebugLevel, ParseFmt(ParseXMLData, "(ote_group_record)"
					"cm_group_id: %d;"
					"node_group: %I64d")
					, groupRec.cm_group_id
					, groupRec.node_group);
				_dbDatas.add_ote_group_rec(groupRec);
			}
			else 
				glog(InfoLevel, ParseFmt(ParseXMLData, "Axiom format, <CMGroup> <NodeGroup> content empty"));

			// search for next child
			if (true == pCmGroup->hasNextChild())
			{
				ZQ::common::XMLPreferenceEx* pTemp = pNodeGroup;
				SmartPreferenceEx smtTemp(pTemp);
				pNodeGroup = pCmGroup->nextChild();
			}
			else break;
		}
		nCMGroupID ++;

		// search for next child
		if (true == _pRoot->hasNextChild())
		{
			ZQ::common::XMLPreferenceEx* pTemp = pCmGroup;
			SmartPreferenceEx smtTemp(pTemp);
			pCmGroup = _pRoot->nextChild();
		}
		else break;
	}
}

void ParseXMLData::ote_cm_apps_axm()
{
	glog(DebugLevel, ParseFmt(ParseXMLData, "do ote_cm_apps_axm()"));

	_dbDatas.erase_ote_cm_apps();

	char szBuf[1024];
	memset(szBuf, 0, sizeof(szBuf));

	// get StartCMGroupID, EndCMGroupID from configuration, they must be configurated for Axiom
	// generated xml files.
	int nCMGroupID = 0;
	int nStartCMGroupID = 0, nEndCMGroupID = 0;
	nStartCMGroupID = atoi(_properties["StartCMGroupID"].c_str());
	nEndCMGroupID = atoi(_properties["EndCMGroupID"].c_str());
	if (nStartCMGroupID < 1 || nStartCMGroupID > 10000 || nEndCMGroupID < 1 || nEndCMGroupID > 10000 || nStartCMGroupID > nEndCMGroupID)
	{// nStartCMGroupID, nEndCMGroupID must be in [1, 10000], and nEndCMGroupID must be larger than or equal to nStartCMGroupID.
		glog(ErrorLevel, ParseFmt(ParseXMLData, "nStartCMGroupID, nEndCMGroupID must be in [1, 10000], and nEndCMGroupID must be larger than or equal to nStartCMGroupID."));
		return;
	}
	nCMGroupID = nStartCMGroupID;

	ZQ::common::XMLPreferenceEx* pCmGroup = _pRoot->firstChild("CMGroup");
	SmartPreferenceEx smtCmGroup(pCmGroup);
	while (NULL != pCmGroup)
	{
		if (nCMGroupID > nEndCMGroupID)
		{// if current file has a lot of <CMGroup> which is larger than reserved for it, exit the loop.
			glog(WarningLevel, ParseFmt(ParseXMLData, "CMGroupID range[%d, %d] is not enough for %s"), nStartCMGroupID, nEndCMGroupID, _fileName.c_str());
			break;
		}

		ZQ::common::XMLPreferenceEx* pAppType = pCmGroup->firstChild("AppType");
		SmartPreferenceEx smtAppType(pAppType);
		while (NULL != pAppType)
		{
			ote_cm_app_rec cmAppRec;
			memset(&cmAppRec, 0, sizeof(cmAppRec));

			cmAppRec.cm_group_id = nCMGroupID;

			std::string sAppType;
			if (true == pAppType->getPreferenceText(szBuf, sizeof(szBuf) - 1))
			{
				sAppType = NULL != szBuf ? szBuf : "";
				if (0 == stricmp(String::nLeftStr(sAppType, 2).c_str(), "0x"))// È¥µô0X
					sAppType = String::rightStr(sAppType, 1);
			}
			snprintf(cmAppRec.app_type, sizeof(cmAppRec.app_type) - 1, "%s", sAppType.c_str());

			if (0 != strlen(cmAppRec.app_type))
			{
				glog(DebugLevel, ParseFmt(ParseXMLData, "(ote_cm_app_record)"
					"cm_group_id: %d;"
					"app_type: %s")
					, cmAppRec.cm_group_id
					, cmAppRec.app_type);
				_dbDatas.add_ote_cm_app_rec(cmAppRec);
			}
			else 
				glog(InfoLevel, ParseFmt(ParseXMLData, "Axiom format, <CMGroup> <AppType> content empty"));

			// search for next child
			if (true == pCmGroup->hasNextChild())
			{
				ZQ::common::XMLPreferenceEx* pTemp = pAppType;
				SmartPreferenceEx smtTemp(pTemp);
				pAppType = pCmGroup->nextChild();
			}
			else break;
		}
		nCMGroupID ++;

		// search for next child
		if (true == _pRoot->hasNextChild())
		{
			ZQ::common::XMLPreferenceEx* pTemp = pCmGroup;
			SmartPreferenceEx smtTemp(pTemp);
			pCmGroup = _pRoot->nextChild();
		}
		else break;
	}
}

}

