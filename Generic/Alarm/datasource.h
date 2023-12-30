
#ifndef _ZQ_DATASOURCE_H_
#define _ZQ_DATASOURCE_H_

#include "alarmcommon.h"
#include <string>

TG_BEGIN

class DataSource
{
public:
	virtual size_t countSource() = 0;
	virtual std::string listSource(size_t sourceid) = 0;
	
	virtual size_t countSyntax(size_t sourceid) = 0;
	virtual std::string listSyntax(size_t sourceid, size_t syntaxid) = 0;

	virtual size_t countData(size_t sourceid , size_t syntaxid) = 0;
	virtual std::string listDataKey(size_t sourceid , size_t syntaxid, size_t dataid) = 0;
	virtual std::string getDataValue(size_t sourceid , size_t syntaxid, const char* datakey) = 0;
};

TG_END

#endif//_ZQ_DATASOURCE_H_
