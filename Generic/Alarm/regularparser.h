
#ifndef _ZQ_REGULARPARSER_H_
#define _ZQ_REGULARPARSER_H_
	
#include "syntaxparser.h"
	
TG_BEGIN
	
class RegularParser : public SyntaxParser
{
public:
	RegularParser(DataSource& datasource);

	size_t OnMatch(const char* line, const char* syntax, char** params,
		size_t max_buffer, size_t max_count);
};
	
TG_END
	
#endif//_ZQ_REGULARPARSER_H_
	