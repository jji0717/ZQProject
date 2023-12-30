
#ifndef _ZQ_LT_COMMON_H_
#define _ZQ_LT_COMMON_H_

#include <screporter.h>
#include <snmp_pp\snmp_pp.h>
//#include <regex.h>
#include <boost/regex.hpp>

#include "registryex.h"
#include "ini.h"

#define SERVICE_NAME "SnmpSvcs"

#define LOG_TRAPPER_SETTING_REGISTRY "Software\\SeaChange\\ITV\\CurrentVersion\\Services\\SnmpSvcs"
#define DEFAULT_CONFIG_FILE "C:\\logtp.ini"
#define DEFAULT_LOG_FILE "C:\\logtp.log"
#define DEFAULT_LOCALADDR "127.0.0.1"

#define REG_ERR_NO_ERROR			0
#define REG_ERR_CANNOT_CREATE		1
#define REG_ERR_CANNOT_READITEM		2

#define INI_ERR_NO_ERROR			0
#define INI_ERR_CANNOT_CREATE		1
#define INI_ERR_CANNOT_READITEM		2

#define REG_ITEM_LOGFILE "LogFileName"
#define REG_ITEM_CONFIGFILE "ConfigFileName"
#define REG_ITEM_LOCALADDR "SenderAddress"
#define REG_ITEM_COMMUNITY "Community"

#define INI_ITEM_STARTUPAID		"StartUpAlarmId"
#define INI_ITEM_SHUTDOWNAID		"ShutDownAlarmId"
#define INI_ITEM_STARTUPMSG		"StartUpAlarmMsg"
#define INI_ITEM_SHUTDOWNMSG		"ShutDownAlarmMsg"
#define INI_ITEM_LOGFILE "LogLocation"
#define INI_ITEM_HOSTNAME "HostName"
#define INI_ITEM_SAFESTOREFILE "SafeStoreFile"
#define INI_ITEM_COUNT "TriggerNumber"
#define INI_ITEM_TIME "Time"
#define INI_ITEM_ID "Id"
#define INI_ITEM_SYNTAX "Syntax"
#define INI_ITEM_DEST_COUNT "DestNumber"
#define INI_ITEM_ADDR "Address"
#define INI_ITEM_PORT "Port"

#define INI_COMMON_SECTION	"Common"

#define MAX_REG_STRING_LEN 1024
#define MAX_INI_STRING_LEN 1024

namespace ZQ
{
	namespace SNMP
	{

		//Setting items in registry
		struct RegSetting
		{
		private:
			RegistryEx_T	m_reg;

			std::string		m_strConfigFileName;//INI file(full path)
			std::string		m_strLogFileName;	//Log file(full path)
			std::string		m_strSenderAddress;
			std::string		m_strCommunity;


			int				m_error;			//0 to ok, 
		public:

			//constructor and distructor
			RegSetting();

			~RegSetting();

			//return true if get the registry setting, and false the error
			bool Ok();

			//clear errors
			//return error number
			int Clear();

			//get config file name in registry 
			const std::string& ConfigFileName();

			//get log file name in registry
			const std::string& LogFileName();

			//get localhost address in registry
			const std::string& SenderAddress();

			//get read community 
			const std::string& Community();
		};

		//a block in ini section
		struct IniStruct_Trigger
		{
			//regex			reg;	//regular paser

			std::string		syntax;
			std::string		time;
			std::string		id;
		};

		struct IniStruct_DestAddr
		{
			std::string		addr;
			std::string		port;
		};

		//a ini section
		struct IniStruct_Section
		{
			std::string						logfile;
			std::string						hostname;
			std::string						ssfile;
			std::vector<IniStruct_DestAddr>	dests;
			std::vector<IniStruct_Trigger>	triggers;
		};

		//Configration items in ini file
		struct IniSetting : public std::vector<IniStruct_Section>
		{
		private:
			IniFile			m_ini;

			int				m_error;			//0 to ok, 
		public:
			//constructor and distructor
			IniSetting(const std::string& strIniFile);

			~IniSetting();

			
			//return true if get the registry setting, and false the error
			bool Ok();

			//clear errors
			//return error number
			int Clear();

			//members for startup,shutdown notify message
			std::string		m_hostname;
			std::string			m_startupAID;
			std::string		m_strStartUpMsg;
			std::string			m_shutdownAID;
			std::string		m_strShutDownMsg;
			std::vector<IniStruct_DestAddr>	m_dests;
		};
	}
}

//the registry is the first instance
extern ZQ::SNMP::RegSetting g_reg_setting;
#define REG_ITEM g_reg_setting

#endif//_ZQ_LT_COMMON_H_
