/// File Name : Environment.h

#ifndef __EVENT_IS_VODI5_ENVIROMENT_H__
#define __EVENT_IS_VODI5_ENVIROMENT_H__

// ICE include file
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>

// ZQ Common
#include "FileLog.h"
#include "NativeThread.h"
#include "NativeThreadPool.h"

// TianShan Commom
#include "EventChannel.h"
#include "TianShanDefines.h"

// RtspProxy
#include "StreamSmithModule.h"

// version infos
#include "ZQResource.h" 

#include "SystemUtils.h"

#define SESSION_META_DATA_STREAMER_NET_ID "StreamerNetID"
#define SESSION_META_DATA_STREAM_ID "StreamID"
#define SESSION_META_DATA_STB_CONNECTION_ID "STBConnectionID"
#define SESSION_META_DATA_REQUEST_URL "RequestURL"
#define SESSION_META_DATA_USED_BANDWIDTH "UsedBandwidth"
#define SESSION_META_DATA_IMPORT_CHANNEL_NAME "ImportChannelName"
#define SESSION_META_DATA_STREAMER_SOURCE "StreamerSource"


/// forward declare 
class SelectionEnv;
class NgodResourceManager;

namespace GBss
{

/// forward declare 
class CRGSessoionManager;
class StreamEventSinkI;

class Environment
{
public:
	Environment();

	~Environment();

public:
	/// set configuration path, fullPath = pConfigPath + "\\ssm_GBss.xml" 
	/// for example, C:\TianShan\etc\ssm_GBss.xml
	/// set log path, fullPath = pConfigPath + "\\ssm_GBss" for example, C:\TianShan\log\ssm_GBss
	/// initial environment, including open log file, load configuration, initial ICE Run Time,
	/// open freeze database, connect event channel, start session watch dog, sync with real streaming server
	int doInit(const char* pConfigPath, const char* pLogDir, IStreamSmithSite* pSite);

	/// fix up request
	RequestProcessResult doFixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq);

	/// handle request 
	RequestProcessResult doContentHandler(IStreamSmithSite* pSite, IClientRequestWriter* pReq);

	/// fix up response 
	RequestProcessResult doFixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq);

	/// uninitialized ICE Run Time, close freeze database, disconnect event channel, stop session watch dog
	/// stop sync with real streaming server
	int doUninit();

public:
	/// get global announce sequence
	std::string getAnnounceSequence();

	/// add (SRM-ID, SRM-Connection-ID) into a map
	void addIntoSRMMap(const std::string& SRMID, const std::string& SRMConnectionId);

	/// get SRM ConnectionIDs to send announce 
	void getSRMConnectionIDs(std::vector<std::string>& connections) const;

	/// called by setup handler to see if a connection have authorize to setup a request
	bool authorized(const std::string& srmConnID) const;

	/// get streamer source IP
	std::string getStreamerSource(const std::string& streamerNetId) const;

	/// get proxy string 
	std::string getProxyString(const Ice::ObjectPrx& objectPrx) const;

	std::string getUTCTime() const;

public:
	IStreamSmithSite& getStreamSmithSite();

	const std::string& getGlobalSession() const;

	NgodResourceManager& getResourceManager();

	SelectionEnv& getSelectionEnv();

	CRGSessoionManager& getSessionManager();

	TianShanIce::Streamer::StreamEventSinkPtr& getStreamEventSink();

	void watchSession(::Ice::Identity& ident, long timeout = 0);
	void watchSession(const std::string& sessId, long timeout = 0);

private:
	/// load configuration and adjust some configuration
	bool loadConfig(const char* pConfigPath);

	/// create ICE Communicator and Adapter
	bool initIceRunTime();

	/// Connect IceStorm
	bool ConnectIceStorm(const std::string& endpoint);

	/// initial streamer selection manager
	bool initResourceManager();

	/// destroy ICE Communicator and Adapter
	void uninitIceRunTime();

private:
	IStreamSmithSite* _site; 
	std::string _globalSession;
	CRGSessoionManager* _sessionManager;

private:
	ZQTianShan::TimerWatchDog * _sessionWatchDog;
	ZQ::common::NativeThreadPool _sessionWatchDogPool;
	TianShanIce::Streamer::StreamEventSinkPtr _sEvent;

private:
	SelectionEnv* _selectionEnv;
	NgodResourceManager* _resourceManager;

private:
	/// make sure to connect ice storm, call ConnectIceStorm(const std::string& endpoint)
	/// Inner class purpose : this class is accessed only Environment and it needs access private function
	///                       in Environment
	class IceStormClient : public ZQ::common::NativeThread
	{
	public:
		IceStormClient(Environment& env);

		~IceStormClient();

		void stop();

	protected:
		virtual bool init(void);

		virtual int run(void);

		virtual void final();

	private:
		bool _bQuit;
		::SYS::SingleObject _hEvent;
		Environment& _env;
	};

	IceStormClient* _iceStromClient;
	ZQ::common::SysLog _sysLog;
	ZQ::common::FileLog _fileLog;
	ZQ::common::FileLog _iceLog;

private:
	ZQADAPTER_DECLTYPE _pEventAdapter;
	::Ice::CommunicatorPtr _pCommunicator;
	TianShanIce::Events::EventChannelImpl::Ptr _pEventChannel;

private:
	bool _bInit;

	// announce message global sequence numbers
	uint32 _globalSequence;
	ZQ::common::Mutex _sequenceLock;

	// streamerNetId->streamer Source IP map 
	std::map<std::string, std::string> _IdToSourceMap;

	// SRM ID -> SRM Connection map : to see Connections have authorize to setup request
	ZQ::common::Mutex _SRMMapLock;
	std::map<std::string, std::string> _SRMMap;
};

} // end GBss

#endif // end __EVENT_IS_VODI5_ENVIROMENT_H__
