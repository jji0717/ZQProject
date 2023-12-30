#pragma once

#include <tchar.h>
#include "fltinit.h"
#include <windows.h>

class CSvcLog: public ISvcLog {
public:

	enum {
		MAX_QUEUE_SIZE = 128
	};

	enum {
		BEEP_INIT_FAILED,
	};

	CSvcLog();
	virtual ~CSvcLog();

	virtual void log(LogLevel level, const char* fmt, ...);
	virtual void log0(LogLevel level, const char* str);
	virtual LogLevel getLevel();
	virtual unsigned int getMaxSize();

	bool init(const TCHAR* fileName, LogLevel level, 
		unsigned int fileSize);
	void uninit();
	bool setSevel(LogLevel level);
	bool setMaxSize(unsigned int size);
	void beep(int type);

	void flush();

protected:
	void _writeLog(const char* str);
	void writeLog(LogLevel level, const char* str);
	void writeHdr();
	static unsigned long __stdcall _logThreadProc(void* param);
	unsigned long run();
	void pushLog(LogLevel level, const char* str);

	bool _popItem(char* str);

	void quit();

	static const char* getLevelName(LogLevel level);

protected:
	LogLevel			_curLevel;
	size_t				_maxLogSize;
	size_t				_hdrLen;
	// unsigned int		_index;
	CRITICAL_SECTION	_logCritSect;
	CRITICAL_SECTION	_logQueueCritSect;
	HANDLE				_logFile;
	HANDLE				_logEvent;
	HANDLE				_logThread;
	HANDLE				_logSem;
	bool				_quit;
	char				_logStrings[MAX_QUEUE_SIZE][1024];
	size_t				_queueSize;
};
