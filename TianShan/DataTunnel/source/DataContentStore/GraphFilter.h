// ============================================================================================
// Copyright (c) 2006, 2007 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Ken Qian
// Desc  : Define the Content Process Graph and Filter filter class
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/TianShan/DataTunnel/source/DataContentStore/GraphFilter.h 1     10-11-12 16:05 Admin $
// $Log: /ZQProjs/TianShan/DataTunnel/source/DataContentStore/GraphFilter.h $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 1     09-03-09 10:33 Li.huang
// 
// 1     08-12-08 11:11 Li.huang
// 
// 1     08-10-30 16:45 Ken.qian
// Move files from /ZQProjs/Generic/ContentProcess to local folder, since
// files at ContentProcess were never used by others components. And
// remove the pacing codes from NTFSIORender to indepent on Vstrm DLL
// 
// 24    08-01-16 15:20 Fei.huang
// add: adjust session time 
// 
// 23    08-01-02 18:35 Ken.qian
// 1. Add minBuffCount for Graph and as the parameter for BufferPool
// 2. move minAvailBuffCount from SourceFilter to Graph, and change the
// name to reservedCount
// 
// 22    07-12-14 13:57 Ken.qian
// use _vsnprintf to replace _vsprintf, and adjust the head include order
// to avoid compiling error: '_vsnprintf' : ambiguous call to overloaded
// function
// 
// 21    07-11-16 19:13 Ken.qian
// Graph support statechange event, such as Streamable
// 
// 20    07-11-15 15:16 Ken.qian
// make endOfStream virtual
// 
// 19    07-11-14 14:09 Ken.qian
// add startTime and endTime
// 
// 18    07-10-23 18:28 Ken.qian
// add mutex in report completion logic for issue of NGOD62
// 
// 17    07-10-15 18:12 Ken.qian
// make buffer processing log frequency MICRO based.
// 
// 16    07-09-11 14:27 Ken.qian
// add waitHandle in Graph quit
// 
// 15    07-08-09 17:10 Ken.qian
// 
// 14    07-07-26 18:08 Ken.qian
// fix vstrm io vvx issue
// 
// 13    07-07-23 15:29 Ken.qian
// vstrmiorender support subfiles and 64K IO writting
// 
// 12    07-07-17 14:18 Ken.qian
// 
// 11    07-06-27 15:13 Ken.qian
// Change kbps to bps
// 
// 10    07-06-14 11:14 Cary.xiao
// 
// 9     07-06-06 15:37 Ken.qian
// 
// 9     07-06-05 18:25 Ken.qian
// add the detail errorcode
// 
// 8     07-06-01 17:05 Ken.qian
// 
// 7     07-04-26 21:26 Ken.qian
// 
// 6     07-04-20 16:33 Ken.qian
// 
// 5     07-04-20 10:36 Ken.qian
// 
// 4     07-04-19 17:31 Ken.qian
// 
// 3     07-04-16 11:04 Ken.qian
// 
// 2     07-04-10 19:17 Ken.qian
// GraphPool Unitest Pass
// 
// 1     07-03-30 19:26 Ken.qian
// 
// 8     06-09-19 11:55 Ken.qian
//



#ifndef __ZQ_GraphFilter_H__
#define __ZQ_GraphFilter_H__

#pragma warning (disable : 4786)

#include <list>
#include <map>

#include "Variant.h"
#include "NativeThread.h"
#include "bufferpool.h"
#include "PropertyKeyDef.h"

#define DEFAULT_REPORT_INTERVAL     20
#define DEFAULT_STRAMABLE_INTERVAL  30
#define DEFAULT_FILETER_LOGING_FEQ  10000   // per 10000 buffer log a time
#define DEFAULT_SLEEP_TIME          1

#define DEFAULT_AVAILABLE_BUFF_SIZE 10

namespace ZQ { 
namespace Content { 
namespace Process {

// source filter's protocol
const std::string SRCFILTER_PROTO_FTP   = "ftp";
const std::string SRCFILTER_PROTO_FILE  = "file";


/// error code 

const int ERRCODE_UNSUPPORT_PROTO       = 100;
const int ERRCODE_INVALID_URL           = 101;
const int ERRCODE_NO_AUTH               = 102;

const int ERRCODE_OPENFILE_FAIL         = 200;
const int ERRCODE_READFILE_FAIL         = 201;
const int ERRCODE_CREATEFILE_FAIL       = 202;
const int ERRCODE_WRITEFILE_FAIL        = 203;

const int ERRCODE_VSTRM_INIT_FAIL       = 301;
const int ERRCODE_VSTRM_NO_BANDWIDTH    = 302;

const int ERRCODE_RTFLIB_OPENSESSSION_FAIL = 401;
const int ERRCODE_RTFLIB_PROCESSBUFF_FAIL  = 402;

const int ERRCODE_TRICKGEN_CREATECONTEXT   = 501;
const int ERRCODE_TRICKGEN_PROCESSPIC_FAIL = 502;

/// content property name definition
const std::string CNTPRY_SUBFILE_NO       = "FileNo";            // type = DWORD
//
// specify the buff position in IO, if not specified, just write to current pos
// if no specifying, the Buffer for IO: 
//                                       CNTPRY_IO_SEEK_ORIGION = SEEK_ORIGIN_CUR
//                                       CNTPRY_IO_SEEK_OFFSET = 0 (long type for Variant)
//
const std::string CNTPRY_IO_SEEK_ORIGION  = "IOSeekOrigin";     // type = unsigned long
const std::string CNTPRY_IO_SEEK_OFFSET   = "IOSeekOffset";     // type = int64
// const for CNTPRY_IO_SEEK_ORIGION 
const unsigned long SEEK_ORIGIN_BEG = 0;
const unsigned long SEEK_ORIGIN_CUR = 1;
const unsigned long SEEK_ORIGIN_END = 2;

//
// specify the vvx output buffer type output by Trick-Gen
//
const std::string CNTPRY_TRICK_GEN_VVX  = "TrickGenVVX";        // special for Trick-Gen, type = DWORD
const unsigned long TRICK_GEN_VVX_NORMAL = 0;
const unsigned long TRICK_GEN_VVX_FLUSH  = 1;
const unsigned long TRICK_GEN_VVX_HEADER = 2;

static std::string SEEK_ORIGIN_STR[3] = { "SEEK_ORIGIN_BEG", "SEEK_ORIGIN_CUR", "SEEK_ORIGIN_END" };

typedef std::map<std::string, ZQ::common::Variant> ContentProperty;

class Filter;
class GraphPool;
class SourceFilter;
class ProvisionRequest;

static void getSystemErrorText(std::string& strErr)
{
	const int MAX_SYS_ERROR_TEXT = 256;
	char sErrorText[MAX_SYS_ERROR_TEXT+50]={0};
  
	DWORD lastError = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
					 NULL, 
					 lastError,
					 0,
					 sErrorText, 
					 MAX_SYS_ERROR_TEXT, 
					 NULL);
	char errcode[24];
	sprintf(errcode, "[%d]", lastError);

    strErr = std::string(sErrorText)+ std::string(errcode);
}
// -----------------------------
// class Graph
// -----------------------------
/// The Graph is container of all the filter derived objects, there is only one source in the graph
///  and the manager of the BufferPool
class Graph : protected ZQ::common::NativeThread 
{
	friend class GraphPool;
	friend class Filter;
	friend class SourceFilter;
public:
	typedef enum { CNT_STEAMABLE = 0 } GRAPH_CONTENT_STATE;

	/// constructor
	///@param[in]	poolSize	The buffer pool size
	///@param[in]	buffSize    The buffer size of each buffdata in buffer pool. 
	///                         if > 0, the Buffer in pool is pre-allocated. But the individual buff size in Pool can changed dynamically, 
	//                                  in "free()" if its size is more than buffSize, which be physically released from memory
	///                         if == 0, the buff in pool is allocated at running time.
	Graph(ZQ::common::Log* pLog = NULL, 
		bool sourceFirst=false, 
		int buffPoolSize = DEFAULT_BUFFER_POOL_SIZE, 
		int buffSize = DEFAULT_POOL_BUFFER_SIZE, 
		int minBuffPoolSize = DEFAULT_BUFFER_POOL_SIZE_MIN, 
		int reservedBuffCount = 0);

protected:	
	virtual ~Graph();

public:
	//get the source filter by protoType
	//@param[in] protoType    if only one source filter, it can be not specified
	SourceFilter* getSource(std::string protoType="");
	const Filter* getRender(std::string name);

public:
	/// Initial the starting of content processing by the filter derived objects in Graph
	///@param[in]	contentURL	The content's url
	///@param[in]   contentName The content name
	///@param[in]   startTime   The provision start time
	///@param[in]   endTime     The provision end time
	///@param[in]   maxbps      The max processing speed for this content, 0 means no limitation
	///@param[in]   orderSrcFirst The start order of filters, orderSrcFirst - true, start source, then renders
	///@return true if filter derived objects are all successfully started
	virtual bool provision(std::string contentURL, std::string contentName, 
							DWORD maxbps=0, time_t startTime=0, time_t endTime=0,
							bool orderSrcFirst=false);
	
	/// Abort all the Filter derived object's processing
	///@return true if the abort operation succeed
	bool abortProvision(void);

	/// stop the Graph processing
	bool stopProvision();

	/// stop graph and all filter's thread, after stopping, Graph can not use anymore
	/// so Stop must be invoked in another thread
	bool quit();

	/// notify completion by Filters - this function must be invoked when filter
	/// processing is completed
	void notifyCompletion(Filter& filter);

	void notifyStateChange(Filter& filter, GRAPH_CONTENT_STATE state);
public:
	/// set one filter as the ProgessReporter, 
	/// in a graph, ONLY one filter could as the reporter.
	///@param[in]	filter	The reporter
	bool setProgressReporter(Filter* render);

	DWORD getReservedBuffCount() { return _reservedBuffCount; };

public:
	/// set Graph property to map
	/// @param[in] property   The property name, as the key of map
	/// @param[in] value  The value of the property specified by name
	void setProperty(std::string property, ZQ::common::Variant value) { _properties[property] = value; };

	/// set the last error by Filters
	void setLastError(int errorcode, std::string errstr) { _errorCode = errorcode; _lastError = errstr; };

protected:
	/// add a source filter derived object to the Graph
	/// A Graph can ONLY have one source
	///@param[in]	src	The source object in the Graph
	///@return true if the adding succeed
	bool addSource(SourceFilter* src);
	
	/// add a filter derived object to the Graph
	/// Which are all behind the source object.
	///@param[in]	filter	The filter derived object
	///@return true if the adding succeed
	bool addRender(Filter* render);

	/// notify completion 
	/// processing is completed
	void notifyCompletion();

	/// Check whether the task is completed since calling start()
	/// @return true if all the filter has completed
	bool hasCompleted();
	
	/// Block the graph, until the processing is completed
	///@param[in]	expectedInterval	The interval of expected processing time. (ms)
	///@return true if the Graph successfully completed the task
	virtual bool waitForCompletion(ProvisionRequest& request);//, DWORD expectedInterval = INFINITE);
	
	/// implementation of NativeThread virtual function
	int run(void);

public:
	/// Get the the content URL
	///@return       the content source URL
	std::string getSourceURL(void) { return _sourceURL; };

	/// Get the the content name
	///@return       the content name
	std::string getContentName(void) { return _contentName; };

	/// Get the the max bandwidth
	///@return       the maxbps
	DWORD getMaxbps() { return _maxbps; };

	/// Get the the provision start time
	///@return       the begin time
	time_t getStartTime() const {
		ZQ::common::MutexGuard guard(_intervalMutex);
		return _startTime; 
	}

	/// Get the the provision end time
	///@return       the end time
	time_t getEndTime() const {
		ZQ::common::MutexGuard guard(_intervalMutex);
		return _endTime; 
	}

	void adjustSessionTime(time_t newStart, time_t newEnd);

	/// Get execute interval
	inline DWORD getExecInterval() const {
		ZQ::common::MutexGuard guard(_intervalMutex);
		return _expectInterval;
	}

	/// update exec interval
	inline void setExecInterval(DWORD interval) {
		ZQ::common::MutexGuard guard(_intervalMutex);
		_expectInterval = interval;
	}

	/// Get the the content size, only can be used after graph begin succeed
	///@return       the maxbps
	__int64 getContentSize();

	/// Get the reference of BufferPool
	///@return       reference the buffer pool
    ZQ::Content::BufferPool& getBuffPool(void) { return _buffPool; };

	// report properties
	void reportProperty(const std::string fileName, ContentProperty properies);

	// write log
	void writeLog(ZQ::common::Log::loglevel_t level, DWORD tid, char* fmt, ...);
	void traceLog(DWORD tid, char* fmt, ...);

	ZQ::common::Log* getLoggerPointer() { return _pLog; };

	bool isTracingDetails() { return _traceDetails; };
protected:
	// property related variables
	typedef std::map<std::string, ZQ::common::Variant> PropertyMap;
	PropertyMap                      _properties;
	
	// buff pool
	ZQ::Content::BufferPool          _buffPool;

	// provision parameters
	std::string                      _sourceURL;
	std::string                      _contentName;
	DWORD                            _maxbps;
	time_t                           _startTime;
	time_t                           _endTime;
	
	// Graph Filters
	GraphPool*                       _graphPool;

	typedef std::map<std::string, SourceFilter*> SourceFilterMap;
	SourceFilterMap                  _sources;           // source filters, only one filter is available while processing
	                                                     // the key is the proto type, defined in const at the head of this file 
	SourceFilter*                    _activeSource;      // the active source filter

	std::list<Filter*>               _renders;
	bool                             _orderSourceFirst;  // the provision order of filters.
	                                                     // true - start source thread, then renders when begin to do provision

	// Variables for Graph thread control
	bool                             _bContinue;
	HANDLE                           _hStop;
	HANDLE                           _hCompleted;

	HANDLE                           _hOtherStateChange;
	WORD                             _bitStateChange;
	ZQ::common::Mutex                _bitSCMutex;

	ZQ::common::Mutex                _tepNotifiedMutex;
	std::map<DWORD, Filter*>         _tmpNotifiedList;

	bool                             _provResult;

	int                              _errorCode;
	std::string                      _lastError;

	ZQ::common::Mutex                _pryMutex;
	ContentProperty                  _curProperties;

	// others
	ZQ::common::Log*                 _pLog;
	std::string                      _logHearder; 

	bool                             _traceDetails;

	// The progress reporter
	Filter*                          _progressReporter; 

	DWORD                            _reservedBuffCount;

	HANDLE							 _execIntervalChange;
	mutable ZQ::common::Mutex		 _intervalMutex;
	DWORD							 _expectInterval;
};

// -----------------------------
// class filter
// -----------------------------
/// The filter is a filter class for content process object
///  each filter drived class's object represent a type of content processing
class Filter : public ZQ::common::NativeThread
{	
	friend class Graph;
public:
	/// constructor
	///@param[in]	graph		    The Content Graph that container all the filter object
	///@param[in]   myName      The name of each Filter, if "", it will use class Name as the myName
	Filter(ZQ::Content::Process::Graph& graph, std::string myName="");
	
protected:
	/// destructor
	virtual ~Filter(void);

public:
	typedef enum _ProcessStatus
	{
		PAUSED = 0,
		ACTIVE,
		STOPPED,
		ABORTED
	}ProcessStatus;
		
	/// get filter's Name
	std::string getMyName() {return _myName; };	

	/// get Graph 
	Graph& getGraph() {return _graph; };

	/// receiving the buffer coming from previous content process filter
	/// The received Content Process filter is required to re allocate buffer from pool
	/// and copy it. To Source object, you are not required to implement this function
	/// @param[in]  upObj   the obj who call this function
	/// @param[in]  buff      the buff in the pool which allocated in above filter
	virtual bool receive(Filter* upObj, ZQ::Content::BufferData* buff) = 0;
	
	/// start to process the incoming BufferData
	/// @return true if it start successfully
	virtual bool begin(void) = 0;
	
	/// pause to process the incoming BufferData
	/// @return true if it pause successfully
	virtual bool pause(void) = 0;
	
	/// abort current buffer processing, generally, this is invoked by the Graph
	/// in case of any filter obj failed during the processing, 
	/// and this failure require all object need to aborted. And all the had received data
	/// will not be process any more.
	/// @return true if it abort successfully
	virtual bool abort(void) = 0;

	/// stop content processing, just a little bit different with abort(), 
	/// it is a normal stopping, but abort() is abnormal.
	virtual void stop(void) = 0;
	
	/// stop the processing and exit the Filter thread
    virtual void quit(void) = 0;

	/// Notify the filter derived object that there is no any coming data
	/// After receiving this notification, the thread process all the received data
	//  then flush the data if required, then stop process until the next starting
	virtual void endOfStream(void)=0;

	/// this virtual function must be render, to get know current processing progress,
	/// bytes or something else.
	virtual __int64 getProcessedStuff() = 0;

public:	
	/// Check whether the filter is now active (running or pause)
	/// @return true if it is active
	bool isActive(void);
	
	/// Check whether the filter is now processing
	/// @return true if it is processing
	bool isProcessing(void);
	
public:
	/// Add one down side filter object to the vector
	/// @param[in]  obj      The down side filter object
	/// @param[in]  copyBuff Whether the passed BuffData to connected downlink filter need to be duplicated,
	///                      if yes(copyBuff==true), the downlink filter need to copy the passed BuffData, 
	///                                              and the uplink filter is responsible for release the buff.
	///                      if not(copyBuff==false), the downlink filter does not copy the passed BuffData, 
    ///                                              and the uplink filter can not release the BuffData,
	///                                              and the downlink filter is responsible for that.
	///                      Notice: copyBuff setting must be used carefully, if one DataBuff is used by multiple downlink
	///                              filters or it is still useful in its own after receive() invoking, in this case, 
	///                              copyBuff must be true, otherwise, these is going to memory access problem.
	virtual bool connectTo(Filter* obj, bool copyBuff=true);

	
	/// Remove the down side filter object from the vector
	/// @param[in]  obj      The down side filter object
	void disconnect(Filter* obj);
	
protected:
	/// Deliver the BufferData to all its down filter object
	/// @param[in]  buff      the BufferData delivered to down connected filter object
	void deliverBuffer(ZQ::Content::BufferData* buff);

	/// Release the buff according the downlink's _copyUplinkDataBuff, 
	/// if there is no downlink filter, just free it.
	/// @param[in]  buff      the BufferData is going to be released
	/// @param[in]  render    specify to release the BuffData which passed to the render, 
	///                       if NULL, it is used when you use deliverBuffer() to pass BuffData
	///                       if NOT NULL, the BuffData is passed individually by invoking render's receive() function.
	/// return true - means the BufferData is really released. 
	bool releaseBuffer(ZQ::Content::BufferData* buff, Filter* render = NULL);
	
	/// Notify the end of stream event to all the renders connected this filter derived object
	virtual void notifyEndOfStream(void);
	
	/// Deliver content property to all the direct renders
	virtual void deliverContentProperty(std::string property, ZQ::common::Variant value);

public:
	/// set Filter property to map, 
	/// mostly it is invoked by source filter to let sub filter render
	/// know properties about the source content, such as content file size, create time, etc
	/// @param[in] property   The property name, as the key of map
	/// @param[in] value  The value of the property specified by name
	void setProperty(std::string property, ZQ::common::Variant value);

	/// get Filter property from map, 
	/// mostly it is invoked by connected filter to get properties passed from source fileter
	/// @param[in] property   The property name, as the key of map
	/// @param[in] value  The value of the property specified by name
	bool getProperty(std::string property, ZQ::common::Variant& value);
	
protected:
	typedef std::map<std::string, ZQ::common::Variant> PropertyMap;
	PropertyMap                            _srcProperty;

	std::list<Filter*>                     _desObjs;
	Filter*                                _srcObj;
	ZQ::Content::Process::Graph&           _graph;
	ZQ::Content::BufferPool&               _pool;
	
	ProcessStatus                          _processStatus;

	std::string                            _myName;

	/// flag to determine whether copy BuffData in receive() function.
	bool                                   _copyUplinkDataBuff;
};

class SourceFilter : public Filter
{
	friend class Graph;

public:
	/// constructor
	///@param[in]	graph		The Content Graph that container all the filter object
	///@param[in]	protoType   The source type filter supported rotocol: ftp, file, etc, they are defined as const
	///@param[in]   myName      The name of each Filter, if "", it will use class Name as the myName
	SourceFilter(ZQ::Content::Process::Graph& graph, std::string protoType, std::string myName="");

protected:
	/// destructor
	virtual ~SourceFilter(void);

protected:
	/// receiving the buffer coming from previous content process filter
	/// The received Content Process filter is required to re allocate buffer from pool
	/// and copy it. To Source object, you are not required to implement this function
	/// @param[in]  upObj   the obj who call this function
	/// @param[in]  buff      the buff in the pool which allocated in above filter
	inline bool receive(Filter* upObj, ZQ::Content::BufferData* buff) {
		return false; 
	};

public:
	/// Notify the filter derived object that there is no any coming data
	/// After receiving this notification, the thread process all the received data
	//  then flush the data if required, then stop process until the next starting
	virtual void endOfStream() {
	};

	/// this virtual function MUST be implemented by source filter, to get know 
	/// how many process stuff in the whole processing, this could be source file total
	/// bytes or something else. 
	/// Currently seems only the source could provide the total number
	virtual __int64 getTotalStuff() = 0;

	/// get the protocol type
	inline std::string getProtoType() const {
		return _protoType; 
	};
	
private: 
	std::string  _protoType;     // the supported protocol of this source filter
};

} } }

#endif // __ZQ_GraphFilter_H__
