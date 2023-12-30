#include "TsPumperService.h"
#include <ZQ_common_conf.h>
#include "ConfigHelper.h"

#ifdef ZQ_OS_MSWIN
#include "MiniDump.h"
#endif
//#include "Log.h"
using ZQ::common::BaseZQServiceApplication;
//using ZQ::common::Log;

#ifdef ZQ_OS_MSWIN
#define TSFOLDERNAME "d:\\temp\\aaa\\ts"
#elif defined ZQ_OS_LINUX
#define TSFOLDERNAME "/opt/TianShan/data/DsmccCRG/sgts"
#endif


TsPumperService		tsPumperSvc;

BaseZQServiceApplication* Application = &tsPumperSvc;

//ZQ::common::Log ZQ::common::NullLogger;

//ZQ::common::Log* ZQ::common::pGlog = NULL;


using namespace ZQ::common;

struct TsPumperSvcCfg
{
	std::string      _destAddr;
	int				_destPort;
	std::string		_folderName;
	std::string		_sHex;
	bool			_bHex;
	int				_interval;
	std::string		_crashDumpPath;
	std::string		_dexHexCommand;
	int				_scanFolderInterval;
	int             _subfolderDepth;

	static void structure(Config::Holder<TsPumperSvcCfg> &holder)
	{

		holder.addDetail("TSPump/ServiceGroupAds","port",&TsPumperSvcCfg::_destPort,"0",Config::optReadOnly);
		holder.addDetail("TSPump/ServiceGroupAds","tsFolder",&TsPumperSvcCfg::_folderName,TSFOLDERNAME);
		holder.addDetail("TSPump/ServiceGroupAds","bindIp",&TsPumperSvcCfg::_destAddr, "0.0.0.0");
		holder.addDetail("TSPump/ServiceGroupAds","hexMode",&TsPumperSvcCfg::_sHex,"false");
		holder.addDetail("TSPump/ServiceGroupAds","interval",&TsPumperSvcCfg::_interval,"100",Config::optReadOnly);
		holder.addDetail("TSPump/ServiceGroupAds","deHexCommand",&TsPumperSvcCfg::_dexHexCommand,"xxd -s");
		holder.addDetail("TSPump/ServiceGroupAds","scanFolderInterval",&TsPumperSvcCfg::_scanFolderInterval,"300000",Config::optReadOnly);
		holder.addDetail("TSPump/ServiceGroupAds","subfolderDepth",&TsPumperSvcCfg::_subfolderDepth,"0",Config::optReadOnly);
		holder.addDetail("TSPump/CrashDump","path",&TsPumperSvcCfg::_crashDumpPath,"");


	}
	TsPumperSvcCfg()
	{
		_destPort = 0;
		_folderName = TSFOLDERNAME;
		_bHex = 0;
		_interval = 100;

	}
};

ZQ::common::Config::Loader<TsPumperSvcCfg> gTsPumperSvcCfg("TSPump.xml");
ZQ::common::Config::ILoader *configLoader = &gTsPumperSvcCfg;

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;
ZQ::common::MiniDump g_miniDump;

void WINAPI MiniDumpCallback(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();

	glog(ZQ::common::Log::L_ERROR,
		"Crash exception callback called,ExceptionCode 0x%08x, "
		"ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
		ExceptionCode, ExceptionAddress, dwThreadID);
}

bool
TsPumperService::initMiniDump()
{
	std::string dumpPath = gTsPumperSvcCfg._crashDumpPath;
	if (dumpPath[dumpPath.size() - 1] != '\\' && dumpPath[dumpPath.size() - 1] != '/')
		dumpPath += '\\';

	if(!g_miniDump.setDumpPath((char*)dumpPath.c_str()))
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(TsPumperService, 
			"key [CrashDumpPath] is not correct directory"));
		return false;
	}
	g_miniDump.enableFullMemoryDump(true);
	g_miniDump.setExceptionCB(MiniDumpCallback);
	return true;
}
#elif defined MS_OS_LINUX

#endif

TsPumperService::TsPumperService()
{
	_pTsPumper = NULL;
}

TsPumperService::~TsPumperService()
{

}

HRESULT
TsPumperService::OnInit()
{
	BaseZQServiceApplication::OnInit();

//	ZQ::common::pGlog = m_pReporter;
	setGlogger(m_pReporter);

	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumperService, "OnInit() enter"));

	if(0 == strcmp(gTsPumperSvcCfg._sHex.c_str(),"true"))
		gTsPumperSvcCfg._bHex = true;
	else
		gTsPumperSvcCfg._bHex = false;

	TsPumper::_cmd_Dehex = gTsPumperSvcCfg._dexHexCommand;
#ifdef ZQ_OS_MSWIN
	if(-1 == ::GetFileAttributes(gTsPumperSvcCfg._folderName.c_str()))
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumperService, "the folder is not exist"));
		return E_HANDLE;
	}
#elif defined ZQ_OS_LINUX
	if(opendir(gTsPumperSvcCfg._folderName.c_str()) == NULL)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumperService, "the folder is not exist"));
		return false;
	}		
#endif
	ZQ::common::InetHostAddress destAddr(gTsPumperSvcCfg._destAddr.c_str());
	_pTsPumper = new TsPumper(destAddr,gTsPumperSvcCfg._destPort,
									const_cast<char*>(gTsPumperSvcCfg._folderName.c_str()),
										gTsPumperSvcCfg._bHex,(int16)gTsPumperSvcCfg._interval,gTsPumperSvcCfg._scanFolderInterval, gTsPumperSvcCfg._subfolderDepth);

	glog(ZQ::common::Log::L_INFO,CLOGFMT(TsPumperService,"the configure information: destIP:%s port:%d file name:%s file type:%s interval:%d scanFolderInterval:%dsubfolderDepth:%d" ),
												gTsPumperSvcCfg._destAddr.c_str(),gTsPumperSvcCfg._destPort,gTsPumperSvcCfg._folderName.c_str(),
													gTsPumperSvcCfg._bHex?".hex":".ts",gTsPumperSvcCfg._interval,gTsPumperSvcCfg._scanFolderInterval,
													gTsPumperSvcCfg._subfolderDepth);

#ifdef ZQ_OS_MSWIN
	// init Minidump		
	if (!initMiniDump())
	{
		return E_HANDLE;
	}
#endif
	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumperService, "OnInit() leave"));

	return S_OK;
}

HRESULT
TsPumperService::OnStart()
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumperService, "OnStart() enter"));

	BaseZQServiceApplication::OnStart();

	try
	{
		if(_pTsPumper)
			_pTsPumper->start();
	}
	catch (...)
	{

	}


	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumperService, "OnStart() leave"));

	return S_OK;
}

HRESULT
TsPumperService::OnStop()
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumperService, "OnStop() enter"));

	try
	{
		if(_pTsPumper)
			_pTsPumper->stop();
	}
	catch (...)
	{
		
	}


	BaseZQServiceApplication::OnStop();

	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumperService, "OnStop() leave"));

	//Sleep(10);

	return S_OK;
}

HRESULT
TsPumperService::OnUnInit()
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumperService, "OnUnInit enter()!"));

	try{
		if(_pTsPumper)
		{
			delete _pTsPumper;
			_pTsPumper = NULL;
		}
	}
	catch (...)
	{

	}

	BaseZQServiceApplication::OnUnInit();

	glog(ZQ::common::Log::L_INFO, CLOGFMT(TsPumperService, "OnUnInit leave()!"));
  
	return S_OK;	
}

