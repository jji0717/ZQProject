#ifndef __Server_H__
#define __Server_H__

#include "RpcGeekCommon.h"
#include "Locks.h"
#include "PollingTimer.h"

namespace ZQ {
namespace RpcGeek {

class RPCGEEK_API Server;
class RPCGEEK_API ServerHelper;
class RPCGEEK_API ServerObject;
class RPCGEEK_API ServerObjectHelper;

/// -----------------------------
/// class ServerHelper
/// -----------------------------
#define BEGIN_SKELHELPER_METHODS() virtual bool execMethod(const char* host, const char* method_name, ZQ::common::Variant& params, ZQ::common::Variant& result) { if (NULL ==method_name) return false; if (0);
#define SKELHELPER_METHOD(_CLASS_NAME, _METHOD_NAME)  else if (0==strcmp(method_name, #_CLASS_NAME "." #_METHOD_NAME)) { if (!preProcess(params, result, host)) return true; _METHOD_NAME(params, result, host); postProcess(params, result, host); return true; }
#define SKELHELPER_METHOD2(_CLASS_NAME, _METHOD_NAME, _IMPL_MNAME)  else if (0==strcmp(method_name, #_CLASS_NAME "." #_METHOD_NAME)) { if (!preProcess(params, result, host)) return true; _IMPL_MNAME(params, result, host); postProcess(params, result, host); return true; }
#define END_SKELHELPER_METHODS() return false; }
#define END_SKELHELPER_METHODS2(_SUPER_CLASS) return _SUPER_CLASS::execMethod(host, method_name, params, result); }

class ServerHelper
{
	friend class Server;

public:
	ServerHelper(Server& skel);
	virtual ~ServerHelper();

	virtual bool preProcess(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host)
	{
		return true;
	}
	
	virtual void postProcess(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host)
	{
	}

	BEGIN_SKELHELPER_METHODS()
		SKELHELPER_METHOD(ServerHelper, hello)
//		SKELHELPER_METHOD(Callee, sub)
//		SKELHELPER_METHOD(Callee, mul)
//		SKELHELPER_METHOD(Callee, div)
	END_SKELHELPER_METHODS()
		
	virtual void hello(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host)
	{
	}
	
	Server& _skel;
};

/// -----------------------------
/// class Server
/// -----------------------------
class Server
{
public:
	Server(const char* servEndpoint, const int maxConn= DEFAULT_MAX_CONN);
	virtual ~Server();

	virtual void serv();
	virtual void stop();
	
private:
	friend class ServerHelper;
	friend class SkelNest;

	void addHelper(ServerHelper& helper);
	void removeHelper(ServerHelper& helper);

	SkelNest *_nest;
};

/// -----------------------------
/// class ServerObject
/// -----------------------------
class ServerObject : virtual public ZQ::common::PollingTimer
{
	friend class ServerObjectHelper;
	
public:
	ServerObject(ServerObjectHelper& helper, const ZQ::common::Guid& guid);

	virtual ~ServerObject();

	void renew(void);

//	const char* guidStr(void);
	
protected:
	ServerObjectHelper& _helper;
	ZQ::common::Guid _guid;
};

/// -----------------------------
/// class ServerObjectHelper
/// -----------------------------
class ServerObjectHelper : public ZQ::RpcGeek::ServerHelper
{
	friend class ServerObject;
	friend class ServerObjectMap;

public:

	ServerObjectHelper(ZQ::RpcGeek::Server& skel, timeout_t objLeaseTerm = DEFAULT_SERVOBJ_LEASETERM);
	virtual ~ServerObjectHelper();

	BEGIN_SKELHELPER_METHODS()
		SKELHELPER_METHOD(ServerObject, create)
		SKELHELPER_METHOD(ServerObject, release)
		SKELHELPER_METHOD(ServerObject, lookup)
	END_SKELHELPER_METHODS()

	size_t size(void);
	timeout_t getLeaseTerm(void);

	virtual isValid(void) { return NULL !=_pMap; }

	static void setError(ZQ::common::Variant& result, bool bReset, const char* errcode, const char* fmt, ...);

protected:
		
	virtual void postProcess(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host)
	{
		result.set(LEASE_TERM, (int) getLeaseTerm());
	}

	void create(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host);

	void release(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host);

	void lookup(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host);

protected:


	ServerObject* findObject(ZQ::common::Variant& params, ZQ::common::Guid& objGuid);

	virtual ServerObject* createEx(const ZQ::common::Guid& objGuid)
	{
		return new ServerObject(*this, objGuid);
	}

	virtual void releaseEx(ServerObject* pSO)
	{
		if (NULL != pSO)
			delete pSO;
	}

private:

	ServerObjectMap* _pMap;
	timeout_t		 _objLeaseTerm;

};

} // namespace RpcGeek
} // namespace ZQ

#endif __Server_H__
