#ifndef __ZQ_Sentry_TimeConv_H__
#define __ZQ_Sentry_TimeConv_H__
#include <ZQ_common_conf.h>
#include <TimeUtil.h>
#include <string>

std::string time2utc(int64 t);

// MM-DD HH:MM:SS.mmm
int64 getFilelogTime(const char* tmstr);
// MM/DD HH:MM:SS:mmm
int64 getSclogTime(const char* tmstr);
// MMM DD HH:MM:SS 
int64 getSyslogTime1(const char* strStamp);
// YYYY/MMM/DD_HH:MM:SS
int64 getSyslogTime2(const char* strStamp);

#endif
