// RtspServer.cpp : Defines the entry point for the console application.
//

#include "ZQ_common_conf.h"

#ifdef ZQ_OS_MSWIN
#include "stdafx.h"
#include <MiniDump.h>
#else
extern "C"
{
#include <sys/resource.h>
}
#endif

#include "StreamSmithService.h"
#include "global.h"
#include "StreamSmithSite.h"
#include "RtspDialog.h"
#include "DialogCreator.h"
#include "RtspSessionMgr.h"
#include "rtspProxyConfig.h"
#include "SystemUtils.h"

#if defined(ZQ_OS_MSWIN) && defined(_DEBUG)
	#include "adebugmem.h"
#endif

using namespace ZQ;
using namespace ZQ::StreamSmith;

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType=1;
DWORD gdwServiceInstance=11;
#else
extern const char* DUMP_PATH;
#endif

RtspDialogCreatorPtr dialogCreator = NULL;
DataPostHouseService* serviceFrm = NULL;

ZQ::common::Config::Loader<RtspProxyCfg> gRtspProxyConfig("RtspProxy.xml");
ZQ::common::Config::ILoader	*configLoader=&gRtspProxyConfig;

StreamSmithService	ssApp;
ZQ::common::BaseZQServiceApplication		*Application = &ssApp;

bool gServiceHealthy = true;

//////////////////////////////////////////////////////////////////////////

#include "NativeThreadPool.h"
namespace ZQ {
namespace StreamSmith {
extern RequestProcessResult SSMH_DefaultContentHandle (IStreamSmithSite* pSite, IClientRequestWriter* pReq);
}}

#if defined(_RTSP_PROXY) && defined(_LOCAL_TEST)
std::string			strSetupConnid="";
//_RTSP_PROXY
RequestProcessResult SSMH_ContentHandletest(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	return RequestPhaseDone;
}
RequestProcessResult FixupRequestHandler(IStreamSmithSite* pSite, 
										 IClientRequestWriter* pReq)
{

	IServerResponse* response = pReq->getResponse();
	
	char buf[512];
	uint16 len;
	ZQ::StreamSmith::RtspSessionMgr* sessionMgr= ZQ::StreamSmith::_GlobalObject::getSessionMgr();
	IClientSession* session = 0;
	Variant v;
	try {
		
		switch (pReq->getVerb()) 
		{
		case RTSP_MTHD_OPTIONS:
			{
				response->printf_preheader("RTSP/1.0 200 OK");
				sprintf(buf, "%s", RtspUtil::getMethodInString(pReq->getVerb()));
				response->setHeader("Method-Code", buf);
				response->setHeader("Public", 
					"DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, PING, REDIRECT");
				len = sizeof(buf);
				pReq->getHeader("CSeq", buf, &len);
				response->setHeader("CSeq", buf);
				response->setHeader("Transport", "MP2T/AVP/UDP");
				response->post();
			}
			break;
			
		case RTSP_MTHD_DESCRIBE:
			{
				response->printf_preheader("RTSP/1.0 200 OK");
				response->printf_postheader(			
					"m=video 8888 RAW/RAW/UDP 33" CRLF
					"c=IN IP4 225.0.0.100/255" CRLF
					"a=control:track2");
				response->post();
			}
			break;

		case RTSP_MTHD_SETUP:
			{
				unsigned __int16 bufLen=sizeof(buf);
				ZeroMemory(buf,sizeof(buf));
				pReq->getHeader("SYS#ConnID",buf,&bufLen);
				strSetupConnid=buf;
				glog(ZQ::common::Log::L_INFO,"####Get setup ID=[%s]",buf);
				sessionMgr = ZQ::StreamSmith::_GlobalObject::getSessionMgr();
				session = sessionMgr->createSession(NULL,IClientSession::LocalSession,NULL);
				if(!session)
				{
					response->printf_preheader("RTSP/1.0 500 INTERNAL SERVER ERROR");
					response->post();
					break;
				}
				response->printf_preheader("RTSP/1.0 200 OK");
				sprintf(buf, "%s", RtspUtil::getMethodInString(pReq->getVerb()));
				response->setHeader("Method-Code", buf);
				response->setHeader("Accept-Ranges", "NPT");
				len = sizeof(buf);
				pReq->getHeader("CSeq", buf, &len);
				response->setHeader("CSeq", buf);
				sprintf(buf, "%s;timeout=60", session->getSessionID());
				response->setHeader("Session", buf);
				pReq->setHeader("Session", buf);
				len = sizeof(buf);
				pReq->getHeader("Session", buf, &len);
				char szTransport[1096] = {0};
				uint16 transportBufSize =sizeof(szTransport)-1;
				pReq->getHeader("Transport",szTransport,&transportBufSize);
				glog(ZQ::common::Log::L_DEBUG,"%s",szTransport);
				
				//pReq->getParameter(KEY_TRANS, v);
				sprintf(buf, "MP2T/AVP/UDP;multcast;destination=225.0.0.100;client_port=%d-%d", 
					8888, 8888);
				response->setHeader("Transport", buf);
				
				response->post();
				sessionMgr->onAccessingSession(pReq);
			}
			break;
		case RTSP_MTHD_PAUSE:
			{
				response->printf_preheader("RTSP/1.0 200 OK");
				sprintf(buf, "%s", RtspUtil::getMethodInString(pReq->getVerb()));
				response->setHeader("Method-Code", buf);
				response->setHeader("Accept-Ranges", "NPT");
				len = sizeof(buf);
				pReq->getHeader("CSeq", buf, &len);
				response->setHeader("CSeq", buf);
				response->setHeader("Range", "npt=1.000-");
				response->setHeader("Scale", "1.0000");
				response->setHeader("Session", pReq->getClientSessionId());
				response->post();
				glog(Log::L_INFO,"###Before OnAccessingSession");
				sessionMgr->onAccessingSession(pReq);
				glog(Log::L_INFO,"###After OnAccessingSession");
			}
			break;

		case RTSP_MTHD_PLAY:
			{
				response->printf_preheader("RTSP/1.0 200 OK");
				sprintf(buf, "%s", RtspUtil::getMethodInString(pReq->getVerb()));
				response->setHeader("Method-Code", buf);
				response->setHeader("Accept-Ranges", "NPT");
				len = sizeof(buf);
				pReq->getHeader("CSeq", buf, &len);
				response->setHeader("CSeq", buf);
				response->setHeader("Range", "npt=0.000-");
				response->setHeader("Scale", "1.0000");
				response->setHeader("Session", pReq->getClientSessionId());
				response->post();
				glog(Log::L_INFO,"###Before OnAccessingSession");
				sessionMgr->onAccessingSession(pReq);
				glog(Log::L_INFO,"###After OnAccessingSession");
			}
			break;
		case RTSP_MTHD_TEARDOWN:
			{
				sessionMgr = ZQ::StreamSmith::_GlobalObject::getSessionMgr();			
				response->printf_preheader("RTSP/1.0 200 OK");
				sprintf(buf, "%s", RtspUtil::getMethodInString(pReq->getVerb()));
				response->setHeader("Method-Code", buf);
				len = sizeof(buf);
				pReq->getHeader("CSeq", buf, &len);
				response->setHeader("CSeq", buf);
				response->setHeader("Session", pReq->getClientSessionId());
				response->post();
				sessionMgr->deleteSession(pReq->getClientSessionId());				
			}
			break;
		case RTSP_MTHD_GET_PARAMETER:
			{
				response->printf_preheader("RTSP/1.0 200 OK");
				sprintf(buf, "%s", RtspUtil::getMethodInString(pReq->getVerb()));
				response->setHeader("Method-Code", buf);
				len = sizeof(buf);
				pReq->getHeader("CSeq", buf, &len);
				response->setHeader("CSeq", buf);
				response->setHeader("Session", pReq->getClientSessionId());
				response->post();
				sessionMgr->onAccessingSession(pReq);
			}
			break;
		default:
			{
				glog( Log::L_ERROR,  "Unknown method: %s", 
					RtspUtil::getMethodInString(pReq->getVerb()));
				
				response->printf_preheader("RTSP/1.0 500 Internal Server Error");
				response->post();
				sessionMgr->onAccessingSession(pReq);
			}
		}
	} catch(...) {
		glog( Log::L_ERROR, "FixupRequestHandler(): exception has occurred");
		assert(false);
	}
		
	return RequestDone;
}

#ifdef ZQ_OS_MSWIN
#include "signal.h"
void sig_break(int)
{
	glog( Log::L_NOTICE,  "User Break...");
#ifdef _DEBUG
	extern void _dumpForDebugging();
	_dumpForDebugging();
#endif

	raise(SIGTERM);
}
#endif

#endif // #ifdef _RTSP_PROXY

#ifndef _NO_NAMESPACE
using namespace ZQ;
using namespace ZQ::StreamSmith;
#endif // #ifndef _NO_NAMESPACE

#ifdef ZQ_OS_MSWIN
ZQ::common::MiniDump			_crashDump;
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);


void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();
	
	glog( Log::L_ERROR,  L"Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
		ExceptionCode, ExceptionAddress, dwThreadID);
	
	glog.flush();
}


bool validatePath( const char *     szPath )
{
    if (-1 != ::GetFileAttributesA(szPath))
        return true;
	
    DWORD dwErr = ::GetLastError();
    if ( dwErr == ERROR_PATH_NOT_FOUND || dwErr == ERROR_FILE_NOT_FOUND )
    {
        if (!::CreateDirectoryA(szPath, NULL))
        {
            dwErr = ::GetLastError();
            if ( dwErr != ERROR_ALREADY_EXISTS)
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }
	
    return true;
}
#endif

void uninitServer()
{
	try
	{
		int64 dwTest=ZQ::common::now();

		ZQ::StreamSmith::ParseProcThrd::close();
		glog(Log::L_INFO,"RTSP parse threads stopped, time count=%d",ZQ::common::now()-dwTest);

		dialogCreator->stopIdleMonitor();
		glog(Log::L_INFO,"RTSP idle connection monitor stopped");

		dwTest=ZQ::common::now();
		serviceFrm->uninit();
		glog(Log::L_INFO,"serviceFrm->uninit() time count=%d",ZQ::common::now()-dwTest);

		dwTest=ZQ::common::now();
		StreamSmithSite::DestroyStreamSmithSite();

		glog(Log::L_INFO,"DestroyStreamSmithSite time count=%d",ZQ::common::now()-dwTest);

		dwTest=ZQ::common::now();

		_GlobalObject::uninit();

		_gThreadPool.stop();
		glog(Log::L_DEBUG,"_GlobalObject::uninit() time count=%d",ZQ::common::now()-dwTest);
	}
	catch (...) 
	{
		glog(Log::L_ERROR,"error occurred when un-initialize server");
	}
}

StreamSmithService::StreamSmithService()
    :m_pLog(0),m_pPluginLog(0)
{
}
StreamSmithService::~StreamSmithService()
{

}

// for rtsp performance counter
#define RTSP_PERFCOUNTER_RESET "Measure Reset"
static int32 _perfCounterReset = 0;
// register rtsp performance counter to snmp

// static void registerPerfCounter()
// {
// 	
//     SNMPManageVariable( ZQSNMP_VARNAME("Statistics",RTSP_PERFCOUNTER_RESET), 
// 						&_perfCounterReset, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						FALSE,
//                         40);
// 	
//     SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Measured Since"),
// 						_GlobalObject::perfmon.nowUtc,
// 						ZQSNMP_VARTYPE_STRING, 
// 						TRUE,
//                         41);
//     SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Request Count"),
// 						&_GlobalObject::perfmon.requestCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         42);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Total Succeeded Request Count"),
// 						&_GlobalObject::perfmon.succeededCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         43);
// 
//     SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Average Process Latency"),
// 						&_GlobalObject::perfmon.processedTimeAvg, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         44);
//     SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Max Process Latency"),
// 						&_GlobalObject::perfmon.processedTimePeak, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         45);
//     SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Average Latency"),
// 						&_GlobalObject::perfmon.durationAvg, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         46);
//     SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Max Latency"),
// 						&_GlobalObject::perfmon.durationPeak, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         47);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Skip Count"),
// 						&_GlobalObject::perfmon.skipCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						FALSE,
//                         48);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SETUP Latency-Max"),
// 						&_GlobalObject::perfmon.setupMax, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         49);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SETUP Latency-Min"),
// 						&_GlobalObject::perfmon.setupMin, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         50);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SETUP Latency-Average"),
// 						&_GlobalObject::perfmon.setupAvarage, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         51);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SETUP Request Count"),
// 						&_GlobalObject::perfmon.setupCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         52);
// 	
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SETUP Succeeded Count"),
// 						&_GlobalObject::perfmon.setupSucceededCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         53);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","PLAY Latency-Max"),
// 						&_GlobalObject::perfmon.playMax, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         54);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","PLAY Latency-Min"),
// 						&_GlobalObject::perfmon.playMin, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         55);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","PLAY Latency-Average"),
// 						&_GlobalObject::perfmon.playAvarage, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         56);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","PLAY Request Count"),
// 						&_GlobalObject::perfmon.playCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         57);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","PLAY Succeeded Count"),
// 						&_GlobalObject::perfmon.playSucceededCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         58);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","GET_PARAMETER Latency-Max"),
// 						&_GlobalObject::perfmon.getParaMax, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         59);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","GET_PARAMETER Latency-Min"),
// 						&_GlobalObject::perfmon.getParaMin, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         60);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","GET_PARAMETER Latency-Average"),
// 						&_GlobalObject::perfmon.getParaAvarage, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         61);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","GET_PARAMETER Request Count"),
// 						&_GlobalObject::perfmon.getParaCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         62);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","GET_PARAMETER Succeeded Count"),
// 						&_GlobalObject::perfmon.getParaSucceededCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         63);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SET_PARAMETER Latency-Max"),
// 						&_GlobalObject::perfmon.setParaMax, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         64);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SET_PARAMETER Latency-Min"),
// 						&_GlobalObject::perfmon.setParaMin, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         65);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SET_PARAMETER Latency-Average"),
// 						&_GlobalObject::perfmon.setParaAvarage, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         66);
// 	
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SET_PARAMETER Request Count"),
// 						&_GlobalObject::perfmon.setParaCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         67);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SET_PARAMETER Succeeded Count"),
// 						&_GlobalObject::perfmon.setParaSucceededCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         68);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","OPTIONS Latency-Max"),
// 						&_GlobalObject::perfmon.optionMax, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         69);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","OPTIONS Latency-Min"),
// 						&_GlobalObject::perfmon.optionMin, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         70);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","OPTIONS Latency-Average"),
// 						&_GlobalObject::perfmon.optionAvarage, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         71);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","OPTIONS Request Count"),
// 						&_GlobalObject::perfmon.optionCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         72);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","OPTIONS Succeeded Count"),
// 						&_GlobalObject::perfmon.optionSucceededCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         73);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","TEARDOWN Latency-Max"),
// 						&_GlobalObject::perfmon.teardownMax, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         74);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","TEARDOWN Latency-Min"),
// 						&_GlobalObject::perfmon.teardownMin, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         75);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","TEARDOWN Latency-Average"),
// 						&_GlobalObject::perfmon.teardownAvarage, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         76);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","TEARDOWN Request Count"),
// 						&_GlobalObject::perfmon.teardownCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         77);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","TEARDOWN Succeeded Count"),
// 						&_GlobalObject::perfmon.teardownSucceededCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
//                         78);
// 	
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," Pending Requests"),
// 						&_GlobalObject::pendingRequest,
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
// 						79);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," Active Threads"),
// 						&_GlobalObject::activeThreads,
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
// 						80);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," Inactive Time"),
// 						&_GlobalObject::lastIdleStampDelta, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE,
// 						81);
// 
// /*	// about the response counters
// 		ci_SETUP,     ci_PLAY,     ci_PAUSE,     ci_TEARDOWN,     ci_GET_PARAMETER,
// 		ci_500_SETUP, ci_500_PLAY, ci_500_PAUSE, ci_500_TEARDOWN, ci_500_GET_PARAMETER,
// 		ci_503_SETUP, ci_503_PLAY, ci_503_PAUSE, ci_503_TEARDOWN, ci_503_GET_PARAMETER,
// 		ci_404,
// 		ci_454,
// 		ci_455,
// 		ci_OtherErrs,
// */
// 	int i=82;
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," Responses-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_MAX)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," SETUP-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_SETUP)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," PLAY-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_PLAY)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," PAUSE-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_PAUSE)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," TEARDOWN-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_TEARDOWN)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," GET_PARAMETER-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_GET_PARAMETER)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," SETUP500-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_500_SETUP)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," PLAY500-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_500_PLAY)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," PAUSE500-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_500_PAUSE)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," TEARDOWN500-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_500_TEARDOWN)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," GET_PARAMETER500-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_500_GET_PARAMETER)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," SETUP503-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_503_SETUP)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," PLAY503-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_503_PLAY)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," PAUSE503-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_503_PAUSE)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," TEARDOWN503-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_503_TEARDOWN)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," GET_PARAMETER503-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_503_GET_PARAMETER)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," 404-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_404)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," 454-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_454)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," 455-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_455)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," OtherErrs-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_OtherErrs)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 
// 	// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Others Latency-max"),
// // 						&_GlobalObject::perfmon.otherMax, 
// // 						ZQSNMP_VARTYPE_INT32, 
// // 						TRUE);
// // 
// // 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Others Latency-min"),
// // 						&_GlobalObject::perfmon.otherMin, 
// // 						ZQSNMP_VARTYPE_INT32, 
// // 						TRUE);
// // 
// // 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Others Latency-average"),
// // 						&_GlobalObject::perfmon.otherAvarage, 
// // 						ZQSNMP_VARTYPE_INT32, 
// // 						TRUE);
// // 
// // 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Others request count"),
// // 						&_GlobalObject::perfmon.otherCount, 
// // 						ZQSNMP_VARTYPE_INT32, 
// // 						TRUE);
// 
// 	// append at 11/14/2012
// 	i=300; // start from 300 to avoid conflict with other existings
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics"," ANNOUNCE-per10min"),
// 						&_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_ANNOUNCE)->countInWin, 
// 						ZQSNMP_VARTYPE_INT32, TRUE,	i++);
// 
// 
// 
// }

#define STRSWITCH() if(0){
#define STRCASE(x)	} else if(::strncmp(varName , x ,strlen(x) ) == 0 ){
#define STRENDCASE() }

/*
void StreamSmithService::OnSnmpSet(const char *varName)
{
    if(varName && 0 == strcmp(ZQSNMP_VARNAME("Statistics",RTSP_PERFCOUNTER_RESET), varName))
    {
        if(_perfCounterReset)
            _GlobalObject::perfmon.reset();
        _perfCounterReset = 0;
        return;
    }
	STRSWITCH( )
	STRCASE( ZQSNMP_VARNAME("Statistics","skip count") )
		glog(ZQ::common::Log::L_INFO,"Set statistics skip count to [%d]" ,_GlobalObject::perfmon.skipCount );

	STRCASE("RtspProxy/RequestProcess/maxPendingRequest")
		glog(ZQ::common::Log::L_INFO,"set RtspProxy/RequestProcess/maxPendingRequest to [%d]",GAPPLICATIONCONFIGURATION.lMaxPendingRequest);
	
	STRCASE("RtspProxy/EventPublisher/timeout")
		glog(ZQ::common::Log::L_INFO , "set RtspProxy/EventPublisher/timeout to [%d]",GAPPLICATIONCONFIGURATION.lEventSinkTimeout);

	STRCASE("RtspProxy/Response/RtspHeader/useLocaltime")
		glog(ZQ::common::Log::L_INFO , "set RtspProxy/Response/RtspHeader/useLocaltime to [%d]",GAPPLICATIONCONFIGURATION.lEnableUseLocaltimeInDatHeader);

	STRENDCASE()	
}
*/

HRESULT StreamSmithService::OnInit()
{
	m_pLog = m_pReporter;
	//ZQ::common::setGlogger(m_pLog);
#ifdef    ZQ_NO_LICENSE
	std::string  strTime = "2090-12-31T03:38:52.609+08:00";
	gRtspProxyConfig.licenseTime = ZQ::common::TimeUtil::ISO8601ToTime(strTime.c_str());
	//gRtspProxyConfig.licenseTime = ZQ::common::TimeUtil::now() + 50 * 12* 30 * 24 * 3600;
#else
	readLicenseFile(gRtspProxyConfig.licenseFile);
#endif

#ifdef ZQ_OS_MSWIN
	StreamSmithSite::_strApplicationLogFolder = m_wsLogFolder;

	if (!validatePath(gRtspProxyConfig.szMiniDumpPath))
	{
		glog(Log::L_ERROR, L"CrashDumpPath %s error", gRtspProxyConfig.szMiniDumpPath);
		logEvent(ZQ::common::Log::L_ERROR,_T("invalid minidump path %s"),gRtspProxyConfig.szMiniDumpPath);
		return S_FALSE;
	}	
	_crashDump.setDumpPath(gRtspProxyConfig.szMiniDumpPath);
	_crashDump.enableFullMemoryDump(gRtspProxyConfig.lEnableMiniDump);
	_crashDump.setExceptionCB(CrashExceptionCallBack);
#else
	StreamSmithSite::_strApplicationLogFolder = _logDir;
    DUMP_PATH = gRtspProxyConfig.szMiniDumpPath;

    // set ulimit  
    struct rlimit rt;
    rt.rlim_max = rt.rlim_cur = max((double)10000, max(gRtspProxyConfig.lMaxSessionCount*1.5, (double)gRtspProxyConfig.lMaxClientConnection) + 100);;
    setrlimit(RLIMIT_NOFILE, &rt);
#endif

	/*--------------------------------------------------------------------
	get local time zone	
	--------------------------------------------------------------------*/
#ifdef ZQ_OS_MSWIN
	TIME_ZONE_INFORMATION tzInfo;
	ZeroMemory(&tzInfo,sizeof(tzInfo));
	if (GetTimeZoneInformation(&tzInfo) == TIME_ZONE_ID_INVALID)
	{
		glog(ZQ::common::Log::L_ERROR,"Can't get local machine's time zone,something is wrong");
		logEvent(ZQ::common::Log::L_ERROR,_T("Can't get local machine's time zone,something is wrong"));
		//service down
	}
	else
	{
		gRtspProxyConfig.lCurTimeZone = 0 - (tzInfo.Bias/60);
		glog(ZQ::common::Log::L_DEBUG,"set local time zone bias to [%d]",tzInfo.Bias);
	}
#else
	struct timeval tav;
	struct timezone tz;
	int rc = gettimeofday(&tav, &tz);
	if(rc != 0)
	{
		glog(ZQ::common::Log::L_ERROR,"PtspProxyService::OnInit() get time zone failed");
		return false;
	}
	
	gRtspProxyConfig.lCurTimeZone = 0 - (tz.tz_minuteswest/60);
	glog(ZQ::common::Log::L_DEBUG,"OnInit() set local zone [%d]",tz.tz_minuteswest);
#endif

	StreamSmithSite::_pDefaultSite=&defaultSite;
	
#if !defined _RTSP_PROXY || defined _LOCAL_TEST
	StreamSmithSite::_defaultContentHandler = SSMH_DefaultContentHandle;
#endif
	m_usrtsPort = GAPPLICATIONCONFIGURATION.lRtspSocketServerPort ; 
	m_uslscPort = GAPPLICATIONCONFIGURATION.lLscpSocketServerPort;
	GAPPLICATIONCONFIGURATION.lListenPort = m_usrtsPort;

	if (!_GlobalObject::init()) 
	{
		glog(Log::L_ERROR,"_GlobalObject::init() failed");
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_ERROR,_T("_GlobalObject::init() failed"));
#endif
		return S_FALSE;
	}
	
	StreamSmithSite::m_pSessionMgr = _GlobalObject::getSessionMgr();
	
	// for service frame
	
	ServiceConfig cfg;
	
#pragma message(__MSGLOC__"TODO:修改这里，在serviceFrame内部进行初始化")
	

	dialogCreator = new RtspDialogCreator();
	if (!dialogCreator)
	{
		return S_FALSE;
	}

	// start idle connection monitor
	dialogCreator->setIdleTimeout(gRtspProxyConfig.lServiceFrameIdleTimeout);
	dialogCreator->setIdleScanInterval(gRtspProxyConfig.lServiceFrameIdleScanInterval);
	dialogCreator->startIdleMonitor();	

	serviceFrm = new DataPostHouseService(0 != GAPPLICATIONCONFIGURATION.lServiceFrameIPv6Enabled);
	if (!serviceFrm)
	{
		return S_FALSE;
	}

	if(1)
	{
		strncpy( cfg._cfg_publicKeyFile , GAPPLICATIONCONFIGURATION.szServiceFramepublicKeyFile , sizeof(cfg._cfg_publicKeyFile) -1 );
		strncpy( cfg._cfg_privateKeyFile, GAPPLICATIONCONFIGURATION.szServiceFrameprivateKeyFile, sizeof(cfg._cfg_privateKeyFile) );
		strncpy( cfg._cfg_privateKeyFilePwd,GAPPLICATIONCONFIGURATION.szServiceFrameprivatePassword,sizeof(cfg._cfg_privateKeyFilePwd));
		strncpy( cfg._cfg_dhParamFile , GAPPLICATIONCONFIGURATION.szServiceFramedhParamFile,sizeof(cfg._cfg_dhParamFile));
		strncpy( cfg._cfg_randFile , GAPPLICATIONCONFIGURATION.szServiceFramerandFile , sizeof(cfg._cfg_randFile));

		cfg._cfg_debugLevel = gRtspProxyConfig.lServiceFrameDebugLevel;
		//cfg._cfg_postMen = gRtspProxyConfig.lServiceThreadCount;
		cfg._cfg_postMen = gRtspProxyConfig.lServiceFrameThreadPoolSize;
		cfg._cfg_readBufferSize = gRtspProxyConfig.lReadBufferSize;
		cfg._cfg_encryptBufferSize = gRtspProxyConfig.lEncryptBufferSize;
		cfg._cfg_maxConn = gRtspProxyConfig.lMaxClientConnection; // todo 
		cfg._cfg_log = m_pLog;
		dialogCreator->setMaxConnection(gRtspProxyConfig.lMaxClientConnection);
		dialogCreator->setExpirationTime(gRtspProxyConfig.licenseTime);
		cfg._cfg_dialogCreator = dialogCreator;
		
		if (!serviceFrm->init(cfg))
		{
			return S_FALSE;
		}
		//defaultSite.SetIdsServerAddr(std::string(cfg._cfg_idsAddress));
		m_usrtsPort = GAPPLICATIONCONFIGURATION.lRtspSocketServerPort ; 
		m_uslscPort = GAPPLICATIONCONFIGURATION.lLscpSocketServerPort;
		
		RtspSessionMgr* sessionMgr = _GlobalObject::getSessionMgr();
		sessionMgr->setSessionTimeout( 300000 );
		sessionMgr->sinkSessionEvent(SessionEvent::SessionRemoved, 
			dynamic_cast<SessionEvent* >(&defaultSite));
		sessionMgr->enable(true);
		
#ifdef ZQ_OS_MSWIN
		std::string strLogFolder= m_wsLogFolder;
#else
		std::string strLogFolder= _logDir;
#endif
		if (strLogFolder.at(strLogFolder.length()-1)!=FNSEPC) 
		{
			strLogFolder+=FNSEPS;
		}

		m_pPluginLog=new ZQ::common::FileLog((std::string(strLogFolder)+"rtspProxy.plugin.log").c_str(),
												gRtspProxyConfig.lPluginLogLevel,
												ZQLOG_DEFAULT_FILENUM,
												gRtspProxyConfig.lPluginLogSize,
												gRtspProxyConfig.lPluginLogBuffer,
												gRtspProxyConfig.lPluginLogTimeout);
		defaultSite.setLogInstance(m_pPluginLog);
		
		std::vector<std::string> configPath;
//		ZQ::common::stringHelper::SplitString("RtspProxy/applicationSite",configPath,"/"," ;\t\"/");
	
#if defined(_RTSP_PROXY) && defined(_LOCAL_TEST)		
		StreamSmithSite :: _defaultContentHandler =(SSMH_ContentHandle)SSMH_ContentHandletest;
#endif
		if(!StreamSmithSite::SetupStreamSmithSite("",configPath))
		{
			glog(Log::L_ERROR,"SetUp Stream Smith Site fail");			
#ifdef ZQ_OS_MSWIN
			logEvent(ZQ::common::Log::L_ERROR,_T("SetUp Stream Smith Site fail"));
#endif
			return S_FALSE;
		}
		
		if(gRtspProxyConfig.lServiceThreadCount<=2)
			gRtspProxyConfig.lServiceThreadCount = 3;
		ZQ::StreamSmith::_gThreadPool.resize(gRtspProxyConfig.lServiceThreadCount);

		if(gRtspProxyConfig.lRtspParserThreadPoolSize<2)
			gRtspProxyConfig.lRtspParserThreadPoolSize = 2;
		ZQ::StreamSmith::ParseProcThrd::create(gRtspProxyConfig.lRtspParserThreadPoolSize, gRtspProxyConfig.lRtspParserThreadPriority);
		glog(Log::L_INFO, L"Set RTSP parser threadpool size to %d", gRtspProxyConfig.lRtspParserThreadPoolSize);

#ifdef ZQ_OS_MSWIN
		if (gRtspProxyConfig.lProcessPriority)
		{
			if (SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
			{
				glog(Log::L_INFO, L"SetPriorityClass HIGH_PRIORITY_CLASS for current process succeeded");
			}
			else
			{
				glog(Log::L_WARNING, L"Failed to SetPriorityClass HIGH_PRIORITY_CLASS for current process");
			}
		}
#endif
	}
	else
	{
		glog(Log::L_ERROR,"Load Configuaration fail,Service stopped");
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_ERROR,_T("Load Configuaration fail,Service stopped"));
#endif
		return false;
	}
	
#if defined(_RTSP_PROXY) && defined(_LOCAL_TEST)
	signal(SIGINT, sig_break);
	signal(SIGBREAK, sig_break);
	//defaultSite.RegisterContentHandle("tianshan_s1",SSMH_ContentHandle(SSMH_ContentHandletest));
	defaultSite.RegisterFixupRequest(SSMH_FixupRequest(FixupRequestHandler));
#endif

    // for rtsp performance counter
    // registerPerfCounter();
	// gRtspProxyConfig.snmpRegister("");
	return BaseZQServiceApplication::OnInit();
}

void StreamSmithService::doEnumSnmpExports()
{
	BaseZQServiceApplication::doEnumSnmpExports();
#ifdef _RTSP_PROXY
	// {".2.1.135", "rtspProxy-RequestProcess-threads" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-RequestProcess-threads(135)
	// {".2.1.137", "rtspProxy-RequestProcess-maxPendingRequest" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-RequestProcess-maxPendingRequest(137)
	// {".2.1.146", "rtspProxy-EventPublisher-timeout" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-EventPublisher-timeout(146)
	// {".2.1.154", "rtspProxy-SocketServer-rtspPort" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-SocketServer-rtspPort(154)
	// {".2.1.156", "rtspProxy-SocketServer-maxConnections" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-SocketServer-maxConnections(156)
	// {".2.1.157", "rtspProxy-SocketServer-threads" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-SocketServer-threads(157)
	// {".2.1.162", "rtspProxy-SocketServer-idleScanInterval" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-SocketServer-idleScanInterval(162)
	// {".2.1.163", "rtspProxy-SocketServer-maxSessions" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-SocketServer-maxSessions(163)
	// {".2.1.171", "rtspProxy-SocketServer-IncomingMessage-maxLen" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-SocketServer-IncomingMessage-maxLen(171)
	SvcMIB_ExportReadOnlyVar("rtspProxy-RequestProcess-threads",           gRtspProxyConfig.lServiceThreadCount, "");
	SvcMIB_ExportReadOnlyVar("rtspProxy-RequestProcess-maxPendingRequest", gRtspProxyConfig.lMaxPendingRequest, "");
	SvcMIB_ExportReadOnlyVar("rtspProxy-EventPublisher-timeout",           gRtspProxyConfig.lEventSinkTimeout, "");
	SvcMIB_ExportReadOnlyVar("rtspProxy-SocketServer-rtspPort",            gRtspProxyConfig.lRtspSocketServerPort, "");
	SvcMIB_ExportReadOnlyVar("rtspProxy-SocketServer-maxConnections",      gRtspProxyConfig.lMaxClientConnection, "");
	SvcMIB_ExportReadOnlyVar("rtspProxy-SocketServer-threads",             gRtspProxyConfig.lServiceFrameThreadPoolSize, "");
	SvcMIB_ExportReadOnlyVar("rtspProxy-SocketServer-idleScanInterval",    gRtspProxyConfig.lServiceFrameIdleTimeout, "");
	SvcMIB_ExportReadOnlyVar("rtspProxy-SocketServer-maxSessions",         gRtspProxyConfig.lMaxSessionCount, "");
	SvcMIB_ExportReadOnlyVar("rtspProxy-SocketServer-IncomingMessage-maxLen", gRtspProxyConfig.lIncomingMsgMaxLen, "");

	//{".2.1.40", "rtspProxy-Statistics-Measure-Reset" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-Measure-Reset(40)
	//{".2.1.41", "rtspProxy-Statistics-Measured-Since" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-Measured-Since(41)
	//{".2.1.42", "rtspProxy-Statistics-Request-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-Request-Count(42)
	//{".2.1.43", "rtspProxy-Statistics-Total-Succeeded-Request-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-Total-Succeeded-Request-Count(43)
	//{".2.1.44", "rtspProxy-Statistics-Average-Process-Latency" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-Average-Process-Latency(44)
	//{".2.1.45", "rtspProxy-Statistics-Max-Process-Latency" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-Max-Process-Latency(45)
	//{".2.1.46", "rtspProxy-Statistics-Average-Latency" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-Average-Latency(46)
	//{".2.1.47", "rtspProxy-Statistics-Max-Latency" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-Max-Latency(47)
	//{".2.1.48", "rtspProxy-Statistics-Skip-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-Skip-Count(48)
	//{".2.1.49", "rtspProxy-Statistics-SETUP-Latency-Max" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-SETUP-Latency-Max(49)
	//{".2.1.50", "rtspProxy-Statistics-SETUP-Latency-Min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-SETUP-Latency-Min(50)
	//{".2.1.51", "rtspProxy-Statistics-SETUP-Latency-Average" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-SETUP-Latency-Average(51)
	//{".2.1.52", "rtspProxy-Statistics-SETUP-Request-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-SETUP-Request-Count(52)
	//{".2.1.53", "rtspProxy-Statistics-SETUP-Successed-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-SETUP-Successed-Count(53)
	//{".2.1.54", "rtspProxy-Statistics-PLAY-Latency-Max" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-PLAY-Latency-Max(54)
	//{".2.1.55", "rtspProxy-Statistics-PLAY-Latency-Min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-PLAY-Latency-Min(55)
	//{".2.1.56", "rtspProxy-Statistics-PLAY-Latency-Average" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-PLAY-Latency-Average(56)
	//{".2.1.57", "rtspProxy-Statistics-PLAY-Request-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-PLAY-Request-Count(57)
	//{".2.1.58", "rtspProxy-Statistics-PLAY-Succeeded-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-PLAY-Succeeded-Count(58)
	//{".2.1.59", "rtspProxy-Statistics-GETPARAMETER-Latency-Max" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-GETPARAMETER-Latency-Max(59)
	//{".2.1.60", "rtspProxy-Statistics-GETPARAMETER-Latency-Min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-GETPARAMETER-Latency-Min(60)
	//{".2.1.61", "rtspProxy-Statistics-GETPARAMETER-Latency-Average" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-GETPARAMETER-Latency-Average(61)
	//{".2.1.62", "rtspProxy-Statistics-GETPARAMETER-Request-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-GETPARAMETER-Request-Count(62)
	//{".2.1.63", "rtspProxy-Statistics-GETPARAMETER-Succeeded-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-GETPARAMETER-Succeeded-Count(63)
	//{".2.1.64", "rtspProxy-Statistics-SETPARAMETER-Latency-Max" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-SETPARAMETER-Latency-Max(64)
	//{".2.1.65", "rtspProxy-Statistics-SETPARAMETER-Latency-Min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-SETPARAMETER-Latency-Min(65)
	//{".2.1.66", "rtspProxy-Statistics-SETPARAMETER-Latency-Average" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-SETPARAMETER-Latency-Average(66)
	//{".2.1.67", "rtspProxy-Statistics-SETPARAMETER-Request-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-SETPARAMETER-Request-Count(67)
	//{".2.1.68", "rtspProxy-Statistics-SETPARAMETER-Succeeded-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-SETPARAMETER-Succeeded-Count(68)
	//{".2.1.69", "rtspProxy-Statistics-OPTIONS-Latency-Max" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-OPTIONS-Latency-Max(69)
	//{".2.1.70", "rtspProxy-Statistics-OPTIONS-Latency-Min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-OPTIONS-Latency-Min(70)
	//{".2.1.71", "rtspProxy-Statistics-OPTIONS-Latency-Average" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-OPTIONS-Latency-Average(71)
	//{".2.1.72", "rtspProxy-Statistics-OPTIONS-Request-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-OPTIONS-Request-Count(72)
	//{".2.1.73", "rtspProxy-Statistics-OPTIONS-Succeeded-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-OPTIONS-Succeeded-Count(73)
	//{".2.1.74", "rtspProxy-Statistics-TEARDOWN-Latency-Max" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-TEARDOWN-Latency-Max(74)
	//{".2.1.75", "rtspProxy-Statistics-TEARDOWN-Latency-Min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-TEARDOWN-Latency-Min(75)
	//{".2.1.76", "rtspProxy-Statistics-TEARDOWN-Latency-Average" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-TEARDOWN-Latency-Average(76)
	//{".2.1.77", "rtspProxy-Statistics-TEARDOWN-Request-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-TEARDOWN-Request-Count(77)
	//{".2.1.78", "rtspProxy-Statistics-TEARDOWN-Succeeded-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-TEARDOWN-Succeeded-Count(78)
	//{".2.1.82", "rtspProxy-Statistics-Responses-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-Responses-per-10min(82)
	//{".2.1.83", "rtspProxy-Statistics-SETUP-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-SETUP-per-10min(83)
	//{".2.1.84", "rtspProxy-Statistics-PLAY-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-PLAY-per-10min(84)
	//{".2.1.85", "rtspProxy-Statistics-PAUSE-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-PAUSE-per-10min(85)
	//{".2.1.86", "rtspProxy-Statistics-TEARDOWN-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-TEARDOWN-per-10min(86)
	//{".2.1.87", "rtspProxy-Statistics-GET_PARAMETER-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-GET_PARAMETER-per-10min(87)
	//{".2.1.88", "rtspProxy-Statistics-SETUP500-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-SETUP500-per-10min(88)
	//{".2.1.89", "rtspProxy-Statistics-PLAY500-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-PLAY500-per-10min(89)
	//{".2.1.90", "rtspProxy-Statistics-PAUSE500-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-PAUSE500-per-10min(90)
	//{".2.1.91", "rtspProxy-Statistics-TEARDOWN500-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-TEARDOWN500-per-10min(91)
	//{".2.1.92", "rtspProxy-Statistics-GET_PARAMETER500-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-GET_PARAMETER500-per-10min(92)
	//{".2.1.93", "rtspProxy-Statistics-SETUP503-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-SETUP503-per-10min(93)
	//{".2.1.94", "rtspProxy-Statistics-PLAY503-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-PLAY503-per-10min(94)
	//{".2.1.95", "rtspProxy-Statistics-PAUSE503-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-PAUSE503-per-10min(95)
	//{".2.1.96", "rtspProxy-Statistics-TEARDOWN503-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-TEARDOWN503-per-10min(96)
	//{".2.1.97", "rtspProxy-Statistics-GET_PARAMETER503-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-GET_PARAMETER503-per-10min(97)
	//{".2.1.98", "rtspProxy-Statistics-404-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-404-per-10min(98)
	//{".2.1.99", "rtspProxy-Statistics-454-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-454-per-10min(99)
	//{".2.1.100", "rtspProxy-Statistics-455-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-455-per-10min(100)
	//{".2.1.101", "rtspProxy-Statistics-OtherErrs-per-10min" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-OtherErrs-per-10min(101)
	//{".2.1.300", "rtspProxy-Statistics-ANNOUNCE-Count" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspProxyAttrs(2).rtspProxyAttr(1).rtspProxy-Statistics-ANNOUNCE-Count(300)
	ServiceMIB_ExportByAPI(_pServiceMib, "rtspProxy-stat-reset", StreamSmithService, *this, uint32, AsnType_Int32, &StreamSmithService::dummyGet, &StreamSmithService::resetMeasure, ".40");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-since",             _GlobalObject::perfmon.strStampSince,              ".41");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-cReq",              _GlobalObject::perfmon.requestCount, ".42");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-cSucc",             _GlobalObject::perfmon.succeededCount, ".43");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-avgPLatency",       _GlobalObject::perfmon.processedTimeAvg, ".44");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-maxPLatency",       _GlobalObject::perfmon.processedTimePeak, ".45");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-avgLatency",        _GlobalObject::perfmon.durationAvg, ".46");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-maxLatency",        _GlobalObject::perfmon.durationPeak, ".47");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-cSkipped",          _GlobalObject::perfmon.skipCount, ".48");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-SETUP-latMax",      _GlobalObject::perfmon.setupMax, ".49");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-SETUP-latMin",      _GlobalObject::perfmon.setupMin, ".50");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-SETUP-latAvg",      _GlobalObject::perfmon.setupAvarage, ".51");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-SETUP-cReq",        _GlobalObject::perfmon.setupCount, ".52");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-SETUP-cSucc",       _GlobalObject::perfmon.setupSucceededCount, ".53");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-PLAY-latMax",       _GlobalObject::perfmon.playMax, ".54");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-PLAY-latMin",       _GlobalObject::perfmon.playMin, ".55");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-PLAY-latAvg",       _GlobalObject::perfmon.playAvarage, ".56");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-PLAY-cReq",         _GlobalObject::perfmon.playCount, ".57");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-PLAY-cSucc",        _GlobalObject::perfmon.playSucceededCount, ".58");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-GPARM-latMax",      _GlobalObject::perfmon.getParaMax, ".59");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-GPARM-latMin",      _GlobalObject::perfmon.getParaMin, ".60");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-GPARM-latAvg",      _GlobalObject::perfmon.getParaAvarage, ".61");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-GPARM-cReq",        _GlobalObject::perfmon.getParaCount, ".62");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-GPARM-cSucc",       _GlobalObject::perfmon.getParaSucceededCount, ".63");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-SPARM-latMax",      _GlobalObject::perfmon.getParaMax, ".64");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-SPARM-latMin",      _GlobalObject::perfmon.getParaMin, ".65");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-SPARM-latAvg",      _GlobalObject::perfmon.getParaAvarage, ".66");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-SPARM-cReq",        _GlobalObject::perfmon.getParaCount, ".67");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-SPARM-cSucc",       _GlobalObject::perfmon.getParaSucceededCount, ".68");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-OPTNS-latMax",      _GlobalObject::perfmon.optionMax, ".69");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-OPTNS-latMin",      _GlobalObject::perfmon.optionMin, ".70");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-OPTNS-latAvg",      _GlobalObject::perfmon.optionAvarage, ".71");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-OPTNS-cReq",        _GlobalObject::perfmon.optionCount, ".72");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-OPTNS-cSucc",       _GlobalObject::perfmon.optionSucceededCount, ".73");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-TRDWN-latMax",      _GlobalObject::perfmon.teardownMax, ".74");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-TRDWN-latMin",      _GlobalObject::perfmon.teardownMin, ".75");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-TRDWN-latAvg",      _GlobalObject::perfmon.teardownAvarage, ".76");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-TRDWN-cReq",        _GlobalObject::perfmon.teardownCount, ".77");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-TRDWN-cSucc",       _GlobalObject::perfmon.teardownSucceededCount, ".78");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-pendingReqs",       _GlobalObject::pendingRequest, ".79");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-activeThrds",       _GlobalObject::activeThreads, ".80");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat-lastIdleTime",      _GlobalObject::lastIdleStampDelta, ".81");

	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-Responses",    _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_MAX)->countInWin,			  ".82");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-SETUPs",       _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_SETUP)->countInWin ,         ".83");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-PLAYs",        _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_PLAY)->countInWin,           ".84");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-PAUSEs",       _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_PAUSE)->countInWin,          ".85");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-TEARDOWNs",    _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_TEARDOWN)->countInWin,       ".86");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-GET_PARAMs",   _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_GET_PARAMETER)->countInWin,  ".87");

	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-SETUPs-500",    _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_500_SETUP)->countInWin,          ".88");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-PLAYs-500",     _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_500_PLAY)->countInWin,           ".89");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-PAUSEs-500",    _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_500_PAUSE)->countInWin,          ".90");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-TEARDOWNs-500", _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_500_TEARDOWN)->countInWin,       ".91");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-GET_PARAMs-500",_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_500_GET_PARAMETER)->countInWin,  ".92");

	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-SETUPs-503",    _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_503_SETUP)->countInWin,          ".93");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-PLAYs-503",     _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_503_PLAY)->countInWin,           ".94");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-PAUSEs-503",    _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_503_PAUSE)->countInWin,          ".95");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-TEARDOWNs-503", _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_503_TEARDOWN)->countInWin,      ".96");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-GET_PARAMs-503",_GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_503_GET_PARAMETER)->countInWin,  ".97");

	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-404s",          _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_404)->countInWin,        ".98");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-454s",          _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_454)->countInWin,       ".99");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-455s",          _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_455)->countInWin,        ".100");
	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-OtherErrs",     _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_OtherErrs)->countInWin,  ".101");

	SvcMIB_ExportReadOnlyVar("rtspProxy-stat10min-ANNOUNCEs",     _GlobalObject::responseCounters.getCounter(RtspPerformance::ResponseCounters::ci_ANNOUNCE)->countInWin,   ".300");

	//{".2.1.200.2", "rtspCrStat-SessionCount" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspSvcApp(2).rtspProxyAttr(1).rtspCrStat(200).rtspCrStat-SessionCount(2)
	//{".2.1.200.3", "rtspCrStat-PendingSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspSvcApp(2).rtspProxyAttr(1).rtspCrStat(200).rtspCrStat-PendingSize(3)
	//{".2.1.200.4", "rtspCrStat-BusyThreads" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspSvcApp(2).rtspProxyAttr(1).rtspCrStat(200).rtspCrStat-BusyThreads(4)
	//{".2.1.200.5", "rtspCrStat-ThreadPoolSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).rtspProxy(1000).rtspSvcApp(2).rtspProxyAttr(1).rtspCrStat(200).rtspCrStat-ThreadPoolSize(5)
	ServiceMIB_ExportByAPI(_pServiceMib, "rtspCrStat-SessionCount",   StreamSmithService, *this, uint32, AsnType_Int32, &StreamSmithService::getSessionCount, NULL, "");
	ServiceMIB_ExportByAPI(_pServiceMib, "rtspCrStat-PendingSize",    StreamSmithService, *this, uint32, AsnType_Int32, &StreamSmithService::getPendingSize,  NULL, "");
	ServiceMIB_ExportByAPI(_pServiceMib, "rtspCrStat-BusyThreads",    StreamSmithService, *this, uint32, AsnType_Int32, &StreamSmithService::getBusyThreads,  NULL, "");
	ServiceMIB_ExportByAPI(_pServiceMib, "rtspCrStat-ThreadPoolSize", StreamSmithService, *this, uint32, AsnType_Int32, &StreamSmithService::getThreads,      NULL, "");
#else
#endif // _RTSP_PROXY

}

uint32 StreamSmithService::getSessionCount()
{
	ZQ::StreamSmith::RtspSessionMgr* smgr = ZQ::StreamSmith::_GlobalObject::getSessionMgr();
	if (NULL == smgr)
		return 0;
	return smgr->getSessionCount();
}

uint32 StreamSmithService::getPendingSize()
{ return _gThreadPool.pendingRequestSize(); }

uint32 StreamSmithService::getBusyThreads()
{ return _gThreadPool.activeCount(); }

uint32 StreamSmithService::getThreads()
{ return _gThreadPool.size(); }

HRESULT StreamSmithService::OnStart()
{
    _GlobalObject::perfmon.reset();
	
	//TODO: modify here
	
	if (!serviceFrm->begin())
	{
		return S_FALSE;
	}
	
	char temp[32];
	sprintf(temp, "%d", m_usrtsPort);
	serviceFrm->bindRtsp (gRtspProxyConfig.szRtspIPv4, gRtspProxyConfig.szRtspIPv6, temp);
	sprintf(temp, "%d", m_uslscPort);
	serviceFrm->bindLscp (gRtspProxyConfig.szLscpIPv4, gRtspProxyConfig.szLscpIPv6, temp);

	if (gRtspProxyConfig.lServiceFrameSSLEnabled)
	{
		// add ssl communictaor
		serviceFrm->setCertAndKeyFile(gRtspProxyConfig.szServiceFramepublicKeyFile, gRtspProxyConfig.szServiceFrameprivateKeyFile, gRtspProxyConfig.szServiceFrameprivatePassword);
		
		serviceFrm->bindSSLLscp(gRtspProxyConfig.szLscpIPv4, gRtspProxyConfig.szLscpIPv6, gRtspProxyConfig.szLscpSSLPort);
		serviceFrm->bindSSLRtsp(gRtspProxyConfig.szRtspIPv4, gRtspProxyConfig.szRtspIPv6, gRtspProxyConfig.szRtspSSLPort);
	}
	
	//register rtsp port and lscp port into configuration so I can use it in the future and at any where
	GAPPLICATIONCONFIGURATION.lListenPort = m_usrtsPort;
	GAPPLICATIONCONFIGURATION.lLscpPort = m_uslscPort;
	GAPPLICATIONCONFIGURATION.lRtspPort = m_usrtsPort;
	
	RtspSessionMgr* sessionMgr = _GlobalObject::getSessionMgr();
	sessionMgr->enable(true);

	return BaseZQServiceApplication::OnStart();
}
HRESULT StreamSmithService::OnStop()
{
	if(serviceFrm)
		serviceFrm->end();

	return BaseZQServiceApplication::OnStop();
}
HRESULT StreamSmithService::OnUnInit()
{
	int64 dwTest=ZQ::common::now();
	uninitServer();
	delete serviceFrm;
	serviceFrm = 0;
	glog(Log::L_DEBUG,"unintiliaze service time count=%d",ZQ::common::now()-dwTest);
	
	if(m_pPluginLog)
	{
		try{delete m_pPluginLog;}catch(...){ }
		m_pPluginLog=NULL;
	}
	return BaseZQServiceApplication::OnUnInit();
}
bool StreamSmithService::isHealth(void)
{
	return gServiceHealthy;
}
void StreamSmithService::readLicenseFile(const std::string& fileName)
{
	gRtspProxyConfig.lMaxClientConnection = NO_LICENSE_SESSIONS_MAX;
	gRtspProxyConfig.lMaxSessionCount = NO_LICENSE_SESSIONS_MAX;
	gRtspProxyConfig.licenseTime = ZQ::common::TimeUtil::now();
	
	// read the body of license file into data
	std::string data;
	{
		FILE* file = fopen(fileName.c_str(),"rb");
		if (file == NULL)
			return;

		char buffer[512];
		int res = 0;
		while((res = fread(buffer, 1, sizeof(buffer), file)) != 0)
			data.append(buffer, res);

		fclose(file);
	}
	
	// parse the license and validate the finger-print
	MachineFingerPrint machieFingerPrint(*m_pLog);
	std::string licenseData = machieFingerPrint.loadLicense(data);
	Json::Reader jsonReader;
	Json::Value licenseValue;
	jsonReader.parse(licenseData, licenseValue);
	std::string serviceName = "";

#ifdef ZQ_OS_MSWIN
	serviceName.assign(m_sServiceName);
#else
	serviceName = getServiceName();
#endif

	// read the licensed items
	if (licenseValue.isMember("expiration"))
	{
		try
		{
			std::string strExpriation = licenseValue["expiration"].asString();
			gRtspProxyConfig.licenseTime = ZQ::common::TimeUtil::ISO8601ToTime(strExpriation.c_str());
		}
		catch(...)
		{
			glog(Log::L_ERROR, CLOGFMT(StreamSmithService,"readLicenseFile() invalid expiration[%s] specified"), licenseData.c_str());
		}
	}

	if (!licenseValue.isMember(serviceName))
		glog(Log::L_ERROR, CLOGFMT(StreamSmithService,"readLicenseFile() service[%s] not found"), serviceName.c_str());
	else
	{
		Json::Value serviceValue = licenseValue[serviceName];
		try
		{
			if (serviceValue.isMember("maxSessions"))
			{
// 				std::string sessionNum = Json::FastWriter().write(serviceValue["maxSessions"]);
//  				gRtspProxyConfig.lMaxSessionCount = atoi(sessionNum.c_str());
				gRtspProxyConfig.lMaxSessionCount = serviceValue["maxSessions"].asInt();
			}

			if (serviceValue.isMember("maxConnections"))
				gRtspProxyConfig.lMaxClientConnection = serviceValue["maxConnections"].asInt();
		}
		catch(...)
		{
			glog(Log::L_ERROR, CLOGFMT(StreamSmithService,"readLicenseFile() invalid license body: %s"), licenseData.c_str());
		}
	}
	
	char timeBuffer[64] ="\0";
	ZQ::common::TimeUtil::TimeToUTC(gRtspProxyConfig.licenseTime, timeBuffer, sizeof(timeBuffer) - 1, true);
	glog(Log::L_NOTICE, CLOGFMT(StreamSmithService,"readLicenseFile() result: maxSessions[%d], maxConnections[%d] licensed till expiration[%s]"), 
		             gRtspProxyConfig.lMaxSessionCount, gRtspProxyConfig.lMaxClientConnection, timeBuffer);
}

void StreamSmithService::resetMeasure(const uint32& iDummy)
{ _GlobalObject::perfmon.reset(); }
