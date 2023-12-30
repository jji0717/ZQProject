#include "OpenVBOResourceManager.h"

extern "C" {
#ifdef ZQ_OS_LINUX
#include <arpa/inet.h>
#endif
}

OpenVBOResourceManager::OpenVBOResourceManager(SelectionEnv &env)
:NgodResourceManager(env)
{
}

OpenVBOResourceManager::~OpenVBOResourceManager()
{

}

//const ResourceStreamerAttrMap OpenVBOResourceManager::findByIdentifier(const std::string &identifier) const
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

NgodResourceManager::StreamerIterMap OpenVBOResourceManager::findByIdentifier(const std::string& identifier)
{
	NgodResourceManager::StreamerIterMap streamerMap;
	size_t nStart = identifier.find("/");
	if ( nStart != std::string::npos)
	{
		// mask 
		std::string strMask = identifier.substr(nStart + 1);
		int bits = atoi(strMask.c_str());
		unsigned long mask = 1;
		mask = (mask << 32) - (mask << (32 - bits)) - 1;
		mask = htonl(mask);

		// subNet
		std::string strIP = identifier.substr(0, nStart);
		unsigned long subnet = inet_addr(strIP.c_str());
		subnet &= mask;

		SOPS::iterator sopsIter = mSops.begin();
		ResourceStreamerAttrMap::iterator streamersIter = sopsIter->second.begin();
		for (; streamersIter != sopsIter->second.end(); streamersIter++)
		{
			unsigned long result = streamersIter->second.sourceIP & mask;
			if (result == subnet)
			{
				streamerMap.insert(std::make_pair(streamersIter->first, streamersIter));
			}
		}
	}
	else
	{
		unsigned long ip = inet_addr(identifier.c_str());
		SOPS::iterator sopsIter = mSops.begin();
		ResourceStreamerAttrMap::iterator streamersIter = sopsIter->second.begin();
		for (; streamersIter != sopsIter->second.end(); streamersIter++)
		{
			if (streamersIter->second.sourceIP == ip)
			{
				streamerMap.insert(std::make_pair(streamersIter->first, streamersIter));
			}
		}
	}
	return streamerMap;
}
