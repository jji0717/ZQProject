#include "AquaClientCLR.h"
#include <vcclr.h>
#include <json/json.h>
//#include <msclr/marshal.h>

using namespace System::Collections::Generic;
using namespace  System::Text;

using namespace System::Runtime::InteropServices;

using namespace std;
namespace XOR_Media {
	namespace AquaClient {

		// System.String to std::string
		void ManagedString2UnmanagedStringA (String ^ s, std::string& os )
		{
			const char* chars = (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
			os = chars;
			Marshal::FreeHGlobal(IntPtr((void*)chars));
		}
/*
		//std::String to System.String
		void Marshalstring (std::string& s, String^ os ) {

			os = gcnew String(s.c_str(), s.length()); 
		}
*/
		AquaClientRef::AquaClientRef(String^ rrootURL, String^ rhomeContainer, String^ rjsonProps)
		{
			m_pImp = NULL;
			std::string rootURL, homeContainer, jsonProps;
			ManagedString2UnmanagedStringA(rrootURL, rootURL);
			ManagedString2UnmanagedStringA(rhomeContainer, homeContainer);
			ManagedString2UnmanagedStringA(rjsonProps, jsonProps);
			
		    m_pImp = AquaClient::newClient(rootURL, homeContainer, jsonProps);
		}

		AquaClientRef::~AquaClientRef()
		{
			if(m_pImp)
				delete m_pImp;
		}

	 bool AquaClientRef::initSDK(Dictionary<String^, String^>^rjsonSettings)
	{
		std::string jsonSettings;
		Json::Value jsonConfig;

		KeyValuePair<String^, String^> kvp;

		for each( KeyValuePair<String^, String^> kvp in rjsonSettings )
		{
			std::string key, value;
			ManagedString2UnmanagedStringA(kvp.Key, key);
			ManagedString2UnmanagedStringA(kvp.Value, value);
			if(key == "log.level" || key == "log.size" || key == "log.count" || key == "threads")
				jsonConfig[key] = atoi(value.c_str());
			else
				jsonConfig[key] = value.c_str();
		}
		Json::FastWriter writer;
		jsonSettings = writer.write(jsonConfig);

		return AquaClient::initSDK(jsonSettings);
	}

	void AquaClientRef::uninitSDK()
	{
		AquaClient::uninitSDK();
	}

	int   AquaClientRef::cdmi_CreateDataObject(StringBuilder^ rjsonResult, String^ ruri,
		String^ rmimetype, String^ rjsonMetadata, String^ rvalue,
		array<String^>^ rvaluetransferencoding, String^ rdomainURI,
		String^ rdeserialize, String^ rserialize, String^rcopy, String^ rmove,
		String^ rreference, String^ rdeserializeValue)
	{
		std::string jsonResult,  uri, mimetype, jsonMetadata,  value, domainURI;
		std::string deserialize, serialize, copy, move,reference,deserializeValue;

		ManagedString2UnmanagedStringA(ruri, uri);
		ManagedString2UnmanagedStringA(rmimetype, mimetype);
		ManagedString2UnmanagedStringA(rjsonMetadata, jsonMetadata);
		ManagedString2UnmanagedStringA(rvalue, value);
		ManagedString2UnmanagedStringA(rdomainURI, domainURI);
		ManagedString2UnmanagedStringA(rdeserialize, deserialize);
		ManagedString2UnmanagedStringA(rserialize, serialize);
		ManagedString2UnmanagedStringA(rcopy, copy);
		ManagedString2UnmanagedStringA(rmove, move);
		ManagedString2UnmanagedStringA(rreference, reference);
		ManagedString2UnmanagedStringA(rdeserializeValue, deserializeValue);

		std::vector <std::string> valuetransferencoding;
		for(int i = 0; i < rvaluetransferencoding->Length; i++)
		{
			std::string valueTrans;
			ManagedString2UnmanagedStringA(rvaluetransferencoding[i], valueTrans);
			valuetransferencoding.push_back(valueTrans);
		}

		int nret = m_pImp->cdmi_CreateDataObject(jsonResult,uri,mimetype, jsonMetadata, value,
			valuetransferencoding,  domainURI,deserialize,  serialize,  copy,  move,reference,  deserializeValue);


		if(nret >=200 && nret <= 300)
		{
			rjsonResult->Remove(0, rjsonResult->Length);
			rjsonResult->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)jsonResult.c_str())));
		}
		return nret;
	}

	int  AquaClientRef::nonCdmi_CreateDataObject(String^ ruri,  String^ rcontentType, array<byte>^ rvalue, int len)
	{
		std::string uri, contentType;

		ManagedString2UnmanagedStringA(ruri, uri);
		ManagedString2UnmanagedStringA(rcontentType, contentType);

/*		////这里需要修改, 
		char* buf = NULL;
		if(len > 0)
		{
			buf = new char [len];
		}
		for(int i = 0; i < len; i++)
			buf[i] = (char)rvalue[i];

		int nret = m_pImp->nonCdmi_CreateDataObject(uri, contentType, buf,len);

		if(buf)
			delete []buf;*/

		IntPtr p = Marshal::UnsafeAddrOfPinnedArrayElement(rvalue, 0);
		int nret = m_pImp->nonCdmi_CreateDataObject(uri, contentType, (char*)p.ToPointer(),len);

		return nret;
	}

	int   AquaClientRef::cdmi_ReadDataObject(StringBuilder^ rjsonResult, String^ ruri, StringBuilder^ rlocation)
	{
		std::string jsonResult, uri, location;

		ManagedString2UnmanagedStringA(ruri, uri);

		int nret = m_pImp->cdmi_ReadDataObject(jsonResult, uri, location);

		if(nret >= 200 && nret <= 300)
		{
			rjsonResult->Remove(0, rjsonResult->Length);
			rjsonResult->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)jsonResult.c_str())));

			rlocation->Remove(0, rlocation->Length);
			rlocation->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)location.c_str())));
		}
		return nret;
	}

	int   AquaClientRef::nonCdmi_ReadDataObject(String^ ruri, StringBuilder^ rcontentType,
		StringBuilder^ rlocation, long startOffset, LongVariable^ rbyteRead,
		array<byte>^ rbuffer, int len, bool direct)
	{
		std::string uri, contentType, location;

		ManagedString2UnmanagedStringA(ruri, uri);

		long byteRead = 0;
/*		byte* buf = NULL;
		if(len > 0)
		{
			buf = new byte [len];
		}

	// int nret =  m_pImp->nonCdmi_ReadDataObject(uri, contentType,location,  startOffset,  byteRead, (char*)buf, len, direct);

*/
		IntPtr p = Marshal::UnsafeAddrOfPinnedArrayElement(rbuffer, 0);
		int nret =  m_pImp->nonCdmi_ReadDataObject(uri, contentType,location,  startOffset,  byteRead, (char*)p.ToPointer(), len, direct);

		if(nret >= 200 && nret <= 300)
		{
//			Marshal::Copy((IntPtr)buf, rbuffer, 0, byteRead);
			rbyteRead->set(byteRead);

			rlocation->Remove(0, rlocation->Length);
			rlocation->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)location.c_str())));

			rcontentType->Remove(0, rcontentType->Length);
			rcontentType->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)contentType.c_str())));
		}
//		if(buf)
//			delete []buf;

		return nret;
	}
	int   AquaClientRef::cdmi_UpdateDataObject(String^ ruri, StringBuilder^ rlocation, String^ rjsonMetadata,
		int startOffset, String^ rvalue, int base_version, bool partial, array<String^>^ rvaluetransferencoding,
		String^ rdomainURI, String^ rdeserialize, String^ rcopy, String^ rdeserializevalue)
	{

		std::string uri, location, jsonMetadata, value;
		std::string domainURI, deserialize,copy,deserializevalue;     

		ManagedString2UnmanagedStringA(ruri, uri);
		ManagedString2UnmanagedStringA(rjsonMetadata, jsonMetadata);
		ManagedString2UnmanagedStringA(rvalue, value);
		ManagedString2UnmanagedStringA(rdomainURI, domainURI);
		ManagedString2UnmanagedStringA(rdeserialize, deserialize);
		ManagedString2UnmanagedStringA(rcopy, copy);
		ManagedString2UnmanagedStringA(rdeserializevalue, deserializevalue);

		std::vector <std::string> valuetransferencoding;
		for(int i = 0; i < rvaluetransferencoding->Length; i++)
		{
			std::string valueTrans;
			ManagedString2UnmanagedStringA(rvaluetransferencoding[i], valueTrans);
			valuetransferencoding.push_back(valueTrans);
		}

		int nret = m_pImp->cdmi_UpdateDataObject(uri,  location,  jsonMetadata,
			startOffset,  value,  base_version,  partial, valuetransferencoding,
			domainURI,  deserialize, copy,
			deserializevalue);

		if(nret >=200 && nret <= 300)
		{
			rlocation->Remove(0, rlocation->Length);
			rlocation->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)location.c_str())));
		}
		return nret;
	}

	int  AquaClientRef::nonCdmi_UpdateDataObject(String^ ruri, StringBuilder^ rlocation, String^ rcontentType, long startOffset, long objectSize, array<byte>^ rbuffer, int len, bool partial, bool direct)
	{
		std::string uri, location, contentType;

		ManagedString2UnmanagedStringA(ruri, uri);
		ManagedString2UnmanagedStringA(rcontentType, contentType);
        
	/*	char* buf = NULL;
		if(len > 0)
		{
			buf = new char [len];
		}
		//这里需要修改
		for(int i = 0; i < len; i++)
			buf[i] = (char)rbuffer[i];

		int nret = m_pImp->nonCdmi_UpdateDataObject(uri, location,  contentType,  startOffset,  objectSize, buf, len,  partial,  direct);

		if(nret >=200 && nret <= 300)
		{
			rlocation->Remove(0, rlocation->Length);
			rlocation->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)location.c_str())));
		}

		if(buf)
			delete []buf;*/

		IntPtr p = Marshal::UnsafeAddrOfPinnedArrayElement(rbuffer, 0);
		int nret = m_pImp->nonCdmi_CreateDataObject(uri, contentType, (char*)p.ToPointer(),len);

		if(nret >=200 && nret <= 300)
		{
			rlocation->Remove(0, rlocation->Length);
			rlocation->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)location.c_str())));
		}

		return nret;
	}

	int   AquaClientRef::cdmi_DeleteDataObject(StringBuilder^ rjsonResult, String^ ruri)
	{
		std::string jsonResult, uri;
		ManagedString2UnmanagedStringA(ruri, uri);

		int nret =  m_pImp->cdmi_DeleteDataObject(jsonResult, uri);

		if(nret >=200 && nret <= 300)
		{
			rjsonResult->Remove(0, rjsonResult->Length);
			rjsonResult->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)jsonResult.c_str())));
		}

		return nret;
	}
	int   AquaClientRef::nonCdmi_DeleteDataObject(String^ ruri)
	{
		std::string uri;
		ManagedString2UnmanagedStringA(ruri, uri);
		return m_pImp->nonCdmi_DeleteDataObject(uri);
	}
	int   AquaClientRef::cdmi_CreateContainer(StringBuilder^ rjsonResult, String^ ruri,String^ rjsonMetadata,
		String^ rjsonExports, String^ rdomainURI, String^ rdeserialize, 
		String^ rcopy, String^ rmove, String^ rreference, 
		String^ rdeserializeValue)
	{

		std::string jsonResult, uri,  jsonMetadata,jsonExports,  domainURI;
		std::string deserialize, copy,  move,  reference, deserializeValue;

		ManagedString2UnmanagedStringA(ruri, uri);
		ManagedString2UnmanagedStringA(rjsonMetadata, jsonMetadata);
		ManagedString2UnmanagedStringA(rjsonExports, jsonExports);
		ManagedString2UnmanagedStringA(rdomainURI, domainURI);
		ManagedString2UnmanagedStringA(rdeserialize, deserialize);
		ManagedString2UnmanagedStringA(rcopy, copy);
		ManagedString2UnmanagedStringA(rmove, move);
		ManagedString2UnmanagedStringA(rreference, reference);
		ManagedString2UnmanagedStringA(rdeserializeValue, deserializeValue);

		int nret = m_pImp->cdmi_CreateContainer(jsonResult, uri, jsonMetadata,
			jsonExports, domainURI,  deserialize, copy,  move,  reference,  deserializeValue);


		if(nret >=200 && nret <= 300)
		{
			rjsonResult->Remove(0, rjsonResult->Length);
			rjsonResult->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)jsonResult.c_str())));
		}
		
		return nret;
	}
	int   AquaClientRef::nonCdmi_CreateContainer( String^ ruri)
	{
		std::string uri;
		ManagedString2UnmanagedStringA(ruri, uri);
		return m_pImp->nonCdmi_CreateContainer(uri);
	}
	int   AquaClientRef::cdmi_ReadContainer(StringBuilder^ rjsonResult, String^ ruri)
	{
		std::string jsonResult, uri;
		ManagedString2UnmanagedStringA(ruri, uri);

		int nret = m_pImp->cdmi_ReadContainer(jsonResult, uri);

		if(nret >=200 && nret <= 300)
		{
			rjsonResult->Remove(0, rjsonResult->Length);
			rjsonResult->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)jsonResult.c_str())));
		}

		return nret;
	}
	int   AquaClientRef::cdmi_UpdateContainer(StringBuilder^ rjsonResult, String^ ruri, String^ rlocation,
		String^ rjsonMetadata, String^ rjsonExports,
		String^ rdomainURI, String^ rdeserialize, String^ rcopy,
		String^ rsnapshot, String^ rdeserializeValue)
	{
		std::string jsonResult, uri, location,jsonMetadata, jsonExports;
		std::string	domainURI, deserialize, copy,snapshot, deserializeValue;

		ManagedString2UnmanagedStringA(ruri, uri);
		ManagedString2UnmanagedStringA(rlocation, location);
		ManagedString2UnmanagedStringA(rjsonMetadata, jsonMetadata);
		ManagedString2UnmanagedStringA(rjsonExports, jsonExports);
		ManagedString2UnmanagedStringA(rdomainURI, domainURI);
		ManagedString2UnmanagedStringA(rdeserialize, deserialize);
		ManagedString2UnmanagedStringA(rcopy, copy);
		ManagedString2UnmanagedStringA(rsnapshot, snapshot);
		ManagedString2UnmanagedStringA(rdeserializeValue, deserializeValue);

		int nret = m_pImp->cdmi_UpdateContainer(jsonResult, uri, location,jsonMetadata, jsonExports,domainURI,
			deserialize,  copy, snapshot,  deserializeValue);

		if(nret >=200 && nret <= 300)
		{
			rjsonResult->Remove(0, rjsonResult->Length);
			rjsonResult->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)jsonResult.c_str())));
		}
		return nret;
	}
	int AquaClientRef::cdmi_DeleteContainer(StringBuilder^ rjsonResult, String^ ruri)
	{
		std::string jsonResult, uri;
		ManagedString2UnmanagedStringA(ruri, uri);

		int nret =  m_pImp->cdmi_DeleteContainer(jsonResult, uri);

		if(nret >=200 && nret <= 300)
		{
			rjsonResult->Remove(0, rjsonResult->Length);
			rjsonResult->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)jsonResult.c_str())));
		}

		return nret;
	}
	int  AquaClientRef::nonCdmi_DeleteContainer(String^ ruri)
	{
		std::string uri;
		ManagedString2UnmanagedStringA(ruri, uri);
		return m_pImp->nonCdmi_DeleteContainer(uri);
	}
	int  AquaClientRef::cdmi_ReadAquaDomain(StringBuilder^ rjsonResult, String^ rdomainURI)
	{
		std::string jsonResult, domainURI;
		ManagedString2UnmanagedStringA(rdomainURI, domainURI);

		int nret = m_pImp->cdmi_ReadAquaDomain(jsonResult, domainURI);

		if(nret >=200 && nret <= 300)
		{
			rjsonResult->Remove(0, rjsonResult->Length);
			rjsonResult->Append(Marshal::PtrToStringAnsi(static_cast<IntPtr>((char*)jsonResult.c_str())));
		}

		return nret;
	}
	int  AquaClientRef::flushDataObject(String ^ ruri )
	{
		std::string uri;
		ManagedString2UnmanagedStringA(ruri, uri);
		return m_pImp->flushDataObject(uri);
	}

}
}
