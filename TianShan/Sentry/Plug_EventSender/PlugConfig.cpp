// PlugConfig.cpp: implementation of the PlugConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "PlugConfig.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::common;

PlugConfig::PlugConfig()
{
	memset(chName,0,sizeof(chName));
	strcpy(chName,"MsgProPerty");

	memset(logPath,0,sizeof(logPath));
	logSize = 0;
	logLevel = 0;

	memset(jmsIpPort,0,sizeof(jmsIpPort));
	memset(jmsContext,0,sizeof(jmsContext));
	memset(jmsConnFactory,0,sizeof(jmsConnFactory));
	memset(jmsDesName,0,sizeof(jmsDesName));
	jmsTimeToLive = 0;
	jmsDequeSize = 0;
	memset(jmsSavePath,0,sizeof(jmsSavePath));

	memset(iceManagerStr,0,sizeof(iceManagerStr));
	iceTimeout = 0;
	iceDequeSize = 0;
	memset(iceSavePath,0,sizeof(iceSavePath));

	memset(textPath,0,sizeof(textPath));
}

PlugConfig::~PlugConfig()
{

}
