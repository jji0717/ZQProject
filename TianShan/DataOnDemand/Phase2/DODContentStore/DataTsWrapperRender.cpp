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
// Desc  : Define the Data Ts Wrapping render
// --------------------------------------------------------------------------------------------

#include "DataTsWrapperRender.h"
#include "PrivateDataDefine.h"

namespace ZQ { 
namespace Content { 
namespace Process {


TsObjectData::TsObjectData(ZQ::Content::BufferData& bufferData)
:_buffData(bufferData)
{
	//
	// init variables
	//
	_readTotal = 0;
	_curTableIndex = 0;
	_curObjReadPos = 0;
	_objectSize = bufferData.getActualLength();
	
	//
	// calculate _totalSize
	//
	ZQ::common::Variant value;
	bool ret = bufferData.getProperty(CNTPRY_TS_TABLE_COUNT, value);
	
	_tableCount = ret ? (BYTE)value : 1;

	_totalSize = _tableCount * sizeof(ObjectHeader) + _objectSize;

	//
	// set ObjectHeader
	//
	if(bufferData.getProperty(CNTPRY_TS_OBJTAG, value))
	{
		DWORD dwValue = (DWORD)value;

		_objHeader.szObjectTag[0] = (char) ((dwValue >> 24) & 0xff);
		_objHeader.szObjectTag[1] = (char) ((dwValue >> 16) & 0xff);
		_objHeader.szObjectTag[2] = (char) ((dwValue >> 8 ) & 0xff);		
	}
	if(bufferData.getProperty(CNTPRY_TS_OBJKEY, value))
	{
		ZQ::common::Variant::BinaryData& bvalue = (ZQ::common::Variant::BinaryData)value;
		int nsize = bvalue.size() < DEFAULT_OBJECT_KEY_LENGTH ? bvalue.size() : DEFAULT_OBJECT_KEY_LENGTH;
		for(int i=0; i<nsize; i++)
		{
			_objHeader.szObjectKey[i] = bvalue[i];
		}
	}
}

// the max of size is a SECTION length
size_t TsObjectData::read(unsigned char* buf, size_t size)
{
	// trace total readed
	_readTotal += size;

	DWORD readsize = 0;
	DWORD toReadSize = size;
	
	if(_curObjReadPos < _objectSize && toReadSize > 0)
	{
		//
		// Read Object Header
		//
		if((_curObjReadPos % MAX_OBJECT_PAYLOAD_LENGTH) == 0)
		{
			DWORD contentSize = 0;

			//
			// read Object Header
			//
			if(_tableCount-1 == _curTableIndex)
			{ 
				// this is the last table
				contentSize = _totalSize - (_curTableIndex+1)*sizeof(ObjectHeader);
				
// 				printf("Current Table Number %d, total = %d\n", _curTableIndex+1, _tableCount);
			}
			else
			{
				// this is NOT the last table, so table payload is the max
				contentSize = MAX_OBJECT_PAYLOAD_LENGTH;

// 				printf("Current Table Number %d, total = %d\n", _curTableIndex+1, _tableCount);
			}

			// set Object Header's content length
			DWORD contentSizeEx = htonl(contentSize);
			memcpy(_objHeader.nObjectContentLength, &contentSizeEx, sizeof(DWORD));
			
			// set position, toReadSize must larger than sizeof(ObjectHeader)
			if(toReadSize > sizeof(ObjectHeader))
			{
				memcpy(buf, (void*)(&_objHeader), sizeof(ObjectHeader));
	
				readsize = sizeof(ObjectHeader);
				toReadSize -= sizeof(ObjectHeader);
			}
			else
			{
				// something must be wrong
				return size - toReadSize;
			}
		}

		//
		// Read Object Data
		//
		DWORD len = 0; 
		BYTE* pData = _buffData.getPointerForRead(len) + _curObjReadPos;

		// re-check the size
		toReadSize = toReadSize <= (_objectSize-_curObjReadPos) ? toReadSize : (_objectSize-_curObjReadPos);

		memcpy(buf+readsize, pData, toReadSize);
		
		// set position
		_curObjReadPos += toReadSize;

		// increase index
		if((_curObjReadPos % MAX_OBJECT_PAYLOAD_LENGTH) == 0)
		{
			_curTableIndex++;
		}
	}

//	printf("currently read bytes %d, total = %d\n", _readTotal, _totalSize);

	return size;
}
	


DataTsWrapperRender::DataTsWrapperRender(ZQ::Content::Process::Graph& graph, std::string myName)
: Filter(graph, myName), _bEndOfStream(false), _processedObjectCount(0)
{
	if(myName == "")
	{
		_myName = "DataTsWrapperRender";
	}
	_tidAbortBy = 0;

	_hStop = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);

	// allocate temp buff
	_tmpWrapTableCount = 2;
	_tmpWrapOutputBuf = new BYTE[_tmpWrapTableCount * MAX_ONE_TABLE_TS_LEN];

	start();
}
			
DataTsWrapperRender::~DataTsWrapperRender(void)
{
	if(isRunning())
	{
		SetEvent(_hStop);
		
		// to make sure if the thread is stopped even it is suspended
		resume();
		
		// is there any issue if the _hStop has been destruct
		// so wait until all the 
		waitHandle(INFINITE);
	}
	
	// close the handle
	if(_hStop != NULL)
	{
		CloseHandle(_hStop);
		_hStop = NULL;
	}
	if(_hNotify != NULL)
	{
		CloseHandle(_hNotify);
		_hNotify = NULL;
	}
	
	// free temp memory
	_tmpWrapTableCount = 0;
	if(_tmpWrapOutputBuf != NULL)
	{
		delete [] _tmpWrapOutputBuf;
		_tmpWrapOutputBuf = NULL;
	}
}

void DataTsWrapperRender::emptyDataQueue(void)
{
	ZQ::common::MutexGuard guard(_dataMutex);
	// remove all the buffer data pointer from the queue.
	while (!_dataQueue.empty())
	{
		ZQ::Content::BufferData* pBuffData = _dataQueue.front();
		
		_pool.free(pBuffData);
		_dataQueue.pop();

		_graph.traceLog(id(), "DataTsWrapperRender: free buffData from pool in emptyDataQueue[BuffData Address: 0x%08X]",
			            pBuffData);
	}	
}

bool DataTsWrapperRender::receive(Filter* upObj, ZQ::Content::BufferData* buff)
{
	// get the BufferData from Uplink Filter
	ZQ::Content::BufferData* receivedBuff = NULL;
	if(_copyUplinkDataBuff)
	{	// copy BuffData
		ZQ::Content::BufferPool& buffpool = _graph.getBuffPool();
		_graph.traceLog(id(), "BufferPool: usage: %d / %d [used/total]", 
				  buffpool.getUsedCount(), buffpool.getPoolSize());
		
		// copy the buffer to the new one
		receivedBuff = _pool.alloc();
		*receivedBuff = *buff;

		_graph.traceLog(id(), "DataTsWrapperRender: alloc buffData from pool. [BuffData Address: 0x%08X]", 
						receivedBuff);
	}
	else
	{   // does not copy BuffData
		receivedBuff = buff;
	}

	if(STOPPED == _processStatus || ABORTED == _processStatus)
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "DataTsWrapperRender: it is in STOPPED status, does not receive data any more");
	
		bool bReleased = releaseBuffer(receivedBuff, this);	
		if(bReleased)
		{
			_graph.traceLog(id(), "DataTsWrapperRender: free buffData from pool. [BuffData Address: 0x%08X]", 
							receivedBuff);
		}
		return false;
	}

	if(_bEndOfStream)
	{
		_graph.writeLog(ZQ::common::Log::L_WARNING, GetCurrentThreadId(), "DataTsWrapperRender: it is end of stream, does not receive data more");
	
		bool bReleased = releaseBuffer(receivedBuff, this);	
		if(bReleased)
		{
			_graph.traceLog(id(), "DataTsWrapperRender: free buffData from pool. [BuffData Address: 0x%08X]", 
							receivedBuff);
		}
		return false;
	}

	
	// put the data to the queue without copying it, so this buffer need to be released after processing
	ZQ::common::MutexGuard guard(_dataMutex);
	_dataQueue.push(receivedBuff);
	
	_graph.traceLog(GetCurrentThreadId(), "DataTsWrapperRender: Receive Buffer Data from up side process object with actual length %d", buff->getActualLength());

	SetEvent(_hNotify);

	return true;
}

bool DataTsWrapperRender::begin(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "DataTsWrapperRender::begin() enter");
	
	// To avoid invoking begin() during its processing
	if(_processStatus == ACTIVE)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "DataTsWrapperRender: the task did not complete yet, can not initial new work");

		return false;
	}

	//
	// check the begin is invoked by pause - have begun or just begin
	//
	if(_processStatus != PAUSED)    // begin a new job
	{	
		_tidAbortBy = 0;
		
		_bEndOfStream = false;

		// make sure to empty the queue
		emptyDataQueue();
	}
	else
	{
		// resume the native thread
		start();
	}
	// resume the thread
	_processStatus = ACTIVE;

	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "DataTsWrapperRender::begin() leave");

	return true;
}

bool DataTsWrapperRender::pause(void)
{
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DataTsWrapperRender::pause() enter");

	_processStatus = PAUSED;

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DataTsWrapperRender::pause() leave");
	
	// suspend the thread
	suspend();
		
	return true;
}

bool DataTsWrapperRender::abort(void)
{
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DataTsWrapperRender::abort() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = ABORTED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DataTsWrapperRender::abort() leave");
	
	return true;
}
	

void DataTsWrapperRender::stop(void)
{
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DataTsWrapperRender::stop() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = STOPPED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DataTsWrapperRender::stop() leave");
}

void DataTsWrapperRender::quit(void)
{
	SetEvent(_hStop);
}

void DataTsWrapperRender::endOfStream(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "DataTsWrapperRender: Get end of stream notification");

	_bEndOfStream = true;

	SetEvent(_hNotify);
}

int DataTsWrapperRender::run(void)
{
	_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "DataTsWrapperRender::run() enter");

	bool bContinue = true;
	DWORD dwWaitStatus = 0;

	HANDLE handles[2] = { _hStop, _hNotify };

	ZQ::Content::BufferData* buffData = NULL; 
	while(bContinue)
	{
		dwWaitStatus = WaitForMultipleObjects(2, handles, false, INFINITE);
		switch(dwWaitStatus)
		{
		// received stop event, the thread will stop
		case WAIT_OBJECT_0:
			bContinue = false;
			_graph.writeLog(ZQ::common::Log::L_INFO, id(), "DataTsWrapperRender: get a thread exit event");
			break;

		// received the Notify event
		case WAIT_OBJECT_0 + 1:
		{	
			//
			// check whether this thread is abort by Graph
			//
			if(STOPPED == _processStatus || ABORTED == _processStatus)
			{
				// make sure to remove, 
				emptyDataQueue();

				if(ABORTED == _processStatus)
				{
					_graph.writeLog(ZQ::common::Log::L_INFO, id(), "DataTsWrapperRender: It was aborted by Graph, triggered by thread 0x%08X, release queued objects", _tidAbortBy);
				}
				
				// notify graph this filter processing completed
				_graph.notifyCompletion(*this);
				
				continue;	
			}

			//
			// get one data
			//
			ZQ::Content::BufferData* pBuffData = NULL;
			{
				ZQ::common::MutexGuard guard(_dataMutex);
				if(!_dataQueue.empty())
				{
					pBuffData = _dataQueue.front();
					_dataQueue.pop();
				}
			}

			// write the buff to disk
			if(pBuffData != NULL)
			{			
				_processedObjectCount++;
				
				ZQ::common::Variant vObjName;
				pBuffData->getProperty(CNTPRY_TS_OBJECT_NAME, vObjName);
				// get TableCount, TableId, TableIdExt and PID
				ZQ::common::Variant vTableCount = BYTE(0x01);
				pBuffData->getProperty(CNTPRY_TS_TABLE_COUNT, vTableCount);
				ZQ::common::Variant vTableId = BYTE(0x10);
				pBuffData->getProperty(CNTPRY_TS_TABLE_ID, vTableId);
				ZQ::common::Variant vTableIdExt = WORD(0x0100);
				pBuffData->getProperty(CNTPRY_TS_TABLE_IDEXT, vTableIdExt);
				ZQ::common::Variant vPid;
				pBuffData->getProperty(CNTPRY_TS_PID, vPid);
				ZQ::common::Variant vVersionNum = BYTE(0x00);
				pBuffData->getProperty(CNTPRY_TS_VERSION_NUM, vVersionNum);
				
				std::string objName = (std::string)vObjName;
				BYTE tableCount = (BYTE)vTableCount;
				BYTE tableId = (BYTE)vTableId;
				WORD tableIdExt = (WORD)vTableIdExt;
				WORD pid = (WORD)vPid;

				_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "DataTsWrapperRender: %d buffer data for object %s has been processed", 
					            _processedObjectCount, objName.c_str());

				_graph.traceLog(id(), "DataTsWrapperRender: objectName=%s, PID=0x%04X, tableCount=%d, tableId=0x%02X, tableIdExt=0x%04X", 
					objName.c_str(), pid, tableCount, tableId, tableIdExt);

				DWORD wrapStart = GetTickCount();
				// wrapper the object
				DataStream::TsEncoder tsEncoder;
				TsObjectData tsObjData(*pBuffData);
				DataStream::TsSectionEncoder objSecEnc(tableId, tableIdExt, (BYTE)vVersionNum);
				tsEncoder.encode(pid, &objSecEnc, tsObjData);
				
				// check the temp output memory, if necessary, re-allocate the buff
				if((DWORD)tableCount > _tmpWrapTableCount)
				{
					_tmpWrapTableCount = (DWORD)tableCount;
					if(_tmpWrapOutputBuf != NULL)
					{
						delete []_tmpWrapOutputBuf;
						_tmpWrapOutputBuf = NULL;
					}
				}
				if(NULL == _tmpWrapOutputBuf)
				{
					_tmpWrapOutputBuf = new BYTE[_tmpWrapTableCount * MAX_ONE_TABLE_TS_LEN];
				}

				// set the buff max length
				DWORD actualLen = 0;

				// read wrapped data to output DataBuffer
				size_t len = 0;
				BYTE* tsPointer = NULL;
				while( (STOPPED != _processStatus) && (tsPointer = tsEncoder.nextTable(len)) != NULL) 
				{
					memcpy(&_tmpWrapOutputBuf[actualLen], tsPointer, len);
					actualLen += len;
				}
				DWORD wrapEnd = GetTickCount();
				_graph.traceLog(id(), "DataTsWrapperRender: Wrapper Object spend %d ms", wrapEnd - wrapStart);

				// reuse pBuffData to store the wrapped output data and forward to next filter
				pBuffData->writeData(_tmpWrapOutputBuf, actualLen);

				// deliver buff to IO renders
				deliverBuffer(pBuffData);

				// release the buff 
				bool bReleased = releaseBuffer(pBuffData);
				if(bReleased)
				{
					_graph.traceLog(id(), "DataTsWrapperRender: free buffData from pool. [BuffData Address: 0x%08X]", 
									pBuffData);
				}
				
				// go on next loop
				ZQ::common::MutexGuard guard(_dataMutex);
				if(!_dataQueue.empty())
				{
					SetEvent(_hNotify);
					continue;
				}
			}

			// endOfStream() set the _hNotify event
			if(_bEndOfStream)
			{	// end of stream
				_graph.writeLog(ZQ::common::Log::L_INFO, id(), "DataTsWrapperRender: reaches the end of stream, there are %d buffer processed in all", 
								_processedObjectCount);

				// notify all the connected renders that reaches the end of stream
				notifyEndOfStream();

				_processedObjectCount = 0;
				// set the status
				_processStatus = STOPPED;
				
				// make sure to remove, 
				emptyDataQueue();

				// notify graph this filter processing completed
				_graph.notifyCompletion(*this);

				continue;
			}
			break;
		}
		// received timeout or failed, exit the thread.
		case WAIT_TIMEOUT:
		case WAIT_FAILED:
		default:
			bContinue = false;
			break;
		}		
	}
	_graph.writeLog(ZQ::common::Log::L_INFO, id(), "DataTsWrapperRender::run() leave");
	
	return 1;	
}

}}}