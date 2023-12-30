// DataStream.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "datastreammain.h"
#include "DataDef.h"
#include "ZQThreadPool.h"
#include "BufferManager.h"
#include "DataDistributer.h"
#include "TsEncoder.h"
#include "DataSource.h"
#include "PsiPusher.h"
#include "DataSender.h"
#include "DataPusher.h"
#include "IceService.h"
#include "DataDebug.h"
#include "datastreamcfg.h"
#include "MiniDump.h"
ZQ::common::MiniDump			crash_dumper;

//////////////////////////////////////////////////////////////////////////

ZQ::common::Config::Loader<DataStreamCfg> gDataStreamConfig("DataStream.xml");
ZQ::common::Config::ILoader *configLoader = &gDataStreamConfig;
ZQ::common::NativeThreadPool* pThreadPool = NULL;
namespace DataStream {

ZQLIB::ZQThreadPool* readerThreadPool = NULL;
ZQLIB::ZQThreadPool* senderThreadPool = NULL;
BufferManager* bufferManager = NULL;
DataSender* dataSender = NULL;
PsiPusher* psiPusher = NULL;

#define DS_WORKINGSET_MIN			(8 * 1024 * 1024)
#define DS_WORKINGSET_MAX			(512 * 1024 * 1024)

static bool initializeDataStream()
{

	SetProcessWorkingSetSize(GetCurrentProcess(), 
		DS_WORKINGSET_MIN, DS_WORKINGSET_MAX);

	if (gDataStreamConfig.higherPriority) {
		SetPriorityClass(GetCurrentProcess(), 
			ABOVE_NORMAL_PRIORITY_CLASS);
	}

	readerThreadPool = new ZQLIB::ZQThreadPool(
		gDataStreamConfig.readerThreadPoolMinSize, 
		gDataStreamConfig.readerThreadPoolMaxSize, 
		3);
	
	if (!readerThreadPool->start()) {

		glog( ZQLIB::Log::L_ERROR,  "initializeDataStream()\t"
			"readerThreadPool->start() failed");

		return false;
	}

	senderThreadPool = new ZQLIB::ZQThreadPool(
		gDataStreamConfig.senderThreadPoolMinSize, 
		gDataStreamConfig.senderThreadPoolMaxSize, 
		3);
	if (!senderThreadPool->start()) {

		glog( ZQLIB::Log::L_ERROR,  "initializeDataStream()\t"
			"senderThreadPool->start() failed");
		return false;
	}

	bufferManager = new BufferManager();
	if (!bufferManager->start()) {

		glog( ZQLIB::Log::L_ERROR,  "initializeDataStream()\t"
			"bufferManager->start() failed");

		return false;
	}
	
	dataSender = new DataSender(*senderThreadPool);
	psiPusher = new PsiPusher(*senderThreadPool, *dataSender);

	// create native thread pool
	pThreadPool = new ZQ::common::NativeThreadPool(gDataStreamConfig.playThreadPoolSize);
	if(!pThreadPool)
		return false;
	return true;
}

static void uninitializeDataStream()
{
	if(pThreadPool)
	{
		delete pThreadPool;
		pThreadPool = NULL;
	}
	if (readerThreadPool) {
		readerThreadPool->stop();
		delete readerThreadPool;
	}

	if (senderThreadPool) {
		senderThreadPool->stop();
		delete senderThreadPool;
	}

	if (bufferManager) {
		bufferManager->stop();
		delete bufferManager;
	}

	if (dataSender)
		delete dataSender;

	if (psiPusher)
		delete psiPusher;

#ifdef _DEBUG
	dataDebug.finalDetect();
#endif

}

} // namespace DataStream {

void WINAPI crashHandler(DWORD excepCode, PVOID excepAddr)
{
	DWORD threadId = GetCurrentThreadId();
	char symName[MINIDUMP_SYMBOL_SIZE];
	char fileName[MAX_PATH];
	int line;
	OutputDebugStringA("crashHandler...\n");

	if (crash_dumper.resolveSym(excepAddr, symName, sizeof(symName), 
		fileName, sizeof(fileName), &line)) {

		OutputDebugStringA("crashHandler 1...\n");
		glog( ZQLIB::Log::L_ERROR,  "crashHandler()\t"
			"excepCode 0x%08x, excepAddr 0x%08x, threadId: 0x%04x"
			"sym: %s, fileName: %s, line: %d", excepCode, excepAddr, 
			threadId, symName, fileName, line);

	} else {

		OutputDebugStringA("crashHandler 2...\n");
		glog( ZQLIB::Log::L_ERROR,  "crashHandler()\t"
			"excepCode 0x%08x, excepAddr 0x%08x, threadId: 0x%04x",
			excepCode, excepAddr, threadId);
	}

	OutputDebugStringA("crashHandler 3...\n");

	glog.flush();
}

void initMiniDump()
{
	TCHAR dumpPath[MAX_PATH];
	GetModuleFileName(NULL, dumpPath, sizeof(dumpPath));

	TCHAR* c = dumpPath + strlen(dumpPath) - 1;
	while (*c) {
		if (*c == _T('\\')) {
			*c = 0;
			break;
		}

		c --;
	}
    
	crash_dumper.setDumpPath(dumpPath);
	crash_dumper.initSym(dumpPath, true);
	crash_dumper.enableFullMemoryDump(TRUE);
	crash_dumper.setExceptionCB(crashHandler);
}

//////////////////////////////////////////////////////////////////////////

IceService iceService;

DataStreamShell dataStreamShell;
ZQLIB::BaseZQServiceApplication* Application = &dataStreamShell;
DWORD gdwServiceType = 0;
DWORD gdwServiceInstance = 0;

//////////////////////////////////////////////////////////////////////////

DataStreamShell::DataStreamShell()
{

}

DataStreamShell::~DataStreamShell()
{

}

static void clearCache(const std::string& path)
{
	char cmdLine[512];
	sprintf(cmdLine, "cmd /c del /Q /S %s\\*.dod", path.c_str());
	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;
	PROCESS_INFORMATION pi;
	
	CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, 
		NULL, NULL, &si, &pi);
}

static void createCacheDir(const char* dirName)
{
	char pathName[MAX_PATH];
	const char* c = dirName;
	while (*c) {
		if (*c == '\\') {
			size_t len = c - dirName;
			strncpy(pathName, dirName, len);
			pathName[len] = 0;
			CreateDirectoryA(pathName, NULL);
		}

		c ++;
	}

	CreateDirectoryA(dirName, NULL);
}

HRESULT DataStreamShell::OnStart(void)
{
	HRESULT result = BaseZQServiceApplication::OnStart();

	clearCache(gDataStreamConfig.catchDir);
	createCacheDir(gDataStreamConfig.catchDir);

	if (gDataStreamConfig.netId.size() <= 0) {
		glog(ZQLIB::Log::L_ERROR, "netId not found.");
		return E_FAIL;
	}

	if (gDataStreamConfig.stdPeriod == 0)
		gDataStreamConfig.stdPeriod = DEFAULT_STD_PERIOD;

	if (gDataStreamConfig.stdPeriod > 1000)
		gDataStreamConfig.stdPeriod = 1000;

	if (gDataStreamConfig.stdMaxQueue == 0)
		gDataStreamConfig.stdMaxQueue = DEFAULT_STD_MAXQUEUE;

	if (gDataStreamConfig.stdMaxQueue > 256)
		gDataStreamConfig.stdMaxQueue = 256;

	if (gDataStreamConfig.stdMinQueue == 0)
		gDataStreamConfig.stdMinQueue = DEFAULT_STD_MINQUEUE;

	if (gDataStreamConfig.stdMinQueue > 256)
		gDataStreamConfig.stdMinQueue = 256;

	if (!::DataStream::initializeDataStream()) {
		glog(ZQLIB::Log::L_ERROR, "initializeDataStream() failed");
		return E_FAIL;
	}

	glog(ZQLIB::Log::L_INFO, "DataStreamShell::OnInit()" LOG_FMT, 
		LOG_ARG);

	iceService.start();

	return result;
}

HRESULT DataStreamShell::OnStop(void)
{	
	iceService.stop();
	::DataStream::uninitializeDataStream();
	return BaseZQServiceApplication::OnStop();
}

HRESULT DataStreamShell::OnInit(void)
{
	HRESULT result = BaseZQServiceApplication::OnInit();
	if (result != S_OK)
		return result;

	srand(timeGetTime());

	/*
	ZQLIB::CSvcLog* logger = new ZQLIB::CSvcLog();
	logger->setAppName(_T("DataStream"));
	if (!logger->init(cfg.logFile, 
		(ZQLIB::Log::loglevel_t )cfg.logLevel,
		cfg.logFileSize)) {
		
		logger->beep(0);
		return E_FAIL;
	}
	*/
	
	initMiniDump();
	bool ret = DataStream::Transfer::InitSocket();
	if(!ret)
	{
		glog(ZQLIB::Log::L_INFO, "DataStreamShell::OnInit() init WSAStartup() failed!");
		return S_FALSE;
	};
	return S_OK;
}

HRESULT DataStreamShell::OnUnInit(void)
{	
	try
	{
		WSACleanup();
	}
	catch(...)
	{

	}
	return BaseZQServiceApplication::OnUnInit();
}

//////////////////////////////////////////////////////////////////////////
