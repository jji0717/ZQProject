// ============================================================================================
// Copyright (c) 2006, 2007 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Ken Qian
// Desc  : Define the NTFS file source for processing
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/ContentProcess/FtpFileIOSource.h 1     10-11-12 15:58 Admin $
// $Log: /ZQProjs/Generic/ContentProcess/FtpFileIOSource.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 8     08-01-02 18:35 Ken.qian
// change since minAvailBuffCount was move to Graph
// 
// 7     07-11-02 16:27 Ken.qian
// ftpConnRelease() had check with errorcode, if errorcode is
// ERROR_INTERNET_TIMEOUT,  no InternetCloseHandle() call to avoid
// possible block.
// 
// 6     07-10-23 18:27 Ken.qian
// add available buffer count checking
// 
// 5     07-09-29 18:39 Ken.qian
// 1. Change MFC FTP to be Windows API
// 2. Add option to get file size by "SIZE" and "LIST"
// 
// 4     07-08-23 10:57 Ken.qian
// Add timeout setting to ftp connection
// 
// 3     07-07-23 15:29 Ken.qian
// vstrmiorender support subfiles and 64K IO writting
// 
// 2     07-06-27 15:13 Ken.qian
// Change kbps to bps
// 
// 1     07-06-07 10:04 Ken.qian
//
//

#ifndef __ZQ_FTPFileIOSource_Process_H__
#define __ZQ_FTPFileIOSource_Process_H__

#include "GraphFilter.h"
#include <WinInet.h>

class	CInternetSession;
class	CFtpConnection;
class   CInternetFile;

namespace ZQ { 
namespace Content { 
/// namespace Process presents the processing to the content, such as reading/writing
namespace Process {

#define DEFAULT_SOURCE_BUFF_COUNT	30
#define MAX_READ_SIZE               (1024*64)     // do not change this, coz to VStrm, it is fixed
#define DEF_FTP_CONN_TIMEOUT        5000

class FTPFileIOSource : public SourceFilter
{
	friend class Graph;
public:
	FTPFileIOSource(ZQ::Content::Process::Graph& graph, std::string myName="FTPFileIOSource", bool ftpFileSizeBySIZEcmd = true);
	
protected:	
	/// destructor
	virtual ~FTPFileIOSource(void);

public:
	/// receiving the buffer coming from previous content process object
	/// The received Content Process Filter is required to re allocate buffer from pool
	/// and copy it.
	///@param[in]  upObj   the obj who call this function
	///@param[in]  buff      the buff in the pool which allocated in above Filter
	virtual bool receive(Filter* upObj, ZQ::Content::BufferData* buff);
	
	/// start to process the incoming BufferData
	///@return true if it start successfully
	virtual bool begin(void);
	
	/// pause to process the incoming BufferData
	///@return true if it pause successfully
	virtual bool pause(void);
	
	/// abort current buffer processing, generally, this is invoked by the Graph
	/// in case of any Filter obj failed during the processing, and this failure require all object need to aborted.
	/// @return true if it abort successfully
	virtual bool abort(void);
		
	/// stop content processing, just a little bit different with abort(), 
	/// it is a normal stopping, but abort() is abnormal.
	virtual void stop(void);
	
	/// stop the processing and exit the Filter thread
    virtual void quit(void);

	/// this virtual function MUST be implemented by source filter, to get know 
	/// how many process stuff in the whole processing, this could be source file total
	/// bytes or something else. 
	/// Currently seems only the source could provide the total number
	virtual __int64 getTotalStuff() { return _fileSize; };

	/// this virtual function must be render, to get know current processing progress,
	/// bytes or something else.
	virtual __int64 getProcessedStuff() { return _processedBytes; };

public:
	void setReadSize(int readSize);
public:
	/// implementation of NativeThread virtual function
	int run(void);

private:
	void ftpConnRelease(DWORD errorCode=0);

private:
	HANDLE                        _hStop;
	HANDLE                        _hNotify;
	
	std::string                   _szSourceFile;
	
	DWORD                         _maxbps;
	DWORD                         _lasttimer;

	HINTERNET                     _hSession;
	HINTERNET                     _hConnection;
	HINTERNET                     _hFile;
	bool                          _ftpFileSizeBySIZEcmd;
	
	DWORD                         _tidAbortBy;
	
	__int64                       _fileSize;
	__int64                       _processedBytes;

	DWORD                         _readSize;
};



} } }

#endif // __ZQ_FTPFileIOSource_Process_H__
