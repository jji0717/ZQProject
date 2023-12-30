
#include "syntaxparser.h"
#include "loghandler.h"
	
TG_BEGIN

SyntaxParser::SyntaxParser(DataSource& datasource)
:m_datasource(datasource)
{
}

bool SyntaxParser::compare(size_t sourceid, const char* line)
{
	char** pstrParam = new char*[MAX_PARAM_LINE_COUNT];
	for (size_t i = 0; i < MAX_PARAM_LINE_COUNT; ++i)
	{
		pstrParam[i] = new char[MAX_PARAM_LINE_LENGTH];
		memset(pstrParam[i], 0, MAX_PARAM_LINE_LENGTH);
	}

	size_t zSyntaxCount = m_datasource.countSyntax(sourceid);
	for (size_t j = 0; j < zSyntaxCount; ++j)
	{
		size_t match_count = OnMatch(line, m_datasource.listSyntax(sourceid, j).c_str(), 
			pstrParam, MAX_PARAM_LINE_LENGTH, MAX_PARAM_LINE_COUNT);

		if (match_count > 0)
		{
			for (int k = 0; k < m_arrImpl.size(); ++k)
			{
				if (!(m_arrImpl[k])->action(sourceid, j, line, pstrParam, match_count))
				{
					Log(LogHandler::L_ERROR, "[SyntaxParser::compare] action return false");
				}
			}
		}
	}

	for (i = 0; i < MAX_PARAM_LINE_COUNT; ++i)
		delete[] pstrParam[i];
	delete[] pstrParam;

	return true;
}

void SyntaxParser::addImpl(Implementation& impl)
{
	m_arrImpl.push_back(&impl);
}

bool SyntaxParser::removeImpl(size_t index)
{
	return (m_arrImpl.end() != m_arrImpl.erase(m_arrImpl.begin()+index));
}

Implementation& SyntaxParser::listImpl(size_t index)
{
	return *m_arrImpl[index];
}

size_t SyntaxParser::countImpl()
{
	return m_arrImpl.size();
}

TG_END
	
