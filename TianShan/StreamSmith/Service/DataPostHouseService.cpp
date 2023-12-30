// File Name: DataPostHouseService.cpp
// Date: 2009-03
// Description: implement of DataPostHouse Service 

#include "DataPostHouseService.h"
#include "DialogType.h"

using namespace ZQ::DataPostHouse;

namespace ZQ
{
	namespace StreamSmith
	{

DataPostHouseService::DataPostHouseService(bool includeIPv6)
:_postHouseEnv(NULL), _postDak(NULL), _postMen(10), _bIncludeIPv6(includeIPv6),
_socketRtspTcpIPv4(NULL), _socketRtspTcpIPv6(NULL), _socketRtspSSLIPv4(NULL), _socketRtspSSLIPv6(NULL),
_socketLscTcpIPv4(NULL), _socketLscTcpIPv6(NULL),_socketLscSSLIPv4(NULL), _socketLscSSLIPv6(NULL), _socketLscUdpIPv4(NULL), _socketLscUdpIPv6(NULL), 
_bStartRtspTcpIPv4(false), _bStartRtspTcpIPv6(false), _bStartRtspSSLIPv4(false), _bStartRtspSSLIPv6(false),
_bStartLscTcpIPv4(false), _bStartLscTcpIPv6(false), _bStartLscSSLIPv4(false), _bStartLscSSLIPv6(false), _bStartLscUdpIPv4(false), _bStartLscUdpIPv6(false)
{
}

DataPostHouseService::~DataPostHouseService(void)
{
	_socketRtspTcpIPv4 = NULL;
	_socketRtspTcpIPv6 = NULL;

	//RTSP SSL
	_socketRtspSSLIPv4 = NULL;
	_socketRtspSSLIPv6 = NULL;

	//LSCP TCP
	_socketLscTcpIPv4 = NULL;
	_socketLscTcpIPv6 = NULL;

	//LSCP SSL
	_socketLscSSLIPv4 = NULL;
	_socketLscSSLIPv6 = NULL;

	//LSCP UDP
	_socketLscUdpIPv4 = NULL;
	_socketLscUdpIPv6 = NULL;

	delete _postDak;
	delete _postHouseEnv;
}

//bool DataPostHouseService::init(ZQ::common::Log* log, int32 postMen, int32 nReadBufferSize, int32 nEncryptBufferSize, ZQ::DataPostHouse::IDataDialogFactoryPtr dialogCreator)
bool DataPostHouseService::init(ServiceConfig &cfg)
{
	if ((NULL == cfg._cfg_log) || ( 0 == cfg._cfg_dialogCreator))
	{
		return false;
	}
	_postHouseEnv = new DataPostHouseEnv();
	if (NULL == _postHouseEnv)
	{
		glog(ZQ::common::Log::L_ERROR, "Fail to create DataPostHouseEnv");
		return false;
	}
	_postHouseEnv->mReadBufferSize = cfg._cfg_readBufferSize;
	_postHouseEnv->mEncryptBufferSize = cfg._cfg_encryptBufferSize;
	_postHouseEnv->dataFactory = cfg._cfg_dialogCreator;
	_postHouseEnv->mLogger = cfg._cfg_log; // 
	_postDak = new DataPostDak(*_postHouseEnv, cfg._cfg_dialogCreator);
	if (NULL == _postDak)
	{
		glog(ZQ::common::Log::L_ERROR, "Fail to create DataPostDak");
		return false;
	}
	_postMen = cfg._cfg_postMen;
	_strCertificateFile = cfg._cfg_publicKeyFile;
	_strPrivatekeyFile = cfg._cfg_privateKeyFile;
	_strCertPasswd = cfg._cfg_privateKeyFilePwd;
	return true;
}

bool DataPostHouseService::begin()
{
	if (!_postDak->startDak(_postMen))
	{
		glog(ZQ::common::Log::L_ERROR, "Data Post Dak isn't started");
		return false;
	}
	glog(ZQ::common::Log::L_DEBUG, "Data Post Dak is running");
	return true;
}

void DataPostHouseService::end()
{
	// RTSP TCP IPv4
	stopCommunicator(_socketRtspTcpIPv4, _bStartRtspTcpIPv4, "Stop RTSP TCP IPv4 communicator");

	// RTSP TCP IPv6
	stopCommunicator(_socketRtspTcpIPv6, _bStartRtspTcpIPv6, "Stop RTSP TCP IPv6 communicator");
	
	// RTSP SSL IPv4
	stopCommunicator(_socketRtspSSLIPv4, _bStartRtspSSLIPv4, "Stop RTSP SSL IPv4 communicator");
	
	// RTSP SSL IPv6
	stopCommunicator(_socketRtspSSLIPv6, _bStartRtspSSLIPv6, "Stop RTSP SSL IPv6 communicator");

    // LSCP TCP IPv4
	stopCommunicator(_socketLscTcpIPv4, _bStartLscTcpIPv4, "Stop LSCP TCP IPv4 communicator");
	
	// LSCP TCP IPv6
	stopCommunicator(_socketLscTcpIPv6, _bStartLscTcpIPv6, "Stop LSCP TCP IPv4 communicator");

	// LSCP SSL IPv4
	stopCommunicator(_socketLscSSLIPv4, _bStartLscSSLIPv4, "Stop LSCP SSL IPv4 communicator");
	
	// LSCP SSL IPv6
	stopCommunicator(_socketLscSSLIPv6, _bStartLscSSLIPv6, "Stop LSCP SSL IPv6 communicator");

	// LSCP UDP IPv4
	stopCommunicator(_socketLscUdpIPv4, _bStartLscUdpIPv4, "Stop LSCP UDP IPv4 communicator");

	// LSCP UDP IPv6
	stopCommunicator(_socketLscUdpIPv6, _bStartLscUdpIPv6, "Stop LSCP UDP IPv4 communicator");

	// stopDak can be called many times
	_postDak->stopDak();
	glog(ZQ::common::Log::L_DEBUG, "Close Data Post Dak");

}

void DataPostHouseService::uninit()
{

}

void DataPostHouseService::setCertAndKeyFile(const std::string strCertFile, const std::string strKeyFile, const std::string strCertPasswd)
{
	_strCertificateFile = strCertFile;
	_strPrivatekeyFile = strKeyFile;
	_strCertPasswd = strCertPasswd;
}

bool DataPostHouseService::bindRtsp(const std::string strLocalIPv4, const std::string strLocalIPv6, const std::string strLocalPort)
{
	DialogTypePtr rtspType = new DialogType(ZQ::DataPostHouse::APP_TYPE_RTSP);
	if (0 == rtspType)
	{
		glog(ZQ::common::Log::L_ERROR, "Fail to create RTSP dialog type");
		return false;
	}
	// TCP IPv4
	_socketRtspTcpIPv4 = new AServerSocketTcp(*_postDak, *_postHouseEnv);
	if (0 == _socketRtspTcpIPv4)
	{
		glog(ZQ::common::Log::L_ERROR, "Fail to create RTSP TCP IPv4 communicator");
		return false;
	}
	_bStartRtspTcpIPv4 = _socketRtspTcpIPv4->startServer(strLocalIPv4, strLocalPort, rtspType);
	if (_bStartRtspTcpIPv4)
	{
		glog(ZQ::common::Log::L_DEBUG, "RTSP TCP IPv4 communicator is listening");
	}

	if (_bIncludeIPv6)
	{
		// TCP IPv6
		_socketRtspTcpIPv6 = new AServerSocketTcp(*_postDak, *_postHouseEnv);
		if (0 == _socketRtspTcpIPv6)
		{
			glog(ZQ::common::Log::L_ERROR, "Fail to create RTSP TCP IPv6 communicator");
			return false;
		}

		_bStartRtspTcpIPv6 = _socketRtspTcpIPv6->startServer(strLocalIPv6, strLocalPort, rtspType);
		if (_bStartRtspTcpIPv6)
		{
			glog(ZQ::common::Log::L_DEBUG, "RTSP TCP IPv6 communicator is listening");
		}
	}

	return (_bStartRtspTcpIPv4 || _bStartRtspTcpIPv6);
}

bool DataPostHouseService::bindSSLRtsp(const std::string strLocalIPv4, const std::string strLocalIPv6, const std::string strLocalPort)
{
	DialogTypePtr rtspType = new DialogType(ZQ::DataPostHouse::APP_TYPE_RTSP);
	if (0 == rtspType)
	{
		glog(ZQ::common::Log::L_ERROR, "Fail to create RTSP dialog type");
		return false;
	}
	// TCP IPv4
	_socketRtspSSLIPv4 = new SSLServer(*_postDak, *_postHouseEnv);
	if (0 == _socketRtspSSLIPv4)
	{
		glog(ZQ::common::Log::L_ERROR, "Fail to create RTSP SSL IPv4 communicator");
		return false;
	}
	_socketRtspSSLIPv4->setCertAndKeyFile(_strCertificateFile, _strPrivatekeyFile, _strCertPasswd);
	_bStartRtspSSLIPv4 = _socketRtspSSLIPv4->startServer(strLocalIPv4, strLocalPort, rtspType);
	if (_bStartRtspSSLIPv4)
	{
		glog(ZQ::common::Log::L_DEBUG, "RTSP SSL IPv4 communicator is listening");
	}

	// TCP IPv6
	if (_bIncludeIPv6)
	{
		_socketRtspSSLIPv6 = new SSLServer(*_postDak, *_postHouseEnv);
		if (0 == _socketRtspSSLIPv6)
		{
			glog(ZQ::common::Log::L_ERROR, "Fail to create RTSP SSL IPv6 communicator");
			return false;
		}
		_socketRtspSSLIPv6->setCertAndKeyFile(_strCertificateFile, _strPrivatekeyFile, _strCertPasswd);
		_bStartRtspSSLIPv6 = _socketRtspSSLIPv6->startServer(strLocalIPv6, strLocalPort, rtspType);
		if (_bStartRtspSSLIPv6)
		{
			glog(ZQ::common::Log::L_DEBUG, "RTSP SSL IPv6 communicator is listening");
		}
	}

	return (_bStartRtspSSLIPv4 || _bStartRtspSSLIPv6);
}

bool DataPostHouseService::bindLscp(const std::string strLocalIPv4, const std::string strLocalIPv6, const std::string strLocalPort)
{
	DialogTypePtr lscpType = new DialogType(ZQ::DataPostHouse::APP_TYPE_LSCP);
	if (0 == lscpType)
	{
		glog(ZQ::common::Log::L_ERROR, "Fail to create LSCP dialog type");
		return false;
	}
	// TCP IPV4
	_socketLscTcpIPv4 = new AServerSocketTcp(*_postDak, *_postHouseEnv);
	if (0 == _socketLscTcpIPv4)
	{
		glog(ZQ::common::Log::L_ERROR, "Fail to create LSCP TCP IPv4 communicator");
		return false;
	}
	_bStartLscTcpIPv4 = _socketLscTcpIPv4->startServer(strLocalIPv4, strLocalPort, lscpType);
	if (_bStartLscTcpIPv4)
	{
		glog(ZQ::common::Log::L_DEBUG, "LSCP TCP IPv4 communicator is listening");
	}

	// TCP IPV6
	if (_bIncludeIPv6)
	{
		_socketLscTcpIPv6 = new AServerSocketTcp(*_postDak, *_postHouseEnv);
		if (0 == _socketLscTcpIPv6)
		{
			glog(ZQ::common::Log::L_ERROR, "Fail to create LSCP TCP IPv6 communicator");
			return false;
		}
		_bStartLscTcpIPv6 = _socketLscTcpIPv6->startServer(strLocalIPv6, strLocalPort, lscpType);
		if (_bStartLscTcpIPv6)
		{
			glog(ZQ::common::Log::L_DEBUG, "LSCP TCP IPv6 communicator is listening");
		}
	}

	// udp IPV4
	_socketLscUdpIPv4 = new AServerSocketUdp(*_postDak, *_postHouseEnv);
	if (0 == _socketLscUdpIPv4)
	{
		glog(ZQ::common::Log::L_ERROR, "Fail to create LSCP UDP IPv4 communicator");
		return false;
	}
	_bStartLscUdpIPv4 = _socketLscUdpIPv4->startServer(strLocalIPv4, strLocalPort, lscpType);
	if (_bStartLscUdpIPv4)
	{
		glog(ZQ::common::Log::L_DEBUG, "LSCP UDP IPv4 communicator is listening");
	}

	//udp IPV6
	if (_bIncludeIPv6)
	{
		_socketLscUdpIPv6 = new AServerSocketUdp(*_postDak, *_postHouseEnv);
		if (0 == _socketLscUdpIPv6)
		{
			glog(ZQ::common::Log::L_ERROR, "Fail to create LSCP UDP IPv6 communicator");
			return false;
		}
		_bStartLscUdpIPv6 = _socketLscUdpIPv6->startServer(strLocalIPv6, strLocalPort, lscpType);
		if (_bStartLscUdpIPv6)
		{
			glog(ZQ::common::Log::L_DEBUG, "LSCP UDP IPv6 communicator is listening");
		}
	}

	return (_bStartLscUdpIPv4 || _bStartLscTcpIPv4 || _bStartLscTcpIPv6 || _bStartLscUdpIPv6);
}

bool DataPostHouseService::bindSSLLscp(const std::string strLocalIPv4, const std::string strLocalIPv6, const std::string strLocalPort)
{
	DialogTypePtr lscpType = new DialogType(ZQ::DataPostHouse::APP_TYPE_LSCP);
	if (0 == lscpType)
	{
		glog(ZQ::common::Log::L_ERROR, "Fail to create LSCP dialog type");
		return false;
	}
	// SSL IPv4
	_socketLscSSLIPv4 = new SSLServer(*_postDak, *_postHouseEnv);
	if (0 == _socketLscSSLIPv4)
	{
		glog(ZQ::common::Log::L_ERROR, "Fail to create LSCP SSL IPv4 communicator");
		return false;
	}
	_socketLscSSLIPv4->setCertAndKeyFile(_strCertificateFile, _strPrivatekeyFile, _strCertPasswd);
	_bStartLscSSLIPv4 = _socketLscSSLIPv4->startServer(strLocalIPv4, strLocalPort, lscpType);
	if (_bStartLscSSLIPv4)
	{
		glog(ZQ::common::Log::L_DEBUG, "LSCP TCP IPv4 communicator is listening");
	}

	// SSL IPv6
	if (_bIncludeIPv6)
	{

		_socketLscSSLIPv6 = new SSLServer(*_postDak, *_postHouseEnv);
		if (0 == _socketLscSSLIPv6)
		{
			glog(ZQ::common::Log::L_ERROR, "Fail to create LSCP SSL IPv6 communicator");
			return false;
		}
		_socketLscSSLIPv6->setCertAndKeyFile(_strCertificateFile, _strPrivatekeyFile, _strCertPasswd);
		_bStartLscSSLIPv6 = _socketLscSSLIPv6->startServer(strLocalIPv6, strLocalPort, lscpType);
		if (_bStartLscSSLIPv6)
		{
			glog(ZQ::common::Log::L_DEBUG, "LSCP SSL IPv6 communicator is listening");
		}
	}

	return (_bStartLscSSLIPv4 || _bStartLscSSLIPv6);
}


//////////////////////////////////////////////////////////////////////////
ServiceConfig::ServiceConfig()
{
	strcpy(_cfg_publicKeyFile, "certificate.pem");
	strcpy(_cfg_privateKeyFile, "private.key");
	strcpy(_cfg_privateKeyFilePwd, "SeaChange");
	strcpy(_cfg_dhParamFile, "dh1024.pem");
	strcpy(_cfg_randFile, "rand.pem");
	_cfg_debugLevel = SVCDBG_LEVEL_DEBUG;
	_cfg_postMen = 20;
	_cfg_readBufferSize = 1024;
	_cfg_encryptBufferSize = 1024;
	_cfg_maxConn = 2000;
	_cfg_log = NULL;
	_cfg_dialogCreator = NULL;
}

ServiceConfig::~ServiceConfig()
{

}

//const ServiceConfig::_ConfigEntry* ServiceConfig::getBaseConfigMap()
//{
//	return NULL;
//}
//
//const ServiceConfig::_ConfigEntry* ServiceConfig::getConfigMap()
//{
//	static _ConfigEntry cfgMap[] = 
//	{
//		{"publicKeyFile", _ConfigEntry::STRING_VALUE, 
//		&_cfg_publicKeyFile, sizeof(_cfg_publicKeyFile)}, 
//		{"privateKeyFile", _ConfigEntry::STRING_VALUE, 
//		&_cfg_privateKeyFile, sizeof(_cfg_privateKeyFile)}, 
//		{"privateKeyFilePwd", _ConfigEntry::STRING_VALUE, 
//		&_cfg_privateKeyFilePwd, sizeof(_cfg_privateKeyFilePwd)}, 
//		{"dhParamFile", _ConfigEntry::STRING_VALUE, 
//		&_cfg_dhParamFile, sizeof(_cfg_dhParamFile)}, 
//		{"randFile", _ConfigEntry::STRING_VALUE, 
//		&_cfg_randFile, sizeof(_cfg_randFile)}, 
//		{"isSecure", _ConfigEntry::USHORT_VALUE, 
//		&_cfg_isSecure, sizeof(_cfg_isSecure)}, 
//		{"threadCount", _ConfigEntry::USHORT_VALUE, 
//		&_cfg_threadCount, sizeof(_cfg_threadCount)}, 
//		{"maxConnection", _ConfigEntry::ULONG_VALUE, 
//		&_cfg_maxConn, sizeof(_cfg_maxConn)},
//		{"debugLevel", _ConfigEntry::LONG_VALUE, 
//		&_cfg_debugLevel, sizeof(_cfg_debugLevel)},	
//		{"idleTimeout", _ConfigEntry::LONG_VALUE, 
//		&sf_idle_timeout, sizeof(sf_idle_timeout)},
//		{"maxPollTime", _ConfigEntry::ULONG_VALUE, 
//		&sf_max_polltime, sizeof(sf_max_polltime)},
//		{"recvBufSize", _ConfigEntry::ULONG_VALUE, 
//		&_cfg_recvBufSize, sizeof(_cfg_recvBufSize)},
//		{"threadPriority", _ConfigEntry::ULONG_VALUE, 
//		&_cfg_threadPriority, sizeof(_cfg_threadPriority)},
//		{"minWorkingSet", _ConfigEntry::ULONG_VALUE, 
//		&_cfg_minWorkingSet, sizeof(_cfg_minWorkingSet)},
//		{NULL, _ConfigEntry::INVALID_TYPE, NULL, 0}, 
//	};
//
//	return cfgMap;
//}
//
//bool ServiceConfig::load(const char* cfgfile,std::vector<std::string>& pathVec)
//{
//	/* read configuration from XML file */
//	//using XMLPreferenceEx
//	//ComInitializer init;
//	XMLPreferenceDocumentEx root;
//	XMLPreferenceEx* pref = NULL, * child = NULL;
//
//	try {
//		if(!root.open(cfgfile)) {
////			printError("ServiceFrmBase::initByConfigureFile() failed");
//			return false;
//		}
//
//		pref = root.getRootPreference();
//		if (pref == NULL)
//			return false;
//		if(pathVec.size() <= 0)
//		{
//			child = pref->firstChild("Service");		
//			pref->free();
//		}
//		else
//		{
//			while (pathVec.size() > 0)
//			{
//				child=pref->firstChild(pathVec[0].c_str());
//				if(!child)
//					return false;
//				pathVec.erase(pathVec.begin());
//				pref->free();
//				pref=child;
//			}
//		}
//		if (!child)
//			return false;
//
//		pref = child;
//		const _ConfigEntry* cfgEntry;
//		cfgEntry = getBaseConfigMap();
//		if (cfgEntry != NULL)
//			processEntry(pref, cfgEntry);
//
//		cfgEntry = getConfigMap();
//		if (cfgEntry != NULL)
//			processEntry(pref, cfgEntry);
//
//		pref->free();
//
//	}  catch(Exception ) {
//		if (pref)
//			pref->free();
//		return false;
//	}
//
//	if (sf_max_polltime < MAX_CONN_PER_REQ)
//		sf_max_polltime = MAX_CONN_PER_REQ;
//
//	return true;
//}
//
//bool ServiceConfig::processEntry(XMLPreferenceEx* pref, 
//								 const _ConfigEntry* cfgEntry)
//{
//	XMLPreferenceEx* child = NULL;
//	char buf[512];
//	char* fmt;
//	try {
//		while(cfgEntry->type != _ConfigEntry::INVALID_TYPE) {
//			child = pref->firstChild(cfgEntry->key);
//			if (child == NULL)
//				goto L_Next;
//			if (child->get("value", buf, "", sizeof(buf) - 1) == "") {
//				child->free();
//				goto L_Next;
//			}
//			switch(cfgEntry->type) {
//			case _ConfigEntry::STRING_VALUE:
//				fmt = "%s";
//				break;
//			case _ConfigEntry::SHORT_VALUE:
//				fmt = "%hd";
//				break;
//			case _ConfigEntry::USHORT_VALUE:
//				fmt = "%hu";
//				break;
//			case _ConfigEntry::LONG_VALUE:
//				fmt = "%ld";
//				break;
//			case _ConfigEntry::ULONG_VALUE:
//				fmt = "%lu";
//				break;
//			default:
//				fmt = "%s";
//				assert(false);
//				break;
//			}
//			if(cfgEntry->type==_ConfigEntry::STRING_VALUE)
//				strncpy((char*)cfgEntry->value,buf,cfgEntry->maxValue);
//			else
//				sscanf(buf, fmt, cfgEntry->value);
//			child->free();
//L_Next:
//			cfgEntry ++;
//		}
//
//	} catch(Exception ) {
//		if (child)
//			child->free();
//		return false;
//	}
//	return true;
//}

	}
}
