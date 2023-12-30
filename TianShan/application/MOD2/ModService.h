#ifndef __ZQMODApplication_ModService_H__
#define __ZQMODApplication_ModService_H__

#include "ZQ_common_conf.h"

#ifdef ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#else
#include "ZQDaemon.h"
#endif
#include <Ice/Ice.h>
#include "TianShanDefines.h"
#include "ModSvcIceImpl.h"
#include "Stream2Purchase.h"
#include "ote.h"
#include "LAMFacade.h"
#include "MODMRTProxy.h"
#include "../../StreamSmith/RtspRelevant.h"
#include "urlstr.h"
#include "WatchDog.h"
#include "MODHelperMgr.h"

namespace ZQMODApplication
{

#define Servant_ModPurchaseItem "ModPurItem"
#define Servant_ModPurchase "ModPur"
#define ModAppServant "ModApp"
#define INDEXFILENAME(_X) #_X "Idx"
#define SerfAppPath "60010003" // special application path for surf
#define PurchaseRenderUse "RenderUse#"
//#define DEFAULTAPPPATH "DefaultAppPath"
#define LAMENDPOINT "LAMEndPoint"
#define OTEENDPOINT "OTEEndPoint"
#define ADMENDPOINT "ADMEndpoint"
#define ALENDPOINT  "ALEndpoint"
#define APSENDPOINT  "APSEndpoint"
#define MinRenewValue 120000 // 120s

#define AquaStorageLibEntry		"NameFormatter"
#define AENAMEFORMAT			"aeNameFormat"
#define MRTStreamEntry			"MRTStreamEntry"
#define IPTVEntry				"IPTVEntry"


void dumpLine(const char* line, void* pCtx = NULL);

typedef std::map<std::string, std::string> StringMap;
typedef StringMap::iterator StringMapItor;
		
//////////////////////////////////////////////////////////////////////////
// a useful class used to operate string
//////////////////////////////////////////////////////////////////////////
class String
{
public:

	static std::string leftStr(const std::string& cstStr, int pos);

	// Get the left string which behinds the char lies in the string "splitStr"
	// the param first indicates the first occurance or the last occurance
	static std::string getLeftStr(const std::string& cstStr, const std::string& splitStr, bool first = true);

	static std::string rightStr(const std::string& cstStr, int pos);

	// Get the right string which behinds the char lies in the string "splitStr"
	// the param first indicates the first occurance or the last occurance
	static std::string getRightStr(const std::string& cstStr, const std::string& splitStr, bool first = true);

	// Functionality: Get the path of a filename
	// Example: if the filename is "E:\TianShan\xxx.doc", then it will return "E:\TianShan"
	static std::string getPath(const std::string& cstStr);

	// Get the string lies between f_pos and l_pos in the string "cstStr"
	// Notice the chars lie at f_pos and l_pos are not included
	static std::string midStr(const std::string& cstStr, int f_pos, int l_pos);

	// Split the string "cstStr" into a lot of string, and storing them into a string vector strVect
	static void splitStr(const std::string& cstStr, const std::string split, std::vector<std::string>& strVect);

	// Replace the char 'from' to the char 'to'
	static std::string replaceChar(std::string& Str, const char& from, const char& to);

	// Replace all the chars lies in string from to the char 'to'
	static std::string replaceChars(std::string& Str, const std::string& from, const char& to);

	// Replace the string "from" to string "to"
	static std::string replaceStr(std::string& Str, const std::string& from, const std::string& to);

	// Stores the first position of the any char lies in string Str to pos
	static bool hasChars(const std::string& Str, const std::string& idetifier, int& pos);

	// Stores the first position of the char 'ch' in the string Str to pos
	static bool hasChar(const std::string& Str, const char& ch, int& pos);

	// Stores the position of the char 'ch' in the string Str to vector poss
	static bool hasChar(const std::string& Str, const char& ch, std::vector<int>& poss);

	// Remove the chars lies in idetifier from the string Str
	static void removeChars(std::string& Str, const std::string& idetifier);

	// Remove the char 'ch' from the string Str
	static void removeChar(std::string& Str, const char& ch);

	// Get the left num chars of the string cstStr
	static std::string nLeftStr(const std::string& cstStr, int num);

	// Get the right num chars of the string cstStr
	static std::string nRightStr(const std::string& cstStr, int num);

	static void trimRight(std::string& Str);

	static void trimLeft(std::string& Str);

	static void trimAll(std::string& Str);

	static std::string getTrimRight(const std::string& Str);

	static std::string getTrimLeft(const std::string& Str);

	static std::string getTrimAll(const std::string& Str);
};


//////////////////////////////////////////////////////////////////////////
// implemete lce log interface
//////////////////////////////////////////////////////////////////////////
class ModIceLog : public Ice::Logger
{
public: 
	ModIceLog(ZQ::common::Log& log) : _logger(log)
	{
	}
	~ModIceLog()
	{
	}
	void print(const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_DEBUG, message.c_str());
	}
	void trace(const ::std::string& category, const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_DEBUG, "catagory %s,message %s", category.c_str(), message.c_str());
	}
	void warning(const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_WARNING, message.c_str());
	}
	void error(const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_ERROR, message.c_str());
	}
	virtual ::std::string getPrefix(){return "";};
	virtual ::Ice::LoggerPtr cloneWithPrefix(const ::std::string&) { return NULL;}

private: 
	ZQ::common::Log& _logger;

}; // class ModIceLog


//////////////////////////////////////////////////////////////////////////
// Object factory for creating servants used by MOD service
//////////////////////////////////////////////////////////////////////////
class ModFactory: public Ice::ObjectFactory
{
public: 
	ModFactory(ModEnv& env) : _env(env)
	{
	}
	virtual ~ModFactory()
	{
	}

public: 
	Ice::ObjectPtr create(const std::string& strID)
	{
		if(strID == ZQTianShan::Application::MOD::ModPurchase::ice_staticId())
		{
			try
			{
				return new ZQMODApplication::ModPurchaseImpl(_env);
			}
			catch (...)
			{
				return NULL;
			}
		}
		return NULL;
	}
	void destroy()
	{
	}

private: 
	ModEnv& _env;
}; // class ModEnv
typedef IceInternal::Handle<ModFactory>	ModFactoryPtr;


//////////////////////////////////////////////////////////////////////////
// class ModApplication used to create purchase
//////////////////////////////////////////////////////////////////////////
class ModApplication : public TianShanIce::Application::AppService
{
public: 
	ModApplication(ModEnv& env);
	virtual ~ModApplication();

	::TianShanIce::Application::PurchasePrx createPurchase(const ::TianShanIce::SRM::SessionPrx& weiwooSessPrx, 
															const ::TianShanIce::Properties& siteProperties, 
															const ::Ice::Current& = ::Ice::Current());
	::std::string getAdminUri(const ::Ice::Current& = ::Ice::Current());
    ::TianShanIce::State getState(const ::Ice::Current& = ::Ice::Current());

private: 
	ModEnv& _env;

}; // class ModApplication
typedef IceUtil::Handle<ModApplication> ModApplicationPtr;


//////////////////////////////////////////////////////////////////////////
// A resource manager of ModService, such as db, ice resource etc.
//////////////////////////////////////////////////////////////////////////
class ServiceGroupPump;
class ModEnv
{
public: 
	ModEnv();
	virtual ~ModEnv();

	bool doInit(const char * logfolder, std::string InstanceID);
	void doUninit();

protected: 
	bool initIceRuntime();
	void uninitIceRuntime();
	bool openSafeStore();
	void closeSafeStore();
	bool loadConfig(const std::string& cfgPath);
	void setCrushDump();
	bool loadMho();

public: 
	// ice communicator and adapter
	Ice::CommunicatorPtr _iceComm;
	ZQADAPTER_DECLTYPE _iceAdap;

	Freeze::ConnectionPtr _frzConn; // freeze connection
	ZQTianShan::Application::MOD::Stream2PurchasePtr _stream2Purchase; // 
	Freeze::EvictorPtr _evctPurchase; // freeze evictor used to store purchase
	Freeze::EvictorPtr _evctPurItem; // freeze evictor used to store purchase item
//	ZQ::common::Mutex _lockPurchase; // lock purchase evictor
//	ZQ::common::Mutex _lockPurItem; // lock purchase item evictor
	ModFactoryPtr _objFactory;

	// log for ice itself
	ZQ::common::FileLog* _pIceLog;
	Ice::LoggerPtr _logForIce;

	ModApplicationPtr _modApp;

	WatchDog* _pWatchDog;
	ServiceGroupPump* _serviceGroupPump;
	
	std::string _logFolder;
	
	std::string				_programRootPath;
	ZQTianShan::Application::MOD::MODAppHelperMgr		_MHOHelperMgr;
	std::string m_InstanceID;

	std::string _configFilePath;

    int _IceTimeout;
protected:
	TianShanIce::Streamer::StreamServicePtr _service;
	Ice::ObjectAdapterPtr  _mrtAdapter;
	ReplicaUpdater*        _replicaUpdater;
}; // class ModEnv


//////////////////////////////////////////////////////////////////////////
// the ModService enter
//////////////////////////////////////////////////////////////////////////
class ModService : public ZQ::common::BaseZQServiceApplication
{
public: 
	ModService();
	virtual ~ModService();

protected: 
	HRESULT OnInit();
	HRESULT OnStart();
	HRESULT OnStop();
	HRESULT OnUnInit();
    void OnSnmpSet(const char *varName);
private: 
	ModEnv _env;

}; //class ModService

} // namespace ZQMODApplication

#endif // #define __ZQMODApplication_ModService_H__

