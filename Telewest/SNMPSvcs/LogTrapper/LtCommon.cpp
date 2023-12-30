
#include "ltcommon.h"
#include "..\LogTrapper\logtrigger.h"
#include "log.h"

namespace ZQ
{
	namespace SNMP
	{
		RegSetting::RegSetting()
			:m_reg(LOG_TRAPPER_SETTING_REGISTRY), 
			m_error(REG_ERR_NO_ERROR),
			m_strConfigFileName(DEFAULT_CONFIG_FILE),
			m_strLogFileName(DEFAULT_LOG_FILE)
		{
			if (m_reg.IsError())
			{
				m_error = REG_ERR_CANNOT_CREATE;
				return;
			}

			char strLogFileName[MAX_REG_STRING_LEN] = {0};
			if (!m_reg.LoadStr(REG_ITEM_LOGFILE, strLogFileName, MAX_REG_STRING_LEN))
			{
				m_error = REG_ERR_CANNOT_READITEM;
			}
			else
			{
				m_strLogFileName = strLogFileName;
			}

			char strConfigFileName[MAX_REG_STRING_LEN] = {0};
			if (!m_reg.LoadStr(REG_ITEM_CONFIGFILE, strConfigFileName, MAX_REG_STRING_LEN))
			{
				m_error = REG_ERR_CANNOT_READITEM;
			}
			else
			{
				m_strConfigFileName = strConfigFileName;
			}

			char strAddress[MAX_REG_STRING_LEN] = {0};
			if (!m_reg.LoadStr(REG_ITEM_LOCALADDR, strAddress, MAX_REG_STRING_LEN))
			{
				m_error = REG_ERR_CANNOT_READITEM;
			}
			else
			{
				m_strSenderAddress = strAddress;
			}

			char strCommunity[MAX_REG_STRING_LEN] = {0};
			if (!m_reg.LoadStr(REG_ITEM_COMMUNITY, strCommunity, MAX_REG_STRING_LEN))
			{
				m_error = REG_ERR_CANNOT_READITEM;
			}
			else
			{
				m_strCommunity = strCommunity;
			}
		}

		RegSetting::~RegSetting()
		{
		}

		bool RegSetting::Ok()
		{
			return REG_ERR_NO_ERROR == m_error;
		}

		int RegSetting::Clear()
		{
			int nRtn = m_error;
			m_error = REG_ERR_NO_ERROR;
			return nRtn;
		}

		const std::string& RegSetting::ConfigFileName()
		{
			return m_strConfigFileName;
		}

		const std::string& RegSetting::LogFileName()
		{
			return m_strLogFileName;
		}

		const std::string& RegSetting::SenderAddress()
		{
			return m_strSenderAddress;
		}

		const std::string& RegSetting::Community()
		{
			return m_strCommunity;
		}




		IniSetting::IniSetting(const std::string& strIniFile)
			:m_ini(strIniFile.c_str()),
			m_error(INI_ERR_NO_ERROR)
		{
			glog(ZQ::common::Log::L_INFO, "Read INI file: %s", strIniFile.c_str());

			{
				m_hostname = m_ini.ReadKey(INI_COMMON_SECTION, INI_ITEM_HOSTNAME);
				m_startupAID = m_ini.ReadKey(INI_COMMON_SECTION, INI_ITEM_STARTUPAID);
				m_shutdownAID = m_ini.ReadKey(INI_COMMON_SECTION, INI_ITEM_SHUTDOWNAID);
				m_strStartUpMsg = m_ini.ReadKey(INI_COMMON_SECTION, INI_ITEM_STARTUPMSG);
				m_strShutDownMsg = m_ini.ReadKey(INI_COMMON_SECTION, INI_ITEM_SHUTDOWNMSG);
			
				int nAddrCount		= atoi((m_ini.ReadKey(INI_COMMON_SECTION, INI_ITEM_DEST_COUNT)).c_str());
				m_dests.clear();
				for (int nAddr = 0; nAddr < nAddrCount; ++nAddr)
				{
					char strFormatTemp[MAX_INI_STRING_LEN] = {0};
					IniStruct_DestAddr isdTemp;
					memset(strFormatTemp, 0, MAX_INI_STRING_LEN);
					sprintf(strFormatTemp, "%s%03d", INI_ITEM_ADDR, nAddr+1);
					isdTemp.addr = m_ini.ReadKey(INI_COMMON_SECTION, strFormatTemp);

					if (isdTemp.addr.empty())
					{
						glog(ZQ::common::Log::L_WARNING, "Cannot read %s from INI file", INI_ITEM_ADDR);
						m_error = INI_ERR_CANNOT_READITEM;
					}
					
					memset(strFormatTemp, 0, MAX_INI_STRING_LEN);
					sprintf(strFormatTemp, "%s%03d", INI_ITEM_PORT, nAddr+1);
					isdTemp.port = m_ini.ReadKey(INI_COMMON_SECTION, strFormatTemp);

					if (isdTemp.port.empty())
					{
						glog(ZQ::common::Log::L_WARNING, "Cannot read %s from INI file", INI_ITEM_PORT);
						m_error = INI_ERR_CANNOT_READITEM;
					}

					m_dests.push_back(isdTemp);
				}
			}
			
			std::vector<std::string> arrSecLists;
			if (!m_ini.EnumSection(arrSecLists))
			{
				glog(ZQ::common::Log::L_ERROR, "Cannot read INI file");

				m_error = INI_ERR_CANNOT_CREATE;
				return;
			}

			// remove common section
			{
				std::vector<std::string>::iterator it = arrSecLists.begin();
				for(;it!=arrSecLists.end();it++)
				{
					if (!stricmp((*it).c_str(), INI_COMMON_SECTION))
					{
						arrSecLists.erase(it);
						break;
					}
				}
			}


			IniStruct_Section issTemp;
			IniStruct_Trigger istTemp;
			IniStruct_DestAddr isdTemp;
			char strFormatTemp[MAX_INI_STRING_LEN] = {0};
			this->clear();

			for (std::vector<std::string>::iterator itor = arrSecLists.begin();
				itor != arrSecLists.end(); ++itor)
			{
				issTemp.logfile		= m_ini.ReadKey((*itor).c_str(), INI_ITEM_LOGFILE);
				issTemp.hostname	= m_ini.ReadKey((*itor).c_str(), INI_ITEM_HOSTNAME);
				issTemp.ssfile		= m_ini.ReadKey((*itor).c_str(), INI_ITEM_SAFESTOREFILE);

				int nAddrCount		= atoi(m_ini.ReadKey((*itor).c_str(), INI_ITEM_DEST_COUNT).c_str());
				issTemp.dests.clear();
				for (int nAddr = 0; nAddr < nAddrCount; ++nAddr)
				{
					memset(strFormatTemp, 0, MAX_INI_STRING_LEN);
					sprintf(strFormatTemp, "%s%03d", INI_ITEM_ADDR, nAddr+1);
					isdTemp.addr = m_ini.ReadKey((*itor).c_str(), strFormatTemp);

					if (isdTemp.addr.empty())
					{
						glog(ZQ::common::Log::L_WARNING, "Cannot read %s from INI file", INI_ITEM_ADDR);
						m_error = INI_ERR_CANNOT_READITEM;
					}
					
					memset(strFormatTemp, 0, MAX_INI_STRING_LEN);
					sprintf(strFormatTemp, "%s%03d", INI_ITEM_PORT, nAddr+1);
					isdTemp.port = m_ini.ReadKey((*itor).c_str(), strFormatTemp);

					if (isdTemp.port.empty())
					{
						glog(ZQ::common::Log::L_WARNING, "Cannot read %s from INI file", INI_ITEM_PORT);
						m_error = INI_ERR_CANNOT_READITEM;
					}

					issTemp.dests.push_back(isdTemp);
				}

				int nTriggerCount	= atoi(m_ini.ReadKey((*itor).c_str(), INI_ITEM_COUNT).c_str());

				issTemp.triggers.clear();
				for (int nTrigger = 0; nTrigger < nTriggerCount; ++nTrigger)
				{
					memset(strFormatTemp, 0, MAX_INI_STRING_LEN);
					sprintf(strFormatTemp, "%s%03d", INI_ITEM_SYNTAX, nTrigger+1);
					istTemp.syntax	= m_ini.ReadKey((*itor).c_str(), strFormatTemp);

					if (istTemp.syntax.empty())
					{
						glog(ZQ::common::Log::L_WARNING, "Cannot read %s from INI file", INI_ITEM_SYNTAX);
						m_error = INI_ERR_CANNOT_READITEM;
					}
					
					memset(strFormatTemp, 0, MAX_INI_STRING_LEN);
					sprintf(strFormatTemp, "%s%03d", INI_ITEM_TIME, nTrigger+1);
					istTemp.time	= m_ini.ReadKey((*itor).c_str(), strFormatTemp);

					if (istTemp.time.empty())
					{
						glog(ZQ::common::Log::L_WARNING, "Cannot read %s from INI file", INI_ITEM_TIME);
						m_error = INI_ERR_CANNOT_READITEM;
					}
					
					memset(strFormatTemp, 0, MAX_INI_STRING_LEN);
					sprintf(strFormatTemp, "%s%03d", INI_ITEM_ID, nTrigger+1);
					istTemp.id		= m_ini.ReadKey((*itor).c_str(), strFormatTemp);

					if (istTemp.id.empty())
					{
						glog(ZQ::common::Log::L_WARNING, "Cannot read %s from INI file", INI_ITEM_ID);
						m_error = INI_ERR_CANNOT_READITEM;
					}

					issTemp.triggers.push_back(istTemp);
				}

				this->push_back(issTemp);
			}
		}

		IniSetting::~IniSetting()
		{
		}

		bool IniSetting::Ok()
		{
			return INI_ERR_NO_ERROR == m_error;
		}

		int IniSetting::Clear()
		{
			int nRtn = m_error;
			m_error = INI_ERR_NO_ERROR;
			return nRtn;
		}
	}
}

ZQ::SNMP::RegSetting g_reg_setting;
//ZQ::SNMP::IniSetting g_ini_setting(REG_ITEM.ConfigFileName());


