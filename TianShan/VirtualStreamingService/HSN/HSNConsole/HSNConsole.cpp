// HSNConsole.cpp : 定义控制台应用程序的入口点。
//

#include "IceLog.h"
#include "TMVSStreamserviceImpl.h"
#include "TMVSSCfgLoader.h"
#include <iostream>
using namespace std;

#define HSNVSSService "TMVSS"
#define HSNVSSProduct "TianShan"

char *DefaultConfigPath = "C:\\TianShan\\etc\\TMVSS.xml";
ZQ::common::Config::Loader< ::ZQTianShan::VSS::TM::TMVSSCfg > pConfig(DefaultConfigPath);

Ice::CommunicatorPtr	_ic;

::TianShanIce::common::IceLogIPtr _iceLog;

::ZQTianShan::VSS::TM::TMVStreamServiceImplPtr			 _cps;
::ZQTianShan::ContentStore::ContentStoreImpl::Ptr _contentStore;
::ZQ::common::FileLog _fileLog;

BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

void initWithConfig(Ice::PropertiesPtr proper )
{
	proper->setProperty("Ice.ThreadPool.Client.Size","5");
	proper->setProperty("Ice.ThreadPool.Client.SizeMax","10");
	proper->setProperty("Ice.ThreadPool.Server.Size","5");
	proper->setProperty("Ice.ThreadPool.Server.SizeMax","10");

	for (ZQTianShan::VSS::TM::IceProperties::props::iterator iceIter = pConfig._iceProperties._props.begin();
		iceIter != pConfig._iceProperties._props.end(); iceIter++)
	{
		proper->setProperty((*iceIter).name.c_str(), (*iceIter).value.c_str());
		_fileLog(::ZQ::common::Log::L_INFO, CLOGFMT(TMVSSConsole, "Set ICE Property %s=%s"), (*iceIter).name.c_str(), (*iceIter).value.c_str());			
	}
}

void ConfigTest()
{
	::ZQTianShan::VSS::TM::TMVSSConfig  vssConfig("C:\\TianShan\\etc\\TMVSS.xml");
	vssConfig.ConfigLoader();
	cout << "load config" << endl;
	cout << "<TianShan>" << endl;

	cout << "  <Definitions src=\"TianShanDef.xml\">" << endl;
	cout << "  </Definitions>" << endl;

	cout << "  <default>" << endl;

	cout << "    "
		<< "<CrashDump path=\"" << vssConfig._crashDump.path 
		<< "\" enabled=\"" << vssConfig._crashDump.enabled 
		<< "\" />" << endl;

	cout << "    "
		<< "<IceTrace enabled=\"" << vssConfig._iceTrace.enabled
		<< "\" level=\"" << vssConfig._iceTrace.level
		<< "\" size=\"" << vssConfig._iceTrace.size 
		<< "\"/>" << endl;

	cout << "    "
		<< "<EventChannel endPoint=\"" << vssConfig._iceStorm.endPoint <<"\" />" << endl;

	cout << "    " << "<IceProperties>" << endl;
	for (::ZQTianShan::VSS::TM::IceProperties::props::iterator iter = vssConfig._iceProperties._props.begin(); iter != vssConfig._iceProperties._props.end(); iter++)
		cout << "      " << "<prop name=\"" << iter->name << "\" value=\"" << iter->value << "\" />" << endl;
	cout << "    " << "</IceProperties>" << endl;

	cout << "    " << "<PublishedLogs>" << endl;
	for (std::vector<::ZQTianShan::VSS::TM::PublishedLogs::PublishLogHolder>::iterator iter = vssConfig._publishedLogs._logDatas.begin(); iter != vssConfig._publishedLogs._logDatas.end(); iter++)
		cout << "      " << "<Log  path=\"" << iter->_path <<"\" syntax=\"" << iter->_syntax << "\" />" << endl;
	cout << "    " << "</PublishedLogs>" << endl;

	cout << "    " << "<Database path=\"" << vssConfig._dataBase.path <<"\" runtimePath=\"" << vssConfig._dataBase.runtimePath << "\" />" << endl;

	cout << "  </default>" << endl;

	cout << "  <TMVSS>" << endl;

	cout << "    " << "<LogFile path=\"" << vssConfig._cLogFile.path
		<< "\" level=\"" << vssConfig._cLogFile.level
		<< "\" maxCount=\"" << vssConfig._cLogFile.maxCount
		<< "\" size=\"" << vssConfig._cLogFile.size
		<< "\" buffer=\"" << vssConfig._cLogFile.bufferSize
		<< "\" flushTimeout=\"" << vssConfig._cLogFile.flushTimeout
		<< "\" />" << endl;

	cout << "    " << "<Bind endPoint=\"" << vssConfig._bind.endPoint
		<< "\" dispatchSize=\"" << vssConfig._bind.dispatchSize
		<< "\" dispatchMax=\"" << vssConfig._bind.dispatchMax
		<< "\" evictorSize=\"" << vssConfig._bind.evictorSize
		<< "\" threadPoolSize=\"" << vssConfig._bind.threadPoolSize
		<< "\"/>" << endl;

	cout << "    " << "<RTSPProp threadPoolSize=\"" << vssConfig._rtspProp.threadPoolSize
		<< "\" timeOut=\"" << vssConfig._rtspProp.timeOut
		<< "\" bufferMaxSize=\"" << vssConfig._rtspProp.bufferMaxSize
		<< "\"/>" << endl;

	cout << "    " << "<IceLog path=\"" << vssConfig._tmvssIceLog.path
		<< "\" level=\"" << vssConfig._tmvssIceLog.level
		<< "\" maxCount=\"" << vssConfig._tmvssIceLog.maxCount
		<< "\" size=\"" << vssConfig._tmvssIceLog.size
		<< "\" buffer=\"" << vssConfig._tmvssIceLog.bufferSize
		<< "\" flushTimeout=\"" << vssConfig._tmvssIceLog.flushTimeout
		<< "\"/> " << endl;


	cout << "    " << "<SoapLocalInfo"
		<< "\" ip=\"" << vssConfig._localInfo.ip
		<< "\" port=\"" << vssConfig._localInfo.port
		<< "\"/>" << endl;

	cout << "    " << "<SoapServerInfo"
		<< "\" ip=\"" << vssConfig._soapServerInfo.ip
		<< "\" port=\"" << vssConfig._soapServerInfo.port
		<< "\"/>" << endl;

	cout << "    " << "<Debug>" << endl;

	cout << "        " << "<PrivateData>" << endl;
	for (::ZQTianShan::VSS::TM::PrivateData::params::iterator iter = vssConfig._privateData._params.begin(); iter != vssConfig._privateData._params.end(); iter++)
		cout << "          " << "<param key=\"" << iter->key << "\" value=\"" << iter->value << "\" />" << endl;
	cout << "        " << "</PrivateData>" << endl;

	cout << "        " << "<ResourceMap>" << endl;
	for (::ZQTianShan::VSS::TM::ResourceMap::params::iterator iter = vssConfig._resourceMap._params.begin(); iter != vssConfig._resourceMap._params.end(); iter++)
		cout << "          " << "<param key=\"" << iter->key << "\" value=\"" << iter->value << "\" />" << endl;

	cout << "        " << "</ResourceMap>" << endl;

	cout << "    " << "</Debug>" << endl;

	cout << "  </TMVSS>" << endl;
	cout << "</TianShan>" << endl;
	system("PAUSE");
}

int main(int argc, char* argv[])
{
	//ConfigTest();
	//return 1;

	pConfig.load(DefaultConfigPath, true);
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, TRUE)==FALSE)
	{
		printf("Unable to install handler!                      \n");
		return -1;
	}

	//initialize ice log
	::ZQ::common::FileLog _iceFileLog(pConfig._tmvssIceLog.path.c_str(), pConfig._tmvssIceLog.level, pConfig._tmvssIceLog.maxCount, pConfig._tmvssIceLog.size, pConfig._tmvssIceLog.bufferSize, pConfig._tmvssIceLog.flushTimeout);

	//initialize CVSS file log
	_fileLog.open(pConfig._logFile.path.c_str(), pConfig._logFile.level, pConfig._logFile.maxCount,pConfig._logFile.size, pConfig._logFile.bufferSize, pConfig._logFile.flushTimeout);

	//initialize content store file log
	//::ZQ::common::FileLog _csFileLog(pConfig._logFile.path.c_str(), pConfig._logFile.level, pConfig._logFile.maxCount, pConfig._soapLog.size, pConfig._soapLog.buffer, pConfig._soapLog.flushTimeout);

	//initialize thread pool
	::ZQ::common::NativeThreadPool _pVSSThreadPool(10);
	_pVSSThreadPool.resize( pConfig._rtspProp.threadPoolSize> 10 ? pConfig._rtspProp.threadPoolSize : 10 );

	//initialize service
	_iceLog = new ::TianShanIce::common::IceLogI(&_iceFileLog);

	int i=0;

#if ICE_INT_VERSION / 100 >= 303

	::Ice::InitializationData initData;
	initData.properties = Ice::createProperties(i, NULL);
	initWithConfig(initData.properties);

	initData.logger = _iceLog;

	_ic = Ice::initialize(initData);

#else
	Ice::PropertiesPtr proper = Ice::createProperties(i,NULL);
	initWithConfig(proper);	

	_ic=Ice::initializeWithPropertiesAndLogger(i,NULL,proper, _iceLog);
#endif // ICE_INT_VERSION

	std::string	strDbPath ;
	strDbPath = pConfig._dataBase.path;


	//get media server config
	std::string strServerPath = pConfig._soapServerInfo.ip;
	uint16 uServerPort = pConfig._soapServerInfo.port;
	std::string strLocalPath = pConfig._localInfo.ip;
	uint16 uLocalPort = pConfig._localInfo.port;

	ZQTianShan::VSS::TM::TMVSSEnv *_pVSSEnv = new ZQTianShan::VSS::TM::TMVSSEnv(_fileLog, _pVSSThreadPool,
		strServerPath, uServerPort,
		strLocalPath, uLocalPort,
		_ic,
		pConfig._iceStorm.endPoint.c_str(),
		pConfig._bind.endPoint.c_str(),
		pConfig._dataBase.path.c_str(),
		pConfig._dataBase.runtimePath.c_str());

	_cps = new ::ZQTianShan::VSS::TM::TMVStreamServiceImpl(_fileLog, _ic, strServerPath,uServerPort, strLocalPath, uLocalPort, pConfig._bind.evictorSize, *_pVSSEnv);
	_cps->start();

	::std::string csDataPath = pConfig._dataBase.path + FNSEPS + HSNVSSService;
	_contentStore = new ZQTianShan::ContentStore::ContentStoreImpl(_fileLog,_fileLog, _pVSSThreadPool, _pVSSEnv->_adapter, csDataPath.c_str());

	try{
		::std::string CSPath =  std::string("HSNCS") + FNSEPS;
		_contentStore->initializeContentStore();
		_contentStore->mountStoreVolume("70001", CSPath.c_str(), true);
	}
	catch(...)
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(HSN, "mount store volume name (%s) path (%s) catch an exception\n"), "70001", "HSNCS");
		return -1;
	}

	_fileLog(::ZQ::common::Log::L_INFO, CLOGFMT(HSN, "HSN Virtual Stream Service and ContentStore Service created"));
	while (!bQuit)
	{
		static const char* chs="-\\|/";
		static int chi=0;
		chi = ++chi %4;
		//printf("\rTMVSS is now listening %c", chs[chi]);
		printf("\nTMVSS is now listening");
		Sleep(5000);
	}

	ZQ::common::setGlogger(NULL);
	_iceLog = NULL;
	printf("\rTMVSS Console is quiting                   \r");

	_cps->uninitializeService();
	_cps = NULL;
	_contentStore->unInitializeContentStore();
	_contentStore = NULL;

	_pVSSThreadPool.stop();
	delete _pVSSEnv;
	_pVSSEnv = NULL;
	//_pCVSSEnv->_adapter->deactivate();
	_ic->shutdown();
	_ic->destroy();
	_ic = NULL;

	Sleep(1000);

	printf("\rTMVSS stopped                    \n");
	return 0;
}

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	switch(CEvent)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		bQuit = true;
		break;
	}
	return TRUE;
}

