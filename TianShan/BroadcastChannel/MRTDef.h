#ifndef __MRTStreamDef_h__
#define __MRTStreamDef_h__

#include <map>
#include <string>

typedef struct
{
	std::string mrtEndpoint;
	int connectTimeout;
	int sendTimeout;
	int receiverTimeout;
}MRTEndpointInfo;

typedef std::map<std::string, MRTEndpointInfo> StreamNetIDToMRTEndpoints;

#define SOAP_EndPoint    "soap_endpoint"
#define SOAP_SessionID   "soap_SessionID"
#define SOAP_AssetID     "soap_AssetID"
#define SOAP_StreamNetID "soap_StreamNetId"

#endif