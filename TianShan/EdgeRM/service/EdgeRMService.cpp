#include "EdgeRMService.h"
#include "EdgeRMCfgLoader.h"
#include "TianShanDefines.h"
#include "Guid.h"
#include "Log.h"
#include "ZQResource.h"
#include "FileLog.h"

using namespace ZQ::SNMP;

#ifdef ZQ_OS_MSWIN
#include "MiniDump.h"
#include "io.h"
#endif

#define ADAPTER_NAME_EdgeRM  "EdgeRM"

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;
#endif

extern int32 iTimeOut;

ZQTianShan::EdgeRM::EdgeRMSvc g_server;
ZQ::common::BaseZQServiceApplication *Application = &g_server;

char *DefaultConfigPath = "EdgeRMSvc.xml";

ZQ::common::Config::Loader< ::ZQTianShan::EdgeRM::EdgeRMCfgLoader > pConfig(DefaultConfigPath);

#ifdef ZQ_OS_MSWIN
ZQ::common::MiniDump			_crashDump;
extern bool validatePath(const char* szPath );
#else
extern const char* DUMP_PATH;
#endif

ZQ::common::Config::ILoader		*configLoader=&pConfig;

#define EdgeRMService	"EdgeRMService"

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
	proper->setProperty("Ice.ThreadPool.Server.Size","5");
	proper->setProperty("Ice.ThreadPool.Server.SizeMax","10");

	for (std::map<std::string, std::string>::const_iterator iceIter = pConfig.iceProperties.begin();
		iceIter != pConfig.iceProperties.end(); iceIter++)
	{
		proper->setProperty((*iceIter).first.c_str(), (*iceIter).second.c_str());
		glog(::ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "Set ICE Property %s=%s"), (*iceIter).first.c_str(), (*iceIter).second.c_str());			
	}

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace ZQTianShan{

namespace EdgeRM{

EdgeRMSvc::EdgeRMSvc()
{
//	strcpy(servname, "EdgeRMSvc");
//	strcpy(prodname, "TianShan");
	_edgeRMEnv = NULL;
	_ic = NULL;
	_adapter = NULL;
}

EdgeRMSvc::~EdgeRMSvc()
{

}

HRESULT EdgeRMSvc::OnInit()
{
	BaseZQServiceApplication::OnInit();

	int32 iLogLevel = ZQ::common::Log::L_DEBUG;
#ifdef ZQ_OS_MSWIN
	_strLogFolder = m_wsLogFolder;
	iLogLevel = m_dwLogLevel;
#else
	_strLogFolder = _logDir;
	iLogLevel = _logLevel;
#endif

#ifdef ZQ_OS_MSWIN
	if(_access(_strLogFolder.c_str(), 0))
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMSvc, "Invalid logFolder %s"),_strLogFolder.c_str() );
		logEvent(ZQ::common::Log::L_ERROR,_T("Invalid logFolder %s"),_strLogFolder.c_str() );
		return S_FALSE;
	}

	m_strProgramRootPath = ZQTianShan::getProgramRoot();
	m_strProgramRootPath += FNSEPS;

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMSvc, "program root path [%s]"), m_strProgramRootPath.c_str());
	if(_access(m_strProgramRootPath.c_str(), 0))
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMSvc, "failed to access program root path [%s]"), m_strProgramRootPath.c_str());
		return S_FALSE;
	}

	std::string	strCrashDumpPath;
	if (strstr(pConfig.crashdumpPath.c_str(),":")!=NULL) 
	{
		strCrashDumpPath = pConfig.crashdumpPath;
	}
	else
	{
		strCrashDumpPath = m_strProgramRootPath;
		strCrashDumpPath += pConfig.crashdumpPath;		
	}

	if (strCrashDumpPath[strCrashDumpPath.size() - 1] != '\\' && strCrashDumpPath[strCrashDumpPath.size() - 1] != '/')
		strCrashDumpPath += '\\';

	if(_access(strCrashDumpPath.c_str(), 0))
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(EdgeRMSvc, "failed to access dumpPath path [%s]"), m_strProgramRootPath.c_str());
		return S_FALSE;
	}
	if(!_crashDump.setDumpPath((char*)strCrashDumpPath.c_str()))
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMSvc, "failed to access crash dump path '%s'"), strCrashDumpPath.c_str());
		logEvent(ZQ::common::Log::L_ERROR, "invalid mini-dump path %s",strCrashDumpPath.c_str());
		return false;
	}
	_crashDump.enableFullMemoryDump(true);
	_crashDump.setExceptionCB(CrashExceptionCallBack);

	std::string strIceLog		 = _strLogFolder + FNSEPS + servname + "_ice.log";
	std::string strERMEventLog = _strLogFolder + FNSEPS + servname + "_Event.log";

#else
	DUMP_PATH = pConfig.crashdumpPath.c_str();

	std::string strIceLog		 = _strLogFolder + FNSEPS + getServiceName() + std::string("_ice.log");
	std::string strERMEventLog = _strLogFolder + FNSEPS + getServiceName() + std::string("_Event.log");
#endif

	glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "OnInit() enter"));

	std::string strRtspEngineLog = _strLogFolder + FNSEPS + pConfig.logfilename;
	//ERM ice log
	_iceFileLog.open(strIceLog.c_str(), pConfig.iceTraceLevel, pConfig.iceTraceCount, pConfig.iceTraceSize, pConfig.iceTraceBuffersize, pConfig.iceTraceFlashtimeout);
	//ERM event log
	_eventFileLog.open(strERMEventLog.c_str(), iLogLevel);
	//ERM rtspEngine log
	_RtspEngineLog.open(strRtspEngineLog.c_str(), pConfig.lLogLevel, pConfig.llogFileCount, pConfig.lLogFileSize,
						pConfig.lLogBufferSize, pConfig.lLogWriteTimteout);

	int i=0;
	_iceLog = new ::TianShanIce::common::IceLogI(&_iceFileLog);
	if(!_iceLog)
	{
      return S_FALSE;
	}

#if ICE_INT_VERSION / 100 >= 303

	::Ice::InitializationData initData;
	initData.properties = Ice::createProperties(i, NULL);
	initWithConfig(initData.properties);

	initData.logger = _iceLog;

	_ic = Ice::initialize(initData);

#else
	Ice::PropertiesPtr proper = Ice::createProperties(i,NULL);
	initWithConfig(proper);	

	_ic = Ice::initializeWithPropertiesAndLogger(i,NULL,proper, _iceLog);
#endif // ICE_INT_VERSION

	std::string	strDbPath ;
	strDbPath = pConfig.databasePath;

	ZQADAPTER_DECLTYPE adapter;

	std::string strEndpoint;

	try
	{
		if (pConfig.edgeRMEndpoint.empty())
			strEndpoint = DEFAULT_ENDPOINT(EdgeRM);
		else
			strEndpoint = pConfig.edgeRMEndpoint;

		glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "open adapter %s at %s"), ADAPTER_NAME_EdgeRM, strEndpoint.c_str());

		_adapter = ZQADAPTER_CREATE(_ic, ADAPTER_NAME_EdgeRM, strEndpoint.c_str(), glog);
        
		if(!_adapter)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMSvc, "failed to open adapter %s at %s"), ADAPTER_NAME_EdgeRM, strEndpoint.c_str());
			return S_FALSE;
		}
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMSvc, "Create adapter failed with endpoint=%s and exception is %s"), strEndpoint.c_str(), ex.ice_name().c_str());
		return S_FALSE;
	}
   
	if(pConfig.dispatchSize < 30)
		pConfig.dispatchSize = 30;
	_RequestThreadPool.resize(pConfig.dispatchSize);

	// create EdgeRMEnv object
	_edgeRMEnv = new ::ZQTianShan::EdgeRM::EdgeRMEnv(glog, _eventFileLog, _RequestThreadPool, _adapter, strDbPath, _RtspEngineLog);
	if(!_edgeRMEnv)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMSvc, "failed to create EdgeRM object"));
		return S_FALSE;
	}

	//publish event log
	for (std::vector<MonitoredLog>::iterator iter = pConfig.monitoredLogs.begin(); iter != pConfig.monitoredLogs.end(); iter++)
	{
		if (!_adapter->publishLogger(iter->name.c_str(), iter->syntax.c_str(), iter->syntaxKey.c_str()))	
		{				
			glog(::ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMSvc, "Failed to publish logger name[%s] syntax[%s] key[%s]"), 			
				iter->name.c_str(), iter->syntax.c_str(), iter->syntaxKey.c_str());				
		}			
		else				
		{				
			glog(::ZQ::common::Log::L_INFO,CLOGFMT(EdgeRMSvc, "Publish logger name[%s] syntax[%s] key[%s] successful"), 			
				iter->name.c_str(), iter->syntax.c_str(), iter->syntaxKey.c_str());				
		}
	}

	glog(::ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "Edge Resouce Management Service created"));

	pConfig.snmpRegister(std::string(""));

	glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "OnInit() leave"));
	return S_OK;
}
HRESULT EdgeRMSvc::OnStart()
{	
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "OnStart() enter"));

	BaseZQServiceApplication::OnStart();

	if(!_edgeRMEnv->initialize())
	{	
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(EdgeRMSvc, "No enviroment is setuped and adapter fail to activate"));
		return S_FALSE;	  
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "OnStart() leave"));
	return S_OK;	
}
HRESULT EdgeRMSvc::OnStop()
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "OnStop() enter"));

	if(_edgeRMEnv)
	{
		try
		{
			delete _edgeRMEnv;
			_edgeRMEnv = NULL;
		}
		catch(...){ }		
	}

	BaseZQServiceApplication::OnStop();

	glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "OnStop() leave"));

	return S_OK;
}

HRESULT EdgeRMSvc::OnUnInit()
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "OnUnInit() enter"));
	try
	{
		if(_adapter)
			_adapter->deactivate();
	}
	catch (...)
	{
	}
	try
	{
		if(_ic)
			_ic->destroy();
	}
	catch (...)
	{
	}
	_ic = NULL;
	_adapter = NULL;

	BaseZQServiceApplication::OnUnInit();

	glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "OnUnInit() leave"));

	return S_OK;		
}
/*
const ZQ::SNMP::ModuleMIB::MIBE gTblMib_EdgeRM[] = {
	{".2", "EdgeRMApp" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2)
	{".2.2", "EdgeRMAttr" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2)
	{".2.2.200", "ErmDevices" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmDevices(200)
	{".2.2.200.1", "devicesTable" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmDevices(200).devicesTable(1)
	{".2.2.200.1.1", "devicesEntry" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmDevices(200).devicesTable(1).devicesEntry(1)
	{".2.2.200.1.1.1", "edZone" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmDevices(200).devicesTable(1).devicesEntry(1).edZone(1)
	{".2.2.200.1.1.2", "edDeviceName" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmDevices(200).devicesTable(1).devicesEntry(1).edDeviceName(2)
	{".2.2.200.1.1.3", "edVendor" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmDevices(200).devicesTable(1).devicesEntry(1).edVendor(3)
	{".2.2.200.1.1.4", "edModel" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmDevices(200).devicesTable(1).devicesEntry(1).edModel(4)
	{".2.2.200.1.1.5", "edDescription" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmDevices(200).devicesTable(1).devicesEntry(1).edDescription(5)
	{".2.2.200.1.1.6", "edTftp" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmDevices(200).devicesTable(1).devicesEntry(1).edTftp(6)
	{".2.2.200.1.1.7", "edAdminUrl" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmDevices(200).devicesTable(1).devicesEntry(1).edAdminUrl(7)
	{".2.2.201", "ErmPorts" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmPorts(201)
	{".2.2.201.1", "portsTable" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmPorts(201).portsTable(1)
	{".2.2.201.1.1", "portsEntry" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmPorts(201).portsTable(1).portsEntry(1)
	{".2.2.201.1.1.1", "epPortID" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmPorts(201).portsTable(1).portsEntry(1).epPortID(1)
	{".2.2.201.1.1.2", "epPowerLevel" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmPorts(201).portsTable(1).portsEntry(1).epPowerLevel(2)
	{".2.2.201.1.1.3", "epModulationFormat" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmPorts(201).portsTable(1).portsEntry(1).epModulationFormat(3)
	{".2.2.201.1.1.4", "epInterLeaverMode" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmPorts(201).portsTable(1).portsEntry(1).epInterLeaverMode(4)
	{".2.2.201.1.1.5", "epFEC" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmPorts(201).portsTable(1).portsEntry(1).epFEC(5)
	{".2.2.201.1.1.6", "epDeviceIP" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmPorts(201).portsTable(1).portsEntry(1).epDeviceIP(6)
	{".2.2.201.1.1.7", "epDeviceGroup" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmPorts(201).portsTable(1).portsEntry(1).epDeviceGroup(7)
	{".2.2.202", "ErmChannels" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202)
	{".2.2.202.1", "channelsTable" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1)
	{".2.2.202.1.1", "channelsEntry" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1)
	{".2.2.202.1.1.1", "ecName" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecName(1)
	{".2.2.202.1.1.10", "ecLowBandwidthUtilization" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecLowBandwidthUtilization(10)
	{".2.2.202.1.1.11", "ecHighBandwidthUtilization" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecHighBandwidthUtilization(11)
	{".2.2.202.1.1.12", "ecMaxSessions" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecMaxSessions(12)
	{".2.2.202.1.1.13", "ecIntervalPAT" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecIntervalPAT(13)
	{".2.2.202.1.1.14", "ecIntervalPMT" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecIntervalPMT(14)
	{".2.2.202.1.1.15", "ecSymbolRate" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecSymbolRate(15)
	{".2.2.202.1.1.16", "ecAllocations" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecAllocations(16)
	{".2.2.202.1.1.2", "ecStampLastUpdated" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecStampLastUpdated(2)
	{".2.2.202.1.1.3", "ecEnabled" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecEnabled(3)
	{".2.2.202.1.1.4", "ecFreqRF" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecFreqRF(4)
	{".2.2.202.1.1.5", "ecTSID" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecTSID(5)
	{".2.2.202.1.1.6", "ecNITPID" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecNITPID(6)
	{".2.2.202.1.1.7", "ecStartUDPPort" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecStartUDPPort(7)
	{".2.2.202.1.1.8", "ecUdpPortStepByPn" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecUdpPortStepByPn(8)
	{".2.2.202.1.1.9", "ecStartProgramNumber" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).EdgeRM(2300).EdgeRMApp(2).EdgeRMAttr(2).ErmChannels(202).channelsTable(1).channelsEntry(1).ecStartProgramNumber(9)
	{NULL, NULL} };
	const ZQ::SNMP::ModuleMIB::MIBE* gSvcMib_EdgeRM = gTblMib_EdgeRM; // to export
*/

void EdgeRMSvc::doEnumSnmpExports()
{
	BaseZQServiceApplication::doEnumSnmpExports();
	_pServiceMib->addObject(new ZQ::SNMP::SNMPObjectByAPI<EdgeRMSvc, uint32>("ErmDevices", *this, ZQ::SNMP::AsnType_Int32, &EdgeRMSvc::snmp_dummyGet, &EdgeRMSvc::snmp_refreshDevices));
	_pServiceMib->addObject(new ZQ::SNMP::SNMPObjectByAPI<EdgeRMSvc, uint32>("ErmPorts", *this, ZQ::SNMP::AsnType_Int32, &EdgeRMSvc::snmp_dummyGet, &EdgeRMSvc::snmp_refreshErmPorts));
	_pServiceMib->addObject(new ZQ::SNMP::SNMPObjectByAPI<EdgeRMSvc, uint32>("ErmChannels", *this, ZQ::SNMP::AsnType_Int32, &EdgeRMSvc::snmp_dummyGet, &EdgeRMSvc::snmp_refreshErmChannels));
}

bool EdgeRMSvc::isHealth(void)
{
	static int64 stampLastRefreshCacheStat = 0;
	int64 stampNow = ZQ::common::now();
	if (stampNow - stampLastRefreshCacheStat > 30000)
	{
		static int iStep =0;
		switch (iStep++)
		{
		case 0: 
			snmp_refreshDevices(0);
			break;
			
//TOO many rows:
		//case 1: 
		//	snmp_refreshErmPorts(0);
		//	break;

//TOO many rows:
//  		case 2: 
//			snmp_refreshErmChannels(0);
//			break;

		default:
			iStep =0;
			stampLastRefreshCacheStat = stampNow;
		}
	}

	return BaseZQServiceApplication::isHealth();
}

void  EdgeRMSvc::snmp_refreshDevices(const uint32&)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "snmp_refreshDevices() enter"));
	static ZQ::SNMP::Oid subOidTbl;
	if (subOidTbl.isNil())
		_pServiceMib->reserveTable("devicesTable", 7, subOidTbl);
	if (subOidTbl.isNil())
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMSvc,"snmp_refreshDevices() failed to locate devices Table in MIB"));
		return;
	}

	// step 1. clean up the table content
	ZQ::SNMP::Oid tmpOid(subOidTbl);
	tmpOid.append(1);
	_pServiceMib->removeSubtree(tmpOid);

	int idxRow = 1;
	TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
	TianShanIce::StrValues expectedMetaData;
	try
	{
		ZQTianShan::EdgeRM::EdgeRMImpl::Ptr edgeRmPtr = _edgeRMEnv->getEdgeRmPtr();
		TianShanIce::EdgeResource::EdgeRM*  edgeRmP = edgeRmPtr.get();

		if (!edgeRmPtr)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "EdgeRM create ErmDevicesTable Table end, edgeRmPtr[empty] row[%d]"), idxRow);
			return ;
		}
		expectedMetaData.push_back(SYS_PROP(Zone));
		expectedMetaData.push_back(SYS_PROP(Vendor));
		expectedMetaData.push_back(SYS_PROP(Model));
		expectedMetaData.push_back(SYS_PROP(Desc));
		expectedMetaData.push_back(SYS_PROP(Tftp));
		expectedMetaData.push_back(SYS_PROP(AdminUrl));
		deviceInfos = edgeRmP->listDevices(expectedMetaData, Ice::Current());
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "snmp_refreshDevices() failed to list devices info caught ice exception[%s]"),ex.ice_name().c_str());
		return;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "snmp_refreshDevices() failed to list devices info caught unkonwn exception"));
		return;
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "snmp_refreshDevices() devices count[%d]"), deviceInfos.size());

	for(TianShanIce::EdgeResource::EdgeDeviceInfos::iterator it = deviceInfos.begin(); it != deviceInfos.end(); ++it, ++idxRow)
	{
		TianShanIce::ObjectInfo& deviceInfo = *it;
		TianShanIce::Properties& props = deviceInfo.props;
		TianShanIce::Properties::iterator itTemp;

#define GET_STR_FROM_MAP_BY_INDEX(itTemp,mapSrc,index) \
	(itTemp = mapSrc.find(index), (itTemp != mapSrc.end()) ? itTemp->second : "" )

		std::string zone        = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(Zone));
		std::string deviceName  = deviceInfo.ident.name;
		std::string vendor      = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(Vendor));
		std::string model       = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(Model));
		std::string description = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(Desc));
		std::string tftp        = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(Tftp));
		std::string adminUrl    = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(AdminUrl));

		_pServiceMib->addTableCell(subOidTbl,  1, idxRow, new ZQ::SNMP::SNMPObjectDupValue("edZone", zone));
		_pServiceMib->addTableCell(subOidTbl,  2, idxRow, new ZQ::SNMP::SNMPObjectDupValue("edDeviceName",deviceName));
		_pServiceMib->addTableCell(subOidTbl,  3, idxRow, new ZQ::SNMP::SNMPObjectDupValue("edVendor", vendor));
		_pServiceMib->addTableCell(subOidTbl,  4, idxRow, new ZQ::SNMP::SNMPObjectDupValue("edModel",model));
		_pServiceMib->addTableCell(subOidTbl,  5, idxRow, new ZQ::SNMP::SNMPObjectDupValue("edDescription", description));
		_pServiceMib->addTableCell(subOidTbl,  6, idxRow, new ZQ::SNMP::SNMPObjectDupValue("edTftp", tftp));
		_pServiceMib->addTableCell(subOidTbl,  7, idxRow, new ZQ::SNMP::SNMPObjectDupValue("edAdminUrl",adminUrl));
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "snmp_refreshDevices() leave"));
}

int regexInt(TianShanIce::ValueMap& resAtscData, std::string target)
{
	typedef TianShanIce::ValueMap::iterator ItType;
	int destInt = -1;
	ItType itor = resAtscData.find(target);
	if (itor != resAtscData.end())
	{
		const TianShanIce::Variant& destVar = itor->second;
		if (TianShanIce::vtBin == destVar.type && destVar.bin.size() > 0)
			destInt = destVar.bin[0];
	}

	return destInt;
}
std::string regexStr(TianShanIce::ValueMap& resPhyChData, std::string target)
{
	typedef TianShanIce::ValueMap::iterator ItType;
	std::string destStr;
	ItType itor = resPhyChData.find(target);

	if (itor != resPhyChData.end())
	{
		const TianShanIce::Variant& destVar = itor->second;
		if (TianShanIce::vtStrings == destVar.type && destVar.strs.size() > 0)
			destStr = destVar.strs[0];
	}

	return destStr;
}
void  EdgeRMSvc::snmp_refreshErmPorts(const uint32&)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "snmp_refreshErmPorts() enter"));
	static ZQ::SNMP::Oid subOidTb2;
	if (subOidTb2.isNil())
		_pServiceMib->reserveTable("portsTable", 7, subOidTb2);
	if (subOidTb2.isNil())
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMSvc,"snmp_refreshDevices() failed to locate devices Table in MIB"));
		return;
	}

	// step 1. clean up the table content
	ZQ::SNMP::Oid tmpOid(subOidTb2);
	tmpOid.append(1);
	_pServiceMib->removeSubtree(tmpOid);

	try
	{
		int idxRow = 1;
		ZQTianShan::EdgeRM::EdgeRMImpl::Ptr edgeRmPtr =  _edgeRMEnv->getEdgeRmPtr();
		TianShanIce::EdgeResource::EdgeRM*  edgeRmP   = edgeRmPtr.get();

		TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
		TianShanIce::StrValues expectedMetaData;

		deviceInfos = edgeRmP->listDevices(expectedMetaData, Ice::Current());
		glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "snmp_refreshErmPorts() devices count[%d]"), deviceInfos.size());
		for (TianShanIce::EdgeResource::EdgeDeviceInfos::iterator it = deviceInfos.begin(); it != deviceInfos.end(); ++it)
		{
			TianShanIce::ObjectInfo& deviceInfo = *it;
			TianShanIce::EdgeResource::EdgePortInfos  portInfos;
			TianShanIce::EdgeResource::EdgeDevicePrx  devicePrx;
			try
			{
				devicePrx = edgeRmP->openDevice(deviceInfo.ident.name);
				portInfos = devicePrx->listEdgePorts();
			}
			catch (const TianShanIce::BaseException& ex)
			{
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMSvc, "ErmRM create ErmPortsTable exception[TianShanIce::BaseExceptionn %s], device[%s], row[%d]"), 
					ex.ice_name().c_str(), idxRow, deviceInfo.ident.name.c_str());
				continue;
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMSvc, "ErmRM create ErmPortsTable exception[Ice::Exception %s], device[%s], row[%d]"), 
					ex.ice_name().c_str(), idxRow, deviceInfo.ident.name.c_str());
				continue;
			}
			catch (...) 
			{
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMSvc, "ErmRM create ErmPortsTable exception[unknown], device[%s], row[%d]"), 
					idxRow, deviceInfo.ident.name.c_str());
				continue;
			}

			glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "snmp_refreshErmPorts() devices[%s] port count[%d]"), deviceInfo.ident.name.c_str(), portInfos.size());

			for (TianShanIce::EdgeResource::EdgePortInfos::iterator itPI = portInfos.begin(); itPI != portInfos.end(); ++itPI, ++idxRow)
			{
				TianShanIce::ValueMap& resAtscData  = itPI->resAtscModulationMode.resourceData;
				TianShanIce::ValueMap& resPhyChData = itPI->resPhysicalChannel.resourceData;
				int portID           = itPI->Id;
				int powerLevel       = itPI->powerLevel;
				int modulationFromat = regexInt(resAtscData, std::string("modulationFormat"));
				int interLeaverMode  = regexInt(resAtscData, std::string("interleaveDepth"));
				int FEC              = regexInt(resAtscData, std::string("FEC"));
				std::string deviceIP    = regexStr(resPhyChData, std::string("edgeDeviceIP"));
				std::string deviceGroup = regexStr(resPhyChData, std::string("edgeDeviceZone"));

				_pServiceMib->addTableCell(subOidTb2,  1, idxRow, new ZQ::SNMP::SNMPObjectDupValue("epPortID", (int32)portID));
				_pServiceMib->addTableCell(subOidTb2,  2, idxRow, new ZQ::SNMP::SNMPObjectDupValue("epPowerLevel",(int32)powerLevel));
				_pServiceMib->addTableCell(subOidTb2,  3, idxRow, new ZQ::SNMP::SNMPObjectDupValue("epModulationFormat", (int32)modulationFromat));
				_pServiceMib->addTableCell(subOidTb2,  4, idxRow, new ZQ::SNMP::SNMPObjectDupValue("epInterLeaverMode",(int32)interLeaverMode));
				_pServiceMib->addTableCell(subOidTb2,  5, idxRow, new ZQ::SNMP::SNMPObjectDupValue("epFEC", (int32)FEC));
				_pServiceMib->addTableCell(subOidTb2,  6, idxRow, new ZQ::SNMP::SNMPObjectDupValue("epDeviceIP", deviceIP));
				_pServiceMib->addTableCell(subOidTb2,  7, idxRow, new ZQ::SNMP::SNMPObjectDupValue("epDeviceGroup",deviceGroup));
			}
		}
	}
	catch (Ice::Exception& ex)
	{
		return;
	}
	catch (...)
	{
		return;
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "snmp_refreshErmPorts() leave"));
}
bool EdgeRMSvc::getEdgeChannelInfosList(EdgeChannelInfosList& channelInfosList, ZQ::common::Log& reporter)//all EdgeChannelInfos store in list
{
	int nRev = false;
	ZQTianShan::EdgeRM::EdgeRMImpl::Ptr edgeRmPtr =  _edgeRMEnv->getEdgeRmPtr();
	TianShanIce::EdgeResource::EdgeRM*  edgeRmP   = edgeRmPtr.get();

	if (!edgeRmPtr)
	{
		(reporter)(ZQ::common::Log::L_INFO, CLOGFMT(EdgeRMSvc, "EdgeRM getEdgeChannelInfosList end, edgeRmPtr[empty]"));
		return nRev;
	}

	TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
	TianShanIce::StrValues expectedMetaData;

	deviceInfos = edgeRmP->listDevices(expectedMetaData, Ice::Current());
	for (TianShanIce::EdgeResource::EdgeDeviceInfos::iterator it = deviceInfos.begin(); it != deviceInfos.end(); ++it)
	{
		TianShanIce::ObjectInfo& deviceInfo = *it;
		Ice::ObjectPrx  objPrx = NULL;
		TianShanIce::EdgeResource::EdgeDevicePrx    devicePrx;
		TianShanIce::EdgeResource::EdgePortInfos    portInfos;

		try
		{ 
			devicePrx = edgeRmP->openDevice(deviceInfo.ident.name);
			#if  ICE_INT_VERSION / 100 >= 306
				objPrx    = devicePrx->ice_collocationOptimized(false);
			#else
				objPrx    = devicePrx->ice_collocationOptimization(false);
			#endif
			devicePrx = TianShanIce::EdgeResource::EdgeDevicePrx::uncheckedCast(objPrx);
			portInfos = devicePrx->listEdgePorts();
		}
		catch (const TianShanIce::BaseException& ex)
		{
			(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMSvc, "ErmRM getEdgeChannelInfosList ErmPortsTable exception[TianShanIce::BaseExceptionn %s], device[%s]"), 
				ex.ice_name().c_str(), deviceInfo.ident.name.c_str());
			continue;//deviceInfos
		}
		catch (const Ice::Exception& ex)
		{
			(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMSvc, "ErmRM getEdgeChannelInfosList ErmPortsTable exception[Ice::Exception %s], device[%s]"), 
				ex.ice_name().c_str(), deviceInfo.ident.name.c_str());
			continue;//deviceInfos
		}
		catch (...) 
		{
			(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMSvc, "ErmRM getEdgeChannelInfosList ErmPortsTable exception[unknown], device[%s]"), 
				deviceInfo.ident.name.c_str());
			continue;//deviceInfos
		}

		for (TianShanIce::EdgeResource::EdgePortInfos::iterator itPI = portInfos.begin(); itPI != portInfos.end(); ++itPI)
		{
			try
			{ 
				TianShanIce::StrValues expectedMetaData;
				TianShanIce::EdgeResource::EdgeChannelInfos channelInfos;

				expectedMetaData.push_back(SYS_PROP(FreqRF));
				expectedMetaData.push_back(SYS_PROP(symbolRate));
				expectedMetaData.push_back(SYS_PROP(TSID));
				expectedMetaData.push_back(SYS_PROP(IntervalPAT));
				expectedMetaData.push_back(SYS_PROP(IntervalPMT));
				expectedMetaData.push_back(SYS_PROP(StampLastUpdated));
				expectedMetaData.push_back(SYS_PROP(NITPID));
				expectedMetaData.push_back(SYS_PROP(StartUDPPort));
				expectedMetaData.push_back(SYS_PROP(UdpPortStepByPn));
				expectedMetaData.push_back(SYS_PROP(StartProgramNumber));
				expectedMetaData.push_back(SYS_PROP(MaxSessions));
				expectedMetaData.push_back(SYS_PROP(LowBandwidthUtilization));
				expectedMetaData.push_back(SYS_PROP(HighBandwidthUtilization));
				expectedMetaData.push_back(SYS_PROP(Enabled));
				expectedMetaData.push_back(SYS_PROP(AllocationCount));

				channelInfos = devicePrx->listChannels(itPI->Id, expectedMetaData, false);
				channelInfosList.push_back(channelInfos);
			}
			catch (const TianShanIce::BaseException& ex)
			{
				(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMSvc, "ErmRM listChannels ErmPortsTable exception[TianShanIce::BaseExceptionn %s], device[%s], portID[%d]"), 
					ex.ice_name().c_str(), deviceInfo.ident.name.c_str(), itPI->Id);
				continue;//portInfos
			}
			catch (const Ice::Exception& ex)
			{
				(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMSvc, "ErmRM listChannels ErmPortsTable exception[Ice::Exception %s], device[%s], portID[%d]"), 
					ex.ice_name().c_str(), deviceInfo.ident.name.c_str(),itPI->Id);
				continue;//portInfos
			}
			catch (...) 
			{
				(reporter)(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMSvc, "ErmRM listChannels ErmPortsTable exception[unknown], device[%s], portID[%d]"), 
					deviceInfo.ident.name.c_str(), itPI->Id);
				continue;//portInfos
			}
		}// end loop portInfos			

		nRev = true;
	}//end loop deviceInfos

	return nRev;
}
void  EdgeRMSvc::snmp_refreshErmChannels(const uint32&)
{
	static ZQ::SNMP::Oid subOidTb3;
	if (subOidTb3.isNil())
		_pServiceMib->reserveTable("devicesTable", 16, subOidTb3);
	if (subOidTb3.isNil())
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(EdgeRMSvc,"snmp_refreshDevices() failed to locate devices Table in MIB"));
		return;
	}

	// step 1. clean up the table content
	ZQ::SNMP::Oid tmpOid(subOidTb3);
	tmpOid.append(1);
	_pServiceMib->removeSubtree(tmpOid);


	int idxRow = 1;
	EdgeChannelInfosList channelInfosList;
	getEdgeChannelInfosList(channelInfosList, glog);
	for (EdgeChannelInfosList::iterator itList = channelInfosList.begin(); itList != channelInfosList.end(); ++itList)
	{
		TianShanIce::EdgeResource::EdgeChannelInfos& channelInfos = *itList;
		for (TianShanIce::EdgeResource::EdgeChannelInfos::iterator itCI = channelInfos.begin(); itCI != channelInfos.end(); ++itCI, ++idxRow)
		{

			TianShanIce::Properties& props = itCI->props;
			TianShanIce::Properties::iterator itTemp;

#define GET_STR_FROM_MAP_BY_INDEX(itTemp,mapSrc,index) \
	(itTemp = mapSrc.find(index), (itTemp != mapSrc.end()) ? itTemp->second : "" )

			std::string name = itCI->ident.name;
			std::string stampLastUpdated = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(StampLastUpdated));
			std::string enabled = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(Enabled));
			std::string freqRF  = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(FreqRF));
			std::string tsid    = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(TSID));
			std::string nitPid  = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(NITPID));
			std::string startUdpPort    = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(StartUDPPort));
			std::string udpPortStepByPN = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(UdpPortStepByPn));
			std::string startProgramNumber       = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(StartProgramNumber));
			std::string lowBandwidthUtilization  = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(LowBandwidthUtilization));
			std::string highBandwidthUtilization = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(HighBandwidthUtilization));
			std::string maxSessions = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(MaxSessions));
			std::string intervalPAT = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(IntervalPAT));
			std::string intervalPMT = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(IntervalPMT));
			std::string symbolRate  = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(symbolRate));
			std::string allocation  = GET_STR_FROM_MAP_BY_INDEX(itTemp,props,SYS_PROP(AllocationCount));
			char timeBuffer[128]    = {0};
			ZQ::common::TimeUtil::TimeToUTC(_atoi64(stampLastUpdated.c_str()),timeBuffer,sizeof(timeBuffer),true);
			std::string stampLastUpdatedConvert(timeBuffer);

			_pServiceMib->addTableCell(subOidTb3,  1, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecName", name));
			_pServiceMib->addTableCell(subOidTb3,  2, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecStampLastUpdated",stampLastUpdatedConvert));
			_pServiceMib->addTableCell(subOidTb3,  3, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecEnabled", enabled));
			_pServiceMib->addTableCell(subOidTb3,  4, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecFreqRF",freqRF));
			_pServiceMib->addTableCell(subOidTb3,  5, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecTSID", tsid));
			_pServiceMib->addTableCell(subOidTb3,  6, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecNITPID", nitPid));
			_pServiceMib->addTableCell(subOidTb3,  7, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecStartUDPPort",startUdpPort));
			_pServiceMib->addTableCell(subOidTb3,  8, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecUdpPortStepByPn",udpPortStepByPN));
			_pServiceMib->addTableCell(subOidTb3,  9, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecStartProgramNumber",startProgramNumber));
			_pServiceMib->addTableCell(subOidTb3,  10, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecLowBandwidthUtilization", lowBandwidthUtilization));
			_pServiceMib->addTableCell(subOidTb3,  11, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecHighBandwidthUtilization",highBandwidthUtilization ));
			_pServiceMib->addTableCell(subOidTb3,  12, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecMaxSessions",maxSessions));
			_pServiceMib->addTableCell(subOidTb3,  13, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecIntervalPAT", intervalPAT));
			_pServiceMib->addTableCell(subOidTb3,  14, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecIntervalPMT",intervalPMT));
			_pServiceMib->addTableCell(subOidTb3,  15, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecSymbolRate", symbolRate));
			_pServiceMib->addTableCell(subOidTb3,  16, idxRow, new ZQ::SNMP::SNMPObjectDupValue("ecAllocations", allocation));
		}
	}
}
}//namespace EdgeRM
}//namespace ZQTianShan
