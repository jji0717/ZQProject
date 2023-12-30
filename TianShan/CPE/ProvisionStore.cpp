
#include "ProvisionStore.h"
#include <utility>

ProvisionStore::ProvisionStore(void)
{
}

ProvisionStore::~ProvisionStore(void)
{
}

bool ProvisionStore::addProvisionStore( const Ice::Identity& ident, const std::string& methodType, unsigned int timeStart, unsigned int timeEnd, unsigned int bandwidthKBps )
{
	//glog( ZQ::common::Log::L_INFO,  L"bandwidth:%d",bandwidthBps);
	ZQ::common::MutexGuard	op(_psLock);
	if (methodType.empty() ||!bandwidthKBps)
		return false;
	Provisionmap::iterator iter = _provisionMap.find(ident);
	if (iter != _provisionMap.end())
	{
#ifdef ZQ_OS_MSWIN
		if (stricmp(methodType.c_str(),(*iter).second.methodType.c_str()))
			return false;
#else
		if (strcasecmp(methodType.c_str(),(*iter).second.methodType.c_str()))
			return false;
#endif
		(*iter).second.timeStart = timeStart;
		(*iter).second.timeStop = timeEnd;
		(*iter).second.bandwidthKBps = bandwidthKBps;
	}
	else
	{
		ProvisionStoreTtem item;
		item.methodType = methodType;
		item.timeStart = timeStart;
		item.timeStop = timeEnd;

		item.bandwidthKBps = bandwidthKBps;
		_provisionMap.insert(std::make_pair(ident,item));
	}
	return true;
}


bool ProvisionStore::delProvisionStore( Ice::Identity& itent)
{
	ZQ::common::MutexGuard	op(_psLock);
	Provisionmap::iterator iter = _provisionMap.find(itent);
	if (iter != _provisionMap.end())
	{
		_provisionMap.erase(iter);
		return true;
	}
	else
	{
		return false;
	}

}


ProvisionStore::ProvisionStoreTtem* ProvisionStore::findProvisionStore( const Ice::Identity& ident )
{
	ZQ::common::MutexGuard	op(_psLock);
	ProvisionStoreTtem* item = NULL;
	Provisionmap::iterator iter = _provisionMap.find(ident);
	if (iter != _provisionMap.end())
	{
		item = &(*iter).second;
	}
	else
	{
		return NULL;
	}
	return item;
}

bool ProvisionStore::addResourceBookForProvision(ZQTianShan::ContentProvision::ProvisionResourceBook& presourcebook)
{
	//glog( ZQ::common::Log::L_INFO,  L"There is %d sessions in map",_provisionMap.size());
	for (Provisionmap::iterator iter = _provisionMap.begin();iter != _provisionMap.end();iter++)
	{
		if(!presourcebook.addProvision((*iter).second.methodType,(*iter).second.timeStart,(*iter).second.timeStop,(*iter).second.bandwidthKBps))
		{
			return false;
		}
	}
	return true;
}

bool ProvisionStore::updateProvisionStore( const Ice::Identity& ident, const std::string& methodType, unsigned int timeStart, unsigned int timeEnd,unsigned int bandwidthKBps)
{
	ZQ::common::MutexGuard	op(_psLock);
	if (methodType.empty())
		return false;
	Provisionmap::iterator iter = _provisionMap.find(ident);
	if (iter != _provisionMap.end())
	{
#ifdef ZQ_OS_MSWIN
		if (stricmp(methodType.c_str(),(*iter).second.methodType.c_str()))
			return false;
#else
		if (strcasecmp(methodType.c_str(),(*iter).second.methodType.c_str()))
			return false;
#endif
		(*iter).second.timeStart = timeStart;
		(*iter).second.timeStop = timeEnd;
		(*iter).second.bandwidthKBps = bandwidthKBps;
	}
	else
	{
		ProvisionStoreTtem item;
		item.methodType = methodType;
		item.timeStart = timeStart;
		item.timeStop = timeEnd;

		item.bandwidthKBps = bandwidthKBps;
		_provisionMap.insert(std::make_pair(ident,item));
	}
	return true;
}

