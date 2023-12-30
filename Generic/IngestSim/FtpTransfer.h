// FtpTransfer.h: interface for the FtpTransfer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FTPTRANSFER_H__570E5AD8_91D3_4D89_B3F2_73632F4ADBA0__INCLUDED_)
#define AFX_FTPTRANSFER_H__570E5AD8_91D3_4D89_B3F2_73632F4ADBA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "afx.h"
#include <afxwin.h>
#include <afxinet.h>

template <class T>
class CMFCAutoPtr
{
public:
	CMFCAutoPtr()
	{
		p = NULL;
	}
	~CMFCAutoPtr()
	{
		if (p != NULL)
		{
			p->Close();
			delete p;			
			p = NULL;
		}
	}
	T*& operator->()
	{
		return p;
	}
	operator T*() const
	{
		return p;
	}
	T& operator*() const
	{
		return *p;
	}
	T* operator=(T* lp)
	{
		p = lp;
		return p;
	}
	bool operator!() const
	{
		return (p == NULL);
	}

private:	
	T* p;
};

extern void LogMsg(DWORD dwTraceLevel, LPCTSTR lpszFmt, ...);

class FtpPullToPush
{
public:
	FtpPullToPush()
	{
		_llProcBytes = 0;
		_llTotalBytes = 0;
	}

	LONGLONG getProcBytes() {return _llProcBytes;}
	LONGLONG getTotalBytes() { return _llTotalBytes;}

	BOOL PullToPush(LPCTSTR sUrlToPull, LPCTSTR sUrlToPush, DWORD dwMaxSpeedbps = 0);
	LPCTSTR getErrorDesc() {return _strError.GetBuffer(0);}

private:
	LONGLONG	_llProcBytes;
	LONGLONG	_llTotalBytes;
	CString		_strError;
};


class FtpPullBuf
{
public:
	FtpPullBuf():_csPull(_T("Pull"))
	{
		_llProcBytes = 0;
		_llTotalBytes = 0;
	}
	~FtpPullBuf()
	{
	}

	LONGLONG getProcBytes() {return _llProcBytes;}
	LONGLONG getTotalBytes() { return _llTotalBytes;}

	bool OpenFile(LPCTSTR sUrl);
	int getData(void* buf, int nLen);
	LPCTSTR getErrorDesc() {return _strError.GetBuffer(0);}

private:
	LONGLONG	_llProcBytes;
	LONGLONG	_llTotalBytes;
	CString		_strError;

	CInternetSession			_csPull;
	CMFCAutoPtr<CFtpConnection> _pPullFtp;
	CMFCAutoPtr<CInternetFile>	_pPullFile;
};

#include <deque>

using namespace std;

class DownloadBufferQueue
{
public:
	enum 
	{
		MAX_DATA_QUEUE_DEPTH = 1800,
		DATA_BUFFER_SIZE = 4096*16
	};

	DownloadBufferQueue(int nBufSize = DATA_BUFFER_SIZE, int nMaxQueueDepth = MAX_DATA_QUEUE_DEPTH):_buffersize(nBufSize), _maxqueuedepth(nMaxQueueDepth), _queuedepth(0)
	{
		InitializeCriticalSection(&_mutex);
		_dataend = false;
	}

	~DownloadBufferQueue()
	{
		EnterCriticalSection(&_mutex);

		void* pBuf;
		while(!_allbuffers.empty())
		{
			pBuf = _allbuffers.front();
			delete pBuf;

			_allbuffers.pop_front();
		}

		LeaveCriticalSection(&_mutex);

		DeleteCriticalSection(&_mutex);
	}

/*   d Initialize(int nBufSize)
	{

	}

	void UnInitialize();
*/
	BOOL AllocateBuffer(void**pPointer, int*pLength)
	{
		EnterCriticalSection(&_mutex);
		
		void* pBuf;
		if (_bufferqueue.empty())
		{
			pBuf = new BYTE[_buffersize + 4];
			if (pBuf)
			{
				_allbuffers.push_back(pBuf);

				*pPointer = (void*)((BYTE*)pBuf + 4);
				*pLength = _buffersize;
			}
			else
			{
				*pPointer = 0;
				*pLength = 0;
			}
		}
		else
		{
			pBuf = _bufferqueue.front();
			_bufferqueue.pop_front();
			*pPointer = (void*)((BYTE*)pBuf + 4);
			*pLength = _buffersize;
		}

		LeaveCriticalSection(&_mutex);

		return (*pLength != 0);
	}

	void AddDataBuffer(void* pPointer, int nLength)
	{
		//
		// add the element to the queue
		//
		EnterCriticalSection(&_mutex);
		
		_queuedepth++;
		void* pBuf;
		if (pPointer)
		{
			pBuf =  (void*)((BYTE*)pPointer - 4);
			*((int*)pBuf) = nLength;
		}
		else
		{
			pBuf = NULL;
		}

		_dataqueue.push_back(pBuf);

		_dataend = false;
		
		LeaveCriticalSection(&_mutex);

		if (_queuedepth >= _maxqueuedepth/2)
		{
			LogMsg(7, "buffer deep %d", _queuedepth);
		}

		while (_queuedepth >= _maxqueuedepth)
		{
			Sleep(10);
		}
	}

	void DataBufferEnd()
	{
		AddDataBuffer(0, 0);
	}

	// once the return value is false, cann't call this any more
	// return false means all data is get
	BOOL GetDataBuffer(void**pPointer, int*pLength);

	void ReleaseBuffer(void*pPointer)
	{
		void* pBuf =  (void*)((BYTE*)pPointer - 4);

		EnterCriticalSection(&_mutex);

		_bufferqueue.push_back(pBuf);						

		LeaveCriticalSection(&_mutex);	
	}

	int getDep(){return _queuedepth;}
private:

	CRITICAL_SECTION  _mutex;

	deque<void*>	_allbuffers;
	deque<void*>	_bufferqueue;
	deque<void*>	_dataqueue;
	long			_queuedepth;				// IO queue count		
	long			_buffersize;
	long			_maxqueuedepth;

	bool			_dataend;		//if true, no more data, all is read to data queue
};

#include "NativeThread.h"

class FtpPullToPush_Queue :public ZQ::common::NativeThread
{
public:
	FtpPullToPush_Queue():_csPull(_T("Pull")), _csPush(_T("Push"))
	{
		_llProcBytes = 0;
		_llTotalBytes = 0;
		_bRunning = false;
	}

	~FtpPullToPush_Queue();

	LONGLONG getProcBytes() {return _llProcBytes;}
	LONGLONG getTotalBytes() { return _llTotalBytes;}

	BOOL PullToPush(LPCTSTR sUrlToPull, LPCTSTR sUrlToPush, DWORD dwMaxSpeedbps = 0);
	LPCTSTR getErrorDesc() {return _strError.GetBuffer(0);}

	virtual int run(void);

private:
	LONGLONG	_llProcBytes;
	LONGLONG	_llTotalBytes;
	CString		_strError;

	CInternetSession _csPull;
	CMFCAutoPtr<CFtpConnection> _pPullFtp;
	CMFCAutoPtr<CInternetFile> _pPullFile;

	CInternetSession _csPush;
	CMFCAutoPtr<CFtpConnection> _pPushFtp;
	CMFCAutoPtr<CInternetFile> _pPushFile;

	bool				_bRunning;
	DownloadBufferQueue		_buffQueue;
};

class FtpPush_Queue :public ZQ::common::NativeThread
{
public:
	FtpPush_Queue():_csPush(_T("Push"))
	{
		_llProcBytes = 0;
		_llTotalBytes = 0;
		_bRunning = false;
	}

	~FtpPush_Queue();

	LONGLONG getProcBytes() {return _llProcBytes;}
	LONGLONG getTotalBytes() { return _llTotalBytes;}

	BOOL StartPush(LPCTSTR sUrlToPush, DWORD dwMaxSpeedbps = 0);
	LPCTSTR getErrorDesc() {return _strError.GetBuffer(0);}

	BOOL AllocateBuffer(void**pPointer, int*pLength)
	{
		return _buffQueue.AllocateBuffer(pPointer, pLength);
	}

	void AddDataBuffer(void* pPointer, int nLength)
	{
		_buffQueue.AddDataBuffer(pPointer, nLength);
	}

	void ReleaseBuffer(void*pPointer)
	{
		_buffQueue.ReleaseBuffer(pPointer);
	}

	void DataBufferEnd()
	{
		_buffQueue.DataBufferEnd();
	}

	bool IsRunning(){return _bRunning;
	}

	void WaitForFinish(DWORD dwTimeOut = INFINITE);

	void Terminal();

protected:
	virtual int run(void);

private:
	LONGLONG	_llProcBytes;
	LONGLONG	_llTotalBytes;
	CString		_strError;
	DWORD		_dwMaxSpeedbps;

	CInternetSession _csPush;
	CMFCAutoPtr<CFtpConnection> _pPushFtp;
	CMFCAutoPtr<CInternetFile> _pPushFile;

	bool				_bRunning;
	DownloadBufferQueue		_buffQueue;
};


class FtpPullBuf_Queue :public ZQ::common::NativeThread
{
public:
	FtpPullBuf_Queue():_csPull(_T("Pull"))
	{
		_llProcBytes = 0;
		_llTotalBytes = 0;
		_bRunning = false;
	}
	~FtpPullBuf_Queue()
	{
		if (_bRunning)
		{
			_bRunning = false;
			waitHandle(INFINITE);
		}		
	}

	LONGLONG getProcBytes() {return _llProcBytes;}
	LONGLONG getTotalBytes() { return _llTotalBytes;}

	bool OpenFile(LPCTSTR sUrl);

	BOOL getDataBuffer(void** ppBuf, int* nLen) 
	{
		BOOL bRet = _buffQueue.GetDataBuffer(ppBuf, nLen);
		if (bRet)
		{
			_llProcBytes+=*nLen;
		}
		return bRet;
	}
	void freeDataBuffer(void* pBuf){_buffQueue.ReleaseBuffer(pBuf);}

	LPCTSTR getErrorDesc() {return _strError.GetBuffer(0);}

	virtual int run(void);

private:
	LONGLONG	_llProcBytes;
	LONGLONG	_llTotalBytes;
	CString		_strError;

	CInternetSession			_csPull;
	CMFCAutoPtr<CFtpConnection> _pPullFtp;
	CMFCAutoPtr<CInternetFile>	_pPullFile;

	bool				_bRunning;
	DownloadBufferQueue		_buffQueue;
};

#endif // !defined(AFX_FTPTRANSFER_H__570E5AD8_91D3_4D89_B3F2_73632F4ADBA0__INCLUDED_)
