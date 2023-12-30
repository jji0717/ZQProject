
#include "rpcwpservermethod.h"
#include "rpcwpvalue.h"
#include "rpcwpserver.h"
#include "xmlrpc_plus/src/xmlrpcservermethod.h"
#include "xmlrpc_plus/src/xmlrpcvalue.h"
#include "RpcWpReporter.h"

#define VALUE_BODY(_X) (*(XmlRpc::XmlRpcValue*)_X.m_pInst)

namespace XmlRpc
{
}

namespace ZQ
{
	namespace rpc
	{

		class RpcServerMethodNest : public XmlRpc::XmlRpcServerMethod
	{
	private:
		ZQ::rpc::RpcServerMethod*	_redirect;	//bad code

//		ZQ::rpc::RpcValue coparams;
	public:
		RpcServerMethodNest(const std::string& name, RpcServerMethod* method, XmlRpc::XmlRpcServer* server =NULL)
			:XmlRpcServerMethod(name, server), _redirect(method)
		{
		}

		virtual void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result)
		{
			if (NULL != _redirect)
			_redirect->_execute(&params, &result);
		}

	};

		RpcServerMethod::RpcServerMethod(const char* name, RpcServer* server)
		{
			std::string strName(name);
			m_pInst = new RpcServerMethodNest(strName, this, (XmlRpc::XmlRpcServer *)(server?server->m_pInst: NULL));
		}

		RpcServerMethod::~RpcServerMethod()
		{
			if (NULL != m_pInst)
			{
				delete ((RpcServerMethodNest*)m_pInst);
				m_pInst = NULL;
			}
		}

		char* RpcServerMethod::name(char* strBuffer, int nMax)
		{
			std::string strGet = ((RpcServerMethodNest*)m_pInst)->name();
			return sfstrncpy(strBuffer, strGet.c_str(), nMax);
		}

		char* RpcServerMethod::help(char* strBuffer, int nMax)
		{
			std::string strGet = ((RpcServerMethodNest*)m_pInst)->help();
			return sfstrncpy(strBuffer, strGet.c_str(), nMax);
		}

		void RpcServerMethod::_execute(void* params, void* result)
		{
			RpcValue CoParams, CoResult;

			if (NULL != CoParams.m_pInst)
				delete CoParams.m_pInst;

			CoParams.m_pInst = new XmlRpc::XmlRpcValue(*((XmlRpc::XmlRpcValue*) params));

			execute(CoParams, CoResult);

			*((XmlRpc::XmlRpcValue*) result) = VALUE_BODY(CoResult);
		}
	}
}
