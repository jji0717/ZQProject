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
// Desc  : Implement the NTFS Folder source for processing
// --------------------------------------------------------------------------------------------
 


#include "DataTunnelDataSource.h"
#include "urlstr.h"
#include "bufferpool.h"

#include <algorithm>

extern "C" {

#include "itv_parse\Parser\inc\parse.h"

}

namespace ZQ { 
namespace Content { 
namespace Process {

#define PATH_IS_INVALID    -1
#define PATH_IS_A_FOLDER    1
#define PATH_IS_A_FILE      0

#define DATA_TYPE_BOOT_EPG             50
#define DATA_TYPE_FULL_EPG             51
#define MIN_FULL_EPG_FILENAME_LEN      10

BYTE DataObject::_tmpEncryptionOutput[MAX_ONE_TABLE_TS_LEN];

static bool fixpath(std::string& path, bool normal2back)
{
	char* pathbuf = new char[path.length() +2];
	if (NULL ==pathbuf)
		return false;

	strcpy(pathbuf, path.c_str());
	pathbuf[path.length()] = '\0';
	for (char* p = pathbuf; *p; p++)
	{
		if ('\\' == *p || '/' == *p)
		{
			if(normal2back)
			{
				*p = '/';
			}
			else
			{
				*p = '\\';
			}
		}
	}
	
	path = pathbuf;
	
	delete []pathbuf;

	return true;

}

static unsigned long timeval()
{
	unsigned long rettime = 1;

	FILETIME systemtimeasfiletime;
	LARGE_INTEGER litime;

	GetSystemTimeAsFileTime(&systemtimeasfiletime);
	memcpy(&litime,&systemtimeasfiletime,sizeof(LARGE_INTEGER));
	litime.QuadPart /= 10000;  //convert to milliseconds
	litime.QuadPart &= 0xFFFFFFFF;    //keep only the low part
	rettime = (unsigned long)(litime.QuadPart);

	return rettime;
}
	
char convertHexChar(char ch) 
{
	if((ch>='0')&&(ch<='9'))
		return ch-0x30;
	else if((ch>='A')&&(ch<='Z'))
		return ch-'A'+10;
	else if((ch>='a')&&(ch<='z'))
		return ch-'a'+10;
	else 
		return (-1);
}

// construction for OBJ_FTYPE_FILE
DataObject::DataObject(DWORD tag, WORD basePID, DWORD encryptType, WORD streamCount, BYTE versionNum, 
					   std::string objName, std::string fileName)
: _tag(tag), _pid(basePID), _encryptType(encryptType), _pCntObjs(NULL)
{
	//
	// initialize 
	//
	_isValid = false;
	_versionNumber = versionNum;
	_tableCount = 0;

	_hContent = INVALID_HANDLE_VALUE;
	_startPos = 0;
	_cntLength = 0;

	_objType = OBJ_TYPE_FILE;
	
	_fileName = fileName;
	_objName = objName;

	memset(_objectKey, 0x0, DEFAULT_OBJECT_KEY_LENGTH);

	//
	// remove extension from objName, if there is 
	//
	int nIndex = _objName.find_last_of('.');

	if(nIndex != std::string::npos)
	{
		_objName = _objName.substr(0, nIndex);
	}
	
	//
	// trim the characters extends 16 bytes from the object header
	//
	int objLen = objName.size();
	if(objLen > DEFAULT_INDEX_TABLE_DESC_LENGTH)
	{
		objName = objName.substr(objLen-DEFAULT_INDEX_TABLE_DESC_LENGTH, DEFAULT_INDEX_TABLE_DESC_LENGTH);
	}
	
	//
	// re-set pid if it is sub stream
	//
	if(streamCount > 0)
	{
		int len = _objName.length();

		if(len >= 2)
		{
			_pid += ((_objName.at(len-1) + _objName.at(len-2)) % streamCount);
		}
	}
}

// construction for OBJ_TYPE_INDEXCONTENT
DataObject::DataObject(DWORD tag, WORD basePID, DWORD encryptType, WORD streamCount, BYTE versionNum, 
					   std::string objName, HANDLE hContent, DWORD startPos, DWORD count)
: _tag(tag), _pid(basePID), _encryptType(encryptType), _pCntObjs(NULL), _hContent(hContent), _startPos(startPos), _cntLength(count)
{
	//
	// initialize 
	//
	_isValid = false;
	_versionNumber = versionNum;
	_tableCount = 0;
	
	_objType = OBJ_TYPE_INDEXCONTENT;
	_objName = objName;

	memset(_objectKey, 0x0, DEFAULT_OBJECT_KEY_LENGTH);

	//
	// remove extension from objName, if there is 
	//
	int nIndex = _objName.find_last_of('.');

	if(nIndex != std::string::npos)
	{
		_objName = _objName.substr(0, nIndex);
	}

	//
	// re-set pid if it is sub stream
	//
	if(streamCount > 0)
	{
		int len = _objName.length();

		if(len >= 2)
		{
			_pid += ((_objName.at(len-1) + _objName.at(len-2)) % streamCount);
		}
	}
}

// construction for OBJ_TYPE_INDEXTABLE
DataObject::DataObject(DWORD tag, WORD basePID, DWORD encryptType, WORD streamCount, BYTE versionNum, 
					   std::vector<DataObject*>& cntObjs)
: _tag(tag), _pid(basePID), _encryptType(encryptType), _pCntObjs(&cntObjs), _position(1)
{
	//
	// initialize 
	//
	_isValid = false;
	_versionNumber = versionNum;
	_tableCount = 0;

	_hContent = INVALID_HANDLE_VALUE;
	_startPos = 0;
	_cntLength = 0;

	_objType = OBJ_TYPE_INDEXTABLE;
	
	memset(_objectKey, 0x0, DEFAULT_OBJECT_KEY_LENGTH);

	//
	// set the default object name
	//
	_objName = "IndexTable";
	
	//
	// reset the pid
	//
	_pid = basePID + streamCount;
}

// construction for OBJ_TYPE_MSG
DataObject::DataObject(DWORD tag, WORD basePID, DWORD encryptType, BYTE versionNum, 
					   std::string msgDestination, std::string msgContent)
: _tag(tag), _pid(basePID), _encryptType(encryptType), _pCntObjs(NULL)
{
	//
	// initialize 
	//
	_isValid = false;
	_versionNumber = versionNum;
	_tableCount = 0;

	_hContent = INVALID_HANDLE_VALUE;
	_startPos = 0;
	_cntLength = 0;
	
	_objType = OBJ_TYPE_MSG;
	
	_objName = msgDestination;    // objname as the destination
	_fileName = msgContent;       // filename as the msg content

	memset(_objectKey, 0x0, DEFAULT_OBJECT_KEY_LENGTH);	
}

DataObject::~DataObject()
{
}

//
// NOTICE:
// The allocated buffsize must with additional one byte for STBParser encryption, 
// otherwise there will be memory overflow. 
//
bool DataObject::loadDataSource(BYTE tableId, WORD tableIdExt, ZQ::Content::BufferData& bufferData)
{
	//
	// save TableId and TableIdEx
	//
	_tableId = tableId; 
	_tableIdExt = tableIdExt;

	_isValid = false;

	BYTE* buffInput = NULL;
	DWORD objectSize = 0;

	switch(_objType)
	{
	case OBJ_TYPE_MSG:
		{
			//
			// Use DataBuffer in the pool to store the memory
			// 
			objectSize = _fileName.size();
			buffInput = bufferData.getPointerForWrite(objectSize+1);
			
			//
			// copy data to buffer
			//
			bufferData.writeData((BYTE*)_fileName.c_str(), objectSize);
		}
		break;
	case OBJ_TYPE_FILE:
		{
			//
			// create file handle for data reading
			//
			HANDLE hFile = ::CreateFileA(_fileName.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

			if (INVALID_HANDLE_VALUE == hFile)
			{
				_lastError = "Fail to open file " + _fileName;
				return false;
			}
			objectSize = GetFileSize(hFile, NULL);

			//
			// Use DataBuffer in the pool to store the memory
			// 
			buffInput = bufferData.getPointerForWrite(objectSize+1);
			
			//
			// Read data from file to buffer
			//
			DWORD toRead = 0;
			if(!ReadFile(hFile, buffInput, objectSize, &toRead, NULL) || objectSize != toRead)
			{
				_lastError = "Fail to read file " + _fileName;

				objectSize = 0;

				CloseHandle(hFile);
				return false;
			}
			
			//
			// reset the buffer size, it is must
			//
			bufferData.setActualLength(objectSize);

			CloseHandle(hFile);
		}
		break;
	case OBJ_TYPE_INDEXTABLE:
		{
			//
			// calculate IndexTable
			//
			objectSize = (_pCntObjs->size() - _position) * sizeof(IndexTableDescriptor);

			//
			// Use DataBuffer in the pool to store the memory
			// 
			buffInput = bufferData.getPointerForWrite(objectSize+1);

			IndexTableDescriptor tmpIndexTableDes;
			
			for(DWORD i=_position; i<_pCntObjs->size(); i++)
			{
				tmpIndexTableDes.byTableCount = (*_pCntObjs)[i]->getTableCount();
				int objkeyLen = 0;
				memcpy(tmpIndexTableDes.szPID, (*_pCntObjs)[i]->getObjectKey(objkeyLen), DEFAULT_OBJECT_KEY_LENGTH);
				
				memcpy(tmpIndexTableDes.szDesc, (*_pCntObjs)[i]->getObjName().c_str(), DEFAULT_INDEX_TABLE_DESC_LENGTH);

				memcpy(buffInput + (i-_position)*sizeof(IndexTableDescriptor), &tmpIndexTableDes, sizeof(IndexTableDescriptor));
			}

			//
			// reset the buffer size 
			//
			bufferData.setActualLength(objectSize);
		}
		break;
	case OBJ_TYPE_INDEXCONTENT:
		{
			//
			// read data from the IndexContent, not implemented now
			//
			_lastError = "IndexContent Mode is not support currently";
			return false;
			
		}
		break;
	default:
		{
			_lastError = "Programming level Fetal error, improper invoking function";
			return false;
		}
		break;
	} // end of switch();

	
	//
	// do encryption
	//
	bool bRet = true;
	if(_encryptType != ENCRYPT_MODE_NONE )
	{
		bool bRet = doEncryption(bufferData);
		if(!bRet)
		{
			_lastError = "Do encryption failed on object " + _objName;
			return false;
		}
	}
	
	//
	// calculate table count
	//
	DWORD bufLength = bufferData.getActualLength();
	DWORD tbcount = (bufLength / MAX_OBJECT_PAYLOAD_LENGTH) + (bufLength % MAX_OBJECT_PAYLOAD_LENGTH ? 1 : 0);
	_tableCount = (BYTE)tbcount;

	//
	// set ObjectKey
	//
	_objectKey[1] = _tableId;                     // (0x) 0010 -> 0010  
	WORD wTmp = htons(_tableIdExt);	              // (0x) 0100 -> 0001
	memcpy(_objectKey+2, &wTmp, sizeof(wTmp));

	// this object is valid.
	_isValid = true;

	//
	// set the object properties to the buffer
	//
	setBufferProperties(bufferData);

	return bRet;
}

bool DataObject::doEncryption(ZQ::Content::BufferData& bufferData)
{
	//
	// get source buffer
	//
	DWORD srcLength = 0;
	BYTE* pData = bufferData.getPointerForRead(srcLength);

	//
	// do encryption
	//
	int curEncType = _encryptType;
	BYTE compress = (curEncType & 0x4) >> 2;
	curEncType &= 0x03;

	// check file size
	if(srcLength < 3)
	{
		_lastError = "File content length is less than 3";
		return false;
	}
		
	ITV_DWORD itv_err;

	WI_ParseInfo encryptData;

	if( (pData[0] == 0xEF)
	 && (pData[1] == 0xBB)
	 && (pData[2] == 0xBF) )
	{
		pData += 3;

		srcLength -= 3;   // no such logic in OLD DOD, need to verify
	}
	else if( (pData[0] == 0xFF)
		&& (pData[1] == 0xFE) )
	{
		pData += 2;

		srcLength -= 2;   // no such logic in OLD DOD, need to verify
	}
	
	encryptData.input = pData;
	encryptData.input_size = srcLength;
	encryptData.output = _tmpEncryptionOutput;
	encryptData.output_size = MAX_OBJECT_LENGTH;

	itv_err = WI_Parse_init(&encryptData);

	if(itv_err != 0)
	{
		char msg[256];
		sprintf(msg, "WI_Parse_init() failed with error code %d", itv_err);
		
		_lastError = msg;

		return false;
	}

	char funName[100];
	switch(curEncType)
	{
	case ENCRYPT_MODE_NONE:
		if(ENCRYPT_MODE_COMPRESS_SIMPLE == compress)
		{
			itv_err = ItvParse_Compress(encryptData.output, &encryptData.real_outsize, 
				encryptData.input, encryptData.input_size);

			sprintf(funName, "ItvParse_Compress()");
		}
		break;
	case ENCRYPT_MODE_ILP_NORMAL:
		if (ENCRYPT_MODE_COMPRESS_SIMPLE == compress) 
		{
			itv_err = ItvParse_ParseAndCompress(&encryptData);	
			
			sprintf(funName, "ItvParse_ParseAndCompress()");
		} 
		else
		{
			itv_err = WI_Parse_parse(&encryptData);
			sprintf(funName, "WI_Parse_parse()");
		}
		break;
	case ENCRYPT_MODE_ILP_SIMPLE:
		if (compress == ENCRYPT_MODE_COMPRESS_SIMPLE) 
		{
			itv_err = ItvParse_SimpleParseAndCompress(&encryptData);			
			sprintf(funName, "ItvParse_SimpleParseAndCompress()");
		}
		else 
		{
			itv_err = Itv_Parse_SimpleParse(&encryptData);			
			sprintf(funName, "Itv_Parse_SimpleParse()");
		}
		break;
	default:
		{
			char msg[256];
			sprintf(msg, "Invalid encryption type %d", _encryptType);

			_lastError = msg;

			return false;
		}
	}

	if(itv_err != 0)
	{
		char msg[256];
		sprintf(msg, "%s failed with error code %d", funName, itv_err);

		_lastError = msg;

		return false;
	}

	//
	// use the encrypted buffer to replace raw data.
	//
	DWORD desLength = encryptData.real_outsize;
	bufferData.writeData(_tmpEncryptionOutput, desLength);

	return true;
}

void DataObject::setBufferProperties(ZQ::Content::BufferData& bufferData)
{	
	// set object PID
	bufferData.setProperty(CNTPRY_TS_PID, _pid);

	// set object name
	bufferData.setProperty(CNTPRY_TS_OBJECT_NAME, _objName);
	
	// set object TableCount
	bufferData.setProperty(CNTPRY_TS_TABLE_COUNT, _tableCount);

	// set object TableId
	bufferData.setProperty(CNTPRY_TS_TABLE_ID, _tableId);

	// set object TableIdExt
	bufferData.setProperty(CNTPRY_TS_TABLE_IDEXT, _tableIdExt);

	// set object Object Key
	ZQ::common::Variant varObjKey((void*)_objectKey, DEFAULT_OBJECT_KEY_LENGTH);
	bufferData.setProperty(CNTPRY_TS_OBJKEY, varObjKey);

	// set object Version Number
	bufferData.setProperty(CNTPRY_TS_VERSION_NUM, _versionNumber);

	// set object Tag
	bufferData.setProperty(CNTPRY_TS_OBJTAG, _tag);
}

time_t DODDataSource::_basetime = 0;

DODDataSource::DODDataSource(ZQ::Content::Process::Graph& graph, DWORD yieldTime, std::string myName)
: SourceFilter(graph, "DOD", myName), _pObjIndexTable(NULL), _yieldTime(yieldTime)
{
	_basePID = 0;
	_streamType = 0;
	_encryptType = 0;
	_streamCount = 0;
	_sendIndexTable = false;
	_dataType = 0;

	if(myName == "")
	{
		_myName = "DODDataSource";
	}

	_tidAbortBy = 0;

	_hStop = CreateEvent(NULL, false, false, NULL);
	_hNotify = CreateEvent(NULL, false, false, NULL);

	start();
}

DODDataSource::~DODDataSource(void)
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
	
	// release objects
	releaseObjects();
}

void DODDataSource::releaseObjects(void)
{
	for(DWORD i=0; i<_objCollection.size(); i++)
	{
		DataObject* pObj = _objCollection[i];
		delete pObj;
	}
	_objCollection.clear();

	if(_pObjIndexTable != NULL)
	{
		delete _pObjIndexTable;
		_pObjIndexTable = NULL;
	}
}

bool DODDataSource::begin(void)
{
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DODDataSource::begin() enter");
	
	// To avoid invoking begin() during its processing
	if(_processStatus == ACTIVE)
	{
		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "DODDataSource: The task did not complete yet, can not initial new work");

		return false;
	}

	// check the begin is invoked by pause - begin or just begin
	if(_processStatus != PAUSED)
	{
		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "DODDataSource: Data wrapping parameters are:"
			"ObjTag=0x%08X, PID=0x%04X, StreamType=%d, StreamCount=%d, DataType=%d, SendIndexTable=%s, EncryptionType=%d, Version=%d", 
			_objTag, _basePID, _streamType, _streamCount, _dataType, _sendIndexTable?"true":"false", _encryptType, _versionNum);

		_tidAbortBy = 0;
		
		//
		// release objects from collection, if there are
		//
		releaseObjects();

		//
		// check whether the URL is a legal one, if yes, and fix the path
		//
		std::string srcPath = _graph.getSourceURL();
		fixpath(srcPath, true);

		ZQ::common::URLStr srcUrl(srcPath.c_str());
		const char* protocol = srcUrl.getProtocol();

		if (stricmp("msg", protocol) == 0)
		{
			// DODSource did not check length of destination, since DODApp make sure it is right
			// msg protocol: msg://destination/message_content
			// the destination is 6 characters string

			int pos = 6;   // 6 - length of msg://
			// get message destination, it should be 6 characters
			std::string msgDestination = srcPath.substr(pos, 6);

			pos += 6 + 1;   // 6 - length of destination, 1 - / after destination
			// get the message content
			std::string _msgContent = srcPath.substr(pos, srcPath.length() - pos);
			
			// create object
			DataObject* pObj = new DataObject(_objTag, _basePID, _encryptType, _versionNum, msgDestination, _msgContent);
			_objCollection.push_back(pObj);
		}
		else if(stricmp("file", protocol) == 0)  // single file or folder
		{
			//
			// fix the file path
			//
			std::string host = srcUrl.getHost();
			srcPath = srcUrl.getPath();

			if (host.empty() || 0 == host.compare("."))
			{
				fixpath(srcPath, true);
			}
			else
			{
				srcPath = std::string(LOGIC_FNSEPS LOGIC_FNSEPS) + srcUrl.getHost() + LOGIC_FNSEPS +srcPath; 
				fixpath(srcPath, false);
			}

			//
			// add index table object to object collection, if it is required, 
			// if send IndexTable, the first object in the indextable object
			//
			int sortPos = 0;
			if(_sendIndexTable)
			{
				// 
				// new the IndexTable Object, use the sourceURL as the object name
				//
				DataObject* pObj = new DataObject(_objTag, _basePID, ENCRYPT_MODE_NONE, 
					                              _streamCount, _versionNum, _objCollection);  // calculate IndexTable from 1, not include myself

				// insert IndexTable object to the ahead of object collection.
				_objCollection.push_back(pObj);

				sortPos = 1;
			}

			//
			// folder/SingleFile process
			//
			std::string filename;
			int ret = pathIsFolder(srcPath, filename);
			switch(ret)
			{
			case PATH_IS_A_FOLDER:	
				{
					//
					// get all the objects 
					//
					if(srcPath[srcPath.size()-1] != '\\' || srcPath[srcPath.size()-1] != '/' )
					{
						srcPath += "/";
					}

					DWORD recStart = GetTickCount();
					if(!recursiveFolder(srcPath))
					{
						_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "DODDataSource: Failed to recursive %s", srcPath);
						return false;
					}
					DWORD recEnd = GetTickCount();
					_graph.traceLog(GetCurrentThreadId(), "DODDataSource: recursiveFolder spend %d ms", recEnd - recStart);

					if( (_sendIndexTable && _objCollection.size() == 1) || _objCollection.size() == 0 )
					{
						_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "DODDataSource: No object found from %s", srcPath);
						return false;
					}
					//
					// sort the objects by order of object name
					//
					OBJECTCOLLECTION::iterator it = _objCollection.begin();
					if(1 == sortPos)
					{
						it++;
					}
					std::sort(it, _objCollection.end(), ObjectSort());

					_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DODDataSource: %d object was found", _objCollection.size());
				}				
				break;
			case PATH_IS_A_FILE:
				{
					DataObject* pObj = new DataObject(_objTag, _basePID, _encryptType, _streamCount, _versionNum, filename, srcPath);
					_objCollection.push_back(pObj);
				}
				break;
			case PATH_IS_INVALID :
			default:
				{
					std::string errorstr = "DODDataSource: The source URL " + srcPath + " does not exist";
					_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "%s", errorstr.c_str());

					// notify Graph the last error
					_graph.setLastError(0,errorstr);
					return false;
				}
			}
		}
		else
		{
			_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "DODDataSource: The source URL %s is not legal or not supported", srcPath.c_str());
			return false;
		}

		//
		// set the total objects
		//
		_totalObjects = _objCollection.size();
		if(_sendIndexTable)
		{
			_totalObjects++;
		}
	}
	else
	{
		// resume the thread
		start();
	}

	_processStatus = ACTIVE;

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DODDataSource::begin() leave");

	SetEvent(_hNotify);

	return true;
}

bool DODDataSource::pause(void)
{
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DODDataSource::pause() enter");

	_processStatus = PAUSED;

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DODDataSource::pause() leave");
	
	// suspend the thread
	suspend();
		
	return true;
}

bool DODDataSource::abort(void)
{
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DODDataSource::abort() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = ABORTED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DODDataSource::abort() leave");
	
	return true;
}

void DODDataSource::stop(void)
{
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DODDataSource::stop() enter");
		
	// remember who trigger the Graph abort
	_tidAbortBy = GetCurrentThreadId();

	// set the status
	_processStatus = STOPPED;

	SetEvent(_hNotify);

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DODDataSource::stop() leave");
	
}

void DODDataSource::quit(void)
{
	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DODDataSource::quit() enter");

	SetEvent(_hStop);

	_graph.writeLog(ZQ::common::Log::L_DEBUG, GetCurrentThreadId(), "DODDataSource::quit() leave");
}

void DODDataSource::setObjectParam(WORD pid, WORD streamType, WORD streamCount, 
								   bool sendIndexTable, DWORD tag, int encryptType, 
								   WORD versionNum, int dataType)
{
	_basePID = pid;
	_streamType = streamType;
	_encryptType = encryptType;
	_streamCount = streamCount;
	_sendIndexTable = sendIndexTable;
	_objTag = tag;
	_versionNum = versionNum <= 31 ? versionNum : 0;
	_dataType = dataType;
}


int DODDataSource::run(void)
{	
	_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "DODDataSource::run() enter");
		
	bool bContinue = true;
	DWORD dwWaitStatus = 0;

	HANDLE handles[2] = { _hStop, _hNotify };

	_lasttimer = timeval();

	// index for vector OBJECTCOLLECTION, the index check has been done in begin()
	int objIndex = 0;
	DataObject* pObj = NULL;
	_pObjIndexTable = NULL;

	BYTE byTableId   = 0x10;      // 0x10 ~ 0xFF
	WORD wTableIdExt = 0x0100;    // 0x0100 ~ 0xFF00

	ZQ::Content::BufferData* buffData = NULL; 
	while(bContinue)
	{
		dwWaitStatus = WaitForMultipleObjects(2, handles, false, INFINITE);
		switch(dwWaitStatus)
		{
		// received stop event, the thread will stop
		case WAIT_OBJECT_0:
			_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "DODDataSource: get a thread exit event");
			bContinue = false;
			break;

		// received the Notify event
		case WAIT_OBJECT_0 + 1:
		{
			DWORD dwBytesRead = 0;
			//
			// if this thread is abort in other thread, the status is set to be STOPPED
			//
			if(STOPPED == _processStatus || ABORTED == _processStatus)
			{
				// reset variables
				objIndex = 0;
				pObj = NULL;
				byTableId   = 0x10;      // 0x10 ~ 0xFF
				wTableIdExt = 0x0100;    // 0x0100 ~ 0xFF00

				// release source file handle 
				releaseObjects();
				
				buffData = NULL;

				if(ABORTED == _processStatus)
				{
					_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), 
									 "DODDataSource: It was aborted or stopped by Graph, triggered by thread 0x%08X, release file handle", 
									 _tidAbortBy);
				}
				// notify graph this filter processing completed
				_graph.notifyCompletion(*this);

				continue;
			}

			int colSize = _objCollection.size();
			//
			// allocated buff
			//
			if(ACTIVE == _processStatus)
			{
				buffData = _pool.alloc(BUFF_WAIT_TIMEOUT);
				if(NULL == buffData)
				{				
					// trigger the loop to fetching data
					SetEvent(_hNotify);
					continue;
				}
				_graph.traceLog(id(), "DODDataSource: alloc buffData from pool. [BuffData Address: 0x%08X]", buffData);
			}
			
			//
			// get the object pointer
			//
			if(objIndex < colSize)								  // process object
			{
				pObj = _objCollection[objIndex];
				objIndex ++;
			}
			else if(objIndex == colSize && _sendIndexTable)  // process Index Table
			{
				// when _sendIndexTable == true, there is already IndexTable object created in _objCollection, 
				// but that only use to occupy one object space. 
				// Here the new object index table is to replace the first one with right data,
				// the replace is done by buffData->setProperty(CNTPRY_IO_SEEK_ORIGION, SEEK_ORIGIN_BEG)

				// 
				// create the IndexTable Object and Load it
				//
				byTableId = 0x80;
				wTableIdExt = 0x0080;

				// re-generate and load IndexTable, but does not input the list
				_pObjIndexTable = new DataObject(_objTag, _basePID, ENCRYPT_MODE_NONE, _streamCount, _versionNum, _objCollection);
				pObj = _pObjIndexTable;

				// import, coz objIndex as the condition to state that end of stream.
				objIndex++;

				//
				// set buffer property as the IndexTable, intend to re-write original index table data
				//
				buffData->setProperty(CNTPRY_IO_SEEK_ORIGION, SEEK_ORIGIN_BEG);
			}
			else												  // end of stream
			{
				// to last loop, allocate a buffData, but not use it
				if(buffData != NULL)
				{
					_pool.free(buffData);
					_graph.traceLog(id(), "DODDataSource: free buffData from pool. [BuffData Address: 0x%08X]", buffData);
				}
				buffData = NULL;

				// notify all the down renders that no coming data anymore
				notifyEndOfStream();
				
				// reset variables
				objIndex = 0;
				pObj = NULL;
				byTableId   = 0x10;      // 0x10 ~ 0xFF
				wTableIdExt = 0x0100;    // 0x0100 ~ 0xFF00

				// release source file handle 
				releaseObjects();

				_processStatus = STOPPED;
				
				// notify graph this filter processing completed
				_graph.notifyCompletion(*this);
				continue;				
			}
			
			//
			// special case for message TableId and TableIdExt
			//
			if(pObj->getObjectType() == DataObject::OBJ_TYPE_MSG)
			{
				std::string destination = pObj->getObjName();

				int iLength = destination.size();
				DWORD wFilenameASSIC[6];

				WORD wTmp = 0;
				///////////////////////////////////////////////////
				wFilenameASSIC[5] = wTmp = convertHexChar(destination[iLength-6] );

				byTableId = (BYTE)(wTmp*16);
				wFilenameASSIC[4] = wTmp = convertHexChar(destination[iLength-5] );

				byTableId += (BYTE)wTmp;
				byTableId = byTableId | 0x80;

				wFilenameASSIC[3] = wTmp = convertHexChar(destination[iLength-4] );

				wTableIdExt = wTmp*16*16*16;
				wFilenameASSIC[2] = wTmp = convertHexChar(destination[iLength-3] );

				wTableIdExt += wTmp*16*16;
				wFilenameASSIC[1] = wTmp = convertHexChar(destination[iLength-2] );

				wTableIdExt += wTmp*16;
				wFilenameASSIC[0] = wTmp = convertHexChar(destination[iLength-1] );

				wTableIdExt += wTmp;				
			}
			else if(DATA_TYPE_BOOT_EPG == _dataType && pObj->getObjectType() != DataObject::OBJ_TYPE_INDEXTABLE)
			{
				// To BOOT EPG, the tableid for objects are in range of [0x800001, 0x80000N]
				byTableId = 0x80;
				if(_sendIndexTable)
					wTableIdExt = (WORD)(objIndex - 1);   // there is index table
				else
					wTableIdExt = (WORD)objIndex;

				std::string fileName = pObj->getObjName();
				_graph.traceLog(id(), "DODDataSource: object %s table_id = 0x80, table_id_extension = 0x%08X", 
								fileName.c_str(), wTableIdExt);
			}
			else if(DATA_TYPE_FULL_EPG == _dataType && pObj->getObjectType() != DataObject::OBJ_TYPE_INDEXTABLE)
			{
				// FileName format YYYYMMDDhhmm, TableId is the elapsed hours since based time 2000-01-01 00:00:00
				std::string fileName = pObj->getObjName();
				if(fileName.length() < MIN_FULL_EPG_FILENAME_LEN)
				{
					_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "DODDataSource: object %s is not legal, name length less than %d, ignore it", 
						fileName.c_str(), MIN_FULL_EPG_FILENAME_LEN);

					continue;
				}
				const char* filepos = fileName.c_str();

				// make the file time
				char stryear[5], strMM[3], strDD[3], strhh[3];
				strncpy(stryear, filepos, 4);
				strncpy(strMM, filepos+4, 2);
				strncpy(strDD, filepos+6, 2);
				strncpy(strhh, filepos+8, 2);

				struct tm filetm;
				memset(&filetm, 0x00, sizeof(filetm));

				filetm.tm_year = atoi(stryear) - 1900;
				filetm.tm_mon = atoi(strMM) - 1;
				filetm.tm_mday = atoi(strDD);
				filetm.tm_hour = atoi(strhh);

				time_t filet = mktime(&filetm);
				
				// make the base time
				if(0 == _basetime)
				{
					struct tm basetm;
					memset(&basetm, 0x00, sizeof(basetm));

					basetm.tm_year = 2000 - 1900;
					basetm.tm_mon =  1 - 1;
					basetm.tm_mday = 1;
					basetm.tm_hour = 0;

					_basetime = mktime(&basetm);
				}
				
				// get elapsed hours
				time_t elapsedhours = (filet - _basetime) / 3600;
				if(-1 == elapsedhours)
				{
					_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "DODDataSource: object %s is not a legal format YYYYMMDDhhmm, ignore it", 
						fileName.c_str(), MIN_FULL_EPG_FILENAME_LEN);
					continue;
				}

				byTableId = (BYTE) ((elapsedhours >> 16) & 0x000000FF);
				wTableIdExt = (WORD) (elapsedhours & 0x0000FFFF);

				_graph.traceLog(id(), "DODDataSource: object %s elapsedhours = %d ((%d-%d)/3600), table_id = 0x%02X, table_id_extension = 0x%04X", 
								fileName.c_str(), elapsedhours, filet, _basetime, byTableId, wTableIdExt);
			}
			//
			// load the data and deliver to connected filter
			//
			_graph.traceLog(id(), "DODDataSource: Began loadDataSource() for object %s", pObj->getObjName().c_str());
			if(pObj->loadDataSource(byTableId, wTableIdExt, *buffData))
			{				
				_graph.traceLog(id(), "DODDataSource: Completed loadDataSource() for object %s", pObj->getObjName().c_str());

				// pass the buff data to renders
				deliverBuffer(buffData);
				
				// release the buff 
				bool bReleased = releaseBuffer(buffData);
				if(bReleased)
				{
					_graph.traceLog(id(), "DODDataSource: free buffData from pool. [BuffData Address: 0x%08X]", buffData);
				}
				// trigger next loop to fetching data
				SetEvent(_hNotify);

				//
				// recalculate TableId and TableIdExt for next round setting
				//
				if( wTableIdExt >= 0xFF00 )
				{
					wTableIdExt = 0x0100;

					if( byTableId >= 0xFF )
					{
						byTableId = 0x10;
					}
					else
					{
						byTableId ++;
						if( byTableId == 0x80 )
						{
							byTableId ++;
						}
					}
				}
				else
				{
					wTableIdExt = wTableIdExt+0x0100;
				}

				// yield the data loading speed
				if(colSize > 1 && _yieldTime > 0)  // yield only there are more than one object
				{
					Sleep(_yieldTime);
				}

			} // end of loadDataSource()
			else
			{
				//
				// loadDataSource() failed, abort Graph
				// 
				_pool.free(buffData);
				_graph.traceLog(id(), "DODDataSource: free buffData from pool. [BuffData Address: 0x%08X]", buffData);
				
				// set last error to Graph
				_graph.setLastError(0,pObj->getLastError());

				_graph.writeLog(ZQ::common::Log::L_ERROR, id(), "DODDataSource: object %s is failed to load with reason: %s", 
								pObj->getObjName().c_str(), pObj->getLastError().c_str());

				// reset variables
				objIndex = 0;
				pObj = NULL;
				byTableId   = 0x10;      // 0x10 ~ 0xFF
				wTableIdExt = 0x0100;    // 0x0100 ~ 0xFF00

				// release source file handle 
				releaseObjects();

				_processStatus = ABORTED;

				_graph.abortProvision();

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

	_graph.writeLog(ZQ::common::Log::L_DEBUG, id(), "DODDataSource::run() leave");
	
	return 1;
}

bool DODDataSource::recursiveFolder(std::string folderName)
{
	//
	// Currently not support IndexContent Mode, if to support, new logic must here to determine how to get data
	//
	std::string wildPath = folderName + "*.*";

	WIN32_FIND_DATA FindData;
	HANDLE hFindFile = FindFirstFileA(wildPath.c_str(), &FindData);
	if( hFindFile == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	do
	{
		std::string fileName = FindData.cFileName;

		if(FILE_ATTRIBUTE_DIRECTORY != FindData.dwFileAttributes)
		{
			DataObject* pObj = new DataObject(_objTag, _basePID, _encryptType, _streamCount, _versionNum, 
				                              fileName, folderName+fileName);

			// save to object collection
			_objCollection.push_back(pObj);
		}
		else if(fileName.compare(".") == 0 || fileName.compare("..") == 0)
		{
			// ignore
		}
		else
		{
			if(!recursiveFolder(folderName + fileName + "\\"))
			{
				FindClose(hFindFile);
				return false;
			}
		}
	}
	while(FindNextFileA(hFindFile, &FindData));

	FindClose(hFindFile);

	return true;
}

bool DODDataSource::initIndexContentObjects()
{
	// not implement now

	return false;
}

int DODDataSource::pathIsFolder(const std::string& path, std::string& fileName)
{
	char chPath[MAX_PATH];
	strcpy(chPath, path.c_str());
	
	// remove \ or / from the end of path, otherwise FindFirstFile() will fail
	int len = strlen(chPath);
	if(len == 0)
		return PATH_IS_INVALID;

	if(chPath[len-1] == '\\' || chPath[len-1] == '/')
	{
		chPath[len-1] = '\0';
	}

	WIN32_FIND_DATA FindData;
	HANDLE hFindFile = FindFirstFile(chPath, &FindData);
	if( hFindFile == INVALID_HANDLE_VALUE )
	{
		return PATH_IS_INVALID;
	}

	if(FILE_ATTRIBUTE_DIRECTORY != FindData.dwFileAttributes)
	{
		fileName = FindData.cFileName;

		FindClose(hFindFile);
		return PATH_IS_A_FILE;
	}
	else
	{
		FindClose(hFindFile);
		return PATH_IS_A_FOLDER;
	}
		
	FindClose(hFindFile);
	return PATH_IS_INVALID;
}


} } }


