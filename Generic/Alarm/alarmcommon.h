
#ifndef _ZQ_ALARMCOMMON_H_
#define _ZQ_ALARMCOMMON_H_
	
#ifndef NAMESPACE
#define NAMESPACE ALARM
#endif
	
#ifndef TG_BEGIN
#define TG_BEGIN namespace ZQ{namespace NAMESPACE{
#endif
	
#ifndef TG_END
#define TG_END }}
#endif

#define SKIP_HEAD_CHARS 10
#define NEXT_LINE_SIGN "\r\n"
#define MAX_SYNTAX_LINE_LENGTH 1024
#define MAX_PARAM_LINE_LENGTH 1024
#define MAX_PARAM_LINE_COUNT 256

#define DEF_LOG_TAIL_TIME		250
#define DEF_LOG_LINE_LENGTH		2048
#define DEF_WAIT_EXIT_TIMEOUT	2000
	
	
#endif//_ZQ_ALARMCOMMON_H_
	