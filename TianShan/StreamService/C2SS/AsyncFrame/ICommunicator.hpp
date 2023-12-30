#ifndef __I_COMMUNICATOR_HPP__
#define __I_COMMUNICATOR_HPP__

#include <list>
#include <map>
#include <vector>
#include <string>
#include "common_async.hpp"

#include "Locks.h"
#include "NativeThread.h"
//#include "FileLog.h"
#include "Log.h"
#include "Pointer.h"


#ifdef ZQ_OS_MSWIN
#include <MSWSock.h>
#endif//ZQ_OS_MSWIN

struct Message;
class  IMgr;
class  IAsyncServer;
class  IAsyncClientHandler;
class  IAsyncClientFactory;

class ICommuncator : public ZQ::common::SharedObject
{
public:
	typedef ZQ::common::Pointer<ICommuncator>  Ptr;

	ICommuncator(){}
	virtual     ~ICommuncator(){};

	virtual		int		onAsyncIn(Message* acceptMsg) = 0;
	virtual		int		onAsyncOut(Message* acceptMsg){return true;}
	
	virtual		int		doRecvSync (char* buffer, unsigned int bufSize, int32 timeout = -1){return false;};
	virtual		int		doRecvAsync(Message* recvMsg){return false;};

	virtual		int		doSendSync (const char* buffer, unsigned int bufSize, int32 timeout = -1){return false;};
	virtual		int		doSendAsync(Message* sendMsg){return false;};
					
	virtual		int		onClose (Message* closeMsg){return true;}
	virtual		int		onError(Message* errMsg){return true;};
};




class CLASSINDLL_CLASS_DECL AcceptCommuncator: public ICommuncator, public ZQ::common::NativeThread
{
public:
	AcceptCommuncator(IAsyncServer* asyncServer, IMgr* mgr, IAsyncClientFactory* asyncClientFactory, int poster);
	virtual ~AcceptCommuncator();

	virtual		int onAsyncIn(Message* acceptMsg)
	{
		return this->onAccept(acceptMsg);
	}

	virtual		int onAccept(Message* acceptMsg);
	virtual		int onError(Message* errMsg);
	virtual		int onClose(Message* errMsg);

	virtual		int		init(const char* addr, unsigned int port);
	virtual		int		unInit();

private:
	int  postAccept(int listenSock, Message* nextAcceptMsg);
	int  preAccept (const char* addr, unsigned int port);

	int  run(void);

private:
#ifdef DEBUG
	long                _clientCount;
#endif
	int                          _listenSock;
	std::list<Message*>          _msgListen;
	
#ifdef ZQ_OS_MSWIN
	HANDLE                       _hAcceptEvent; 
	LPFN_ACCEPTEX                _lpfnAcceptEx;                // AcceptEx pfn
	LPFN_GETACCEPTEXSOCKADDRS    _lpfnGetAcceptExSockAddrs;    // GetAcceptExSockaddrs pfn
#endif//ZQ_OS_MSWIN

	int                  _quit;
	IMgr*                _mgr;
	int                  _maxWorker;
	IAsyncServer*        _asyncServer;
	IAsyncClientFactory* _asyncClientFactory;
	ZQ::common::Log&     _log;
};

class CommonTcpCommuncator: public ICommuncator
{
public:
	typedef ZQ::common::Pointer<CommonTcpCommuncator>  Ptr;

	CommonTcpCommuncator(IAsyncServer* asyncServer, IMgr* mgrWin, Message* msgHold);
	virtual ~CommonTcpCommuncator();
	virtual		int         onAsyncIn(Message* inMsg);
	virtual		int         onAsyncOut(Message* outMsg);

	//data in
	virtual		int  onRecvSync (char* buffer, size_t bufSize);
	virtual		int  onRecvAsync(Message* recvMsg);
	virtual		int  onSendSync (const char* buffer, size_t bufSize);
	virtual		int  onSendAsync(Message* sendMsg);
	virtual		int  onClose (Message* closeMsg);
	virtual		int  onError(Message* errMsg);

	//data out
	virtual		int		doRecvSync (char* buffer, unsigned int bufSize, int32 timeout = -1);
	virtual		int		doRecvAsync(Message* recvMsg);
	virtual		int		doSendSync (const char* buffer, unsigned int bufSize, int32 timeout = -1);
	virtual		int		doSendAsync(Message* sendMsg);

	virtual		int		init(const char* addr = NULL, unsigned int port = 0);
	virtual		int		unInit();

private:
	friend class ConnectCommuncator;
	typedef struct _MessageChain_
	{
		int                    _defSock;
		Message*               _defASendTrigger;
		Message*               _defARecvTrigger;
	    IAsyncClientHandler*   _defHandle;
		std::list<Message*>    _msgList;
		ZQ::common::Mutex      _msgChainMutex;
	} MsgChain;

	MsgChain             _msgChain;
	IMgr*                _mgr;
	IAsyncServer*        _asyncServer;
	int                  _isManualShutdown;

	ZQ::common::Log& _log;
};

class CLASSINDLL_CLASS_DECL ConnectCommuncator : public CommonTcpCommuncator
{
public:
	typedef ZQ::common::Pointer<ConnectCommuncator>  Ptr;

	ConnectCommuncator(IAsyncServer* asyncServer, IMgr* mgr, Message* msgHold);
	virtual ~ConnectCommuncator();
    
	virtual int init(const char* sockDst, unsigned int port);

private:
	int connectTo(const char* sockDst, unsigned int port);

private:
	Message*     _connHold;
	IMgr*        _mgr;
	std::string  _sockDst;
	unsigned int _port;

	ZQ::common::Log&  _log;
};

#endif//__I_COMMUNICATOR_HPP__