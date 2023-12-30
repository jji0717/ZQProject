
// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Dev  : Microsoft Developer Studio
// Name  : RpcWpValue.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-26
// Desc  : rpc value 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/XmlRpc/RpcWpValue.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 17    05-07-07 14:52 Jie.zhang
// 
// 16    05-07-07 14:50 Jie.zhang
// 
// 15    05-07-06 15:42 Jie.zhang
// 
// 14    05-07-01 7:44p Daniel.wang
// 
// 13    05-06-20 11:09p Daniel.wang
// 
// 12    5/18/05 10:40a Daniel.wang
// 
// 11    5/16/05 8:32p Daniel.wang
// 
// 10    5/12/05 10:30p Daniel.wang
// 
// 9     05-04-26 15:54 Daniel.wang
// ===========================================================================

#ifndef _RPC_WP_VALUE_H_
#define _RPC_WP_VALUE_H_

#include "RpcWrapper.h"

#include <tchar.h>
#include <time.h>

#define MAX_VALUE_STRUCT_KEY_LEN 256

namespace XmlRpc
{
	class XmlRpcValue;
}

namespace ZQ
{
	namespace rpc
	{
		//////////////////////////////////////////////////////////////////////////
		///RpcValue\n
		///RPC Value is a mixed type for C++.\n
		///\n
		///nested array or structure value sample:\n
		///ZQ::rpc::RpcValue sample1;\n
		///sample1.SetStruct("nested2 test", 233);\n
		///sample1.SetStruct("nested2 test1", "test value");\n
		///ZQ::rpc::RpcValue sample2;\n
		///sample2.SetArray(0, sample1);\n
		///sample2.SetArray(1, "test value2");\n
		///ZQ::rpc::RpcValue sample3;\n
		///sample3.SetStruct("nested1 test", sample1);\n
		///sample3.SetStruct("nested1 test1", sample2);\n
		//////////////////////////////////////////////////////////////////////////
		class RPCWAPPER_API RpcValue
		{
		public:
			enum Type 
			{
				TypeInvalid,
				TypeBoolean,
				TypeInt,
				TypeDouble,
				TypeString,
				TypeDateTime,
				TypeBase64,
				TypeArray,
				TypeStruct
			};

		friend class RpcClient;
		friend class RpcServer;
		friend class RpcServerMethod;

		private:
			void*	m_pInst;
		public:

			///constructor\n
			/// the value type is TypeInvalid
			RpcValue();

			///constructor\n
			/// the value type is TypeBoolean
			///@param value - initialization value
			explicit RpcValue(bool value);

			///constructor\n
			/// the value type is TypeInt
			///@param value - initialization value	
			explicit RpcValue(int value);

			///constructor\n
			/// the value type is TypeDouble
			///@param value - initialization value
			explicit RpcValue(double value);

			///constructor\n
			/// the value type is TypeString
			///@param value - initialization value
			explicit RpcValue(const char* value);

			///constructor\n
			/// the value type is TypeDateTime
			///@param value - initialization value
			explicit RpcValue(const tm* value);

			///constructor\n
			/// the value type is TypeBase64
			///@param value - initialization value buffer
			///@param size - buffer size
			RpcValue(void* value, int size);

			///constructor\n
			/// get value from XML sting
			///@param strXml - XML string
			///@param offset - start of the value in XML string buffer
			RpcValue(const char* strXml, int* offset);

			///copy constructor
			RpcValue(const RpcValue& value);

			///destructor
			virtual ~RpcValue();

			///clear the data value and set the data type to TypeInvalid
			void clear();

			///operator=\n
			///@param rhs - get data value from another RPC value
			///@return - return current value 
			RpcValue& operator=(const RpcValue& rhs);
			
			///operator=\n
			///@param rhs - assign the value to integer type
			///@return - return current value 
			RpcValue& operator=(int rhs);

			///operator=\n
			///@param rhs - assign the value to double type
			///@return - return current value 
			RpcValue& operator=(double rhs);
			
			///operator=\n
			///@param rhs - assign the value to string type
			///@return - return current value 
			RpcValue& operator=(const char* rhs);

			///operator==\n
			/// compare the two rpc value if the same
			///@param other - the compared value 
			///@return - return true if the two rpc value are the same
			bool operator==(const RpcValue& other) const;

			///operator==\n
			/// compare the two rpc value if the not same
			///@param other - the compared value 
			///@return - return true if the two rpc value are the not same
			bool operator!=(const RpcValue& other) const;

			///operator type\n
			/// convert rpc value to boolean type
			///@return - boolean type
			operator bool&();

			///operator type\n
			/// convert rpc value to integer type
			///@return - integer type
			operator int&();

			///operator type\n
			/// convert rpc value to double type
			///@return - double type
			operator double&();
			
			///ToString\n
			/// convert rpc value to string type
			///@param strBuffer - string buffer
			///@param nBufSize - string buffer size
			///@return - the pointer of strBuffer
			char* ToString(char* strBuffer, int nBufSize);
			char* toString(char* strBuffer, int nBufSize)
			{ return ToString(strBuffer, nBufSize); }

			///ToBinaryData\n
			/// convert rpc value to binary string
			///@param strBuffer - binary string buffer
			///@param nBufSize - binary string buffer size
			///@return - how many size of buffer used
			int ToBinaryData(char* strBuffer, int nBufSize);
			int toBinaryData(char* strBuffer, int nBufSize)
			{ return ToBinaryData(strBuffer, nBufSize); }

			///operator type\n
			/// convert rpc value to tm type
			///@return - tm type(time)
			operator struct tm&();

			///operator[]\n
			///get the sub value of array
			///@param i - the value position in the array
			///@return - return the value in the array
			RpcValue operator[](int i) const;

			///SetArray\n
			/// insert an value into array\n
			///\n
			///It must begin from 0, and increase by ordinal. \n
			///If you have skip some number, the empty items will be set to ZQ::rpc::RpcValue::TypeInvalid type.\n
			///e.g. \n
			///ZQ::rpc::RpcValue sample;\n
			///sample.SetArray(5, "test string");\n
			///then you will get sample[0] is ZQ::rpc::RpcValue::TypeInvalid type, and the same to sample[1,2,3,4].\n
			///\n
			///If you have set the value's type with not ZQ::rpc::RpcValue::TypeInvalid or ZQ::rpc::RpcValue::TypeArray, you can not reset this value to array. \n
			///e.g.\n
			///ZQ::rpc::RpcValue sample = 5; //have set be integer type.\n
			///sample.SetArray(0, "test value");//ERROR! You can not reset the type to array.\n
			///ZQ:: rpc::RpcValue sample2;//have set be invalid type.\n
			///sample2.SetArray(0, "test value");//OK! The type is ZQ::rpc::RpcValue::TypeInvalid.\n
			///@param i - the value position in the array
			///@param rpc - the new value data
			bool SetArray(int i, const RpcValue& rpc);
			bool setArray(int i, const RpcValue& rpc)
			{ return SetArray(i, rpc); }

			///operator[]\n
			///get the sub value of structure
			///@param key - the key of rpc value
			///@return return the value in the structure
			RpcValue operator[](const char* key);
			
			///SetStruct\n
			///insert an value into structure\n
			///If you have set the value's type with not ZQ::rpc::RpcValue::TypeInvalid or ZQ::rpc::RpcStruct::TypeArray, you can not reset this value to structure. \n
			///e.g.\n
			///ZQ::rpc::RpcValue sample = 5; //have set be integer type.\n
			///sample.SetStruct("test key", "test value");//ERROR! You can not reset the type to structure.\n
			///\n
			///ZQ:: rpc::RpcValue sample2;//have set be invalid type.\n
			///sample2.SetStruct("test key", "test value");//OK! The type is ZQ::rpc::RpcValue::TypeInvalid.\n
			///@param key - the key of rpc value
			bool SetStruct(const char* key, const RpcValue& rpc);
			bool setStruct(const char* key, const RpcValue& rpc)
			{ return SetStruct(key, rpc); }

			///valid
			///@return - return true if the rpc value is valided
			bool valid();

			///getType\n
			/// get rpc value type
			///@return - rpc value type
			Type getType() const;

			///size\n
			///get rpc value size
			///@return - rpc value size
			int size() const;

			///setSize\n
			///reset rpc value size
			///@param size - rpc value size
			bool setSize(int size);

			///hasMember\m
			///this is only for structure type rpc value
			///@param name - member key string
			///@return - return true if there is member in the rpc value
			bool hasMember(const char* name);

			///fromXml\n
			///convert XML string to rpc value\n
			///the XML string if only for RpcValue, can not convert every XML string to rpc value.
			///@param valueXml - XML string
			///@param offset - the begin of XML string for value
			///@return - return ture if converted
			bool fromXml(const char* valueXml, int* offset);

			///toXml\n
			///convert rpc value to XML string 
			///@param strBuffer - string buffer
			///@param nMax - the buffer size
			///@return - the pointer of string buffer
			char* toXml(char* strBuffer, int nMax) const;

			///getDoubleFormat\n
			///get the double format setting
			///@param strBuffer - string buffer
			///@param nMax - the buffer size
			///@return - the pointer of string buffer
			static char* getDoubleFormat(char* strBuffer, int nMax);

			///setDoubleFormat\n
			///set the double format setting\n
			///the format is same to printf format string, but only for %f\n
			///sample: RpcValue::setDoubleFormat("%5.6f");
			///@param f - format string
			static void setDoubleFormat(const char* f);

			///listStruct\n
			///list data in the rpc value with structure type\n
			///sample:\n
			///rpc::RpcValue structValue;\n
			///...//insert data into structValue\n
			///char strKey[256] = {0};\n
			///if (!listStruct(strKey, 256, true))\n
			///		printf("value is empty\n");\n
			///printf("get value key : %s\n", strKey);\n
			///memset(strKey, 256, 0);\n
			///while (listStruct(strKey, 256))\n
			///{\n
			///		printf("get value key : %s\n", strKey);\n
			///		memset(strKey, 256, 0);\n
			///}\n
			///@param strKey - rpc value key string buffer 
			///@param nBufLen - string buffer size
			///@param bFirst - get first key if ture
			///@return - return true if get a data from structure\n
			bool listStruct(char* strKey, int nBufLen, bool bFirst = false);
			
			///this is only for Array type
			bool pushBack(const RpcValue& rpc);

			///print value
			//depth must be zero, you can not change this parameter
			void print(void (*PRINT_PROC)(const char* message), int depth = 0);
			
			//bool procedure(bool (*PROC_PROC)(const RpcValue& value, RpcValue::Type valtype, int depth), bool alldone);
		};
	}

}

//std::ostream& operator<<(std::ostream& os, ZQ::rpc::RpcValue& v);
// void RPCWAPPER_API PrintValue(ZQ::rpc::RpcValue& v);

#endif//_RPC_WP_VALUE_H_
