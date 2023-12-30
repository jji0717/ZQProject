
#ifndef _ZQ_INIDATASOURCE_H_
#define _ZQ_INIDATASOURCE_H_

#pragma warning(disable:4786)
#include "datasource.h"
#include <string>
#include <vector>
#include <map>

#define MAIN_SECTION		"Main"
#define GROUP_SECTION		"Group"

#define LOG_COUNT_KEY		"LogCount"
#define LOG_LOCATION_KEY	"LogLocation"
#define TRIGGER_NUMBER_KEY	"TriggerNumber"
#define SECTION_NAME_KEY	"SectionName"
#define SYNTAX_KEY			"InputSyntax"

TG_BEGIN

struct DataBlock
{
	std::string							syntax;
	std::map<std::string, std::string>	kvpair;
};

struct LogBlock
{
	std::string				filename;
	std::vector<DataBlock>	trigger;
};

struct SourceBlock
{
	std::string				inifile;
	std::vector<LogBlock>	log;
};

class INIDataSource : public DataSource
{
private:
	SourceBlock	m_source;

public:
	INIDataSource(const char* inifile);

	size_t countSource();
	std::string listSource(size_t sourceid);
	
	size_t countSyntax(size_t sourceid);
	std::string listSyntax(size_t sourceid, size_t syntaxid);

	size_t countData(size_t sourceid , size_t syntaxid);
	std::string listDataKey(size_t sourceid , size_t syntaxid, size_t dataid);
	std::string getDataValue(size_t sourceid , size_t syntaxid, const char* datakey);
};

TG_END

#endif//_ZQ_INIDATASOURCE_H_
