// SnmpConfig.cpp: implementation of the SnmpConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SnmpConfig.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SnmpConfig::SnmpConfig()
{
	memset(logPath,0,sizeof(logPath));
	logSize = 0;
	logLevel = 0;


	memset(agentIp,0,sizeof(agentIp));
	agentPort = 0;
	memset(snmpMenu,0,sizeof(snmpMenu));
	strcpy(snmpMenu,"Target");
	memset(snmpSavePath,0,sizeof(snmpSavePath));
	snmpDequeSize = 0;
}

SnmpConfig::~SnmpConfig()
{

}
