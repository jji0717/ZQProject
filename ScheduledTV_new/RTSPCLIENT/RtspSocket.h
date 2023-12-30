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
// Name  : RtspSocket.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-6-9
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/RTSPCLIENT/RtspSocket.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// ===========================================================================

#ifndef _RTSPSOCKET_H_
#define _RTSPSOCKET_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(_MSC_VER)
# pragma warning(disable:4786)    // identifier was truncated in debug info
#endif

#if defined(_WINDOWS)
# pragma comment(lib, "Ws2_32.lib")
#endif

#ifndef MAKEDEPEND
# include <string>
#endif

//! A platform-independent socket API.
class RtspSocket {
public:

//! Creates a stream (TCP) socket. Returns -1 on failure.
static int socket();

//! Closes a socket.
static void close(int socket);


//! Sets a stream (TCP) socket to perform non-blocking IO. Returns false on failure.
static bool setNonBlocking(int socket);

//! Read text from the specified socket. Returns false on error.
static int nbRead(int socket, std::string& s, bool *eof, int& error);

//! Write text to the specified socket. Returns false on error.
static int nbWrite(int socket, std::string& s, int *bytesSoFar);


// The next four methods are appropriate for servers.

//! Allow the port the specified socket is bound to to be re-bound immediately so 
//! server re-starts are not delayed. Returns false on failure.
static bool setReuseAddr(int socket);

//! Bind to a specified port with localhost
static bool bind(int socket, int port, const char* strIp = NULL);

//! Set socket in listen mode
static bool listen(int socket, int backlog);

//! Accept a client connection request
static int accept(int socket);


//! Connect a socket to a server (from a client)
static bool connect(int socket, std::string& host, int port);

//! Check if is fatal IO error for socket
static bool nonFatalError(int errorcode);

//! Returns last errno
static int getError(int socket);

//! Returns message corresponding to error
static std::string getErrorMsg(int error);
};

#endif	// _RTSPSOCKET_H_

