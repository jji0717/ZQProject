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
// Desc  : Implement the NTFS file IO Render
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/ContentProcess/NTFSFileIORender.cpp 1     10-11-12 15:58 Admin $
// $Log: /ZQProjs/Generic/ContentProcess/NTFSFileIORender.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 27    08-04-09 15:29 Ken.qian
// Support Pacing
// 
// 26    08-03-11 18:03 Ken.qian
// Support supportfilesize
// 
// 25    08-02-25 17:39 Ken.qian
// Support SubFile
// 
// 24    07-10-23 18:28 Ken.qian
// 
// 23    07-10-15 18:12 Ken.qian
// make buffer processing log frequency MICRO based.
// 
// 22    07-09-20 17:59 Ken.qian
// add log while processing buffer
// 
// 21    07-09-17 15:29 Ken.qian
// 1. replace getUnallocatedCount()  with getUsedCount()
// 2. add protection in receive() to avoid too much data queued
// 
// 20    07-09-11 14:29 Ken.qian
// Support TrickFilesLibraryUser index file output
// 
// 19    07-08-16 12:46 Ken.qian
// 
// 18    07-08-09 17:10 Ken.qian
// 
// 17    07-08-01 17:57 Ken.qian
// 
// 16    07-07-23 15:29 Ken.qian
// NTFSFileIORender support subfiles and 64K IO writting
// 
// 15    07-07-19 17:45 Ken.qian
// normal release for rtfcpnode
// 
// 14    07-07-18 16:10 Ken.qian
// 
// 13    07-07-17 14:18 Ken.qian
// 
// 12    07-06-28 15:43 Fei.huang
// 
// 11    07-06-26 17:10 Ken.qian
// 
// 10    07-06-15 13:49 Cary.xiao
// 
// 9     07-06-06 15:37 Ken.qian
// 
// 9     07-06-05 18:25 Ken.qian
// add the detail errorcode
// 
// 8     07-06-01 17:04 Ken.qian
// add checksum
// 
// 7     07-05-14 16:32 Ken.qian
// 
// 6     07-04-26 21:23 Ken.qian
// 
// 5     07-04-26 19:50 Ken.qian
// 
// 4     07-04-20 16:33 Ken.qian
// 
// 3     07-04-20 10:36 Ken.qian
// 
// 2     07-04-19 17:31 Ken.qian
// 
// 1     07-04-16 11:04 Ken.qian
// 
// 7     06-09-06 15:27 Ken.qian
// 
// 6     06-08-24 11:02 Ken.qian
// 
// 5     06-07-13 16:02 Ken.qian
// 
// 4     06-07-12 12:12 Ken.qian
// 
// 3     06-07-04 10:34 Ken.qian
// 
// 2     06-06-30 11:32 Ken.qian
// Change the abort processing workflow
// 
// 1     06-06-26 14:49 Ken.qian
// Initial Implementation
// 

#include "NTFSFileIORender.h"
#include "urlstr.h"
#include "bufferpool.h"

#include "vstrmuser.h"
#include "PacedIndex.h"

#pragma comment(lib, "PacedIndex.lib")

#define NTFSIO_MAX_QUEUED_SIZE  20 
namespace ZQ { 
namespace Content { 
/// namespace Process presents the processing to the content, such as reading/writing
namespace Process {

ZQ::Content::Process::Graph*  NTFSFileIORender::_pStaticGraph = NULL;
	
NTFSFileIORender::NTFSFileIORender(ZQ::Content::Process::Graph& graph, bool enableMD5Checksum, std::string myName,
								   bool delProvedFile, bool delErrorFile, bool pacing)
: Filter(graph, myName), 
_bEndOfStream(true), _enableMD5Checksum(enableMD5Checksum), 
_delProvedFile(delProvedFile), _delErrorFile(delErrorFile), _fpmType(FPMT_NONE), 
_indexFileNo(0), _mainFileNo(0),
_pacing(pacing)
{	
	if(myName == "")
	{
		_myName = "NTFSFileIORender";
	}

	_tidAbortBy = 0;
	
	_hStop = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);

	if(_pStaticGraph != NULL)
	{
		_pStaticGraph = &graph;
	}

	// start the thread
	start();
}

NTFSFileIORender::~NTFSFileIORender(void)
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
	
	// release source file handle 
	uninitSubFiles();
}

void NTFSFileIORender::emptyDataQueue(void)
{
	ZQ::common::MutexGuard guard(_dataMutex);
	// remove all the buffer data pointer from the queue.
	while (!_dataQueue.empty())
	{
		ZQ::Content::BufferData* pBuffData = _dataQueue.front();
		
		_pool.free(pBuffData);
		_dataQueue.pop();

		_graph.traceLog(id(), "NTFSFileIORender: free buffData from pool. [BuffData Address: 0x%08X]", 
					    pBuffData);
	}	
}

void NTFSFileIORender::setHomeDirectory(std::string path)
{
	_szHomeDirectory = path;
	if(!_szHomeDirectory.empty())
	{
		if(_szHomeDirectory[_szHomeDirectory.size()-1] != '/' && 
			_szHomeDirectory[_szHomeDirectory.size()-1] != '\\')
		{
			_szHomeDirectory = _szHomeDirectory + "/";
		}
	}
}

std::string NTFSFileIORender::getHomeDirectory()
{
	return _szHomeDirectory;
}
	
void NTFSFileIORender::setFileExtension(std::string extension)
{
	_szFileExtension = extension;

	if(!_szFileExtension.empty())
	{
		if(_szFileExtension[0] != '.')
		{
			_szFileExtension = "." + _szFileExtension;
		}
	}
}

bool NTFSFileIORender::receive(Filter* upObj, ZQ::Content::BufferData* buff)
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

		_graph.traceLog(id(), "NTFSFileIORender: alloc buffData from pool. [BuffData Address: 0x%08X]", 
						receivedBuff);
	}
	else
	{   // does not copy BuffData
		receivedBuff = buff;
	}

	if(STOPPED == _processStatus || ABORTED == _processStatus)
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "NTFSFileIORender: it is in STOPPED status, does not receive data any more");
	
		bool bReleased = releaseBuffer(receivedBuff, this);	
		if(bReleased)
		{
			_graph.traceLog(id(), "NTFSFileIORender: free buffData from pool. [BuffData Address: 0x%08X]", 
							receivedBuff);
		}
		return false;
	}

	if(_bEndOfStream)
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "NTFSFileIORender: it is end of stream, does not receive data more");

		bool bReleased = releaseBuffer(receivedBuff, this);	
		if(bReleased)
		{
			_graph.traceLog(id(), "NTFSFileIORender: free buffData from pool. [BuffData Address: 0x%08X]", 
							receivedBuff);
		}
		return false;
	}

	// put the data to the queue
	ZQ::common::MutexGuard guard(_dataMutex);
	_dataQueue.push(receivedBuff);
	
	_graph.traceLog(GetCurrentThreadId(), 
		"%s: Receive Buffer Data from up side process object with actual length %d", 
		_myName.c_str(), buff->getActualLength());


	// avoid too much buffer queued here
	if(_dataQueue.size() > NTFSIO_MAX_QUEUED_SIZE)
	{
		Sleep(DEFAULT_SLEEP_TIME);
	}

	SetEvent(_hNotify);

	return true;
}

bool NTFSFileIORender::begin(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender::begin() enter");
	
	// To avoid invoking begin() during its processing
	if(_processStatus == ACTIVE)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIORender: the task did not complete yet, can not initial new work");

		return false;
	}

	//
	// check the begin is invoked by pause - have begun or just begin
	//
	if(_processStatus != PAUSED)    // begin a new job
	{	
		_tidAbortBy = 0;
		
		_bEndOfStream = false;

		// make sure to empty the queue
		emptyDataQueue();

		// create the file handle for later reading data in the thread::run
		if(!initSubFiles())
		{
			uninitSubFiles(true);
			return false;
		}
	}
	else
	{
		// resume the native thread
		start();	
	}
	// resume the thread
	_processStatus = ACTIVE;
		
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender::begin() leave");

	return true;
}

bool NTFSFileIORender::pause(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender::pause() enter");

	_processStatus = PAUSED;

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender::pause() leave");
	
	// suspend the native thread
	suspend();
		
	return true;
}

bool NTFSFileIORender::abort(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender::abort() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = ABORTED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender::abort() leave");
	
	return true;
}

void NTFSFileIORender::stop(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender::stop() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = STOPPED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender::stop() leave");
}

void NTFSFileIORender::quit(void)
{
	SetEvent(_hStop);
}

void NTFSFileIORender::endOfStream(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender: Get end of stream notification");

	_bEndOfStream = true;

	SetEvent(_hNotify);
}

int NTFSFileIORender::run(void)
{	
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "NTFSFileIORender::run() enter");

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
			_graph.writeLog(ZQ::common::Log::L_INFO, id(), "NTFSFileIORender: get a thread exit event");
			break;

		// received the Notify event
		case WAIT_OBJECT_0 + 1:
		{	
			// check whether this thread is abort by Graph
			if(STOPPED == _processStatus || ABORTED == _processStatus)
			{				
				if(ABORTED == _processStatus)
				{
					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "NTFSFileIORender: It was aborted by Graph, triggered by thread 0x%08X", _tidAbortBy);
				}

				// close the file handle
				uninitSubFiles(_delErrorFile);
				
				// make sure to remove, 
				emptyDataQueue();

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
					_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "NTFSFileIORender: processBuffer No.%d, there are %d buffers in queue", 
						dwCounter, _dataQueue.size());
				}
				
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
							// the source met problem, abort all the renders in the graph
							_graph.abortProvision();
							continue;
						}
					}
				}

				// proces the buff
				if(!_subFiles[fileNo]->outputBuffer(pBuffData))				
				{
					// set last error
					std::string errmsg;
					getSystemErrorText(errmsg);

					_graph.setLastError(ERRCODE_WRITEFILE_FAIL, errmsg);
					
					// set the status, must be previous EmptyDataQueue() invoking
					_processStatus = ABORTED;
					_bEndOfStream = false;
					
					_pool.free(pBuffData);
					_graph.traceLog(id(), "NTFSFileIORender: free buffData from pool. [BuffData Address: 0x%08X]", 
									pBuffData);
										
					// release sub file handle
					bool delOutput = false;
					uninitSubFiles(_delErrorFile);

					dwCounter = 0;

					// make sure to remove, 
					emptyDataQueue();

					_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "NTFSFileIORender: WriteFile fail with error %s", errmsg.c_str());

					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "NTFSFileIORender: Met IO Error, trigger Graph abort");
					
					// the source met problem, abort all the renders in the graph
					_graph.abortProvision();
					continue;				
				}

				dwCounter++;

				// free the buffer
				// release the buff 
				bool bReleased = releaseBuffer(pBuffData);
				if(bReleased)
				{
					_graph.traceLog(id(), "NTFSFileIORender: free buffData from pool. [BuffData Address: 0x%08X]", 
									pBuffData);
				}

				_graph.traceLog(id(), "NTFSFileIORender: %d buffer data has been processed, processed size is %I64d bytes", 
								dwCounter, getProcessedStuff());
				
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
				_graph.writeLog(ZQ::common::Log::L_INFO, id(), "NTFSFileIORender: reaches the end of stream, there are %d buffer processed in all, total output size is %I64d bytes", 
								dwCounter, getProcessedStuff());

				__int64 supportFileSize = 0;
				// report MD5 checksum
				for(DWORD i=0; i<_subFiles.size(); i++)
				{
					// report MD5 checksum
					_subFiles[i]->updateMD5Property();

					if(_mainFileNo != i && _indexFileNo != i)
						supportFileSize += _subFiles[i]->_processedBytes;
				}
				
				// update support file size properties
				ZQ::common::Variant vsfs(supportFileSize);

				ContentProperty sfsize;
				sfsize.insert(ContentProperty::value_type(CNTPRY_SUPPORT_FILESIZE, vsfs));
				
				_graph.reportProperty(_graph.getContentName(), sfsize);

				// notify all the connected renders that reaches the end of stream
				notifyEndOfStream();

				// release subfile handles
				uninitSubFiles(_delProvedFile);

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
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "NTFSFileIORender::run() leave");
	
	return 1;
}

void NTFSFileIORender::freeSubFilesObjs(void)
{
	for(DWORD i=0; i<_subFiles.size(); i++)
	{
		SubFile* subfile = _subFiles[i];
		delete subfile;
	}
	_subFiles.clear();
}

void NTFSFileIORender::setSubFileCount(int fileCount)
{
	freeSubFilesObjs();
	
	// create sub file object
	for(int i=0; i<fileCount; i++)
	{
		SubFile* subfile = new SubFile(i, *this, _graph, _pacing);

		_subFiles.push_back(subfile);
	}
}

bool NTFSFileIORender::setSubFileInfo(int fileNo, std::string extension, bool enableMD5, bool progressRpt, FILE_POINT_MOVING_TYPE mtype)
{
	if(fileNo<0 || fileNo>=_subFiles.size())
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "NTFSFileIORender: fileNo %d is invalid", fileNo);
		return false;
	}

	// set the parameter
	_subFiles[fileNo]->_fileNo = fileNo;
	_subFiles[fileNo]->_extension = extension;
	_subFiles[fileNo]->_enableMD5 = enableMD5;
	_subFiles[fileNo]->_filePMT = mtype;
	_subFiles[fileNo]->_isIndexFile = false;

	// set the reporter
	if(progressRpt)
	{
		_progressRptFileNo = fileNo;
	}
	
	if( (stricmp(extension.c_str(), ".VVX") == 0) || (stricmp(extension.c_str(), ".VV2") == 0) )
	{
		_pacingType = extension.substr(1, 3);		
		_indexFileNo = fileNo;
		_subFiles[fileNo]->_isIndexFile = true;
	}
	else if(stricmp(extension.c_str(), "") == 0)
	{
		_mainFileNo = fileNo;
	}

	return true;
}

bool NTFSFileIORender::initSubFiles()
{
	// if no invoking setOutputFileXX function, just by default, one subfile
	if(_subFiles.size() == 0)
	{
		setSubFileCount(1);
		setSubFileInfo(0, _szFileExtension, _enableMD5Checksum, true, _fpmType);

		_progressRptFileNo = 0;
	}

	// compose the file full name
	std::string cntName = _graph.getContentName();
	
	for(DWORD i=0; i<_subFiles.size(); i++)
	{
		if(!_subFiles[i]->init(cntName))
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
				_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIORender: PacedIndexAdd() failed %s with error", 
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
			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIORender: PacedIndexAdd() failed %s with error", 
							_subFiles[_indexFileNo]->_outputFileName.c_str(), DecodePacedIndexError(paceresult));
			return false;	
		}
	}

	return true;
}

void NTFSFileIORender::uninitSubFiles(bool delOutput)
{
	if(!_pacing)  // none pacing
	{	
		for(DWORD i=0; i<_subFiles.size(); i++)
		{
			_subFiles[i]->uninit(delOutput);
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
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender: close file handle for none index file...");
		for(i=0; i<_subFiles.size(); i++)
		{
			if(i != _indexFileNo)
			{
				_subFiles[i]->uninit(delOutput);
			}
		}

		//
		// Remove content files from pacing group so lib knows they're
		// closed and it's OK to write offsets into index
		//
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender: PacedIndexRemove() none index file context...");
		for(i=0; i<_subFiles.size(); i++)
		{
			if(i != _indexFileNo && _subFiles[i]->_pacingIndexCtx != NULL)
			{
				paceresult = PacedIndexRemove(_subFiles[i]->_pacingIndexCtx);
				if (paceresult)
				{
					_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIORender: PacedIndexRemove() failed %s with error", 
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
				_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIORender: PacedIndexGetPacedFileMD5() failed with error", 
								DecodePacedIndexError(paceresult));
			}
			_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender: PacedIndexGetPacedFileMD5() return %s", md5);
		}
		
		//
		// Remove index file
		//
		if(_subFiles[_indexFileNo]->_pacingIndexCtx != NULL)
		{
			_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender: PacedIndexRemove() index file context...");
			paceresult = PacedIndexRemove(_subFiles[_indexFileNo]->_pacingIndexCtx);
			if (paceresult)
			{
				_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIORender: PacedIndexRemove() failed %s with error", 
								_subFiles[_indexFileNo]->_outputFileName.c_str(), DecodePacedIndexError(paceresult));
			}
			_subFiles[_indexFileNo]->_pacingIndexCtx = NULL;
		}
		//
		// Close destination index file
		//
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender: close file handle for index file...");
		_subFiles[_indexFileNo]->uninit(delOutput);		
	}
}


int NTFSFileIORender::pacingAppWrite(const void * const pCbParam, const int len, const char buf[])
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

int NTFSFileIORender::pacingAppSeek(const void * const pCbParam, const LONGLONG offset)
{
	SubFile* subfile = (SubFile*) pCbParam;
	
	return subfile->pacingSeek(offset) ? 0 : -1;
}

int NTFSFileIORender::pacingAppSetEOF(const void * const pCbParam, const LONGLONG offset)
{
	SubFile* subfile = (SubFile*) pCbParam;
	
	return subfile->pacingSetEOF(offset) ? 0 : -1;
}

void NTFSFileIORender::pacingAppReportOffsets(const void * const pCbParam, const LONGLONG offset1, const LONGLONG offset2)
{
	UNREFERENCED_PARAMETER(pCbParam);
	UNREFERENCED_PARAMETER(offset1);
	UNREFERENCED_PARAMETER(offset2);
}

#define XX(a,b) {a, b}
char* NTFSFileIORender::DecodePacedIndexError(const unsigned long err)
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

void NTFSFileIORender::pacingAppLogCbk(const char * const pMsg)
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

NTFSFileIORender::SubFile::SubFile(int fileNo, NTFSFileIORender& ntfsIoRender, Graph& graph, bool pacing)
: _ntfsIoRender(ntfsIoRender), _graph(graph), _pacing(pacing)
{
	_fileNo = 0;
	_extension = "";
	_enableMD5 = false;	
	_outputFileFullName = "";
	_outputFileName = "";
	_outputFileHandle = INVALID_HANDLE_VALUE;
	_filePMT = NTFSFileIORender::FPMT_NONE;
	_offset.QuadPart = 0; 
	_processedBytes = 0;

	_isIndexFile = false;
}

NTFSFileIORender::SubFile::~SubFile()
{

}

bool NTFSFileIORender::SubFile::init(const std::string& contentName)
{
	_offset.QuadPart = 0; 
	_processedBytes = 0;

	_posIndexWrite.QuadPart = 0;
	_posIndexRead.QuadPart = 0;	
	_ntfsCacheHandle = NULL;
	_vvxByteRead = 0;

	// compose the file full name
	_outputFileName = contentName + _extension;
	_outputFileFullName = _ntfsIoRender.getHomeDirectory() + _outputFileName;

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender: The render's destination file is %s", _outputFileFullName.c_str());

	// create the file handle for later reading data in the thread::run
	_outputFileHandle = ::CreateFileA(_outputFileFullName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	if (INVALID_HANDLE_VALUE == _outputFileHandle)
	{
		std::string errmsg;
		getSystemErrorText(errmsg);

		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIORender: CreateFile failed with error: %s", errmsg.c_str());
		return false;
	}

	if(_pacing && _isIndexFile)	
	{
		char sfPath[MAX_PATH] = {0};
		char sfName[MAX_PATH] = {0};;
		
		if (::GetModuleFileNameA(NULL, sfPath, MAX_PATH-1) > 0)
		{
			char* p = strrchr(sfPath, FNSEPC);
			if (NULL != p)
			{
				*p = '\0';

				p = strrchr(sfPath, FNSEPC);
				if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
				{
					*p='\0';
				}
				
				strcat(sfPath, FNSEPS "TempIndexFiles" FNSEPS);
			}
			// create the temp path
			CreateDirectoryA(sfPath, NULL);
			_tempIndexFileName = sfPath + _outputFileName;
		}

		_ntfsCacheHandle = CreateFileA(_tempIndexFileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

		if (INVALID_HANDLE_VALUE == _ntfsCacheHandle)
		{
			std::string errmsg;
			getSystemErrorText(errmsg);

			_graph.setLastError(ERRCODE_CREATEFILE_FAIL, errmsg);

			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIORender: CreateFile failed with error: %s", errmsg.c_str());
			return false;
		}		
	}
	return true;
}

void NTFSFileIORender::SubFile::uninit(bool delFile)
{
	if(_outputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(_outputFileHandle);
		_outputFileHandle = INVALID_HANDLE_VALUE;
	}

	if(_pacing && _ntfsCacheHandle != NULL)
	{
		CloseHandle(_ntfsCacheHandle);
		_ntfsCacheHandle = NULL;

		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender[%s]: delete local temp file %s", 
			_outputFileName.c_str(), _tempIndexFileName.c_str());
		DeleteFileA(_tempIndexFileName.c_str());
	}

	if(delFile)
	{
		if(DeleteFile(_outputFileFullName.c_str()))
		{
			_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender: file %s was successfully deleted", 
				_outputFileFullName.c_str());
		}
		else
		{
			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIORender: failed to delete file %s", 
				_outputFileFullName.c_str());
		}

	}
}

bool NTFSFileIORender::SubFile::outputBuffer(ZQ::Content::BufferData* pBuffData)
{
	// write the buff to disk
	bool bWriteSuccess = true;
	DWORD amountWritten;
	DWORD dwLen = 0;
	DWORD lowPos = 0;
	LPVOID pointer = pBuffData->getPointerForRead(dwLen);
	
	// determine where to output
	HANDLE indexOutputHandle = (_pacing && _isIndexFile) ? _ntfsCacheHandle : _outputFileHandle;

	_processedBytes += dwLen;

	switch(_filePMT)
	{
	case FPMT_NONE:
		// calculate MD5 checksum
		if(_enableMD5)
		{
			_md5ChecksumUtil.checksum((const char*)pointer, dwLen);
		}
		
		if(_pacing)
		{
			DWORD paceresult = 0;
			if(PacedIndexWrite(_pacingIndexCtx, dwLen, (char*)pointer, &paceresult) < 0)
			{
				_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIORender: PacedIndexWrite() failed %s with error", 
								_outputFileName.c_str(), NTFSFileIORender::DecodePacedIndexError(paceresult));
				bWriteSuccess = false;
			}
		}
		else
		{
			bWriteSuccess = WriteFile(_outputFileHandle, pointer, dwLen, &amountWritten, NULL);
		}
		break;
		
	case FPMT_RTF_LIB:
		{
			// reset the write position
			if(_pacing)
			{
				SetFilePointer(indexOutputHandle, _posIndexWrite.LowPart, &_posIndexWrite.HighPart, FILE_BEGIN);
			}

			// moving file pointer
			DWORD dwSeekOrigion = SEEK_ORIGIN_CUR;
			ZQ::common::Variant seekOrigion = dwSeekOrigion;
			if(pBuffData->getProperty(CNTPRY_IO_SEEK_ORIGION, seekOrigion))
			{
				long loffset = 0;
				ZQ::common::Variant seekOffset = loffset;
				pBuffData->getProperty(CNTPRY_IO_SEEK_OFFSET, seekOffset);

				dwSeekOrigion = (DWORD)seekOrigion;
				loffset = (long)seekOffset;

				_graph.traceLog(GetCurrentThreadId(), "NTFSFileIORender: move file pointer to %s, offset %d", 
							SEEK_ORIGIN_STR[dwSeekOrigion].c_str(), loffset);
				lowPos = SetFilePointer(indexOutputHandle, loffset, NULL, dwSeekOrigion);

				if(_pacing)
				{
					// remember the write position
					_posIndexWrite.LowPart = lowPos;
				}

			}
			// flush buffer 
			bWriteSuccess = WriteFile(indexOutputHandle, pointer, dwLen, &amountWritten, NULL);
			
			if(_pacing)
			{
				// remember the write position
				_posIndexWrite.QuadPart += amountWritten;
			}
		}
		break;
		
	case FPMT_TRICKUSER_LIB:
		{
			//
			// index file is generated by Trick-Gen
			//

			// reset the write position
			if(_pacing)
			{
				SetFilePointer(indexOutputHandle, _posIndexWrite.LowPart, &_posIndexWrite.HighPart, FILE_BEGIN);
			}

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
				bWriteSuccess = WriteFile(indexOutputHandle, pointer, dwLen, &amountWritten, NULL);
				_offset.QuadPart += amountWritten;

				if(_pacing)
				{
					// remember the write position
					_posIndexWrite = _offset;
				}

				break;
			case TRICK_GEN_VVX_FLUSH:
				bWriteSuccess = WriteFile(indexOutputHandle, pointer, dwLen, &amountWritten, NULL);
				lowPos = SetFilePointer(indexOutputHandle, _offset.LowPart, &_offset.HighPart, FILE_BEGIN);

				if(_pacing)
				{
					// remember the write position
					_posIndexWrite.LowPart = lowPos;
					_posIndexWrite.HighPart = _offset.HighPart;
				}
				
				break;
			case TRICK_GEN_VVX_HEADER:
				{
					// this case only happens when the trick file is completing, 
					// so even WriteFile failed, do not abort
					LARGE_INTEGER off1, off2;
					off1.QuadPart = off2.QuadPart = 0;

					off1.LowPart = SetFilePointer(indexOutputHandle, off2.LowPart, &off1.HighPart,FILE_CURRENT);
					SetFilePointer(indexOutputHandle, off2.LowPart, &off2.HighPart, FILE_BEGIN);
					
					bWriteSuccess = WriteFile(indexOutputHandle, pointer, dwLen, &amountWritten, NULL);
					
					lowPos = SetFilePointer(indexOutputHandle, off1.LowPart, &off1.HighPart, FILE_BEGIN);

					if(_pacing)
					{
						// remember the write position
						_posIndexWrite.LowPart = lowPos;
						_posIndexWrite.HighPart = off1.HighPart;
					}
				}
				break;
			}
		}
		break;
	}

	// index file
	if(_pacing && _isIndexFile)
	{
		// move file point for reading
		_graph.traceLog(GetCurrentThreadId(), "Set index reader position: %I64d", _posIndexRead.QuadPart);

		SetFilePointer(_ntfsCacheHandle, _posIndexRead.LowPart, &_posIndexRead.HighPart, FILE_BEGIN);
		
		DWORD byteRead = 0;
		if(!ReadFile(_ntfsCacheHandle, _tmpbuffer, MAX_READ_SIZE, &byteRead, NULL))
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
			_posIndexRead = tmp;				
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
				_posIndexRead = tmp;
			}
			else
			{
				_vvxByteRead += byteRead;

				// remember the position for reading
				_posIndexRead.QuadPart = byteRead;
			}
		}

		_graph.traceLog(GetCurrentThreadId(), "Save index reader position: %I64d", _posIndexRead.QuadPart);


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
				_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIORender: PacedIndexWrite(%s) failed with error", 
								_outputFileName.c_str(), NTFSFileIORender::DecodePacedIndexError(paceresult));
				bWriteSuccess = false;
			}
		}
		catch(...)
		{
			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIORender: PacedIndexWrite(%s) met unknown exception", 
							_outputFileName.c_str());

			return false;
		}
		
	}
	
	return bWriteSuccess;
}

void NTFSFileIORender::SubFile::updateMD5Property()
{
	// report MD5 checksum
	if(_enableMD5)
	{
		const char* md5value = _md5ChecksumUtil.lastChecksum();
		ZQ::common::Variant vcs(md5value);

		ContentProperty checksumcp;
		checksumcp.insert(ContentProperty::value_type(CNTPRY_MD5_CHECKSUM, vcs));
		
		_graph.reportProperty(_outputFileName, checksumcp);
	}
}
	
bool NTFSFileIORender::SubFile::pacingWrite(const char* buff, DWORD& len)
{
	_graph.traceLog(GetCurrentThreadId(), "NTFSFileIORender: pacingWrite buffer size %d bytes for %s", len, _outputFileName.c_str());

	DWORD amountWritten;
	bool ret;

	ret = WriteFile(_outputFileHandle, (char*)buff, len, &amountWritten, NULL);

	len = amountWritten;

	return ret;
}

bool NTFSFileIORender::SubFile::pacingSeek(__int64 offset)
{
	_graph.traceLog(GetCurrentThreadId(), "NTFSFileIORender: pacingSeek offset %I64d bytes for %s", offset, _outputFileName.c_str());

	LARGE_INTEGER	tmp;
	tmp.QuadPart = offset;
	
	SetFilePointer(_outputFileHandle, tmp.LowPart, &tmp.HighPart, FILE_BEGIN);

	return true;
}

bool NTFSFileIORender::SubFile::pacingSetEOF(__int64 offset)
{
	_graph.traceLog(GetCurrentThreadId(), "NTFSFileIORender: pacingSetEOF offset %I64d bytes for %s failed", offset, _outputFileName.c_str());

	LARGE_INTEGER	tmp;
	tmp.QuadPart = offset;

	SetFilePointer(_outputFileHandle, tmp.LowPart, &tmp.HighPart, FILE_BEGIN);
	SetEndOfFile(_outputFileHandle);

	return true;
}

} } }