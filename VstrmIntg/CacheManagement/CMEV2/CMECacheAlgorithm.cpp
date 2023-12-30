#include "CMECacheAlgorithm.h"
#include "VSISCacheIO.h"

#define MAX_DEL_CONTENT_COUNT_A_TIME	    20
#define MIN_AGING_WINDOW			4*3600   //  4 hours

#define DEFAULT_CUSHION                  5   //  5%
#define DEFAULT_IMPORT_THRESHOLD		70	 // 70%

namespace CacheManagement {

CMECacheAlgorithm::CMECacheAlgorithm(ZQ::common::FileLog& cacheEventLog)
:_cacheEventLog(cacheEventLog)
{
}

CMECacheAlgorithm::~CMECacheAlgorithm()
{
}

void CMECacheAlgorithm::setContentAgingParameters(uint32 periodSeconds, uint32 denominator)
{
	_agingPeriodSeconds = periodSeconds;
	_agingPeriodSeconds = (_agingPeriodSeconds < MIN_AGING_WINDOW) ? MIN_AGING_WINDOW : _agingPeriodSeconds;

	_agingDenominator = denominator;
	_agingDenominator < 1 ? 1 : _agingDenominator;
	_agingDenominator > 100 ? 100 : _agingDenominator;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMECacheAlgorithm, "agingPeriod=%d hours, agingDenominator=%d"),
		_agingPeriodSeconds/3600, _agingDenominator);

}

void CMECacheAlgorithm::setStorageParameters(uint32 threshold, uint32 cushion, uint32 importTrigger, uint32 importTrigger2)
{
	_cushion = cushion;
	_cushion = ( _cushion < (float)DEFAULT_CUSHION ) ? DEFAULT_CUSHION : _cushion;
	_cushion = ( _cushion >= 95.0f) ? DEFAULT_CUSHION : _cushion;

	_threshold = threshold;
	_threshold = ( _threshold > (uint32)(100.0f-_cushion)) ? DEFAULT_IMPORT_THRESHOLD : _threshold;

	_importTrigger = importTrigger;
	_importTrigger = (_importTrigger > 1) ? _importTrigger : 2;

	_importTrigger2 = importTrigger2;
	float minTrigger2 = (float)_importTrigger * 1.5f;
	if( (float)_importTrigger2 < minTrigger2)
		_importTrigger2 = (uint32)(minTrigger2 + 0.5f);

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMECacheAlgorithm, "threshold=%d%%, cushion=%d%%, importTrigger=%d, importTrigger2=%d"),
		_threshold, _cushion, _importTrigger, _importTrigger2);
}

void CMECacheAlgorithm::setCacheBWParameters(uint32 threshold, uint32 reserved)
{
	// _bwThreshold between 51 ~ 99
	_bwThreshold = threshold;
	_bwThreshold = _bwThreshold <= 50 ? 51 : _bwThreshold;
	_bwThreshold = _bwThreshold >= 99 ? 99 : _bwThreshold;

	_bwReserved = reserved;
}


bool CMECacheAlgorithm::importEvaluate(CacheStorage& cs, CacheIO& cacheIO, Content& content)
{
	VSISCacheIO* vsisCacheIO = (VSISCacheIO*) &cacheIO;

	uint64 freeSpace=0;
	uint64 totalSpace=0;
	cs.getUsage(freeSpace, totalSpace);

	if(0 == totalSpace || freeSpace > totalSpace)
		return false;

	// current space usage
	float curusg = 100.0f * (float)((double)(totalSpace-freeSpace) / (double)totalSpace);

	// current total import bw
	uint32 bwUsage = vsisCacheIO->getRecentImportBWUsage();
	
	// There's plenty of cache space, cache it anyway
	if(curusg < (float)_threshold)
	{
		if(bwUsage < 50)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(SeacCacheAlgorithm, "%s: current space usage is %.2f%%, less than threshold %d%%, importBWUsage=%d%%, going to import content pid=%s paid=%s anyway"),
				cs.getName().c_str(), curusg, _threshold, bwUsage, content._pid.c_str(), content._paid.c_str());

			_cacheEventLog(ZQ::common::Log::L_INFO, "%s: Decide to import content pid=%s paid=%s because space usage=%.2f%% threshold=%d%% cushion=%.2f%% importBWUsage=%d%%",
				cs.getName().c_str(), content._pid.c_str(), content._paid.c_str(), curusg, _threshold, (float)_cushion, bwUsage);

			return true;
		}
	}

	float curAdjPlay = content.getAdjPlay();
	// the space usage is over the threshold, check its plays count
	if(curAdjPlay > (float)_importTrigger)
	{
		if(bwUsage < _bwThreshold)	// meet importTrigger condition
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(SeacCacheAlgorithm, "%s: content pid=%s paid=%s adjusted play count is %.2f, larger than import trigger %d, importBWUsage=%d%%, going to import it"),
				cs.getName().c_str(), content._pid.c_str(), content._paid.c_str(), content.getAdjPlay(), _importTrigger, bwUsage);

			_cacheEventLog(ZQ::common::Log::L_INFO, "%s: Decide to import content pid=%s paid=%s because adjPlay=%.2f importTrigger=%d spaceUsage=%.2f%% threshold=%d%% cushion=%.2f%% importBWUsage=%d%%",
				cs.getName().c_str(), content._pid.c_str(), content._paid.c_str(), content.getAdjPlay(), _importTrigger, curusg, _threshold, (float)_cushion, bwUsage);

			return true;
		}
		else if(curAdjPlay > (float)_importTrigger2)  // meet importTrigger2 condition
		{
			if(vsisCacheIO->getRecentFreeBW() > 0)
			{
				glog(ZQ::common::Log::L_INFO, CLOGFMT(SeacCacheAlgorithm, "%s: content pid=%s paid=%s adjusted play count is %.2f, larger than import trigger2 %d, importBWUsage=%d%%, going to import it"),
					cs.getName().c_str(), content._pid.c_str(), content._paid.c_str(), content.getAdjPlay(), _importTrigger2, bwUsage);

				_cacheEventLog(ZQ::common::Log::L_INFO, "%s: Decide to import content pid=%s paid=%s because adjPlay=%.2f importTrigger2=%d spaceUsage=%.2f%% threshold=%d%% cushion=%.2f%% importBWUsage=%d%%",
					cs.getName().c_str(), content._pid.c_str(), content._paid.c_str(), content.getAdjPlay(), _importTrigger2, curusg, _threshold, (float)_cushion, bwUsage);

				return true;
			}
			else
			{
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(SeacCacheAlgorithm, "%s: content pid=%s paid=%s adjusted play count is %.2f, larger than import trigger2 %d, importBWUsage=%d%%, but no free import bandwidth, no import"),
					cs.getName().c_str(), content._pid.c_str(), content._paid.c_str(), content.getAdjPlay(), _importTrigger2, bwUsage);

				_cacheEventLog(ZQ::common::Log::L_WARNING, "%s: Not to import content pid=%s paid=%s because adjPlay=%.2f importTrigger2=%d spaceUsage=%.2f%% threshold=%d%% cushion=%.2f%% importBWUsage=%d%%, but no import bandwidth is available now",
					cs.getName().c_str(), content._pid.c_str(), content._paid.c_str(), content.getAdjPlay(), _importTrigger2, curusg, _threshold, (float)_cushion, bwUsage);

				return false;
			}
		}
	}

	return false;
}

bool CMECacheAlgorithm::deleteEvaluate(CacheStorage& cs, CONTENTS& contents, CONTENTS_ARRAY& candidated, uint64& toBeFreedSize)
{
	uint64 freeSpace=0;
	uint64 totalSpace=0;
	cs.getUsage(freeSpace, totalSpace);

	if(0 == totalSpace || freeSpace > totalSpace)
		return false;

	float curFreePer = 100.0f * (float)((double)freeSpace / (double)totalSpace);
	float targetFreePer = (float)_cushion;

	////// test code
	//curFreePer = 5.0f;
	//targetFreePer = 5.000001;
	if( targetFreePer < curFreePer )
	{
		if(rand()%10 == 1)	// with less log
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CMECacheAlgorithm, "%s: current free space %.2f%%, target free space %.2f%%, no content deletion required"),
				cs.getName().c_str(), curFreePer, targetFreePer);
		}
		return false;
	}

	float relPercentage = (float)2.0f * (targetFreePer-curFreePer);
	
	if(relPercentage > 1.0f)
	{
		relPercentage = 1.0f;
	}
	uint64 delSize = (uint64) ( (double)relPercentage * (double)totalSpace ) / 100;	// the storage space need to be released

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMECacheAlgorithm, "%s: current free space %.2f%%, target free space %.2f%%, going to release space %llu MB %.2f%% of totalSpace"),
			cs.getName().c_str(), curFreePer, targetFreePer, delSize/1000000, relPercentage);

	// priority the content based on reference play time
	Content* content = NULL;
	CONTENT_LRU2_QUEUE lru2Queue;
	CONTENTS::iterator itcnt;
	for(itcnt=contents.begin(); itcnt!=contents.end(); itcnt++)
	{
		content = (Content*) itcnt->second;
		if(content->getReferencePlayTime() > 0)
		{
			lru2Queue.push(itcnt->second);
		}
	}

	CONTENTS_ARRAY secRoundContents;

	int mainRoundCount = 0;
	int secRoundCount = 0;
	uint64 mainRoundSize = 0;
	uint64 secRoundSize = 0;

	bool mainRoundConditionMet = false;
	bool selected = false;
	while(!lru2Queue.empty())
	{
		Content* content= (Content*) lru2Queue.top();
		lru2Queue.pop();

		selected = false;	// used to avoid same content selected twice
		if(content->isScheduleEnded())		
		{	// content is not being viewed & vis exapired
			selected = true;
			mainRoundSize += content->getSize();
			mainRoundCount++;

			candidated.push_back(content);

			if(mainRoundCount > MAX_DEL_CONTENT_COUNT_A_TIME || mainRoundSize > delSize)
			{
				mainRoundConditionMet = true;
				break;
			}
		}

		// easy condition #2
		if(!selected && (secRoundCount < MAX_DEL_CONTENT_COUNT_A_TIME) && (secRoundSize < delSize) )
		{
			secRoundSize += content->getSize();
			secRoundCount++;

			secRoundContents.push_back(content);
		}
	}

	// copy the secondRound content to mainRound if mainRound did not meet. 
	int nCounter = 0;
	if(!mainRoundConditionMet)
	{
		while( nCounter<secRoundContents.size() && ((mainRoundCount < MAX_DEL_CONTENT_COUNT_A_TIME) || (mainRoundSize < delSize)) )
		{
			candidated.push_back(secRoundContents[nCounter]);
			
			mainRoundSize += secRoundContents[nCounter]->getSize();

			nCounter++;
			mainRoundCount++;
		}
	}
	// to be freed size
	toBeFreedSize = mainRoundSize;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMECacheAlgorithm, "%s: %d content are candidated for deletion to release space %llu MB, target release space %llu MB"),
		cs.getName().c_str(), candidated.size(), mainRoundSize/1000000, delSize/1000000);

	_cacheEventLog(ZQ::common::Log::L_INFO, "%s: %d content are candidated for deletion to release space %llu MB, target release space %llu MB %.2f%% of totalSpace",
		cs.getName().c_str(), candidated.size(), mainRoundSize/1000000, delSize/1000000, relPercentage);

	int i=0;
	for(i=0; i<candidated.size(); i++)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMECacheAlgorithm, "%s: No.%02d content pid=%s paid=%s is to release space %llu MB"),
			cs.getName().c_str(), i+1, candidated[i]->_pid.c_str(), candidated[i]->_paid.c_str(), candidated[i]->getSize()/1000000 );

		_cacheEventLog(ZQ::common::Log::L_INFO, "%s: Decide to delete content pid=%s paid=%s lastSecondPlayTime=%s lastPlayTime=%s referencePlayTime=%s adjPlay=%.2f size=%llu MB",
			cs.getName().c_str(), candidated[i]->_pid.c_str(), candidated[i]->_paid.c_str(), 
			time64ToUTCwZoneStr(candidated[i]->getLastSecondPlayTime()).c_str(), 
			time64ToUTCwZoneStr(candidated[i]->getLastPlayStartTime()).c_str(), 
			time64ToUTCwZoneStr(candidated[i]->getReferencePlayTime()).c_str(), 
			candidated[i]->getAdjPlay(),
			candidated[i]->getSize() /1000000);
	}

	return true;
}

void CMECacheAlgorithm::agingContent(CacheStorage& cs, Content& content)
{
	uint64 timeDiffSeconds;				// Difference between prev and current timestamps
	uint64 curtime = ZQ::common::now();

	uint64 lastPlaytime = content.getLastPlayStartTime();
	if(0 == lastPlaytime)
		return;

	float adjPlay = content.getAdjPlay();

	if(adjPlay <= 0)
	{
		adjPlay = 0;
		return;
	}
	// We only bother with calculating if the difference between the two is
	//  greater than a single 24 hour period
	//
	timeDiffSeconds = (uint32) ((curtime > lastPlaytime) ? (curtime-lastPlaytime)/1000 : 0);

	if(timeDiffSeconds > _agingPeriodSeconds)
	{
		float preAdjPlay = adjPlay;

		float times = (float)timeDiffSeconds / (float)_agingPeriodSeconds;			// Calculate # of 24 hour periods (integer)
		adjPlay = (adjPlay / ((float)_agingDenominator * times));	// add 1.0 to count this session

		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMECacheAlgorithm, "%s: content pid=%s paid=%s agjPlay now is aging from %.2f to %.2f, lastPlayTime=%s currentTime=%s elapsed %llu hours"),
			cs.getName().c_str(), content._pid.c_str(), content._paid.c_str(), preAdjPlay, adjPlay, 
			time64ToUTCwZoneStr(lastPlaytime).c_str(),  time64ToUTCwZoneStr(curtime).c_str(), timeDiffSeconds/3600 );

		_cacheEventLog(ZQ::common::Log::L_INFO, "%s: Decide to aging content pid=%s paid=%s adjPlay from %.2f to %.2f because lastPlayTime=%s currentTime=%s elapsed %llu hours",
			cs.getName().c_str(), content._pid.c_str(), content._paid.c_str(), 
			preAdjPlay, adjPlay, 
			time64ToUTCwZoneStr(lastPlaytime).c_str(),  time64ToUTCwZoneStr(curtime).c_str(), timeDiffSeconds/3600 );

	}

	content.setAdjPlay(adjPlay);
}

void CMECacheAlgorithm::importCompleted(CacheStorage& cs, Content& content, bool success)
{
	if(success)
	{
		_cacheEventLog(ZQ::common::Log::L_INFO, "%s: content pid=%s paid=%s import succeeded",
			cs.getName().c_str(), content._pid.c_str(), content._paid.c_str());
	}
	else
	{
		_cacheEventLog(ZQ::common::Log::L_INFO, "%s: content pid=%s paid=%s import failed",
			cs.getName().c_str(), content._pid.c_str(), content._paid.c_str());
	}
}

void CMECacheAlgorithm::cacheDeleted(CacheStorage& cs, Content& content, bool success)
{
	if(!success)
	{
		_cacheEventLog(ZQ::common::Log::L_INFO, "%s: content pid=%s paid=%s deletion failed",
			cs.getName().c_str(), content._pid.c_str(), content._paid.c_str());

		return;
	}

	uint64 lastPlaytime = content.getLastPlayStartTime();
	if(0 == lastPlaytime)
		return;

	float adjPlay = content.getAdjPlay();

	if(adjPlay <= 0)
	{
		adjPlay = 0;
		return;
	}
	
	float preAdjPlay = adjPlay;
	// reset the adjPlay insead of aging it, coz even after aging, the adjPlay may still lager than threshold and easy to get re-import by one play request
	adjPlay = 0.0f; // (adjPlay / (float)_agingDenominator);

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMECacheAlgorithm, "%s: content pid=%s paid=%s agjPlay now is aging from %.2f to %.2f because has been deleted from cache. Its lastPlayTime=%s"),
		cs.getName().c_str(), content._pid.c_str(), content._paid.c_str(), preAdjPlay, adjPlay, 
		time64ToUTCwZoneStr(lastPlaytime).c_str());

	_cacheEventLog(ZQ::common::Log::L_INFO, "%s: Decide to aging content pid=%s paid=%s lastPlayTime=%s, adjPlay from %.2f to %.2f because has been deleted from cache",
		cs.getName().c_str(), content._pid.c_str(), content._paid.c_str(), 
		time64ToUTCwZoneStr(lastPlaytime).c_str(), preAdjPlay, adjPlay);

	content.setAdjPlay(adjPlay);
}

void CMECacheAlgorithm::printLog(char* buff)
{
	_cacheEventLog(ZQ::common::Log::L_INFO, "%s", buff);
}

VSISEntity* CMECacheAlgorithm::candidateVSISEntity(VSISENTITIES& vsisEntities)
{
	if(vsisEntities.size() == 0)
		return false;

	VSISEntity* tmpVSIS = NULL;
	VSISEntity* avaVSIS = NULL;
	uint64 maxFreeImportBW = 0;
	
	VSISENTITIES::iterator itvsis;
	
	for(itvsis=vsisEntities.begin(); itvsis!=vsisEntities.end(); itvsis++)
	{
		tmpVSIS = itvsis->second;

		if( tmpVSIS->isConnected() && tmpVSIS->isAvailable() 
			&&  tmpVSIS->isRecentReported() && (tmpVSIS->getFreeImportBW() > 0) 
		  )
		{
			// this vsis reported within maxReportInterval & import bw has not reached the limitation
			if(tmpVSIS->getFreeImportBW() > maxFreeImportBW)
			{
				maxFreeImportBW = tmpVSIS->getFreeImportBW();
				avaVSIS = tmpVSIS;
			}
		}
	}

	if(NULL != avaVSIS)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMECacheAlgorithm, "%s: VSIS %s is candidated to do IO operation"),
			avaVSIS->_clusterID.c_str(), avaVSIS->_name.c_str());
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(CMECacheAlgorithm, "%s: No VSIS can be candidated to do IO operation"),
			tmpVSIS->_clusterID.c_str());
	}
	
	// it is possible return NULL if no proper node found
	return avaVSIS;
}

CacheStorage* CMECacheAlgorithm::candidateCacheStorage(CACHE_STORAGES& cstorages)
{
	if(cstorages.size() == 0)
		return NULL;

	uint64 totalHits = 0;
	uint64 local = 0;
	uint64 total = 0;
	
	CacheStorage* cstorage = NULL;
	CACHE_STORAGES::iterator itcs;
	// get total hits
	for(itcs=cstorages.begin(); itcs!=cstorages.end(); itcs++)
	{
		cstorage = (CacheStorage*)itcs->second;
		
		cstorage->getHits(local, total);
		totalHits += total;
	}
	
	if(0 == totalHits)
	{
		return (CacheStorage*)(cstorages.begin()->second);
	}

	uint32 randomPos = rand() % 90;		// total 100, 90 would be make sure hit the range
	uint32 percent = 0;

	for(itcs=cstorages.begin(); itcs!=cstorages.end(); itcs++)
	{
		cstorage = (CacheStorage*)itcs->second;

		// hit won't not change much between twice call
		cstorage->getHits(local, total);
		
		percent += (uint32)(( (double)total / (double)totalHits ) * (double)100.0f);

		if(percent <= randomPos)
		{
			return cstorage;
		}
	}

	return (CacheStorage*)(cstorages.begin()->second);
}


void CMECacheAlgorithm::printHitrate(CacheStorage& cs)
{
	uint64 local, total;
	cs.getHits(local, total);

	if(total >= local && 0 != total)
	{
		float hitrate = (float)100.0f * (float)((double)local / (double)total);

		_cacheEventLog(ZQ::common::Log::L_INFO, "%s: cache hitrate is %.2f%% (%llu:%llu)",
			cs.getName().c_str(), hitrate, local, total);
	}
}

void CMECacheAlgorithm::printHitrate(uint64 local, uint64 total)
{
	if(total >= local && 0 != total)
	{
		float hitrate = (float)100.0f * (float)((double)local / (double)total);

		_cacheEventLog(ZQ::common::Log::L_INFO, "system cache hitrate is %.2f%% (%llu:%llu)",
			hitrate, local, total);
	}
}

void CMECacheAlgorithm::printImportBW(VSISCacheIO& vsisCacheIO)
{
	uint64 free, used, total;
	uint32 usg;

	vsisCacheIO.getRecentImportBW(used, total);
	free = vsisCacheIO.getRecentFreeBW();
	usg = vsisCacheIO.getRecentImportBWUsage();

	_cacheEventLog(ZQ::common::Log::L_INFO, "%s: cluster wide recent valid freeImportBW=%llu Mbs usedImportBW=%llu Mbps totalImportBW=%llu Mbps importBWUsage=%d%%",
		vsisCacheIO.getClusterID().c_str(), free/1000000, used/1000000, total/1000000, usg);
}


}