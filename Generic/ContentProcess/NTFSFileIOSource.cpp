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
// Desc  : Implement the NTFS file source for processing
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/ContentProcess/NTFSFileIOSource.cpp 1     10-11-12 15:58 Admin $
// $Log: /ZQProjs/Generic/ContentProcess/NTFSFileIOSource.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 20    08-03-12 13:49 Fei.huang
// GetFileSizeEx for large files
// 
// 19    08-01-02 18:35 Ken.qian
// change since minAvailBuffCount was move to Graph
// 
// 18    07-11-13 17:21 Ken.qian
// 
// 17    07-11-05 11:38 Ken.qian
// set dwCount=0 in abortion
// 
// 16    07-10-23 18:29 Ken.qian
// add available buffer count checking
// 
// 15    07-10-15 18:12 Ken.qian
// make buffer processing log frequency MICRO based.
// 
// 14    07-09-20 17:59 Ken.qian
// add log while processing buffer
// 
// 13    07-08-16 12:46 Ken.qian
// 
// 12    07-08-09 17:10 Ken.qian
// 
// 11    07-08-01 17:57 Ken.qian
// 
// 10    07-07-23 15:29 Ken.qian
// vstrmiorender support subfiles and 64K IO writting
// 
// 9     07-07-19 17:45 Ken.qian
// normal release for rtfcpnode
// 
// 8     07-07-17 14:18 Ken.qian
// 
// 7     07-06-28 15:43 Fei.huang
// 
// 6     07-06-27 15:13 Ken.qian
// Change kbps to bps
// 
// 5     07-06-26 17:10 Ken.qian
// 
// 4     07-06-15 13:49 Cary.xiao
// 
// 3     07-06-06 15:37 Ken.qian
// 
// 3     07-06-05 18:25 Ken.qian
// add the detail errorcode
// 
// 2     07-06-01 17:04 Ken.qian
// add report filesize property to Graph
// 
// 1     07-05-29 17:10 Ken.qian
// 
// 7     06-09-06 15:27 Ken.qian
// 
// 6     06-08-24 11:02 Ken.qian
// 
// 5     06-07-17 18:31 Fei.huang
// 
// 4     06-07-12 18:56 Ken.qian
// 
// 3     06-07-12 12:19 Ken.qian
// 
// 2     06-06-30 11:32 Ken.qian
// Change the abort processing workflow
// 
// 1     06-06-26 14:49 Ken.qian
// Initial Implementation
// 


#include "NTFSFileIOSource.h"
#include "urlstr.h"
#include "bufferpool.h"

namespace ZQ { 
namespace Content { 
/// namespace Process presents the processing to the content, such as reading/writing
namespace Process {

static bool fixpath(std::string& path, bool normal2back)
{
	char* pathbuf = new char[path.length() +2];
	if (NULL ==pathbuf)
		return false;

	strcpy(pathbuf, path.c_str());
	pathbuf[path.length()] = '\0';
	for (char* p = pathbuf; *p; p++)
	{
		if ('\\' == *p || '/' == *p)
		{
			if(normal2back)
			{
				*p = '/';
			}
			else
			{
				*p = '\\';
			}
		}
	}
	
	path = pathbuf;
	
	delete []pathbuf;

	return true;
}

static unsigned long timeval()
{
	unsigned long rettime = 1;

	FILETIME systemtimeasfiletime;
	LARGE_INTEGER litime;

	GetSystemTimeAsFileTime(&systemtimeasfiletime);
	memcpy(&litime,&systemtimeasfiletime,sizeof(LARGE_INTEGER));
	litime.QuadPart /= 10000;  //convert to milliseconds
	litime.QuadPart &= 0xFFFFFFFF;    //keep only the low part
	rettime = (unsigned long)(litime.QuadPart);

	return rettime;
}
	
NTFSFileIOSource::NTFSFileIOSource(ZQ::Content::Process::Graph& graph, std::string myName)
: SourceFilter(graph, SRCFILTER_PROTO_FILE, myName), _hSourceFile(INVALID_HANDLE_VALUE), _processedBytes(0)
{	
	_tidAbortBy = 0;

	// get the default read size if it is not set by setReadSize()
	_readSize = _pool.getPreAllocBufferSize();
	
	if(myName == "")
	{
		_myName = "FTPFileIOSource";
	}

	_hStop = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);

	// start the thread
	start();
}

NTFSFileIOSource::~NTFSFileIOSource(void)
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
	releaseFileHandle();
}

void NTFSFileIOSource::releaseFileHandle(void)
{
	// release the file handle
	if(_hSourceFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(_hSourceFile);
	}
	_hSourceFile = INVALID_HANDLE_VALUE;	
}


bool NTFSFileIOSource::receive(Filter* upObj, ZQ::Content::BufferData* buff)
{
	// to the source, no implementation of the receive
	return true;
}

bool NTFSFileIOSource::begin(void)
{
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "NTFSFileIOSource::begin() enter");
	
	// To avoid invoking begin() during its processing
	if(_processStatus == ACTIVE)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIOSource: The task did not complete yet, can not initial new work");

		return false;
	}

	// check the begin is invoked by pause - begin or just begin
	if(_processStatus != PAUSED)
	{		
		_tidAbortBy = 0;
		_fileSize = 0;
		_processedBytes = 0;

		// get the max bandwidth in Kbps
		_maxbps = _graph.getMaxbps();

		// check whether the URL is a legal one
		std::string srcPath = _graph.getSourceURL();
		fixpath(srcPath, true);

		ZQ::common::URLStr srcUrl(srcPath.c_str());

		if (stricmp("file", srcUrl.getProtocol()) != 0)
		{
			char errmsg[256];
			sprintf(errmsg, "The source URL %s is not file protocol", srcPath.c_str());
			_graph.setLastError(ERRCODE_UNSUPPORT_PROTO, errmsg);
			
			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIOSource: The source URL %s is not a legal file protocol", srcPath.c_str());
			return false;
		}
		
		// fix the file path
		std::string host = srcUrl.getHost();
		srcPath = srcUrl.getPath();

		if (host.empty() || 0 == host.compare("."))
		{
			fixpath(srcPath, true);
		}
		else
		{
			srcPath = std::string(LOGIC_FNSEPS LOGIC_FNSEPS) + srcUrl.getHost() + LOGIC_FNSEPS +srcPath; 
			fixpath(srcPath, false);
		}

		// create the file handle for later reading data in the thread::run
		_hSourceFile = ::CreateFileA(srcPath.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if (INVALID_HANDLE_VALUE == _hSourceFile)
		{
			std::string errmsg;
			getSystemErrorText(errmsg);
			_graph.setLastError(ERRCODE_OPENFILE_FAIL, errmsg);

			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "NTFSFileIOSource: Failed to open source file: %s with error: %s", srcPath.c_str(), errmsg.c_str());
			return false;
		}
// 		DWORD dwLowSize = 0;
// 		DWORD dwHighSize = 0;
// 		dwLowSize = GetFileSize(_hSourceFile, &dwHighSize);
// 		_fileSize = (dwHighSize << 32) + dwLowSize;
		LARGE_INTEGER fSize;
		if(!GetFileSizeEx(_hSourceFile, &fSize)) {
			std::string errmsg;
			getSystemErrorText(errmsg);
			_graph.setLastError(ERRCODE_OPENFILE_FAIL, errmsg);

			_graph.writeLog(ZQ::common::Log::L_ERROR, 
					GetCurrentThreadId(), 
					"NTFSFileIOSource: Failed to get size for %s with error: %s", 
					srcPath.c_str(), errmsg.c_str());
		}
		else {
			_fileSize = fSize.QuadPart;
		}


		// update the FileSize to Graph
		ContentProperty filesizecp;
		filesizecp.insert(ContentProperty::value_type(CNTPRY_FILESIZE, _fileSize));
		_graph.reportProperty(_graph.getContentName(), filesizecp);

		_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "NTFSFileIOSource: The Source file %s size is %I64d bytes", 
					srcPath.c_str(), _fileSize);
	}
	else
	{
		// resume the thread
		start();
	}
	_processStatus = ACTIVE;
	
	// initial the executing of run function
	_lasttimer = timeval();

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "NTFSFileIOSource::begin() leave");

	SetEvent(_hNotify);
	
	return true;
}

bool NTFSFileIOSource::pause(void)
{
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "NTFSFileIOSource::pause() enter");

	_processStatus = PAUSED;

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "NTFSFileIOSource::pause() leave");
	
	// suspend the thread
	suspend();
		
	return true;
}

bool NTFSFileIOSource::abort(void)
{
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "NTFSFileIOSource::abort() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = STOPPED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "NTFSFileIOSource::abort() leave");
	
	return true;
}

void NTFSFileIOSource::stop(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender::stop() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = STOPPED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "NTFSFileIORender::stop() leave");
}

void NTFSFileIOSource::quit(void)
{
	SetEvent(_hStop);	
}

int NTFSFileIOSource::run(void)
{	
	_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "NTFSFileIOSource::run() enter");
		
	bool bContinue = true;
	DWORD dwWaitStatus = 0;
	_processedBytes = 0;

	HANDLE handles[2] = { _hStop, _hNotify };

	DWORD dwCounter = 0;
	
	_lasttimer = timeval();

	while(bContinue)
	{
		dwWaitStatus = WaitForMultipleObjects(2, handles, false, INFINITE);
		switch(dwWaitStatus)
		{
		// received stop event, the thread will stop
		case WAIT_OBJECT_0:
			_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "NTFSFileIOSource: get a thread exit event");
			bContinue = false;
			break;

		// received the Notify event
		case WAIT_OBJECT_0 + 1:
		{
			// control the avaliable buffer count
			while( (STOPPED != _processStatus || ABORTED != _processStatus)
				&& (_pool.getPoolSize() - _pool.getUsedCount()) < _graph.getReservedBuffCount() )
			{
				Sleep(DEFAULT_SLEEP_TIME);
			}
			
			// check whether this thread is abort by Graph
			if(STOPPED == _processStatus || ABORTED == _processStatus)
			{
				dwCounter = 0;
				// release source file handle 
				releaseFileHandle();
				
				if(ABORTED == _processStatus)
				{
					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "NTFSFileIOSource: It was aborted by Graph, triggered by thread 0x%08X", _tidAbortBy);
				}
				
				// notify graph this filter processing completed
				_graph.notifyCompletion(*this);
				continue;
			}

			ZQ::Content::BufferData* pBuffData = NULL;
			if(ACTIVE == _processStatus)
			{
				pBuffData = _pool.alloc(DEFAULT_BUFF_WAIT_TIMEOUT);
				if(NULL == pBuffData)
				{				
					// trigger the loop to fetching data
					SetEvent(_hNotify);
					continue;
				}
				_graph.traceLog(id(), "NTFSFileIOSource: alloc buffData from pool. [BuffData Address: 0x%08X]", pBuffData);
			}

			DWORD dwBytesRead = 0;
			// if this thread is abort in other thread, the status is set to be STOPPED
			if(!ReadFile(_hSourceFile, (LPVOID)pBuffData->getPointerForWrite(_readSize), _readSize, &dwBytesRead, NULL))
			{
				// set last error
				std::string errmsg;
				getSystemErrorText(errmsg);

				_graph.setLastError(ERRCODE_READFILE_FAIL, errmsg);

				dwCounter = 0;

				// release the buffdata
				if(pBuffData != NULL)
				{
					_pool.free(pBuffData);
				}

				// release source file handle 
				releaseFileHandle();
				
				_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "NTFSFileIOSource: ReadFile failed with error: %s", errmsg.c_str());
				_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "NTFSFileIOSource: Met IO Error, trigger Graph abort()");

				_processStatus = STOPPED;

				_graph.abortProvision();

				continue;
			}
		
			if(dwCounter % DEFAULT_FILETER_LOGING_FEQ == 0)
			{
				_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "NTFSFileIOSource: processBuffer No.%d", 
					dwCounter);
			}
			
			// process the read data
			if(dwBytesRead != 0)
			{
				// set the actual length of the buffer
				pBuffData->setActualLength(dwBytesRead);
				
				// pass the buff data to renders
				deliverBuffer(pBuffData);
				
				// release the buffdata
				bool bReleased = releaseBuffer(pBuffData);
				if(bReleased)
				{
					_graph.traceLog(id(), "NTFSFileIOSource: free buffData from pool. [BuffData Address: 0x%08X]", pBuffData);
				}
				
				dwCounter++;

				_graph.traceLog(id(), "NTFSFileIOSource: %d buffer data has been processed", dwCounter);

				// control the data read speed
				_processedBytes += dwBytesRead;
				if (_maxbps > 0)
				{
					DWORD expected = (DWORD)((_processedBytes * 8 * 1024) / _maxbps ); // in msec

					DWORD now = timeval();
					DWORD interval = (now > _lasttimer) ? (now - _lasttimer) : 0;
					
					if (interval < expected)
					{
						_graph.traceLog(id(), "NTFSFileIOSource: processed %I64d bytes expected %d ms but used %d ms, will sleep for %d ms before reading next buff", 
							_processedBytes, expected, interval, (expected-interval));
						::Sleep(expected - interval);
					}
				}
			}

			if(dwBytesRead < _readSize)
			{
				// release the buffdata
				if(0 == dwBytesRead)
				{
					bool bReleased = releaseBuffer(pBuffData, this);
					if(bReleased)
					{
						_graph.traceLog(id(), "NTFSFileIOSource: free buffData from pool. [BuffData Address: 0x%08X]", pBuffData);
					}
				}

				_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "NTFSFileIOSource: reaches the end of stream, there are %d buff read, the last buffer size is %d", dwCounter, dwBytesRead);
				
				dwCounter = 0;

				// notify all the down renders that no coming data anymore
				notifyEndOfStream();
							
				// release source file handle 
				releaseFileHandle();
				
				// set the status
				_processStatus = STOPPED;
				
				// notify graph this filter processing completed
				_graph.notifyCompletion(*this);
				
				continue;
			}
			// trigger next loop to fetching data
			SetEvent(_hNotify);
			
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

	_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "NTFSFileIOSource::run() leave");
	
	return 1;
}

	
} } }