#include <fstream>
#include "Log.h"
#include "FileLog.h"
#include "CPESvc.h"
#include "NativeThread.h"
#include "NativeThreadPool.h"
#include "CPECfg.h"
#include  "TimeUtil.h"
#include <sys/stat.h>
#include <sys/types.h>
#ifdef ZQ_OS_MSWIN
#include "MiniDump.h"
#endif

#include "CPEEnv.h"
#include "IceLog.h"
using namespace ZQ::SNMP;

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;
#endif

ZQTianShan::CPE::ContentProvisionEngineSvc g_server;
ZQ::common::BaseZQServiceApplication *Application = &g_server;

ZQ::common::Config::ILoader *configLoader=&_gCPECfg;
Ice::CommunicatorPtr	_ic;					/*<!-ice communicator*/	

ZQ::common::NativeThreadPool*	_pCPEThreadPool=NULL;
ZQ::common::NativeThreadPool*	_pCPETimerThreadPool=NULL;

TianShanIce::ContentProvision::ContentProvisionServicePtr _cps;

#ifdef ZQ_OS_MSWIN
ZQ::common::MiniDump			_crashDump;
extern bool validatePath(const char* szPath );
#else
extern const char* DUMP_PATH;
#endif

#define MOLOG		glog
#define CPESvc		"CPESvc"	

#ifdef ZQ_OS_MSWIN
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();	
	glog( ZQ::common::Log::L_ERROR,  L"Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",ExceptionCode, ExceptionAddress, dwThreadID);	
	glog.flush();
}
#endif

void initWithConfig(Ice::PropertiesPtr proper )
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
#ifdef ZQ_OS_LINUX

class MountpointTypeChecker
{
public:
	MountpointTypeChecker(){}
	virtual ~MountpointTypeChecker(){}
public:
	bool init()
	{
		std::ifstream mounts;
		mounts.open("/proc/mounts");
		if(!mounts.is_open())
		{
			MOLOG(ZQ::common::Log::L_ERROR,"CAN'T OPEN /proc/mounts");
			return false;
		}
		char szline[2048];
		while(!mounts.eof())
		{
			szline[0] = 0;
			szline[sizeof(szline)-1]= 0;
			mounts.getline(szline,sizeof(szline)-1);

			if(strlen(szline)<=0 || szline[0] =='#')
				continue;// skip comment

			std::string line =  szline;
			ZQ::common::stringHelper::TrimExtra(line," \t\r\n");

			std::vector<std::string> tmp;
			ZQ::common::stringHelper::SplitString(line,tmp," \t"," \t\r\n");
			if( tmp.size() < 3 )
				continue;
			const std::string& mountpath = tmp[1];
			const std::string& type = tmp[2];
			addnewdeviceinfo(mountpath,type);
		}
		mounts.close();
		return true;
	}

	void addnewdeviceinfo( const std::string& path , const std::string& type )
	{
		struct stat st;
		memset(&st,sizeof(st),sizeof(st));
		if( stat(path.c_str() , &st) != 0 )
		{
			MOLOG(ZQ::common::Log::L_WARNING,CLOGFMT(MountpointTypeChecker,"failed to stat [%s]"),path.c_str());
			return;
		}
		mMountTypeMap[st.st_dev] =  type;
	}
	bool isMountPathLegal( const std::string& path, const std::string& type)
	{
		if(path.empty() )
			return false;
		if( type.empty() )
			return true;

		struct stat st;
		memset(&st,sizeof(st),sizeof(st));
		if( stat(path.c_str() , &st) != 0 )
		{
			MOLOG(ZQ::common::Log::L_WARNING,CLOGFMT(MountpointTypeChecker,"failed to stat [%s]"),path.c_str());
			return false;
		}
		std::map<dev_t,std::string>::const_iterator it = mMountTypeMap.find(st.st_dev);
		if( it == mMountTypeMap.end())
			return false;
		return it->second == type;
	}
private:
	std::map<dev_t,std::string> mMountTypeMap; };

#endif

bool ContentProvisionEngineSvc::InitIce(void)
{
	int i=0;
	Ice::InitializationData iceInitData;
	iceInitData.properties =Ice::createProperties(i,NULL);
	initWithConfig(iceInitData.properties);	

	if (_gCPECfg._dwEnableIceLog)
	{
		char strFile[512] ={0};
#ifdef ZQ_OS_MSWIN
	#if defined _UNICODE  || defined UNICODE
	WideCharToMultiByte(CP_ACP,NULL,m_wsLogFolder,-1,strFile,sizeof(strFile),NULL,NULL);
	#else
	sprintf(strFile,"%s",m_wsLogFolder);
	#endif
#else
	sprintf(strFile,"%s",_strLogFolder.c_str());
#endif
		
		// append the log name
		{
			char* pPtr = strFile + strlen(strFile);
			while(pPtr>strFile && *(pPtr - 1)==' ') pPtr--;
			
			if (pPtr == strFile || *(pPtr - 1)==FNSEPC) {
				sprintf(pPtr, "%s.IceTrace.log", 
#ifdef ZQ_OS_MSWIN
							m_sServiceName
#else
							getServiceName().c_str()
#endif
					   );
			}
			else {
				sprintf(pPtr, "%s%s.IceTrace.log", FNSEPS,
#ifdef ZQ_OS_MSWIN
							m_sServiceName
#else
							getServiceName().c_str()
#endif
					   );
			}
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

namespace ZQTianShan {
namespace CPE {


ContentProvisionEngineSvc::ContentProvisionEngineSvc()
:_iceLogFile(NULL)
,_pCPEEnv(NULL) 
#ifdef ZQ_OS_LINUX
,_svcLog(NULL)
#endif
{
#ifdef ZQ_OS_MSWIN
	strcpy(servname, "CPESvc");
	strcpy(prodname, "TianShan");	
#endif
}

ContentProvisionEngineSvc::~ContentProvisionEngineSvc()
{

}

HRESULT ContentProvisionEngineSvc::OnInit()
{
//  	std::string			_strLogFolder;
//  	std::string			_strPluginFolder;

#ifdef ZQ_OS_MSWIN
	// covered by basesvc: ZQ::common::setGlogger(m_pReporter);
#else
	_strLogFolder = _logDir;
#endif

#ifdef ZQ_OS_MSWIN
	char	szProgramRootPath[2048];
	ZeroMemory(szProgramRootPath,sizeof(szProgramRootPath));
	GetModuleFileNameA(NULL,szProgramRootPath,sizeof(szProgramRootPath)-1);
	int iLens = strlen(szProgramRootPath)-1;
	bool bSecondToken = false;
	while ( iLens > 0 )
	{
		if (szProgramRootPath[iLens]=='\\') 
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
		logEvent(ZQ::common::Log::L_ERROR,_T("Can't get install root path"));
		return S_FALSE;
	}
	m_strProgramRootPath = szProgramRootPath;
	
	std::string	strCrashDumpPath ;
	if (strstr(_gCPECfg._szCrashDumpPath,":")!=NULL) 
	{
		strCrashDumpPath = _gCPECfg._szCrashDumpPath;
	}
	else
	{
		strCrashDumpPath=szProgramRootPath;
		strCrashDumpPath+=_gCPECfg._szCrashDumpPath;		
	}
	
	if (!validatePath(strCrashDumpPath.c_str()))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPESvc, "CrashDumpPath %s error"), strCrashDumpPath.c_str());
		logEvent(ZQ::common::Log::L_ERROR, "invalid minidump path %s",strCrashDumpPath.c_str());
		return S_FALSE;
	}	

	if (_gCPECfg._crashDumpEnabled)
	{
		_crashDump.setDumpPath((char*)strCrashDumpPath.c_str());
		_crashDump.enableFullMemoryDump(_gCPECfg._dwDumpFullMemory);
		_crashDump.setExceptionCB(CrashExceptionCallBack);
		_crashDump.setServiceName(m_sServiceName);
	}
#else
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
	if(_gCPECfg.volumeMounts._enableVoumeMounts && !_gCPECfg.volumeMounts.volumes.empty())
	{

		bool bMounted = false;
			MountpointTypeChecker typechecker;
			typechecker.init();
			Volumes::const_iterator iter = _gCPECfg.volumeMounts.volumes.begin();
			for(; iter != _gCPECfg.volumeMounts.volumes.end(); ++iter)
			{
				std::string path = iter->path;
				if(iter->path.at(iter->path.length()-1) != FNSEPC) {
					path += FNSEPC;
				}
				if(!typechecker.isMountPathLegal(path,iter->fstype) )
				{
					MOLOG(ZQ::common::Log::L_WARNING,"path[%s] is not mounted with type[%s]",path.c_str(),iter->fstype.c_str());
					continue;
				}
				MOLOG(ZQ::common::Log::L_INFO,"Mount volume [%s] with path[%s] type[%s]",iter->name.c_str() , path.c_str(), iter->fstype.c_str());
				bMounted = true;
				break;
			}
		if(!bMounted)
		{
			MOLOG(ZQ::common::Log::L_ERROR,"no volume is mounted, quit");
			return S_FALSE;
		}
	}
#endif

	_pCPEThreadPool=new ZQ::common::NativeThreadPool();

	if ( !_pCPEThreadPool) 
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPESvc, "Can't create threadpool instance"));
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_CRIT,"Can't create threadpool instance ,service down");
#endif
		return S_FALSE;
	}

	if ( _pCPEThreadPool ) 
	{
		_pCPEThreadPool->resize( _gCPECfg._dwThreadPool> 10 ? 
								_gCPECfg._dwThreadPool : 10);
	}

	_pCPETimerThreadPool=new ZQ::common::NativeThreadPool();

	if ( !_pCPETimerThreadPool) 
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPESvc, "Can't create threadpool instance"));
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_CRIT,"Can't create threadpool instance ,service down");
#endif
		return S_FALSE;
	}

	if ( _pCPETimerThreadPool ) 
	{
		_pCPETimerThreadPool->resize( _gCPECfg._dwtimerThreadPool> 5 ? 
			_gCPECfg._dwtimerThreadPool : 5);
	}

#ifdef ZQ_OS_MSWIN
	TCHAR	szBuf[512];
	DWORD dwSize=sizeof(szBuf)/sizeof(szBuf[0]);
	ZeroMemory(szBuf,sizeof(szBuf));
	
	_strLogFolder=m_wsLogFolder;
	if(_strLogFolder.size()<=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPESvc, "Invalid logFolder %s"),_strLogFolder.c_str() );
		logEvent(ZQ::common::Log::L_ERROR,_T("Invalid logFolder %s"),_strLogFolder.c_str() );
		return S_FALSE;
	}
	if(!( _strLogFolder[_strLogFolder.length()-1]=='\\' || _strLogFolder[_strLogFolder.length()-1]=='/'))
		_strLogFolder+="\\";

	{
		_strPluginFolder = szProgramRootPath;
		_strPluginFolder += "modules\\";
	}

	if(_strPluginFolder.size() >0 && !( _strPluginFolder[_strPluginFolder.length()-1]=='\\' || _strPluginFolder[_strPluginFolder.length()-1]=='/'))
		_strPluginFolder+="\\";
		
#endif

	if(!InitIce())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPESvc, "failed to initialize ice property"));
		return false;
	}

	std::string	strDbPath ;
	strDbPath = _gCPECfg._szIceDbFolder;

	bool bEnv = true;
	try
	{	
#ifdef ZQ_OS_MSWIN
		_pCPEEnv = new CPEEnv(*m_pReporter, *_pCPEThreadPool, _ic, *_pCPETimerThreadPool);		
#else
		_pCPEEnv = new CPEEnv(*_svcLog, *_pCPEThreadPool, _ic, *_pCPETimerThreadPool);		
#endif
		_pCPEEnv->setMediaSampleBuffer(_gCPECfg._mediasamplebuffer._mediaSampleBufferSize, 
			_gCPECfg._mediasamplebuffer._maxBufferPoolSize,
			_gCPECfg._mediasamplebuffer._minBufferPoolSize);

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

		return S_FALSE;
	}

	_cps = new ZQTianShan::CPE::CPEImpl(*_pCPEEnv);
	MOLOG(Log::L_INFO, CLOGFMT(CPESvc, "ContentProvisionService created"));

    _gCPECfg.snmpRegister("");

	return BaseZQServiceApplication::OnInit();
}

HRESULT ContentProvisionEngineSvc::OnStart()
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
		return S_FALSE;
	}

	snmp_MothodTable(0);
	return BaseZQServiceApplication ::OnStart();	
}

HRESULT ContentProvisionEngineSvc::OnStop()
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

	_iceLogger = NULL;
	return BaseZQServiceApplication::OnStop();	
}

HRESULT ContentProvisionEngineSvc::OnUnInit()
{
	try
	{
		_ic->destroy();
	}
	catch (...)
	{
	}	

	try
	{
		if (_iceLogFile)
		{
			delete _iceLogFile;
			_iceLogFile= NULL;
		}
	}
	catch (...)
	{
		_iceLogFile= NULL;
	}

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
	
	ZQ::common::setGlogger(NULL);
	return BaseZQServiceApplication ::OnUnInit();	
}

void ContentProvisionEngineSvc::OnSnmpSet( const char* varName)
{
	if(0 == strcmp(varName, "default/IceTrace/level"))
	{
		if(_iceLogFile)
		{
			_iceLogFile->setVerbosity(_gCPECfg._iceLogLevel);
		}
	}

}
/*
const ZQ::SNMP::ModuleMIB::MIBE gTblMib_CPESVC[] = {
	{".2", "cpesvcApp" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).CPESVC(700).cpesvcApp(2)
	{".2.1", "cpesvcAttr" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).CPESVC(700).cpesvcApp(2).cpesvcAttr(1)
	{".2.1.100", "contentProvSessionTable" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).CPESVC(700).cpesvcApp(2).cpesvcAttr(1).contentProvSessionTable(100)
	{".2.1.100.1", "contentProvSessionEntry" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).CPESVC(700).cpesvcApp(2).cpesvcAttr(1).contentProvSessionTable(100).contentProvSessionEntry(1)
	{".2.1.100.1.1", "cpeMethod" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).CPESVC(700).cpesvcApp(2).cpesvcAttr(1).contentProvSessionTable(100).contentProvSessionEntry(1).cpeMethod(1)
	{".2.1.100.1.2", "cpeSessSubtotal" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).CPESVC(700).cpesvcApp(2).cpesvcAttr(1).contentProvSessionTable(100).contentProvSessionEntry(1).cpeSessSubtotal(2)
	{".2.1.100.1.3", "cpeBwSubtotal" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).CPESVC(700).cpesvcApp(2).cpesvcAttr(1).contentProvSessionTable(100).contentProvSessionEntry(1).cpeBwSubtotal(3)
	{".2.1.100.1.4", "cpeSessMax" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).CPESVC(700).cpesvcApp(2).cpesvcAttr(1).contentProvSessionTable(100).contentProvSessionEntry(1).cpeSessMax(4)
	{".2.1.100.1.5", "cpeBwMax" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).CPESVC(700).cpesvcApp(2).cpesvcAttr(1).contentProvSessionTable(100).contentProvSessionEntry(1).cpeBwMax(5)
	{NULL, NULL} };
*/
void ContentProvisionEngineSvc::doEnumSnmpExports()
{
	BaseZQServiceApplication::doEnumSnmpExports();
}
void ContentProvisionEngineSvc::snmp_MothodTable(const uint32& iDummy)
{
	if(NULL == _pCPEEnv)
		return;

	Oid subOidTable;
	_pServiceMib->reserveTable("contentProvSessionTable", 5, subOidTable);

	::TianShanIce::StrValues methods = _pCPEEnv->_provisionFactory->listSupportedMethods();

	int idxRow = 1;
	for (size_t sizeStep =0; sizeStep < methods.size(); sizeStep++)
	{
		try
		{
			::TianShanIce::ContentProvision::MethodInfo info;
			info.methodType = methods[sizeStep];

			uint32  allocatedKbps, maxKbps;
			uint sessions, maxsessions;

			ICPHelper* pHelper = _pCPEEnv->_provisionFactory->findHelper(info.methodType.c_str());
			if (NULL != pHelper && pHelper->getLoad(info.methodType.c_str(), allocatedKbps, maxKbps, sessions, maxsessions))
			{
				_pServiceMib->addTableCell(subOidTable, 1, idxRow, new SNMPObjectDupValue("cpeMethod", info.methodType));
				_pServiceMib->addTableCell(subOidTable, 2, idxRow, new SNMPObjectDupValue("cpeSessSubtotal",  (int32)sessions));
				_pServiceMib->addTableCell(subOidTable, 3, idxRow, new SNMPObjectDupValue("cpeBwSubtotal", (int32)allocatedKbps));
				_pServiceMib->addTableCell(subOidTable, 4, idxRow, new SNMPObjectDupValue("cpeSessMax", (int32)maxsessions));
				_pServiceMib->addTableCell(subOidTable, 5, idxRow, new SNMPObjectDupValue("cpeBwMax", (int)maxKbps));

				++idxRow;
			}																														 
		}
		catch (...) 
		{
		}
	}
}
}}
