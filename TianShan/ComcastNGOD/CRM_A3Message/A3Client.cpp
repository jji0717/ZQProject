#include "A3Client.h"
#include "SystemUtils.h"
#include "A3MsgEnv.h"
namespace CRM
{
	namespace A3Message
	{

		A3Client::A3Client(A3MsgEnv& env)
			:_env(env), _sequence(0)
		{
		}

		A3Client::~A3Client()
		{
		}


		int A3Client::SendRequest(std::string strContent, const std::string &url, std::string &buffer)
		{	
			ZQ::common::HttpClient a3MsgClient;
			int statusCode = -1;

			int64 sequence = 0;
			{
				ZQ::common::MutexGuard gd(_lock);
				sequence = ++ _sequence;
			}

			try
			{
				a3MsgClient.init();
				a3MsgClient.setRecvTimeout(20);
				a3MsgClient.setLog(&_env._log);

				// set http head
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(A3Client, "[%s]sending TransferStatus to [%s]: %s"), strContent.c_str(), url.c_str(), buffer.c_str());
				char length[64];
				snprintf(length, sizeof(length), "%ld", buffer.length());
				a3MsgClient.setHeader("Content-Type", "text/xml");
				// Content-Length will be automatically added: a3MsgClient.setHeader("Content-Length", length);

				snprintf(length, sizeof(length), "%ld", sequence);
				a3MsgClient.setHeader("CSeq", length);

				if(a3MsgClient.httpConnect(url.c_str())) 
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT( A3Client, "[%s] post TransferStatus failed to connect to endpoint[%s]"), strContent.c_str(), url.c_str());
					a3MsgClient.uninit();
					return -1;
				}

				if(a3MsgClient.httpSendContent(buffer.data(), buffer.length()) || a3MsgClient.httpEndSend()) 
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT( A3Client, "[%s] post TransferStatus failed, error[%s]"), strContent.c_str(), a3MsgClient.getErrorstr());
					a3MsgClient.uninit();
					return -1;
				}

				if(a3MsgClient.httpBeginRecv())
				{
					envlog(ZQ::common::Log::L_ERROR, CLOGFMT( A3Client, "[%s] post TransferStatus failed to receive reponse, error[%s]"), strContent.c_str(), a3MsgClient.getErrorstr());
					a3MsgClient.uninit();
					return -1;
				}

	            std::string response;
				while(!a3MsgClient.isEOF()) 
				{

					if(a3MsgClient.httpContinueRecv()) 
					{
						envlog(ZQ::common::Log::L_ERROR, CLOGFMT( A3Client, "[%s]post TransferStatus failed to continue receive reponse with errror[%s]"), strContent.c_str(), a3MsgClient.getErrorstr());
						a3MsgClient.uninit();	
						return -1;
					}
					std::string strRC;
					a3MsgClient.getContent(strRC);
					response += strRC;
				}

				a3MsgClient.getContent(response);		

				statusCode = a3MsgClient.getStatusCode();
				a3MsgClient.uninit(); // may be have a problem here

				envlog(ZQ::common::Log::L_INFO, CLOGFMT(A3Client, "[%s] post TransferStatus response status code[%d][%s]"), strContent.c_str(), statusCode, response.c_str());

			}
			catch (...)
			{
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(A3Client, "[%s] post TransferStatus caught exception[%s]"), strContent.c_str(), SYS::getErrorMessage().c_str());
			}

			return statusCode;
		}
	}///end namespace A3Server
}///end namespace CRM