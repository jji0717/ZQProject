
#ifndef _ZQ_ALARM_CLIENT_H_
#define _ZQ_ALARM_CLIENT_H_

#include "../alarminclude.h"
#include "registryex.h"

using namespace ZQ::ALARM;

#define REG_KEY_CONFIG_FILE		"ConfigFile"
#define REG_KEY_IMPLEMENTATION	"Implementation"

#define MAX_TRIGGER_COUNT 256

class AlarmClient
{
private:
	INIDataSource*	m_pSource;
	Implementation*	m_pImpl;
	RegularParser*	m_pParser;
	SCLogTrigger*	m_pTrigger[MAX_TRIGGER_COUNT];
	TriggerList		m_list;
	RegistryEx_T	m_reg;

public:
	AlarmClient(const char* registrypath);
	~AlarmClient();

	bool startTrigger();
	bool stopTrigger();
};

#endif//_ZQ_ALARM_CLIENT_H_
