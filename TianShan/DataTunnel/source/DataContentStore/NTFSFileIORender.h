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
// Desc  : Define the NTFS IO Render
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/TianShan/DataTunnel/source/DataContentStore/NTFSFileIORender.h 1     10-11-12 16:05 Admin $
// $Log: /ZQProjs/TianShan/DataTunnel/source/DataContentStore/NTFSFileIORender.h $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 1     09-03-09 10:33 Li.huang
// 
// 1     08-12-08 11:11 Li.huang
// 
// 1     08-10-30 16:45 Ken.qian
// Move files from /ZQProjs/Generic/ContentProcess to local folder, since
// files at ContentProcess were never used by others components. And
// remove the pacing codes from NTFSIORender to indepent on Vstrm DLL
// 
// 10    08-10-29 10:12 Ken.qian
// add macro to disable pacing
// 
// 9     08-04-09 15:29 Ken.qian
// Support Pacing
// 
// 8     08-03-11 18:03 Ken.qian
// Support supportfilesize
// 
// 7     08-02-25 17:39 Ken.qian
// Support SubFile
// 
// 6     07-09-11 14:29 Ken.qian
// Support TrickFilesLibraryUser index file output
// 
// 5     07-07-18 16:10 Ken.qian
// 
// 4     07-06-06 15:37 Ken.qian
// 
// 4     07-06-05 18:25 Ken.qian
// add the detail errorcode
// 
// 3     07-06-01 17:04 Ken.qian
// add checksum
// 
// 2     07-04-20 16:33 Ken.qian
// 
// 1     07-04-16 11:04 Ken.qian
//
//

#ifndef __ZQ_NTFSFileIORender_Process_H__
#define __ZQ_NTFSFileIORender_Process_H__

#include "MD5CheckSumUtil.h"
#include "GraphFilter.h"
#include <queue>

namespace ZQ { 
namespace Content{ 
/// namespace Process presents the processing to the content, such as reading/writing
namespace Process {

#define DEFAULT_SOURCE_BUFF_COUNT	30
#define MAX_READ_SIZE                   (1024*64)     // do not change this, coz to VStrm, it is fixed

class NTFSFileIORender : public Filter
{
	friend class Graph;
public:
	typedef enum { FPMT_NONE=0, FPMT_RTF_LIB, FPMT_TRICKUSER_LIB } FILE_POINT_MOVING_TYPE;

	NTFSFileIORender(ZQ::Content::Process::Graph& graph, bool enableMD5Checksum = false, std::string myName="", 
					 bool delProvedFile = false, bool delErrorFile = true);
				
protected:	
	/// destructor
	virtual ~NTFSFileIORender(void);

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

	/// this virtual function must be render, to get know current processing progress,
	/// bytes or something else.
	virtual __int64 getProcessedStuff() { return _subFiles[_progressRptFileNo]->_processedBytes; };
	
	/// Notify the base derived object that there is no any coming data
	/// After receiving this notification, the thread process all the received data
	//  then flush the data if required, then stop process until the next starting
	virtual void endOfStream(void);

	/// Set output home directory
	/// @param[in]   path      the home directory of output file
	void setHomeDirectory(std::string path);
	
	/// Set File Name Extension, 
	/// @param[in]   extension      the extension of output file
	void setFileExtension(std::string extension);

	/// Set File Pointer moving type
	void setFilePointerMovingType(FILE_POINT_MOVING_TYPE mtype) { _fpmType = mtype; };

	void enableMD5Checksum(bool enable) { _enableMD5Checksum = enable; };
	
	// functions to support multiple subfiles
	void setSubFileCount(int fileCount);
	bool setSubFileInfo(int fileNo, std::string extension, bool enableMD5=false, bool progressRpt = false, 
						FILE_POINT_MOVING_TYPE mtype=FPMT_NONE);

	std::string getHomeDirectory();
private:
	void freeSubFilesObjs();
	bool initSubFiles();
	void uninitSubFiles(bool delOutput=false);
	
public:
	/// implementation of NativeThread virtual function
	int run(void);

protected:
	HANDLE                                _hStop;
	HANDLE                                _hNotify;
	HANDLE                                _hThdEnter;

	bool                                  _bEndOfStream;
	
	std::queue<ZQ::Content::BufferData*>  _dataQueue;
	ZQ::common::Mutex                     _dataMutex;

	std::string                           _szHomeDirectory;
	std::string                           _szFileExtension;
	
	DWORD                                 _tidAbortBy;

	FILE_POINT_MOVING_TYPE                _fpmType;
	bool                                  _enableMD5Checksum;

	bool                                  _delProvedFile;
	bool                                  _delErrorFile;
	
private:	
	void emptyDataQueue(void);

protected:
	class SubFile
	{
		public:
			SubFile(int fileNo, NTFSFileIORender& ntfsIoRender, Graph& graph);
			virtual ~ SubFile();

		public:
			bool init(const std::string& contentName);
			void uninit(bool delFile=true);

			bool outputBuffer(ZQ::Content::BufferData* pBuffData);
			void updateMD5Property();

		public:
			NTFSFileIORender&    _ntfsIoRender;
			Graph&               _graph;
			
			int                  _fileNo;
			std::string          _extension;
			bool                 _enableMD5;

			HANDLE               _outputFileHandle;
			std::string          _outputFileFullName;
			std::string          _outputFileName;

			FILE_POINT_MOVING_TYPE _filePMT;
			LARGE_INTEGER        _offset;
			unsigned __int64     _processedBytes;


			ZQ::common::MD5ChecksumUtil _md5ChecksumUtil;

			bool                 _isIndexFile;
	};

public:	
	typedef std::vector<SubFile*>         SubFileVector;
	SubFileVector                         _subFiles;
	int                                   _progressRptFileNo;
	DWORD                                 _indexFileNo;     // index file No
	DWORD                                 _mainFileNo;      // main file No
};



} } }

#endif // __ZQ_NTFSFileIORender_Process_H__
