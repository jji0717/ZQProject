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
// $Header: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/NTFSFileIORender.cpp 2     12/12/13 1:52p Hui.shao $
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/NTFSFileIORender.cpp $
// 
// 2     12/12/13 1:52p Hui.shao
// %lld
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 1     08-12-08 11:11 Li.huang
// 
// 2     08-11-03 17:14 Ken.qian
// 
// 1     08-10-30 16:45 Ken.qian
// Move files from /ZQProjs/Generic/ContentProcess to local folder, since
// files at ContentProcess were never used by others components. And
// remove the pacing codes from NTFSIORender to indepent on Vstrm DLL
// 
// 28    08-10-29 10:12 Ken.qian
// add macro to disable pacing
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


#define NTFSIO_MAX_QUEUED_SIZE  20 
namespace ZQ { 
namespace Content { 
/// namespace Process presents the processing to the content, such as reading/writing
namespace Process {


NTFSFileIORender::NTFSFileIORender(ZQ::Content::Process::Graph& graph, bool enableMD5Checksum, std::string myName,
								   bool delProvedFile, bool delErrorFile)
: Filter(graph, myName), 
_bEndOfStream(true), _enableMD5Checksum(enableMD5Checksum), 
_delProvedFile(delProvedFile), _delErrorFile(delErrorFile), _fpmType(FPMT_NONE), 
_indexFileNo(0), _mainFileNo(0)
{	
	if(myName == "")
	{
		_myName = "NTFSFileIORender";
	}

	_tidAbortBy = 0;
	
	_hStop = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);

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
					// close the file handle
					uninitSubFiles(_delErrorFile);

					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "NTFSFileIORender: It was aborted by Graph, triggered by thread 0x%08X", _tidAbortBy);
				}
				
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

				_graph.traceLog(id(), "NTFSFileIORender: %d buffer data has been processed, processed size is %lld bytes", 
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
				_graph.writeLog(ZQ::common::Log::L_INFO, id(), "NTFSFileIORender: reaches the end of stream, there are %d buffer processed in all, total output size is %lld bytes", 
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
		SubFile* subfile = new SubFile(i, *this, _graph);

		_subFiles.push_back(subfile);
	}
}

bool NTFSFileIORender::setSubFileInfo(int fileNo, std::string extension, bool enableMD5, bool progressRpt, FILE_POINT_MOVING_TYPE mtype)
{
	if(fileNo<0 || fileNo>=(int)_subFiles.size())
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

	return true;
}

void NTFSFileIORender::uninitSubFiles(bool delOutput)
{
	for(DWORD i=0; i<_subFiles.size(); i++)
	{
		_subFiles[i]->uninit(delOutput);
	}
}

NTFSFileIORender::SubFile::SubFile(int fileNo, NTFSFileIORender& ntfsIoRender, Graph& graph)
: _ntfsIoRender(ntfsIoRender), _graph(graph)
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
	return true;
}

void NTFSFileIORender::SubFile::uninit(bool delFile)
{
	if(_outputFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(_outputFileHandle);
		_outputFileHandle = INVALID_HANDLE_VALUE;
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
	
	_processedBytes += dwLen;

	switch(_filePMT)
	{
	case FPMT_NONE:
		// calculate MD5 checksum
		if(_enableMD5)
		{
			_md5ChecksumUtil.checksum((const char*)pointer, dwLen);
		}
		bWriteSuccess = WriteFile(_outputFileHandle, pointer, dwLen, &amountWritten, NULL);

		break;
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

} } }