#ifndef __ZQTianShan_ERMICMDS_H__
#define __ZQTianShan_ERMICMDS_H__

#include "../common/TianShanDefines.h"
//xml parser header
#include "XMLPreferenceEx.h"

namespace ZQTianShan {
	namespace EdgeRM {

		class EdgeRMEnv;
		// -----------------------------
		// class ERMICmds
		// -----------------------------
		///
		class ClientCmds : protected ZQ::common::ThreadRequest
		{
		protected:
			/// constructor
			///@note no direct instantiation of ProvisionCmd is allowed
			ClientCmds(EdgeRMEnv& env);
			virtual ~ClientCmds();

		public:

			void execute(void) { start(); }

		protected: // impls of ThreadRequest

			virtual bool init(void){return true;};
			virtual int run(void) = 0;

			// no more overwrite-able
			void final(int retcode =0, bool bCancelled =false) { delete this; }

		protected:

			EdgeRMEnv&     _env;
		};

		// -----------------------------
		// class ERMISessSetupCmd
		// -----------------------------
		///
		class ERMISessSetupCmd : public ClientCmds
		{
		public:
			typedef struct _Message
			{
				std::string sessionType;//unicast or mutilcast;
				int64         bit_rate;
				std::string source_address;
				std::string destination;
				int         destination_port;
				std::string	multicast_address;
				int			rank;
			}Message;
			typedef std::vector<Message>Messages;

			typedef struct _TransportInfo
			{
				std::string qamName;
				std::string qam_Destination;
				std::string clabClientSessId;

				Messages     messages;

				bool         encryptSess;// turn or false;
				std::string  casId;
				std::string  clientMac;
				std::string  cciLevel;
				std::string  apsLevel;
				std::string  CIT;
				std::string  encryptESK;
			}TransportInfo;

		public:
			/// constructor
			///@note no direct instantiation of SessionCommand is allowed
			ERMISessSetupCmd(EdgeRMEnv& env, std::string& clabClientSessId);
			virtual ~ERMISessSetupCmd() {}

		protected: // impls of BaseCmd

			virtual int run(void);
		protected:
//			TransportInfo& _transPort;
			std::string _clabClientSessId;
		};

		// -----------------------------
		// class ERMISessTearDownCmd
		// -----------------------------
		class ERMISessTearDownCmd : public ClientCmds
		{
		public:
			/// constructor
			///@note no direct instantiation of SessionCommand is allowed
			ERMISessTearDownCmd(EdgeRMEnv& env, const std::string& clabClientSessId, const std::string& qamName, const std::string& sessionId);
			virtual ~ERMISessTearDownCmd() {}

		protected: // impls of BaseCmd

			virtual int run(void);
		protected:
			std::string _clabClientSessId;
			std::string _qamName;
			std::string _sessionId;
		};

		//////////////////////////////
		//class R6SessPreSetupCmd
		//////////////////////////////
		// this class is not needed
		/*
		class R6SessPreSetupCmd : public ClientCmds
		{
		public:
			typedef struct _Message
			{
				//type qam
				std::string		qamSessType;
				std::string		qamDestination;
				int64			bandwidth;
				std::string		qamName;
				std::string		qamClient;
				//type udp
				std::string		udpSessType;
				std::string		udpDestination;
				int32			clientPort;
				std::string		source;
				int32			serverPort;
				std::string		udpClient;
			}Message;

			typedef struct _TransportInfo
			{
				int8										provisionPort;
				Message										message;
				TianShanIce::EdgeResource::ProvisionPort	provisionPortMsg;
				std::string									onDemandSessionId;
			}TransportInfo;

		public:
			/// constructor
			///@note no direct instantiation of SessionCommand is allowed
			R6SessPreSetupCmd(EdgeRMEnv& env, std::string& clabClientSessId);
			virtual ~R6SessPreSetupCmd() {}

		protected: // impls of BaseCmd

			virtual int run(void);
		protected:
			//			TransportInfo& _transPort;
			std::string _clabClientSessId;
		};
		*/
		// -----------------------------
		// class R6SessSetupCmd
		// -----------------------------
		///
		class R6ProvPortCmd : public ClientCmds
		{
		public:
			typedef struct _Message
			{
				//type qam
				std::string		qamSessType;
				std::string		qamDestination;
				int64			bandwidth;
				std::string		qamName;
				std::string		qamClient;
				//type udp
				std::string		udpSessType;
				std::string		udpDestination;
				int32			clientPort;
				std::string		source;
				int32			serverPort;
				std::string		udpClient;
			}Message;

			typedef struct _TransportInfo
			{
				int8										provisionPort;
				Message										message;
				TianShanIce::EdgeResource::ProvisionPort	provisionPortMsg;
				std::string									onDemandSessionId;
			}TransportInfo;

		public:
			/// constructor
			///@note no direct instantiation of SessionCommand is allowed
			R6ProvPortCmd(EdgeRMEnv& env, std::string& clabClientSessId);
			virtual ~R6ProvPortCmd() {}

		protected: // impls of BaseCmd

			virtual int run(void);
		protected:
			//			TransportInfo& _transPort;
			std::string _clabClientSessId;
		};

		// -----------------------------
		// class R6SessStartChecking
		// -----------------------------
		class R6StartCheckCmd : public ClientCmds
		{
			friend class R6Client;
		public:
			typedef struct _Message
			{
				//type udp
				std::string		udpSessType;
				std::string		udpDestination;
				int32			clientPort;
				std::string		source;
				int32			serverPort;
				std::string		udpClient;
			}Message;

			typedef struct _TransportInfo
			{
				int											startChecking;
				Message										message;
				TianShanIce::EdgeResource::ProvisionPort	provisionPortMsg;
				std::string									onDemandSessionId;
			}TransportInfo;

		public:
			/// constructor
			///@note no direct instantiation of SessionCommand is allowed
			R6StartCheckCmd(EdgeRMEnv& env, std::string& clabClientSessId, std::string& sessionId, std::string& sourceIP, ZQ::common::tpport_t sourcePort);
			virtual ~R6StartCheckCmd() {}

		protected: // impls of BaseCmd
			virtual int run(void);
		protected:
			//			TransportInfo& _transPort;
			std::string _clabClientSessId;
			std::string _sessionId;
			std::string _sourceIP;
			ZQ::common::tpport_t _sourcePort;
		};

		// -----------------------------
		// class R6SessStopChecking
		// -----------------------------
		class R6StopCheckCmd : public ClientCmds
		{
		public:
			typedef struct _Message
			{
				//type udp
				std::string		udpSessType;
				std::string		udpDestination;
				int32			clientPort;
				std::string		source;
				int32			serverPort;
				std::string		udpClient;
			}Message;

			typedef struct _TransportInfo
			{
				int8										stopChecking;
				Message										message;
				TianShanIce::EdgeResource::ProvisionPort	provisionPortMsg;
				std::string									onDemandSessionId;
			}TransportInfo;

		public:
			/// constructor
			///@note no direct instantiation of SessionCommand is allowed
			R6StopCheckCmd(EdgeRMEnv& env, const std::string& clabClientSessId,const std::string& sessionID);
			virtual ~R6StopCheckCmd() {}

		protected: // impls of BaseCmd

			virtual int run(void);
		protected:
			//			TransportInfo& _transPort;
			std::string _clabClientSessId;
			std::string _sessionId;
		};


		// -----------------------------
		// class R6SessTearDownCmd
		// -----------------------------
		class  R6SessTearDownCmd : public ClientCmds
		{
		public:
			/// constructor
			///@note no direct instantiation of SessionCommand is allowed
			R6SessTearDownCmd(EdgeRMEnv& env, const std::string& _onDemandSessionId, const std::string& qamName, const std::string& sessionId,const std::string& reason);
			virtual ~R6SessTearDownCmd() {}

		protected: // impls of BaseCmd

			virtual int run(void);
		protected:
			std::string		_onDemandSessionId;
			std::string		_qamName;
			std::string		_sessionId;
			std::string		_reason;
		};
}
}
#endif //__ZQTianShan_ERMICMDS_H__
