// HttpService.h: interface for the HttpService class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HTTPSERVICE_H__8F84D315_6A0D_49B1_883F_A0B4F9E47C56__INCLUDED_)
#define AFX_HTTPSERVICE_H__8F84D315_6A0D_49B1_883F_A0B4F9E47C56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <NativeThread.h>
#include <NativeThreadPool.h>
#include <ssllib/ssllib.h>

class HttpService: public ZQ::common::NativeThread
{
	friend class HttpRequest;
public:
	HttpService(ZQ::common::NativeThreadPool& threadPool, 
		const sockaddr& bindAddr);
	virtual ~HttpService();

	virtual bool init();
	virtual int run();
	virtual void final();

protected:
	class IceEnv {
	public:
		bool init()
		{
			int argc = 0;
			_ic = Ice::initialize(argc, NULL);
			return true;
		}

		virtual Ice::ObjectPrx getObject(const std::string& name);

	protected:
		Ice::CommunicatorPtr	_ic;
	};

protected:
	ZQ::common::NativeThreadPool&	_threadPool;
	sockaddr			_bindAddr;
	IceEnv				_iceEnv;
	ZQ::SSLSocket			_sock;
};

#endif // !defined(AFX_HTTPSERVICE_H__8F84D315_6A0D_49B1_883F_A0B4F9E47C56__INCLUDED_)
