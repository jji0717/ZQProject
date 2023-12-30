#include "svclog.h"
#include <atlbase.h>
#include <stdio.h>
#include <assert.h>

#define _NO_LOG_OPTIMIZATION

size_t calcDigitalLen(unsigned int n)
{
	char buf[128];
	sprintf(buf, "%u", 0xffffffff);
	return strlen(buf) + 2 /*\r\n*/ + 1;
}


CSvcLog::CSvcLog()
{
	_curLevel = L_DEBUG;
	_maxLogSize = 0x6400000;
	_logFile = NULL;
	// _index = 0;

	_hdrLen = calcDigitalLen(_maxLogSize);

	_quit = false;
	_logEvent = NULL;
	_logThread = NULL;
	_logSem = NULL;
}

CSvcLog::~CSvcLog()
{

}

void CSvcLog::log(LogLevel level, const char* fmt, ...)
{
	char outbuf[2048];
	va_list vlist;

	if (level > _curLevel)
		return;

	va_start(vlist, fmt);
	vsprintf(outbuf, fmt, vlist);

#ifndef _NO_LOG_OPTIMIZATION
	pushLog(level, outbuf);
#else
	writeLog(level, outbuf);
#endif
}

void CSvcLog::log0(LogLevel level, const char* str)
{
	if (level > _curLevel)
		return;

#ifndef _NO_LOG_OPTIMIZATION
	pushLog(level, str);
#else
	writeLog(level, str);
#endif
}

#define GetFilePointer(hFile)		SetFilePointer(hFile, 0, NULL, FILE_CURRENT)

void CSvcLog::writeHdr()
{
	char buf[128];
	ULONG pos =	GetFilePointer(_logFile);
    sprintf(buf, "%u", pos);
	size_t realHdrLen = strlen(buf);
	assert(realHdrLen < _hdrLen - 2);
	memset(buf + realHdrLen, 0x20, _hdrLen - realHdrLen - 2);
	buf[_hdrLen - 2] = '\r';
	buf[_hdrLen - 1] = '\n';
	SetFilePointer(_logFile, 0, 0, FILE_BEGIN);
	DWORD written;
    WriteFile(_logFile, buf, _hdrLen, &written, NULL);
	SetFilePointer(_logFile, pos, 0, FILE_BEGIN);
}

void CSvcLog::writeLog(LogLevel level, const char* str)
{
	SYSTEMTIME t;

	if (_logFile == INVALID_HANDLE_VALUE) {
		ATLASSERT(false);
		ATLTRACE("CSvcLog::writeLog()\tInvalid file handle.\n");
		return;
	}

	GetLocalTime(&t);
	char outbuf[2048];

	wsprintf(outbuf,"%02d-%02d-%02d %02d:%02d:%02d:%03d[%s]\t%s\r\n",
		t.wYear,
		t.wMonth,
		t.wDay,
		t.wHour,
		t.wMinute,
		t.wSecond,
		t.wMilliseconds, 
		getLevelName(level), 
		str);

	_writeLog(outbuf);
}

unsigned long __stdcall CSvcLog::_logThreadProc(void* param)
{
	CSvcLog* thisPtr = (CSvcLog* )param;
	return thisPtr->run();
}

//FIXME: maybe some queued log will lose.
unsigned long CSvcLog::run()
{
	unsigned long r;

	while(!_quit) {
		r = WaitForSingleObject(_logEvent, 1000);
		if (r == WAIT_FAILED) {
			writeLog(L_EMERG, "CSvcLog::run()\tWaitForSingleObject(_logEvent) failed\n");
			return -1;
		}
		
		if (_quit) {
			writeLog(L_INFO, "CSvcLog::run()\tQuit...\n");
			break;
		}

		if (r == WAIT_TIMEOUT)
			continue;
	
		EnterCriticalSection(&_logQueueCritSect);
		if (_queueSize > 0) {
			const char* str = _logStrings[-- _queueSize];
			_writeLog(str);
			if (_queueSize <= 0) {
				ResetEvent(_logEvent);
			}

			ReleaseSemaphore(_logSem, 1, NULL);

		} else {
			assert(false);
		}

		LeaveCriticalSection(&_logQueueCritSect);
		
	}

	return 0;
}

void CSvcLog::_writeLog(const char* str)
{
	DWORD written;
	BOOL r;

	EnterCriticalSection(&_logCritSect);
	ULONG pos =	GetFilePointer(_logFile);
	if (pos == 0 || pos >= _maxLogSize) {
		SetFilePointer(_logFile, _hdrLen, 0, FILE_BEGIN);
	}

	r = WriteFile(_logFile, str, strlen(str), &written, NULL);
	if (!r) {
		ATLTRACE("CSvcLog::writeLog()\tWriteFile() failed.\n");
	}

	writeHdr();
	// ::FlushFileBuffers(_logFile);
	ATLTRACE(str);
	LeaveCriticalSection(&_logCritSect);
}

void CSvcLog::pushLog(LogLevel level, const char* str)
{
	SYSTEMTIME t;

	if (_logFile == INVALID_HANDLE_VALUE) {
		ATLASSERT(false);
		ATLTRACE("CSvcLog::writeLog()\tInvalid file handle.\n");
		return;
	}

	if (_quit) {
		// quit() already called.
		return;
	}

	GetLocalTime(&t);
	char outbuf[2048];

	wsprintf(outbuf,"%02d-%02d-%02d %02d:%02d:%02d:%03d[%s]\t%s\r\n",
		t.wYear,
		t.wMonth,
		t.wDay,
		t.wHour,
		t.wMinute,
		t.wSecond,
		t.wMilliseconds, 
		getLevelName(level), 
		str);

	unsigned long r;
	r = WaitForSingleObject(_logSem, 10000);

	if (r == WAIT_FAILED) {
		writeLog(L_EMERG, "CSvcLog::pushLog()\tWaitForSingleObject(_logEvent) failed\n");
		return;
	}

	if (r == WAIT_TIMEOUT) {
		writeLog(L_WARNING, "CSvcLog::pushLog()\tWaitForSingleObject(_logEvent) timeout\n");
		return;
	}

	EnterCriticalSection(&_logQueueCritSect);
	if (_queueSize >= MAX_QUEUE_SIZE) {
		assert(false);
		return;
	}

	strncpy(_logStrings[_queueSize ++], outbuf, 1023);
	if (_queueSize == 1) {
		SetEvent(_logEvent);
	}

	LeaveCriticalSection(&_logQueueCritSect);
}

CSvcLog::LogLevel CSvcLog::getLevel()
{
	return _curLevel;
}

unsigned int CSvcLog::getMaxSize()
{
	return _maxLogSize;
}

bool CSvcLog::setSevel(LogLevel level)
{
	_curLevel = level;
	return true;
}

bool CSvcLog::setMaxSize(unsigned int size)
{
	_maxLogSize = size;
	return true;
}

bool CSvcLog::init(const TCHAR* fileName, LogLevel level, 
				   unsigned int fileSize)
{
	_logFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (_logFile == INVALID_HANDLE_VALUE) {
		ATLTRACE("CSvcLog::init()\tCreateFile() failed.\n");
		return false;
	}

    char hdr[128];
	DWORD readlen;
	DWORD pos;
	if (!ReadFile(_logFile, hdr, _hdrLen, &readlen, NULL) || 
		readlen < _hdrLen) {

		pos = _hdrLen;
	} else {
		hdr[_hdrLen] = 0;
		sscanf(hdr, "%u", &pos);
		if (pos <= 0)
			pos = _hdrLen;
	}

	SetFilePointer(_logFile, pos, 0, FILE_BEGIN);
	_curLevel = level;
	_maxLogSize = fileSize;
	InitializeCriticalSection(&_logCritSect);

#ifndef _NO_LOG_OPTIMIZATION
	InitializeCriticalSection(&_logQueueCritSect);
	_logEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (_logEvent == NULL) {
		writeLog(L_EMERG, "CSvcLog::init()\tcreate _logEvent failed.");
		return false;
	}

	_logSem = CreateSemaphore(NULL, MAX_QUEUE_SIZE, MAX_QUEUE_SIZE, NULL);
	if (_logSem == NULL) {
		writeLog(L_EMERG, "CSvcLog::init()\tcreate _logSem failed.");
		return false;
	}
	unsigned long threadId;
	_logThread = CreateThread(NULL, 0, _logThreadProc, this, 0, &threadId);
	if (_logThread == NULL) {
		writeLog(L_EMERG, "CSvcLog::init()\tcreate _logThread failed.");
		return false;
	}
#endif // #ifndef _NO_LOG_OPTIMIZATION

	writeLog(L_INFO, "---- Restarting -----");
	return true;
}

void CSvcLog::beep(int type)
{
	switch(type) {
	case BEEP_INIT_FAILED:
		::Beep(1500, 500);
		::Sleep(700);
		::Beep(1500,500);
		::Sleep(700);
		::Beep(1500, 500);
		break;
	default:
		::Beep(1500, 1000);
		break;
	}
}

void CSvcLog::flush()
{
	FlushFileBuffers(_logFile);
}

void CSvcLog::quit()
{
	if (_quit)
		return;

	_quit = 1;
	if (_logEvent) {
		SetEvent(_logEvent);
		WaitForSingleObject(_logThread, INFINITE);
	}
}

void CSvcLog::uninit()
{
	if (_logFile)
		writeLog(L_INFO, "CSvcLog::uninit()\tcalling...");

	quit();
	if (_logFile)
		CloseHandle(_logFile);
	DeleteCriticalSection(&_logCritSect);
	DeleteCriticalSection(&_logQueueCritSect);
	if (_logSem)
		CloseHandle(_logSem);
	if (_logEvent)
		CloseHandle(_logEvent);
	if (_logThread)
		CloseHandle(_logThread);
}

const char* CSvcLog::getLevelName(LogLevel level)
{
	static const char* levelNames[] = {
		"EMER", "ALER",  "CRIT", "ERRO",
		"WARN", "NOTI", "INFO", "DEBU", 
		"DEBD", "UNKN", 
	};

	int index;
    if (level < L_EMERG || level > L_DEBUG_DETAIL)
		index = (int )L_DEBUG_DETAIL + 1;
	else
		index = (int )level;

	return levelNames[index];
}
