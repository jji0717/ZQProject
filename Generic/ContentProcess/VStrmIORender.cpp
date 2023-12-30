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
// Desc  : Implement the Vstrm file IO Render
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/ContentProcess/VStrmIORender.cpp 1     10-11-12 15:58 Admin $
// $Log: /ZQProjs/Generic/ContentProcess/VStrmIORender.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 46    08-03-11 18:03 Ken.qian
// Support supportfilesize
// 
// 45    08-03-06 17:00 Ken.qian
// 
// 44    08-02-25 17:40 Ken.qian
// notify endofstream to connected filter only once if
// RTFLibFilter/TrickFileGenFilter output to one filter(support subfile)
// 
// 43    08-02-20 11:44 Build
// Change "VstrmDLL.lib" to "VstrmDLLEx.lib"
// 
// 42    08-02-20 10:44 Build
// change codes to pass Vs2005 compile under x64
// 
// 41    08-02-02 11:24 Ken.qian
// use FILE_FLAG_CACHED to determine VsOpenEx or VsOpen
// 
// 40    08-02-01 16:19 Ken.qian
// And #ifdef to make VsOpenEx pass the compiling in Vstrm5.3
// 
// 39    08-01-30 14:48 Ken.qian
// VsOpenEx replace VsOpen to fix the slow Vstrm IO issue (NGOD90)
// 
// 38    07-11-16 19:16 Ken.qian
// add logic to fire Streamable event
// 
// 37    07-11-15 15:17 Ken.qian
// 
// 36    07-11-14 10:52 Ken.qian
// add #pragma comment(lib, "VstrmDLL.lib")
// 
// 35    07-11-13 17:20 Ken.qian
// 1. Support Pacing
// 2. VsIOLib Replace VstrmAPI
// 3. Support disable  BufDrv Throttle
// 
// 34    07-11-07 12:10 Ken.qian
// if bwClient=0, no reservation
// 
// 33    07-10-23 18:30 Ken.qian
// add vstrm get last error text
// 
// 32    07-10-15 18:12 Ken.qian
// make buffer processing log frequency MICRO based.
// 
// 31    07-09-20 17:59 Ken.qian
// add log while processing buffer
// 
// 30    07-09-17 15:26 Ken.qian
// 1. add protection in receive() to avoid keep too much buffer in the
// queue
// 2. The vvx file is generated to specified cache folder
// 
// 29    07-09-11 14:30 Ken.qian
// 1) Support TrickFilesLibraryUser index file output
// 2) Support the buffer size more than specified vstrm flush size
// 
// 28    07-08-16 12:46 Ken.qian
// 
// 27    07-08-14 15:32 Ken.qian
// 
// 26    07-08-13 9:52 Ken.qian
// fix the output file size is not same orignial one, it caused by
// _cacheCurLength was not reset
// 
// 25    07-08-09 17:10 Ken.qian
// 
// 24    07-08-06 15:35 Ken.qian
// 
// 23    07-08-03 15:55 Ken.qian
// 
// 22    07-08-02 17:07 Ken.qian
// 
// 21    07-08-01 17:57 Ken.qian
// 
// 20    07-07-31 21:06 Ken.qian
// 
// 19    07-07-26 18:08 Ken.qian
// fix vstrm io vvx issue
// 
// 18    07-07-23 15:29 Ken.qian
// vstrmiorender support subfiles and 64K IO writting
// 
// 17    07-07-19 17:45 Ken.qian
// normal release for rtfcpnode
// 
// 16    07-07-18 16:10 Ken.qian
// 
// 15    07-07-17 21:16 Ken.qian
// 
// 14    07-07-17 18:19 Ken.qian
// 
// 13    07-07-17 17:46 Ken.qian
// 
// 12    07-07-17 17:10 Ken.qian
// 
// 11    07-07-17 17:05 Ken.qian
// 
// 10    07-07-17 14:18 Ken.qian
// 
// 9     07-06-28 15:43 Fei.huang
// 
// 7     07-06-27 15:14 Ken.qian
// Change kbps to bps
// support vstrm bandwidth control
// 
// 6     07-06-26 17:09 Ken.qian
// 
// 5     07-06-22 12:05 Fei.huang
// suppress debug log output
// 
// 4     07-06-18 13:34 Fei.huang
// 
// 3     07-06-06 15:37 Ken.qian
// 
// 3     07-06-05 18:25 Ken.qian
// add the detail errorcode
// 
// 2     07-06-01 17:04 Ken.qian
// add checksum
// 
// 1     07-05-31 17:06 Ken.qian
// 
// 10    06-09-19 11:55 Ken.qian
// 
// 9     06-09-06 15:27 Ken.qian
// 
// 8     06-08-24 11:02 Ken.qian
// 
// 7     06-07-14 16:20 Shuai.chen
// 
// 6     06-07-13 13:28 Ken.qian
// 
// 5     06-07-13 13:16 Shuai.chen
// 
// 4     06-07-12 10:57 Shuai.chen
// 
// 3     06-07-12 9:26 Ken.qian
// 
// 2     06-06-30 11:32 Ken.qian
// Change the abort processing workflow
// 
// 1     06-06-26 14:50 Ken.qian
// Initial Implementation
// 


#include "VstrmIORender.h"
#include "urlstr.h"
#include "bufferpool.h"
#include <assert.h>
#include "PacedIndex.h"
#include "vsiolib.h"

#define VSTRMIO_MAX_QUEUED_SIZE  30 

#pragma comment(lib, "VstrmDLLEx.lib")
#pragma comment(lib, "PacedIndex.lib")
#pragma comment(lib, "vsiolib.lib")

namespace ZQ { 
namespace Content { 
/// namespace Process presents the processing to the content, such as reading/writing
namespace Process {

ZQ::Content::Process::Graph*  VstrmIORender::_pStaticGraph = NULL;
	
VstrmIORender::VstrmIORender(ZQ::Content::Process::Graph& graph, HANDLE vstrm, 
							 DWORD bwmgrClientId, bool pacing, DWORD streamablePlaytimeInSecond,
							 bool disableBufDrvThrottle, std::string myName)
: Filter(graph, myName), _bwmgrClientId(bwmgrClientId), 
_bEndOfStream(true), _progressRptFileNo(0), 
_pacing(pacing), _disableBufDrvThrottle(disableBufDrvThrottle), _streamablePlaytime(streamablePlaytimeInSecond),
_indexFileNo(0), _mainFileNo(0)
{	
	if(_pStaticGraph != NULL)
	{
		_pStaticGraph = &graph;
	}

	if(_pacing && _streamablePlaytime == 0)
	{
		_streamablePlaytime = DEF_STEAMABLE_PLAYTIME;
	}

	_tidAbortBy = 0;

	if(myName == "")
	{
		_myName = "VstrmIORender";
	}
	_bRunningOnNode = true;
	_nodeName = "";

	if(vstrm != INVALID_HANDLE_VALUE)
	{
		_hVstrm = vstrm;
		_initedLocal = false;
	}
	else
	{
		_initedLocal = true;
		_hVstrm = INVALID_HANDLE_VALUE;
	}
	
	_hStop = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);

	// start the thread
	start();

	// initialize the vstrm handle, this make only one handle in the whole filter life
	if(!initMyVstrm(_vstrmErrMsg))
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "initVstrm failed with reason: %s", _vstrmErrMsg.c_str());
	}
}

VstrmIORender::~VstrmIORender(void)
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
	
	// release source file handle and bandwidth
	uninitSubFiles();

	// free resource
	freeSubFilesObjs();

	// uninitialize the vstrm handle
	uninitMyVstrm();
}

void VstrmIORender::freeSubFilesObjs(void)
{
	for(DWORD i=0; i<_subFiles.size(); i++)
	{
		SubFile* subfile = _subFiles[i];
		delete subfile;
	}
	_subFiles.clear();
}

bool VstrmIORender::initSubFiles()
{
	// if no invoking setOutputFileXX function, just by default, one subfile
	if(_subFiles.size() == 0)
	{
		setSubFileCount(1);
		setSubFileInfo(0, DEF_VSTRM_WRITE_SIZE, true);

		_progressRptFileNo = 0;
	}

	// compose the file full name
	std::string cntName = _graph.getContentName();
	DWORD maxbps = _graph.getMaxbps();
	
	for(DWORD i=0; i<_subFiles.size(); i++)
	{
		if(!_subFiles[i]->init(cntName, maxbps))
		{
			return false;
		}
	}

	// add the output to pacing lib
	if(_pacing)
	{
		DWORD pacingGroup = GetCurrentThreadId();

		PacedIndexSetLogCbk(2, pacingAppLogCbk);
		DWORD paceresult = 0;
		for(DWORD i=0; i<_subFiles.size(); i++)
		{
			// skip the vvx, make sure it was added last
			if(i == _indexFileNo)
				continue;

			paceresult = PacedIndexAdd((void *) &pacingGroup, _pacingType.c_str(), _subFiles[i]->_outputFileName.c_str(), 
										pacingAppWrite, pacingAppSeek, pacingAppSetEOF, pacingAppReportOffsets, 
										(void*)_subFiles[i], &_subFiles[i]->_pacingIndexCtx);
			if(paceresult)
			{
				_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: PacedIndexAdd() failed %s with error", 
								_subFiles[i]->_outputFileName.c_str(), DecodePacedIndexError(paceresult));
				return false;	
			}
		}

		// add vvx last
		paceresult = PacedIndexAdd((void*) &pacingGroup, _pacingType.c_str(), _subFiles[_indexFileNo]->_outputFileName.c_str(), 
									pacingAppWrite, pacingAppSeek, pacingAppSetEOF, pacingAppReportOffsets, 
									(void*)_subFiles[_indexFileNo], &_subFiles[_indexFileNo]->_pacingIndexCtx);
		if(paceresult)
		{
			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: PacedIndexAdd() failed %s with error", 
							_subFiles[_indexFileNo]->_outputFileName.c_str(), DecodePacedIndexError(paceresult));
			return false;	
		}
	}

	return true;
}

void VstrmIORender::uninitSubFiles(bool delFile)
{	
	if(!_pacing)  // none pacing
	{
		for(DWORD i=0; i<_subFiles.size(); i++)
		{
			_subFiles[i]->uninit(delFile);
		}
	}
	else 
	{   
		// pacing un-initialization, need follow the this sequence

		DWORD paceresult = 0;
		DWORD i=0;
		//
		// Close the content files before closing the index
		// file so the content bytes are flushed to disk before
		// adding final references to index file
		//		
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: close file handle for none index file...");
		for(i=0; i<_subFiles.size(); i++)
		{
			if(i != _indexFileNo)
			{
				_subFiles[i]->uninit(delFile);
			}
		}

		//
		// Remove content files from pacing group so lib knows they're
		// closed and it's OK to write offsets into index
		//
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: PacedIndexRemove() none index file context...");
		for(i=0; i<_subFiles.size(); i++)
		{
			if(i != _indexFileNo && _subFiles[i]->_pacingIndexCtx != NULL)
			{
				paceresult = PacedIndexRemove(_subFiles[i]->_pacingIndexCtx);
				if (paceresult)
				{
					_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: PacedIndexRemove() failed %s with error", 
									_subFiles[i]->_outputFileName.c_str(), DecodePacedIndexError(paceresult));
				}
				_subFiles[i]->_pacingIndexCtx = NULL;
			}
		}

		//
		// Grab MD5 from index file
		//
		char md5[33] = "";

		if(_subFiles[_indexFileNo]->_pacingIndexCtx != NULL)
		{
			paceresult = PacedIndexGetPacedFileMD5(_subFiles[_indexFileNo]->_pacingIndexCtx, md5);
			if (paceresult)
			{
				_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: PacedIndexGetPacedFileMD5() failed with error", 
								DecodePacedIndexError(paceresult));
			}
			_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: PacedIndexGetPacedFileMD5() return %s", md5);
		}
		
		//
		// Remove index file
		//
		if(_subFiles[_indexFileNo]->_pacingIndexCtx != NULL)
		{
			_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: PacedIndexRemove() index file context...");
			paceresult = PacedIndexRemove(_subFiles[_indexFileNo]->_pacingIndexCtx);
			if (paceresult)
			{
				_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: PacedIndexRemove() failed %s with error", 
								_subFiles[_indexFileNo]->_outputFileName.c_str(), DecodePacedIndexError(paceresult));
			}
			_subFiles[_indexFileNo]->_pacingIndexCtx = NULL;
		}
		//
		// Close destination index file
		//
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: close file handle for index file...");
		_subFiles[_indexFileNo]->uninit(delFile);
	}
}

void VstrmIORender::emptyDataQueue(void)
{
	ZQ::common::MutexGuard guard(_dataMutex);
	// remove all the buffer data pointer from the queue.
	while (!_dataQueue.empty())
	{
		ZQ::Content::BufferData* pBuffData = _dataQueue.front();

		_pool.free(pBuffData);
		_dataQueue.pop();
	}
}

void VstrmIORender::updateMD5Property()
{
	for(DWORD i=0; i<_subFiles.size(); i++)
	{
		// report MD5 checksum
		_subFiles[i]->updateMD5Property();
	}
}

void VstrmIORender::updateSupportFileSize()
{
	__int64 supportSize = 0;
	for(DWORD i=0; i<_subFiles.size(); i++)
	{
		if(_mainFileNo == i || _indexFileNo == i)
			continue;

		// report MD5 checksum
		supportSize += _subFiles[i]->_processedBytes;
	}

	ZQ::common::Variant vsfs(supportSize);

	ContentProperty sfsize;
	sfsize.insert(ContentProperty::value_type(CNTPRY_SUPPORT_FILESIZE, vsfs));
	
	_graph.reportProperty(_graph.getContentName(), sfsize);
}

void VstrmIORender::setSubFileCount(DWORD fileCount)
{
	// free first to avoid invoking multiple times
	freeSubFilesObjs();

	// create sub file object
	for(DWORD i=0; i<fileCount; i++)
	{
		SubFile* subfile = new SubFile(i, _hVstrm, _bwmgrClientId, _cachePath, _pacing, _disableBufDrvThrottle, _graph);

		_subFiles.push_back(subfile);
	}
}

bool VstrmIORender::setSubFileInfo(DWORD fileNo, DWORD outputBuffSize, 
								   bool enableMD5, std::string fileExt, 
								   DWORD bwPercentageN, DWORD bwPercentageD, 
								   bool progressRpter)
{
	if(fileNo<0 || fileNo>=_subFiles.size())
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "VstrmIORender: fileNo %d is invalid", fileNo);
		return false;
	}

	// set the parameter
	_subFiles[fileNo]->_fileNo = fileNo;
	_subFiles[fileNo]->_extension = fileExt;
	_subFiles[fileNo]->_bwPercentageN = bwPercentageN;
	_subFiles[fileNo]->_bwPercentageD = bwPercentageD;
	_subFiles[fileNo]->_enableMD5Checksum = enableMD5;

	_subFiles[fileNo]->setCacheBufferSize(outputBuffSize);

	// get pacing type
	if(_pacing)
	{
		if( (stricmp(fileExt.c_str(), ".VVX") == 0) || (stricmp(fileExt.c_str(), ".VV2") == 0) )
		{
			_pacingType = fileExt.substr(1, 3);
			_indexFileNo = fileNo;
		}
		else if(stricmp(fileExt.c_str(), "") == 0)
		{
			_mainFileNo = fileNo;
		}
	}

	// set the reporter
	if(progressRpter)
	{
		_progressRptFileNo = fileNo;
	}
	return true;
}

bool VstrmIORender::vvxSubFileByTrickGen()
{
	for(DWORD i=0; i<_subFiles.size(); i++)
	{
		SubFile* subfile = _subFiles[i];

		// find the vvx sub file 
		if(stricmp(subfile->_extension.c_str(), ".vvx") == 0 && 0 == subfile->_cacheBufferSize)
		{
			subfile->_vvxByTrickGen = true;

			return true;
		}
	}
	return false;
}
void VstrmIORender::setVstrmNode(bool bRunningOnServer, std::string nodeName)
{
	_bRunningOnNode = bRunningOnServer;
	_nodeName = nodeName;
}

/// set cache path
void VstrmIORender::setCacheDirectory(std::string path) 
{ 
	_cachePath = path; 
	if(_cachePath == "")
		return;

	int pos = _cachePath.length() - 1;
	if(_cachePath[pos] != '\\' && _cachePath[pos] != '/' )
	{
		_cachePath = _cachePath + "\\";
	}
}

bool VstrmIORender::receive(Filter* upObj, ZQ::Content::BufferData* buff)
{
	ZQ::Content::BufferData* receivedBuff = NULL;
	if(_copyUplinkDataBuff)
	{	// copy BuffData
		_graph.traceLog(id(), "BufferPool: usage: %d / %d [used/total]", 
				  _pool.getUsedCount(), _pool.getPoolSize());
		
		// copy the buffer to the new one
		receivedBuff = _pool.alloc();
		*receivedBuff = *buff;

		_graph.traceLog(id(), "VstrmIORender: alloc buffData from pool. [BuffData Address: 0x%08X]", 
						receivedBuff);
	}
	else
	{   // does not copy BuffData
		receivedBuff = buff;
	}

	if(STOPPED == _processStatus || ABORTED == _processStatus)
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "VstrmIORender is in STOPPED or ABORTED status, can not receive any data");

		bool bReleased = releaseBuffer(receivedBuff, this);	
		if(bReleased)
		{
			_graph.traceLog(id(), "VstrmIORender: free buffData from pool. [BuffData Address: 0x%08X]", 
							receivedBuff);
		}
		return false;
	}

	if(_bEndOfStream)
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "VstrmIORender is end of stream, can not receive any data");

		bool bReleased = releaseBuffer(receivedBuff, this);	
		if(bReleased)
		{
			_graph.traceLog(id(), "VstrmIORender: free buffData from pool. [BuffData Address: 0x%08X]", 
							receivedBuff);
		}
		return false;
	}
	// put the data to the queue
	ZQ::common::MutexGuard guard(_dataMutex);
	_dataQueue.push(receivedBuff);
	
	_graph.traceLog(GetCurrentThreadId(), "VstrmIORender: Receive Buffer Data from up side process object with actual length %d", buff->getActualLength());

	// avoid too much buffer queued here
	if(_dataQueue.size() > VSTRMIO_MAX_QUEUED_SIZE)
	{
		Sleep(DEFAULT_SLEEP_TIME);
	}

	SetEvent(_hNotify);

	return true;
}

bool VstrmIORender::begin(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender::begin() enter");
	
	// To avoid invoking begin() during its processing
	if(_processStatus == ACTIVE)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: The task did not complete yet, can not initial new work");

		return false;
	}

	// check the begin is invoked by pause - begin or just begin
	if(_processStatus != PAUSED)
	{	
		// clear the properties, it includes bitrate, which passed from its source filter.
		_srcProperty.clear();
		_streamableEvt = false;

		// check the static vstrm handle which initialized in construction function
		if(INVALID_HANDLE_VALUE == _hVstrm )	
		{
			_graph.setLastError(ERRCODE_VSTRM_INIT_FAIL, _vstrmErrMsg);
			return false;
		}
		
		// reset the variables
		_tidAbortBy = 0;
		_bEndOfStream = false;

		_startTickCount = GetTickCount();

		// make sure to empty the queue
		emptyDataQueue();

		// initialize the output sub files
		if(!initSubFiles())
		{
			// uninit sub files - close handle, release bandwidth
			uninitSubFiles(true);
			
			return false;
		}
	}
	else
	{
		// resume the thread
		start();
	}
	
	_processStatus = ACTIVE;
	
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender::begin() leave");

	return true;
}

bool VstrmIORender::pause(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender::pause() enter");

	_processStatus = PAUSED;

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender::pause() leave");
	
	// suspend the thread
	suspend();
		
	return true;
}

bool VstrmIORender::abort(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender::abort() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = ABORTED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender::abort() leave");
	
	return true;
}

void VstrmIORender::stop(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender::stop() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = STOPPED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender::stop() leave");
}

void VstrmIORender::quit(void)
{
	SetEvent(_hStop);
}

void VstrmIORender::endOfStream(void)
{
	// the up render should not call endofstream once event vstrm io render has multiple subfile
	_bEndOfStream = true;
	SetEvent(_hNotify);
}

int VstrmIORender::run(void)
{	
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "VstrmIORender::run() enter");

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
			bContinue = false;
			_graph.writeLog(ZQ::common::Log::L_INFO, id(), "VstrmIORender: get a thread exit event");
			break;

		// received the Notify event
		case WAIT_OBJECT_0 + 1:
		{		
			// check whether this thread is abort by Graph
			if(STOPPED == _processStatus || ABORTED == _processStatus)
			{
				// make sure to remove, 
				emptyDataQueue();

				bool delErrorFile = false;
				if(ABORTED == _processStatus)
				{
					// RemoveFile
					delErrorFile = true;

					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "VstrmIORender: It was aborted by Graph, trigger by thread 0x%08X", _tidAbortBy);
				}

				// close the file handle
				uninitSubFiles(delErrorFile);
				
				// notify graph this filter processing completed
				_graph.notifyCompletion(*this);

				continue;
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

			// write the buff to disk
			if(pBuffData != NULL)
			{			
				if(dwCounter % DEFAULT_FILETER_LOGING_FEQ == 0)
				{
					if(0 == dwCounter )
					{
						_startTickCount = GetTickCount();
					}
					
					_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "VstrmIORender: processBuffer No.%d, there are %d buffers in queue", 
						dwCounter, _dataQueue.size());
				}
				
				// get the sub file handle
				DWORD fileNo = 0;
				if(_subFiles.size() > 1)
				{
					ZQ::common::Variant subFileNo = fileNo;
					if(pBuffData->getProperty(CNTPRY_SUBFILE_NO, subFileNo))
					{
						fileNo = (DWORD)subFileNo;
						if(fileNo<0 || fileNo>=_subFiles.size())
						{						
							dwCounter = 0;
							onProcessFailed();
							continue;
						}
					}
				}

				// proces the buff
				if(!_subFiles[fileNo]->outputBuffer(pBuffData))
				{					
					dwCounter = 0;
					onProcessFailed();

					continue;		
				}
				
				bool bReleased = releaseBuffer(pBuffData);
				if(bReleased)
				{
					_graph.traceLog(id(), "VstrmIORender: free buffData from pool. [BuffData Address: 0x%08X]", 
									pBuffData);
				}

				if(!_streamableEvt)
				{
					ZQ::common::Variant varBitrate = 0;
					if(getProperty(CNTPRY_BITRATE, varBitrate))
					{
						DWORD dwBitrate = (DWORD)varBitrate;
						if(dwBitrate > 0)
						{
							if( ( (_subFiles[_mainFileNo]->_processedBytes * 8) / dwBitrate) >= _streamablePlaytime)
							{
								__int64 expectedSize = ((__int64)dwBitrate * (__int64)_streamablePlaytime) / (__int64)8;

								_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "VstrmIORender: fire streamable event"
									"after received %I64d bytes, in case of larger than %I64d bytes (%d bps * %d seconds). Actual delay is %d ms", 
									_subFiles[_mainFileNo]->_processedBytes, expectedSize, 
									dwBitrate, _streamablePlaytime, (GetTickCount() - _startTickCount) );
								
								_streamableEvt = true;

								// fire stream event to Graph
								_graph.notifyStateChange(*this, Graph::CNT_STEAMABLE);
							}
						}
					}
				}

				dwCounter++;
				// go on next loop
				ZQ::common::MutexGuard guard(_dataMutex);
				if(!_dataQueue.empty())
				{
					SetEvent(_hNotify);
					continue;
				}
			}
			if(_bEndOfStream)
			{	
				for(DWORD i=0; i<_subFiles.size(); i++)
				{
					// process last buffer
					if(!_subFiles[i]->outputLeftBuffer())
					{
						onProcessFailed();
						continue;
					}

					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "VstrmIORender[%s]: reaches the end of stream, there are %d buffer processed in all, total output size is %I64d bytes", 
									_subFiles[i]->_outputFileName.c_str(), _subFiles[i]->_buffCount, _subFiles[i]->_processedBytes);
				}

				dwCounter = 0;
				// on succeed
				onProcessSucceed();
				
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
	
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "VstrmIORender::run() leave");
	
	return 1;
}

void VstrmIORender::onProcessSucceed()
{
	// report support file size
	updateSupportFileSize();

	// report MD5 checksum
	updateMD5Property();

	// notify all the connected renders that reaches the end of stream
	notifyEndOfStream();

	// close the file handle
	uninitSubFiles();

	// make sure to remove, 
	emptyDataQueue();
	
	// set the status
	_processStatus = STOPPED;
	
	// notify graph this filter processing completed
	_graph.notifyCompletion(*this);
}

void VstrmIORender::onProcessFailed()
{
	std::string errstr;
	getSystemErrorText(errstr);

	char msg[256];
	sprintf(msg, "VsWrite() failed with error: %s", errstr.c_str());
	_graph.setLastError(ERRCODE_WRITEFILE_FAIL, msg);

	// make sure to remove, 
	emptyDataQueue();

	// close the file handle
	uninitSubFiles(true);
										
	_processStatus = ABORTED;
	_bEndOfStream = false;

	// the source met problem, abort all the renders in the graph
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "VstrmIORender: %s, trigger Graph abort", msg);
	_graph.abortProvision();
}

bool VstrmIORender::initMyVstrm(std::string& errmsg)
{
	// the handle is passed outside
	if(_hVstrm != INVALID_HANDLE_VALUE)
	{
		return true;
	}
	
	_hVstrm = initVstrm(errmsg, _bRunningOnNode);

	if(INVALID_HANDLE_VALUE == _hVstrm)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender::initVstrm() failed with reason: %s", errmsg.c_str());
	}
	return _hVstrm != INVALID_HANDLE_VALUE;
}

bool VstrmIORender::uninitMyVstrm()
{
	// the handle is passed outside
	if(!_initedLocal || _hVstrm == INVALID_HANDLE_VALUE)
	{
		return true;
	}

	return uninitVstrm(_hVstrm);
}

HANDLE VstrmIORender::initVstrm(std::string& errmsg, bool runningOnNode, std::string  nodeName)
{

	VSTATUS vStatus;
	char szBuf[255] = {0};

	HANDLE hvstrm = INVALID_HANDLE_VALUE;
	vStatus = VstrmClassOpenEx(&hvstrm);
	if (vStatus != VSTRM_SUCCESS) 
	{
		VstrmClassGetErrorText(hvstrm, vStatus, szBuf, sizeof(szBuf));

		errmsg = szBuf;

		return INVALID_HANDLE_VALUE;
	} 
	
	// running on MediaCluster Node, additional codes to bind
	if(!runningOnNode)
	{	
		char szNodeName[33];
		strcpy(szNodeName, nodeName.c_str());
		vStatus = VstrmClassBind(hvstrm, szNodeName);
		
		if(vStatus != VSTRM_SUCCESS)
		{
			VstrmClassGetErrorText(hvstrm, vStatus, szBuf, sizeof(szBuf));
						
			errmsg = szBuf;

			return INVALID_HANDLE_VALUE;
		}
	}
	
	return hvstrm;
}


bool VstrmIORender::uninitVstrm(HANDLE hvstrm)
{
	if(hvstrm != INVALID_HANDLE_VALUE)
	{
		// It is not recommended to invoke VstrmClassCloseEx function, but seems for streaming
		// but in case of !bRunningOnMC, must invoked
		VstrmClassCloseEx(hvstrm);
	}

	return true;
}

bool VstrmIORender::releaseAllBW(DWORD bwmgrClientId, std::string& errstr, HANDLE vstrm)
{
	VSTATUS vStatus;
	char szBuf[255] = {0};
	HANDLE tmpvstm = vstrm;

	// create handle if not specified
	bool releasehandel = false;
	if(INVALID_HANDLE_VALUE == tmpvstm)
	{
		vStatus = VstrmClassOpenEx(&tmpvstm);
		if (vStatus != VSTRM_SUCCESS) 
		{
			VstrmClassGetErrorText(tmpvstm, vStatus, szBuf, sizeof(szBuf));

			errstr = szBuf;

			return false;
		} 
		releasehandel = true;
	}

	// create handle if not specified
	VSTATUS status = VstrmClassReleaseAllBandwidth(tmpvstm, bwmgrClientId, 0);

	if (status != VSTRM_SUCCESS)
	{
		char szBuf[255] = {0};
		VstrmClassGetErrorText(tmpvstm, status, szBuf, sizeof(szBuf));

		errstr = szBuf;

		return false;
	}

	// release handle if not specified
	if(releasehandel && tmpvstm != INVALID_HANDLE_VALUE)
	{
		VstrmClassCloseEx(tmpvstm);
	}

	return true;
}


VstrmIORender::SubFile::SubFile(int fileNo, HANDLE vstramHandle, DWORD bwmgrClientId, std::string cachePath, 
								bool pacing, bool disableBufDrvThrottle, Graph& graph)
: _hVstrm(vstramHandle), _bwmgrClientId(bwmgrClientId), _graph(graph), _cachePath(cachePath), 
_pacing(pacing), _disableBufDrvThrottle(disableBufDrvThrottle)
{
	_fileNo = fileNo;
	_extension = "";
	_outputFileName = ""; 
	_hOutputFile = INVALID_HANDLE_VALUE;
	_bwTicket = 0;
	_bwPercentageN = 1; 
	_bwPercentageD = 1;
	_reservedBW = 0;
	_processedBytes = 0;
	_buffCount = 0;
	_enableMD5Checksum = false;
	_cacheBufferSize = 0;
	_cacheBuffer = NULL;
	_cacheCurLength = 0;

	_vvxByTrickGen = false;
	_offset.QuadPart = 0;
}

VstrmIORender::SubFile::~SubFile()
{
	if(_cacheBuffer != NULL)
	{
		delete []_cacheBuffer;
		_cacheBuffer = NULL;	
	}
}

void VstrmIORender::SubFile::setCacheBufferSize(DWORD size)
{
	if(_cacheBuffer != NULL)
	{
		delete []_cacheBuffer;
		_cacheBuffer = NULL;
		_cacheBufferSize = 0;
	}

	if(size != 0)
	{
		_cacheBufferSize = size;
		_cacheBuffer = new char[_cacheBufferSize];
	}
}

bool VstrmIORender::SubFile::init(std::string cntName, DWORD maxbps)
{
	// reset
	_processedBytes = 0;
	_buffCount = 0;
	_cacheCurLength = 0;

	_offset.QuadPart = 0;

	_hOutputFile = NULL;
	_ntfsCacheHandle = NULL;
	
	// set output file name
	_outputFileName = cntName + _extension;		

	// reset checksum
	if(_enableMD5Checksum)
	{
		_md5ChecksumUtil.reset();
	}

	// reset pacing index context
	_pacingIndexCtx = NULL;
	_ntfsTempHandle = NULL;
	_vvxByteRead = 0;
	_lastReadSize = 0;
	
	_posCacheWrite.QuadPart = 0;
	_posCacheRead.QuadPart = 0;	

	// if no cache, just cache the file in NTFS, when _cacheBufferSize == 0 means the sub file is index file
	if(0 == _cacheBufferSize)
	{
		std::string filePath = _cachePath + _outputFileName;
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: Create output file %s in local disk", 
						filePath.c_str());

		_ntfsCacheHandle = CreateFileA(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

		if (INVALID_HANDLE_VALUE == _ntfsCacheHandle)
		{
			std::string errmsg;
			getSystemErrorText(errmsg);

			_graph.setLastError(ERRCODE_CREATEFILE_FAIL, errmsg);

			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: CreateFile failed with error: %s", errmsg.c_str());
			return false;
		}
		if(_pacing)
		{
			// seems Windows does not allow create and write and read handle to one file in one process/thread.
			// so have to share the same handle(read/write) and remember the read/write position.
	/*
			// create ntfs read file handle for pacing
			_ntfsTempHandle = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
			if (INVALID_HANDLE_VALUE == _ntfsTempHandle)
			{
				std::string errmsg;
				getSystemErrorText(errmsg);

				_graph.setLastError(ERRCODE_CREATEFILE_FAIL, errmsg);

				_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: CreateFile for pacing failed with error: %s", errmsg.c_str());
				return false;
			}
	*/
			// create VstrmIO file handle for pacing
#ifdef FILE_FLAG_CACHED
			_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: Create output file %s in Vstrm with VsOpenEx()", 
							_outputFileName.c_str());

			VSTATUS vstat = VsOpenEx( &_hOutputFile,
									(char*)_outputFileName.c_str(), 
									GENERIC_WRITE,
									FILE_SHARE_READ | FILE_SHARE_WRITE,
									CREATE_ALWAYS,
									FILE_FLAG_CACHED, 
									NULL,
									&_objectId);
#else
			_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: Create output file %s in Vstrm with VsOpen()", 
							_outputFileName.c_str());

			VSTATUS vstat = VsOpen( &_hOutputFile,
									(char*)_outputFileName.c_str(), 
									GENERIC_WRITE,
									FILE_SHARE_READ | FILE_SHARE_WRITE,
									CREATE_ALWAYS,
									NULL,
									&_objectId);

#endif

			if(!IS_VSTRM_SUCCESS(vstat))
			{
				std::string errstr;
				getSystemErrorText(errstr);
				
				char msg[256];

#ifdef FILE_FLAG_CACHED
				sprintf(msg, "VsOpenEx() failed with error: %s", errstr.c_str());
#else
				sprintf(msg, "VsOpen() failed with error: %s", errstr.c_str());
#endif

				_graph.setLastError(ERRCODE_VSTRM_INIT_FAIL, msg);
				
				_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: Failed to create output file: %s with error: %s",
								_outputFileName.c_str(), errstr.c_str());
				return false;
			}

			// disable BufDrv throttling to have better Vstrm IO performance
			if(_disableBufDrvThrottle)
			{
				std::string errmsg;
				if(VstrmIORender::disableBufDrvThrottle(_hVstrm, _hOutputFile, _objectId, errmsg))
				{
					_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: BufDrv throttling enabled");
				}
				else
				{
					_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: failed to enalbe BufDrv throttling with error: %s", errmsg.c_str());
				}
			} // end of if(_disableBufDrvThrottle)
		} // end of if(_pacing)
	} // end of if(0 == _cacheBufferSize)
	else
	{
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: Create output file %s in Vstrm", 
						_outputFileName.c_str());

#ifdef FILE_FLAG_CACHED

		VSTATUS vstat = VsOpenEx( &_hOutputFile,
								(char*)_outputFileName.c_str(), 
								GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								CREATE_ALWAYS,
								FILE_FLAG_CACHED, 
								NULL,
								&_objectId);
#else

		VSTATUS vstat = VsOpen( &_hOutputFile,
								(char*)_outputFileName.c_str(), 
								GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								CREATE_ALWAYS,
								NULL,
								&_objectId);
#endif

		if(!IS_VSTRM_SUCCESS(vstat))
		{
			std::string errstr;
			getSystemErrorText(errstr);
			
			char msg[256];

#ifdef FILE_FLAG_CACHED
			sprintf(msg, "VsOpenEx() failed with error: %s", errstr.c_str());
#else
			sprintf(msg, "VsOpen() failed with error: %s", errstr.c_str());
#endif
			_graph.setLastError(ERRCODE_VSTRM_INIT_FAIL, msg);
			
			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: Failed to create output file: %s with error: %s",
							_outputFileName.c_str(), errstr.c_str());
			return false;
		}
		
		// disable BufDrv throttling to have better Vstrm IO performance
		if(_disableBufDrvThrottle)
		{
			std::string errmsg;
			if(VstrmIORender::disableBufDrvThrottle(_hVstrm, _hOutputFile, _objectId, errmsg))
			{
				_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: BufDrv throttling enabled");
			}
			else
			{
				_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: failed to enalbe BufDrv throttling with error: %s", errmsg.c_str());
			}
		} // end of if(_disableBufDrvThrottle)
	}

	// set the reserved bandwidth
	if(maxbps != 0)
	{
		// calculate the bandwidth
		_reservedBW = maxbps; 

		if(_bwPercentageD != 0 && _bwPercentageN != 0)
		{
			_reservedBW = (DWORD)((float)(maxbps * _bwPercentageN) / (float)_bwPercentageD + 0.5);
		}
		else if(0 == _bwPercentageN || 0 == _bwPercentageD)
		{
			_reservedBW = 0;
		}

		// reserve VStrm bandwidth
		if(_bwTicket != 0)
		{
			releaseVstrmBW();
		}
		
		std::string strVstrmErr;
		if(!reserveVstrmBW(strVstrmErr))
		{
			_graph.setLastError(ERRCODE_VSTRM_NO_BANDWIDTH, strVstrmErr);
			return false;
		}
	}
	return true;
}

bool VstrmIORender::SubFile::uninit(bool delFile)
{
	// close file handle
	if(_hOutputFile != INVALID_HANDLE_VALUE)
	{
		if(0 == _cacheBufferSize)
		{
			if(_ntfsCacheHandle != NULL)
			{
				CloseHandle(_ntfsCacheHandle);
				_ntfsCacheHandle = NULL;
			}
			
			if(_pacing)
			{
				if(_ntfsTempHandle != NULL)
				{
					CloseHandle(_ntfsTempHandle);
					_ntfsTempHandle = NULL;
				}
				if(_hOutputFile != NULL)
				{
					VsClose(_hOutputFile, _objectId);
					_hOutputFile = NULL;
				}
			}

			std::string filePath = _cachePath + _outputFileName;

			_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender[%s]: delete local temp file %s", 
				_outputFileName.c_str(), filePath.c_str());
			DeleteFileA(filePath.c_str());

		}
		else
		{
			VsClose(_hOutputFile, _objectId);
		}
		_hOutputFile = INVALID_HANDLE_VALUE;
	}

	// release the reserved VStrm bandwidth 
	releaseVstrmBW();

	// delete error files
	if(delFile)
	{
		delOutputFile();
	}
	// must reset here
	_outputFileName = "";

	return true;
}

bool VstrmIORender::SubFile::delOutputFile()
{
	if("" == _outputFileName)
	{
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: output file %s%s not generated yet, no deletion happen", _graph.getContentName().c_str(), _extension.c_str());
		return true;
	}
	
	// delete the file
	if(VstrmDeleteFile(_hVstrm, _outputFileName.c_str()))
	{
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: file %s was successfully deleted", _outputFileName.c_str());
		return true;
	}
	else
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "VstrmIORender: failed to delete file %s", _outputFileName.c_str());
		return false;
	}
	return true;
}

void VstrmIORender::SubFile::updateMD5Property()
{
	// report MD5 checksum
	if(_enableMD5Checksum)
	{
		const char* md5value = _md5ChecksumUtil.lastChecksum();
		ZQ::common::Variant vcs(md5value);

		ContentProperty checksumcp;
		checksumcp.insert(ContentProperty::value_type(CNTPRY_MD5_CHECKSUM, vcs));
		
		_graph.reportProperty(_outputFileName, checksumcp);
	}
}

bool VstrmIORender::SubFile::outputBuffer(ZQ::Content::BufferData* pBuffData)
{
	DWORD amountWritten = 0;
	DWORD dwActualLen = 0;
	BYTE* pointer = pBuffData->getPointerForRead(dwActualLen);
	
	// calculate MD5 checksum
	if(_enableMD5Checksum)
	{
		_md5ChecksumUtil.checksum((const char*)pointer, dwActualLen);
	}

	_buffCount++;
	_processedBytes += dwActualLen;

	_graph.traceLog(GetCurrentThreadId(), "VstrmIORender[%s]: %d buffer data will be processed, current buffer actual length is %d, total buffsize is %I64d", 
					_outputFileName.c_str(), _buffCount, dwActualLen, _processedBytes);


	// no cache buffer, means now it is writing index file to ntfs cache folder
	if(0 == _cacheBufferSize)
	{
		// reset the write position
		if(_pacing)
		{
			_graph.traceLog(GetCurrentThreadId(), "Set index writer position %I64d", _posCacheWrite.QuadPart);

			if(INVALID_SET_FILE_POINTER == SetFilePointer(_ntfsCacheHandle, _posCacheWrite.LowPart, &_posCacheWrite.HighPart, FILE_BEGIN))
			{
				return false;//
			}
		}

		DWORD lowPos = 0;
		if(!_vvxByTrickGen)
		{
			//
			// index file is generated by RTFLib
			//

			// get the seek position, and move file pointer
			DWORD dwSeekOrigion = SEEK_ORIGIN_CUR;
			ZQ::common::Variant seekOrigion = dwSeekOrigion;
			if(pBuffData->getProperty(CNTPRY_IO_SEEK_ORIGION, seekOrigion))
			{
				long loffset = 0;
				ZQ::common::Variant seekOffset = loffset;
				pBuffData->getProperty(CNTPRY_IO_SEEK_OFFSET, seekOffset);

				dwSeekOrigion = (DWORD)seekOrigion;
				loffset = (long)seekOffset;

				_graph.traceLog(GetCurrentThreadId(), "VstrmIORender[%s]: move file pointer to %s, offset %d at NTFS File Sytem", 
								_outputFileName.c_str(), SEEK_ORIGIN_STR[dwSeekOrigion].c_str(), loffset);
				lowPos = SetFilePointer(_ntfsCacheHandle, loffset, NULL, dwSeekOrigion);

				if(_pacing)
				{
					// remember the write position
					_posCacheWrite.LowPart = lowPos;
				}
			}

			// write to IO
			if(!WriteFile(_ntfsCacheHandle, pointer, dwActualLen, &amountWritten, NULL))
			{
				return false;
			}

			if(_pacing)
			{
				// remember the write position
				_posCacheWrite.QuadPart += amountWritten;
			}
		}
		else
		{
			//
			// index file is generated by old Trick-Gen
			//
	
			// get the type
			DWORD dwVVXTrickGen = TRICK_GEN_VVX_NORMAL;
			ZQ::common::Variant vvxTrickGen = dwVVXTrickGen;
			if(pBuffData->getProperty(CNTPRY_TRICK_GEN_VVX, vvxTrickGen))
			{
				dwVVXTrickGen = (DWORD)vvxTrickGen;
			}
			// flush vvx output to disk and move file pointer
			switch(dwVVXTrickGen)
			{
			case TRICK_GEN_VVX_NORMAL:
				if(!WriteFile(_ntfsCacheHandle, pointer, dwActualLen, &amountWritten, NULL))
				{
					return false;
				}

				_offset.QuadPart += amountWritten;

				if(_pacing)
				{
					// remember the write position
					_posCacheWrite = _offset;
				}
				break;
			case TRICK_GEN_VVX_FLUSH:
				if(!WriteFile(_ntfsCacheHandle, pointer, dwActualLen, &amountWritten, NULL))
				{
					return false;
				}

				lowPos = SetFilePointer(_ntfsCacheHandle, _offset.LowPart, &_offset.HighPart, FILE_BEGIN);
				
				if(_pacing)
				{
					// remember the write position
					_posCacheWrite.LowPart = lowPos;
					_posCacheWrite.HighPart = _offset.HighPart;
				}

				break;
			case TRICK_GEN_VVX_HEADER:
				{
					// this case only happens when the trick file is completing, 
					// so even WriteFile failed, do not abort
					LARGE_INTEGER off1, off2;
					off1.QuadPart = off2.QuadPart = 0;

					off1.LowPart = SetFilePointer(_ntfsCacheHandle, off2.LowPart, &off1.HighPart,FILE_CURRENT);
					SetFilePointer(_ntfsCacheHandle, off2.LowPart, &off2.HighPart, FILE_BEGIN);
					
					WriteFile(_ntfsCacheHandle, pointer, dwActualLen, &amountWritten, NULL);
					
					lowPos = SetFilePointer(_ntfsCacheHandle, off1.LowPart, &off1.HighPart, FILE_BEGIN);
					
					if(_pacing)
					{
						// remember the write position
						_posCacheWrite.LowPart = lowPos;
						_posCacheWrite.HighPart = off1.HighPart;
					}
				}
				break;
			}
			_graph.traceLog(GetCurrentThreadId(), "Save index writer position: %I64d", _posCacheWrite.QuadPart);

		}

		// read vvx for pacing
		if(_pacing)
		{
			// move file point for reading
			_graph.traceLog(GetCurrentThreadId(), "Set index reader position: %I64d", _posCacheRead.QuadPart);

			if(INVALID_SET_FILE_POINTER == SetFilePointer(_ntfsCacheHandle, _posCacheRead.LowPart, &_posCacheRead.HighPart, FILE_BEGIN))
			{
				return false;
			}
			
			DWORD byteRead = 0;
			if(!ReadFile(_ntfsCacheHandle, _tmpbuffer, DEF_VSTRM_WRITE_SIZE, &byteRead, NULL))
			{
				// log for fail
				return false;
			}

			// if we get a single terminator record, and the other writer is gone,
			// then return the terminator - the next read will be 0 bytes
			if(sizeof(VVX_V7_RECORD) == byteRead)
			{
				VVX_V7_RECORD *pRecord = (VVX_V7_RECORD *)(_tmpbuffer);
				
				if(TRICK_INDEX == pRecord->recordType && 0 == pRecord->trick.timeCode.time)
				{
					LARGE_INTEGER tmp;
					tmp.QuadPart = _vvxByteRead;
					if(0xFFFFFFFF == SetFilePointer(_ntfsCacheHandle, tmp.LowPart, &tmp.HighPart, FILE_BEGIN))
					{
						// log for fail
						return false;
					}
					else
					{
						// remember the position for reading
						// _posCacheRead = tmp; 
						return true;						
					}
				}
			}
			
			// not enough data found in first read
			if(0 == _vvxByteRead)
			{
				if(byteRead < sizeof(VVX_V7_INDEX_HEADER))
				{
					// initial read too shot
					return false;
				}
			}

			// there are two conditions where we'll adjust buffer length
			// and reset the NT file pointer
			// 1: last record is not a complete vvx index record
			// 2: last record is a terminator

			int iPartial;
			if(0 == _vvxByteRead)
			{
				// first read - offset is calculated from the frame data offset
				VVX_V7_INDEX_HEADER *pTmp = (VVX_V7_INDEX_HEADER *)_tmpbuffer;

				iPartial = (byteRead - pTmp->frameDataOffset) % sizeof(VVX_V7_RECORD);
			}
			else
			{
				// not the first read - offset is calculated from the beginning of
				// the buffer because we always reset the file pointer to the start
				// of a record
				iPartial = byteRead % sizeof(VVX_V7_RECORD);
			}

			if(iPartial)
			{
				// reduce buffer length
				byteRead -= iPartial;
				
				// set file pointer
				_vvxByteRead += byteRead;

				LARGE_INTEGER tmp;
				tmp.QuadPart = _vvxByteRead;
				if(0xFFFFFFFF == SetFilePointer(_ntfsCacheHandle, tmp.LowPart, &tmp.HighPart, FILE_BEGIN))
				{
					// log for fail
					return false;
				}
				
				// remember the position for reading
				_posCacheRead = tmp;				
			}
			else
			{
				// check if the last record in the buffer is a terminator
				VVX_V7_RECORD *pRecord = 
					(VVX_V7_RECORD *)(_tmpbuffer + byteRead - sizeof(VVX_V7_RECORD));

				if(TRICK_INDEX == pRecord->recordType && 
					0 == pRecord->trick.timeCode.time)
				{
					// gotcha
					byteRead -= sizeof(VVX_V7_RECORD);

					_vvxByteRead += byteRead;

					LARGE_INTEGER tmp;
					tmp.QuadPart = _vvxByteRead;
					if(0xFFFFFFFF == SetFilePointer(_ntfsCacheHandle, tmp.LowPart, &tmp.HighPart, FILE_BEGIN))
					{
						// log for fail
						return false;
					}
					// remember the position for reading
					_posCacheRead = tmp;
				}
				else
				{
					_vvxByteRead += byteRead;

					// remember the position for reading
					_posCacheRead.QuadPart = byteRead;
				}
			}

			_graph.traceLog(GetCurrentThreadId(), "Save index reader position: %I64d", _posCacheRead.QuadPart);


			if(0 == byteRead)
				return true;

			// log the last index read
			VVX_V7_RECORD *pTmp = 
				(VVX_V7_RECORD *)(_tmpbuffer + byteRead - sizeof(VVX_V7_RECORD));

			_graph.traceLog(GetCurrentThreadId(), 
				"(%s) last index read: %#I64x (%#I64x)(%#I64x)",
				_outputFileName.c_str(),
				pTmp->trick.frameByteOffset[0],
				pTmp->trick.frameByteOffset[1],
				pTmp->trick.frameByteOffset[2]);

			// pass the buffer to pacing
			DWORD paceresult = 0;
			try
			{
				if(PacedIndexWrite(_pacingIndexCtx, byteRead, _tmpbuffer, &paceresult) < 0)
				{
					_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: PacedIndexWrite(%s) failed with error", 
									_outputFileName.c_str(), VstrmIORender::DecodePacedIndexError(paceresult));
					return false;
				}
			}
			catch(...)
			{
					_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: PacedIndexWrite(%s) met unknown exception", 
									_outputFileName.c_str());

				return false;
			}
		}
		return true;
	}
	else   // have cache
	{
		if(0 == _cacheCurLength && _cacheBufferSize == dwActualLen)
		{
			if(_pacing)
			{
				DWORD paceresult = 0;
				if(PacedIndexWrite(_pacingIndexCtx, dwActualLen, (char*)pointer, &paceresult) < 0)
				{
					_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: PacedIndexWrite() failed %s with error", 
									_outputFileName.c_str(), VstrmIORender::DecodePacedIndexError(paceresult));
					return false;
				}
			}
			else
			{
				if(!VsWrite(_hOutputFile, dwActualLen, (char*)pointer, &amountWritten, NULL))
				{
					return false;
				}
			}
		}
		else if(_cacheCurLength + dwActualLen < _cacheBufferSize)
		{
			memcpy(&_cacheBuffer[_cacheCurLength], pointer, dwActualLen);
			_cacheCurLength += dwActualLen;		
		}
		else // if(_cacheCurLength + dwActualLen >= _cacheBufferSize)
		{
			// the _temp64Buff will reach 64K
			DWORD dwLeftBytes = dwActualLen;
			DWORD dwCopyBytes = _cacheBufferSize - _cacheCurLength;			

			int totalCopiedBytes = 0;
			do 
			{
				memcpy(&_cacheBuffer[_cacheCurLength], (pointer+totalCopiedBytes), dwCopyBytes);
				
				totalCopiedBytes += dwCopyBytes;

				if(_pacing)
				{
					DWORD paceresult = 0;
					if(PacedIndexWrite(_pacingIndexCtx, _cacheBufferSize, (char*)_cacheBuffer, &paceresult) < 0)
					{
						_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: PacedIndexWrite() failed %s with error", 
										_outputFileName.c_str(), VstrmIORender::DecodePacedIndexError(paceresult));
						return false;
					}
				}
				else
				{
					// flush the 64 data to disk
					DWORD amountWritten;
					if(!VsWrite(_hOutputFile, _cacheBufferSize, _cacheBuffer, &amountWritten, NULL))
					{
						return false;
					}
				}

				dwLeftBytes = dwLeftBytes - dwCopyBytes;
				dwCopyBytes = dwLeftBytes < _cacheBufferSize ? dwLeftBytes : _cacheBufferSize;


			}while(dwLeftBytes >= _cacheBufferSize);

			// copy the left bytes in pBufferData to cache buffer for later flushing
			memcpy(&_cacheBuffer[0], (pointer+totalCopiedBytes), dwLeftBytes);
			_cacheCurLength = dwLeftBytes;

		}
		return true;
	}
	return false;
}

bool VstrmIORender::SubFile::outputLeftBuffer()
{
	if(0 == _cacheBufferSize) 
	{
		if(_pacing)
		{
			return true;   // if support pacing, no copy anymore
		}

		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender[%s]: copy temp file %s to vstrm ", 
			_outputFileName.c_str(), _outputFileName.c_str());

		std::string errmsg;
		std::string filePath = _cachePath + _outputFileName;
		if(!copyFileToVstrm(filePath, _outputFileName, errmsg, _disableBufDrvThrottle, _hVstrm))
		{
			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender[%s]: failed to copy file from NTFS to vstrm with error: %s", 
				_outputFileName.c_str(), errmsg.c_str());

			return false;
		}
	}
	else if(_cacheCurLength > 0)
	{	
		// flush the last buff to disk
		if(_pacing)
		{
			DWORD paceresult = 0;
			if(PacedIndexWrite(_pacingIndexCtx, _cacheCurLength, (char*)_cacheBuffer, &paceresult) < 0)
			{
				_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: PacedIndexWrite() failed %s with error", 
								_outputFileName.c_str(), VstrmIORender::DecodePacedIndexError(paceresult));
				return false;
			}
		}
		else
		{
			DWORD amountWritten;
			if(!VsWrite(_hOutputFile, _cacheCurLength, _cacheBuffer, &amountWritten, NULL))
			{
				return false;
			}
		}
	}
	return true;
}

bool VstrmIORender::SubFile::pacingWrite(const char* buff, DWORD& len)
{
	_graph.traceLog(GetCurrentThreadId(), "VstrmIORender: pacingWrite buffer size %d bytes for %s", len, _outputFileName.c_str());

	DWORD amountWritten;
	bool ret;
	try
	{
		ret = VsWrite(_hOutputFile, len, (char*)buff,  &amountWritten, NULL);
	}
	catch(...)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: pacingWrite(%s) met unknown exception", 
						_outputFileName.c_str());
		return false;
	}

	len = amountWritten;

	return ret;
}

bool VstrmIORender::SubFile::pacingSeek(__int64 offset)
{
	_graph.traceLog(GetCurrentThreadId(), "VstrmIORender: pacingSeek offset %I64d bytes for %s", offset, _outputFileName.c_str());

	LARGE_INTEGER	tmp;
	tmp.QuadPart = offset;
	
	DWORD dwError = 0;
	DWORD dwPtrLow = 0;
	try
	{
		VSTATUS vStat = VsSeek(_hOutputFile, _objectId, &tmp, FILE_BEGIN);
		if (!IS_VSTRM_SUCCESS(vStat))
		{
			_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: pacingSeek offset %I64d bytes for %s failed", offset, _outputFileName.c_str());
			return false;
		}
	}
	catch (...) 
	{
		return false;
	}

	return true;
}

bool VstrmIORender::SubFile::pacingSetEOF(__int64 offset)
{
	_graph.traceLog(GetCurrentThreadId(), "VstrmIORender: pacingSetEOF offset %I64d bytes for %s failed", offset, _outputFileName.c_str());

	LARGE_INTEGER	tmp;
	tmp.QuadPart = offset;

	VSTATUS vstat = VsSetEndOfFile(_hOutputFile, _objectId, &tmp);
	if (!IS_VSTRM_SUCCESS(vstat))
	{
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: pacingSetEOF offset %I64d bytes for %s", offset, _outputFileName.c_str());
		return false;
	}
	return true;
}

bool VstrmIORender::SubFile::reserveVstrmBW(std::string& errmsg)
{
	if(0 == _bwmgrClientId || 0 == _reservedBW)
	{
		return true;
	}
	
	// reserve VStrm bandwidth
	VSTATUS	statusTicket = ERROR_SUCCESS;
	VSTRM_BANDWIDTH_RESERVE_BLOCK   rbFile = {0};
    PVSTRM_BANDWIDTH_RESERVE_BLOCK	pRbFile=&rbFile;

	// The Bw Mgr considers bandwidth requests
    // to be from the perspective of the PCI Bus, not the disks. So, to get data
    // onto the disks they must READ from the PCI Bus, so ask for READ BW here,
    // even tho we are putting data onto the disks using writes. 
	rbFile.ClientId         = _bwmgrClientId;
	rbFile.Type				= kVSTRM_BANDWIDTH_TYPE_READ;
	rbFile.TargetType		= kVSTRM_BANDWIDTH_TARGETTYPE_FILE;

    rbFile.BwTarget         = (void*)(_outputFileName.c_str()); 
		
 	rbFile.MaxBandwidth		= _reservedBW;	// passed in with request
	rbFile.MinBandwidth		= _reservedBW;	// passed in with request
	rbFile.ReservedBandwidth = NULL;

	statusTicket = VstrmClassReserveBandwidth(_hVstrm, pRbFile, &_bwTicket);

	if (statusTicket != VSTRM_SUCCESS)
	{
		char szBuf[255] = {0};
		VstrmClassGetErrorText(_hVstrm, statusTicket, szBuf, sizeof(szBuf));

		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: VstrmClassReserveBandwidth(BW - %dbps, ClientId - %d) failed with error %s", 
						_reservedBW, _bwmgrClientId, szBuf);		

		errmsg = szBuf;

		_bwTicket = 0;

		return false;
	}

    _graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: VstrmClassReserveBandwidth(BW - %dbps, ClientId - %d) return ticket 0x%I64X",  
					_reservedBW, _bwmgrClientId, _bwTicket);

	return true;
}

bool VstrmIORender::SubFile::releaseVstrmBW()
{	
	if(0 == _bwmgrClientId || 0 == _reservedBW || 0 == _bwTicket)
		return true;
	
	VSTATUS	statusTicket = ERROR_SUCCESS;
	
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: VstrmClassReleaseBandwidth(BW - %dbps) for ticket: 0x%I64X", 
					_reservedBW, _bwTicket);
	statusTicket = VstrmClassReleaseBandwidth(_hVstrm, _bwTicket);

	//  it always return failure: VSTRM_NOT_SUPPORTED even the bandwidth was released indeed
	if (statusTicket != VSTRM_SUCCESS)
	{
		char szBuf[255] = {0};
		VstrmClassGetErrorText(_hVstrm, statusTicket, szBuf, sizeof(szBuf));

		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: VstrmClassReleaseBandwidth(%d pbs) for ticket 0x%I64X failed with error %s", 
						_reservedBW, _bwTicket, szBuf);

		_bwTicket = 0;

		return false;
	}

	_bwTicket = 0;
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: VstrmClassReleaseBandwidth(BW - %dbps) succeed",  _reservedBW);

	return true;
}

bool VstrmIORender::copyFileToVstrm(std::string sourceFile, std::string desFile, std::string& errmsg, bool disableBufDrvThrottle, HANDLE hVstrm)
{	
	OBJECT_ID objectId;
	HANDLE vstrmFileHandle;
	// create Vstrm file handle
	
#ifdef FILE_FLAG_CACHED
	VSTATUS vstat = VsOpenEx(&vstrmFileHandle, 
							(char*)desFile.c_str(), 
							GENERIC_WRITE, 
							FILE_SHARE_READ|FILE_SHARE_WRITE, 
							CREATE_ALWAYS, 
							FILE_FLAG_CACHED, 
							NULL, 
							&objectId);
#else	
	VSTATUS vstat = VsOpen(&vstrmFileHandle, 
							(char*)desFile.c_str(), 
							GENERIC_WRITE, 
							FILE_SHARE_READ|FILE_SHARE_WRITE, 
							CREATE_ALWAYS, 
							NULL, 
							&objectId);
#endif	

	if(!IS_VSTRM_SUCCESS(vstat))
	{
		errmsg = "Failed to create vstrm file handle for " + desFile;

		return false;
	}
	
	// disable BufDrv throttling to have better Vstrm IO performance
	if(disableBufDrvThrottle)
	{
		std::string errstr;
		VstrmIORender::disableBufDrvThrottle(hVstrm, vstrmFileHandle, objectId, errstr);
	}

	DWORD dwBytesRead = 0;
	DWORD amountWritten = 0;	
	char buffer[DEF_VSTRM_WRITE_SIZE];
		
	HANDLE ntfsFileHandle = ::CreateFileA(sourceFile.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	bool succeed = true;
	if(ntfsFileHandle != INVALID_HANDLE_VALUE)
	{
		while(ReadFile(ntfsFileHandle, (LPVOID)buffer, DEF_VSTRM_WRITE_SIZE, &dwBytesRead, NULL) && dwBytesRead > 0)
		{
			if(!VsWrite(vstrmFileHandle, dwBytesRead, buffer, &amountWritten, NULL))
			{
				std::string errstr;
				getSystemErrorText(errstr);

				errmsg = "Vstrm failed to write file %s " + desFile + " with error:" + errstr;
				succeed = false;
				break;
			}
		}
		CloseHandle(ntfsFileHandle);
		ntfsFileHandle = NULL;
	}
	else
	{
		errmsg = "Failed to open file %s " + sourceFile;
	}

	VsClose(vstrmFileHandle, objectId);

	return succeed;
}

void VstrmIORender::getVstrmError(HANDLE hVstrm, std::string& strErr)
{
	if(hVstrm == INVALID_HANDLE_VALUE)
		return;

	char sErrorText[256]={0};

	DWORD lastError = VstrmGetLastError();

	VstrmClassGetErrorText(hVstrm, lastError, sErrorText, 255);
  
	char errcode[24];
	sprintf(errcode, "[%d]", lastError);

    strErr = std::string(sErrorText)+ std::string(errcode);
}

int VstrmIORender::pacingAppWrite(const void * const pCbParam, const int len, const char buf[])
{
	SubFile* subfile = (SubFile*) pCbParam;
	
	DWORD fileNo = subfile->_fileNo;

	DWORD writelen = len;
	if(!subfile->pacingWrite(buf, writelen))
	{
		return -1;
	}
	return writelen;
}

int VstrmIORender::pacingAppSeek(const void * const pCbParam, const LONGLONG offset)
{
	SubFile* subfile = (SubFile*) pCbParam;
	
	return subfile->pacingSeek(offset) ? 0 : -1;
}

int VstrmIORender::pacingAppSetEOF(const void * const pCbParam, const LONGLONG offset)
{
	SubFile* subfile = (SubFile*) pCbParam;
	
	return subfile->pacingSetEOF(offset) ? 0 : -1;
}

void VstrmIORender::pacingAppReportOffsets(const void * const pCbParam, const LONGLONG offset1, const LONGLONG offset2)
{
	UNREFERENCED_PARAMETER(pCbParam);
	UNREFERENCED_PARAMETER(offset1);
	UNREFERENCED_PARAMETER(offset2);
}

#define XX(a,b) {a, b}
char* VstrmIORender::DecodePacedIndexError(const unsigned long err)
{
	char *errString = "unknown error";

	static struct
	{
		unsigned long code;
		char *str;
	} 
	errors[] = 
	{
		PACED_INDEX_ERROR_TABLE
	};

	for (int i = 0; i < (sizeof(errors) / sizeof(errors[0])); i++)
	{
		if (err == errors[i].code)
		{
			errString = errors[i].str;
		}
	}

	return errString;
}

void VstrmIORender::pacingAppLogCbk(const char * const pMsg)
{
	char				buf[1024];
	int					len;
	unsigned long		written = 0;
	
	//
	// If a msg has arrived without a CRLF, give it one now
	//
	len = strlen(pMsg);

	if (len > 1024 - 2)
	{
		len = 1024 - 2;
	}

	memcpy(buf, pMsg, len);
	buf[len] = 0;

	if(_pStaticGraph != NULL)
	{
		_pStaticGraph->traceLog(GetCurrentThreadId(), buf);
	}
}

bool VstrmIORender::disableBufDrvThrottle(HANDLE vstrmHandle, HANDLE fileHandle, OBJECT_ID objectId, std::string& errMsg)
{
	VSTATUS vstat;
	ULONG disableThrottle = 1;
	ATTRIBUTE_ARRAY attributes;
		
	attributes.setAllorFailAll = 1;
	attributes.attributeCount = 1;
	attributes.objectId = objectId;

	attributes.attributeArray[0].setExactly = 1;
	attributes.attributeArray[0].attributeCode = VSTRM_ATTR_GEN_OVERRIDE_IO_THROTTLE;
	attributes.attributeArray[0].attributeQualifier = 0;
	attributes.attributeArray[0].attributeValueP = &disableThrottle;
	attributes.attributeArray[0].attributeValueLength = sizeof(disableThrottle);

	vstat = VstrmClassSetSessionAttributesEx(fileHandle, objectId, &attributes);

	if (!IS_VSTRM_SUCCESS(vstat))
	{
		// check free space before returning an error - if there's 0 bytes available
		// then the set attributes request will return an INVALID_SESSION error, not a
		// disk space error even though it looks like SeaFile did the right thing
		
		// if Vstrm Class handle does not pass in, create it.
		HANDLE vstrmClass = INVALID_HANDLE_VALUE;
		if(INVALID_HANDLE_VALUE == vstrmHandle)
		{
			vstrmClass = initVstrm(errMsg);
			if(INVALID_HANDLE_VALUE == vstrmClass)
			{
				return false;
			}
		}
		else
		{
			vstrmClass = vstrmHandle;
		}

		LARGE_INTEGER	free, total;
		
		VSTATUS vStatus = VstrmClassGetStorageData(vstrmClass, &free, &total);
		if(IS_VSTRM_SUCCESS(vStatus))
		{
			if(0 == free.QuadPart)
			{
				errMsg = "VstrmClassSetSessionAttributesEx() set session attributes failed, no free space on disk";
			}
			else
			{
				char szBuf[255] = {0};
				VstrmClassGetErrorText(vstrmClass, vstat, szBuf, sizeof(szBuf));
				
				errMsg = "VstrmClassSetSessionAttributesEx() set session attributes failed with error " + std::string(szBuf);
			}
		}

		// release the create VstrmClass handle if it is created here
		if(INVALID_HANDLE_VALUE == vstrmHandle && INVALID_HANDLE_VALUE != vstrmClass)
		{
			uninitVstrm(vstrmClass);
		}

		return false;
	}

	return true;
}




} } }