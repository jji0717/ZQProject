// global.h: interface for the global class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLOBAL_H__073603A8_AC8D_44C3_8567_CA7D3040DFD1__INCLUDED_)
#define AFX_GLOBAL_H__073603A8_AC8D_44C3_8567_CA7D3040DFD1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// #define _NO_TRACK_MEMORY
#include "RtspSessionMgr.h"
#include "Counter.h"

#ifdef ZQ_OS_LINUX
extern "C"
{
#include <netinet/in.h>
}
#endif

#ifndef _NO_NAMESPACE
namespace ZQ {
	namespace StreamSmith {
#endif // #ifndef _NO_NAMESPACE
        
// rtsp performance monitor

namespace RtspPerformance
{
    // request processed time
    struct RequestRecord
    {
		uint32	rtspVerb;		
        uint32	processedTimeMSec;
        uint32	durationMSec;
		bool	bSucceeded;
        RequestRecord()
		{
			rtspVerb = 0;
            processedTimeMSec = 0;
            durationMSec = 0;
			bSucceeded = true;
        }
    };

    class Monitor 
    {
    public:
        Monitor()
        {
            reset();
        }
        
		void collect(RequestRecord record);
        
		void reset();

    private:
        void clear();
        void now();
    public:
		


        int32	requestCount;		//total request count

		uint32	succeededCount;		
        
        uint32	processedTimeAvg;	//avarage processing time
        uint32	processedTimePeak;	//max processing time
        
        uint32	durationAvg;
        uint32	durationPeak;

		uint32	skipCount;

		uint32	sampleWindowSize;
		uint32	countWindowSize;

		uint32	setupMax;
		uint32	setupMin;
		uint32	setupAvarage;
		uint32	setupCount;
		uint32	setupTotal;
		uint32	setupSucceededCount;

		uint32	playMax;
		uint32	playMin;
		uint32	playAvarage;
		uint32	playCount;
		uint32	playTotal;
		uint32	playSucceededCount;
		
		uint32	getParaMax;
		uint32	getParaMin;
		uint32	getParaAvarage;
		uint32	getParaCount;
		uint32	getParaTotal;
		uint32	getParaSucceededCount;

		uint32	setParaMax;
		uint32	setParaMin;
		uint32	setParaAvarage;
		uint32	setParaCount;
		uint32	setParaTotal;
		uint32	setParaSucceededCount;

		uint32	optionMax;
		uint32	optionMin;
		uint32	optionAvarage;
		uint32	optionCount;
		uint32	optionTotal;
		uint32	optionSucceededCount;

		uint32	teardownMax;
		uint32	teardownMin;
		uint32	teardownAvarage;
		uint32	teardownCount;
		uint32	teardownTotal;
		uint32	teardownSucceededCount;

		uint32	otherMax;
		uint32	otherMin;
		uint32	otherAvarage;
		uint32	otherCount;
		uint32	otherTotal;
		uint32	otherSucceededCount;

		char nowUtc[40];
		std::string	strStampSince;

    private:

        double	_processedTimeTotalMSec;
        double	_durationTotalMSec;
        ZQ::common::Mutex _lock;
    };

class ResponseCounters : public ZQ::common::FloatWindowCounters
{
public:
	typedef enum _CountIdx
	{
		ci_SETUP,     ci_PLAY,     ci_PAUSE,     ci_TEARDOWN,     ci_GET_PARAMETER,
		ci_500_SETUP, ci_500_PLAY, ci_500_PAUSE, ci_500_TEARDOWN, ci_500_GET_PARAMETER,
		ci_503_SETUP, ci_503_PLAY, ci_503_PAUSE, ci_503_TEARDOWN, ci_503_GET_PARAMETER,
		ci_404,
		ci_454,
		ci_455,
		ci_OtherErrs,

		ci_ANNOUNCE,
		ci_MAX
	} CountIdx;

	ResponseCounters(uint winSize=1000) : FloatWindowCounters(winSize, ci_MAX+1) {}

	virtual void addCount(const std::string& verb, const char* reponseLine);
};

}

// Added by xiaohui.chai for performance counter

class _GlobalObject
{
public:
	_GlobalObject();
	~_GlobalObject();
	static bool init();
	static void uninit();
	static RtspSessionMgr* getSessionMgr();
	static const char* getServerHdr();

    // added by xiaohui.chai for rtsp performance counter
    static RtspPerformance::Monitor perfmon;

	static int32 pendingRequest;
	static int32 activeThreads;
	static int64 lastIdleStampDelta;
	static RtspPerformance::ResponseCounters  responseCounters;

protected:
	static RtspSessionMgr	_sessionMgr;
	static char				_serverHdr[512];

};

// added by Cary

#define DEBUG_DETAIL_DEFAULT		0
#define DEBUG_DETAIL_LEVEL1			1
#define DEBUG_DETAIL_LEVEL2			2
#define DEBUG_DETAIL_LEVEL3			3

extern uint32 cfg_debugDetail;

#define DEBUG_DETAIL(level) \
	if (glog.getVerbosity() == ZQ::common::Log::L_DEBUG && \
		cfg_debugDetail >= (level))


/// Add by Hongquan.Zhang for configuration

#ifdef ZQ_OS_LINUX
//convert xocket address to string
extern bool sockaddrToString(struct ::sockaddr* psaddr, char* buf, size_t bufLen, int& port );
#endif

#ifndef _NO_NAMESPACE
	} // namespace StreamSmith {
} // namespace ZQ {
#endif // #ifndef _NO_NAMESPACE

#endif // !defined(AFX_GLOBAL_H__073603A8_AC8D_44C3_8567_CA7D3040DFD1__INCLUDED_)

