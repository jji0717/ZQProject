
#include "snmpwrapper.h"

namespace ZQ
{
	namespace SNMP
	{

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
				//add log 
				glog(ZQ::common::Log::L_ERROR, "Cannot create SnmpInst: %s", this->error_msg(nStatus));
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
			if (SNMP_CLASS_SUCCESS == nStatus)
				return true;

			glog(ZQ::common::Log::L_ERROR, "Trap send error: Status = %s", this->error_msg(nStatus));
			return false;
			
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

			OctetStr community(READ_COMMUNITY);

			st.set_readcommunity(community);

			return true;
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

