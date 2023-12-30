
#include <ZQ_common_conf.h>
#include <assert.h>
#include <sstream>
#include <strHelper.h>
#include "NgodConfig.h"
#include "TimeUtil.h"
#include "NgodHelper.h"

char* MonthIndexToString[] ={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

KVPairGroupWalker::KVPairGroupWalker(const std::string& sdpContent , const std::string& delimiter )
:mDelimiter(delimiter)
{
	parseSdp(sdpContent);
}

KVPairGroupWalker::~KVPairGroupWalker( )
{

}

void KVPairGroupWalker::parseSdp( const std::string& strSdp )
{

	std::string::size_type posCurrent = 0;
	std::string::size_type posNext = 0;
	size_t szDelimiter = mDelimiter.length();
	do 
	{
		KVAttr	attr;

		std::string::size_type posEqualMark = strSdp.find( '=' , posCurrent );
		std::string::size_type posDelimiter = strSdp.find( mDelimiter , posCurrent );

		if( POSOK(posEqualMark) )
		{			
			if( posEqualMark < posDelimiter )
			{// key and value are both available
				attr.key			= strSdp.substr( posCurrent , posEqualMark -posCurrent );
				attr.value			= strSdp.substr( posEqualMark + 1 , posDelimiter - posEqualMark -1 );
			}
			else
			{
				attr.key			= strSdp.substr( posCurrent , posDelimiter -posCurrent );
				attr.value			= "";
			}			
		}
		else
		{				
			attr.key			= strSdp.substr( posCurrent , posDelimiter -posCurrent );
			attr.value			= "";			
		}
		if( POSOK(posDelimiter) ) 
		{
			posCurrent = posDelimiter + szDelimiter;
		}
		else
		{
			posCurrent = std::string::npos;
		}

		ZQ::common::stringHelper::TrimExtra(attr.key);
		ZQ::common::stringHelper::TrimExtra(attr.value);
		if( !attr.key.empty() )
			mSdpLine.push_back( attr );

	} while ( POSOK(posCurrent) );
}

//////////////////////////////////////////////////////////////////////////
///SettingDispatcher
SettingDispatcher::SettingDispatcher()
{
}
SettingDispatcher::~SettingDispatcher( )
{
}

void SettingDispatcher::regSetting( const std::string& key , SETTINGFUNC func )
{
	mMap[key] = func;
}
void SettingDispatcher::dispatch( const std::string& key ,const std::string& value)
{
	SETTINGMAP::iterator it = mMap.find( key );
	if( it != mMap.end() )
	{
		(this->*(it->second))(value);
	}
	else
	{
		//要不要写个log message啥的呀
	}
}


//////////////////////////////////////////////////////////////////////////
///SettingWalker
void		walkSettings( SettingDispatcher& dispatcher , KVPairGroupWalker& walker )
{
	KVPairGroupWalker::const_iterator it = walker.begin();
	for( ; it != walker.end() ;it ++ )
	{
		dispatcher.dispatch( it->key , it->value );
	}
	dispatcher.onComplete();
}

std::string generatorOffsetString( int64 offset , int require )
{	
	//TODO: need configuration
	char szTemp[64];
	if( ngodConfig.messageFmt.rtspNptUsage>= 1)
	{		
		snprintf(szTemp, sizeof(szTemp) - 1, "%lld.%03lld", offset/1000, offset%1000 );
	}
	else
	{
		if ( ngodConfig.protocolVersioning.enableVersioning > 0 &&
			( require == NgodVerCode_C1_DecNpt || require ==NgodVerCode_R2_DecNpt ))
		{
			snprintf(szTemp, sizeof(szTemp) - 1, "%lld.%03lld", offset/1000, offset%1000 );
		}
		else
		{
			snprintf(szTemp, sizeof(szTemp) - 1, "%llx", offset );
		}
	}
	return std::string(szTemp);
}

std::string generatorNoticeString(	const std::string strNoticeCode, 
								  const std::string strNoticeString, 
								  const std::string strNpt )
{
	char szBuf[512];
	std::string notice_str;
	notice_str = strNoticeCode + " \"" + strNoticeString + "\" " + "event-date=";
#ifdef ZQ_OS_MSWIN
	SYSTEMTIME time;
	GetLocalTime(&time);
	memset(szBuf, 0, sizeof(szBuf));
	snprintf(szBuf, sizeof(szBuf) - 1, "%04d%02d%02dT%02d%02d%02d.%03dZ npt=",time.wYear,time.wMonth,time.wDay,
		time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
	notice_str += szBuf;
#else
	struct timeval tval;
	gettimeofday(&tval,(struct timezone*)NULL);
	struct tm* ptm = NULL;
	ptm = localtime(&tval.tv_sec);

	szBuf[sizeof(szBuf)-1]='\0';		
	snprintf (szBuf,sizeof(szBuf)-1,"%02d %3s %04d %02d:%02d:%02d.%03ld npt=",
		ptm->tm_mday,
		MonthIndexToString[ptm->tm_mon],
		ptm->tm_year,
		ptm->tm_hour,
		ptm->tm_min,
		ptm->tm_sec,
		tval.tv_usec/1000);
	notice_str += szBuf;
#endif

	notice_str += strNpt;
	return notice_str;
}
std::string	getISOTimeString( )
{
#ifdef ZQ_OS_MSWIN
	SYSTEMTIME curTime;
	GetSystemTime(&curTime);
#else
	struct timeval curTime;
	gettimeofday(&curTime,(struct timezone*)NULL);
	struct tm* ptm = NULL;
	ptm = gmtime(&curTime.tv_sec);
#endif

	char buf[256];
	ZQ::common::TimeUtil::Time2Iso(curTime, buf, sizeof(buf) - 1);
	std::string strISOTime = buf;
	return strISOTime;
}

std::string convertIntToNptString( int64 npt )
{
	std::ostringstream oss;
	char szBuffer[64];
	sprintf(szBuffer,"%lld.%03lld",npt/1000,npt%1000);	
	return std::string(szBuffer);
}
std::string getGMTString()
{
	typedef long long int int64;
	char stampUTP[20];
	memset(stampUTP, 0, 20);
	time_t tnow = time(NULL);
	int64 seconds1900 = int64(tnow) + 2208988800;
	snprintf(stampUTP, 19, "%lld", seconds1900);
	return stampUTP;
}
