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
// $Header: /ZQProjs/Generic/ContentProcess/NTFSFileIOSource.h 1     10-11-12 15:58 Admin $
// $Log: /ZQProjs/Generic/ContentProcess/NTFSFileIOSource.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 6     08-01-02 18:35 Ken.qian
// change since minAvailBuffCount was move to Graph
// 
// 5     07-10-23 18:29 Ken.qian
// add available buffer count checking
// 
// 4     07-07-23 15:29 Ken.qian
// vstrmiorender support subfiles and 64K IO writting
// 
// 3     07-06-27 15:13 Ken.qian
// Change kbps to bps
// 
// 2     07-06-14 17:37 Cary.xiao
// 
// 1     07-05-29 17:10 Ken.qian
//

#ifndef __ZQ_NTFSFileIOSource_Process_H__
#define __ZQ_NTFSFileIOSource_Process_H__

#include "GraphFilter.h"

namespace ZQ { 
namespace Content { 
/// namespace Process presents the processing to the content, such as reading/writing
namespace Process {

#define DEFAULT_SOURCE_BUFF_COUNT	30
#define MAX_READ_SIZE               (1024*64)     // do not change this, coz to VStrm, it is fixed

class NTFSFileIOSource : public SourceFilter
{
	friend class Graph;
public:
	NTFSFileIOSource(ZQ::Content::Process::Graph& graph, std::string myName="NTFSFileIOSource");
				
protected:	
	/// destructor
	virtual ~NTFSFileIOSource(void);

public:
	/// receiving the buffer coming from previous content process object
	/// The received Content Process Base is required to re allocate buffer from pool
	/// and copy it.
	///@param[in]  upObj   the obj who call this function
	///@param[in]  buff      the buff in the pool which allocated in above Base
	virtual bool receive(Filter* upObj, ZQ::Content::BufferData* buff);
	
	/// start to process the incoming BufferData
	///@return true if it start successfully
	virtual bool begin(void);
	
	/// pause to process the incoming BufferData
	///@return true if it pause successfully
	virtual bool pause(void);
	
	/// abort current buffer processing, generally, this is invoked by the Graph
	/// in case of any base obj failed during the processing, and this failure require all object need to aborted.
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
	virtual __int64 getProcessedStuff() { return 0; };
	DWORD getReadSize() {	return _readSize;	}
	
	bool setReadSize(DWORD readSize) 
	{	
		if (readSize > _pool.getPreAllocBufferSize())
			return false;
			
		_readSize = readSize;	
		return true;
	}
	
public:
	/// implementation of NativeThread virtual function
	int run(void);

private:
	void releaseFileHandle(void);

private:
	HANDLE 			              _hStop;
	HANDLE                        _hNotify;
	
	HANDLE                        _hSourceFile;
	std::string                   _szSourceFile;
	
	DWORD                         _readSize;
	DWORD                         _maxbps;

	DWORD                         _lasttimer;
	DWORD                         _tidAbortBy;
	__int64                       _processedBytes;

	__int64                       _fileSize;
};



} } }

#endif // __ZQ_NTFSFileIOSource_Process_H__
