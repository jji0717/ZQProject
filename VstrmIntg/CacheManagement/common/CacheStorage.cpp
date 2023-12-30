#include <sys/stat.h>
#include "CacheStorage.h"
#include "TimeUtil.h"



#define MAX_CONT_PLAY_TIME		        6 * 3600  // in seconds, 6 hours. A session suppose not to be played over 6 hours
#define MAX_CONT_PLAY_DUR_TIMES			2		  // 2 times of content duration if duraiton is available

#define MAX_CACHE_TIME				    4 * 3600  // seconds
#define MAX_CACHE_DUR_TIMES				1.5f	  // 1.5x duration (for VOD, pwe, don't know the duration)

#define MAX_MEDIATE_TIME				10		  // seconds

#define DEFAULT_CONTENT_CHECK_INTERVAL  24 * 3600  // one day

#define MIN_CONTENT_SAVE_DAYS		    5
#define MAX_CONTENT_SAVE_DAYS		    30
#define A_DAY_IN_SECONDS			    24*3600    // a day's seconds

#define MAX_FULL_SYNC_RETRY			    5          // 

#define DATETIME_NA					   "N/A"
#define DATA_KEY_VERSION			   "Version"
#define DATA_LINE_BUFF_SIZE				8192

#define DATA_PREV_VERSION				"V1.1"
#define DATA_CURR_VERSION				"V1.2"

namespace CacheManagement {


// convert the UTC time64 to local time string for logging 
std::string time64ToUTCwZoneStr(uint64 t)
{
	if(0 == t)
	{
		return std::string(DATETIME_NA);
	}

	char localtime[100]={0};
	ZQ::common::TimeUtil::TimeToUTC(t, localtime, 90, true);
	return std::string(localtime);
}

std::string CONTENT_STATUS_2_TEXT(Content::CONTENT_STATUS status)
{
	std::string strStatus;

	switch (status)
	{
	case Content::CNT_UNCACHED:
		strStatus = "CNT_UNCACHED";
		break;

	case Content::CNT_CACHE_PENDING:
		strStatus = "CNT_CACHE_PENDING";
		break;

	case Content::CNT_CACHING:
		strStatus = "CNT_CACHING";
		break;

	case Content::CNT_CACHED:
		strStatus = "CNT_CACHED";
		break;

	case Content::CNT_CACHE_FAIL:
		strStatus = "CNT_CACHE_FAIL";
		break;

	case Content::CNT_DELETING:
		strStatus = "CNT_DELETING";
		break;

	case Content::CNT_DELETED:
		strStatus = "CNT_DELETED";
		break;

	case Content::CNT_DELETE_FAILED:
		strStatus = "CNT_DELETE_FAILED";
		break;

	case Content::CNT_NOT_EXIST:
		strStatus = "CNT_NOT_EXIST";
		break;

	default:
		strStatus = "(Unknown Content status)";
		break;
	}

	return strStatus;
}

Content::CONTENT_STATUS CONTENT_STATUS_2_ENUM(std::string& strStatus)
{
	if(strStatus == "CNT_UNCACHED")
		return Content::CNT_UNCACHED;

	if(strStatus == "CNT_CACHE_PENDING")
		return Content::CNT_CACHE_PENDING;

	if(strStatus == "CNT_CACHING")
		return Content::CNT_CACHING;

	if(strStatus == "CNT_CACHED")
		return Content::CNT_CACHED;

	if(strStatus == "CNT_CACHE_FAIL")
		return Content::CNT_CACHE_FAIL;

	if(strStatus == "CNT_DELETING")
		return Content::CNT_DELETING;

	if(strStatus == "CNT_DELETED")
		return Content::CNT_DELETED;

	if(strStatus == "CNT_DELETE_FAILED")
		return Content::CNT_DELETE_FAILED;

	if(strStatus == "CNT_NOT_EXIST")
		return Content::CNT_NOT_EXIST;
	
	return Content::CNT_UNCACHED;
}
//////////////////////////////////////////////////////////////////
CacheIO::CacheIO()
{
}

CacheIO::~CacheIO()
{
}

void CacheIO::setOwner(CacheStorage& cs)
{
	_cstorage = &cs;
}

//////////////////////////////////////////////////////////////////
ContentOpData::ContentOpData(CONTENT_OP_TYPE type, uint64 timeStamp, uint64 interval, float playCount, bool isLocalHit)
{
	_type = type;
	_timeStamp = timeStamp;
	_interval = interval;
	_playCount = playCount;
	_isLocalHit = isLocalHit;
}

ContentOpData::~ContentOpData()
{
}

std::string ContentOpData::typeToStr()
{
	std::string typeStr;
	switch(_type)
	{
	case CNT_OP_PLAY:
		typeStr = "P";
		break;
	case CNT_OP_IMP:
		typeStr = "I";
		break;
	case CNT_OP_DEL:
		typeStr = "D";
		break;
	}
	return typeStr;
}

Content::Content(CacheStorage& cstorage, std::string& pid, std::string& paid)
:_cstorage(cstorage)
{
	_name = PAID_PID(paid,pid);

	_pid = pid;
	_paid = paid;

	_size = 0;
	_bitrate = 0;
	_duration = 0;

	_status = CNT_UNCACHED;
	
	_starts = 0;
	_finishes = 0;

	_nplay = 0;
	_adjplay = 0.0f;
	_peakplay = 0;

	_maxPlayDuration = 0;
	_minPlayDuration = 0;
	_avgPlayDuration = 0;
	_totalPlayDuration = 0;

	uint64 tnow = ZQ::common::now();

	_createTime = tnow;
	_lastStatusTime = tnow;

	_firstStartTime = 0;
	_lastStartTime = 0;
	_lastSecondStartTime = 0;		
	_referencePlayTime = tnow;		//set time, set _referencePlayTime to now if there is no 

	_scheduleStartTime = 0;
	_scheduleEndTime = 0;

	_lastCacheOpTime = 0;
	_imortTimesInWindow = 0;
	_deleteTimesInWindow = 0;
	_playsInWindow = 0;
	_localPlaysInWindow = 0;

	_lastCacheOpAdjPlay = 0.0f;
}

Content::~Content()
{
	ContentOpData* opData;
	CONTENTOPDATAS::iterator itOp;
	for(itOp=_operations.begin(); itOp!=_operations.end();itOp++)
	{
		opData = (ContentOpData*)(*itOp);
		
		delete opData;
	}
	_operations.clear();
}

std::string Content::statusToString()
{
	return CONTENT_STATUS_2_TEXT(_status);
}

void Content::setMetadata(uint32 bitrate, uint32 duration, uint64 size)
{
	// not replace original value if input value is not available
	if(bitrate != 0)
		_bitrate = bitrate;
	if(duration != 0)
		_duration = duration;
	if(size != 0)
		_size = size;

//	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s status=%s metadata updated with bitrate=%d duration=%d size=%llu"),
//		_cstorage.getName().c_str(), _pid.c_str(), _paid.c_str(), statusToString().c_str(), _bitrate, _duration, _size);	
}

void Content::setStatus(CONTENT_STATUS status)
{
	_status = status;
	_lastStatusTime = ZQ::common::now();
}

uint32 Content::lastUpdateToNowSeconds()
{
	uint64 timenow = ZQ::common::now();

	return (uint32) ((timenow > _lastStatusTime) ? (timenow - _lastStatusTime)/1000 : 0);
}

uint32 Content::lastPlayToNowSeconds()
{
	uint64 timenow = ZQ::common::now();
	
	return (uint32) ((timenow > _lastStartTime) ? (timenow - _lastStartTime)/1000 : 0);
}

void Content::calReferencePlayTime()       
{ 
	if(0 == _lastStartTime)
		return;

	if(0 == _lastSecondStartTime)
	{
		_referencePlayTime = _lastStartTime;
		return;
	}
	
	_referencePlayTime = _lastSecondStartTime + ((_lastStartTime > _lastSecondStartTime) ? (_lastStartTime - _lastSecondStartTime)/2 : 0);
}

bool Content::isCaching()
{
	if(CNT_CACHING == _status || CNT_CACHE_PENDING == _status || CNT_DELETING == _status)
		return true;
	return false;
}

bool Content::isLocal()
{
	if(CNT_CACHING==_status || CNT_CACHED==_status)
		return true;
	return false;
}

bool Content::checkPlayCount()
{
	if(_finishes == _starts)
	{	// is not beging viewed
		return false;
	}

	// is being viewed to see if timeout happens in case of playEnd event lost
	uint32 lastStartToNowSeconds = lastPlayToNowSeconds();
	uint32 timeoutSeconds = _duration > 0 ? (MAX_CONT_PLAY_DUR_TIMES * _duration) : MAX_CONT_PLAY_TIME;

	if(lastStartToNowSeconds > timeoutSeconds)
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s status=%s lastStartTime=%s starts=%d finishes=%d, no play stop after %d seconds."),
			_cstorage.getName().c_str(), _pid.c_str(), _paid.c_str(), statusToString().c_str(), time64ToUTCwZoneStr(_lastStatusTime).c_str(), _starts, _finishes, timeoutSeconds);

		_finishes = _starts;
		return true;
	}

	return false;
}

bool Content::playStart(uint64 time)
{
	if(_cstorage.isStatEnabled())
	{
		// save the play opt
		uint64 interval = 0;
		if(0 == _lastStartTime) // TBD: || _operations.size()==0) //size=0 means no new session since start and _lastStartTime was from data file
			interval = 0;
		else
			interval = time > _lastStartTime ? ((time - _lastStartTime)/1000) : 0;

		ContentOpData* playOpData = new ContentOpData(ContentOpData::CNT_OP_PLAY, time, interval, _adjplay, isLocal());
		_operations.push_back(CONTENTOPDATAS::value_type(playOpData));
		_cstorage.outputStatDataToFile(*this, *playOpData);
	}

	// save play count and time
	_starts++;
	_nplay++;

	if (_nplay > _peakplay)
	{
		_peakplay = _nplay;
	}
	
	if(0 == _firstStartTime)
	{
		_firstStartTime = time;
	}

	// set last start times	
	_lastSecondStartTime = _lastStartTime!=0 ? _lastStartTime : time;
	_lastStartTime = time;

	// calcuate the reference play time
	calReferencePlayTime();

	return true;
}

bool Content::playEnd(uint32 duration)
{
	bool bIdle = false;

	_finishes++;
	_nplay--;

	if(_finishes >= _starts)		// there is possible >, in case of finishes were reset when load from db file
	{
		bIdle = true;
		
		// reset if finishes==starts
		_finishes = _starts;		
	}
	
	if(duration > _maxPlayDuration)
		_maxPlayDuration = duration;

	if( (_minPlayDuration == 0) || (duration < _minPlayDuration) )
		_minPlayDuration = duration;

	_totalPlayDuration += duration;
	_avgPlayDuration = (uint32)(_totalPlayDuration / (uint64)_finishes);

	return bIdle;
}

bool Content::statusTimeout()
{
	bool ret=false;

	uint64 timeNow = ZQ::common::now();
	uint64 timeDiffSeconds = (timeNow > _lastStatusTime) ? (timeNow - _lastStatusTime)/1000 : 0;

	switch(_status)
	{
	case CNT_UNCACHED: 
	case CNT_CACHED:
	case CNT_CACHE_FAIL:
	case CNT_DELETED:
	case CNT_DELETE_FAILED:
	case CNT_NOT_EXIST:
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s status=%s lastStatusTime=%s, suppose not in the CACHING LIST"),
			_cstorage.getName().c_str(), _pid.c_str(), _paid.c_str(), statusToString().c_str(), time64ToUTCwZoneStr(_lastStatusTime).c_str());
		
		ret = false;
		break;

	case CNT_CACHING:
		{
			uint32 timeoutSeconds = _duration > 0 ? (uint32)((float)_duration * (float)MAX_CACHE_DUR_TIMES) : MAX_CACHE_TIME;
			if(timeDiffSeconds > timeoutSeconds)
			{
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s status=%s lastStatusTime=%s, timeout with %d seconds"),
					_cstorage.getName().c_str(), _pid.c_str(), _paid.c_str(), statusToString().c_str(), time64ToUTCwZoneStr(_lastStatusTime).c_str(), timeoutSeconds);
		
				ret = true;
			}
		}
		break;
		
	case CNT_CACHE_PENDING:
	case CNT_DELETING:
		if(timeDiffSeconds > MAX_MEDIATE_TIME)
		{
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s status=%s lastStatusTime=%s, timeout with %d seconds"),
				_cstorage.getName().c_str(), _pid.c_str(), _paid.c_str(), statusToString().c_str(), time64ToUTCwZoneStr(_lastStatusTime).c_str(), MAX_MEDIATE_TIME);

			ret = true;
		}
		break;

	default:
		break;
	}

	return ret;
}


void Content::saveOneImpOp()
{
	if(!_cstorage.isStatEnabled())
		return;

	uint64 timestamp = ZQ::common::now();

	uint64 interval = 0;
	if(0 == _lastCacheOpTime)
		interval = 0;
	else
		interval = timestamp > _lastCacheOpTime ? (timestamp - _lastCacheOpTime)/1000 : 0;

	_lastCacheOpTime = timestamp;

	ContentOpData* impOpData = new ContentOpData(ContentOpData::CNT_OP_IMP, timestamp, interval, _lastCacheOpAdjPlay);	
	_operations.push_back(CONTENTOPDATAS::value_type(impOpData));
	_cstorage.outputStatDataToFile(*this, *impOpData);
}

void Content::saveOneDelOp()
{
	if(!_cstorage.isStatEnabled())
		return ;

	uint64 timestamp = ZQ::common::now();

	uint64 interval = 0;
	if(0 == _lastCacheOpTime)
		return;		          // only save the del after there is already one import op
	else
		interval = timestamp > _lastCacheOpTime ? (timestamp - _lastCacheOpTime)/1000 : 0;

	_lastCacheOpTime = timestamp;

	ContentOpData* delOpData = new ContentOpData(ContentOpData::CNT_OP_DEL, timestamp, interval, _adjplay);
	_operations.push_back(CONTENTOPDATAS::value_type(delOpData));
	_cstorage.outputStatDataToFile(*this, *delOpData);
}

void Content::calPlaysInWindow(uint64 winStart, uint64 winStop)
{
	_playsInWindow = 0;
	_localPlaysInWindow = 0;
	_imortTimesInWindow = 0;
	_deleteTimesInWindow = 0;

	std::vector<ContentOpData*> toBeDeleted;

	ContentOpData* opData;
	CONTENTOPDATAS::iterator itOp;
	for(itOp=_operations.begin(); itOp!=_operations.end();)
	{
		opData = (ContentOpData*)(*itOp);

		if(opData->_timeStamp < winStart)
		{	// to remove the old data
			itOp = _operations.erase(itOp);	// itOp will move to next one by erase()
			toBeDeleted.push_back(opData);

			continue;
		}
		if(opData->_timeStamp > winStop)
		{	// reach the window
			break;
		}
		// in the window

		switch(opData->_type)
		{
		case ContentOpData::CNT_OP_PLAY:
			_playsInWindow++;
			if(opData->_isLocalHit)
			{
				_localPlaysInWindow++;
			}
			break;
		case ContentOpData::CNT_OP_IMP:
			_imortTimesInWindow++;
			break;
		case ContentOpData::CNT_OP_DEL:
			_deleteTimesInWindow++;
			break;
		}

		itOp++;
	}
	
	for(size_t i=0; i<toBeDeleted.size(); i++)
	{
		opData = (ContentOpData*) toBeDeleted[i];
		delete opData;
	}
}



CacheStorage::CacheStorage(std::string& name, CacheIO& io, CacheAlgorithm& algorithm)
:_cacheIO(io), _algorithm(algorithm)
{
	_name = name;
	_bThreadRunning = false;
	_checkInterval = 30 * 1000; // ms
	_cacheIO.setOwner(*this);

	_totalSpace = 2000000000000;			// 2T
	_freeSpace = 5 * (_totalSpace / 100);	

	_subFileCount = 3;		// default 3

	_supportDataFile = true;
	_syncAtStart = false;

	_localHits = 0;
	_totalHits = 0;

	_trimInSeconds = 7 * A_DAY_IN_SECONDS;
	_cntCheckInterval = DEFAULT_CONTENT_CHECK_INTERVAL;

	_statEnabled = false;
	_statWindow = 24*3600;		// 24 hours
	_statInterval = 3600;       // 1  hour
	_statDelay = 2*3600;        // 2 hours
	_statExtraSizePer = 0;
	_statDay = 0;
	_statPrintDetails = false;

	_cacheRetryInterval = 300;	// 5 minutes
}

CacheStorage::~CacheStorage()
{
}


bool CacheStorage::initialize(bool syncAtStart)
{
	_syncAtStart = syncAtStart;

	if(_bThreadRunning)
		return true;

	_bThreadRunning = true;
	ZQ::common::NativeThread::start();

	return true;
}

void CacheStorage::uninitialize()
{
	if(!_bThreadRunning)
		return ;

	_bThreadRunning = false;
	_waitEvent.signal();

	waitHandle(5000);
}

bool CacheStorage::setDataFile(std::string& dataType, std::string& dataPath, int32 flushInterval)
{
	_dataFlushInterval = flushInterval;
	if(_dataFlushInterval < 10 || _dataFlushInterval > 10*60)
		_dataFlushInterval = 5*60;	

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: dataType=%s dataPath=%s flushInterval=%d seconds"),
		_name.c_str(), dataType.c_str(), dataPath.c_str(), _dataFlushInterval);

	_dataType = dataType;
	_dataPath = dataPath;

	_supportDataFile = true;

	// generate the full name
	_dataFileName = _dataPath + _name + std::string("_contents.dat");

	return true;
}

void CacheStorage::setSubFileCount(uint32 subFileCount)
{
	_subFileCount = subFileCount;
	_subFileCount = _subFileCount > 7 ? 7 : _subFileCount;
	_subFileCount = _subFileCount < 1 ? 1 : _subFileCount;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: subFileCount=%d"),
		_name.c_str(), _subFileCount);
}


void CacheStorage::setDefaultVisa(uint32 visa)
{
	_defVisa = visa;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: default visa=%d seconds"),
		_name.c_str(), _defVisa);
}

void CacheStorage::setTrimdays(uint32 days)
{
	days = days < MIN_CONTENT_SAVE_DAYS ? MIN_CONTENT_SAVE_DAYS : days;
	days = days > MAX_CONTENT_SAVE_DAYS ? MAX_CONTENT_SAVE_DAYS : days;

	_trimInSeconds = (uint64)days * A_DAY_IN_SECONDS;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: default trimDays=%llu"),
		_name.c_str(), _trimInSeconds/A_DAY_IN_SECONDS);
}

void CacheStorage::setCushion(uint32 cushion)
{
	_cushion = cushion;

	_cushion = _cushion >= 95 ?  95 : _cushion;
	_cushion = _cushion <   1 ?   1 : _cushion;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: default cushion=%d"),
		_name.c_str(), _cushion);
}

void CacheStorage::setStatParams(bool enabled, std::string& dataPath, uint32 window, uint32 interval, uint32 delay, uint32 extraSizePer, bool printDetails)
{
	_statEnabled =  enabled;

	_statDataPath = dataPath;

	_statInterval = interval < 60 ? 60 : interval;
	
	_statWindow = window;
	//_statWindow = _statWindow < 1800 ? 1800 : _statWindow;

	_statDelay = delay;

	_statExtraSizePer = extraSizePer;
	_statExtraSizePer = _statExtraSizePer > 60 ? 60 : _statExtraSizePer;

	_statPrintDetails = printDetails;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: statistics enabled=%d dataPath=%s window=%d seconds interval=%d seconds delay=%d seconds statExtraSizePer=%d%% printDetails=%d"),
		_name.c_str(), enabled ? 1:0, dataPath.c_str(), window, interval, delay, _statExtraSizePer, _statPrintDetails ? 1 : 0);
}

void CacheStorage::setCacheRetryInterval(uint32 interval)
{
	_cacheRetryInterval = interval;
	_cacheRetryInterval = (0 == _cacheRetryInterval) ? 300 : _cacheRetryInterval;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: Proactive CacheRetryInterval=%d seconds"),
		_name.c_str(), _cacheRetryInterval);
}

void CacheStorage::setUsage(uint64 freeSize, uint64 totalSize)
{
	_freeSpace = freeSize;
	_totalSpace = totalSize;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: update storage usage freeSpace=%llu MB totalSpace=%llu MB free=%.2f%%"),
		_name.c_str(), _freeSpace/1000000, _totalSpace/1000000, 
		_totalSpace > 0 ? 100.0f * (float)((double)_freeSpace / (double)_totalSpace) : 100);
}

void CacheStorage::getUsage(uint64& freeSize, uint64& totalSize)
{
	freeSize = _freeSpace;
	totalSize = _totalSpace;
}

int CacheStorage::setNewline(char* buff, int len)
{
#ifdef ZQ_OS_MSWIN
	buff[len] = '\r';
	buff[len+1] = '\n';
	buff[len+2] = '\0';

	return len+2;
#else 
	buff[len] = '\n';
	buff[len+1] = '\0';

	return len+1;
#endif
}

// <key>:<value>;<key>:<value> ....
bool CacheStorage::parseKeyValue(char* buff, std::map<std::string, std::string>& keyValues)
{
	char key[100], value[100];

	char* posOut;
	char* posIn;
	char* curBuff = buff;

	do
	{
		posOut = strchr(curBuff, ';');
		if(NULL == posOut)
		{
			posIn = strchr(curBuff, ':');
			if(posIn != NULL)
			{
				memset(key, 0x0, sizeof(key));
				memset(value, 0x0, sizeof(value));

				strncpy(key, curBuff, posIn-curBuff);
				strcpy(value, posIn+1);
				
				keyValues.insert(std::map<std::string, std::string>::value_type(std::string(key), std::string(value)));
			}
			break;
		}
		else
		{
			posIn = strchr(curBuff, ':');
			if( !(posIn == NULL || posIn >= posOut) )
			{
				memset(key, 0x0, sizeof(key));
				memset(value, 0x0, sizeof(value));

				strncpy(key, curBuff, posIn-curBuff);
				strncpy(value, posIn+1, posOut-posIn-1);
				keyValues.insert(std::map<std::string, std::string>::value_type(std::string(key), std::string(value)));
			}

			curBuff = posOut+1;
		}
	}while(posOut != NULL);
	
	return keyValues.size() > 0;
}

// this routine has to be called before this thread start
bool CacheStorage::loadData()
{
	if(!_supportDataFile)
	{
		return false;
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: start to load data file %s"),
		_name.c_str(), _dataFileName.c_str());

	// will add version condition
	bool ret = loadTxtData();

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: finish to load data file %s"),
		_name.c_str(), _dataFileName.c_str());

	return ret;
}

void CacheStorage::postworkLoadData(Content& cnt)
{
	if(Content::CNT_UNCACHED != cnt.getStatus())
	{	
		// calculate the reference time // temp for last & last2nd implementation
		cnt.calReferencePlayTime();

		// set idle status since I don't know the content is streaing or not
		makeContentIdle(cnt);
	}

	// check content if need to import
	if( Content::CNT_UNCACHED == cnt.getStatus() && cnt._scheduleStartTime != 0)
	{
		uint64 tnow = ZQ::common::now();
		if( tnow > cnt._scheduleStartTime && 
			tnow < ( cnt._scheduleEndTime - (uint64)(_cacheRetryInterval*1000) )  )
		{
			makeContentScheduled(cnt);
		}
	}
}

bool CacheStorage::writeData()
{
	if(!_supportDataFile)
	{
		return false;
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: start to write data file %s"),
		_name.c_str(), _dataFileName.c_str());

	// will add version condition
	bool ret = writeTxtData();

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: finish to write data file %s"),
		_name.c_str(), _dataFileName.c_str());

	return ret;
}

bool CacheStorage::loadTxtData()
{	
	std::ifstream datRd(_dataFileName.c_str());
	 
	if(!datRd.is_open())
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: failed to open data file %s, no data will be read"),
			_name.c_str(), _dataFileName.c_str());
		return false;
	}
	
	char lineBuff[DATA_LINE_BUFF_SIZE];
	datRd.getline(lineBuff, DATA_LINE_BUFF_SIZE);
	
	if(datRd.rdstate() != std::ifstream::goodbit)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: failed to read first line from file data file %s"),
			_name.c_str(), _dataFileName.c_str());

		datRd.close();
		return false;
	}

	int len = strlen(lineBuff);
	if(len < 5)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: data file currupted, first line should be: Version:<version>;Time:<Time>"),
			_name.c_str());

		datRd.close();
		return false;
	}

	// remove new line from the buffer
#ifdef ZQ_OS_MSWIN
	lineBuff[len-3] = '\0';
#else 
	lineBuff[len-2] = '\0';	
#endif
	std::map<std::string, std::string> keyvalues;
	
	std::string verInFile;

	bool ret = parseKeyValue(lineBuff, keyvalues);
	if(ret)
	{
		std::map<std::string, std::string>::iterator it = keyvalues.find(DATA_KEY_VERSION);
		if(it == keyvalues.end())
		{
			ret = false;
		}
		else
		{
			verInFile = it->second;
		}
	}

	if(!ret)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: data file currupted, first line should be: Version:<version>;Time:<Time>"),
			_name.c_str());

		datRd.close();
		return false;
	}

	// reaa line, but don't parse this line, this is title line
	datRd.getline(lineBuff, DATA_LINE_BUFF_SIZE);

	std::string sPID, sPAID;
	char szPid[100], szPaid[100], szStatus[100], szStatusTime[100];
	int32 bitrate, duration, size;
	float adjPlays;
	int32 starts, finishes, minPlayTime, avgPlayTime, maxPlayTime;
	char szFirst[100], szLastSecond[100], szLast[100];
	char szSchdStart[100], szSchdEnd[100];

	// PID    PAID   STATUS    STATUSTIME    BITRATE    DURATION    SIZE    
	// 1. %s  2.%s   3.%s      4.%s          5.%d       6.%d        7.%d      
	
	// PLAYS    STARTS    FINISHES    MINPLAYTIME    AVGPLAYTIME    MAXPLAYTIME    
	// 8.%f     9.%d      10.%d        11.%d          12.%d          13.%d             
	
	// 1STPLAY     LAST2NDPLAY    LASTPLAY   
	// 14.%s       15.%s          16.%s      

	// SCHDSTART   SCHDEND
	// 17.%s       18.%s

	// read the real data  lines
	std::string prePaidPid;
	int recordsCount = 0;
	int narg = 0;
	
	datRd.getline(lineBuff, DATA_LINE_BUFF_SIZE); // eofbit
	while( !datRd.eof() && datRd.good() )
	{
		memset(szPid,0x00, 100); memset(szPaid,0x00, 100); memset(szStatus,0x00, 100); memset(szStatusTime, 0x00, 100);
		bitrate=0; duration=0; size=0;
		adjPlays = 0.0f; starts = 0; finishes = 0; minPlayTime = 0; avgPlayTime = 0; maxPlayTime = 0;
		memset(szFirst, 0x00, 100); memset(szLastSecond, 0x00, 100); memset(szLast, 0x00, 100);
		memset(szSchdStart, 0x00, 100); memset(szSchdEnd, 0x00, 100);

		if(DATA_CURR_VERSION == verInFile)
		{
			narg = sscanf(lineBuff, "%s %s %s %s %d %d %llu"
									" %f %d %d %d %d %d"
									" %s %s %s"
									" %s %s\n", 
									&szPid[0], &szPaid[0], &szStatus[0], &szStatusTime[0], &bitrate, &duration, &size, 
									&adjPlays, &starts, &finishes, &minPlayTime, &avgPlayTime, &maxPlayTime, 
									&szFirst[0], &szLastSecond[0], &szLast[0], 
									&szSchdStart[0], &szSchdEnd[0]);
		}
		else
		{
			narg = sscanf(lineBuff, "%s %s %s %s %d %d %llu"
									" %f %d %d %d %d %d"
									" %s %s %s\n", 
									&szPid[0], &szPaid[0], &szStatus[0], &szStatusTime[0], &bitrate, &duration, &size, 
									&adjPlays, &starts, &finishes, &minPlayTime, &avgPlayTime, &maxPlayTime, 
									&szFirst[0], &szLastSecond[0], &szLast[0]);
		}
		if(narg < 4)
		{
			len = strlen(lineBuff);
			if(len>3)
			{
#ifdef ZQ_OS_MSWIN
				lineBuff[len-3] = '\0';
#else 
				lineBuff[len-2] = '\0';	
#endif
			}
			glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: Failed to parse from data line: %s"),
				_name.c_str(), lineBuff);

			continue;
		}

		// use this to step out the loop if there is other io error 
		// just make sure no dead loop here
		sPID = std::string(szPid);
		sPAID = std::string(szPaid);
		if(prePaidPid == PAID_PID(sPAID,sPID))
		{
			break;
		}
		prePaidPid = PAID_PID(sPAID,sPID);
	
		// print log of this population	
		recordsCount++;
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: No.%04d content pid=%s paid=%s status=%s populated from data file"),
			_name.c_str(), recordsCount, szPid, szPaid, szStatus);

		std::string strStatus = std::string(szStatus);
		Content::CONTENT_STATUS cntStatus = CONTENT_STATUS_2_ENUM(strStatus);
		
		if(Content::CNT_CACHED != cntStatus)
		{	// reset all the middle-status to CNT_UNCACHED as we don't know what happened between the stop period of time
			// if the content is really caching, the status will be updated in sync operation after loading local data
			cntStatus = Content::CNT_UNCACHED;
		}
		
		Content* content = createContentAndSave(sPID, sPAID, cntStatus, bitrate>0 ? bitrate:0, duration>0 ? duration:0, size>0 ? size:0);
		
		content->_lastStatusTime = strcmp(szStatusTime, DATETIME_NA) == 0 ? 0 : ZQ::common::TimeUtil::ISO8601ToTime(szStatusTime);
		content->setAdjPlay(adjPlays<0.0f ? 0:adjPlays);
		content->_starts = starts>0 ? starts:0;
		content->_finishes = content->_starts;			// force it to starts coz there's possible there are finishes missed
		content->_minPlayDuration = minPlayTime>0 ? minPlayTime:0;
		content->_avgPlayDuration = avgPlayTime>0 ? avgPlayTime:0;
		content->_maxPlayDuration = maxPlayTime>0 ? maxPlayTime:0;
		content->_firstStartTime = strcmp(szFirst, DATETIME_NA) == 0 ? 0 : ZQ::common::TimeUtil::ISO8601ToTime(szFirst);
		content->_lastSecondStartTime = strcmp(szLastSecond, DATETIME_NA) == 0 ? 0 : ZQ::common::TimeUtil::ISO8601ToTime(szLastSecond);
		content->_lastStartTime = strcmp(szLast, DATETIME_NA) == 0 ? 0 : ZQ::common::TimeUtil::ISO8601ToTime(szLast);
		content->_scheduleStartTime = strcmp(szSchdStart, DATETIME_NA) == 0 ? 0 : ZQ::common::TimeUtil::ISO8601ToTime(szSchdStart);
		content->_scheduleEndTime = strcmp(szSchdEnd, DATETIME_NA) == 0 ? 0 : ZQ::common::TimeUtil::ISO8601ToTime(szSchdEnd);

		// post work on the loaded content
		postworkLoadData(*content);

		// get a new line
		datRd.getline(lineBuff, DATA_LINE_BUFF_SIZE);
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: total %d valid content records were populated from data file %s"),
		_name.c_str(), recordsCount, _dataFileName.c_str());

	// close file
	datRd.close();

	return true;
}


bool CacheStorage::writeTxtData()
{	
	std::ofstream datWt(_dataFileName.c_str(), std::ofstream::out|std::ofstream::trunc);
	
	if(!datWt.is_open())
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: failed to open data file %s, no data will be writen"),
			_name.c_str(), _dataFileName.c_str());
		return false;
	}
	
	// PID    PAID   STATUS    STATUSTIME    BITRATE    DURATION    SIZE    
	// 1. %s  2.%s   3.%s      4.%s          5.%d       6.%d        7.%d      
	
	// PLAYS    STARTS    FINISHES    MINPLAYTIME    AVGPLAYTIME    MAXPLAYTIME    
	// 8.%f     9.%d      10.%d        11.%d          12.%d          13.%d             
	
	// 1STPLAY     LAST2NDPLAY    LASTPLAY
	// 14.%s       15.%s          16.%s      

	// SCHDSTART   SCHDEND
	// 17.%s       18.%s

	int len;
	char lineBuff[DATA_LINE_BUFF_SIZE];

	// write the version informaiton first
	//
	// don't add \r\n etc at the end of the line here
	len = snprintf(lineBuff, DATA_LINE_BUFF_SIZE-3, "Version:%s;Time:%s", DATA_CURR_VERSION, time64ToUTCwZoneStr(ZQ::common::now()).c_str());
	len = setNewline(lineBuff, len);	
	datWt.write(lineBuff, len);

	// 
	len = snprintf(lineBuff, DATA_LINE_BUFF_SIZE-3, "%20s  %36s  %20s  %30s  %7s  %8s  %12s" 
													 "  %5s  %6s  %8s  %11s  %11s  %11s"
													 "  %30s  %30s  %30s"
													 "  %30s  %30s",
													 "PID", "PAID", "STATUS", "STATUSTIME", "BITRATE", "DURATION", "SIZE",
													 "PLAYS", "STARTS", "FINISHES", "MINPLAYTIME", "AVGPLAYTIME", "MAXPLAYTIME", 
													 "1STPLAY", "LAST2NDPLAY", "LASTPLAY", "SCHDSTART", "SCHDEND");								 
	len = setNewline(lineBuff, len);
	datWt.write(lineBuff, len);

	int recordsCount=0;
	CONTENTS::iterator itcnt;
	// save cached content to file
	for(itcnt=_cachedContents.begin(); itcnt!=_cachedContents.end(); itcnt++)
	{
		recordsCount++;

		Content* content = itcnt->second;

		len = snprintf(lineBuff, DATA_LINE_BUFF_SIZE-3, "%20s  %36s  %20s  %30s  %7d  %8d  %12llu" 
														 "  %5.2f  %6d  %8d  %11d  %11d  %11d"
														 "  %30s  %30s  %30s"
													     "  %30s  %30s",
														 content->_pid.c_str(), content->_paid.c_str(), 
														 content->statusToString().c_str(), time64ToUTCwZoneStr(content->_lastStatusTime).c_str(),
														 content->_bitrate, content->_duration, content->_size, 

														 content->getAdjPlay(), content->_starts, content->_finishes, 
														 content->_minPlayDuration, content->_avgPlayDuration, content->_maxPlayDuration, 

														 time64ToUTCwZoneStr(content->_firstStartTime).c_str(), 
														 time64ToUTCwZoneStr(content->_lastSecondStartTime).c_str(),
														 time64ToUTCwZoneStr(content->_lastStartTime).c_str(),

														 time64ToUTCwZoneStr(content->_scheduleStartTime).c_str(), 
														 time64ToUTCwZoneStr(content->_scheduleEndTime).c_str() );
		len = setNewline(lineBuff, len);
		datWt.write(lineBuff, len);
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: total %d cached content records were write into data file %s"),
		_name.c_str(), recordsCount, _dataFileName.c_str());

	// save the uncached data file
	recordsCount = 0;
	for(itcnt=_uncachedContents.begin(); itcnt!=_uncachedContents.end(); itcnt++)
	{
		recordsCount++;

		Content* content = itcnt->second;

		len = snprintf(lineBuff, DATA_LINE_BUFF_SIZE-3, "%20s  %36s  %20s  %30s  %7d  %8d  %12llu" 
														 "  %5.2f  %6d  %8d  %11d  %11d  %11d"
														 "  %30s  %30s  %30s"
													     "  %30s  %30s",
														 content->_pid.c_str(), content->_paid.c_str(), 
														 content->statusToString().c_str(), time64ToUTCwZoneStr(content->_lastStatusTime).c_str(),
														 content->_bitrate, content->_duration, content->_size, 

														 content->getAdjPlay(), content->_starts, content->_finishes, 
														 content->_minPlayDuration, content->_avgPlayDuration, content->_maxPlayDuration, 

														 time64ToUTCwZoneStr(content->_firstStartTime).c_str(), 
														 time64ToUTCwZoneStr(content->_lastSecondStartTime).c_str(),
														 time64ToUTCwZoneStr(content->_lastStartTime).c_str(),

														 time64ToUTCwZoneStr(content->_scheduleStartTime).c_str(), 
														 time64ToUTCwZoneStr(content->_scheduleEndTime).c_str() );

		len = setNewline(lineBuff, len);
		datWt.write(lineBuff, len);
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: total %d uncached content records were write into data file %s"),
		_name.c_str(), recordsCount, _dataFileName.c_str());

	// close file
	datWt.close();

	return true;
}

int CacheStorage::run(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: enter run()"), 
		_name.c_str());

	// read data from local db
	loadData();
	// wait until IO is ready
	bool bReady = false;
	while(_bThreadRunning && !bReady)
	{
		bReady = _cacheIO.isReady();
		if(!bReady)
		{
			sleep(1000);
		}
	}

	bool bFirstRun = true;
	uint64 runStartTime = ZQ::common::now();
	uint64 timenow;

	// sync all content 
	bool bSynced = false;
	int tryCount = 0;
	while(_syncAtStart && _bThreadRunning && !bSynced)
	{
		tryCount++;

		bSynced = doIOListContent();
		if(!bSynced)
		{
			if(tryCount>MAX_FULL_SYNC_RETRY)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(CacheStorage, "%s: Failed to do content full sync from IO after %d retries. No full sync was done"),
					_name.c_str(), MAX_FULL_SYNC_RETRY);
				break;
			}

			sleep(1000);
		}
	}
	if(_syncAtStart && bSynced)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: Full sync content request to IO is done"),
			_name.c_str());
	}

	int ncount = 0;
	ContentEvent* evt = NULL;

	// execution time
	uint64 lastTimeoutExecuted = ZQ::common::now();
	uint64 lastDataFlush = ZQ::common::now();
	uint64 lastContentsCheck = ZQ::common::now();
	uint64 lastStatistics = ZQ::common::now();
	uint64 nextWinStartTime = lastStatistics;

	uint32 waittime = _checkInterval;

	// get mday of local time
	time_t localNow = time(NULL);
	struct tm* localtmNow = localtime(&localNow);
	int dayOfCheckUncached = localtmNow->tm_mday;
	int dayOfReset = localtmNow->tm_mday;

	while(_bThreadRunning)
	{
		SYS::SingleObject::STATE st = _waitEvent.wait(waittime);
		switch(st)
		{
		case SYS::SingleObject::SIGNALED:
			while(_bThreadRunning && !_cntEvents.empty())
			{
				_cntEventLock.ReadLock();

				evt = _cntEvents.front();
				_cntEvents.pop();

				_cntEventLock.ReadUnlock();

				// process this event
				evt->process();
				
				// free the memory
				delete evt;

				// let the timeout get chance to executed since there is many playStart/Stop events in busy hour 
				if((ZQ::common::now() - lastTimeoutExecuted) >= waittime)
				{
					waittime = 0;
					break;		// break the internal while() loop
				}
			}
			
			break;
		case SYS::SingleObject::TIMEDOUT:
			// values are used in multiple times in following codes
			timenow = ZQ::common::now();
			localNow = time(NULL);
			localtmNow = localtime(&localNow);
			
			// reset hitrate
			if(dayOfReset != localtmNow->tm_mday)
			{
				_localHits = 0;
				_totalHits = 0;
				dayOfReset = localtmNow->tm_mday;

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: hitrate now is reset"),
					_name.c_str());
			}

			// calcuate and print the hitrate
			calcHitrate();
			// check cushion usage
			checkUsage();
			// check if there is content status timeout
			checkCachingList();
			// check the scheduled import list
			checkScheduledImport();

			// check cached list every 15 minutes
			if( (timenow - lastContentsCheck)/1000 >= 900)
			{				
				// recheck the "cached" content but sync did not return this content status if IO is ASYNC
				if(bFirstRun)
				{
					_cntCheckInterval = (timenow - runStartTime)/1000 + 1;
					bFirstRun = false;
				}
				else
				{
					_cntCheckInterval = DEFAULT_CONTENT_CHECK_INTERVAL;
				}
				// check contents
				checkCachedContents();

				// 
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: there are %d contents in IDLE LIST"),
					_name.c_str(), _idleContents.size() );

				// check uncached content during 3:00 ~ 4:00
				int mi = rand() % 50;
				if( (3 == localtmNow->tm_hour) && (localtmNow->tm_min >= mi) && (dayOfCheckUncached != localtmNow->tm_mday) )
				{	// only executed once a day
					dayOfCheckUncached = localtmNow->tm_mday;	

					checkUncachedContent();
				}

				// reset the last time
				lastContentsCheck = ZQ::common::now();
			}

			// flush data to file
			if((timenow - lastDataFlush) >= _dataFlushInterval*1000)
			{
				// flush data to disk
				writeData();

				lastDataFlush = timenow;
			}

			if( _statEnabled 
				&& (timenow - runStartTime)/1000 >= (_statDelay+_statWindow)
				&& (timenow - lastStatistics)/1000 >= _statInterval )
			{
				// flush the data to file before calculating
				try
				{
					_statOutput.flush();
				}
				catch(std::ifstream::failure e)
				{
					glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: flush stat file failed with error: %s"), 
						_name.c_str(), e.what());
				}
				// set the window start/end
				uint64 winStart = nextWinStartTime;
				uint64 winStop = winStart + (uint64)(_statWindow*1000);
				
				nextWinStartTime = winStart + uint64(_statInterval*1000);
				lastStatistics = timenow;

				doStatistics(winStart, winStop);
			}

			// reset the timer
			lastTimeoutExecuted = ZQ::common::now();
			if(0 == waittime)
			{
				waittime = _checkInterval;
				_waitEvent.signal();
			}

			break;
		default:
			break;
		}
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: exit run()"), 
		_name.c_str());

	return 0;
}

void CacheStorage::final(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: enter final()"), 
		_name.c_str());

	// flush data before exit
	writeData();
	
	// close the statistics file
	_statOutput.close();

	ContentEvent* evt = NULL;
	while(!_cntEvents.empty())
	{
		evt = (ContentEvent*) _cntEvents.front();
		_cntEvents.pop();

		delete evt;
	}
		
	Content* content = NULL;
	CONTENTS::iterator itcnt;
	while(_uncachedContents.size()>0)
	{
		itcnt = _uncachedContents.begin();
		content = (Content*) itcnt->second;

		_uncachedContents.erase(itcnt);
		delete content;
	}

	while(_cachedContents.size()>0)
	{
		itcnt = _cachedContents.begin();
		content = (Content*) itcnt->second;

		_cachedContents.erase(itcnt);
		delete content;
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: exit final()"), 
		_name.c_str());
}

void CacheStorage::calcHitrate()
{
	if(_totalHits >= _localHits && 0 != _totalHits)
	{
		float hitrate = (float)100.0f * (float)((double)_localHits / (double)_totalHits);

		glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: cache hitrate is %.2f%% (%llu:%llu)"),
			_name.c_str(), hitrate, _localHits, _totalHits);

	}
}

void CacheStorage::doStatistics(uint64 winStart, uint64 winStop)
{
//	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: start to calcuate ideal hitrate in window %s ~ %s"),
//		_name.c_str(), time64ToUTCwZoneStr(winStart).c_str(), time64ToUTCwZoneStr(winStop).c_str() );

	uint64 stampStart = ZQ::common::now();

	CONTENT_PLAYS_QUEUE contentPlayQueue;

	Content* content = NULL;
	CONTENTS::iterator it;

	// put _cachedContents in the the queue
	for(it=_cachedContents.begin(); it!=_cachedContents.end(); it++)
	{
		content = (Content*)it->second;
		
		// no statistics in following case: out of the stat window & and no op
		if( content->_lastStartTime < winStart 
			|| content->_firstStartTime > winStop 
			|| content->getOpCount() == 0)
		{
			continue;
		}

		// calcuate the play count in the window
		content->calPlaysInWindow(winStart, winStop);

		if(content->getPlaysInWindow() == 0)
			continue;

		contentPlayQueue.push(content);
	}
	// put _uncachedContents in the the queue
	for(it=_uncachedContents.begin(); it!=_uncachedContents.end(); it++)
	{
		content = (Content*)it->second;

		// no statistics in following case: out of the stat window & and no op
		if( content->_lastStartTime < winStart 
			|| content->_firstStartTime > winStop 
			|| content->getOpCount() == 0)
		{
			continue;
		}
		// calcuate the play count in the window
		content->calPlaysInWindow(winStart, winStop);

		if(content->getPlaysInWindow() == 0)
			continue;

		contentPlayQueue.push(content);
	}

	uint64 cntSize;

	uint32 actualLocalPlaysInIdeal = 0;
	uint32 actualLocalPlaysOutofIdeal = 0;

	uint32 idealMinLocalPlays = 0;
	uint32 idealLocalPlays = 0;
	uint32 idealLocalCntCount = 0;
	
	uint32 totalPlays = 0;
	uint32 totalCntCount = 0;
	
	uint64 sumSize = 0;
	uint64 maxSize = (_totalSpace * (uint64)(100-_cushion)) / 100;
	uint64 usedSpace = _totalSpace - _freeSpace;
	
	maxSize = usedSpace < maxSize ? usedSpace : maxSize;  // in case the storage has not full filled

	double extraSize = 0.0;
	while(!contentPlayQueue.empty())
	{
		content= (Content*)contentPlayQueue.top();
		contentPlayQueue.pop();
				
		cntSize = content->getSize();
		if(0 != _statExtraSizePer)
		{
			extraSize = (double)cntSize * (double)( (float)_statExtraSizePer / 100.0f );
			cntSize += (uint64)extraSize ;
		}

		if(sumSize < maxSize)
		{
			// set default size if it is not availiable
			sumSize += ( cntSize == 0 ? 2000000000 : cntSize);	// 2G by default

			idealLocalPlays += content->getPlaysInWindow();
			actualLocalPlaysInIdeal += content->getLocalPlaysInWindow();
			totalPlays += content->getPlaysInWindow();

			idealLocalCntCount++;
			totalCntCount++;

			if( (0 == idealMinLocalPlays) || (idealMinLocalPlays > content->getPlaysInWindow()) )
			{
				idealMinLocalPlays = content->getPlaysInWindow();
			}
			
			if(_statPrintDetails)
			{
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: No.%06d content pid=%s paid=%s totalPlays=%d actualLocalPlays=%d imports=%d deletes=%d usedSize=%llu was taken as local in ideal hitrate counting "),
					_name.c_str(), idealLocalCntCount, content->_pid.c_str(), content->_paid.c_str(), 
					content->getPlaysInWindow(), content->getLocalPlaysInWindow(),
					content->getImportTimesInWindow(), content->getDeleteTimesInWindow(),
					cntSize);
			}
		}
		else
		{
			actualLocalPlaysOutofIdeal += content->getLocalPlaysInWindow();
			totalPlays += content->getPlaysInWindow();

			totalCntCount++;

			if(_statPrintDetails)
			{
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: No.%06d content pid=%s paid=%s totalPlays=%d localPlays=%d imports=%d deletes=%d usedSize=%llu was taken as remote in ideal hitrate counting "),
					_name.c_str(), totalCntCount, content->_pid.c_str(), content->_paid.c_str(), 
					content->getPlaysInWindow(), content->getLocalPlaysInWindow(),
					content->getImportTimesInWindow(), content->getDeleteTimesInWindow(),
					cntSize);
			}
		}
	}

	if(0 == totalPlays)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: ideal hitrate is NA (%d:%d) in window %s ~ %s"),
			_name.c_str(), 
			idealLocalPlays, totalPlays,
			time64ToUTCwZoneStr(winStart).c_str(), time64ToUTCwZoneStr(winStop).c_str() );

		return ;
	}
	float idealHitrate = (float)100.0f * (float)((float)idealLocalPlays / (float)totalPlays);
	float actualHitrate = (float)100.0f * (float)((float)(actualLocalPlaysInIdeal+actualLocalPlaysOutofIdeal) / (float)totalPlays);

	char strLogBuff[2048];

	// print ideal hitrate
	sprintf(strLogBuff, "%s: ideal hitrate is %.2f%% plays(%d:%d) content(%d:%d) minPlays=%d in window %s ~ %s, sumSize=%llu GB storageSize=%llu GB", 
		_name.c_str(), 
		idealHitrate, 
		idealLocalPlays, totalPlays,
		idealLocalCntCount, totalCntCount,
		idealMinLocalPlays,
		time64ToUTCwZoneStr(winStart).c_str(), time64ToUTCwZoneStr(winStop).c_str(), 
		sumSize/1000000000, maxSize/1000000000);
	
	_algorithm.printLog(strLogBuff);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s"), strLogBuff);

	// print actual hitrate
	sprintf(strLogBuff, "%s: actual hitrate is %.2f%% plays(%d:%d:%d) in window %s ~ %s", 
		_name.c_str(), 
		actualHitrate, 
		actualLocalPlaysInIdeal, actualLocalPlaysOutofIdeal, totalPlays,
		time64ToUTCwZoneStr(winStart).c_str(), time64ToUTCwZoneStr(winStop).c_str() );

	_algorithm.printLog(strLogBuff);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s"), strLogBuff);

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: calcuate ideal hitrate in window %s ~ %s, cost %d ms"),
		_name.c_str(), time64ToUTCwZoneStr(winStart).c_str(), time64ToUTCwZoneStr(winStop).c_str(), ZQ::common::now()-stampStart );
}

void CacheStorage::scheduledErrorHandle(Content& cnt)
{
	if(!cnt.isScheduled())
		return;

	uint64 nextSchedule = cnt._scheduleStartTime = ZQ::common::now() + (uint64)(_cacheRetryInterval*1000);
	if(nextSchedule < cnt._scheduleEndTime)
	{
		cnt._scheduleStartTime = nextSchedule;

		glog(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStorage, "%s: pid=%s paid=%s previous scheduled import failed, schedule it to %s"),
			_name.c_str(), cnt._pid.c_str(), cnt._paid.c_str(), time64ToUTCwZoneStr(nextSchedule).c_str() );

		makeContentScheduled(cnt);
	}
	else
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStorage, "%s: pid=%s paid=%s previous scheduled import failed, reach the end of schedule %s, no more retry"),
			_name.c_str(), cnt._pid.c_str(), cnt._paid.c_str(), time64ToUTCwZoneStr(cnt._scheduleEndTime).c_str() );
	}
}

void CacheStorage::outputStatDataToFile(Content& content, ContentOpData& opData)
{
	char statBuff[1024];
	int nlen;

	time_t t = time(NULL);
	struct tm* localtm = localtime(&t);

	if(0 == _statDay || _statDay != localtm->tm_mday)
	{
		if(_statOutput.is_open())
			_statOutput.close();

		_statDay = localtm->tm_mday;

		char filename[255];
		sprintf(filename, "%s%s_stat_%04d%02d%02d.csv", 
				_statDataPath.c_str(), 
				_name.c_str(), 
				localtm->tm_year+1900, localtm->tm_mon+1, localtm->tm_mday);
		
		// check if file already exist
		bool csvhead = false;
#ifdef ZQ_OS_MSWIN
		struct _stat filestat;
		if(0 != _stat(filename, &filestat) || 0 == filestat.st_size)
		{
			csvhead = true;
		}
#else
		struct stat filestat;
		if(0 != stat(filename, &filestat) || 0 == filestat.st_size)
		{
			csvhead = true;
		}
#endif
		_statOutput.open(filename, std::ofstream::out|std::ofstream::app);
		
		if(csvhead)
		{
			nlen = snprintf(statBuff, 1024, "optype,timestamp,pid,paid,playcount,seconds,minutes,hours,local\n");
			 _statOutput.write(statBuff, nlen);
		}

		// delete expired file (7 days ago)
		t -= (1+7)*86400;
		localtm = localtime(&t);
		sprintf(filename, "%s%s_stat_%04d%02d%02d.csv", 
				_statDataPath.c_str(), 
				_name.c_str(), 
				localtm->tm_year+1900, localtm->tm_mon+1, localtm->tm_mday);
		unlink(filename);	// delete the file
	}

	nlen = snprintf(statBuff, 1000, "%s,%s,%s,%s,%.2f,%llu,%llu,%llu,%s\n", 
			 opData.typeToStr().c_str(), time64ToUTCwZoneStr(opData._timeStamp).c_str(), 
			 content._pid.c_str(), content._paid.c_str(), 
			 opData._playCount,
			 opData._interval, opData._interval/60, opData._interval/3600,
			 (ContentOpData::CNT_OP_PLAY == opData._type) ? (opData._isLocalHit ? "T":"F") : "N");
	_statOutput.write(statBuff, nlen);	
}

void CacheStorage::checkUsage()
{
	bool logging = false;
	if(rand() % 10 == 0)
	{
		logging = true;
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: checking space usage ..."),
			_name.c_str());
	}

	CONTENTS_ARRAY toBeDeleted;
	uint64 toBeFreedSpace=0;
	
	bool ret = false;
	if(0 == _idleContents.size())
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStorage, "%s: no idle contents, something is wrong. Have to candidate deleting contennt from cached contents"),
				_name.c_str());
		
		ret = _algorithm.deleteEvaluate(*this, _cachedContents, toBeDeleted, toBeFreedSpace);
	}
	else
	{
		ret = _algorithm.deleteEvaluate(*this, _idleContents, toBeDeleted, toBeFreedSpace);
	}
	if(!ret)
	{
		// no need to delete since has not reached the cushion
		return;
	}

	uint64 preFreeSpace = _freeSpace;
	_freeSpace = (_freeSpace > toBeFreedSpace) ?  (_freeSpace - toBeFreedSpace) : 0;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: update free space from %llu bytes to %llu bytes after deletion candiating"),
			_name.c_str(), preFreeSpace, _freeSpace);

	int i=0;
	for(i=0; i<toBeDeleted.size(); i++)
	{
		// delete the content
		doIODelete(*toBeDeleted[i]);
	}

	if(logging)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: checking space usage done"),
			_name.c_str());
	}
}

void CacheStorage::printCachingList()
{
	int i=0;
	CONTENTS::iterator it;
	Content* content = NULL;

	for(it=_cachingContents.begin(); it!=_cachingContents.end(); it++)
	{
		// to exit the processing if serivce is stopping to avoid possible crash at exit since it is time cost routine depends on the list size
		if(!_bThreadRunning)
			return ;

		i++;
		content = (Content*)it->second;
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: WORINGLIST CONTENT No.%03d: pid=%s paid=%s status=%s lastUpdateTime=%s"),
			_name.c_str(), i, content->_pid.c_str(), content->_paid.c_str(), content->statusToString().c_str(), time64ToUTCwZoneStr(content->_lastStatusTime).c_str() );
	}
}

void CacheStorage::checkCachingList()
{
	if(_cachingContents.size() == 0)
		return ;

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: enter checkCachingList(), there are %d content in the CACHING LIST"),
		_name.c_str(), _cachingContents.size());	

	printCachingList();

	CONTENTS_ARRAY toBeDone;
	CONTENTS_ARRAY toBeRemoved;

	Content* content = NULL;
	CONTENTS::iterator it;
	for(it=_cachingContents.begin(); it!=_cachingContents.end(); it++)
	{
		// to exit the processing if serivce is stopping to avoid possible crash at exit since it is time cost routine depends on the list size
		if(!_bThreadRunning)
			return ;

		content = (Content*)it->second;

		// find out if it is timeout
		if(!content->isCaching())
		{
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s status=%s lastStatusTime=%s, suppose not in the CACHING LIST"),
				_name.c_str(), content->_pid.c_str(), content->_paid.c_str(), content->statusToString().c_str(), time64ToUTCwZoneStr(content->_lastStatusTime).c_str());

			// the content is NOT caching/deleting, but in the working list, need to remove it
			toBeDone.push_back(content);

			continue;
		}

		if(!content->statusTimeout() )
		{
			continue;
		}
		
		// now see timeout
		Content::CONTENT_STATUS curStatus = content->getStatus();

		switch(curStatus)
		{
		case Content::CNT_CACHE_PENDING:
		case Content::CNT_CACHING:
		case Content::CNT_DELETING:
			// reset it to uncached to uncahced & move the uncached list, wait for next play to trigger recache
			content->setStatus(Content::CNT_UNCACHED);
			// moved from cached to uncached, means moved out of idle/caching list too
			toBeRemoved.push_back(content);

			// check if need to retry later if it is schedled importing
			// content will be moved to uncached list later
			if(Content::CNT_DELETING != curStatus)
			{
				scheduledErrorHandle(*content);
			}
			break;
		default:
			break;
		}
	}
	
	// remove the content from the caching list if it is not suppose in the working list for some reason
	for(int i=0; i<toBeDone.size(); i++)
	{
		makeCacheWorkDone(*toBeDone[i]);
	}

	for(int i=0; i<toBeRemoved.size(); i++)
	{
		cached2UnCached(*toBeRemoved[i]);
		// query the status again
		PROPERTIES properties;
		doIORead(*toBeRemoved[i], properties);
	}

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: exit checkCachingList(), there are %d content in the CACHING LIST"),
		_name.c_str(), _cachingContents.size());
}

void CacheStorage::checkScheduledImport()
{
	if(_scheduledContents.size() == 0)
		return;

	uint64 tnow = ZQ::common::now();

	CONTENTS_ARRAY todoList;

	Content* content = NULL;
	CONTENTS::iterator it;
	for(it=_scheduledContents.begin(); it!=_scheduledContents.end(); it++)
	{
		content = (Content*)it->second;
		
		if( (content->_scheduleStartTime < tnow) && (content->_scheduleEndTime > tnow) )
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s scheduleStartTime=%s scheduleEndTime=%s, time reached, going to import"),
				_name.c_str(), content->_pid.c_str(), content->_paid.c_str(), 
				time64ToUTCwZoneStr(content->_scheduleStartTime).c_str(), time64ToUTCwZoneStr(content->_scheduleEndTime).c_str() );

			//going to import this content
			todoList.push_back(content);
		}
	}

	// remove from retry list
	for(int i=0; i<todoList.size(); i++)
	{
		// move from uncachedContents to cachedContents
		uncached2Cached(*content);
		// move out of the scheduled list since it is on the way to import
		makeContentNoneScheduled(*todoList[i]);

		// import it
		doIOImport(*todoList[i]);

		//
		todoList[i]->_lastCacheOpAdjPlay = todoList[i]->_adjplay;
	}
}

void CacheStorage::checkCachedContents()
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: enter checkCachedContents() total %d cached contents in the list"),
		_name.c_str(), _cachedContents.size() );

	Content* content = NULL;
	CONTENTS::iterator it;
	int count=0;
	for(it=_cachedContents.begin(); it!=_cachedContents.end(); it++)
	{
		// to exit the processing if serivce is stopping to avoid possible crash at exit since it is time cost routine depends on the list size
		if(!_bThreadRunning)
			return ;

		content = (Content*) it->second;

		// periodly check content status 
		if( content->lastUpdateToNowSeconds() >= _cntCheckInterval )
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s check content status since lastUpdateTime=%s, periodly check it after %d hours"),
				_name.c_str(), content->_pid.c_str(), content->_paid.c_str(), time64ToUTCwZoneStr(content->getLastStatusTime()).c_str(), _cntCheckInterval/3600);

			PROPERTIES properties;
			doIORead(*content, properties);
		}

		// check nplay & reset if there is playend missed
		bool bIdle = content->checkPlayCount();
		if(bIdle)
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s force to be idle"),
				_name.c_str(), content->_pid.c_str(), content->_paid.c_str());

			// its playcount was reset
			makeContentIdle(*content);
		}
	}

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: exit checkCachedContents() total %d cached contents in the list"),
		_name.c_str(), _cachedContents.size() );
}

void CacheStorage::checkUncachedContent()
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: enter checkUncachedContent(), total %d uncached content"),
		_name.c_str(), _uncachedContents.size() );

	CONTENTS_ARRAY toBeDeleted;

	Content* content = NULL;
	CONTENTS::iterator it;
	int count=0;
	for(it=_uncachedContents.begin(); it!=_uncachedContents.end(); it++)
	{
		// to exit the processing if serivce is stopping to avoid possible crash at exit since it is time cost routine depends on the list size
		if(!_bThreadRunning)
			return ;

		content = (Content*) it->second;

		if(content->lastPlayToNowSeconds() > _trimInSeconds)
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s has not been viewed since %s, remove it"),
				_name.c_str(), content->_pid.c_str(), content->_paid.c_str(), time64ToUTCwZoneStr(content->_lastStartTime).c_str());
			
			toBeDeleted.push_back(content);
		}
	}

	// remove the content from memory
	int i=0;
	for(; i<toBeDeleted.size(); i++)
	{
		content = (Content*) toBeDeleted[i];
		_uncachedContents.erase(content->_name);

		delete content;
	}

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: exit checkUncachedContent(), total %d uncached content left"),
		_name.c_str(), _uncachedContents.size() );
}

void CacheStorage::playStart(std::string& pid, std::string& paid, uint64 timestamp)
{
	_cntEventLock.WriteLock();

	PlayStartEvent* evt = new PlayStartEvent(*this, pid, paid, timestamp);
	_cntEvents.push(evt);

	_cntEventLock.WriteUnlock();

	_waitEvent.signal();
}

void CacheStorage::playEnd(std::string& pid, std::string& paid, uint32 duration)
{
	_cntEventLock.WriteLock();

	PlayStopEvent* evt = new PlayStopEvent(*this, pid, paid, duration);
	_cntEvents.push(evt);

	_cntEventLock.WriteUnlock();

	_waitEvent.signal();
}

void CacheStorage::proactiveImport(std::string& pid, std::string& paid, uint64 timeStart, uint64 timeExpired)
{
	_cntEventLock.WriteLock();

	ProactiveImportEvent* evt = new ProactiveImportEvent(*this, pid, paid, timeStart, timeExpired);
	_cntEvents.push(evt);

	_cntEventLock.WriteUnlock();

	_waitEvent.signal();
}


void CacheStorage::doPlayStart(std::string& pid, std::string& paid, uint64 timestamp)
{
	bool needToCache = false;

	_totalHits++;		// count the total plays in this cachestorage

	// 1. check if content already cached, if yes, only update its play data
	//
	Content* cachedContent = findContentInCachedList(pid, paid);
	if(NULL != cachedContent)
	{	
		bool bLocal=false;
		// count local hit
		if(cachedContent->isLocal())
		{
			bLocal = true;
			_localHits++;
		}

		// aging this content before set play start
		_algorithm.agingContent(*this, *cachedContent);

		float preAdjPlay = cachedContent->getAdjPlay();
		float newAdjPlay = preAdjPlay + 1.0f;
		cachedContent->setAdjPlay(newAdjPlay);

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: pid=%s paid=%s update adjPlay from %.2f to %.2f. (C:%s)"),
			_name.c_str(), cachedContent->_pid.c_str(), cachedContent->_paid.c_str(), preAdjPlay, newAdjPlay, bLocal ? "Local" : "CDN");

		// the content is caching or cached
		cachedContent->playStart(timestamp);

		// make it busy
		makeContentBusy(*cachedContent);

		return;
	}

	// 2. content has not cached yet, manage this content and evaluate it if need to cache
	//
	Content* uncachedContent = findContentInUncachedList(pid, paid);
	if(NULL == uncachedContent)
	{
		// create a new content and save it to uncached list
		uncachedContent = createContentAndSave(pid, paid, Content::CNT_UNCACHED);
	}

	// aging this content before set play start
	_algorithm.agingContent(*this, *uncachedContent);
	float preAdjPlay = uncachedContent->getAdjPlay();
	float newAdjPlay = preAdjPlay + 1.0f;
	uncachedContent->setAdjPlay(newAdjPlay);

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: pid=%s paid=%s update adjPlay from %.2f to %.2f. (U:%s)"),
		_name.c_str(), uncachedContent->_pid.c_str(), uncachedContent->_paid.c_str(), preAdjPlay, newAdjPlay, "CDN");

	// eavluate if the content need to be cached
	needToCache = _algorithm.importEvaluate(*this, _cacheIO, *uncachedContent);
	if(needToCache)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "%s: pid=%s paid=%s going to import it"),
			_name.c_str(), uncachedContent->_pid.c_str(), uncachedContent->_paid.c_str() );

		// move from uncachedContents to cachedContents
		uncached2Cached(*uncachedContent);

		// import it
		doIOImport(*uncachedContent);

		// save the adjPlay when start the cache operation because the import may take long time and meanwhile there is request coming in
		uncachedContent->_lastCacheOpAdjPlay = newAdjPlay;
	}

	// count the play on the content no matter it is going to cache or not
	uncachedContent->playStart(timestamp);
}

void CacheStorage::doPlayEnd(std::string& pid, std::string& paid, uint32 duration)
{
	// 1. check if the content has been cached, if yes, count the play & set it idle if necessary
	//
	Content* cachedContent = findContentInCachedList(pid, paid);
	if(NULL != cachedContent)
	{	
		bool bIdle = cachedContent->playEnd(duration);

		if(bIdle)
		{
			makeContentIdle(*cachedContent);
		}
		return;
	}

	// 2. content has not cached yet, only update its paly count
	//
	Content* uncachedContent = findContentInUncachedList(pid, paid);
	if(NULL != uncachedContent)
	{
		uncachedContent->playEnd(duration);
	}
}

void CacheStorage::doStatusUpdate(std::string& pid, std::string& paid, Content::CONTENT_STATUS status, uint32 bitrate, uint32 duration, uint64 size)
{
	Content* cachedContent = findContentInCachedList(pid, paid);
	Content* uncachedContent = findContentInUncachedList(pid, paid);

	Content::CONTENT_STATUS preStatus = Content::CNT_UNCACHED;
	Content* content = NULL;

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: pid=%s paid=%s doStatusUpdate() status=%s, bitrate=%d bps, duration=%d seconds, size=%llu bytes"),
		_name.c_str(), pid.c_str(), paid.c_str(), CONTENT_STATUS_2_TEXT(status).c_str(), bitrate, duration, size);

	switch(status)
	{
	case Content::CNT_UNCACHED:
	case Content::CNT_DELETED:
	case Content::CNT_CACHE_FAIL:
	case Content::CNT_NOT_EXIST:
		if(NULL != cachedContent)
		{
			// move it uncached list first
			cached2UnCached(*cachedContent);

			preStatus = cachedContent->getStatus();

			// sometime content was deleted not because of me, no aging its adjPlay
			if( (Content::CNT_DELETING == preStatus) && (Content::CNT_DELETED == status) )
			{	// aging content after its deletion
				_algorithm.cacheDeleted(*this, *cachedContent, true);
				// save the delete op
				cachedContent->saveOneDelOp();
			}

			// possibly import failed because content not exist in CDN, and the host can not distingush the content not exist
			// from other cache fail in ASYNC IO call like CMEV2 with VSIS
			if(  ( Content::CNT_CACHE_FAIL == status)
				 || 
				 ( (Content::CNT_CACHING == preStatus || Content::CNT_CACHE_PENDING == preStatus) && (Content::CNT_NOT_EXIST == status) )
			   )
			{
				_algorithm.importCompleted(*this, *cachedContent, false);
				
				// content has been moved to uncached list above
				scheduledErrorHandle(*cachedContent);
			}

			// in this case the bitrate/duration/size may be 0, so don't update it since we want to keep the previous metadata values
			cachedContent->setStatus(Content::CNT_UNCACHED);
		}
		else
		{
			if(NULL == uncachedContent)
			{
				uncachedContent = createContentAndSave(pid, paid, Content::CNT_UNCACHED, bitrate, duration, size);
			}
			else
			{
				uncachedContent->setStatus(Content::CNT_UNCACHED);
			}
		}
		break;
	case Content::CNT_CACHING:
	case Content::CNT_CACHED:
		if(NULL != cachedContent)
		{
			content = cachedContent;

			// get previous status
			preStatus = content->getStatus();

			content->setStatus(status);
		}
		else 
		{
			if(NULL != uncachedContent)
			{
				content = uncachedContent;
				// move content to cached list
				content->setStatus(status);
				uncached2Cached(*content);
			}
			else
			{
				// create content to cached list
				content = createContentAndSave(pid, paid, status, bitrate, duration, size);
				content->setStatus(status);
			}
		}
		// set the metadata
		content->setMetadata(bitrate, duration, size);

		if(Content::CNT_CACHING == status)
		{
			makeCacheWorkStart(*content);
		}
		else if(Content::CNT_CACHED == status)
		{	
			// only in case of previous status is CNT_CACHING to avoid the already cached content since they were also updated by this routine
			if(Content::CNT_CACHING == preStatus)
			{
				_algorithm.importCompleted(*this, *content, true);
				// save the delete op
				content->saveOneImpOp();
			}

			// cache work is done, move out of the caching list
			makeCacheWorkDone(*content);
		}

		break;
	case Content::CNT_DELETE_FAILED:
		if(NULL != cachedContent)
		{	// reset to cached, wait for next round deletion
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(CacheStorage, "%s: pid=%s paid=%s deletion failed, reset status to %s"),
				_name.c_str(), pid.c_str(), paid.c_str(), CONTENT_STATUS_2_TEXT(Content::CNT_CACHED).c_str() );

			// 
			_algorithm.cacheDeleted(*this, *cachedContent, false);

			cachedContent->setStatus(Content::CNT_CACHED);
			// deleting is done
			makeCacheWorkDone(*cachedContent);
		}
		break;
	case Content::CNT_CACHE_PENDING: // internal status only
	case Content::CNT_DELETING:      // internal status only
	default:
		break;
	}
}

void CacheStorage::doProactiveImport(std::string& pid, std::string& paid, uint64 startTime, uint64 expirationTime)
{
	Content* content = findContentInCachedList(pid, paid);
	if(NULL == content)
	{
		content = findContentInUncachedList(pid, paid);
	}
	if(NULL != content)
	{
		// update visa time if the content is already in the list
		content->_scheduleStartTime = startTime;
		content->_scheduleEndTime = expirationTime;

		// put it into the queue (no effect if it is already in it)
		makeContentScheduled(*content);

		return;
	}

	// it's a new content
	content = createContentAndSave(pid, paid, Content::CNT_UNCACHED);

	content->_scheduleStartTime = startTime;
	content->_scheduleEndTime = expirationTime;

	makeContentScheduled(*content);
}

bool CacheStorage::doIODelete(Content& content)
{
	// set status
	content.setStatus(Content::CNT_DELETING);
	// add it into the Caching list
	makeCacheWorkStart(content);

	IO_OP_RESULT ioRet = _cacheIO.deleteContent(content._pid, content._paid);
	if(IO_PENDING != ioRet)
	{
		// TODO: will process other IO result later
	}
	return IO_FAIL != ioRet;
}

bool CacheStorage::doIOImport(Content& content)
{
	// set status
	content.setStatus(Content::CNT_CACHE_PENDING);
	// add it into the Caching list
	makeCacheWorkStart(content);

	// import the content
	IO_OP_RESULT ioRet = _cacheIO.importContent(content._pid, content._paid);
	if(IO_PENDING != ioRet)
	{
		// TODO: will process other IO result later
	}

	return IO_FAIL != ioRet;
}

bool CacheStorage::doIORead(Content& content, PROPERTIES& properties)
{
	IO_OP_RESULT ioRet = _cacheIO.readContent(content._pid, content._paid, properties);
	if(IO_PENDING != ioRet)
	{
		// TODO: will process other IO result later
	}

	return IO_FAIL != ioRet;
}

bool CacheStorage::doIOListContent()
{
	IO_OP_RESULT ioRet = _cacheIO.listContents(_uncachedContents);
	if(IO_PENDING != ioRet)
	{
		// TODO: will process other IO result later
	}

	return IO_FAIL != ioRet;
}

void CacheStorage::onMetadata(std::string& pid, std::string& paid, PROPERTIES& properites)
{
	// did not implement this at this moment
}

void CacheStorage::onStatusUpdate(std::string& pid, std::string& paid, Content::CONTENT_STATUS status, uint32 bitrate, uint32 duration, uint64 size)
{
	_cntEventLock.WriteLock();

	StatusUpdateEvent* evt = new StatusUpdateEvent(*this, pid, paid, status, bitrate, duration, size);
	_cntEvents.push(evt);

	_cntEventLock.WriteUnlock();

	_waitEvent.signal();
}

void CacheStorage::onUsageUpdate(uint64 freeSpace, uint64 totalSpace)
{
	setUsage(freeSpace, totalSpace);
}


Content* CacheStorage::findContentInUncachedList(std::string& pid, std::string& paid)
{
	CONTENTS::iterator itcnt = _uncachedContents.find(PAID_PID(paid,pid));
	
	if(itcnt != _uncachedContents.end())
	{
		return (Content*) itcnt->second;
	}
	return NULL;
}

Content* CacheStorage::findContentInCachedList(std::string& pid, std::string& paid)
{
	CONTENTS::iterator itcnt = _cachedContents.find(PAID_PID(paid,pid));
	
	if(itcnt != _cachedContents.end())
	{
		return (Content*) itcnt->second;
	}
	return NULL;
}

void CacheStorage::cached2UnCached(Content& cnt)
{
	_cachedContents.erase(cnt._name);
	_idleContents.erase(cnt._name);
	_cachingContents.erase(cnt._name);

	CONTENTS::iterator itcnt = _uncachedContents.find(cnt._name);
	if(itcnt == _uncachedContents.end())
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s status=%s is moved to uncachedContents"),
			_name.c_str(), cnt._pid.c_str(), cnt._paid.c_str(), cnt.statusToString().c_str() );

		_uncachedContents.insert(CONTENTS::value_type(cnt._name, &cnt));
	}
}

void CacheStorage::uncached2Cached(Content& cnt)
{
	_uncachedContents.erase(cnt._name);
	_scheduledContents.erase(cnt._name);

	CONTENTS::iterator itcnt = _cachedContents.find(cnt._name);
	if(itcnt == _cachedContents.end())
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s status=%s is moved into cachedContents"),
			_name.c_str(), cnt._pid.c_str(), cnt._paid.c_str(), cnt.statusToString().c_str() );

		_cachedContents.insert(CONTENTS::value_type(cnt._name, &cnt));
	}
}

Content* CacheStorage::createContentAndSave(std::string& pid, std::string& paid, Content::CONTENT_STATUS status, uint32 bitrate, uint32 duration, uint64 size)
{
	Content* content = NULL;
	std::string paidpid = PAID_PID(paid,pid);

	if(Content::CNT_UNCACHED == status)
	{
		CONTENTS::iterator itcnt = _uncachedContents.find(paidpid);
		if(itcnt == _uncachedContents.end())
		{
			content = new Content(*this, pid, paid);
			content->setStatus(status);
			content->setMetadata(bitrate, duration, size);

			_uncachedContents.insert(CONTENTS::value_type(paidpid, content));
		}
		else
		{
			content = (Content*)itcnt->second;
		}
	}
	else
	{
		CONTENTS::iterator itcnt = _cachedContents.find(paidpid);
		if(itcnt == _cachedContents.end())
		{
			content = new Content(*this, pid, paid);
			content->setStatus(status);
			content->setMetadata(bitrate, duration, size);

			_cachedContents.insert(CONTENTS::value_type(paidpid, content));
		}
		else
		{
			content = (Content*)itcnt->second;
		}
	}
	return content;
}

void CacheStorage::makeContentBusy(Content& cnt)
{
	CONTENTS::iterator itcnt = _idleContents.find(cnt._name);
	if(itcnt != _idleContents.end())
	{	
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s is moved out of IDLE LIST"),
			_name.c_str(), cnt._pid.c_str(), cnt._paid.c_str() );

		_idleContents.erase(itcnt);
	}
}

void CacheStorage::makeContentIdle(Content& cnt)
{
	CONTENTS::iterator itcnt = _idleContents.find(cnt._name);
	if(itcnt != _idleContents.end())
	{	// already in idle
		return;
	}

	itcnt = _cachingContents.find(cnt._name);
	if(itcnt != _cachingContents.end())
	{	// the content is caching or deleting, not to add into idle
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s status=%s is in the caching list, does not move to IDLE LIST"),
			_name.c_str(), cnt._pid.c_str(), cnt._paid.c_str(), cnt.statusToString().c_str() );

		return;		
	}

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s is moved into IDLE LIST"),
		_name.c_str(), cnt._pid.c_str(), cnt._paid.c_str() );

	_idleContents.insert(CONTENTS::value_type(cnt._name, &cnt));
}

void CacheStorage::makeCacheWorkDone(Content& cnt)
{
	CONTENTS::iterator itcnt = _cachingContents.find(cnt._name);
	if(itcnt != _cachingContents.end())
	{	
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s status=%s is moved out of CACHING LIST"),
			_name.c_str(), cnt._pid.c_str(), cnt._paid.c_str(), cnt.statusToString().c_str() );

		// erase from the caching list
		_cachingContents.erase(cnt._name);
	}

	// if content is not being viewed, set it to be idle
	bool bBusy = cnt.isViewing();
	if(!bBusy)
	{
		makeContentIdle(cnt);
	}
}

void CacheStorage::makeCacheWorkStart(Content& cnt)
{
	makeContentBusy(cnt);

	CONTENTS::iterator itcnt = _cachingContents.find(cnt._name);
	if(itcnt == _cachingContents.end())
	{	// 
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s status=%s is moved into CACHING LIST"),
			_name.c_str(), cnt._pid.c_str(), cnt._paid.c_str(), cnt.statusToString().c_str() );

		_cachingContents.insert(CONTENTS::value_type(cnt._name, &cnt));
	}
}

void CacheStorage::makeContentScheduled(Content& cnt)
{
	CONTENTS::iterator itcnt = _scheduledContents.find(cnt._name);
	if(itcnt == _scheduledContents.end())
	{	// 
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s status=%s is moved into SCHEDULED LIST"),
			_name.c_str(), cnt._pid.c_str(), cnt._paid.c_str(), cnt.statusToString().c_str() );

		_scheduledContents.insert(CONTENTS::value_type(cnt._name, &cnt));		
	}
}

void CacheStorage::makeContentNoneScheduled(Content& cnt)
{
	CONTENTS::iterator itcnt = _scheduledContents.find(cnt._name);
	if(itcnt != _scheduledContents.end())
	{	
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStorage, "%s: content pid=%s paid=%s is moved out of SCHEDULED LIST"),
			_name.c_str(), cnt._pid.c_str(), cnt._paid.c_str() );

		_scheduledContents.erase(itcnt);
	}
}


//////////////////////////////////////////////////////////////////

ContentEvent::ContentEvent(CacheStorage& cs, std::string& pid, std::string& paid)
:_cachestorage(cs)
{
	_pid = pid;
	_paid = paid;
}

ContentEvent::~ContentEvent()
{
}


PlayStartEvent::PlayStartEvent(CacheStorage& cs, std::string& pid, std::string& paid, uint64 timestamp)
:ContentEvent(cs, pid, paid)
{
	_startTime = timestamp;
}

PlayStartEvent::~PlayStartEvent()
{
}

void PlayStartEvent::process()
{
	_cachestorage.doPlayStart(_pid, _paid, _startTime);
}


PlayStopEvent::PlayStopEvent(CacheStorage& cs, std::string& pid, std::string& paid, uint32 duration)
:ContentEvent(cs, pid, paid)
{
	_duration = duration;
}

PlayStopEvent::~PlayStopEvent()
{
}

void PlayStopEvent::process()
{
	_cachestorage.doPlayEnd(_pid, _paid, _duration);
}

StatusUpdateEvent::StatusUpdateEvent(CacheStorage& cs, std::string& pid, std::string& paid, Content::CONTENT_STATUS status, uint32 bitrate, uint32 duration, uint64 size)
:ContentEvent(cs, pid, paid)
{
	_status = status;
	_bitrate = bitrate;
	_duration = duration;
	_size = size;	
}

StatusUpdateEvent::~StatusUpdateEvent()
{
}

void StatusUpdateEvent::process()
{
	_cachestorage.doStatusUpdate(_pid, _paid, _status, _bitrate, _duration, _size);
}

ProactiveImportEvent::ProactiveImportEvent(CacheStorage& cs, std::string& pid, std::string& paid, uint64 startTime, uint64 expirationTime)
:ContentEvent(cs, pid, paid)
{
	_startTime = startTime;
	_expirationTime = expirationTime;
}

ProactiveImportEvent::~ProactiveImportEvent()
{
}

void ProactiveImportEvent::process()
{
	_cachestorage.doProactiveImport(_pid, _paid, _startTime, _expirationTime);
}

}