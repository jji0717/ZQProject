#ifndef __CRG_A3SERVER_A3MSGENV_HANDLER_H__
#define __CRG_A3SERVER_A3MSGENV_HANDLER_H__

#include "TianShanDefines.h"
#include "FileLog.h"
#include "TsStorage.h"
#include "IceLog.h"
#include "TsRepository.h"
#include "A3MessageHandler.h"
#include <NativeThread.h>
#include "EventChannel.h"
#include "SystemUtils.h"
#include "A3Client.h"
#include "LRUMap.h"
#include "A3ContentProvisionWrapper.h"
#ifdef ZQ_OS_LINUX
#ifdef max
#undef max
#endif
#endif
#include "CdmiFuseOps.h"  
#include "AquaUpdate.h"

namespace CRM
{
	namespace A3Message
	{
		class A3MessageHandler;
		class A3Client;
		class ConnectEventChannelThread;

		typedef struct 
		{
			std::string responseUrl;
			std::string pid;
			std::string paid;
			std::string sessionCPEProxy;
		}ContentInfo;

		class A3MsgEnv
		{
		public:
			A3MsgEnv(void);
			~A3MsgEnv(void);

		public:
			bool doInit(const std::string& strLogPath, const std::string& strCfgPath);

			/// this method must be called no matter if doInit() success or fail
			void doUninit();

            TianShanIce::Repository::ContentLibPrx connectToContentLib();
			TianShanIce::Storage::CacheStorePrx connectToCacheStore();

			bool connectToEventChannel();
			void setCRMmanager(CRG::ICRMManager* mgr){_mgr = mgr;};

		public:
			CRM::A3Message::A3MessageHandler* _A3MsgHandler;
			CRM::A3Message::A3Client* _A3Client;
			Ice::CommunicatorPtr _communicator;

			TianShanIce::Events::EventChannelImpl::Ptr		_eventChannel;

			bool _bUseContentLib;
			BackStoreType _backStoreType;
			ZQ::common::FileLog _log;

			ContentProvisionWrapper::Ptr  _cpWrapper;

			CdmiFuseOps* _pCdmiFuse;
			ZQ::common::NativeThreadPool _thrdPool;

			A3AquaContentMetadata::Ptr	_aquaContentMdata;
			A3CPESessionMgr::Ptr		_cpeSessionMgr;
			bool						_bUpdate; //updata AquaMetadata and Mainfile Metadata;


		private: 
			ZQ::common::Log* _pIceLog ;
			TianShanIce::common::IceLogIPtr	_iceLog ;
			TianShanIce::Repository::ContentLibPrx _clPrx;
			TianShanIce::Storage::CacheStorePrx    _csPrx;

			::Ice::ObjectAdapterPtr					_evtAdap;

            CRM::A3Message::ConnectEventChannelThread* _pConnectEventChannel; 

			CRG::ICRMManager* _mgr;

			ZQ::common::Mutex _lockContentInfos;

		public:
			bool addContentInfo(const std::string& contentName, ContentInfo& contentInfo);
			bool removeContentInfo(const std::string& contentName);
			ContentInfo getContentInfo(const std::string& contentName);

		private:
			bool loadConfig(const std::string& strCfgPath);
			void initWithConfig(Ice::PropertiesPtr proper );
		};

		class ConnectEventChannelThread : public ZQ::common::NativeThread
		{
		public: 
			ConnectEventChannelThread(CRM::A3Message::A3MsgEnv& env);
			virtual ~ConnectEventChannelThread();
            void toExit(void);
		protected: //
			virtual bool init();
			virtual int run(void);

		protected: 
			CRM::A3Message::A3MsgEnv&		_env;
			SYS::SingleObject _event;
			bool			_bExit;

		}; // class ConnectIceStrom

#define envlog			(_env._log)
	}///end namespace A3Message
}///end namespace CRM
#endif /// enddefine __CRG_A3SERVER_A3MSGENV_HANDLER_H__
