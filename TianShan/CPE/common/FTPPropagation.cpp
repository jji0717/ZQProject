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


#include "FTPPropagation.h"
#include "Log.h"
#include "urlstr.h"
#include "ErrorCode.h"
#include "CTFLib.h"
#include "ParseIndexFile.h"
#ifdef ZQ_OS_MSWIN
#include <io.h>
#else
extern "C"
{
#include <errno.h>
#include <sys/time.h>
}
#endif

#include "SystemUtils.h"
#define MOLOG				(glog)
#define FTPPropn			"FTPPropn"


using namespace ZQ::common;



namespace ZQTianShan {
	namespace ContentProvision {

/*bool getIdxSubFileInfo(const char* szIdxFile, int indexType, std::vector<SubFileInfo>& subFiles, MediaInfo& info)
{
	if (indexType == INDEX_VVX)
	{
		VvxParser vvx;
		if(!vvx.parse(szIdxFile, true))
		{
			//
			return false;
		}

		info.videoResolutionH = vvx.getVideoHorizontalSize(); 
		info.videoResolutionV = vvx.getVideoVerticalSize();
		info.bitrate = vvx.GetBitRate();
		info.framerate = atof(vvx.getFrameRateString(vvx.getFrameRateCode()));
		info.playTime = vvx.GetPlayTime();

//		subFiles.resize(vvx.getSubFileCount());
		for(int i=0;i<vvx.getSubFileCount();i++)
		{
			SubFileInfo subinfo;
			subinfo.firstOffset = 0;
			subinfo.finalOffset = 0;
			subinfo.totalFilesize = 0;

			vvx.getSubFileExtension(i,subinfo.ext, sizeof(subinfo.ext));
			vvx.getSubFileSpeed((uint32)i,subinfo.numerator,*(uint32*)&subinfo.denominator);
			if (subinfo.numerator<0)
			{
				subinfo.numerator = 0 - subinfo.numerator;
				subinfo.direction = -1;
			}
			else
			{
				subinfo.direction = 1;
			}
			subFiles.push_back(subinfo);
		}		
	}
	else if(indexType == INDEX_VV2)
	{
		VV2Parser vv2;
		if(!vv2.parse(szIdxFile, true))
		{
			//
			return false;
		}

		info.bitrate = vv2.getBitrate();
		info.playTime = vv2.getPlayTime();

//		subFiles.resize(vv2.getSubFileCount());
		for(unsigned int i=0;i<vv2.getSubFileCount();i++)
		{
			SubFileInfo subinfo;
			subinfo.firstOffset = 0;
			subinfo.finalOffset = 0;
			subinfo.totalFilesize = 0;
			vv2.getSubFileExtension(i,subinfo.ext, sizeof(subinfo.ext));
			vv2.getSubFileSpeed(i, subinfo.numerator, subinfo.denominator, subinfo.direction);			
			subFiles.push_back(subinfo);
		}		
	}
	else   if(indexType == INDEX_VVC)// parser VVC index file
	{
		ZQ::IdxParser::IdxParserEnv  env;
		env.AttchLogger(&glog);
		ZQ::IdxParser::IndexFileParser idxParser(env);
		ZQ::IdxParser::IndexData	idxData;
		if(!idxParser.ParserIndexFileFromCommonFS("",idxData, true, szIdxFile)) {
			return false;
		}

		info.videoResolutionH = idxData.getVideoHorizontalSize(); 
		info.videoResolutionV = idxData.getVideoVerticalSize();
		info.bitrate = idxData.getMuxBitrate();
		info.framerate = atof(idxData.getFrameRateString());
		info.playTime = idxData.getPlayTime();

//		subFiles.resize(idxData.getSubFileCount());
		for(int i=0;i<idxData.getSubFileCount();i++)
		{
			SubFileInfo subinfo;
			ZQ::IdxParser::IndexData::SubFileInformation subfileinfo;
			idxData.getSubFileInfo(i, subfileinfo);
			subinfo.firstOffset = subfileinfo.startingByte;
			subinfo.finalOffset = subfileinfo.endingByte;
			subinfo.totalFilesize = subfileinfo.fileSize;
			const std::string& strExtension =  idxData.getSubFileName( i ) ;
			memset(subinfo.ext, 0 , sizeof(subinfo.ext));
			strncpy(subinfo.ext, strExtension.c_str(), sizeof(subinfo.ext));

			SPEED_IND speed = idxData.getSubFileSpeed((uint32)i);
			subinfo.numerator = speed.numerator;
			subinfo.denominator = speed.denominator;

			if (subinfo.numerator<0)
			{
				subinfo.numerator = 0 - subinfo.numerator;
				subinfo.direction = -1;
			}
			else
			{
				subinfo.direction = 1;
			}

			subFiles.push_back(subinfo);
		}		
	}
	else
		return false;

	return true;
}*/

FTPPropagation::FTPPropagation(FTPClientFactory* pFTPClientFactory, FileIoFactory* pFileIoFactory)
	:_pFTPClientFactory(pFTPClientFactory), _pFileIoFactory(pFileIoFactory)
{
#ifdef ZQ_OS_MSWIN
	_tempDir = "C:\\"; //temp use;
#else
	_tempDir = "/tmp/";
#endif
	_totalBytes = 0;

	_bEnableMD5 = false;
	_supportFilesize = 0;

	_processBytes = 0;

	_bStop = false;
//	_log = &NullLogger;
	_dWaitTime = 40000;
	_OrigfileSize = 0;
	_bFailYet = false;

	_bNormalize = false;
#ifdef ZQ_OS_MSWIN
	_hwait = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
	sem_init(&_hwait,0,0);	
	_bSignaled = false;
#endif
}

FTPPropagation::~FTPPropagation()
{
#ifdef ZQ_OS_MSWIN
	 CloseHandle(_hwait);
#else
	try
	{
		sem_destroy(&_hwait);
	}catch(...){}
#endif
	_pFileIoFactory->deleteFile(_strTempIndexFile.c_str());
}

void FTPPropagation::setFilename(const char* szFilename)
{
	//set the target file name
	_strFilename = szFilename;
	_strLogHint = _strFilename;
}

int64 FTPPropagation::getProcessSize()
{
	return _processBytes;
}

void FTPPropagation::setMaxBandwidth(unsigned int nBandwidthBps)
{
	_nBandwidthBps = nBandwidthBps;
	
	MOLOG(Log::L_DEBUG, CLOGFMT(FTPPropn, "[%s] set MaxBandwidth=%d bps"), _strLogHint.c_str(), nBandwidthBps);
}

void FTPPropagation::setTargetDir(const char* szDir)
{
	if(szDir == NULL || strlen(szDir) == 0)
		return;

	if (szDir[strlen(szDir)-1] != FNSEPC)
		_outputDir = std::string(szDir) + std::string(FNSEPS);
	else
		_outputDir = szDir;
}

void FTPPropagation::setCacheDir(const char* szCachDir)
{
	if (szCachDir[strlen(szCachDir)-1] != FNSEPC)
		_tempDir = szCachDir + std::string(FNSEPS);
	else
		_tempDir = szCachDir;
}

bool FTPPropagation::openTargetFile(const Fileset& targetName,int createway)
{
	_pFileIo.reset(_pFileIoFactory->create());

	std::string path = _outputDir + targetName.filename;	
	if (!_pFileIo->openfile(path.c_str(),FileIo::ACCESS_WRITE,
		(FileIo::ShareMode)(FileIo::SHARE_READ | FileIo::SHARE_WRITE),
		(FileIo::CreationWay)createway,
		(FileIo::FileAttrib)0))
	{
		std::string strErr;
		int nErrorCode;
		_pFileIo->getLastError(strErr, nErrorCode);

		setLastError(strErr, nErrorCode);			

		return false;
	}

	//set sparse file
	if(targetName.firstOffset >0 && targetName.finalOffset >  targetName.firstOffset)
	{
		int64 firstOffset = targetName.firstOffset;
		int64 finalOffset = targetName.finalOffset;

		if (!_pFileIo.get())
			return false;
#ifdef ZQ_OS_MSWIN
		bool sp = _pFileIo->enableSparse();
		if (!_pFileIo->setEndOfFile(finalOffset))
		{
			std::string strErr;
			int nErrCode;
			_pFileIo->getLastError(strErr, nErrCode);
			setLastError(strErr, nErrCode);
			return false;
		}
#else
		if (!_pFileIo->seekfile(finalOffset,ZQTianShan::ContentProvision::FileIo::POS_BEGIN))
		{
			std::string strErr;
			int nErrCode;
			_pFileIo->getLastError(strErr, nErrCode);
			setLastError(strErr, nErrCode);
			return false;
		}
#endif
		if (!_pFileIo->seekfile(firstOffset,ZQTianShan::ContentProvision::FileIo::POS_BEGIN))
		{
			std::string strErr;
			int nErrCode;
			_pFileIo->getLastError(strErr, nErrCode);
			setLastError(strErr, nErrCode);
			return false;
		}
	}

	return true;

}

void FTPPropagation::closeTargetFile()
{
	if (_pFileIo.get())
	{
		_pFileIo->closefile();
		_pFileIo.reset(0);
	}
}

bool FTPPropagation::writeTargetFile(char* pBuf, int nLen)
{
	if (!_pFileIo.get())
		return false;

	unsigned int writelen;
	if (!_pFileIo->writefile((char*)pBuf,nLen,writelen))
	{
		std::string strErr;
		int nErrCode;
		_pFileIo->getLastError(strErr, nErrCode);
		setLastError(strErr, nErrCode);
		return false;
	}

	return true;
}

void FTPPropagation::getMD5(std::string& strMD5)
{
	strMD5 = _md5ChecksumUtil.lastChecksum();
}

void FTPPropagation::enableMD5(bool bEnable)
{
	_bEnableMD5 = bEnable;
}

std::string FTPPropagation::getVvxIndexFileName( const std::string& strMainFile )
{
	return strMainFile + ".VVX";
}

std::string FTPPropagation::getVv2IndexFileName( const std::string& strMainFile )
{
	return strMainFile + ".VV2";
}
std::string FTPPropagation::getVvcIndexFileName( const std::string& strMainFile )
{
	return strMainFile + ".index";
}

int FTPPropagation::isRemoteVvxIndex(const std::string& strRemoteMainFile)
{
	if (_pFtpDownloader->getFileSize(getVvxIndexFileName(strRemoteMainFile))>0)
	{
		return INDEX_VVX;
	}
	else
		if((_pFtpDownloader->getFileSize(getVv2IndexFileName(strRemoteMainFile))>0))
		{
			return INDEX_VV2;
		}
		else 
			return INDEX_VVC;
}

bool FTPPropagation::downloadIndexFile( const std::string& strIndexFile, const std::string& strLocalFile)
{
	if (!_pFtpDownloader->downloadFile(strIndexFile, strLocalFile))
	{
		std::string errstr = _pFtpDownloader->getLastError();
		setLastError(std::string("failed to download file ") + strIndexFile + " with error:" + errstr, ERRCODE_NTFS_READFILE);
		MOLOG(Log::L_ERROR, CLOGFMT(FTPPropn, "[%s] failed to download file %s with error: %s"), _strLogHint.c_str(), 
			strIndexFile.c_str(), errstr.c_str());

		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPPropn, "[%s] index file %s downloaded to local %s"), _strLogHint.c_str(), 
		strIndexFile.c_str(), strLocalFile.c_str());

	return true;
}

bool FTPPropagation::openRemoteFile( const Fileset& sourceName )
{
	int64 offset = 0;
	if(sourceName.firstOffset >0 && sourceName.finalOffset >  sourceName.firstOffset)
		offset = sourceName.firstOffset;
		
	if (!_pFtpDownloader->openFile(sourceName.filename,(ZQTianShan::ContentProvision::FTPClient::FileMode)0 , offset))
	{
		std::string errstr = _pFtpDownloader->getLastError();
		setLastError(std::string("failed to open remote file ") + sourceName.filename + " with error:" + errstr, ERRCODE_NTFS_READFILE);
		MOLOG(Log::L_ERROR, CLOGFMT(FTPPropn, "[%s] failed to open remote file %s with error: %s"), _strLogHint.c_str(), 
			sourceName.filename.c_str(), errstr.c_str());

		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPPropn, "[%s] remote file %s opened"), _strLogHint.c_str(), sourceName.filename.c_str());

	return true;
}

void FTPPropagation::setLog( ZQ::common::Log* pLog )
{
	ZQ::common::setGlogger(pLog);
}

bool FTPPropagation::prepare()
{
	SubFileInfo subf;
	std::vector<SubFileInfo> subFiles;
	std::vector<SubFileInfo>::iterator iter;
	char szBuf[512];
	std::string errmsg;
	//std::string strSourcePath ;

	MOLOG(Log::L_DEBUG, CLOGFMT(FTPPropn, "[%s] enter prepare()"), _strLogHint.c_str());

	ZQ::common::URLStr srcUrl(_srcURL.c_str());
	std::string strServer = srcUrl.getHost();
	std::string strOrgFile = srcUrl.getPath();
	std::string strUserName = srcUrl.getUserName();
	std::string strPassword = srcUrl.getPwd();
	int nPort = srcUrl.getPort();

	if (_bDecodeSourceURL)
	{
		char strTemp[512] = "";
		memset(strTemp, 0, 512);

		URLStr::decode(strServer.c_str(), strTemp, 512);
		strServer = strTemp;

		memset(strTemp, 0, 512);
		 URLStr::decode(strOrgFile.c_str(), strTemp, 512);
		strOrgFile = strTemp;

		memset(strTemp, 0, 512);
		URLStr::decode(strUserName.c_str(), strTemp, 512);
		strUserName = strTemp;

		memset(strTemp, 0, 512);
		URLStr::decode(strPassword.c_str(), strTemp, 512);
		strPassword = strTemp;
	}

	int npos = strOrgFile.rfind('.');
	if(npos > 0)
	{
		std::string strExt = strOrgFile.substr(npos);
		if(strExt.size() > 3 && stricmp(strExt.substr(0, 3).c_str(), ".0x") == 0)
		{
			strOrgFile = strOrgFile.substr(0, npos);
		}
	}
	MOLOG(Log::L_DEBUG, CLOGFMT(FTPPropn, "[%s] SourceUrl=%s, Host=%s, Port=%d, User=%s, Password=%s, File=%s"),
		_strLogHint.c_str(), _srcURL.c_str(), strServer.c_str(),nPort, strUserName.c_str(), strPassword.c_str(), strOrgFile.c_str());

	FTPClient* pFTPClient = _pFTPClientFactory->create();
	if (!pFTPClient)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPPropn, "[%s] failed to create FTPClient instance"), _strLogHint.c_str());
		return false;
	}

	_pFtpDownloader.reset(pFTPClient);
	_pFtpDownloader->setTransmitMode(FTPClient::binary);
	_pFtpDownloader->setFTPMode(FTPClient::active);
	_pFtpDownloader->setIoTimeout(30000);
	_pFtpDownloader->setLog(&glog);
	if (!_pFtpDownloader->open(strServer, nPort, strUserName, strPassword, _strLocalNetIf))
	{
		std::string strErr = "failed to connect to url " + _srcURL + " with error: " + _pFtpDownloader->getLastError();

		MOLOG(Log::L_ERROR, CLOGFMT(FTPPropn, "[%s] %s"), _strLogHint.c_str(), strErr.c_str());
		setLastError(strErr, ERRCODE_INVALID_SRCURL);

		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPPropn, "[%s] ftp server connected"), _strLogHint.c_str());

	int indexType = isRemoteVvxIndex(strOrgFile);
	std::string content;
	if (strOrgFile.find_last_of(FNSEPC) != std::string::npos)
		content = strOrgFile.substr(strOrgFile.find_last_of(FNSEPC)+1,strOrgFile.size()-strOrgFile.find_last_of(FNSEPC)-1);
	else
		content = strOrgFile;
	
	std::string strRemoteIndexFile;
	std::string strTempIndexFile;
	std::string strIndexExt;
	if (indexType == INDEX_VVX)
	{
		strRemoteIndexFile = getVvxIndexFileName(strOrgFile);
		strTempIndexFile = getVvxIndexFileName(_tempDir + content);
		_indexType = CTF_INDEX_TYPE_VVX;
		strcpy(subf.ext, ".VVX");
		strIndexExt = "VVX";
	}
	else if(indexType == INDEX_VV2)
	{
		strRemoteIndexFile = getVv2IndexFileName(strOrgFile);
		strTempIndexFile = getVv2IndexFileName(_tempDir + content);
		_indexType = CTF_INDEX_TYPE_VV2;
		strcpy(subf.ext, ".VV2");
		strIndexExt = "VV2";
	}
	else
	{
		strRemoteIndexFile = getVvcIndexFileName(strOrgFile);
		strTempIndexFile = getVvcIndexFileName(_tempDir + content);
#ifndef ZQ_OS_MSWIN
		_indexType = CTF_INDEX_TYPE_VVC;
		strcpy(subf.ext, ".index");
		strIndexExt = "index";
#endif
	}

	_strTempIndexFile = strTempIndexFile;
	//cache index file in a temp directory
	if (!downloadIndexFile(strRemoteIndexFile, strTempIndexFile))
		return false;

	if(!ParseIndexFileInfo::getIdxSubFileInfo(strTempIndexFile.c_str(), strIndexExt, subFiles, _info))
	{
		//failed to parse the index file
		snprintf(szBuf, sizeof(szBuf)-1, "Failed to parse VVX/VV2/index for (%s) Indexfile extension[%s]", strOrgFile.c_str(), strIndexExt.c_str());
		MOLOG(Log::L_ERROR, CLOGFMT(FTPPropn, "[%s] %s"), _strLogHint.c_str(), szBuf);
		setLastError(szBuf, ERRCODE_NTFS_READFILE);	
		return false;
	}

	subf.firstOffset = 0;
	subf.finalOffset = 0;
	subf.totalFilesize = 0;
	subFiles.push_back(subf);
	for (iter = subFiles.begin();iter != subFiles.end();iter ++)
	{
		Fileset fileset;
		fileset.filename = strOrgFile + iter->ext;
		fileset.firstOffset = iter->firstOffset;
		fileset.finalOffset = iter->finalOffset;
		fileset.totalFilesize = iter->totalFilesize;
		_srcFileset.push_back(fileset);

		fileset.filename = _strFilename + iter->ext;
		_targetFileset.push_back( fileset);

		if(iter->firstOffset > 0  && iter->finalOffset > 0)
			_bNormalize = true;
	}

	Filesets::iterator fsIter;
	int i = 0;
	for (fsIter = _srcFileset.begin(); fsIter != _srcFileset.end(); fsIter++, i++)
	{	
		Fileset inputFile = *fsIter;
		int64 tempSize = _pFtpDownloader->getFileSize(inputFile.filename);
		_totalBytes += tempSize;
		MOLOG(Log::L_DEBUG, CLOGFMT(FTPPropn, "[%s] file size of [%s] is %lld bytes"), 
			_strLogHint.c_str(),(*fsIter).filename.c_str(), tempSize);

		if (fsIter!=_srcFileset.begin())
		{
			_supportFilesize += tempSize;
		}

		if(tempSize < inputFile.firstOffset)
		{
			fsIter->firstOffset = 0;
			fsIter->finalOffset = 0;
			_targetFileset[i].firstOffset = 0;
			_targetFileset[i].finalOffset = 0;
			_bNormalize = false;
		}
	}

	_bitrateCtrl.setBitrate(_nBandwidthBps);
	return true;
}

bool FTPPropagation::propagate()
{
	_bitrateCtrl.start();

	int64 nStart = SYS::getTickCount();
	unsigned int nFileIndex;
	for (nFileIndex = 0; nFileIndex < _srcFileset.size(); nFileIndex++)
	{
		bool bMainFileFlag = !nFileIndex;
		copy(_srcFileset[nFileIndex],_targetFileset[nFileIndex],bMainFileFlag);
		_pFtpDownloader->closeFile();
	}

	//
	// release resources
	//

	_totalBytes = _OrigfileSize;
	_pFtpDownloader->close();
//	closeTargetFile();

	if (!_strErr.empty())
	{
		return false;
	}

	calculateBitrate(nStart);

	return true;
}

void FTPPropagation::stop()
{
	_bStop = true;
#ifdef ZQ_OS_MSWIN
	SetEvent(_hwait);
#else
	sem_post(&_hwait);
	_bSignaled = true;
#endif
	MOLOG(Log::L_INFO, CLOGFMT(FTPPropn, "[%s] stop() called"), _strLogHint.c_str());
}

void FTPPropagation::close()
{

    _pFileIoFactory->deleteFile(_strTempIndexFile.c_str());
	MOLOG(Log::L_INFO, CLOGFMT(FTPPropn, "[%s] close()"), _strLogHint.c_str());
}

int64 FTPPropagation::getTotalSize()
{
	return _totalBytes;
}

bool FTPPropagation::getMediaInfo( MediaInfo& mInfo )
{
	mInfo = _info;
	return true;
}

void FTPPropagation::OnProgress( int64 nProcessBytes )
{
}

void FTPPropagation::setLastError( const std::string& strErr, int errCode )
{
	if (!_bFailYet)
	{
		_strErr = strErr;
		_errCode = errCode;
		_bFailYet = true;
	}
}

bool FTPPropagation::resumePropagate()
{
	bool result = true;
	_bitrateCtrl.start();

	int64 nStart = SYS::getTickCount();

	Filesets::iterator iter = _srcFileset.begin();
	if (!copy(*iter,_targetFileset[0],true,false))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPPropn, "[%s] copy main file failed"), _strLogHint.c_str());
		_pFtpDownloader->closeFile();
		_pFtpDownloader->close();
		closeTargetFile();
		return false;
	}

#ifdef ZQ_OS_MSWIN
	while (WaitForSingleObject(_hwait,_dWaitTime) == WAIT_TIMEOUT && !_bStop)
	{
		int64 tempFileSize = _pFtpDownloader->getFileSize((*iter).filename);
		if (tempFileSize > _OrigfileSize)
		{
			result = copy(*iter,_targetFileset[0],true,true);
			if(!result)
				SetEvent(_hwait);

			if (_OrigfileSize < tempFileSize)
			{
				result = false;
				SetEvent(_hwait);
			}
		}
		else if (tempFileSize == -1)
		{
			std::string errStr = _pFtpDownloader->getLastError();
			setLastError(std::string("failed to get file ") + (*iter).filename.c_str() +", " +errStr, ERRCODE_VSTRM_API_ERROR);
			MOLOG(Log::L_ERROR, CLOGFMT(FTPPropn, "[%s] failed to get file size, %s"), _strLogHint.c_str(), errStr.c_str());
			result = false;
			SetEvent(_hwait);
		}
		else
			SetEvent(_hwait);

		_pFtpDownloader->closeFile();	
	}
#else
	struct timespec tsp;
	struct timeval tval;
	gettimeofday(&tval,NULL);
	int64 macros = _dWaitTime*1000l + tval.tv_usec;
	
	tsp.tv_sec = tval.tv_sec + macros/1000000l;
	tsp.tv_nsec = (macros%1000000l)*1000;
	int rt = 0;
	//int retry = 3;
	while ((rt = sem_timedwait(&_hwait,&tsp)) && !_bStop && (errno == ETIMEDOUT || errno == EINTR))
	{
		int64 tempFileSize = _pFtpDownloader->getFileSize((*iter).filename);
		if (tempFileSize > _OrigfileSize)
		{	
			//retry = 3;
			result =  copy(*iter,_targetFileset[0],true,true);
			if(!result)
			{
				sem_post(&_hwait);
				_bSignaled = true;
			}

			if (_OrigfileSize < tempFileSize)
			{
				result = false;
				sem_post(&_hwait);
				_bSignaled = true;
			}
		}
		else if (tempFileSize == -1)
		{
			std::string errStr = _pFtpDownloader->getLastError();
			setLastError(std::string("failed to get file ") + (*iter).filename.c_str() +", " +errStr, ERRCODE_VSTRM_API_ERROR);
			MOLOG(Log::L_ERROR, CLOGFMT(FTPPropn, "[%s] failed to get file size, %s"), _strLogHint.c_str(), errStr.c_str());
			{
				result = false;
				sem_post(&_hwait);
				_bSignaled = true;
			}
		}
		else
		{
			//retry--;
			//if(retry == 0)
			{
				sem_post(&_hwait);
				_bSignaled = true;
			}
		}

		gettimeofday(&tval,NULL);
		int64 macros = _dWaitTime*1000l + tval.tv_usec;
	
		tsp.tv_sec = tval.tv_sec + macros/1000000l;
		tsp.tv_nsec = (macros%1000000l)*1000;

		_pFtpDownloader->closeFile();	
	}
//	printf("run out while signal[%d] retry[%d]\n",_bSignaled,retry);
	if(_bSignaled)
	{
		int nval = 0;
		if((sem_getvalue(&_hwait,&nval) == 0) && nval < 1)
		{
			sem_post(&_hwait);
		}
	}
#endif
	if(!result)	
		MOLOG(Log::L_ERROR, CLOGFMT(FTPPropn, "[%s] copy main file failed error[%s]"),
			 _strLogHint.c_str(), _pFtpDownloader->getLastError().c_str());

	if (result && !_bStop)
	{
		iter ++;

		for (int i = 1; (iter != _srcFileset.end()) &&( i <(int)_targetFileset.size()); iter++,i++)
		{
			copy(*iter,_targetFileset[i],false);
			if (strstr((*iter).filename.c_str(),"VVX")|| strstr((*iter).filename.c_str(),"VV2")|| strstr((*iter).filename.c_str(),"index"))
				_indexSize = _pFtpDownloader->getFileSize((*iter).filename);
		}

		_supportFilesize = _processBytes - _OrigfileSize - _indexSize;
	}
  	
	_totalBytes = _OrigfileSize; 
	_pFtpDownloader->close();
//	closeTargetFile();

	if (!_strErr.empty())
	{
		return false;
	}

	calculateBitrate(nStart);

	return result;
}

bool FTPPropagation::seekTargetFilePos( int64 fileEnd )
{
	if (!_pFileIo.get())
		return false;

	if (!_pFileIo->seekfile(fileEnd,ZQTianShan::ContentProvision::FileIo::POS_BEGIN))
	{
		std::string strErr;
		int nErrCode;
		_pFileIo->getLastError(strErr, nErrCode);
		setLastError(strErr, nErrCode);
		return false;
	}
	return true;

}

bool FTPPropagation::copy(const Fileset& sourceName,const Fileset& targetName,bool bmainFileFlag,bool bresume)
{
	int createway = 1;
	bool result = true;

	if (bresume)
		createway = 2;

	if (bresume && _OrigfileSize)
	{
		if (!_pFtpDownloader->openFile(sourceName.filename.c_str(),(ZQTianShan::ContentProvision::FTPClient::FileMode)0,_OrigfileSize))
		{
			std::string errstr = _pFtpDownloader->getLastError();
			setLastError(std::string("failed to open remote file ") + sourceName.filename + " with error:" + errstr, ERRCODE_VSTRM_API_ERROR);
			MOLOG(Log::L_ERROR, CLOGFMT(FTPPropn, "[%s] failed to open remote file %s with error: %s"), _strLogHint.c_str(), 
				sourceName.filename.c_str(), errstr.c_str());
			return false;
		}
	}
	else
	{
		if (!openRemoteFile(sourceName))
		{
			return false;
		}
	}
	
	if (!openTargetFile(targetName,createway))
	{
		_pFtpDownloader->closeFile();
		return false;
	}

	if (bresume && _OrigfileSize)
		if (!seekTargetFilePos(_OrigfileSize))
			return false;

	while(!_bStop)
	{
		char buf[64*1024];	
		int nReadData = 0;
		_pFtpDownloader->readFile(buf, sizeof(buf), nReadData);
/*
		if(!bRead && !nReadData)
		{
			std::string errmsg = _pFtpDownloader->getLastError();
			setLastError(std::string("Read file failed with error: ") + errmsg, ERRCODE_NTFS_READFILE);
			MOLOG(Log::L_ERROR, CLOGFMT(FTPPropn, "[%s] failed to copy file [%s] with error: %s"), _strLogHint.c_str(), targetName.c_str(), errmsg.c_str());
			result = false;
			break;
		}
*/
		if(nReadData>0)
		{
			if (!writeTargetFile(buf, nReadData))
			{
				std::string errmsg = _pFtpDownloader->getLastError();
				setLastError(std::string("write file failed with error: ") + errmsg, ERRCODE_NTFS_READFILE);
				MOLOG(Log::L_ERROR, CLOGFMT(FTPPropn, "[%s] failed to write file [%s] with error: %s"), _strLogHint.c_str(), targetName.filename.c_str(), errmsg.c_str());
				result  = false;
				break;
			}

			if (_bEnableMD5 && bmainFileFlag)
				_md5ChecksumUtil.checksum(buf, nReadData);
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(FTPPropn, "[%s] file [%s] copied"), _strLogHint.c_str(), targetName.filename.c_str());
			break;
		}

		_processBytes += nReadData;
		if (bmainFileFlag)
			_OrigfileSize += nReadData;
		_bitrateCtrl.control(_processBytes);
	}

	closeTargetFile();	

	return result;
}

void  FTPPropagation::calculateBitrate(int64& nstart)
{
	uint32 dwDuration =(uint32)(SYS::getTickCount() - nstart);	//here maybe wrong, but does not matter, just a log
	uint32 dwTranferRate;
	if (dwDuration)
	{
		dwTranferRate = (uint32)(_totalBytes*8000/dwDuration);
	}
	else
	{
		dwTranferRate = 0;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPPropn, "[%s] Actual copy rate is [%d]bps"), _strLogHint.c_str(), dwTranferRate);
}

void FTPPropagation::getLastError( std::string& strErr, int& errCode )
{
	strErr = _strErr;
	errCode = _errCode;
}
void FTPPropagation::setDecodeSourceURL(bool bDecodeSourceURL)
{
	_bDecodeSourceURL = bDecodeSourceURL;
}
bool FTPPropagation::getOutputFiles(std::vector<std::string>& filelists, int& outputfilecount, int& indexType)
{
	indexType = _indexType;	
	int nSize = _targetFileset.size();

	if(nSize < 1)
		return false;

	filelists.push_back(_targetFileset[nSize -1 ].filename);
	for(int i = 1; i< nSize -1; i++)
	{
		filelists.push_back(_targetFileset[i].filename);
	}
	outputfilecount = nSize -1;
	return true;
}
}
}

