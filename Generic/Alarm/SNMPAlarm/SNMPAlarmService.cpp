
#include "BaseSchangeServiceApplication.h"
#include "manutiloption.h"
#include "nativethread.h"
#include "alarmclient.h"
#include "registryex.h"
#include "ini.h"
#include <exception>
#include "loghandler.h"
	
#define ALARM_SOFTWARE_REG_PATH "Software\\ZQ\\SNMPAlarm"

class SNMPAlarmService : public ZQ::common::BaseSchangeServiceApplication
{
private:
	static RegistryEx_T	m_reg;
	static IniFile		m_ini;
	AlarmClient*		m_pAlarm;


protected:
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);
	HRESULT OnStart(void);
	
	HRESULT OnStop(void);

public:
	//this function must run before than "setupUtilIniFile" function
	static UINT setupUtilRegistry(WCHAR* pCmds, WCHAR** pstrBuffer, DWORD* pLength);
	static UINT setupUtilIniFile(WCHAR* pCmds, WCHAR** pstrBuffer, DWORD* pLength);
};

RegistryEx_T SNMPAlarmService::m_reg(ALARM_SOFTWARE_REG_PATH);
IniFile SNMPAlarmService::m_ini;

UINT SNMPAlarmService::setupUtilRegistry(WCHAR* pCmds, WCHAR** pstrBuffer, DWORD* pLength)
{
	WORD wCommand;
	swscanf(pCmds, L"%c\t", &wCommand);
	if (wCommand == MAN_FREE)
	{
		if (*pstrBuffer != NULL)
		{
			delete[] *pstrBuffer;
			*pstrBuffer = NULL;
		}
		return MAN_SUCCESS;
	}
	else if (wCommand != MAN_READ_VARS)
	{
		return MAN_BAD_PARAM;
	}

	char strIniFileName[MAX_PATH] = {0};

	ZQ::common::ManUtilBuilder mubuilder;
	for (int i = 0; true; ++i)
	{
		char strKey[256] = {0};
		char strValue[256] = {0};
		if (!m_reg.EnumValue(i, strKey, 255, strValue, 255))
			break;

		mubuilder.AddVariable(strKey, MAN_STR, strValue);
	}

	wcscpy(*pstrBuffer, mubuilder.BuildString().c_str());
	*pLength = mubuilder.BuildString().size();
	
	return MAN_SUCCESS;
}

UINT SNMPAlarmService::setupUtilIniFile(WCHAR* pCmds, WCHAR** pstrBuffer, DWORD* pLength)
{
	WORD wCommand;
	swscanf(pCmds, L"%c\t", &wCommand);
	if (wCommand == MAN_FREE)
	{
		if (*pstrBuffer != NULL)
		{
			delete[] *pstrBuffer;
			*pstrBuffer = NULL;
		}
		return MAN_SUCCESS;
	}
	else if (wCommand != MAN_READ_VARS)
	{
		return MAN_BAD_PARAM;
	}

	ZQ::common::ManUtilBuilder mubuilder;
	std::vector<std::string> arrMain;
	if (!m_ini.EnumKey(MAIN_SECTION, arrMain))
	{
		return MAN_BAD_PARAM;
		//throw std::exception("[SNMPAlarmService::setupUtilIniFile] can not get \"Main\" section in config file");
	}

	int nLogCount = 0;
	for (int i = 0; i < arrMain.size(); ++i)
	{
		int nDivPos = arrMain[i].find('=');
		std::string strKey = arrMain[i].substr(0, nDivPos);
		std::string strValue = arrMain[i].substr(nDivPos+1);

		mubuilder.AddVariable(strKey.c_str(), MAN_STR, strValue.c_str());

		if (0 == stricmp(strKey.c_str(), LOG_COUNT_KEY))
			nLogCount = atoi(strValue.c_str());
	}

	std::map<std::string, std::vector<std::string> >  mapRowList;
	std::vector<std::string> arrHeadRow;

	for (i = 0; i < nLogCount; ++i)
	{
		char strGroupSection[MAX_INI_SECTION_LENGTH] = {0};
		_snprintf(strGroupSection, MAX_INI_SECTION_LENGTH-1, "%s_%3d", GROUP_SECTION, i);

		std::string strLogFile = m_ini.ReadKey(strGroupSection, LOG_LOCATION_KEY);
		std::string strSyntaxCount = m_ini.ReadKey(strGroupSection, TRIGGER_NUMBER_KEY);
		int nSyntaxCount = atoi(strSyntaxCount.c_str());

		std::string strTriggerSection = m_ini.ReadKey(strGroupSection, SECTION_NAME_KEY);

		for (int j = 0; j < nSyntaxCount; ++j)
		{
			char strSyntaxSection[MAX_INI_SECTION_LENGTH] = {0};
			_snprintf(strSyntaxSection, MAX_INI_SECTION_LENGTH-1, "%s_%3d", strTriggerSection.c_str(), j);
			char strRowHead[2*MAX_INI_SECTION_LENGTH] = {0};
			_snprintf(strRowHead, 2*MAX_INI_SECTION_LENGTH-1, "%s\\%s",
				strLogFile.c_str(), strTriggerSection.c_str());

			arrHeadRow.push_back(strRowHead);

			std::vector<std::string> arrRow;
			if (m_ini.EnumKey(strSyntaxSection, arrRow))
			{
				for (int k = 0; k < arrRow.size(); ++k)
				{
					int nDivPos = arrRow[k].find('=');
					std::string strKey = arrRow[k].substr(0, nDivPos);
					std::string strValue = arrRow[k].substr(nDivPos+1);

					mapRowList[strKey].push_back(strValue);
				}
			}
		}

	}

	for (std::map<std::string, std::vector<std::string> >::iterator itor = mapRowList.begin();
	itor != mapRowList.end(); ++itor)
	{
		mubuilder.AddColumn(itor->first.c_str(), MAN_STR, itor->second);
	}
	
	wcscpy(*pstrBuffer, mubuilder.BuildString().c_str());
	*pLength = mubuilder.BuildString().size();
	
	return MAN_SUCCESS;
}

HRESULT SNMPAlarmService::OnInit(void)
{
	BaseSchangeServiceApplication::OnInit();
	//m_pReporter->setReportLevel(NT_EVENT_LOG, ZQ::common::Log::L_NOTICE);
	//m_pReporter->setReportLevel(SERVICE_LOG, ZQ::common::Log::L_DEBUG);

	m_pAlarm = new AlarmClient(ALARM_SOFTWARE_REG_PATH);

	char strConfigFile[MAX_PATH] = {0};
	char strImplementation[MAX_PATH] = {0};
	if (!m_reg.LoadStr(REG_KEY_CONFIG_FILE, strConfigFile, MAX_PATH-1))
	{
		delete m_pAlarm;
		return -1;
		//throw std::exception("[SNMPAlarmService::OnInit] can not get config file setting in registry");
	}
	if (!m_reg.LoadStr(REG_KEY_CONFIG_FILE, strImplementation, MAX_PATH-1))
	{
		delete m_pAlarm;
		return -1;
		//throw std::exception("[SNMPAlarmService::OnInit] can not get implementation file setting in registry");
	}

	m_ini.SetFileName(strConfigFile);

	wchar_t strwConfigFile[MAX_PATH] = {0};
	wchar_t strwImplementation[MAX_PATH] = {0};
	mbstowcs(strwConfigFile, strConfigFile, MAX_PATH);
	mbstowcs(strwImplementation, strImplementation, MAX_PATH);

	DWORD dwErr = 0;
	manageVar(L"ConfigFile", MAN_STR, (DWORD)strwConfigFile, true, &dwErr);
	manageVar(L"Implememtation", MAN_STR, (DWORD)strwImplementation, true, &dwErr);
	manageVar(L"Registry", MAN_COMPLEX, reinterpret_cast<DWORD>(setupUtilRegistry), true, &dwErr);
	manageVar(L"INI", MAN_COMPLEX, reinterpret_cast<DWORD>(setupUtilIniFile), true, &dwErr);

	return (NULL == m_pAlarm)?-1:S_OK;
}

HRESULT SNMPAlarmService::OnUnInit(void)
{
	if (NULL != m_pAlarm)
	{
		delete m_pAlarm;
		m_pAlarm = NULL;
	}
	
	return S_OK;
}

HRESULT SNMPAlarmService::OnStart(void)
{
	if (!m_pAlarm->startTrigger())
	{
		Log(LogHandler::L_ERROR, "[SNMPAlarmService::OnStart] can not start service list");
		return -1;
	}

	return S_OK;
}

HRESULT SNMPAlarmService::OnStop(void)
{
	if (!m_pAlarm->stopTrigger())
	{
		Log(LogHandler::L_ERROR, "[SNMPAlarmService::OnStart] can not stop service list");
		return -1;
	}

	return S_OK;
}

DWORD gdwServiceType		= 11;
DWORD gdwServiceInstance	= 11;
SNMPAlarmService gApplicationService;
ZQ::common::BaseSchangeServiceApplication * Application = &gApplicationService;