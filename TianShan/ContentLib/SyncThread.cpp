#include "SyncThread.h"
#include "ContentLibConfig.h"

//////////////////////////////////////////////////////////////////////////
// class SyncThread
//////////////////////////////////////////////////////////////////////////

SyncThread::SyncThread(ContentLibImpl::Ptr contentLibPtr)
	:_contentLibPtr(contentLibPtr), _bStop(false), _dTime(_config.timeToSync * 1000)
{
}

SyncThread::~SyncThread()
{
	stop();
}

bool SyncThread::init(void)
{
	if(!openHandles())
		return false;
	else 
		return true;
}

void SyncThread::stop() 
{
	_bStop = true;
	notify();
	waitHandle(1000);
}

void SyncThread::final(void)
{
	glog(ZQ::common::Log::L_INFO, "SyncThread exist");
}

bool SyncThread::openHandles()
{
	return true;
}

void SyncThread::closeHandles()
{
}

int SyncThread::run()
{
	SYS::SingleObject::STATE  status;
	while (true)
	{
		status = _hNotify.wait( _dTime);
		if( _bStop)
			break;

		if(	SYS::SingleObject::SIGNALED == status )
			runNotify();
		else if( SYS::SingleObject::TIMEDOUT == status )
			runTimeout();
		else
			break;
	}

	return 1;
}

bool SyncThread::notify()
{
	_hNotify.signal();
	return true;
}

bool SyncThread::runNotify()
{
	glog(ZQ::common::Log::L_DEBUG, "SyncThread runNotify");
	ContentLibConfig::CSParams::iterator iter = _config.csParams.begin();
	for (; iter != _config.csParams.end(); ++iter) 
	{
		std::string strNetId;
		if(_contentLibPtr->connectContentStore(iter->endpoint, strNetId))
			_contentLibPtr->addContentStoreReplica(strNetId, iter->endpoint);
	}
	return true;
}

bool SyncThread::runTimeout()
{
	glog(ZQ::common::Log::L_DEBUG, "SyncThread runTimeout");
	runNotify(); 
	return true;
}

//////////////////////////////////////////////////////////////////////////
// class ContentCacheThread
//////////////////////////////////////////////////////////////////////////

ContentCacheThread::ContentCacheThread(ContentLibImpl::Ptr contentLibPtr, ::ZQTianShan::MetaLib::MetaLibImpl& lib)
	 :_contentLibPtr(contentLibPtr), _lib(lib), _bStop(false), 
	 _dTime(1800000), _lastQuery(ZQTianShan::now())
{
	cleanCache();
}

ContentCacheThread::~ContentCacheThread()
{
	stop();
}

bool ContentCacheThread::init(void)
{
	if(!openHandles())
		return false;
	else 
		return true;
}

void ContentCacheThread::stop()
{ 
	_bStop = true;
	notify();
	waitHandle(1000);
}

void ContentCacheThread::final(void)
{
	glog(ZQ::common::Log::L_INFO, "ContentCacheThread exist");
}

bool ContentCacheThread::openHandles()
{
	return true;
}

void ContentCacheThread::closeHandles()
{
}


int ContentCacheThread::run()
{
	SYS::SingleObject::STATE status;
	while (true)
	{
		status = _hNotify.wait(_dTime); // half an hour
		if(_bStop)
			break;
		
		if(SYS::SingleObject::SIGNALED == status)
			runNotify();
		else if( SYS::SingleObject::TIMEDOUT == status)
			runTimeout();
		else
			break;

	}

	return 1;
}

bool ContentCacheThread::notify()
{
	_hNotify.signal();
	return true;
}

bool ContentCacheThread::runNotify()
{
	glog(ZQ::common::Log::L_DEBUG, "ContentCacheThread runNotify");
	return rebuild();
}

bool ContentCacheThread::runTimeout()
{
	glog(ZQ::common::Log::L_DEBUG, "ContentCacheThread runTimeout");
	if(ZQTianShan::now() - _lastQuery > 3600000)
		return cleanCache();
	else
		return true;
}
	
bool ContentCacheThread::rebuild()
{
	try	{
		{
			ZQ::common::MutexGuard guard(_lockOfMap);
			_ContentsInDB.clear();
			::Freeze::EvictorIteratorPtr itptr = _lib.getObjectEvictor()->getIterator("", 1000);
			while (itptr && itptr->hasNext())
			{
				Ice::Identity ident = itptr->next();
				if(ident.name.find("@") == std::string::npos || ident.name.find("$") == std::string::npos)
					continue;
				std::string contentName =  ident.name.substr(0, ident.name.find("@"));
				std::string nidVol =  ident.name.substr(ident.name.find("@") + 1, ident.name.size());
				if(nidVol == "" || contentName == "")
					continue;
				std::map<std::string , std::vector < std::string > >::iterator it = _ContentsInDB.find(nidVol);
				if(it == _ContentsInDB.end())
				{
					std::vector <std::string> vtr;
					vtr.push_back(ident.name);
					_ContentsInDB.insert(std::make_pair(nidVol, vtr));
				}
				else
				{
					it->second.push_back(ident.name);
				}
			}
		}

		{
			cleanCache();
		}

		return true;
	}
	catch(const ::Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentCacheThread, "rebuild() caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentCacheThread, "rebuild() caught unknown exception"));
	}
	return false;
}

TianShanIce::Repository::MetaObjectInfos
ContentCacheThread::getContentList(std::string NetId, std::string VolumeName, int startCount, int maxCount, int& total, const ::TianShanIce::StrValues& expectedMetaDataNames)
{
	TianShanIce::Repository::MetaObjectInfos contentList;
	if(startCount < 0)
		return contentList;
	if (maxCount > 0)
	{
		contentList.reserve(maxCount);
	}
	{
		ZQ::common::MutexGuard guard(_lockOfTime);
		_lastQuery = ZQTianShan::now();
	}
	if(_currentCache._netId != NetId || _currentCache._volumeName != VolumeName || _currentCache.bExpire)
	{
		cache(NetId, VolumeName);
	}
	total = _currentCache._cacheList.size();
	if(maxCount > 0)
	{
		for(int i = startCount, j = 0; i < (int)_currentCache._cacheList.size() && j < maxCount; i++, j++)
		{
			TianShanIce::Repository::MetaObjectInfo contentInfo;
			contentInfo.id = _currentCache._cacheList[i];
			contentInfo.type = CONTENTREPLICA;
			if (!expectedMetaDataNames.empty())
			{
				::TianShanIce::Repository::MetaDataMap metaDataMap;
				std::string fullName = _currentCache._cacheList[i];
				std::string contentName =  fullName.substr(0, fullName.find("@"));
				std::string strNetId = fullName.substr(fullName.find("@") + 1, fullName.find_first_of("$") - fullName.find("@") - 1);
				std::string volName = fullName.substr(fullName.find_first_of("$") + 1);

				try
				{
					::TianShanIce::Repository::ContentReplicaPrx contentPrx = _contentLibPtr->toContentReplica(strNetId + "$" + volName + "/" + contentName);
					metaDataMap = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(contentPrx)->getMetaDataMap();
					for (size_t i=0; i < expectedMetaDataNames.size(); i++)
					{
						::TianShanIce::Repository::MetaDataMap::iterator map_iter = metaDataMap.find(expectedMetaDataNames[i]);
						if(map_iter != metaDataMap.end())
							contentInfo.metaDatas.insert(::TianShanIce::Repository::MetaDataMap::value_type(expectedMetaDataNames[i], map_iter->second));
					}
				}
				catch (...)
				{
					continue;
				}
			}
			contentList.push_back(contentInfo);
		}
	}
	else
	{
		for(size_t i = startCount; i < _currentCache._cacheList.size(); i++)
		{
			TianShanIce::Repository::MetaObjectInfo contentInfo;
			contentInfo.id = _currentCache._cacheList[i];
			contentInfo.type = CONTENTREPLICA;
			if (!expectedMetaDataNames.empty())
			{
				::TianShanIce::Repository::MetaDataMap metaDataMap;
				std::string fullName = _currentCache._cacheList[i];
				std::string contentName =  fullName.substr(0, fullName.find("@"));
				std::string strNetId = fullName.substr(fullName.find("@") + 1, fullName.find_first_of("$") - fullName.find("@") - 1);
				std::string volName = fullName.substr(fullName.find_first_of("$") + 1);

				try
				{
					::TianShanIce::Repository::ContentReplicaPrx contentPrx = _contentLibPtr->toContentReplica(strNetId + "$" + volName + "/" + contentName);
					metaDataMap = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(contentPrx)->getMetaDataMap();
					for (size_t i=0; i < expectedMetaDataNames.size(); i++)
					{
						::TianShanIce::Repository::MetaDataMap::iterator map_iter = metaDataMap.find(expectedMetaDataNames[i]);
						if(map_iter != metaDataMap.end())
							contentInfo.metaDatas.insert(::TianShanIce::Repository::MetaDataMap::value_type(expectedMetaDataNames[i], map_iter->second));
					}
				}
				catch (...)
				{
					continue;
				}
			}
			contentList.push_back(contentInfo);
		}
	}
	
	return contentList;
}

bool ContentCacheThread::cache(std::string NetId, std::string VolumeName)
{
	ZQ::common::MutexGuard guard(_lockOfCache);
	_currentCache._netId = NetId;
	_currentCache._volumeName = VolumeName;
	_currentCache._cacheList.clear();
	if(NetId != "" && VolumeName != "")
	{
		std::string key = NetId + "$" + VolumeName;
		std::map<std::string , std::vector < std::string > >::iterator map_it = _ContentsInDB.find(key);
		if(map_it != _ContentsInDB.end())
		{
			for (std::vector < std::string >::iterator iter = map_it->second.begin(); iter != map_it->second.end(); iter++ )
			{
				_currentCache._cacheList.push_back(*iter);
			}
		}
	}
	else if(NetId != "" && VolumeName == "")
	{
		std::map<std::string , std::vector < std::string > >::iterator map_it = _ContentsInDB.begin();
		for (; map_it != _ContentsInDB.end(); map_it++)
		{
			if( map_it->first.substr(0, map_it->first.find("$")) != NetId)
				continue;
			for (std::vector < std::string >::iterator iter = map_it->second.begin(); iter != map_it->second.end(); iter++ )
			{
				_currentCache._cacheList.push_back(*iter);
			}
		}	
	}
	else if(NetId == "" && VolumeName != "")
	{
		std::map<std::string , std::vector < std::string > >::iterator map_it = _ContentsInDB.begin();
		for (; map_it != _ContentsInDB.end(); map_it++)
		{
			if( map_it->first.substr(map_it->first.find("$")+1, map_it->first.size()) != VolumeName)
				continue;
			for (std::vector < std::string >::iterator iter = map_it->second.begin(); iter != map_it->second.end(); iter++ )
			{
				_currentCache._cacheList.push_back(*iter);
			}
		}	
	}
	else if(NetId == "" && VolumeName == "")
	{
		std::map<std::string , std::vector < std::string > >::iterator map_it = _ContentsInDB.begin();
		for (; map_it != _ContentsInDB.end(); map_it++)
		{
			for (std::vector < std::string >::iterator iter = map_it->second.begin(); iter != map_it->second.end(); iter++ )
			{
				_currentCache._cacheList.push_back(*iter);
			}
		}	
	}
	_currentCache.bExpire = false;
	return true;
}


bool ContentCacheThread::addContent(std::string NetId, std::string VolumeName, std::string ContentName)
{
	std::string fullname = ContentName + "@" + NetId + "$" + VolumeName;
	{
		ZQ::common::MutexGuard guard(_lockOfMap);
		std::string nidVol =  NetId + "$" + VolumeName;
		std::map<std::string , std::vector < std::string > >::iterator it = _ContentsInDB.find(nidVol);
		if(it == _ContentsInDB.end())
		{
			std::vector <std::string> vtr;
			vtr.push_back(fullname);
			_ContentsInDB.insert(std::make_pair(nidVol, vtr));
		}
		else
		{
			if(std::find(it->second.begin(), it->second.end(), fullname) == it->second.end())
				it->second.push_back(fullname);
		}
	}
	if(_currentCache._netId == NetId && _currentCache._volumeName == VolumeName)
	{
		ZQ::common::MutexGuard guard(_lockOfCache);
		if (std::find(_currentCache._cacheList.begin(), _currentCache._cacheList.end(), fullname) == _currentCache._cacheList.end())
			_currentCache._cacheList.push_back(fullname);
		_currentCache.bExpire = false;
	}
	return true;
}
bool ContentCacheThread::removeContent(std::string NetId, std::string VolumeName, std::string ContentName)
{
	std::string fullname = ContentName + "@" + NetId + "$" + VolumeName;
	{
		ZQ::common::MutexGuard guard(_lockOfMap);
		std::string nidVol =  NetId + "$" + VolumeName;
		std::map<std::string , std::vector < std::string > >::iterator it = _ContentsInDB.find(nidVol);
		if(it != _ContentsInDB.end())
		{
			std::vector < std::string >::iterator search_it = std::find(it->second.begin(), it->second.end(), fullname);
			if(search_it != it->second.end())
				it->second.erase(search_it);
		}
	}
	if(_currentCache._netId == NetId && _currentCache._volumeName == VolumeName)
	{
		ZQ::common::MutexGuard guard(_lockOfCache);
		std::vector < std::string >::iterator search_it = std::find(_currentCache._cacheList.begin(), _currentCache._cacheList.end(), fullname);
		if (search_it != _currentCache._cacheList.end())
			_currentCache._cacheList.erase(search_it);
		_currentCache.bExpire = false;
	}
	return true;
}

bool ContentCacheThread::cleanCache()
{
	ZQ::common::MutexGuard guard(_lockOfCache);
	_currentCache._netId = "";
	_currentCache._volumeName = "";
	_currentCache._cacheList.clear();
	_currentCache.bExpire = true;
	return true;
}
