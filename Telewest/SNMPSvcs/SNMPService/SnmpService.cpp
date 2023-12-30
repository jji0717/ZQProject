
#include <baseschangeserviceapplication.h>
#include "..\LogTrapper\logtrigger.h"
#include "log.h"
//ZQ::common::Log * pGlog;
bool g_bRunning = false;

DWORD WINAPI SNMP_MAIN(void* pMain)
{
	g_bRunning = true;
	glog(ZQ::common::Log::L_DEBUG, "Enter SNMP_MAIN");
	
	{
		ZQ::SNMP::Triggers trg(REG_ITEM.ConfigFileName());
	
		if (trg.Ok())
		{
			//Log: run Triggers
			glog(ZQ::common::Log::L_DEBUG, "Run Triggers");
			trg.run();
		}		
	}
	g_bRunning = false;
	
	//Log: Can not enter trigger
	glog(ZQ::common::Log::L_DEBUG, "Leave SNMP_MAIN");
	return 0;
}

namespace ZQ
{
	namespace SNMP
	{
		class SnmpService: public ZQ::common::BaseSchangeServiceApplication
		{	
		private:
			HANDLE	m_hThread;
			DWORD	m_dwTrdId;
		public:
			SnmpService()
				:m_hThread(NULL)
			{
			}
			
			virtual ~SnmpService()
			{
				if (NULL != m_hThread)
				{
					//close thread
					//and set m_hThread to NULL
					OnStop();

					m_hThread = NULL;
				}
			}
			
			HRESULT OnInit(void)
			{	
				BaseSchangeServiceApplication::OnInit();
				ZQ::common::pGlog = m_pReporter;
				
				glog(ZQ::common::Log::L_INFO, _T("SnmpSvcs Service Initialized"));
//				glog(ZQ::common::Log::L_INFO, _T("Initlize Service [Start]"));
//				
//				glog(ZQ::common::Log::L_INFO, "Initlize Service [End]");
				return S_OK;
			}
			
			HRESULT OnStop(void)
			{
				BaseSchangeServiceApplication::OnStop();

				g_bRun = false;

				glog(ZQ::common::Log::L_INFO, _T("SnmpSvcs Service Stopping..."));
				return S_OK;
			}
			
			HRESULT OnPause(void)
			{
				glog(ZQ::common::Log::L_INFO, "Pause Service [Start]");
				
				DWORD dwNo = SuspendThread(m_hThread);
				BaseSchangeServiceApplication::OnPause();
				
				
				if (-1 != dwNo)
				{
					
					glog(ZQ::common::Log::L_INFO, "Pause Service [End]");
					return S_OK;
				}
				
				
				glog(ZQ::common::Log::L_ERROR, "Can not Pause Service");
				
				return -1;
			}
			
			HRESULT OnContinue(void)
			{
				
				glog(ZQ::common::Log::L_INFO, "Continue Service [Start]");
				
				DWORD dwNo = ResumeThread(m_hThread);
				BaseSchangeServiceApplication::OnContinue();
				
				
				
				if (-1 != dwNo)
				{
					
					glog(ZQ::common::Log::L_INFO, "Continue Service [End]");
					return S_OK;
				}
				
				
				glog(ZQ::common::Log::L_ERROR, "Can not Continue Service");
				
				return -1;
			}
			
			HRESULT OnStart(void)
			{
				BaseSchangeServiceApplication::OnStart();
				
				m_hThread = CreateThread(NULL, 0, SNMP_MAIN, NULL, 0, &m_dwTrdId);
				if (NULL != m_hThread)
				{
					glog(ZQ::common::Log::L_INFO, _T("SnmpSvcs Service Started"));
					return S_OK;
				}
				
				glog(ZQ::common::Log::L_ERROR, _T("Create Thread: SupplierMain - FAIL"));
				return -1;
			}
			
			HRESULT OnUnInit(void)
			{
				if (m_hThread)
				{
					DWORD dwTimeout = 6000;
					glog(ZQ::common::Log::L_INFO, "Waiting for SNMP_MAIN thread 0x%04x exit", m_dwTrdId);
					DWORD dwRet = WaitForSingleObject(m_hThread, dwTimeout);
				
					if (dwRet == WAIT_TIMEOUT || g_bRunning)
					{
						glog(ZQ::common::Log::L_INFO, "Waiting for SNMP_MAIN thread 0x%04x exit timeout in %d ms, terminate thread", m_dwTrdId, dwTimeout);
						TerminateThread(m_hThread, 1);
					}

					CloseHandle(m_hThread);
					m_hThread = NULL;	
					glog(ZQ::common::Log::L_INFO, "End of waiting for SNMP_MAIN thread 0x%04x exit", m_dwTrdId);
				}								
				
				BaseSchangeServiceApplication::OnUnInit();
				
				glog(ZQ::common::Log::L_INFO, "SnmpSvcs Service Uninitialized");

				return S_OK;
			}
		};
	}
}

extern DWORD gdwServiceType = 11;
ZQ::SNMP::SnmpService ssApp;
ZQ::common::BaseSchangeServiceApplication * Application = &ssApp;
unsigned long  gdwServiceInstance = 11;
