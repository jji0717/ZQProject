#include "CPEService.h"
#include "CPEEnv.h"
#include "CPECfg.h"
#include <IceLog.h>


#define CPESvc		"CPEService"	
#define ICELog		"ICELog"
#define MOLOG		glog

ZQTianShan::CPE::ContentProvisionEngineSvc g_server;
ZQ::common::ZQDaemon* Application = &g_server;

ZQ::common::Config::ILoader* configLoader = &_gCPECfg;

ZQ::common::NativeThreadPool*	_pCPEThreadPool=NULL;
::ZQ::common::NativeThreadPool*	_pCPETimerThreadPool=NULL;
TianShanIce::ContentProvision::ContentProvisionServicePtr _cps;
Ice::CommunicatorPtr	_ic;

extern const char* DUMP_PATH;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace ZQTianShan {
namespace CPE {

/////////////////////////////////////////
// class IceLogger ///////////////
/////////////////////////////////////////
/*
void IceLogger::print(const ::std::string& message)
{
	_logger(ZQ::common::Log::L_INFO, CLOGFMT(ICELog, "%s"), message.c_str());
}

void IceLogger::trace(const ::std::string& category, const ::std::string& message)
{
	_logger(ZQ::common::Log::L_DEBUG, CLOGFMT(ICELog, "catagory [%s],message [%s]"), category.c_str(),message.c_str());
}

void IceLogger::warning(const ::std::string& message)
{
	_logger(ZQ::common::Log::L_WARNING, CLOGFMT(ICELog, "%s"), message.c_str());
}

void IceLogger::error(const ::std::string& message)
{
	_logger(ZQ::common::Log::L_ERROR, CLOGFMT(ICELog, "%s"), message.c_str());
}
*/
////////////////////////////////////////////
//class ContentProvisionEngineSvc ///
//////////////////////////////////////////
ContentProvisionEngineSvc::ContentProvisionEngineSvc()
: _iceLogFile(NULL), _svcLog(NULL), _pCPEEnv(NULL)
{

}

ContentProvisionEngineSvc::~ContentProvisionEngineSvc()
{

}

bool ContentProvisionEngineSvc::OnInit()
{
	//log folder
	_strLogFolder = _logDir;
	if(_strLogFolder.length()< 1)
	{
		//get the install root
		char szProgramRootPath[1024];
		memset(szProgramRootPath,0,sizeof(szProgramRootPath));
		readlink("/proc/self/exe",szProgramRootPath,sizeof(szProgramRootPath)-1);
		int iLens = strlen(szProgramRootPath)-1;
		bool bSecondToken = false;
		while ( iLens > 0 )
		{
			if (szProgramRootPath[iLens]=='/') 
			{
				if (bSecondToken) 
				{
					szProgramRootPath[iLens+1]='\0';
					break;
				}
				else
				{
					bSecondToken =true;
				}			
			}
			iLens--;
		}

		if (iLens <= 0) 
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPESvc, "Can't get install root path"));
			return false;
		}
		_strLogFolder = szProgramRootPath;
		_strLogFolder += "logs/";
	}
	
	size_t nEnd = _strLogFolder.length() - 1;
	if(_strLogFolder[nEnd] != FNSEPC)
		_strLogFolder += FNSEPS;
	
	_svcLog = _logger;
	ZQ::common::setGlogger(_svcLog);

	if(_svcLog == NULL)
	{
		

		std::string strLogFile = _strLogFolder + getServiceName() + ".log";
		try
		{
			_svcLog = new ZQ::common::FileLog(strLogFile.c_str(),
					ZQ::common::Log::L_DEBUG);
			(*_svcLog)(Log::L_DEBUG,"/////////////////////////////////////////////CPEService OnInit///////////////////////////////////");
			
			ZQ::common::setGlogger(_svcLog);
			_logger = _svcLog;
		}
		catch (ZQ::common::FileLogException& ex)
		{
			_svcLog = NULL;
			MOLOG(Log::L_ERROR, CLOGFMT(CPESvc, "Failed to create log file [%s] with error[%s]")
				, strLogFile.c_str(), ex.getString());
			return false;			
		}		
	}	
	
	DUMP_PATH = _gCPECfg._szCrashDumpPath;
	
	//set threadpool
	_pCPEThreadPool = new ZQ::common::NativeThreadPool();
	if ( !_pCPEThreadPool) 
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPESvc, "Can't create threadpool instance"));
		return false;
	}

	if ( _pCPEThreadPool )
		_pCPEThreadPool->resize( _gCPECfg._dwThreadPool> 10 ? _gCPECfg._dwThreadPool : 10);
	
		_pCPETimerThreadPool=new ZQ::common::NativeThreadPool();

	if ( !_pCPETimerThreadPool) 
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPESvc, "Can't create threadpool instance"));
		return false;
	}

	if ( _pCPETimerThreadPool ) 
	{
		_pCPETimerThreadPool->resize( _gCPECfg._dwtimerThreadPool> 5 ? 
			_gCPECfg._dwtimerThreadPool : 5);
	}

	//init ice
	if(!InitIce())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPESvc, "failed to initialize ice property"));
		return false;
	}
	
	std::string strDbPath = _gCPECfg._szIceDbFolder;
	bool bEnv = true;
	try
	{	
		_pCPEEnv = new CPEEnv(*_svcLog, *_pCPEThreadPool, _ic, *_pCPETimerThreadPool);		
		if (!_pCPEEnv->init(_gCPECfg._cpeEndPoint, strDbPath.c_str(), strDbPath.c_str()))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPESvc, "failed to initialize CPE enviroment on path %s"), strDbPath.c_str());
			bEnv = false;
		}
	}
	catch (...)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPESvc, "failed to initialize CPE enviroment on path %s, caught unknown exception"), strDbPath.c_str());
		bEnv = false; 		
	}	

	if (!bEnv)
	{
		if (_pCPEEnv)
		{
			delete _pCPEEnv;
			_pCPEEnv = NULL;
		}

		return false;
	}

	_cps = new ZQTianShan::CPE::CPEImpl(*_pCPEEnv);
	MOLOG(Log::L_INFO, CLOGFMT(CPESvc, "ContentProvisionService created"));

    	_gCPECfg.snmpRegister("");
	
	return ZQDaemon::OnInit();
}

bool ContentProvisionEngineSvc::OnStart(void)
{
	if(_pCPEEnv)
	{
#ifndef _INDEPENDENT_ADAPTER
		std::vector<MonitoredLog>::iterator iter;
		for (iter = _gCPECfg.monitoredLogs.begin(); iter != _gCPECfg.monitoredLogs.end(); ++iter) 
		{			
			if (!_pCPEEnv->_adapter->publishLogger(iter->name.c_str(), iter->syntax.c_str(),iter->syntaxKey.c_str()))				
			{				
				glog(ZQ::common::Log::L_ERROR, "Failed to publish logger name[%s] synax[%s] key[%s]", 					
					iter->name.c_str(), iter->syntax.c_str(),iter->syntaxKey.c_str());				
			}			
			else				
			{				
				glog(ZQ::common::Log::L_INFO, "Publish logger name[%s] synax[%s] key[%s] successful", 					
					iter->name.c_str(), iter->syntax.c_str(),iter->syntaxKey.c_str());				
			}			
		}
#endif
		_pCPEEnv->start();        
	}
	else
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPESvc, "No enviroment is setuped"));
		return false;
	}
	return ZQDaemon::OnStart();
}

void ContentProvisionEngineSvc::OnStop(void)
{
	if (_cps)
	{
		try
		{
			_cps = NULL;
		}
		catch(...){ }		
	}

	if(_pCPEEnv)
	{
		try
		{
			delete _pCPEEnv;
			_pCPEEnv = NULL;
		}
		catch(...){ }		
	}
	
	try
	{
		_iceLogger = NULL;
	}
	catch(...){}
	
	ZQDaemon::OnStop();
}

void ContentProvisionEngineSvc::OnUnInit(void)
{
	
	try
	{
		_ic->destroy();
	}
	catch (...){}
	
	
	if (_iceLogFile)
	{
		try
		{
			delete _iceLogFile;
			_iceLogFile= NULL;
		}
		catch (...){}
	}
/*	
	if(_svcLog)
	{
		try
		{
			delete _svcLog;
			_svcLog = NULL;
		}
		catch(...){}
	}
*/	
	if(_pCPEThreadPool)
	{
		try
		{
			delete _pCPEThreadPool;
			_pCPEThreadPool=NULL;
		}
		catch(...){ }
	}
	if(_pCPETimerThreadPool)
	{
		try
		{
			delete _pCPETimerThreadPool;
			_pCPETimerThreadPool=NULL;
		}
		catch(...){ }
	}

	ZQDaemon::OnUnInit();
}

bool ContentProvisionEngineSvc::InitIce(void)
{
	int i=0;
	Ice::InitializationData iceInitData;
	iceInitData.properties =Ice::createProperties(i,NULL);
	initWithConfig(iceInitData.properties);	

	if (_gCPECfg._dwEnableIceLog)
	{
		char strFile[512] ={0};
		sprintf(strFile,"%s",_strLogFolder.c_str());
		
		// append the log name
		{
			char* pPtr = strFile + strlen(strFile);
			while(pPtr>strFile && *(pPtr - 1)==' ') pPtr--;
			
			if (pPtr == strFile || *(pPtr - 1)=='/') 
				sprintf(pPtr, "%s.IceTrace.log", getServiceName().c_str());
			else
				sprintf(pPtr, "/%s.IceTrace.log", getServiceName().c_str());
		}
		try
		{
			_iceLogFile = new ZQ::common::FileLog(strFile,
					_gCPECfg._iceLogLevel,
					_gCPECfg._dwIceLogFileCount,
					_gCPECfg._dwIceLogFileSize);
		}
		catch (ZQ::common::FileLogException& ex)
		{
			_iceLogFile = NULL;
			MOLOG(Log::L_WARNING, CLOGFMT(CPESvc, "failed to open ice log file [%s] with error[%s]")
				, strFile, ex.getString());
			return false;			
		}
		if (_iceLogFile)
		{
			iceInitData.logger = new TianShanIce::common::IceLogI(_iceLogFile);
		}
	}
	_ic=Ice::initialize(i,NULL,iceInitData);
	
	return true;
}

void ContentProvisionEngineSvc::initWithConfig(Ice::PropertiesPtr proper)
{
	proper->setProperty("Ice.ThreadPool.Client.Size","5");
	proper->setProperty("Ice.ThreadPool.Client.SizeMax","10");

    	std::map<std::string, std::string>::iterator itProper;
	for( itProper = _gCPECfg.iceProperties.begin(); 
		itProper!=_gCPECfg.iceProperties.end();
		itProper++)
	{		
		proper->setProperty(itProper->first, itProper->second);//((*itProper)["name"],(*itProper)["value"]);
		MOLOG(Log::L_INFO, CLOGFMT(CPESvc, "Set ICE Property %s=%s"), itProper->first.c_str(), itProper->second.c_str());			
	}

	char tmp[64];
	sprintf(tmp, "%d", _gCPECfg._cpeDispatchSize);
	proper->setProperty("Ice.ThreadPool.Server.Size", tmp);
	MOLOG(Log::L_INFO, CLOGFMT(CPESvc, "Set ICE Property %s=%s"), "Ice.ThreadPool.Server.Size", tmp);			
	sprintf(tmp, "%d", _gCPECfg._cpeDispatchSizeMax);
	proper->setProperty("Ice.ThreadPool.Server.SizeMax", tmp);
	MOLOG(Log::L_INFO, CLOGFMT(CPESvc, "Set ICE Property %s=%s"), "Ice.ThreadPool.Server.SizeMax", tmp);
}


}}

