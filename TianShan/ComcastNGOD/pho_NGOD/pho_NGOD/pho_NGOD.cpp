// pho_NGOD.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "IpEdgePHO.h"
#include <filelog.h>
#include "Configuration.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

typedef ::IceInternal::Handle< ZQTianShan::AccreditedPath::IpEdgePHO > IpEdgePHOPtr;

IpEdgePHOPtr   _gIpEdgePHOPtr = NULL;
bool bInService = false;
ZQ::common::Config::Loader<PHOConfig>       cfg("Weiwoo.xml");

extern "C"
{
__declspec(dllexport) void InitPHO( ZQTianShan::AccreditedPath::IPHOManager& mgr,
									void* pCtx,
									const char* configFile,
									const char* logFolder)
{
	if (bInService) 
		return;	
	std::string		strLogFolder;
	std::string		strLogFileName	= "pho_NGOD.log";
	long			logFileSize		= 50*1024*1024;
	long			logBufferSize	= 204800;
	long			logWriteTimeout	= 2;
	long			logLevel=7;
	if(configFile&&strlen(configFile)>0)
	{
		//read the config file

		if(cfg.load(configFile))
		{
			strLogFileName=cfg.szPHOLogFileName;
			logFileSize=cfg.lPHOLogFileSize;
			logBufferSize=cfg.lPHOLogBufferSize;
			logWriteTimeout=cfg.lPHOLogWriteTimteout;				
		}
	}
	if( logFolder&&strlen(logFolder) > 0 )
	{
		strLogFolder=logFolder;
	}
	else
	{
		char szBuf[1024];
		GetModuleFileNameA( NULL , szBuf , sizeof(szBuf)-1 );
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



	ZQ::common::Log* phoNgodLog = new ZQ::common::FileLog( (strLogFolder+strLogFileName).c_str(),
												logLevel,
												ZQLOG_DEFAULT_FILENUM,
												logFileSize,
												logBufferSize,
												logWriteTimeout);
	
	ZQ::common::setGlogger(phoNgodLog);
	if (!_gIpEdgePHOPtr)
		_gIpEdgePHOPtr = new ZQTianShan::AccreditedPath::IpEdgePHO(mgr);
	if ( _gIpEdgePHOPtr )
	{
		mgr.registerStreamLinkHelper(STRMLINK_TYPE_NGOD, *_gIpEdgePHOPtr, pCtx);
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR , CLOGFMT(pho_NGOD,"create IpEdgePHO failed"));
	}
	
	bInService = ( _gIpEdgePHOPtr != NULL );

}

__declspec(dllexport)  void UninitPHO(void)
{
	ZQ::common::Log* phoNgodLog = ZQ::common::getGlogger();
	ZQ::common::setGlogger(NULL);
	if(phoNgodLog)
	{
		try
		{
			delete phoNgodLog;
			phoNgodLog = NULL;
		}
		catch (...)
		{
		}
	}
	bInService = false;	
	_gIpEdgePHOPtr = NULL;
}


}