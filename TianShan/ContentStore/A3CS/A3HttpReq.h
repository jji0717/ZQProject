#ifndef __A3REQUEST_h__
#define __A3REQUEST_h__

#include "HttpClient.h"
#include "Log.h"
#include "XMLPreferenceEx.h"
#include "TianShanIce.h"
#include <string>

//content state
#define  A3_STATE_UNKNOWN		"Unknown"
#define  A3_STATE_PENDING		"Pending"
#define  A3_STATE_TRANSFER		"Transfer"
#define  A3_STATE_STREAMABLE	"Transfer/Play"
#define  A3_STATE_COMPLETE		"Complete"
#define  A3_STATE_CANCELED		"Canceled"
#define  A3_STATE_FAILED		"Failed"

//Streaming Server to APM message type
#define TYPE_TRANSFER	"TransferStatus"
#define TYPE_CHECKSUM	"ContentChecksum"

#define HTTP_SUCC(_CODE) (_CODE>=200 && _CODE<300)

namespace ZQTianShan {
namespace NGOD_CS {
class NGODStorePortal;
}};

class A3Request
{
public:
	A3Request();
	A3Request(ZQTianShan::NGOD_CS::NGODStorePortal& csp);
	virtual ~A3Request(void);

	void setHost(const std::string& strHost, const std::string& str2ndHost= std::string());
	void setLog(ZQ::common::Log* pLog);
public:
	typedef enum _VolumeState 
	{
		OPERATIONAL = 200,
		VOLUME_FAILED = 500,
		OUT_OF_SERVICE = 501,
		REDUCED_CAPACITY = 502
	}VolumeState;

	typedef enum _CancelReason
	{
		CANCEL_OPR_INIT=200, 
		DIST_PENDING=201, 
		NO_RIGHT=401, 
		NOT_IN_SCHEDULE=404, 
		DUP=409
	} CancelReason;

	typedef enum _DeleteReason
	{
		DEL_OPR_INIT=200, 
		END_OF_WIND=201, 
		COLLECTION_DEL=202, 
		REVOKED=403, 
		INVALID=409
	} DeleteReason;

	typedef enum _A3MsgType
	{
		A3_TransferContent,
		A3_GetTransferStatus,
		A3_CancelTransfer,
		A3_ExposeContent,
		A3_GetContentChecksum,
		A3_GetContentInfo,
		A3_DeleteContent,
		A3_GetVolumeInfo

	} A3MsgType;

	typedef struct _MessageCtx
	{
		int errorCode;
		::TianShanIce::Properties params;
		::std::vector< ::TianShanIce::Properties > table;
		_MessageCtx() : errorCode(-1){}

	} MessageCtx;

	//@timeout: set http timeout,unit millisecond
	int request(const A3MsgType type, MessageCtx& msgCtx, int timeout = -1);

	std::string getStatusMessage();

private:
	int transferContent(MessageCtx& msgCtx);

	int getTransferStatus(MessageCtx& msgCtx);

	int cancelTransfer(MessageCtx& msgCtx);

	int exposeContent(MessageCtx& msgCtx);

	int getContentChecksum(MessageCtx& msgCtx);

	int getContentInfo(MessageCtx& msgCtx);

	int deleteContent(MessageCtx& msgCtx);

	int getVolumeInfo(MessageCtx& msgCtx);


private:

	int sendRequest(const char* uri, IN OUT std::string& buffer, bool longRequest = false);

private:
	ZQ::common::HttpClient _http;

	std::string _strHost, _str2ndHost;
	ZQ::common::Log* _pLog;	

};

class ContentOprtXml 
{
public:
	ContentOprtXml(ZQ::common::Log* pLog=0);
	virtual ~ContentOprtXml();

public:
	void setLog(ZQ::common::Log* pLog);

	std::string makeTransferContent(A3Request::MessageCtx& msgCtx);
	
	//GetTransferStatus message
	std::string makeGetTransferStatus(A3Request::MessageCtx& msgCtx);
	bool parseGetTransferStatus(const char* buf, size_t bufLen,A3Request::MessageCtx& msgCtx);

	std::string makeCancelTransfer(A3Request::MessageCtx& msgCtx);
	std::string makeDeleteContent(A3Request::MessageCtx& msgCtx);
	
	//Expose message
	std::string makeExposeContent(A3Request::MessageCtx& msgCtx);
	bool parseExposeContent(const char* buf, size_t bufLen, A3Request::MessageCtx& msgCtx);

	std::string makeGetContentChecksum(A3Request::MessageCtx& msgCtx);

	//GetContentInfo message
	std::string makeGetContentInfo(A3Request::MessageCtx& msgCtx);
	bool parseGetContentInfo(const char *buf, size_t bufLen, A3Request::MessageCtx& msgCtx);

	//GetVolumeInfo message
	std::string makeGetVolumeInfo(A3Request::MessageCtx& msgCtx);
	bool parseGetVolumeInfo(const char *buf, size_t bufLen, A3Request::MessageCtx& msgCtx);
	
	bool parseContentChecksum(const char* buf, size_t bufLen, A3Request::MessageCtx& msgCtx);
private:
	std::string setAttrStr(const char* key, const char* value);
	std::string setAttrStr(const char* key, int value);

	bool getXmlRootItem(const char* buf, size_t bufLen, ZQ::common::XMLPreferenceDocumentEx& xmlDoc, ZQ::common::XMLPreferenceEx** pRoot);

private:
	ZQ::common::Log* _pContOprtLog;
};


#endif //__A3REQUEST_h__
