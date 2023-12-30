#ifndef __MONITOR_WIN_IMPL_HPP__
#define __MONITOR_WIN_IMPL_HPP__

#include <winsock2.h>
#include <MSWSock.h>
#include <list>

#include "IMgr.hpp"
#include "Locks.h"
#include "NativeThread.h"

class MgrWinWorker;

class CLASSINDLL_CLASS_DECL MgrWinImpl : public IMgr
{
public:
	explicit  MgrWinImpl(unsigned int hashSize);
	explicit  MgrWinImpl(ZQ::common::Log* log, unsigned int hashSize);
	virtual   ~MgrWinImpl();

	virtual	 void  onConnectTo();
	virtual	 void  onAccept (Message* acceptMsg);

	virtual	 void  onCreate();
	virtual	 void  onClose (Message* closeMsg, ICommuncator* closeConn);

	virtual	 bool  onRecvSync (int8* buffer, size_t bufSize);
	virtual  bool  onRecvAsync(Message* recvMsg);

	virtual	 bool  onSendSync (const int8* buffer, size_t bufSize);
	virtual	 bool  onSendAsync(Message* sendMsg);

	virtual	 void  onError(Message* errMsg, ICommuncator* errConn);

private:
	int                          _maxWorker;

	std::list<Message*>          _msgAccept;
	ZQ::common::Mutex            _msgAcceptMutex;

	std::list<Message*>          _msgActive;
	ZQ::common::Mutex            _msgActiveMutex;

	std::list<Message*>          _msgDeActive;
	ZQ::common::Mutex            _msgDeActiveMutex;	

	std::list<MgrWinWorker*> _winWorks;

	std::list<ICommuncator*>     _connDeActive;
	ZQ::common::Mutex            _connDeActiveMutex;

	std::list<ICommuncator*>     _connActive;
	ZQ::common::Mutex            _connActiveMutex;

#ifdef DEBUG
	long                _clientCount;
#endif

	friend class MgrWinWorker;
	friend class AcceptCommuncator;
};

class MgrWinWorker : public ZQ::common::NativeThread
{
public:
	MgrWinWorker(MgrWinImpl& mgrWin, IAsyncServer& asyncServerWin);
	~MgrWinWorker();

	virtual  int32   run();
	virtual	 bool    start(void);
	virtual  void    stop(void) ;

private:
	uint32           _quit;
	MgrWinImpl&      _mgrWin;
	IAsyncServer&    _asyncServerWin; 
};

#endif//__MONITOR_WIN_IMPL_HPP__