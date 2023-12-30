
#include "LogHandler.h"
#include <stdio.h>

TG_BEGIN

// Default log verbosity: 0 for no messages through 7 (writes everything)
int LogHandler::_verbosity = 7;


/// DefaultLogHandler just output messages to console window under DEBUG mode
static class DefaultLogHandler : public LogHandler
{
public:

	void writeMessage(const char* msg) 
	{ 
#ifdef _DEBUG
		printf("%s\n",msg);
#endif  
	}

} defaultLogHandler;

LogHandler::~LogHandler()
{
	_logHandler = NULL;
}

// Message log singleton
LogHandler* LogHandler::_logHandler = &defaultLogHandler;

void LogHandler::operator() (int level, const char* fmt, ...)
{
	if (NULL == _logHandler)
		return;

	if ((level & 0xff) > _verbosity)
		return;

	char msg[2048];
	va_list args;

	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	writeMessage(msg);
}

LogHandler* LogHandler::getLogHandler()
{
	return _logHandler;
}

void LogHandler::setLogHandler(LogHandler* lh)
{
	if (NULL == lh)
		_logHandler = &defaultLogHandler;
	else
		_logHandler = lh;
}

int LogHandler::getVerbosity()
{
	return _verbosity;
}

void LogHandler::setVerbosity(int v)
{
	_verbosity = v;
}

TG_END
