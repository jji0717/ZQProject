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
// Desc  : Implement the NTFS file source for processing
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/ContentProcess/TrickFileGenFilter.h 1     10-11-12 15:58 Admin $
// $Log: /ZQProjs/Generic/ContentProcess/TrickFileGenFilter.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 7     08-02-20 10:44 Build
// change codes to pass Vs2005 compile under x64
// 
// 6     07-12-04 10:36 Ken.qian
// remove two useless variables
// 
// 5     07-11-23 15:11 Ken.qian
// add addSplicePoint() function
// 
// 4     07-11-16 19:15 Ken.qian
// seperate the time of update bitrate,framecode, etc property with
// playtime
// 
// 3     07-11-15 18:56 Ken.qian
// change getOutputFileInfo() parameter
// 
// 2     07-09-20 17:59 Ken.qian
// add log while processing buffer
// 
// 2     07-09-09 21:23 Ken.qian
// 1. add protection for buffer dead-lock since input and output are
// sharing same buffer pool
// 2. fix splicing failure at back by set dsp's c++ configuraiton 's Debug
// Info from "Program Database" to "Program Database for Edit and
// Continue" 
// 
// 1     07-09-07 14:00 Ken.qian
// initial codes, but splicing had issue
#ifndef __ZQ_TrickFileGenFilter_Process_H__
#define __ZQ_TrickFileGenFilter_Process_H__

#include "GraphFilter.h"

#define USER_VERSION

#include "trickfileslibrary.h"

#ifndef STATUS_EVENT_DONE
#define STATUS_EVENT_DONE                ((NTSTATUS)0x40000012L)
#endif

#define TRICKGEN_MAX_TRICKSPEED_NUMBER	        3
#define TRICKGEN_MAX_OUTPUT_FILE_COUNT          2 * TRICKGEN_MAX_TRICKSPEED_NUMBER + 1 + 1 + 1  // trick files + index file + main file + splicing file

#define DEF_SPLICE_MIN_GOP_SIZE         3
#define DEF_SPLICE_GRADUAL_TIMING       1
#define DEF_SPLICE_HOLD_PIC_COUNT       6

#define DEF_SPLICE_FRONT_ID		       (-1)
#define DEF_SPLICE_BACK_ID		       (-2)

#define MPEG_TS_PACKET_SIZE            188
#define TRICKGEN_BUFFER_SIZE           (300 * MPEG_TS_PACKET_SIZE)

namespace ZQ {
namespace Content {
namespace Process {

class TrickFileGenFilter : public Filter
{
	friend class Graph;

public:
	typedef enum {OPTFT_MAIN, OPTFT_VVX, OPTFT_FF, OPTFT_FR, OPTFT_SPLICE} OutputFileType;

	typedef enum { SPLICE_CUE_IN, SPLICE_CUE_OUT } SplicePointType;

	/// @param[in] outputToOneIORender       if ture, RTFLibRender connect to one IO render who support multiple sub file
	///                                      if false, RTFLibRender's each output file is connected to one IO render
	TrickFileGenFilter(ZQ::Content::Process::Graph& graph, bool outputToOneIORender=true, int numTrickSpeed=1, 
		DWORD trickBufferSize = TRICKGEN_BUFFER_SIZE, 
		DWORD maxMpegCodingError=10, bool enableSplicing = true, std::string myName="");
	virtual ~TrickFileGenFilter();
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
	bool getOutputFileInfo(int fileNo, char* ext, int len, DWORD& numerator, DWORD& denominator, OutputFileType& outputFileType);

	// add splice point, splicetime is ms
	bool addSplicePoint(SplicePointType splicetype, DWORD splicetime);
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
	bool                          _endOfStream;

	HANDLE                        _hThdEnter;
	std::string                   _homeDirectory;

	std::string                   _szDesFileName; 
	// total processed bytes
	__int64                       _processedBytes;
	DWORD                         _dwCounter;



	bool                          _outputToOneIORender;
// RTF section
public:

	bool initOutputSetting(int outputFileIndex);

	bool copyVvxHeaderInfo();

	bool acquireSourceMpegBufferProcess(int fileNo, PMPEG_BUFFER* ppMpegBuffer);
	void releaseSourceMpegBufferProcess(int fileNo, PMPEG_BUFFER pMpegBuffer, DWORD consumedBytes);
	
	bool acquireOutputMpegBufferProcess(int fileNo, PMPEG_BUFFER* ppMpegBuffer);
	void releaseOutputMpegBufferProcess(int fileNo, PMPEG_BUFFER pMpegBuffer, DWORD consumedBytes);
	void flushOutputMpegBufferProcess(int fileNo, PMPEG_BUFFER pMpegBuffer, DWORD consumedBytes);

	bool import(bool& bProcessNextPic, std::string& errMsg);
	void importDone(bool importStatus);

private:
	bool openSession(std::string& errMsg);
	bool closeSession();

	void initOutputFileInfo();
	void getTrickExt(int index, char* ext1, char* ext2);

	void updateFilesetProperties();
	void updateContentProperties();
	void updateContentPlayTime();

	double getFrameRateByCode(WORD framecode);

private:
	static PVOID		memoryAllocate(PVOID context, ULONG size);
	static void			memoryFree(PVOID context, PVOID buffer);
	static void			logMsgFromTrickLibrary(PVOID context, const char *format, ...);

	static PMPEG_BUFFER acquireSourceMpegBuffer(PVOID context);
	static void			releaseSourceMpegBuffer(PVOID context, PMPEG_BUFFER mpegBuffer, ULONG bytesConsumed);
	
	static PMPEG_BUFFER acquireOutputMpegBuffer(PVOID context);
	static void			releaseOutputMpegBuffer(PVOID context, PMPEG_BUFFER mpegBuffer, ULONG bytesConsumed);
	static void			flushOutputMpegBuffer(PVOID	context, PMPEG_BUFFER mpegBuffer, ULONG bytesConsumed);
	
private:
	typedef struct _OUTPUTFILEINFO
	{	
		int             fileNo;
		OutputFileType  fileType;
		long            speed;
		char            extension[16];

		__int64         offset;

		DWORD           speedDirection;
		DWORD           speedNumerator;
		DWORD           speedDenominator;
		
		Filter*         thisFilter;
		Filter*         ioRender;
	}OutputFileInfo, *POutputFileInfo;

	int                     _numTrickSpeed;
	int                     _outputFileCount;

	OutputFileInfo          _outputInfo[TRICKGEN_MAX_OUTPUT_FILE_COUNT];
	int                     _connectedIORenderCount;	

	PTRICK_CONTEXT			_trickContext;				// trick file library context
	TRICK_CHARACTERISTICS	_trickCharacteristics;      // interesting video characteristics
	DWORD                   _maxMpegCodingError;        // the max error count that trick API can stand by. 
	DWORD					_dwPictureCount;            // pic count
	DWORD                   _vvxFlushNeeded;            // flag for VVX and FR
    DWORD                   _flushCount;

	DWORD                   _dwTrickBufferSize;

	bool                    _enableSplicing;
	
	DWORD                   _contentBitrate;

    PVOID					(*_mallocRoutine)(size_t size);
	void					(*_freeRoutine)(PVOID buffer);
private:
	typedef struct _SPLICEPOINTS
	{
		SplicePointType splicetype;
		DWORD           spliceTime;
	}SPLICEPOINTS, *PSPLICEPOINTS;

	std::vector<SPLICEPOINTS> _splicePoints;

private:
	typedef std::queue<PMPEG_BUFFER>     MPEGBUFFERQUEUE;
	MPEGBUFFERQUEUE                      _usefulMpegBufferPool;

	typedef std::map<long, PMPEG_BUFFER> MPEGBUFFERMAP;
	MPEGBUFFERMAP                        _usedMpegBufferPool;

	ZQ::common::Mutex                    _mpegBufferMutex;

	PMPEG_BUFFER allocMpegBuffer();
	void freeMpegBuffer(PMPEG_BUFFER mpgBuffer);

	void freeMpegBufferPoolMemory();
};


} } }

#endif // !defined(__ZQ_TrickFileGenFilter_Process_H__)
