
#include "alarmclient.h"
#include <exception>

AlarmClient::AlarmClient(const char* registrypath)
:m_reg(registrypath)
{
	char strConfigFile[MAX_PATH]		= {0};
	char strImplementation[MAX_PATH]	= {0};
	if (!m_reg.LoadStr(REG_KEY_CONFIG_FILE, strConfigFile, MAX_PATH-1))
	{
		throw std::exception("[AlarmClient::AlarmClient] the config file setting has not found in registry");
	}
	if (!m_reg.LoadStr(REG_KEY_IMPLEMENTATION, strImplementation, MAX_PATH-1))
	{
		throw std::exception("[AlarmClient::AlarmClient] the implementation file setting has not found in registry");
	}

	m_pSource	= new INIDataSource(strConfigFile);
	try
	{
		m_pImpl		= new Implementation(strImplementation, *m_pSource);
	}
	catch(std::exception& exp)
	{
		delete m_pSource;
		throw std::exception("[AlarmClient::AlarmClient] can not create implementation");
	}

	m_pParser	= new RegularParser(*m_pSource);

	for (size_t i = 0; i < m_pSource->countSource(); ++i)
	{
		try
		{
			m_pTrigger[i] = new SCLogTrigger(m_pSource->listSource(i).c_str(), *m_pParser, i);
			m_list.push_back(m_pTrigger[i]);
		}
		catch(std::exception& exp)
		{
			for (int j = 0; j < i; ++j)
			{
				delete m_pTrigger[j];
				delete m_pParser;
				delete m_pImpl;
				delete m_pSource;
			}
			throw std::exception([AlarmClient::AlarmClient] can not create trigger);
		}
	}
}

AlarmClient::~AlarmClient()
{
	for (size_t i = 0; i < m_list.size(); ++i)
	{
		delete m_list[i];
	}

	delete m_pParser;
	delete m_pImpl;
	delete m_pSource;
}

bool AlarmClient::startTrigger()
{
	return m_list.startTrigger();
}

bool AlarmClient::stopTrigger()
{
	return m_list.stopTrigger();
}
