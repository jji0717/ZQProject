
#ifndef _ZQ_SNMP_WRAPPER_H_
#define _ZQ_SNMP_WRAPPER_H_

#include "SNMPLib\snmp_pp.h"
#include "registryex.h"

#include <string>

//#define READ_COMMUNITY "public"

#define NOTIFY_ID "1.3.6.1.4.1.60000.3.3"
#define SEQ_NO_ID "1.3.6.1.4.1.60000.3.5.1"
#define ALARM_ID "1.3.6.1.4.1.60000.3.5.2"
#define TIME_ID "1.3.6.1.4.1.60000.3.5.3"
#define HOST_ID "1.3.6.1.4.1.60000.3.5.4"
#define TEXT_ID "1.3.6.1.4.1.60000.3.5.5"

#define ENTERPRISE "1.3.6.1.4.1.1971"

#define SNMP_REG_PATH "Software\\ZQ\\SNMPSvcs"

#define MAX_REG_KEY_STR_LENGTH 256
#define REG_READ_COMMUNITY_KEY "ReadCommunity"
#define REG_ADDRESS_KEY	"Address"

namespace ZQ
{
	namespace SNMP
	{
		//SNMP++ wrapper
		class SnmpInst
		{
		private:
			Snmp*					m_pSnmp;

			static RegistryEx_T		m_regInst;
			static time_t			m_timeStamp;

		public:
			//constructor and distructor
			SnmpInst(bool bIpv4 = true);
			~SnmpInst();
			
			//send data by SNMP
			bool Send(Pdu* pdu, SnmpTarget* st);

			//returnt true if no error exists
			bool Ok();
			
			//add some datas into pdu structure
			static bool GetPdu(Pdu& pdu, const std::string& strAddr);
			
			//add some datas into target structure
			static bool GetTarget(CTarget& st, snmp_version version = version1);

			//get error message
			const char* error_msg(int status);

			static std::string getAddress();
		};
		
	}
}

//extern ZQ::SNMP::SnmpInst g_snmp_inst;
//#define SENDER g_snmp_inst

#endif//_ZQ_SNMP_WRAPPER_H_
