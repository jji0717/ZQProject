#ifndef __ZQ_CRM_A3MESSAGE_DEFINE_H_
#define __ZQ_CRM_A3MESSAGE_DEFINE_H_
#include  <string>
#include <vector>
#include <map>
#include "TsStorage.h"
namespace CRM
{
	namespace A3Message
	{
const std::string XML_HEADER = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
const std::string contentCategory = "A3Content";
const std::string A3EventCategory = "Content";
const std::string A3CreateEvent = "Created";
const std::string A3DestroyEvent = "Destroyed";
const std::string A3StateChangeEvent = "StateChanged";
const std::string EventField_Content = "content";
const std::string EventField_Volume = "volume";
const std::string EventField_Name = "name";
const std::string EventField_NetId = "netId";
const std::string EventField_NewStatus = "newState";
const std::string EventField_OldStatus = "oldState";
// A3 Content State
const std::string UNKNOWN = "Unknown";

///#define http request URL
#define URL_GETVOLUMEINFO        ".*/GetVolumeInfo"
#define URL_GETCONTENTINFO       ".*/GetContentInfo"
#define URL_TRANSFERCONTENT      ".*/TransferContent"
#define URL_CANCELTRANSFER       ".*/CancelTransfer"
#define URL_EXPOSECONTENT        ".*/ExposeContent"
#define URL_GETCONTENTCHECKSUM   ".*/GetContentChecksum"
#define URL_DELETECONTENT        ".*/DeleteContent"
#define URL_GETTRANSFERSTATUS    ".*/GetTransferStatus"

///#define http request key
#define Key_volumeName           "volumeName"
#define Key_providerID           "providerID"
#define Key_assetID              "assetID"
#define Key_contentSize          "contentSize"
#define Key_contentState         "contentState"
#define Key_createDate           "createDate"
#define Key_reasonCode           "reasonCode"
#define Key_state                "state"
#define Key_volumeSize           "volumeSize"
#define Key_freeSize             "freeSize"
#define Key_captureStart         "captureStart"
#define Key_captureEnd           "captureEnd"
#define Key_transferBitRate      "transferBitRate"
#define Key_sourceURL            "sourceURL"
#define Key_sourceIP             "sourceIP"
#define Key_sourceURL1           "sourceURL1"
#define Key_sourceIP1            "sourceIP1"
#define Key_responseURL          "responseURL"
#define Key_userName             "userName"
#define Key_password             "password"
#define Key_protocol             "protocol"
#define Key_ttl                  "ttl"
#define Key_md5Checksum          "md5Checksum"
#define Key_md5DateTime          "md5DateTime"
#define Key_resultCode           "vresultCode"
#define Key_contentSize          "contentSize"
#define Key_supportFileSize      "supportFileSize"
#define Key_netId                "netId"

///define content status
#define CONTENTSTATUS_PENDING         "Pending"
#define CONTENTSTATUS_TRANSFER        "Transfer"
#define CONTENTSTATUS_TRANSFER_PLAY   "Transfer/Play"
#define CONTENTSTATUS_COMPLETE        "Complete"
#define CONTENTSTATUS_CANCELED        "Canceled"
#define CONTENTSTATUS_FAILED          "Failed"
#define CONTENTSTATUS_UNKNOWN         "Unknown"
///define  Transfer Content http status  code
#define TCSTATUS_200		"Server accepted the distribution request"
#define TCSTATUS_400		"Invalid request format"
#define TCSTATUS_401		"Unauthorized (bad user name or password)"
#define TCSTATUS_403		"Cannot alter distribution parameters"
#define TCSTATUS_404		"Not Found"
#define TCSTATUS_409		"Resource conflict"
#define TCSTATUS_451		"Unsupported transfer protocol"
#define TCSTATUS_452		"Invalid Volume"
#define TCSTATUS_500		"Internal server error"

///define Get Transfer Status  http status  code
#define GTSSTATUS_200		"OK"
#define GTSSTATUS_404		"Unknown PAID"

///define Transfer Status http status  code
#define TSSTATUS_200		"OK"
#define TSSTATUS_404		"Unknown PAID"

///define Cancel Transfer http status code
#define CTSTATUS_200		"OK (Transfer canceled)"
#define CTSTATUS_404		"Unknown PAID"

///define Expose Content http status code
#define ECSTATUS_200		"OK (XML data returned)"
#define ECSTATUS_404		"Content Not Found"
#define ECSTATUS_409		"Content State Bad"
#define ECSTATUS_451		"Unsupported transfer protocol"
#define ECSTATUS_453		"Not Enough Bandwidth"

///define Get Content Checksum http status code
#define GCCSTATUS_200		"OK (Async request accepted)"
#define GCCSTATUS_404		"Content Not Found"

///define Content Checksum http status code
#define CCSTATUS_200		"OK"
#define CCSTATUS_404		"Unknown PAID"

///define Get Content Info http status code
#define GCISTATUS_200	"OK"
#define GCISTATUS_404	"Content Not Found"
#define GCISTATUS_500	"Internal server error"

///define Delete Content http status code
#define DCSTATUS_200		"OK"
#define DCSTATUS_404		"Content Not Found"
#define DCSTATUS_409		"File in use.  Retry later."

///define Get Volume info http status code
#define GVISTATUS_200		"OK"
#define GVITATUS_404		"Unknown Volume"

#define VOLUMESTATUS_200	"Operational"
#define VOLUMESTATUS_500	"Failed"
#define VOLUMESTATUS_501	"Out of Service"
#define VOLUMESTATUS_502	"Reduced Capacity"

///define Get Transfer Status reason code
#define GTSREASON_200		"OK"
#define GTSREASON_401		"Unauthorized"
#define GTSREASON_409		"Resource Conflict"
#define GTSREASON_500		"Internal Server Error"

///define Cancel Transfer reason code
#define CTREASON_200		"Operator initiated cancel"
#define CTREASON_201		"Higher Priority Distribution Pending"
#define CTREASON_401		"No capture rights"
#define CTREASON_404		"Asset no longer in schedule"
#define CTREASON_409		"Duplicate"

///define Delete Content reason code
#define DCREASON_200		"Operator initiated"
#define DCREASON_201		"End of Availability Window"
#define DCREASON_202		"Collection Deleted"
#define DCREASON_403		"Rights revoked"
#define DCREASON_409		"Content invalid (Bad size or MD5)"

///define Content Checksum result code
#define CTRESULT_200		"OK"
#define CTRESULT_404		"Content Not Found"
#define CTRESULT_409		"Content file invalid"

typedef std::map<std::string, std::string> StringMap;
typedef std::vector<std::string> StringVector;


/// convert contentState to A3 Content state
extern std::string convertState(TianShanIce::Storage::ContentState contentState);

/// convert content event state to A3 Content state
extern std::string eventStateToA3State(std::string strEventContentState);

/// convert TianShanIce::Storage::ContentState to TianShanIce::State
extern TianShanIce::State ContentStateToState(TianShanIce::Storage::ContentState contentState);

/// generate current UTC time
extern std::string GenerateUTCTime();
extern std::string  GetDeleteContentReason(int reasonCode);

typedef enum
{
  backContentLib = 0,
  backCacheStore = 1,
  backAuqaServer=2
}BackStoreType;

extern const char* backStoreTypeStr(BackStoreType type);
// netId
// volumeName
// contentName
// contentState
}
}
#endif//__ZQ_CRM_A3MESSAGE_DEFINE_H_

