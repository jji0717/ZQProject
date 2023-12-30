#include "rpcwpclient.h"
#include "rpcwpvalue.h"
#include "xmlrpc_plus/src/xmlrpcclient.h"

#include "RpcClientThread.h"
#include "RpcWpAsynClient.h"

namespace ZQ
{
	namespace rpc
	{
		int RpcAsynClient::_sock_type = SOCK_STREAM;
		const char * RpcAsynClient::_host = NULL;
		int RpcAsynClient::_port = 0;
		const char * RpcAsynClient::_uri = 0;
		int RpcAsynClient::_timeout = -1;
		RpcValue RpcAsynClient::_params;
		const char * RpcAsynClient::_method;
		response_callback RpcAsynClient::_callback;
		ZQ::common::Mutex  RpcAsynClient::_mutex;
		thread_list RpcAsynClient::_list;

		/////////////////////////////////////////////////////////////////////////
		// implementation of class RpcAsynClient
		RpcAsynClient::RpcAsynClient(const char *host, int port, const char *uri, int type)
		{
			_host = host;
			_port = port;
			_uri = uri;
			_sock_type = type;
		}

		RpcAsynClient::~RpcAsynClient()
		{
		}

		void RpcAsynClient::setResponseTimeout(int timeout)
		{
			_timeout = timeout;
		}

		bool RpcAsynClient::execute(const char *method, RpcValue& params, response_callback callback)
		{
			// lock
			_mutex.enter();

			_method = method;
			_params = params;
			_callback = callback;

			RpcClientThread *th = new RpcClientThread;

			_list.push_back(th);
			_cur_list.push_back(th);

			// unlock
			_mutex.leave();

			th->start();

			return true;
		}

		void RpcAsynClient::setTcp()
		{
			_sock_type = SOCK_STREAM;
		}

		void RpcAsynClient::setUdp()
		{
			_sock_type = SOCK_DGRAM;
		}

		void RpcAsynClient::waitAsyn()
		{
			thread_list::iterator it;

			while(true)
			{
				_mutex.enter();
				deleteInactiveThread();
				if(_list.empty())
				{
					_mutex.leave();
					break;
				}
				else
				{
					it = _list.begin();
					RpcClientThread *th = *it;
					deleteInactiveThread();
					_mutex.leave();
					th->join();
				}
			}
		}

		void RpcAsynClient::deleteInactiveThread()
		{
			thread_list::iterator it;

			for(it = _cur_list.begin(); it != _cur_list.end();)
			{
				if(!(*it)->isRunning())
				{
					delete *it;
					_cur_list.erase(it++);
				}
				else
					++it;
			}
		}
	}
}