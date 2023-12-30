#ifndef  __WTLOG_DEFINE__H_
#define  __WTLOG_DEFINE__H_
#include "TianShanDefines.h"
#include <TianShanIceHelper.h>
#include <TianShanIce.h>
#include <map>
#include <string>
#include <utility>
#include "FileLog.h" 


#define CCLOGFMT(_MOD, _X) CLOGFMT(_MOD, "[%5ld] mtclient " _X),  GetCurrentThreadId()

class CWTLOG
{
public:
	CWTLOG(){}
	CWTLOG(const char* filelog);
	~CWTLOG();
	bool	initLogger(const char* logfolder );
	ZQ::common::FileLog		mMainLogger;
private:
	std::string	mErrMsg;
	void	setErroMsg(const char* fmt , ... );
	char filename[256];
public://temporarily
	ZQ::common::Log  *traceFileLog;
	std::string *tracefile;

};

#define hlog  (mMainLogger)

#endif  //__WTLOG_DEFINE__H_

