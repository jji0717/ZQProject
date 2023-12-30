
#include "rpcwpclient.h"
#include "rpcwpvalue.h"
#include "xmlrpc_plus/src/xmlrpcclient.h"
#include "RpcWpReporter.h"

namespace ZQ
{
	namespace rpc
	{
		RpcClient::RpcClient(const char* host, int port, const char* uri)
			:m_pInst(NULL)
		{
			if (NULL != host && port >0)
				open(host, port, uri);
		}

		bool RpcClient::open(const char* host, int port, const char* uri)
		{
			if (NULL != m_pInst)
				close();

			m_pInst = (void*) new XmlRpc::XmlRpcClient(host, port, uri);
			return (NULL !=m_pInst);
		}


		RpcClient::~RpcClient()
		{
			close();
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
			if (NULL != m_pInst)
			{
				((XmlRpc::XmlRpcClient*)m_pInst)->close();
				delete ((XmlRpc::XmlRpcClient*)m_pInst);
			}
			m_pInst = NULL;
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
			memset(strBuffer, 0, nMax);
			return sfstrncpy(strBuffer, strGet.c_str(), nMax);
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
	}
}
