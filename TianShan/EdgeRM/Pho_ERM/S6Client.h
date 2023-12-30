#ifndef __ZQS6ClIENT_H__
#define __ZQS6ClIENT_H__

#include <NativeThread.h>
#include <NativeThreadPool.h>
#include <Ice/Identity.h>
#include "SystemUtils.h"
#include "RTSPClient.h"
#include  "Pointer.h"
#include  "TianShanUtils.h"

using namespace ZQ::common;

namespace ZQTianShan {
	namespace EdgeRM {

		
        class S6Session;
		class S6SessionGroup;

		class PhoEdgeRMEnv;
		// -----------------------------
		// class S6Client
		// -----------------------------
		class S6Client : public ZQ::common::RTSPClient
		{
			friend class S6Session;
			friend class S6SessionGroup;
		public:
			class Event : public SYS::SingleObject, virtual public ZQ::common::SharedObject
			{
			public: 
				typedef ZQ::common::Pointer < Event > Ptr;
			};

			typedef struct Session_Pair
			{
				std::string _sessionId;         ///< rtsp-session-id
				std::string _onDemandSessionId; ///< on-demand-session-id
			} Session_Pair;

			class FindByOnDemandSessionID
			{
			public:
				FindByOnDemandSessionID(std::string& strOnDemandSessionID):m_strOnDemandSessionID(strOnDemandSessionID){}

				bool operator() (const Session_Pair& pSession_list)
				{
					return (0 == m_strOnDemandSessionID.compare(pSession_list._onDemandSessionId));
				}

			private:

				const std::string& m_strOnDemandSessionID;
			};

		public:

			S6Client(S6SessionGroup& sessGroup, Log& log, NativeThreadPool& thrdpool, ZQ::common::InetHostAddress& bindAddress, const std::string& baseURL, const char* userAgent =NULL, ZQ::common::Log::loglevel_t verbosityLevel = ZQ::common::Log::L_WARNING, ZQ::common::tpport_t bindPort =0);
			virtual ~S6Client();

		public:
			bool    waitForResponse(uint32 cseq);

			int sendTousyMsg(const std::string& startLine, RTSPMessage::AttrMap& headers, const std::string& body=std::string(), const int cseq =-1);

		protected:	
			void OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
			virtual void OnServerRequest(RTSPClient& rtspClient, const char* cmdName, const char* url, RTSPMessage::Ptr& pInMessage);
			virtual void OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc=NULL);
			virtual void OnConnected();
			virtual void OnError();

			virtual int  OnRequestPrepare(RTSPRequest::Ptr& pReq);
			virtual void OnRequestClean(RTSPRequest& req);

			void    wakeupByCSeq(uint32 cseq);

			typedef std::map<uint32, Event::Ptr> EventMap;
			EventMap          _eventMap;
			ZQ::common::Mutex _lkEventMap;
            
			S6SessionGroup& _sessGroup;

			int        _cContinuousTimeout;
		};
	
		// -----------------------------
		// class S6Session
		// -----------------------------
		class S6Session :public ZQ::common::RTSPSession //, public PhoAllocationImpl
		{
			friend class S6Client;
			friend class RTSPSession;
			friend class S6SessionGroup;

		public:
			typedef Pointer < S6Session > Ptr;
			typedef std::vector < Ptr > List;

			typedef struct _QamInfo
			{
				Ice::Long channelId;
				Ice::Int symbolRate;
				Ice::Byte modulationFormat;
				Ice::Long PnId;
				std::string edgeDeviceIP;
				Ice::Int destPort;
				std::string edgeDeviceMac;
				std::string client;
				std::string edgeDeviceZone;
				std::string edgeDeviceName;
				std::string qam_group;
				std::string qam_input_group;
			}QamInfo;

		public:
			S6Session(S6SessionGroup& sessGroup, std::string ODSessId);
			virtual ~S6Session();


			virtual void	destroy();
			std::string		getBaseURL();
			std::string		getSessionGroupName() const;

			std::string     getSessionId() { return _sessionId; };
			void			setSessionId(std::string sessionId);

			std::string     getOnDemandSessionId() { return _sessGuid; };
			int32			getTimeout() { return _sessTimeout; };
			S6Client*		getS6Client();

			S6Session::QamInfo&  getQamInfo(){ return _qamInfo;};

        protected:
			virtual void OnResponse_SETUP(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
			virtual void OnResponse_TEARDOWN(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
			virtual void OnResponse_SET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
			virtual void OnResponse_GET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
			virtual void OnANNOUNCE(RTSPClient& rtspClient, RTSPMessage::Ptr& pInMessage);
			virtual void OnSessionTimer();

			virtual void OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
			virtual void OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc=NULL);

		protected:

			S6SessionGroup& _sessGroup;

			QamInfo        _qamInfo;
		public:
		    int                           _resultCode;
			std::string                   _errorMsg;
		};
		// -----------------------------
		// class S6SessionGroup
		// -----------------------------
		class S6SessionGroup : virtual public TianShanUtils::TimeoutObj
		{
			friend class S6Client;
			friend class S6Session;

		public:

			typedef ::IceInternal::Handle< S6SessionGroup > Ptr;

			typedef std::map<std::string, Ptr> SessionGroupMap; // map of groupName to SessionGroup

			typedef std::multimap<std::string, std::string> StringIndex;  
			typedef std::pair<StringIndex::iterator, StringIndex::iterator> StringIdxRange;  

			typedef std::map<std::string, S6Session::Ptr> SessionMap; // Map of ODSess to session

			typedef enum _SessionGroupStatus
			{
				Sync = 1,
				Idle = 2
			} SessionGroupStatus;

			S6SessionGroup(PhoEdgeRMEnv& env, const std::string& name, const std::string& baseURL, int max =600, Ice::Long syncInterval =60000);
			virtual ~S6SessionGroup();

			virtual int     sync(bool bOnConnected =false); // the response would be handled by OnSessionListOfSS()
			virtual void    OnSessionListOfSS(const std::vector<S6Client::Session_Pair>& listOnSS);


			size_t			size() { return _sessMap.size(); }
			int				getMaxSize() { return _max; };
			void			setMaxSize(int max) { _max = max; }

			SessionGroupStatus	getStatus() { return _status; }
			void            setStatus(SessionGroupStatus st) { _status = st; }

			std::string     getName() const { return _name; }
			std::string     getBaseURL() const { return _baseURL; }
//            std::string     getStreamLinkId() const { return _streamLinkId; };

			Ice::Long		getSyncInterval() const { return _syncInterval; }
			Ice::Long		getLastSync() { MutexGuard g(_lockSyncTimeStamp); return _stampLastSync; }
			void			setLastSync(Ice::Long time, SessionGroupStatus st) { MutexGuard g(_lockSyncTimeStamp); _stampLastSync = time; _status = st; }

			virtual void OnTimer(const ::Ice::Current& = ::Ice::Current());

			virtual void add(S6Session& sess);
			virtual void remove(S6Session& sess);

			virtual S6Session::Ptr lookupByOnDemandSessionId(const char* ODSessId);
			S6Session::List lookupByIndex(const char* sessionId, const char* indexName);

			virtual void updateIndex(S6Session& sess, const char* indexName="SessionId", const char* oldValue=NULL);
			void eraseSessionId(const std::string& sessionId, S6Session& sess);

			S6Client* getS6Client(); // to select one RTSPClient

			static S6SessionGroup::Ptr findSessionGroup(const std::string& name); // to find session group
			static std::vector<std::string> getAllSessionGroup(); // return all group name

			static std::vector<std::string> getSessionGroupNames(const std::string baseURL);
//			static std::vector<std::string> getSessionGroupNames(const std::string& streamLinkId);

			static void clearSessionGroup();
			static Ice::Long checkAll();

			static S6Session::Ptr createSession(const std::string& ODSessId, const char* baseURL, const std::string& streamLinkId);
			static S6Session::Ptr openSession(const std::string& ODSessId, const std::string& groupName, bool bRestore =false);

		protected:

			PhoEdgeRMEnv&      _env;

			std::string  _name;
			std::string  _baseURL;
//			std::string  _streamLinkId;

			int                 _max;
			SessionGroupStatus  _status;
			::Ice::Long         _syncInterval;
			::Ice::Long         _stampLastSync;
		private:

			S6Client				_S6Client;

			friend class PhoEdgeRMEnv;

			static SessionGroupMap	_groupMap;
			static StringIndex   	_idxGroupBaseUrl;
//			static StringIndex   	_idxGroupStreamLinkId;

			static Mutex			_lockGroups;

			StringIndex  			_sessIdIndex; // S6 sessionId to OnDemandSessionId

			SessionMap				_sessMap;     // store all sessions of group
			Mutex					_lockSessMap; // map of OnDemandSessionId to NGODSession of the group

			Mutex					_lockSyncTimeStamp;

		};

} // end namespace EdgeRM
}// end namespace ZQTianShan

#endif //__ZQS6ClIENT_H__
