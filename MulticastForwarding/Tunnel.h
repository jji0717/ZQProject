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
// Ident : $Id: Tunnel.h,v 1.7 2004/08/05 07:14:09 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : Tunnel connection and listener for the project of McastFwd
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/Tunnel.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 18    05-04-15 10:19 Build
// 
// 17    04-11-08 18:01 Hui.shao
// const paramters
// 
// 16    04-09-13 15:51 Kaliven.lee
// add failover status for tunnel
// 
// 15    04-09-13 15:14 Kaliven.lee
// 
// 14    04-09-13 15:04 Kaliven.lee
// add a method to get current status of the tunnels
// 
// 13    04-09-09 17:46 Kaliven.lee
// 
// 12    04-09-09 10:33 Kaliven.lee
// make some change so that service class can inherit from mcastfwd
// 
// 11    04-08-27 9:56 Kaliven.lee
// add check isclosed when TunnelManager destruct
// 
// 10    04-08-26 17:42 Kaliven.lee
// 
// 9     04-08-26 10:26 Kaliven.lee
// add scan thread
// 
// 8     04-08-22 15:03 Jian.shen
// 
// 7     04-08-22 14:34 Jian.shen
// Revision 1.7  2004/08/05 07:14:09  shao
// comments
//
// Revision 1.6  2004/07/22 06:17:48  shao
// adjusted some method/paramter names, and added comments
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

#ifndef __TUNNEL_H__
#define __TUNNEL_H__

#include "McastFwdConf.h"
#include "FtTcpComm.h"
#include "Conversation.h"
#include "Socket.h"
#include "Guid.h"

class TunnelConnection;
class TunnelManager;
class TunnelException;

extern TunnelManager gTunnelManager;

// -----------------------------
// class TunnelConnection
// -----------------------------
/// a dual rail tcp connection by invoking FtTcpConnection.
class MCastFwdServ;

class TunnelConnection : public ZQ::common::FtTcpConnection
{
	friend class TunnelManager;
	friend class Conversation;
	friend class MCastFwdServ;
	
public:

	typedef struct _msg_header
	{
		uint8 version;
		uint8 type;
		uint8 reserved[2];
		ZQ::common::Socket::saddr_t source;
		ZQ::common::Socket::saddr_t mcgroup;
	} msg_header_t;

	typedef enum _status_e {IDLE = 0, CONNECT, HANDSHAKING, READY, ACTIVE, CLOSED, FAILOVER} status_e;

public:

	/// destructor
	virtual ~TunnelConnection();

	/// forward an amount of data thru the connection
	/// @param data            the address of the data want to send
	/// @param datalen         size of the data in bytes to send
	/// @param source          the source machine where the message was sent from
	/// @param sport           the source port that the message was sent
	/// @param group           the target multicast group
	/// @param gport           the target multicast port
	/// @return                true if succ
	bool forwardData(const void* data, const int datalen,
		             const ZQ::common::InetHostAddress& source, const int sport,
					 const ZQ::common::InetMcastAddress& group, const int gport,
					 const int timeout=DEFAULT_TIMEOUT);

	ZQ::common::Guid& guid() { return _localid; }
	ZQ::common::Guid& peerid() { return _peerid; }
	const char* peeridstr() { _peerid.toString(_peeridstr, sizeof(_peeridstr) -2); return _peeridstr; }
	
	std::string getStatus(void);

protected:

	/// client connection constructor, the object must be created by TunnelManager
	/// @param bind     the IP to bind when connect to others
	TunnelConnection(const ZQ::common::FtHostAddress& bind, ConnectOptions_t* pOptions=NULL);

	/// server connection constructor, must be called by TunnelListener when accept an incoming connection
	/// @param bind     the IP to bind when connect to others
	TunnelConnection(TCP_CHANDLE handle);

	/// connect to other machines, this is a blockable operation with timeout
	/// @param remote     the target machine to conect
	/// @param timeout        timeout for the operation
	/// @return               true if succ
	bool connect(const ZQ::common::FtHostAddress& remote, const int port,  timeout_t timeout);

	/// merge the owners of the source tunnel connection into the owner list
	/// @param source     the source tunnel connection to merge from
	/// @return           true if successful
	bool addOwner(Conversation* owner);

	/// remove an owner of this tunnel
	/// @param owner     pointer to the owner conversation
	/// @return          true if successful
	bool removeOwner(Conversation* owner);

	void OnConnectedFailover(const FtCommAddr_t* pLocalAddr, const FtCommAddr_t* pRemoteAddr, DWORD dwError, int iWsaError);

	void OnConnectedNormal(const FtCommAddr_t* pLocalAddr, const FtCommAddr_t* pRemoteAddr, DWORD dwError, int iWsaError);
	
	void OnFalingOver(const FtCommAddr_t* pLocalAddr, const FtCommAddr_t* pRemoteAddr, DWORD dwError, int iWsaError);

protected:

	// overide some callback event

	/// to make connect() blockable
	virtual void OnConnected(DWORD dwError, int iError);

	virtual void OnDataArrive(void *message, int len);

	virtual void OnClosed(DWORD dwError, int iWsaError);


protected:

	ZQ::common::Guid _peerid;
	char			 _peeridstr[64]; // for log

	ZQ::common::FtHostAddress  _bind;
	ZQ::common::FtHostAddress  _peer;

	HANDLE _eventReady;

	status_e _status;

	typedef std::vector < Conversation* > ownerlist_t;
	ownerlist_t _owners;
	ZQ::common::Mutex _ownersLock;

public:
	int _id;
	static int _lastid;
	static ZQ::common::Guid _localid;
	static const char* localidstr();

	static Conversation* _pDefaultConversation;
	
};

// -----------------------------
// class TunnelManager
// -----------------------------
/// manages all the TunnelConnection instances
/// and work as the tunnel listener to accept incoming connections
class TunnelManager : protected ZQ::common::FtTcpListener, public ZQ::common::Thread
{
	friend class TunnelConnection;
	//friend class TunnelScaner;
	friend class MCastFwdServ;

public:
	TunnelManager();
	~TunnelManager();

	int size() { return _list.size(); }
	const TunnelConnection* operator[](int index) { return (index<0 || index>=size()) ? NULL: _list[index]; }

	/// connect to other machines, this is a blockable operation with timeout
	/// @param remote     the target machine to conect
	/// @param timeout        timeout for the operation
	/// @return               true if succ
	TunnelConnection* connect(const ZQ::common::FtHostAddress& bind, const ZQ::common::FtHostAddress& remote, const int port,  timeout_t timeout);

	bool listen(ZQ::common::FtHostAddress& bind, const int listenport = DEFAULT_TUNNEL_LISTERN_PORT);
	void close(void);

//	bool isLoop(void);

	const char* getBindAddr() { return FtTcpListener::getBindAddrs(); }

	void wakeupScanner(void);

protected:

	// register and unregister connections
	bool reg(TunnelConnection* conn);
	bool unreg(TunnelConnection* conn);

	/// to new TunnelConnection instead of FtTcpConnection
	virtual void OnAccept(ZQ::common::FtTcpConnection **ppConn, const TCP_CHANDLE hConn);

	/// Listener is closed
	virtual void OnClosed(DWORD dwError);

	virtual void checkConnections(void);
//	inherit from thread 
	virtual int run(void);

private:
	typedef std::vector< TunnelConnection* > tunnels_t;
	tunnels_t _list;
	ZQ::common::Mutex _lock;

	bool _bQuit;
	HANDLE _hScan; // event to wake up the scan thread

};


// -----------------------------
// class TunnelException
// -----------------------------
class TunnelException : public ZQ::common::IOException
{
public:
	TunnelException(const std::string &what_arg) throw() :ZQ::common::IOException(what_arg){};
	virtual ~TunnelException() throw(){};
};

#endif // __TUNNEL_H__
