
#include "trigger.h"
#include "loghandler.h"
	
TG_BEGIN
	
int Trigger::run()
{
	char* strLine = new char[m_zLogLineLength];
	while(!m_bQuit)
	{
		memset(strLine, 0, m_zLogLineLength);
		if (OnTail(strLine, m_zLogLineLength))
		{
			if (m_bQuit)
				break;
			if (!OnAction(m_zSourceId, strLine))
			{
				Log(LogHandler::L_ERROR, "[Trigger::run] action return false");
			}
			if (m_bQuit)
				break;
		}
		::Sleep(m_zTailTime);
	}
	delete[] strLine;
	return 0;
}

Trigger::Trigger(const char* logfile, SyntaxParser& parser, 
				 size_t sourceid, size_t zLogLineLength, size_t zTailTime)
	:m_zTailTime(zTailTime), m_zLogLineLength(zLogLineLength),
	m_bQuit(false), m_zSourceId(sourceid), m_parser(parser)
{
	if (NULL == logfile)
		throw std::exception("[Trigger::Trigger] log file is NULL pointer");

	m_strLogFile = logfile;
}

bool Trigger::OnAction(size_t sourceid, const char* logline)
{
	return m_parser.compare(sourceid, logline);
}

bool Trigger::quit()
{
	m_bQuit = true;
	exit();

	return 0 == waitHandle(DEF_WAIT_EXIT_TIMEOUT);
}
	
TG_END
	