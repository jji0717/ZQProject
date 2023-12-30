// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id:  Connection.h,v 1.2 2005/07/26 10:08:12 li Exp $
// Branch: $Name:  $
// Author: Jianjun Li
// Desc  : define BytesMessage, a message type
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataTunnel/source/include/Connection.h $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 1     09-03-09 10:34 Li.huang
// 
// 1     08-12-08 11:11 Li.huang
// 
// 1     07-03-13 18:27 Li.huang
// 
// 1     07-01-03 10:39 Li.huang
// 
// 2     06-09-19 12:15 Li.huang
// 
// 2     05-07-28 16:58 Jianjun.li
// 
// 1     05-07-28 10:42 Jianjun.li
// 
// 1     05-07-27 18:48 Jianjun.li
// 
// 5     05-07-27 18:12 Jianjun.li
// 
// 4     05-07-27 15:27 Jianjun.li
// 
// 3     05-07-26 19:17 Jianjun.li
// 
// 2     05-07-26 15:10 Jianjun.li
//
// Revision 1.2 2005/07/26 10:08:12 li
// notation added
//
// Revision 1.1 2005/07/22 14:23:46 li 
// created
//
// ===========================================================================
#ifndef __Connection_h__
#define __Connection_h__

#include "Session.h"


namespace ZQ{
namespace JMSCpp{

/// a function to receive exception call back
/// @return void
/// @param ErrType system will pass it to you when exception fired
/// @param lpdata is your own var
typedef void (*ConnExceptionCallBack) (int ErrType, void * lpData);
// -----------------------------
// class Connection
// -----------------------------
/// A Connection object is a client's active connection to its JMS provider. 
class Connection  
{
public:
	Connection() : _connection(NULL),_pCallBack(NULL) {};
	~Connection();

	/// Initialize the Session object
	/// @return true on success,false on failure
	/// @param ss The object that will be initialized
    bool createSession(Session &ss);

	/// Gets the client identifier for this connection
	/// @return true on success,false on failure
	/// @param identifier Must not be NULL.On uccess will contains
	/// the unique client identifier
	/// @param size the buf length of identifier
    bool getClientID(char *identifier,int size);

	/// Sets the client identifier for this connection.
	/// @return true on success,false on failure
	/// @param identifier The unique identifier to set
    bool setClientID(char *identifier);

	/// Starts (or restarts) a connection's delivery of incoming messages.
	/// A call to start on a connection that has already been started is ignored. 
	/// @return true on success,false on failure
    bool start();

	/// Temporarily stops a connection's delivery of incoming messages.
	/// Delivery can be restarted using the connection's start method.
	/// When the connection is stopped, delivery to all the connection's 
	/// message consumers is inhibited: synchronous receives block, 
	/// and messages are not delivered to message listeners.
	/// @return true on success,false on failure
	bool stop();

	/// Close connection explicitly
	/// @return void
	/// @param void
	void close();

	/// Set Connection exception callback function pointer
	/// @return void
	/// @param pCallBack a pointer to a callback function
	/// @param lpData which you set in for your own use
	void SetConnectionCallback(ConnExceptionCallBack pCallBack,void* lpData);

	/// Fire the connection excption call back,SYSTEM WILL CALL THIS FUNCTION
	void fireConnCallback(int ErrType);

public:

	/// A void pointer that represents a java Connection object.
	/// Some other classes in the lib need to access it directly.
	/// It is made public to avoid using too much friend classes
	/// So avoid accessing it directly in your program.
    SS_HANDLE			_connection;
	ConnExceptionCallBack	_pCallBack;
	void*				_lpData;

};
}
}
#endif // __Connection_h__
