// global.cpp: implementation of the global class.
//
//////////////////////////////////////////////////////////////////////

#include "global.h"
//#include "ServiceFrame.h"
#include "DataPostHouseService.h"
#include <TimeUtil.h>

#ifdef ZQ_OS_LINUX
#include <arpa/inet.h>
#endif

#ifndef _NO_NAMESPACE
namespace ZQ {
	namespace StreamSmith {
#endif // #ifndef _NO_NAMESPACE

RtspPerformance::Monitor _GlobalObject::perfmon;

RtspSessionMgr		_GlobalObject::_sessionMgr;
char				_GlobalObject::_serverHdr[512];

int32 _GlobalObject::pendingRequest = 0 ;
int32 _GlobalObject::activeThreads = 0;
int64 _GlobalObject::lastIdleStampDelta = 0;
RtspPerformance::ResponseCounters  _GlobalObject::responseCounters(600*1000);

_GlobalObject::_GlobalObject()
{

}

_GlobalObject::~_GlobalObject()
{
	
}
#include "ZQResource.h"

bool _GlobalObject::init()
{
	sprintf(_serverHdr, "%s/%d.%d", ZQ_INTERNAL_FILE_NAME, ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR);
	return _sessionMgr.init();
}

void _GlobalObject::uninit()
{
	_sessionMgr.uninit();
#if defined (ZQ_OS_MSWIN) && defined(_DEBUG)	
	void _dumpForDebugging();
	_dumpForDebugging();
#endif
}

RtspSessionMgr* _GlobalObject::getSessionMgr()
{
	return &_sessionMgr;
}

const char* _GlobalObject::getServerHdr()
{
	return _serverHdr;
}

static _GlobalObject theGlobalObject;

uint32 cfg_debugDetail = DEBUG_DETAIL_DEFAULT;

// rtsp performance monitor

void RtspPerformance::Monitor::collect(RequestRecord record)
{
    ZQ::common::MutexGuard guard(_lock);

    requestCount += 1;
    if(requestCount <= 0)
    {
        // overflow
        clear();
        return;
    }
    _processedTimeTotalMSec		+= record.processedTimeMSec;
    _durationTotalMSec			+= record.durationMSec;
    // compute the statistic value
    processedTimeAvg = (uint32)(_processedTimeTotalMSec / requestCount);
    if(processedTimePeak < record.processedTimeMSec)
        processedTimePeak = record.processedTimeMSec;
    
    durationAvg = (uint32)(_durationTotalMSec / requestCount);
    if(durationPeak < record.durationMSec)
        durationPeak = record.durationMSec;

	if( requestCount > (int32) skipCount)
	{
		if( record.bSucceeded )
		{
			succeededCount ++;
		}
		switch(record.rtspVerb)
		{
		case RTSP_MTHD_SETUP:
			{
				setupMax = setupMax < record.durationMSec ? record.durationMSec : setupMax;
				setupMin = setupCount > 0 && setupMin < record.durationMSec ? setupMin : record.durationMSec;
				setupCount ++;
				setupTotal += record.durationMSec;
				setupAvarage = setupTotal / setupCount;
				setupSucceededCount += record.bSucceeded ? 1 : 0 ;
			}
			break;
		case RTSP_MTHD_PLAY:
			{
				playMax	= playMax < record.durationMSec ? record.durationMSec : playMax;
				playMin = playCount > 0 && playMin < record.durationMSec ? playMin : record.durationMSec;
				playCount ++;
				playTotal += record.durationMSec;
				playAvarage = playTotal/ playCount;
				playSucceededCount += record.bSucceeded ? 1 : 0 ;
			}
			break;
		case RTSP_MTHD_GET_PARAMETER:
			{
				getParaMax = getParaMax < record.durationMSec ? record.durationMSec : getParaMax;
				getParaMin = getParaCount > 0 && getParaMin < record.durationMSec ? getParaMin : record.durationMSec;
				getParaCount ++;
				getParaTotal += record.durationMSec;
				getParaAvarage = getParaTotal/getParaCount;
				getParaSucceededCount += record.bSucceeded ? 1 : 0 ;
			}
			break;
		case RTSP_MTHD_SET_PARAMETER:
			{
				setParaMax = setParaMax < record.durationMSec ? record.durationMSec : setParaMax;
				setParaMin = setParaCount > 0 && setParaMin < record.durationMSec ? setParaMin : record.durationMSec;
				setParaCount ++;
				setParaTotal += record.durationMSec;
				setParaAvarage = setParaTotal / setParaCount;
				setParaSucceededCount += record.bSucceeded ? 1 : 0 ;
			}
			break;
		case RTSP_MTHD_OPTIONS:
			{
				optionMax = optionMax < record.durationMSec ? record.durationMSec : optionMax;
				optionMin = optionCount > 0 && optionMin < record.durationMSec ? optionMin : record.durationMSec;
				optionCount ++;
				optionTotal += record.durationMSec;
				optionAvarage = optionTotal/optionCount;
				optionSucceededCount += record.bSucceeded ? 1 : 0 ;
			}
			break;
		case RTSP_MTHD_TEARDOWN:
			{
				teardownMax = teardownMax < record.durationMSec ? record.durationMSec : teardownMax;
				teardownMin = teardownCount > 0 && teardownMin < record.durationMSec ? teardownMin : record.durationMSec;
				teardownCount ++;
				teardownTotal += record.durationMSec;
				teardownAvarage = teardownTotal / teardownCount;
				teardownSucceededCount += record.bSucceeded ? 1 : 0 ;
			}
			break;
		default:
			{
				otherMax = otherMax < record.durationMSec ? record.durationMSec : otherMax;
				otherMin = otherCount > 0 && otherMin < record.durationMSec ? otherMin : record.durationMSec;
				otherCount ++;
				otherTotal += record.durationMSec;
				otherAvarage = otherTotal / otherCount;
				otherSucceededCount += record.bSucceeded ? 1 : 0 ;
			}
			break;
		}
	}
}

void RtspPerformance::Monitor::reset()
{
    ZQ::common::MutexGuard guard(_lock);
    clear();
	strStampSince = ZQ::common::TimeUtil::TimeToUTC( ZQ::common::now(), nowUtc, sizeof(nowUtc)-1 );
}

void RtspPerformance::Monitor::clear()
{
    requestCount = 0;
	succeededCount = 0;

    processedTimeAvg = 0;
    processedTimePeak = 0;

    durationAvg = 0;
    durationPeak = 0;

    _processedTimeTotalMSec = 0;
    _durationTotalMSec = 0;


	skipCount			=	0;

	sampleWindowSize	=	1000;
	countWindowSize		=	5000;

	setupMax	=	0;
	setupMin	=	0;
	setupAvarage=	0;
	setupCount	=	0;
	setupTotal	=	0;
	setupSucceededCount = 0;

	playMax		=	0;
	playMin		=	0;
	playAvarage	=	0;
	playCount	=	0;
	playTotal	=	0;
	playSucceededCount = 0;

	getParaMax	=	0;
	getParaMin	=	0;
	getParaAvarage=	0;
	getParaCount=	0;
	getParaTotal=	0;
	getParaSucceededCount = 0;

	setParaMax	=	0;
	setParaMin	=	0;
	setParaAvarage=	0;
	setParaCount=	0;
	setParaTotal=	0;
	setParaSucceededCount = 0;

	optionMax	=	0;
	optionMin	=	0;
	optionAvarage=	0;
	optionCount	=	0;
	optionTotal	=	0;
	optionSucceededCount = 0;

	teardownMax	=	0;
	teardownMin	=	0;
	teardownAvarage=0;
	teardownCount=	0;
	teardownTotal=	0;
	teardownSucceededCount = 0;

	otherMax	=	0;
	otherMin	=	0;
	otherAvarage=	0;
	otherCount	=	0;
	otherTotal	=	0;
	otherSucceededCount = 0;
}

void RtspPerformance::ResponseCounters::addCount(const std::string& verb, const char* reponseLine)
{
	CountIdx verbIdx = ci_MAX;

	if      (0 == verb.compare("SETUP"))          verbIdx = ci_SETUP;
	else if (0 == verb.compare("PLAY"))           verbIdx = ci_PLAY;
	else if (0 == verb.compare("PAUSE"))	      verbIdx = ci_PAUSE;
	else if (0 == verb.compare("TEARDOWN"))       verbIdx = ci_TEARDOWN;
	else if (0 == verb.compare("GET_PARAMETER"))  verbIdx = ci_GET_PARAMETER;
	else if (0 == verb.compare("ANNOUNCE"))       verbIdx = ci_ANNOUNCE;

	int64 stampNow = ZQ::common::now();
// #define INC_AND_REFRESH(_CI) _incCount(_subCounters[_CI +1], 1, stampNow);  _decCount(_subCounters[_CI +1], stampNow)
#define INC_AND_REFRESH(_CI) FloatWindowCounters::addCount(_CI, stampNow); // _decCount(_subCounters[_CI +1], stampNow)

	if (ci_ANNOUNCE == verbIdx)
	{
		INC_AND_REFRESH(ci_ANNOUNCE);
		return;
	}

	uint respCode =0;
	if (NULL == reponseLine)
		respCode =200;
	else
	{
		respCode = atoi(reponseLine);
		if (respCode < 100)
		{
			reponseLine += sizeof("RTSP/1.0");
			respCode = atoi(reponseLine);
		}
		
		if (respCode < 100)
			respCode =200;
	}

	if (771 == respCode)
		respCode = 404;
	else if (775 == respCode)
		respCode = 453;

//	ZQ::common::MutexGuard g(_locker);
	static int64 stampLast =0;
	if (stampNow - stampLast > 1500) // refresh all counter every 1.5 sec
	{
//		for (int i =0; i <= ci_MAX; i++)
//			_decCount(_counters[i], stampNow);

		FloatWindowCounters::levelageAll(stampNow);
		stampLast = stampNow;
	}
//	else _decCount(_counters[0], stampNow);

	if (respCode >= 1000) // this "add" was just intend to refresh the counter per timestamp, no true add
		return;

	INC_AND_REFRESH(ci_MAX); // count the overall responses

/*	// about the response counters
		ci_SETUP,     ci_PLAY,     ci_PAUSE,     ci_TEARDOWN,     ci_GET_PARAMETER,
		ci_500_SETUP, ci_500_PLAY, ci_500_PAUSE, ci_500_TEARDOWN, ci_500_GET_PARAMETER,
		ci_503_SETUP, ci_503_PLAY, ci_503_PAUSE, ci_503_TEARDOWN, ci_503_GET_PARAMETER,
		ci_404,
		ci_454,
		ci_455,
		ci_OtherErrs,
*/

	if (verbIdx < ci_MAX) // the recoganized verbs
	{
		INC_AND_REFRESH(verbIdx); // count per the recoganized verb first
		switch(respCode)
		{
		case 500: INC_AND_REFRESH(verbIdx + ci_500_SETUP); return;
		case 503: INC_AND_REFRESH(verbIdx + ci_503_SETUP); return;
		default: break;
		}
	}

	if (respCode >= 200 && respCode < 300)
		return; // success response stops counting here

	switch(respCode)
	{
	case 404: INC_AND_REFRESH(ci_404); return;
	case 454: INC_AND_REFRESH(ci_454); return;
	case 455: INC_AND_REFRESH(ci_455); return;
	default:  INC_AND_REFRESH(ci_OtherErrs); return;
	}
}

#ifdef ZQ_OS_LINUX
bool sockaddrToString(struct ::sockaddr* psaddr, char* buf, size_t bufLen, int& port )
{
	if(buf == NULL)
        return false;

    sockaddr_in* inadd;
    sockaddr_in6* in6add;

    if(psaddr->sa_family == AF_INET)
    {
        inadd = (sockaddr_in*)psaddr;
        port = ntohs(inadd->sin_port);
        const char* pd = inet_ntop(AF_INET,(void*)&(inadd->sin_addr), buf,bufLen);
        if(pd == NULL)
            return false;
    }

    else if(psaddr->sa_family == AF_INET6)
    {
        in6add = (sockaddr_in6*)psaddr;
        port = ntohs(in6add->sin6_port);
        const char* pd = inet_ntop(AF_INET6,(void*)&(in6add->sin6_addr), buf,bufLen);
        if(pd == NULL)
            return false;
    }

    else
        return false;

    return true;

}
#endif

#ifndef _NO_NAMESPACE
	} // namespace StreamSmith {
} // namespace ZQ {
#endif // #ifndef _NO_NAMESPACE
