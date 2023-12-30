#pragma once
#include "HttpClient.h"
#include "ContentOprtXml.h"
#include "Log.h"
#include <string>

namespace {

	const char* UNKNOWN = "Unknown";
	const char* PENDING = "Pending";
	const char* TRANSFER = "Transfer";
	const char* STREAMABLE = "Transfer/Play";
	const char* COMPLETE = "Complete";
	const char* CANCELED = "Canceled";
	const char* FAILED = "Failed";
	

	typedef enum {
		CANCEL_OPR_INIT=200, DIST_PENDING=201, NO_RIGHT=401, NOT_IN_SCHEDULE=404, DUP=409
	} CancelReason;

	typedef enum {
		DEL_OPR_INIT=200, END_OF_WIND=201, COLLECTION_DEL=202, REVOKED=403, INVALID=409
	} DeleteReason;
};

class A3Request
{
public:
	A3Request();
	A3Request(std::string& strHost, ZQ::common::Log* pLog = NULL);
	~A3Request(void);

	void setHost(std::string& strHost);
	void setLog(ZQ::common::Log* pLog);
public:
	
	int TransferContent(IN const TransferInfo& info);

	int GetTransferStatus(IN OUT TransferStatus& info);

	int CancelTransfer(const DeleteCancelContent& info);

	int ExposeContent(const ExposeContentInfo& info, ExposeResponse& response);

	int GetContentChecksum(IN const ContentChecksumInfo& info);

	int GetContentInfo(IN const ContentInfo& info, OUT std::vector<ContentInfo>& res);

	int DeleteContent(IN const DeleteCancelContent& info);

	int GetVolumeInfo(IN OUT VolumeInfo& info);

public:

	std::string getStatusMessage();

private:

	int SendRequest(IN const std::string& url, IN OUT std::string& buffer);
			

private:
	ContentOprtXml _xmlOp;
	ZQ::common::HttpClient _http;

	std::string _strHost;
	ZQ::common::Log* _pLog;	

};

