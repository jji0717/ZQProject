#ifndef _GB_CS_A5_CMD_
#define _GB_CS_A5_CMD_ 

#include "GBCSReq.h"
#include "GBCSportal.h"
#include "ContentImpl.h"

namespace ZQTianShan {
namespace ContentStore {

class A5StreamIngestReq : public IGBA5Req
{
public:
	A5StreamIngestReq(const std::string & sourceUrl, 
		const std::string & contentName, 
		ZQTianShan::ContentStore::ContentImpl& content,
		ZQTianShan::ContentStore::ContentStoreImpl & store, 
		int maxTransferBitrate, 
		const ::std::string& sourceType, 
		const ::std::string& startTimeUTC, 
		const ::std::string& stopTimeUTC,
		std::string & responseAddr);

	virtual ~A5StreamIngestReq(){}

	virtual std::string getCmdStr(void){return _a5StreamIngestReq;}

private:
	virtual std::string  makeContentBody(void);
	virtual std::string  makeContentHeader(void);

	std::string makeTransferInfoTagStart(void);

private:
	std::string  _a5StreamIngestReq;

	int   _maxTransferBitrate;

	std::string  _sourceType;
	std::string  _startTimeUTC;
	std::string  _stopTimeUTC;
	std::string  _sourceUrl;
	std::string  _contentName;
	std::string _responseAddr;

    ZQTianShan::ContentStore::ContentImpl &  _content;
    ZQTianShan::ContentStore::ContentStoreImpl & _store;
};

class A5StreamBatIngestReq : public IGBA5Req // TODO: n/a
{
public:
	A5StreamBatIngestReq();
	virtual ~A5StreamBatIngestReq(){}

	virtual std::map<std::string, std::string>  parseHttpResponse(std::string & httpResponse);
	virtual std::string getCmdStr(void){return _a5StreamBatIngestReq;}

private:
	virtual std::string  makeContentBody(void);
	virtual std::string  makeContentHeader(void);

private:
	std::string  _a5StreamBatIngestReq;
};


class A5StreamStateReq : public IGBA5Req
{
public:
	A5StreamStateReq(std::string & contentName, ContentImpl& content);
	virtual ~A5StreamStateReq(){}

	virtual std::map<std::string, std::string>  parseHttpResponse(std::string & httpResponse);
	virtual std::string getCmdStr(void){return _a5StreamStateReq;}

private:
	virtual std::string  makeContentBody(void);
	virtual std::string  makeContentHeader(void);

private:
	std::string _a5StreamStateReq;
	std::string _contentName;
	ZQTianShan::ContentStore::ContentImpl & _content;
};


class A5StreamStateNotify :	public IGBA5Req	// as same as A4FileStateReq
{
public:
	A5StreamStateNotify();
	virtual	~A5StreamStateNotify(){}

	virtual	std::map<std::string, std::string>	parseHttpResponse(std::string &	httpResponse);
	virtual	std::string	getCmdStr(void){return _a5StreamStateNotify;}

	int	setReturnCode(std::string returnCode);
	int	setErrorMessage(std::string	errorMessage);
	int	setTransactionId(std::string transactionId);

private:
	virtual	std::string  makeContentBody(void);
	virtual	std::string  makeContentHeader(void);

private:
	std::string	 _returnCode;
	std::string	 _errorMessage;
	std::string	 _transactionId;
	std::string	 _a5StreamStateNotify;
}; 	 

class A5StreamIngestCancel : public IGBA5Req
{
public:
	typedef enum _IngestCancelReason
	{
		CANCEL_OPR_INIT  = 200, 
		METADATA_DISTRIBUTE_FAIL = 400, 
		ASSET_OUT_OF_DATE = 404,
		REASON_COUNT_END
	} IngestCancelReason;

public:
	A5StreamIngestCancel(std::string & contentName, ContentImpl& content, int reasonCode = CANCEL_OPR_INIT);
	virtual ~A5StreamIngestCancel(){}

public:	
	//virtual std::map<std::string, std::string>  parseHttpResponse(std::string & httpResponse);// base is enough
	virtual std::string getCmdStr(void){return _a5StreamIngestCancel;}

private:
	virtual std::string  makeContentBody(void);
	virtual std::string  makeContentHeader(void);

private:
	std::string _a5StreamIngestCancel;
	std::string _contentName;
	int         _reasonCode;

	ZQTianShan::ContentStore::ContentImpl & _content;
};
}//namespace  ContentStore
}//	namespace ZQTianShan 
#endif//_GB_CS_A5_CMD_