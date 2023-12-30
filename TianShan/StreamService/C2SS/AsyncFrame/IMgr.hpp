#ifndef __I_MONITOR_HPP__
#define __I_MONITOR_HPP__


#include <algorithm>
#include <map>

#include "common_async.hpp"

#include "FileLog.h"

#include "ICommunicator.hpp"
#include "IAsyncServer.hpp"
#include "IAsyncClientHandler.hpp"

class IMgr
{
public:
	IMgr(unsigned int hashSize = 6)
		:_hashSize(hashSize), _asyncServer(0), _log(NULL)
	{
	}

	IMgr(ZQ::common::Log* log, unsigned int hashSize = 6)
		:_hashSize(hashSize), _asyncServer(0), _log(log)
	{
	}

	virtual ~IMgr()
	{
		if (0 != _asyncServer)
			_asyncServer = 0;
	}

	int attachAsyncServer(IAsyncServer* asyncServer)
	{
		if (0 != _asyncServer)
			return false;

		_asyncServer = asyncServer;
		return true;
	}

	virtual int  add(ICommuncator::Ptr val, IAsyncClientHandler::Ptr client);
	virtual int  lookup(ICommuncator::Ptr val);
	virtual int  remove(ICommuncator::Ptr val);

	virtual		void		doConnectTo(){return;}
	virtual		void		doAccept (Message* acceptMsg){return;}

	virtual		void		onCreate() = 0;
	virtual		void		onClose (Message* closeMsg, ICommuncator* closeConn) = 0;

	virtual		bool		onRecvSync (char* buffer, size_t bufSize) = 0;
	virtual		bool		onRecvAsync(Message* recvMsg) = 0;

	virtual		bool		onSendSync (const int8* buffer, size_t bufSize) = 0;
	virtual		bool		onSendAsync(Message* sendMsg) = 0;

	virtual		void		onError(Message* errMsg, ICommuncator* errConn) = 0;
	virtual		void		onIdle(void){return;}


public:
	ZQ::common::Log*   _log;

protected:
	IAsyncServer*     _asyncServer;
	unsigned int      _hashSize;
	
	typedef   std::map<ICommuncator::Ptr, IAsyncClientHandler::Ptr> Items;
	Items                 _conns;
	ZQ::common::Mutex     _connsMutex;

};

#endif//__I_MONITOR_HPP__