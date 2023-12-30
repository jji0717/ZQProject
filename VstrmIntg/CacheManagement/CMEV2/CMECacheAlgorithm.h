#ifndef _CMECACHEALGORITHM_H_
#define _CMECACHEALGORITHM_H_

#include <queue>
#include "FileLog.h"
#include "CacheStorage.h"

namespace CacheManagement {

class VSISEntity;
class VSISCacheIO;

typedef std::map<std::string, VSISEntity*>	VSISENTITIES;

class CMECacheAlgorithm : public CacheAlgorithm
{
public:
	struct lru2_greater : std::binary_function<Content*, Content*, bool>
	{
		bool operator()(Content* _X, Content* _Y) const
		{
			uint64 curtime = ZQ::common::now();
			uint64 xInterval = ( curtime > _X->getReferencePlayTime() ) ? ( curtime - _X->getReferencePlayTime() ) : 0;
			uint64 yInterval = ( curtime > _Y->getReferencePlayTime() ) ? ( curtime - _Y->getReferencePlayTime() ) : 0;

			// test 
//			glog(ZQ::common::Log::L_INFO, CLOGFMT(CMECacheAlgorithm, "LRU2: curTime=%llu X: paid=%s lastSecond=%s lastSecond=%llu interval=%llu "),
//				curtime, _X->_paid.c_str(), time64ToUTCwZoneStr(_X->getLastSecondPlayTime()).c_str(), _X->getLastSecondPlayTime(), xInterval);
			
//			glog(ZQ::common::Log::L_INFO, CLOGFMT(CMECacheAlgorithm, "LRU2: curTime=%llu Y: paid=%s lastSecond=%s lastSecond=%llu interval=%llu "),
//				curtime, _Y->_paid.c_str(), time64ToUTCwZoneStr(_Y->getLastSecondPlayTime()).c_str(), _Y->getLastSecondPlayTime(), yInterval);


			return ( xInterval < yInterval );
		}
	};

	typedef std::priority_queue <Content*, std::vector<Content*>, CMECacheAlgorithm::lru2_greater> CONTENT_LRU2_QUEUE;

public:
	CMECacheAlgorithm(ZQ::common::FileLog& cacheEventLog);
	virtual ~CMECacheAlgorithm();

public:
	bool importEvaluate(CacheStorage& cs, CacheIO& cacheIO, Content& content);
	bool deleteEvaluate(CacheStorage& cs, CONTENTS& contents, CONTENTS_ARRAY& candidated, uint64& toBeFreedSize);
	void agingContent(CacheStorage& cs, Content& content);

	void importCompleted(CacheStorage& cs, Content& content, bool success);
	void cacheDeleted(CacheStorage& cs, Content& content, bool success);

	virtual void printLog(char* buff);

	VSISEntity* candidateVSISEntity(VSISENTITIES& vsisEntities);
	CacheStorage* candidateCacheStorage(CACHE_STORAGES& cstorages);
	
	void printHitrate(CacheStorage& cs);
	void printHitrate(uint64 local, uint64 total);
	void printImportBW(VSISCacheIO& vsisCacheIO);

protected:
	uint32	_agingPeriodSeconds;	//
	uint32  _agingDenominator;		//

	uint32	_threshold;	         // Storage in use threshold (ex: 70%), less than this then import; value 0~100
	uint32	_cushion;			 // % of space to reserve, value 0 ~ 100
	uint32	_importTrigger;		 // play count before caching the content
	uint32	_importTrigger2;		 

	uint32  _bwThreshold;
	uint32  _bwReserved;

	ZQ::common::FileLog&	_cacheEventLog;
public:
	uint32 getCushion()				{ return _cushion; };
	uint32 getImportTrigger()		{ return _importTrigger; };

public:
	void setContentAgingParameters(uint32 periodSeconds, uint32 denominator);
	void setStorageParameters(uint32 threshold, uint32 cushion, uint32 importTrigger, uint32 importTrigger2);
	void setCacheBWParameters(uint32 threshold, uint32 reserved);
};

}
#endif
