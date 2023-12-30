#ifndef __CRG_A3SERVER_A3MESSAGEEVENTSINK_HANDLER_H__
#define __CRG_A3SERVER_A3MESSAGEEVENTSINK_HANDLER_H__

#include "A3MsgEnv.h"
#include "TsEvents.h"
#include  <EventChannel.h>

namespace CRM
{
	namespace A3Message
	{
		class A3MsgEnv;
		class A3MessageEventSinkI: public TianShanIce::Events::GenericEventSink
		{
		public:
			A3MessageEventSinkI(CRM::A3Message::A3MsgEnv& env);
			~A3MessageEventSinkI(void);
		public:
			void ping(Ice::Long timestamp, const Ice::Current &cur);

			void post(const ::std::string& category, ::Ice::Int eventId, const ::std::string& eventName, 
				const ::std::string& stampUTC, const ::std::string& sourceNetId, 
				const ::TianShanIce::Properties& params, const ::Ice::Current& cur);
		protected:
			void sendTransferStatus(const std::string& strPID, 
				const std::string& strPAID, 
				const std::string& strNetId,
				const std::string& strVolume, 
				const std::string& contentState);
		protected:
			CRM::A3Message::A3MsgEnv& _env;
		};
	}///end namespace A3Message
}///end namespace CRM
#endif
