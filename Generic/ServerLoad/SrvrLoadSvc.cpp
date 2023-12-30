#include "./SrvrLoadSvc.h"
#include "./ZQResource.h"

DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

SrvrLoad::SrvrLoadSvc g_SrvrLoadSvc;
ZQ::common::BaseZQServiceApplication *Application = &g_SrvrLoadSvc;

namespace ZQ{
	namespace common
	{
		ZQ::common::Log NullLogger;
		ZQ::common::Log* pGlog = &NullLogger;
	}
}

namespace SrvrLoad
{

SrvrLoadSvc::SrvrLoadSvc()
{
	strcpy(servname, "ServerLoad");
	strcpy(prodname, "TianShan");
}

SrvrLoadSvc::~SrvrLoadSvc()
{
}

HRESULT SrvrLoadSvc::OnInit()
{
	// init global log
	ZQ::common::pGlog = m_pReporter;
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(SrvrLoadSvc, "SrvrLoadSvc::OnInit(), version(%s)"), ZQ_PRODUCT_NAME);

	// call the version of base class;
	BaseZQServiceApplication::OnInit();

	// to gain the configuration file path
	char configPath[260];
	DWORD szConfigPath = sizeof(configPath);
	memset(configPath, 0, szConfigPath);
	TCHAR subStr[260];
	memset(subStr, 0, sizeof(subStr));
	snprintf(subStr, sizeof(subStr) - 1, "SOFTWARE\\ZQ Interactive\\%s\\CurrentVersion\\Services\\%s", prodname, servname);
	HKEY hKey;
	if (ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, subStr, 0, KEY_ALL_ACCESS, &hKey))
	{
		glog(ZQ::common::Log::L_EMERG, CLOGFMT(SrvrLoadSvc, "RegOpenKeyEx(%s) failed"), subStr);
		return S_FALSE;
	}
	DWORD dwType = REG_SZ;
	if (ERROR_SUCCESS != ::RegQueryValueEx(hKey, _T("configDir"), NULL, &dwType, (unsigned char*)configPath, &szConfigPath))
	{
		glog(ZQ::common::Log::L_EMERG, CLOGFMT(SrvrLoadSvc, "RegQueryValueEx(configDir) failed"));
		return S_FALSE;
	}
	::RegCloseKey(hKey);
	std::string strConfig = NULL != configPath ? configPath : "";
	strConfig += "\\ServerLoad.xml";

	// call enviroment doInit() to do most important work;
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(SrvrLoadSvc, "ServerLoad configuration path gained: %s"), strConfig.c_str());
	HRESULT hResult = _env.doInit(strConfig);

	return hResult;
	
}

HRESULT SrvrLoadSvc::OnStart()
{
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(SrvrLoadSvc, "SrvrLoadSvc::OnStart()"));
	HRESULT hResult = BaseZQServiceApplication::OnStart();
	_env.start();
	return hResult;
}

HRESULT SrvrLoadSvc::OnStop()
{
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(SrvrLoadSvc, "SrvrLoadSvc::OnStop()"));
	HRESULT hResult = BaseZQServiceApplication::OnStop();
	_env.doStop();
	return hResult;
}

HRESULT SrvrLoadSvc::OnUnInit()
{
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(SrvrLoadSvc, "SrvrLoadSvc::OnUnInit()"));
	_env.doUninit();
	return BaseZQServiceApplication::OnUnInit();
}

}

