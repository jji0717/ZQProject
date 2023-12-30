#if !defined(AFX_DODAPPSERVICE_H__95E43CAF_5C0C_407F_9C54_A3DBE6E2220C__INCLUDED_)
#define AFX_DODAPPSERVICE_H__95E43CAF_5C0C_407F_9C54_A3DBE6E2220C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "BaseZQServiceApplication.h"
#include <Ice/Ice.h>
#include <string>
#include <vector>
#include <dodapp.h>
#include <datastream.h>
#include <configloader.h>
#include <Log.h>
#include <filelog.h>
#define MAXNAMELEN						256
class DODAppSVC : public ZQ::common::BaseZQServiceApplication 
{
public:

	DODAppSVC();
	virtual ~DODAppSVC();
	
public:
	
	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);	

private:
	ZQ::common::Log*		m_pIceLog;
};
/*class DODAppConfig: public ZQ::common::ConfigLoader
{
public:
	virtual ConfigSchemaItem*		getSchema()
	{
		static ConfigSchemaItem entry[]=
		{
			{"DODApp/Adapter",
				"endpoint",		&szDODAppEndpoint,
				sizeof(szDODAppEndpoint),	true,ConfigSchemaItem::TYPE_STRING},		
			{"DODApp/DatabaseFolder",
				"dbpath",			&szDODAppDBFolder,
				sizeof(szDODAppDBFolder),	true,ConfigSchemaItem::TYPE_STRING},
			{"DODApp/IceTrace",
				"level",		&lIceTraceLogLevel,
				sizeof(lIceTraceLogLevel),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"DODApp/IceTrace",
				"size",			&lIceTraceLogSize,
				sizeof(lIceTraceLogSize),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"DODApp/LocalFold",
				"notifytime",			&lLocalFoldNotifytime,
				sizeof(lLocalFoldNotifytime),true,ConfigSchemaItem::TYPE_INTEGER},
			{"DODApp/Stream",
				"renewtime",			&lRenewtime,
				sizeof(lRenewtime),true,ConfigSchemaItem::TYPE_INTEGER},
			{"DODApp/Stream",
				"streampingtime",			&lStreamPingtime,
				sizeof(lStreamPingtime),true,ConfigSchemaItem::TYPE_INTEGER},
			{"DODApp/ServiceGroup",
				"groupID",			&lDODAppServiceGroup,
				sizeof(lDODAppServiceGroup),true,ConfigSchemaItem::TYPE_INTEGER},
			{"DODApp/SpaceName",
				"configname",			&szSpaceName,
				sizeof(szSpaceName),true,ConfigSchemaItem::TYPE_STRING},				 
			{"DODApp/RtspURL",
				"url",		&szDODRtspURL,
				sizeof(szDODRtspURL),	true,ConfigSchemaItem::TYPE_STRING},
	 <IceProperties> 
			{"DODApp/IceProperties",
			    "prop", NULL, 0, true, ConfigSchemaItem::TYPE_ENUM},				
			{"DODContent/Adapter",
				"endpoint",		&szDODContentEndpoint,
				sizeof(szDODContentEndpoint),	true,ConfigSchemaItem::TYPE_STRING},				
			{"SRM/Adapter",
				"endpoint",		&szSRMEndpoint,
				sizeof(szSRMEndpoint),	true,ConfigSchemaItem::TYPE_STRING},
			{"JmsDispatch/JBossIPPort",
				"ipport",		&szJBossIpPort,
				sizeof(szJBossIpPort),	true,ConfigSchemaItem::TYPE_STRING},
			{"JmsDispatch/ConfigQueueName",
				"queuename",		&szConfigQueueName,
				sizeof(szConfigQueueName),	true,ConfigSchemaItem::TYPE_STRING},
			{"JmsDispatch/ConfigMsgTimeOut",
				"timeout",		&lConfigTimeOut,
				sizeof(lConfigTimeOut),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"JmsDispatch/UsingJboss",
				"useflag",		&lUsingJboss,
				sizeof(lUsingJboss),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"JmsDispatch/SourceCachePath",
				"CachePath",		&szCachepath,
				sizeof(szCachepath),	true,ConfigSchemaItem::TYPE_STRING},		
			{NULL,NULL,NULL,0,true,ConfigSchemaItem::TYPE_STRING}
		};

		return entry;
	}
public:
	char				szDODAppEndpoint[512];
	char				szDODAppDBFolder[512];

	long				lDODAppServiceGroup;
	long				lIceTraceLogLevel;
	long				lIceTraceLogSize;

	long                lLocalFoldNotifytime;
	long				lRenewtime;
	long				lStreamPingtime;
    char				szSpaceName[512];
	char				szDODRtspURL[512];

	char				szSRMEndpoint[512];

	char				szDODContentEndpoint[512];

	char				szJBossIpPort[512];
	char				szConfigQueueName[512];
	long				lConfigTimeOut;
	long				lUsingJboss;
	char				szCachepath[512];				

public:
	DODAppConfig()
	{
		setConfigFileName(_T("DODAppServices.xml"));

		ZeroMemory(szDODAppEndpoint,sizeof(szDODAppEndpoint));
		ZeroMemory(szDODAppDBFolder,sizeof(szDODAppDBFolder));

		lIceTraceLogLevel=7;
		lIceTraceLogSize=10*1024*1024;

		lLocalFoldNotifytime = 300;
		lDODAppServiceGroup  = 9999;
        ZeroMemory(szSpaceName,sizeof(szSpaceName));
		ZeroMemory(szDODRtspURL,sizeof(szDODRtspURL));

		ZeroMemory(szSRMEndpoint,sizeof(szSRMEndpoint));
		ZeroMemory(szDODContentEndpoint,sizeof(szDODContentEndpoint));

		ZeroMemory(szJBossIpPort,sizeof(szJBossIpPort));
		ZeroMemory(szConfigQueueName,sizeof(szConfigQueueName));
		ZeroMemory(szCachepath,sizeof(szCachepath));
		lConfigTimeOut = 30000;
		lUsingJboss = 1;
	}	
};*/
#endif // !defined(AFX_DODAPPSERVICE_H__95E43CAF_5C0C_407F_9C54_A3DBE6E2220C__INCLUDED_)