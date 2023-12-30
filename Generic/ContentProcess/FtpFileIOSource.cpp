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
// $Header: /ZQProjs/Generic/ContentProcess/FtpFileIOSource.cpp 1     10-11-12 15:58 Admin $
// $Log: /ZQProjs/Generic/ContentProcess/FtpFileIOSource.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 28    08-02-20 18:22 Ken.qian
// user ZQ::common::URLStr to replace AfxParseURLEx
// 
// 27    08-01-02 18:35 Ken.qian
// change since minAvailBuffCount was move to Graph
// 
// 26    07-11-14 10:52 Ken.qian
// Add #pragma comment(lib, "Wininet.lib")
// 
// 25    07-11-06 15:22 Ken.qian
// 
// 24    07-11-05 11:38 Ken.qian
// set dwCount=0 in abortion
// 
// 23    07-11-02 16:27 Ken.qian
// ftpConnRelease() had check with errorcode, if errorcode is
// ERROR_INTERNET_TIMEOUT,  no InternetCloseHandle() call to avoid
// possible block.
// 
// 22    07-10-23 18:27 Ken.qian
// add available buffer count checking
// 
// 21    07-10-15 18:12 Ken.qian
// make buffer processing log frequency MICRO based.
// 
// 20    07-10-09 10:39 Ken.qian
// Change SIZE to be "BINARY" mode
// 
// 19    07-09-29 18:39 Ken.qian
// 1. Change MFC FTP to be Windows API
// 2. Add option to get file size by "SIZE" and "LIST"
// 
// 18    07-09-20 17:59 Ken.qian
// add log while processing buffer
// 
// 17    07-09-17 15:30 Ken.qian
// check whether the total read buffer size is same to file size, it not,
// regard it as failure
// 
// 16    07-09-13 10:59 Ken.qian
// if can not find file, just return.
// 
// 15    07-08-23 10:57 Ken.qian
// Add timeout setting to ftp connection
// 
// 14    07-08-16 12:46 Ken.qian
// 
// 13    07-08-09 17:10 Ken.qian
// 
// 12    07-08-02 17:07 Ken.qian
// 
// 11    07-08-01 17:57 Ken.qian
// 
// 10    07-07-23 15:29 Ken.qian
// vstrmiorender support subfiles and 64K IO writting
// 
// 9     07-07-19 17:45 Ken.qian
// normal release for rtfcpnode
// 
// 8     07-07-18 19:36 Ken.qian
// fix buffpool issue
// 
// 7     07-07-17 14:18 Ken.qian
// 
// 6     07-06-28 15:43 Fei.huang
// 
// 5     07-06-27 15:13 Ken.qian
// Change kbps to bps
// 
// 4     07-06-26 17:10 Ken.qian
// 
// 3     07-06-18 18:15 Fei.huang
// 
// 2     07-06-07 13:38 Fei.huang
// 
// 1     07-06-07 10:04 Ken.qian
// 
// 7     06-09-19 11:55 Ken.qian
// 
// 6     06-09-06 15:27 Ken.qian
// 
// 5     06-08-24 11:02 Ken.qian
// 
// 4     06-07-12 18:56 Ken.qian
// 
// 3     06-07-12 12:12 Ken.qian
// 
// 2     06-06-30 11:32 Ken.qian
// Change the abort processing workflow
// 
// 1     06-06-26 14:49 Ken.qian
// Initial Implementation
// 

#include "ZQ_common_conf.h"
#include "FTPFileIOSource.h"
#include "bufferpool.h"
#include "urlstr.h"

#pragma comment(lib, "Wininet.lib")

namespace ZQ { 
namespace Content { 
/// namespace Process presents the processing to the content, such as reading/writing
namespace Process {

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

FTPFileIOSource::FTPFileIOSource(ZQ::Content::Process::Graph& graph, std::string myName, bool ftpFileSizeBySIZEcmd)
: SourceFilter(graph, SRCFILTER_PROTO_FTP, myName), _ftpFileSizeBySIZEcmd(ftpFileSizeBySIZEcmd),
_hSession(NULL), _hConnection(NULL), _hFile(NULL), _processedBytes(0)
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

FTPFileIOSource::~FTPFileIOSource(void)
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
	
	// release ftp resource
	ftpConnRelease();

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

void FTPFileIOSource::ftpConnRelease(DWORD errorCode)
{
	// if the error code is ERROR_INTERNET_TIMEOUT(12002), 
	// can not inovke InternetCloseHandle(), it may block
	if(ERROR_INTERNET_TIMEOUT == errorCode)
	{
		_graph.writeLog(ZQ::common::Log::L_INFO, id(), "FTPFileIOSource: Last error code is ERROR_INTERNET_TIMEOUT, no calling InternetCloseHandle() to avoid block");
		
		// reset to avoid later call it again without providing error code
		_hFile = NULL; 
		_hConnection = NULL;
		_hSession = NULL;
		return;
	}

	if(_hFile != NULL)
	{
		InternetCloseHandle(_hFile);
		_hFile = NULL;
	}
	
	if(_hConnection != NULL)
	{
		InternetCloseHandle(_hConnection);
		_hConnection = NULL;
	}

	if(_hSession != NULL)
	{
		InternetCloseHandle(_hSession);
		_hSession = NULL;
	}	
}

bool FTPFileIOSource::receive(Filter* upObj, ZQ::Content::BufferData* buff)
{
	// to the source, no implementation of the receive
	return true;
}

bool FTPFileIOSource::begin(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIOSource::begin() enter");
	
	// To avoid invoking begin() during its processing
	if(_processStatus == ACTIVE)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "FTPFileIOSource: task did not complete yet, can not initial new work");

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
		
		ZQ::common::URLStr srcUrl(srcPath.c_str());

		std::string strServer = srcUrl.getHost();
		std::string strOrgFile = srcUrl.getPath();
		std::string strUserName = srcUrl.getUserName();
		std::string strPassword = srcUrl.getPwd();
		INTERNET_PORT nPort = srcUrl.getPort();
		
		_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "FTPFileIOSource: Host=%s, File=%s, User=%s, Pwd=%s, Port=%d", 
			strServer.c_str(), strOrgFile.c_str(), strUserName.c_str(), strPassword.c_str(), nPort);

		// replace %20 with space: TODO

		TCHAR ftpAgent[256] = _T("FTPSource");

		_hSession = InternetOpen(ftpAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_DONT_CACHE);

		if(NULL == _hSession)
		{
			std::string errmsg;
			getSystemErrorText(errmsg);

			_graph.setLastError(ERRCODE_NO_AUTH, errmsg);

			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "FTPFileIOSource: InternetOpen() failed with error: %s", errmsg.c_str());

			return false;
		}
		
		DWORD value = DEF_FTP_CONN_TIMEOUT;
		InternetSetOption(_hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &value, sizeof(value));
		InternetSetOption(_hSession, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, &value, sizeof(value));
		InternetSetOption(_hSession, INTERNET_OPTION_DATA_SEND_TIMEOUT, &value, sizeof(value));
		InternetSetOption(_hSession, INTERNET_OPTION_DISCONNECTED_TIMEOUT, &value, sizeof(value));
				 		

		_hConnection = InternetConnect(_hSession, 
									   (LPCSTR)strServer.c_str(), 
									   nPort, 
									   (LPCSTR)strUserName.c_str(), 
									   (LPCSTR)strPassword.c_str(), 
									   INTERNET_SERVICE_FTP,
									   NULL, 
									   1);

		if(NULL == _hConnection)
		{
			DWORD lastError = GetLastError();
			
			std::string errmsg;
			getSystemErrorText(errmsg);

			_graph.setLastError(ERRCODE_NO_AUTH, errmsg);

			// release ftp resource
			ftpConnRelease(lastError);

			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "FTPFileIOSource: InternetConnect() failed with error: %s", errmsg.c_str());

			return false;
		}
		
		if(_ftpFileSizeBySIZEcmd)
		{
			char ftpcmd[MAX_PATH];
			sprintf(ftpcmd, "SIZE %s", (LPCSTR)strOrgFile.c_str());
			FtpCommand(_hConnection, FALSE, FTP_TRANSFER_TYPE_BINARY, ftpcmd, 0, NULL);
			DWORD dwErr = 0;
			DWORD buflen = MAX_PATH;
			if(InternetGetLastResponseInfo(&dwErr, ftpcmd, &buflen))
			{
				_fileSize = _atoi64(ftpcmd+3);
			}
		}
		else
		{
			WIN32_FIND_DATA fileInfo;
			memset(&fileInfo, 0x0, sizeof(WIN32_FIND_DATA));
			HINTERNET hFind = FtpFindFirstFile(_hConnection, (LPCSTR)strOrgFile.c_str(), &fileInfo, INTERNET_FLAG_RELOAD, 1);
			
			if(hFind == NULL)
			{
				DWORD lastError = GetLastError();
					
				std::string strError;
				getSystemErrorText(strError);

				// must close, otherwise, there will memory access error.
				InternetCloseHandle(hFind);

				// release ftp resource
				ftpConnRelease(lastError);
				
				_graph.setLastError(ERRCODE_OPENFILE_FAIL, strError);

				_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "FTPFileIOSource: can not find the file at ftp server with error: %s", strError.c_str());
				return false;
			}
			
			_fileSize = (__int64)fileInfo.nFileSizeHigh;
			_fileSize = (__int64)fileInfo.nFileSizeLow + (__int64)(_fileSize << 32);

			// must close, otherwise, there will memory access error.
			InternetCloseHandle(hFind);
		}		

		_hFile = FtpOpenFile(_hConnection, 
					(LPCSTR)strOrgFile.c_str(), 
					GENERIC_READ, 
					FTP_TRANSFER_TYPE_BINARY|INTERNET_FLAG_DONT_CACHE, 
					1);
		
		if(NULL == _hFile)
		{
			DWORD lastError = GetLastError();

			std::string strError;
			getSystemErrorText(strError);
			_graph.setLastError(ERRCODE_OPENFILE_FAIL, strError);

			// release ftp resource
			ftpConnRelease(lastError);
			
			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "FTPFileIOSource: FtpOpenFile() failed with error: %s", 
				strError.c_str());
			
			return false;
		}
		
		// update the FileSize to Graph
		ContentProperty filesizecp;
		filesizecp.insert(ContentProperty::value_type(CNTPRY_FILESIZE, _fileSize));
		_graph.reportProperty(_graph.getContentName(), filesizecp);

		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIOSource: FTP file is opened successfully, file size is %I64d", _fileSize);
	}
	else
	{
		// resume the thread
		start();
	}
	
	_processStatus = ACTIVE;
	
	// initial the executing of run function
	_lasttimer = timeval();
	SetEvent(_hNotify);		

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIOSource::begin() Leave");

	return true;
}

bool FTPFileIOSource::pause(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIOSource::pause() enter");

	_processStatus = PAUSED;

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIOSource::pause() leave");
	
	// suspend the thread
	suspend();
		
	return true;
}

bool FTPFileIOSource::abort(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIOSource::abort() enter");
		
	// remember who initial the Graph abort.
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = ABORTED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIOSource::abort() leave");
	
	return true;
}

void FTPFileIOSource::stop(void)
{
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "FTPFileIOSource::stop() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = STOPPED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "FTPFileIOSource::stop() leave");
	
}

void FTPFileIOSource::quit(void)
{
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "FTPFileIOSource::quit() enter");

	SetEvent(_hStop);

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "FTPFileIOSource::quit() leave");
}

void FTPFileIOSource::setReadSize(int readSize)
{
	if(readSize > 0)
	{
		_readSize = readSize;
	}
}

int FTPFileIOSource::run(void)
{	
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "FTPFileIOSource::run() enter");

	bool bContinue = true;
	DWORD dwWaitStatus = 0;

	HANDLE handles[2] = { _hStop, _hNotify };

	DWORD dwCounter = 0;
	_lasttimer = timeval();

	_processedBytes = 0;

	while(bContinue)
	{
		dwWaitStatus = WaitForMultipleObjects(2, handles, false, INFINITE);
		switch(dwWaitStatus)
		{
		// received stop event, the thread will stop
		case WAIT_OBJECT_0:
			bContinue = false;
			_graph.writeLog(ZQ::common::Log::L_INFO, id(), "FTPFileIOSource: get a thread exit event");
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
				// release ftp connection
				ftpConnRelease();
				
				if(ABORTED == _processStatus)
				{
					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "FTPFileIOSource: It was aborted by Graph, triggered by thread 0x%08X", _tidAbortBy);
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
					_graph.traceLog(id(), "FTPFileIOSource: No avaliable buffer in pool, waiting for %d ms", DEFAULT_BUFF_WAIT_TIMEOUT);
					// trigger the loop to fetching data
					SetEvent(_hNotify);
					continue;
				}
				_graph.traceLog(id(), "FTPFileIOSource: alloc buffData from pool. [BuffData Address: 0x%08X]", pBuffData);
			}
						
			DWORD dwBytesRead = 0;
			bool bRead = InternetReadFile(_hFile, (LPVOID)pBuffData->getPointerForWrite(_readSize), _readSize, &dwBytesRead);
			if(!bRead)
			{
				DWORD lastError = GetLastError();
				dwCounter = 0;

				std::string errmsg;
				getSystemErrorText(errmsg);

				// release the buffdata
				_pool.free(pBuffData);	
				
				_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "FTPFileIOSource: Read file failed with error: %s", errmsg.c_str());
				
				// the source met problem, abort all the renders in the graph
				// release ftp resource
				ftpConnRelease(lastError);

				_graph.setLastError(ERRCODE_READFILE_FAIL, errmsg);

				_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "FTPFileIOSource: Met IO Error, trigger Graph abort()");

				_processStatus = STOPPED;
				_graph.abortProvision();
				
				continue;
			}
			
			if(dwCounter % DEFAULT_FILETER_LOGING_FEQ == 0)
			{
				_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "FTPFileIOSource: processBuffer No.%d", 
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
					_graph.traceLog(id(), "FTPFileIOSource: free buffData from pool. [BuffData Address: 0x%08X]", pBuffData);
				}
				
				dwCounter++;

				_graph.traceLog(id(), "FTPFileIOSource: %d buffer data has been processed", dwCounter);
				
				// control the speed
				_processedBytes += dwBytesRead;

				if (_maxbps > 0)
				{
					DWORD expected = (DWORD) ((_processedBytes * 8 * 1000) / _maxbps); // in ms

					DWORD now = timeval();
					DWORD interval = (now > _lasttimer) ? (now - _lasttimer) : 0;
					
					if (interval < expected)
					{
						_graph.traceLog(id(), "FTPFileIOSource: processed %I64d bytes expected=%d ms but used %d ms, will sleep for %d ms before reading next buff", 
							_processedBytes, expected, interval, expected-interval);
						::Sleep(expected - interval);
					}
				}
			}

			// check whether reaches the end of stream
			if(dwBytesRead < _readSize)
			{
				// release the buffdata
				if(0 == dwBytesRead)
				{
					bool bReleased = releaseBuffer(pBuffData, this);
					if(bReleased)
					{
						_graph.traceLog(id(), "FTPFileIOSource: free buffData from pool. [BuffData Address: 0x%08X]", pBuffData);
					}
				}

				if(_processedBytes < _fileSize)
				{
					//
					// reach the end of stream, but the size is not same to file size, this could be caused by FTPServer stop
					//

					// release ftp resource
					ftpConnRelease();

					std::string msg = "Failed to get more data from FTPServer";
					_graph.setLastError(ERRCODE_READFILE_FAIL, msg);

					_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "FTPFileIOSource: Failed to get more data from FTPServer, %I64d/%I64d(read/total)", 
						_processedBytes, _fileSize);
					
					_processStatus = STOPPED;
					_graph.abortProvision();

				}
				else
				{
					_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "FTPFileIOSource: reaches the end of stream, there are %d buff read, the last buffer size is %d", dwCounter, dwBytesRead);
					
					dwCounter = 0;

					// notify all the down renders that no coming data anymore
					notifyEndOfStream();
					
					// release ftp resource
					ftpConnRelease();

					// set the status
					_processStatus = STOPPED;
				
					// notify graph this filter processing completed
					_graph.notifyCompletion(*this);

					continue;
				}
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
	
	return 1;
}

	
} } }