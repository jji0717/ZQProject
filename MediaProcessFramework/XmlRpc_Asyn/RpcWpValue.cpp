
#include "rpcwpvalue.h"
#include "rpcwpexception.h"
#include <iostream>

#include "xmlrpc_plus/src/xmlrpcvalue.h"


namespace ZQ
{
	namespace rpc
	{
		RpcValue::RpcValue()
			:m_pInst(NULL)
		{
			m_pInst = new XmlRpc::XmlRpcValue;
		}

		RpcValue::RpcValue(bool value)
			:m_pInst(NULL)
		{
			m_pInst = new XmlRpc::XmlRpcValue(value);
		}

		RpcValue::RpcValue(int value)
			:m_pInst(NULL)
		{
			m_pInst = new XmlRpc::XmlRpcValue(value);
		}

		RpcValue::RpcValue(double value)
			:m_pInst(NULL)
		{
			m_pInst = new XmlRpc::XmlRpcValue(value);
		}

		RpcValue::RpcValue(const char* value)
			:m_pInst(NULL)
		{
			m_pInst = new XmlRpc::XmlRpcValue(value);
		}

		RpcValue::RpcValue(const tm* value)
			:m_pInst(NULL)
		{
			m_pInst = new XmlRpc::XmlRpcValue(const_cast<tm*>(value));
		}

		RpcValue::RpcValue(void* value, int size)
			:m_pInst(NULL)
		{
			m_pInst = new XmlRpc::XmlRpcValue(value, size);
		}

		RpcValue::RpcValue(const char* strXml, int* offset)
			:m_pInst(NULL)
		{
			m_pInst = new XmlRpc::XmlRpcValue(strXml, offset);
		}

		RpcValue::RpcValue(const RpcValue& value)
			:m_pInst(NULL)
		{

			m_pInst = new XmlRpc::XmlRpcValue(*(static_cast<XmlRpc::XmlRpcValue*>(value.m_pInst)));
		}

		RpcValue::~RpcValue()
		{
			if (NULL != m_pInst)
			{
				delete (static_cast<XmlRpc::XmlRpcValue*>(m_pInst));
				m_pInst = NULL;
			}
		}

		void RpcValue::clear()
		{
			static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->clear();
		}

		RpcValue& RpcValue::operator=(const RpcValue& rhs)
		{
			if (NULL != m_pInst)
			{
				delete (static_cast<XmlRpc::XmlRpcValue*>(m_pInst));
				m_pInst = NULL;
			}

			m_pInst = new XmlRpc::XmlRpcValue(*(static_cast<XmlRpc::XmlRpcValue*>(rhs.m_pInst)));
			return *this;
		}

		RpcValue& RpcValue::operator=(int rhs)
		{
			if (NULL != m_pInst)
			{
				delete (static_cast<XmlRpc::XmlRpcValue*>(m_pInst));
				m_pInst = NULL;
			}
			m_pInst = new XmlRpc::XmlRpcValue(rhs);
			return *this;
		}

		RpcValue& RpcValue::operator=(double rhs)
		{
			if (NULL != m_pInst)
			{
				delete (static_cast<XmlRpc::XmlRpcValue*>(m_pInst));
				m_pInst = NULL;
			}
			m_pInst = new XmlRpc::XmlRpcValue(rhs);
			return *this;
		}

		RpcValue& RpcValue::operator=(const char* rhs)
		{
			if (NULL != m_pInst)
			{
				delete (static_cast<XmlRpc::XmlRpcValue*>(m_pInst));
				m_pInst = NULL;
			}
			m_pInst = new XmlRpc::XmlRpcValue(rhs);
			return *this;
		}

		bool RpcValue::operator==(const RpcValue& other) const
		{
			return *(static_cast<XmlRpc::XmlRpcValue*>(m_pInst))==\
				*(static_cast<XmlRpc::XmlRpcValue*>(other.m_pInst));
		}

		bool RpcValue::operator!=(const RpcValue& other) const
		{
			return !(*this==other);
		}

		RpcValue::operator bool&()
		{
			return *(static_cast<XmlRpc::XmlRpcValue*>(m_pInst));
		}

		RpcValue::operator int&()
		{
			return *(static_cast<XmlRpc::XmlRpcValue*>(m_pInst));
		}

		RpcValue::operator double&()
		{
			return *(static_cast<XmlRpc::XmlRpcValue*>(m_pInst));
		}

		char* RpcValue::ToString(char* strBuffer, int nBufSize)
		{
			std::string strGet = *(static_cast<XmlRpc::XmlRpcValue*>(m_pInst));

			strncpy(strBuffer, strGet.c_str(), nBufSize);
			return strBuffer;
		}

		int RpcValue::ToBinaryData(char* strBuffer, int nBufSize)
		{
			std::vector<char> arrGet = *(static_cast<XmlRpc::XmlRpcValue*>(m_pInst));

			int nCopyedData = 0;
			for (std::vector<char>::const_iterator i = arrGet.begin();
			i < arrGet.end(); ++i)
			{
				if (nCopyedData >= nBufSize)
					break;

				strBuffer[nCopyedData++] = *i;
			}
			return nCopyedData;
		}

		RpcValue::operator struct tm&()
		{
			return *(static_cast<XmlRpc::XmlRpcValue*>(m_pInst));
		}

		RpcValue RpcValue::operator[](int i) const
		{
			try
			{
				XmlRpc::XmlRpcValue xrv = static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->operator[](i);			RpcValue ret;
				if (NULL != ret.m_pInst)
					delete ret.m_pInst;

				ret.m_pInst = new XmlRpc::XmlRpcValue(xrv);

				return ret;

			}
			catch(...)
			{
				RpcValue ret;
				return ret;
			}

		}

		void RpcValue::SetArray(int i, const RpcValue& rpc)
		{
			static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->operator[](i) = \
				*(static_cast<XmlRpc::XmlRpcValue*>(rpc.m_pInst));
		}

		RpcValue RpcValue::operator[](const char* key)
		{
			try
			{
				XmlRpc::XmlRpcValue xrv = static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->operator[](key);			RpcValue ret;
				if (NULL != ret.m_pInst)
					delete ret.m_pInst;

				ret.m_pInst = new XmlRpc::XmlRpcValue(xrv);

				return ret; 
			}
			catch(...)
			{
				RpcValue ret;
				return ret;
			}
		}

		void RpcValue::SetStruct(const char* key, const RpcValue& rpc)
		{
			static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->operator[](key) = \
				*(static_cast<XmlRpc::XmlRpcValue*>(rpc.m_pInst));
		}

		bool RpcValue::valid()
		{
			return static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->valid();
		}

		RpcValue::Type RpcValue::getType() const
		{
			return RpcValue::Type(static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->getType());
		}

		int RpcValue::size() const
		{
			return static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->size();
		}

		void RpcValue::setSize(int size)
		{
			static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->setSize(size);
		}

		bool RpcValue::hasMember(const char* name)
		{
			return static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->hasMember(name);
		}

		//modi by salien ,from return type bool to int
		bool RpcValue::fromXml(const char* valueXml, int* offset)
		{
			return static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->fromXml(valueXml, offset);
		}

		char* RpcValue::toXml(char* strBuffer, int nMax) const
		{
			std::string strGet = static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->toXml();
			
			char*p = strncpy(strBuffer, strGet.c_str(), nMax);
			strBuffer[nMax-1] ='\0';
			return p;
		}

		char* RpcValue::getDoubleFormat(char* strBuffer, int nMax)
		{
			std::string strGet = XmlRpc::XmlRpcValue::getDoubleFormat();

			return strncpy(strBuffer, strGet.c_str(), nMax);
		}

		void RpcValue::setDoubleFormat(const char* f)
		{
			XmlRpc::XmlRpcValue::setDoubleFormat(f);
		}

		bool RpcValue::listStruct(char* strKey, int nBufLen, bool bFirst)
		{
			static std::map<std::string, XmlRpc::XmlRpcValue>::iterator itor = \
				static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->_value.asStruct->begin();
			if (bFirst)
			{
				itor = static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->_value.asStruct->begin();
			}

			if (itor == static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->_value.asStruct->end())
			{
				return false;
			}

			strncpy(strKey, itor->first.c_str(), nBufLen);
			return true;
		}
	}
}

//void PrintValue(ZQ::rpc::RpcValue& v)
//{
//	(static_cast<XmlRpc::XmlRpcValue*>v.m_>write(std::cout);
//}