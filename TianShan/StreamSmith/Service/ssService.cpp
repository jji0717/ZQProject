
#include <MiniDump.h>
#include <filelog.h>

#include <streamsmithconfig.h>
#include <streamsmithsite.h>
#include <strhelper.h>
#include "ssservice.h"
#include "DialogCreator.h"
#include "vvxParser/VstrmProc.h"
#include "VV2Parser.h"
#include "embededContentStore.h"
#include "SystemUtils.h"
#include <IceLog.h>
#ifdef _DEBUG
#include <adebugmem.h>
#endif

using namespace ZQ;
using namespace ZQ::StreamSmith;

DWORD gdwServiceType=1110;;
DWORD gdwServiceInstance=1;

//static ZQ::common::NativeThreadPool			_plManThreadPool(5);

RtspDialogCreatorPtr                dialogCreator = NULL;
DataPostHouseService*				serviceFrm;

static VstrmClass							*pStreamClass=NULL;
static PlaylistManager						*pListMan=NULL;

ZQ::common::Config::Loader<StreamSmithCfg> gStreamSmithConfig("StreamSmith.xml");

//StreamSmithConfig					gDummyConfig;
ZQ::common::Config::ILoader		*configLoader = &gStreamSmithConfig;

StreamSmithService					gStreamSmithServiceInstance;
extern BaseZQServiceApplication		*Application=&gStreamSmithServiceInstance;

namespace ZQ {
namespace StreamSmith {
extern RequestProcessResult SSMH_DefaultContentHandle (IStreamSmithSite* pSite,IClientRequestWriter* pReq);
}}

ZQ::common::MiniDump			_crashDump;
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);


void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	assert( pListMan!= NULL );
	if( pListMan )
	{
		pListMan->ServiceDown();
	}

	DWORD dwThreadID = GetCurrentThreadId();
	
	glog( Log::L_ERROR,  L"Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
		ExceptionCode, ExceptionAddress, dwThreadID);
	
	glog.flush();
	try
	{
		if ( gStreamSmithServiceInstance.m_pSessMonLog )
		{
			gStreamSmithServiceInstance.m_pSessMonLog->flush();
		}
	}
	catch (...) 
	{
	}


}
bool validatePath( const TCHAR *     wszPath )
{
    if (-1 != ::GetFileAttributes(wszPath))
        return true;
	
    DWORD dwErr = ::GetLastError();
    if ( dwErr == ERROR_PATH_NOT_FOUND || dwErr == ERROR_FILE_NOT_FOUND )
    {
        if (!::CreateDirectory(wszPath, NULL))
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


///streamsmith service
StreamSmithService::StreamSmithService()
{
}
StreamSmithService::~StreamSmithService()
{	
}

void StreamSmithService::doEnumSnmpExports() {
	BaseZQServiceApplication::doEnumSnmpExports();
	//{".2", "streamSmithAttrs" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).streamSmith(100).streamSmithAttrs(2)
	//{".2.1", "streamSmithAttr" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).streamSmith(100).streamSmithAttrs(2).streamSmithAttr(1)
	//{".2.1.1", "streamSmith-Version" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).streamSmith(100).streamSmithAttrs(2).streamSmithAttr(1).streamSmith-Version(1)
	//{".2.1.10", "streamSmith-configDir" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).streamSmith(100).streamSmithAttrs(2).streamSmithAttr(1).streamSmith-configDir(10)
	
	SvcMIB_ExportReadOnlyVar("streamSmith-default-eventChannel-endpoint",		gStreamSmithConfig.szEventChannelEndpoint,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-default-CrashDump-path",				gStreamSmithConfig.szMiniDumpPath, "");

	SvcMIB_ExportReadOnlyVar("streamSmith-default-CrashDump-enabled",			gStreamSmithConfig.lEnableMiniDump,  "");

	SvcMIB_ExportReadOnlyVar("streamSmith-default-IceTrace-enabled",			gStreamSmithConfig.lEnableIceTrace,  "");

	SvcMIB_ExportReadOnlyVar("streamSmith-default-IceTrace-enabled",			gStreamSmithConfig.lEnableIceTrace, "");

	SvcMIB_ExportReadOnlyVar("streamSmith-default-IceTrace-size",				gStreamSmithConfig.lIceLogSize,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-default-Database-path",				gStreamSmithConfig.szFailOverDatabasePath,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-netId",								gStreamSmithConfig.szServiceID,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-mode",								gStreamSmithConfig.serverMode,  "");
	//{".2.1.2", "streamSmith-SnmpLoggingMask" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).streamSmith(100).streamSmithAttrs(2).streamSmithAttr(1).streamSmith-SnmpLoggingMask(2)
	
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SuperPlugin-path",					gStreamSmithConfig.szSuperPluginPath,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SuperPlugin-enabled",					gStreamSmithConfig.lEnableSuperPlugin, "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SuperPlugin-eventSinkTimeout",		gStreamSmithConfig.lEventSinkTimeout,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SuperPlugin-enableShowDetail",		gStreamSmithConfig.lEnableShowEventSinkDetail,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Bind-endpoint",						gStreamSmithConfig.szSvcEndpoint,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Bind-dispatchSize",					gStreamSmithConfig.szAdapterThreadpoolSize,"");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Bind-dispatchMax",					gStreamSmithConfig.szAdapterThreadpoolSizeMax,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-RequestProcess-threads",				gStreamSmithConfig.lServiceThreadCount,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-LocalResource-configuration",			gStreamSmithConfig.szResourceManagerFileName,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-LocalResource-Streamer-defaultSpigotId",	gStreamSmithConfig.szDefaultSpigotID,  "");
	//{".2.1.3", "streamSmith-LogDir" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).streamSmith(100).streamSmithAttrs(2).streamSmithAttr(1).streamSmith-LogDir(3)
	
	SvcMIB_ExportReadOnlyVar("streamSmith-DatabaseCache-playlistSize",			gStreamSmithConfig.lEvictorPlaylistSize,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-DatabaseCache-dbHealthCheckThreshold",gStreamSmithConfig.lDbHealththreshold,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-DatabaseCache-itemSize",				gStreamSmithConfig.lEvictorItemSize,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Playlist-pauseWhenUnload",			gStreamSmithConfig.lPauseWhenUnloadItem,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Playlist-sessionScanInterval",		gStreamSmithConfig.lVstrmSessionScanInterval,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Playlist-bandwidthUsageScanInterval",	gStreamSmithConfig.lBandwidthUsageScanInterval,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Playlist-timeout",					gStreamSmithConfig.lPlaylistTimeout,  "");
	//{".2.1.4", "streamSmith-KeepAliveIntervals" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).streamSmith(100).streamSmithAttrs(2).streamSmithAttr(1).streamSmith-KeepAliveIntervals(4)
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Playlist-keepOnPause",				gStreamSmithConfig.lKeepOnPause,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Playlist-QueryLastItemPlayTime",		gStreamSmithConfig.lQueryLastItemPlayTime,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Playlist-progressInterval",			gStreamSmithConfig.lProgressEventSendoutInterval,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Playlist-EOTsize",					gStreamSmithConfig.lEOTProtectionTime,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Playlist-PreLoadTime",				gStreamSmithConfig.lPreloadTime,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Playlist-ForceNormalTimeBeforeEnd",	gStreamSmithConfig.lForceNormalTimeBeforeEnd, "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Playlist-delayedCleanup",				gStreamSmithConfig.lKillPlInterval,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Playlist-repositionInaccuracy",		gStreamSmithConfig.lRepositioInaccuracy, "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-CriticalStartPlayWait-enabled",		gStreamSmithConfig.lEnablemaxWaitTimeToStartFirstItem,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-CriticalStartPlayWait-timeout",		gStreamSmithConfig.lmaxWaitTimeToStartFirstItem,  "");
	//{".2.1.5", "streamSmith-ShutdownWaitTime" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).streamSmith(100).streamSmithAttrs(2).streamSmithAttr(1).streamSmith-ShutdownWaitTime(5)

	SvcMIB_ExportReadOnlyVar("streamSmith-MotoPreEncryption-cycle1",			gStreamSmithConfig.lEncryptionCycle1,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-MotoPreEncryption-freq1",				gStreamSmithConfig.lEncryptionFreq1,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-MotoPreEncryption-cycle2",			gStreamSmithConfig.lEncryptionCycle2, "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-MotoPreEncryption-freq2",				gStreamSmithConfig.lEncryptionFreq2,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Plugin-log-level",					gStreamSmithConfig.lPluginLogLevel,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Plugin-log-fileSize",					gStreamSmithConfig.lPluginLogSize, "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Plugin-log-buffer",					gStreamSmithConfig.lPluginLogBuffer,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-Plugin-log-flushTimeout",				gStreamSmithConfig.lPluginLogTimeout,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SessionMonitor-log-level",			gStreamSmithConfig.lSessMonLogLevel,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SessionMonitor-log-fileSize",			gStreamSmithConfig.lSessMonLogSize,  "");
	//{".2.1.6", "streamSmith-LogFileSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).streamSmith(100).streamSmithAttrs(2).streamSmithAttr(1).streamSmith-LogFileSize(6)
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SessionMonitor-log-buffer",			gStreamSmithConfig.lSessMonLogBuffer,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SessionMonitor-log-flushTimeout",		gStreamSmithConfig.lSessMonLogTimeout,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-PerfTune-IceFreezeEnvironment-CheckpointPeriod",	gStreamSmithConfig.szIceEnvCheckPointPeriod,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-PerfTune-IceFreezeEnvironment-DbRecoverFatal",	gStreamSmithConfig.szIceEnvDbRecoverFatal,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-PerformanceTune-playlist-SavePeriod",				gStreamSmithConfig.szFreezePlaylistSavePeriod, "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-PerformanceTune-playlist-SaveSizeTrigger",		gStreamSmithConfig.szFreezePlaylistSaveSizeTrigger,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-PerformanceTune-item-SavePeriod",					gStreamSmithConfig.szFreezeItemSavePeriod,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-PerformanceTune-item-SaveSizeTrigger",			gStreamSmithConfig.szFreezeItemSaveSizeTrigger,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SocketServer-enabled",							gStreamSmithConfig.lIsSvcFrameWork,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SocketServer-rtspPort",							gStreamSmithConfig.lRtspSocketServerPort,  "");
	//{".2.1.7", "streamSmith-LogWriteTimeOut" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).streamSmith(100).streamSmithAttrs(2).streamSmithAttr(1).streamSmith-LogWriteTimeOut(7)
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SocketServer-lscpPort",							gStreamSmithConfig.lLscpSocketServerPort,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SocketServer-maxConnections",						gStreamSmithConfig.lMaxClientConnection,  "");

	SvcMIB_ExportReadOnlyVar("streamSmith-SocketServer-threads",							gStreamSmithConfig.lServiceFrameThreadPoolSize,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SocketServer-threadPriority",						gStreamSmithConfig.lServiceFrameThreadPriority,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SocketServer-debugLevel",							gStreamSmithConfig.lServiceFrameDebugLevel,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SocketServer-debugDetail",						gStreamSmithConfig.lServiceFrameDebugDetail,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SocketServer-idleTimeout",						gStreamSmithConfig.lServiceFrameIdleTimeout,  "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SocketServer-idleScanInterval",					gStreamSmithConfig.lServiceFrameIdleScanInterval, "");
	
	SvcMIB_ExportReadOnlyVar("streamSmith-SocketServer-SSL-enabled",						gStreamSmithConfig.lServiceFrameSSLEnabled,  "");
	//{".2.1.8", "streamSmith-LogBufferSize" }, //{enterprises}.zqInteractive(22839).tianShanComponents(4).tianShanService(1).streamSmith(100).streamSmithAttrs(2).streamSmithAttr(1).streamSmith-LogBufferSize(8)
	
	SvcMIB_ExportReadOnlyVar("streamSmith-IncomingMessage-maxLen",							gStreamSmithConfig.lIncomingMsgMaxLen,  "");	
}

#define STRSWITCH() if(0){
#define STRCASE(x)	} else if(::strncmp(varName , x ,strlen(x) ) == 0 ){
#define STRENDCASE() }

void StreamSmithService::OnSnmpSet(const char *varName)
{
	if ( !varName || strlen(varName) <= 0 ) 
	{
		glog(ZQ::common::Log::L_INFO , "WeiwooService Invalid varName in OnSnmpSet , just return directly ");
		return ; 
	}
	STRSWITCH()
	STRCASE("StreamSmith/SuperPlugin/eventSinkTimeout")
		if( gStreamSmithConfig.lEventSinkTimeout <= 5000 )
		{
			glog(ZQ::common::Log::L_INFO , 
				"set StreamSmith/SuperPlugin/eventSinkTimeout to %d , but be adjusted to [%d] ",
				 gStreamSmithConfig.lEventSinkTimeout , 5000);
			 gStreamSmithConfig.lEventSinkTimeout = 5000;
		}
		else
		{
			glog(ZQ::common::Log::L_INFO , "Set to StreamSmith/SuperPlugin/eventSinkTimeout [%d]" ,gStreamSmithConfig.lEventSinkTimeout);
		}

	STRCASE("StreamSmith/SuperPlugin/enableShowDetail")
		glog(ZQ::common::Log::L_INFO , "set StreamSmith/SuperPlugin/enableShowDetail to [%d] ",gStreamSmithConfig.lEnableShowEventSinkDetail);

	STRCASE("StreamSmith/ContentStore/enableLocalVvx")
		glog(ZQ::common::Log::L_INFO , "set StreamSmith/ContentStore/enableLocalVvx to [%d]",gStreamSmithConfig.lUseLocalVvxParser);

	STRCASE("StreamSmith/ContentStore/EnableQueryFromContentStore")
		glog(ZQ::common::Log::L_INFO , "set StreamSmith/ContentStore/EnableQueryFromContentStore to [%d]",gStreamSmithConfig.lEnableQueryFromContentStore);

	STRCASE("StreamSmith/ContentStore/UseLocalParserValue")
		glog(ZQ::common::Log::L_INFO , "set to StreamSmith/ContentStore/UseLocalParserValue [%d]",gStreamSmithConfig.lUseLocalParserValue);

	STRCASE("StreamSmith/Playlist/timeout")
		if( gStreamSmithConfig.lPlaylistTimeout < 10000 )
		{
			glog(ZQ::common::Log::L_INFO , 
				"set StreamSmith/Playlist/timeout to [%d] but be adjusted to [%d]",
				gStreamSmithConfig.lPlaylistTimeout, 10000);
		}
		else
		{
			glog(ZQ::common::Log::L_INFO , "set StreamSmith/Playlist/timeout to [%d]" , gStreamSmithConfig.lPlaylistTimeout );
		}
	STRCASE("StreamSmith/Playlist/EOTsize")
		if(gStreamSmithConfig.lEOTProtectionTime < 0 )
			gStreamSmithConfig.lEOTProtectionTime = 0;
		glog(ZQ::common::Log::L_INFO , "set StreamSmith/Playlist/EOTsize to [%d]",gStreamSmithConfig.lEOTProtectionTime);
		gStreamSmithConfig.lForceNormalTimeBeforeEnd = gStreamSmithConfig.lEOTProtectionTime;
	
	STRCASE("StreamSmith/Playlist/delayedCleanup")
		if ( gStreamSmithConfig.lKillPlInterval < 10000 ) 
		{
			glog(ZQ::common::Log::L_INFO , 
				"set StreamSmith/Playlist/delayedCleanup to [%d] but be adjusted to [%d] ",
				gStreamSmithConfig.lKillPlInterval , 10000 );
		}
		else
		{
			glog(ZQ::common::Log::L_INFO , 
				"set StreamSmith/Playlist/delayedCleanup to [%d] ",
				gStreamSmithConfig.lKillPlInterval);
		}
	
	STRCASE("StreamSmith/CriticalStartPlayWait/enable")
		glog(ZQ::common::Log::L_INFO , "set  StreamSmith/CriticalStartPlayWait/enable to [%d]",gStreamSmithConfig.lEnablemaxWaitTimeToStartFirstItem);

	STRCASE("StreamSmith/CriticalStartPlayWait/timeout")
		if( gStreamSmithConfig.lmaxWaitTimeToStartFirstItem > 5000 || gStreamSmithConfig.lmaxWaitTimeToStartFirstItem < 0 )
		{
			glog(ZQ::common::Log::L_INFO , "Set to StreamSmith/CriticalStartPlayWait/timeout [%d] but be adjusted to [%d]",
				gStreamSmithConfig.lmaxWaitTimeToStartFirstItem ,
				5000 );
			gStreamSmithConfig.lmaxWaitTimeToStartFirstItem = 5000;
		}
		else
		{
			glog(ZQ::common::Log::L_INFO , "Set to StreamSmith/CriticalStartPlayWait/timeout [%d] ",
				gStreamSmithConfig.lmaxWaitTimeToStartFirstItem );
		}

	STRCASE("StreamSmith/MotoPreEncryption/cycle1")
		glog(ZQ::common::Log::L_INFO,"set StreamSmith/MotoPreEncryption/cycle1 to [%d]",gStreamSmithConfig.lEncryptionCycle1);

	STRCASE("StreamSmith/MotoPreEncryption/freq1")
		glog(ZQ::common::Log::L_INFO,"set StreamSmith/MotoPreEncryption/freq1 to [%d]",gStreamSmithConfig.lEncryptionFreq1);
	
	STRCASE("StreamSmith/MotoPreEncryption/cycle2")
		glog(ZQ::common::Log::L_INFO,"set StreamSmith/MotoPreEncryption/cycle2 to [%d]",gStreamSmithConfig.lEncryptionCycle2);

	STRCASE("StreamSmith/MotoPreEncryption/freq2")
		glog(ZQ::common::Log::L_INFO,"set StreamSmith/MotoPreEncryption/freq2 to [%d]",gStreamSmithConfig.lEncryptionFreq2);

	STRENDCASE()
}

class NodeUpChecker : public ZQ::common::ThreadRequest
{
public:
	NodeUpChecker(	ZQ::common::NativeThreadPool& pool , 
					ZQADAPTER_DECLTYPE 	objAdapter,
					VstrmClass *pStreamClass,
					std::string& strConfigFolder,
					ZQ::common::Log* pSessLog )
		:ZQ::common::ThreadRequest(pool)
	{
		m_pStreamClass = pStreamClass;
		m_Adapter = objAdapter;
		waitInterval = 1 * 1000;
		m_strConfigConfigFolder = strConfigFolder;
		m_pSessLog = pSessLog;
	}
	bool startNodeContentStore( VstrmClass* pClass )
	{
		VV2Parser::vsmInit(pClass->handle());

		vsm_Initialize(pClass->handle());

		Ice::ObjectPrx objPrx = NCSBridge::StartContentStore( pListMan->m_Adapter , m_strConfigConfigFolder , glog, pClass->handle());

		if ( !objPrx ) 
		{
			//logEvent(Log::L_ERROR,"Initialize node content store failed");
			return false;
		}

		pListMan->setContentStoreProxy( objPrx );
		
		return true;
	}
	bool startStreamService( VstrmClass *pStreamClass )
	{
		pListMan->StartFailOver(m_pSessLog);
		
		pListMan->StartSessionMonitor( gStreamSmithConfig.szDefaultSpigotID );

		pListMan->start();
		return true;
	}
	int run( )
	{
		bool contentStoreStarted =false;
		bool bVstrmClassInit = false;
		assert( m_pStreamClass != NULL );
		while( true )
		{
			if( m_pStreamClass->isNodeReady() )
			{
				if( !bVstrmClassInit && m_pStreamClass->initVstrmClass( ) )
				{
					bVstrmClassInit = true;
					if( !contentStoreStarted && !startNodeContentStore(m_pStreamClass) )
					{
						glog(ZQ::common::Log::L_ERROR,"failed to start embeded node content store,quiting...");
						return 0;
					}
				
					contentStoreStarted = true;
				}
				if( m_pStreamClass->refreshPortInfo() && m_pStreamClass->isValid() )
				{
					startStreamService( m_pStreamClass );
					
					int32 waitInterval = min(5000, gStreamSmithConfig.lPlaylistTimeout);
					glog(ZQ::common::Log::L_INFO, "wait another [%d]ms and then activate adapter",waitInterval);

					ZQ::common::delay( waitInterval );

					m_Adapter->activate( );

					NCSBridge::mountContentStore();

					glog(ZQ::common::Log::L_INFO,"Node Is Ready to go");
					break;
				}							
			}
			glog(ZQ::common::Log::L_ERROR,"Node is not available");
			waitInterval = waitInterval * 2;
			if( waitInterval > 32 * 1000 )
				waitInterval = 32 * 1000;
			//Sleep( waitInterval );
			//m_StopEvent
			if( WaitForSingleObject(gStreamSmithServiceInstance.getStopEvent(),waitInterval ) ==WAIT_OBJECT_0 )
			{
				break;
			}
		}
		
		return 1;
	}
	void final(int retcode /* =0 */, bool bCancelled /* =false */)
	{
		delete this;
	}
private:
	ZQADAPTER_DECLTYPE 		m_Adapter;
	int						waitInterval;
	VstrmClass				*m_pStreamClass;
	std::string				m_strConfigConfigFolder;
	ZQ::common::Log*		m_pSessLog;
};


HRESULT StreamSmithService::OnInit()
{
	WSAData sockData;
	WORD wVersion=MAKEWORD(2,0);
	WSAStartup(wVersion,&sockData);

	//	{
//        gStreamSmithConfig.setLogger(m_pReporter);
//        if(!gStreamSmithConfig.loadInFolder(m_wsConfigFolder))
//        {
//            glog(ZQ::common::Log::L_ERROR,"Can't load config in folder [%s]", m_wsConfigFolder);
//            logEvent(ZQ::common::Log::L_ERROR,"Can't load config in folder [%s]", m_wsConfigFolder);
//            return S_FALSE;
//        }
//    }
	StreamSmithSite::_strApplicationLogFolder = m_wsLogFolder;

	///config crash dump
	if (!validatePath(gStreamSmithConfig.szMiniDumpPath))
	{
		glog(Log::L_ERROR, L"CrashDumpPath %s error", gStreamSmithConfig.szMiniDumpPath);
		logEvent(ZQ::common::Log::L_ERROR,_T("invalid minidump apth %s"),gStreamSmithConfig.szMiniDumpPath);
		return S_FALSE;
	}	
	_crashDump.setDumpPath(gStreamSmithConfig.szMiniDumpPath);
	_crashDump.enableFullMemoryDump(gStreamSmithConfig.lEnableMiniDump);
	_crashDump.setExceptionCB(CrashExceptionCallBack);

	///initialize log instance
//	if(m_pLog!=NULL)
//	{
//		delete m_pLog;
//		m_pLog=NULL;
//	}
	if(m_pPluginLog)
	{
		delete m_pPluginLog;
		m_pPluginLog=NULL;
	}
//	m_pLog=new ZQ::common::FileLog( (std::string(m_wsLogFolder)+gStreamSmithConfig.szSvcLogFileName).c_str(),
//											gStreamSmithConfig.lSvcLogLevel,
//											ZQLOG_DEFAULT_FILENUM,
//											gStreamSmithConfig.lSvcLogSize,
//											gStreamSmithConfig.lSvcLogbuffer,
//											gStreamSmithConfig.lSvcLogTimeout);
	m_pLog = m_pReporter;
	
	std::string strLogFolder= m_wsLogFolder;
	if (strLogFolder.at(strLogFolder.length()-1)!='\\') 
	{
		strLogFolder+="\\";
	}

	m_pPluginLog=new ZQ::common::FileLog( (std::string(strLogFolder)+"StreamSmith.plugin.log").c_str(),
											gStreamSmithConfig.lPluginLogLevel,
											ZQLOG_DEFAULT_FILENUM,
											gStreamSmithConfig.lPluginLogSize,
											gStreamSmithConfig.lPluginLogBuffer,
											gStreamSmithConfig.lPluginLogTimeout,
											ZQLOG_DEFAULT_EVENTLOGLEVEL,
											m_sServiceName);
	m_pSessMonLog=new ZQ::common::FileLog( (std::string(strLogFolder)+"StreamSmith.sess.log").c_str(),
											gStreamSmithConfig.lSessMonLogLevel,
											ZQLOG_DEFAULT_FILENUM,
											gStreamSmithConfig.lSessMonLogSize,
											gStreamSmithConfig.lSessMonLogBuffer,
											gStreamSmithConfig.lSessMonLogTimeout,
											ZQLOG_DEFAULT_EVENTLOGLEVEL,
											m_sServiceName);


	gStreamSmithConfig.serverMode = 0 ;
	if( stricmp(gStreamSmithConfig.strServerMode.c_str() , "nPVRServer") == 0 )
	{
		logEvent(ZQ::common::Log::L_INFO,_T("run as nPVR Server"));
		gStreamSmithConfig.serverMode = 1;
	}
	else if( stricmp(gStreamSmithConfig.strServerMode.c_str() , "EdgeServer") == 0 )
	{
		logEvent(ZQ::common::Log::L_INFO,_T("run as Edge Server"));
		gStreamSmithConfig.serverMode = 2;
	}
	else
	{
		logEvent(ZQ::common::Log::L_INFO,_T("run as Normal Server"));
	}

	int i=0;
	Ice::InitializationData iceInitData;
	iceInitData.properties =Ice::createProperties(i,NULL);
	iceInitData.properties->setProperty("Ice.ThreadPool.Client.Size","5");
	iceInitData.properties->setProperty("Ice.ThreadPool.Client.SizeMax","10");
	iceInitData.properties->setProperty("Ice.ThreadPool.Server.Size","5");
	iceInitData.properties->setProperty("Ice.ThreadPool.Server.SizeMax","10");
	iceInitData.properties->setProperty("PLInfoStore.ThreadPool.Size",gStreamSmithConfig.szAdapterThreadpoolSize);
	iceInitData.properties->setProperty("PLInfoStore.ThreadPool.SizeMax",gStreamSmithConfig.szAdapterThreadpoolSizeMax);
	
	std :: map<std::string, std::string>::const_iterator it = gStreamSmithConfig.iceProperties.begin();
	for( ; it != gStreamSmithConfig.iceProperties.end(); it++ )
	{		
		iceInitData.properties->setProperty( it->first , it->second );
	}		
	
	///initialize ice communicator
	m_pIceLogger = NULL;
	if( gStreamSmithConfig.lEnableIceTrace >= 1)
	{
		if( gStreamSmithConfig.lIceTraceStandAlone >= 1)
		{
			if(m_pIceLogger!=NULL)
			{
				delete m_pIceLogger;
				m_pIceLogger=NULL;
			}
			m_pIceLogger=new ZQ::common::FileLog( ((std::string(strLogFolder)+"StreamSmith.IceTrace.log")).c_str(),
				gStreamSmithConfig.lIceLogLevel,
				ZQLOG_DEFAULT_FILENUM,
				gStreamSmithConfig.lIceLogSize,
				gStreamSmithConfig.lIceLogBuffer,
				gStreamSmithConfig.lIceLogTimeout);
			iceInitData.logger = new TianShanIce::common::IceLogI(m_pIceLogger);
		}
		else
		{
			iceInitData.logger = new TianShanIce::common::IceLogI(m_pLog);
		}
		m_ic=Ice::initialize(i,NULL,iceInitData);
	}
	else
	{
		m_ic=Ice::initialize(i,NULL,iceInitData);
	}

	///initialize streamsmith site
	StreamSmithSite::_pDefaultSite=&defaultSite;
	StreamSmithSite::_defaultContentHandler = SSMH_DefaultContentHandle;

	if (!_GlobalObject::init()) 
	{
		logEvent(ZQ::common::Log::L_ERROR,_T("GlobalObject initialize failed,service down"));
		return S_FALSE;
	}
	StreamSmithSite::m_pSessionMgr = _GlobalObject::getSessionMgr();

	ServiceConfig cfg;
	dialogCreator = new RtspDialogCreator();
	if (0 == dialogCreator)
	{
		return false;
	}
// 	serviceFrm = new DataPostHouseService();
// 	if (NULL == serviceFrm)
// 	{
// 		return false;
// 	}

	if(pListMan)
	{
		delete pListMan;
		pListMan=NULL;
	}
	if(pStreamClass)
	{
		delete pStreamClass;
		pStreamClass=NULL;
	}

	pStreamClass=new VstrmClass(ZQ::StreamSmith::_gThreadPool);

#ifdef _ICE_INTERFACE_SUPPORT
	try
	{
		pListMan=new PlaylistManager(*pStreamClass,
										m_ic,
										strlen(gStreamSmithConfig.szSvcEndpoint)>0?gStreamSmithConfig.szSvcEndpoint:NULL);
	}
	catch(...)
	{
		logEvent(ZQ::common::Log::L_ERROR,_T("construct playlistManager failed,service down"));
		return S_FALSE;
	}
#else//_ICE_INTERFACE_SUPPORT
	try
	{
		pListMan=new PlaylistManager(*pStreamClass);
	}
	catch (...)
	{
		logEvent(ZQ::common::Log::L_ERROR,_T("construct playlistManager failed,service down"));
		return S_FALSE;
	}
#endif//_ICE_INTERFACE_SUPPORT
	
	///validate configuration
	int32 retryCount = gStreamSmithConfig.lRetryCountAtBusyChangeSpeed ;
	gStreamSmithConfig.lRetryCountAtBusyChangeSpeed = MAX(1,retryCount);

	StreamSmithSite::m_pvstrmClass=pStreamClass;
	StreamSmithSite::m_pPlayListManager=pListMan;
	{		
// 		cfg._cfg_debugLevel		= GAPPLICATIONCONFIGURATION.lServiceFrameDebugLevel;
// 		cfg._cfg_isSecure		= false;
// 		cfg._cfg_threadCount	= GAPPLICATIONCONFIGURATION.lServiceFrameThreadPoolSize;
// 		cfg._cfg_maxConn		= GAPPLICATIONCONFIGURATION.lMaxClientConnection;
// 		cfg._cfg_recvBufSize	= GAPPLICATIONCONFIGURATION.lIncomingMsgRecvBufSize;
// 		cfg._cfg_threadPriority	= GAPPLICATIONCONFIGURATION.lServiceFrameThreadPriority;
// 		cfg._cfg_minWorkingSet	= 20*1024*1024;
// 		strncpy( cfg._cfg_publicKeyFile , GAPPLICATIONCONFIGURATION.szServiceFramepublicKeyFile , sizeof(cfg._cfg_publicKeyFile) -1 );
// 		strncpy( cfg._cfg_privateKeyFile, GAPPLICATIONCONFIGURATION.szServiceFrameprivateKeyFile, sizeof(cfg._cfg_privateKeyFile) );
// 		strncpy( cfg._cfg_privateKeyFilePwd,GAPPLICATIONCONFIGURATION.szServiceFrameprivatePassword,sizeof(cfg._cfg_privateKeyFilePwd));
// 		strncpy( cfg._cfg_dhParamFile , GAPPLICATIONCONFIGURATION.szServiceFramedhParamFile,sizeof(cfg._cfg_dhParamFile));
// 		strncpy( cfg._cfg_randFile , GAPPLICATIONCONFIGURATION.szServiceFramerandFile , sizeof(cfg._cfg_randFile));
	}
	//serviceFrm.init(cfg);
	m_usPort = GAPPLICATIONCONFIGURATION.lRtspSocketServerPort ; 
	
	// use specified value of configuration
	//serviceFrm.setThreadCount(gStreamSmithConfig.lSvcFrameThreadCount);
	//addr->sin_port = htons((u_short)(u_short)cfg._cfg_serverPort);
	//serviceFrm.setSecure(gStreamSmithConfig.lSvcFrameIsSecure);
	//serviceFrm.setMaxConnection(gStreamSmithConfig.lSvcFrameMaxConnection);
	//defaultSite.SetIdsServerAddr(std::string(cfg._cfg_idsAddress));	
	RtspSessionMgr* sessionMgr = _GlobalObject::getSessionMgr();
	sessionMgr->setSessionTimeout( 300000 );
	sessionMgr->sinkSessionEvent(SessionEvent::SessionRemoved,dynamic_cast<SessionEvent* >(&defaultSite));
	sessionMgr->enable(true);
	
	defaultSite.setLogInstance(m_pPluginLog);
	
	std::vector<std::string> configPath;
	//ZQ::common::stringHelper::SplitString("StreamSmith/applicationSite",configPath,"/"," ;\t\"/");	
	if(!StreamSmithSite::SetupStreamSmithSite( "" , configPath))
	{
		logEvent(Log::L_ERROR,"streamSmithService SetUp Stream Smith Site fail");
		return S_FALSE;
	}
	if(gStreamSmithConfig.lServiceThreadCount <= 2)
		gStreamSmithConfig.lServiceThreadCount = 3;
	ZQ::StreamSmith::_gThreadPool.resize(gStreamSmithConfig.lServiceThreadCount);

	if( strlen( gStreamSmithConfig.szResourceManagerFileName ) <=0)
	{
		logEvent(Log::L_INFO,"no resourceManager,run as normal!");
	}
	else
	{
		if(!pListMan->ParseResourceManager( (char*)(std::string(m_wsConfigFolder)+gStreamSmithConfig.szResourceManagerFileName).c_str()))
		{
			logEvent(Log::L_ERROR,"Parse resource config file fail");
			return S_FALSE;
		}
	}


// 	pListMan->StartSessionMonitor(m_pSessMonLog,
// 									strlen(gStreamSmithConfig.szSuperPluginPath)>0?gStreamSmithConfig.szSuperPluginPath:NULL,
// 									strlen(gStreamSmithConfig.szEventChannelEndpoint)>0?gStreamSmithConfig.szEventChannelEndpoint:NULL ,
// 									gStreamSmithConfig.szServiceID,
// 									gStreamSmithConfig.szDefaultSpigotID);


	pListMan->initIceInterface(m_pSessMonLog,
									strlen(gStreamSmithConfig.szSuperPluginPath)>0?gStreamSmithConfig.szSuperPluginPath:NULL,
									strlen(gStreamSmithConfig.szEventChannelEndpoint)>0?gStreamSmithConfig.szEventChannelEndpoint:NULL ,
									gStreamSmithConfig.szServiceID,
									gStreamSmithConfig.szDefaultSpigotID);
	

	pListMan->SetKillerThreadWaitInterval(gStreamSmithConfig.lKillPlInterval);
	pListMan->SetPlaylistTimeout(gStreamSmithConfig.lPlaylistTimeout,
								gStreamSmithConfig.lKeepOnPause > 0 ? false : true );

	

	gStreamSmithConfig.snmpRegister("");
	return BaseZQServiceApplication::OnInit();
}
HRESULT StreamSmithService::OnStart()
{	
	NodeUpChecker* checker = new NodeUpChecker( ZQ::StreamSmith::_gThreadPool ,
												pListMan->m_Adapter , 
												pStreamClass , 
												std::string(m_wsConfigFolder),
												m_pSessMonLog);
	assert(checker!=NULL);
	checker->start();

	return BaseZQServiceApplication::OnStart();
}
HRESULT StreamSmithService::OnStop()
{
	try
	{
		pListMan->m_Adapter->deactivate();
	}
	catch (...)
	{}

	NCSBridge::StopContentStore(glog);
	
	WSACleanup();

	if(gStreamSmithConfig.lIsSvcFrameWork)
	{
		//serviceFrm.uninit();
	}
		
	if(pListMan)
	{
		pListMan->ServiceDown();
	}
	
	return BaseZQServiceApplication::OnStop();
}

HRESULT StreamSmithService::OnUnInit()
{	

	ZQ::StreamSmith::_gThreadPool.stop();
	if(pListMan)
	{
		try
		{			
			delete pListMan;
			pListMan=NULL;
			defaultSite.m_pPlayListManager=NULL;
		}
		catch(...)
		{
		}
	}
	if(pStreamClass)
	{
		try
		{
			delete pStreamClass;
			pStreamClass=NULL;
		}
		catch(...)
		{
		}
	}

	
	try
	{
		DWORD dwTest=GetTickCount();
		StreamSmithSite::DestroyStreamSmithSite();
		
		glog(Log::L_DEBUG,"destroyStreamSmithSite, used [%d]ms",GetTickCount()-dwTest);
		dwTest=GetTickCount();
		
		
		//serviceFrm.uninit();
		glog(Log::L_DEBUG,"serviceFrm.uninit(), used [%d]ms",GetTickCount()-dwTest);
		dwTest=GetTickCount();
		
		_GlobalObject::uninit();
		glog(Log::L_DEBUG,"_GlobalObject::uninit(), used [%d] ms",GetTickCount()-dwTest);
	}
	catch (...) 
	{
		glog(Log::L_ERROR,"error occurred here");
	}	

	try
	{
		//m_ic->shutdown();
		//m_ic->destroy();		
	}
	catch (...) 
	{
	}

	try
	{	
		try{if(m_pSessMonLog){	delete m_pSessMonLog;m_pSessMonLog=NULL;}}
		catch(...){ }

		try{if(m_pPluginLog){	delete m_pPluginLog;m_pPluginLog=NULL;}}
		catch(...){ }

		try{if(m_pIceLogger){	delete m_pIceLogger;m_pIceLogger=NULL;}}
		catch(...){ }		

	}
	catch (...)
	{
	}

	return BaseZQServiceApplication::OnUnInit();
}
