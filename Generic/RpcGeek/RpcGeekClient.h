#ifndef __Client_H__
#define __Client_H__

#include "RpcGeekCommon.h"
#include "Locks.h"

namespace ZQ {
namespace RpcGeek {

class RPCGEEK_API Client;
class RPCGEEK_API ProxyObject;

/// -----------------------------
/// class Client
/// -----------------------------
class Client
{
	friend class  ClientNest;
public:
	Client(const char* clientName =CLIENT_NAME, const char* protocolVer =PROTOCOL_VERSION);
	virtual ~Client();
	const char* getError(void);
	
	bool open(const char* serverEndpoint);
	bool isValid();

	typedef void (ASYNC_EXEC_CALLBACK) (const char* server_url, const char* method_name, void* user_data, ZQ::common::Variant& params, const int faultcode, ZQ::common::Variant& result);
	
	// protected:
	
	virtual bool call_sync(const char* remoteMethodName, ZQ::common::Variant& paramArray, ZQ::common::Variant& result);
	
	virtual bool call_async(const char* remoteMethodName, ZQ::common::Variant& paramArray, ASYNC_EXEC_CALLBACK * cbResponse=NULL, void* userData=NULL);
	
	virtual void OnDefaultAsynResponse(const char* methodName, ZQ::common::Variant& paramArray, ZQ::common::Variant& result, const int faultcode) {}
	
	static void _cbDefaultAsynResponse(const char* server_url, const char* method_name, void* user_data, ZQ::common::Variant& params, const int faultcode, ZQ::common::Variant& result);
	
	ClientNest* _nest;
	ZQ::common::Mutex _calllock;
};

/// -----------------------------
/// class ProxyObject
/// -----------------------------
class ProxyObject
{
public:
	ProxyObject(const char* endpoint, const char* serverClassname ="ServerObject", const char* guidstr=NULL);
	virtual ~ProxyObject() {};

	void initParams(ZQ::common::Variant& params, ZQ::common::Variant& result);
	bool release();

protected:
	bool call(const char* methodname, tstring& errorcode, ZQ::common::Variant& params, ZQ::common::Variant& result);
	bool call0(const char* methodname, tstring& errorcode, ZQ::common::Variant& params, ZQ::common::Variant& result);

	ZQ::common::Guid   _guid; // unique id to identify this instance
	std::string        _serverClassname;
	timeout_t		   _soLeaseTerm;
	Client			   _client;
//	std::string        _endpoint; // client end to StreamSmith service
};

} // namespace RpcGeek
} // namespace ZQ

#endif // __Client_H__
