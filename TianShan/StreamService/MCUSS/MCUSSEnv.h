#ifndef		_MCUSS_ENV_H
#define		_MCUSS_ENV_H

#include "MCUSSCfg.h"
#include "TianShanIceHelper.h"
#include "IPathHelperObj.h"
#include "SsServiceImpl.h"
#include "Log.h"
#include "SsEnvironment.h"
#include "InetAddr.h"
#include "SystemUtils.h"

#define MAX_IDLE                    (60*60*1000) // 1hour
#define DEFAULT_IDLE                (5* 60*1000) // 5sec
#define	SPEED_LASTIDX				"MCUSS_MultiplySpeedLastIndex"
#define	SPEED_LASTDIR				"MCUSS_MultiplySpeedLastDirection"



namespace ZQTianShan{
	namespace MCUSS{
		// -----------------------------
		// class MCUSSEnv
		// -----------------------------
		class MCUSSEnv : public ZQ::StreamService::SsEnvironment
		{
		public:
			//constructor
			MCUSSEnv(ZQ::common::Log& mainLog, ZQ::common::Log& sessLog, ZQ::common::NativeThreadPool& rtspThpool, ZQ::common::NativeThreadPool& ssThpool);
			virtual ~MCUSSEnv();
			typedef struct _ca
			{
				std::string	name;
				int			stampAsOf;
			}CA;

		public:
			void init();
			void uninit();

			void start();

			bool isRunning() const { return _bRun; }

			void setNetId(std::string netId) { _netId = netId; };
			bool mcussLocateReady(const std::string& sessId);
			void setServiceName(const char* szServiceName) { _strServiceName = szServiceName; }

			void deleteSessFromMap(const std::string& sessId, const std::string& contentName);

		public:
			ZQ::common::FileLog   _logger;	
			std::string _netId, _userAgent;
			std::string			_strServiceName;

			ZQ::common::InetHostAddress _bindAddr;
			ZQ::common::Log::loglevel_t _rtspTraceLevel;
			ZQ::common::NativeThreadPool& _rtspThpool;

			//std::map < contentName, sessionIdList > _interestMap;
			typedef std::vector<std::string> SessionIDList;
			typedef std::map <std::string , SessionIDList >InterestMAP;
			ZQ::common::Mutex	_lkInterestMap;
			InterestMAP     _interestMap;

			//std::map < sessionId, CAList > _CAOfSessionMap;
			typedef std::vector <CA> CAList;
			typedef std::map <std::string , CAList> CAOFSessionMAP;
			ZQ::common::Mutex _lkCAOfSessionMap;
			CAOFSessionMAP	_CAOfSessionMap;

			typedef SYS::SingleObject Event; 
			Event _wakeup;


			int32  _sessTimeout; // in msec

		protected:

			//WatchDog* _watchDog;
			bool      _bRun;
		};

	}//namespace MCUSS
} //namespace ZQTianShan


#endif
