// ===========================================================================
// Copyright (c) 2008 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================


#ifndef ZQTS_CPE_HTTP_PROPAGATION_H
#define ZQTS_CPE_HTTP_PROPAGATION_H
#include "ZQ_common_conf.h"
#include "FileIo.h"
#include "MD5CheckSumUtil.h"
#include <vector>
#include "BaseClass.h"
#include "HTTPClientFactory.h"
#ifdef ZQ_OS_LINUX
#include <semaphore.h>
#endif

typedef struct 
{
	std::string filename;
	int64 firstOffset;
	int64 finalOffset;
	int64 totalFilesize;
	bool  bIsSparseFile;
}Fileset;
typedef std::vector<Fileset> Filesets;

using namespace ZQTianShan::ContentProvision;
namespace ZQTianShan {
	namespace ContentProvision {

class HTTPPropagation
{	
public:
	HTTPPropagation(HTTPClientFactory* _pHttpClientFactory, FileIoFactory* pFileIoFactory);
	virtual ~HTTPPropagation();

	void setLog(ZQ::common::Log* pLog);
	void setURL(const char* szURL){_srcURL=szURL;}
	void setTargetDir(const char* szDir);
	void setCacheDir(const char* szCachDir);
	void setFilename(const char* szFilename);
	void setMaxBandwidth(unsigned int nBandwidthBps);
	void setBandwidthCtrlInterval(int nIntervalMs);
	void enableMD5(bool bEnable = true);
	void setLocalNetIf(const std::string& strLocalNetIf){_strLocalNetIf = strLocalNetIf;}
	void setWaitTimeForGrowing(int waitTime){_dWaitTime = waitTime;}

	void setPIDAndPAID(std::string PID, std::string PAID){_strProvideId = PID; _strProvideAssetId = PAID;}
	void setLocateFileIP(std::string bindIP, std::string transferip){_bindIP = bindIP; _transferip = transferip;};
	void setTransferServerPort(int nport){_transferServerPort = nport;};
	void setSpeed(int nspeed){_nspeed = nspeed;};
	void setTransferDelay(int transferdelay){_transferDelay = transferdelay;};
	void setTestCsicoPara(int nspeed, int transferDelay, int bandWidth, std::string range){_nspeed = nspeed; _transferDelay = transferDelay; _testBandwidth = bandWidth * 1000; _testRange = range;};
	void setTimeout(int timeout);
	void setLocatePos(int begin, int end){_beginpos = begin; _endpos = end;};
	void setVstreamIO(bool bVstreamIO){_bVstreamIO = bVstreamIO;};
	void getMD5(std::string& strMD5);
	void getSupportFileSize(int64& supportFilesize){supportFilesize = _supportFilesize;}
	int64 getProcessSize();
	int64 getTotalSize();
	void getLastError(std::string& strErr, int& errCode);
	bool getMediaInfo(MediaInfo& mInfo);

	bool getOutputFiles(std::vector<std::string>& filelists, int& outputfilecount, int& indexType);
    void getOutputFileInfo(std::map<std::string, std::string>& fileMap );
	void getOutputFileExtCol(std::string& ext);
	void setIngressCapcaity(int IngressCapcaity){_IngressCapcaity  = IngressCapcaity;}
	static bool getTransferbuf(void* pCtx, char*pBuf, int64 len);

	void deleteOutput(bool del);
	void stop();
	void close();

	void setSleeptime(int nsleepTime){ _sleepTime = nsleepTime;};
protected:
	bool prepare();
	bool propagate();
	bool resumePropagate();
	bool propagateEx();

	virtual void OnProgress(int64 nProcessBytes);

	void setLastError(const std::string& strErr, int errCode);

	bool openTargetFile(const Fileset& targetName,int createway = 1);
	bool writeTargetFile(char* pBuf, int nLen);
	void closeTargetFile();
	bool seekTargetFilePos(int64 fileEnd);
	bool copy(const Fileset& sourceNameExt,const Fileset& targetName, bool bresume = false);
	
	bool downloadIndexFile();
	void calculateBitrate(int64& nstart);

protected:

	std::string		              _srcURL;
	std::string					  _strServer;
	std::string                   _strPort;
	std::string                   _tempDir;
	std::string                   _outputDir;

	std::string                   _strFilename;		//target file name 
	std::string                   _strTempIndexFile;

	std::string		              _strLocalNetIf;

	std::string					  _strProvideAssetId;
	std::string                   _strProvideId;
	std::string                   _bindIP;
	std::string                   _transferip;
	int                           _transferServerPort;

	Filesets                      _srcFileset;
	Filesets                      _targetFileset;
    Filesets                      _outputFileset;

	bool						  _bEnableMD5;
	ZQ::common::MD5ChecksumUtil   _md5ChecksumUtil;
	int64						  _supportFilesize;		//subfile size, except for main file

	unsigned int				_nBandwidthBps;
	int64						_totalBytes;
	int64						_processBytes;
	
	bool						_bStop;
	bool                        _bWriteFileError;
	//for bitrate control
	BitrateControlor			 _bitrateCtrl;
	MediaInfo					 _info;

	std::auto_ptr<ZQTianShan::ContentProvision::C2HttpClient>	_pHttpDownloader;
	std::auto_ptr<FileIo>       _pFileIo;


	std::string					_strLogHint;

	std::string					_strErr;
	int							_errCode;

//	ZQ::common::Log*			_log;
	HTTPClientFactory*			__pHttpClientFactory;
	FileIoFactory*				_pFileIoFactory;
	uint32                      _dWaitTime;
	int64                       _OrigfileSize;
	int64                       _indexSize;
	bool                        _bFailYet;

	int64                       _IngressCapcaity;
	bool                        _bmainFileFlag;

	int32                       _indexType;

	int                         _nspeed; 
	int                         _transferDelay;
	int                         _timeout;
	int64                       _testBandwidth;
	std::string                 _testRange;
	int                         _beginpos;
	int                         _endpos;

	bool                        _bVstreamIO;

   ///for cache gateway
	std::string                 _transferId;
	std::string                 _transferUrl;
	int                         _sleepTime;
	bool                        _bFileset;
	int64                       _indexFileSize;

#ifdef ZQ_OS_MSWIN 
	HANDLE                      _hwait;
#else
	sem_t						_hwait;
	bool						_bSignaled;
#endif
};


}}

#endif

