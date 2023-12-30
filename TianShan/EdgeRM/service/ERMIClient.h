#ifndef  __ERMICLIENT_HEADER_FILE_H__
#define  __ERMICLIENT_HEADER_FILE_H__
#include "RTSPClient.h"
#include "TianShanUtils.h"
#include "definition.h"
//#include "EdgeRMEnv.h"

using namespace ZQ::common;

namespace ZQTianShan {
	namespace EdgeRM {

		class ERMISession;
		class ERMISessionGroup;	
        class EdgeRMEnv;
 		// -----------------------------
		// class ERMIClient
		// -----------------------------
		class ERMIClient : public ZQ::common::RTSPClient
		{
			friend class ERMISession;
			friend class ERMISessionGroup;
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

			ERMIClient(ERMISessionGroup& sessGroup, Log& log, NativeThreadPool& thrdpool, ZQ::common::InetHostAddress& bindAddress, const std::string& baseURL, const char* userAgent =NULL, ZQ::common::Log::loglevel_t verbosityLevel = ZQ::common::Log::L_WARNING, ZQ::common::tpport_t bindPort =0);
			virtual ~ERMIClient();

		public:

			int sendTousyMsg(const std::string& startLine, RTSPMessage::AttrMap& headers, const std::string& body=std::string(), const int cseq =-1);

		protected:	
			void OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
			virtual void OnServerRequest(RTSPClient& rtspClient, const char* cmdName, const char* url, RTSPMessage::Ptr& pInMessage);
			virtual void OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc=NULL);
			virtual void OnConnected();
			virtual void OnError();

		protected:
			ERMISessionGroup&	_sessGroup;
		};

		// -----------------------------
		// class ERMISession
		// -----------------------------
		class ERMISession :public ZQ::common::RTSPSession
		{
			friend class ERMIClient;
			friend class RTSPSession;
			friend class ERMISessionGroup;

		public:
			typedef Pointer < ERMISession > Ptr;
			typedef std::vector < Ptr > List;

		public:
			ERMISession(ERMISessionGroup& sessGroup, std::string ODSessId,  size_t idxClient = 0);
			virtual ~ERMISession();

			virtual void	destroy();
			std::string		getBaseURL();
			std::string		getSessionGroupName() const;

			std::string     getSessionId() { return _sessionId; };
			void			setSessionId(std::string sessionId);

			std::string     getOnDemandSessionId() { return _sessGuid; };
			int32			getTimeout() { return _sessTimeout; };
			ERMIClient*		getERMIClient();

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

			ERMISessionGroup& _sessGroup;

		public:
			int         _resultCode;
			int			_idxClient;
		};

		// -----------------------------
		// class ERMISessionGroup
		// -----------------------------
		class ERMISessionGroup : virtual public TianShanUtils::TimeoutObj
		{
			friend class ERMIClient;
			friend class ERMISession;

		public:

			typedef ::IceInternal::Handle< ERMISessionGroup > Ptr;

			typedef std::map<std::string, Ptr> SessionGroupMap; // map of groupName to SessionGroup

			typedef std::multimap<std::string, std::string> StringIndex;  
			typedef std::pair<StringIndex::iterator, StringIndex::iterator> StringIdxRange;  

			typedef std::map<std::string, ERMISession::Ptr> SessionMap; // Map of ODSess to session

			typedef enum _SessionGroupStatus
			{
				Sync = 1,
				Idle = 2
			} SessionGroupStatus;

			ERMISessionGroup(EdgeRMEnv& env, const std::string& name, const std::string& baseURL,int max =600, Ice::Long syncInterval =60000);
			virtual ~ERMISessionGroup();

			virtual int     sync(bool bOnConnected =false); // the response would be handled by OnSessionListOfSS()
			virtual void    OnSessionListOfSS(const std::vector<ERMIClient::Session_Pair>& listOnSS);


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

			virtual void add(ERMISession& sess);
			virtual void remove(ERMISession& sess);

			virtual ERMISession::Ptr lookupByOnDemandSessionId(const char* ODSessId);
			ERMISession::List lookupByIndex(const char* sessionId, const char* indexName);

			virtual void updateIndex(ERMISession& sess, const char* indexName="SessionId", const char* oldValue=NULL);
			void eraseSessionId(const std::string& sessionId, ERMISession& sess);

			ERMIClient* getERMIClient(size_t idxClient = 0); // to select one RTSPClient

			void clearERMIClient();


			static ERMISessionGroup::Ptr findSessionGroup(const std::string& name); // to find session group
			static std::vector<std::string> getAllSessionGroup(); // return all group name

			static void clearSessionGroup();
			static Ice::Long checkAll();

			static ERMISession::Ptr createSession(const std::string& ODSessId, const char* qamName);
			static ERMISession::Ptr openSession(const std::string& ODSessId, const std::string& groupName, bool bRestore =false);

		protected:

			EdgeRMEnv&          _env;

			std::string         _qamName;
			std::string         _baseURL;

			int                 _max;
			SessionGroupStatus  _status;
			::Ice::Long         _syncInterval;
			::Ice::Long         _stampLastSync;
		private:

			ERMIClient			_ERMIClient;

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

#endif ///end define __ERMICLIENT_HEADER_FILE_H__
