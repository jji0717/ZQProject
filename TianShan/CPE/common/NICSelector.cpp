
#include "NICSelector.h"


bool Comp(NetworkIFSelector::InterfaceInfo first, NetworkIFSelector::InterfaceInfo second)
{
	return float(first.nUsedBandwdith)/float(first.nTotalBandwidth) < float(second.nUsedBandwdith)/float(second.nTotalBandwidth);
}

NetworkIFSelector::NetworkIFSelector(ZQ::common::Log& log)
:_log(log)
{
}

void NetworkIFSelector::listInterfaceInfo( InterfaceInfoList& infoList )
{
	infoList = _interfacelist;
}

bool NetworkIFSelector::allocInterface( int nBandwidth, std::string& strIp,std::string contentName)
{
	ZQ::common::MutexGuard gd(_lock);
	if (contentName.empty())
	{
		return false;
	}

	InterfaceInfoList templist;
	templist.assign(_interfacelist.begin(),_interfacelist.end());
	for (InterfaceInfoList::iterator iter = templist.begin(); iter != templist.end(); iter++)
	{
		(*iter).nUsedBandwdith += nBandwidth;
		if ((*iter).nTotalBandwidth == 0)
			(*iter).nTotalBandwidth = 1000000000;
	}
	std::sort(templist.begin(),templist.end(),Comp);

	if (templist.size() == 0)
		return false;

	InterfaceInfoList::iterator minpos = templist.begin();
	while (minpos!= templist.end() && float((*minpos).nUsedBandwdith)/float((*minpos).nTotalBandwidth) > 1)
	{
		minpos++;
	}
	if (minpos != templist.end())
	{
		strIp = (*minpos).strInteraceIp;
		if (strIp.empty())
			return false;
		for (minpos = _interfacelist.begin();minpos != _interfacelist.end();minpos++)
		{
			if (stricmp((*minpos).strInteraceIp.c_str(),strIp.c_str()) == 0)
			{
				(*minpos).nUsedBandwdith += nBandwidth;
				ContentInfo conInfo;
				conInfo.strInterfaceIp = strIp;
				conInfo.bandwidth = nBandwidth;
				_contentMap.insert(std::make_pair(contentName,conInfo));
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(NetworkIFSelector, "Successfully add bandwidth for content [%s], current load bandwidth is %d "),contentName.c_str(),(*minpos).nUsedBandwdith);
				break;
			}
		}
	}
	else
	{
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(NetworkIFSelector, "[%s]There is no idle network"), contentName.c_str());
		return false;
	}
	return true;
}

bool NetworkIFSelector::freeInterface(std::string contentName)
{
	ZQ::common::MutexGuard gd(_lock);
	ContentMap::iterator iter = _contentMap.find(contentName);
	if (iter == _contentMap.end())
	{
		return false;
	}
	else
	{
		for(InterfaceInfoList::iterator listiter = _interfacelist.begin(); listiter != _interfacelist.end(); listiter++)
		{
			if (stricmp((*listiter).strInteraceIp.c_str(),(*iter).second.strInterfaceIp.c_str()) == 0)
			{
				if ((*listiter).nUsedBandwdith - (*iter).second.bandwidth < 0)
					return false;
				(*listiter).nUsedBandwdith -= (*iter).second.bandwidth;
				_contentMap.erase(iter);
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(NetworkIFSelector, "Successfully free bandwidth for content [%s], current load bandwidth is %d "),contentName.c_str(),(*listiter).nUsedBandwdith);
				break;
			}
		}
		return true;
	}
}

void NetworkIFSelector::addInterface( const std::string& strIp, int nTotalBandwidth )
{
	if (strIp.empty() || !nTotalBandwidth)
		return;

	InterfaceInfo interfInfo;
	interfInfo.strInteraceIp = strIp;
	interfInfo.nTotalBandwidth = nTotalBandwidth;
	interfInfo.nUsedBandwdith = 0;

	ZQ::common::MutexGuard gd(_lock);
	bool bFound = false;
	for(InterfaceInfoList::iterator it = _interfacelist.begin(); it != _interfacelist.end(); it++)
	{
		if (!stricmp(it->strInteraceIp.c_str(), strIp.c_str()))
		{
			bFound = true;
			break;
		}
	}

	if (!bFound)
		_interfacelist.push_back(interfInfo);
}


