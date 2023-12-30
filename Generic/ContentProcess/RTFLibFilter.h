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
// Desc  : Implement the RFTLib Filter
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/ContentProcess/RTFLibFilter.h 1     10-11-12 15:58 Admin $
// $Log: /ZQProjs/Generic/ContentProcess/RTFLibFilter.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 21    08-04-28 8:36 Ken.qian
// Make FAILED_SESSION_THRESHOLD default value 0 - no error limitation
// 
// 20    07-11-16 19:15 Ken.qian
// seperate the time of update bitrate,framecode, etc property with
// playtime
// 
// 19    07-11-15 18:55 Ken.qian
// move OutputFileType definition to class
// 
// 18    07-11-07 12:01 Ken.qian
// handle the main file output in receive() instead of create addtional IO
// render to receive main file buffers outside
// 
// 17    07-10-23 18:29 Ken.qian
// make rtf input/output buffer count configurable, default 64. Instead of
// original 16K
// 
// 16    07-10-15 18:12 Ken.qian
// make buffer processing log frequency MICRO based.
// 
// 15    07-09-20 17:59 Ken.qian
// add log while processing buffer
// 
// 14    07-09-11 14:28 Ken.qian
// add protection for buffer-deadlock since input and output are sharing
// same buffer pool. 
// 
// 13    07-08-30 14:08 Ken.qian
// fix the logic of update property after opensession
// 
// 12    07-08-23 10:58 Ken.qian
// 1. move opensession to run() instead of begin()
// 2. does not invoke rtfCloseSession when abort happen
// 
// 11    07-08-17 14:32 Ken.qian
// 
// 10    07-08-14 14:31 Ken.qian
// 
// 9     07-08-09 17:10 Ken.qian
// 
// 8     07-07-26 18:08 Ken.qian
// fix vstrm io vvx issue
// 
// 7     07-07-23 15:29 Ken.qian
// vstrmiorender support subfiles and 64K IO writting
// 
// 6     07-07-19 17:45 Ken.qian
// normal release for rtfcpnode
// 
// 5     07-07-19 11:19 Ken.qian
// 
// 4     07-07-17 22:06 Ken.qian
// 
// 3     07-07-17 17:05 Ken.qian
// 
// 2     07-07-17 14:18 Ken.qian
// 
// 1     07-07-14 20:10 Ken.qian
// Initial Coding

#ifndef __ZQ_RTFLibFilter_Process_H__
#define __ZQ_RTFLibFilter_Process_H__


#include "GraphFilter.h"

extern "C" {
	#include "RTFLib.h"
}

#define DEFAULT_RENDER_BUFF_COUNT	30
#define MAX_READ_SIZE               (1024*64)     // do not change this, coz to VStrm, it is fixed
#define DEFAULT_FTP_PORT            21

#define INPUT_FILE_BUFFER_BYTES_16		(16 * 1024)
#define INPUT_FILE_BUFFER_BYTES_64		(64 * 1024)
#define INPUT_FILE_BUFFER_BYTES_128		(128 * 1024)

#define DEF_OUTPUT_FILE_BUFFER_BYTES	(64 * 1024)

#define MAX_GROUPS_PER_SEQUENCE			1
#define MAX_PICTURES_PER_GROUP			64

#define INPUT_BUFFERS_16K_PER_SESSION	496
#define INPUT_BUFFERS_64K_PER_SESSION	128
#define INPUT_BUFFERS_128K_PER_SESSION	64

#define FAILED_SESSION_THRESHOLD		0  // 0 - no limitation

namespace ZQ {
namespace Content {
namespace Process {

#define RTF_MAX_TRICKSPEED_NUMBER	        3
#define RTF_MAX_OUTPUT_FILE_COUNT           2 * RTF_MAX_TRICKSPEED_NUMBER + 1 + 1 + 1 // trick files + index file + main file + splice file
	

class RTFLibFilter : public Filter
{
	friend class Graph;

public:
	typedef enum {OPTFT_MAIN, OPTFT_VVX, OPTFT_VV2, OPTFT_FF, OPTFT_FR, OPTFT_FFR} OutputFileType;

	/// @param[in] outputToOneIORender       if ture, RTFLibRender connect to one IO render who support multiple sub file
	///                                      if false, RTFLibRender's each output file is connected to one IO render
	RTFLibFilter(ZQ::Content::Process::Graph& graph, bool outputToOneIORender=true, int numTrickSpeed=1, bool unifiedTrickFile = false, std::string myName="");
	virtual ~RTFLibFilter();
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

	virtual void endOfStream(void);

	// re-implement connect for maintain the connected io render
	virtual bool connectTo(Filter* obj, bool copyBuff=true);

	// get output file count and extension
	int getOutputFileCount() { return _outputFileCount; };
	// get output file info
	bool getOutputFileInfo(int fileNo, char* ext, int len, DWORD& numerator, DWORD& denominator, OutputFileType& fileType);

	// set the trick gen parameter at run time before invoke request start.
	void setTrickGenParam(RTF_INDEX_TYPE indexType=RTF_INDEX_TYPE_VVX, RTF_VIDEO_CODEC_TYPE codecType=RTF_VIDEO_CODEC_TYPE_MPEG2);
public:
	/// implementation of NativeThread virtual function
	int run(void);

private:
	void emptyDataQueue(void);

	ZQ::common::Mutex                     _dataMutex;
	std::queue<ZQ::Content::BufferData*>  _dataQueue;

private:
	HANDLE                        _hStop;
	HANDLE                        _hNotify;
	HANDLE                        _hDesFile;
	
	DWORD                         _maxbps;
	DWORD                         _lasttimer;

	DWORD                         _tidAbortBy;
	
	__int64                       _fileSize;

	DWORD                         _readSize;
	bool                          _bEndOfStream;

	HANDLE                        _hThdEnter;
	std::string                   _homeDirectory;

	std::string                   _szDesFileName; 
	// total processed bytes
	__int64                       _processedBytes;


	bool                          _outputToOneIORender;
// RTF section
public:
	// initialize/uninitialize RTFLib
	static bool initRTFLib(DWORD maxSession, ZQ::common::Log* pLog, 
						   DWORD inputBufferSize = INPUT_FILE_BUFFER_BYTES_64, DWORD outputBufferSize = DEF_OUTPUT_FILE_BUFFER_BYTES, 
						   bool trickKeyFrame = true, bool rtfWarningTolerance = true);
	static void uninitRTFLib();

	int releaseInputBufferProcess(ZQ::Content::BufferData* pBuffData);
	int trickFileCloseOutputProcess(int fileNo);
	int trickFilePutOutputBufferProcess(int fileNo, ZQ::Content::BufferData* pBuffData, unsigned long occupancy, RTF_BUFSEEK bufSeek, INT64 bufSeekOffset);
	int trickFileGetOutputBufferProcess(int fileNo, ZQ::Content::BufferData** pBuffData);

private:
	static void* appAlloc(RTF_APP_HEAP_HANDLE hAppHeap, int bytes);
	static void  appFree(RTF_APP_HEAP_HANDLE hAppHeap, void *ptr);
	static void  appLog(void *hAppSession, const char *pShortString, char *pLongString);
	static void  rtfLibErrorLog(char *pMessage);
	static void  sessionErrorNotifier(RTF_APP_SESSION_HANDLE hAppSession, char *pMessage);

	static int releaseInputBuffer(void *hAppSession, void *hAppFile, void *hAppBuffer, unsigned char *pBuffer);
	static int trickFileCloseOutput( void *hAppSession, void *hAppFile );
	static int trickFilePutOutputBuffer(void *hAppSession, void *hAppFile, void *hAppBuffer,unsigned char *pBuffer, unsigned long occupancy, RTF_BUFSEEK bufSeek, INT64 bufSeekOffset);
	static int trickFileGetOutputBuffer( void *hAppSession, void *hAppFile, void **phAppBuffer,unsigned char **ppBuffer, unsigned long *pCapacity);
	
	bool initOutputSetting(int outputFileIndex, RTF_APP_OUTPUT_SETTINGS* setting);

	bool openSession(std::string& errMsg);
	bool closeSession();
	bool processBuffer(ZQ::Content::BufferData* dataBuf, std::string& errmsg);

	void initOutputFileInfo();
	void getTrickExt( int speedNo, char* ext1, char* ext2);

	void updateFilesetProperties();
	void updateContentProperties();
	void updateContentPlayTime();

	double getFrameRateByCode(WORD framecode);
private:
	typedef struct _OUTPUTFILEINFO
	{	
		int             fileNo;
		OutputFileType  fileType;
		int             speedNo;
		char            extension[16];

		DWORD           speedDirection;
		DWORD           speedNumerator;
		DWORD           speedDenominator;
		
		Filter*         ioRender;
	}OutputFileInfo, *POutputFileInfo;

	static char                  _rtfVersion[16];
	static bool                  _rtfInited;
	static ZQ::common::Log*      _rtfLogger;
	static bool                  _trickKeyFrame;
	static RTF_WARNING_TOLERANCE _rtfTolerance;
	static DWORD                 _rtfInputBuffSize;
	static DWORD                 _rtfOutputBuffSize;
	

	RTF_VIDEO_CODEC_TYPE    _vcdType;
	RTF_INDEX_TYPE          _indexType;
	RTF_INDEX_MODE          _indexMode;
	RTF_INDEX_OPTION        _indexOption;

	bool                    _unifiedTrickFile;
	int                     _numTrickSpeed;
	int                     _outputFileCount;

	OutputFileInfo          _outputInfo[RTF_MAX_OUTPUT_FILE_COUNT];
	int                     _connectedIORenderCount;	

	RTF_WARNING_COUNTS		_warningThresholds;	
	RTF_SES_HANDLE			_sessionHandle;

	int                     _eosSubFileCount;  // end of stream sub file count
	bool                    _sessClosed;
	
	bool                    _rtfBeginReleaseBuffer;
	bool                    _rtfReleaseBuffCheck;

	DWORD                   _contentBitrate;
};


} } }

#endif // !defined(__ZQ_RTFLibFilter_Process_H__)
