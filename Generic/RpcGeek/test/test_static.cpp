#include "RpcGeekClient.h"
#include "RpcGeekServer.h"

class myClient : public ZQ::RpcGeek::Client
{
public:
	myClient(const char* serverUrl, const char* clientName =CLIENT_NAME, const char* protocolVer =PROTOCOL_VERSION)
		:Client(serverUrl){}
	~myClient(){}

	virtual void OnDefaultAsynResponse(const char* methodName, ZQ::common::Variant& params, ZQ::common::Variant& result, const int faultcode)
	{
		int v = result;
		printf ("\n%s(%d, %d) = %3d", methodName, (int)params[0], (int)params[1], v);
	}

};

class Sample : public ZQ::RpcGeek::SkelHelper
{
public:
	Sample(ZQ::RpcGeek::Server& skel) : SkelHelper(skel) {}
	virtual ~Sample() {}
	
	BEGIN_SKELHELPER_METHODS()
		SKELHELPER_METHOD(Sample, add)
		SKELHELPER_METHOD(Sample, sub)
		SKELHELPER_METHOD(Sample, mul)
		//	REG_METHOD(Callee, div)
	END_SKELHELPER_METHODS()
		
	virtual void add(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host)
	{
		result = ZQ::common::Variant((int)params[0] + (int)params[1]);
	}

	virtual void sub(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host)
	{
		result = ZQ::common::Variant((int)params[0] - (int)params[1]);
	}

	virtual void mul(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host)
	{
		result = ZQ::common::Variant((int)params[0] * (int)params[1]);
	}
};

void main()
{
	ZQ::RpcGeek::Server skel("http://localhost:2999/RPC2");
	Sample sample(skel);

	skel.serv();

	::Sleep(1000);

	myClient cs("http://localhost:2999/RPC2");
	
	for (int i =0; i<1000; i++)
	{
		ZQ::common::Variant param, result;
		int a= ((int)(94.23556 *i)) %20;
		int b= ((int)(74.6433 *i)) %30;
		
		param.set(0, ZQ::common::Variant(a));
		param.set(1, ZQ::common::Variant(b));
		
		cs.call_async("Sample.add", param);
		cs.call_async("Sample.sub", param);
		cs.call_async("Sample.mul", param);
	}

	::Sleep(10000);
}
