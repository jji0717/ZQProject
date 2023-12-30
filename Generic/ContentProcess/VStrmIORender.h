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
// Desc  : Define the Vstrm file IO Render
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/ContentProcess/VStrmIORender.h 1     10-11-12 15:58 Admin $
// $Log: /ZQProjs/Generic/ContentProcess/VStrmIORender.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 18    08-03-11 18:03 Ken.qian
// Support supportfilesize
// 
// 17    08-02-25 17:40 Ken.qian
// notify endofstream to connected filter only once if
// RTFLibFilter/TrickFileGenFilter output to one filter(support subfile)
// 
// 16    07-11-16 19:16 Ken.qian
// add logic to fire Streamable event
// 
// 15    07-11-15 15:17 Ken.qian
// 
// 14    07-11-13 17:20 Ken.qian
// 1. Support Pacing
// 2. VsIOLib Replace VstrmAPI
// 3. Support disable  BufDrv Throttle
// 
// 13    07-10-23 18:30 Ken.qian
// add vstrm get last error text
// 
// 12    07-09-11 14:30 Ken.qian
// 1) Support TrickFilesLibraryUser index file output
// 2) Support the buffer size more than specified vstrm flush size
// 
// 11    07-08-14 15:32 Ken.qian
// 
// 10    07-07-26 18:08 Ken.qian
// fix vstrm io vvx issue
// 
// 9     07-07-23 15:29 Ken.qian
// vstrmiorender support subfiles and 64K IO writting
// 
// 8     07-07-19 17:45 Ken.qian
// normal release for rtfcpnode
// 
// 7     07-07-18 16:10 Ken.qian
// 
// 6     07-07-17 17:05 Ken.qian
// 
// 5     07-07-17 14:18 Ken.qian
// 
// 4     07-06-27 15:14 Ken.qian
// Change kbps to bps
// support vstrm bandwidth control
// 
// 3     07-06-06 15:37 Ken.qian
// 
// 3     07-06-05 18:25 Ken.qian
// add the detail errorcode
// 
// 2     07-06-01 17:04 Ken.qian
// add checksum
// 
// 1     07-05-31 17:06 Ken.qian
//

#ifndef __ZQ_VstrmIORender_Process_H__
#define __ZQ_VstrmIORender_Process_H__

#include "MD5CheckSumUtil.h"
#include "GraphFilter.h"
#include <queue>

#include "vstrmuser.h"

namespace ZQ { 
namespace Content { 
/// namespace Process presents the processing to the content, such as reading/writing
namespace Process {

#define DEFAULT_SOURCE_BUFF_COUNT	30
#define DEF_VSTRM_WRITE_SIZE        (1024*64)     // do not change this, coz to VStrm, it is fixed
#define MC_RAID_DRIVER              "\\SeaFileDevices\\"
#define DEF_STEAMABLE_PLAYTIME      10             // 10 seconds
//////////////////////////////////////////////////////////////////////////
// the IO is support to output multiple files, 
// different files are distinguished by the extension
//////////////////////////////////////////////////////////////////////////
class VstrmIORender : public Filter
{
	friend class Graph;
public:
	VstrmIORender(ZQ::Content::Process::Graph& graph, HANDLE vstrm = INVALID_HANDLE_VALUE, 
		          DWORD bwmgrClientId=0, bool pacing = false, DWORD streamablePlaytimeInSecond = DEF_STEAMABLE_PLAYTIME, 
				  bool disableBufDrvThrottle = true, 
				  std::string myName="");

protected:	
	/// destructor
	virtual ~VstrmIORender(void);

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

	/// this virtual function must be render, to get know current processing progress,
	/// bytes or something else.
	virtual __int64 getProcessedStuff() { return _subFiles[_progressRptFileNo]->_processedBytes; };

	/// Notify the Filter derived object that there is no any coming data
	/// After receiving this notification, the thread process all the received data
	//  then flush the data if required, then stop process until the next starting
	virtual void endOfStream(void);

	/// set the output file count and each file extension. They are for one IO output multiple files
	void setSubFileCount(DWORD fileCount);
	/// fileNo index is from 0, this function must be invoked for each one if fileCount > 1
	bool setSubFileInfo(DWORD fileNo=0, DWORD outputBuffSize = DEF_VSTRM_WRITE_SIZE, bool enableMD5=false, std::string fileExt="", 
						   DWORD bwPercentageN=1, DWORD bwPercentageD=1, bool progressRpter = false);

	/// set cache path
	void setCacheDirectory(std::string path);

	// get the cache path
	std::string getCacheDirectory() { return _cachePath; };

	/// set the flag that index file is generated by trick-gen, 
	/// coz there is difference between Trick-Gen and RTFLib generated VVX in moving file writing pointer
	/// invoke this after setSubFileInfo
	bool vvxSubFileByTrickGen();

	/// Set this render running on MediaCluster Node or not
	/// @param[in]   bRunningOnNode    true - means running on Node, nodeName is not useful
	///                                false - NOT running on Node, RPC connect to nodeName
	/// @param[in]   nodeName          The node that will be connected if not running on node
	void setVstrmNode(bool bRunningOnNode=true, std::string nodeName="");
		
	/// initialize vstrm handle
	static HANDLE initVstrm(std::string& errmsg, bool runningOnNode=true, std::string nodeName="");

	/// uninitialize vstrm handle
	static bool uninitVstrm(HANDLE hvstrm);

	/// copy a file from NTFS to Vstrm without speed limitation
	static bool copyFileToVstrm(std::string sourceFile, std::string desFile, std::string& errmsg, bool disableBufDrvThrottle = false, HANDLE hVstrm = INVALID_HANDLE_VALUE);

protected:
	/// implement and invoke this function when buffer processing succeed
	void onProcessSucceed();

	/// implement and invoke this function when buffer processing failed
	void onProcessFailed();

public:
	/// implementation of NativeThread virtual function
	int run(void);


protected:
	
	class SubFile
	{
	public:
		SubFile(int fileNo, HANDLE vstramHandle, 
				DWORD bwmgrClientId, std::string cachePath, bool pacing, bool disableBufDrvThrottle, 
				ZQ::Content::Process::Graph& graph);
		virtual ~SubFile();

	public:
		bool init(std::string cntName, DWORD maxbps);
		bool uninit(bool delFile = false);
		void setCacheBufferSize(DWORD size = DEF_VSTRM_WRITE_SIZE);

		void updateMD5Property();
		bool outputBuffer(ZQ::Content::BufferData* pBuffData);
		bool outputLeftBuffer();

		bool delOutputFile();
		// following functions are for pacing write
		bool pacingWrite(const char* buff, DWORD& len);
		bool pacingSeek(__int64 offset);
		bool pacingSetEOF(__int64 offset);

	protected:
		bool reserveVstrmBW(std::string& errmsg);
		bool releaseVstrmBW();

	public:
		HANDLE                        _hVstrm;
		ZQ::Content::Process::Graph&  _graph;
		DWORD                         _bwmgrClientId;
		std::string	                  _cachePath;
		// key of Out
		int				_fileNo;

		// basic output file 
		std::string     _extension;
		std::string     _outputFileName;   // with extension
		HANDLE          _hOutputFile;
		HANDLE          _ntfsCacheHandle;
		OBJECT_ID		_objectId;

		// additional info
		ULONG64         _bwTicket;
		DWORD           _bwPercentageN;   // numerator for calculating bandwidth
		DWORD           _bwPercentageD;   // denominator for calculating bandwidth
		DWORD           _reservedBW;      // _maxbps * (_bwPercentageN / _bwPercentageD)

		// total processed bytes
		DWORD           _buffCount;
		__int64         _processedBytes;

		DWORD           _cacheBufferSize;
		char*           _cacheBuffer;
		DWORD           _cacheCurLength;
		
		bool                        _enableMD5Checksum;
		ZQ::common::MD5ChecksumUtil _md5ChecksumUtil;

		// following fields are only for vvx index sub file
		bool           _vvxByTrickGen;
		LARGE_INTEGER  _offset;

		// following fields are only for pacing
		bool           _pacing;
		const void*    _pacingIndexCtx;
		HANDLE         _ntfsTempHandle;
		__int64        _vvxByteRead;
		char           _tmpbuffer[DEF_VSTRM_WRITE_SIZE];
		DWORD          _lastReadSize;

		LARGE_INTEGER  _posCacheWrite;
		LARGE_INTEGER  _posCacheRead;

		// BufDrv Throttling
		bool           _disableBufDrvThrottle;
	};

	typedef std::vector<SubFile*>         SubFileVector;
	SubFileVector                         _subFiles;
	int                                   _progressRptFileNo;

	HANDLE                                _hStop;
	HANDLE                                _hNotify;
	bool                                  _bEndOfStream;
	
	std::queue<ZQ::Content::BufferData*>  _dataQueue;
	ZQ::common::Mutex                     _dataMutex;

	DWORD                                 _tidAbortBy;

	HANDLE                                _hVstrm;
	std::string                           _vstrmErrMsg;

	bool                                  _bRunningOnNode;
	bool                                  _initedLocal;
	std::string                           _nodeName;

	bool                                  _disableBufDrvThrottle;

	DWORD                                 _bwmgrClientId;   // bandwidth manager client id

	std::string                           _cachePath;

	bool                                  _pacing;          // flag to support pacing or not
	std::string                           _pacingType;      // pacing type, vvx or vv2
	DWORD                                 _indexFileNo;     // index file No
	DWORD                                 _mainFileNo;      // main file No
	DWORD                                 _streamablePlaytime; // Streamable delay in seconds
	DWORD                                 _startTickCount;  // the start tickcount of ingestion

	bool                                  _streamableEvt;   // true - streamable event already sent, false - not sent yet
private:
	void updateMD5Property();
	void updateSupportFileSize();

	bool initSubFiles(void);
	void uninitSubFiles(bool delFile=false);
	void freeSubFilesObjs(void);

	void emptyDataQueue(void);
	
	bool reserveVstrmBW(DWORD fileNo, std::string& errmsg);
	bool releaseVstrmBW(DWORD fileNo);

	bool initMyVstrm(std::string& errmsg);
	bool uninitMyVstrm();

public:
	static bool releaseAllBW(DWORD bwmgrClientId, std::string& errstr, HANDLE vstrm = INVALID_HANDLE_VALUE);

	static void getVstrmError(HANDLE hVstrm, std::string& strErr);

protected:
	static ZQ::Content::Process::Graph*  _pStaticGraph;  // this is for pacingAppLogCbk callback loging
    // pacing callback functions
	static int pacingAppWrite(const void * const pCbParam, const int len, const char buf[]);
	static int pacingAppSeek(const void * const pCbParam, const LONGLONG offset);
    static int pacingAppSetEOF(const void * const pCbParam, const LONGLONG offset);
	static void pacingAppReportOffsets(const void * const pCbParam, const LONGLONG offset1, const LONGLONG offset2);
	static void pacingAppLogCbk(const char * const pMsg);

public:
	static char *DecodePacedIndexError(const unsigned long err);

	static bool disableBufDrvThrottle(HANDLE vstrmHandle, HANDLE fileHandle, OBJECT_ID objectId, std::string& errMsg); 
};


} } }

#endif // __ZQ_VstrmIORender_Process_H__
