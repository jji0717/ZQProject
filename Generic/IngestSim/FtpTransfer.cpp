         // FtpTransfer.cpp: implementation of the FtpTransfer class.
//
//////////////////////////////////////////////////////////////////////

#include "FtpTransfer.h"
#include "Wininet.h"
#define GLOABL_ADJUST_SPEED
#define GLOABL_MAXBBYTES (0x80000000)

#pragma comment(lib, "Wininet.lib")
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//means every this time to adjust the transfer speed
#define FTP_XFERRATE_INTERVAL	500


#include "log.h"

using namespace ZQ::common;

void LogMsg(DWORD dwTraceLevel, LPCTSTR lpszFmt, ...)
{
    va_list    marker;
    TCHAR		szMsg[4096];

    // Initialize access to variable arglist
    va_start(marker, lpszFmt);

    // Expand message
    _vsntprintf(szMsg, 4095 * sizeof(TCHAR), lpszFmt, marker);
    szMsg[4095] = 0;

	glog(dwTraceLevel, _T("%s"), szMsg);
}


BOOL DownloadBufferQueue::GetDataBuffer(void**pPointer, int*pLength)
{			
//	printf("GetDataBuffer enter\n");
	while(_queuedepth < 1 && !_dataend)
	{
		Sleep(10);
	}

	if (_dataend)
	{
		*pPointer = NULL;
		*pLength = 0;
//		printf("GetDataBuffer leave with end\n");
		return false;
	}

	EnterCriticalSection(&_mutex);

	void* pBuf = _dataqueue.front();
	_dataqueue.pop_front();
	if (pBuf)
	{
		*pPointer = (void*)((BYTE*)pBuf + 4);
		*pLength = *((int*)pBuf);
	}
	else
	{
		*pPointer = NULL;
		*pLength = 0;
		_dataend = true;
	}
	_queuedepth--;

	LeaveCriticalSection(&_mutex);

//	printf("GetDataBuffer leave\n");
	return (*pPointer != 0);
}



BOOL FtpPullToPush::PullToPush(LPCTSTR sUrlToPull, LPCTSTR sUrlToPush, DWORD dwMaxSpeedbps)
{
	try
	{
		CInternetSession csPull(_T("Pull"));
		CMFCAutoPtr<CFtpConnection> pPullFtp;
		CMFCAutoPtr<CInternetFile> pPullFile;
		
		{
			CString	strServer, strObject, strUsername, strPassword;
			INTERNET_PORT nPort;
			DWORD	dwServiceType;

			//get server, user, pwd, port from url
			BOOL bRet = AfxParseURLEx(
				sUrlToPull,
				dwServiceType,
				strServer,
				strObject, 
				nPort, 
				strUsername, 
				strPassword);

			if (!bRet)
			{
				_strError.Format(_T("url: [%s] parse error."), sUrlToPull);
				return false;
			}

			if (dwServiceType != AFX_INET_SERVICE_FTP)
			{
				_strError.Format(_T("url: [%s] error, not ftp protocol."), sUrlToPull);
				return false;
			}

			//if some value is not set, set with default value(port is not need to set)
			if (strUsername.IsEmpty())
			{
				strUsername = _T("Anonymous");
				strPassword = _T("foo@bar.com");
			}

			//log all param parsed
			LogMsg(Log::L_DEBUG, _T("Pull FTP: Server[%s], Port[%d], File[%s], User[%s], Pwd[%s]."), 
				strServer.GetBuffer(0),
				(int)nPort,
				strObject.GetBuffer(0),
				strUsername.GetBuffer(0),
				strPassword.GetBuffer(0));
			
			try
			{
				pPullFtp = csPull.GetFtpConnection(strServer, strUsername, strPassword, nPort);
			}
			catch (CInternetException* pEx)
			{
				TCHAR sz[1024];
				pEx->GetErrorMessage(sz, 1024);
				_strError.Format(_T("Caught CInternetException in GetFtpConnection of Pull[%s]: %s"), sUrlToPull, sz);
				pEx->Delete();
				return false;
			}

			// don't use this, this will fail on StrService(block, and fail)
			//if need find file first, get file size
			//
/*		    CFtpFileFind ftpFileFind(pPullFtp);

			if (ftpFileFind.FindFile(strObject.GetBuffer(0)))
			{
				ftpFileFind.FindNextFile();
				_llTotalBytes = ftpFileFind.GetLength64();
				ftpFileFind.Close();
			}
			else
			{
//				_strError.Format(_T("File(%s) not found."), strObject.GetBuffer(0));
//				return false;
				DWORD dwError = GetLastError();
				LogMsg(Log::L_NOTICE, _T("FtpFindFile(%s) fail, maybe ftpserver not support FtpFindFile or file not exist."), strObject.GetBuffer(0));
			}		*/	

			//if file not exist, exception
			try
			{
				pPullFile = pPullFtp->OpenFile(strObject, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY);
			}
			catch (CInternetException* pEx)
			{
				TCHAR sz[1024];
				pEx->GetErrorMessage(sz, 1024);
				_strError.Format(_T("Caught CInternetException in CFtpConnection::OpenFile() : %s"), sz);
				pEx->Delete();
				return false;
			}

			if (!pPullFile)
			{
				_strError.Format(_T("CFtpConnection::OpenFile() : Open File(%s) fail."), strObject.GetBuffer(0));
				return false;
			}

			//when file is bigger than 4G, the return value is error, need to report to Maynard.
			DWORD dwLow = 0, dwHigh = 0;
			HINTERNET  hFile = (HINTERNET)(*pPullFile);
			dwLow = FtpGetFileSize(hFile, &dwHigh);
			((DWORD*)&_llTotalBytes)[1]  = dwHigh;
			((DWORD*)&_llTotalBytes)[0]  = dwLow;			
		}

		// open upload ftp connection
		CInternetSession csPush(_T("Push"));
		CMFCAutoPtr<CFtpConnection> pPushFtp;
		CMFCAutoPtr<CInternetFile> pPushFile;
		
		{
			CString	strServer, strObject, strUsername, strPassword;
			INTERNET_PORT nPort;
			DWORD	dwServiceType;

			//get server, user, pwd, port from url
			BOOL bRet = AfxParseURLEx(
				sUrlToPush,
				dwServiceType,
				strServer,
				strObject, 
				nPort, 
				strUsername, 
				strPassword);

			if (!bRet)
			{
				_strError.Format(_T("url: [%s] parse error."), sUrlToPush);
				return false;
			}

			if (dwServiceType != AFX_INET_SERVICE_FTP)
			{
				_strError.Format(_T("url: [%s] error, not ftp protocol."), sUrlToPush);
				return false;
			}

			//if some value is not set, set with default value(port is not need to set)
			if (strUsername.IsEmpty())
			{
				strUsername = _T("Anonymous");
				strPassword = _T("foo@bar.com");
			}

			//log all param parsed
			LogMsg(Log::L_DEBUG, _T("Push FTP: Server[%s], Port[%d], File[%s], User[%s], Pwd[%s]."), 
				strServer.GetBuffer(0),
				(int)nPort,
				strObject.GetBuffer(0),
				strUsername.GetBuffer(0),
				strPassword.GetBuffer(0));
			
			try
			{
				pPushFtp = csPush.GetFtpConnection(strServer, strUsername, strPassword, nPort);
			}
			catch (CInternetException* pEx)
			{
				TCHAR sz[1024];
				pEx->GetErrorMessage(sz, 1024);
				_strError.Format(_T("Caught CInternetException in GetFtpConnection of Push[%s]: %s"), sUrlToPush, sz);
				pEx->Delete();
				return false;
			}

			//if file exist, will?
			try
			{
				pPushFile = pPushFtp->OpenFile(strObject, GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY);
			}
			catch (CInternetException* pEx)
			{
				TCHAR sz[1024];
				pEx->GetErrorMessage(sz, 1024);
				_strError.Format(_T("Caught CInternetException in CFtpConnection::OpenFile() : %s"), sz);
				pEx->Delete();
				return false;
			}

			if (!pPushFile)
			{
				_strError.Format(_T("CFtpConnection::OpenFile() : Open File(%s) fail."), strObject.GetBuffer(0));
				return false;
			}			
		}

		//start process pull to push
		BYTE	buf[4096*16];
		DWORD	dwRead;

		DWORD	dwT1 = GetTickCount();
#ifdef GLOABL_ADJUST_SPEED
		//for speed limit
		DWORD dwIntervalSent = 0; DWORD dwLast, dwCur;
		DWORD dwCheckSent = DWORD(((float)dwMaxSpeedbps) * FTP_XFERRATE_INTERVAL/(8*1000));
		DWORD dwShouldSent, dwTotalSent = 0;
		dwLast = dwT1;

		dwRead = pPullFile->Read(buf, sizeof(buf));
		while(dwRead)
		{
			pPushFile->Write(buf, dwRead);
			_llProcBytes += dwRead;
			LogMsg(Log::L_DEBUG, _T("Writed: %d"), dwRead);

			//This function always fulfills the user's request. If more data is requested 
			//than is available, the function waits until enough data to complete the request
			//is available. The only time that less data is returned than requested is when the
			//end of the file has been reached.
/*			if (dwRead != sizeof(buf))
			{
				break;
			}*/

			if (dwMaxSpeedbps)
			{
				dwIntervalSent += dwRead;
				
				if (dwIntervalSent >= dwCheckSent)
				{
					dwTotalSent += dwIntervalSent;
					dwCur  = GetTickCount();
					if (dwCur >= dwLast)
					{
						dwShouldSent = int(((double)dwMaxSpeedbps) * (dwCur - dwLast)/(8*1000));

						if (dwTotalSent > dwShouldSent)
						{
							int nToSleep = int((dwTotalSent - dwShouldSent)*8000.0/dwMaxSpeedbps);
							Sleep(nToSleep);

							dwCur  = GetTickCount();
							if (dwCur != dwLast)
								LogMsg(Log::L_DEBUG, _T("Sleep time is %d, speed: %d"), nToSleep, int(dwTotalSent*8000.0/(dwCur - dwLast)));
						}
					}

					if (dwTotalSent > GLOABL_MAXBBYTES)
					{
						// reset the counter to process file bigger than 4G
						dwLast = GetTickCount();
						dwTotalSent = 0;
					}
					
					dwIntervalSent = 0;
				}
			}
			dwRead = pPullFile->Read(buf, sizeof(buf));
			LogMsg(Log::L_DEBUG, _T("Read: %d"), dwRead);
		}		
#else
		//for speed limit
		int nIntervalSent = 0; DWORD dwLast, dwCur;
		int nCheckSent = int(((float)dwMaxSpeedbps) * FTP_XFERRATE_INTERVAL/(8*1000));
		int nShouldSent;
		dwLast = dwT1;

		dwRead = pPullFile->Read(buf, sizeof(buf));
		while(dwRead)
		{
			pPushFile->Write(buf, dwRead);
			_llProcBytes += dwRead;
			
			if (dwMaxSpeedbps)
			{
				nIntervalSent += dwRead;
				
				if (nIntervalSent >= nCheckSent)
				{
					dwCur  = GetTickCount();
					nShouldSent = int(((float)dwMaxSpeedbps) * (dwCur - dwLast)/(8*1000));
					
					if (nShouldSent >= 0 && nIntervalSent > nShouldSent)
					{
						int nToSleep = int((nIntervalSent - nShouldSent)*8000.0/dwMaxSpeedbps);

						LogMsg(Log::L_DEBUG, _T("Sleep time is %d, checktimediff: %d nIntervalSent %d %d"), nToSleep, dwCur - dwLast, nIntervalSent,nShouldSent);

						Sleep(nToSleep);
						
						dwLast = GetTickCount();						
					}
					else
					{
						dwLast = dwCur;
					}
					
					nIntervalSent = 0;
				}
			}
			dwRead = pPullFile->Read(buf, sizeof(buf));
		}			
#endif
		DWORD	dwT2 = GetTickCount();

		DWORD	dwRate;
		if (dwT2 == dwT1)
		{
			dwRate = 0;
		}
		else
		{
			dwRate = (_llProcBytes*8000/(dwT2 - dwT1));
		}

		LogMsg(Log::L_DEBUG, _T("speed is:  %d bps"), dwRate);
	}
	catch (CInternetException* pEx)
	{
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		_strError.Format(_T("Caught CInternetException : %s"), sz);
		pEx->Delete();
		return false;
	}

	return TRUE;
}




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



bool FtpPullBuf::OpenFile(LPCTSTR sUrl)
{
	{
		CString	strServer, strObject, strUsername, strPassword;
		INTERNET_PORT nPort;
		DWORD	dwServiceType;

		//get server, user, pwd, port from url
		BOOL bRet = AfxParseURLEx(
			sUrl,
			dwServiceType,
			strServer,
			strObject, 
			nPort, 
			strUsername, 
			strPassword);

		if (!bRet)
		{
			_strError.Format(_T("url: [%s] parse error."), sUrl);
			return false;
		}

		if (dwServiceType != AFX_INET_SERVICE_FTP)
		{
			_strError.Format(_T("url: [%s] error, not ftp protocol."), sUrl);
			return false;
		}

		//if some value is not set, set with default value(port is not need to set)
		if (strUsername.IsEmpty())
		{
			strUsername = _T("Anonymous");
			strPassword = _T("foo@bar.com");
		}

		//log all param parsed
		LogMsg(Log::L_DEBUG, _T("Pull FTP: Server[%s], Port[%d], File[%s], User[%s], Pwd[%s]."), 
			strServer.GetBuffer(0),
			(int)nPort,
			strObject.GetBuffer(0),
			strUsername.GetBuffer(0),
			strPassword.GetBuffer(0));
		
		try
		{
			_pPullFtp = _csPull.GetFtpConnection(strServer, strUsername, strPassword, nPort);
		}
		catch (CInternetException* pEx)
		{
			TCHAR sz[1024];
			pEx->GetErrorMessage(sz, 1024);
			_strError.Format(_T("Caught CInternetException in GetFtpConnection of Pull[%s]: %s"), sUrl, sz);
			pEx->Delete();
			return false;
		}

		
		// don't use this, this will fail on StrService(block, and fail)
		//if need find file first, get file size
		//
/*		CFtpFileFind ftpFileFind(_pPullFtp);

		if (ftpFileFind.FindFile(strObject.GetBuffer(0)))
		{
			ftpFileFind.FindNextFile();
			_llTotalBytes = ftpFileFind.GetLength64();
			ftpFileFind.Close();
		}
		else
		{
//				_strError.Format(_T("File(%s) not found."), strObject.GetBuffer(0));
//				return false;
			LogMsg(Log::L_NOTICE, _T("FtpFindFile(%s) fail, maybe ftpserver not support FtpFindFile or file not exist."), strObject.GetBuffer(0));
		}			
*/
		//if file not exist, exception
		try
		{
			_pPullFile = _pPullFtp->OpenFile(strObject, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY);
		}
		catch (CInternetException* pEx)
		{
			TCHAR sz[1024];
			pEx->GetErrorMessage(sz, 1024);
			_strError.Format(_T("Caught CInternetException in CFtpConnection::OpenFile() : %s"), sz);
			pEx->Delete();
			return false;
		}

		if (!_pPullFile)
		{
			_strError.Format(_T("CFtpConnection::OpenFile() : Open File(%s) fail."), strObject.GetBuffer(0));
			return false;
		}

		//when file is bigger than 4G, the return value is error, need to report to Maynard.
		DWORD dwLow = 0, dwHigh = 0;
		HINTERNET  hFile = (HINTERNET)(*_pPullFile);
		dwLow = FtpGetFileSize(hFile, &dwHigh);
		((DWORD*)&_llTotalBytes)[1]  = dwHigh;
		((DWORD*)&_llTotalBytes)[0]  = dwLow;
	}

	return true;
}

int FtpPullBuf::getData(void* buf, int nLen)
{
	if (!_pPullFile)
		return -1;

	int 	nRead = 0;

	try
	{
		nRead = _pPullFile->Read(buf, nLen);
		_llProcBytes += nRead;
	}
	catch (CInternetException* pEx)
	{
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		_strError.Format(_T("Caught CInternetException in CFtpConnection::OpenFile() : %s"), sz);
		pEx->Delete();
		return -1;
	}

	return nRead;
}




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



FtpPullToPush_Queue::~FtpPullToPush_Queue()
{

}

BOOL FtpPullToPush_Queue::PullToPush(LPCTSTR sUrlToPull, LPCTSTR sUrlToPush, DWORD dwMaxSpeedbps)
{
	try
	{
		{
			CString	strServer, strObject, strUsername, strPassword;
			INTERNET_PORT nPort;
			DWORD	dwServiceType;

			//get server, user, pwd, port from url
			BOOL bRet = AfxParseURLEx(
				sUrlToPull,
				dwServiceType,
				strServer,
				strObject, 
				nPort, 
				strUsername, 
				strPassword);

			if (!bRet)
			{
				_strError.Format(_T("url: [%s] parse error."), sUrlToPull);
				return false;
			}

			if (dwServiceType != AFX_INET_SERVICE_FTP)
			{
				_strError.Format(_T("url: [%s] error, not ftp protocol."), sUrlToPull);
				return false;
			}

			//if some value is not set, set with default value(port is not need to set)
			if (strUsername.IsEmpty())
			{
				strUsername = _T("Anonymous");
				strPassword = _T("foo@bar.com");
			}

			//log all param parsed
			LogMsg(Log::L_DEBUG, _T("Pull FTP: Server[%s], Port[%d], File[%s], User[%s], Pwd[%s]."), 
				strServer.GetBuffer(0),
				(int)nPort,
				strObject.GetBuffer(0),
				strUsername.GetBuffer(0),
				strPassword.GetBuffer(0));
			
			try
			{
				_pPullFtp = _csPull.GetFtpConnection(strServer, strUsername, strPassword, nPort);
			}
			catch (CInternetException* pEx)
			{
				TCHAR sz[1024];
				pEx->GetErrorMessage(sz, 1024);
				_strError.Format(_T("Caught CInternetException in GetFtpConnection of Pull[%s]: %s"), sUrlToPull, sz);
				pEx->Delete();
				return false;
			}

			//if file not exist, exception
			try
			{
				_pPullFile = _pPullFtp->OpenFile(strObject, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY);
			}
			catch (CInternetException* pEx)
			{
				TCHAR sz[1024];
				pEx->GetErrorMessage(sz, 1024);
				_strError.Format(_T("Caught CInternetException in CFtpConnection::OpenFile() : %s"), sz);
				pEx->Delete();
				return false;
			}

			if (!_pPullFile)
			{
				_strError.Format(_T("CFtpConnection::OpenFile() : Open File(%s) fail."), strObject.GetBuffer(0));
				return false;
			}

			//when file is bigger than 4G, the return value is error, need to report to Maynard.
			DWORD dwLow = 0, dwHigh = 0;
			HINTERNET  hFile = (HINTERNET)(*_pPullFile);
			dwLow = FtpGetFileSize(hFile, &dwHigh);
			((DWORD*)&_llTotalBytes)[1]  = dwHigh;
			((DWORD*)&_llTotalBytes)[0]  = dwLow;			
		}

		// open upload ftp connection
		{
			CString	strServer, strObject, strUsername, strPassword;
			INTERNET_PORT nPort;
			DWORD	dwServiceType;

			//get server, user, pwd, port from url
			BOOL bRet = AfxParseURLEx(
				sUrlToPush,
				dwServiceType,
				strServer,
				strObject, 
				nPort, 
				strUsername, 
				strPassword);

			if (!bRet)
			{
				_strError.Format(_T("url: [%s] parse error."), sUrlToPush);
				return false;
			}

			if (dwServiceType != AFX_INET_SERVICE_FTP)
			{
				_strError.Format(_T("url: [%s] error, not ftp protocol."), sUrlToPush);
				return false;
			}

			//if some value is not set, set with default value(port is not need to set)
			if (strUsername.IsEmpty())
			{
				strUsername = _T("Anonymous");
				strPassword = _T("foo@bar.com");
			}

			//log all param parsed
			LogMsg(Log::L_DEBUG, _T("Push FTP: Server[%s], Port[%d], File[%s], User[%s], Pwd[%s]."), 
				strServer.GetBuffer(0),
				(int)nPort,
				strObject.GetBuffer(0),
				strUsername.GetBuffer(0),
				strPassword.GetBuffer(0));
			
			try
			{
				_pPushFtp = _csPush.GetFtpConnection(strServer, strUsername, strPassword, nPort);
			}
			catch (CInternetException* pEx)
			{
				TCHAR sz[1024];
				pEx->GetErrorMessage(sz, 1024);
				_strError.Format(_T("Caught CInternetException in GetFtpConnection of Push[%s]: %s"), sUrlToPush, sz);
				pEx->Delete();
				return false;
			}

			//if file exist, will?
			try
			{
				_pPushFile = _pPushFtp->OpenFile(strObject, GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY);
			}
			catch (CInternetException* pEx)
			{
				TCHAR sz[1024];
				pEx->GetErrorMessage(sz, 1024);
				_strError.Format(_T("Caught CInternetException in CFtpConnection::OpenFile() : %s"), sz);
				pEx->Delete();
				return false;
			}

			if (!_pPushFile)
			{
				_strError.Format(_T("CFtpConnection::OpenFile() : Open File(%s) fail."), strObject.GetBuffer(0));
				return false;
			}			
		}

		
		//start the read data thread
		_bRunning = true;
		start();

		void*	buf;
		int		nlen;

		_llProcBytes = 0;
		DWORD	dwT1 = GetTickCount();

#ifdef GLOABL_ADJUST_SPEED
		//for speed limit
		DWORD dwIntervalSent = 0; DWORD dwLast, dwCur;
		DWORD dwCheckSent = DWORD(((float)dwMaxSpeedbps) * FTP_XFERRATE_INTERVAL/(8*1000));
		DWORD dwShouldSent, dwTotalSent = 0;
		dwLast = dwT1;

		while(_buffQueue.GetDataBuffer(&buf, &nlen))
		{
			_pPushFile->Write(buf, nlen);
			LogMsg(Log::L_DEBUG, _T("%4d  %4d"), *(((int*)buf)-1), _buffQueue.getDep());
			*(((int*)buf)-1) = 0;
			_buffQueue.ReleaseBuffer(buf);
			_llProcBytes += nlen;

			buf = NULL;
			
			if (dwMaxSpeedbps)
			{
				dwIntervalSent += nlen;
				
				if (dwIntervalSent >= dwCheckSent)
				{
					dwTotalSent += dwIntervalSent;
					dwCur  = GetTickCount();
					if (dwCur >= dwLast)
					{
						dwShouldSent = int(((double)dwMaxSpeedbps) * (dwCur - dwLast)/(8*1000));

						if (dwTotalSent > dwShouldSent)
						{
							int nToSleep = int((dwTotalSent - dwShouldSent)*8000.0/dwMaxSpeedbps);
							Sleep(nToSleep);

							dwCur  = GetTickCount();
							if (dwCur != dwLast)
								LogMsg(Log::L_DEBUG, _T("Sleep time is %d, speed: %d"), nToSleep, int(dwTotalSent*8000.0/(dwCur - dwLast)));
						}
					}

					if (dwTotalSent > GLOABL_MAXBBYTES)
					{
						// reset the counter to process file bigger than 4G
						dwLast = GetTickCount();
						dwTotalSent = 0;
					}
					
					dwIntervalSent = 0;
				}
			}
		}		
#else
		//for speed limit
		int nIntervalSent = 0; DWORD dwLast, dwCur;
		int nCheckSent = int(((float)dwMaxSpeedbps) * FTP_XFERRATE_INTERVAL/(8*1000));
		int nShouldSent;
		dwLast = dwT1;

		while(_buffQueue.GetDataBuffer(&buf, &nlen))
		{
			_pPushFile->Write(buf, nlen);
			LogMsg(Log::L_DEBUG, _T("%4d  %4d"), *(((int*)buf)-1), _buffQueue.getDep());
			*(((int*)buf)-1) = 0;
			_buffQueue.ReleaseBuffer(buf);
			_llProcBytes += nlen;

			buf = NULL;
			
			if (dwMaxSpeedbps)
			{
				nIntervalSent += nlen;
				
				if (nIntervalSent >= nCheckSent)
				{
					dwCur  = GetTickCount();
					nShouldSent = int(((float)dwMaxSpeedbps) * (dwCur - dwLast)/(8*1000));
					
					if (nShouldSent >= 0 && nIntervalSent > nShouldSent)
					{
						int nToSleep = int((nIntervalSent - nShouldSent)*8000.0/dwMaxSpeedbps);

						LogMsg(Log::L_DEBUG, _T("Sleep time is %d, checktimediff: %d nIntervalSent %d %d"), nToSleep, dwCur - dwLast, nIntervalSent,nShouldSent);

						Sleep(nToSleep);
						
						dwLast = GetTickCount();						
					}
					else
					{
						dwLast = dwCur;
					}
					
					nIntervalSent = 0;
				}
			}
		}			
#endif
		DWORD	dwT2 = GetTickCount();

		DWORD	dwRate;
		if (dwT2 == dwT1)
		{
			dwRate = 0;
		}
		else
		{
			dwRate = (_llProcBytes*8000/(dwT2 - dwT1));
		}

		LogMsg(Log::L_DEBUG, _T("speed is:  %d bps"), dwRate);

		_bRunning = false;
	}
	catch (CInternetException* pEx)
	{
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		_strError.Format(_T("Caught CInternetException : %s"), sz);
		pEx->Delete();

		_bRunning = false;
		return false;
	}

	return TRUE;
}

int FtpPullToPush_Queue::run(void)
{
	try
	{
		void*	buf;
		int		nLen, nRead;
		BOOL	bRet;

		LogMsg(Log::L_DEBUG, _T("FtpPullToPush_Queue ReadData Thread Enter"));

		do
		{
			bRet = _buffQueue.AllocateBuffer(&buf, &nLen);
			if (!bRet)
			{
				LogMsg(Log::L_ERROR, _T("AllocateBuffer fail"));
				_strError.Format(_T("AllocateBuffer fail"));
				break;
			}

			//This function always fulfills the user's request. If more data is requested 
			//than is available, the function waits until enough data to complete the request
			//is available. The only time that less data is returned than requested is when the
			//end of the file has been reached.
			nRead = _pPullFile->Read(buf, nLen);
			if(nRead<1)
			{
				_buffQueue.ReleaseBuffer(buf);
				break;
			}

			_buffQueue.AddDataBuffer(buf, nRead);			
		}while(nRead == nLen && _bRunning);
	}
	catch (CInternetException* pEx)
	{
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		_strError.Format(_T("Caught CInternetException : %s"), sz);
		pEx->Delete();		
		LogMsg(Log::L_ERROR, _strError.GetBuffer(0));
	}
	catch(...)
	{
		_strError.Format(_T("Caught Unknown Exception in FtpPullToPush_Queue::run"));
		LogMsg(Log::L_ERROR, _strError.GetBuffer(0));
	}

	//add buffer queue end flag
	_buffQueue.DataBufferEnd();
	LogMsg(Log::L_DEBUG, _T("FtpPullToPush_Queue ReadData Thread Leave"));

	return 0;
}







//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


	
bool FtpPullBuf_Queue::OpenFile(LPCTSTR sUrl)
{
	{
		CString	strServer, strObject, strUsername, strPassword;
		INTERNET_PORT nPort;
		DWORD	dwServiceType;

		//get server, user, pwd, port from url
		BOOL bRet = AfxParseURLEx(
			sUrl,
			dwServiceType,
			strServer,
			strObject, 
			nPort, 
			strUsername, 
			strPassword);

		if (!bRet)
		{
			_strError.Format(_T("url: [%s] parse error."), sUrl);
			return false;
		}

		if (dwServiceType != AFX_INET_SERVICE_FTP)
		{
			_strError.Format(_T("url: [%s] error, not ftp protocol."), sUrl);
			return false;
		}

		//if some value is not set, set with default value(port is not need to set)
		if (strUsername.IsEmpty())
		{
			strUsername = _T("Anonymous");
			strPassword = _T("foo@bar.com");
		}

		//log all param parsed
		LogMsg(Log::L_DEBUG, _T("Pull FTP: Server[%s], Port[%d], File[%s], User[%s], Pwd[%s]."), 
			strServer.GetBuffer(0),
			(int)nPort,
			strObject.GetBuffer(0),
			strUsername.GetBuffer(0),
			strPassword.GetBuffer(0));
		
		try
		{
			_pPullFtp = _csPull.GetFtpConnection(strServer, strUsername, strPassword, nPort);
		}
		catch (CInternetException* pEx)
		{
			TCHAR sz[1024];
			pEx->GetErrorMessage(sz, 1024);
			_strError.Format(_T("Caught CInternetException in GetFtpConnection of Pull[%s]: %s"), sUrl, sz);
			pEx->Delete();
			return false;
		}

		
		//if file not exist, exception
		try
		{
			_pPullFile = _pPullFtp->OpenFile(strObject, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY);
		}
		catch (CInternetException* pEx)
		{
			TCHAR sz[1024];
			pEx->GetErrorMessage(sz, 1024);
			_strError.Format(_T("Caught CInternetException in CFtpConnection::OpenFile() : %s"), sz);
			pEx->Delete();
			return false;
		}

		if (!_pPullFile)
		{
			_strError.Format(_T("CFtpConnection::OpenFile() : Open File(%s) fail."), strObject.GetBuffer(0));
			return false;
		}

		//when file is bigger than 4G, the return value is error, need to report to Maynard.
		DWORD dwLow = 0, dwHigh = 0;
		HINTERNET  hFile = (HINTERNET)(*_pPullFile);
		dwLow = FtpGetFileSize(hFile, &dwHigh);
		((DWORD*)&_llTotalBytes)[1]  = dwHigh;
		((DWORD*)&_llTotalBytes)[0]  = dwLow;
	}

	_bRunning = true;
	start();

	return true;
}

int FtpPullBuf_Queue::run(void)
{
	try
	{
		void*	buf;
		int		nLen, nRead;
		BOOL	bRet;

		LogMsg(Log::L_DEBUG, _T("FtpPullBuf_Queue ReadData Thread Enter"));

		do
		{
			bRet = _buffQueue.AllocateBuffer(&buf, &nLen);
			if (!bRet)
			{
				LogMsg(Log::L_ERROR, _T("AllocateBuffer fail"));
				_strError.Format(_T("AllocateBuffer fail"));
				LogMsg(Log::L_ERROR, _strError.GetBuffer(0));
				break;
			}

			nRead = _pPullFile->Read(buf, nLen);
			if(nRead<1)
			{
				_buffQueue.ReleaseBuffer(buf);
				break;
			}

			_buffQueue.AddDataBuffer(buf, nRead);			
		}while(nRead == nLen && _bRunning);
	}
	catch (CInternetException* pEx)
	{
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		_strError.Format(_T("Caught CInternetException : %s"), sz);
		pEx->Delete();	
		LogMsg(Log::L_ERROR, _strError.GetBuffer(0));
	}

	_bRunning = false;
	//add buffer queue end flag
	_buffQueue.DataBufferEnd();
	LogMsg(Log::L_DEBUG, _T("FtpPullBuf_Queue ReadData Thread Leave"));

	return 0;
}


BOOL FtpPush_Queue::StartPush(LPCTSTR sUrlToPush, DWORD dwMaxSpeedbps /*= 0*/)
{
	_dwMaxSpeedbps = dwMaxSpeedbps;

	try
	{
		_llTotalBytes = 0;

		// open upload ftp connection
		{
			CString	strServer, strObject, strUsername, strPassword;
			INTERNET_PORT nPort;
			DWORD	dwServiceType;

			//get server, user, pwd, port from url
			BOOL bRet = AfxParseURLEx(
				sUrlToPush,
				dwServiceType,
				strServer,
				strObject, 
				nPort, 
				strUsername, 
				strPassword);

			if (!bRet)
			{
				_strError.Format(_T("url: [%s] parse error."), sUrlToPush);
				return false;
			}

			if (dwServiceType != AFX_INET_SERVICE_FTP)
			{
				_strError.Format(_T("url: [%s] error, not ftp protocol."), sUrlToPush);
				return false;
			}

			//if some value is not set, set with default value(port is not need to set)
			if (strUsername.IsEmpty())
			{
				strUsername = _T("Anonymous");
				strPassword = _T("foo@bar.com");
			}

			//log all param parsed
			LogMsg(Log::L_DEBUG, _T("Push FTP: Server[%s], Port[%d], File[%s], User[%s], Pwd[%s]."), 
				strServer.GetBuffer(0),
				(int)nPort,
				strObject.GetBuffer(0),
				strUsername.GetBuffer(0),
				strPassword.GetBuffer(0));
			
			try
			{
				_pPushFtp = _csPush.GetFtpConnection(strServer, strUsername, strPassword, nPort);
			}
			catch (CInternetException* pEx)
			{
				TCHAR sz[1024];
				pEx->GetErrorMessage(sz, 1024);
				_strError.Format(_T("Caught CInternetException in GetFtpConnection of Push[%s]: %s"), sUrlToPush, sz);
				pEx->Delete();
				return false;
			}

			//if file exist, will?
			try
			{
				_pPushFile = _pPushFtp->OpenFile(strObject, GENERIC_WRITE, FTP_TRANSFER_TYPE_BINARY);
			}
			catch (CInternetException* pEx)
			{
				TCHAR sz[1024];
				pEx->GetErrorMessage(sz, 1024);
				_strError.Format(_T("Caught CInternetException in CFtpConnection::OpenFile() : %s"), sz);
				pEx->Delete();
				return false;
			}

			if (!_pPushFile)
			{
				_strError.Format(_T("CFtpConnection::OpenFile() : Open File(%s) fail."), strObject.GetBuffer(0));
				return false;
			}			
		}
		
		//start the write data thread
		_bRunning = true;
		start();
	}
	catch (CInternetException* pEx)
	{
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		_strError.Format(_T("Caught CInternetException : %s"), sz);
		pEx->Delete();

		_bRunning = false;
		return false;
	}

	return TRUE;	
}

int FtpPush_Queue::run(void)
{
	try
	{
		void*	buf;
		int		nLen;

		LogMsg(Log::L_DEBUG, _T("FtpPush_Queue WriteData Thread Enter"));


		_llProcBytes = 0;
		DWORD	dwT1 = GetTickCount();

		//for speed limit
		DWORD dwIntervalSent = 0; DWORD dwLast, dwCur;
		DWORD dwCheckSent = DWORD(((float)_dwMaxSpeedbps) * FTP_XFERRATE_INTERVAL/(8*1000));
		DWORD dwShouldSent, dwTotalSent = 0;
		dwLast = dwT1;

		while(_bRunning && _buffQueue.GetDataBuffer(&buf, &nLen))
		{
//			printf("before writeing-------\n");
			_pPushFile->Write(buf, nLen);		
//			printf("writeing......\n");
			_buffQueue.ReleaseBuffer(buf);
			_llProcBytes += nLen;
//			printf("writeing-------\n");

			buf = NULL;
			
			if (_dwMaxSpeedbps)
			{
				dwIntervalSent += nLen;
				
				if (dwIntervalSent >= dwCheckSent)
				{
					dwTotalSent += dwIntervalSent;
					dwCur  = GetTickCount();
					if (dwCur >= dwLast)
					{
						dwShouldSent = int(((double)_dwMaxSpeedbps) * (dwCur - dwLast)/(8*1000));

						if (dwTotalSent > dwShouldSent)
						{
							int nToSleep = int((dwTotalSent - dwShouldSent)*8000.0/_dwMaxSpeedbps);
							Sleep(nToSleep);

							dwCur  = GetTickCount();
							if (dwCur != dwLast)
								LogMsg(Log::L_DEBUG, _T("Sleep time is %d, speed: %d"), nToSleep, int(dwTotalSent*8000.0/(dwCur - dwLast)));
						}
					}

					if (dwTotalSent > GLOABL_MAXBBYTES)
					{
						// reset the counter to process file bigger than 4G
						dwLast = GetTickCount();
						dwTotalSent = 0;
					}
					
					dwIntervalSent = 0;
				}
			}
		}		

		DWORD	dwT2 = GetTickCount();

		DWORD	dwRate;
		if (dwT2 == dwT1)
		{
			dwRate = 0;
		}
		else
		{
			dwRate = (_llProcBytes*8000/(dwT2 - dwT1));
		}

		LogMsg(Log::L_DEBUG, _T("speed is:  %d bps"), dwRate);
	}
	catch (CInternetException* pEx)
	{
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		_strError.Format(_T("Caught CInternetException : %s"), sz);
		pEx->Delete();		
		LogMsg(Log::L_ERROR, _strError.GetBuffer(0));
	}
	catch(...)
	{
		_strError.Format(_T("Caught Unknown Exception in FtpPush_Queue::run"));
		LogMsg(Log::L_ERROR, _strError.GetBuffer(0));
	}

	_bRunning = FALSE;
	
	//add buffer queue end flag
	_buffQueue.DataBufferEnd();
	LogMsg(Log::L_DEBUG, _T("FtpPush_Queue WriteData Thread Leave"));

	return 0;
}

void FtpPush_Queue::WaitForFinish(DWORD dwTimeOut/* = INFINITE*/)
{
	NativeThread::waitHandle(dwTimeOut);
}

void FtpPush_Queue::Terminal()
{
	_buffQueue.DataBufferEnd();
	_bRunning = false;
	NativeThread::waitHandle(INFINITE);
}

FtpPush_Queue::~FtpPush_Queue()
{
	if (_bRunning)
	{
		_bRunning = false;
		WaitForFinish();
	}
}