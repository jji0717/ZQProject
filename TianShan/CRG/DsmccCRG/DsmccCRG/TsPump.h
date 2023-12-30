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
// Ident : $Id: Socket.cpp,v 1.7 2004/07/29 06:25:44 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : a simple TS pumper by monitoring a folder, each file specifies dest
//         of UDP destination
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CRG/DsmccCRG/DsmccCRG/TsPump.h $
// 
// 1     8/09/13 2:47p Zonghuan.xiao
// seprated from crm_dsmcc.h for error code
// 
// 1     2/15/12 11:05a Hui.shao
// TsPump designed for ServiceGroup parameter advertizing
// ---------------------------------------------------------------------------

#ifndef __TSPUMPER_H__
#define __TSPUMPER_H__

#include "TimeUtil.h"
#include "NativeThread.h"
#include "UDPSocket.h"
#include "InetAddr.h"

extern "C" {
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
}

#include <vector>
#include <map>
#include <queue>

#ifdef HAVE_ST_BIRTHTIME
#define CREATE_TIME(x) x.st_birthtime
#else
#define CREATE_TIME(x) x.st_ctime
#endif

#ifndef _DEBUG
#  define MAX_WAIT (1000*60*5)
#  define DBGTRACE(...)         do { ; } while(0)
#else
#  define MAX_WAIT (1000*90)
#  define DBGTRACE(...)         do { printf(##__VA_ARGS__); } while(0)
#endif // _DEBUG

// -----------------------------
// class TsPumper
// -----------------------------
/// the TsPumper monitors the files of a given file folder. the names of the files should
/// be in format of  SG<SGId>_<DestIP or domain name>_<DestPort>.{hex|ts}
///   - bHex=false, the TsPumper reads those .ts file directly when the file was detected changed
///                 then pump to the destIP/port specified in the filename periodicaly with given
///                 interval
///   - bHex=true, the TsPumper reads those .hex file and execute de-hex cmd, specified by 
///                TsPumper::_cmd_Dehex, to read the content when the file was detected changed
class TsPumper : public ZQ::common::NativeThread, public ZQ::common::UDPSocket
{
public:
	TsPumper(ZQ::common::InetHostAddress& bindAddr, ZQ::common::tpport_t bport, const char* fileFolder=NULL, bool bHex=false, uint16 interval=100);
	virtual ~TsPumper();

	void stop();

	typedef struct _Packet {
		uint8 bytes[188];
	} Packet;
	typedef std::vector < Packet > Packets;

protected:

	typedef struct _Dest
	{
		std::string     filename;
		ZQ::common::InetHostAddress destAddr;
		int             destPort;
		Packets         packets;
		int64           stampLastSent;
		int64           stampMessageAsOf;
		int64			stampFileSeen;
	} Dest;

	typedef std::map < int, Dest > DestMap;

	bool readFileToDest(Dest& dest, const std::string& filename);

	static int64 FileTimeToTime(FILETIME filetime);
	static std::string _cmd_Dehex;

public:
	virtual bool init(void)	{ return true; }
	virtual int  run(void);
	virtual void final(int retcode =0, bool bCancelled =false) {}

protected:
	HANDLE  _hWakeup;
	bool    _bQuit,  _bHex;
	uint    _interval;
	std::string _folderName;


//	ZQ::common::Mutex _lock;
};

/* test program
int main()
{
	ZQ::common::InetHostAddress bindAddr;
	// TODO: if want to read hex file instead of ts file, pls specify the de-hex command first, such as
	//         TsPumper::_cmd_Dehex = "xxd -s";

	TsPumper ts(bindAddr, 22222, "d:\\temp\\aaa\\ts", true, 000);
	ts.start();
	Sleep(1000*60*30);
	return 0;
}
*/

#endif // __TSPUMPER_H__

