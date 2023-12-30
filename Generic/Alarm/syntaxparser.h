
#ifndef _ZQ_SYNTAXPARSER_H_
#define _ZQ_SYNTAXPARSER_H_
	
#pragma warning(disable:4786)
#include <list>
#include <vector>
#include <string>
#include "implementation.h"
#include "datasource.h"
	
TG_BEGIN
	
class SyntaxParser
{
private:
	std::vector<Implementation*>	m_arrImpl;
	DataSource&						m_datasource;
	
public:
	SyntaxParser(DataSource& datasource);

	bool compare(size_t sourceid, const char* line);
	virtual size_t OnMatch(const char* line, const char* syntax, char** params,
		size_t max_buffer, size_t max_count) = 0;

	void addImpl(Implementation& impl);
	bool removeImpl(size_t index);
	Implementation& listImpl(size_t index);
	size_t countImpl();
};
	
TG_END
	
#endif//_ZQ_SYNTAXPARSER_H_
	