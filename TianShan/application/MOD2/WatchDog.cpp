#include "WatchDog.h"
#include "ModService.h"
#include "SystemUtils.h"
extern ZQ::common::Config::Holder<ModCfg> gNewCfg;
namespace ZQMODApplication
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

void WatchObject::operator=(const WatchObject& watchObj)
{
	_ident = watchObj._ident;
	_expiration = watchObj._expiration;
}

void WatchObject::setIdent(const std::string& ident)
{
	_ident = ident;
}

void WatchObject::setExpiration(const int64& expiration)
{
	_expiration = expiration;
}	

std::string WatchObject::getIdent() const
{
	return _ident;
}

int64 WatchObject::getExpiration() const
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

PurchaseTimeout::PurchaseTimeout(ZQ::common::NativeThreadPool& thrdPool, ModEnv& env, const std::string& sessID)
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
		glog(ZQ::common::Log::L_INFO, "purchase [%s] OnTimer", _sessID.c_str());
		Ice::Identity ident;
		ident.name = _sessID;
		ident.category = Servant_ModPurchase;
		// get purchase proxy
		ZQTianShan::Application::MOD::ModPurchasePrx purPrx = NULL;
		try
		{
			purPrx = ZQTianShan::Application::MOD::ModPurchasePrx::checkedCast(_env._iceAdap->createProxy(ident));
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_INFO, "purchase checkedcast caught %s", ex.ice_name().c_str());
			return 1;
		}
		// check if weiwoo live
		bool isWeiwooLive = true;
		try
		{
			purPrx->getSession()->ice_ping();
		}
		catch (const Ice::ObjectNotExistException& ex)
		{
			isWeiwooLive = false;
			glog(ZQ::common::Log::L_WARNING, "purchase[%s] ice_ping weiwoo session caught %s", _sessID.c_str(), ex.ice_name().c_str());
			// notice can't return here
			// playlist doesn't exist, so here have to detach purchase
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_WARNING, "purchase[%s] ice_ping weiwoo session caught %s", _sessID.c_str(), ex.ice_name().c_str());
			// notice can't return here
			// playlist doesn't exist, so here have to detach purchase
		}

		// DO: check if purchase is in service status
		bool bPurchaseInService = false;
		try
		{
			bPurchaseInService = purPrx->isInService();
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_WARNING, "purchase[%s] isInService() caught %s", _sessID.c_str(), ex.ice_name().c_str());
		}

		// detach purchase if weiwoo session not exist or purchase not in 
		// service status(such as provision failed but create purchase success)
		if (!isWeiwooLive || !bPurchaseInService)
		{
			try
			{
				std::string detachReason = "";
				::TianShanIce::Properties props;

				if (!bPurchaseInService)
				{
					detachReason = "220040 purchase failed";
				}
				else
					detachReason = "220030 purchase timeout";

				if (!isWeiwooLive)
					detachReason += " as an orphan";

				props[SYS_PROP(terminateReason)] = detachReason;
				props[SYS_PROP(PurchaseTimeoutInternal)] = "MOD purchase Internal timeout";
				purPrx->detach("timeout", props);
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_INFO, "detach purchase[%s] caught %s", _sessID.c_str(), ex.ice_name().c_str());
				return 1;
			}
		}
		else // stream is living
		{
			glog(ZQ::common::Log::L_DEBUG, "purchase [%s]'s weiwoo session is living", _sessID.c_str());
			_env._pWatchDog->watch(_sessID, gNewCfg.purchaseTimeout);
		}
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, "purchase[%s] timeout caught unexpect exception", _sessID.c_str());
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

WatchDog::WatchDog(ModEnv& env)
: _env(env)
, _pThreadPool(NULL)
, _bExit(false)
, _nextWakeup(0)
{
}

WatchDog::~WatchDog()
{
	stop();
	if (NULL != _pThreadPool)
		delete _pThreadPool;
	_pThreadPool = NULL;
}

bool WatchDog::init(void)
{
	// hard code 3 threads
	_pThreadPool = new ZQ::common::NativeThreadPool(3);
	bool bRet = (_pThreadPool) ? true : false;
	if (!bRet)
		glog(ZQ::common::Log::L_ERROR, "WatchDog initialize failed");
	return bRet;
}

void WatchDog::stop()
{
	_bExit = true;
	_event.signal();
	SYS::sleep(1); // yield for 1 milli second.
}

void WatchDog::final(void)
{
	glog(ZQ::common::Log::L_INFO, "PurchaseWatchDog exist, %d purchase under watch now", getWatchSize());
}

void WatchDog::watch(std::string ident, int64 milliSec)
{
	// 调整为最小的watch duration
	if (milliSec <= 1)
		milliSec = 1;
	int64 tmNow = ZQTianShan::now();
	int64 expiration = milliSec + tmNow;
	
	ZQ::common::MutexGuard lk(_lock);
	
	// 移出_watchList中具有相同ident的对象
	bool bFoundInList = false;// false: _watchList中没有与ident相同的对象; true: _watchList中有与ident相同的对象
	std::list<WatchObject>::iterator lstItor;
	int64 maxExprtInList = 0;// 0: 意味着_watchList大小为0，因为_watchList中没有小于等于0的expiration
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
		_event.signal();
	}
}

void WatchDog::unwatch(std::string ident)
{
	glog(ZQ::common::Log::L_DEBUG, "unwatch purchase(%s)", ident.c_str());
	ZQ::common::MutexGuard lk(_lock);
	std::map<std::string, int64>::iterator map_itor = _watchMap.find(ident);
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

void WatchDog::buildList(const int64& tmNow)
{
	std::vector<WatchObject> watchs;
	watchs.reserve(_watchMap.size());
	for (std::map<std::string, int64>::const_iterator mapItor = _watchMap.begin(); mapItor != _watchMap.end(); mapItor ++)
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
		int64 tmNow = ZQTianShan::now();
		STRINGVECTOR timeouts;
		int64 sleepTime;
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
		
		_event.wait(sleepTime);
	}
	
	return 0;
}

int WatchDog::getWatchSize()
{
	ZQ::common::MutexGuard lk(_lock);
	
	return _watchMap.size() + _watchList.size();
}

void WatchDog::clear()
{
	ZQ::common::MutexGuard lk(_lock);	
	_watchMap.clear();
	_watchList.clear();
}

void WatchDog::onTimer(const std::vector<std::string>& idents)
{
	for (int i = 0; i < (int)idents.size(); i ++)
	{
		PurchaseTimeout* pOnTimer = new PurchaseTimeout(*_pThreadPool, _env, idents[i]);
		pOnTimer->start();
	}
}

} // namespace ZQMODApplication

