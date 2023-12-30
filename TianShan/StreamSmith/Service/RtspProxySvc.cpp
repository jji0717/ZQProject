
#include "RtspProxySvc.h"
#include "global.h"
#include "StreamSmithSite.h"
#include "RtspDialog.h"
#include "DialogCreator.h"
#include "rtspProxyConfig.h"
#include "NativeThreadPool.h"
#include "SystemUtils.h"
extern "C"
{
#include <sys/resource.h>
}

using namespace std;
//using namespace ZQ;
using namespace ZQ::StreamSmith;

RtspProxyService                    gService;
ZQ::common::ZQDaemon* Application = &gService;


ZQ::common::Config::Loader<RtspProxyCfg>  gRtspProxyConfig("RtspProxy.xml");
ZQ::common::Config::ILoader* configLoader = & gRtspProxyConfig;


RtspDialogCreatorPtr                dialogCreator = NULL;
DataPostHouseService*				serviceFrm;

extern const char* DUMP_PATH;

namespace ZQ {
	namespace StreamSmith {
extern RequestProcessResult SSMH_DefaultContentHandle (IStreamSmithSite* pSite, IClientRequestWriter* pReq);
}}

#if defined(_RTSP_PROXY) && defined(_LOCAL_TEST)
std::string         strSetupConnid="";
//_RTSP_PROXY
RequestProcessResult SSMH_ContentHandletest(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
    return RequestPhaseDone;
}
RequestProcessResult FixupRequestHandler(IStreamSmithSite* pSite,
                                         IClientRequestWriter* pReq)
{

    IServerResponse* response = pReq->getResponse();
    char buf[512] = {0};
    uint16 len;
    ZQ::StreamSmith::RtspSessionMgr* sessionMgr= ZQ::StreamSmith::_GlobalObject::getSessionMgr();
    IClientSession* session = NULL;
    Variant v;
    try {

        switch (pReq->getVerb())
        {
//      case RTSP_RESPONSE_MTHD:
//          {
//              ZeroMemory(buf,sizeof(buf));
//              pReq->getStartline(buf,sizeof(buf));
//              uint16 uiLen =sizeof(buf)-1;
//              pReq->getHeader("OnDemandSessionId",buf,&uiLen);
//              const char *p=pReq->getClientSessionId();
//              if(p&&strlen(p)>0)
//                  pSite->CoreLog(ZQ::common::Log::L_DEBUG,"session:%s",p);
//              pReq->setArgument(RTSP_MTHD_SETUP,"hello/888/?",pReq->getProtocol(buf,sizeof(buf)-1));
//
//          }
//          break;
        case RTSP_MTHD_OPTIONS:
			{
				response->printf_preheader("RTSP/1.0 200 OK");
				sprintf(buf, "%s", (RtspUtil::getMethodInString(pReq->getVerb())).getPtr());
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
				uint16 bufLen=sizeof(buf);
				memset(buf,0,sizeof(buf));
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
				sprintf(buf, "%s", (RtspUtil::getMethodInString(pReq->getVerb())).getPtr());
				response->setHeader("Method-Code", buf);
				response->setHeader("Accept-Ranges", "");
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
				sprintf(buf, "%s", (RtspUtil::getMethodInString(pReq->getVerb())).getPtr());
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
                sprintf(buf, "%s", (RtspUtil::getMethodInString(pReq->getVerb())).getPtr());
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
                {
//                  const   char* pSessId=pReq->getClientSessionId();
//                  unsigned __int16 bufLen=sizeof(buf);
//                  pReq->getHeader("SYS#ConnID",buf,&bufLen);
//                  glog(Log::L_INFO,"###Before newServerRequest");
//                  IServerRequest* pSReq=pSite->newServerRequest(pSessId,buf);
//                  glog(Log::L_INFO,"###after newServerRequest");
//                  if(pSReq)
//                  {
//                      pSReq->printCmdLine("\n\nThis is for METHOD PLAY and send to client");
//                      pSReq->printHeader("1","1");
//                      pSReq->post();
//                      pSReq->release();
//                      glog(Log::L_INFO,"###after newServerRequest POST");
//                  }
//
//                  glog(Log::L_INFO,"###Before newServerRequest with sess[%s] and connID[%s]",pSessId,strSetupConnid.c_str());
//                  pSReq=pSite->newServerRequest(pSessId,strSetupConnid.c_str());
//                  glog(Log::L_INFO,"###after newServerRequest");
//                  if(pSReq)
//                  {
//                      pSReq->printCmdLine("\n\nThis is for METHOD PLAY and send to ONDEMAND");
//                      pSReq->printHeader("1","1");
//                      pSReq->post();
//                      pSReq->release();
//                      glog(Log::L_INFO,"###after newServerRequest POST");
//                  }
                }
            }
            break;
		case RTSP_MTHD_TEARDOWN:
            {
                sessionMgr = ZQ::StreamSmith::_GlobalObject::getSessionMgr();
                response->printf_preheader("RTSP/1.0 200 OK");
                sprintf(buf, "%s", (RtspUtil::getMethodInString(pReq->getVerb())).getPtr());
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
                sprintf(buf, "%s", (RtspUtil::getMethodInString(pReq->getVerb())).getPtr());
                response->setHeader("Method-Code", buf);
                len = sizeof(buf);
                pReq->getHeader("CSeq", buf, &len);
                response->setHeader("CSeq", buf);
                response->setHeader("Session", pReq->getClientSessionId());
                response->post();
                sessionMgr->onAccessingSession(pReq);
                {
//                  const char* pSessId=pReq->getClientSessionId();
//                  unsigned __int16 bufLen=sizeof(buf);
//                  pReq->getHeader("SYS#ConnID",buf,&bufLen);
//                  IServerRequest* pSReq=pSite->newServerRequest(pSessId,buf);
//                  if(pSReq)
//                  {
//                      pSReq->printCmdLine("\n\nThis is for METHOD PLAY and send to client");
//                      pSReq->printHeader("1","1");
//                      pSReq->post();
//                      pSReq->release();
//                  }
//
//                  pSReq=pSite->newServerRequest(pSessId,strSetupConnid.c_str());
//                  if(pSReq)
//                  {
//                      pSReq->printCmdLine("\n\nThis is for METHOD PLAY and send to ONDEMAND");
//                      pSReq->printHeader("1","1");
//                      pSReq->post();
//                  }
                }
            }
            break;
        default:
            {
                glog( Log::L_ERROR,  "Unknown method: %s",
                    (RtspUtil::getMethodInString(pReq->getVerb())).getPtr());

                response->printf_preheader("RTSP/1.0 500 Internal Server Error");
                response->post();
                sessionMgr->onAccessingSession(pReq);
            }
        }
    } catch(...) {
        glog( Log::L_ERROR, "FixupRequestHandler(): exception has occurred");
        assert(false);
    }

    // ÔÚ Site ÖÐµ÷ÓÃ
    // pReq->release();

    return RequestDone;
}



#endif





// for rtsp performance counter
#define RTSP_PERFCOUNTER_RESET "Measure Reset"
static int32 _perfCounterReset = 0;
// register rtsp performance counter to snmp
static void registerPerfCounter()
{
	
    SNMPManageVariable( ZQSNMP_VARNAME("Statistics",RTSP_PERFCOUNTER_RESET), 
						&_perfCounterReset, 
						ZQSNMP_VARTYPE_INT32, 
						FALSE);
	
    SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Measured Since"),
						_GlobalObject::perfmon.nowUtc,
						ZQSNMP_VARTYPE_STRING, 
						TRUE);
    SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Request Count"),
						&_GlobalObject::perfmon.requestCount, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);
    SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Average Process Latency"),
						&_GlobalObject::perfmon.processedTimeAvg, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);
    SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Max Process Latency"),
						&_GlobalObject::perfmon.processedTimePeak, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);
    SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Average Latency"),
						&_GlobalObject::perfmon.durationAvg, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);
    SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Max Latency"),
						&_GlobalObject::perfmon.durationPeak, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","skip count"),
						&_GlobalObject::perfmon.skipCount, 
						ZQSNMP_VARTYPE_INT32, 
						FALSE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SETUP Latency-Max"),
						&_GlobalObject::perfmon.setupMax, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SETUP Latency-Min"),
						&_GlobalObject::perfmon.setupMin, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SETUP Latency-Avarage"),
						&_GlobalObject::perfmon.setupAvarage, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);


	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SETUP request Count"),
						&_GlobalObject::perfmon.setupCount, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","PLAY Latency-max"),
						&_GlobalObject::perfmon.playMax, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","PLAY Latency-min"),
						&_GlobalObject::perfmon.playMin, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","PLAY Latency-avarage"),
						&_GlobalObject::perfmon.playAvarage, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","PLAY request count"),
						&_GlobalObject::perfmon.playCount, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","GET_PARAMETER Latency-max"),
						&_GlobalObject::perfmon.getParaMax, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","GET_PARAMETER Latency-min"),
						&_GlobalObject::perfmon.getParaMin, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","GET_PARAMETER Latency-avarage"),
						&_GlobalObject::perfmon.getParaAvarage, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","GET_PARAMETER request count"),
						&_GlobalObject::perfmon.getParaCount, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SET_PARAMETER Latency-max"),
						&_GlobalObject::perfmon.setParaMax, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SET_PARAMETER Latency-min"),
						&_GlobalObject::perfmon.setParaMin, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SET_PARAMETER Latency-avarage"),
						&_GlobalObject::perfmon.setParaAvarage, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);
	
	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","SET_PARAMETER request count"),
						&_GlobalObject::perfmon.setParaCount, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","OPTIONS Latency-max"),
						&_GlobalObject::perfmon.optionMax, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","OPTIONS Latency-min"),
						&_GlobalObject::perfmon.optionMin, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","OPTIONS Latency-avarage"),
						&_GlobalObject::perfmon.optionAvarage, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","OPTIONS request count"),
						&_GlobalObject::perfmon.optionCount, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","TEARDOWN Latency-max"),
						&_GlobalObject::perfmon.teardownMax, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","TEARDOWN Latency-min"),
						&_GlobalObject::perfmon.teardownMin, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","TEARDOWN Latency-avarage"),
						&_GlobalObject::perfmon.teardownAvarage, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);

	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","TEARDOWN request count"),
						&_GlobalObject::perfmon.teardownCount, 
						ZQSNMP_VARTYPE_INT32, 
						TRUE);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Others Latency-max"),
// 						&_GlobalObject::perfmon.otherMax, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Others Latency-min"),
// 						&_GlobalObject::perfmon.otherMin, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Others Latency-avarage"),
// 						&_GlobalObject::perfmon.otherAvarage, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE);
// 
// 	SNMPManageVariable( ZQSNMP_VARNAME("Statistics","Others request count"),
// 						&_GlobalObject::perfmon.otherCount, 
// 						ZQSNMP_VARTYPE_INT32, 
// 						TRUE);


}

void uninitServer()
{
	try
	{
		int64 testT = SYS::getTickCount();

		ZQ::StreamSmith::ParseProcThrd::close();
		glog(Log::L_INFO,"RTSP parse threads stopped");

		dialogCreator->stopIdleMonitor();
		glog(Log::L_INFO,"RTSP idle connection monitor stopped");

//		_gThreadPool.stop();

		serviceFrm->uninit();
		glog(Log::L_INFO,"serviceFrm->uninit() time count="FMT64, SYS::getTickCount()-testT);

		testT=SYS::getTickCount();
		StreamSmithSite::DestroyStreamSmithSite();

		_gThreadPool.stop();

		glog(Log::L_INFO,"DestroyStreamSmithSite time count="FMT64, SYS::getTickCount()-testT);

		testT=SYS::getTickCount();

		_GlobalObject::uninit();
		glog(Log::L_DEBUG,"_GlobalObject::uninit() time count="FMT64, SYS::getTickCount()-testT);
	}
	catch (...) 
	{
		glog(Log::L_ERROR,"error occurred when un-initialize server");
	}
}


RtspProxyService::RtspProxyService()
{
	_svcLog = NULL;
	_pluginLog = NULL;
}
RtspProxyService::~RtspProxyService()
{

}

bool RtspProxyService::OnInit(void)
{
    _svcLog = _logger;
	ZQ::common::setGlogger(&_svcLog);

    DUMP_PATH = gRtspProxyConfig.szMiniDumpPath;

	StreamSmithSite::_strApplicationLogFolder = _logDir;
/*	
	//set file limit
	struct rlimit rt;
	rt.rlim_max = rt.rlim_cur = 50000;
    int rc = setrlimit(RLIMIT_NOFILE, &rt);
	if(rc == -1)
	{
		glog(ZQ::common::Log::L_ERROR,"RtspProxyService::OnInit() setrlimit set file size failed code{%d} string{%s}",
			errno,strerror(errno));
		return false;
	}
*/
	//get local time zone
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

	StreamSmithSite::_pDefaultSite=&defaultSite;
		
#if !defined _RTSP_PROXY || defined _LOCAL_TEST
	StreamSmithSite::_defaultContentHandler = SSMH_DefaultContentHandle;
#endif
	if (!_GlobalObject::init()) 
	{
		glog(Log::L_ERROR,"_GlobalObject::init() failed");
		return false;
	}
	
	StreamSmithSite::m_pSessionMgr = _GlobalObject::getSessionMgr();

	ServiceConfig cfg;
	dialogCreator = new RtspDialogCreator();
	if (0 == dialogCreator)
	{
		return false;
	}
	// start idle connection monitor
	dialogCreator->setIdleTimeout(gRtspProxyConfig.lServiceFrameIdleTimeout);
	dialogCreator->setIdleScanInterval(gRtspProxyConfig.lServiceFrameIdleScanInterval);
	dialogCreator->startIdleMonitor();	

	serviceFrm = new DataPostHouseService();
	if (NULL == serviceFrm)
	{
		return false;
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
		cfg._cfg_log = _svcLog;
		dialogCreator->setMaxConnection(gRtspProxyConfig.lMaxClientConnection);
		cfg._cfg_dialogCreator = dialogCreator;
		
		// use specified value of configuration
		// serviceFrm.setThreadCount(cfg._cfg_threadCount);
		
		// serviceFrm.setSecure(cfg._cfg_isSecure);
		// serviceFrm.setMaxConnection(cfg._cfg_maxConn);


		if (!serviceFrm->init(cfg))
		{
			return false;
		}
		_usrtsPort = GAPPLICATIONCONFIGURATION.lRtspSocketServerPort ; 
		_uslscPort = GAPPLICATIONCONFIGURATION.lLscpSocketServerPort;
		//defaultSite.SetIdsServerAddr(std::string(cfg._cfg_idsAddress));
		
		RtspSessionMgr* sessionMgr = _GlobalObject::getSessionMgr();
		sessionMgr->setSessionTimeout( 300000 );
		sessionMgr->sinkSessionEvent(SessionEvent::SessionRemoved, 
			dynamic_cast<SessionEvent* >(&defaultSite));
		sessionMgr->enable(true);
		
		std::string strLogFolder= _logDir;
		if (strLogFolder.at(strLogFolder.length()-1)!='/') 
		{
			strLogFolder+="/";
		}

		_pluginLog=new ZQ::common::FileLog((std::string(strLogFolder)+"rtspProxy.plugin.log").c_str(),
												gRtspProxyConfig.lPluginLogLevel,
												ZQLOG_DEFAULT_FILENUM,
												gRtspProxyConfig.lPluginLogSize,
												gRtspProxyConfig.lPluginLogBuffer,
												gRtspProxyConfig.lPluginLogTimeout);
		defaultSite.setLogInstance(_pluginLog);
		
		std::vector<std::string> configPath;
//		ZQ::common::stringHelper::SplitString("RtspProxy/applicationSite",configPath,"/"," ;\t\"/");
	
#if defined(_RTSP_PROXY) && defined(_LOCAL_TEST)		
		StreamSmithSite::_defaultContentHandler =(SSMH_ContentHandle)SSMH_ContentHandletest;
#endif
		if(!StreamSmithSite::SetupStreamSmithSite("",configPath))
		{
			glog(Log::L_ERROR,"SetUp Stream Smith Site fail");			
			return false;
		}
		
		if(gRtspProxyConfig.lServiceThreadCount<=2)
			gRtspProxyConfig.lServiceThreadCount = 3;
		ZQ::StreamSmith::_gThreadPool.resize(gRtspProxyConfig.lServiceThreadCount);

		if(gRtspProxyConfig.lRtspParserThreadPoolSize<2)
			gRtspProxyConfig.lRtspParserThreadPoolSize = 2;
		ZQ::StreamSmith::ParseProcThrd::create(gRtspProxyConfig.lRtspParserThreadPoolSize, gRtspProxyConfig.lRtspParserThreadPriority);
		glog(Log::L_INFO, L"Set RTSP parser threadpool size to %d", gRtspProxyConfig.lRtspParserThreadPoolSize);

//set process priority
		if (gRtspProxyConfig.lProcessPriority)
		{

		}
	}
	else
	{
		glog(Log::L_ERROR,"Load Configuaration fail,Service stopped");
		return false;
	}

#if defined(_RTSP_PROXY) && defined(_LOCAL_TEST)
	defaultSite.RegisterFixupRequest(SSMH_FixupRequest(FixupRequestHandler));
#endif	

// for rtsp performance counter
	registerPerfCounter();
	gRtspProxyConfig.snmpRegister("");

	return ZQDaemon::OnInit();
}

bool RtspProxyService::OnStart(void)
{
	_GlobalObject::perfmon.reset();
	
	if (!serviceFrm->begin())
	{
		return false;
	}
	
	char temp[32];
	sprintf(temp, "%d", _usrtsPort);
	serviceFrm->bindRtsp (gRtspProxyConfig.szRtspIPv4, gRtspProxyConfig.szRtspIPv6, temp);
	sprintf(temp, "%d", _uslscPort);
	serviceFrm->bindLscp (gRtspProxyConfig.szLscpIPv4, gRtspProxyConfig.szLscpIPv6, temp);

	if (gRtspProxyConfig.lServiceFrameSSLEnabled)
	{
		// add ssl communictaor
		serviceFrm->setCertAndKeyFile(gRtspProxyConfig.szServiceFramepublicKeyFile, gRtspProxyConfig.szServiceFrameprivateKeyFile, gRtspProxyConfig.szServiceFrameprivatePassword);
		
		serviceFrm->bindSSLLscp(gRtspProxyConfig.szLscpIPv4, gRtspProxyConfig.szLscpIPv6, gRtspProxyConfig.szLscpSSLPort);
		serviceFrm->bindSSLRtsp(gRtspProxyConfig.szRtspIPv4, gRtspProxyConfig.szRtspIPv6, gRtspProxyConfig.szRtspSSLPort);
	}
	
	//register rtsp port and lscp port into configuration so I can use it in the future and at any where
	GAPPLICATIONCONFIGURATION.lListenPort = _usrtsPort;
	GAPPLICATIONCONFIGURATION.lLscpPort = _uslscPort;
	GAPPLICATIONCONFIGURATION.lRtspPort = _usrtsPort;
	
	RtspSessionMgr* sessionMgr = _GlobalObject::getSessionMgr();
	sessionMgr->enable(true);

	return ZQDaemon::OnStart();
}

void RtspProxyService::OnStop(void)
{
	serviceFrm->end();

	ZQDaemon::OnStop();
}

void RtspProxyService::OnUnInit(void)
{
	int64 uninitB = SYS::getTickCount();
	uninitServer();
	delete serviceFrm;
	serviceFrm = NULL;
	glog(Log::L_DEBUG,"unintiliaze service time count="FMT64, SYS::getTickCount()-uninitB);
	

	if(_pluginLog)
	{
		try
		{
			delete _pluginLog;
		}
		catch(...){ }
		_pluginLog=NULL;
	}

	ZQDaemon::OnUnInit();
}









