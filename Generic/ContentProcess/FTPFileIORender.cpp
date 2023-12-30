#include <afx.h>
#include <afxwin.h>         // MFC core and standard 
#include <afxinet.h>

#include "FTPFileIORender.h"
#include "urlstr.h"
#include "bufferpool.h"

namespace ZQ { 
namespace Content { 
namespace Process {

FTPFileIORender::FTPFileIORender(ZQ::Content::Process::Graph& graph, bool enableMD5Checksum, std::string myName)
: Filter(graph, myName), _hDesFile(INVALID_HANDLE_VALUE), _processedBytes(0),
_bEndOfStream(true), _enableMD5Checksum(enableMD5Checksum)
{	
	if(myName == "")
	{
		_myName = "FTPFileIORender";
	}

	_tidAbortBy = 0;
	
	_hStop = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);
}

FTPFileIORender::~FTPFileIORender(void)
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
	
     // release ftp resource
	ftpConnRelease();
}

void FTPFileIORender::ftpConnRelease(bool removeFile)
{
	if(_pFtpFile != NULL)
	{
		_pFtpFile->Close();
		delete _pFtpFile;
		_pFtpFile = NULL;
	}

	if(removeFile && !_szDesFileName.empty())
	{
		_pFtpConn->Remove(_szDesFileName.c_str());
	}

	if(_pFtpConn != NULL)
	{
		_pFtpConn->Close();
		delete _pFtpConn;
		_pFtpConn = NULL;
	}
	
	if(_pISession != NULL)
	{
		_pISession->Close();
		delete _pISession;
		_pISession = NULL;
	}	
}

void FTPFileIORender::setAccessProperty(std::string server,std::string userName,std::string password, unsigned int port)
{
	_szServer = server;
	_szUserName = userName;
	_szPassword = password;
	_nPort = port;
}

void FTPFileIORender::emptyDataQueue(void)
{
	ZQ::common::MutexGuard guard(_dataMutex);
	// remove all the buffer data pointer from the queue.
	while (!_dataQueue.empty())
	{
		ZQ::Content::BufferData* pBuffData = _dataQueue.front();
		
		_pool.free(pBuffData);
		_dataQueue.pop();

		_graph.traceLog(id(), "FTPFileIORender: free buffData from pool. [BuffData Address: 0x%08X]", 
					    pBuffData);
	}	
}

bool FTPFileIORender::receive(Filter* upObj, ZQ::Content::BufferData* buff)
{
	if(STOPPED == _processStatus)
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "FTPFileIORender: it is in STOPPED status, does not receive data any more");
		return false;
	}

	if(_bEndOfStream)
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "FTPFileIORender: it is end of stream, does not receive data more");
		return false;
	}
	
	ZQ::Content::BufferData* receivedBuff = NULL;
	if(_copyUplinkDataBuff)
	{	// copy BuffData
		ZQ::Content::BufferPool& buffpool = _graph.getBuffPool();
		_graph.traceLog(id(), "BufferPool: usage: %d / %d [used/total]", 
				  buffpool.getUsedCount(), buffpool.getPoolSize());
		
		// copy the buffer to the new one
		receivedBuff = _pool.alloc();
		*receivedBuff = *buff;

		_graph.traceLog(id(), "FTPFileIORender: alloc buffData from pool. [BuffData Address: 0x%08X]", 
						receivedBuff);
	}
	else
	{   // does not copy BuffData
		receivedBuff = buff;
	}

	// put the data to the queue
	ZQ::common::MutexGuard guard(_dataMutex);
	_dataQueue.push(receivedBuff);
	
	_graph.traceLog(GetCurrentThreadId(), "FTPFileIORender: Receive Buffer Data from up side process object with actual length %d", buff->getActualLength());

	SetEvent(_hNotify);

	return true;
}

bool FTPFileIORender::begin(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIORender::begin() enter");
	
	// To avoid invoking begin() during its processing
	if(_processStatus == ACTIVE)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "FTPFileIORender: the task did not complete yet, can not initial new work");

		return false;
	}

	//
	// check the begin is invoked by pause - have begun or just begin
	//
	if(_processStatus != PAUSED)    // begin a new job
	{	
		_processedBytes = 0;

		_tidAbortBy = 0;

		// compose the file full name
		_szDesFileName = _graph.getContentName() ;
		
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIORender: The render's destination file is %s", _szDesFileName.c_str());
		
		_bEndOfStream = false;

		// make sure to empty the queue
		emptyDataQueue();
		
		// reset MD5 checksum utility
		if(_enableMD5Checksum)
		{
			_md5ChecksumUtil.reset();
		}
		
		// connect the ftp server and create the file handle for later reading data in the thread::run
		try 
		{
			TCHAR ftpAgent[256] = _T("FTPRender");
			
			_pISession = new CInternetSession(ftpAgent, 
				1,
				INTERNET_OPEN_TYPE_PRECONFIG,
				NULL,
				NULL,
				INTERNET_FLAG_DONT_CACHE);
			
			_pFtpConn = _pISession->GetFtpConnection(
				(LPCTSTR)_szServer.c_str(), 
			    (LPCTSTR)_szUserName.c_str(), 
				(LPCTSTR)_szPassword.c_str(),
				_nPort,
				FALSE);
		}
		catch(CInternetException* pEx)
		{
			TCHAR tszErr[255]; 
			char msg[255];

			pEx->GetErrorMessage(tszErr, 255);
			_snprintf(msg, 255, "%s[%d]", tszErr, pEx->m_dwError);
			
			pEx->Delete(); 
			
			_pFtpConn = NULL; 
			_pFtpFile = NULL;
			// release ftp resource
			ftpConnRelease();
			
			_graph.setLastError(ERRCODE_NO_AUTH, msg);
			
			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "FTPFileIOSource: GetFtpConnection failed with error: %s", msg);

			return false;
		}

		try
		{
			_pFtpFile = _pFtpConn->OpenFile(_szDesFileName.c_str(), 
				GENERIC_WRITE, 
				FTP_TRANSFER_TYPE_BINARY, 
				1);
			if(_pFtpFile == NULL)
			{
				// release ftp resource
				ftpConnRelease();
				
				return false;
			}
		}
		catch(CInternetException* pEx)
		{
			TCHAR tszErr[255]; 
			char msg[255];

			pEx->GetErrorMessage(tszErr, 255);
			_snprintf(msg, 255, "%s[%d]", tszErr, pEx->m_dwError);
			
			pEx->Delete(); 
			
			_pFtpFile = NULL;
			// release ftp resource
			ftpConnRelease();

			_graph.setLastError(ERRCODE_CREATEFILE_FAIL, msg);
			
			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "FTPFileIOSource: OpenFile failed with error: %s", msg);
			return false;
		}
	}

	// resume the thread
	_processStatus = ACTIVE;

	// resume the native thread
	start();	
		
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIORender::begin() leave");

	return true;
}

bool FTPFileIORender::pause(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIORender::pause() enter");

	_processStatus = PAUSED;

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIORender::pause() leave");
	
	// suspend the native thread
	suspend();
		
	return true;
}

bool FTPFileIORender::abort(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIORender::abort() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = ABORTED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIORender::abort() leave");
	
	return true;
}

void FTPFileIORender::stop(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIORender::stop() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = STOPPED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIORender::stop() leave");
}

void FTPFileIORender::quit(void)
{
	SetEvent(_hStop);
}

void FTPFileIORender::endOfStream(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "FTPFileIORender: Get end of stream notification");

	_bEndOfStream = true;

	SetEvent(_hNotify);
}

int FTPFileIORender::run(void)
{	
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "FTPFileIORender::run() enter");

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
			_graph.writeLog(ZQ::common::Log::L_INFO, id(), "FTPFileIORender: get a thread exit event");
			break;

		// received the Notify event
		case WAIT_OBJECT_0 + 1:
		{	
			// check whether this thread is abort by Graph
			if(STOPPED == _processStatus || ABORTED == _processStatus)
			{
				bool bRemoveFile = (ABORTED == _processStatus);
				// make sure to remove, 
				emptyDataQueue();

				if(ABORTED == _processStatus)
				{
					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "FTPFileIORender: It was aborted by Graph, triggered by thread 0x%08X, release file handle", _tidAbortBy);
				}

				// close the file handle
				ftpConnRelease(bRemoveFile);
				
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
			// write the buff to file
			if(pBuffData != NULL)
			{
				DWORD dwLen = 0;
				LPVOID pointer = pBuffData->getPointerForRead(dwLen);
								
				// calculate MD5 checksum
				if(_enableMD5Checksum)
				{
					_md5ChecksumUtil.checksum((const char*)pointer, dwLen);
				}

				// Write data to file
				try
				{
					_pFtpFile->Write(pointer,dwLen);
				}
				catch (CInternetException* pEx)
				{
					TCHAR tszErr[255]; 
					char msg[255];

					pEx->GetErrorMessage(tszErr, 255);
					_snprintf(msg, 255, "%s[%d]", tszErr, pEx->m_dwError);
					
					pEx->Delete(); 
					
					// set graph last error
					_graph.setLastError(ERRCODE_WRITEFILE_FAIL, msg);
					
					_pFtpConn = NULL; 
					_pFtpFile = NULL;
	
					// set the status, must be previous EmptyDataQueue() invoking
					_processStatus = ABORTED;
					_bEndOfStream = false;
					
					_pool.free(pBuffData);
					_graph.traceLog(id(), "FTPFileIORender: free buffData from pool. [BuffData Address: 0x%08X]", 
									pBuffData);
					
					dwCounter = 0;
					// release ftp resource
					ftpConnRelease(true);
					
					// make sure to remove, 
					emptyDataQueue();


					_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "FTPFileIORender: Write file fail with error: %s", msg);

					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "FTPFileIORender: Met IO Error, trigger Graph abort");
					
					// the render met problem, abort all the renders in the graph
					_graph.abortProvision();
					continue;				
				};

				dwCounter++;
				_processedBytes += dwLen;

				// free the buffer
				// release the buff 
				bool bReleased = releaseBuffer(pBuffData);
				if(bReleased)
				{
					_graph.traceLog(id(), "FTPFileIORender: free buffData from pool. [BuffData Address: 0x%08X]", 
									pBuffData);
				}

				_graph.traceLog(id(), "FTPFileIORender: %d buffer data has been processed, processed size is %I64d bytes", 
								dwCounter, _processedBytes);
				
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
				_graph.writeLog(ZQ::common::Log::L_INFO, id(), "FTPFileIORender: reaches the end of stream, there are %d buffer processed in all, total output size is %I64d bytes", 
								dwCounter, _processedBytes);

				// report MD5 checksum
				if(_enableMD5Checksum)
				{
					const char* md5value = _md5ChecksumUtil.lastChecksum();
                    ZQ::common::Variant vcs(md5value);

					ContentProperty checksumcp;
					checksumcp.insert(ContentProperty::value_type(CNTPRY_MD5_CHECKSUM, vcs));
					
					_graph.reportProperty(_graph.getContentName(), checksumcp);
				}

				// notify all the connected renders that reaches the end of stream
				notifyEndOfStream();

				// release ftp resource
				ftpConnRelease();
				
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
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "FTPFileIORender::run() leave");
	
	return 1;
}

	
} } }