
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
mAttrBridge(NULL),
miSeqBase(0)
{
	mLatencyMap.resize(5000);
}

C2StreamerEnv::~C2StreamerEnv(void)
{
}

bool  C2StreamerEnv::setLatencyMap(std::string& fileName, int64 startOffset, int64 time)
{
	char buf[128];
	memset(buf, '\0', sizeof(char) * 128);
	snprintf(buf, 128, "%s-%ld", fileName.c_str(), startOffset);
	std::string key(buf);
	{
		ZQ::common::MutexGuard g(mLatencyMapMutex);
		if( mLatencyMap.find(key) !=  mLatencyMap.end() )
			return false;
		mLatencyMap[key]=time;
	}
	return true;
}

int   C2StreamerEnv::getLatencyMap(std::string& fileName, int64 startOffset, int64 time)
{
	char buf[128];
	memset(buf, '\0', sizeof(char) * 128);
	snprintf(buf, 128, "%s-%ld", fileName.c_str(), startOffset);
	int latency = -1;
	std::string key(buf);
	{
		ZQ::common::MutexGuard g (mLatencyMapMutex);
		if(mLatencyMap.find(key) == mLatencyMap.end() )
		{		
			latency = -1;
		}
		else
		{
			latency = (int)(time - mLatencyMap[key]);
			mLatencyMap.erase(key);
		}
	}
	return latency;
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
	return std::string(TRANSFERSESSION_PREFIX) + sessId;
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

}//namespace C2Streamer

