// ActiveChannel.cpp: implementation of the ActiveChannel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ActiveChannel.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ActiveChannel::ActiveChannel()
{

}

ActiveChannel::~ActiveChannel()
{

}

bool ActiveChannel::initchannel()
{
	return true;
}

void ActiveChannel::uninit()
{


}
void 
ActiveChannel::GetCurrentDatatime(std::string& strTime)
{
	char strtime[20];
	SYSTEMTIME time; 
	GetLocalTime(&time);
	sprintf(strtime,"%04d%02d%02d%02d%02d%02d%03d",time.wYear, time.wMonth, 
		time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	strTime = strtime;
}