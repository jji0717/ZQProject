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


#ifndef ZQTS_CPE_FTP_PROPAGATION_H
#define ZQTS_CPE_FTP_PROPAGATION_H


#include "FtpClient.h"
#include "FileIo.h"
#include "MD5CheckSumUtil.h"
#include <vector>
#include "BaseClass.h"
#ifdef ZQ_OS_LINUX
#include <semaphore.h>
#endif


namespace ZQTianShan {
	namespace ContentProvision {

typedef struct 
{
	std::string filename;
	int64 firstOffset;
	int64 finalOffset;
	int64 totalFilesize;
}Fileset;
typedef std::vector<Fileset> Filesets;
enum {INDEX_VVX = 0, INDEX_VV2, INDEX_VVC};
class FTPPropagation
{	
public:
	FTPPropagation(FTPClientFactory* pFTPClientFactory, FileIoFactory* pFileIoFactory);
	virtual ~FTPPropagation();

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

	void getMD5(std::string& strMD5);
	void getSupportFileSize(int64& supportFilesize){supportFilesize = _supportFilesize;}
	int64 getProcessSize();
	int64 getTotalSize();
	void getLastError(std::string& strErr, int& errCode);
	bool getMediaInfo(MediaInfo& mInfo);

	bool prepare();
	bool propagate();
	bool resumePropagate();
	void stop();
	void close();
	void setDecodeSourceURL(bool bDecodeSourceURL);
	bool getOutputFiles(std::vector<std::string>& filelists, int& outputfilecount, int& indexType);
	bool getNormalize(){return _bNormalize;};
protected:
	virtual void OnProgress(int64 nProcessBytes);

	void setLastError(const std::string& strErr, int errCode);

	bool openTargetFile(const Fileset& targetName,int createway = 1);
	bool openRemoteFile(const Fileset& sourceName);
	bool writeTargetFile(char* pBuf, int nLen);
	void closeTargetFile();
	bool seekTargetFilePos(int64 fileEnd);
	bool copy(const Fileset& sourceName,const Fileset& targetName,bool bmainFileFlag,bool bresume=false);

	std::string getVvxIndexFileName(const std::string& strMainFile);
	std::string getVv2IndexFileName(const std::string& strMainFile);
	std::string getVvcIndexFileName( const std::string& strMainFile);
	int isRemoteVvxIndex(const std::string& strRemoteMainFile);
	bool downloadIndexFile(const std::string& strIndexFile, const std::string& strLocalFile);
	void calculateBitrate(int64& nstart);

protected:

	std::string		              _srcURL;
	std::string                   _tempDir;
	std::string                   _outputDir;

	std::string                   _strFilename;		//target file name 
	std::string                   _strTempIndexFile;

	std::string		              _strLocalNetIf;

	Filesets                      _srcFileset;
	Filesets                      _targetFileset;

	bool						_bEnableMD5;
	ZQ::common::MD5ChecksumUtil _md5ChecksumUtil;
	int64						 _supportFilesize;		//subfile size, except for main file

	unsigned int				_nBandwidthBps;
	int64						_totalBytes;
	int64						_processBytes;
	
	bool						_bStop;
	//for bitrate control
	BitrateControlor			 _bitrateCtrl;
	MediaInfo					 _info;

	std::auto_ptr<FTPClient>	_pFtpDownloader;
	std::auto_ptr<FileIo>       _pFileIo;


	std::string					_strLogHint;

	std::string					_strErr;
	int							_errCode;
//	ZQ::common::Log*			_log;
	FTPClientFactory*			_pFTPClientFactory;
	FileIoFactory*				_pFileIoFactory;
	uint32                      _dWaitTime;
	int64                       _OrigfileSize;
	int64                       _indexSize;
	bool                        _bFailYet;
	bool                        _bDecodeSourceURL;
	int                         _indexType;
	bool                        _bNormalize;
#ifdef ZQ_OS_MSWIN
	HANDLE                      _hwait;
#else
	sem_t						_hwait;
	bool						_bSignaled;
#endif
};


}}

#endif

