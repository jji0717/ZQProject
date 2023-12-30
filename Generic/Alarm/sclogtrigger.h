
#ifndef _ZQ_SCLOGTRIGGER_H_
#define _ZQ_SCLOGTRIGGER_H_
	
#include "trigger.h"
#include <string>
#include "syntaxparser.h"
		
TG_BEGIN
	
class SCLogTrigger : public Trigger
{
private:
	size_t			m_zFilePointer;

protected:
	bool OnTail(char* logline, size_t length);
	
public:
	SCLogTrigger(const char* logfile, SyntaxParser& parser, size_t sourceid);

	static unsigned long getLogLength(HANDLE hLogFile);
	static int readLine(HANDLE hLogFile, char* strLine, size_t zLen, const char* sign = NEXT_LINE_SIGN);
	static int readLogFileLine(HANDLE hLogFile, char* strLine, size_t zLen);
};
	
TG_END
	
#endif//_ZQ_SCLOGTRIGGER_H_
	