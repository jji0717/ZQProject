#ifndef __TianShanS1_Environment_H__
#define __TianShanS1_Environment_H__

#include "ZQ_common_conf.h"

#ifdef ZQ_OS_MSWIN
#include <mtsevents.h>
#endif

#include <string>
#include <map>
#include <vector>
#include <list>

// zq common
#include <FileLog.h>
#include <Log.h>
#include <Locks.h>
#include <XMLPreferenceEx.h>
#include <NativeThreadPool.h>

// tianshanice
#include <TsSRM.h>
#include <TsApplication.h>
#include <TsStreamer.h>
#include <TsEvents.h>

// ice
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include "./StreamIdx.h"
#include <IceLog.h>

// current directory
#include "./stroprt.h"
#include "./Factory.h"
#include "./SessionContextImpl.h"
#include "./ZQResource.h"
#include "../../RtspRelevant.h"
#include "./ConnectService.h"
#include "./WatchDog.h"
#include "./TsConfig.h"

// other
#include "../../StreamSmithModule.h"

// zqprojs/tianshan/common
#include "../../../common/TianShanDefines.h"
#include "../../../common/EventChannel.h"

#define SSMLOG (*s1log)

namespace TianShanS1
{
	
#define INDEXFILENAME(_X) #_X "Idx"
	
#define ServantType "RtspSess"
	
// log level
#define DebugLevel			ZQ::common::Log::L_DEBUG
#define InfoLevel			ZQ::common::Log::L_INFO
#define NoticeLevel			ZQ::common::Log::L_NOTICE
#define WarningLevel		ZQ::common::Log::L_WARNING
#define ErrorLevel			ZQ::common::Log::L_ERROR
	
// the key for in out map
#define MapKeyClientSession			"ClientSession"
#define MapKeyMethod				"Method"
#define MapKeySequence				"Sequence"
#define MapKeyRequestIdentity		"RequestIdentity"
#define MapKeyPhase					"Phase"
#define MapKeyUserAgent				"UserAgent"
#define MapKeyXReason				"XReason"
#define MapKeyWeiwooSession			"WeiwooSession"
#define MapKeyError					"ErrorDept"
#define MapKeyDestroyReason			"DestroyReason"
#define MapKeyWeiwooSessionPrxID	"WeiwooSessionPrxID"

#define ICE_CONTEXT_EVENT_SEQUENCE	"EventSeq" // ice context's event sequence
	
// SeaChange announce code
#define SC_ANNOUNCE_ENDOFSTREAM				"2101"
#define SC_ANNOUNCE_ENDOFSTREAM_STRING		"End-of-Stream Reached"
#define SC_ANNOUNCE_BEGINOFSTREAM			"2104"
#define SC_ANNOUNCE_BEGINOFSTREAM_STRING	"Start-of-Stream Reached"
#define SC_ANNOUNCE_ITEMSTEPPED				"8803"
#define SC_ANNOUNCE_ITEMSTEPPED_STRING		"\"End-of-Item Reached\""
#define SC_ANNOUNCE_SCALECHANGED			"8801"
#define SC_ANNOUNCE_SCALECHANGED_STRING		"\"Scale Changed\""
#define SC_ANNOUNCE_STATECHANGED			""
#define SC_ANNOUNCE_STATECHANGED_STRING		""
#define SC_ANNOUNCE_STREAMEXIT				""
#define SC_ANNOUNCE_STREAMEXIT_STRING		""
#define SC_ANNOUNCE_PING					""
#define SC_ANNOUNCE_PING_STRING				""

// TianShan announce code
#define NUMERIC_CATEGORY
#ifdef NUMERIC_CATEGORY
#  define TS_ANNOUNCE_ENDOFSTREAM			"0001::0001 End-of-Stream Reached"
#  define TS_ANNOUNCE_BEGINOFSTREAM			"0001::0002 Beginning-of-Stream Reached"
#  define TS_ANNOUNCE_STATECHANGED			"0001::0003 State Changed"
#  define TS_ANNOUNCE_SCALECHANGED			"0001::0004 Scale Changed"
#  define TS_ANNOUNCE_ITEMSTEPPED			"0002::0001 Item Stepped"
#  define TS_ANNOUNCE_SESSIONTIMEOUT		"0100::0001 Session Timeout"
#  define TS_ANNOUNCE_STREAMEXIT			""
#  define TS_ANNOUNCE_PING					""
#else
#  define TS_ANNOUNCE_ENDOFSTREAM			"Stream::0001 End-of-Stream Reached"
#  define TS_ANNOUNCE_BEGINOFSTREAM			"Stream::0002 Beginning-of-Stream Reached"
#  define TS_ANNOUNCE_STATECHANGED			"Stream::0003 State Changed"
#  define TS_ANNOUNCE_SCALECHANGED			"Stream::0004 Scale Changed"
#  define TS_ANNOUNCE_ITEMSTEPPED			"Playlist::0001 Item Stepped"
#  define TS_ANNOUNCE_SESSIONTIMEOUT		"Session::0001 Session Timeout"
#  define TS_ANNOUNCE_STREAMEXIT			""
#  define TS_ANNOUNCE_PING					""
#endif // NUMERIC_CATEGORY
	
	typedef std::map<std::string, std::string> STRINGMAP;
	typedef STRINGMAP::iterator STRINGMAP_ITOR;
	typedef STRINGMAP::const_iterator STRINGMAP_CITOR;
	
	typedef std::vector<std::string> STRINGVECTOR;
	typedef STRINGVECTOR::iterator STRINGVECTOR_ITOR;
	typedef STRINGVECTOR::const_iterator STRINGVECTOR_CITOR;
	
	//////////////////////////////////////////////////////////////////////////
	// class CNtpTime
	//////////////////////////////////////////////////////////////////////////
	class CNtpTime
	{
	public:
		//Constructors / Destructors
		CNtpTime();
		CNtpTime(const SYS::TimeStamp& ts);
//		static CNtpTime GetCurrentTime();
		operator uint64() const 
		{
			return m_Time; 
		};

	protected: 
		static uint32 m_MsToNTP[1000];
		static long GetJulianDay(uint16 Year, uint16 Month, uint16 Day);
		static uint32 MsToNtpFraction(uint16 wMilliSeconds);

	private:
		//The actual data
		uint64 m_Time;
	};
	
	//////////////////////////////////////////////////////////////////////////
	// class Environment
	//////////////////////////////////////////////////////////////////////////	
	class Environment
	{
	public:
		
		Environment();
		virtual ~Environment();
		
		bool doInit(const char* cfgPath, IStreamSmithSite* pSite);
		void doUninit();
		
		RequestProcessResult doFixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq);
		RequestProcessResult doContentHandler(IStreamSmithSite* pSite, IClientRequestWriter* pReq);
		RequestProcessResult doFixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq);
		
		// connect services IceStorm.
		bool connectIceStorm();

		bool getWeiwooSessionPrx(TianShanIce::SRM::SessionPrx& srvrSessPrx, const std::string& srvrSessPrxID);
		bool getPurchasePrx(TianShanIce::Application::PurchasePrx& purchasePrx, const std::string& purchasePrxID);
		bool getPlaylistPrx(TianShanIce::Streamer::PlaylistPrx& playlistPrx, const std::string& streamPrxID);
		bool getStreamState(const TianShanIce::Streamer::PlaylistPrx& playlistPrx, const std::string& streamPrxID, TianShanIce::Streamer::StreamState& strmState, std::string& statDept);
		bool getStreamPlayInfo(const TianShanIce::Streamer::PlaylistPrx& playlist, const std::string& streamPrxID, std::string& scale, Ice::Int& iCurrentPos, Ice::Int& iTotalPos);
		bool getPlaylistPlayInfo(const TianShanIce::Streamer::PlaylistPrx& playlist, const std::string& streamPrxID, std::string& scale, Ice::Int& ctrlNum, Ice::Int& offset);
		bool PlayInfo2UtcTime(const TianShanIce ::Application ::PurchasePrx& purchasePrx, const std::string& purchasePrxID, const ::Ice::Int& ctrlNum, const ::Ice::Int& offset, std::string& utcTime);
		bool renewWeiwooSession(const TianShanIce::SRM::SessionPrx& srvrSessPrx, const std::string& srvrSessPrxID, const int64& rn_time);
		
		// session context manager
		bool openSessionCtx(const Ice::Identity& ident, SessionContextPrx& cltSessPrx, SessionData& cltSessCtx);
		bool removeSessionCtx(const Ice::Identity& ident, const std::string& reason);
		
		void setLogDir(const char* logDir);
	private:
		
		// configuration
		bool loadConfig();
		
		// ice run-time
		bool initIceRuntime();
		void uninitIceRuntime();
		
		// safe store
		bool openSafeStore();
		void closeSafeStore();
		
	public: 
		std::string										_serverHeader;
		
		ZQ::common::SysLog								_sysLog;
		ZQ::common::FileLog								_fileLog;
		ZQ::common::FileLog								_iceFileLog;
		TianShanIce::common::IceLogIPtr					_iceLogger;
		std::string										_logPath;
		std::string										_logDir;
		std::string										_cfgDir;
		
		IStreamSmithSite*								_pSite;
		
		TianShanIce::Events::EventChannelImpl::Ptr		_eventChannel;
		
		::Ice::CommunicatorPtr							_pCommunicator;
		ZQADAPTER_DECLTYPE								_pAdapter;
		::Freeze::ConnectionPtr							_pFreezeConnection;
		::Freeze::EvictorPtr							_pContextEvictor;
		ZQ::common::Mutex								_lockEvictor;
		FactoryPtr										_pFactory;
		StreamIdxPtr									_pStreamIdx;

		ConnectService*									_pConnectService;
		WatchDog*										_pWatchDog;
		
	}; // class Environment

	//////////////////////////////////////////////////////////////////////////
	// class SmartServerRequest
	//////////////////////////////////////////////////////////////////////////
	class SmartServerRequest
	{
	public: 
		SmartServerRequest(IServerRequest*& pServerRequest);
		virtual ~SmartServerRequest();

	protected: 
		IServerRequest*& _pServerRequest;
	};

} // namespace TianShanS1

#endif // __TianShanS1_Environment_H__

