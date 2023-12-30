// CiscoVideoServerConsole.cpp : 定义控制台应用程序的入口点。
//

#include "IceLog.h"
#include "CVSSImpl.h"
#include "CVSSCfgLoader.h"
#include "CiscoAIMSoap11Impl.h"

char *DefaultConfigPath = "C:\\TianShan\\etc\\CVSS.xml";
::ZQ::common::Config::Loader< ::ZQTianShan::CVSS::CVSSCfg > pConfig(DefaultConfigPath);
int32 iDefaultBufferSize = 16*1024;
extern int32 iTimeOut;

Ice::CommunicatorPtr	_ic;

::TianShanIce::common::IceLogIPtr _iceLog;

::ZQTianShan::CVSS::CiscoVirtualStreamServiceImplPtr _cps;
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

	for (ZQTianShan::CVSS::IceProperties::props::iterator iceIter = pConfig._iceProperties._props.begin();
		iceIter != pConfig._iceProperties._props.end(); iceIter++)
	{
		proper->setProperty((*iceIter).name.c_str(), (*iceIter).value.c_str());
		_fileLog(::ZQ::common::Log::L_INFO, CLOGFMT(CVSSvc, "Set ICE Property %s=%s"), (*iceIter).name.c_str(), (*iceIter).value.c_str());			
	}

}

void ConfigTest()
{
	::ZQTianShan::CVSS::CVSSConfig  cvssConfig("C:\\TianShan\\etc\\CVSS.xml");
	cvssConfig.ConfigLoader();
	cout << "load config" << endl;
	cout << "<TianShan>" << endl;

	cout << "  <Definitions src=\"TianShanDef.xml\">" << endl;
	cout << "  </Definitions>" << endl;

	cout << "  <default>" << endl;

	cout << "    "
		 << "<CrashDump path=\"" << cvssConfig._crashDump.path 
		 << "\" enabled=\"" << cvssConfig._crashDump.enabled 
		 << "\" />" << endl;

	cout << "    "
		 << "<IceTrace enabled=\"" << cvssConfig._iceTrace.enabled
		 << "\" level=\"" << cvssConfig._iceTrace.level
		 << "\" size=\"" << cvssConfig._iceTrace.size 
		 << "\"/>" << endl;

	cout << "    "
		 << "<EventChannel endPoint=\"" << cvssConfig._iceStorm.endPoint <<"\" />" << endl;

	cout << "    " << "<IceProperties>" << endl;
	for (::ZQTianShan::CVSS::IceProperties::props::iterator iter = cvssConfig._iceProperties._props.begin(); iter != cvssConfig._iceProperties._props.end(); iter++)
		cout << "      " << "<prop name=\"" << iter->name << "\" value=\"" << iter->value << "\" />" << endl;
	cout << "    " << "</IceProperties>" << endl;

	cout << "    " << "<PublishedLogs>" << endl;
	for (std::vector<::ZQTianShan::CVSS::PublishedLogs::PublishLogHolder>::iterator iter = cvssConfig._publishedLogs._logDatas.begin(); iter != cvssConfig._publishedLogs._logDatas.end(); iter++)
		cout << "      " << "<Log  path=\"" << iter->_path <<"\" syntax=\"" << iter->_syntax << "\" />" << endl;
	cout << "    " << "</PublishedLogs>" << endl;

	cout << "    " << "<Database path=\"" << cvssConfig._dataBase.path <<"\" runtimePath=\"" << cvssConfig._dataBase.runtimePath << "\" />" << endl;

	cout << "  </default>" << endl;

	cout << "  <CVSS>" << endl;

	cout << "    " << "<LogFile path=\"" << cvssConfig._cLogFile.path
		 << "\" level=\"" << cvssConfig._cLogFile.level
		 << "\" maxCount=\"" << cvssConfig._cLogFile.maxCount
		 << "\" size=\"" << cvssConfig._cLogFile.size
		 << "\" buffer=\"" << cvssConfig._cLogFile.buffer
		 << "\" flushTimeout=\"" << cvssConfig._cLogFile.flushTimeout
		 << "\" />" << endl;

	cout << "    " << "<Bind endPoint=\"" << cvssConfig._bind.endPoint
		 << "\" dispatchSize=\"" << cvssConfig._bind.dispatchSize
		 << "\" dispatchMax=\"" << cvssConfig._bind.dispatchMax
		 << "\" evictorSize=\"" << cvssConfig._bind.evictorSize
		 << "\" threadPoolSize=\"" << cvssConfig._bind.threadPoolSize
		 << "\"/>" << endl;

	cout << "    " << "<IceLog path=\"" << cvssConfig._iceLog.path
		 << "\" level=\"" << cvssConfig._iceLog.level
		 << "\" maxCount=\"" << cvssConfig._iceLog.maxCount
		 << "\" size=\"" << cvssConfig._iceLog.size
		 << "\" buffer=\"" << cvssConfig._iceLog.buffer
		 << "\" flushTimeout=\"" << cvssConfig._iceLog.flushTimeout
		 << "\"/> " << endl;

	cout << "    " << "<RTSPProp threadPoolSize=\"" << cvssConfig._rtspProp.threadPoolSize
		 << "\" timeOut=\"" << cvssConfig._rtspProp.timeOut
		 << "\" bufferMaxSize=\"" << cvssConfig._rtspProp.bufferMaxSize
		 << "\"/>" << endl;

	cout << "    " << "<StreamingServer name=\"" << cvssConfig._streamingServer.name
		 << "\" ip=\"" << cvssConfig._streamingServer.ip
		 << "\" port=\"" << cvssConfig._streamingServer.port
		 << "\"/>" << endl;

	cout << "    " << "<SoapLog path=\"" << cvssConfig._soapLog.path
		 << "\" level=\"" << cvssConfig._soapLog.level
		 << "\" maxCount=\"" << cvssConfig._soapLog.maxCount
		 << "\" size=\"" << cvssConfig._soapLog.size
		 << "\" buffer=\"" << cvssConfig._soapLog.buffer
		 << "\" flushTimeout=\"" << cvssConfig._soapLog.flushTimeout
		 << "\"/> " << endl;

	//storeinfo layer
	cout << "    " << "<StoreInfo netId=\"" << cvssConfig._storeInfo.netId
		 << "\" type=\"" << cvssConfig._storeInfo.type
		 << "\" streamableLength=\"" << cvssConfig._storeInfo.streamableLength
		 << "\">" << endl;

	cout << "      " << "<StoreReplica groupId=\"" << cvssConfig._storeInfo._storeReplica.groupId
		 << "\" replicaId=\"" << cvssConfig._storeInfo._storeReplica.replicaId
		 << "\" replicaPriority=\"" << cvssConfig._storeInfo._storeReplica.replicaPriority
		 << "\" timeout=\"" << cvssConfig._storeInfo._storeReplica.timeout
		 << "\" contentSize=\"" << cvssConfig._storeInfo._storeReplica.contentSize
		 << "\" volumeSize=\"" << cvssConfig._storeInfo._storeReplica.volumeSize
		 << "\" />" << endl;

	cout << "      " << "<LocalSoapInfo ip=\"" << cvssConfig._storeInfo._localSoapInfo.ip
		 << "\" port=\"" << cvssConfig._storeInfo._localSoapInfo.port
		 << "\" />" << endl;

	cout << "      " << "<ServerInfo ip=\"" << cvssConfig._storeInfo._serverInfo.ip
		<< "\" port=\"" << cvssConfig._storeInfo._serverInfo.port
		<< "\" />" << endl;

	cout << "      " << "<VolumeInfo name=\"" << cvssConfig._storeInfo._volumeInfo.name
		<< "\" path=\"" << cvssConfig._storeInfo._volumeInfo.path
		<< "\" />" << endl;

	cout << "      " << "<HeartBeat interval=\"" << cvssConfig._storeInfo._heartBeat.interval
		<< "\" />" << endl;

	cout << "    " << "</StoreInfo>" << endl;
	cout << "  </CVSS>" << endl;
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
	::ZQ::common::FileLog _iceFileLog(pConfig._iceLog.path.c_str(), pConfig._iceLog.level, pConfig._iceLog.maxCount, pConfig._iceLog.size, pConfig._iceLog.buffer, pConfig._iceLog.flushTimeout);

	//initialize CVSS file log
	_fileLog.open(pConfig._logFile.path.c_str(), pConfig._logFile.level, pConfig._logFile.maxCount,pConfig._logFile.size, pConfig._logFile.buffer, pConfig._logFile.flushTimeout);

	//initialize content store file log
	::ZQ::common::FileLog _csFileLog(pConfig._soapLog.path.c_str(), pConfig._soapLog.level, pConfig._soapLog.maxCount, pConfig._soapLog.size, pConfig._soapLog.buffer, pConfig._soapLog.flushTimeout);

	iDefaultBufferSize = pConfig._rtspProp.bufferMaxSize * 1024;
	iTimeOut = pConfig._rtspProp.timeOut * 1000;

	//initialize thread pool
	::ZQ::common::NativeThreadPool _pCVSSThreadPool(10);
	_pCVSSThreadPool.resize( pConfig._rtspProp.threadPoolSize> 10 ? pConfig._rtspProp.threadPoolSize : 10 );
	
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
	string strServerPath;
	uint16 uServerPort;
	strServerPath = pConfig._streamingServer.ip;
	uServerPort =  pConfig._streamingServer.port;


	::ZQTianShan::CVSS::CVSSEnv _pCVSSEnv(_fileLog, _pCVSSThreadPool, _ic, pConfig._iceStorm.endPoint.c_str(), pConfig._bind.endPoint.c_str(), pConfig._dataBase.path.c_str(), pConfig._dataBase.runtimePath.c_str());

	_cps = new ::ZQTianShan::CVSS::CiscoVirtualStreamServiceImpl(_fileLog, _pCVSSThreadPool, _pCVSSThreadPool, _ic, strServerPath,uServerPort, pConfig._bind.evictorSize, _pCVSSEnv);

	::std::string csDataPath = pConfig._dataBase.path + FNSEPS + "CVSS";
	_contentStore = new ::ZQTianShan::ContentStore::ContentStoreImpl(_csFileLog,_csFileLog, _pCVSSThreadPool, _pCVSSEnv._adapter, csDataPath.c_str());

	try{
		::std::string CSPath = pConfig._storeInfo._volumeInfo.path + FNSEPS;
		_contentStore->mountStoreVolume(pConfig._storeInfo._volumeInfo.name.c_str(), CSPath.c_str(), true);
	}
	catch(...)
	{
		_csFileLog(ZQ::common::Log::L_ERROR, CLOGFMT(CVSSvc, "mount store volume name (%s) path (%s) catch an exception\n"),pConfig._storeInfo._volumeInfo.name.c_str(),pConfig._storeInfo._volumeInfo.path.c_str());
		return -1;
	}

	_fileLog(::ZQ::common::Log::L_INFO, CLOGFMT(CVSSvc, "Cisoco Stream Service and ContentStore Service created"));
	while (!bQuit)
	{
		static const char* chs="-\\|/";
		static int chi=0;
		chi = ++chi %4;
		printf("\rCVSS is now listening %c", chs[chi]);
		Sleep(200);
	}

	ZQ::common::setGlogger(NULL);
	printf("\rCVSS Console is quiting                   ");

	_contentStore = NULL;
	_cps = NULL;
	_pCVSSEnv._adapter->deactivate();
	_ic->shutdown();
	_ic->destroy();

	Sleep(1000);

	printf("\rCVSS stopped                    \n");
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

