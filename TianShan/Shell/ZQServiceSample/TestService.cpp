// TestService.cpp: implementation of the TestService class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestService.h"
#include <configloader.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

class ZQServiceTestConfigLoader:public ZQ::common::ConfigLoader
{
public:
	virtual ConfigSchemaItem*		getSchema()
	{
		static ConfigSchemaItem entry[]=
		{
			{"default/IceStorm",
				"endpoint",		&szIceStormEndPoint,	
				sizeof(szIceStormEndPoint),	true,ConfigSchemaItem::TYPE_STRING},
			{"default/IceTrace",
				"enabled",		&lIcetraceLogEnable,	
				sizeof(lIcetraceLogEnable),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"default/IceTrace",
				"logfileSuffix",&szIceTraceLogSuffix,	
				sizeof(szIceTraceLogSuffix),true,ConfigSchemaItem::TYPE_STRING},
			{"default/IceTrace",
				"level",		&lIceTraceLogLevel,
				sizeof(lIceTraceLogLevel),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"default/IceTrace",
				"size",			&lIceTraceLogSize,
				sizeof(lIceTraceLogSize),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"default/DatabaseFolder",
				"path",			&szIceDbFolder,
				sizeof(szIceDbFolder),		true,ConfigSchemaItem::TYPE_STRING},
			{"default/LogFolder",
				"path",			&szLogFolder,
				sizeof(szLogFolder),		true,ConfigSchemaItem::TYPE_STRING},
			{"default/IceProperties",
				"prop",			NULL,
				0,							true,ConfigSchemaItem::TYPE_ENUM},
			{"ZQServiceTest/Adapter",
				"endpoint",		&szWeiwooEndpoint,
				sizeof(szWeiwooEndpoint),	true,ConfigSchemaItem::TYPE_STRING},
			{"ZQServiceTest/Adapter",
				"threadpool",	&szWeiwooAdpaterThreadpool,
				sizeof(szWeiwooAdpaterThreadpool),true,ConfigSchemaItem::TYPE_STRING},
			{"ZQServiceTest/DatabaseFolder",
				"path",			&szWeiwooDBFolder,
				sizeof(szWeiwooDBFolder),	true,ConfigSchemaItem::TYPE_STRING},
			{"ZQServiceTest/log",
				"level",		&lWeiwooLogLevel,
				sizeof(lWeiwooLogLevel),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"ZQServiceTest/log",
				"size",			&lWeiwooLogSize,
				sizeof(lWeiwooLogSize),		true,ConfigSchemaItem::TYPE_INTEGER},
			{"ZQServiceTest/log",
				"buffer",		&lWeiwooLogBuffer,
				sizeof(lWeiwooLogBuffer),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"ZQServiceTest/log",
				"flushTimeout",	&lWeiwooLogTimeOut,
				sizeof(lWeiwooLogTimeOut),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"PathManager/log",
				"level",		&lPathLogLevel,
				sizeof(lPathLogLevel),		true,ConfigSchemaItem::TYPE_INTEGER},
			{"PathManager/log",
				"size",			&lPathLogSize,
				sizeof(lPathLogSize),		true,ConfigSchemaItem::TYPE_INTEGER},
			{"PathManager/log",
				"buffer",		&lPathLogBuffer,
				sizeof(lPathLogBuffer),		true,ConfigSchemaItem::TYPE_INTEGER},
			{"PathManager/log",
				"flushTimeout",	&lPathLogTimeOut,
				sizeof(lPathLogTimeOut),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"PathManager/ModuleFolder",
				"path",			&szPathPLuginFolder,
				sizeof(szPathPLuginFolder),	true,ConfigSchemaItem::TYPE_STRING},		
			{NULL,NULL,NULL,0,true,ConfigSchemaItem::TYPE_STRING}
		};
		return entry;
	}
public:
	char				szIceStormEndPoint[512];
	long				lIcetraceLogEnable;
	char				szIceTraceLogSuffix[512];
	long				lIceTraceLogLevel;
	long				lIceTraceLogSize;

	char				szIceDbFolder[512];
	char				szLogFolder[512];

	char				szWeiwooEndpoint[512];
	char				szWeiwooAdpaterThreadpool[20];
	char				szWeiwooDBFolder[512];

	
	long				lWeiwooLogLevel;
	long				lWeiwooLogSize;
	long				lWeiwooLogBuffer;
	long				lWeiwooLogTimeOut;

	long				lPathLogLevel;
	long				lPathLogSize;
	long				lPathLogBuffer;
	long				lPathLogTimeOut;
	char				szPathPLuginFolder[512];

public:
	ZQServiceTestConfigLoader()
	{
		setConfigFileName(_T("ZQServiceTest.xml"));
		ZeroMemory(szIceStormEndPoint,sizeof(szIceStormEndPoint));
		lIcetraceLogEnable=0;
		ZeroMemory(szIceTraceLogSuffix,sizeof(szIceTraceLogSuffix));
		lIceTraceLogLevel=7;
		lIceTraceLogSize=10*1024*1024;

		ZeroMemory(szIceDbFolder,sizeof(szIceDbFolder));
		ZeroMemory(szLogFolder,sizeof(szLogFolder));

		ZeroMemory(szWeiwooEndpoint,sizeof(szWeiwooEndpoint));
		strcpy(szWeiwooAdpaterThreadpool,"5");
		ZeroMemory(szWeiwooDBFolder,sizeof(szWeiwooDBFolder));

		lWeiwooLogLevel=7;
		lWeiwooLogSize=10*1024*1024;
		lWeiwooLogBuffer=100*1024;
		lWeiwooLogTimeOut=2000;

		lPathLogLevel=7;
		lPathLogSize=10*1024*1024;
		lPathLogBuffer=100*1024;
		lPathLogTimeOut=2000;
		ZeroMemory(szPathPLuginFolder,sizeof(szPathPLuginFolder));
	}
};

DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;


ZQServiceTestConfigLoader					gZQServiceTestConfig;
ZQ::common::ConfigLoader		*configLoader=&gZQServiceTestConfig;


TestService g_server;
ZQ::common::BaseZQServiceApplication *Application = &g_server;

TestService::TestService()
{

}

TestService::~TestService()
{
	int ia = 10;

}

HRESULT TestService::OnInit(void)
{
	BaseZQServiceApplication::OnInit();
	BaseZQServiceApplication::logEvent(_T("Service Onit"),1);
	return S_OK;
}

HRESULT TestService::OnStop(void)
{
	Sleep(5000);
	BaseZQServiceApplication::OnStop();
	BaseZQServiceApplication::logEvent(_T("Service OnStop"),1);
	return S_OK;
}

HRESULT TestService::OnPause(void)
{
	BaseZQServiceApplication::OnPause();
	BaseZQServiceApplication::logEvent(_T("Service OnPause"),1);
	return S_OK;
}

HRESULT TestService::OnContinue(void)
{
	BaseZQServiceApplication::OnContinue();
	BaseZQServiceApplication::logEvent(_T("Service OnContinue"),1);
	return S_OK;
}

bool    TestService::isHealth(void)
{

	BaseZQServiceApplication::isHealth();
	return true;
}

HRESULT TestService::OnStart(void)
{

	BaseZQServiceApplication::OnStart();
	BaseZQServiceApplication::logEvent(_T("Service OnStart"),1);
	return S_OK;

}

HRESULT TestService::OnUnInit(void)
{
	BaseZQServiceApplication::OnUnInit();
	BaseZQServiceApplication::logEvent(_T("Service OnUnInit"),1);
	return S_OK;

}