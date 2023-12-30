#if !defined(__ZQTIANSHAN_GBSVC_IOCP_THREAD_POOL_H__)
#define __ZQTIANSHAN_GBSVC_IOCP_THREAD_POOL_H__

#include "NativeThreadPool.h"
#include "SocketClientContext.h"
#include "HttpClientContext.h"
#include "Log.h"

namespace ZQTianShan {	  
namespace GBServerNS { 
	
class ReleaseResource: public ZQ::common::ThreadRequest 
{
public:
	explicit ReleaseResource(ZQ::common::NativeThreadPool& releasePool, ClientSocketContext* dataKey)
		:ZQ::common::ThreadRequest(releasePool), _dataKey(dataKey)
	{
	}

	virtual bool init(void)	{ return true; };

	virtual int run(void);

	virtual void final(int retcode, bool bCancelled)
	{
		delete this;
	}

private:
   ClientSocketContext* _dataKey;
};


class ProcessIncoming : public ZQ::common::ThreadRequest 
{
public:
	explicit ProcessIncoming(ZQ::common::NativeThreadPool& processPool, ZQ::common::NativeThreadPool& releasePool, ClientSocketContext* dataKey, ZQ::common::Log& log)
		:ZQ::common::ThreadRequest(processPool), _releasePool(releasePool), _dataKey(dataKey), _log(log)
	{
	}

	virtual bool init(void)	{ return true; };

	virtual int run(void);

	virtual void final(int retcode, bool bCancelled)
	{
		delete this;
	}

private:			
	ZQ::common::Log &     _log;
	ClientSocketContext * _dataKey;
	ZQ::common::NativeThreadPool& _releasePool;
};


class AssociateWithIOCP : public ZQ::common::ThreadRequest 
{
public:
	explicit AssociateWithIOCP(ZQ::common::NativeThreadPool& releasePool, ClientSocketContext* pContext, ZQ::common::Log& log)
		:ZQ::common::ThreadRequest(releasePool), _pContext(pContext), _log(log), _releasePool(releasePool)
	{
	}

	virtual bool init(void)	{ return true; };

	virtual int run(void);

	virtual void final(int retcode, bool bCancelled)
	{
		delete this;
	}

private:
	ZQ::common::Log &     _log;
	ClientSocketContext * _pContext;
	ZQ::common::NativeThreadPool& _releasePool;
};

class FeedBack : public ZQ::common::ThreadRequest 
{
public:
	explicit FeedBack(ZQ::common::NativeThreadPool& releasePool, XMLFileInf  contentInf, unsigned int ngbCmdCode, ZQ::common::Log& log)
		:ZQ::common::ThreadRequest(releasePool), _contentInf(contentInf), _ngbCmdCode(ngbCmdCode), _log(log)
	{
	}

	virtual bool init(void)	{ return true; };

	virtual int run(void);

	virtual void final(int retcode, bool bCancelled)
	{
		delete this;
	}

private:			
	unsigned int  _ngbCmdCode;
	XMLFileInf    _contentInf;

	ZQ::common::Log &     _log;
};


class ConnectDispatch : public ZQ::common::ThreadRequest 
{
public:
	ConnectDispatch(ZQ::common::NativeThreadPool& _connectPool, ZQ::common::NativeThreadPool& processPool, ZQ::common::NativeThreadPool& releasePool, HANDLE  hIOCompletionPort, ZQ::common::Log& log)
		:ZQ::common::ThreadRequest(_connectPool), _processPool(processPool), _releasePool(releasePool), _hIOCompletionPort(hIOCompletionPort), _log(log)
	{
	}

	virtual bool init(void);

	virtual int run(void);

	virtual void final(int retcode, bool bCancelled)
	{
		delete this;
	}

private:
	int     _running;
	HANDLE  _hIOCompletionPort;

	ZQ::common::Log &     _log;
	ZQ::common::NativeThreadPool& _releasePool;
	ZQ::common::NativeThreadPool& _processPool;
};

}//GBServerNS
}//	ZQTianShan

#endif//__ZQTIANSHAN_GBSVC_IOCP_THREAD_POOL_H__