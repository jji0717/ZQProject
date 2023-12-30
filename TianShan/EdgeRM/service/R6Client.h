#ifndef  __R6CLIENT_HEADER_FILE_H__
#define  __R6CLIENT_HEADER_FILE_H__
#include "RTSPClient.h"
#include "TianShanUtils.h"
#include "definition.h"
//#include "EdgeRMEnv.h"

#define LASTREQUEST_PROPORT		"provisonPort"
#define LASTREQUEST_STOPCHECK	"stopChecking"
#define LASTREQUEST_NULL		""
using namespace ZQ::common;

namespace ZQTianShan {
	namespace EdgeRM {

		class R6Session;
		class R6SessionGroup;	
        class EdgeRMEnv;
 		// -----------------------------
		// class R6Client
		// -----------------------------
		class R6Client : public ZQ::common::RTSPClient
		{
			friend class R6Session;
			friend class R6SessionGroup;
		public:

			typedef struct Session_Pair
			{
				std::string _sessionId;         ///< rtsp-session-id
				std::string _onClientSessionId; ///< on-demand-session-id
			} Session_Pair;

			class FindByClientSessionID
			{
			public:
				FindByClientSessionID(std::string& strClientSessionID):m_strClientSessionID(strClientSessionID){}

				bool operator() (const Session_Pair& pSession_list)
				{
					return (0 == m_strClientSessionID.compare(pSession_list._onClientSessionId));
				}

			private:

				const std::string& m_strClientSessionID;
			};

		public:

			R6Client(R6SessionGroup& sessGroup, Log& log, NativeThreadPool& thrdpool, ZQ::common::InetHostAddress& bindAddress, const std::string& baseURL, const char* userAgent =NULL, ZQ::common::Log::loglevel_t verbosityLevel = ZQ::common::Log::L_WARNING, ZQ::common::tpport_t bindPort =0);
			virtual ~R6Client();

		public:

			int  sendTousyMsg(const std::string& startLine, RTSPMessage::AttrMap& headers, const std::string& body=std::string(), const int cseq =-1);

		protected:	
			void OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
			virtual void OnServerRequest(RTSPClient& rtspClient, const char* cmdName, const char* url, RTSPMessage::Ptr& pInMessage);
			virtual void OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc=NULL);
			virtual void OnConnected();
			virtual void OnError();

		protected:
			R6SessionGroup&	_sessGroup;
		};

		// -----------------------------
		// class R6Session
		// -----------------------------
		class R6Session :public ZQ::common::RTSPSession
		{
			friend class R6Client;
			friend class RTSPSession;
			friend class R6SessionGroup;

		public:
			typedef Pointer < R6Session > Ptr;
			typedef std::vector < Ptr > List;

		public:
			R6Session(R6SessionGroup& sessGroup, std::string ODSessId,  size_t idxClient = 0);
			virtual ~R6Session();

			virtual void	destroy();
			std::string		getBaseURL();
			std::string		getSessionGroupName() const;

			std::string     getSessionId() { return _sessionId; };
			void			setSessionId(std::string sessionId);

			std::string     getOnDemandSessionId() { return _sessGuid; };
			int32			getTimeout() { return _sessTimeout; };
			R6Client*		getR6Client();

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

			R6SessionGroup& _sessGroup;
		public:
			int				_resultCode;
			int				_idxClient;
			std::string		_lastRequest;
			bool			_bProPortDone;
		};

		// -----------------------------
		// class R6SessionGroup
		// -----------------------------
		class R6SessionGroup : virtual public TianShanUtils::TimeoutObj
		{
			friend class R6Client;
			friend class R6Session;

		public:

			typedef ::IceInternal::Handle< R6SessionGroup > Ptr;

			typedef std::map<std::string, Ptr> SessionGroupMap; // map of groupName to SessionGroup

			typedef std::multimap<std::string, std::string> StringIndex;  
			typedef std::pair<StringIndex::iterator, StringIndex::iterator> StringIdxRange;  

			typedef std::map<std::string, R6Session::Ptr> SessionMap; // Map of ODSess to session

			typedef enum _SessionGroupStatus
			{
				Sync = 1,
				Idle = 2
			} SessionGroupStatus;

			R6SessionGroup(EdgeRMEnv& env, const std::string& name, const std::string& baseURL,int max =600, Ice::Long syncInterval =60000);
			virtual ~R6SessionGroup();

			virtual int     sync(bool bOnConnected =false); // the response would be handled by OnSessionListOfSS()
			virtual void    OnSessionListOfSS(const std::vector<R6Client::Session_Pair>& listOnSS);


			size_t			size() { return _sessMap.size(); }
			int				getMaxSize() { return _max; };
			void			setMaxSize(int max) { _max = max; }

			SessionGroupStatus	getStatus() { return _status; }
			void            setStatus(SessionGroupStatus st) { _status = st; }

			std::string     getName() const { return _qamName; }
			std::string     getBaseURL() const { return _baseURL; }

			Ice::Long		getSyncInterval() const { return _syncInterval; }
			Ice::Long		getLastSync() { MutexGuard g(_lockSyncTimeStamp); return _stampLastSync; }
			void			setLastSync(Ice::Long time, SessionGroupStatus st) { MutexGuard g(_lockSyncTimeStamp); _stampLastSync = time; _status = st; }

			virtual void OnTimer(const ::Ice::Current& = ::Ice::Current());

			virtual void add(R6Session& sess);
			virtual void remove(R6Session& sess);

			virtual R6Session::Ptr lookupByOnDemandSessionId(const char* ODSessId);
			R6Session::List lookupByIndex(const char* sessionId, const char* indexName);

			virtual void updateIndex(R6Session& sess, const char* indexName="SessionId", const char* oldValue=NULL);
			void eraseSessionId(const std::string& sessionId, R6Session& sess);

			R6Client* getR6Client(size_t idxClient = 0); // to select one RTSPClient

			void clearR6Client();


			static R6SessionGroup::Ptr findSessionGroup(const std::string& name); // to find session group
			static std::vector<std::string> getAllSessionGroup(); // return all group name

			static void clearSessionGroup();
			static Ice::Long checkAll();

			static R6Session::Ptr createSession(const std::string& ODSessId, const char* qamName);
			static R6Session::Ptr openSession(const std::string& ODSessId, const std::string& groupName, bool bRestore =false);

		protected:

			EdgeRMEnv&          _env;

			std::string         _qamName;
			std::string         _baseURL;

			int                 _max;
			SessionGroupStatus  _status;
			::Ice::Long         _syncInterval;
			::Ice::Long         _stampLastSync;
		private:

			R6Client			_R6Client;

			friend class EdgeRMEnv;
			static SessionGroupMap	_groupMap;
			static StringIndex   	_idxGroupBaseUrl;

			static Mutex			_lockGroups;

			StringIndex  			_sessIdIndex; // S6 sessionId to OnDemandSessionId

			SessionMap				_sessMap;     // store all sessions of group
			Mutex					_lockSessMap; // map of OnDemandSessionId to NGODSession of the group

			Mutex					_lockSyncTimeStamp;

		};

	} // end namespace EdgeRM
}// end namespace ZQTianShan

#endif ///end define __R6CLIENT_HEADER_FILE_H__
