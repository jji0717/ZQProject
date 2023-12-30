
#include <ZQ_common_conf.h>
#include <sstream>

#include <Guid.h>
#include <SystemUtils.h>
#include <TimeUtil.h>
#include "C2StreamerEnv.h"

namespace C2Streamer
{

C2StreamerEnv::C2StreamerEnv(void)
:mLogger(NULL),
mThreadPool(NULL),
mLocateThreadPool(NULL),
mAttrBridge(NULL),
miSeqBase(0)
{
	//mLatencyMap.resize(5000);
}

C2StreamerEnv::~C2StreamerEnv(void)
{
}


std::string C2StreamerEnv::getRequestSequence( )
{
	char szBuffer[128];
	{
		ZQ::common::MutexGuard gd(mEnvMutex);
		miSeqBase ++;
		sprintf( szBuffer , "%06llX" , (long long)miSeqBase);
	}
	return std::string(szBuffer);
}

std::string	C2StreamerEnv::generateSessionId( ) const
{
	ZQ::common::Guid uid;
	uid.create();
	char szUidBuffer[128];
	szUidBuffer[ sizeof(szUidBuffer) - 1 ] = 0;
	uid.toCompactIdstr( szUidBuffer , sizeof(szUidBuffer) - 2 );

	char szSessIdBuffer[256];
	sprintf( szSessIdBuffer , "%s",szUidBuffer );
	return std::string(szSessIdBuffer);
}

std::string dumpStringVector( const std::vector<std::string>& value ,const std::string& delimiter )
{
	std::ostringstream strRet ;
	std::vector<std::string>::const_iterator it = value.begin( );
	for( ; it != value.end() ; it ++ )
	{
		strRet << *it << delimiter;
	}
	return strRet.str();
}



//////////////////////////////////////////////////////////////////////////
///
std::string timeToString( uint64 timeTicket )
{
	char szLocalBuffer[512];
	szLocalBuffer[ sizeof(szLocalBuffer) - 1 ] = 0;
	const char* p = ZQ::common::TimeUtil::TimeToUTC( timeTicket, szLocalBuffer, sizeof(szLocalBuffer)-1 , true );
	if( p && p[0] != 0 )
	{
		return std::string(p);
	}
	else
	{
		return std::string("");
	}
}

std::string constructResponseSessId( const std::string& sessId)
{
	return std::string(TRANSFERSESSION_PREFIX) + "/" + sessId;
}
std::string getSessionIdFromCompoundString( const std::string& str )
{
	std::string::size_type pos = str.find_last_of('/');
	if( pos != std::string::npos )
	{
		return str.substr( pos + 1);
	}
	else
	{
		return str;
	}
}

static const char* c2Readers[] = {"DiskAio","C2Client","HttpFetcher","AquaReader","HttpFetcher2", "Hybrid"};

int readerStr2Type( const std::string& readerTag ) {
	static size_t count = sizeof(c2Readers)/sizeof(c2Readers[0]);
	for( int i = 0 ; i < count; i ++ ) {
		if( strcasecmp(readerTag.c_str(), c2Readers[i]) == 0 ) {
			return i;
		}
	}
	return -1;
}

const char* readerType2Str( int type ) {
	static int count = sizeof(c2Readers) / sizeof(c2Readers[0]);
	if( type < 0 || type >= count ) {
		return "InvalidReader";
	}
	return c2Readers[type];
}


}//namespace C2Streamer

