// ActiveData.cpp: implementation of the ActiveData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ActiveData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ActiveData::ActiveData()
{

}

ActiveData::~ActiveData()
{

}

bool ActiveData::initchannel()
{
	return true;
}

void ActiveData::uninit()
{


}
void 
ActiveData::GetCurrentDatatime(std::string& strTime)
{
	char strtime[20];
	SYSTEMTIME time; 
	GetLocalTime(&time);
	sprintf(strtime,"%04d%02d%02d%02d%02d%02d%03d",time.wYear, time.wMonth, 
		time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	strTime = strtime;
}