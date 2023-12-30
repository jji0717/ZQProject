
#include "rpcwpvalue.h"
#include "rpcwpexception.h"
#include <iostream>
#include "RpcWpReporter.h"

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
			std::string strGet;
			try
			{
				strGet = *(static_cast<XmlRpc::XmlRpcValue*>(m_pInst));
			}
			catch(...)
			{
				strBuffer[0] = '\0';
				return NULL;
			}

			sfstrncpy(strBuffer, strGet.c_str(), nBufSize);
			return strBuffer;
		}

		int RpcValue::ToBinaryData(char* strBuffer, int nBufSize)
		{
			std::vector<char> arrGet;
			try
			{
				arrGet = *(static_cast<XmlRpc::XmlRpcValue*>(m_pInst));
			}
			catch(...)
			{
				return 0;
			}

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
				XmlRpc::XmlRpcValue xrv = static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->operator[](i);
				RpcValue ret;
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

		bool RpcValue::SetArray(int i, const RpcValue& rpc)
		{
			try
			{
				if (TypeArray != getType()&& TypeInvalid != getType())
					return false;

				static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->operator[](i) = \
					*(static_cast<XmlRpc::XmlRpcValue*>(rpc.m_pInst));
				return true;
			}
			catch(...)
			{
				return false;
			}
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

		bool RpcValue::SetStruct(const char* key, const RpcValue& rpc)
		{
			try
			{
				if (TypeStruct != getType() && TypeInvalid != getType())
					return false;

				static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->operator[](key) = \
					*(static_cast<XmlRpc::XmlRpcValue*>(rpc.m_pInst));
				return true;
			}
			catch(...)
			{
				return false;
			}
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

		bool RpcValue::setSize(int size)
		{
			try
			{
				static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->setSize(size);
				return size==this->size();
			}
			catch(...)
			{
				return false;
			}
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
			std::string strGet;
			try
			{
				strGet = static_cast<XmlRpc::XmlRpcValue*>(m_pInst)->toXml();
			}
			catch(...)
			{
				return NULL;
			}
			
			char*p = sfstrncpy(strBuffer, strGet.c_str(), nMax);
			strBuffer[nMax-1] ='\0';
			return p;
		}

		char* RpcValue::getDoubleFormat(char* strBuffer, int nMax)
		{
			std::string strGet = XmlRpc::XmlRpcValue::getDoubleFormat();

			return sfstrncpy(strBuffer, strGet.c_str(), nMax);
		}

		void RpcValue::setDoubleFormat(const char* f)
		{
			XmlRpc::XmlRpcValue::setDoubleFormat(f);
		}

		bool RpcValue::listStruct(char* strKey, int nBufLen, bool bFirst)
		{
			if (TypeStruct != getType())
				return false;

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

			/*
			char strTemp[512] = {0};
			toXml(strTemp, 512);
			printf("!!!%s\n", strTemp);
			*/
			
			sfstrncpy(strKey, itor->first.c_str(), nBufLen);
			++itor;
			return true;
		}
		
		bool RpcValue::pushBack(const RpcValue& rpc)
		{
			if (TypeArray == getType())
				return SetArray(size(), rpc);
			else
				return false;
		}

#define PRINT_TAB(count,print_proc) {for(int _TAB_SPEC_COUNT = 0;_TAB_SPEC_COUNT<(count);++_TAB_SPEC_COUNT)\
	(print_proc)("	");}

		void RpcValue::print(void (*PRINT_PROC)(const char* message), int depth)
		{
			PRINT_TAB(depth, PRINT_PROC);

			Type valuetype = getType();
			if (TypeInvalid == valuetype)
				PRINT_PROC("[Invalid]");
			else if (TypeBoolean == valuetype)
			{
				if (bool(*this))
					PRINT_PROC("[Boolean] true");
				else
					PRINT_PROC("[Boolean] false");
			}
			else if (TypeInt == valuetype)
			{
				char strTemp[21] = {0};
				_snprintf(strTemp, 20, "[Integer] %d", int(*this));
				PRINT_PROC(strTemp);
			}
			else if (TypeDouble == valuetype)
			{
				char strTemp[31] = {0};
				_snprintf(strTemp, 30, "[Integer] %lf", double(*this));
				PRINT_PROC(strTemp);
			}
			else if (TypeString == valuetype)
			{
				char strValue[256] = {0};
				toString(strValue, 256);
				char strTemp[301] = {0};
				_snprintf(strTemp, 300, "[String] %s", strValue);
				PRINT_PROC(strTemp);
			}
			else if (TypeDateTime == valuetype)
			{
				char strTemp[201] = {0};
				_snprintf(strTemp, 200, "[DataTime] %s", asctime(&(tm)(*this)));
				PRINT_PROC(strTemp);
			}
			else if (TypeBase64 == valuetype)
				PRINT_PROC("[Base64] ...");
			else if (TypeArray == valuetype)
			{
				PRINT_PROC("[Array]");
				//PRINT_TAB(depth, PRINT_PROC);

				for (int i = 0; i < size(); ++i)
				{
					PRINT_TAB(depth, PRINT_PROC);
					char strTemp[21] = {0};
					_snprintf(strTemp, 20, " # %d", i);
					PRINT_PROC(strTemp);
					(*this)[i].print(PRINT_PROC, depth+1);
				}
			}
			else if (TypeStruct == valuetype)
			{
				PRINT_PROC("[Structure]");
				char strKey[256] = {0};
				if (!listStruct(strKey, 256, true))
					return;
				do 
				{
					PRINT_TAB(depth, PRINT_PROC);
					char strTemp[301] = {0};
					_snprintf(strTemp, 300, " # %s", strKey);
					(*this)[strKey].print(PRINT_PROC, depth+1);
					memset(strKey, 0, 256);
				} while(listStruct(strKey, 256));
			}
			else
				PRINT_PROC("[Error] !!!");
		}
	}
}

//void PrintValue(ZQ::rpc::RpcValue& v)
//{
//	(static_cast<XmlRpc::XmlRpcValue*>v.m_>write(std::cout);
//}