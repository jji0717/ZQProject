// RtspDaemon.cpp: implementation of the RtspDaemon class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4786)

#include "../mainctrl/ScheduleTVServ.h"
#include <exception>
#include <time.h>
#include "RtspDaemon.h"
#include "RtspClient.h"


// this macro is used with FD_SET, FD_ZERO, FD_CLR, and FD_ISSET
// to copy all socket from one set to another
#ifndef FD_COPY 
#define FD_COPY(src, des) do { \
	u_int __i; \
	((fd_set FAR *)(des))->fd_count = ((fd_set FAR *)(src))->fd_count; \
	for (__i = 0; __i < ((fd_set FAR *)(src))->fd_count ; __i++) { \
		((fd_set FAR *)(des))->fd_array[__i]=((fd_set FAR *)(src))->fd_array[__i];\
	} \
} while(0)

#endif // FD_COPY

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool RtspDaemon::_trace = false;	// by default it is disabled

RtspDaemon::RtspDaemon(int daemonid)
{
	_daemonId	= daemonid;
	_dwMinHb	= RTSP_DEFAULT_HB;
	_hExit		= ::CreateEvent(NULL, false, false, NULL);
	_hWakeup	= ::CreateEvent(NULL, false, false, NULL);
	_bIsRunning	= true;
	_lastHbTime = 0;
	_clients.clear();
	_pTrace		= NULL;

	if(_trace)
	{
		wchar_t idbuf[24] = {0};
		std::wstring tracefile = ScheduleTVServ::m_wszTraceFile;
		size_t dotpos= tracefile.find_last_of(L".log");
		tracefile = tracefile.substr(0, dotpos-strlen(".log")+1);
		swprintf(idbuf, L"_daemon_trace%03d.log", daemonid);
		tracefile += idbuf;

		_pTrace = new ZQ::common::ScLog(tracefile.c_str(), ZQ::common::Log::L_DEBUG, 4*1024*1024);
	}
}

RtspDaemon::~RtspDaemon()
{
	if(isRunning())
	{
		signalStop();
		waitHandle(1000);
	}
		
	destroyAllClients();

	::CloseHandle(_hExit);
	::CloseHandle(_hWakeup);

	if(_pTrace)
	{
		delete _pTrace;
	}
}

RtspClient* RtspDaemon::createClient(DWORD purchaseid, bool createnew /* = false */)
{
	RtspClient*	newclient = NULL;

	trace("createClient(): Client (channel=%06d) is request to be created", purchaseid);

	if(!createnew)
	{	// check if exited client with this purchase id
		if(newclient = queryClientByPurchase(purchaseid))
		{
			trace("createClient(): Former client (channel=%06d) created", purchaseid);
			return newclient;
		}
	}

	// not found, or always create new
	newclient = new RtspClient(*this);
	newclient->purId() = purchaseid;

	// add envelop mask
	if(!regEnv(newclient, ENVMASK_NONE))
	{
		trace("createClient(): regEnv failed");
		delete newclient;
		return NULL;
	}

	trace("createClient(): New client (channel=%06d) created", purchaseid);
	return newclient;
}

bool	RtspDaemon::destroyClient(RtspClient* exclient)
{
	bool bRet;
	
	trace("destroyClient(): Client (channel=%06d) is request to be destroyed", exclient->purId());

	bRet = unregEnv(exclient);

	if(bRet)	// found, delete it
	{
		trace("createClient(): Former client (channel=%06d) destroyed", exclient->purId());
		delete	exclient;
	}
	
	trace("destroyClient():  unregEnv failed");
	return bRet;
}

int		RtspDaemon::destroyAllClients()
{
	int retnum=0;

	trace("destroyAllClients(): All clients are request to be destroyed");
	
	ZQ::common::MutexGuard	tmpGd(_cltLock);

	trace("destroyAllClients(): Got mutex");

	while(!_clients.empty())
	{
		cltEnv exEnv = _clients.back();
		if(exEnv.getClient())
		{
			exEnv.getClient()->close();
			trace("destroyAllClients(): Former client (channel=%06d) destroyed", exEnv.getClient()->purId());
			delete exEnv.getClient();
			retnum++;
		}
		_clients.pop_back();
	}

	trace("destroyAllClients(): Leave");
	return retnum;
}

RtspClient*	RtspDaemon::queryClientByPurchase(DWORD purchaseid)
{
	RtspClient*	queryclient = NULL;

	trace("queryClientByPurchase(): Search client of channel %06d", purchaseid);

	ZQ::common::MutexGuard	tmpGd(_cltLock);

	trace("queryClientByPurchase(): Got mutex");

	std::vector<cltEnv>::iterator iter = NULL;
	for(iter=_clients.begin(); iter!=_clients.end(); iter++)
	{
		if(iter->getClient()->purId() == purchaseid)
		{
			queryclient = iter->getClient();
			trace("queryClientByPurchase(): Found");
			break;
		}
	}
	
	trace("queryClientByPurchase(): Leave");
	return queryclient;
}

RtspClient* RtspDaemon::queryClientBySocket(SOCKET fd)
{
	RtspClient*	queryclient = NULL;

	trace("queryClientBySocket(): Search client with socket %d", fd);

	ZQ::common::MutexGuard	tmpGd(_cltLock);

	trace("queryClientBySocket(): Got mutex");

	std::vector<cltEnv>::iterator iter = NULL;
	for(iter=_clients.begin(); iter!=_clients.end(); iter++)
	{
		if(iter->getClient()->sd() == fd)
		{
			queryclient = iter->getClient();
			trace("queryClientBySocket(): Found");
			break;
		}
	}
	
	trace("queryClientBySocket(): Leave");
	return queryclient;
}

int		RtspDaemon::getClientNum()
{
	int	retnum=0;

	retnum = _clients.size();
	
	trace("getClientNum(): Totally %d client(s)", retnum);

	return retnum;
}

bool RtspDaemon::regEnv(RtspClient* client, int mask/* =ENVMASK_EXCP|ENVMASK_SEND */)
{
	if(NULL==client)
		return false;

	trace("regEnv(): Client (channel=%06d) is request to be registered", client->purId());

	ZQ::common::MutexGuard	tmpGd(_cltLock);

	trace("regEnv(): Got mutex");

	std::vector<cltEnv>::iterator iter = NULL;
	for(iter=_clients.begin(); iter!=_clients.end(); iter++)
	{
		if(iter->getClient()==client)
		{
			trace("regEnv(): Already existing, leave");
			return false;
		}
	}

	_clients.push_back(cltEnv(client, mask));
	glog(ZQ::common::Log::L_DEBUG, "<%03d>RtspDaemon::regEnv()  New client (sd=%d) registered, with list size %d", _daemonId, client->sd(), _clients.size());

	signalWakeup();	// call main loop to wakeup and handle new request

	trace("regEnv(): Leave");
	return true;
}

bool RtspDaemon::unregEnv(RtspClient* client)
{
	if(NULL==client)
		return false;

	trace("unregEnv(): Client (channel=%06d) is request to be unregistered", client->purId());

	ZQ::common::MutexGuard	tmpGd(_cltLock);

	trace("unregEnv(): Got mutex");

	std::vector<cltEnv>::iterator iter = NULL;
	for(iter=_clients.begin(); iter!=_clients.end(); iter++)
	{
		if(iter->getClient()==client)
		{
			_clients.erase(iter);
			trace("unregEnv(): Found, leave");
			glog(ZQ::common::Log::L_DEBUG, "<%03d>RtspDaemon::unregEnv()  Old client (sd=%d) unregistered, with list size %d", _daemonId, client->sd(), _clients.size());
			return true;
		}
	}

	trace("unregEnv(): Leave");
	return false;
}

bool RtspDaemon::updateEnv(RtspClient* client, int mask, bool append/* =false */)
{
	if(NULL==client)
		return false;

	trace("updateEnv(): Client (channel=%06d) is request to be updated with mask=%d", client->purId(), mask);

	ZQ::common::MutexGuard	tmpGd(_cltLock);

	trace("updateEnv(): Got mutex");

	std::vector<cltEnv>::iterator iter = NULL;
	for(iter=_clients.begin(); iter!=_clients.end(); iter++)
	{
		if(iter->getClient()==client)
		{
			trace("updateEnv(): Found");
			if(append)
			{
				iter->mask() = iter->mask() | mask;
			}
			else
			{
				iter->mask() = mask;
			}
			
			signalWakeup();	// call main loop to wakeup and handle new request
			trace("updateEnv(): Finish, leave");
			return true;
		}
	}

	trace("updateEnv(): Leave");
	return false;
}

//////////////////////////////////////////////////////////////////////////

void RtspDaemon::updateHb(DWORD hb)
{
	if( hb<_dwMinHb )
		_dwMinHb = hb;
}

void RtspDaemon::signalStop()
{
	::SetEvent(_hExit);
	_bIsRunning = false;
}

void RtspDaemon::signalWakeup()
{
	::SetEvent(_hWakeup);
}

//////////////////////////////////////////////////////////////////////////

bool RtspDaemon::init()
{
	_bIsRunning = true;
	
	return TRUE;
}

void RtspDaemon::final()
{
}

int RtspDaemon::run()
{
	int nRet = 0;

	fd_set	inFd, outFd, excFd;
	fd_set	inbak, outbak, excbak;
	fd_set	*pIn, *pOut, *pExc;

	time(&_lastHbTime);

	glog(ZQ::common::Log::L_NOTICE, "<%03d>RtspDaemon::run()  Enter Deamon loop", _daemonId);
	
	trace("run(): Enter daemon loop");
	for(;;)	// main loop
	{

		try
		{
			// check for exit signal
			if(!_bIsRunning)
			{
				trace("run(): Should stop");
				break;
			}

			//////////////////////////////////////////////////////////////////////////
			
			// set socket sets
			FD_ZERO(&inbak);
			FD_ZERO(&outbak);
			FD_ZERO(&excbak);

			int maxFd = INVALID_SOCKET;     // Not used on windows

			// set socket sets
			{
				time_t curr;
				time(&curr);	// update heartbeat send time if necessary
				bool sendHB = ((curr-_lastHbTime) >= (_dwMinHb*RTSP_DEFAULT_HB_FAC/1000));
				_lastHbTime = (sendHB)? curr : _lastHbTime;
				
				trace("run(): Begin scan sockets");

				ZQ::common::MutexGuard	tmpGd(_cltLock);

				trace("run(): Got mutex");

				if(_clients.size()!=0)
				{
					std::vector<cltEnv>::iterator iter = NULL;
					for(iter=_clients.begin(); iter!=_clients.end(); iter++)
					{
						SOCKET fd = iter->getClient()->sd();
						if((int)fd<=0)
							continue;
						if(iter->getClient()->status() < RtspClient::CLIENT_CONNECTED)
							continue;
						
						if(iter->mask() & ENVMASK_SEND) { FD_SET(fd, &outbak); }
						if(iter->mask() & ENVMASK_RECV) { FD_SET(fd, &inbak); }
						if(iter->mask() & ENVMASK_EXCP) { FD_SET(fd, &excbak); }
						if(iter->mask() && (int)fd > maxFd)	maxFd = fd;

						// sent heartbeat if necessary
						if(sendHB && (iter->getClient()->status() >= RtspClient::CLIENT_READY))
							iter->getClient()->handleAlive();
					}
				}

				trace("run(): Finish scan sockets");
			}
			
			// The following circle is to lower the thread load, while waiting for new client request
			// this circle calls select() for 10 times, with each 1000 msec, so this circle waits for total 10000 msec
			// this circle exit with the following situations:
			//	1. If signalStop() or signalWakeup() is called, exit circle and re-scan the client list
			//	2. If select() returns non-zero, exit circle, and continue following work
			//	3. If total timeout 10000 msec reached, exit circle, and re-scan the client list
			
			
			if(maxFd <=0 )
			{	// no socket in set

				trace("run(): No valid socket, sleep %d msec", RTSP_DEFAULT_FREQ*10);

				HANDLE	handles[2] = {_hWakeup, _hExit };
				DWORD waitRet=::WaitForMultipleObjects(2, handles, false, RTSP_DEFAULT_FREQ*10);
				if(WAIT_OBJECT_0+1	== waitRet) {	glog(ZQ::common::Log::L_NOTICE, "<%03d>RtspDaemon::run()  maxFd=0, Exit Deamon loop", _daemonId);	return 0; }	// exit daemon
				else if(WAIT_FAILED	== waitRet) {	glog(ZQ::common::Log::L_ERROR, "<%03d>RtspDaemon::run()  maxFd=0, ::WaitForMultipleObjects() error %d", _daemonId, GetLastError());}
				continue;
			}

			trace("run(): Begin monitor sockets");
			bool goon = true;
			for(int trytimes=0; trytimes<10 && goon; trytimes++)
			{
				int nEvents = -1;
				
				FD_ZERO(&inFd);
				FD_ZERO(&outFd);
				FD_ZERO(&excFd);
				
				FD_COPY(&inbak, &inFd);
				FD_COPY(&outbak, &outFd);
				FD_COPY(&excbak, &excFd);
				
				pIn		= (inFd.fd_count==0)?  (NULL) : (&inFd);
				pOut	= (outFd.fd_count==0)? (NULL) : (&outFd);
				pExc	= (excFd.fd_count==0)? (NULL) : (&excFd);
				
				struct timeval tv;
				tv.tv_sec = RTSP_DEFAULT_FREQ/1000;
				tv.tv_usec = RTSP_DEFAULT_FREQ%1000*1000;
				nEvents = select(maxFd+1, pIn, pOut, pExc, &tv);
				if (nEvents < 0)	{	goon = false; glog(ZQ::common::Log::L_ERROR, "<%03d>RtspDaemon::run()  select(pIn=%X, pOut=%X, pExc=%X) error %d", _daemonId, pIn, pOut, pExc, WSAGetLastError());}
				else if(nEvents>0)	{	goon = true; break;	}	// something happened in socket, go check it
			
				HANDLE	handles[2] = {_hWakeup, _hExit };
				DWORD waitRet=::WaitForMultipleObjects(2, handles, false, 50);
				if(WAIT_OBJECT_0	== waitRet)		{	goon = false;	break; }	// wakeup and re-scan to clients
				else if(WAIT_OBJECT_0+1	== waitRet) {	goon = false;	glog(ZQ::common::Log::L_NOTICE, "<%03d>RtspDaemon::run()  Exit Deamon loop", _daemonId);	return 0; }	// exit daemon
				else if(WAIT_FAILED		== waitRet) {	goon = false;	glog(ZQ::common::Log::L_ERROR, "<%03d>RtspDaemon::run()  ::WaitForMultipleObjects() error %d", _daemonId, GetLastError());	break; }
			
			}

			if(trytimes>=10)
				goon = false;
			
			trace("run(): Finish monitor sockets, goon=%d", goon);

			if(!goon)
				continue;	// jump to outer loop to re-scan the clients

			// Process events
			{
				trace("run(): Begin handle sockets");

				ZQ::common::MutexGuard	tmpGd(_cltLock);

				trace("run(): Got mutex");

				if(_clients.size()!=0)
				{
					std::vector<cltEnv>::iterator iter = NULL;
					for(iter=_clients.begin(); iter!=_clients.end(); iter++)
					{
						SOCKET fd = iter->getClient()->sd();

						trace("run(): Handle socket %d", fd);

						int	newMask = ULONG_MAX;
						bool bperform=true;
						if((int)fd<=0 || (int)fd>maxFd)
							continue;
						
						if(pExc && FD_ISSET(fd, &excFd))
						{
							trace("run(): Exception on socket %d", fd);
							newMask &= iter->getClient()->handleExcp();
						}

						if(pIn && FD_ISSET(fd, &inFd))
						{
							trace("run(): Receive on socket %d", fd);
							newMask &= iter->getClient()->handleRecv();
							bperform = false;	// after recv, do not perform anything
						}

						if(pOut && FD_ISSET(fd, &outFd) && bperform)
						{
							trace("run(): Send on socket %d", fd);
							newMask &= iter->getClient()->handleSend();
						}
						
						if(newMask!=ULONG_MAX)	/* changed already*/
						{
							iter->mask() = newMask;
						}
						
//						// close the client if necessary and has not been closed by itself
//						if((ENVMASK_NONE == newMask) && (RtspClient::CLIENT_DISCONNECT != iter->getClient()->status()) )
//						{
//							iter->getClient()->close();
//						}
					}
				}

				trace("run(): Finish handle sockets");
			}

			::Sleep(0);	// turn the thread opportunity to others


		}	// try
		catch(std::exception &e)
		{
		glog(ZQ::common::Log::L_CRIT, "<%03d>RtspDaemon::run()  Got std exception: %s", _daemonId, e.what());
		}
		catch(...)
		{ 
		glog(ZQ::common::Log::L_CRIT, "<%03d>RtspDaemon::run()  Got unknown exception", _daemonId);
		}

	
	}	// for(;;)
	
	return nRet;
}

void RtspDaemon::trace(const char *fmt, ...)
{
	if(!_trace)
	{
		return;	// trace flag not set, so do not log
	}

	char msg[2048]={0};
	va_list args;

	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	(*_pTrace)(ZQ::common::Log::L_DEBUG, "(TRACE) - %s", msg);
}