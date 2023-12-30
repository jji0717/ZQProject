// PlugConfig.h: interface for the PlugConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLUGCONFIG_H__6B317DEE_0B43_486C_8C55_4787D957CC5C__INCLUDED_)
#define AFX_PLUGCONFIG_H__6B317DEE_0B43_486C_8C55_4787D957CC5C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <ConfigLoader.h>

class PlugConfig : public ZQ::common::ConfigLoader  
{
public:
	PlugConfig();
	virtual ~PlugConfig();
	PConfigSchema getSchema()
	{
		
		static ConfigSchemaItem entry[]=
			{
				{"Log",
					"logPath",		&logPath,	
					sizeof(logPath),	true,ConfigSchemaItem::TYPE_STRING},
				{"Log",
					"logFileSize",		&logSize,	
					sizeof(logSize),	true,ConfigSchemaItem::TYPE_INTEGER},
				{"Log",
					"logLevel",		&logLevel,	
					sizeof(logLevel),	true,ConfigSchemaItem::TYPE_INTEGER},

				//about jms configuration item
				{"JmsSender/Basic",
					"context",		&jmsContext,	
					sizeof(jmsContext),	true,ConfigSchemaItem::TYPE_STRING},
				{"JmsSender/Basic",
					"ipPort",		&jmsIpPort,	
					sizeof(jmsIpPort),	true,ConfigSchemaItem::TYPE_STRING},
				{"JmsSender/Basic",
					"destinationName",		&jmsDesName,	
					sizeof(jmsDesName),	true,ConfigSchemaItem::TYPE_STRING},
				{"JmsSender/Basic",
					"connectionFactory",		&jmsConnFactory,	
					sizeof(jmsConnFactory),	true,ConfigSchemaItem::TYPE_STRING},
				{"JmsSender/JmsMessageProperty",
					"MsgProPerty",		NULL,	
					0,	true,ConfigSchemaItem::TYPE_ENUM},
				{"JmsSender/ProducerOpt",
					"timeToLive",		&jmsTimeToLive,	
					sizeof(jmsTimeToLive),	true,ConfigSchemaItem::TYPE_INTEGER},
				{"JmsSender/Other",
					"dequeSize",		&jmsDequeSize,	
					sizeof(jmsDequeSize),	true,ConfigSchemaItem::TYPE_INTEGER},
				{"JmsSender/Other",
					"savePath",		&jmsSavePath,	
					sizeof(jmsSavePath),	true,ConfigSchemaItem::TYPE_STRING},
				//about ice configuration item
				{"IceSender/Basic",
					"endPoint",		&iceManagerStr,	
					sizeof(iceManagerStr),	true,ConfigSchemaItem::TYPE_STRING},
				{"IceSender/Basic",
					"timeout",		&iceTimeout,	
					sizeof(iceTimeout),	true,ConfigSchemaItem::TYPE_INTEGER	},
				{"IceSender/Other",
					"dequeSize",		&iceDequeSize,	
					sizeof(iceDequeSize),	true,ConfigSchemaItem::TYPE_INTEGER},
				{"IceSender/Other",
					"savePath",		&iceSavePath,	
					sizeof(iceSavePath),	true,ConfigSchemaItem::TYPE_STRING},
				//TextWriter configuration item
				{"TextWriter/Basic",
					"receiveFile",		&textPath,	
					sizeof(textPath),	true,ConfigSchemaItem::TYPE_STRING},

				{NULL,NULL,NULL,0,true,ConfigSchemaItem::TYPE_STRING}
			};
		
		return entry;

	};

public:

	char chName[30];

	char  logPath[512];
	DWORD logSize;
	int   logLevel;
	//jms configuration item
	char  jmsContext[256];
	char  jmsIpPort[50];
	char  jmsDesName[128];
	char  jmsConnFactory[128];
	int   jmsTimeToLive;
	int   jmsDequeSize;
	char  jmsSavePath[512];

	//ice configuration item
	char  iceManagerStr[256];
	int   iceTimeout;
	int   iceDequeSize;
	char  iceSavePath[512];

	//TextWriter 
	char  textPath[512];
};

#endif // !defined(AFX_PLUGCONFIG_H__6B317DEE_0B43_486C_8C55_4787D957CC5C__INCLUDED_)
