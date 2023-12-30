
#ifndef _ZQ_LOG_TRIGGER_H_
#define _ZQ_LOG_TRIGGER_H_

#include "snmpwrapper.h"
#include "tailtrigger.h"
#include "Locks.h"

#pragma comment(lib, "snmp_pp.lib")

//true to run and false to exit
extern bool g_bRun;

namespace ZQ
{
	namespace SNMP
	{
		//a log trigger instance
		class SnmpTrigger: public ZQ::common::TailTrigger
		{
		public:
			static SnmpInst*	si;
			static IniSetting*	is;
			//static bool			trigged;

			/// the static trap sequence
			static int			nSeqNum;

			/// the mutex object to prevent deadlock between threads
			static ZQ::common::Mutex	reslock;

		private:
			//in which ini section
			int		m_nSection;
			FILETIME	_ftLastTrap;
			std::string _strLastTrap;
		public:
			//constructor and distructor
			SnmpTrigger(const std::string& strLogFile, int nSection, const std::string& strSSFile);

			~SnmpTrigger();

			static bool StartUpNotify();
			static bool ShutDownNotify();

			//[call back function]
			//if dected a log line which match the syntax line
			//strFileName -- log file name
			//strSyntax -- syntax line
			//strLine -- current log line
			//arrParam -- the paraments which defined in ini file
			//nBlock -- in which ini block
			bool OnAction(const char* strFileName, const char* strSyntax, const char* strLine,
				const std::vector<std::string>& arrParam, int nBlock);
		};

		//triggers pool
		class Triggers: public std::vector<SnmpTrigger*>
		{
		private:
			IniSetting		m_is;
		public:
			//constructor and distructor
			Triggers(const std::string& strConfigFile);

			~Triggers();

			//return true if the system is all right
			bool Ok();

			//run the triggers
			//it is the main function for user interface
			void run();
		};
	}
}

#endif//_ZQ_LOG_TRIGGER_H_
