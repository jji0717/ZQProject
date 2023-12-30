#include "stdafx.h"
#include "objbase.h"

#include "MPFLogHandler.h"
USE_MPF_NAMESPACE

#include "MPFCommon.h"
using namespace ZQ::MPF::utils;

#include "RpcWpClient.h"

#include "Daemon.h"	
#include "ZQResource.h"

MPF_WORKNODE_NAMESPACE_BEGIN

HbTrigger::HbTrigger(Daemon& daemon)
:_daemon(daemon)
{
	
}

HbTrigger::~HbTrigger()
{
}

bool HbTrigger::init()
{
	return NativeThread::init();
}

int HbTrigger::run()
{
	while(!_daemon.m_bStop)
	{
		if(_daemon.m_LeaseTerm==-1)	// heartbeat is disabled
		{
			::Sleep(MPF_MINIMUM_HEARTBEAT_TIME);
			continue;
		}

		_daemon.heartbeat();

		if(WAIT_OBJECT_0 ==WaitForSingleObject(_daemon.m_hWakeUp, _daemon.m_LeaseTerm*MPF_DEFAULT_LEASE_FACTOR))
		{
			return 1;
		}
		
	}
	
	return 0;
}

void HbTrigger::final()
{
	NativeThread::final();
}

Daemon::Daemon(const char* worknodeUrl)
{
	ZeroMemory(m_WorkNodeID,MPF_NODEID_LEN);
	
	m_pTaskAcceptor = NULL;
	m_pHeartbeat = NULL;
	m_bStop = FALSE;
	m_nInterfaceIndex = -1;

	//assign work node ip & port
	URLStr validUrl(worknodeUrl);
	if(strlen(validUrl.getProtocol())==0)
	{
		validUrl.setProtocol("MPF");
	}
	if(strlen(validUrl.getHost())==0)
	{
		validUrl.setHost("0.0.0.0");
	}
	if(validUrl.getPort()==0)
	{
		validUrl.setPort(MPF_DEFAULT_WORKNODE_TAPORT);
	}
	m_WorkNodeURL = validUrl.generate();

	SystemInfo::Init();
	
	m_nInterfaceIndex = SystemInfo::NetworkGetInterfaceIndexByIP(validUrl.getHost());

	m_hWakeUp = ::CreateEvent(NULL, true, false, NULL);
//	::ResetEvent(m_hWakeUp);

	ZQ::MPF::utils::GetNodeGUID(m_WorkNodeID,MPF_NODEID_LEN);

	m_LeaseTerm=MPF_DEFAULT_HEARTBEAT_TIME;

}


Daemon::~Daemon()
{
	SystemInfo::UnInit();
	
	if(!m_bStop)
	{
		stop();
	}
	::CloseHandle(m_hWakeUp);
	if(m_pHeartbeat)
	{
		delete m_pHeartbeat;
		m_pHeartbeat = NULL;
	}
}

bool Daemon::query(const char* mgmURL, const RpcValue& param, RpcValue& result)
{
	if (NULL == mgmURL)
		return false;

	URLStr mgmUrl(mgmURL);
	string strManageNodeIp	=mgmUrl.getHost();
	int nManageNodePort		=mgmUrl.getPort();

	RpcClient c(strManageNodeIp.c_str(), nManageNodePort);
			
	c.setResponseTimeout(MPF_DEFAULT_WORKNODE_TIME_OUT);
	
	return c.execute(QUERY_METHOD, param, result);
}

bool Daemon::addMgmNode(const char* mgmURL)
{
	bool bRet=TRUE;

	for(int i=0; i<m_ManageNodes.size(); i++)
	{
		if(m_ManageNodes[i]==mgmURL) {
			bRet = FALSE;
			break;
		}
	}
	
	if(bRet)
	{
		m_ManageNodes.push_back(mgmURL);
		MPFLog(MPFLogHandler::L_NOTICE, "New manager node %s added", mgmURL);
	}
	
	return bRet;
}

bool Daemon::removeMgmNode(const char* mgmURL)
{
	bool bRet=FALSE;

	for(int i=0; i<m_ManageNodes.size(); i++)
	{
		if(m_ManageNodes[i]==mgmURL) {
			bRet = TRUE;
			break;
		}
	}
	
	if(bRet)
	{
		m_ManageNodes.erase(m_ManageNodes.begin()+i);
	}
	
	return bRet;
}

int Daemon::heartbeat()
{
	try
	{
		if(m_bStop)return -1;
		
		RpcValue hbAttr;
		
		if(*m_WorkNodeID==0)
		{
			MPFLog(MPFLogHandler::L_ERROR,"Daemon::heartBeat() Work Node ID is null,exit daemon thread\n");
			return -1;
		}
		//////////////////////////////////////////////////////////////////////////
		// prepare data
		//////////////////////////////////////////////////////////////////////////
		
		// construct work node url
		URLStr wnurl(m_WorkNodeURL.c_str());
		wnurl.setPath(URL_PATH_WORKNODE);
		wnurl.setVar(URL_VARNAME_WORKNODE_ID, m_WorkNodeID);

		hbAttr.SetStruct(WORKNODE_ID_KEY, RpcValue(wnurl.generate()));
		
		RpcValue TaskTypeArray;

		if(!m_pTaskAcceptor)
		{
			//exit thread
			MPFLog(MPFLogHandler::L_DEBUG,"Daemon::heartBeat() No Task Acceptor registered\n");

		}
		else
		{
		
			map<string, workCount> tmpList=m_pTaskAcceptor->getTaskTypeVector();
			map<string, workCount>::const_iterator iter;
			int i;
			for(iter=tmpList.begin(),i=0; iter!=tmpList.end(); iter++, i++)
			{
				RpcValue TaskType;
				std::string typestr  = iter->first;
				int			acount	 = iter->second.availCount;
				int			rcount	 = iter->second.totalCount - acount;
				
				TaskType.SetStruct(TYPE_NAME_KEY,		RpcValue(typestr.c_str()));
				TaskType.SetStruct(RUN_INSTANCE_KEY,	RpcValue(rcount));
				TaskType.SetStruct(AVBLE_INSTANCE_KEY,	RpcValue(acount));
				
				TaskTypeArray.SetArray(i,TaskType);
			}
		}
		
		hbAttr.SetStruct(TASK_TYPES_KEY,TaskTypeArray);
		
		//sys info
		RpcValue sysAttr;

		string strCPUInfo	= SystemInfo::CPUGetFixInfo();	// cpu type
		sysAttr.SetStruct(CPU_KEY, RpcValue(strCPUInfo.c_str()));

		long totalMem		= SystemInfo::MemoryGetTotal();	// total memory
		sysAttr.SetStruct(TOTAL_MEMORY_KEY,RpcValue(totalMem));

		long cpuUsage=SystemInfo::CPUGetUsage();			// cpu usage
		sysAttr.SetStruct(CPU_USAGE_KEY, RpcValue(cpuUsage));

		long memUsage		=SystemInfo::MemoryGetAvailable();	// available memory
		sysAttr.SetStruct(MEMORY_USAGE_KEY, RpcValue(memUsage));

		double curTraffic		=SystemInfo::NetworkGetInterfaceTraffic(m_nInterfaceIndex);	// interface traffic
		sysAttr.SetStruct(CURRENT_TRAFFIC_KEY, RpcValue(curTraffic));

		long bandwidth		=SystemInfo::NetworkGetInterfaceBandwidth(m_nInterfaceIndex);	// bandwidth
		sysAttr.SetStruct(BAND_WIDTH_KEY, RpcValue(bandwidth));
		
		sysAttr.SetStruct(MPF_VERSION_KEY, RpcValue(ZQ_PRODUCT_VER_STR3));		// MPF version
		
		string strOSInfo	=SystemInfo::OSGetVersion();	// OS info
		sysAttr.SetStruct(OS_KEY, RpcValue(strOSInfo.c_str()));
		
		hbAttr.SetStruct(WORKNODE_INFO_KEY,sysAttr);
		
		// user defined info
		RpcValue UserAttr;
		OnUserHeartbeat(UserAttr);

		hbAttr.SetStruct(USER_HBINFO_KEY, UserAttr);
		
		//////////////////////////////////////////////////////////////////////////
		// send info to every manage node according to different ip
		//////////////////////////////////////////////////////////////////////////
		
		for(int idx=0; idx<m_ManageNodes.size(); idx++)
		{
			if(m_ManageNodes[idx].empty())continue;
			
			URLStr mgmUrl(m_ManageNodes[idx].c_str());
			string strManageNodeIp	=mgmUrl.getHost();
			int nManageNodePort		=mgmUrl.getPort();
			
			if(nManageNodePort==0 || strManageNodeIp.empty())continue;
			
			RpcClient c(strManageNodeIp.c_str(), nManageNodePort);
			
			c.setResponseTimeout(MPF_DEFAULT_WORKNODE_TIME_OUT);
			RpcValue retAttr;
			
			/*
			char strTemp[512] = {0};
			hbAttr.toXml(strTemp, 512);
			printf("!!!%s\n", strTemp);
			hbAttr.print(print_screen);
			*/
			if (c.execute(HEARTBEAT_METHOD, hbAttr, retAttr))
			{
				if(retAttr.getType()==retAttr.TypeStruct)
				{
					int newlease=(int)retAttr[LEASE_TERM_KEY];
					if(newlease && newlease>MPF_MINIMUM_HEARTBEAT_TIME && newlease<m_LeaseTerm)
					{
						m_LeaseTerm = newlease;
						MPFLog(MPFLogHandler::L_DEBUG, "Daemon::heartBeat() Lease time out has been updated to %d msec", newlease);
					}

				}
				
				c.close();
			}
			else
			{
				MPFLog(MPFLogHandler::L_WARNING,"Daemon::HeartBeat() heart Beat fail because of rpc error\n");
			}
		}
		
	}
	catch(...)
	{
		MPFLog(MPFLogHandler::L_ERROR,"Daemon::heartBeat() Got exception\n");
	}
	
	return 0;
}

bool Daemon::init()
{
	if(!m_pHeartbeat)
		m_pHeartbeat = new HbTrigger(*this);

 	try
 	{
		URLStr wnUrl(m_WorkNodeURL.c_str());
		int bindPort = wnUrl.getPort();
 		if(bindPort <=0 || !bindAndListen(bindPort))
 		{

			MPFLog(MPFLogHandler::L_ERROR,"Daemon::init() Failed to listen on port %d\n", bindPort);
 
 			return false;
 		}
 	}
 	catch(...) { return false; }

	return NativeThread::init();
}

int Daemon::run()
{
	MPFLog(MPFLogHandler::L_NOTICE, "Work node Daemon started");
	m_pHeartbeat->start();
	work(-1.0);
	return 0;
}

void Daemon::final()
{
	NativeThread::final();
}

void Daemon::stop()
{
	m_bStop = TRUE;
	::SetEvent(m_hWakeUp);
	MPFLog(MPFLogHandler::L_NOTICE, "Work node Daemon stopped");
	RpcServer::exit();
}

void Daemon::regAcceptor(TaskAcceptor* pAcceptor)
{
	if(!pAcceptor)return;
	
	// init RPCServer
	addMethod(pAcceptor);
	enableIntrospection(true);

	m_pTaskAcceptor = pAcceptor;
}


MPF_WORKNODE_NAMESPACE_END
