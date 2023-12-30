// pho_NGOD_ss.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <log.h>
#include "IpEdgePHO.h"
#include "Config.h"
#include <filelog.h>

HANDLE _gCurrentModule=NULL;
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	_gCurrentModule=hModule;
    return TRUE;
}

//typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::Raid5sqrPHO > Raid5sqrPHOPtr;
typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::IpEdgePHO > IpEdgePHOPtr;

//Raid5sqrPHOPtr _gRaid5sqrPHOPtr = NULL;
IpEdgePHOPtr   _gIpEdgePHOPtr = NULL;

bool bInService = false;

extern "C"
{
__declspec(dllexport) void InitPHO(ZQTianShan::AccreditedPath::IPHOManager& mgr, void* pCtx,const char* configFile,const char* logFolder)
{
	if (bInService) 
		return;	
	std::string		strLogFolder;
	std::string		strLogFileName="pho_NGOD_ss.log";
	long			logFileSize=50*1024*1024;
	long			logBufferSize=204800;
	long			logWriteTimeout=2;
	long			logLevel=7;
	if(configFile&&strlen(configFile)>0)
	{
		//read the config file
		Config cfg;
		if(cfg.load((char*)configFile))
		{
			strLogFileName=cfg.szPHOLogFileName;
			logFileSize=cfg.lPHOLogFileSize;
			logBufferSize=cfg.lPHOLogBufferSize;
			logWriteTimeout=cfg.lPHOLogWriteTimteout;				
		}
	}
	if(logFolder&&strlen(logFolder)>0)
	{
		strLogFolder=logFolder;
	}
	else
	{
		char szBuf[1024];
		GetModuleFileNameA((HMODULE)_gCurrentModule,szBuf,sizeof(szBuf)-1);
		strLogFolder=szBuf;
		std::string::size_type iPos=strLogFolder.rfind("\\");
		if(iPos!=std::string::npos)
		{
			strLogFolder=strLogFolder.substr(0,iPos);
		}
	}
	if(  !(strLogFolder.at(strLogFolder.length()-1)=='\\' || strLogFolder.at(strLogFolder.length()-1)=='/'  )   )
	{
		strLogFolder+='\\';
	}
	
	
	
	ZQ::common::Log* phoNgodSSLog=new ZQ::common::FileLog( (strLogFolder+strLogFileName).c_str(),
		logLevel,
		ZQLOG_DEFAULT_FILENUM,
		logFileSize,
		logBufferSize,
		logWriteTimeout);
	
	ZQ::common::setGlogger(phoNgodSSLog);
//	if (!_gRaid5sqrPHOPtr)
//		_gRaid5sqrPHOPtr = new ZQTianShan::AccreditedPath::Raid5sqrPHO(mgr);
	
	if (!_gIpEdgePHOPtr)
		_gIpEdgePHOPtr = new ZQTianShan::AccreditedPath::IpEdgePHO(mgr);
	
	//mgr.registerStorageLinkHelper(STORLINK_TYPE_RAID5SQR, *_gRaid5sqrPHOPtr, pCtx);
	mgr.registerStreamLinkHelper(STRMLINK_TYPE_NGOD_SS, *_gIpEdgePHOPtr, pCtx);	
	
	bInService = (/*NULL != _gRaid5sqrPHOPtr && */NULL != _gIpEdgePHOPtr);
	glog(ZQ::common::Log::L_DEBUG,"");
}
}
extern "C"
{
__declspec(dllexport) void UninitPHO(void)
{
	ZQ::common::Log* phoNgodSSLog = ZQ::common::getGlogger();
	ZQ::common::setGlogger(NULL);
	if(phoNgodSSLog)
	{
		try
		{
			delete phoNgodSSLog;
			phoNgodSSLog = NULL;
		}
		catch (...)
		{
		}
	}
	bInService = false;
//	_gRaid5sqrPHOPtr = NULL;
	_gIpEdgePHOPtr = NULL;
}
}