#include "StdAfx.h"
#include "SyncProXplannerCfg.h"
#include "SyncDataTrd.h"
extern ZQ::common::Config::Loader<SyncProXplannerCfg> gSyncProXplannerCfg;
extern ZQ::common::FileLog *syncLog;
extern bool bQuit;
void compartStr( const std::string& src, char delimiter, INTS& result)
{
	std::string::const_iterator it, beginPos = src.begin();
	for (it = src.begin(); it != src.end(); it ++) {
		if (*it == delimiter) {
			std::string str(beginPos, it);
			beginPos = it + 1;

			int npos = str.find_first_not_of("0123456");
			if(npos < 0) 
				//not find other letter except "01234567"
			{
				result.push_back(atoi(str.c_str()));
			}
			else
			{
				result.clear();
				return;
			}
		}
	}
	if(beginPos != src.end())
	{
		std::string str(beginPos, it);

		int npos = str.find_first_not_of("0123456");
		if(npos < 0) 
			//not find other letter except "0123456"
		{
			result.push_back(atoi(str.c_str()));
		}
		else
		{
			result.clear();
			return;
		}
	}
}

SyncDataTrd::SyncDataTrd(Peoples peoples, std::string INIfilepath): 
_peoples(peoples),_INIfilepath(INIfilepath)
{
#ifdef _DEBUG
	printf("start to automatic sync data... ");
#endif
	(*syncLog)(ZQ::common::Log::L_INFO,">>>>>>>Start to automatic sync data... ");

	_hQuit = NULL;
	_hQuit = CreateEvent(NULL, TRUE, false, NULL);
	_bExit = false;
}

SyncDataTrd::~SyncDataTrd(void)
{
	if(_hQuit)
	{
		CloseHandle(_hQuit);
		_hQuit = NULL;
	}
}

bool SyncDataTrd::init(void)	
{ 
	HANDLE hFile = ::CreateFile(_INIfilepath.c_str(),
		0,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("fail to create file '%s'\n", _INIfilepath.c_str());
		bQuit = true;
		return false;
	}
	CloseHandle(hFile);
	time_t synctime;
	if(!GetTime(synctime))
	{
		synctime = 0;
		if(!SaveTime(synctime))
		{
#ifdef _DEBUG
			printf("Fail to write sync time to file\n");
#endif
			(*syncLog)(ZQ::common::Log::L_ERROR, "Fail to write sync time to file");
			bQuit = true;
			return false;
		}
	}
    
	(*syncLog)(ZQ::common::Log::L_INFO, "day of week is '%s'",gSyncProXplannerCfg.dayofweek.c_str());

	std::string  dayofweek = gSyncProXplannerCfg.dayofweek;
	compartStr(dayofweek, ';', _dayofweek);
	if(_dayofweek.size() < 1)
	{
		(*syncLog)(ZQ::common::Log::L_ERROR, "Fail to parse weekDay: the format is '1;2;3;4'");
		bQuit = true;
		return false;
	}
	
	return true;
};
int SyncDataTrd::run(void)
{
	int interval = 60;
	if(gSyncProXplannerCfg.interval <=0 || gSyncProXplannerCfg.interval > 3540)
		gSyncProXplannerCfg.interval = 1800;
	else
        interval = gSyncProXplannerCfg.interval;

	while(!_bExit)
	{
		if(WaitForSingleObject(_hQuit, interval *1000)== WAIT_OBJECT_0)
			break;
        if(!checksync())
			continue;
		if(!AutoSyncData())
			continue;
		time_t  tnow = time(0);
		SaveTime(tnow);
	}
	return -1;
}
void SyncDataTrd::stop()
{
#ifdef _DEBUG
	printf(">>>>>>>Exit to automatic sync data");
#endif
	(*syncLog)(ZQ::common::Log::L_INFO,">>>>>>>Exit to automatic sync data");

	_bExit = true;
	if(_hQuit)
		SetEvent(_hQuit);
}

bool SyncDataTrd::GetTime(time_t& synctime)
{
	char buf[512];

	DWORD dwSize = ::GetPrivateProfileString("Time", "synctime",  NULL, buf, sizeof(buf)/sizeof(buf[0]), _INIfilepath.c_str());
	if(dwSize > 0)
	{
		synctime = (time_t)atoi(buf);
	}
	else
	{
		return false;
	}
	return true;
}
bool SyncDataTrd::SaveTime(time_t& synctime)
{
	char buf[64]="";
	itoa(synctime, buf, 10); 
	BOOL bSucc = ::WritePrivateProfileString("Time", "synctime", buf, _INIfilepath.c_str());
	if(!bSucc)
	{
		return false;
	}

	return true;
}
bool SyncDataTrd::checksync()
{
   int syncday = gSyncProXplannerCfg.syncDays;
   int hours  = gSyncProXplannerCfg.hours;

   if(syncday < 1)
	   syncday = 7;
   if(hours > 23 || hours < 0)
      hours = 20;
   
   char strLastSyncTime[64]="0";
   {
	   time_t lastsynctime;
	   if(!GetTime(lastsynctime))
		   return false;
	   if(lastsynctime > 0)
	   {
		   struct tm* lasttime;
		   lasttime = localtime(&lastsynctime);
		   sprintf(strLastSyncTime, "%04d-%02d-%02d %02d:%02d:%02d\0", 
			   lasttime->tm_year + 1900, lasttime->tm_mon + 1, lasttime->tm_mday,
			   lasttime->tm_hour, lasttime->tm_min, lasttime->tm_sec);
	   }
   }

 

#ifdef _DEBUG
   printf("SyncTime: day of week '%d', hour of day '%d', sync days '%d'\n", weekday, hours, syncday);
#endif
   (*syncLog)(ZQ::common::Log::L_DEBUG,"SyncTime: hour of day '%d', sync days '%d', last synctime '%s'", hours, syncday, strLastSyncTime);

   time_t  t = time(0);
   struct tm* tmnow;
   tmnow = localtime(&t);

   int nowYear;
   int nowMonth;
   int nowDay;
   int nowhour;

   INTS::iterator itor =std::find(_dayofweek.begin(),_dayofweek.end(), tmnow->tm_wday);
   if(itor == _dayofweek.end())
	   return false;

   nowYear = tmnow->tm_year;
   nowMonth = tmnow->tm_mon;
   nowDay = tmnow->tm_mday;
   nowhour = tmnow->tm_hour;

   time_t lastsynctime;
   if(!GetTime(lastsynctime))
	   return false;

   ///如果当前时间大于上一次的同步时间 并且 年月日相等 说明是已经同步过的同一天
   if(t > lastsynctime)
   {
	   struct tm* _lasttime;
	   _lasttime = localtime(&lastsynctime);

	   if(_lasttime->tm_year == nowYear && 
		   _lasttime->tm_mon == nowMonth && 
		   _lasttime->tm_mday == nowDay) //上次的同步日期 年月日跟现在相同，则说明这一天已经同步过了
		   return false;
	   else
		   if(nowhour < hours) // 当前时间还不到同步时间点
			   return false;
		   else
			   return true;
   }
   else
	   return false;
}
bool SyncDataTrd::AutoSyncData()
{
	std::string dayofweek = gSyncProXplannerCfg.dayofweek;
	int syncday = gSyncProXplannerCfg.syncDays;

	if(syncday < 1)
		syncday = 7;

#ifdef _DEBUG
	printf("syncData: day of week '%s',sync days '%d'\n", dayofweek.c_str(), syncday);
#endif
	(*syncLog)(ZQ::common::Log::L_INFO,"Enter SyncDataTrd::AutoSyncData():sync days '%d'", syncday);

	std::string startDate="";
	std::string endDate="";	

	time_t tnow = time(0);

	time_t endtime = tnow - 24 *3600;
    time_t starttime = tnow - 24 * 3600 * syncday;

	char timeTemp[64]="";
	struct tm* tmendtime = localtime(&endtime);	
	sprintf(timeTemp, "%04d-%02d-%02d\0",
		tmendtime->tm_year + 1900, tmendtime->tm_mon + 1, tmendtime->tm_mday);
	endDate = timeTemp;

	memset(timeTemp, 0, 64);
	struct tm* tmstarttime = localtime(&starttime);	
	sprintf(timeTemp, "%04d-%02d-%02d\0",
		tmstarttime->tm_year + 1900, tmstarttime->tm_mon + 1, tmstarttime->tm_mday);
	startDate = timeTemp;
#ifdef _DEBUG
	printf("syncData: startdate '%s', enddate '%s'\n", startDate.c_str(), endDate.c_str());
#endif
	(*syncLog)(ZQ::common::Log::L_INFO,"***** syncData: StartDate '%s', EndDate '%s' *****", startDate.c_str(), endDate.c_str());

	Peoples::iterator itorPeople;
	for(itorPeople = _peoples.begin(); itorPeople != _peoples.end(); itorPeople++)
	{
		syncData(itorPeople->second, startDate, endDate);
	}
	(*syncLog)(ZQ::common::Log::L_INFO,"***** Leave SyncDataTrd::AutoSyncData()*****");
	return true;
}
