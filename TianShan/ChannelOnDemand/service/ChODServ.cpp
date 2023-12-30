// hODServ.cpp: implementation of the ChODServ class.
//
//////////////////////////////////////////////////////////////////////

#include "Log.h"
#include "ChODServ.h"
#include "MiniDump.h"
#include "ChODSvcEnv.h"
#include "IceLog.h"
#include "ChODSvcEnv.h"
#include "PlaylistEventSinkImpl.h"
#include "JmsMsgSender.h"
#include "PurchaseImpl.h"
#include "FileSystemOp.h"

#define LOG_MODULE_NAME			"Service"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

ChODServ g_chODService;
ZQ::common::BaseZQServiceApplication *Application = &g_chODService;
//ChodConfig _config;
ZQ::common::Config::Loader<ChodConfigII> _config("ChannelOnDemand.xml");
ZQ::common::Config::ILoader *configLoader = &_config;

ZQ::common::MiniDump _minidump;
static void WINAPI MiniDumpCallback(DWORD ExceptionCode, PVOID ExceptionAddress);

#ifdef ZQ_OS_MSWIN
	#define REL_JVM_JDK "jre\\bin\\client\\jvm.dll" // windows jvm rel_path for jdk installation
	#define REL_JVM_JRE "bin\\client\\jvm.dll" // windows jvm rel_path for jre installation
#endif

JmsMsgSender::InitInfo _iniInfo;
using namespace ZQ::JndiClient;

class CodJMSMsgSender
{
public:
	CodJMSMsgSender()
	{
		_hThread = NULL;
		_hExit = NULL;
	}
	~CodJMSMsgSender()
	{
		stop();
	}

	void start()
	{
		DWORD dwThreadId;
		_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, &dwThreadId);		
		_hExit = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	void stop()
	{
		if (_hExit)
		{
			SetEvent(_hExit);
		}

		if (_hThread)
		{
			WaitForSingleObject(_hThread, 3000);
			CloseHandle(_hThread);
			_hThread = NULL;
		}

		if (_hExit)
		{
			CloseHandle(_hExit);
			_hExit = NULL;
		}
	}

protected:
	static Log::loglevel_t toLogLevel(int lvl, Log::loglevel_t defVal)
	{
		switch(lvl) 
		{
			case 0: return Log::L_EMERG;
			case 1: return Log::L_ALERT;
			case 2: return Log::L_CRIT;
			case 3: return Log::L_ERROR;
			case 4: return Log::L_WARNING;
			case 5: return Log::L_NOTICE;
			case 6: return Log::L_INFO;
			case 7: return Log::L_DEBUG;
			default: return defVal;
		}
	}
	static std::string pathCat(const std::string &dir, const std::string& sub)
	{
		std::string path;
		path.reserve(dir.size() + 1 + sub.size());
		if(!dir.empty() && dir[dir.size() - 1] != FNSEPC)
		{
			path = dir + FNSEPS;
		}
		else
		{
			path = dir;
		}

		path += sub;

		return path;
	}
	static ULONG  __stdcall ThreadProc(void* pParam)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "JMS Sender thread entered"));

		HANDLE hExit = NULL;
		if (pParam)
		{
			hExit = ((CodJMSMsgSender*)pParam)->_hExit;
		}
		else
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Param is null, JMS Sender thread leaved"));
			return 0;
		}

		// get the jvm path
		std::string jvmPath;
		if(!_iniInfo.strJavaHome.empty())
		{
			jvmPath = pathCat(_iniInfo.strJavaHome, REL_JVM_JDK);
			if(!FS::FileAttributes(jvmPath).exists()) {
				jvmPath = pathCat(_iniInfo.strJavaHome, REL_JVM_JRE);
			}
		} 
		else
		{
			const char* envJavaHome = getenv("JAVA_HOME");
			if(envJavaHome)
			{
				jvmPath = pathCat(envJavaHome, REL_JVM_JDK);
				if(!FS::FileAttributes(jvmPath).exists()) 
				{
					jvmPath = pathCat(envJavaHome, REL_JVM_JRE);
				}
			} 
			else 
			{ // bad setting!
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Bad java environment settings: can't get the JVM path. need the xml conf or sys env 'JAVA_HOME'"));
				return 0;
			}
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Init JVM [%s] with classpath=%s"), jvmPath.c_str(), _iniInfo.strClassPath.c_str());

		if(!ZQ::JndiClient::ClientContext::initJVM(glog, _iniInfo.strClassPath.c_str(),jvmPath.c_str())) {
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Failed to init JVM [%s]"), jvmPath.c_str());
			return 0;
		}

		ClientContext::Properties ctxProps;
		ctxProps[JNDICTXPROP_INITIAL_CONTEXT_FACTORY] = _iniInfo.strNamingCtx;

		ZQ::JndiClient::ClientContext* context = new ClientContext(_iniInfo.strSrvIpPort, toLogLevel(_iniInfo.nLogLevel, Log::L_WARNING), ctxProps);

		if(NULL == context)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Failed to create jvm client contxt"));
			return 0;
		}

		JmsMsgSender msgSdr(*context, "", _iniInfo.strDestName);
		msgSdr.init(_iniInfo);

		char szMsg[128];
		SYSTEMTIME st;
		GetLocalTime(&st);
		sprintf(szMsg, "InService;%4d%02d%02dT%02d%02d%02d", st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
		std::string strMsg = szMsg;			
		
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Start to send a JMS message \"%s\" to TOD"), szMsg);
		
		msgSdr.SendMsg(strMsg);

		while(1)
		{
			if (msgSdr.SendAllMsg())
				break;
			
			DWORD dwRet = WaitForSingleObject(hExit, 15000);
			if (dwRet == WAIT_OBJECT_0)
				break;
		}
		
		if (WaitForSingleObject(hExit, 0) != WAIT_OBJECT_0)
		{
			//afraid of the JMS have not send out the message
			WaitForSingleObject(hExit, 2000);
		}

		msgSdr.close();
        
		// release the context
		if(context) {
			delete context;
			context = NULL;
		}

		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Uninit the JVM..."));
		ZQ::JndiClient::ClientContext::uninitJVM();
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "JVM uninited."));

		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "JMS Sender thread leaved"));

		return 0;
	}

private:
	HANDLE _hThread;
	HANDLE _hExit;
};

CodJMSMsgSender		_jmsSender;


ChODServ::ChODServ()
{
	strcpy(servname, "ChODSvc");
	strcpy(prodname, "TianShan");
}

ChODServ::~ChODServ()
{

}

HRESULT ChODServ::OnInit(void)
{
	BaseZQServiceApplication::OnInit();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "OnInit() enter"));


	showAppDataPattern(_config.authAppDataMap);

//    SNMPOpenSession("ChannelOnDemand", "TianShan", 2);
//    _config.setLogger(m_pReporter);
//	char ConfigPath[MaxPath];
//	strcpy(ConfigPath, m_wsConfigFolder);
//	strcat(ConfigPath, "ChannelOnDemand.xml");
//    if(!_config.load(ConfigPath))
//    {
//        glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "load config failed"));
//        return S_FALSE;
//    }

/*	
	{
		printf("%d\n", _config.dumpFullMemory);
		printf("%s\n", _config.crushDumpPath.c_str());

	// the directory of database
		printf("%s\n", _config.safeStorePath.c_str());
		printf("%s\n", _config.dbRuntimeDataPath.c_str());

	// defines the ChannelPublisher properties 
		printf("%s\n", _config.ChannelPubEndPoint.c_str());
		printf("%d\n", _config.ChannelPublishPointEvitSize);
		printf("%d\n", _config.InputSystemTime);
		printf("%d\n", _config.DefaultChannelMaxBitrate);
		printf("%d\n", _config.ProtectTimeInMs);

	// defines the PurchaseManagement properties 
		printf("%d\n", _config.PurchaseEvitSize);
		printf("%d\n", _config.PurchaseItemEvitSize);
		printf("%d\n", _config.purchaseTimeout);
	
	// size of ThreadPool
		printf("%d\n", _config.ThreadPoolSize);

	// defines the TianshanEvent properties
		printf("%s\n", _config.TopicMgrEndPoint.c_str());
		printf("%s\n", _config.ListenEventEndPoint.c_str());
		printf("%s\n", _config.TSEventRuntimeDataPath.c_str());

		printf("%s\n", _config._szEventPubType.c_str());

		printf("%s\n", _config.authInfo.module.c_str());
		printf("%s\n", _config.authInfo.entry.c_str());
		printf("%d\n", _config.authInfo.enable);

		for(ChodConfigII::IceProps::iterator it = _config.iceProps.begin(); it != _config.iceProps.end(); ++it)
		{
			printf("%s--%s\n", it->name.c_str(), it->value.c_str());
		}
		for(ChodConfigII::JMSParams::iterator jms_it = _config.jmsParams.begin(); jms_it != _config.jmsParams.end(); ++jms_it)
		{
			printf("%s--%s\n", jms_it->name.c_str(), jms_it->value.c_str());
		}
		for(std::map<std::string, ZQ::common::Config::Holder< AuthorizationParam > >::iterator it3 = _config.authInfo.authorizationParams.begin(); it3 != _config.authInfo.authorizationParams.end(); ++it3)
		{
			printf("%s--%s\n", it3->first.c_str(), it3->second.value.c_str());
		}

	}

	if(!loadConfig())
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "load config failed"));
		return S_FALSE;
	}
*/	
	if(!_config.ChannelPubEndPoint.size())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "key [ChannelOnDemandEndPoint] must be configured"));
		return E_HANDLE;
	}
	
	// get the endpoint of topic manager
	if(!_config.TopicMgrEndPoint.size())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "key [TopicManagerEndPoint] must be configured"));
		return E_HANDLE;
	}	

	//
	// Get Configuration from Application's sub Registry Level - ICE
	//	
	_properties = Ice::createProperties();
		
	ChodConfigII::IceProps::iterator iter = _config.iceProps.begin();
	for (; iter != _config.iceProps.end(); ++iter) 
	{
		_properties->setProperty(iter->name, iter->value);
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "ChODServ::OnInit() set ICE properties <%s>--<%s>."), (iter->name).c_str(), (iter->value).c_str());
	}

	{
		// get the crash dump path		
		if (!_minidump.setDumpPath((char *)(_config.crushDumpPath.c_str())))
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "key [CrashDumpPath] is not correct directory"));
			return E_HANDLE;
		}

		_minidump.enableFullMemoryDump(_config.dumpFullMemory);
		_minidump.setExceptionCB(MiniDumpCallback);		
	}

	{
		// parameters for JMS message

		_iniInfo.strNamingCtx.assign("org.jnp.interfaces.NamingContextFactory");
		_iniInfo.strSrvIpPort.assign("192.168.80.70:1099");
		_iniInfo.strDestName.assign("queue/TodQueue");
		_iniInfo.strConnFactory.assign("ConnectionFactory");
		_iniInfo.strSafestoreFile.assign("c:\\storageFile.txt");
		_iniInfo.strMsgProperty.assign("string,MESSAGECLASS,NOTIFICATION;int,MESSAGECODE,1101");
		_iniInfo.strClassPath="c:\\TianShan\\bin\\java\\JndiClient.jar;c:\\TianShan\\bin\\java\\jbossall-client.jar";
		_iniInfo.strJavaHome ="";
		_iniInfo.nReConnectCount = 5;
		_iniInfo.nReConnectInterval = 60000;
		_iniInfo.nFlushToFileCount = 100;
		_iniInfo.nKeepAliveTime = 0;

		for(ChodConfigII::JMSParams::iterator jms_it = _config.jmsParams.begin(); jms_it != _config.jmsParams.end(); ++jms_it)
		{
			if(jms_it->name.compare("NamingContext") == 0)
			{
				if (jms_it->value.size())
					_iniInfo.strNamingCtx = jms_it->value;
			}
			else if(jms_it->name.compare("ServerIPPort") == 0)
			{
				if (jms_it->value.size())
					_iniInfo.strSrvIpPort = jms_it->value;
			}
			else if(jms_it->name.compare("DestinationName") == 0)
			{
				if (jms_it->value.size())
					_iniInfo.strDestName = jms_it->value;
			}
			else if(jms_it->name.compare("ConnectionFactory") == 0)
			{
				if (jms_it->value.size())
					_iniInfo.strConnFactory = jms_it->value;
			}
			else if(jms_it->name.compare("MessageStoreFile") == 0)
			{
				if (jms_it->value.size())
					_iniInfo.strSafestoreFile = jms_it->value;
			}
			else if(jms_it->name.compare("MessagePropertys") == 0)
			{
				if (jms_it->value.size())
					_iniInfo.strMsgProperty = jms_it->value;
			}
			else if(jms_it->name.compare("ReconnectCount") == 0)
			{
				if (jms_it->value.size())
					_iniInfo.nReConnectCount = atoi(jms_it->value.c_str());
			}
			else if(jms_it->name.compare("ReconnectInterval") == 0)
			{
				if (jms_it->value.size())
					_iniInfo.nReConnectInterval = atoi(jms_it->value.c_str());
			}
			else if(jms_it->name.compare("NoneSendMsgFlushCount") == 0)
			{
				if (jms_it->value.size())
					_iniInfo.nFlushToFileCount = atoi(jms_it->value.c_str());
			}
			else if(jms_it->name.compare("KeepAliveTime") == 0)
			{
				if (jms_it->value.size())
					_iniInfo.nKeepAliveTime = atoi(jms_it->value.c_str());
			}
			else if(jms_it->name.compare("ClassPath") == 0)
			{
				if (jms_it->value.size())
					_iniInfo.strClassPath = jms_it->value;
			}
			else if(jms_it->name.compare("JavaHome") == 0)
			{
				if (jms_it->value.size())
					_iniInfo.strJavaHome =  jms_it->value;
			}
			else if(jms_it->name.compare("JmsLogLevel") == 0)
			{
				if (jms_it->value.size())
					_iniInfo.nLogLevel = atoi(jms_it->value.c_str());
			}
			else{
				continue;
			}
		}	
	}
	

	{
		// get the endpoint of topic manager
		if (_config.authInfo.enable)
		{
			map<std::string, ZQ::common::Config::Holder< AuthorizationParam > >::const_iterator iter;
			iter = _config.authInfo.authorizationParams.find("endpoint");
			if(iter != _config.authInfo.authorizationParams.end())
			{
				if(!iter->second.value.size())
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "key [TodasEndPoint] must be configured"));
					return E_HANDLE;
				}
			}
		}
	}

	//init ice trace
	std::string strLogFolder = m_wsLogFolder;

	int size = strLogFolder.size();
	if(size > 0 && strLogFolder[size -1] != '\\' && strLogFolder[size -1] != '/')
		strLogFolder += "\\";
	_iceFileLog = new ZQ::common::FileLog( (strLogFolder+"ChOdSvc_IceTrace.log").c_str(),
		_config.iceLogLevel,
		_config.iceLogCount,
		_config.iceLogSize);

    _config.snmpRegister("");
	glog(ZQ::common::Log::L_INFO, "ChODServ::OnInit() leave");

	return S_OK;
}

HRESULT ChODServ::OnUnInit(void)
{
	if (_pChOdSvcEnv)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Start to delete ChODSvcEnv object"));
		delete _pChOdSvcEnv;
		_pChOdSvcEnv = NULL;
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "ChODSvcEnv object deleted"));
	}

	if	(_iceFileLog)
	{
		delete _iceFileLog;
		_iceFileLog = NULL;
	}

	return S_OK;	
}


HRESULT ChODServ::OnStart(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "OnStart() enter"));

	BaseZQServiceApplication::OnStart();
	
	//
	// Initialize Ice communicator properties
	//
	int argc = 0;
	try
	{
		_icelog = new TianShanIce::common::IceLogI(_iceFileLog);
		Ice::InitializationData initData;
		if (_config.iceTraceEnable)
		{
			initData.logger = _icelog;
		}
		initData.properties = _properties;
		_communicator = Ice::initialize(argc, 0, initData);
		glog(ZQ::common::Log::L_INFO, "Ice communicator created");

		_pChOdSvcEnv = new ZQChannelOnDemand::ChODSvcEnv(_communicator);	
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "ChODSvcEnv object created"));		
		if (!_pChOdSvcEnv->init())
		{
			return E_HANDLE;
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "ChODSvcEnv object initialized"));

		// send a messag to TOD
		if (_config.authInfo.enable)
		{
			_jmsSender.start();
		}
	}
	catch(Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Ice initialize properties met exception with error: %s, %s @ line %d"), 
			ex.ice_name().c_str(), ex.ice_file(), ex.ice_line());
		return E_HANDLE;					
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "OnStart() leaved"));
	
	return S_OK;
}

HRESULT ChODServ::OnStop(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "OnStop() enter"));

	try
	{
		if (_pChOdSvcEnv)
		{
			_pChOdSvcEnv->unInit();
			delete _pChOdSvcEnv;
			_pChOdSvcEnv = NULL;
		}

		if(_communicator != NULL)
		{
			_communicator->destroy();
		}		
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Ice destroy met exception with error: %s @ line %d"), 
						ex.ice_file(), ex.ice_line());
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Ice destroy met unknown exception"));
	}
	
	// Ice object are not required to release, coz they are smart pointer object

	try
	{
		_jmsSender.stop();
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "JMS sender stop caught unknown exception"));
	}
	

	BaseZQServiceApplication::OnStop();
	
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "OnStop() leave"));

	return S_OK;
}

bool ChODServ::isHealth(void)
{
	return true;
}

bool ChODServ::loadConfig()
{
//////////////////////////////////////////////////////////////////////////
// to gain the configuration file path
//////////////////////////////////////////////////////////////////////////
/*
	char configPath[MAX_PATH];
	memset(configPath, 0, sizeof(configPath));
	TCHAR subStr[MAX_PATH];
	memset(subStr, 0, sizeof(subStr));
	snprintf(subStr, sizeof(subStr) - 1, "SOFTWARE\\ZQ Interactive\\%s\\CurrentVersion\\Services\\%s", prodname, servname);
	HKEY hKey;
	if (ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, subStr, 0, KEY_ALL_ACCESS, &hKey))
	{
		glog(ZQ::common::Log::L_EMERG, CLOGFMT("CodService", "RegOpenKeyEx(%s) failed"), subStr);
		return S_FALSE;
	}
	DWORD dwType = REG_SZ;
	DWORD szConfigPath = sizeof(configPath);
	if (ERROR_SUCCESS != ::RegQueryValueEx(hKey, _T("configDir"), NULL, &dwType, (unsigned char*)configPath, &szConfigPath))
	{
		glog(ZQ::common::Log::L_EMERG, CLOGFMT("CodService", "RegQueryValueEx(configDir) failed"));
		return S_FALSE;
	}
	::RegCloseKey(hKey);
	
	std::string cfgPath = configPath;
	if (cfgPath.size() > 0 && cfgPath[cfgPath.size() - 1] == '\\' || cfgPath[cfgPath.size() - 1] == '/')
		cfgPath.resize(cfgPath.size() - 1);
	cfgPath += "\\ChannelOnDemand.xml";
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT("CodService", "Cod application config [%s]"), cfgPath.c_str());

//////////////////////////////////////////////////////////////////////////
//start to parse .xml 
//////////////////////////////////////////////////////////////////////////

	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	try
	{
		bool bRet = xmlDoc.open(cfgPath.c_str());
		if (!bRet)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT("CodService", "open xml document [%s] failed"), cfgPath.c_str());
			return false;
		}
	}
	catch (const ZQ::common::XMLException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT("CodService", "open xml document [%s] caught %s"), cfgPath.c_str(), ex.getString());
		return false;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT("CodService", "open xml document [%s] caught unexpect exception"), cfgPath.c_str());
		return false;
	}

	ZQ::common::XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	SmartPreference sRoot(pRoot);
	ZQ::common::XMLPreferenceEx* pChodSvc = NULL;
	SmartPreference sChodSvc(pChodSvc);
	if (pRoot != NULL)
		pChodSvc = pRoot->findChild("ChannelOnDemand");
	if (!pChodSvc)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT("CodService", "no \\TianShan\\ChannelOnDemand node found"));
		return false;
	}

	int intValue;
	char strBuff[MAX_PATH];
	memset(strBuff, 0, sizeof(strBuff));

	// <CrashDump path="C:\TianShan\CrashDump" fullDump="1" />
	_config.dumpFullMemory = false;
	ZQ::common::XMLPreferenceEx* pCrashDump = pChodSvc->findChild("CrashDump");
	SmartPreference sCrashDump(pCrashDump);
	if (pCrashDump)
	{
		if (sCrashDump.getIntProp("fullDump", intValue) && intValue == 0)
			_config.dumpFullMemory = false;
		else 
			_config.dumpFullMemory = true;
		if (sCrashDump.getStrProp("path", strBuff, sizeof(strBuff)))
			_config.crushDumpPath = strBuff;
		if (_config.dumpFullMemory && _config.crushDumpPath.size() == 0)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT("CodService", "config error, \\TianShan\\ChannelOnDemand\\CrashDump.path is empty, but \\TianShan\\ChannelOnDemand\\CrashDump.enable is 1"));
			return false;
		}
	}

	// <Database path="C:\TianShan\data" runtimeData="C:\TianShan\data\runtime"/>
	ZQ::common::XMLPreferenceEx* pDatabase = pChodSvc->findChild("Database");
	SmartPreference sDatabase(pDatabase);
	if (pDatabase)
	{
		if (sDatabase.getStrProp("path", strBuff, sizeof(strBuff)))
			_config.safeStorePath = strBuff;
		if (sDatabase.getStrProp("runtimeData", strBuff, sizeof(strBuff)))
			_config.dbRuntimeDataPath = strBuff;
		if (_config.safeStorePath.size() == 0)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT("CodService", "config error, \\TianShan\\ChannelOnDemand\\Database.path is empty"));
			return false;
		}
	}
	else 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT("CodService", "config error, \\TianShan\\ChannelOnDemand\\Database not found"));
		return false;
	}

	//	<IceProperties>
	//		<prop name="Ice.Override.Timeout" value="-1"/>
	//		<prop name="Ice.ThreadPool.Server.Size" value="5"/>
	//	</IceProperties>
	_config.iceProps.clear();
	ZQ::common::XMLPreferenceEx* pIceProp = pChodSvc->findChild("IceProperties");
	SmartPreference sIceProp(pIceProp);
	if (pIceProp)
	{
		int subIce = 1;
		ZQ::common::XMLPreferenceEx* pSubIce = pIceProp->findChild("prop", subIce);
		while (pSubIce)
		{
			{// here bracket can't be omitted, otherwise it can cause memory leak
			SmartPreference sSubIce(pSubIce);
			std::string iceKey, iceVal;
			if (sSubIce.getStrProp("name", strBuff, sizeof(strBuff)))
				iceKey = strBuff;
			if (sSubIce.getStrProp("value", strBuff, sizeof(strBuff)))
				iceVal = strBuff;
			if (iceKey.size() && iceVal.size())
				_config.icePropMap[iceKey] = iceVal;
			}
			pSubIce = pIceProp->findChild("prop", ++subIce);
		}
	}

	// <ChannelPublisher endpoint="tcp -h 0.0.0.0 -p 33388"
	//					 cacheSize="60"
	//					 inputAsLocalTime="1"
	//					 defaultChannelBitrate="4000000"
	//					 protectionWindow="20000"/>
	ZQ::common::XMLPreferenceEx* pChannelPublisher = pChodSvc->findChild("ChannelPublisher");
	SmartPreference sChannelPublisher(pChannelPublisher);
	if (pChannelPublisher)
	{
		if (sChannelPublisher.getStrProp("endpoint", strBuff, sizeof(strBuff)))
			_config.ChannelPubEndPoint = strBuff;
		if (_config.ChannelPubEndPoint.size() == 0)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT("CodService", "config error, \\TianShan\\ChannelOnDemand\\ChannelPublisher.endpoint is empty"));
			return false;
		}
		if (sChannelPublisher.getIntProp("cacheSize", intValue))
			_config.ChannelPublishPointEvitSize = intValue;
		if (sChannelPublisher.getIntProp("inputAsLocalTime", intValue) && intValue==1)
			_config.InputSystemTime = true;
		else 
			_config.InputSystemTime = false;
		if (sChannelPublisher.getIntProp("defaultChannelBitrate", intValue))
			_config.DefaultChannelMaxBitrate = intValue;
		if (sChannelPublisher.getIntProp("protectionWindow", intValue))
			_config.ProtectTimeInMs = intValue;		
	}

	// <PurchaseManagement cacheSize="200" itemCacheSize="1000" timeout="3600"/>
	_config.purchaseTimeout = 3600;
	ZQ::common::XMLPreferenceEx* pPurchaseManagement = pChodSvc->findChild("PurchaseManagement");
	SmartPreference sPurchaseManagement(pPurchaseManagement);
	if (pPurchaseManagement)
	{
		if (sPurchaseManagement.getIntProp("cacheSize", intValue))
			_config.PurchaseEvitSize = intValue;
		if (sPurchaseManagement.getIntProp("timeout", intValue))
		{
			if(intValue >= 1800)
				_config.purchaseTimeout = intValue;
			else
				_config.purchaseTimeout = 1800;
		}
		if (sPurchaseManagement.getIntProp("itemCacheSize", intValue))
		{
			if(intValue >= _config.PurchaseEvitSize)
				_config.PurchaseItemEvitSize = intValue;
			else
				_config.PurchaseItemEvitSize = _config.PurchaseEvitSize;
		}
	}

	// <ThreadPool size="20"/>
	ZQ::common::XMLPreferenceEx* pThreadPool = pChodSvc->findChild("ThreadPool");
	SmartPreference sThreadPool(pThreadPool);
	if (pThreadPool)
	{
		if (sThreadPool.getIntProp("size", intValue))
		{
			if(intValue >= 10)
				_config.ThreadPoolSize = intValue;
			else
				_config.ThreadPoolSize = 10;
		}
	}

//		<Authorization module="Internal" entry="TODAS"		<!-- reserved, fixed value for now-->
//			enable="1"					<!-- former-configuration: /TianShan/ChannelOnDemand/TodasEnable -->
//			>
//			<param name="endpoint" value="TodasForCod:tcp -h 192.168.80.70 -p 6789" /> <!-- former-configuration: /TianShan/ChannelOnDemand/TodasEndPoint -->	
//			<param name="bookmarkOnLeave" value="1" /> 	<!-- former-configuration: /TianShan/ChannelOnDemand/BookmarkLastView -->	
//		</Authorization>

	ZQ::common::XMLPreferenceEx* pAuthorization = pChodSvc->findChild("Authorization");
	SmartPreference sAuthorization(pAuthorization);
	if (pAuthorization)
	{		
		if (sAuthorization.getIntProp("enable", intValue) && intValue == 1)
			_config.authInfo.enableAuthorize = true;
		else 
			_config.authInfo.enableAuthorize = false;
		if (sAuthorization.getStrProp("module", strBuff, sizeof(strBuff)))
			_config.authInfo.moduleName = strBuff;
		if (sAuthorization.getStrProp("entry", strBuff, sizeof(strBuff)))
			_config.authInfo.funcEntry = strBuff;
		
		_config.authInfo.paramMap.clear();
		int subParam = 1;
		ZQ::common::XMLPreferenceEx* pParam = pAuthorization->findChild("param", subParam);
		while (pParam)
		{
			{
				SmartPreference sParam(pParam);
				std::string paramKey, paramVal;
				if (sParam.getStrProp("name", strBuff, sizeof(strBuff)))
					paramKey = strBuff;
				if (sParam.getStrProp("value", strBuff, sizeof(strBuff)))
					paramVal = strBuff;
				if (paramKey.size() && paramVal.size())
					_config.authInfo.paramMap[paramKey] = paramVal;
			}
			pParam = pAuthorization->findChild("param", ++subParam);
		}
	}
	else 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT("CodService", "config error, \\TianShan\\ChannelOnDemand\\Authorization not found"));
		return false;
	}

//	<TianShanEvents  iceStormEndPoint="tcp -h x.x.x.x -p 10000" 
//			listenEndpoint="tcp -h x.x.x.x -p 11000" 
//			runtimeData="C:\TianShan\data\runtime"/>
	ZQ::common::XMLPreferenceEx* pTianShanEvents = pChodSvc->findChild("TianShanEvents");
	SmartPreference sTianShanEvents(pTianShanEvents);
	if (pTianShanEvents)
	{
		if (sTianShanEvents.getStrProp("iceStormEndPoint", strBuff, sizeof(strBuff)))
			_config.TopicMgrEndPoint = strBuff;
		if (sTianShanEvents.getStrProp("listenEndpoint", strBuff, sizeof(strBuff)))
			_config.ListenEventEndPoint = strBuff;
		if (sTianShanEvents.getStrProp("runtimeData", strBuff, sizeof(strBuff)))
			_config.TSEventRuntimeDataPath = strBuff;
	}
	else 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT("CodService", "config error, \\TianShan\\ChannelOnDemand\\TianShanEvents not found"));
		return false;
	}

//		<EventPublisher type="JMS">	
//			<param name="NamingContext" value="org.jnp.interfaces.NamingContextFactory" />
//			<param name="ServerIPPort" value="192.168.80.70:1099" /> 
//			<param name="DestinationName" value="queue/TodQueue" /> 
//			<param name="ConnectionFactory" value="ConnectionFactory"/> 
//			<param name="MessageStoreFile" value="c:\storageFile.txt"/> 
//			<param name="MessagePropertys" value="string,MESSAGECLASS,NOTIFICATION;int,MESSAGECODE,1101" /> 
//			<param name="ReconnectCount" value="5" /> 
//			<param name="ReconnectInterval" value="60000" /> 
//		</EventPublisher>		
	ZQ::common::XMLPreferenceEx* pJmsParam = pChodSvc->findChild("EventPublisher");
	SmartPreference sJmsParam(pJmsParam);
	if (pJmsParam)
	{
		if (sJmsParam.getStrProp("type", strBuff, sizeof(strBuff)))
			_config._szEventPubType = strBuff;
		if(_config._szEventPubType.compare("JMS") == 0)
		{
			int subJms = 1;
			ZQ::common::XMLPreferenceEx* pSubJms = pJmsParam->findChild("param", subJms);
			while (pSubJms)
			{
				{// here bracket can't be omitted, otherwise it can cause memory leak
					SmartPreference sSubJms(pSubJms);
					std::string jmsKey, jmsVal;
					if (sSubJms.getStrProp("name", strBuff, sizeof(strBuff)))
						jmsKey = strBuff;
					
					if(jmsKey.compare("NamingContext") == 0)
					{
						if (sSubJms.getStrProp("value", strBuff, sizeof(strBuff)))
						_config._szJMSNamingContext = strBuff;
					}
					else if(jmsKey.compare("ServerIPPort") == 0)
					{
						if (sSubJms.getStrProp("value", strBuff, sizeof(strBuff)))
						_config._szJMSServerIPPort = strBuff;
					}
					else if(jmsKey.compare("DestinationName") == 0)
					{
						if (sSubJms.getStrProp("value", strBuff, sizeof(strBuff)))
						_config._szJMSDestinationName = strBuff;
					}
					else if(jmsKey.compare("ConnectionFactory") == 0)
					{
						if (sSubJms.getStrProp("value", strBuff, sizeof(strBuff)))
						_config._szJMSConnectionFactory = strBuff;
					}
					else if(jmsKey.compare("MessageStoreFile") == 0)
					{
						if (sSubJms.getStrProp("value", strBuff, sizeof(strBuff)))
						_config._szJMSMessageStoreFile = strBuff;
					}
					else if(jmsKey.compare("MessagePropertys") == 0)
					{
						if (sSubJms.getStrProp("value", strBuff, sizeof(strBuff)))
						_config._szJMSMessagePropertys = strBuff;
					}
					else if(jmsKey.compare("ReconnectCount") == 0)
					{
						if (sSubJms.getIntProp("value", intValue))
							_config._dwJMSReconnectCount = intValue;
					}
					else if(jmsKey.compare("ReconnectInterval") == 0)
					{
						if (sSubJms.getIntProp("value", intValue))
							_config._dwJMSReconnectInterval = intValue;
					}
					else if(jmsKey.compare("NoneSendMsgFlushCount") == 0)
					{
						if (sSubJms.getIntProp("value", intValue))
							_config._dwJMSNoneSendMsgFlushCount = intValue;
					}
					else if(jmsKey.compare("KeepAliveTime") == 0)
					{
						if (sSubJms.getIntProp("value", intValue))
							_config._dwJMSKeepAliveTime = intValue;
					}
					else{
						continue;
					}
		//			if (jmsKey.size() && jmsVal.size())
		//				_config.jmsParamMap[jmsKey] = jmsVal;
				}
				pSubJms = pJmsParam->findChild("param", ++subJms);
			}
		}
	}
*/
	return true;

}

void WINAPI MiniDumpCallback(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();
	
	glog(ZQ::common::Log::L_ERROR,  "Application crashed, ExceptionCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
		ExceptionCode, ExceptionAddress, dwThreadID);

	glog.flush();
}
