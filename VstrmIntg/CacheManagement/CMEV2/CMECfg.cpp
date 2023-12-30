#include "CMECfg.h"

namespace CacheManagement
{

ZQ::common::Config::Loader<CMECfg> _gCMECfg("CME.xml");

CMECfg::CMECfg()
{
	_crashDumpEnabled = 1;
	_dumpFullMemory = 1;

	_datFlushInterval = 600;
	_syncIOAtStart = 1;

	_vsisRecBuffSize = 1024;
	_vsisConnTimeout = 5000;

	_bwThreshold = 75;
	_bwReserved = 20000000;
}

void CMECfg::structure(ZQ::common::Config::Holder<CMECfg> &holder)
{
    using namespace ZQ::common;

    holder.addDetail("default/CrashDump", "path", &CMECfg::_crashDumpPath, "", Config::optReadOnly);
    holder.addDetail("default/CrashDump", "enabled", &CMECfg::_crashDumpEnabled, "1", Config::optReadOnly);

    holder.addDetail("default/Database", "path", &CMECfg::_datPath, "", Config::optReadOnly);
    holder.addDetail("default/Database", "type", &CMECfg::_datType, "text", Config::optReadOnly);
    holder.addDetail("default/Database", "flushInterval", &CMECfg::_datFlushInterval, "60", Config::optReadOnly);
	holder.addDetail("default/Database", "syncIOAtStart", &CMECfg::_syncIOAtStart, "1", Config::optReadOnly);

    holder.addDetail("CacheManagement", "netId", &CMECfg::_cmeNetId, "", Config::optReadOnly);

    holder.addDetail("CacheManagement/CacheLogFile", "size", &CMECfg::_cacheLogSize, "50000000", Config::optReadOnly);
	holder.addDetail("CacheManagement/CacheLogFile", "level", &CMECfg::_cacheLogLevel, "7", Config::optReadOnly);
    holder.addDetail("CacheManagement/CacheLogFile", "maxCount", &CMECfg::_cacheLogCount, "10", Config::optReadOnly);
	holder.addDetail("CacheManagement/CacheLogFile", "bufferSize", &CMECfg::_cacheLogBuff, "8192", Config::optReadOnly);

    holder.addDetail("CacheManagement/Connection", "cmeEndpoint", &CMECfg::_cmeEndPointRaw, "", Config::optReadOnly);
	holder.addDetail("CacheManagement/Connection", "lamEndPoint", &CMECfg::_lamEndPointRaw, "", Config::optReadOnly);
	
	holder.addDetail("CacheManagement/VSIS", "buffSize", &CMECfg::_vsisRecBuffSize, "65536", Config::optReadOnly);
	holder.addDetail("CacheManagement/VSIS", "connectionTimeout", &CMECfg::_vsisConnTimeout, "5", Config::optReadOnly);
	holder.addDetail("CacheManagement/VSIS", "sendTimeout", &CMECfg::_vsisSendTimeout, "5", Config::optReadOnly);
	holder.addDetail("CacheManagement/VSIS", "idleTimeout", &CMECfg::_vsisIdleTimeout, "3600", Config::optReadOnly);
	holder.addDetail("CacheManagement/VSIS", "reportInterval", &CMECfg::_vsisReportInterval, "150", Config::optReadOnly);
	holder.addDetail("CacheManagement/VSIS", "scanInterval", &CMECfg::_vsisScanInterval, "60", Config::optReadOnly);
	holder.addDetail("CacheManagement/VSIS", "lengthOfPaid", &CMECfg::_lengthOfPAID, "20", Config::optReadOnly);

	holder.addDetail("CacheManagement/CacheControl", "storageThreshold", &CMECfg::_storageThreshold, "70", Config::optReadOnly);
    holder.addDetail("CacheManagement/CacheControl", "storageCushion", &CMECfg::_storageCushion, "5", Config::optReadOnly);
    holder.addDetail("CacheManagement/CacheControl", "adDuration", &CMECfg::_adDuration, "121", Config::optReadOnly);
	holder.addDetail("CacheManagement/CacheControl", "visaSeconds", &CMECfg::_visaSeconds, "3600", Config::optReadOnly);
	holder.addDetail("CacheManagement/CacheControl", "trimDays", &CMECfg::_trimDays, "7", Config::optReadOnly);
	holder.addDetail("CacheManagement/CacheControl", "playTrigger", &CMECfg::_playTrigger, "4", Config::optReadOnly);
	holder.addDetail("CacheManagement/CacheControl", "playTrigger2", &CMECfg::_playTrigger2, "8", Config::optReadOnly);
	holder.addDetail("CacheManagement/CacheControl", "ageDenominator", &CMECfg::_ageDenominator, "3", Config::optReadOnly);
	holder.addDetail("CacheManagement/CacheControl", "agePeriod", &CMECfg::_agePeriod, "79200", Config::optReadOnly);
	holder.addDetail("CacheManagement/CacheControl", "subFileCount", &CMECfg::_subFileCount, "1", Config::optReadOnly);

	holder.addDetail("CacheManagement/CacheBandwidth", "threshold", &CMECfg::_bwThreshold, "75", Config::optReadOnly);
	holder.addDetail("CacheManagement/CacheBandwidth", "reserved", &CMECfg::_bwReserved, "20000000", Config::optReadOnly);

	holder.addDetail("CacheManagement/Statistics", "enabled", &CMECfg::_statEnabled, "1", Config::optReadOnly);
	holder.addDetail("CacheManagement/Statistics", "window", &CMECfg::_statWindow, "86400", Config::optReadOnly);
	holder.addDetail("CacheManagement/Statistics", "interval", &CMECfg::_statInterval, "3600", Config::optReadOnly);
	holder.addDetail("CacheManagement/Statistics", "delay", &CMECfg::_statDelay, "7200", Config::optReadOnly);
	holder.addDetail("CacheManagement/Statistics", "cotentExtraSizePercentage", &CMECfg::_statCntExtraSizePer, "0", Config::optReadOnly);
	holder.addDetail("CacheManagement/Statistics", "printDetails", &CMECfg::_statPrintDetails, "0", Config::optReadOnly);

	holder.addDetail("CacheManagement/Proactive", "enabled", &CMECfg::_proactiveEnabled, "0", Config::optReadOnly);
	holder.addDetail("CacheManagement/Proactive", "monitorPath", &CMECfg::_proactivePath, "", Config::optReadOnly);
	holder.addDetail("CacheManagement/Proactive", "retryInterval", &CMECfg::_proactiveRetryInterval, "60", Config::optReadOnly);
}
CMECfg::~CMECfg()
{

}

bool CMECfg::parseCMEEndpoint()
{
	std::string httpPrefix = "http://";

	// set the init value in case caller want to know the value from config file
	_cmeEndPoint = _cmeEndPointRaw;

	// parse <IP:Port> format 
	size_t pos = _cmeEndPoint.find_last_of(':');
	if(pos == std::string::npos)
	{
		return false;
	}

	_cmeBindIP = _cmeEndPoint.substr(0, pos);
	std::string szBindPort = _cmeEndPoint.substr(pos+1, _cmeEndPoint.size()-pos-1);

	_cmeBindPort = atoi(szBindPort.c_str());

	pos  = _cmeEndPoint.find(httpPrefix);
	if(pos != std::string::npos)
	{	// find http://, fetch ip from it
		pos = pos + httpPrefix.size();
		_cmeBindIP = _cmeBindIP.substr(pos, _cmeBindIP.size()-pos-1);

		// _cmeEndPointRaw already include http://
		httpPrefix = "";
	}

	// format endpoint to http://%s:%d/services/CMEService?wsdl
	_cmeEndPoint = httpPrefix + _cmeEndPointRaw + std::string("/services/CMEService?wsdl");

	return true;
}

bool CMECfg::parseLAMEndpoint()
{
	std::string httpPrefix = "http://";

	// set the init value in case caller want to know the value from config file
	_lamEndPoint = _lamEndPointRaw;

	// parse <IP:Port> or http://<IP:Port>format 
	size_t pos = _lamEndPoint.find_last_of(':');
	if(pos == std::string::npos)
	{
		return false;
	}

	pos  = _lamEndPoint.find(httpPrefix);
	if(pos != std::string::npos)
	{	// find http://
		httpPrefix = "";
	}

	// format endpoint to http://%s:%d/services/CMEService?wsdl
	_lamEndPoint = httpPrefix + _lamEndPointRaw + std::string("/services/LAMServiceForCME");

	return true;
}

} //namespace CacheManagement