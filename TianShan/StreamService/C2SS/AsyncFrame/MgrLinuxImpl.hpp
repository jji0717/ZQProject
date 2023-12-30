#ifndef __MONITOR_LINUX_IMPL_HPP__
#define __MONITOR_LINUX_IMPL_HPP__

#include "IMgr.hpp"
#include "NativeThread.h"
#include "Locks.h"

class MgrLinuxWorker;

class CLASSINDLL_CLASS_DECL MgrLinuxImpl : public IMgr
{
public:
	explicit  MgrLinuxImpl(/*IAsyncServer* asyncServer, */unsigned int hashSize);

	virtual ~MgrLinuxImpl();

	virtual		void		onConnect();
	virtual		void		onAccept (Message* acceptMsg);

	virtual		void		onCreate();
	virtual		void		onClose (Message* acceptMsg, ICommuncator* closeConn);

	virtual		bool		onRecvSync (int8* buffer, size_t bufSize );
	virtual		bool		onRecvAsync(Message* acceptMsg);

	virtual		bool		onSendSync (const int8* buffer, size_t bufSize );
	virtual		bool		onSendAsync(Message* acceptMsg);

	virtual		void		onError(Message* acceptMsg, ICommuncator* errConn);

	int   onStartUp();

private:
	int                          _maxWorker;

	std::list<Message*>          _msgAccept;
	ZQ::common::Mutex            _msgAcceptMutex;

	std::list<Message*>          _msgActive;
	ZQ::common::Mutex            _msgActiveMutex;

	std::list<Message*>          _msgDeActive;
	ZQ::common::Mutex            _msgDeActiveMutex;	

	std::list<MgrLinuxWorker*> _linuxWorks;
#ifdef DEBUG
	long                _clientCount;
#endif

	friend class MgrLinuxWorker;
};



class MgrLinuxWorker : public ZQ::common::NativeThread
{
public:
	MgrLinuxWorker(MgrLinuxImpl& mgrLinux, IAsyncServer& asyncServerLinux);
	~MgrLinuxWorker();

	virtual  int32   run();
	virtual	 bool    start(void);
	virtual  void    stop(void) ;

private:
	uint32           _quit;
	MgrLinuxImpl&    _mgrLinux;
	IAsyncServer&    _asyncServerLinux; 
};

#endif//__MONITOR_LINUX_IMPL_HPP__