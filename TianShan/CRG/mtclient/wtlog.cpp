#include "wtlog.h"

CWTLOG::CWTLOG(const char* filelog)
{
	traceFileLog =new ZQ::common::FileLog(filelog, ZQ::common::Log::L_DEBUG,5, 1024*1024*50);
	if(traceFileLog)
		(*traceFileLog)(ZQ::common::Log::L_INFO,CLOGFMT(mtclient, "----  LOG STARTED ---- "));	
	else
	{
		delete tracefile;
		printf("Trace fatal error: log could not be opened!\n"); 
	}
}
CWTLOG::~CWTLOG()
{
	(*traceFileLog)(ZQ::common::Log::L_INFO,CLOGFMT(mtclient, "----  LOG ENDED ---- "));	
	if(traceFileLog) { traceFileLog->flush(); delete  traceFileLog;}
}
void  CWTLOG::setErroMsg( const char* fmt , ... )
{
	char szLocalBuffer[1024];
	va_list args;
	va_start(args, fmt);
	int nCount = _vsnprintf( szLocalBuffer, sizeof(szLocalBuffer)-1, fmt, args );
	va_end(args);
	if(nCount == -1)
	{
		szLocalBuffer[ sizeof(szLocalBuffer) - 1 ] = 0;
	}
	else
	{
		szLocalBuffer[nCount] = '\0';
	}

	mErrMsg = szLocalBuffer;

}
bool  CWTLOG::initLogger(const char* logfolder)
{
	if( !( logfolder && logfolder[0] != 0 ) )
	{
		setErroMsg("null log folder passed in");
		return false;
	}

	try
	{
		std::string  path = ZQTianShan::Util::fsConcatPath(logfolder,"mtclient.log") ;
		mMainLogger.open( path.c_str() ,ZQ::common::Log::L_DEBUG,10,50000000,8192);

	}
	catch(const ZQ::common::FileLogException& ex)
	{
		setErroMsg("failed to open main log due to [%s]", ex.what() );
		return false;
	}

	return true ;
}