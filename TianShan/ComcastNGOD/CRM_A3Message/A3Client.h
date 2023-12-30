#ifndef __CRG_A3MESSAGE_A3CLIENT_H__
#define __CRG_A3MESSAGE_A3CLIENT_H__

#include "HttpClient.h"
#include "Locks.h"

namespace CRM
{
	namespace A3Message
	{
		class A3MsgEnv;
		class A3Client
		{
		public:
			A3Client(A3MsgEnv& env);
			~A3Client();

		public:
			int SendRequest(const std::string strContent, IN const std::string& url, IN OUT std::string& buffer);

		private:
			/// forbidden copy and assign
			A3Client(const A3Client& a3Client);
			A3Client& operator=(const A3Client& a3Client);
		private:
			ZQ::common::Mutex _lock;
			int64 _sequence;

			A3MsgEnv& _env;
		};

	}///end namespace A3Server
}///end namespace CRM

#endif

