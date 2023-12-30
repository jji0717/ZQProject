#ifndef _GB_CS_A4_CMD_
#define _GB_CS_A4_CMD_

#include <string>

#include "ContentImpl.h"

#include "GBCSportal.h"
#include "GBCSReq.h"



namespace ZQTianShan {
namespace ContentStore {
	
using namespace ZQTianShan::NGOD_CS;

class A4FilePropagationReq : public IGBA4Req
{
public:
	A4FilePropagationReq(const std::string & sourceUrl, const std::string & contentName, const ContentImpl& content, GBCSStorePortal* pPortalCtx, int maxTransferBitrate);

private:
	virtual std::map<std::string, std::string>  parseHttpResponse(std::string & httpResponse); 
	virtual std::string getCmdStr(void){return _a4FileProReq;};

	virtual std::string  makeContentHeader(void);
	virtual std::string  makeContentBody(void);

private:
	int  _maxTransferBitrate;

	std::string  _a4FileProReq;
	std::string  _sourceUrl;
	std::string  _contentName;

	const ContentImpl &     _content;
	GBCSStorePortal * _pPortalCtx;
};


class A4BatFilePropagationReq : public IGBA4Req
{
public:
	A4BatFilePropagationReq();

private:
	virtual std::map<std::string, std::string>  parseHttpResponse(std::string & httpResponse); 
	virtual std::string getCmdStr(void){return _a4CmdName;};

	virtual std::string  makeContentHeader(void);
	virtual std::string  makeContentBody(void);

private:
	std::string  _a4CmdName;
};


class A4FileStateReq : public IGBA4Req
{
public:
	A4FileStateReq(std::string & contentName, ContentImpl& content);

private:
	virtual std::map<std::string, std::string>  parseHttpResponse(std::string & httpResponse);
	virtual std::string getCmdStr(void){ return _a4FileStateReq;};

	virtual std::string  makeContentHeader(void);
	virtual std::string  makeContentBody(void);

private:
	std::string   _contentName;
	std::string   _a4FileStateReq;

	ContentImpl&  _content;
};


class A4FileStateNotify : public IGBA4Req
{
public:
	A4FileStateNotify();
	std::map<std::string, std::string>  parseHttpResponse(const std::string & httpResponse); 
	int setReturnCode(std::string returnCode);
	int setErrorMessage(std::string errorMessage);
	int setTransactionId(std::string transactionId);

private:
	virtual std::map<std::string, std::string>  parseHttpResponse(std::string & httpResponse);
	virtual std::string getCmdStr(void){ return _a4FileStateNotify;};

	virtual std::string  makeContentHeader(void);
	virtual std::string  makeContentBody(void);

private:
	std::string  _returnCode;
	std::string  _errorMessage;
	std::string  _transactionId;
	std::string  _a4FileStateNotify;
};


class A4FilePropagationCancel : public IGBA4Req
{
public:
	typedef enum _CancelReason
	{
		CANCEL_OPR_INIT  = 200, 
		EDN_OF_WIND      = 201, 
		CONTENT_DELETED  = 202,
		RIGHT_REVOKE     = 403,
		CONTENT_ILLEGAL  = 409,
		OTHER_REASON     = 500,
		REASON_COUNT_END
	} CancelReason;

public:
	A4FilePropagationCancel(std::string & contentName, ContentImpl& content, int reasonCode = CANCEL_OPR_INIT);

private:
	virtual std::map<std::string, std::string>  parseHttpResponse(std::string & httpResponse); 
	virtual std::string getCmdStr(void){ return _a4FileProCancel;};

	virtual std::string  makeContentHeader(void);
	virtual std::string  makeContentBody(void);

private:
	int          _reasonCode;
	std::string  _a4FileProCancel;
	std::string  _contentName;

	ContentImpl &      _content;
	GBCSStorePortal *  _pPortalCtx;
};

class A4FileDelete : public IGBA4Req
{
public:
	typedef enum _DeleteReason
	{
		DELETE_OPR_INIT  = 200, 
		END_OF_WIND      = 201, 
		RIGHT_REVOKE     = 403,
		CONTENT_ILLEGAL  = 409,
		OTHER_REASON     = 500,
		REASON_COUNT_END
	} DeleteReason;

public:
	A4FileDelete(std::string & contentName, const ContentImpl & content, int reasonCode = DELETE_OPR_INIT);

private:
	virtual std::map<std::string, std::string>  parseHttpResponse(std::string & httpResponse);
	virtual std::string getCmdStr(void){ return _a4FileDel;};

	virtual std::string  makeContentHeader(void);
	virtual std::string  makeContentBody(void);

private:
	int  _reasonCode;
	std::string    _a4FileDel;
	std::string    _contentName;

    const ContentImpl &  _content;
};

}//namespace  ContentStore
}//	namespace ZQTianShan 
#endif//_GB_CS_A4_CMD_