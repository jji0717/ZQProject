
#include "rpcwpexception.h"

#include "xmlrpc_plus/src/XmlRpcException.h"

namespace ZQ
{
	namespace rpc
	{
		RpcException::RpcException(const char* strMessage, int nCode)
			:m_pInst(NULL)
		{
			m_pInst = new XmlRpc::XmlRpcException(strMessage, nCode);
		}

		RpcException::RpcException(const RpcException& exp)
			:m_pInst(NULL)
		{
			m_pInst = new XmlRpc::XmlRpcException(exp.getMessage(), exp.getCode());
		}

		RpcException& RpcException::operator=(const RpcException& exp)
		{
			if (NULL != m_pInst)
			{
				delete m_pInst;
				m_pInst = NULL;
			}
			m_pInst = new XmlRpc::XmlRpcException(exp.getMessage(), exp.getCode());

			return *this;
		}

		RpcExpString RpcException::getMessage() const
		{
			return m_pInst->getMessage().c_str();
		}

		int RpcException::getCode() const
		{
			return m_pInst->getCode();
		}

		RpcException Exception_Conv(const XmlRpc::XmlRpcException& pInst)
		{
			return RpcException(pInst.getMessage().c_str(), pInst.getCode());
		}
	}
}
