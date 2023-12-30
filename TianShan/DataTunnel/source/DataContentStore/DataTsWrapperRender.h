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
// Revision History: 


#ifndef __ZQ_DataTsWrapperRender_Process_H__
#define __ZQ_DataTsWrapperRender_Process_H__

#include "GraphFilter.h"
#include "PrivateDataDefine.h"
#include "../Common/TsEncoder.h"


namespace ZQ { 
namespace Content { 
namespace Process {

class TsObjectData : public DataStream::TsData
{
public:
	TsObjectData(ZQ::Content::BufferData& bufferData);
	virtual ~TsObjectData(){};

public:
	size_t read(unsigned char* buf, size_t size);
	
private:
	ZQ::Content::BufferData& _buffData;

	BYTE   _tableCount;

	ObjectHeader _objHeader;

	BYTE   _curTableIndex;    /// index from 0

	DWORD  _objectSize;       /// the object size
	DWORD  _curObjReadPos;    /// index of object 

	bool   _readObjectHeader; /// flag to read object Head

	DWORD  _readTotal;        /// for trace
};



class DataTsWrapperRender : public Filter
{
	friend class Graph;
public:
	DataTsWrapperRender(ZQ::Content::Process::Graph& graph, std::string myName="");
				
protected:	
	/// destructor
	virtual ~DataTsWrapperRender(void);

public:
	/// receiving the buffer coming from previous content process object
	/// since its source does not release the buffer after call receive(), 
	/// so need to release it after processing.
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

	/// this virtual function must be render, to get know current processing progress,
	/// bytes or something else.
	virtual __int64 getProcessedStuff() { return _processedObjectCount; };
	
	/// Notify the base derived object that there is no any coming data
	/// After receiving this notification, the thread process all the received data
	//  then flush the data if required, then stop process until the next starting
	virtual void endOfStream(void);

public:
	/// implementation of NativeThread virtual function
	int run(void);

private:
	void emptyDataQueue(void);

private:
	HANDLE 			              _hStop;
	HANDLE                        _hNotify;
	
	std::string                   _szSourceFile;
	
	DWORD                         _defReadSize;
	DWORD                         _maxKbps;

	DWORD                         _lasttimer;
	DWORD                         _tidAbortBy;

	std::queue<ZQ::Content::BufferData*>   _dataQueue;
	ZQ::common::Mutex                      _dataMutex;	

	bool                                  _bEndOfStream;

	/// variables for store the wrapped output data
	BYTE*                          _tmpWrapOutputBuf;
	DWORD                          _tmpWrapTableCount;


	/// processed object count
	int                            _processedObjectCount; 
	
};

}}}

#endif