
#include "rpcwpserver.h"
#include "rpcwpservermethod.h"
#include "rpcwpvalue.h"
#include "xmlrpc_plus/src/xmlrpcserver.h"
#include "xmlrpc_plus/src/xmlrpcservermethod.h"
#include "xmlrpc_plus/src/xmlrpcserverconnection.h"


namespace ZQ
{
	namespace rpc
	{
		RpcServer::RpcServer(SockType st)
			:m_pInst(NULL), m_bExit(false), m_bShutdown(false)
		{
			m_pInst = new XmlRpc::XmlRpcServer((int)st);
		}

		RpcServer::~RpcServer()
		{
			if (NULL != m_pInst)
			{
				delete ((XmlRpc::XmlRpcServer*)m_pInst);
				m_pInst = NULL;
			}
		}

		void RpcServer::enableIntrospection(bool enabled)
		{
			((XmlRpc::XmlRpcServer*)m_pInst)->enableIntrospection(enabled);
		}

		void RpcServer::addMethod(RpcServerMethod* method)
		{
			((XmlRpc::XmlRpcServer*)m_pInst)->addMethod(((XmlRpc::XmlRpcServerMethod*)method->m_pInst));
		}

		void RpcServer::removeMethod(RpcServerMethod* method)
		{
			((XmlRpc::XmlRpcServer*)m_pInst)->removeMethod(((XmlRpc::XmlRpcServerMethod*)method->m_pInst));
		}

		void RpcServer::removeMethod(const char* methodName)
		{
			((XmlRpc::XmlRpcServer*)m_pInst)->removeMethod(methodName);
		}

		/*
		RpcServerMethod* RpcServer::findMethod(const char* name, RpcServerMethod*& rsm) const
		{
			XmlRpc::XmlRpcServerMethod* xrsm = ((XmlRpc::XmlRpcServer*)m_pInst)->findMethod(name);
			//
			rsm->GetInst() = xrsm;
			return rsm;
		}
		*/

		bool RpcServer::bindAndListen(int port, const char* strip, int backlog)
		{
			return ((XmlRpc::XmlRpcServer*)m_pInst)->bindAndListen(port, strip, backlog);
		}

		void RpcServer::work(double msTime)
		{
			((XmlRpc::XmlRpcServer*)m_pInst)->work(msTime);
		}

		void RpcServer::exit()
		{
			if (!m_bExit)
			{
				((XmlRpc::XmlRpcServer*)m_pInst)->exit();
				m_bExit = true;
			}
		}

		void RpcServer::shutdown()
		{
			exit();
			if (!m_bShutdown)
			{
				((XmlRpc::XmlRpcServer*)m_pInst)->shutdown();
				m_bShutdown = true;
			}
		}

#define VALUE_BODY(_X) (*(XmlRpc::XmlRpcValue*)_X.m_pInst)
		void RpcServer::listMethods(RpcValue& result)
		{
			((XmlRpc::XmlRpcServer*)m_pInst)->listMethods(VALUE_BODY(result));
		}

		// added by lorenzo, 2005-05-30
		void RpcServer::setTcp()
		{
			((XmlRpc::XmlRpcServer*)m_pInst)->setTcp();
		}

		void RpcServer::setUdp()
		{
			((XmlRpc::XmlRpcServer*)m_pInst)->setUdp();
		}
		// add end
	}
}