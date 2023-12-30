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
// Desc  : Implement the RFTLib Filter
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/ContentProcess/RTFLibFilter.cpp 1     10-11-12 15:58 Admin $
// $Log: /ZQProjs/Generic/ContentProcess/RTFLibFilter.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 31    08-03-06 17:00 Ken.qian
// 
// 30    08-02-25 17:40 Ken.qian
// notify endofstream to connected filter only once if
// RTFLibFilter/TrickFileGenFilter output to one filter(support subfile)
// 
// 29    07-11-16 19:15 Ken.qian
// seperate the time of update bitrate,framecode, etc property with
// playtime
// 
// 28    07-11-07 12:01 Ken.qian
// handle the main file output in receive() instead of create addtional IO
// render to receive main file buffers outside
// 
// 27    07-10-31 15:10 Ken.qian
// add try catch in uninitRTFLib()
// 
// 26    07-10-31 14:27 Ken.qian
// add closeSession() in thread exiting
// 
// 25    07-10-23 18:29 Ken.qian
// make rtf input/output buffer count configurable, default 64. Instead of
// original 16K
// 
// 24    07-10-15 18:12 Ken.qian
// make buffer processing log frequency MICRO based.
// 
// 23    07-09-20 17:59 Ken.qian
// add log while processing buffer
// 
// 22    07-09-17 15:28 Ken.qian
// 1. replace getUnallocatedCount()  with getUsedCount()
// 2. close session in abort case
// 3. RTF_ABSMAX_SESCOUNT as the max RTFLib session count
// 
// 21    07-09-11 14:28 Ken.qian
// add protection for buffer-deadlock since input and output are sharing
// same buffer pool. 
// 
// 20    07-08-30 14:11 Ken.qian
// set session handle to be NULL in begin()
// 
// 19    07-08-30 14:08 Ken.qian
// fix the logic of update property after opensession
// 
// 18    07-08-28 13:59 Ken.qian
// move updateStartProperty to be run() also
// 
// 17    07-08-23 10:58 Ken.qian
// 1. move opensession to run() instead of begin()
// 2. does not invoke rtfCloseSession when abort happen
// 
// 16    07-08-17 14:32 Ken.qian
// 
// 15    07-08-16 12:46 Ken.qian
// 
// 14    07-08-14 14:31 Ken.qian
// 
// 13    07-08-09 17:10 Ken.qian
// 
// 12    07-08-01 17:57 Ken.qian
// 
// 11    07-07-26 18:08 Ken.qian
// fix vstrm io vvx issue
// 
// 10    07-07-23 15:29 Ken.qian
// vstrmiorender support subfiles and 64K IO writting
// 
// 9     07-07-19 17:45 Ken.qian
// normal release for rtfcpnode
// 
// 8     07-07-18 20:03 Ken.qian
// 
// 7     07-07-18 19:36 Ken.qian
// fix buffpool issue
// 
// 6     07-07-18 16:10 Ken.qian
// 
// 5     07-07-17 22:06 Ken.qian
// 
// 4     07-07-17 17:47 Ken.qian
// 
// 3     07-07-17 17:05 Ken.qian
// 
// 2     07-07-17 14:18 Ken.qian
// 
// 1     07-07-14 20:10 Ken.qian
// Initial Coding
// 
#include "RTFLibFilter.h"
#include "urlstr.h"
#include "bufferpool.h"
#include "mpeg.h"

namespace ZQ { 
namespace Content { 
namespace Process {


#define DEF_MAX_QUEUE_DEPTH     100
ZQ::common::Log* RTFLibFilter::_rtfLogger = NULL;
bool RTFLibFilter::_trickKeyFrame = false;
RTF_WARNING_TOLERANCE RTFLibFilter::_rtfTolerance = RTF_WARNING_TOLERANCE_RELAXED;
bool RTFLibFilter::_rtfInited = false;
char RTFLibFilter::_rtfVersion[16];

DWORD RTFLibFilter::_rtfInputBuffSize = INPUT_FILE_BUFFER_BYTES_64;
DWORD RTFLibFilter::_rtfOutputBuffSize = DEF_OUTPUT_FILE_BUFFER_BYTES;

static unsigned long trickSpeedNumerator[RTF_MAX_TRICKSPEEDS] = { 30, 30, 30 };
static unsigned long trickSpeedDenominator[RTF_MAX_TRICKSPEEDS] = { 4,  2,  1 };

static char* VIDEO_CODEC_TYPE_STR[] = {"Invalid", "Mpeg2", "H264", "VC1"};
#define SUB_CONTNT_TYPE_VVX      "VVX"
#define SUB_CONTNT_TYPE_VV2      "VV2"

RTFLibFilter::RTFLibFilter(ZQ::Content::Process::Graph& graph, bool outputToOneIORender, 
						   int numTrickSpeed, bool unifiedTrickFile, 
						   std::string myName)
: Filter(graph, myName), _hDesFile(INVALID_HANDLE_VALUE), _processedBytes(0),
_bEndOfStream(true), _outputToOneIORender(outputToOneIORender), _eosSubFileCount(0), _sessClosed(false)
{
	//
	// set basic trick configuration
	//
	_sessionHandle = NULL;

	// default index Type
	_indexType = RTF_INDEX_TYPE_VVX;
	_indexOption.vvx = (true == _trickKeyFrame) ? RTF_INDEX_OPTION_VVX_7_3 : RTF_INDEX_OPTION_VVX_7_2; 

	// default video type 
	_vcdType = RTF_VIDEO_CODEC_TYPE_MPEG2;

	// set index mode as REAL TIME as default
	_indexMode = RTF_INDEX_MODE_REALTIME;

	// set trick speeds
	_numTrickSpeed = numTrickSpeed;
	if(_numTrickSpeed <= 0)
	{
		_numTrickSpeed = 1;
	}
	else if(_numTrickSpeed > RTF_MAX_TRICKSPEED_NUMBER)
	{
		_numTrickSpeed = RTF_MAX_TRICKSPEED_NUMBER;
	}

	// set output file count
	_unifiedTrickFile = unifiedTrickFile;

	//
	// Initialize output file information
	//
	_connectedIORenderCount = 0;
	initOutputFileInfo();

	//
	// filter related 
	//
	if(myName == "")
	{
		_myName = "RTFLibFilter";
	}

	_tidAbortBy = 0;
	
	_hStop = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);

	// start the thread
	start();
}

RTFLibFilter::~RTFLibFilter(void)
{
	if(isRunning())
	{
		SetEvent(_hStop);
		
		// to make sure if the thread is stopped even it is suspended
		resume();
		
		// is there any issue if the _hStop has been destruct
		// so wait until all the 
		waitHandle(INFINITE);
	}
	
	// close the handle
	if(_hStop != NULL)
	{
		CloseHandle(_hStop);
		_hStop = NULL;
	}
	if(_hNotify != NULL)
	{
		CloseHandle(_hNotify);
		_hNotify = NULL;
	}	
}

void RTFLibFilter::emptyDataQueue(void)
{
	ZQ::common::MutexGuard guard(_dataMutex);
	// remove all the buffer data pointer from the queue.
	while (!_dataQueue.empty())
	{
		ZQ::Content::BufferData* pBuffData = _dataQueue.front();
		
		_pool.free(pBuffData);
		_dataQueue.pop();

		_graph.traceLog(id(), "RTFLibFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
					    pBuffData);
	}	
}

bool RTFLibFilter::receive(Filter* upObj, ZQ::Content::BufferData* buff)
{	
	//
	// Since RTFLib output main file size maybe not as same as source file, 
	// so have a copy of buffer and deliver to IO render directly
	//
	ZQ::Content::BufferData* cpiedBuff = NULL;
	
	// copy BuffData
	_graph.traceLog(id(), "BufferPool: usage: %d / %d [used/total]", 
			  _pool.getUsedCount(), _pool.getPoolSize());
	
	// copy the buffer to the new one
	cpiedBuff = _pool.alloc();
	*cpiedBuff = *buff;

	_graph.traceLog(id(), "RTFLibFilter: alloc buffData from pool. [BuffData Address: 0x%08X]", 
					cpiedBuff);

	DWORD fileNo = 0;  // main file id is 0
	// set the fileNo in the buffer property
	if(_connectedIORenderCount)
	{
		cpiedBuff->setProperty(ZQ::Content::Process::CNTPRY_SUBFILE_NO, (DWORD)fileNo);
	}

	// pass the buffer to the connected IO render
	_outputInfo[fileNo].ioRender->receive(this, cpiedBuff);

	// release the buffer
	bool bReleased = releaseBuffer(cpiedBuff);
	if(bReleased)
	{
		_graph.traceLog(id(), "RTFLibFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
						cpiedBuff);
	}
	
	//
	// put the received buffer to the queue for trick-gen
	//
	ZQ::Content::BufferData* receivedBuff = NULL;
	if(_copyUplinkDataBuff)
	{	// copy BuffData
		_graph.traceLog(id(), "BufferPool: usage: %d / %d [used/total]", 
				  _pool.getUsedCount(), _pool.getPoolSize());
		
		// copy the buffer to the new one
		receivedBuff = _pool.alloc();
		*receivedBuff = *buff;

		_graph.traceLog(id(), "RTFLibFilter: alloc buffData from pool. [BuffData Address: 0x%08X]", 
						receivedBuff);
	}
	else
	{   // does not copy BuffData
		receivedBuff = buff;
	}

	if(STOPPED == _processStatus || ABORTED == _processStatus)
	{
		_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "RTFLibFilter: it is in STOPPED or ABORTED status, does not receive data any more");
		bool bReleased = releaseBuffer(receivedBuff, this);	
		if(bReleased)
		{
			_graph.traceLog(id(), "RTFLibFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
							receivedBuff);
		}

		return false;
	}

	if(_bEndOfStream)
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "RTFLibFilter: it is end of stream, does not receive data more");

		bool bReleased = releaseBuffer(receivedBuff, this);	
		if(bReleased)
		{
			_graph.traceLog(id(), "RTFLibFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
							receivedBuff);
		}
		return false;
	}

	// put the data to the queue
	ZQ::common::MutexGuard guard(_dataMutex);
	_dataQueue.push(receivedBuff);
	
	// since input and output are sharing same buffer pool,
	// but if there is no avaliable buffer for trick-gen, it would be block the process
	// This is intend to slow down the source 
	if(_dataQueue.size() > DEF_MAX_QUEUE_DEPTH)
	{
		Sleep(DEFAULT_SLEEP_TIME);
	}

	_graph.traceLog(GetCurrentThreadId(), "RTFLibFilter: Receive Buffer Data from up side process object with actual length %d", buff->getActualLength());

	SetEvent(_hNotify);

	return true;
}

bool RTFLibFilter::begin(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "RTFLibFilter::begin() enter");
	
	// To avoid invoking begin() during its processing
	if(_processStatus == ACTIVE)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "RTFLibFilter: the task did not complete yet, can not initial new work");

		return false;
	}

	//
	// check the begin is invoked by pause - have begun or just begin
	//
	if(_processStatus != PAUSED)    // begin a new job
	{	
		_rtfBeginReleaseBuffer = false;
		_rtfReleaseBuffCheck = true;

		_sessClosed = false;
		_eosSubFileCount = 0;

		_processedBytes = 0;

		_tidAbortBy = 0;

		_contentBitrate = 0;

		// compose the file full name
		_szDesFileName = _graph.getContentName() ;
		
		_bEndOfStream = false;

		// make sure to empty the queue
		emptyDataQueue();

		_sessionHandle = NULL;

//		std::string errmsg;
//		if(!openSession(errmsg))
//		{
//			// reset session name
//
//			_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "RTFLibFilter: openSession() failed for vcdtype: %s, try VC1 type", VIDEO_CODEC_TYPE_STR[_vcdType]);
//
//			// this is a trick, the type should be specified outside
//			// the _vcdType must reset back original value
//			RTF_VIDEO_CODEC_TYPE orgCodeType = _vcdType;
//			_vcdType = RTF_VIDEO_CODEC_TYPE_VC1;
//			if(!openSession(errmsg))
//			{
//				_graph.setLastError(ERRCODE_RTFLIB_OPENSESSSION_FAIL, errmsg.c_str());
//
//				_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "RTFLibFilter: %s", errmsg.c_str());
//				
//				_vcdType = orgCodeType;
//
//				return false;
//			}
//			_vcdType = orgCodeType;
//		}

		// update properties at started
//		updateFilesetProperties();
	}
	else
	{
		// resume the native thread
		start();	
	}

	// resume the thread
	_processStatus = ACTIVE;
		
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "RTFLibFilter::begin() leave");

	return true;
}

bool RTFLibFilter::pause(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "RTFLibFilter::pause() enter");

	_processStatus = PAUSED;

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "RTFLibFilter::pause() leave");
	
	// suspend the native thread
	suspend();
		
	return true;
}

bool RTFLibFilter::abort(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "RTFLibFilter::abort() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = ABORTED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "RTFLibFilter::abort() leave");
	
	return true;
}

void RTFLibFilter::stop(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "RTFLibFilter::stop() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = STOPPED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "RTFLibFilter::stop() leave");
}

void RTFLibFilter::quit(void)
{
	SetEvent(_hStop);
}

void RTFLibFilter::endOfStream(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "RTFLibFilter: Get end of stream notification");

	_bEndOfStream = true;

	SetEvent(_hNotify);
}

int RTFLibFilter::run(void)
{	
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter::run() enter");

	bool bContinue = true;
	DWORD dwWaitStatus = 0;

	HANDLE handles[2] = { _hStop, _hNotify };

	DWORD dwCounter = 0;

	while(bContinue)
	{
		dwWaitStatus = WaitForMultipleObjects(2, handles, false, INFINITE);
		switch(dwWaitStatus)
		{
		// received stop event, the thread will stop
		case WAIT_OBJECT_0:
			if(_sessionHandle != NULL && !_sessClosed)
			{
				closeSession();
			}

			bContinue = false;
			_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter: get a thread exit event");
			break;

		// received the Notify event
		case WAIT_OBJECT_0 + 1:
		{	
			// check whether this thread is abort by Graph
			if(STOPPED == _processStatus || ABORTED == _processStatus)
			{
				// make sure to remove, 
				emptyDataQueue();

				if(!_sessClosed)
				{
					closeSession();
				}
				else
				{
					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter: session has been closed by RTFLib, no invoking of rtfCloseSession()");
				}

				if(ABORTED == _processStatus)
				{
					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter: It was aborted by Graph, triggered by thread 0x%08X", _tidAbortBy);
				}
				
				// notify graph this filter processing completed
				_graph.notifyCompletion(*this);

				continue;
			}

			std::string errmsg;
			if(NULL == _sessionHandle)
			{
				dwCounter = 0;

				if(!openSession(errmsg))
				{
					_graph.setLastError(ERRCODE_RTFLIB_OPENSESSSION_FAIL, errmsg.c_str());

					_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "RTFLibFilter: %s", errmsg.c_str());
					
					// the render met problem, abort all the renders in the graph
					_graph.abortProvision();

					continue;
				}
				else
				{
					// update properties at started
					updateFilesetProperties();
				}
			}

			ZQ::Content::BufferData* pBuffData = NULL;
			{
				ZQ::common::MutexGuard guard(_dataMutex);
				if(!_dataQueue.empty())
				{
					pBuffData = _dataQueue.front();
					_dataQueue.pop();
				}
			}
			// write the buff to file
			if(pBuffData != NULL)
			{
				if(_rtfReleaseBuffCheck && _rtfBeginReleaseBuffer)
				{
					// there is buffer releasing, means RTFLib had already parsed the necessary information, such as bitrate, etc
					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter: RTFLib consumed %d buffers before first releasing", 
						dwCounter);

					_rtfReleaseBuffCheck = false;

					// Take this chance to fetch the bitrate, etc. And update to graph
					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter:  update content properties ...", 
						dwCounter);
					updateContentProperties();
				}

				if(dwCounter % DEFAULT_FILETER_LOGING_FEQ == 0)
				{
					_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "RTFLibFilter: processBuffer No.%d, there are %d buffers in queue", 
						dwCounter, _dataQueue.size());
				}
				
				DWORD dwLen = 0;
				LPVOID pointer = pBuffData->getPointerForRead(dwLen);
				
				std::string errmsg;
				bool bRet = processBuffer(pBuffData, errmsg);
				if(!bRet)
				{	
					// release the buffer since it has been poped from queue
					bool bReleased = releaseBuffer(pBuffData, this);
					if(bReleased)
					{
						_graph.traceLog(id(), "RTFLibFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
										pBuffData);
					}
					
					// set graph last error
					_graph.setLastError(ERRCODE_RTFLIB_PROCESSBUFF_FAIL, errmsg);
					
					_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "RTFLibFilter: %s", errmsg.c_str());

					// set the status, must be before EmptyDataQueue() invoking
					_processStatus = ABORTED;
					_bEndOfStream = false;
					
					//
					// CAUTION: RTFLib callback should release buff when process buff failed
					// Need to confirm by testing
					//
					
					dwCounter = 0;
					
					// make sure to remove, 
					emptyDataQueue();

					// the render met problem, abort all the renders in the graph
					_graph.abortProvision();
					continue;				
				};

				dwCounter++;
				_processedBytes += dwLen;

				//
				// CAUTION: RTFLib callback should release buff when process buff failed
				// Need to confirm by testing
				//
				
				// go on next loop
				ZQ::common::MutexGuard guard(_dataMutex);
				if(!_dataQueue.empty())
				{
					SetEvent(_hNotify);
					continue;
				}
			}

			// endOfStream() set the _hNotify event
			if(_bEndOfStream)
			{	// end of stream
				_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter: reaches the end of stream, there are %d buffer processed in all, total output size is %I64d bytes", 
								dwCounter, _processedBytes);
				
				// update the playtime, the other properties, such as bitrate, framecode has been updated by updateContentProperties() 
				updateContentPlayTime();

				//
				// reach the end of source file, close RTFLib session
				//
				closeSession();

				//
				// CAUTION: No notifying endofstream there
				// RTFLib trickFileCloseOutput() callback trigger the endofstream event to corresponding IO render
				// notify all the connected renders that reaches the end of stream
				// _outputFileCount include main file, but main file is not output from RTFLib
				if(_eosSubFileCount == _outputFileCount - 1)
				{
					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter: reaches the end of stream, notify IO render the event of end of stream");
					// notify endofstream, include main file
					if(_outputToOneIORender)
					{
						// if output to one render who support sub file, all outputInfo[i]'s ioRender are same
						// so, just notify one
						_outputInfo[0].ioRender->endOfStream();
					}
					else
					{
						// notify endofstream, include main file
						for(int i=0; i<_outputFileCount; i++)
						{
							_outputInfo[i].ioRender->endOfStream();
						}
					}				
				}
				dwCounter = 0;
				
				// make sure to remove, 
				emptyDataQueue();

				// set the status
				_processStatus = STOPPED;
				
				// notify graph this filter processing completed
				_graph.notifyCompletion(*this);

				continue;
			}
			break;
		}
		// received timeout or failed, exit the thread.
		case WAIT_TIMEOUT:
		case WAIT_FAILED:
		default:
			bContinue = false;
			break;
		}		
	}
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter::run() leave");
	
	return 1;
}

bool RTFLibFilter::connectTo(Filter* obj, bool copyBuff)
{
	// invoke base class connectTo
	bool result = Filter::connectTo(obj, copyBuff);
	if(!result)
	{
		return false;
	}

	// maintain the IO render internally
	if(_outputToOneIORender)
	{
		for(int i=0; i<_outputFileCount; i++)
		{
			_outputInfo[i].ioRender = obj;
			_connectedIORenderCount = 1;
		}
	}
	else
	{
		_outputInfo[_connectedIORenderCount].ioRender = obj;
		_connectedIORenderCount++;
	}

	return true;
}


bool RTFLibFilter::getOutputFileInfo(int fileNo, char* ext, int len, DWORD& numerator, DWORD& denominator, OutputFileType& fileType)
{
	if(fileNo<0 || fileNo>_outputFileCount)
	{
		return false;
	}

	fileType = _outputInfo[fileNo].fileType;
	numerator = _outputInfo[fileNo].speedNumerator;
	denominator = _outputInfo[fileNo].speedDenominator;

	strncpy(ext, _outputInfo[fileNo].extension, len);
	
	return true;
}

double RTFLibFilter::getFrameRateByCode(WORD framecode)
{
	switch(framecode)
	{
	case VMPEG_FRAME_RATE_23_976:
		return (double) 23.976;

	case VMPEG_FRAME_RATE_24:
		return (double) 24;

	case VMPEG_FRAME_RATE_25:
		return (double) 25;

	case VMPEG_FRAME_RATE_29_97:
		return (double) 29.97;

	case VMPEG_FRAME_RATE_30:
		return (double) 30;

	case VMPEG_FRAME_RATE_50:
		return (double) 50;
		
	case VMPEG_FRAME_RATE_59_94:
		return (double) 59.94;

	case VMPEG_FRAME_RATE_60:
		return (double) 60;

	default:
		return (double) 0;
	}
	return 0;
}

void RTFLibFilter::updateFilesetProperties()
{
	if (_sessionHandle)
	{
		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "update property at provision started...");
		
		ContentProperty streamProperites;
		
		// set subtype 
		std::string subtype = (RTF_INDEX_TYPE_VV2 == _indexType) ? SUB_CONTNT_TYPE_VV2 : SUB_CONTNT_TYPE_VVX; 
		streamProperites.insert(ContentProperty::value_type(CNTPRY_SEAC_SUBTYPE, subtype));

		// set file count and list
		DWORD fileCount = _outputFileCount;
		streamProperites.insert(ContentProperty::value_type(CNTPRY_FILE_COUNT, fileCount)); 
		
		std::string cntname = _graph.getContentName();
		char pryFileName[256];
		char valFileName[256];
		// main file - FileName1
		sprintf(pryFileName, "%s%d", CNTPRY_FILE_NAME.c_str(), 1);
		streamProperites.insert(ContentProperty::value_type(pryFileName, cntname)); 
		// index file and trick file
		for(int i=1; i<_outputFileCount; i++)
		{
			sprintf(pryFileName, "%s%d", CNTPRY_FILE_NAME.c_str(), i+1); 
			sprintf(valFileName, "%s%s", cntname.c_str(), _outputInfo[i].extension);
			streamProperites.insert(ContentProperty::value_type(pryFileName, valFileName));
		}
		
		_graph.reportProperty(_graph.getContentName(), streamProperites);
	}
}

void RTFLibFilter::updateContentProperties()
{
	if (_sessionHandle)
	{
		RTF_STREAM_PROFILE* profile = NULL;			
		if(rtfSesGetStreamProfile(_sessionHandle, &profile) != RTF_PASS) 
		{
			return;
		}
		
		ContentProperty streamProperites;

		_contentBitrate = profile->bitsPerSecond;

		// update video property
		streamProperites.insert(ContentProperty::value_type(CNTPRY_BITRATE, (DWORD)_contentBitrate));
		streamProperites.insert(ContentProperty::value_type(CNTPRY_VIDEO_BITRATE, (DWORD)profile->videoSpec.eStream.video.bitsPerSecond));
		streamProperites.insert(ContentProperty::value_type(CNTPRY_VIDEO_VERTICAL, (DWORD)profile->videoSpec.eStream.video.height));
		streamProperites.insert(ContentProperty::value_type(CNTPRY_VIDEO_HORIZONTAL, (DWORD)profile->videoSpec.eStream.video.width));
		streamProperites.insert(ContentProperty::value_type(CNTPRY_VIDEO_FRAMERATE, (double)getFrameRateByCode(profile->videoSpec.eStream.video.frameRateCode)));

		// deliver the bitrate to connected filter, since the connected IO need to use this.
		// this added because VstrmIO in case of pacing enable, it need to fire "streamable" event,
		deliverContentProperty(CNTPRY_VIDEO_VERTICAL, (DWORD)profile->videoSpec.eStream.video.height);
		deliverContentProperty(CNTPRY_VIDEO_HORIZONTAL, (DWORD)profile->videoSpec.eStream.video.width);
		deliverContentProperty(CNTPRY_BITRATE, (DWORD)_contentBitrate);

/*	    Move the playtime property update to function updateContentPlayTime()
		__int64 playtime = 0;
		if (_contentBitrate) 
		{
			playtime = (_graph.getContentSize() * 8 * 1000) / ((__int64) _contentBitrate);  // ms
			streamProperites.insert(ContentProperty::value_type(CNTPRY_VIDEO_PLAYTIME, playtime));
		}
*/		
		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter: content bitrate is %d bps", _contentBitrate);

		_graph.reportProperty(_graph.getContentName(), streamProperites);
	}
}

void RTFLibFilter::updateContentPlayTime()
{
	if (_sessionHandle && _contentBitrate > 0)
	{		
		ContentProperty streamProperites;
		
		__int64 playtime = 0;

		playtime = (_graph.getContentSize() * 8 * 1000) / ((__int64) _contentBitrate);  // ms
		streamProperites.insert(ContentProperty::value_type(CNTPRY_VIDEO_PLAYTIME, playtime));

		_graph.reportProperty(_graph.getContentName(), streamProperites);

		_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "RTFLibFilter: content playtime is %d ms", playtime);
	}
}

//////////////////////////////////////////////////////////////////////////
//                  codes for RTFLib wrapper                            //
//////////////////////////////////////////////////////////////////////////
void* RTFLibFilter::appAlloc(RTF_APP_HEAP_HANDLE hAppHeap, int bytes)
{
	return malloc( bytes );
}

void RTFLibFilter::appFree(RTF_APP_HEAP_HANDLE hAppHeap, void *ptr)
{
	free( ptr );
}

void RTFLibFilter::appLog(void *hAppSession, const char *pShortString, char *pLongString)
{
	RTFLibFilter* This = (RTFLibFilter*)hAppSession;

	if(_rtfLogger != NULL && pShortString != NULL && pLongString != NULL)
	{
		(*_rtfLogger)(ZQ::common::Log::L_DEBUG, "[%s] [0x%08X] (RTFLib %s) (RTFLib Log) %s %s", 
			This->_graph.getContentName().c_str(), GetCurrentThreadId(), _rtfVersion, pShortString, pLongString);
	}
}

void RTFLibFilter::rtfLibErrorLog(char *pMessage)
{
	if(_rtfLogger != NULL && pMessage != NULL)
	{
		(*_rtfLogger)(ZQ::common::Log::L_WARNING, "[0x%08X] (RTFLib %s) (RTFLib Error) %s", 
			GetCurrentThreadId(), _rtfVersion, pMessage);
	}
}
	
void RTFLibFilter::sessionErrorNotifier(RTF_APP_SESSION_HANDLE hAppSession, char *pMessage)
{
	RTFLibFilter* This = (RTFLibFilter*)hAppSession;
	
	if(_rtfLogger != NULL && pMessage != NULL)
	{
		(*_rtfLogger)(ZQ::common::Log::L_ERROR,  "[%s] [0x%08X] (RTFLib %s) (Session Error) %s", 
			This->_graph.getContentName().c_str(), GetCurrentThreadId(), _rtfVersion, pMessage);
	}
}

void RTFLibFilter::getTrickExt(int speedNo, char* ext1, char* ext2)
{
	if(_unifiedTrickFile)
	{
		if(0 == speedNo)
		{
			sprintf(ext1, ".FFR");
		}
		else
		{
			sprintf(ext1, ".FFR%d", speedNo);
		}
	}
	else
	{
		if(0 == speedNo)
		{
			sprintf(ext1, ".FF");
			sprintf(ext2, ".FR");
		}
		else
		{
			sprintf(ext1, ".FF%d", speedNo);
			sprintf(ext2, ".FR%d", speedNo);
		}		
	}
}

void RTFLibFilter::setTrickGenParam(RTF_INDEX_TYPE indexType, RTF_VIDEO_CODEC_TYPE codecType)
{
	// index Type
	switch(indexType)
	{
	case RTF_INDEX_TYPE_VV2:
		_indexType = RTF_INDEX_TYPE_VV2;
		_indexOption.vv2 = RTF_INDEX_OPTION_VV2_TSOIP;
		break;
	case RTF_INDEX_TYPE_VVX:
	default:
		_indexType = RTF_INDEX_TYPE_VVX;
		_indexOption.vvx = (true == _trickKeyFrame) ? RTF_INDEX_OPTION_VVX_7_3 : RTF_INDEX_OPTION_VVX_7_2; 
		break;
	}
	
	// set codec type
	switch(codecType)
	{
	case RTF_VIDEO_CODEC_TYPE_H264:
		_vcdType = RTF_VIDEO_CODEC_TYPE_H264;
		break;
	case RTF_VIDEO_CODEC_TYPE_MPEG2:
	default:
		_vcdType = RTF_VIDEO_CODEC_TYPE_MPEG2;
		break;
	}
}

void RTFLibFilter::initOutputFileInfo()
{
	int outputIndex = 0;

	//
	// Attention: only set output file info for the trick file and index, without main file
	//            to the main file, if do this way, the output main file size by RTFLib is smaller than original one
	//            so the main file is output to file in releaseInputBuffer

	// main file
	_outputInfo[outputIndex].fileNo = outputIndex;
	_outputInfo[outputIndex].fileType = OPTFT_MAIN;
	_outputInfo[outputIndex].speedNo = 0;
	_outputInfo[outputIndex].ioRender = NULL;
	strcpy(_outputInfo[outputIndex].extension, "");

	_outputInfo[outputIndex].speedDirection = 1;
	_outputInfo[outputIndex].speedNumerator = 1;
	_outputInfo[outputIndex].speedDenominator = 1;

	outputIndex++;


	// index file
	if(RTF_INDEX_TYPE_VV2 == _indexType)
	{
		_outputInfo[outputIndex].fileNo = outputIndex;
		_outputInfo[outputIndex].fileType = OPTFT_VV2;
		_outputInfo[outputIndex].speedNo = 0;
		_outputInfo[outputIndex].ioRender = NULL;
		strcpy(_outputInfo[outputIndex].extension, ".vv2");

		_outputInfo[outputIndex].speedDirection = 0;
		_outputInfo[outputIndex].speedNumerator = 0;
		_outputInfo[outputIndex].speedDenominator = 1;
	}
	else
	{
		_outputInfo[outputIndex].fileNo = outputIndex;
		_outputInfo[outputIndex].fileType = OPTFT_VVX;
		_outputInfo[outputIndex].speedNo = 0;
		_outputInfo[outputIndex].ioRender = NULL;
		strcpy(_outputInfo[outputIndex].extension, ".vvx");

		_outputInfo[outputIndex].speedDirection = 0;
		_outputInfo[outputIndex].speedNumerator = 0;
		_outputInfo[outputIndex].speedDenominator = 1;
	}
	outputIndex++;
	
	// set the trick file output setting
	for(int i=0; i<_numTrickSpeed; i++)
	{
		if(_unifiedTrickFile)
		{
			char ext[16];
			getTrickExt(i, ext, NULL);

			// set FFR
			_outputInfo[outputIndex].fileNo = outputIndex;
			_outputInfo[outputIndex].fileType = OPTFT_FFR;
			_outputInfo[outputIndex].speedNo = i;
			_outputInfo[outputIndex].ioRender = NULL;
			strcpy(_outputInfo[outputIndex].extension, ext);

			_outputInfo[outputIndex].speedDirection = 1;
			_outputInfo[outputIndex].speedNumerator = trickSpeedNumerator[i];
			_outputInfo[outputIndex].speedDenominator = trickSpeedDenominator[i];

			outputIndex++;
		}
		else
		{
			char extFF[16];
			char extFR[16];

			getTrickExt(i, extFF, extFR);

			// set FF
			_outputInfo[outputIndex].fileNo = outputIndex;
			_outputInfo[outputIndex].fileType = OPTFT_FF;
			_outputInfo[outputIndex].speedNo = i;
			_outputInfo[outputIndex].ioRender = NULL;
			strcpy(_outputInfo[outputIndex].extension, extFF);

			_outputInfo[outputIndex].speedDirection = 1;
			_outputInfo[outputIndex].speedNumerator = trickSpeedNumerator[i];
			_outputInfo[outputIndex].speedDenominator = trickSpeedDenominator[i];

			outputIndex++;

			// set FR
			_outputInfo[outputIndex].fileNo = outputIndex;
			_outputInfo[outputIndex].fileType = OPTFT_FR;
			_outputInfo[outputIndex].speedNo = i;
			_outputInfo[outputIndex].ioRender = NULL;
			strcpy(_outputInfo[outputIndex].extension, extFR);

			_outputInfo[outputIndex].speedDirection = -1;
			_outputInfo[outputIndex].speedNumerator = trickSpeedNumerator[i];
			_outputInfo[outputIndex].speedDenominator = trickSpeedDenominator[i];

			outputIndex++;
		}
	}
	// set total file count
	_outputFileCount = outputIndex;	
}

bool RTFLibFilter::initRTFLib(DWORD maxSession, ZQ::common::Log* pLog, DWORD inputBufferSize, DWORD outputBufferSize, bool trickKeyFrame, bool rtfWarningTolerance)
{
	unsigned long poolBytes;

	_rtfLogger = pLog;
	_trickKeyFrame = trickKeyFrame;

	// get version of RTFLib
	rtfGetVersionString(_rtfVersion);
	if(_rtfLogger != NULL) (*_rtfLogger)(ZQ::common::Log::L_DEBUG, "RTFLibFilter::initRTFLib() - Initializing trickFile library, version %s", _rtfVersion);
	
	DWORD maxInputBuffersPerSession;
	if(inputBufferSize < INPUT_FILE_BUFFER_BYTES_64)
	{   // less than 64K
		maxInputBuffersPerSession = INPUT_BUFFERS_16K_PER_SESSION;
	}
	else if(inputBufferSize > INPUT_FILE_BUFFER_BYTES_64)
	{   // more than 64K
		maxInputBuffersPerSession = INPUT_BUFFERS_128K_PER_SESSION;
	}
	else
	{	// 64K 
		maxInputBuffersPerSession = INPUT_BUFFERS_64K_PER_SESSION;
	}
	// 
	poolBytes = rtfGetLibraryStorageRequirement(maxSession,
                                                maxInputBuffersPerSession, 
												MAX_GROUPS_PER_SEQUENCE,
		                                        MAX_PICTURES_PER_GROUP,
												inputBufferSize);
	if(_rtfLogger != NULL) (*_rtfLogger)(ZQ::common::Log::L_DEBUG, "RTFLibFilter::initRTFLib() - rtfGetLibraryStorageRequirement() returned %d bytes", poolBytes);
	
	if(!rtfInitializeLibrary(NULL,
		                      appAlloc, 
							  appFree, 
							  appLog,
                              maxSession, 
							  maxInputBuffersPerSession,
							  MAX_GROUPS_PER_SEQUENCE, 
							  MAX_PICTURES_PER_GROUP,
							  inputBufferSize, 
							  FAILED_SESSION_THRESHOLD,
							  rtfLibErrorLog) ) 
	{
		if(_rtfLogger != NULL) (*_rtfLogger)(ZQ::common::Log::L_ERROR, "RTFLibFilter::initRTFLib() - rtfInitializeLibrary() failed");
		
		return false;
	}

	if(rtfWarningTolerance)
		_rtfTolerance = RTF_WARNING_TOLERANCE_RELAXED;
	else
		_rtfTolerance = RTF_WARNING_TOLERANCE_STRICT;

	if(_rtfLogger != NULL) (*_rtfLogger)(ZQ::common::Log::L_DEBUG, "RTFLibFilter::initRTFLib() - RTF trickFile library initialization completed");

	_rtfInited = true;

	return true;
}

void RTFLibFilter::uninitRTFLib()
{
	if(!_rtfInited)
		return;

	if(_rtfLogger != NULL) (*_rtfLogger)(ZQ::common::Log::L_DEBUG, "RTFLibFilter::uninitRTFLib() - shutting down FTF trick library");
	try
	{
		if(!rtfShutdownLibrary())
		{
			if(_rtfLogger != NULL) (*_rtfLogger)(ZQ::common::Log::L_DEBUG, "RTFLibFilter::uninitRTFLib() - rtfShutdownLibrary() failed");
		}
	}
	catch(...)
	{
		if(_rtfLogger != NULL) (*_rtfLogger)(ZQ::common::Log::L_DEBUG, "RTFLibFilter::uninitRTFLib() - rtfShutdownLibrary() failed with unknown exception");
	}

	if(_rtfLogger != NULL) (*_rtfLogger)(ZQ::common::Log::L_DEBUG, "RTFLibFilter::uninitRTFLib() - FTF trick library was shut down");	
}

bool RTFLibFilter::initOutputSetting(int outputFileIndex, RTF_APP_OUTPUT_SETTINGS* setting)
{	
	OutputFileType fileType = _outputInfo[outputFileIndex].fileType;
	
	int speedDirection = _outputInfo[outputFileIndex].speedDirection;
	int speedNumerator = _outputInfo[outputFileIndex].speedNumerator;
	int speedDenominator = _outputInfo[outputFileIndex].speedDenominator;

	char ext[16];
	sprintf(ext, _outputInfo[outputFileIndex].extension);

	if (!rtfInitializeOutputSettings(setting, 
	                                 _vcdType, 
									 _indexType, 
									 (RTF_APP_FILE_HANDLE)(&_outputInfo[outputFileIndex]), 
									 speedDirection, 
									 speedNumerator, 
									 speedDenominator, 
									 ext, 
									 trickFileGetOutputBuffer, 
									 trickFilePutOutputBuffer, 
									 trickFileCloseOutput)) 
	{

		return false;
	}

	if(fileType != OPTFT_MAIN && fileType != OPTFT_VVX && fileType != OPTFT_VV2) 
	{
		setting->trickSpec.restampPCR = TRUE;
		setting->trickSpec.restampPTSDTS = TRUE;
	}

	return true;
}

bool RTFLibFilter::openSession(std::string& errMsg)
{
	bool bRet = false;
	RTF_APP_OUTPUT_SETTINGS settings[RTF_MAX_OUTPUT_FILE_COUNT];
	memset(settings, 0x0, RTF_MAX_OUTPUT_FILE_COUNT * sizeof(RTF_APP_OUTPUT_SETTINGS));

	int settingIndex = 0;
	// set the trick file output setting
	// Attention: The index of i from 1, does not include main file, since RTFLib output main file
	//            size may be NOT same as source content. 
	//            The source content buffer comes from receive() 
	for(int i=1; i<_outputFileCount; i++)
	{
		// set FFR file output setting
		bRet = initOutputSetting(i, settings + settingIndex);
		if(!bRet)
		{
			errMsg = "RTFLib initialize output setting failed";
			return false;
		}
		settingIndex++;
	}

	// initialize a set of warning thresholds for this session
	rtfInitializeWarningThresholds(_rtfTolerance,
									&_warningThresholds);

	bRet = rtfOpenSession(&_sessionHandle, 
							  this, 
							  NULL,
							  "",
							  0,
							  &_warningThresholds, 
							  _indexMode, 
							  _indexType, 
							  _indexOption, 
							  sessionErrorNotifier, 
							  releaseInputBuffer, 
							  _outputFileCount-1,  // RTFLib does not output main file, since the output size may not be same as source
							  settings);

	if(!bRet)
	{
		errMsg = "RTFLib open session failed";
	}
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter: rtfOpenSession() succeed with returned session handle 0x%08X", _sessionHandle);
	return bRet;
}

bool RTFLibFilter::closeSession()
{
	bool bRet = true;
	if(_sessionHandle) 
	{
		try
		{
			bRet = rtfCloseSession(_sessionHandle);
			
			if(bRet)
			{
				_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter: rtfCloseSession() succeed for handle 0x%08X", _sessionHandle);
			}
		}
		catch(...)
		{
			_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "RTFLibFilter: rtfCloseSession() met unknown exception");
		}

		_sessionHandle = NULL;
	}

	return bRet;
}

bool RTFLibFilter::processBuffer(ZQ::Content::BufferData* dataBuf, std::string& errmsg)
{
	if(_sessClosed)
	{
		// this is the case that RTFLib's all sub files are closed, just ignore this without further processing
		_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "RTFLib output files are all closed, do not invoke rtfProcessInput() to the left bytes");
		return true;
	}

	unsigned char* buf;

	unsigned long bytes = _rtfInputBuffSize;
	buf = dataBuf->getPointerForRead(bytes);

	_graph.traceLog(id(), "RTFLib Process BufferData 0x%08X, buffer size is %d", dataBuf, bytes);

	BOOL result = rtfProcessInput(_sessionHandle, 
	                             this, 
								 NULL, 
								 dataBuf, 
								 buf, 
								 (unsigned long)bytes);

	if(!result)
	{
		errmsg = "rtfProcessInput() failed";
		return false;
	}

	return result;
}

int RTFLibFilter::releaseInputBuffer(void *hAppSession, void *hAppFile, void *hAppBuffer, unsigned char *pBuffer)
{
	RTFLibFilter* This = (RTFLibFilter*)hAppSession;
	ZQ::Content::BufferData* pBuffData = (ZQ::Content::BufferData*)hAppBuffer;

	Graph& graph = This->getGraph();
	graph.traceLog(GetCurrentThreadId(), "(RTFLib %s) releaseInputBufferProcess() callback entering for releasing BuffData 0X%08X", _rtfVersion, pBuffData);

	return This->releaseInputBufferProcess(pBuffData);
}

int RTFLibFilter::trickFileCloseOutput( void *hAppSession, void *hAppFile )
{
	RTFLibFilter* This = (RTFLibFilter*)hAppSession;
	POutputFileInfo pOutputFile = (POutputFileInfo)hAppFile;

	Graph& graph = This->getGraph();
	graph.traceLog(GetCurrentThreadId(), "(RTFLib %s) trickFileCloseOutput() callback entering for No.%d %s file", _rtfVersion, pOutputFile->fileNo, pOutputFile->extension);

	return This->trickFileCloseOutputProcess(pOutputFile->fileNo);
}

int RTFLibFilter::trickFilePutOutputBuffer(void *hAppSession, void *hAppFile, void *hAppBuffer,unsigned char *pBuffer, unsigned long occupancy, RTF_BUFSEEK bufSeek, INT64 bufSeekOffset)
{
	RTFLibFilter* This = (RTFLibFilter*)hAppSession;
	POutputFileInfo pOutputFile = (POutputFileInfo)hAppFile;

	ZQ::Content::BufferData* pBuffData = (ZQ::Content::BufferData*)hAppBuffer;

	Graph& graph = This->getGraph();
	graph.traceLog(GetCurrentThreadId(), "(RTFLib %s) trickFilePutOutputBuffer() callback entering for No.%d %s file, for BuffData: 0X%08X, Len: %d", 
		_rtfVersion, pOutputFile->fileNo, pOutputFile->extension, pBuffData, occupancy);

	return This->trickFilePutOutputBufferProcess(pOutputFile->fileNo, pBuffData, occupancy, bufSeek, bufSeekOffset);
}

int RTFLibFilter::trickFileGetOutputBuffer(void *hAppSession, void *hAppFile, void **phAppBuffer,unsigned char **ppBuffer, unsigned long *pCapacity)
{
	RTFLibFilter* This = (RTFLibFilter*)hAppSession;
	POutputFileInfo pOutputFile = (POutputFileInfo)hAppFile;

	ZQ::Content::BufferData* pBuffData = NULL;

	This->trickFileGetOutputBufferProcess(pOutputFile->fileNo, &pBuffData);

	*phAppBuffer = pBuffData;
	int len = pBuffData->getTotalSize(); 

	*pCapacity = _rtfOutputBuffSize;

	*ppBuffer = pBuffData->getPointerForWrite(*pCapacity);

	Graph& graph = This->getGraph();
	graph.traceLog(GetCurrentThreadId(), "(RTFLib %s) trickFileGetOutputBuffer() callback entering for No.%d %s file, the BuffData is 0X%08X", 
					_rtfVersion, pOutputFile->fileNo, pOutputFile->extension, pBuffData);

	return 0;
}

int RTFLibFilter::releaseInputBufferProcess(ZQ::Content::BufferData* pBuffData)
{
	if(_rtfReleaseBuffCheck)
	{
		_rtfBeginReleaseBuffer = true;
	}
	bool bReleased = releaseBuffer(pBuffData, this);
	if(bReleased)
	{
		_graph.traceLog(id(), "RTFLibFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
						pBuffData);
	}

	return 0;
}

int RTFLibFilter::trickFileCloseOutputProcess(int fileNo)
{
	// if the session is aborted by cloaseSession, this function will be triggered for each of RTFLib session output file, 
	// to abortion case, this FileClose should not trigger "end of stream" event to connected render
	if(ABORTED == _processStatus)
	{
		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter: trickFileCloseOutput() for %s%s, in case of status is ABORTED, ingore it",
			_graph.getContentName().c_str(), _outputInfo[fileNo].extension);

		return 0;
	}
	
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter: trickFileCloseOutput() - %s%s output close by RTFLib, trigger end of stream", 
	                                     _graph.getContentName().c_str(), _outputInfo[fileNo].extension);

	// judge whether all of out put file are closed
	_eosSubFileCount++;

	// if all RTFLib output file closed, but did not got endOfStream event, something must be wrong
	// _outputFileCount include main file, but main file is not output from RTFLib
	if(_eosSubFileCount == _outputFileCount-1 && !_bEndOfStream)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "RTFLibFilter: trickFileCloseOutput() - all RTFLib close all output file, but RTFLibFilter did not get end of stream event from source, something must be wrong, abort provision");

		_sessClosed = true;

		_graph.abortProvision();
	}

	return 0;
}

int RTFLibFilter::trickFilePutOutputBufferProcess(int fileNo, ZQ::Content::BufferData* pBuffData, unsigned long occupancy, RTF_BUFSEEK bufSeek, INT64 bufSeekOffset)
{
	// RTFLib logic is when a new session is open, it will get output buffer right now, even there is no data input yet.
	// So if the provision aborted for other reason, means close the session without input any buffer to RTFLib,
	// RTFLib will release the acquired buffer when openSession happen, then this function will be invoked, 
	// and the occupancy size will be 0. To this buffer, need to release directly without pass to next render.
	if(0 == occupancy)
	{
		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "RTFLibFilter: RTFLib trickFile put outputBuffer size is 0");

		// release the buffer
		bool bReleased = releaseBuffer(pBuffData, this);
		if(bReleased)
		{
			_graph.traceLog(id(), "RTFLibFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
							pBuffData);
		}

		return 0;
	}

	switch( bufSeek )
	{
	case RTF_BUFSEEK_NONE:
		break;
	case RTF_BUFSEEK_SET:
		pBuffData->setProperty(ZQ::Content::Process::CNTPRY_IO_SEEK_ORIGION, 
		                  (unsigned long)FILE_BEGIN);
		pBuffData->setProperty(ZQ::Content::Process::CNTPRY_IO_SEEK_OFFSET, 
		                  (long)bufSeekOffset);
		
		break;
	case RTF_BUFSEEK_CUR:
		pBuffData->setProperty(ZQ::Content::Process::CNTPRY_IO_SEEK_ORIGION, 
		                  (unsigned long)FILE_CURRENT);		
		pBuffData->setProperty(ZQ::Content::Process::CNTPRY_IO_SEEK_OFFSET, 
		                  (long)bufSeekOffset);
		
		break;
	case RTF_BUFSEEK_END:
		pBuffData->setProperty(ZQ::Content::Process::CNTPRY_IO_SEEK_ORIGION, 
		                  (unsigned long)FILE_END);		
		pBuffData->setProperty(ZQ::Content::Process::CNTPRY_IO_SEEK_OFFSET, 
		                  (long)bufSeekOffset);

		break;
	default:
		return -1;
	}

	pBuffData->setActualLength(occupancy);

	// set the fileNo in the buffer property
	if(_connectedIORenderCount)
	{
		pBuffData->setProperty(ZQ::Content::Process::CNTPRY_SUBFILE_NO, (DWORD)fileNo);
	}

	// pass the buffer to the connected IO render
	_outputInfo[fileNo].ioRender->receive(this, pBuffData);

	// release the buffer
	bool bReleased = releaseBuffer(pBuffData);
	if(bReleased)
	{
		_graph.traceLog(id(), "RTFLibFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
						pBuffData);
	}

	return 0;
}

int RTFLibFilter::trickFileGetOutputBufferProcess(int fileNo, ZQ::Content::BufferData** pBuffData)
{	
	// allocate the buffer
	while(1)
	{
		*pBuffData = _pool.alloc(DEFAULT_BUFF_WAIT_TIMEOUT);
		if(NULL == (*pBuffData) )
		{				
			_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "RTFLibFilter: trickFileGetOutputBuffer() - No avaliable buffer in pool, waiting for %d ms", DEFAULT_BUFF_WAIT_TIMEOUT);
			continue;
		}
		break;
	}

	return 0;
}



} } }