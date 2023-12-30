
#ifndef _ZQ_SNMP_WRAPPER_H_
#define _ZQ_SNMP_WRAPPER_H_

#include "ltcommon.h"

//#define READ_COMMUNITY "public"
#define READ_COMMUNITY REG_ITEM.Community().c_str()

#define NOTIFY_ID "1.3.6.1.4.1.60000.3.3"
#define SEQ_NO_ID "1.3.6.1.4.1.60000.3.5.1"
#define ALARM_ID "1.3.6.1.4.1.60000.3.5.2"
#define TIME_ID "1.3.6.1.4.1.60000.3.5.3"
#define HOST_ID "1.3.6.1.4.1.60000.3.5.4"
#define TEXT_ID "1.3.6.1.4.1.60000.3.5.5"


#define ENTERPRISE "1.3.6.1.4.1.1971"

namespace ZQ
{
	namespace SNMP
	{
		//SNMP++ wrapper
		class SnmpInst
		{
		private:
			Snmp*	m_pSnmp;

			static int		m_nSeq;	//snmp seq number
			static time_t	m_timeStamp;
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
			const char* error_msg(int status)
			{
				return m_pSnmp->error_msg(status);
			}
		};
		
	}
}

//extern ZQ::SNMP::SnmpInst g_snmp_inst;
//#define SENDER g_snmp_inst

#endif//_ZQ_SNMP_WRAPPER_H_
