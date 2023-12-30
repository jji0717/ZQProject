#ifndef __CDMIHttpClient_H__
#define __CDMIHttpClient_H__

#include "CURLClient.h"

namespace ZQ
{
	namespace CDMIClient
	{
		class CDMIHttpClient: public ZQ::common::CURLClient
		{
		public:
			typedef ZQ::common::Pointer < CDMIHttpClient > Ptr;
			CDMIHttpClient(char* url, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdpool, int flag = 0, HTTPMETHOD method = HTTP_GET, char* bindIp = "0.0.0.0", std::string clientId ="");
			virtual ~CDMIHttpClient(void) {};

		protected:
			virtual void OnTxnCompleted(CURLcode code);
		};

}} ///end namespace
#endif /// end define __CDMIHttpClient_H__

