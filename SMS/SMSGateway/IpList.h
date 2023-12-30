#if !defined (IPLIST_H)
#define IPLIST_H

#include "afx.h"

struct IP
{
	char	m_ip[20];
	long	m_port;
	long	m_nRefs;//reference count
};

class IPList
{
public:
	IPList(CStringList& ip, CStringList& port);
	~IPList();

	int GetIP(char* ip, long& port);
	void ReleaseIP(int index);

private:
	IP* m_ipList;
	int m_currentCount;
};

#endif