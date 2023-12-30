#include "IpList.h"

#include "Log.h"
#include "ScLog.h"

using namespace ZQ::common;

IPList::IPList(CStringList& ip, CStringList& port)
{
	m_currentCount = ip.GetCount();
	m_ipList = new IP[m_currentCount];

	for (int i = 0; i < m_currentCount; i++)
	{
		//copy ip
		memset(m_ipList[i].m_ip, 0x00, 20*sizeof(char));
		//strcpy(m_ipList[i].m_ip, ip.GetHead());
		wcstombs(m_ipList[i].m_ip, ip.GetHead(), ip.GetHead().GetLength());
		
		//copy port
		//m_ipList[i].m_port = atoi(port.GetHead());
		m_ipList[i].m_port = _wtol(port.GetHead());
		
		glog(Log::L_DEBUG, "Ticp: IP<%s>, Port<%d>", m_ipList[i].m_ip, 
													 m_ipList[i].m_port);

		//set reference 0
		m_ipList[i].m_nRefs = 0;

		ip.RemoveHead();
		port.RemoveHead();
	}
}

IPList::~IPList()
{
	if (m_currentCount > 0)
	{
		if (m_ipList)
		{
			delete m_ipList;
			m_ipList = NULL;
		}
	}
}

int IPList::GetIP(char* ip, long& port)
{
	int min = 0;
	for (int i = 0; i < m_currentCount; i++)
	{
		if (m_ipList[i].m_nRefs < m_ipList[min].m_nRefs)
		{
			min = i;
		}
	}
	// copy ip
	strcpy(ip, m_ipList[min].m_ip);

	// copy port
	port = m_ipList[min].m_port;
	
	// reference add 1
	m_ipList[min].m_nRefs ++;
	
	return min;
}

void IPList::ReleaseIP(int index)
{
	if (m_ipList[index].m_nRefs > 0)
	{
		m_ipList[index].m_nRefs --;
	}
}