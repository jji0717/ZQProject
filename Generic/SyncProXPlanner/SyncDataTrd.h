#pragma once
#include "Nativethread.h"
#include "SyncPorcXplannerDEF.h"
#include <vector>

typedef std::vector< int > INTS;
class SyncDataTrd: public ZQ::common::NativeThread
{
public:
	SyncDataTrd(Peoples peoples, std::string INIfilepath);
	~SyncDataTrd(void);
public:
	virtual bool init(void);
	virtual int run(void);
	void stop();
protected:
	bool GetTime(time_t& synctime);
	bool SaveTime(time_t& synctime);
	bool checksync();
	bool AutoSyncData();
protected:
   HANDLE _hQuit;
   
   Peoples _peoples;

   bool  _bExit;

   std::string _INIfilepath;

   INTS _dayofweek;

};
