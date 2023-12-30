#ifndef _CACHESTORAGE_H_
#define _CACHESTORAGE_H_

#include <string>
#include <map>
#include <queue>
#include <list>
#include <fstream>

#include "NativeThread.h"
#include "Locks.h"
#include "SystemUtils.h"
#include "TimeUtil.h"

#define PAID_PID(paid,pid) (paid + "_" + pid)

namespace CacheManagement {

class CacheStorage;
class CacheIO;
class ContentEvent;
class Content;

typedef std::map<std::string, CacheStorage*> CACHE_STORAGES;
typedef std::vector<CacheStorage*> CACHE_STORAGE_ARRAY;

typedef std::map<std::string, Content*>	CONTENTS;
typedef std::vector<Content*> CONTENTS_ARRAY;

typedef std::queue<ContentEvent*> CONTENT_EVENTS;

typedef std::map<std::string, std::string> PROPERTIES;

const std::string cs_metadata_bitrate  = "bitrate";
const std::string cs_metadata_size	   = "size";
const std::string cs_metadata_duration = "duration";

typedef	enum _IO_OP_RESULT {
		IO_FAIL = 0, 
		IO_DONE, 
		IO_PENDING
	} IO_OP_RESULT;

extern std::string time64ToUTCwZoneStr(uint64 t);

class CacheIO 
{
	friend class CacheStorage;
public:
	CacheIO();
	virtual ~CacheIO();

protected:
	CacheStorage* _cstorage;
	// this need to be invoked if the IO is aysncall call
	void setOwner(CacheStorage& cs);

public:
	virtual bool isReady() = 0;
	virtual bool isActivity() = 0;
	virtual IO_OP_RESULT importContent(std::string& pid, std::string& paid) = 0;
	virtual IO_OP_RESULT deleteContent(std::string& pid, std::string& paid) = 0;
	virtual IO_OP_RESULT readContent(std::string& pid, std::string& paid, PROPERTIES& properites) = 0;
	virtual IO_OP_RESULT listContents(CONTENTS& contents) = 0;
};

class CacheAlgorithm
{
public:
	CacheAlgorithm() {};
	virtual ~CacheAlgorithm() {};

public:
	virtual bool importEvaluate(CacheStorage& cs, CacheIO& cacheIO, Content& content) = 0;
	virtual bool deleteEvaluate(CacheStorage& cs, CONTENTS& contents, CONTENTS_ARRAY& candidated, uint64& toBeFreedSize) = 0;
	virtual void agingContent(CacheStorage& cs, Content& content) = 0;

	virtual void importCompleted(CacheStorage& cs, Content& content, bool success) {};
	virtual void cacheDeleted(CacheStorage& cs, Content& content, bool success) {};

	virtual void printLog(char* buff) {};

};

class ContentOpData
{
public:
	typedef enum _CONTENT_OP_TYPE { 
		CNT_OP_PLAY = 0,
		CNT_OP_IMP, 
		CNT_OP_DEL
	} CONTENT_OP_TYPE;

public:
	ContentOpData(CONTENT_OP_TYPE type, uint64 timeStamp, uint64 interval, float playCount, bool isLocalHit=true);
	virtual ~ContentOpData();

	std::string typeToStr();
public:
	CONTENT_OP_TYPE _type;
	uint64          _timeStamp;
	uint64          _interval;
	float           _playCount;
	bool            _isLocalHit;
};

typedef std::list<ContentOpData*> CONTENTOPDATAS;

class Content 
{
public:
	typedef enum _CONTENT_STATUS { 
		CNT_UNCACHED = 0, 
		CNT_CACHE_PENDING,		// internal status
		CNT_CACHING, 
		CNT_CACHED,
		CNT_CACHE_FAIL,
		CNT_DELETING,			// internal status
		CNT_DELETED, 
		CNT_DELETE_FAILED,
		CNT_NOT_EXIST			// the content does not exist even in CDN
	} CONTENT_STATUS;

	Content(CacheStorage& cstorage, std::string& pid, std::string& paid);
	virtual ~Content();

protected:
	CacheStorage&  _cstorage;
public:
	// key 
	std::string     _name;
	std::string		_pid;
	std::string		_paid;

	// metadata
	uint32			_bitrate;
	uint64			_size;
	uint32			_duration;		// in seconds
	PROPERTIES		_properties;	// other properties

	uint32			_nplay;	        // not used
	float		    _adjplay;       // Adjusted play count (accounting for DEDUCT)

	uint32		    _starts;		// Number of starts
	uint32		    _finishes;		// Number of completions

	// cache / delete control data		
	uint64		    _createTime;
	uint64		    _firstStartTime;		// Initial start time (GMT, from session start message)
	uint64		    _lastStartTime;			// Most recent start time (GMT, from session start message)
	uint64		    _lastSecondStartTime;	// the last second start time
	uint64          _referencePlayTime;     // 

	CONTENT_STATUS	_status;				// ASSET_STATUS
	uint64			_lastStatusTime;		// last "status" update time 

	// statistics data (will be used in future)
	uint32			_maxPlayDuration;
	uint32			_minPlayDuration;
	uint32			_avgPlayDuration;
	uint64			_totalPlayDuration;
	uint32			_peakplay;		        // Maximum number of simultaneous sessions

	uint64          _scheduleStartTime;     // the schedule start time to import
	uint64			_scheduleEndTime;	    // expiration time, content is not allowed to delete before the time reached
public:
	std::string statusToString();

	void setMetadata(uint32 bitrate, uint32 duration, uint64 size);

	void setStatus(CONTENT_STATUS status);
	Content::CONTENT_STATUS getStatus() { return _status;                     };

	void  setAdjPlay(float adj)			{ _adjplay = adj;                     };
	float getAdjPlay()					{ return _adjplay>=0.0f?_adjplay : 0; };

	bool isViewing()					{ return (_finishes > _starts);       };
	bool isScheduleEnded()	            { return (ZQ::common::now() > _scheduleEndTime); };
	bool isScheduled()                  { return (0 != _scheduleStartTime);   };

	uint64 getSize()					{ return _size;                       };

	uint32 lastUpdateToNowSeconds();
	uint32 lastPlayToNowSeconds();
	uint64 getLastStatusTime()          { return _lastStatusTime;             }
	uint64 getLastPlayStartTime()		{ return _lastStartTime;              };
	uint64 getLastSecondPlayTime()		{ return _lastSecondStartTime;        };
	uint64 getReferencePlayTime()       { return _referencePlayTime;          };
	void calReferencePlayTime();

	bool   isCaching();
	bool   isLocal(); 

	bool checkPlayCount();

	bool playStart(uint64 time);
	bool playEnd(uint32 duration);		

	bool statusTimeout();

private:
	uint64	      _lastCacheOpTime;	   // import or delete

	uint32        _imortTimesInWindow;
	uint32        _deleteTimesInWindow;
	uint32        _playsInWindow;
	uint32        _localPlaysInWindow;

	CONTENTOPDATAS  _operations;
public:
	float         _lastCacheOpAdjPlay; //
	void saveOneImpOp();
	void saveOneDelOp();
	void calPlaysInWindow(uint64 winStart, uint64 winStop);

	uint32 getPlaysInWindow()		{ return _playsInWindow;             };
	uint32 getLocalPlaysInWindow()	{ return _localPlaysInWindow;        };
	uint32 getImportTimesInWindow() { return _imortTimesInWindow;        };
	uint32 getDeleteTimesInWindow() { return _deleteTimesInWindow;       };

	uint32 getOpCount()       { return (uint32)_operations.size(); };

};

class CacheStorage : public ZQ::common::NativeThread
{
	friend class ContentEvent;
	friend class PlayStartEvent;
	friend class PlayStopEvent;
	friend class StatusUpdateEvent;
	friend class ProactiveImportEvent;
public:
	struct cnt_playcount_greater : std::binary_function<Content*, Content*, bool>
	{
		bool operator()(Content* _X, Content* _Y) const
		{
			return _X->getPlaysInWindow() <= _Y->getPlaysInWindow();
		}
	};

public:
	CacheStorage(std::string& name, CacheIO& io, CacheAlgorithm& algorithm);
	virtual ~CacheStorage();

protected:
	CacheIO&			_cacheIO;
	CacheAlgorithm&		_algorithm;

protected:
	std::string         _name;

	uint32				_subFileCount;	

	uint32				_defVisa;		     // default visa for content imported in this cluster in seconds, not been used
	uint64			    _trimInSeconds;		 // how long time the content will be keeped in uncached list before deleting

	uint32              _cushion;
	uint64				_freeSpace;
	uint64				_totalSpace;

	bool				_syncAtStart;
	std::string         _dataType;
	std::string			_dataPath;			// only path
	int32				_dataFlushInterval; // seconds

	bool				_supportDataFile;	//
	std::string			_dataFileName;		// full name, set in initialize()

	uint64				_localHits;
	uint64				_totalHits;

	uint32              _cntCheckInterval;	// DEFAULT_CONTENT_CHECK_INTERVAL
	
	bool                _statEnabled;		// configuration
	std::string         _statDataPath;
	uint32              _statWindow;
	uint32              _statInterval;
	uint32              _statDelay;
	uint32              _statExtraSizePer;
	bool                _statPrintDetails;

	std::ofstream		_statOutput;
	uint32              _statDay;

	uint32              _cacheRetryInterval;
public:
	// content in following map need to be released
	CONTENTS	_uncachedContents;			// all the contents that has been  played
	CONTENTS    _scheduledContents;			// the contents that needs to import based on schedule, like pro-active cache
											// Sub map of _uncachedContents

	CONTENTS	_cachedContents;			// all the contents has been cached, or importing, or deleting
	CONTENTS	_idleContents;				// the cached content is not being viewed & cacing. Sub map of _idleContents
	CONTENTS	_cachingContents;			// the content is caching or deleting. Sub map of _cachingContents

	CONTENT_EVENTS	   _cntEvents;		// content events
	ZQ::common::RWLock _cntEventLock;			

	SYS::SingleObject  _waitEvent;
	bool			   _bThreadRunning;
	uint32			   _checkInterval;

protected:
	int setNewline(char* buff, int len);
	bool parseKeyValue(char* buff, std::map<std::string, std::string>& keyValues);

	bool loadData();
	bool writeData();
	void postworkLoadData(Content& cnt);

	bool loadTxtData();
	bool writeTxtData();

	int run(void);
	void final(void);

	void calcHitrate();					// calcuate the hitreate
	void checkUsage();					// check usage and delete content if necessary
	void checkCachingList();			// check the content in caching list to see if timeout 
	void checkCachedContents();			// check _cachedContents in idle time
	void checkUncachedContent();		// remove the uncahced content if last play is x days ago
	void checkScheduledImport();		// 

	void printCachingList();
	void doStatistics(uint64 winStart, uint64 winStop);

	void scheduledErrorHandle(Content& cnt);
public:

	// thread related functions
	bool initialize(bool syncAtStart);
	void uninitialize();

	bool setDataFile(std::string& dataType, std::string& dataPath, int32 flushInterval);
	void setSubFileCount(uint32 subFileCount);
	void setDefaultVisa(uint32 visa);
	void setTrimdays(uint32 days);
	void setCushion(uint32 cushion);
	void setStatParams(bool enable, std::string& dataPath, uint32 window, uint32 interval, uint32 delay, uint32 extraSizePer, bool printDetails);
	void setCacheRetryInterval(uint32 interval);

	void setUsage(uint64 freeSize, uint64 totalSize);
	void getUsage(uint64& freeSize, uint64& totalSize);

	std::string getName() { return _name; };
	uint32 getSubFileCount() { return _subFileCount; };
	
	void getHits(uint64& local, uint64& total) { local = _localHits; total = _totalHits; };

	void outputStatDataToFile(Content& content, ContentOpData& opData);
	bool isStatEnabled() { return _statEnabled; };
public:
	void playStart(std::string& pid, std::string& paid, uint64 timestamp);
	void playEnd(std::string& pid, std::string& paid, uint32 duration);
	void proactiveImport(std::string& pid, std::string& paid, uint64 timeStart, uint64 timeExpired);

	// call back routine by CacheIO, they were called in CacheIO thread
	void onMetadata(std::string& pid, std::string& paid, PROPERTIES& properites);
	void onStatusUpdate(std::string& pid, std::string& paid, Content::CONTENT_STATUS status, uint32 bitrate, uint32 duration, uint64 size);
	void onUsageUpdate(uint64 freeSpace, uint64 totalSpace);

protected:
	void doPlayStart(std::string& pid, std::string& paid, uint64 timestamp);
	void doPlayEnd(std::string& pid, std::string& paid, uint32 duration);
	void doStatusUpdate(std::string& pid, std::string& paid, Content::CONTENT_STATUS status, uint32 bitrate, uint32 duration, uint64 size);
	void doProactiveImport(std::string& pid, std::string& paid, uint64 startTime, uint64 expirationTime);

	bool doIODelete(Content& content);
	bool doIOImport(Content& content);
	bool doIORead(Content& content, PROPERTIES& properties);
	bool doIOListContent();

	Content* findContentInUncachedList(std::string& pid, std::string& paid);
	Content* findContentInCachedList(std::string& pid, std::string& paid);

	// the following two routines can not be nested in the _cachedContents/_uncachedContents loop
	void cached2UnCached(Content& cnt);
	void uncached2Cached(Content& cnt);
	
	Content* createContentAndSave(std::string& pid, std::string& paid, Content::CONTENT_STATUS status, uint32 bitrate=0, uint32 duration=0, uint64 size=0);

	// following two routines are called when playStart/PlayEnd
	void makeContentBusy(Content& cnt);     // move content out of idle list
	void makeContentIdle(Content& cnt);	    // move content in idle list, if it is in caching list, no move
	// following two routines are called when cache import/delete start/end
	void makeCacheWorkDone(Content& cnt);   // will move content to idle list if no viewing on it
	void makeCacheWorkStart(Content& cnt);  // add content to caching list and out of idle list
	// to schedule the importing based on scheduled time
	void makeContentScheduled(Content& cnt);
	void makeContentNoneScheduled(Content& cnt);
};

typedef std::priority_queue <Content*, std::vector<Content*>, CacheStorage::cnt_playcount_greater> CONTENT_PLAYS_QUEUE;

class ContentEvent 
{
public:
	ContentEvent(CacheStorage& cs, std::string& pid, std::string& paid);
	virtual ~ContentEvent();

public:
	virtual void process() = 0;

protected:
	CacheStorage& _cachestorage;

	std::string	  _pid;
	std::string	  _paid;
};

class PlayStartEvent : public ContentEvent
{
public:
	PlayStartEvent(CacheStorage& cs, std::string& pid, std::string& paid, uint64 timestamp);
	virtual ~PlayStartEvent();

public:
	void process();

protected:
	uint64		_startTime;
};

class PlayStopEvent : public ContentEvent
{
public:
	PlayStopEvent(CacheStorage& cs, std::string& pid, std::string& paid, uint32 duration);
	virtual ~PlayStopEvent();

public:
	void process();

protected:
	uint32 _duration;
};

class StatusUpdateEvent : public ContentEvent
{
public:
	StatusUpdateEvent(CacheStorage& cs, std::string& pid, std::string& paid, Content::CONTENT_STATUS status, uint32 bitrate, uint32 duration, uint64 size);
	virtual ~StatusUpdateEvent();

public:
	void process();

protected:
	Content::CONTENT_STATUS _status;

	uint32					_bitrate;
	uint32					_duration;
	uint64					_size;
};

class ProactiveImportEvent : public ContentEvent
{
public:
	ProactiveImportEvent(CacheStorage& cs, std::string& pid, std::string& paid, uint64 startTime, uint64 expirationTime);
	virtual ~ProactiveImportEvent();

public:
	void process();

protected:
	uint64                  _startTime;
	uint64                  _expirationTime;
};

}

#endif