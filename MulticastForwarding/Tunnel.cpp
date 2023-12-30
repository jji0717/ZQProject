// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: Tunnel.cpp,v 1.10 2004/08/05 07:14:09 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : Tunnel connection and listener for the project of McastFwd
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/Tunnel.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 26    06-07-25 11:22 Hui.shao
// 
// 25    04-11-08 18:01 Hui.shao
// const paramters
// 
// 24    04-09-30 11:29 Kaliven.lee
// 
// 23    04-09-28 14:37 Hui.shao
// 
// 22    04-09-28 14:18 Hui.shao
// add/remove owner() operate again the bi-direct association
// 
// 21    04-09-16 20:03 Kaliven.lee
// add tunnel connection to owner's tunnel list
// 
// 20    04-09-16 17:37 Kaliven.lee
// modified lock in scan
// 
// 19    04-09-14 16:51 Hui.shao
// 
// 18    04-09-14 14:20 Hui.shao
// removed to include "mcastserv.h"
// 
// 17    04-09-13 15:51 Kaliven.lee
// add failover status for tunnel
// 
// 16    04-09-13 15:14 Kaliven.lee
// 
// 15    04-09-13 15:04 Kaliven.lee
// add a method to get current status of the tunnels
// 
// 14    04-09-09 17:46 Kaliven.lee
// 
// 13    04-09-09 10:30 Kaliven.lee
// 
// 12    04-09-07 16:25 Kaliven.lee
// 
// 11    04-08-27 15:52 Kaliven.lee
// create event when scaner construct instead of initialized
// 
// 10    04-08-27 9:56 Kaliven.lee
// add check isclosed when TunnelManager destruct
// 
// 9     04-08-26 17:42 Kaliven.lee
// 
// 8     04-08-26 10:26 Kaliven.lee
// add scan thread
// 
// 7     04-08-22 15:03 Jian.shen
// 
// 6     04-08-22 14:34 Jian.shen
// Revision 1.10  2004/08/05 07:14:09  shao
// comments
//
// Revision 1.9  2004/07/29 05:14:03  shao
// catch exp on sending
//
// Revision 1.8  2004/07/22 09:06:27  shao
// no message
//
// Revision 1.7  2004/07/22 06:17:48  shao
// adjusted some method/paramter names, and added comments
//
// Revision 1.6  2004/07/21 07:10:57  shao
// global logger
//
// Revision 1.5  2004/07/20 13:31:40  shao
// 2nd stage impl
//
// Revision 1.4  2004/07/12 09:08:32  shao
// unique conn approach
//
// Revision 1.3  2004/07/09 11:22:32  shao
// can compile
//
// Revision 1.2  2004/06/04 11:02:55  shao
// added listener
//
// Revision 1.1  2004/06/04 10:22:00  shao
// init created
//
// ===========================================================================
#include "Tunnel.h"

#define TUNLLOGFMT(_X) LOGFMT("tunnel[%s] " _X ), peeridstr()

// -----------------------------
// class TunnelConnection
// -----------------------------
// static members

std::string gsTunnelStatus[] = {"IDLE",
				"CONNECT",
				"HANDSHAKING",
				"READY",
				"ACTIVE",
				"CLOSED",
				"FAILOVER"};
int TunnelConnection::_lastid = 0;
ZQ::common::Guid TunnelConnection::_localid;
Conversation* TunnelConnection::_pDefaultConversation =NULL;

TunnelManager gTunnelManager;

const char* TunnelConnection::localidstr()
{
	static char _localidstr[64] = "";
	static bool _filled =false;
	if (!_filled)
	{
		_localid.toString(_localidstr, 60);
		_filled = !_localid.isNil();
	}

	return _localidstr;
}

TunnelConnection::TunnelConnection(const ZQ::common::FtHostAddress& bind, ConnectOptions_t* pOptions/*=NULL*/)
:ZQ::common::FtTcpConnection(TYPEINST_TYPE, TYPEINST_INST, pOptions), _bind(bind)
{
	_id =_lastid++;

	if (_localid.isNil())
		throw TunnelException("TunnelConnection::_localid is Nil");

	// register self in the list
	gTunnelManager.reg(this);

	_status = IDLE;

	// create connected event handle
	_eventReady = ::CreateEvent(NULL, true, false, NULL);
	if(_eventReady ==NULL)
		throw TunnelException("fail to create event handle");
	::ResetEvent(_eventReady);
}

TunnelConnection::TunnelConnection(TCP_CHANDLE handle)
:ZQ::common::FtTcpConnection(handle, &gTunnelManager), _eventReady(NULL)
{
	_id =_lastid++;

	if (handle == NULL)
		throw TunnelException("illegal paramters to initialize a server-side connection");

	if (_localid.isNil())
		throw TunnelException("TunnelConnection::_localid is Nil");

	// register self in the list
	gTunnelManager.reg(this);

	_status = IDLE;
}

/// destructor
TunnelConnection::~TunnelConnection()
{
	if (_eventReady!=NULL)
		::CloseHandle(_eventReady);
	_eventReady = NULL;

	//clean up the association with oweners
	ownerlist_t TmpOwners = _owners;
	for (ownerlist_t::iterator it = TmpOwners.begin(); it < TmpOwners.end(); it++)
		removeOwner(*it);

	close();

	// wait for the OnClosed() callback
	for(int i=0; i< 100 && isConnected(); i++)
		::Sleep(100);

	gTunnelManager.unreg(this);
}

std::string TunnelConnection::getStatus(void)
{
	return gsTunnelStatus[_status];
}

bool TunnelConnection::connect(const ZQ::common::FtHostAddress& remote, const int port, timeout_t timeout)
{
	log(ZQ::common::Log::L_DEBUG, LOGFMT("tunnel status is %d"), _status);
	if (_status != IDLE && _status != CLOSED)
	{
		log(ZQ::common::Log::L_DEBUG, LOGFMT("connection hasn't already been established, ignored"));
		return false;
	}

	if (!FtTcpConnection::connect((WORD) port, _bind, remote))
		return false;

	// blockable connect
	if (timeout >0 && _eventReady!=NULL)
	{
		// wait for timeout for handshaking
		DWORD ret = ::WaitForSingleObject(_eventReady, timeout);
		if (ret == WAIT_TIMEOUT || ret ==WAIT_ABANDONED)
		{
			FtTcpConnection::close();
			std::string addr0 = remote.getHostAddress(0);
			std::string addr1 = remote.getHostAddress(0);
			log(ZQ::common::Log::L_ERROR, LOGFMT("failed to establish connection to %s,%s:%d"), addr0.c_str(), addr1.c_str(), port);
			return false;
		}
	}
	

	
	log(ZQ::common::Log::L_INFO, TUNLLOGFMT("connection established"));

	return true;
}

// to make connect() blockable
void TunnelConnection::OnConnected(DWORD dwError, int iError)
{
	log(ZQ::common::Log::L_DEBUG, LOGFMT("error is %d,%d"), dwError, iError);

	_status = HANDSHAKING;

	//exchange guid
	if (!isPassive())
	{
		log(ZQ::common::Log::L_DEBUG, LOGFMT("client sending local guid: %s"), localidstr());
		send((BYTE*) &((GUID)_localid), sizeof(GUID));
	}

	if (isPassive())
		gTunnelManager.wakeupScanner();
}

void TunnelConnection::OnDataArrive(void *message, int len)
{
	log(ZQ::common::Log::L_DEBUG, LOGFMT("received %d bytes data"), len);

	if (_status == HANDSHAKING)
	{
		if (len < sizeof(GUID))
			return;
        
		// exchange guid
		if (isPassive())
		{
			// response with server guid
			log(ZQ::common::Log::L_DEBUG, LOGFMT("server sending local guid: %s"), localidstr());
			send((BYTE*) &((GUID)_localid), sizeof(GUID));
		}

		// the peer response with its installation id
		GUID id;
		memcpy(&id, message, sizeof(GUID));
		_peerid = id;
		log(ZQ::common::Log::L_DEBUG, LOGFMT("received peer guid: %s"), peeridstr());

		_status = READY;

		if (_eventReady)
			::SetEvent(_eventReady);

		return;
	}

	if (READY != _status && ACTIVE != _status)
	{
		log(ZQ::common::Log::L_DEBUG, LOGFMT("unexpected connection status to received data"));
		return;
	}

	log(ZQ::common::Log::L_DEBUG, LOGFMT("processing received data package"));
	if (len < sizeof(msg_header_t))
	{
		 // invalid data package
		log(ZQ::common::Log::L_DEBUG, LOGFMT("unknown data package received"));
		return;
	}

	msg_header_t header;
	memcpy(&header, message, sizeof(header));
	BYTE *data = ((BYTE*)message) + sizeof(header);
	int datalen = len - sizeof(header);

	// check version
	if (header.version != TUNNEL_VERSION)
	{
		 // invalid data package
		log(ZQ::common::Log::L_DEBUG, LOGFMT("unsupported data package version: %d"), header.version);
		return;
	}

	_status = ACTIVE; // make the connection active when the first valid package arrived

	if (header.type != 0)
	{
		log(ZQ::common::Log::L_DEBUG, LOGFMT("process command"));
		// TODO: commands
		return;
	}

	// process forwarded data
	log(ZQ::common::Log::L_DEBUG, LOGFMT("process data package"));
	try
	{
		int gport =0, sport=0;
		ZQ::common::InetMcastAddress group;
		ZQ::common::InetHostAddress source;
		
		// the mulitcast group info
		if (header.mcgroup.family ==PF_INET6)
		{
			gport =header.mcgroup.sa.a6.sin6_port;
			group = ZQ::common::InetMcastAddress(header.mcgroup.sa.a6.sin6_addr);
		}
		else if (header.mcgroup.family ==PF_INET)
		{
			gport =header.mcgroup.sa.a.sin_port;
			group = ZQ::common::InetMcastAddress(header.mcgroup.sa.a.sin_addr);
		}

		// the source sender info
		if (header.source.family ==PF_INET6)
		{
			sport =header.source.sa.a6.sin6_port;
			source = ZQ::common::InetHostAddress(header.source.sa.a6.sin6_addr);
		}
		else if (header.source.family ==PF_INET)
		{
			sport =header.source.sa.a.sin_port;
			source = ZQ::common::InetHostAddress(header.source.sa.a.sin_addr);
		}

		if (_owners.size() <=0 && _pDefaultConversation!=NULL) // this is an orphan, maybe accept by the tunnel server
			this->addOwner(_pDefaultConversation);

		// fire OnFowardedData() to each owner
		
		for (ownerlist_t::iterator it = _owners.begin(); it <_owners.end(); it++)
		{
			Conversation* pCvst = (*it);
			if (pCvst !=NULL)
			{
				pCvst->OnFowardedData(this, data, datalen, source, sport, group, gport);
			}
			else 
				log(ZQ::common::Log::L_DEBUG, LOGFMT("no conversation needed to forward the data."));
		}

	}
	catch(...) {
		log(ZQ::common::Log::L_ERROR,LOGFMT("Exception comming when process data package"));
	}
}

void TunnelConnection::OnClosed(DWORD dwError, int iWsaError)
{
	log(ZQ::common::Log::L_INFO, TUNLLOGFMT("connection closed"));
	if (isPassive())
	{
		// simply delete it
//		close();
		delete this;
	}
}

bool TunnelConnection::forwardData(const void* data, const int datalen,
								   const ZQ::common::InetHostAddress& source, const int sport,
								   const ZQ::common::InetMcastAddress& group, const int gport,
								   const int timeout/*=DEFAULT_TIMEOUT*/)
{
	if (datalen<0)
		return false;

	log(ZQ::common::Log::L_DEBUG, TUNLLOGFMT("forward %s:%d data to the peer"), group.getHostAddress(), gport);

	if (_status == HANDSHAKING && timeout >0 && _eventReady!=NULL)
	{
		// wait for timeout for handshaking
		DWORD ret = ::WaitForSingleObject(_eventReady, timeout);
		if (ret == WAIT_TIMEOUT || ret ==WAIT_ABANDONED)
		{
			_status = IDLE;
			return false;
		}
	}

	if (!(READY == _status || ACTIVE == _status))
	{
		log(ZQ::common::Log::L_DEBUG, TUNLLOGFMT("unexpected status to forward data package"));
		return false;
	}

	// buildup message header
	BYTE* buf = new BYTE[datalen + sizeof(msg_header_t)];
	msg_header_t* pHeader = (msg_header_t*)buf;

	memset(pHeader, 0, sizeof(msg_header_t));
	pHeader->version = TUNNEL_VERSION;
	pHeader->type =0;

	ZQ::common::InetAddress::inetaddr_t addr = group.getAddress();

	pHeader->mcgroup.family = addr.family;
	if (pHeader->mcgroup.family == PF_INET6)
	{
		pHeader->mcgroup.sa.a6.sin6_addr = addr.addr.a6;
		pHeader->mcgroup.sa.a6.sin6_port = gport;
	}
	else if (pHeader->mcgroup.family == PF_INET)
	{
		pHeader->mcgroup.sa.a.sin_addr = addr.addr.a;
		pHeader->mcgroup.sa.a.sin_port = gport;
	}

	// append with the data
	memcpy(buf+sizeof(msg_header_t), data, datalen);

	bool ret = false;
	try
	{
		ret = send(buf, sizeof(msg_header_t)+datalen);
		log(ZQ::common::Log::L_INFO, TUNLLOGFMT("data %s"), (ret?"sent":"dropped"));
	}
	catch(...) {}

	delete[] buf;

	return ret;
}

bool TunnelConnection::addOwner(Conversation* owner)
{
	if (owner == NULL)
		return false;

	// add owner into the list
	bool bAdded = false;
	{
		ZQ::common::Guard< ZQ::common::Mutex > guard(_ownersLock);
		for (ownerlist_t::iterator it = _owners.begin(); it < _owners.end(); it++)
		{
			if (*it ==owner)
				bAdded = true; // same owner already in the list
		}

		if (!bAdded)
			_owners.push_back(owner);
		bAdded = true;
	}

	// add this connection into owner's list
	{
		bAdded = false;
		ZQ::common::Guard< ZQ::common::Mutex > guard(owner->_tunnelsLock);
		for (Conversation::tunnels_t::iterator it = owner->_tunnels.begin(); it < owner->_tunnels.end(); it++)
		{
			if (*it ==this)
				bAdded = true; // same owner already in the list
		}
		if (!bAdded)
			owner->_tunnels.push_back(this);
		bAdded = true;
	}

	return bAdded;
}

bool TunnelConnection::removeOwner(Conversation* owner)
{
	if (owner == NULL)
		return false;

	{
		// remove from owner list;
		ZQ::common::Guard< ZQ::common::Mutex > guard(_ownersLock);
		for (ownerlist_t::iterator it = _owners.begin(); it < _owners.end(); it++)
		{
			if ((*it) !=NULL)
			{
				// remove the connection from owner's tunnel list;
				ZQ::common::Guard< ZQ::common::Mutex > guard((*it)->_tunnelsLock);
				for (Conversation::tunnels_t::iterator cit = (*it)->_tunnels.begin(); cit < (*it)->_tunnels.end(); it++)
				{
					if (*cit ==this)
						(*it)->_tunnels.erase(cit);
				}
			}

			if ((*it) ==owner || (*it) ==NULL)
				_owners.erase(it);
		}
	}

	return true;
}

// Add by Wang Jinmin
void TunnelConnection::OnConnectedFailover(const FtCommAddr_t* pLocalAddr, const FtCommAddr_t* pRemoteAddr, DWORD dwError, int iWsaError)
{
	_status = HANDSHAKING;
	log(ZQ::common::Log::L_DEBUG, LOGFMT("client sending local guid: %s on failover connected"), localidstr());
	send((BYTE*) &((GUID)_localid), sizeof(GUID));	
	log(ZQ::common::Log::L_INFO, LOGFMT("wake up scaner to inspect the tunnel list."));
	gTunnelManager.wakeupScanner();
}

// Add by Wang Jinmin
void TunnelConnection::OnConnectedNormal(const FtCommAddr_t* pLocalAddr, const FtCommAddr_t* pRemoteAddr, DWORD dwError, int iWsaError)
{
	_status = HANDSHAKING;
	log(ZQ::common::Log::L_DEBUG, LOGFMT("client sending local guid: %s on normal connected"), localidstr());	
	send((BYTE*) &((GUID)_localid), sizeof(GUID));	
	log(ZQ::common::Log::L_INFO, LOGFMT("wake up scaner to inspect the tunnel list."));
	gTunnelManager.wakeupScanner();
}

void TunnelConnection::OnFalingOver(const FtCommAddr_t* pLocalAddr, const FtCommAddr_t* pRemoteAddr, DWORD dwError, int iWsaError)
{
	_status = FAILOVER;
	log(ZQ::common::Log::L_DEBUG, LOGFMT("client retry to connect %s."), localidstr());	
}
// -----------------------------
// class TunnelManager
// -----------------------------
TunnelManager::TunnelManager()
:ZQ::common::FtTcpListener(TYPEINST_TYPE, TYPEINST_INST),
ZQ::common::Thread(), _bQuit(false)
{
	/*_pScaner = new TunnelScaner(this);
	_pScaner->start();*/
	start();
	_hScan = ::CreateEvent(NULL,false,false ,NULL);
}

TunnelManager::~TunnelManager()
{
	close();

	//::Sleep(500);
	//if(_pScaner)
	//	delete _pScaner;
}

bool TunnelManager::reg(TunnelConnection* conn)
{
	if (conn == NULL)
		return false;

	_lock.enter();
	_list.push_back(conn);
	_lock.leave();

	return true;
}

bool TunnelManager::unreg(TunnelConnection* conn)
{
	if (conn == NULL)
		return false;

	_lock.enter();
	for (tunnels_t::iterator it = _list.begin(); it < _list.end(); it++)
	{
		if (*it == conn)
		{
			_list.erase(it);
			break;
		}
	}
	_lock.leave();

	return true;
}

void TunnelManager::OnAccept(ZQ::common::FtTcpConnection **ppConn, const TCP_CHANDLE hConn)
{
	if (ppConn==NULL)
		return;

	if(hConn == NULL)
	{
		*ppConn = NULL;
		return;
	}

	TunnelConnection* pConn=new TunnelConnection(hConn);

	*ppConn = pConn;
	if(pConn ==NULL)
		return;

	log(ZQ::common::Log::L_INFO, LOGFMT("accepted an incoming tunnel connection"));

	pConn->OnConnected(TCPSTATE_SUCCESS, FTSTATE_CONNECTED_NORMAL);
}

// Listener is closed
void TunnelManager::OnClosed(DWORD dwError)
{
	log(ZQ::common::Log::L_DEBUG, LOGFMT("tunnel connection closed: %x"), dwError);
}

bool TunnelManager::listen(ZQ::common::FtHostAddress& bind, const int listenport)
{
	log(ZQ::common::Log::L_DEBUG, LOGFMT("start accepting incoming tunnel connections on %d"), listenport);
	return FtTcpListener::listen(listenport, bind);
}

void TunnelManager::close(void)
{
	if (_bQuit)
		return;

	log(ZQ::common::Log::L_DEBUG, LOGFMT("stop accepting incoming tunnel connections"));
	log(ZQ::common::Log::L_DEBUG, LOGFMT("stop scaner thread"));
	_bQuit = true;
	wakeupScanner();

	FtTcpListener::close(0);

	log(ZQ::common::Log::L_DEBUG, LOGFMT("close tunnel connections"));
	tunnels_t tmpv = _list;
	for (tunnels_t::iterator it = tmpv.begin();	it < tmpv.end(); it++)
	{
		if (*it !=NULL)
		{
			(*it)->close();
			delete (*it);
		}
	}
}

TunnelConnection* TunnelManager::connect(const ZQ::common::FtHostAddress& bind, const ZQ::common::FtHostAddress& remote, const int port,  timeout_t timeout)
{
	TunnelConnection* conn = NULL;
	try
	{
		conn = new TunnelConnection(bind);
		if (conn==NULL)
			return NULL;

		if (!conn->connect(remote, port, timeout))
		{
			delete conn;
			return NULL;
		}
	}
	catch(...) {}

	return conn;
}

void TunnelManager::wakeupScanner(void)
{
	::SetEvent(_hScan);
}

void TunnelManager::checkConnections(void)
{
	log(ZQ::common::Log::L_INFO, LOGFMT("scanning for duplicate tunnel connections"));

	tunnels_t *pList;
	ZQ::common::Mutex* pLock;
	// _list must OK
	pList = &_list;
	std::vector< TunnelConnection* >::iterator itFirst, itSecond, itFirstEnd;
	TunnelConnection *pConnectionOne, *pConnectionTwo;

	if( pList->size() > 1 )
	{
		pLock = &_lock;

		ZQ::common::MutexGuard guard(_lock);
		itFirstEnd = pList->end();
		-- itFirstEnd;
		for( itFirst = pList->begin(); itFirst != itFirstEnd; ++ itFirst )
		{
			pConnectionOne = *itFirst;

			if( pConnectionOne )
			{
				itSecond = itFirst;
				++ itSecond;
				for( ; itSecond != pList->end(); ++ itSecond )
				{
					pConnectionTwo = *itSecond;
					if( pConnectionTwo )
					{

						if( pConnectionTwo->isPassive() )
						{

							// if the peerid is repeat
							if( pConnectionOne->peerid() == pConnectionTwo->peerid() )
							{
								// not passive and local guid greater than peerid
								if( pConnectionOne->guid() < pConnectionTwo->peerid() )
								{							
									// owner
									std::vector < Conversation* >::iterator itOwner;
									for( itOwner = pConnectionOne->_owners.begin(); 
										itOwner != pConnectionOne->_owners.end(); 
										++ itOwner)
									{
										pConnectionTwo->addOwner(*itOwner);
									}
									log(ZQ::common::Log::L_INFO, LOGFMT("scaner withdraw repeated tunnel: (%08x)%s"),pConnectionOne, pConnectionOne->peeridstr());
									pConnectionOne->close();
									delete( pConnectionOne );
								}
							}
						}
					}
				}
			}
		}

		// remove from vector
		//for( itFirst = pList->begin(); itFirst != pList->end(); ++ itFirst )
		//{
		//	pConnectionOne = *itFirst;
		//	if( !pConnectionOne )
		//	{
		//		pList->erase(itFirst);
		//		itFirst = pList->begin();
		//	}
		//}
		
	}
}

int TunnelManager::run(void)
{
	// _list must OK
	if( (&_list == NULL) &&(_list.size() == 0 ))
		return 1;
	::ResetEvent(_hScan);
	while(!_bQuit)
	{
		DWORD dwRtn = WaitForSingleObject(_hScan, TUNNEL_SCANER_TIME_OUT);
		if (_bQuit)
			return 1;

		switch(dwRtn )
		{
		case WAIT_ABANDONED:
		case WAIT_OBJECT_0:
			{
				// yield for the connection hand shaking
				while ((WAIT_TIMEOUT != dwRtn) && !_bQuit)
					dwRtn = WaitForSingleObject(_hScan, TUNNEL_SCANER_TIME_OUT);

				if (_bQuit)
					return 1;

				// now kill the dup connections
				checkConnections();				
				break;
			}

		case WAIT_TIMEOUT:
			{
				break;
			}
		default :
			break;
		}
	}
	if(_hScan)
		::CloseHandle(_hScan);
	_hScan = NULL;
	return 0;
}


