#include "RpcClientThread.h"
#include "RpcWpClient.h"
#include "RpcWpAsynClient.h"


namespace ZQ
{
	namespace rpc
	{
		int RpcClientThread::run()
		{
			// lock
			RpcAsynClient::_mutex.enter();

			// assign locale value from static variable
			const char *host = RpcAsynClient::_host;
			int port = RpcAsynClient::_port;
			const char *uri = RpcAsynClient::_uri;
			int timeout = RpcAsynClient::_timeout;
			const char *method = RpcAsynClient::_method;
			RpcValue params = RpcAsynClient::_params;
			response_callback callback = RpcAsynClient::_callback;

			// unlock
			RpcAsynClient::_mutex.leave();

			// execute method
			RpcClient c(host, port, 0, RpcAsynClient::_sock_type);
			
			c.setSockType(RpcAsynClient::_sock_type);
			
			RpcValue result;
			c.execute(method, params, result);
			
			(*(RpcAsynClient::_callback))(RpcAsynClient::_uri, RpcAsynClient::_method, RpcAsynClient::_params, result);

			// remove thread from _list
			RpcAsynClient::_mutex.enter();
			RpcAsynClient::_list.remove(this);
			RpcAsynClient::_mutex.leave();

			return 0;
		}

		DWORD RpcClientThread::join(timeout_t timeout /* = INFINTE */)
		{
			if(isRunning())
				return waitHandle(timeout);
			else
			{
				sleep(1);
				return -1;
			}
		}
	}
}