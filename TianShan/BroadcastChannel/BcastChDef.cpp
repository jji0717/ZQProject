#include "BcastChDef.h"
#include "TimeUtil.h"
bool localTime2TianShanTime(const char* szTime, int64& lTime)
{
	char bufUTC[64] = "";
	if(!ZQ::common::TimeUtil::Local2Iso(szTime, bufUTC, sizeof(bufUTC)))
		return false;
//	if(ZQ::common::TimeUtil::Iso2Time64(bufUTC, lTime))
//		return false;

	if(ZQ::common::TimeUtil::Iso2Time(bufUTC, lTime))
		return false;
	return  true; 
}

bool systemTime2TianShanTime(const char* szTime, int64& lTime)
{
	return ZQ::common::TimeUtil::Iso2Time(szTime, lTime);
}