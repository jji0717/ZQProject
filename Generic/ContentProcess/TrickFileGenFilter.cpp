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
// $Header: /ZQProjs/Generic/ContentProcess/TrickFileGenFilter.cpp 1     10-11-12 15:58 Admin $
// $Log: /ZQProjs/Generic/ContentProcess/TrickFileGenFilter.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 20    08-03-24 17:08 Ken.qian
// 
// 19    08-03-06 17:00 Ken.qian
// 
// 18    08-02-25 17:40 Ken.qian
// notify endofstream to connected filter only once if
// RTFLibFilter/TrickFileGenFilter output to one filter(support subfile)
// 
// 17    08-02-20 10:44 Build
// change codes to pass Vs2005 compile under x64
// 
// 16    07-12-17 14:44 Ken.qian
// add :: at the front of _vsnprintf to avoid compiling error:
// '_vsnprintf' : ambiguous call to overloaded
// 
// 15    07-12-14 14:10 Ken.qian
// use _vsnprintf replace vsprintf
// 
// 14    07-12-12 15:28 Ken.qian
// fix the issue of opensession fail should not return false, it should be
// continue
// 
// 13    07-12-04 10:36 Ken.qian
// remove two useless variables
// 
// 12    07-11-23 15:11 Ken.qian
// add addSplicePoint() function
// 
// 11    07-11-16 19:15 Ken.qian
// seperate the time of update bitrate,framecode, etc property with
// playtime
// 
// 10    07-11-15 18:56 Ken.qian
// change getOutputFileInfo() parameter
// 
// 9     07-11-13 17:21 Ken.qian
// enable PWE vvx flush
// 
// 8     07-10-31 14:26 Ken.qian
// add closeSession() in thread exiting
// 
// 7     07-10-29 11:08 Ken.qian
// 
// 6     07-10-23 21:24 Ken.qian
// 
// 5     07-10-23 18:30 Ken.qian
// 
// 4     07-09-27 12:39 Ken.qian
// fix streaming fail with invalid file version error while using
// TrickFileUserLibrary, which caused by unproper position of call
// TrickFilesReleaseSourceMpegBuffers()
// 
// 3     07-09-20 17:59 Ken.qian
// add log while processing buffer
// 
// 2     07-09-17 15:27 Ken.qian
// replace getUnallocatedCount()  with getUsedCount()
// 
// 1     07-09-11 15:01 Ken.qian
// Initial checkin
// 
// 2     07-09-09 21:23 Ken.qian
// 1. add protection for buffer dead-lock since input and output are
// sharing same buffer pool
// 2. fix splicing failure at back by set dsp's c++ configuraiton 's Debug
// Info from "Program Database" to "Program Database for Edit and
// Continue" 
// 
// 1     07-09-07 14:00 Ken.qian
// initial codes, but splicing had issue
#ifndef NOT_SUPPORT_OLD_TRICKLIB

#include "TrickFileGenFilter.h"
#include "urlstr.h"
#include "bufferpool.h"
#include "mpeg.h"

#pragma comment (lib, "TrickFilesLibraryUser.lib")
#pragma comment (lib, "MpegLibraryUser.lib")


namespace ZQ { 
namespace Content { 
namespace Process {

#define DEF_MAX_QUEUE_DEPTH     100

static unsigned long trickSpeedNumerator[TRICKGEN_MAX_TRICKSPEED_NUMBER] = { 30, 30, 30 };
static unsigned long trickSpeedDenominator[TRICKGEN_MAX_TRICKSPEED_NUMBER] = { 4,  2,  1 };

static char* SPLICE_STATUS[] = { "Initial", "Queued", "Rejected", "Ignored", "InsertSuccess", "InsertFailed" };
static char* SPLICE_TYPE[] = { "Invalid", "Front", "Back", "CueIn", "CueOut" };

TrickFileGenFilter::TrickFileGenFilter(ZQ::Content::Process::Graph& graph, bool outputToOneIORender, 
						   int numTrickSpeed, DWORD trickBufferSize, DWORD maxMpegCodingError, bool enableSplicing, std::string myName)
: Filter(graph, myName), _hDesFile(INVALID_HANDLE_VALUE), _processedBytes(0), _dwCounter(0),
_endOfStream(true), _outputToOneIORender(outputToOneIORender), _maxMpegCodingError(maxMpegCodingError), 
_enableSplicing(enableSplicing), _dwTrickBufferSize(trickBufferSize)
{
	// set trick speeds
	_numTrickSpeed = numTrickSpeed;
	if(_numTrickSpeed <= 0)
	{
		_numTrickSpeed = 1;
	}
	else if(_numTrickSpeed > TRICKGEN_MAX_TRICKSPEED_NUMBER)
	{
		_numTrickSpeed = TRICKGEN_MAX_TRICKSPEED_NUMBER;
	}

	// trick file API related
	_mallocRoutine = malloc;
	_freeRoutine	= free;

	_dwPictureCount = 0;
	_vvxFlushNeeded = 0;
	_flushCount = 0;
	_trickContext = NULL;

	//
	// Initialize output file information
	//
	_connectedIORenderCount = 0;
	initOutputFileInfo();

	memset(&_trickCharacteristics, 0, sizeof(_trickCharacteristics));

	//
	// filter related 
	//
	if(myName == "")
	{
		_myName = "TrickFileGenFilter";
	}

	_tidAbortBy = 0;
	
	_hStop = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);

	// start the thread
	start();
}

TrickFileGenFilter::~TrickFileGenFilter(void)
{
	// free mpeg buffer pool
	freeMpegBufferPoolMemory();

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

void TrickFileGenFilter::emptyDataQueue(void)
{
	ZQ::common::MutexGuard guard(_dataMutex);
	// remove all the buffer data pointer from the queue.
	while (!_dataQueue.empty())
	{
		ZQ::Content::BufferData* pBuffData = _dataQueue.front();
		
		_pool.free(pBuffData);
		_dataQueue.pop();

		_graph.traceLog(id(), "TrickFileGenFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
					    pBuffData);
	}	
}

bool TrickFileGenFilter::receive(Filter* upObj, ZQ::Content::BufferData* buff)
{	
	ZQ::Content::BufferData* receivedBuff = NULL;
	if(_copyUplinkDataBuff)
	{	// copy BuffData
		ZQ::Content::BufferPool& buffpool = _graph.getBuffPool();
		_graph.traceLog(id(), "BufferPool: usage: %d / %d [used/total]", 
				  buffpool.getUsedCount(), buffpool.getPoolSize());
		
		// copy the buffer to the new one
		receivedBuff = _pool.alloc();
		*receivedBuff = *buff;

		_graph.traceLog(id(), "TrickFileGenFilter: alloc buffData from pool. [BuffData Address: 0x%08X]", 
						receivedBuff);
	}
	else
	{   // does not copy BuffData
		receivedBuff = buff;
	}

	if(STOPPED == _processStatus || ABORTED == _processStatus)
	{
		_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "TrickFileGenFilter: it is in STOPPED or ABORTED status, does not receive data any more");
		bool bReleased = releaseBuffer(receivedBuff, this);	
		if(bReleased)
		{
			_graph.traceLog(id(), "TrickFileGenFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
							receivedBuff);
		}

		return false;
	}

	if(_endOfStream)
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "TrickFileGenFilter: it is end of stream, does not receive data more");

		bool bReleased = releaseBuffer(receivedBuff, this);	
		if(bReleased)
		{
			_graph.traceLog(id(), "TrickFileGenFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
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
	
	_graph.traceLog(GetCurrentThreadId(), "TrickFileGenFilter: Receive Buffer Data from up side process object with actual length %d", buff->getActualLength());

	SetEvent(_hNotify);

	return true;
}

bool TrickFileGenFilter::begin(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "TrickFileGenFilter::begin() enter");
	
	// To avoid invoking begin() during its processing
	if(_processStatus == ACTIVE)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "TrickFileGenFilter: the task did not complete yet, can not initial new work");

		return false;
	}

	//
	// check the begin is invoked by pause - have begun or just begin
	//
	if(_processStatus != PAUSED)    // begin a new job
	{	
		if(_dwTrickBufferSize % MPEG_TS_PACKET_SIZE != 0)
		{
			_graph.writeLog(ZQ::common::Log::L_WARNING, id(), "TrickFileGenFilter: Invalid Trick Buffer Size");
			_graph.setLastError(ERRCODE_TRICKGEN_CREATECONTEXT, "Invalid Trick Buffer Size");

			return false;
		}

		_processedBytes = 0;
		_dwCounter = 0;

		_tidAbortBy = 0;

		_trickContext = NULL;
		// compose the file full name
		_szDesFileName = _graph.getContentName() ;
		
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "TrickFileGenFilter: The render's destination file is %s", _szDesFileName.c_str());
		
		_endOfStream = false;

		// make sure to empty the queue
		emptyDataQueue();
	}
	else
	{
		// resume the native thread
		start();	
	}

	// resume the thread
	_processStatus = ACTIVE;
		
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "TrickFileGenFilter::begin() leave");

	return true;
}

bool TrickFileGenFilter::pause(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "TrickFileGenFilter::pause() enter");

	_processStatus = PAUSED;

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "TrickFileGenFilter::pause() leave");
	
	// suspend the native thread
	suspend();
		
	return true;
}

bool TrickFileGenFilter::abort(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "TrickFileGenFilter::abort() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();
	
	// set the status
	_processStatus = ABORTED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "TrickFileGenFilter::abort() leave");
	
	return true;
}

void TrickFileGenFilter::stop(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "TrickFileGenFilter::stop() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = STOPPED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "TrickFileGenFilter::stop() leave");
}

void TrickFileGenFilter::quit(void)
{
	SetEvent(_hStop);
}

void TrickFileGenFilter::endOfStream(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "TrickFileGenFilter: Get end of stream notification");

	_endOfStream = true;

	SetEvent(_hNotify);
}

int TrickFileGenFilter::run(void)
{	
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter::run() enter");

	bool bContinue = true;
	DWORD dwWaitStatus = 0;

	HANDLE handles[2] = { _hStop, _hNotify };

	while(bContinue)
	{
		dwWaitStatus = WaitForMultipleObjects(2, handles, false, INFINITE);
		switch(dwWaitStatus)
		{
		// received stop event, the thread will stop
		case WAIT_OBJECT_0:
			if(_trickContext != NULL)
			{   // close session while program stop if there is 
				closeSession();
			}
			
			bContinue = false;
			_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: get a thread exit event");
			break;

		// received the Notify event
		case WAIT_OBJECT_0 + 1:
		{	
			// check whether this thread is abort by Graph
			if(STOPPED == _processStatus || ABORTED == _processStatus)
			{
				// make sure to remove, 
				emptyDataQueue();

				// import complete with success status
				importDone(false);
				
				closeSession();

				if(ABORTED == _processStatus)
				{
					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: It was aborted by Graph, triggered by thread 0x%08X", _tidAbortBy);
				}
				
				// notify graph this filter processing completed
				_graph.notifyCompletion(*this);

				continue;
			}

			// create session if not, codes in  run() make sure that process after data ready
			std::string errmsg;
			if(NULL == _trickContext)
			{
				if(!openSession(errmsg))
				{
					_graph.writeLog(ZQ::common::Log::L_WARNING, id(), "TrickFileGenFilter: openSession() failed with reason: %s", errmsg.c_str());
					_graph.setLastError(ERRCODE_TRICKGEN_CREATECONTEXT, errmsg.c_str());

					// set the status, must be before EmptyDataQueue() invoking
					_processStatus = ABORTED;
					_endOfStream = false;
					
					// make sure to remove, 
					emptyDataQueue();
					
					_graph.abortProvision();

					continue;
				}
				else
				{
					// update properties at started
					updateFilesetProperties();

					// update the content properties, since after openSession, the bitrate,etc are ready
					updateContentProperties();
				}
			}
			
			bool bProcessNext = true;
			bool bRet = true;
			try
			{
//				do
//				{
					bRet = import(bProcessNext, errmsg);

// 				}while(bRet && bProcessNext);
			}
			catch(...)
			{
				errmsg = "TrickFile Generation Library met unknown exception";
				_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "TrickFileGenFilter: import() catch unknow exception");
				bRet = false;
			}
			
			if(!bRet)
			{	
				// set graph last error
				_graph.setLastError(ERRCODE_RTFLIB_PROCESSBUFF_FAIL, errmsg);
				
				_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "TrickFileGenFilter: %s", errmsg.c_str());

				// set the status, must be before EmptyDataQueue() invoking
				_processStatus = ABORTED;
				_endOfStream = false;
								
				// make sure to remove, 
				emptyDataQueue();

				// the render met problem, abort all the renders in the graph
				_graph.abortProvision();

//				// import complete with failure status
//				importDone(false);

				continue;				
			};

			if(_endOfStream && !bProcessNext)
			{
				// import complete with success status
				importDone(true);
				
				// update the properties
				updateContentPlayTime();
			
				// make sure to remove, 
				emptyDataQueue();

				//
				// reach the end of source file, close RTFLib session
				//
				closeSession();

				// end of stream
				_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: reaches the end of stream, there are %d buffer processed in all, total output size is %I64d bytes", 
								_dwCounter, _processedBytes);

				_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: reaches the end of stream, notify IO render the event of end of stream");
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
				
				// set the status
				_processStatus = STOPPED;
				
				// notify graph this filter processing completed
				_graph.notifyCompletion(*this);

				continue;
			}

			// go on next loop
			ZQ::common::MutexGuard guard(_dataMutex);
			if(!_dataQueue.empty())
			{
				SetEvent(_hNotify);
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
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter::run() leave");
	
	return 1;
}

bool TrickFileGenFilter::connectTo(Filter* obj, bool copyBuff)
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


bool TrickFileGenFilter::getOutputFileInfo(int fileNo, char* ext, int len, DWORD& numerator, DWORD& denominator, OutputFileType& outputFileType)
{
	if(fileNo<0 || fileNo>_outputFileCount)
	{
		return false;
	}

	outputFileType = _outputInfo[fileNo].fileType; 

	numerator = _outputInfo[fileNo].speedNumerator;
	denominator = _outputInfo[fileNo].speedDenominator;

	strncpy(ext, _outputInfo[fileNo].extension, len);
	
	return true;
}

double TrickFileGenFilter::getFrameRateByCode(WORD framecode)
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

void TrickFileGenFilter::updateFilesetProperties()
{
	if (_trickContext)
	{
		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: update property at provision started...");
		
		ContentProperty streamProperites;
		
		// set subtype 
		std::string subtype = "VVX"; 
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
		for(int i=0; i<_outputFileCount; i++)
		{
			sprintf(pryFileName, "%s%d", CNTPRY_FILE_NAME.c_str(), i+1); // index from 0 
			sprintf(valFileName, "%s%s", cntname.c_str(), _outputInfo[i].extension);
			streamProperites.insert(ContentProperty::value_type(pryFileName, valFileName));
		}
		
		_graph.reportProperty(_graph.getContentName(), streamProperites);
	}
}

void TrickFileGenFilter::updateContentProperties()
{
	if (_trickContext != NULL)
	{		
		ContentProperty streamProperites;

		// update video property
		_contentBitrate = _trickCharacteristics.bitRate;
		streamProperites.insert(ContentProperty::value_type(CNTPRY_BITRATE, _contentBitrate));
		streamProperites.insert(ContentProperty::value_type(CNTPRY_VIDEO_BITRATE, (DWORD)_trickCharacteristics.videoBitRate));
		streamProperites.insert(ContentProperty::value_type(CNTPRY_VIDEO_VERTICAL, (DWORD)_trickCharacteristics.verticalSize));
		streamProperites.insert(ContentProperty::value_type(CNTPRY_VIDEO_HORIZONTAL, (DWORD)_trickCharacteristics.horizontalSize));
		streamProperites.insert(ContentProperty::value_type(CNTPRY_VIDEO_FRAMERATE, (double)getFrameRateByCode((WORD)_trickCharacteristics.frameRateCode)));

		// deliver the bitrate to connected filter, since the connected IO need to use this.
		// this added because VstrmIO in case of pacing enable, it need to fire "streamable" event,
		deliverContentProperty(CNTPRY_VIDEO_VERTICAL, (DWORD)_trickCharacteristics.verticalSize);
		deliverContentProperty(CNTPRY_VIDEO_HORIZONTAL, (DWORD)_trickCharacteristics.horizontalSize);
		deliverContentProperty(CNTPRY_BITRATE, (DWORD)_contentBitrate);

/*	    Move the playtime property update to function updateContentPlayTime()
		
		__int64 playtime = 0;
		if (_contentBitrate > 0) 
		{
			playtime = (_graph.getContentSize() * 8 * 1000) / ((__int64) _contentBitrate);  // ms
			streamProperites.insert(ContentProperty::value_type(CNTPRY_VIDEO_PLAYTIME, playtime));
		}
*/		
		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: content bitrate is %d bps", _contentBitrate);

		_graph.reportProperty(_graph.getContentName(), streamProperites);
	}
}

void TrickFileGenFilter::updateContentPlayTime()
{
	if (_trickContext !=NULL && _contentBitrate > 0)
	{		
		ContentProperty streamProperites;
		
		__int64 playtime = (_graph.getContentSize() * 8 * 1000) / ((__int64) _contentBitrate);  // ms
		streamProperites.insert(ContentProperty::value_type(CNTPRY_VIDEO_PLAYTIME, playtime));

		_graph.reportProperty(_graph.getContentName(), streamProperites);

		_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "TrickFileGenFilter: content playtime is %I64d ms", playtime);
	}
}


//////////////////////////////////////////////////////////////////////////
//                  codes for TrickFile-Gen wrapper                     //
//////////////////////////////////////////////////////////////////////////
PVOID TrickFileGenFilter::memoryAllocate(PVOID context, ULONG size)
{
	if (context)
	{
		TrickFileGenFilter	*This = (TrickFileGenFilter *)context;
		return This->_mallocRoutine(size);
	}

	return malloc(size);
}

void TrickFileGenFilter::memoryFree(PVOID context, PVOID buffer)
{
	if (context)
	{
		TrickFileGenFilter	*This = (TrickFileGenFilter *)context;
		This->_freeRoutine(buffer);
	}
	else
	{
		free(buffer);
	}
}

void TrickFileGenFilter::logMsgFromTrickLibrary(PVOID context, const char *format, ...)
{
	ZQ::common::Log::loglevel_t loglevel = ZQ::common::Log::L_INFO;

	bool bAbortion = false;
	
	TrickFileGenFilter	*This = (TrickFileGenFilter *)context;

	if (!This)
		return;

	//
	// skip info messages if not enabled
	//
	switch(*format)
	{
	case 'E':
		bAbortion = true;
		loglevel = ZQ::common::Log::L_ERROR;
		break;
	case 'W':
		loglevel = ZQ::common::Log::L_ERROR;
		break;
	case 'I':
		loglevel = ZQ::common::Log::L_INFO;
		break;
	default:			// display by default
		break;

	}

    va_list    marker;
    char	   szMsg[1024];

    // Initialize access to variable arglist
    va_start(marker, format);

    // Expand message
    ::_vsnprintf(szMsg, 1023 * sizeof(char), format, marker);
    szMsg[1024] = 0;

	//delete the last \r\n from the string
	{
		char* pPtr = szMsg;
		while(*pPtr)pPtr++;

		pPtr--;

		while(pPtr >=szMsg)
		{
			if (*pPtr == '\n' ||*pPtr == '\r')
				pPtr--;
			else
				break;
		}

		*(pPtr+1) = '\0';

		if (!szMsg[0])
			return;
	}
	
	This->_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "TrickFileGenFilter: (TrickGenLib Message) %s", szMsg);

//	if(bAbortion)
//	{
//		This->_graph.writeLog(loglevel, GetCurrentThreadId(), "TrickFileGenFilter: (TrickGenLib Message) met error, abort TrickFileGenFilter");
//		This->abort();
//	}
}


void TrickFileGenFilter::getTrickExt(int index, char* ext1, char* ext2)
{
	if(0 == index)
	{
		sprintf(ext1, ".FF");
		sprintf(ext2, ".FR");
	}
	else
	{
		sprintf(ext1, ".FF%d", index);
		sprintf(ext2, ".FR%d", index);
	}		
}

void TrickFileGenFilter::initOutputFileInfo()
{
	int outputIndex = 0;

	// main file
	_outputInfo[outputIndex].fileNo = outputIndex;
	_outputInfo[outputIndex].fileType = OPTFT_MAIN;
	_outputInfo[outputIndex].speed = 0;
	_outputInfo[outputIndex].offset = 0;
	_outputInfo[outputIndex].thisFilter = this;
	_outputInfo[outputIndex].ioRender = NULL;
	strcpy(_outputInfo[outputIndex].extension, "");

	_outputInfo[outputIndex].speedNumerator = 1;
	_outputInfo[outputIndex].speedDenominator = 1;

	outputIndex++;
	
	// set the trick file output setting
	for(int i=0; i<_numTrickSpeed; i++)
	{
		char extFF[16];
		char extFR[16];

		getTrickExt(i, extFF, extFR);

		// set FF
		_outputInfo[outputIndex].fileNo = outputIndex;
		_outputInfo[outputIndex].fileType = OPTFT_FF;
		_outputInfo[outputIndex].speed = i+1;
		_outputInfo[outputIndex].offset = 0;
		_outputInfo[outputIndex].thisFilter = this;
		_outputInfo[outputIndex].ioRender = NULL;
		strcpy(_outputInfo[outputIndex].extension, extFF);

		_outputInfo[outputIndex].speedNumerator = trickSpeedNumerator[i];
		_outputInfo[outputIndex].speedDenominator = trickSpeedDenominator[i];
		
		outputIndex++;

		// set FR
		_outputInfo[outputIndex].fileNo = outputIndex;
		_outputInfo[outputIndex].fileType = OPTFT_FR;
		_outputInfo[outputIndex].speed = -(i+1);
		_outputInfo[outputIndex].offset = 0;
		_outputInfo[outputIndex].thisFilter = this;
		_outputInfo[outputIndex].ioRender = NULL;
		strcpy(_outputInfo[outputIndex].extension, extFR);

		_outputInfo[outputIndex].speedNumerator = trickSpeedNumerator[i];
		_outputInfo[outputIndex].speedDenominator = trickSpeedDenominator[i];
		
		outputIndex++;
	}

	// index file
	_outputInfo[outputIndex].fileNo = outputIndex;
	_outputInfo[outputIndex].fileType = OPTFT_VVX;
	_outputInfo[outputIndex].speed = 0;
	_outputInfo[outputIndex].offset = 0;
	_outputInfo[outputIndex].thisFilter = this;
	_outputInfo[outputIndex].ioRender = NULL;
	strcpy(_outputInfo[outputIndex].extension, ".vvx");

	_outputInfo[outputIndex].speedNumerator = 0;
	_outputInfo[outputIndex].speedDenominator = 1;

	outputIndex++;

	// splicing file
	if(_enableSplicing)
	{
		// index file
		_outputInfo[outputIndex].fileNo = outputIndex;
		_outputInfo[outputIndex].fileType = OPTFT_SPLICE;
		_outputInfo[outputIndex].speed = 0;
		_outputInfo[outputIndex].offset = 0;
		_outputInfo[outputIndex].thisFilter = this;
		_outputInfo[outputIndex].ioRender = NULL;
		strcpy(_outputInfo[outputIndex].extension, ".vvt");

		_outputInfo[outputIndex].speedNumerator = 0;
		_outputInfo[outputIndex].speedDenominator = 1;

		outputIndex++;		
	}

	_outputFileCount = outputIndex;
}

bool TrickFileGenFilter::initOutputSetting(int outputFileIndex)
{	
	if(NULL == _trickContext)
		return false;

	// reset the offset
	_outputInfo[outputFileIndex].offset = 0;

	OutputFileType fileType = _outputInfo[outputFileIndex].fileType;
	long speed = _outputInfo[outputFileIndex].speed;

	DWORD status = STATUS_SUCCESS;
	
	switch(fileType)
	{
	case OPTFT_FF:
		status = TrickFilesCreateOutputContext(
				_trickContext, (PVOID)(&_outputInfo[outputFileIndex]), speed, _dwTrickBufferSize,
				acquireOutputMpegBuffer, releaseOutputMpegBuffer);
		break;

	case OPTFT_FR:
		status = TrickFilesCreateOutputContext(
				_trickContext, (PVOID)(&_outputInfo[outputFileIndex]), speed, _dwTrickBufferSize,
				acquireOutputMpegBuffer, releaseOutputMpegBuffer);
		break;

	case OPTFT_VVX:
		status = TrickFilesCreateVvxContext(
				_trickContext, (PVOID)(&_outputInfo[outputFileIndex]), _dwTrickBufferSize,
				acquireOutputMpegBuffer, releaseOutputMpegBuffer, flushOutputMpegBuffer);
		break;

	case OPTFT_MAIN:

		status = TrickFilesInitializeSourceStream(
				_trickContext, (PVOID)(&_outputInfo[outputFileIndex]), _dwTrickBufferSize,
				_enableSplicing,
				acquireSourceMpegBuffer, releaseSourceMpegBuffer);

		//
		// set maximum MPEG coding errors permitted
		//
		TrickFilesSetMaximumMpegCodingErrors(_trickContext, _maxMpegCodingError);

		memcpy(&_trickCharacteristics,
			TrickFilesStreamCharacteristics(_trickContext),
			sizeof(_trickCharacteristics));

		break;

	case OPTFT_SPLICE:
		
		status = TrickFilesCreateSpliceContext(
				_trickContext, (PVOID)(&_outputInfo[outputFileIndex]), _dwTrickBufferSize,
				DEF_SPLICE_MIN_GOP_SIZE, DEF_SPLICE_GRADUAL_TIMING, DEF_SPLICE_HOLD_PIC_COUNT, 
				acquireOutputMpegBuffer, releaseOutputMpegBuffer);
		break;

	default:
		break;
	}

	if (!NT_SUCCESS(status))
	{
		return false;
	}
	return true;
}

bool TrickFileGenFilter::addSplicePoint(SplicePointType splicetype, DWORD splicetime)
{
	SPLICEPOINTS spoint;
	spoint.splicetype = splicetype;
	spoint.spliceTime = splicetime;

	_splicePoints.push_back(spoint);

	return true;
}

bool TrickFileGenFilter::openSession(std::string& errMsg)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter::openSession() enter");

	DWORD status = STATUS_SUCCESS;

	status = TrickFilesLibraryInitialize(
				memoryAllocate, memoryFree, logMsgFromTrickLibrary);

	if (!NT_SUCCESS(status))
	{
		errMsg = "TrickFilesLibraryInitialize failed";
		_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "TrickFileGenFilter: TrickFilesLibraryInitialize() failed");

		return false;
	}	

	//
	// reset session related variables
	//
	_dwPictureCount = 0;
	_vvxFlushNeeded = 0;
	_flushCount = 0;

	//
	// Get a trick file gen context
	//
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: TrickFilesAllocateContext()");
	_trickContext = TrickFilesAllocateContext((PVOID)this);
	if (NULL == _trickContext)
	{
		// shut donw library
		TrickFilesLibraryShutdown();

		errMsg = "TrickFilesAllocateContext() failed";

		_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "TrickFileGenFilter: TrickFilesAllocateContext failed");
		
		return false;
	}

	// set target file name
	TrickFilesSetFileName(_trickContext, (PUCHAR)_graph.getContentName().c_str());
	
	// Need to set this pidremap flag earlier, before the source stream context
	// is created
	TrickFilesSetPidRemapToAtsc(_trickContext, 1);

	// 
	// init trick files
	//
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: initialize trick files");
	for(int i=0; i<_outputFileCount; i++)
	{
			// set FFR file output setting
		bool bRet = initOutputSetting(i);
		if(!bRet)
		{
			errMsg = "TrickFile-Gen initialize trick file failed";

			_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "TrickFileGenFilter: initialize trick file %s%s failed", 
				_graph.getContentName().c_str(), _outputInfo[i].extension);

			closeSession();
			return false;
		}
	}
	
	//
	// create the preliminary index header (VVX) file
	//
	TrickFilesUpdateVvxIndexHeader(_trickContext);

	if(_enableSplicing)
	{
		SPLICE_IDENTIFIER spliceIndentifier;
		//
		// set splicing at the front of mpeg file
		//
		spliceIndentifier.spliceId.familyType = TRICKFILES_SPLICEFAMILY_SYSTEM;
		spliceIndentifier.spliceId.sequence = DEF_SPLICE_FRONT_ID;
		spliceIndentifier.spliceType = TRICKFILES_SPLICETYPE_FRONT;
		spliceIndentifier.spliceTime = 0;
		spliceIndentifier.spliceStatus = TRICKSPLICE_INITIAL;

		TrickFilesAddCues(_trickContext, (TRICKFILES_SPLICEFAMILY)spliceIndentifier.spliceId.familyType, 1, &spliceIndentifier);
		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: TrickFilesAddCues() for Id(familyType::sequence) = %d::%d, type = %d[%s], time = %d, status = %d[%s]", 
						spliceIndentifier.spliceId.familyType,
						spliceIndentifier.spliceId.sequence, 
						spliceIndentifier.spliceType,
						SPLICE_TYPE[spliceIndentifier.spliceType],
						spliceIndentifier.spliceTime,
						spliceIndentifier.spliceStatus,
						SPLICE_STATUS[spliceIndentifier.spliceStatus]);

		//
		// set splicing at the end of mpeg file
		//
		spliceIndentifier.spliceId.familyType = TRICKFILES_SPLICEFAMILY_SYSTEM;
		spliceIndentifier.spliceId.sequence = DEF_SPLICE_BACK_ID;
		spliceIndentifier.spliceType = TRICKFILES_SPLICETYPE_BACK;
		spliceIndentifier.spliceTime = 0;
		spliceIndentifier.spliceStatus = TRICKSPLICE_INITIAL;

		TrickFilesAddCues(_trickContext, (TRICKFILES_SPLICEFAMILY)spliceIndentifier.spliceId.familyType, 1, &spliceIndentifier);

		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: TrickFilesAddCues() for Id(familyType::sequence) = %d::%d, type = %d[%s], time = %d, status = %d[%s]", 
						spliceIndentifier.spliceId.familyType,
						spliceIndentifier.spliceId.sequence, 
						spliceIndentifier.spliceType,
						SPLICE_TYPE[spliceIndentifier.spliceType],
						spliceIndentifier.spliceTime,
						spliceIndentifier.spliceStatus,
						SPLICE_STATUS[spliceIndentifier.spliceStatus]);

		DWORD sequence = 1;
		for(DWORD i=0; i<_splicePoints.size(); i++)
		{
			// if spliceTime == 0, it is front or back, already cued
			if(0 == _splicePoints[i].spliceTime)
				continue;

			//
			// set cue in/out splicing pointer
			//
			spliceIndentifier.spliceId.familyType = TRICKFILES_SPLICEFAMILY_SYSTEM;
			spliceIndentifier.spliceId.sequence = sequence++;
			spliceIndentifier.spliceType = _splicePoints[i].splicetype == SPLICE_CUE_IN ? TRICKFILES_SPLICETYPE_IN : TRICKFILES_SPLICETYPE_OUT;
			spliceIndentifier.spliceTime = _splicePoints[i].spliceTime;
			spliceIndentifier.spliceStatus = TRICKSPLICE_INITIAL;

			TrickFilesAddCues(_trickContext, (TRICKFILES_SPLICEFAMILY)spliceIndentifier.spliceId.familyType, 1, &spliceIndentifier);
			_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: TrickFilesAddCues() for Id(familyType::sequence) = %d::%d, type = %d[%s], time = %d, status = %d[%s]", 
							spliceIndentifier.spliceId.familyType,
							spliceIndentifier.spliceId.sequence, 
							spliceIndentifier.spliceType,
							SPLICE_TYPE[spliceIndentifier.spliceType],
							spliceIndentifier.spliceTime,
							spliceIndentifier.spliceStatus,
							SPLICE_STATUS[spliceIndentifier.spliceStatus]);
		}
	}


	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter::openSession() leave");

	return true;
}

bool TrickFileGenFilter::closeSession()
{
	if (NULL == _trickContext)
		return true;

	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter::closeSession() enter");

	// release context
	TrickFilesReleaseSourceMpegBuffers (_trickContext);

	TrickFilesFreeContext(_trickContext);
	_trickContext = NULL;

	// shut down library
	TrickFilesLibraryShutdown();

	// clear the splice points
	_splicePoints.clear();

	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter::closeSession() leave");

	return true;
}

bool TrickFileGenFilter::import(bool& bProcessNextPic, std::string& errMsg)
{
	_graph.traceLog(id(), "TrickFileGenFilter: Import() enter");

	DWORD status = STATUS_SUCCESS;	

	//
	// call library to process 1 I Frame
	//
	_dwPictureCount++;
	status = TrickFilesProcessNextPicture(_trickContext);

	//
	// check for completion code
	//
	if (status == STATUS_EVENT_DONE)
	{
		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: Import completion with %d pictures", _dwPictureCount);

		bProcessNextPic = false;
		return true;
	}
	else if (!NT_SUCCESS(status))
	{
		errMsg = "TrickFile Generation Library TrickFilesProcessNextPicture() failed";
		_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "TrickFileGenFilter: Import failed when processing picture %d", _dwPictureCount);

		bProcessNextPic = false;
		return false;
	}

	if(_dwPictureCount % 100 == 0)
		_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "TrickFileGenFilter: Import succeed to process picture %d", _dwPictureCount);
	else
		_graph.traceLog(id(), "TrickFileGenFilter: Import succeed to process picture %d", _dwPictureCount);

	//
	// see if a vvx update has been requested
	//
	// A VVX update is needed to keep the index and the source file
	// and speed files written in synchronization in order for a
	// video to be played while trick files are being created.
	//
	// See also ReleaseOutputMpegBuffer routine
	//
	if (_vvxFlushNeeded)
	{
		//
		// update index records and re-write the header
		//
		TrickFilesUpdateVvxIndexHeader(_trickContext);
		copyVvxHeaderInfo();

		_vvxFlushNeeded = 0;
		_flushCount++;

		_graph.traceLog(id(), "TrickFileGenFilter: Import updating VVX header required for the %d time", _flushCount);
	}

	// go on process next pic 
	bProcessNextPic = true;

	_graph.traceLog(id(), "TrickFileGenFilter: Import() leave");

	return true;
}

void TrickFileGenFilter::importDone(bool importStatus)
{
	if(NULL == _trickContext )
		return;

	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter::importDone() enter");

	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: Total  %d Pictures are processed", _dwPictureCount);
	//
	// get a copy of the counters
	//
	TRICK_COUNTERS trickCounters;
	memcpy(&trickCounters, TrickFilesGetCounters(_trickContext), sizeof(trickCounters));

	//
	// re-write the header
	//
//	if(importStatus)
//	{
		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: TrickFilesUpdateVvxIndexHeader at the end of stream");
		
		TrickFilesUpdateVvxIndexHeader(_trickContext);
		TrickFilesReleaseIndexMpegBuffers(_trickContext);
		copyVvxHeaderInfo();

		//
		// Query splice points creation status
		//
		int iCount = TrickFilesQueryCueCount(_trickContext);
		SPLICE_IDENTIFIER *pspliceIndentifier = new SPLICE_IDENTIFIER[iCount];
		TrickFilesQueryCueStatus(_trickContext, (TRICKFILES_SPLICEFAMILY) 0, &iCount, pspliceIndentifier);
		// log splice points created and status
		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: there are %d splice points were created", iCount);
		for (int n = 0; n < iCount; n++)
		{
			_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: Splice point %d - Id(familyType::sequence) = %d::%d, type = %d[%s], time = %d, status = %d[%s]", 
				n+1,
				pspliceIndentifier[n].spliceId.familyType,
				pspliceIndentifier[n].spliceId.sequence, 
				pspliceIndentifier[n].spliceType,
				SPLICE_TYPE[pspliceIndentifier[n].spliceType],
				pspliceIndentifier[n].spliceTime,
				pspliceIndentifier[n].spliceStatus, 
				SPLICE_STATUS[pspliceIndentifier[n].spliceStatus]);
		}
		delete [] pspliceIndentifier;
//	}


	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: Bytes processed(%I64d), pictures decoded(%d), pictures encoded(%d), pictures deinterlaced(%d), pictures with coding errors(%d)", 
			trickCounters.bytesProcessed,
			trickCounters.picturesDecoded,
			trickCounters.picturesEncoded,
			trickCounters.picturesDeinterlaced,
			trickCounters.picturesWithCodingErrors);

	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter::importDone() leave");
}

bool TrickFileGenFilter::copyVvxHeaderInfo()
{
	int fileNo = -1;
	int i=0;
	// publish the buff data
	for(i=0; i<_outputFileCount; i++)
	{
		if(OPTFT_VVX == _outputInfo[i].fileType)
		{
			fileNo = i;
			break;
		}
	}

	if(-1 == fileNo)
	{
		return false;
	}
	
	_graph.traceLog(id(), "TrickFileGenFilter: copyVvxHeaderInfo(%s) enter", 
		_outputInfo[fileNo].extension);
	//
	// allocate buffer from buffer pool
	//
	ZQ::Content::BufferData* pBuffData = _pool.alloc();

	if(NULL == pBuffData)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "TrickFileGenFilter: CopyVvxHeaderInfo failed to alloc buffer from buffer pool");
		return false;
	}	

	_graph.traceLog(id(), "TrickFileGenFilter: alloc buffData from pool. [BuffData Address: 0x%08X]", 
					pBuffData);


	DWORD dwLen = TrickFilesGetVvxIndexHeaderLength(_trickContext);
	void* pointer = pBuffData->getPointerForWrite(dwLen);

	pBuffData->setActualLength(dwLen);

	memcpy(pointer, TrickFilesGetVvxIndexHeader(_trickContext), dwLen);

	// pass buffer to connected IO render
	pBuffData->setProperty(CNTPRY_TRICK_GEN_VVX, (DWORD)TRICK_GEN_VVX_HEADER);

	// set the fileNo in the buffer property
	if(_connectedIORenderCount)
	{
		pBuffData->setProperty(ZQ::Content::Process::CNTPRY_SUBFILE_NO, (DWORD)fileNo);
	}

	_outputInfo[i].ioRender->receive(this, pBuffData);


	//
	// release buff from pool
	//
	bool bReleased = releaseBuffer(pBuffData);
	if(bReleased)
	{
		_graph.traceLog(id(), "TrickFileGenFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
						pBuffData);
	}

	_graph.traceLog(id(), "TrickFileGenFilter: copyVvxHeaderInfo(%s) leave", 
		_outputInfo[fileNo].extension);

	return 1;
}

bool TrickFileGenFilter::acquireSourceMpegBufferProcess(int fileNo, PMPEG_BUFFER* ppMpegBuffer)
{
	_graph.traceLog(id(), "TrickFileGenFilter: acquireSourceMpegBufferProcess(%s) enter", 
		_outputInfo[fileNo].extension);

	//
	// Allocate a data buffer
	//
	ZQ::Content::BufferData* pBuffData = NULL;
	{
		ZQ::common::MutexGuard guard(_dataMutex);
		if(!_dataQueue.empty())
		{
			pBuffData = _dataQueue.front();
			_dataQueue.pop();
		}
	}
	
	DWORD sleepTime = 500;
	// the stream does not end, but the available stream data is not arrived
	// to this render yet, wait for the event. 
	while(NULL == pBuffData)
	{
		// since we have return NULL to the callback if end of stream,
		// but when at the time of end of stream, should be not wait for 500 seconds, so reset it to be 0
		if(_endOfStream)
		{
			sleepTime = 0;
		}
		// wait for coming data
		DWORD dwStatus = WaitForSingleObject(_hNotify, sleepTime);
		switch(dwStatus)
		{
		case WAIT_TIMEOUT:
		case WAIT_OBJECT_0:
			{
				if(STOPPED == _processStatus || ABORTED == _processStatus)
				{
					_graph.traceLog(id(), "TrickFileGenFilter: acquireSourceMpegBufferProcess(%s) left with return NULL", 
						_outputInfo[fileNo].extension);

					return false;
				}
				// not end, there must be new data coming, get it
				ZQ::common::MutexGuard guard(_dataMutex);
				if(!_dataQueue.empty())
				{
					pBuffData = _dataQueue.front();
					_dataQueue.pop();
				}
				else if(_endOfStream)
				{
					_graph.traceLog(id(), "TrickFileGenFilter: reach the end of stream, acquireSourceMpegBufferProcess(%s) return NULL", 
						_outputInfo[fileNo].extension);

					_graph.traceLog(id(), "TrickFileGenFilter: acquireSourceMpegBufferProcess(%s) left with return NULL", 
						_outputInfo[fileNo].extension);
					
					return false;
				}
			}
			break;
		}
	}

	*ppMpegBuffer = allocMpegBuffer();

	//
	// initialize the pMpegBuffer for this callback return value
	//
	DWORD dwLen = 0;
	PUCHAR pointer = (PUCHAR)pBuffData->getPointerForRead(dwLen);

	//
	// format MPEG_BUFFER object with
	// the data buffer pointer, length
	// and file byte offset for this buffer
	//
	MpegLibraryInitializeBuffer(*ppMpegBuffer, pBuffData, _outputInfo[fileNo].offset, dwLen, pointer);
	_outputInfo[fileNo].offset += dwLen;

	_graph.traceLog(id(), "TrickFileGenFilter: acquireSourceMpegBufferProcess(%s) leave with required buffer size %d bytes", 
		_outputInfo[fileNo].extension, dwLen);

	return true;
}

void TrickFileGenFilter::releaseSourceMpegBufferProcess(int fileNo, PMPEG_BUFFER pMpegBuffer, DWORD consumedBytes)
{
	_graph.traceLog(id(), "TrickFileGenFilter: releaseSourceMpegBufferProcess(%s) enter", 
		_outputInfo[fileNo].extension);

	// get the BufferData from mpegBuffer's context
	ZQ::Content::BufferData* pBuffData = (ZQ::Content::BufferData*)pMpegBuffer->bufferContext;

	if (NULL == pBuffData)
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, id(), "TrickFileGenFilter: releaseSourceMpegBufferProcess(%s) released BuffData pointer is invalid", 
			_outputInfo[fileNo].extension);

		// free mpeg buffer
		freeMpegBuffer(pMpegBuffer);

		return;
	}
	
	//
	// In this callback, consumedBytes can not be used, coz for last buffer, it is 0
	//
	// set the processed bytes for progress 
	DWORD dwLen = 0;
	pBuffData->getPointerForRead(dwLen);
	_processedBytes += dwLen;
	_dwCounter++;

	// set the fileNo in the buffer property
	if(_connectedIORenderCount)
	{
		pBuffData->setProperty(ZQ::Content::Process::CNTPRY_SUBFILE_NO, (DWORD)fileNo);
	}

	_outputInfo[fileNo].ioRender->receive(this, pBuffData);
	
	//
	// release buff from pool
	//
	bool bReleased = releaseBuffer(pBuffData);
	if(bReleased)
	{
		_graph.traceLog(id(), "TrickFileGenFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
						pBuffData);
	}

	// free mpeg buffer
	freeMpegBuffer(pMpegBuffer);

	_graph.traceLog(id(), "TrickFileGenFilter: releaseSourceMpegBufferProcess(%s) leave with released buffer size %d bytes",
		_outputInfo[fileNo].extension, dwLen);
}

bool TrickFileGenFilter::acquireOutputMpegBufferProcess(int fileNo, PMPEG_BUFFER* ppMpegBuffer)
{
	_graph.traceLog(id(), "TrickFileGenFilter::acquireOutputMpegBufferProcess(%s) enter",
		_outputInfo[fileNo].extension);

	//
	// allocate buffer from buffer pool
	//
	ZQ::Content::BufferData* pBuffData = _pool.alloc();

	if(NULL == pBuffData)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "TrickFileGenFilter: allocating buffer return NULL");
		return false;
	}

	_graph.traceLog(id(), "TrickFileGenFilter: alloc buffData from pool. [BuffData Address: 0x%08X]", 
					pBuffData);

	// allocate mpeg buffer
	*ppMpegBuffer = allocMpegBuffer();

	_graph.traceLog(id(), "TrickFileGenFilter: buffer map - MpegBuffer [0x%08X], buffer [0x%08X]", *ppMpegBuffer, pBuffData);

	// initialize the pMpegBuffer for this callback return value
	DWORD dwLen = _dwTrickBufferSize;
	// set the size
	pBuffData->setActualLength(dwLen);

	PUCHAR pointer = (PUCHAR)pBuffData->getPointerForWrite(_dwTrickBufferSize);

	//
	// format MPEG_BUFFER object with
	// the data buffer pointer, length
	// and file byte offset for this buffer
	//
	MpegLibraryInitializeBuffer(*ppMpegBuffer, pBuffData, _outputInfo[fileNo].offset, dwLen, pointer);
	_outputInfo[fileNo].offset += (*ppMpegBuffer)->length;


	_graph.traceLog(id(), "TrickFileGenFilter::acquireOutputMpegBufferProcess(%s) leave with required buffer size %d bytes", 
		_outputInfo[fileNo].extension, dwLen);

	return true;
}

void TrickFileGenFilter::releaseOutputMpegBufferProcess(int fileNo, PMPEG_BUFFER pMpegBuffer, DWORD consumedBytes)
{
	_graph.traceLog(id(), "TrickFileGenFilter::releaseOutputMpegBufferProcess(%s) enter", 
		_outputInfo[fileNo].extension);

	// get the BufferData from mpegBuffer's context
	ZQ::Content::BufferData* pBuffData = (ZQ::Content::BufferData*)pMpegBuffer->bufferContext;

	if(NULL == pBuffData)
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, id(), "TrickFileGenFilter: releaseOutputMpegBufferProcess(%s) released BuffData pointer is invalid",
			_outputInfo[fileNo].extension);

		// free Mpeg buffer
		freeMpegBuffer(pMpegBuffer);

		return;
	}

	//
	// parse buff to connected IO render
	//
	if(0 == consumedBytes)
	{
		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: releaseOutputMpegBufferProcess(%s) released buffer with 0 consumed bytes", 
			_outputInfo[fileNo].extension);
	}
	else
	{
		// reset the actual length after processing
		pBuffData->setActualLength(consumedBytes);

		if(OPTFT_VVX == _outputInfo[fileNo].fileType)
		{
			pBuffData->setProperty(CNTPRY_TRICK_GEN_VVX, (DWORD)TRICK_GEN_VVX_NORMAL);
		}

		// set the fileNo in the buffer property
		if(_connectedIORenderCount)
		{
			pBuffData->setProperty(ZQ::Content::Process::CNTPRY_SUBFILE_NO, (DWORD)fileNo);
		}
		_outputInfo[fileNo].ioRender->receive(this, pBuffData);
	}

	//
	// release buff from pool
	//
	bool bReleased = releaseBuffer(pBuffData);
	if(bReleased)
	{
		_graph.traceLog(id(), "TrickFileGenFilter: free buffData from pool. [BuffData Address: 0x%08X]", 
						pBuffData);
	}
	
	//
	// key the update of the index file on writes of the FR file
	// to keep index records and Mpeg file data synchronized
	//

	// This is for PWE vvx flush
	if(0 != consumedBytes && OPTFT_FR == _outputInfo[fileNo].fileType)
	{
		_vvxFlushNeeded = 1;
	}
	
	// free Mpeg buffer
	freeMpegBuffer(pMpegBuffer);

	_graph.traceLog(id(), "TrickFileGenFilter::releaseOutputMpegBufferProcess(%s) leave with released buffer size %d bytes",
		_outputInfo[fileNo].extension, consumedBytes);
}

void TrickFileGenFilter::flushOutputMpegBufferProcess(int fileNo, PMPEG_BUFFER pMpegBuffer, DWORD consumedBytes)
{
//
//	This is unlike ReleaseOutputMpegBufferProcess in that the data buffer being writting is still
//	in use by the trickfile library. So pMpegBuffer and its bufferdata can not be released here. 
//

	_graph.traceLog(id(), "TrickFileGenFilter: flushOutputMpegBufferProcess(%s) enter", 
		_outputInfo[fileNo].extension);

	// get the BufferData from mpegBuffer's context
	ZQ::Content::BufferData* pBuffData = (ZQ::Content::BufferData*)pMpegBuffer->bufferContext;

	if(NULL == pBuffData)
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, id(), "TrickFileGenFilter: flushOutputMpegBufferProcess(%s) released BuffData pointer is invalid",
			_outputInfo[fileNo].extension);
		return;
	}
	
	//
	// parse buff to connected IO render
	//
	if(0 == consumedBytes)
	{
		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "TrickFileGenFilter: releaseOutputMpegBufferProcess(%s) released buffer with 0 consumed bytes", 
			_outputInfo[fileNo].extension);
	}
	else
	{
		// reset the actual length after processing
		pBuffData->setActualLength(consumedBytes);
		
		// in the flush callback, the buffer could not be released, it is still used by Trick-Gen
		// So have a copy of it and pass the copied buffer to connected IO render
		ZQ::Content::BufferData* cpyBuffData = _pool.alloc();
		*cpyBuffData = *pBuffData;

		_graph.traceLog(id(), "TrickFileGenFilter: alloc buffData from pool. [BuffData Address: 0x%08X]", 
						cpyBuffData);

		cpyBuffData->setProperty(CNTPRY_TRICK_GEN_VVX, (DWORD)TRICK_GEN_VVX_FLUSH);

		// set the fileNo in the buffer property
		if(_connectedIORenderCount)
		{
			cpyBuffData->setProperty(ZQ::Content::Process::CNTPRY_SUBFILE_NO, (DWORD)fileNo);
		}
		
		_outputInfo[fileNo].ioRender->receive(this, cpyBuffData);
	}
	
	_graph.traceLog(id(), "TrickFileGenFilter: flushOutputMpegBufferProcess(%s) leave with flushed buffer size %d bytes", 
		_outputInfo[fileNo].extension, consumedBytes);	
}

/////////////////////////////////////////////////////////////////////////////
//
//  AcquireSourceMpegBuffer
//  This routine is called by the trick library when more mpeg data is 
//	needed.
//
//	Returns
//
//		== 0	End of file or error happened
//		!= 0	new MpegBuffer containing mpeg data
//
////////////////////////////////////////////////////////////////////////////
PMPEG_BUFFER TrickFileGenFilter::acquireSourceMpegBuffer(PVOID context)
{
	POutputFileInfo pOutputFile = (POutputFileInfo)context;
	TrickFileGenFilter* This = (TrickFileGenFilter*)pOutputFile->thisFilter;

	PMPEG_BUFFER pMpegBuffer = NULL;

	bool bRet = This->acquireSourceMpegBufferProcess(pOutputFile->fileNo, &pMpegBuffer);

	if (!bRet || NULL == pMpegBuffer)
	{
		return pMpegBuffer;
	}
	
	return pMpegBuffer;
}

/////////////////////////////////////////////////////////////////////////////
//
//  ReleaseSourceMpegBuffer
//
//	This routine is called by the trick file library when it has completed processing
//	of the indicated mpeg data buffer.
//
//	The buffer will likely have been altered so it needs to be written to the output
//	mpeg target file
//
//	Note: bytesConsumed indicates the amount of video bytes contained in the buffer 
//	and should not be used as a byte count of the data.
//
/////////////////////////////////////////////////////////////////////////////
void TrickFileGenFilter::releaseSourceMpegBuffer(PVOID context, PMPEG_BUFFER mpegBuffer, ULONG bytesConsumed)
{
	POutputFileInfo pOutputFile = (POutputFileInfo)context;
	TrickFileGenFilter* This = (TrickFileGenFilter*)pOutputFile->thisFilter;

	if (NULL == mpegBuffer)
	{
		return;
	}

	This->releaseSourceMpegBufferProcess(pOutputFile->fileNo, mpegBuffer, bytesConsumed);
}

/////////////////////////////////////////////////////////////////////////////
//
//  AcquireOutputMpegBuffer
//	This routine is called by the trick file library when it needs more buffers for
//	output data to the trick files and/or vvx file.
//
/////////////////////////////////////////////////////////////////////////////
PMPEG_BUFFER TrickFileGenFilter::acquireOutputMpegBuffer(PVOID context)
{
	POutputFileInfo pOutputFile = (POutputFileInfo)context;
	TrickFileGenFilter* This = (TrickFileGenFilter*)pOutputFile->thisFilter;

	PMPEG_BUFFER pMpegBuffer= NULL;

	bool bRet = This->acquireOutputMpegBufferProcess(pOutputFile->fileNo, &pMpegBuffer);

	if (!bRet || NULL == pMpegBuffer)
	{
		return NULL;
	}

	return pMpegBuffer;
}

/////////////////////////////////////////////////////////////////////////////
//
//  ReleaseOutputMpegBuffer
//
//	This routine is called by the trick file library when it is releasing a vvx or trick
//	file (ff,fr) buffer to be written to disk.
//
//	bytesConsumed is the actual byte count of the data buffer.
//
/////////////////////////////////////////////////////////////////////////////
void TrickFileGenFilter::releaseOutputMpegBuffer(PVOID context, PMPEG_BUFFER mpegBuffer, ULONG bytesConsumed)
{
	POutputFileInfo pOutputFile = (POutputFileInfo)context;
	TrickFileGenFilter* This = (TrickFileGenFilter*)pOutputFile->thisFilter;
	
	if (NULL == mpegBuffer)
	{
		return;
	}

	This->releaseOutputMpegBufferProcess(pOutputFile->fileNo, mpegBuffer, bytesConsumed);
}

/////////////////////////////////////////////////////////////////////////////
//
//  FlushOutputMpegBuffer
//
//	This routine is called by the trick file library when new records are being added
//	to the vvx file.  the write of accumulated Vvx records is triggered by ReleaseOutputMpegBuffer
//	when it detects that a FR speed file is being released.  FF and FR files are updated at the 
//	same time so that's the only time that a vvx file and the speed files are synchronized with
//	respect to each other (i.e. all vvx records point to actual speed file data).
//
//	This routine is unlike ReleaseOutputMpegBuffer in that the data buffer being writting is still
//	in use by the trickfile library.
//
//	bytesConsumed is the actual byte count of the data buffer that is being written and may be
//	less than the total byte count of the buffer.
//
/////////////////////////////////////////////////////////////////////////////
void TrickFileGenFilter::flushOutputMpegBuffer(PVOID	context, PMPEG_BUFFER mpegBuffer, ULONG bytesConsumed)
{
	POutputFileInfo pOutputFile = (POutputFileInfo)context;
	TrickFileGenFilter* This = (TrickFileGenFilter*)pOutputFile->thisFilter;
	
	if (NULL == mpegBuffer)
	{
		return;
	}
		
	This->flushOutputMpegBufferProcess(pOutputFile->fileNo, mpegBuffer, bytesConsumed);
}


PMPEG_BUFFER TrickFileGenFilter::allocMpegBuffer()
{
	ZQ::common::MutexGuard guard(_mpegBufferMutex);
		
	PMPEG_BUFFER pMpegBuffer = NULL;

	if(_usefulMpegBufferPool.size() > 0)
	{
		// get the mpeg buffer from pool
		pMpegBuffer = _usefulMpegBufferPool.front();
		_usefulMpegBufferPool.pop();
	}
	else
	{
		// if no avaliable buffer in pool, 
		pMpegBuffer = new MPEG_BUFFER;
	}

	// put the buffer to used pool
	_usedMpegBufferPool[(long)pMpegBuffer] = pMpegBuffer;

	_graph.traceLog(id(), "TrickFileGenFilter: allocate MPEG buffer 0x%08X", pMpegBuffer);

	return pMpegBuffer;
}

void TrickFileGenFilter::freeMpegBuffer(PMPEG_BUFFER mpgBuffer)
{
	ZQ::common::MutexGuard guard(_mpegBufferMutex);

	_usedMpegBufferPool.erase(long(mpgBuffer));

	_usefulMpegBufferPool.push(mpgBuffer);

	_graph.traceLog(id(), "TrickFileGenFilter: free MPEG buffer 0x%08X", mpgBuffer);
}

void TrickFileGenFilter::freeMpegBufferPoolMemory()
{
	ZQ::common::MutexGuard guard(_mpegBufferMutex);

	// release buffer memory from useful pool
	while(_usefulMpegBufferPool.size() > 0)
	{
		PMPEG_BUFFER pMpgBuffer = _usefulMpegBufferPool.front();
		_usefulMpegBufferPool.pop();
		
		delete pMpgBuffer;
	}

	// release the used buffer
	MPEGBUFFERMAP::iterator it = _usedMpegBufferPool.begin();
	while(it != _usedMpegBufferPool.end())
	{
		PMPEG_BUFFER pMpgBuffer = (PMPEG_BUFFER)it->second;

		it++;
		
		delete pMpgBuffer;
	}
	_usedMpegBufferPool.clear();
}

} } }

#endif
