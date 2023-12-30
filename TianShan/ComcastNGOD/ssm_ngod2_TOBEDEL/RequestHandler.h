#ifndef __RequestHandler_H__
#define __RequestHandler_H__

#include "./NGODEnv.h"
#include "./SessionRenewCmd.h"
#include <HelperClass.h>

typedef std::map<std::string, std::string> STRINGMAP;
typedef STRINGMAP::iterator STRINGMAP_ITOR;
typedef STRINGMAP::const_iterator STRINGMAP_CITOR;

class RequestHandler : public Ice::Object
{
public:
	typedef IceUtil::Handle<RequestHandler> Ptr;

	RequestHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	
	virtual ~RequestHandler();

	virtual RequestProcessResult process() = 0;

public:

	enum ReturnType
	{
		RETURN_ASYNC	= 0,
		RETURN_SYNC		= 1
	};

protected:

	/// 
	bool getServerAddr(std::string& strServerIP, uint16& port);
	bool getRemoteAddr(std::string& strClientIP, uint16& port);

	/// add by zjm to support protocol versioning
	/// valid protocol in NgodVerStrs[from, to)
	/// @Param requireCode, return require protocol code
	/// @Param bRequired, Require header is required in rtsp message
	/// @Return, true if success, false if fail.
	bool handshake(int& requireCode, int from = 0, int to = 4, bool bRequired = true);

	// called by construction function to do some preparing operations
	int preprocess();
	
	// get the value of request header, if no specified header return an empty string
	std::string getRequestHeader(const char* pHeader, int logLevel = 7);

	void getContentBody(int logLevel = 7);

	// response function
	virtual int responseError( const char* statusCodeLine = NULL , const char* szNotice = NULL);

	int responseOK( );

	bool getContext();
	bool getContextByIdentity(::Ice::Identity& ident);
	bool getContextByIdentity(NGODr2c1::ctxData& pContext, NGODr2c1::ContextPrx& pContextPrx, ::Ice::Identity& ident);
	bool addContext();
	bool removeContext(const std::string& reason);
	//add by lxm for update NGOD spec
	bool updateContextProp(const std::string& key, const std::string& val);
	bool updateContextProp(::Ice::Identity& ident, const std::string& key, const std::string& val);

	bool getStream();
//	bool getPurchase();
	
	void changePostFlag(bool bFlag = false);
	
//	bool getWeiwooSession();
//	bool renewWeiwooSession(int timeout_value = -1);
//	bool destroyWeiwooSession(const std::string& reason);

//	void addPrivateData(std::string key, TianShanIce::Variant& var);
//	bool flushPrivateData();
	void destroyStream();

	bool getStreamState(TianShanIce::Streamer::StreamState& streamState, std::string& stateDept);
	bool getPositionAndScale(std::string& range, std::string& scale);

	bool renewSession(Ice::Long ttl = -1);

	

	void setReturnType( ReturnType type );


	bool checkStreamer( );

public:
	std::string getRequestType();
	std::string getSession();
	std::string getSequence();

	Ice::Long	getStartTime( );	
	ReturnType	getReturnType( );

	IServerResponse* getResponse( );

	void		setResponseString( const char* responseString , const char* noticeString = NULL );

	friend class	PlayHandlerAsync;
	friend class	pauseHandlerAsync;
	friend class	PlayResponseAsync;
protected:
		
	NGODEnv&					_ssmNGODr2c1;

	IClientRequestWriter*		_pRequest;

	std::string					_session;
	std::string					_method;
	std::string					_sequence;
	
	NGODr2c1::ctxData			_context;

protected:
	char						szBuf[MY_BUFFER_SIZE];
	uint16						szBufLen;

	std::string					_requestBody;



	IStreamSmithSite*			_pSite;
	
	IServerResponse*			_pResponse;
	
	

	std::string					_userAgent;
	std::string					_xreason;

	//NGODr2c1::ContextImplPtr _pContext;
	
	
	NGODr2c1::ContextPrx				_pContextPrx;
	
	TianShanIce::Streamer::PlaylistPrx mStreamPrx;
	

	bool						_postResponse;
	bool						_canProcess;

	STRINGMAP					_inoutMap;

	TianShanIce::ValueMap		_pdMap; // used to store server session's private data


	Ice::Long					timeStarted;

	ReturnType					_returnType; 

protected:

	std::string					_strResponse; // ?
	std::string					_strNotice;   // ?

	//add by zjm to support protocol versioning
	int                         _requireProtocol;
	static const char* NgodVerStrs[5];

};

// translate log into ssm_ngod2_events.log
#define HANDLEREVENTLOG _ssmNGODr2c1._sentryLog
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

#define REQUEST_STARTPOINT_IDX	"StartPointIdx"
#define REQUEST_STARTPOINT_OFFSET "StartointOffset"

#endif // __RequestHandler_H__

