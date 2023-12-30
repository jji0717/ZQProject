#include "RpcGeekClient.h"

#include "RpcGeekUtils.h"
#include <xmlrpc-c/client.h>

#pragma comment(lib, "WinInet.lib")

namespace ZQ {
namespace RpcGeek {

/// -----------------------------
/// class ClientNest
/// -----------------------------
class  ClientNest
{
	friend class Client;
	Client& _owner;
	xmlrpc_env _env;
	xmlrpc_server_info* _pServer;
	
	//public:
	typedef struct _async_controlblock
	{
		void *userData;
		Client::ASYNC_EXEC_CALLBACK* callback;
		//		char buf[1024*1024];
	} async_controlblock;
	
	ClientNest(Client& owner, const char* clientName, const char* protocolVer)
		: _owner(owner), _pServer(NULL)
	{
		xmlrpc_env_init(&_env);
		if (!_bClientInitized)
		{
			// Create the Xmlrpc-c client object
			xmlrpc_client_init2(&_env, XMLRPC_CLIENT_NO_FLAGS, clientName, protocolVer, NULL, 0);
			if (!_env.fault_occurred)
				_bClientInitized = true;
		}
	}

	~ClientNest()
	{
		if (NULL != (_pServer))
			xmlrpc_server_info_free(_pServer);
		_pServer = NULL;
	}
	
	bool open(const char* serverEndpoint)
	{
		if (!_bClientInitized)
			return false;

		if (NULL != _pServer)
			xmlrpc_server_info_free(_pServer);
		_pServer = NULL;

		_pServer = (xmlrpc_server_info*) xmlrpc_server_info_new(&_env, serverEndpoint);

		return (!_env.fault_occurred && NULL != _pServer);
	}

	static void _cbAsynResponseEx(const char* server_url, const char* method_name, xmlrpc_value* param_array, void* user_data, xmlrpc_env* faultP, xmlrpc_value* resultP);
	static bool _bClientInitized;
};

bool ClientNest::_bClientInitized =false;

/// -----------------------------
/// class Client
/// -----------------------------
Client::Client(const char* clientName, const char* protocolVer)
:_nest(NULL)
{
	_nest = new ClientNest(*this, clientName, protocolVer);
	if (NULL == _nest)
        throw ZQ::common::IOException("Client() out of memory");
}

Client::~Client()
{
	if (_nest)
		delete _nest;
	_nest = NULL;
}

bool Client::open(const char* serverEndpoint)
{
	if (NULL == serverEndpoint || NULL == _nest)
		return false;

	return _nest->open(serverEndpoint);
}

bool Client::isValid()
{
	return (NULL != _nest->_pServer );
}


const char* Client::getError(void)
{
	if (_nest && _nest->_env.fault_occurred)
		return _nest->_env.fault_string;
	else return NULL;
}

bool Client::call_sync(const char* remoteMethodName, ZQ::common::Variant& paramArray, ZQ::common::Variant& result)
{
	if (!_nest)
		return false;

	result = ZQ::common::Variant();

	// convert Variant to xmlrpc_value
	XValueGuard params(_convert2xmlrpc_parameters(&(_nest->_env), paramArray));
	
	if (NULL == params.valueP)
	{
        fprintf(stderr, "failed to convert parameter to xmlrpc_value");
        return false; // TODO: throw exception
	}
	
	xmlrpc_value * pres = NULL;
	{
		ZQ::common::MutexGuard g(_calllock);
		// request the remote procedure call
		pres = xmlrpc_client_call_server_params(&(_nest->_env), (_nest->_pServer), remoteMethodName, params.valueP);;
	}

	if (_nest->_env.fault_occurred || NULL == pres)
		return false;
	XValueGuard resultvalue(pres);
	
	// convert result to Variant
	if (!_convert2variant(&(_nest->_env), resultvalue.valueP, result))
	{
        fprintf(stderr, "failed to convert xmlrpc_value result to variant");
        return false; // TODO: throw exception
    }
	
	return true;
}

bool Client::call_async(const char* remoteMethodName, ZQ::common::Variant& paramArray, ASYNC_EXEC_CALLBACK * cbResponse, void* userData)
{
	if (!_nest)
		return false;

	// convert Variant to xmlrpc_value
	XValueGuard params(_convert2xmlrpc_parameters(&(_nest->_env), paramArray));
	
	if (NULL == params.valueP)
	{
        fprintf(stderr, "failed to convert parameter to xmlrpc_value");
        return false; // TODO: throw exception
	}
	
	ClientNest::async_controlblock* pAsyncCB = new ClientNest::async_controlblock;
	if (NULL == pAsyncCB)
	{
        fprintf(stderr, "call_async() out of memory for asyncCB");
        return false;
	}
	
	memset(pAsyncCB, 0x00, sizeof(ClientNest::async_controlblock));
	pAsyncCB->userData = userData;
	pAsyncCB->callback = cbResponse;
	
	// make sure user_data is valid if callback is _cbDefaultAsynResponse
	if (NULL == userData && NULL == cbResponse)
	{
		pAsyncCB->userData = this;
		pAsyncCB->callback = _cbDefaultAsynResponse;
	}
	
	// request the remote procedure call
//	ZQ::common::MutexGuard g(_calllock);
	xmlrpc_client_call_server_asynch_params((_nest->_pServer), // server info
		remoteMethodName, // server method name
		ClientNest::_cbAsynResponseEx, // Response handler entry
		pAsyncCB,
		params.valueP);
	
	return true;
}

void ClientNest::_cbAsynResponseEx(const char* server_url, const char* method_name, xmlrpc_value* param_array, void* user_data, xmlrpc_env* faultP, xmlrpc_value* resultP)
{
	std::auto_ptr<async_controlblock> pAsyncCB ((async_controlblock*) user_data);

	if (NULL == pAsyncCB.get() || NULL == pAsyncCB->callback)
		return; // nothing to do without further callback
	
	int faultcode = 0;
	if (NULL == faultP)
		faultcode = -102;
	else if (faultP->fault_occurred)
		faultcode = faultP->fault_code;
	
	// converting the xmlvalues to variant
	ZQ::common::Variant result, params;
	if (0==faultcode && !_convert2variant(faultP, resultP, result))
	{
        fprintf(stderr, "_cbAsynResponse() failed to convert xmlrpc_value result to variant");
        faultcode = -100;
    }
	
	if (0==faultcode && !_convert2variant(faultP, param_array, params))
	{
        fprintf(stderr, "_cbAsynResponse() failed to convert xmlrpc_value param to variant");
        faultcode = -101;
    }
	
	pAsyncCB->callback(server_url, method_name, pAsyncCB->userData, params, faultcode, result);
}

void Client::_cbDefaultAsynResponse(const char* server_url, const char* method_name, void* user_data, ZQ::common::Variant& params, const int faultcode, ZQ::common::Variant& result)
{
	if (NULL == user_data)
	{
        printf("NULL user data\n");
		return;
	}
	
	Client* pStub = (Client*) user_data;
	
	pStub->OnDefaultAsynResponse(method_name, params, result, faultcode);
}

/// -----------------------------
/// class ProxyObject
/// -----------------------------
ProxyObject::ProxyObject(const char* endpoint, const char* serverClassname, const char* guidstr)
:_guid(guidstr), _soLeaseTerm(0)
{
	if (NULL != endpoint)
		_client.open(endpoint);

	if (_guid.isNil())
		_guid.create();

	_serverClassname = (serverClassname ? serverClassname: "ServerObject");

	ZQ::common::Variant params, result;
	initParams(params, result);

	bool succ = false;
	
	try
	{
		tstring errorcode;
		if (call((_serverClassname + ".lookup").c_str(), errorcode, params, result))
			succ = true;
		else if (0 == errorcode.compare(ERROR_NOTFOUND))
		{
			// if no playlist found, create a new one then
			result = ZQ::common::Variant();
			succ = call((_serverClassname + ".create").c_str(), errorcode, params, result);
		}
		
	}
	catch( ZQ::common::IOException e) { e.getString(); }
	catch( ... ) {}

//	if (!succ)
//		throw StreamSmithException("could not hook server playlist object");
}

bool ProxyObject::release()
{
	ZQ::common::Variant params, result;
	initParams(params, result);

	tstring errorcode;
	return call((_serverClassname + ".release").c_str(), errorcode, params, result);
}

void ProxyObject::initParams(ZQ::common::Variant& params, ZQ::common::Variant& result)
{
	const static ZQ::common::Variant nil;
	result = nil;
	params = nil;

	// prepare the header
	ZQ::common::Variant header;
	header.set(OBJ_GUID, _guid.toString().c_str());
	params.set(0, header);
}

bool ProxyObject::call(const char* methodname, tstring& errorcode, ZQ::common::Variant& params, ZQ::common::Variant& result)
{
	printf("calling %s()", methodname);
	TRACE_VAR(params);
	bool succ = call0(methodname, errorcode, params, result);
	printf("returned: ");
	TRACE_VAR(result);
	return succ;
}

bool ProxyObject::call0(const char* methodname, tstring& errorcode, ZQ::common::Variant& params, ZQ::common::Variant& result)
{
	if (NULL == methodname)
	{
		result = ZQ::common::Variant();
		result.set(ERROR_CODE, ERROR_REQUEST);
		result.set(ERROR_MSG, "null method name");
		return false;
	}

//	if (_endpoint.empty())
//	{
//		result = ZQ::common::Variant();
//		result.set(ERROR_CODE, ERROR_REQUEST);
//		result.set(ERROR_MSG, "no server endpoint specified");
//		return false;
//	}

	try
	{
//		Client _client(_endpoint.c_str());
		if (!_client.call_sync(methodname, params, result) || ZQ::common::Variant::T_STRUCT != result.type())
		{
			result = ZQ::common::Variant();
			const char* errmsg = _client.getError();
			if (NULL == errmsg)
				errmsg = "failed to execute remote method";
			result.set(ERROR_CODE, ERROR_RESPONSE);
			result.set(ERROR_MSG, errmsg);
			return false;
		}

		errorcode = result[ERROR_CODE];
		bool succ =(0 == errorcode.compare(ERROR_SUCC));
		if (succ)
			_soLeaseTerm = (int) result[LEASE_TERM];
		
		return succ;
	}
	catch(ZQ::common::IOException e)
	{
		errorcode = ERROR_IO_EXP;
		result = ZQ::common::Variant();
		result.set(ERROR_CODE, errorcode);
		result.set(ERROR_MSG, e.getString());
		return false;
	}
	catch(...)
	{
		errorcode = ERROR_IO_EXP;
		result = ZQ::common::Variant();
		result.set(ERROR_CODE, errorcode);
		result.set(ERROR_MSG, "Unknown Exception caught in ProxyObject::call()");
		return false;
	}	

	errorcode = ERROR_IO_FAIL;
	return false;
}

} // namespace RpcGeek
} // namespace ZQ
