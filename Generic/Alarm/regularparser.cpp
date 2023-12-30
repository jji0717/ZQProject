
#include "regularparser.h"
#include <boost\regex.hpp>
#include "loghandler.h"
#include <exception>
	
TG_BEGIN

RegularParser::RegularParser(DataSource& datasource)
:SyntaxParser(datasource)
{
}

size_t RegularParser::OnMatch(const char* line, const char* syntax, char** params,
		size_t max_buffer, size_t max_count)
{
	if (NULL == line)
	{
		Log(LogHandler::L_ERROR, "[RegularParser::OnMatch] the match line is empty");
		//assert(false);
		return 0;
	}

	if (NULL == syntax)
	{
		Log(LogHandler::L_ERROR, "[RegularParser::OnMatch] the match syntax is empty");
		//assert(false);
		return 0;
	}

	if (NULL == params)
	{
		Log(LogHandler::L_ERROR, "[RegularParser::OnMatch] the parameters buffer is NULL, maybe you have not allocater the memory or the memory error");
	}
	
	boost::regex reg(syntax);
	boost::cmatch cm;
	const char* line_end = line+strlen(line);
	if (!boost::regex_match(line, line_end, cm, reg))
		return 0;

	size_t countnumber = __min(max_count, cm.size());
	for (size_t i = 0; i < countnumber; ++i)
	{
		int copynumber = __min(max_buffer-1, cm[i].second-cm[i].first);
		strncpy(params[i], cm[i].first, copynumber);
	}
	return countnumber;
}

TG_END
