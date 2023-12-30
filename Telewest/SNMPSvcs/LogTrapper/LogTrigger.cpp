
#include "logtrigger.h"
#include "formatstr.h"


bool g_bRun = true;


namespace ZQ
{
	namespace SNMP
	{
		SnmpInst g_snmp_inst;
		
		SnmpInst* SnmpTrigger::si = &g_snmp_inst;
		IniSetting*	SnmpTrigger::is = NULL;
		
		ZQ::common::Mutex SnmpTrigger::reslock;
		
		//bool SnmpTrigger::trigged = false;
		int	SnmpTrigger::nSeqNum = 0;
		
		SnmpTrigger::SnmpTrigger(const std::string& strLogFile, int nSection, const std::string& strSSFile)
			:ZQ::common::TailTrigger(strLogFile.c_str(), strSSFile.c_str()),
			m_nSection(nSection)
		{
			glog(ZQ::common::Log::L_INFO, "SnmpTrigger() for %s", strLogFile.c_str());
			_ftLastTrap.dwLowDateTime = 0;			
			_ftLastTrap.dwHighDateTime = 0;
		}
		
		SnmpTrigger::~SnmpTrigger()
		{
			glog(ZQ::common::Log::L_INFO, "~SnmpTrigger()");
		}
		
		bool SnmpTrigger::OnAction(const char* strFileName, 
			const char* strSyntax, const char* strLine, 
			const std::vector<std::string>& arrParam, int nBlock)
		{
			ZQ::common::Guard<ZQ::common::Mutex>	_opLock(reslock);
			
			glog(ZQ::common::Log::L_INFO, "OnAction: File: %s\nLine: %s\n", strFileName, strLine);
	
			std::string strTime = FormatString((*is)[m_nSection].triggers[nBlock].time.c_str(), arrParam);
			
			//
			// keep the last trap and compare with current one, if the current trap time is older, skip and give a warning,
			// if the current trap time is same with the last and trap message also same with last trap, skip and give a warning
			time_t curtimeStamp;
			{
				FILETIME ft;				
				SYSTEMTIME st;			
				GetLocalTime(&st);
				int nYear, nMon, nDay, nHour, nMin, nSec, nMilliseconds;
				sscanf(strTime.c_str(), "%2d/%2d %2d:%2d:%2d:%3d", &nMon, &nDay, &nHour, &nMin, &nSec, &nMilliseconds);
				
				if(nMon > st.wMonth)//last year
				{
					nYear = st.wYear - 1;
				}
				else
				{
					nYear = st.wYear;
				}
				st.wYear = nYear;
				st.wMonth = nMon;
				st.wDay = nDay;
				st.wHour = nHour;
				st.wMinute = nMin;
				st.wSecond = nSec;
				st.wMilliseconds = nMilliseconds;
				SystemTimeToFileTime(&st, &ft);
			
				if (_ftLastTrap.dwLowDateTime || _ftLastTrap.dwHighDateTime)
				{
					int nRet = CompareFileTime(&ft, &_ftLastTrap);
					if (nRet==0)
					{
						if (_strLastTrap == strLine)
						{
							//warning
							glog(ZQ::common::Log::L_WARNING, "[%s] OnAction skipped, message is same with last one", strFileName);
							return false;
						}
					}
					else if (nRet < 0)
					{
						//warning
						glog(ZQ::common::Log::L_WARNING, "[%s] OnAction skipped, message timestamp is older than last one", strFileName);
						return false;
					}
				}

				_ftLastTrap = ft;
				_strLastTrap = strLine;

				FILETIME ft_utc;
				LocalFileTimeToFileTime(&ft, &ft_utc);
				LONGLONG ll = *((LONGLONG*)(&ft_utc));
				ll -= 116444736000000000;
				curtimeStamp =  (time_t)(ll/10000000);    				
			}
			
			Pdu pdu;
			
			Vb vb1,vb2,vb3,vb4,vb5;
			
			vb1.set_oid(SEQ_NO_ID);
			Counter32 seq(nSeqNum++);
			vb1.set_value(seq);
			
			std::string strAlarmId = FormatString((*is)[m_nSection].triggers[nBlock].id.c_str(), arrParam);
			vb2.set_oid(ALARM_ID);
			vb2.set_value(atoi(strAlarmId.c_str()));
			
			vb3.set_oid(TIME_ID);
			vb3.set_value((int)curtimeStamp);
			
			std::string strHost = FormatString((*is)[m_nSection].hostname.c_str(), arrParam);
			vb4.set_oid(HOST_ID);
			vb4.set_value(strHost.c_str());
			
			
			vb5.set_oid(TEXT_ID);
			vb5.set_value(strLine);
			
			pdu += vb1;
			pdu += vb2;
			pdu += vb3;
			pdu += vb4;
			pdu += vb5;
			
			SnmpInst::GetPdu(pdu, REG_ITEM.SenderAddress().c_str());
			
			for (std::vector<IniStruct_DestAddr>::iterator itorAddr = (*is)[m_nSection].dests.begin();
			itorAddr < (*is)[m_nSection].dests.end(); ++itorAddr)
			{
				std::string strDestAddr = FormatString(itorAddr->addr.c_str(), arrParam);
				std::string strDestPort = FormatString(itorAddr->port.c_str(), arrParam);
				//
				//pdu.set_maxsize_scopedpdu(10000);
				//printf("%d", pdu.get_maxsize_scopedpdu());
				
				UdpAddress addTrg(strDestAddr.c_str());
				addTrg.set_port(atoi(strDestPort.c_str()));
				CTarget ctg(addTrg);
				SnmpInst::GetTarget(ctg);
				
				SnmpTarget* st = &ctg;
				
				bool bSuccess = si->Send(&pdu, st);
				glog(ZQ::common::Log::L_INFO, "[%s] OnAction: Trap message sent to %s:%s",strFileName,strDestAddr.c_str(),strDestPort.c_str());
			}
			
			return true;
		}
	
		bool SnmpTrigger::StartUpNotify()
		{
			reslock.enter();	// enter mutex, begin SNMP work
			
			glog(ZQ::common::Log::L_DEBUG, "StartUpNotify...");
			
			Pdu pdu;
			
			{
				Vb vb1,vb2,vb3,vb4,vb5;			
			
				vb1.set_oid(SEQ_NO_ID);
				Counter32 seq(nSeqNum++);
				vb1.set_value(seq);
				
				std::string strAlarmId = is->m_startupAID;
				vb2.set_oid(ALARM_ID);
				vb2.set_value(atoi(strAlarmId.c_str()));
				
				time_t curtime;
				time(&curtime);
				vb3.set_oid(TIME_ID);
				vb3.set_value((int)curtime);
				
				std::string strHost = is->m_hostname;
				vb4.set_oid(HOST_ID);
				vb4.set_value(strHost.c_str());
				
				std::string strMsg = is->m_strStartUpMsg;//
				vb5.set_oid(TEXT_ID);
				vb5.set_value(strMsg.c_str());
				
				pdu += vb1;
				pdu += vb2;
				pdu += vb3;
				pdu += vb4;
				pdu += vb5;
			}
			
			SnmpInst::GetPdu(pdu, REG_ITEM.SenderAddress().c_str());
			
			for (std::vector<IniStruct_DestAddr>::iterator itorAddr = is->m_dests.begin();
			itorAddr < is->m_dests.end(); ++itorAddr)
			{
				std::string strDestAddr = itorAddr->addr;
				std::string strDestPort = itorAddr->port;
				//
				//pdu.set_maxsize_scopedpdu(10000);
				//printf("%d", pdu.get_maxsize_scopedpdu());
				
				UdpAddress addTrg(strDestAddr.c_str());
				addTrg.set_port(atoi(strDestPort.c_str()));
				CTarget ctg(addTrg);
				SnmpInst::GetTarget(ctg);
				
				SnmpTarget* st = &ctg;
				
				bool bSuccess = si->Send(&pdu, st);
			}
			
			reslock.leave();	// leave mutex, end SNMP work

			return true;
		}

		bool SnmpTrigger::ShutDownNotify()
		{
			reslock.enter();	// enter mutex, begin SNMP work
			
			glog(ZQ::common::Log::L_DEBUG, "ShutDownNotify...");
			
			Pdu pdu;
			
			Vb vb1,vb2,vb3,vb4,vb5;
			
			vb1.set_oid(SEQ_NO_ID);
			Counter32 seq(nSeqNum++);
			vb1.set_value(seq);
			
			std::string strAlarmId = is->m_shutdownAID;
			vb2.set_oid(ALARM_ID);
			vb2.set_value(atoi(strAlarmId.c_str()));
			
			time_t curtime;
			time(&curtime);
			vb3.set_oid(TIME_ID);
			vb3.set_value((int)curtime);
			
			std::string strHost = is->m_hostname;
			vb4.set_oid(HOST_ID);
			vb4.set_value(strHost.c_str());
			
			std::string strMsg = is->m_strShutDownMsg;
			vb5.set_oid(TEXT_ID);
			vb5.set_value(strMsg.c_str());
			
			pdu += vb1;
			pdu += vb2;
			pdu += vb3;
			pdu += vb4;
			pdu += vb5;
			
			SnmpInst::GetPdu(pdu, REG_ITEM.SenderAddress().c_str());
			
			for (std::vector<IniStruct_DestAddr>::iterator itorAddr = is->m_dests.begin();
			itorAddr < is->m_dests.end(); ++itorAddr)
			{
				std::string strDestAddr = itorAddr->addr;
				std::string strDestPort = itorAddr->port;
				//
				//pdu.set_maxsize_scopedpdu(10000);
				//printf("%d", pdu.get_maxsize_scopedpdu());
				
				UdpAddress addTrg(strDestAddr.c_str());
				addTrg.set_port(atoi(strDestPort.c_str()));
				CTarget ctg(addTrg);
				SnmpInst::GetTarget(ctg);
				
				SnmpTarget* st = &ctg;
				
				bool bSuccess = si->Send(&pdu, st);
			}
			
			reslock.leave();	// leave mutex, end SNMP work

			return true;
		}
		
		Triggers::Triggers(const std::string& strConfigFile)
			:m_is(REG_ITEM.ConfigFileName())
		{
			
			glog(ZQ::common::Log::L_DEBUG, "Read configration items from config file", strConfigFile.c_str());
			
			SnmpTrigger::is = &m_is;
			
			for (int i = 0; i < m_is.size(); ++i)
			{
				SnmpTrigger* pStTemp = new SnmpTrigger(m_is[i].logfile, i, m_is[i].ssfile);
				for (int j = 0; j < m_is[i].triggers.size(); ++j)
				{
					pStTemp->push_back(m_is[i].triggers[j].syntax);
					
					glog(ZQ::common::Log::L_DEBUG, "Add a syntax line(File: %s): %s\n", m_is[i].logfile.c_str(), m_is[i].triggers[j].syntax.c_str());
				}
				this->push_back(pStTemp);
			}
		}
		
		Triggers::~Triggers()
		{
			std::vector<SnmpTrigger*>::iterator itor;
			for (itor = this->begin();itor != this->end(); ++itor)
			{
				if (*itor!=NULL)
					delete (*itor);
			}
		}
		
		bool Triggers::Ok()
		{
			return m_is.Ok();
		}
		
		void Triggers::run()
		{			
			glog(ZQ::common::Log::L_DEBUG, "Triggers::run() entered");

			SnmpTrigger::StartUpNotify();

			std::vector<SnmpTrigger*>::iterator itor;
			for (itor = this->begin();itor != this->end(); ++itor)
			{
				if (!(*itor)->run())
				{
					//Add log
					glog(ZQ::common::Log::L_ERROR, "Cannot run a trigger");
				}
			}			
			
			while (g_bRun)
			{
				Sleep(250);
			}

			for (itor = this->begin();itor != this->end(); ++itor)
			{
				(*itor)->stop();
			}
			
			for (itor = this->begin();itor != this->end(); ++itor)
			{
				(*itor)->unInit();
			}

			SnmpTrigger::ShutDownNotify();
			glog(ZQ::common::Log::L_DEBUG, "Triggers::run() exited");
		}
	}
}
