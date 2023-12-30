// ============================================================================================
// Copyright (c) 1997, 1998 by
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
// Desc  : Implement the BufferPool 
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/bufferpool.cpp 1     10-11-12 16:05 Admin $
// $Log: /ZQProjs/TianShan/DataOnDemand/Phase2/DODContentStore/bufferpool.cpp $
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
// 19    08-01-02 18:28 Ken.qian
// support dynamically increase the buffer count until the max
// 
// 18    07-11-14 10:53 Fei.huang
// comment out BufferPool::TraceUsedBuff()
// 
// 17    07-10-23 18:20 Ken.qian
// change buffer queue to be d-queue, and freed buffer put into the head
// of d-queue
// 
// 16    07-09-17 15:31 Ken.qian
// 1. replace getUnallocatedCount()  with getUsedCount()
// 2. add freeUsedBuffers()
// 
// 15    07-07-24 15:42 Ken.qian
// 
// 14    07-07-19 11:19 Ken.qian
// 
// 13    07-07-18 19:36 Ken.qian
// fix buffpool issue
// 
// 12    07-07-18 16:10 Ken.qian
// 
// 11    07-07-17 14:18 Ken.qian
// 
// 10    07-06-15 13:49 Cary.xiao
// 
// 9     07-04-26 20:30 Ken.qian
// 
// 8     07-04-20 16:33 Ken.qian
// 
// 7     07-04-20 10:36 Ken.qian
// 
// 6     07-04-19 17:31 Ken.qian
// 
// 5     07-04-19 10:18 Ken.qian
// 
// 4     07-04-16 11:04 Ken.qian
// 
// 3     07-04-10 19:17 Ken.qian
// GraphPool Unitest Pass
// 
// 2     07-03-30 19:26 Ken.qian
// 
// 1     07-03-22 18:46 Ken.qian
// 
// 5     06-09-06 15:27 Ken.qian
// 
// 4     06-06-30 11:32 Ken.qian
// Add function GetUnallocatedCount()
// 
// 3     06-06-27 18:49 Ken.qian
// 
// 2     06-06-23 17:36 Ken.qian
// 
// 1     06-06-15 17:10 Ken.qian
// Initial implementation

#include "bufferpool.h"
#include <assert.h>

extern "C"
{
	#include <process.h>
	#include <stdio.h>
};

namespace ZQ {
namespace Content {
	
BufferPool::BufferPool(DWORD minPoolSize, DWORD maxPoolSize, DWORD reservedCount, DWORD buffSize)
:_poolSize(minPoolSize), _maxPoolSize(maxPoolSize), _bufferSize(buffSize), _reservedCount(reservedCount)
{
	// set the max kept memory size
	_maxKeepMemorySize = (0 == buffSize) ? DEFAULT_POOL_BUFFER_SIZE : buffSize;

	_allocatedCount = 0;

	if(0 == _maxPoolSize)
	{
		_maxPoolSize = DEFAULT_BUFFER_POOL_SIZE;
	}
	if(0 == _poolSize || _poolSize > _maxPoolSize)
	{
		_poolSize = DEFAULT_BUFFER_POOL_SIZE_MIN;
	}
	if(_reservedCount >= _maxPoolSize)
	{
		_reservedCount = 0;
	}
	if(_poolSize < _reservedCount)
	{
		_poolSize = _reservedCount+1;
	}
	
	/// allocate the memory for the pool
	DWORD i=0; 
	int nCount = 0;
	for(i=0; i<_poolSize; i++)
	{
		BufferData* pBuff = new BufferData(_bufferSize);
		if(pBuff != NULL)
		{
			_avaliablePool.push_back(pBuff);
			nCount++;
		}
		// init the semaphore
		_semaphore.post();
	}
	_poolSize = nCount;

	_allocatedCount = 0;	
}

BufferPool::~BufferPool()
{
	//
	// release the avaliable buffer
	//
	while(_avaliablePool.size() > 0)
	{
		BufferData* pBuff = _avaliablePool.front();
		_avaliablePool.pop_front();
		
		delete pBuff;
	}

	//
	// release the used buffer
	//
	BUFFERMAP::iterator it = _usedPool.begin();
	while(it != _usedPool.end())
	{
		BufferData* pBuff = (BufferData*)it->second;

		it++;
		
		delete pBuff;
	}
	_usedPool.clear();	
}

BufferData* BufferPool::atomicAlloc()
{
	BufferData* pBuff = NULL;

	if(_allocatedCount < _poolSize)
	{
		// mutex the atomic allocation
		ZQ::common::MutexGuard guard(_mutex);

		pBuff = _avaliablePool.front();
		_avaliablePool.pop_front();
		pBuff->_allocated = true;
		pBuff->_properties.clear();

		// add the allocated buffer to usedPool, use the object address the key
		_usedPool[(long)pBuff] = pBuff;
		_allocatedCount++;

		// check whether there is still avaliable buffer
		if(_allocatedCount >= (_poolSize - _reservedCount) && _poolSize < _maxPoolSize)
		{
			int availCount = _maxPoolSize - _poolSize;
			int addCount = availCount < DEFAULT_ADDTIONAL_COUNT ? availCount : DEFAULT_ADDTIONAL_COUNT;
			for(int i=0; i<addCount; i++)
			{
				BufferData* pBuff = new BufferData(_bufferSize);
				if(pBuff != NULL)
				{
					_avaliablePool.push_back(pBuff);
					_poolSize++;
				}
				_semaphore.post();
			}
		}

		return pBuff;
	}
	
	return pBuff;
}

void BufferPool::atomicFree(BufferData* buffData)
{
	// mutex the atomic allocation
	ZQ::common::MutexGuard guard(_mutex);
	
	// remove from used pool
	_usedPool.erase((long)buffData);

	// add to unallocated pool
	buffData->_properties.clear();
	buffData->_actualLen = 0;
	buffData->_allocated = false;
	_allocatedCount--;
	
	_avaliablePool.push_front(buffData);
}

BufferData* BufferPool::alloc(DWORD wait)
{	
	while(1)
	{
		//
		// wait for semaphore
		//
		_semaphore.wait();

		//
		// there is avaliable buff, alloc it
		// 
		BufferData* pBuff = atomicAlloc();

		if(NULL == pBuff)
		{
			Sleep(1);
			continue;
		}

		return pBuff;
	}
}


void BufferPool::free(BufferData* buffData)
{
	if(NULL == buffData)
		return;
		
	// this is to avoid the buffData occupy to much memory
	if(buffData->_totalLen > _maxKeepMemorySize)
	{
		buffData->freeDataMemory();
	}

	if(!buffData->_allocated)
	{
		assert(false);
		return; 
	}
	
	// atomic free the buff
	atomicFree(buffData);

	// post the semaphore
	_semaphore.post();
}

void BufferPool::freeUsedBuffers()
{
	// release the used buffer
	//
	BUFFERMAP::iterator it = _usedPool.begin();
	while(it != _usedPool.end())
	{
		BufferData* pBuff = (BufferData*)it->second;
		it++;
		
		free(pBuff);
	}
}
void BufferPool::TraceUsedBuff()
{
//	printf("total used buffdata is %d, they are: \n", _allocatedCount);
//	BUFFERMAP::iterator it = _usedPool.begin();
//	while(it != _usedPool.end())
//	{
//		BufferData* pBuff = (BufferData*)it->second;

//		printf("0X%08X\n", pBuff);

//		it++;	
//	}
}

BufferData::BufferData(DWORD buffSize)
: _actualLen(0), _totalLen(buffSize), _allocated(false), _data(NULL)
{
	// allocate memory if buffSize is specified
	if(_totalLen > 0)
	{
		_data = new BYTE[_totalLen];
	}
}

BufferData::BufferData(const BufferData& rhs)
{
	if(this == &rhs)
		return;
	*this = rhs;
}

BufferData& BufferData::operator=(const BufferData& rhs)
{
	if(this == &rhs)
		return *this;

	// if my buff is not allocated before, just located it.
	if(NULL == _data)
	{
		_data = new BYTE[rhs._actualLen];
		_totalLen = rhs._actualLen;
		_actualLen = 0;
	}

	// my total buffer length is less than rhs, need to re-allocate
	if(_totalLen < rhs._actualLen)
	{
		delete [] _data;
		
		_data = new BYTE[rhs._actualLen];
		_totalLen = rhs._actualLen;
		_actualLen = 0;
	}
	
	// copy data
	memcpy(_data, rhs._data, rhs._actualLen);
	_actualLen = rhs._actualLen;
	_allocated = true;

	// set property
	_properties = rhs._properties;

	return *this;
}

BufferData::~BufferData()
{
	if(_data != NULL)
	{
		delete[] _data;
	}
	_data = NULL;
}

bool BufferData::writeData(const BYTE* data, DWORD len)
{
	if(NULL == _data)	
	{
		_data = new BYTE[len];
		_totalLen = len;
	}
	else if(_totalLen < len)
	{
		delete[] _data;
		_data = new BYTE[len];
		_totalLen = len;
	}

	// copy data
	memcpy(_data, data, len);
	
	_actualLen = len;
	_allocated = true;
	
	return true;
}

BYTE* BufferData::getPointerForRead(DWORD& len)
{
	len = _actualLen;
	return _data;
}	

BYTE* BufferData::getPointerForWrite(DWORD len)
{
	if(NULL == _data)	
	{
		_data = new BYTE[len];
		_totalLen = len;
	}
	else if(_totalLen < len)
	{
		delete[] _data;
		_data = new BYTE[len];
		_totalLen = len;
	}

	return _data;
}	

void BufferData::freeDataMemory()
{
	if(_data != NULL)
	{
		delete []_data;
		_data = NULL;
	}
	_actualLen = 0;
	_totalLen = 0;
	_allocated =false;
}

int BufferData::getTotalSize()
{
	return _totalLen;
}

void BufferData::setProperty(std::string property, const ZQ::common::Variant& value)
{
	_properties[property] = value;
}

void BufferData::setProperty(std::map<std::string, ZQ::common::Variant> properties)
{
	std::map<std::string, ZQ::common::Variant>::iterator it = properties.begin();

	while(it != properties.end())
	{
		_properties[it->first] = it->second;
		it++;
	}
}

bool BufferData::getProperty(std::string property, ZQ::common::Variant& value)
{
	PROPERTY::iterator it;
	if( (it = _properties.find(property)) != _properties.end())
	{
		value = it->second;
		return true;
	}

	return false;
}

}}