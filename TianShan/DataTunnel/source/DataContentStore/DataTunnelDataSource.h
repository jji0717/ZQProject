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
// Desc  : Define the NTFS Folder source for processing
// --------------------------------------------------------------------------------------------
// Revision History: 

#ifndef __ZQ_DODDataSource_Process_H__
#define __ZQ_DODDataSource_Process_H__

#include "GraphFilter.h"
#include "PrivateDataDefine.h"


namespace ZQ { 
namespace Content { 
/// namespace Process presents the processing to the content, such as reading/writing
namespace Process {

#define DEFAULT_SOURCE_BUFF_COUNT	30
#define MAX_READ_SIZE               (1024*64)     // do not change this, coz to VStrm, it is fixed
#define FILE_NAME_LENGTH            1024


class ObjectSort;

class DataObject
{
	friend class ObjectSort;
public:
	typedef enum { 
					OBJ_TYPE_FILE = 0, 
				    OBJ_TYPE_MSG, 
				    OBJ_TYPE_INDEXTABLE, 
				    OBJ_TYPE_INDEXCONTENT 
				} OBJTYPE;

	/// construct for file content object
	DataObject(DWORD tag, WORD basePID, DWORD encryptType, WORD streamCount, BYTE versionNum, 
				std::string objName, std::string fileName);
	/// construct for index content object
	DataObject(DWORD tag, WORD basePID, DWORD encryptType, WORD streamCount, BYTE versionNum, 
				std::string objName, HANDLE hContent, DWORD startPos, DWORD count);
	/// construct for index object
	DataObject(DWORD tag, WORD basePID, DWORD encryptType, WORD streamCount, BYTE versionNum, 
				std::vector<DataObject*>& cntObjNames);
	/// construct for message content object
	DataObject(DWORD tag, WORD basePID, DWORD encryptType, BYTE versionNum, 
				std::string msgDestination, std::string msgContent);

	virtual ~DataObject();

public:
	int  getObjectType() { return  _objType; };
	/// read data to buffer and do encryption
	// load file based content object, Index-Content or index table object
	bool loadDataSource(BYTE tableId, WORD tableIdExt, ZQ::Content::BufferData& bufferData);  

public:
	bool  isValid() { return _isValid; };

	std::string  getObjName() { return _objName; };
	WORD  getPID() { return _pid; };
	BYTE  getTableCount() { return _tableCount; };
	BYTE  getTableId() { return _tableId; }; 
	WORD  getTableIdExt() { return _tableIdExt; };
	BYTE  getVersionNumber() { return _versionNumber; };
	const BYTE* getObjectKey(int& length) { length = DEFAULT_OBJECT_KEY_LENGTH; return _objectKey; };
	DWORD getTag() { return _tag; };
	
	std::string getLastError() { return _lastError; };
	
private:
	bool doEncryption(ZQ::Content::BufferData& bufferData);

	void setBufferProperties(ZQ::Content::BufferData& bufferData);

private:

	std::string        _lastError;

	OBJTYPE            _objType;	
	std::string        _objName;
	int                _encryptType;

	bool               _isValid;          // load successfully or not, if not, discard it.

	// TS parameter
	DWORD              _tag;
	WORD               _pid;
	BYTE               _tableId;
	WORD               _tableIdExt;
	BYTE	           _objectKey[DEFAULT_OBJECT_KEY_LENGTH];
	BYTE               _tableCount;
	BYTE               _versionNumber;    // need to maintain by DODApp
	
	// for file mode data and message mode
	std::string        _fileName;	   
	
	// variables for IndexContent Mode data
	HANDLE             _hContent;
	DWORD              _startPos;
	DWORD              _cntLength;

	// variables for IndexTable object
	DWORD                     _position;
	std::vector<DataObject*>* _pCntObjs;

	/// temp memory block for temp encryption output
	/// Here define it as static is to avoid frequently alloc and free memory in doEncrytion().
	/// And currently, ONLY one DataObject object does encryption at a time, no concurrent doing, 
	/// so, in the further, if there is concurrent, BufferPool is a choice or dynamic alloc-free in doEncryption() directly.
	static BYTE               _tmpEncryptionOutput[MAX_ONE_TABLE_TS_LEN];

};

class ObjectSort
{
public:  
	bool operator() (DataObject* left, DataObject* right) 
	{
		return left->_objName.compare(right->_objName) < 0;
	}
};

class DODDataSource : public SourceFilter
{
	friend class Graph;
public:
	DODDataSource(ZQ::Content::Process::Graph& graph, DWORD yieldTime=0, std::string myName="");
				
protected:	
	/// destructor
	virtual ~DODDataSource(void);

public:
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
	virtual __int64 getTotalStuff() { return _totalObjects; };

	/// this virtual function must be render, to get know current processing progress,
	/// bytes or something else.
	virtual __int64 getProcessedStuff() { return 0; };

	/// set the object param for the folder
	void setObjectParam(WORD pid, WORD streamType, WORD streamCount, bool sendIndexTable, 
						DWORD tag, int encryptType, WORD versionNum, int dataType);

public:
	/// implementation of NativeThread virtual function
	int run(void);

private:
	void releaseObjects(void);

	/// whether the path is a folder or file
	/// param[out] fileName - return the file name, not include the path, if return value is 0
	/// return -  -1 no such folder/file
	///            0 is a file
	///            1 is a Folder
	int pathIsFolder(const std::string& path, std::string& fileName);

private:
	HANDLE 			              _hStop;
	HANDLE                        _hNotify;
	
	std::string                   _szSourceFile;
	
	DWORD                         _lasttimer;
	DWORD                         _tidAbortBy;

	DWORD                         _yieldTime;

private:
	WORD                          _basePID;
	WORD                          _streamType;
	DWORD                         _encryptType;
	WORD                          _streamCount;
	bool                          _sendIndexTable;
	DWORD                         _objTag;
	BYTE                          _versionNum;
	int                           _dataType;

	static time_t                 _basetime;

	typedef std::vector<DataObject*> OBJECTCOLLECTION;

	OBJECTCOLLECTION              _objCollection;
	DataObject*                   _pObjIndexTable;

	int                           _totalObjects;
	
private:
	bool  recursiveFolder(std::string folderName);

	bool  initFileObjects();
	bool  initIndexContentObjects();
	
};

} } }

#endif // __ZQ_DODDataSource_Process_H__
