#ifndef __RequestHandler_H__
#define __RequestHandler_H__

#include "./ssmNGODr2c1.h"
#include "./SessionRenewCmd.h"

typedef std::map<std::string, std::string> STRINGMAP;
typedef STRINGMAP::iterator STRINGMAP_ITOR;
typedef STRINGMAP::const_iterator STRINGMAP_CITOR;

class RequestHandler
{
public:
	RequestHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~RequestHandler();

	virtual RequestProcessResult process() = 0;

protected:
	// called by construction function to do some preparing operations
	int preprocess();
	
	// get the value of request header, if no specified header return an empty string
	std::string getRequestHeader(const char* pHeader, int logLevel = 7);
	void getContentBody(int logLevel = 7);

	// response function
	int responseError(const char* statusCodeLine, const char* szNotice = NULL);
	int responseOK();

	bool getContext();
	bool getContextByIdentity(::Ice::Identity& ident);
	bool getContextByIdentity(NGODr2c1::ContextImplPtr& pContext, NGODr2c1::ContextPrx& pContextPrx, ::Ice::Identity& ident);
	bool addContext();
	bool removeContext(const std::string& reason);

	bool getStream();
	bool getPurchase();
	
	void changePostFlag(bool bFlag = false);
	
	bool getWeiwooSession();
	bool renewWeiwooSession(int timeout_value = -1);
	bool destroyWeiwooSession(const std::string& reason);

	void addPrivateData(std::string key, TianShanIce::Variant& var);
	bool flushPrivateData();

	bool getStreamState(TianShanIce::Streamer::StreamState& streamState, std::string& stateDept);
	bool getPositionAndScale(std::string& range, std::string& scale);

	bool renewSession(Ice::Long ttl = -1);

public:
	std::string getRequestType();
	std::string getSession();

protected:
	char szBuf[MY_BUFFER_SIZE];
	uint16 szBufLen;

	std::string _requestBody;

	ssmNGODr2c1& _ssmNGODr2c1;

	IStreamSmithSite* _pSite;
	IClientRequestWriter* _pRequest;
	IServerResponse* _pResponse;
	
	std::string _session;
	std::string _method;
	std::string _sequence;

	std::string _userAgent;
	std::string _xreason;

	NGODr2c1::ContextImplPtr _pContext;
	NGODr2c1::ContextPrx _pContextPrx;

	TianShanIce::SRM::SessionPrx _sessionPrx;
	TianShanIce::Streamer::StreamPrx _streamPrx;
	TianShanIce::Application::PurchasePrx _purchasePrx;

	bool _postResponse;
	bool _canProcess;

	STRINGMAP _inoutMap;

	TianShanIce::ValueMap _pdMap; // used to store server session's private data

};

#define HANDLERLOG _ssmNGODr2c1._fileLog
#define HANDLERLOGFMT(_X, _T) CLOGFMT(_X, "Sess(%s)Seq(%s)Req(%p)Mtd(%s) " _T), _session.c_str(), _sequence.c_str(), _pRequest, _method.c_str()

class SmartRequestHandler
{
public:
	SmartRequestHandler(RequestHandler*& pHandler);
	virtual ~SmartRequestHandler();

private:
	RequestHandler*& _pHandler;
};

#endif // __RequestHandler_H__

