// DataDebug.cpp: implementation of the DataDebug class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataDebug.h"

#ifdef _DEBUG

namespace DataStream {

DataDebug::DataDebug()
{
	memset(_dbgCounter, 0, sizeof(_dbgCounter));
}

DataDebug::~DataDebug()
{

}

#define DUMP_COUNTER(id)		dbgPrint("%s = %d", #id, _dbgCounter[id])

void DataDebug::finalDetect()
{
	dbgPrint("DataDebug::finalDetect()\tdump:");
	DUMP_COUNTER(DBG_STDSENDER_COUNTER);
	DUMP_COUNTER(DBG_STDREADER_COUNTER);
	DUMP_COUNTER(DBG_PSISENDER_COUNTER);
}

DataDebug& DataDebug::getDataDebug()
{
	static DataDebug dataDebugInstance;
	return dataDebugInstance;
}

} // namespace DataStream {

#endif // #ifdef _DEBUG
