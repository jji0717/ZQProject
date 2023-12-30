
#include "rpcwpclient.h"
#include "rpcwpvalue.h"
#include "xmlrpc_plus/src/xmlrpcclient.h"

namespace ZQ
{
	namespace rpc
	{
		RpcClient::RpcClient(const char* host, int port, const char* uri, int type)
			:m_pInst(NULL), m_bClosed(false)
		{
			m_pInst = (void*) new XmlRpc::XmlRpcClient(host, port, uri, type);
		}

		RpcClient::~RpcClient()
		{
			close();

			if (NULL != m_pInst)
			{
				delete ((XmlRpc::XmlRpcClient*)m_pInst);
				m_pInst = NULL;
			}
		}

		void RpcClient::setResponseTimeout(int timeout/* = -1*/)
		{
			if (NULL != m_pInst)
				((XmlRpc::XmlRpcClient*)m_pInst)->setResponseTimeout(timeout);
		}

#define VALUE_BODY(_X) (*(XmlRpc::XmlRpcValue*)_X.m_pInst)

		bool RpcClient::execute(const char* method, const RpcValue& params, RpcValue& result)
		{
			return ((XmlRpc::XmlRpcClient*)m_pInst)->execute(method, VALUE_BODY(params), VALUE_BODY(result));
		}

		bool RpcClient::isFault() const
		{
			return ((XmlRpc::XmlRpcClient*)m_pInst)->isFault();
		}

		void RpcClient::close()
		{
			if (!m_bClosed)
			{
				((XmlRpc::XmlRpcClient*)m_pInst)->close();
				m_bClosed = true;
			}
		}

		unsigned RpcClient::handleEvent(unsigned eventType)
		{
			return ((XmlRpc::XmlRpcClient*)m_pInst)->handleEvent(eventType);
		}

		bool RpcClient::doConnect()
		{
			return ((XmlRpc::XmlRpcClient*)m_pInst)->doConnect();
		}

		bool RpcClient::setupConnection()
		{
			return ((XmlRpc::XmlRpcClient*)m_pInst)->setupConnection();
		}

		bool RpcClient::generateRequest(const char* method, const RpcValue& params)
		{
			return ((XmlRpc::XmlRpcClient*)m_pInst)->generateRequest(method, VALUE_BODY(params));
		}

		char* RpcClient::generateHeader(const char* body, char* strBuffer, int nMax)
		{
			std::string strGet = ((XmlRpc::XmlRpcClient*)m_pInst)->generateHeader(body);
			return strncpy(strBuffer, strGet.c_str(), nMax);
		}

		bool RpcClient::writeRequest()
		{
			return ((XmlRpc::XmlRpcClient*)m_pInst)->writeRequest();
		}

		bool RpcClient::readHeader()
		{
			return ((XmlRpc::XmlRpcClient*)m_pInst)->readHeader();
		}

		bool RpcClient::readResponse()
		{
			return ((XmlRpc::XmlRpcClient*)m_pInst)->readResponse();
		}

		bool RpcClient::parseResponse(RpcValue& result)
		{
			return ((XmlRpc::XmlRpcClient*)m_pInst)->parseResponse(VALUE_BODY(result));
		}

		// added by lorenzo, 2005-05-30
		void RpcClient::setTcp()
		{
			((XmlRpc::XmlRpcClient*)m_pInst)->setTcp();
		}

		void RpcClient::setUdp()
		{
			((XmlRpc::XmlRpcClient*)m_pInst)->setUdp();
		}

		void RpcClient::setSockType(int socktype)
		{
			((XmlRpc::XmlRpcClient*)m_pInst)->setSockType(socktype);
		}
		// add end
	}
}


