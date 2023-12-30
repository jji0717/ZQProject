#pragma once

//#define AquaClient_EXPORTS

#include "..\AquaClient.h"
#include "wtypes.h"
using namespace System;
using namespace System::Text;
using namespace System::IO;
using namespace System::Collections::Generic;

namespace XOR_Media {
	namespace AquaClient {
		public ref class LongVariable
		{
		public:
			/**
			* Constructor
			* @param val the initial value of the variable
			*/
			LongVariable(long val) {
				set(val);
			}

			/**
			* Set the value of the variable
			* @param val the new integer value is about to set to
			* @return the integer value has just been set
			*/
			long set(long val) {
				_val = val;
				return get();
			}

			/**
			* Get the value of the variable
			* @return the integer value of variable
			*/
			long get() {
				return _val;
			}

		private:
			long _val;
		};

		public ref class AquaClientRef
		{
			// 包装所有类AquaClient的公有成员函数
		public:


			AquaClientRef(String^ rootURL, String^ homeContainer, String^ jsonProps);
			~AquaClientRef();

			static bool initSDK(Dictionary<String^, String^>^rjsonSettings);

			static void uninitSDK();

			int   cdmi_CreateDataObject(StringBuilder^ rjsonResult, String^ ruri,
				String^ rmimetype, String^ rjsonMetadata, String^ rvalue,
				array<String^>^ rvaluetransferencoding, String^ rdomainURI,
				String^ rdeserialize, String^ rserialize, String^rcopy, String^ rmove,
				String^ rreference, String^ rdeserializeValue);

			int   nonCdmi_CreateDataObject(String^ ruri, String^ rcontentType, array<byte>^ rvalue, int len);

			int   cdmi_ReadDataObject(StringBuilder^ rjsonResult, String^ ruri, StringBuilder^ rlocation);

			int   nonCdmi_ReadDataObject(String^ ruri, StringBuilder^ rcontentType,
				StringBuilder^ rlocation, long startOffset, LongVariable^ rbyteRead,
				array<byte>^ rbuffer, int len, bool direct);

			int   cdmi_UpdateDataObject(String^ ruri, StringBuilder^ rlocation, String^ rjsonMetadata,
				int rstartOffset, String^ rvalue, int rbase_version, bool rpartial, array<String^>^ rvaluetransferencoding,
				String^ rdomainURI, String^ rdeserialize, String^ rcopy,String^ rdeserializevalue);

			int   nonCdmi_UpdateDataObject(String^ ruri, StringBuilder^ rlocation, String^ rcontentType, long startOffset, long objectSize, array<byte>^ rbuffer, int len, bool partial, bool direct);

			int   cdmi_DeleteDataObject(StringBuilder^ jsonResult, String^uri);

			int   nonCdmi_DeleteDataObject(String^ uri);

			int   cdmi_CreateContainer(StringBuilder^ rjsonResult, String^ ruri,String^ rjsonMetadata,
				String^ rjsonExports, String^ rdomainURI, String^ rdeserialize, 
				String^ rcopy, String^ rmove, String^ rreference, 
				String^ rdeserializeValue);

			int   nonCdmi_CreateContainer(String^ ruri);

			int   cdmi_ReadContainer(StringBuilder^ rjsonResult, String^ruri);

			int   cdmi_UpdateContainer(StringBuilder^ rjsonResult, String^ ruri, String^ rlocation,
				String^ rjsonMetadata, String^ rjsonExports,
				String^ rdomainURI, String^ rdeserialize, String^ rcopy,
				String^ rsnapshot, String^ rdeserializeValue);

			int   cdmi_DeleteContainer(StringBuilder^ jsonResult, String^ ruri);

			int   nonCdmi_DeleteContainer(String ^ ruri);

			int   cdmi_ReadAquaDomain(StringBuilder^ rjsonResult, String ^ rdomainURI);

			int   flushDataObject(String ^ ruri );

		private:
			// 类AquaClient的指针，用来调用类AquaClient的成员函数
			AquaClient *m_pImp;
		};
	}
}
