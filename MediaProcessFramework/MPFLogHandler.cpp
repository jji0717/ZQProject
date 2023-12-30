#include "MPFLogHandler.h"

MPF_NAMESPACE_BEGIN

// Default log verbosity: 0 for no messages through 7 (writes everything)
int MPFLogHandler::_verbosity = 7;


/// DefaultLogHandler just output messages to console window under DEBUG mode
static class DefaultLogHandler : public MPFLogHandler
{
public:

	void writeMessage(const char* msg) 
	{ 
#ifdef _DEBUG
		printf("%s\n",msg);
#endif  
	}

} defaultLogHandler;

MPFLogHandler::~MPFLogHandler()
{
	_logHandler = NULL;
}

// Message log singleton
MPFLogHandler* MPFLogHandler::_logHandler = &defaultLogHandler;

void MPFLogHandler::operator() (int level, const char* fmt, ...)
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

MPFLogHandler* MPFLogHandler::getLogHandler()
{
	return _logHandler;
}

void MPFLogHandler::setLogHandler(MPFLogHandler* lh)
{
	if (NULL == lh)
		_logHandler = &defaultLogHandler;
	else
		_logHandler = lh;
}

int MPFLogHandler::getVerbosity()
{
	return _verbosity;
}

void MPFLogHandler::setVerbosity(int v)
{
	_verbosity = v;
}

MPF_NAMESPACE_END