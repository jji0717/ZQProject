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
// $Log: /ZQProjs/TianShan/CRG/DsmccCRG/TSPumpService/TsPump.h $
// 
// 4     4/03/14 4:34p Li.huang
// 
// 3     3/27/14 10:45a Li.huang
// add linux build
// 
// 2     3/24/14 11:37a Li.huang
// fix bug 18871
// 
// 6     3/12/14 11:09a Li.huang
// add scan subfolder depth
// 
// 5     5/09/12 4:31p Li.huang
// 
// 4     4/03/12 4:31p Li.huang
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
#include "SystemUtils.h"

extern "C" {
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
}

#include <vector>
#include <map>
#include <queue>
#include <Locks.h>
#include "Log.h"
#include <list>

#ifndef ZQ_OS_MSWIN
#include <pthread.h>
#include <dirent.h>
#endif

#ifdef HAVE_ST_BIRTHTIME
#define CREATE_TIME(x) x.st_birthtime
#else
#define CREATE_TIME(x) x.st_ctime
#endif

#ifdef ZQ_OS_MSWIN
#define ISDIR(fileStruct)	(fileStruct.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
#else
#define ISDIR(fileStruct)	(fileStruct->d_type & DT_DIR)
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

typedef struct _FileInfo
{
	std::string     FilePath;
	ZQ::common::InetHostAddress destAddr;
	int				destPort;
	int64			stampLastSent;	
	int64			stampMessageAsOf;
	int64			stampFileSeen;
	int64			stampFileAsOf;

} FileInfo;

class TsPumper : public ZQ::common::NativeThread, public ZQ::common::UDPSocket
{
public:
	friend class ScanFolder;
	friend class TsPumperService;

	typedef std::map<int,FileInfo>	FileInfoMap;

	typedef struct _Packet 
	{
		uint8 bytes[188];
	} Packet;

	typedef std::vector < Packet > Packets;

	typedef struct _Dest
	{
		FileInfo fileInfo;
		Packets         packets;

	} Dest;

	typedef std::map < int, Dest > DestMap;


	TsPumper(ZQ::common::InetHostAddress& bindAddr, ZQ::common::tpport_t bport,char* folderName,bool bHex, uint16 interval=100,int scanFolderInterval=5*60*1000, int subFolderDepth = 0);
	virtual ~TsPumper();
	void stop();
protected:
#ifdef ZQ_OS_MSWIN
	static int64 FileTimeToTime(FILETIME filetime);
#elif defined ZQ_OS_LINUX
	static int64 FileTimeToTime(time_t time);
#endif
	static std::string _cmd_Dehex;
public:
	virtual bool init(void)	{ return true; }
	virtual int  run(void);
	virtual void final(int retcode =0, bool bCancelled =false) {}

protected:

//	HANDLE		     _hWakeup;
	bool		     _bQuit;
	uint		     _interval;
	int64		     _stampNow;
	char*			 _folderName;
	bool			 _bHex;
	int				_scanInterval;
	int             _subFolderDepth;

	ZQ::common::Cond  _cond;
	ZQ::common::Mutex _lock;
	DestMap		      _dests;
};


//-------------------------------
//class ScanFolder
//-------------------------------
//scan the folder every 1min see if there is any file changed
class ScanFolder : public ZQ::common::NativeThread
{
public:
	ScanFolder(TsPumper &ts, const char* fileFolder=NULL, bool bHex=false, int subfolderDepth=0) : _pumper(ts),_folderName(std::string(fileFolder)),_bHex(bHex), _subfolderDepth(subfolderDepth) 
	{
		if (NULL != fileFolder)
			_folderName = fileFolder;
		else _folderName = "." FNSEPS;

		if (FNSEPC != _folderName[_folderName.length() -1])
			_folderName += FNSEPS;
	}
	int run(void);
private:

	int	 scanFolder(TsPumper::FileInfoMap& fmap);
	std::string getfilename(std::string filePath);

	void listFiles(const std::string folder, std::list<FileInfo>& filelist, int depth);

	bool readFileToDest(TsPumper::Dest& dest, const std::string& filename);

	TsPumper&		_pumper;
	std::string		_folderName;
	bool			_bHex;
    int          	_subfolderDepth;
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

