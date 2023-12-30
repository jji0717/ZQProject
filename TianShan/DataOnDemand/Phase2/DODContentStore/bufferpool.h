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
// Desc  : Define the BufferPool class
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/bufferpool.h 1     10-11-12 16:05 Admin $
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/bufferpool.h $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 1     08-12-08 11:11 Li.huang
// 
// 1     08-10-30 16:45 Ken.qian
// Move files from /ZQProjs/Generic/ContentProcess to local folder, since
// files at ContentProcess were never used by others components. And
// remove the pacing codes from NTFSIORender to indepent on Vstrm DLL
// 
// 18    08-01-02 18:28 Ken.qian
// support dynamically increase the buffer count until the max
// 
// 17    07-12-14 13:57 Ken.qian
// use _vsnprintf to replace _vsprintf, and adjust the head include order
// to avoid compiling error: '_vsnprintf' : ambiguous call to overloaded
// function
// 
// 16    07-11-15 15:14 Ken.qian
// 
// 15    07-11-14 11:26 Ken.qian
// 
// 14    07-10-23 18:20 Ken.qian
// change buffer queue to be d-queue, and freed buffer put into the head
// of d-queue
// 
// 13    07-09-17 15:31 Ken.qian
// 1. replace getUnallocatedCount()  with getUsedCount()
// 2. add freeUsedBuffers()
// 
// 12    07-07-18 19:36 Ken.qian
// fix buffpool issue
// 
// 11    07-07-17 14:18 Ken.qian
// 
// 10    07-06-22 12:05 Fei.huang
// suppress the signed unsigned mismatch warning.
// 
// 9     07-05-14 16:32 Ken.qian
// 
// 8     07-04-28 14:21 Ken.qian
// 
// 7     07-04-20 16:33 Ken.qian
// 
// 6     07-04-20 10:36 Ken.qian
// 
// 5     07-04-19 17:31 Ken.qian
// 
// 4     07-04-19 10:18 Ken.qian
// 
// 3     07-04-10 19:17 Ken.qian
// GraphPool Unitest Pass
// 
// 2     07-03-30 19:26 Ken.qian
// 
// 1     07-03-22 18:46 Ken.qian



#ifndef __ZQ_Common_BufferPool_h__
#define __ZQ_Common_BufferPool_h__

//#include "ZQ_common_conf.h"
#include "Variant.h"
#include "Locks.h"

#include <vector>
#include <queue>
#include <map>


#ifdef _DEBUG
#  include <stdio.h>
#endif

#define DEFAULT_ADDTIONAL_COUNT           10
#define DEFAULT_BUFFER_POOL_SIZE_MIN      64          
#define DEFAULT_BUFFER_POOL_SIZE          128
#define DEFAULT_POOL_BUFFER_SIZE	      (64*1024)		// 64K

#define DEFAULT_BUFF_WAIT_TIMEOUT         500           // ms


namespace ZQ {
namespace Content {

	
class BufferPool
{
	friend class BufferData;
public:
	/// construct
	///@param[in]         buffSize        The size of each buff, 
    ///                                   0   - it is specified at running time
	///                                   > 0 - all buffer were pre-allocated with fixed size
	BufferPool(DWORD minPoolSize=DEFAULT_BUFFER_POOL_SIZE_MIN,  DWORD maxPoolSize = DEFAULT_BUFFER_POOL_SIZE, 
		DWORD reservedCount = 0, DWORD buffSize = DEFAULT_POOL_BUFFER_SIZE);

	/// destruct
	~BufferPool();
	
public: 	
	/// Allocate BufferData from the pool, and return 
	///@param[in]        wait        The wait time in milliseconds
	///return            the avaliable buffer object
	BufferData* alloc(DWORD wait = INFINITE);
	
	/// Free the BufferData
	///@param[in]        pBuff            the will freed BufferData
	void free(BufferData* buff);
	
	/// free all the used buffer
	void freeUsedBuffers();

	/// Get used buffer data count, for checking the pool status
	DWORD getUsedCount() { return _allocatedCount; };

	/// Get buffer pool size
	DWORD getPoolSize() { return _poolSize; }; 

	/// Get the buffer pre-allocated size
	DWORD getPreAllocBufferSize() { return _bufferSize; };

	void TraceUsedBuff();
private:
	// do atomic alloc, Mutex must add outside
	BufferData* atomicAlloc();
	void atomicFree(BufferData* buffData);

private:
	typedef std::deque<BufferData*>     BUFFERQUEUE;
	BUFFERQUEUE                 _avaliablePool;

	typedef std::map<long, BufferData*> BUFFERMAP;
	BUFFERMAP                   _usedPool;

	ZQ::common::Semaphore       _semaphore;
	ZQ::common::Mutex           _mutex;
	
	DWORD                       _poolSize;
	DWORD                       _maxPoolSize;
	DWORD                       _reservedCount; 

	DWORD                       _bufferSize;

	DWORD                       _allocatedCount;
	
	/// if BufferData's _data size extends this value, the _data memory must be freed
	/// Notice: it is _data instead of BuffData itself
	DWORD                       _maxKeepMemorySize;	

};

class BufferData
{
	friend class BufferPool;
public:
	BufferData(const BufferData& rhs);
	BufferData& operator=(const BufferData& rhs);

protected:
	BufferData(DWORD buffSize = 0);

	~BufferData();
	
public:
	///Write the data to the buff
	///@param[in]            pData       The pointer of the data block
	///@param[in]            len         The length of the buff
	///@return               Write data successfully
	bool writeData(const BYTE* data, DWORD len);
	
	///Fetch the point of the buffer data block
	///@param[out]       len             the length of the data block in the buffer
	///@return           the pointer of the data block
	BYTE* getPointerForRead(DWORD& len);
	
	///Fetch the point of the buffer data block for written directly
	///setActualLength() must be invoked after data writing. 
	///@param[in]         len             the expected len for writing, the actual len MUST be less or equal to this.
	///@return              the pointer of the data block
	BYTE* getPointerForWrite(DWORD len);
	
	///get the max buff max size
	///@return              the allocated total size 
	int getTotalSize();

	///Set the actual length of the buffer
	void setActualLength(DWORD len) { _actualLen = len; };

	///get the actual length of the buffer
	DWORD getActualLength() { return _actualLen; };

	/// set buffer property to map, 
	/// @param[in] property   The property name, as the key of map
	/// @param[in] value  The value of the property specified by name
	void setProperty(std::string property, const ZQ::common::Variant& value);

	/// set buffer property to map, 
	/// @param[in] properties   property map
	void setProperty(std::map<std::string, ZQ::common::Variant> properties);

	bool getProperty(std::string property, ZQ::common::Variant& value);

protected:
	/// free the allocated memory
	void freeDataMemory();

protected:
	/// No detail type definition here, it depend on the application
	typedef std::map<std::string, ZQ::common::Variant> PROPERTY;
	PROPERTY  _properties;

	DWORD     _totalLen;
	DWORD     _actualLen;
	bool      _allocated;
	
	BYTE*     _data;	
};
	
	
}}

#endif // __ZQ_Common_BufferPool_h__
