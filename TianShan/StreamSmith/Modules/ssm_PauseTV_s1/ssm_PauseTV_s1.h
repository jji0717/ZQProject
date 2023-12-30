
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SSM_PAUSETV_S1_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SSM_PAUSETV_S1_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#ifndef _SSM_PAUSETV_S1_H_
#define _SSM_PAUSETV_S1_H_

#pragma warning(disable: 4049)
#pragma warning(disable: 4244)

#ifdef SSM_PAUSETV_S1_EXPORTS
#define SSM_PAUSETV_S1_API __declspec(dllexport)
#else
#define SSM_PAUSETV_S1_API __declspec(dllimport)
#endif

#include "stdafx.h"
#ifdef ZQCOMMON_DLL
    #include "ZQ_common_Conf.h"
    #include "Exception.h"
    #include "Locks.h"
#endif ZQCOMMON_DLL

#include	"urlstr.h"
#include	"TianShanIce.h"
#include	<IceUtil/IceUtil.h>
#include	<IceUtil/RecMutex.h>
#include	<Freeze/Freeze.h>
#include	"EventChannel.h"
#include	"tssrm.h"
#include	"TsStreamer.h"
#include	"ChannelOnDemand.h"
#include	<utility>
#include	<string>
#include	<map>
#include	"descCode.h"
#include	"objbase.h"
#include	"StreamSmithModule.h"
#include	"XMLPreference.h"
#include	"ConfigParser.h"
#include	"FileLog.h"
#include	"thrdConnService.h"
#include	"thrdCleanupSession.h"
#include	"Locks.h"
#include	"time.h"
#include	"StreamSmithAdmin.h"
#include	"ChannelOnDemandEx.h"
#pragma  comment(lib,"ole32.lib")
#pragma  comment(lib,"Advapi32.lib")

using namespace std;
using namespace ZQ::common;
using namespace TianShanIce::Events;
using namespace TianShanIce::Streamer;

// SERVER ANNOUCE CODE
#define		SC_ANNOUNCE_ENDOFSTREAM		"2101"	// End-of-Stream
#define		SC_ANNOUNCE_BEGINOFSTREAM	"2102"	// Beginning-of-Stream
#define		ZQ_ANNOUNCE_ENDOFITEM		"2103"	// End-of-Item
#define		ZQ_ANNOUNCE_SPEEDCHANGE		"2104"	// Scale Changed
#define		ZQ_ANNOUNCE_EXCEPTION_EXIT	"3001"	// Exception End
#define		ZQ_ANNOUNCE_STATE_CHANGED	"3002"	// State Changed

// DEFAULT VALUE OF CONFIGURATION
#define	DEFAULT_LOGFILE_NAME						"C:\\ssm_PauseTV_s1.log"
#define	DEFAULT_LOGFILE_SIZE						"100000000"	//	100MB

#define	DEFAULT_CHANNELONDEMANDAPP_NAME				"ChannelOnDemandApp"
#define	DEFAULT_CHANNELONDEMANDAPP_IP				"127.0.0.1"
#define	DEFAULT_CHANNELONDEMANDAPP_PORT				"9832"

#define	DEFAULT_STREAMSERVICE_NAME					"StreamSmith"
#define	DEFAULT_STREAMSERVICE_IP					"127.0.0.1"
#define	DEFAULT_STREAMSERVICE_PORT					"10000"

#define	DEFAULT_LISTEN_EVENT_PORT					"8048"
#define	DEFAULT_ICESTORM_IP							"127.0.0.1"
#define	DEFAULT_ICESTORM_PORT						"10001"

#define	DEFAULT_RECONNECT_INTERNAL					"10000"		//	microsecond
#define	DEFAULT_SESSION_TIME_OUT					"600"		//	second


// BUFFER SIZE
#define	MY_BUFFER_SIZE		2048

#define	SEPARATOR_LINE		"\n                   "

// the file path of ssm_PauseTV_s1.dll's configure file
#define PLUGIN_PATH "SOFTWARE\\SeaChange\\TianShan\\CurrentVersion\\Services\\RtspProxy"

#define	BEFORE_GET_STREAM_INFO	"Before Get Stream's Information"
#define	CANNOT_GET_STREAM_INFO	"Can't Get Stream's Information"
#define	SUCCESS_GET_STREAM_INFO	"Success Get Stream's Information"
#define	BEFORE_FIND_SESSION		"Before Find Session"
#define	CANNOT_FIND_SESSION		"Can't Find Session"
#define	SUCCESS_FIND_SESSION	"Success Find Session"

class CSsm_PauseTV_s1
{
public:
	CSsm_PauseTV_s1();
	~CSsm_PauseTV_s1();
	static void createCleanupSessionThrd();
	static void UpdateLastAccessTime(const ::std::string& strSession,const ::std::string& strMethod);
	static bool HdlSetupReq(IStreamSmithSite *pSite,IClientRequestWriter *pReq,::std::string &retSessionID,::std::string &retStreamString,::std::string &retPurchaseString);
	static bool HdlPlayReq(IStreamSmithSite *pSite,IClientRequestWriter *pReq,::std::string &retSessionID);
	static bool HdlPauseReq(IStreamSmithSite *pSite,IClientRequestWriter *pReq,::std::string &retSessionID);
	static bool HdlTeardownReq(IStreamSmithSite *pSite,IClientRequestWriter *pReq,::std::string &retSessionID);
	static bool HdlGetParameterReq(IStreamSmithSite *pSite,IClientRequestWriter *pReq,::std::string &retSessionID);
	static void ErrorResponse(IServerResponse* pResponse,const char* sHeader,const char* sVerbCode,const char* sSCNotice,const char* sSessionID);
	static RequestProcessResult FixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	static RequestProcessResult ContentHandle(IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	static RequestProcessResult FixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq);
	static void InitIce();
	static void ConnChodsvc();
	static void ConnStreamSmith();
	static bool LoadConfig(::std::string &sPath);
	static void ConnIceStorm();
	static void DestroyAllPurchase();
	static void DestroyPurchaseStream(::std::string sStream,::std::string sPurchase);
	//static void StoreStreamPurchase(const ::std::string resumePath);
	//static void LoadStreamPurchase(const ::std::string resumePath);
	static void ShowConfiguration();
	static void createConnServiceThrd();
	
	struct CLIENT_INFO
	{
		::std::string	stmString;	/// the string of stream proxy.
		::std::string	purString;	/// the string of purchase proxy.
		::std::string	site;			
		::std::string	path;			
		::std::string	channelId;
		time_t			lastAccessTime;
	};
    
public:
    typedef ::std::map<::std::string,CLIENT_INFO>	SESSION_TO_CLIENT_INFO_MAP;     /// map the ClientSession to client_info which stores some client's request information.
	typedef ::std::map<::std::string,::std::string> STREAM_TO_SESSION_MAP;          /// map the string stands for stream to ClientSession.
    typedef ::std::map<::std::string,::std::string> CONFIGURATION_MAP;   /// store configuration string.

public:
	static IceUtil::RecMutex						clientInfoMapMutex;
	static IceUtil::RecMutex						streamToSessionMapMutex;
	static SESSION_TO_CLIENT_INFO_MAP				clientInfoMap;
	static STREAM_TO_SESSION_MAP					streamToSessionMap;
	static CONFIGURATION_MAP						configMap;
    static ::std::vector<::std::string>				configString;
	
public:
	static ::Ice::CommunicatorPtr						ic;
    static ::ChannelOnDemand::ChannelOnDemandAppPrx		codApp;
    static ::TianShanIce::Streamer::StreamSmithAdminPrx	stmservice;

public:
	static bool										bModuleInit;
	static bool										iceInitialized;
	static bool										bConnChodsvcOK;
	static bool										bConnStreamSmithOK;
	static bool										bConnIceStormOK;
	static IStreamSmithSite*						globalSite;
	static ::std::string							_configFilePath;
	static thrdConnService*							pConnServiceThrd;
	static thrdCleanupSession*						pCleanSessionThrd;
	static ZQ::common::FileLog								ssm_PauseTV_s1_Log;
private:
	static Ice::ObjectAdapterPtr					_topicAdapter;
	static TianShanIce::Events::EventChannelImpl::Ptr _eventChannel;
};

void ModuleInit(IStreamSmithSite *pSite);
void ModuleUninit(IStreamSmithSite *pSite);


#endif /// _SSM_PAUSETV_S1_H_
