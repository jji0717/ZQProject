/**********************************************************
 $Id: LogCtrl.h,v 1.2 2003/06/18 20:43:11 shao Exp $
 $Author: Admin $
 $Revision: 1 $
 $Date: 10-11-12 16:00 $

Copyright (C) 2002-2003 Shao Hui.
All Rights Reserved, Shao Hui.
**********************************************************/

#ifndef __LogCtrl_h__
#define __LogCtrl_h__

#include "EntryDB.h"

#ifdef _MT // multi-thread
#  include "Thread.h"
#endif // _MT

#include <fstream>

#ifndef LC_DEFAULT
#  define LC_DEFAULT LC_ERROR
#endif // LC_DEFAULT

ENTRYDB_NAMESPACE_BEGIN

typedef enum _log_level
{
	LC_EMERGENCY = 1,
	LC_ALERT,
	LC_CRITICAL,
	LC_ERROR,
	LC_WARNING,
	LC_NOTICE,
	LC_INFO,
	LC_DEBUG
} log_level_t;

class LogCtrl
{
public:

	LogCtrl(void);
	virtual ~LogCtrl(void);

	virtual void close(void);
	virtual void open(const char *ident, log_level_t level = LC_DEFAULT);
	virtual bool isopened();

	LogCtrl& operator()(log_level_t level, const char *fmt, ...);
	LogCtrl& operator()(const char *fmt, ...);
	LogCtrl& operator()(void);

	// change the default logging level for further output
	log_level_t level(log_level_t level =LC_DEFAULT);

	static const char* level2string(log_level_t level);
	static int string2level(const char* levelstr);

	bool bScrEcho;

protected:

	virtual void writemsg (log_level_t level, const char* msg);
	const char* timestampstr(bool digits =false);

private:

#ifdef _MT
	Mutex lock;
#endif // _MT
	std::ofstream mLog;

	char timestamp[64];

	int priority;
	log_level_t  mLevel;

	static const char* tLevelStr[];
};

#define SCRTRACE

#ifdef _DEBUG
#  define LOGFMT(_X) "%s(%03d) " _X, __FILE__, __LINE__
#  ifdef _TRACE_STDOUT_ECHO
#    undef  SCRTRACE
#    define SCRTRACE printf(LOGFMT("\n"))
#  endif
#else
#  define LOGFMT(_X) _X
#  undef  _TRACE_STDOUT_ECHO
#endif // _DEBUG

extern EDOS_API LogCtrl log;

ENTRYDB_NAMESPACE_END

#endif // __LogCtrl_h__

