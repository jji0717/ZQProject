#include "GBssResourceManager.h"

GBssResourceManager::GBssResourceManager(SelectionEnv &env)
:NgodResourceManager(env)
{
}

GBssResourceManager::~GBssResourceManager()
{

}

//const ResourceStreamerAttrMap GBssResourceManager::findByIdentifier(const std::string &identifier) const
//{
//	ResourceStreamerAttrMap resourceStreamerAttrMap;
//	size_t nStart = identifier.find("/");
//	if ( nStart != std::string::npos)
//	{
//		std::string strMask = identifier.substr(nStart + 1);
//		int bits = atoi(strMask.c_str());
//		unsigned long i = 1;
//		unsigned long mask= (i << 32) - (i << (32 - bits)) + 1;
//
//		std::string strIP = identifier.substr(0, nStart);
//		unsigned long ip = inet_addr(strIP.c_str());
//
//		SOPS::const_iterator sopsIter = mSops.begin();
//		ResourceStreamerAttrMap::const_iterator streamersIter = sopsIter->second.begin();
//		for (; streamersIter != sopsIter->second.end(); streamersIter++)
//		{
//			if (streamersIter->second.sourceIP & mask == ip)
//			{
//				resourceStreamerAttrMap.insert(std::make_pair(streamersIter->first, streamersIter->second));
//			}
//		}
//	}
//	else
//	{
//		unsigned long ip = inet_addr(identifier.c_str());
//		SOPS::const_iterator sopsIter = mSops.begin();
//		ResourceStreamerAttrMap::const_iterator streamersIter = sopsIter->second.begin();
//		for (; streamersIter != sopsIter->second.end(); streamersIter++)
//		{
//			if (streamersIter->second.sourceIP == ip)
//			{
//				resourceStreamerAttrMap.insert(std::make_pair(streamersIter->first, streamersIter->second));
//			}
//		}
//	}
//	return resourceStreamerAttrMap;
//}

NgodResourceManager::StreamerIterMap GBssResourceManager::findByIdentifier(const std::string& identifier)
{
	NgodResourceManager::StreamerIterMap streamerMap;
	SOPS::iterator sopsIter = mSops.begin();
	ResourceStreamerAttrMap::iterator streamersIter = sopsIter->second.begin();
	for (; streamersIter != sopsIter->second.end(); streamersIter++)
	{
		streamerMap.insert(std::make_pair(streamersIter->first, streamersIter));
	}
	return streamerMap;
}