#include "./WatchDog.h"
#include "./ChODSvcEnv.h"
#include "CODConfig.h"

namespace ZQChannelOnDemand
{

#define MaxWatchListSize 100
#define MinSleepTime 100 // milli seconds
#define AccurateTo 16
#define DebugLevel ZQ::common::Log::L_DEBUG

	typedef std::vector<std::string> STRINGVECTOR;
	//////////////////////////////////////////////////////////////////////////
	// class WatchException
	//////////////////////////////////////////////////////////////////////////
	
	WatchException::WatchException(const std::string& msg) : _msg(msg)
	{
	}
	
	WatchException::~WatchException()
	{
	}
	
	std::string WatchException::getMessage() const
	{
		return _msg;
	}

	//////////////////////////////////////////////////////////////////////////
	// class WatchObject
	//////////////////////////////////////////////////////////////////////////
	
	WatchObject::WatchObject()
	{
	}

	WatchObject::WatchObject(const WatchObject& watchObj)
	{
		_ident = watchObj._ident;
		_expiration = watchObj._expiration;
	}

	WatchObject::~WatchObject()
	{
	}

	bool WatchObject::operator==(const WatchObject& watchObj) const
	{
		return _expiration == watchObj._expiration ? true : false;
	}

	bool WatchObject::operator!=(const WatchObject& watchObj) const
	{
		return _expiration != watchObj._expiration ? true : false;
	}

	bool WatchObject::operator>(const WatchObject& watchObj) const
	{
		return _expiration > watchObj._expiration ? true : false;
	}

	bool WatchObject::operator>=(const WatchObject& watchObj) const
	{
		return _expiration >= watchObj._expiration ? true : false;
	}

	bool WatchObject::operator<(const WatchObject& watchObj) const
	{
		return _expiration < watchObj._expiration ? true : false;
	}

	bool WatchObject::operator<=(const WatchObject& watchObj) const
	{
		return _expiration <= watchObj._expiration ? true : false;
	}

	WatchObject& WatchObject::operator=(const WatchObject& watchObj)
	{
		_ident = watchObj._ident;
		_expiration = watchObj._expiration;
		return *this;
	}

	void WatchObject::setIdent(const std::string& ident)
	{
		_ident = ident;
	}

	void WatchObject::setExpiration(const __int64& expiration)
	{
		_expiration = expiration;
	}	

	std::string WatchObject::getIdent() const
	{
		return _ident;
	}

	__int64 WatchObject::getExpiration() const
	{
		return _expiration;
	}

	bool WatchObject::isFullEqual(const WatchObject& watchObj) const
	{
		return (_ident == watchObj._ident && _expiration == watchObj._expiration) ? true : false;
	}

	//////////////////////////////////////////////////////////////////////////
	// class PurchaseTimeout
	//////////////////////////////////////////////////////////////////////////
	
	PurchaseTimeout::PurchaseTimeout(ZQ::common::NativeThreadPool& thrdPool, ChODSvcEnv& env, const std::string& sessID)
		: _thrdPool(thrdPool), ZQ::common::ThreadRequest(thrdPool), _env(env), _sessID(sessID)
	{
	}
	
	PurchaseTimeout::~PurchaseTimeout()
	{
	}
	
	bool PurchaseTimeout::init()
	{
		return true;
	}
	
	int PurchaseTimeout::run(void)
	{
		try
		{
			glog(ZQ::common::Log::L_INFO, "purchase[%s] timeout", _sessID.c_str());
			Ice::Identity ident;
			ident.name = _sessID;
			ident.category = ICE_ChannelPurchase;
			// get purchase proxy
			NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx) purPrx = NULL;
			try
			{
				purPrx = NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx)::checkedCast(_env._adapter->createProxy(ident));
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_INFO, "purchase checkedcast caught %s", ex.ice_name().c_str());
				return 1;
			}
			// check if stream live
			bool isWeiwooLive = true;
			try
			{
				purPrx->getSession()->ice_ping();
			}
			catch (const Ice::ObjectNotExistException& ex)
			{
				isWeiwooLive = false;
				glog(ZQ::common::Log::L_INFO, "purchase[%s] ice_ping weiwoo session caught %s", _sessID.c_str(), ex.ice_name().c_str());
				// notice can't return here
				// playlist doesn't exist, so here have to detach purchase
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_INFO, "purchase[%s] ice_ping weiwoo session caught %s", _sessID.c_str(), ex.ice_name().c_str());
				// notice can't return here
			}

			// check if purchase is in service status
			bool bPurchaseInService = false;
			try
			{
				bPurchaseInService = purPrx->isInService();
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_WARNING, "purchase[%s] isInService() caught %s", _sessID.c_str(), ex.ice_name().c_str());
			}


			// detach purchase
			if (isWeiwooLive == false || bPurchaseInService == false)
			{
				try
				{
					::TianShanIce::Properties props;
					std::string detachReason;

					if (!bPurchaseInService)
						detachReason = "220040 purchase fail";
					else
						detachReason = "220030 purchase timeout";

					if (!isWeiwooLive)
						detachReason += " as an orphan";

// 					if (!bPurchaseInService)
// 						detachReason += " and purchase not in service status";
					props[SYS_PROP(terminateReason)] = detachReason;
					purPrx->detach("", props);
				}
				catch (const Ice::Exception& ex)
				{
					glog(ZQ::common::Log::L_INFO, "detach purchase[%s] caught %s", _sessID.c_str(), ex.ice_name().c_str());
					return 1;
				}
			}
			else // stream is living
			{
				glog(ZQ::common::Log::L_DEBUG, "purchase[%s]'s weiwoo session is living", _sessID.c_str());
				_env._pWatchDog->watch(_sessID, _config.purchaseTimeout * 1000);
			}
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, "purchase[%s] timeout caught unexpected exception", _sessID.c_str());
			return 1;
		}

		return 0;
	}
	
	void PurchaseTimeout::final(int retcode, bool bCancelled)
	{
		delete this;
	}
		
	//////////////////////////////////////////////////////////////////////////
	// class WatchDog
	//////////////////////////////////////////////////////////////////////////
	
	WatchDog::WatchDog(ChODSvcEnv& env)
		: _env(env)
		, _bExit(false)
		, _nextWakeup(0)
		, _event(NULL)
	{
	}

	WatchDog::~WatchDog()
	{
		stop();

		if (NULL != _event)
			::CloseHandle(_event);
		_event = NULL;
	}

	bool WatchDog::init(void)
	{
		_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		return true;
	}

	void WatchDog::stop()
	{
		_bExit = true;
		if (NULL != _event)
			::SetEvent(_event);
		::Sleep(1); // yield for 1 milli second.
	}

	void WatchDog::final(void)
	{
		glog(ZQ::common::Log::L_INFO, "PurchaseWatchDog exist, %d purchase under watch now", getWatchSize());
	}

	void WatchDog::watch(std::string ident, __int64 milliSec)
	{
		// 调整为最小的watch duration
		if (milliSec <= 1)
			milliSec = 1;
		__int64 tmNow = ZQTianShan::now();
		__int64 expiration = milliSec + tmNow;

		ZQ::common::MutexGuard lk(_lock);
		
		std::list<WatchObject>::iterator lstItor;
		// 移出_watchList中具有相同ident的对象
		bool bFoundInList = false;// false: _watchList中没有与ident相同的对象; true: _watchList中有与ident相同的对象
		__int64 maxExprtInList = 0;// 0: 意味着_watchList大小为0，因为_watchList中没有小于等于0的expiration
		for (lstItor = _watchList.begin(); lstItor != _watchList.end(); lstItor ++)
		{
			if (lstItor->_expiration > maxExprtInList)
				maxExprtInList= lstItor->_expiration;
			
			if (lstItor->_ident == ident)
			{
				_watchList.erase(lstItor);
				bFoundInList = true;
				break; // break说明watchList没有遍历完全，bFoundInList = true;此时maxExprtInList值不可靠
			}
		}
		
		if (0 == maxExprtInList) // 此时意味着_watchList大小为0
		{
			_watchMap[ident] = expiration;
		}
		else if (false == bFoundInList) // _watchList中没有与ident相同的WatchObject对象
		{								// 此时maxExprtInList保存着_watchList中最大的expiration
			if (expiration >= maxExprtInList) // 如果要求加入的WatchObject对象的expiration大于等于_watchList中最大的expiration(maxExprtInList)
			{								  // 则将其加入_watchMap中
				_watchMap[ident] = expiration;
			}
			else // 如果要求加入的WatchObject对象的expiration小于_watchList中最大的expiration(maxExprtInList)
			{	 // 那么_watchList中肯定有某个WatchObject对象的expiration大于要加入的expiration, 所以将该WatchObject对象加入到_watchList中
				for (lstItor = _watchList.begin(); lstItor != _watchList.end(); lstItor ++)
				{
					if (expiration <= lstItor->_expiration)
					{
						WatchObject wo;
						wo.setExpiration(expiration);
						wo.setIdent(ident);
						_watchList.insert(lstItor, wo);
						break;
					}
				}
			}
		}
		else // _watchList中有与ident相同的WatchObject对象，而且此时该WatchObject对象已经被移出
		{	 // 此时maxExprtInList不可靠，保存的值不一定是_watchList中的最大值，必须重新遍历_watchList
			bool bAddedToList = false; // 标识是否被加入到_watchList中
			for (lstItor = _watchList.begin(); lstItor != _watchList.end(); lstItor ++)
			{
				if (expiration <= lstItor->_expiration)
				{
					WatchObject wo;
					wo.setExpiration(expiration);
					wo.setIdent(ident);
					_watchList.insert(lstItor, wo);
					bAddedToList = true;
					break;
				}
			}
			if (false == bAddedToList) // 没有被加入到_watchList中，则要求被加入到_watchMap中
			{
				_watchMap[ident] = expiration;
			}
		}

		glog(ZQ::common::Log::L_DEBUG, "watch purchase(%s) for %lldms, total purchase(%d)", ident.c_str(), milliSec, _watchList.size() + _watchMap.size());
		
		// 如果当前加入的expiration小于_nextWakeup，则唤醒run()函数
		if (expiration < _nextWakeup)
		{
			_nextWakeup = expiration;
			::SetEvent(_event);
		}
	}

	void WatchDog::unwatch(std::string ident)
	{
		glog(ZQ::common::Log::L_DEBUG, "unwatch purchase(%s)", ident.c_str());
		ZQ::common::MutexGuard lk(_lock);
		std::map<std::string, __int64>::iterator map_itor = _watchMap.find(ident);
		if (_watchMap.end() != map_itor)
		{
			_watchMap.erase(ident);
			return;
		}
		for (std::list<WatchObject>::iterator list_itor = _watchList.begin(); list_itor != _watchList.end(); list_itor ++)
		{
			if (list_itor->_ident == ident)
			{
				_watchList.erase(list_itor);
				return;
			}
		}
	}

	void WatchDog::preSort(std::vector<WatchObject>& wos, const int& inFrom, const int& inLen, int& nSmallest)
	{
		int iFrom = inFrom;
		int iLen = inLen;
		if (iLen <= 1)
		{
			nSmallest = inLen;
			return;
		}

		iLen += iFrom;
		int iBase = iFrom;
		int iNonSort = iFrom + 1;
		int iSmall = iFrom;
		int iLarge = iFrom;
		while (iNonSort < iLen)
		{
			if (wos[iNonSort] >= wos[iFrom])
				iLarge = iNonSort++;
			else 
			{
				WatchObject tmpwo = wos[iSmall + 1];
				wos[iSmall + 1] = wos[iNonSort];
				wos[iNonSort] = tmpwo;
				iSmall++;
				iNonSort++;
				iLarge++;
			}
		}

		{
			WatchObject tmpwo = wos[iFrom];
			wos[iFrom] = wos[iSmall];
			wos[iSmall] = tmpwo;
		}

		nSmallest = iSmall - iFrom + 1;
	}

	void WatchDog::QuickSort(std::vector<WatchObject>& wos, const int& inFrom, const int& inLen)
	{
		int iFrom = inFrom;
		int iLen = inLen;
		if (iLen <= 1)
			return;

		iLen += iFrom;
		int iBase = iFrom;
		int iNonSort = iFrom + 1;
		int iSmall = iFrom;
		int iLarge = iFrom;
		while (iNonSort < iLen)
		{
			if (wos[iNonSort] >= wos[iFrom])
				iLarge = iNonSort++;
			else 
			{
				WatchObject tmpwo = wos[iSmall + 1];
				wos[iSmall + 1] = wos[iNonSort];
				wos[iNonSort] = tmpwo;
				iSmall++;
				iNonSort++;
				iLarge++;
			}
		}

		{
			WatchObject tmpwo = wos[iFrom];
			wos[iFrom] = wos[iSmall];
			wos[iSmall] = tmpwo;
		}

		QuickSort(wos, iFrom, iSmall - iFrom);
		QuickSort(wos, iSmall + 1, iLen - (iSmall + 1));
	}

	void WatchDog::buildList(const __int64& tmNow)
	{
		std::vector<WatchObject> watchs;
		watchs.reserve(_watchMap.size());
		for (std::map<std::string, __int64>::const_iterator mapItor = _watchMap.begin(); mapItor != _watchMap.end(); mapItor ++)
		{
			WatchObject wo;
			wo.setIdent(mapItor->first);
			wo.setExpiration(mapItor->second);
			watchs.push_back(wo);
			int i = 0;
		}

		int nSmallest = 0;
		do {
			preSort(watchs, 0, watchs.size(), nSmallest);
		} while(nSmallest > MaxWatchListSize);
		
		QuickSort(watchs, 0, nSmallest);

		for (int i = 0; /*i < MaxWatchListSize && */i < nSmallest; i ++)
		{
			_watchList.push_back(watchs[i]);
			_watchMap.erase(watchs[i]._ident);
		}
	}

	int WatchDog::run()
	{
		while (false == _bExit)
		{
			__int64 tmNow = ZQTianShan::now();
			STRINGVECTOR timeouts;
			__int64 sleepTime;
			{
				ZQ::common::MutexGuard lk(_lock);
				
				if (0 == _watchList.size()) // _watchList中没有对象, 则从_watchMap中前n个快要timeout的对象放入_watchList
				{
					buildList(tmNow);
				}

				// 找出所有timeout的purchase
				timeouts.reserve(_watchList.size());
				while (_watchList.size() > 0)
				{
					if (_watchList.front()._expiration - tmNow <= AccurateTo) // 16ms is accurate time
					{
						timeouts.push_back(_watchList.front()._ident);
						_watchList.pop_front();
					}
					else 
						break;
				}
			}

			// call onTimer()
			onTimer(timeouts);

			{
				ZQ::common::MutexGuard lk(_lock);

				if (_watchList.size() > 0)
				{
					_nextWakeup = _watchList.front()._expiration;
					sleepTime = _nextWakeup - tmNow;
				}
				else // _watchList中所有的TimerInfo对象全部timeout，还有一种情况就是当前没有对象在watchMap中
				{
					buildList(tmNow);
					if (0 == _watchList.size())
					{
						_nextWakeup = tmNow + 60000;
						sleepTime = 60000;
					}
					else 
					{
						_nextWakeup = _watchList.front()._expiration;
						sleepTime = _nextWakeup - tmNow;
					}
				}
			}

			if (sleepTime <= MinSleepTime)
				sleepTime = MinSleepTime;

			::WaitForSingleObject(_event, (unsigned long)sleepTime);
		}
		
		return 0;
	}

	int WatchDog::getWatchSize()
	{
		ZQ::common::MutexGuard lk(_lock);

		return _watchMap.size() + _watchList.size();
	}

	void WatchDog::onTimer(const std::vector<std::string>& idents)
	{
		for (int i = 0; i < (int)idents.size(); i ++)
		{
			PurchaseTimeout* pOnTimer = new PurchaseTimeout(*(_env._pThreadPool), _env, idents[i]);
			pOnTimer->start();
		}
	}

} // namespace ZQChannelOnDemand

