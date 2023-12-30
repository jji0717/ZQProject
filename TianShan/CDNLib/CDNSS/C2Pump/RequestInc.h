#ifndef _C2_REQUEST_INC_H
#define _C2_REQUEST_INC_H

//error code for response
#define C2_ERROR_BAD_RESPONSE_CODE					-1
#define C2_ERROR_BAD_RESPONSE_STRING				"Bad Response"
#define C2_ERROR_LOCATE_FAILED_CODE					-2
#define C2_ERROR_LOCATE_FAILED_STRING			    "Locate Request Failed"
#define C2_ERROR_GET_FAILED_CODE					-3
#define C2_ERROR_GET_FAILED_STRING					"Get Request Failed"
#define C2_ERROR_TIMEOUT_CODE						-4
#define C2_ERROR_TIMEOUT_STRING						"Timeout"

#define C2CLIENT_URI								"URI"
#define C2CLIENT_HOST								"HOST"
#define C2CLIENT_UserAgent							"User-Agent"
#define C2CLIENT_ContentType						"Content-Type"
#define C2CLIENT_ContentLength						"Content-Length"
#define C2CLIENT_TransferPort						"TransferPort"
#define C2CLIENT_TransferID							"TransferID"
#define C2CLIENT_PortNum							"PortNum"
#define C2CLIENT_TransferTimeout					"TransferTimeout"
#define C2CLIENT_AvailableRange						"AvailableRange"
#define C2CLIENT_OpenForWrite						"OpenForWrite"
#define C2CLIENT_AssetID							"AssetID"
#define C2CLIENT_ProviderID							"ProviderID"
#define C2CLIENT_SubType							"SubType"
#define C2CLIENT_ObjectID							"ObjectID"
#define C2CLIENT_TransferRate						"TransferRate"
#define C2CLIENT_IngressCapacity					"IngressCapacity"
#define C2CLIENT_ClientTransfer						"ClientTransfer"
#define C2CLIENT_ExclusionList						"ExclusionList"
#define C2CLIENT_Range								"Range"
#define C2CLIENT_TransferDelay						"TransferDelay"

#define C2CLIENT_Recording							"recording"
#define C2CLIENT_PlayTime							"playTime"
#define C2CLIENT_MuxBitrate							"muxBitrate"
#define C2CLIENT_ExtName							"extName"
#define C2CLIENT_StartOffset						"startOffset"
#define C2CLIENT_EndOffset							"endOffset"

#define C2CLIENT_GET_REQUEST_Transfer_Delay			"Transfer-Delay"
#define C2CLIENT_GET_REQUEST_Ingress_Capacity		"Ingress-Capacity"

#endif // _C2_REQUEST_INC_H