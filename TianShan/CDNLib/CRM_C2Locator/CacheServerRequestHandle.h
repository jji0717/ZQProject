#ifndef __C2_CacheServerRequestHandle_H__
#define __C2_CacheServerRequestHandle_H__
#include <ZQ_common_conf.h>
#include <CRMInterface.h>
#include <boost/regex.hpp>
#include <Locks.h>
#include  <C2Env.h>
#include <TsStreamer.h>
#include <TsSRM.h>
#include <urlstr.h>
#include <CDNDefines.h>

typedef std::vector<std::string> Strings;

namespace TianShanIce
{
	namespace SCS
	{
		class C2LocatorImpl;
	}
}

namespace ZQTianShan
{
	namespace CDN
	{	
		struct CacheDeleteRequestData
		{
			std::string clientTransfer;
			std::string transferIdDelete;
		};
		// define 
		struct CacheServerLocatorRequestData
		{
			std::string assetId;
			std::string providerId;
			std::string subType;
			std::string extension; // file extension of the member file
			Ice::Long transferRate;

			std::string clientTransfer;
			Ice::Long ingressCapacity;

			Strings exclusionList;
			std::string range;
			Ice::Long transferDelay;
			CacheServerLocatorRequestData() {
				transferRate = 0;
				ingressCapacity = 0;
				transferDelay = 0;
			}
		};


		typedef struct _CacheServerSession
		{
			std::string transferId;
			::TianShanIce::Streamer::StreamPrx stream;
			int64 stampCreated, stampLastTouch;
		} CacheServerSession;

		typedef std::map < std::string, CacheServerSession> CacheSessionMap;

		class C2Env;
		class ClientManager;
		class CacheDeleteRequestHandle;
		class CacheServerRequestHandle: public CRG::IContentHandler,public ZQ::common::NativeThread
		{
		public:
			CacheServerRequestHandle(C2Env& env, TianShanIce::SCS::C2LocatorImpl& locator);
			virtual ~CacheServerRequestHandle();
			virtual void onRequest(const CRG::IRequest* req, CRG::IResponse* resp);
			bool   stop();
		private:
			//
			void addSession(ZQTianShan::CDN::CacheServerSession& sess);

			//
			void deleteSession(const std::string& transferId);

			//
			bool resolveObject(const std::string& identifier, std::string& porviderId, std::string& assetId, std::string& extentsion) const;
			//
			void cache_locatorequest(const CRG::IRequest* prequest, CRG::IResponse* presponse);
			
			//
			void cache_deleterequest(const CRG::IRequest* prequest, CRG::IResponse* presponse);

			bool cache_forwardRequest(const CRG::IRequest* req, CRG::IResponse* resp, const std::string& reqId, SimpleXMLParser& parser, CacheServerLocatorRequestData reqData);

			//
		protected: // extended data structure for CacheServer
			bool  _bCQuit;
			ZQ::common::Semaphore _semaphore;
			CacheSessionMap _cacheSessMap;
			ZQ::common::Mutex _lkCacheSessMap;
			virtual int run();
			CacheSessionMap::iterator itorc;
			CacheServerRequestHandle* pcacheSThread;
			
		private:
			ZQ::common::Log& _log;
			C2Env& _env;
			TianShanIce::SCS::C2LocatorImpl& _locator;
			ClientManager& _clientMgr;
			struct ObjectResolver
			{
				std::string type;
				boost::regex matcher;
				std::string providerId;
				std::string assetId;
				std::string extension;
			};
			typedef std::vector<ObjectResolver> ObjectResolvers;
			ObjectResolvers _objResolvers;
			std::string     _forwardUrl;
		};

	}
} 

#endif
