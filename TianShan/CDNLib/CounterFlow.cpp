#include "DiskCache.h"
#include "TimeUtil.h"
#include "NativeThreadPool.h"
#include "RedisClient.h"

namespace XOR_Media {
namespace DiskCache {

// -----------------------------
// class RedisCounterMerger
// -----------------------------
class RedisCounterMerger : virtual public ZQ::common::RedisSink
{
	friend class RedisCommand;

public:
	virtual ~RedisCounterMerger() {}
	RedisCounterMerger(CounterFlow& stream, const CounterFlow::NamedCounter& currentValue) 
		: _stream(stream), _currentValue(currentValue)
	{}

protected:

	virtual void OnRequestError(ZQ::common::RedisClient& client, ZQ::common::RedisCommand& cmd, Error errCode, const char* errDesc=NULL)
	{
		// error occurs, just destory self up to the RedisSink::Ptr
	}

	virtual void OnReply(ZQ::common::RedisClient& client, ZQ::common::RedisCommand& cmd, Data& prevData)
	{
		if (prevData.bulks.size() <=0)
			return; // ingore nil value

		CounterFlow::Counter cv;
		if (!CounterFlow::unmarshal(cv, prevData.bulks[0]))
			return; // parse error

		if (_currentValue.cntr.stampLatest < cv.stampLatest)
			return; // ignore if this is a later merging


		_currentValue.cntr = CounterFlow::merge(cv, _currentValue.cntr);
		_stream.queueCounter(CounterFlow::QT_Merged, _currentValue);
		// up to RedisSink::Ptr to delete self
	}

protected:
	CounterFlow&              _stream;
	CounterFlow::NamedCounter _currentValue;
};

// -----------------------------
// class CounterFlow
// -----------------------------
void CounterFlow::count(Category qt, std::string subpath, const Counter& cnt)
{
	NamedCounter nc;
	nc.cntr    = cnt;
	nc.name    = subpath;

	queueCounter(qt, nc);
}

void CounterFlow::count(Category qt, std::string subpath, uint c)
{
	Counter cntr;
	cntr.stampLatest = cntr.stampSince = ZQ::common::now();
	cntr.stamp2ndLatest = 0;
	cntr.c    = c;
	if (c>1)
		cntr.stamp2ndLatest = cntr.stampLatest;

	count(qt, subpath, cntr);
}

size_t CounterFlow::popEntireQueue(Category qt, NamedCounterQueue& queue, bool bCompress)
{
	queue.clear();
	// step 1, read all the counters in the queue
	{
		ZQ::common::MutexGuard g(_lkQueue);
		queue = _cqueues[qt %COUNTER_QUEUES];
		_cqueues[qt %COUNTER_QUEUES].clear();
	}

	if (!bCompress)
		return queue.size();

	CounterMap cmap;
	for (NamedCounterQueue::iterator itQ = queue.begin(); itQ < queue.end(); itQ++)
	{
		CounterMap::iterator itM = cmap.find(itQ->name);
		if (cmap.end() == itM)
			cmap.insert(CounterMap::value_type(itQ->name, itQ->cntr));
		else
			itM->second = add(itM->second, itQ->cntr);
	}

	queue.clear();
	for (CounterMap::iterator itM = cmap.begin(); itM != cmap.end(); itM++)
	{
		NamedCounter nc;
		nc.name    = itM->first;
		nc.cntr    = itM->second;
		queue.push_back(nc);
	}

	return queue.size();
}

std::string CounterFlow::marshal(const Counter& A)
{
	char buf[100];
	snprintf(buf, sizeof(buf)-2, "%d,%lld,%lld,%lld", A.c, A.stampLatest, A.stampLatest -A.stamp2ndLatest, A.stampLatest -A.stampSince);
	return buf;
}

bool CounterFlow::unmarshal(Counter& A, const std::string stream)
{
	memset(&A, 0x00, sizeof(A));
	if (sscanf(stream.c_str(), "%d,%lld,%lld,%lld", &A.c, &A.stampLatest, &A.stamp2ndLatest, &A.stampSince) <4)
		return false;

	A.stamp2ndLatest = A.stampLatest -A.stamp2ndLatest;
	A.stampSince     = A.stampLatest -A.stampSince;
	return true;
}

#define SEPARATOR_OF_HIT        "$"
#define SEPARATOR_OF_MISSED     "^"
#define CATEGORY_ACCESS_COUNTER "AC"

#define BATCH_SIZE          (8)

ZQ::common::NativeThreadPool thrdpool;

int CounterFlow::run() // impl of NativeThread
{
	ZQ::common::RedisClient::Ptr client = new ZQ::common::RedisClient(_dir._log, thrdpool, "localhost");
	client->setClientTimeout(999999, 1999999);

	int32 nextSleep = WAIT_INTERVAL_MAX;
	while (!_dir._bQuit)
	{
		if (nextSleep > (WAIT_INTERVAL_MIN>>1))
			_dir._eventWorkers.wait(nextSleep);
		nextSleep = WAIT_INTERVAL_MAX;

		NamedCounterQueue q2commit;

		// step 1, commit the missed counters of missed to Redis
		if (popEntireQueue(QT_Missed, q2commit, true) >0)
		{
			int64 stampStart = ZQ::common::now();
			size_t c =0;
			for (NamedCounterQueue::iterator it = q2commit.begin(); it< q2commit.end(); it++)
			{
				CounterFlow::NamedCounter nc = *it;
				nc.name = _dir.path() + SEPARATOR_OF_MISSED + nc.name + ":" CATEGORY_ACCESS_COUNTER; // TODO: + "@" +netId
				std::string strCnt = marshal(nc.cntr);

				ZQ::common::RedisSink::Ptr reply = new RedisCounterMerger(*this, nc);
				client->sendGETSET(nc.name.c_str(), (const uint8*) strCnt.c_str(), strCnt.length(), reply);

#pragma message ( __MSGLOC__ "TODO: Approach 1, impl CounterFlow::run() call RedisClient::sendGETSET() async")
				// _client->sendGETSET(strKey.c_str(), strCnt.c_str(), strCnt.length(), reply); // should have a dummy replay to make the invocation async
				c++;
			}
		
			_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CounterFlow, "dir[%s] committed %d counters of missed contents, took %dmsec"), 
				_dir.path().c_str(), c, (int) (ZQ::common::now() - stampStart));
		}

		// step 2. commit the hit counters of hit to Redis
		q2commit.clear();
		if (popEntireQueue(QT_Hit, q2commit, true) >0)
		{
			int64 stampStart = ZQ::common::now();
			size_t c =0;
			for (NamedCounterQueue::iterator it = q2commit.begin(); it< q2commit.end(); it++)
			{
				CounterFlow::NamedCounter nc = *it;
				nc.name = _dir.path() + SEPARATOR_OF_HIT + nc.name + ":" CATEGORY_ACCESS_COUNTER; // TODO: + "@" +netId
				std::string strCnt = marshal(nc.cntr);

				ZQ::common::RedisSink::Ptr reply = new RedisCounterMerger(*this, nc);
				client->sendGETSET(nc.name.c_str(), (const uint8*) strCnt.c_str(), strCnt.length(), reply);

#pragma message ( __MSGLOC__ "TODO: impl CounterFlow::run() call RedisClient::sendGETSET() to flush the hit counters")
			}

			_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CounterFlow, "dir[%s] committed %d counters of hit contents, took %dmsec"), 
				_dir.path().c_str(), c, (int) (ZQ::common::now() - stampStart));
		}

		// step 3, read the flush-demanded from Redis and update the queue in CacheDir for CacheFlusher to execute
		{
			std::string strKey = _dir.path() + ":" + "ToSave"; // TODO: + "@" +netId
#pragma message ( __MSGLOC__ "TODO: impl CounterFlow::run() call RedisClient to read the flush-demanded list and reset")
			// _client->sendGETSET(strKey.c_str(), "", 0, cbFlushDemandedList);
		}

		// step 4, commit the merged values based on the previous replies
		if (popEntireQueue(QT_Merged, q2commit, false) >0)
		{
			int64 stampStart = ZQ::common::now();
			size_t c =0, cBatches =0;
			std::string strValuePairs;
			for (NamedCounterQueue::iterator it = q2commit.begin(); it< q2commit.end(); it++)
			{
				// no name coversion needed here
				std::string strV = marshal(it->cntr);
				strValuePairs = it->name + " " + strV + " ";
				
				// always take SET withno reply for merged queue
				client->sendSET(it->name.c_str(), (const uint8*)strV.c_str(), strV.length());

/*
				if ((BATCH_SIZE-1) == c++ %BATCH_SIZE)
				{

#pragma message ( __MSGLOC__ "TODO: call RedisClient::sendMSET() to submit batch by batch, no reply needed")
				// _client->sendMSET(strKey.c_str(), strCnt.c_str(), strCnt.length(), reply); // should have a dummy replay to make the invocation async

					strValuePairs = "";
					cBatches++;
				}
*/
			}

			if (!strValuePairs.empty())
			{

#pragma message ( __MSGLOC__ "TODO: call RedisClient::sendMSET() to submit batch by batch, no reply needed")
				cBatches++;
			}

			_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CounterFlow, "dir[%s] merged %d counters and updated by %d batches, took %dmsec"), 
				_dir.path().c_str(), c, cBatches, (int) (ZQ::common::now() - stampStart));
		}

	} // while (!_dir._bQuit)

	return 0;
}

void CounterFlow::queueCounter(Category qt, const NamedCounter& nc)
{
	ZQ::common::MutexGuard g(_lkQueue);
	_cqueues[qt % COUNTER_QUEUES].push_back(nc);
}

size_t CounterFlow::inc(Counter& A, size_t c)
{
	A.c += c;
	A.stamp2ndLatest = A.stampLatest;
	A.stampLatest = ZQ::common::now();
	if (c>1)
		A.stamp2ndLatest = A.stampLatest;
	if (A.stampSince <=0)
		A.stampSince = A.stamp2ndLatest = A.stampLatest;
	return A.c;
}

CounterFlow::Counter CounterFlow::add(const Counter& A, const Counter& B)
{
	const Counter* acOld = &A, *acNew = &B;
	
	//swap to keep acOld, acNew true per stampLatest
	if (acOld->stampLatest > acNew->stampLatest)
	{
		const Counter* acTmp = acOld;
		acOld = acNew;
		acNew = acTmp;
	}

	Counter ac = *acNew;
	ac.c += acOld->c;
	ac.stampSince     = min(acOld->stampSince,  acNew->stampSince);
	ac.stamp2ndLatest = max(acOld->stampLatest, acNew->stamp2ndLatest);

	return ac;
}

CounterFlow::Counter CounterFlow::merge(const Counter& A, const Counter& B)
{
	const Counter* acOld = &A, *acNew = &B;
	
	//swap to keep acOld, acNew true per stampLatest
	if (acOld->stampLatest > acNew->stampLatest)
	{
		const Counter* acTmp = acOld;
		acOld = acNew;
		acNew = acTmp;
	}

	Counter ac = *acOld;

	do {
		// non-intersect case 1
		if (acOld->stampSince >= acOld->stampLatest)
		{
			ac = *acNew;
			ac.c += acOld->c /2; // round down for old count
			if (acOld->stampLatest >0 && acOld->stampLatest < acNew->stampSince)
				ac.stampSince = acOld->stampLatest;
			break;
		}

		// non-intersect case 4
		if (acOld->stampLatest <= acNew->stampSince)
		{
			ac.c = acOld->c + acNew->c;

			ac.stampLatest    = acNew->stampLatest;
			ac.stamp2ndLatest = max(acOld->stampLatest, acNew->stamp2ndLatest);
			ac.stampSince     = acOld->stampSince;
			break;
		}

		// intersect case
		int64 overlapWin = acOld->stampLatest - acNew->stampSince;

		size_t overLapCountOfOld = (size_t) (acOld->c * overlapWin / (acOld->stampLatest - acOld->stampSince));
		size_t overLapCountOfNew = (size_t) (acNew->c * overlapWin / (acNew->stampLatest - acNew->stampSince));

		ac.c = (size_t) (acOld->c + acNew->c - (overLapCountOfOld + overLapCountOfNew) /2);
		ac.stampLatest    = acNew->stampLatest;
		ac.stamp2ndLatest = max(acOld->stampLatest, acNew->stamp2ndLatest);
		ac.stampSince     = acOld->stampSince;

	} while (0);

	if (ac.c >1 && ac.stamp2ndLatest <=0)
		ac.stamp2ndLatest = ac.stampLatest;

	return ac;
}

size_t CounterFlow::attenuate(Counter& A, int64 winSince)
{
	size_t reduced =0;
	// adjust by the specified time window
	if (winSince > A.stampSince && A.stampLatest > A.stampSince)
	{
		reduced = (size_t) (A.c * (winSince - A.stampSince) / (A.stampLatest - A.stampSince));
		A.c -= reduced;
		A.stampSince  = winSince;
		// leave A.stamp2ndLatest beyond stampSince if (A.stamp2ndLatest > A.stampSince)
		// 	A.stamp2ndLatest = 0;
	}

	return reduced;
}


}} // namespace
