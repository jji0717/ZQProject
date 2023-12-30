#include "./ssmNGODr2c1.h"

#include "./SetupHandler.h"
#include "./PlayHandler.h"
#include "./PauseHandler.h"
#include "./TeardownHandler.h"
#include "./GetParamHandler.h"
#include "./SetParamHandler.h"
#include "./OptionHandler.h"
#include "./PingHandler.h"

#include "urlstr.h"


#define INDEXFILENAME(_X) #_X "Idx"

#define SYSLOG _sysLog
#define NGODLOG _fileLog
#define NGODLOGFMT(_X, _T) CLOGFMT(_X, "Sess(%s)Seq(%s)Mtd(%s) " _T), session.c_str(), sequence.c_str(), method.c_str()

ssmNGODr2c1::ssmNGODr2c1() 
	: _pSite(NULL)
	, _pThreadPool(NULL)
	, _pCommunicator(NULL)
	, _pEventAdapter(NULL)
	, _pContextAdapter(NULL)
	, _pSessionManager(NULL)
	, _pEventChannal(NULL)
	, _pSafeStoreConn(NULL)
	, _pContextEvtr(NULL)
	, _pStreamIdx(NULL)
	, _pGroupIdx(NULL)
	, _pFactory(NULL)
	, _pSessionWatchDog(NULL)
	, _globalSequence(1)
{
	_sysLog.open("ssm_NGOD_r2c1.dll", ZQ::common::Log::L_WARNING);
}

ssmNGODr2c1::~ssmNGODr2c1()
{
	doUninit();
}

int ssmNGODr2c1::doInit(IStreamSmithSite* pSite)
{
	_pSite = pSite;

	if (false == loadConfig())
		return 1;

	if (false == parseConfig())
		return 1;
	
	try
	{
		_fileLog.open(_config._logFileName.c_str(), _config._logFileLevel, _config._logFileNum, _config._logFileSize, ZQLOG_DEFAULT_BUFFSIZE, ZQLOG_DEFAULT_FLUSHINTERVAL, ZQLOG_DEFAULT_EVENTLOGLEVEL, "ssm_NGOD_r2c1.dll");
	}
	catch (ZQ::common::FileLogException& ex)
	{
		_sysLog(ZQ::common::Log::L_EMERG, CLOGFMT(ssmNGODr2c1, "create log file caught [%s]"), ex.getString());
		return 1;
	}

	showConfig();

	if (false == initIceRunTime())
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "Ice run-time initialization failed, exit!"));
		return 1;
	}

	if (false == initThreadPool())
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "Thread pool initialization failed, exit!"));
		return 1;
	}

	if (false == openSessionWatchDog())
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "Session watch dog initialization failed, exit!"));
		return 1;
	}

	if (false == openSafeStore())
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "Open SafeStore initialization failed, exit!"));
		return 1;
	}

	_thrdConnService.open(this, &_fileLog);
	_thrdConnService.start();
	
	IClientSession* pSession = pSite->createClientSession(NULL, "rtsp://www.cctv.com:554/NGOD?asset=1");
	if (NULL != pSession && NULL != pSession->getSessionID())
		_globalSession = pSession->getSessionID();
	else 
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(ssmNGODr2c1, "global session create failed"));
		return 1;
	}

	return 0;
}

int ssmNGODr2c1::doUninit()
{
	NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "do uninitialization [%s]"), ZQ_PRODUCT_NAME);

	if(_thrdConnService.isRunning())
		_thrdConnService.stop();

	closeSafeStore();

	closeSessionWatchDog();

	uninitThreadPool();

	uninitIceRunTime();

	NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "uninitialization [%s] successfully"), ZQ_PRODUCT_NAME);

	return 0;
}

bool ssmNGODr2c1::openSessionWatchDog()
{
	_pSessionWatchDog = new SessionWatchDog(*this);

	if (NULL != _pSessionWatchDog)
	{
		_pSessionWatchDog->start();
		return true;
	}
	else 
		return false;
}

void ssmNGODr2c1::closeSessionWatchDog()
{
	if (NULL != _pSessionWatchDog)
		_pSessionWatchDog->quit();

	_pSessionWatchDog = NULL;
}

bool ssmNGODr2c1::initIceRunTime()
{
	try
	{
		// DO: initialize properties for ice run-time
		int i=0;
		Ice::PropertiesPtr props = Ice::createProperties(i, NULL);
		if (NULL != props)
		{
			STRINGMAP_CITOR config_itor;
			for (config_itor = _config._iceConfigMap.begin(); 
			config_itor != _config._iceConfigMap.end(); config_itor++)
			{
				props->setProperty(config_itor->first.c_str(), config_itor->second.c_str());
				NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "SetIceProperties: [%s] = [%s]"), config_itor->first.c_str(), config_itor->second.c_str());
			}
		}

		// DO: create communicator
		Ice::InitializationData idt;
		idt.properties = props;
		_pCommunicator=Ice::initialize(idt);
	}
	catch(const Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(ssmNGODr2c1, "create Ice Communicator caught [%s]"), ex.ice_name().c_str());
		return false;
	}
	
	try
	{
		_pEventAdapter = _pCommunicator->createObjectAdapterWithEndpoints("ListenEventAdapter", _config._eventAdapter);
		_pEventAdapter->activate();
	}
	catch(Ice::Exception& ex) 
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(ssmNGODr2c1, "create listen event adapter: [%s] caught [%s]"), _config._eventAdapter.c_str(), ex.ice_name().c_str());
		return false;
	}
	
	try
	{
		_pContextAdapter = _pCommunicator->createObjectAdapterWithEndpoints("ClientSessionAdapter", _config._contextAdapter);
		_pContextAdapter->activate();
	}
	catch(Ice::Exception& ex) 
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(ssmNGODr2c1, "create session context adapter: [%s] caught [%s]"), _config._contextAdapter.c_str(), ex.ice_name().c_str());
		return false;
	}

	return true;
}

void ssmNGODr2c1::uninitIceRunTime()
{
	try
	{
		if (NULL != _pEventAdapter)
		{
			_pEventAdapter->deactivate();
		}
		_pEventAdapter = NULL;
	}
	catch (Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(ssmNGODr2c1, "deactive adapters caught [%s]"), ex.ice_name().c_str());
	}

	try
	{
		if (NULL != _pContextAdapter)
		{
			_pContextAdapter->deactivate();
		}
		_pContextAdapter = NULL;
	}
	catch (Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(ssmNGODr2c1, "deactive adapters caught [%s]"), ex.ice_name().c_str());
	}
}

bool ssmNGODr2c1::initThreadPool()
{
	_pThreadPool = new ZQ::common::NativeThreadPool(_config._threadPoolSize);
	return (NULL != _pThreadPool) ? true : false;
}

void ssmNGODr2c1::uninitThreadPool()
{
	if (NULL != _pThreadPool)
		delete _pThreadPool;

	_pThreadPool = NULL;
}

int ssmNGODr2c1::setConfigPath(const char* pConfigPath)
{
	if (NULL != pConfigPath && strlen(pConfigPath) > 0)
		_configPath = pConfigPath;
	else 
		_sysLog(ZQ::common::Log::L_EMERG, CLOGFMT(ssmNGODr2c1, "configuration file path is empty"));

	return 0;
}

bool ssmNGODr2c1::loadConfig()
{
	char szBuf[2048];
	memset(szBuf, 0, sizeof(szBuf));

	_config._loadConfigMap.clear();

	std::vector<std::string> config_strs;
	config_strs.push_back("LogFile");
	config_strs.push_back("IceStorm");
	config_strs.push_back("SessionManager");
	config_strs.push_back("ListenEventAdapter");
	config_strs.push_back("ClientSession");
	config_strs.push_back("SafeStore");
	config_strs.push_back("NodeGroup");
	config_strs.push_back("Announce");
	config_strs.push_back("SetParameter");

	std::map<std::string, std::string> itemMap;
	ZQ::common::XMLPreferenceDocumentEx xml_doc;

	try
	{
		if (!xml_doc.open(_configPath.c_str()))
		{
			_sysLog(ZQ::common::Log::L_ERROR, CLOGFMT(ssmNGODr2c1, "open config: [%s] failed"), _configPath.c_str());
			return false;
		}

		ZQ::common::XMLPreferenceEx* pRoot = xml_doc.getRootPreference();
		
		ZQ::common::XMLPreferenceEx* pNGODr2c1 = pRoot->findSubPreference("ssm_NGOD_r2c1");
		if (NULL != pNGODr2c1)
		{
			int cfg_strs_size = config_strs.size();
			int tmp_cur = 0;
			for (tmp_cur = 0; tmp_cur < cfg_strs_size; tmp_cur++)
			{
				ZQ::common::XMLPreferenceEx* pItemCfg = pNGODr2c1->firstChild(config_strs[tmp_cur].c_str());
				if (pItemCfg != NULL)
				{
					std::map<std::string, std::string> properties = pItemCfg->getProperties();
					std::map<std::string, std::string>::const_iterator propItor;
					for (propItor = properties.begin(); propItor != properties.end(); propItor ++)
					{
						std::string keyStr,valStr;
						keyStr = config_strs[tmp_cur] + "." + propItor->first;
						valStr = propItor->second;
						_config._loadConfigMap[keyStr] = valStr;
					}

					pItemCfg->free();
				}
			}

			pNGODr2c1->free();
		}

		// <SOPRestriction>
		//   <sop name="sop1" serviceGroup="1" >
		//     <streamer netId="CL70001N0/board1" />
		//   </sop>
		// </SOPRestriction>
		ZQ::common::XMLPreferenceEx* pSOPRestriction = pRoot->findSubPreference("SOPRestriction");
		if (NULL != pSOPRestriction)
		{
			ZQ::common::XMLPreferenceEx* pSOP = NULL;
			char buf[256];
			SOPData sopData;
			sopData.streamers.attr = ::TianShanIce::SRM::raMandatoryNegotiable;
			sopData.streamers.status = ::TianShanIce::SRM::rsRequested;
			
			for (pSOP = pSOPRestriction->firstChild("sop"); NULL != pSOP; pSOP = pSOPRestriction->nextChild())
			{
				if (!pSOP->getAttributeValue("name", buf, sizeof(buf) -2))
					continue;
				std::string sopName = buf;
				sopData.serviceGroup = 0;
				
				if (pSOP->getAttributeValue("serviceGroup", buf, sizeof(buf) -2))
					sopData.serviceGroup = atoi(buf);

				::TianShanIce::Variant streamerNetIds;
				streamerNetIds.type = ::TianShanIce::vtStrings;
				streamerNetIds.bRange = false;
				sopData.streamers.resourceData.clear();
				
				for (ZQ::common::XMLPreferenceEx* pSopItem = pSOP->firstChild("streamer"); NULL != pSopItem; pSopItem = pSOP->nextChild())
				{
					if (!pSopItem->getAttributeValue("netId", buf, sizeof(buf) -2) && strlen(buf) >0)
						continue;
					
					streamerNetIds.strs.push_back(buf);
				}
				
				sopData.streamers.resourceData["NetworkId"] = streamerNetIds;
				
				if (!sopName.empty() && !streamerNetIds.strs.empty())
					_sopRestriction[sopName] = sopData;
			}

			pSOPRestriction->free();
		}

		ZQ::common::XMLPreferenceEx* pIceProperties = pRoot->findSubPreference("IceProperties");
		if (NULL != pIceProperties)
		{
			ZQ::common::XMLPreferenceEx* pProp = NULL;
			pProp = pIceProperties->firstChild("prop");
			while (NULL != pProp)
			{
				STRINGMAP props = pProp->getProperties();
				if (props.end() != props.find("name") && props.end() != props.find("value"))
				{
					std::string keystr, valuestr;
					keystr = props["name"];
					valuestr = props["value"];
					_config._iceConfigMap[keystr] = valuestr;
				}
				if (true == pIceProperties->hasNextChild())
					pProp = pIceProperties->nextChild();
				else 
					pProp = NULL;
			}
			pIceProperties->free();
		}
	}
	catch (ZQ::common::XMLException& ex)
	{
		_sysLog(ZQ::common::Log::L_ERROR, CLOGFMT(ssmNGODr2c1, "Load config: [%s] caught exception: [%s]"), _configPath.c_str(), ex.getString());
		return false;
	}

	return true;
}

bool ssmNGODr2c1::parseConfig()
{
	std::map<string, string>::iterator tItor;

	// 获得log文件的路径
	_config._logFileName = DEFAULT_LOGFILE_NAME; // 设置默认路径，预防没有配置该项
	tItor = _config._loadConfigMap.find("LogFile.name");
	if (tItor != _config._loadConfigMap.end())
		_config._logFileName = tItor->second;

	// 获得log文件的大小
	_config._logFileSize = DEFAULT_LOGFILE_SIZE; // 设置默认大小，预防没有配置该项
	tItor = _config._loadConfigMap.find("LogFile.size");
	if (tItor != _config._loadConfigMap.end() && ZQ::StringOperation::isInt(tItor->second))
		_config._logFileSize = atoi(tItor->second.c_str());
	
	// 获得log文件备份文件的数目
	_config._logFileNum = DEFAULT_LOGFILE_NUM; // 设置默认数目，预防没有配置该项
	tItor = _config._loadConfigMap.find("LogFile.number");
	if (tItor != _config._loadConfigMap.end() && ZQ::StringOperation::isInt(tItor->second))
		_config._logFileNum = atoi(tItor->second.c_str());
	
	// 获得log文件的level
	_config._logFileLevel = DEFAULT_LOGFILE_LEVEL;// 设置默认level，预防没有配置该项
	tItor = _config._loadConfigMap.find("LogFile.level");
	if (tItor != _config._loadConfigMap.end() && ZQ::StringOperation::isInt(tItor->second))
		_config._logFileLevel = atoi(tItor->second.c_str());	
	if (_config._logFileLevel < 0 || _config._logFileLevel > 7)
		_config._logFileLevel = DEFAULT_LOGFILE_LEVEL; // 如果配置的范围不合理，程序给定一个值

	// 获得Session Manager的end point
	tItor = _config._loadConfigMap.find("SessionManager.endPoint"); // 必须配置
	if (tItor != _config._loadConfigMap.end())
		_config._sessionManager = tItor->second;
	else 
	{
		_sysLog(ZQ::common::Log::L_EMERG, "no end point for Session Manager");
		return false; // 没有配置, 返回失败
	}

	// 获得Ice Storm的end point
	tItor = _config._loadConfigMap.find("IceStorm.endPoint"); // 必须配置
	if (tItor != _config._loadConfigMap.end())
		_config._iceStorm = tItor->second;
	else 
	{
		_sysLog(ZQ::common::Log::L_EMERG, "no end point for Ice Storm");
		return false; // 没有配置, 返回失败
	}

	// 获得本机侦听event的end point
	tItor = _config._loadConfigMap.find("ListenEventAdapter.endPoint"); // 必须配置
	if (tItor != _config._loadConfigMap.end())
		_config._eventAdapter = tItor->second;
	else 
	{
		_sysLog(ZQ::common::Log::L_ERROR, "no end point for listening events");
		return false; // 没有配置, 返回失败
	}

	// 获得本机用于管理client session context的end point
	tItor = _config._loadConfigMap.find("ClientSession.adapterEndPoint"); // 必须配置
	if (tItor != _config._loadConfigMap.end())
		_config._contextAdapter = tItor->second;
	else 
	{
		_sysLog(ZQ::common::Log::L_ERROR, "no end point for managing client session context");
		return false; // 没有配置, 返回失败
	}
	
	// 获得client session的timeout
	_config._timeoutInterval = DEFAULT_SESSION_TIMEOUT; // 设置默认值，预防没有配置该项	
	tItor = _config._loadConfigMap.find("ClientSession.timeout");
	if (tItor != _config._loadConfigMap.end() && ZQ::StringOperation::isInt(tItor->second))
		_config._timeoutInterval = atoi(tItor->second.c_str());
	if (_config._timeoutInterval < MIN_SESSION_TIMEOUT)
		_config._timeoutInterval = MIN_SESSION_TIMEOUT; // 如果配置值过小，加以调整
	
	// 获得管理client session context的evictor的active queue的大小
	_config._evictorSize = DEFAULT_EVICTOR_SIZE; // 设置默认值
	tItor = _config._loadConfigMap.find("ClientSession.evictorSize");
	if (tItor != _config._loadConfigMap.end() && ZQ::StringOperation::isInt(tItor->second))
		_config._evictorSize = atoi(tItor->second.c_str());
	if (_config._evictorSize < MIN_EVICTOR_SIZE)
		_config._evictorSize = MIN_EVICTOR_SIZE; // 如果配置值过小，加以调整
	if (_config._evictorSize > MAX_EVICTOR_SIZE)
		_config._evictorSize = MAX_EVICTOR_SIZE; // 如果配置值过大，加以调整

	// 获得session watch dog线程池的大小
	_config._threadPoolSize = DEFAULT_TheadPoolSize;
	tItor = _config._loadConfigMap.find("ClientSession.threadPoolSize");
	if (tItor != _config._loadConfigMap.end() && ZQ::StringOperation::isInt(tItor->second))
		_config._threadPoolSize = atoi(tItor->second.c_str());
	if (_config._threadPoolSize < MIN_TheadPoolSize)
		_config._threadPoolSize = MIN_TheadPoolSize;
	
	// 获得safe store的路径
	tItor = _config._loadConfigMap.find("SafeStore.path"); // 必须配置
	if (tItor != _config._loadConfigMap.end())
		_config._safeStorePath = tItor->second;
	else 
	{
		_sysLog(ZQ::common::Log::L_ERROR, "no safe store path");
		return false; // 没有配置，返回错误
	}
	
	// 获得setup时使用的node group id
	_config._nodeGroupID = "0"; // 设置默认值，预防没有配置	
	tItor = _config._loadConfigMap.find("NodeGroup.id");
	if (_config._loadConfigMap.end() != tItor)
		_config._nodeGroupID = tItor->second;

	_config._useGlobalSeq = false;
	tItor = _config._loadConfigMap.find("Announce.useGlobalSequence");
	if (tItor != _config._loadConfigMap.end() && stricmp(tItor->second.c_str(), "1") == 0)
		_config._useGlobalSeq = true;

	// 获得是否允许用SET_PARAMETER作为heart beat
	_config._bSetParamHeartBeat = false;
	tItor = _config._loadConfigMap.find("SetParameter.enableHeartBeat");
	if (tItor != _config._loadConfigMap.end() && stricmp(tItor->second.c_str(), "1") == 0)
		_config._bSetParamHeartBeat = true;

	return true;
}

void ssmNGODr2c1::showConfig()
{
	NGODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ssmNGODr2c1, "**************** [Load %s] ****************"), ZQ_PRODUCT_NAME);
	STRINGMAP_CITOR citor;
	for (citor = _config._loadConfigMap.begin(); citor != _config._loadConfigMap.end(); citor ++)
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "[%s] = [%s]"), citor->first.c_str(), citor->second.c_str());
	}

	for (citor = _config._iceConfigMap.begin(); citor != _config._iceConfigMap.end(); citor ++)
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "[%s] = [%s]"), citor->first.c_str(), citor->second.c_str());
	}
}

bool ssmNGODr2c1::ConnectIceStorm()
{
	try
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "do connect to IceStorm: [%s]"), _config._iceStorm.c_str());
		
		TianShanIce::Streamer::StreamEventSinkPtr _sEvent = new StreamEventSinkI(this);
		_pEventChannal=new TianShanIce::Events::EventChannelImpl(_pEventAdapter, _config._iceStorm.c_str());
		TianShanIce::Properties qos;
		_pEventChannal->sink(_sEvent, qos);
		
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "connect to IceStorm: [%s] successfully"), _config._iceStorm.c_str());
	}
	catch(const TianShanIce::BaseException& ex)
	{
		NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ssmNGODr2c1, "connect to IceStorm: [%s] caught %s:%s"), _config._iceStorm.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch(const Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ssmNGODr2c1, "connect to IceStorm: [%s] caught an %s"), _config._iceStorm.c_str(), ex.ice_name().c_str());
		return false;
	}

	return true;
}

bool ssmNGODr2c1::ConnectSessionManager()
{
	try
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "do connect to SessionManager: [%s]"), _config._sessionManager.c_str());
		
		_pSessionManager=TianShanIce::SRM::SessionManagerPrx::checkedCast(_pCommunicator->stringToProxy(_config._sessionManager));
		
		if (!_pSessionManager)
		{
			NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ssmNGODr2c1, "Connect to SessionManager: [%s] failed"), _config._sessionManager.c_str());
			return false;
		}

		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "Connect to SessionManager: [%s] successfully"), _config._sessionManager.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ssmNGODr2c1, "Connect to SessionManager: [%s] caught %s"), _config._sessionManager.c_str(), ex.ice_name().c_str());
		return false;
	}

	return true;
}

bool ssmNGODr2c1::openSafeStore()
{
	// DO: create directories
	if (_config._safeStorePath[_config._safeStorePath.size() - 1] != '\\'
		&& _config._safeStorePath[_config._safeStorePath.size() - 1] != '/')
	{
		_config._safeStorePath += '\\';
	}	
	std::string pathStr(_config._safeStorePath);
	std::vector<std::string> paths;
	std::string tmp_path;
	if (pathStr[pathStr.size() - 1] == '\\' || pathStr[pathStr.size() - 1] == '/')
		pathStr[pathStr.size() - 1] = '\0';
	paths.push_back(pathStr);
	tmp_path = ZQ::StringOperation::getLeftStr(pathStr, "\\/", false);
	while(tmp_path.size())
	{
		paths.push_back(tmp_path);
		tmp_path = ZQ::StringOperation::getLeftStr(tmp_path, "\\/", false);
	}
	int paths_size = paths.size();
	for (int i = paths_size - 1; i >= 0; i--)
	{
		::CreateDirectoryA(paths[i].c_str(), NULL);
	}

	try
	{
		_pFactory = new NGODFactory(*this);
		_pCommunicator->addObjectFactory(_pFactory, NGODr2c1::Context::ice_staticId());
	}
	catch (Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(ssmNGODr2c1, "Catch [%s] when create and add object factory "), ex.ice_name().c_str());
		return false;
	}

	try
	{
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "Do create freeze connection"));
		_pSafeStoreConn = ::Freeze::createConnection(_pCommunicator, _config._safeStorePath);
		NGODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ssmNGODr2c1, "Create freeze connection successfully"));
	}
	catch (Freeze::DatabaseException& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(ssmNGODr2c1, "Catch [%s]:[%s] when create freeze connection"), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(ssmNGODr2c1, "Catch [%s] when create freeze connection"), ex.ice_name().c_str());
		return false;
	}
	
	_pStreamIdx = new NGODr2c1::StreamIdx(INDEXFILENAME(StreamIdx));
	_pGroupIdx = new NGODr2c1::GroupIdx(INDEXFILENAME(GroupIdx));

	std::vector<Freeze::IndexPtr> indexs;
	indexs.push_back(_pStreamIdx);
	indexs.push_back(_pGroupIdx);

	ZQ::common::MutexGuard lk(_contextEvtrLock);
	try
	{
		_pContextEvtr = Freeze::createEvictor(_pContextAdapter, _config._safeStorePath.c_str(), "Contexts", 0, indexs);
		_pContextEvtr->setSize(_config._evictorSize);
		_pContextAdapter->addServantLocator(_pContextEvtr, SERVANT_TYPE);
	}
	catch (Freeze::DatabaseException& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(ssmNGODr2c1, "create freeze evictor caught [%s]:[%s]"), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (Ice::Exception& ex)
	{
		NGODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(ssmNGODr2c1, "create freeze evictor caught [%s]"), ex.ice_name().c_str());
		return false;
	}

	INOUTMAP inoutMap;
	inoutMap[MAP_KEY_METHOD] = "init";
	::Freeze::EvictorIteratorPtr tItor = _pContextEvtr->getIterator("", MAX_SESSION_CONTEXT);
	while (tItor->hasNext())
	{
		Ice::Identity ident = tItor->next();
		NGODr2c1::ContextImplPtr pNewContext = new NGODr2c1::ContextImpl(*this);
		NGODr2c1::ContextPrx pNewContextPrx = NULL;
		if (openContext(ident.name, pNewContext, pNewContextPrx, inoutMap))
		{
#pragma message(__MSGLOC__"watch client session here, is this right?")
			_pSessionWatchDog->watchSession(pNewContext->ident, 0);
			if (NULL != _pSite->createClientSession(pNewContext->ident.name.c_str(), pNewContext->resourceURL.c_str()))
				NGODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ssmNGODr2c1, "create client session: [%s] when initialize plug-in"), pNewContext->ident.name.c_str());
		}
	}

	return true;
}

void ssmNGODr2c1::closeSafeStore()
{
	if (NULL != _pSafeStoreConn)
	{
		try
		{
			_pSafeStoreConn->close();
		}
		catch (Freeze::DatabaseException& ex)
		{
			NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ssmNGODr2c1, "Do close freeze connection caught [%s]:[%s]"), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (Ice::Exception& ex)
		{
			NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ssmNGODr2c1, "Do close freeze connection caught [%s]"), ex.ice_name().c_str());
		}
	}
	_pSafeStoreConn = NULL;
}

bool ssmNGODr2c1::openContext(const std::string& sessId, NGODr2c1::ContextImplPtr& pContext, NGODr2c1::ContextPrx& pContextPrx, INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE] = "\0";

	Ice::Identity ident;
	ident.category = SERVANT_TYPE;
	ident.name = sessId;
	if (ident.name.empty())
		ident.name = inoutMap[MAP_KEY_SESSION];

	std::string method, sequence, session;
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];
	session = ident.name;

	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(ssmNGODr2c1, "openContext()"));

	try
	{
		pContextPrx = NGODr2c1::ContextPrx::uncheckedCast(_pContextAdapter->createProxy(ident));
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "uncheckedCast to client session context proxy caught [%s]", ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	if (NULL == pContextPrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "result client session context proxy is NULL");
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	NGODr2c1::ctxData ctxDt;
	try
	{
		ctxDt = pContextPrx->getState();
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "do getState() on session context: [%s] caught [%s]", ident.name.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}
	
	pContext->ident = ctxDt.ident;
	pContext->weiwooFullID = ctxDt.weiwooFullID;
	pContext->onDemandID = ctxDt.onDemandID;
	pContext->streamFullID = ctxDt.streamFullID;
	pContext->streamShortID = ctxDt.streamShortID;
	pContext->normalURL = ctxDt.normalURL;
	pContext->resourceURL = ctxDt.resourceURL;
	pContext->connectID = ctxDt.connectID;
	pContext->groupID = ctxDt.groupID;
	pContext->expiration = ctxDt.expiration;
	pContext->announceSeq = ctxDt.announceSeq;

	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(ssmNGODr2c1, "session context gained successfully, GUID: [%s]"), ident.name.c_str());

	return true;
}

bool ssmNGODr2c1::addContext(const NGODr2c1::ContextImplPtr pContext, INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE];
	szBuf[sizeof(szBuf) - 1] = '\0';

	std::string session, method, sequence;
	session = inoutMap[MAP_KEY_SESSION];
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];	

	if (NULL == pContext)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "context pointer is null");
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(ssmNGODr2c1, "to save session context, GUID: [%s]"), pContext->ident.name.c_str());

	::Ice::ObjectPrx basePrx = NULL;
	try
	{
		ZQ::common::MutexGuard lk(_contextEvtrLock);
		basePrx = _pContextEvtr->add(pContext, pContext->ident);
	}
	catch (Freeze::DatabaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "save context to evictor caught [%s]:[%s], GUID: [%s]", ex.ice_name().c_str(), ex.message.c_str(), pContext->ident.name.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "save context to evictor caught [%s], GUID: [%s]", ex.ice_name().c_str(), pContext->ident.name.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	if (NULL == basePrx)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1, "save context result is null, GUID: [%s]", pContext->ident.name.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(ssmNGODr2c1, "session context saved successfully, GUID: [%s]"), pContext->ident.name.c_str());

	return true;
}

RequestProcessResult ssmNGODr2c1::doFixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	NGODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ssmNGODr2c1, "Req(%p), enter FixupRequest"), pReq);

	char szBuf[MY_BUFFER_SIZE];
	uint16 szBufLen;
	memset(szBuf, 0, sizeof(szBuf));

	IServerResponse* pResponse = NULL;
	pResponse = pReq->getResponse();
	if (NULL == pResponse)
	{
		NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ssmNGODr2c1, "Req(%p), response object is null"), pReq);
		return RequestError;
	}

	std::string session, method, sequence;
	const char* pHeaderStr = NULL;
	szBufLen = sizeof(szBuf) - 1;
	pHeaderStr = pReq->getHeader(NGOD_HEADER_SESSION, szBuf, &szBufLen);
	if (NULL != pHeaderStr)
	{
		NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(ssmNGODr2c1, "Req(%p), session: [%s]"), pReq, session.c_str());
		session = pHeaderStr;
	}
	else 
	{
		NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(ssmNGODr2c1, "Req(%p), No session id, use global session: [%s]"), pReq, _globalSession.c_str());
		session = _globalSession;
		pReq->setHeader(NGOD_HEADER_SESSION, (char*) session.c_str());
	}

	pHeaderStr = NULL;
	szBufLen = sizeof(szBuf) - 1;
	pHeaderStr = pReq->getHeader(NGOD_HEADER_SEQ, szBuf, &szBufLen);
	sequence = (NULL != pHeaderStr) ? pHeaderStr : "";

	// DO: get value of server header
	if (true == _serverHeader.empty())
	{
	szBufLen = sizeof(szBuf) - 1;
	const char* pServer = pReq->getHeader(NGOD_HEADER_SERVER, szBuf, &szBufLen);
	if (pServer != NULL && strlen(pServer) != 0)
	{
		_serverHeader = pServer;
		_serverHeader += "; ";
		_serverHeader += ZQ_PRODUCT_NAME_SHORT;
	}
	else 
		_serverHeader = ZQ_PRODUCT_NAME_SHORT;
	}

	RTSP_VerbCode retVerbCode = pReq->getVerb();
	
	if (retVerbCode == RTSP_MTHD_SETUP)
	{
		std::string originalURI, strResourceURI;
		// DO: get url from ODRM request, and format it to "rtsp://<server>:<port>
		pReq->getUri(szBuf, sizeof(szBuf) - 1);
		originalURI = "rtsp://";
		originalURI += szBuf;
		
		// DO: get content body form request, and then take out the value of providerId and AssetId
		unsigned char contentBody[MY_BUFFER_SIZE];
		uint32 contentLen;
		memset(contentBody, 0, sizeof(contentBody));
		const char* pContentBody = NULL;
		contentLen = sizeof(contentBody) - 1;
		pContentBody = pReq->getContent(contentBody, &contentLen);

		std::string strContent;
		strContent = (NULL != pContentBody) ? pContentBody : "";

		int asset_count = 0;
		const char* next_content = pContentBody;
		const char* pTemp = NULL;
		std::vector<std::string> assets;
		pTemp = strstr(next_content, "a=X-playlist-item:");
		while (pTemp != NULL)
		{
			pTemp += strlen("a=X-playlist-item:");
			next_content = pTemp;
			std::vector<std::string> temp_strs;
			ZQ::StringOperation::splitStr(pTemp, " \r\n\t", temp_strs);
			if (temp_strs.size() >= 2)
			{
				std::string provider_encode, asset_encode;
				int outlen = MY_BUFFER_SIZE - 1;
				ZQ::common::URLStr::encode(temp_strs[0].c_str(), szBuf, outlen);
				provider_encode = szBuf;
				outlen = MY_BUFFER_SIZE - 1;
				ZQ::common::URLStr::encode(temp_strs[1].c_str(), szBuf, outlen);
				asset_encode = szBuf;
				snprintf(szBuf, MY_BUFFER_SIZE - 1, "item%d=%s#%s", asset_count, provider_encode.c_str(), asset_encode.c_str());
				if (temp_strs.size() >= 3) {
					std::string queIn, queOut, tStr(szBuf);
					int tPos;
					if (true == ZQ::StringOperation::hasChar(temp_strs[2], '-', tPos)) {
						queIn = ZQ::StringOperation::midStr(temp_strs[2], -1, tPos);
						queOut = ZQ::StringOperation::rightStr(temp_strs[2], tPos);
						if (false == queIn.empty())
							snprintf(szBuf, MY_BUFFER_SIZE - 1, "%s&cueIn%d=%s", tStr.c_str(), asset_count, queIn.c_str());
						if (false == queOut.empty())
						{
							tStr = szBuf;
							snprintf(szBuf, MY_BUFFER_SIZE - 1, "%s&cueOut%d=%s", tStr.c_str(), asset_count, queOut.c_str());
						}
					}
					else if (ZQ::StringOperation::isInt(temp_strs[2])) {
						snprintf(szBuf, MY_BUFFER_SIZE - 1, "%s&cueIn%d=%s", tStr.c_str(), asset_count, temp_strs[2].c_str());
					}
				}
				assets.push_back(szBuf);
				asset_count++;
			}
			pTemp = strstr(next_content, "a=X-playlist-item:");
		}

		// DO: format the strResourceURI to "rtsp://<server>:<port>/NGOD?asset=<providerId>.<assetId>"
		strResourceURI = originalURI + "NGOD?compatible=Comcast.ngod&";
		int assets_size = assets.size();
		for (int tmp_int = 0; tmp_int < assets_size; tmp_int++)
		{
			strResourceURI += assets[tmp_int];
			if (tmp_int < assets_size - 1)
				strResourceURI += '&';
		}
		
		pReq->setHeader(ORIGINAL_URI, (char*)originalURI.c_str());
		pReq->setHeader(RESOURCE_URI, (char*)strResourceURI.c_str());
		pReq->getProtocol(szBuf, MY_BUFFER_SIZE - 1);
		pReq->setArgument(pReq->getVerb(), strResourceURI.c_str(), szBuf);
	}
	else if (retVerbCode == RTSP_MTHD_RESPONSE)
	{
		NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(ssmNGODr2c1, "Req(%p), CLIENT RESPONSE"), pReq);
		
		NGODr2c1::ContextImplPtr pNewContext = new NGODr2c1::ContextImpl(*this);
		NGODr2c1::ContextPrx pNewContextPrx = NULL;
		INOUTMAP inoutMap;
		inoutMap[MAP_KEY_SESSION] = session;
		inoutMap[MAP_KEY_SEQUENCE] = sequence;

		if (true == openContext(session, pNewContext, pNewContextPrx, inoutMap))
		{
			std::string url_str;
			url_str = pNewContext->resourceURL;
			memset(szBuf, 0, sizeof(szBuf));
			pReq->getStartline(szBuf, sizeof(szBuf) - 1);
			std::vector<std::string> starts;
			ZQ::StringOperation::splitStr(szBuf, " \r\n\t", starts);
			if (starts.size() >= 2 && (strcmp(starts[1].c_str(), "200") == 0 || strcmp(starts[1].c_str(), "454") == 0))
			{
				if (strcmp(starts[1].c_str(), "200") == 0)
					pReq->setArgument(RTSP_MTHD_PING, url_str.c_str(), starts[0].c_str());
				else 
					pReq->setArgument(RTSP_MTHD_TEARDOWN, url_str.c_str(), starts[0].c_str());
			}
			pReq->setHeader("NeedResponse", "no");
		}
	}

	NGODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ssmNGODr2c1, "Req(%p), leave FixupRequest"), pReq);

	return RequestProcessed;
}

RequestProcessResult ssmNGODr2c1::doContentHandler(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	NGODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ssmNGODr2c1, "Req(%p), enter ContentHandler"), pReq);

	RequestHandler* pRequestHandler = NULL;
	SmartRequestHandler smartHandler(pRequestHandler);
	RTSP_VerbCode verbCode = pReq->getVerb();
	switch(verbCode)
	{
	case RTSP_MTHD_SET_PARAMETER:
		{
			pRequestHandler = new SetParamHandler(*this, pSite, pReq);
		}
		break;
	case RTSP_MTHD_SETUP:
		{
			pRequestHandler = new SetupHandler(*this, pSite, pReq);
		}
		break;
	case RTSP_MTHD_PLAY:
		{
			pRequestHandler = new PlayHandler(*this, pSite, pReq);
		}
		break;
	case RTSP_MTHD_PAUSE:
		{
			pRequestHandler = new PauseHandler(*this, pSite, pReq);
		}
		break;
	case RTSP_MTHD_TEARDOWN:
		{
			pRequestHandler = new TeardownHandler(*this, pSite, pReq);
		}
		break;
	case RTSP_MTHD_GET_PARAMETER:
		{
			pRequestHandler = new GetParamHandler(*this, pSite, pReq);
		}
		break;
	case RTSP_MTHD_OPTIONS:
		{
			pRequestHandler = new OptionHandler(*this, pSite, pReq);
		}
		break;
	case RTSP_MTHD_PING:
		{
			pRequestHandler = new PingHandler(*this, pSite, pReq);
		}
		break;
	default:
		{
		}
		break;
	}

	if (NULL == pRequestHandler)
	{
		NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(ssmNGODr2c1, "request(%p) is not acceptable request"));
		return RequestError;
	}

	try
	{
		::Ice::Long timeUsed = ZQTianShan::now();
		RequestProcessResult ret = pRequestHandler->process();
		timeUsed = ZQTianShan::now() - timeUsed;
		if (RequestProcessed == ret)
			NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(ssmNGODr2c1, "[%s][success]process[%s]request, used [%lld]ms"), pRequestHandler->getSession().c_str(), pRequestHandler->getRequestType().c_str(), timeUsed);
		else 
		{
			if (verbCode == RTSP_MTHD_SETUP)
				_pSite->destroyClientSession(pRequestHandler->getSession().c_str());
			NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(ssmNGODr2c1, "[%s][failed]process[%s]request, used [%lld]ms"), pRequestHandler->getSession().c_str(), pRequestHandler->getRequestType().c_str(), timeUsed);
		}
	}
	catch (...)
	{
		char szBuf[MY_BUFFER_SIZE];
		szBuf[sizeof(szBuf) - 1] = '\0';
		snprintf(szBuf, sizeof(szBuf) - 1, "process [%s] request caught an unexpect exception", pRequestHandler->getRequestType().c_str());
		IServerResponse* pResponse = NULL;
		pResponse = pReq->getResponse();
		if (NULL != pResponse)
		{
			pResponse->printf_preheader(RESPONSE_INTERNAL_ERROR);
			if (_globalSession != pRequestHandler->getSession())
				pResponse->setHeader(NGOD_HEADER_SESSION, pRequestHandler->getSession().c_str());
			pResponse->setHeader(NGOD_HEADER_SERVER, _serverHeader.c_str());
			pResponse->setHeader(NGOD_HEADER_SERVER, pRequestHandler->getRequestType().c_str());
			pResponse->setHeader(NGOD_HEADER_NOTICE, szBuf);
			pResponse->post();
		}
	}
	
	NGODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ssmNGODr2c1, "Req(%p), leave ContentHandler"), pReq);

	return RequestDone;
}

RequestProcessResult ssmNGODr2c1::doFixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq)
{
	return RequestDone;
}

bool ssmNGODr2c1::removeContext(const std::string& sessId, INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE] = "\0";

	Ice::Identity ident;
	ident.category = SERVANT_TYPE;
	ident.name = sessId;
	if (ident.name.empty())
		ident.name = inoutMap[MAP_KEY_SESSION];

	std::string method, sequence, session;
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];
	session = ident.name;

	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(ssmNGODr2c1, "removeContext()"));

	try
	{
		ZQ::common::MutexGuard lk(_contextEvtrLock);
		_pContextEvtr->remove(ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform remove() session[%s] caught [%s]:%s", ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform remove() session[%s] caught [%s]", ident.name.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}
	catch (...)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform remove() session[%s] caught unknown exception", ident.name.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(ssmNGODr2c1, "context removed session[%s] successfully"), ident.name.c_str());

	return true;

}

bool ssmNGODr2c1::getWeiwooSession(TianShanIce::SRM::SessionPrx& sessionPrx, INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE];
	szBuf[sizeof(szBuf) - 1] = '\0';

	std::string session, method, sequence;
	session = inoutMap[MAP_KEY_SESSION];
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];

	std::string weiwooPrxID = inoutMap[MAP_KEY_WEIWOOSESSIONFULLID];
	
	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(ssmNGODr2c1, "to gain weiwoo session: [%s]"), weiwooPrxID.c_str());

	try
	{
		sessionPrx = TianShanIce::SRM::SessionPrx::uncheckedCast(_pCommunicator->stringToProxy(weiwooPrxID));
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "get weiwoo session(%s) caught: [%s]", weiwooPrxID.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	if (NULL == sessionPrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "result weiwoo session proxy: [%s] is null", weiwooPrxID.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(ssmNGODr2c1, "weiwoo session: [%s] gained successfully"), weiwooPrxID.c_str());

	return true;
}

bool ssmNGODr2c1::renewWeiwooSession(const TianShanIce::SRM::SessionPrx& sessionPrx, int timeout_value, INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE];
	szBuf[sizeof(szBuf) - 1] = '\0';

	std::string session, method, sequence;
	session = inoutMap[MAP_KEY_SESSION];
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];

	std::string weiwooid = inoutMap[MAP_KEY_WEIWOOSESSIONFULLID];
	
	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(ssmNGODr2c1, "to renew weiwoo session: [%s]"), weiwooid.c_str());

	try
	{
		sessionPrx->renew(timeout_value);
	}
	catch(const TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform renew() on weiwoo session: [%s] proxy caught [%s]:[%s]", weiwooid.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}
	catch(const Ice::Exception& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform renew() on weiwoo session: [%s] proxy caught [%s]", weiwooid.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(ssmNGODr2c1, "weiwoo session: [%s] renewed for [%d]ms successfully"), weiwooid.c_str(), timeout_value);
	
	return true;
}

std::string ssmNGODr2c1::getCurrentConnID(NGODr2c1::ContextImplPtr pContext)
{
	ZQ::common::MutexGuard lk(_connIDGroupsLock);
	
	int size = _connIDGroups.size();
	int i = 0;
	
	for (i = 0; i < size; i++)
	{
		ConnIDGroupPair& _pair = _connIDGroups[i];
		if (_pair._sessionGroup == pContext->groupID)
		{
			return _pair._connectionID;
		}
	}

	return pContext->connectID;
}

bool ssmNGODr2c1::destroyWeiwooSession(const TianShanIce::SRM::SessionPrx& sessPrx, INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE];
	szBuf[sizeof(szBuf) - 1] = '\0';

	std::string session, method, sequence;
	session = inoutMap[MAP_KEY_SESSION];
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];

	std::string weiwooPrxID = inoutMap[MAP_KEY_WEIWOOSESSIONFULLID];
	std::string destroyReason = inoutMap[MAP_KEY_WEIWOOSESSIONDESTROYREASON];

	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(ssmNGODr2c1, "to destory weiwoo session: [%s], reason: [%s]"), weiwooPrxID.c_str(), destroyReason.c_str());

	try
	{
		::Ice::Context ctx;
		ctx["caller"] = destroyReason;
		sessPrx->destroy(ctx);
	}
	catch (TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "reason: [%s], perform destory() on weiwoo session: [%s] caught [%s]:[%s]", destroyReason.c_str(), weiwooPrxID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "reason: [%s], perform destory() on weiwoo session: [%s] caught [%s]", destroyReason.c_str(), weiwooPrxID.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(ssmNGODr2c1, "weiwoo session: [%s] destroyed successfully, reason: [%s]"), weiwooPrxID.c_str(), destroyReason.c_str());

	return true;
}

bool ssmNGODr2c1::getStream(::TianShanIce::Streamer::StreamPrx& streamPrx, INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE];
	szBuf[sizeof(szBuf) - 1] = '\0';

	std::string session, method, sequence;
	session = inoutMap[MAP_KEY_SESSION];
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];

	std::string streamid = inoutMap[MAP_KEY_STREAMFULLID];

	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(ssmNGODr2c1, "to gain stream: [%s]"), streamid.c_str());

	try
	{
		streamPrx = ::TianShanIce::Streamer::StreamPrx::uncheckedCast(_pCommunicator->stringToProxy(streamid));
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform uncheckedCast to stream: [%s] caught [%s]", streamid.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	if (NULL == streamPrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "result stream proxy: [%s] is null", streamid.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(ssmNGODr2c1, "stream: [%s] gained successfully"), streamid.c_str());

	return true;
}

bool ssmNGODr2c1::getPurchase(::TianShanIce::Application::PurchasePrx& purchasePrx
		, const ::TianShanIce::SRM::SessionPrx sessionPrx, INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE];
	szBuf[sizeof(szBuf) - 1] = '\0';

	std::string session, method, sequence;
	session = inoutMap[MAP_KEY_SESSION];
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];

	std::string weiwooid = inoutMap[MAP_KEY_WEIWOOSESSIONFULLID];

	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(ssmNGODr2c1, "to gain purchase proxy via weiwoo session: [%s]"), weiwooid.c_str());

	if (NULL == sessionPrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "weiwoo session: [%s] proxy is null", weiwooid.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	try
	{
		purchasePrx = sessionPrx->getPurchase();
	}
	catch (TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform getPurchase() on weiwoo session: [%s] caught [%s]:[%s]", weiwooid.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}
	catch (Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform getPurchase() on weiwoo session: [%s] caught [%s]", weiwooid.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	if (NULL == purchasePrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform getPurchase() on weiwoo session: [%s] successfully, but result is null", weiwooid.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(ssmNGODr2c1, "purchase proxy gained via weiwoo session: [%s] successfully"), weiwooid.c_str());

	return true;
}

bool ssmNGODr2c1::getStreamState(TianShanIce::Streamer::StreamState& strmState
								 , const TianShanIce::Streamer::StreamPrx& strmPrx
								 , INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE];
	szBuf[sizeof(szBuf) - 1] = '\0';

	std::string session, method, sequence;
	session = inoutMap[MAP_KEY_SESSION];
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];

	std::string streamid = inoutMap[MAP_KEY_STREAMFULLID];

	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(ssmNGODr2c1, "to gain stream: [%s] state"), streamid.c_str());

	if (NULL == strmPrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "stream: [%s] proxy is null", streamid.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	try
	{
		strmState=strmPrx->getCurrentState();
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform getCurrentState() on stream: [%s] caught %s:%s", streamid.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}
	catch(const ::Ice::Exception& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"perform getCurrentState() on stream: [%s] caught %s", streamid.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	switch(strmState)
	{
	case TianShanIce::Streamer::stsSetup:
		{
		inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION] = "init";
		}
		break;
	case TianShanIce::Streamer::stsStreaming: 
		{
		inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION] = "play";
		}
		break;
	case TianShanIce::Streamer::stsPause: 
		{
		inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION] = "pause";
		}
		break;
	case TianShanIce::Streamer::stsStop:
		{
		inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION] = "ready";
		}
		break;
	default: 
		{
		inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION] = "unknown";
		}
		break;
	}
	
	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(ssmNGODr2c1, "stream: [%s], state: [%s] gained successfully"), streamid.c_str(), inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION].c_str());

	return true;
}

bool ssmNGODr2c1::getPositionAndScale(const TianShanIce::Streamer::StreamPrx& strmPrx, INOUTMAP& inoutMap)
{
	char szBuf[MY_BUFFER_SIZE];
	szBuf[sizeof(szBuf) - 1] = '\0';

	std::string session, method, sequence;
	session = inoutMap[MAP_KEY_SESSION];
	method = inoutMap[MAP_KEY_METHOD];
	sequence = inoutMap[MAP_KEY_SEQUENCE];

	std::string streamid = inoutMap[MAP_KEY_STREAMFULLID];
	
	NGODLOG(ZQ::common::Log::L_DEBUG, NGODLOGFMT(ssmNGODr2c1, "to gain stream: [%s] position and scale"), streamid.c_str());

	if (NULL == strmPrx)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "stream: [%s] proxy is null", streamid.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	TianShanIce::Streamer::PlaylistPrx playlist = NULL;
	try
	{
		playlist = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(strmPrx);
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "checkedCast stream: [%s] to playlist caught [%s]:[%s]", streamid.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}
	catch(const ::Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "checkedCast stream: [%s] to playlist caught [%s]", streamid.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}

	if (NULL == playlist)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "checkedCast stream: [%s] to playlist, but result is null", streamid.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}
	
	TianShanIce::ValueMap valMap;
	try
	{
		playlist->getInfo(::TianShanIce::Streamer::infoPLAYPOSITION, valMap);
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform getInfo() on stream: [%s] caught [%s]:[%s]", streamid.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}
	catch(const ::Ice::Exception& ex)
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "perform getInfo() on stream: [%s] caught [%s]", streamid.c_str(), ex.ice_name().c_str());
		NGODLOG(ZQ::common::Log::L_ERROR, NGODLOGFMT(ssmNGODr2c1, "%s"), szBuf);
		inoutMap[MAP_KEY_LASTERROR] = szBuf;
		return false;
	}
	
	TianShanIce::ValueMap::iterator it_cp, it_tp, it_scale;
	it_cp = valMap.find("playposition");
	it_tp = valMap.find("totalplaytime");
	it_scale = valMap.find("scale");
	if (it_cp != valMap.end() && it_cp->second.ints.size()
		&& it_tp != valMap.end()  && it_tp->second.ints.size()
		&& it_scale != valMap.end() && it_scale->second.strs.size())
	{
		int cur, total;
		cur = it_cp->second.ints[0];
		total = it_tp->second.ints[0];
		inoutMap[MAP_KEY_STREAMSCALE] = it_scale->second.strs[0];
		snprintf(szBuf, sizeof(szBuf) - 1, "%d.%d-%d.%d", cur/1000, cur%1000, total/1000, total%1000);
		inoutMap[MAP_KEY_STREAMPOSITION] = szBuf;
	}
	
	NGODLOG(ZQ::common::Log::L_INFO, NGODLOGFMT(ssmNGODr2c1, "stream: [%s]'s position: [%s] and scale: [%s] gained successfully")
		, streamid.c_str(), inoutMap[MAP_KEY_STREAMPOSITION].c_str(), inoutMap[MAP_KEY_STREAMSCALE].c_str());

	return true;
}

bool ssmNGODr2c1::sessionInProgressAnnounce(NGODr2c1::ContextImplPtr pContext)
{
	if (NULL == pContext)
		return false;

	_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(ssmNGODr2c1, "sessionInProgressAnnounce() Session[%s]"), pContext->ident.name.c_str());
	try
	{
		STRINGMAP inoutMap;
		inoutMap[MAP_KEY_SESSION] = pContext->ident.name;
		inoutMap[MAP_KEY_METHOD] = "InProgressAnnounce";
		
		std::string sequence;

		if (true == _config._useGlobalSeq)
		{
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", _globalSequence++);
			sequence = tbuff;
		}
		else 
		{
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", pContext->announceSeq);
			sequence = tbuff;
		}
		inoutMap[MAP_KEY_SEQUENCE] = sequence;
		
		TianShanIce::SRM::SessionPrx sessionPrx = NULL;
		inoutMap[MAP_KEY_WEIWOOSESSIONFULLID] = pContext->weiwooFullID;
		if (true == getWeiwooSession(sessionPrx, inoutMap))
			renewWeiwooSession(sessionPrx, 1000 * _config._timeoutInterval + 60000, inoutMap);
		
		std::string currentConnID;
		currentConnID = getCurrentConnID(pContext);
		
		IServerRequest* pOdrm = NULL;
		SmartServerRequest smtOdrm(pOdrm);
		pOdrm = _pSite->newServerRequest(pContext->ident.name.c_str(), currentConnID);
		if(NULL != pOdrm)
		{
			std::string responseHead = "ANNOUNCE " + pContext->normalURL + " RTSP/1.0";
			
			std::string notice_str;
			notice_str = NGOD_ANNOUNCE_SESSIONINPROGRESS " \"" NGOD_ANNOUNCE_SESSIONINPROGRESS_STRING "\" " "eventdate=";
			SYSTEMTIME time;
			GetLocalTime(&time);
			char t[50];
			memset(t, 0, 50);
			snprintf(t, 49, "%04d%02d%02dT%02d%02d%02d.%03dZ npt=",time.wYear,time.wMonth,time.wDay,
				time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
			notice_str += t;
			
			// TODO: send session timeout announce
			pOdrm->printCmdLine(responseHead.c_str());
			pOdrm->printHeader(NGOD_HEADER_SESSION, (char*) pContext->ident.name.c_str());
			pOdrm->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) pContext->onDemandID.c_str());
			pOdrm->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
			pOdrm->printHeader(NGOD_HEADER_SERVER, (char*) _serverHeader.c_str());
			pOdrm->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
			pOdrm->post();

			NGODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ssmNGODr2c1, "[Session In Progress] has been sent out, session: [%s]"), pContext->ident.name.c_str());
		}
		else 
			NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ssmNGODr2c1, "create server request failed, session: [%s]"), pContext->ident.name.c_str());
	}
	catch (...)
	{
		NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ssmNGODr2c1, "process [Session In Progress] caught unexpect exception, session: [%s]"), pContext->ident.name.c_str());
		return false;
	}

	return true;
}

bool ssmNGODr2c1::terminatAndAnnounce(NGODr2c1::ContextImplPtr pContext)
{
	if (NULL == pContext)
		return false;

	_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(ssmNGODr2c1, "terminatAndAnnounce() Session[%s]"), pContext->ident.name.c_str());

	try
	{
		STRINGMAP inoutMap;
		inoutMap[MAP_KEY_SESSION] = pContext->ident.name;
		inoutMap[MAP_KEY_METHOD] = "SessionTerminated";
	
		std::string sequence;

		if (true == _config._useGlobalSeq)
		{
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", _globalSequence++);
			sequence = tbuff;
		}
		else 
		{
			char tbuff[20];
			tbuff[sizeof(tbuff) - 1] = '\0';
			snprintf(tbuff, sizeof(tbuff) - 1, "%d", pContext->announceSeq);
			sequence = tbuff;
		}
		inoutMap[MAP_KEY_SEQUENCE] = sequence;
		
		std::string currentConnID;
		currentConnID = getCurrentConnID(pContext);
		
		TianShanIce::SRM::SessionPrx sessionPrx = NULL;
		inoutMap[MAP_KEY_WEIWOOSESSIONFULLID] = pContext->weiwooFullID;
		inoutMap[MAP_KEY_WEIWOOSESSIONDESTROYREASON] = "TerminateAnnounce";
		
		// DO: destroy weiwoo session
		if (true == getWeiwooSession(sessionPrx, inoutMap))
				destroyWeiwooSession(sessionPrx, inoutMap);

		// DO: remove session context
		removeContext(pContext->ident.name, inoutMap);
		
		IServerRequest* pOdrm = NULL;
		SmartServerRequest smtOdrm(pOdrm);
		pOdrm = _pSite->newServerRequest(pContext->ident.name.c_str(), currentConnID);
		if(NULL != pOdrm)
		{
			std::string responseHead = "ANNOUNCE " + pContext->normalURL + " RTSP/1.0";
			
			std::string notice_str;
			notice_str = NGOD_ANNOUNCE_CLIENTSESSIONTERMINATED " \"" NGOD_ANNOUNCE_CLIENTSESSIONTERMINATED_STRING "\" " "eventdate=";
			SYSTEMTIME time;
			GetLocalTime(&time);
			char t[50];
			memset(t, 0, 50);
			snprintf(t, 49, "%04d%02d%02dT%02d%02d%02d.%03dZ npt=",time.wYear,time.wMonth,time.wDay,
				time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
			notice_str += t;
			
//			inoutMap[MAP_KEY_STREAMFULLID] = pContext->streamFullID;
//			::TianShanIce::Streamer::StreamPrx strmPrx = NULL;
//			if (true == getStream(strmPrx, inoutMap))
//			{
//				if (true == getPositionAndScale(strmPrx, inoutMap))
//					notice_str += inoutMap[MAP_KEY_STREAMPOSITION];
//			}
			
			// TODO: send session timeout announce
			pOdrm->printCmdLine(responseHead.c_str());
			pOdrm->printHeader(NGOD_HEADER_SESSION, (char*) pContext->ident.name.c_str());
			pOdrm->printHeader(NGOD_HEADER_ONDEMANDSESSIONID, (char*) pContext->onDemandID.c_str());
			pOdrm->printHeader(NGOD_HEADER_NOTICE, (char*) notice_str.c_str());
			pOdrm->printHeader(NGOD_HEADER_SERVER, (char*) _serverHeader.c_str());
			pOdrm->printHeader(NGOD_HEADER_SEQ, (char *)sequence.c_str());
			pOdrm->post();

			NGODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ssmNGODr2c1, "[Client Session Terminated] has been sent out , session: [%s]"), pContext->ident.name.c_str());
		}
		else 
			NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ssmNGODr2c1, "create server request failed, session: [%s]"), pContext->ident.name.c_str());
		
		// DO: destroy rtsp client session
		if (true == _pSite->destroyClientSession(pContext->ident.name.c_str()))
			NGODLOG(ZQ::common::Log::L_INFO, CLOGFMT(ssmNGODr2c1, "rtspProxy session destroyed, session: [%s]"), pContext->ident.name.c_str());
	}
	catch (...)
	{
		NGODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ssmNGODr2c1, "process [Client Session Terminated] caught unexpect exception, session: [%s]"), pContext->ident.name.c_str());
		return false;
	}

	return true;
}

SmartServerRequest::SmartServerRequest(IServerRequest*& pServerRequest) : _pServerRequest(pServerRequest)
{
}

SmartServerRequest::~SmartServerRequest()
{
	if (NULL != _pServerRequest)
		_pServerRequest->release();
	_pServerRequest = NULL;
}

