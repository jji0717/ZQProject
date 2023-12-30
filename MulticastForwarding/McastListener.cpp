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
// Ident : $Id: McastListener.cpp,v 1.5 2004/07/27 09:40:04 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : impl the multicast listener
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/McastListener.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 16    04-09-15 16:41 Kaliven.lee
// 
// 15    04-09-15 16:32 Kaliven.lee
// add flag for packet lost test
// 
// 14    04-09-14 16:29 Hui.shao
// fixed buffer size to recv()
// 
// 13    04-09-13 15:25 Kaliven.lee
// add extern globe MaxUDPBufferSize variable
// 
// 12    04-08-27 9:53 Kaliven.lee
// 
// 11    04-08-26 11:56 Kaliven.lee
// 
// 10    04-08-26 9:55 Kaliven.lee
// init _bind with the bind address
// 
// 9     04-08-22 14:47 Jian.shen
// Revision 1.5  2004/07/27 09:40:04  shao
// renamed paramter name
//
// Revision 1.4  2004/07/22 06:17:48  shao
// adjusted some method/paramter names, and added comments
//
// Revision 1.3  2004/07/20 13:31:09  shao
// ignore if null package
//
// Revision 1.2  2004/07/07 11:43:19  shao
// build the relationship to McastFwd
//
// Revision 1.1  2004/06/24 12:34:05  shao
// created
//
// ===========================================================================

#include "McastListener.h"
#include "Conversation.h"
extern DWORD gdwMaxUDPBufferSize;


McastListener::McastListener(ZQ::common::InetMcastAddress group, int group_port, ZQ::common::InetHostAddress bind, Conversation* owner /*=NULL*/)
:_group(group), _bind(bind), _gport(group_port), ZQ::common::UDPReceive(bind, group_port),
 _bQuit(false), _pOwner(owner), _buf(NULL), _bufsz(gdwMaxUDPBufferSize)
{
	SYSTEMTIME SystemTime ;
	GetSystemTime(&SystemTime);
	sprintf(_sEstablishTime,
		"%4d/%2d/%2d %2d:%2d:%2d",
		SystemTime.wYear,
		SystemTime.wMonth,
		SystemTime.wDay,
		SystemTime.wHour,
		SystemTime.wMinute,
		SystemTime.wSecond);

	if (_bufsz <1024)
		_bufsz = 1024;

	if (_bufsz > 64*1024)
		_bufsz = 64*1024;
}

McastListener::~McastListener()
{
	if(_buf)
		delete _buf;
}

bool McastListener::init(void)	
{
	// allocate for the receiving buffer
	_buf = new char[_bufsz];

	if (_buf ==NULL)
		return false;

	UDPReceive::setMulticast(true);
	ZQ::common::Socket::Error err = UDPReceive::join(_group);
	UDPReceive::setCompletion(true); // make the socket blockable
	

	return (err == ZQ::common::Socket::errSuccess);
}

int McastListener::run()
{
	while (!_bQuit) // TODO: ScThreadPool doesn't support thread::terminate
	{
		ZQ::common::InetHostAddress from;
		int sport;


		int len = UDPReceive::receiveFrom(_buf , _bufsz, from, sport);
		if (len >0)
		{

			OnCapturedData(_buf, len, from, sport);
		}
	}

	return 0;
}

int McastListener::OnCapturedData(const void* data, const int datalen, ZQ::common::InetHostAddress source, int sport)
{
	if (_pOwner ==NULL)
		return datalen;

	return _pOwner->OnCapturedData(data, datalen, source, sport);
}


