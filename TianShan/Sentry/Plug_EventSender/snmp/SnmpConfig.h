// SnmpConfig.h: interface for the SnmpConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SNMPCONFIG_H__CB55A8EE_32FE_4FBE_B217_73E501667301__INCLUDED_)
#define AFX_SNMPCONFIG_H__CB55A8EE_32FE_4FBE_B217_73E501667301__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <ConfigLoader.h>

class SnmpConfig : public ZQ::common::ConfigLoader  
{
public:
	SnmpConfig();
	virtual ~SnmpConfig();

	PConfigSchema getSchema()
	{
		
		static ConfigSchemaItem entry[]=
		{
			//log configuration
			{"Log",
					"logPath",		&logPath,	
					sizeof(logPath),	true,ConfigSchemaItem::TYPE_STRING},
			{"Log",
				"logFileSize",		&logSize,	
				sizeof(logSize),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"Log",
				"logLevel",		&logLevel,	
				sizeof(logLevel),	true,ConfigSchemaItem::TYPE_INTEGER},

			//snmp sender configuration	
			{"SnmpSender/Agent",
					"localIp",		&agentIp,	
					sizeof(agentIp),	true,ConfigSchemaItem::TYPE_STRING},
			{"SnmpSender/Agent",
				"localPort",		&agentPort,	
				sizeof(agentPort),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"SnmpSender/Targets",
					"Target",		NULL,	
					0,	true,ConfigSchemaItem::TYPE_ENUM},
			{"SnmpSender/Other",
					"dequeSize",		&snmpDequeSize,	
					sizeof(snmpDequeSize),	true,ConfigSchemaItem::TYPE_INTEGER},
			{"SnmpSender/Other",
				"savePath",		&snmpSavePath,	
				sizeof(snmpSavePath),	true,ConfigSchemaItem::TYPE_STRING},

			{NULL,NULL,NULL,0,true,ConfigSchemaItem::TYPE_STRING}
		};
		
		return entry;

	};

public:
	//log configuration
	char  logPath[512];
	DWORD logSize;
	int   logLevel;

	//snmp configuration
	char agentIp[20];
	int  agentPort; 
	char snmpMenu[30];
	int   snmpDequeSize;
	char  snmpSavePath[512];


};

#endif // !defined(AFX_SNMPCONFIG_H__CB55A8EE_32FE_4FBE_B217_73E501667301__INCLUDED_)
