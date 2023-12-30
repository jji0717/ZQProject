#ifndef __H_LSCCOMMON_HEADER_2644PU8OP_	
#define __H_LSCCOMMON_HEADER_2644PU8OP_
#include <map>
#include <string>
namespace lsc
{
	typedef std::map<std::string, std::string> StringMap;

	#define ClientRequestMetaData(_MetaData) "$CR." #_MetaData
///// LSC_MessageHeader
#define CRMetaData_LscStreamHandle           ClientRequestMetaData(LscStreamHandle) //uint32 
#define CRMetaData_LscTransactionId          ClientRequestMetaData(LscTransactionId) //uint8 
#define CRMetaData_LscStatusCode             ClientRequestMetaData(LscStatusCode) //uint8 
#define CRMetaData_LscVersion                ClientRequestMetaData(LscVersion) //uint8 
#define CRMetaData_LscOpCode                 ClientRequestMetaData(LscOpCode) //uint8 

//// LSC_Response
#define CRMetaData_ResponseCurrentNpt     ClientRequestMetaData(ResponseCurrentNpt) //uint32 
#define CRMetaData_ResponseNumerator      ClientRequestMetaData(ResponseNumerator ) //uint16 
#define CRMetaData_ResponseDenominator      ClientRequestMetaData(ResponseDenominator) //int16 
#define CRMetaData_ResponseMode           ClientRequestMetaData(ResponseMode) //uint8 

//// LSC_Play,  LSC_Resume
#define CRMetaData_PlayStartNpt         ClientRequestMetaData(PlayStartNpt) //uint32 
#define CRMetaData_PlayStopNpt            ClientRequestMetaData(PlayStopNpt) //uint32 
#define CRMetaData_PlayNumerator           ClientRequestMetaData(PlayNumerator ) //int16 
#define CRMetaData_PlayDenominator          ClientRequestMetaData(PlayDenominator ) //uint16 

//// LSC_Pause
#define CRMetaData_PauseStopNpt           ClientRequestMetaData(PauseStopNpt) //uint32 

//// LSC_status

} // end namespace
#endif // __H_LSCCOMMON_HEADER_2644PU8OP_