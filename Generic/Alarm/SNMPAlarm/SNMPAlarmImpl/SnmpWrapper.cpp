
#include "snmpwrapper.h"
#include "loghandler.h"

namespace ZQ
{
	namespace SNMP
	{
		RegistryEx_T SnmpInst::m_regInst(SNMP_REG_PATH);
		time_t	SnmpInst::m_timeStamp = time(&m_timeStamp);

		SnmpInst::SnmpInst(bool bIpv4)
			:m_pSnmp(NULL)
		{
			Snmp::socket_startup();  // Initialize socket subsystem

			int nStatus;

			if (bIpv4)
				m_pSnmp = new Snmp(nStatus, "0.0.0.0");
			else
				m_pSnmp = new Snmp(nStatus, "::");

			if (nStatus != SNMP_CLASS_SUCCESS)
			{
				Log(LogHandler::L_ERROR, "[SnmpInst::SnmpInst] can not get success snmp class");
			}
		}

		SnmpInst::~SnmpInst()
		{
			if (m_pSnmp)
				delete m_pSnmp;

			m_pSnmp = NULL;
			
			Snmp::socket_cleanup();  // Shut down socket subsystem
		}

		bool SnmpInst::Send(Pdu* pdu, SnmpTarget* st)
		{
			if (NULL == m_pSnmp)
				return false;

			int nStatus = m_pSnmp->trap(*pdu, *st);
			return (SNMP_CLASS_SUCCESS == nStatus);
		}

		bool SnmpInst::Ok()
		{
			return (NULL != m_pSnmp);
		}

		bool SnmpInst::GetPdu(Pdu& pdu, const std::string& strAddr)
		{
			pdu.set_notify_id(NOTIFY_ID);

			Oid ent(ENTERPRISE);
			pdu.set_notify_enterprise(ent);

			UdpAddress addSender(strAddr.c_str());
			pdu.set_v1_trap_address(addSender);

			time_t curtime;
			time(&curtime);
			double lftime = difftime(curtime, m_timeStamp);

			unsigned long ultime = lftime * 100;

			TimeTicks tt(ultime);
			pdu.set_notify_timestamp(tt);

			return true;
		}

		bool SnmpInst::GetTarget(CTarget& st, snmp_version version)
		{
			st.set_version(version);

			char strReadCommunity[MAX_REG_KEY_STR_LENGTH] = {0};
			if (!m_regInst.LoadStr(REG_READ_COMMUNITY_KEY, strReadCommunity, MAX_REG_KEY_STR_LENGTH))
				return false;
			if ('\0' == strReadCommunity[0])
				return false;

			OctetStr community(strReadCommunity);

			st.set_readcommunity(community);

			return true;
		}

		const char* SnmpInst::error_msg(int status)
		{
			return m_pSnmp->error_msg(status);
		}

		std::string SnmpInst::getAddress()
		{
			char strAddress[MAX_REG_KEY_STR_LENGTH] = {0};
			m_regInst.LoadStr(REG_ADDRESS_KEY, strAddress, MAX_REG_KEY_STR_LENGTH);

			return strAddress;
		}
	}
}

/*
#ifdef IPV4
ZQ::SNMP::SnmpInst g_snmp_inst(true);
#else
ZQ::SNMP::SnmpInst g_snmp_inst(false);
#endif
*/

