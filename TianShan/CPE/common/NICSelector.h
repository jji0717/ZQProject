#ifndef _NIC_SELECTOR_H
#define _NIC_SELECTOR_H

#include <string>
#include <vector>
#include <algorithm>   
#include "Locks.h"
#include <map>

class NetworkIFSelector
{
public:
	NetworkIFSelector(ZQ::common::Log& log);
	virtual ~NetworkIFSelector(){};

	struct InterfaceInfo
	{
		std::string		strInteraceIp;
		int				nTotalBandwidth;
		int				nUsedBandwdith;
	};
	struct ContentInfo
	{
		std::string strInterfaceIp;
		int  bandwidth;
	};
	typedef std::vector<InterfaceInfo>	InterfaceInfoList;
	typedef std::map<std::string, ContentInfo> ContentMap;

	void addInterface(const std::string& strIp, int nTotalBandwidth);
	void listInterfaceInfo(InterfaceInfoList& infoList);

	bool allocInterface(int nBandwidth, std::string& strIp,std::string contentName);
	bool freeInterface(std::string contentName);
protected:

	ZQ::common::Mutex		_lock;
	InterfaceInfoList       _interfacelist;
	ContentMap              _contentMap;
	ZQ::common::Log&	    _log;

};

#endif


