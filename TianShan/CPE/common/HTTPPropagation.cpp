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


#include "HTTPPropagation.h"
#include "Log.h"
#include "urlstr.h"
#include "ErrorCode.h"
#include "TianShanDefines.h"
#include "CTFLib.h"
#include "CStdFileIoFactory.h"
#include "ParseIndexFile.h"
#include "SystemUtils.h"
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
#define HTTPPropn			"HTTPPropn"

extern int64 SparseFileSize;
using namespace ZQ::common;

namespace ZQTianShan {
	namespace ContentProvision {
/*bool getIdxSubFileInfoForHttp(const char* szIdxFile, std::string&  strIndexExt, std::vector<SubFileInfo>& subFiles, MediaInfo& info)
{
	{
		strIndexExt = "index";

		ZQ::IdxParser::IdxParserEnv  env;
		env.AttchLogger(&glog);
		ZQ::IdxParser::IndexFileParser idxParser(env);
		ZQ::IdxParser::IndexData	idxData;
		if(!idxParser.ParserIndexFileFromCommonFS("",idxData, true, szIdxFile))
		{
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
		return true;
	}

	return false;
}*/
HTTPPropagation::HTTPPropagation(HTTPClientFactory* _pHttpClientFactory, FileIoFactory* pFileIoFactory)
	:__pHttpClientFactory(_pHttpClientFactory), _pFileIoFactory(pFileIoFactory)
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
#ifdef ZQ_OS_MSWIN
	_hwait = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
	sem_init(&_hwait,0,0);	
	_bSignaled = false;
#endif
    _strProvideAssetId = "";
	_strProvideId = "";
	_bmainFileFlag = false;

	_bWriteFileError = false;

	_indexType = -1;

	_timeout = 10;

	_endpos = 0;

    _beginpos = 0;

	_testBandwidth = 0;
    _testRange = "";
	_nspeed = 1;
	_transferDelay = 0;
	_bVstreamIO = false;

    ///for cache gateway
	_transferUrl = "";
	_transferId = "";
	_sleepTime = 5000;
	_indexFileSize = 0;
	_outputFileset.clear();
}

HTTPPropagation::~HTTPPropagation()
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
void HTTPPropagation::deleteOutput(bool del)
{
	if(!_bVstreamIO)
	{
		CStdFileIoFactory * pIoFactory = (CStdFileIoFactory *)_pFileIoFactory;
		Filesets::iterator itor = _targetFileset.begin();
		while(itor != _targetFileset.end())
		{
			try
			{
				std::string rootDir = pIoFactory ->getRootDir();
				if(del)
				{
					bool bret = pIoFactory->deleteFile(((std::string )(rootDir + itor->filename)).c_str());
					MOLOG(Log::L_INFO, CLOGFMT(HTTPPropn, "[%s] delete file [%s]  %s"),
						_strLogHint.c_str(),itor->filename.c_str(), bret?"success":"failure");

				}
				else
				{
					std::string newName = rootDir + itor->filename + std::string("_bak");

					bool bret = pIoFactory->moveFile(((std::string )(rootDir + itor->filename)).c_str(),newName.c_str());
					MOLOG(Log::L_INFO, CLOGFMT(HTTPPropn, "[%s] rename file [%s] to [%s] %s"),
						_strLogHint.c_str(),itor->filename.c_str(), newName.c_str(), bret?"success":"failure");

				}
			}
			catch (...){
			}
			itor++;
		}	
	}

}
void HTTPPropagation::setFilename(const char* szFilename)
{
	//set the target file name
	_strFilename = szFilename;
	_strLogHint = _strFilename;
}

int64 HTTPPropagation::getProcessSize()
{
	return _processBytes;
}

void HTTPPropagation::setMaxBandwidth(unsigned int nBandwidthBps)
{
	_nBandwidthBps = nBandwidthBps;
	
	MOLOG(Log::L_DEBUG, CLOGFMT(HTTPPropn, "[%s] set MaxBandwidth=%d bps"), _strLogHint.c_str(), nBandwidthBps);
}

void HTTPPropagation::setTargetDir(const char* szDir)
{
	if(szDir == NULL || strlen(szDir) == 0)
		return;

	if (szDir[strlen(szDir)-1] != FNSEPC)
		_outputDir = std::string(szDir) + std::string(FNSEPS);
	else
		_outputDir = szDir;
}

void HTTPPropagation::setCacheDir(const char* szCachDir)
{
	if (szCachDir[strlen(szCachDir)-1] != FNSEPC)
		_tempDir = szCachDir + std::string(FNSEPS);
	else
		_tempDir = szCachDir;
}

bool HTTPPropagation::openTargetFile(const Fileset& targetName,int createway)
{
	_pFileIo.reset(_pFileIoFactory->create());

	FileIo::AccessMode accessmode = FileIo::ACCESS_WRITE;

	if(createway == FileIo::WAY_OPEN_EXISTING)
	{
		accessmode =  FileIo::ACCESS_APPEND;
	}
	if (!_pFileIo->openfile((char*)targetName.filename.c_str(), accessmode,
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
 	if(!_bVstreamIO && targetName.firstOffset >0 && targetName.finalOffset >=  targetName.firstOffset)
 	{
		int64 firstOffset = targetName.firstOffset;
		int64 finalOffset = targetName.finalOffset;
// 		firstOffset = 1024*1024;
// 		finalOffset = 1024*1024*1024;

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

void HTTPPropagation::closeTargetFile()
{
	if (_pFileIo.get())
	{
		_pFileIo->closefile();
		_pFileIo.reset(0);
	}

}

bool HTTPPropagation::writeTargetFile(char* pBuf, int nLen)
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

void HTTPPropagation::getMD5(std::string& strMD5)
{
	strMD5 = _md5ChecksumUtil.lastChecksum();
}

void HTTPPropagation::enableMD5(bool bEnable)
{
	_bEnableMD5 = bEnable;
}

bool HTTPPropagation::downloadIndexFile()
{	
	Ice::Long lStart = ZQTianShan::now();

	MOLOG(Log::L_DEBUG, CLOGFMT(HTTPPropn, "[%s][%s][%s]download index file"), _strLogHint.c_str(), _strProvideId.c_str(), _strProvideAssetId.c_str());

	std::string url = "";
	std::string transferUrl = "";	
	LocateRequest locateRequest;
	LocateResponse locateResponse;	
	int64 maxLen = -1;

	_pHttpDownloader->setSubfile("index");

//	std::string deleteUrl = std::string("http://") + _strServer + ":" + _strPort; 
	std::string deleteUrl = _srcURL;

	if(!_bFileset)
	{
//		url = std::string("http://") + _strServer + ":" + _strPort + "/vodadi.cgi";
		url = _srcURL;
		locateRequest.pid  = _strProvideId;
		locateRequest.paid = _strProvideAssetId;
		locateRequest.subFile = "index";
		locateRequest.beginPos = -1;
		locateRequest.endPos = -1;

		if(_testBandwidth > 0)
		{
			locateRequest.bitRate = _testBandwidth;
		}
		else
			locateRequest.bitRate = 3750000;

		locateRequest.transferDelay = _transferDelay;

		if(!_pHttpDownloader->prepareLocateFile(url, locateRequest, locateResponse))
		{
			int errorCode;
			std::string errstr;
			_pHttpDownloader->getLastErrorMsg(errorCode, errstr);
			setLastError(std::string("failed to prepare locate index file with error:") + errstr, ERRCODE_NTFS_READFILE);
			MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s][%s][%s]failed to prepare locate index file with error: %s"), _strLogHint.c_str(), _strProvideId.c_str(), _strProvideAssetId.c_str(),errstr.c_str());
			return false;
		}
		char urlPort[65] = "";
		snprintf(urlPort, sizeof(urlPort) -1, "%u", _transferServerPort);

		if (locateResponse.transferportnum.length() <= 0)
			locateResponse.transferportnum = urlPort;

		char buf[256] = "";
		snprintf(buf, sizeof(buf)-2, "http://%s:%s/%s", locateResponse.transferHost.c_str(), locateResponse.transferportnum.c_str(), locateResponse.transferId.c_str());

		transferUrl = buf;

		maxLen  = -1 ;
		if(locateResponse.OpenForWrite)
		{
			int64 nBeginPos = 0;
			int64 nEndPos = 0;

			if(_pHttpDownloader->parserAavailableRange(locateResponse.availableRange, nBeginPos, nEndPos))
				maxLen = nEndPos;
			else
				maxLen = 65536;
		}
	}
	else
	{
		transferUrl = _transferUrl + "&subfile=.index";
		maxLen = -1;
	}

	_indexFileSize = _pHttpDownloader->downloadFile(transferUrl, _strTempIndexFile, 0, maxLen, false);

	if(_indexFileSize < 0)
	{
		if(!_bFileset)
		{
			_pHttpDownloader->deleteTransferId(deleteUrl, locateResponse.transferId, locateResponse.transferHost);
		}
		int errorCode;
		std::string errstr;
		_pHttpDownloader->getLastErrorMsg(errorCode, errstr);
		setLastError(std::string("failed to download index file with error:") + errstr, ERRCODE_NTFS_READFILE);
		MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s][%s][%s]failed to download index file with error: %s"), _strLogHint.c_str(), _strProvideId.c_str(), _strProvideAssetId.c_str(), errstr.c_str());
		return false;
	}

	if(!_bFileset)
	{
		_pHttpDownloader->deleteTransferId(deleteUrl, locateResponse.transferId, locateResponse.transferHost);
	}

	MOLOG(Log::L_INFO, CLOGFMT(HTTPPropn, "[%s] index file downloaded to local %s took %dms"), _strLogHint.c_str(), 
		_strTempIndexFile.c_str(), ZQTianShan::now() - lStart);

	return true;
}

void HTTPPropagation::setLog( ZQ::common::Log* pLog )
{
	ZQ::common::setGlogger(pLog);
}

bool HTTPPropagation::prepare()
{
	SubFileInfo subf;
	std::vector<SubFileInfo> subFiles;
	std::vector<SubFileInfo>::iterator iter;
	char szBuf[512];
	std::string errmsg;

	subFiles.clear();

	MOLOG(Log::L_DEBUG, CLOGFMT(HTTPPropn, "[%s] enter prepare()"), _strLogHint.c_str());

	ZQ::common::URLStr srcUrl(_srcURL.c_str());
	_strServer = srcUrl.getHost();
	int nPort = srcUrl.getPort();

	std::string _transferpath = srcUrl.getPath();
	if(!_transferpath.empty())
	{
		_transferId = _transferpath;
	} 
	std::string fileset = srcUrl.getVar("fileset");
	if(!stricmp(fileset.c_str(), "true"))
	{
		_bFileset = true; // reuseable sessionid
		_transferUrl = _srcURL;
	}
	else
	{
		_bFileset = false; //raw c2http protocal

         std::string struri = srcUrl.getPath();
		 if(struri != "vodadi.cgi")
			 srcUrl.setPath("vodadi.cgi");
		 _srcURL = srcUrl.generate();
	}

	///for cache gateway, if port = 0, using configration transfer fileport
	if(nPort == 0)
	{
		nPort = _transferServerPort;
		if(_bFileset)
		{
			ZQ::common::URLStr transferUrl(_srcURL.substr(2).c_str());
			transferUrl.setPort(nPort);
			_transferUrl = transferUrl.generate();
		}
	}

	char strTempPort[64] = "";
	itoa(nPort, strTempPort, 10);
	_strPort = strTempPort;

	MOLOG(Log::L_DEBUG, CLOGFMT(HTTPPropn, "[%s] SourceUrl=%s, Host=%s, Port=%s"),_strLogHint.c_str(), _srcURL.c_str(), _strServer.c_str(), _strPort.c_str());

	if(_bFileset)
	{
		MOLOG(Log::L_DEBUG, CLOGFMT(HTTPPropn, "[%s] TransferUrl=%s, TransferID=%s"),_strLogHint.c_str(), _transferUrl.c_str(), _transferId.c_str());
	}

//	ZQTianShan::ContentProvision::C2HttpClient* pc2HttpClient = __pHttpClientFactory->create();
	ZQTianShan::ContentProvision::C2HttpClient* pc2HttpClient = new C2HttpClient(ZQ::common::HttpClient::HTTP_IO_KEEPALIVE, &glog,_timeout, _bindIP,_transferip);
	if (!pc2HttpClient)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s] failed to create HTTPClient instance"), _strLogHint.c_str());
		return false;
	}
	_pHttpDownloader.reset(pc2HttpClient);
	_pHttpDownloader->setIngressCapacity(_IngressCapcaity);
	_pHttpDownloader->setPIdAndPaid(_strProvideId, _strProvideAssetId);

	//cache index file in a temp directory

	_strTempIndexFile = _tempDir +  _strProvideAssetId + _strProvideId + ".index";	
	if (!downloadIndexFile())
		return false;	

	//get IndexFile extension
	int npos = _strTempIndexFile.rfind('.');
	if(npos <  0)
	{
		snprintf(szBuf, sizeof(szBuf)-1, "[%s] failed to parser index file %s extension.", _strLogHint.c_str(), _strTempIndexFile.c_str());	
		MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s] %s"), _strLogHint.c_str(), szBuf);
		setLastError(szBuf, ERRCODE_NTFS_READFILE);	
		return false;
	}
	std::string strIndexExt = _strTempIndexFile.substr(npos + 1);
// 	std::string strOrgFile =  indexFilename.substr(0, npos);

	if(!ParseIndexFileInfo::getIdxSubFileInfo(_strTempIndexFile.c_str(), strIndexExt, subFiles, _info))
	{
		//failed to parse the index file
		snprintf(szBuf, sizeof(szBuf)-1, "Failed to parse VVX/VV2/INDEX index file for [PID: %s][PAID: %s]", _strProvideId.c_str() , _strProvideAssetId.c_str());
		MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s] %s"), _strLogHint.c_str(), szBuf);
		setLastError(szBuf, ERRCODE_NTFS_READFILE);	
		return false;
	}

	if (stricmp(strIndexExt.c_str(), "VVX") == 0)
	{
		strcpy(subf.ext, ".VVX");
		_indexType = CTF_INDEX_TYPE_VVX;
	}
	else if(stricmp(strIndexExt.c_str(), "VV2") == 0)
	{
		strcpy(subf.ext, ".VV2");
       _indexType = CTF_INDEX_TYPE_VV2;
	}
	else 
	{
		strcpy(subf.ext, ".index");
		_indexType = 3;
#ifndef ZQ_OS_MSWIN
		_indexType = CTF_INDEX_TYPE_VVC;
#endif
	}

	subf.firstOffset = 0;
	///for cache gateway, index filesize -1 is the indexfile findalOffset;
	if(_indexFileSize > 0)
		subf.finalOffset = _indexFileSize -1;
	else
		subf.finalOffset = 0;
	subf.totalFilesize = 0;
	subf.bIsSparseFile = false;
 	subFiles.push_back(subf);
	for (iter = subFiles.begin();iter != subFiles.end();iter ++)
	{
		Fileset fileset;
		fileset.filename = iter->ext;
		fileset.firstOffset = iter->firstOffset;
		fileset.finalOffset = iter->finalOffset;
		fileset.totalFilesize = iter->totalFilesize;
		fileset.bIsSparseFile = iter->bIsSparseFile;
		_srcFileset.push_back(fileset);
//		_srcFileset.push_back(strOrgFile + iter->ext);

		fileset.filename = _strFilename + iter->ext;
		_targetFileset.push_back( fileset);

		MOLOG(Log::L_INFO, CLOGFMT(HTTPPropn, "[%s] FileExt[%s] TotalFileSize[%lld] FirstOffset[%lld] FinalOffset[%lld]IsSparseFile[%d]"), 
			_strLogHint.c_str(), fileset.filename.c_str(), fileset.totalFilesize, fileset.firstOffset, fileset.finalOffset, fileset.bIsSparseFile);
	}


	std::string url = "";
	std::string transferUrl = "";	
	LocateRequest locateRequest;
	LocateResponse locateResponse;	

	///get file size
//	url = std::string("http://") + _strServer + ":" + _strPort + "/vodadi.cgi";
	url = _srcURL;
	locateRequest.pid  = _strProvideId;
	locateRequest.paid = _strProvideAssetId;
	locateRequest.beginPos = -1;
	locateRequest.endPos = -1;
	locateRequest.bitRate = 1;
	locateRequest.transferDelay = _transferDelay;

	Filesets::iterator fsIter;
	for (fsIter = _srcFileset.begin(); fsIter != _srcFileset.end(); fsIter++)
	{
		int64 tempSize;
		if(!_bFileset)//raw c2http protocal
		{
			std::string fileExt = fsIter->filename;
			if(fileExt.empty())
			{
				fileExt = "forward/1";
			}
			locateRequest.subFile = fileExt;
			locateRequest.beginPos = fsIter->firstOffset;
			_pHttpDownloader->setSubfile(locateRequest.subFile);
			tempSize = _pHttpDownloader->getFileSize(url, locateRequest, locateResponse);
		}
		else ///for cache gateway, all file done
		{
			if(!fsIter->bIsSparseFile)
				tempSize = fsIter->finalOffset - fsIter->firstOffset + 1;
			else
			{
				tempSize = fsIter->finalOffset  + 1;
				locateResponse.OpenForWrite = false;
			}
		}

		if(tempSize <= 0)
		{
			snprintf(szBuf, sizeof(szBuf)-1, "Failed to get filesize for [PID: %s][PAID: %s][SubFile: %s]", _strProvideId.c_str() , _strProvideAssetId.c_str(),locateRequest.subFile.c_str());
			MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s] %s"), _strLogHint.c_str(), szBuf);
			setLastError(szBuf, ERRCODE_NTFS_READFILE);	
			return false;
		}
		_totalBytes += tempSize;
		MOLOG(Log::L_DEBUG, CLOGFMT(HTTPPropn, "[%s] file size of [%s] is %lld bytes, Completed file[%d]"), 
			_strLogHint.c_str(),(fsIter->filename).c_str(), tempSize, !locateResponse.OpenForWrite);

		if (fsIter!=_srcFileset.begin())
		{
			_supportFilesize += tempSize;
		}
	}

	if(!_bFileset)//raw c2http protocal
         _supportFilesize += _indexFileSize;
//	_bitrateCtrl.setBitrate(_nBandwidthBps);
	return true;
}

bool HTTPPropagation::propagate()
{
	if(_bFileset)
	{
		return propagateEx();
	}

	_bitrateCtrl.start();

	int64 nStart = SYS::getTickCount();
	Filesets::iterator fsIter;
	unsigned int nFileIndex;
	for (nFileIndex = 0; nFileIndex < _srcFileset.size(); nFileIndex++)
	{
		Fileset inputFileNameExt = _srcFileset[nFileIndex];
		if(inputFileNameExt.filename == "")
			inputFileNameExt.filename = "forward/1";

		Fileset outputFileName = _targetFileset[nFileIndex];
		_bmainFileFlag = !nFileIndex;

		if(!copy(inputFileNameExt, outputFileName))
		{
			return false;
		}
	}

	//
	// release resources
	//

	_totalBytes = _OrigfileSize;

	if (!_strErr.empty())
	{
		return false;
	}

	calculateBitrate(nStart);

	return true;
}
bool HTTPPropagation::propagateEx()
{
	_bitrateCtrl.start();

	int64 nStart = SYS::getTickCount();
	Filesets::iterator fsIter;
	unsigned int nFileIndex;
	for (nFileIndex = 0; nFileIndex < _srcFileset.size(); nFileIndex++)
	{
		Fileset inputFileNameExt = _srcFileset[nFileIndex];
		Fileset outputFileName = _targetFileset[nFileIndex];
		_bmainFileFlag = !nFileIndex;

		if(!copy(inputFileNameExt, outputFileName))
		{
			return false;
		}
	}

	//
	// release resources
	//

	_totalBytes = _OrigfileSize;

	if (!_strErr.empty())
	{
		return false;
	}

	calculateBitrate(nStart);
	return true;
}
void HTTPPropagation::stop()
{
	if(_pHttpDownloader.get())
		_pHttpDownloader->setError(true, std::string("HTTPProSource closed"));

	_bStop = true;
#ifdef ZQ_OS_MSWIN
	SetEvent(_hwait);
#else
	sem_post(&_hwait);
	_bSignaled = true;
#endif
	MOLOG(Log::L_INFO, CLOGFMT(HTTPPropn, "[%s] stop() called"), _strLogHint.c_str());
}

void HTTPPropagation::close()
{

    _pFileIoFactory->deleteFile(_strTempIndexFile.c_str());
	MOLOG(Log::L_INFO, CLOGFMT(HTTPPropn, "[%s] close()"), _strLogHint.c_str());
}

int64 HTTPPropagation::getTotalSize()
{
	return _totalBytes;
}

bool HTTPPropagation::getMediaInfo( MediaInfo& mInfo )
{
	mInfo = _info;
	return true;
}

void HTTPPropagation::OnProgress( int64 nProcessBytes )
{
}

void HTTPPropagation::setLastError( const std::string& strErr, int errCode )
{
	if (!_bFailYet)
	{
		_strErr = strErr;
		_errCode = errCode;
		_bFailYet = true;
	}
}

bool HTTPPropagation::resumePropagate()
{
	if(_bFileset)
	{
		return propagateEx();
	}
	bool result = true;
	_bitrateCtrl.start();

	int64 nStart = SYS::getTickCount();

	Filesets::iterator iter = _srcFileset.begin();
	Fileset mainFile = *iter;
	if(mainFile.filename == "")
		mainFile.filename ="forward/1";

	_bmainFileFlag = true;
	if (!copy(mainFile,_targetFileset[0],false))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s] copy main file failed"), _strLogHint.c_str());
		closeTargetFile();
		return false;
	}

	std::string fileExt = mainFile.filename;
	std::string url = "";
	std::string transferUrl = "";	
	LocateRequest locateRequest;
	LocateResponse locateResponse;	
	int64 maxLen = -1;

//	url = std::string("http://") + _strServer + ":" + _strPort + "/vodadi.cgi";
	url = _srcURL;
	locateRequest.pid  = _strProvideId;
	locateRequest.paid = _strProvideAssetId;
	locateRequest.subFile = fileExt;
	locateRequest.beginPos = -1;
	locateRequest.endPos = -1;
	locateRequest.bitRate = 1;
	locateRequest.transferDelay = _transferDelay;

	bool bIsCompleted = false;

#ifdef ZQ_OS_MSWIN
	while (WaitForSingleObject(_hwait,_dWaitTime) == WAIT_TIMEOUT && !_bStop)
	{
 		int64 tempFileSize = _pHttpDownloader->getFileSize(url, locateRequest, locateResponse);

		bIsCompleted = !locateResponse.OpenForWrite;

		if (tempFileSize > _OrigfileSize)
		{
			_bmainFileFlag = true;

			MOLOG(Log::L_DEBUG, CLOGFMT(HTTPPropn, "[%s] start sleep %d second for copy file"), _strLogHint.c_str(), _sleepTime);
			SYS::sleep(_sleepTime);
			MOLOG(Log::L_DEBUG, CLOGFMT(HTTPPropn, "[%s] finish sleep %d second for copy file "), _strLogHint.c_str(), _sleepTime);

			result = copy(mainFile,_targetFileset[0],true);
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
			std::string strErr;
			int ncode;
			_pHttpDownloader->getLastErrorMsg(ncode, strErr);
			setLastError(std::string("failed to get file ") + (_srcFileset.begin()->filename) +", " +strErr, ERRCODE_C2PROPAGATION_DOWNLOADFILE_FAIL);
			MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s] failed to get file size, %s"), _strLogHint.c_str(), strErr.c_str());
			result = false;
			SetEvent(_hwait);
		}
		else if(bIsCompleted)
			   SetEvent(_hwait);
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
		int64 tempFileSize = _pHttpDownloader->getFileSize(url, locateRequest, locateResponse);
		bIsCompleted = !locateResponse.OpenForWrite;

        if (tempFileSize > _OrigfileSize)
        {
            _bmainFileFlag = true;
            result = copy(mainFile,_targetFileset[0],true);
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
            std::string strErr;
            int ncode;
            _pHttpDownloader->getLastErrorMsg(ncode, strErr);
            setLastError(std::string("failed to get file ") + (_srcFileset.begin()->filename) +", " +strErr, ERRCODE_C2PROPAGATION_DOWNLOADFILE_FAIL);
            MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s] failed to get file size, %s"), _strLogHint.c_str(), strErr.c_str());
            result = false;
            sem_post(&_hwait);
            _bSignaled = true;
        }
        else if(bIsCompleted)
        {
            sem_post(&_hwait);
            _bSignaled = true;
        }

        gettimeofday(&tval,NULL);
        int64 macros = _dWaitTime*1000l + tval.tv_usec;

        tsp.tv_sec = tval.tv_sec + macros/1000000l;
        tsp.tv_nsec = (macros%1000000l)*1000;

    }

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
	{
		std::string strErr;
		int ncode;
		_pHttpDownloader->getLastErrorMsg(ncode, strErr);
 		MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s] copy main file failed error[%s]"), _strLogHint.c_str(), strErr.c_str());
	}

	if (result && !_bStop)
	{
		iter ++;

		for (int i = 1; (iter != _srcFileset.end()) &&( i <(int)_targetFileset.size()); iter++,i++)
		{
			_bmainFileFlag = false;
			if(!copy(*iter,_targetFileset[i]))
			{
				result = false;
				break;
			}
			if (strstr((iter->filename).c_str(),"VVX")|| strstr((iter->filename).c_str(),"VV2")|| strstr((iter->filename).c_str(),"index"))
			{
				locateRequest.subFile = iter->filename;
				_indexSize = _pHttpDownloader->getFileSize(url, locateRequest, locateResponse);
			}
		}

		_supportFilesize = _processBytes - _OrigfileSize - _indexSize;
	}
  	
	_totalBytes = _OrigfileSize; 

	closeTargetFile();

	if (!_strErr.empty())
	{
		return false;
	}

	calculateBitrate(nStart);

	return result;
}

bool HTTPPropagation::seekTargetFilePos( int64 fileEnd )
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

bool HTTPPropagation::copy(const Fileset& sourceNameExt,const Fileset& targetName, bool bresume)
{
	Ice::Long lStart = ZQTianShan::now();
	int createway = FileIo::WAY_CREATE_ALWAYS;
	bool result = true;

	std::string availableRang;

	if (bresume)
		createway = FileIo::WAY_OPEN_EXISTING;

	if (!openTargetFile(targetName,createway))
	{
		return false;
	}
	///printfLog;
    _pHttpDownloader->setSubfile(sourceNameExt.filename);

	std::string url = "";
	std::string transferUrl = "";	
	LocateRequest locateRequest;
	LocateResponse locateResponse;	
	int64 maxLen = -1;
	int64 startOffset = 0;
//	std::string deleteUrl = std::string("http://") + _strServer + ":" + _strPort; 
	std::string deleteUrl =  _srcURL;

	int64 nAvailableBeginPos = 0, nAvailableEndPos = 0;

	if(!_bFileset)//raw c2http protocal
	{
//		url = std::string("http://") + _strServer + ":" + _strPort + "/vodadi.cgi";
		url = _srcURL;

		locateRequest.pid  = _strProvideId;
		locateRequest.paid = _strProvideAssetId;
        locateRequest.transferDelay = _transferDelay;
		locateRequest.subFile = sourceNameExt.filename;

		if (bresume && _OrigfileSize)
		{
			if (!seekTargetFilePos(_OrigfileSize))
			{
				closeTargetFile();	
				return false;
			}
			locateRequest.beginPos = _OrigfileSize ;
		}
		else
		{
			locateRequest.beginPos = sourceNameExt.firstOffset;
		}
		if(sourceNameExt.firstOffset > 0  && sourceNameExt.finalOffset >= sourceNameExt.firstOffset)
		{
			locateRequest.endPos = sourceNameExt.finalOffset;
		}
		else
			locateRequest.endPos = -1;

		if(_testBandwidth > 0)
			locateRequest.bitRate = _testBandwidth;
		else if(_nBandwidthBps <= 0)
			locateRequest.bitRate = _info.bitrate * _nspeed;
		else
			locateRequest.bitRate = _nBandwidthBps * _nspeed;

		//download start offset
		startOffset = locateRequest.beginPos;
       //download maxlength
		maxLen = -1;

		if(!_pHttpDownloader->prepareLocateFile(url, locateRequest, locateResponse))
		{
			int errorCode;
			std::string errstr;
			_pHttpDownloader->getLastErrorMsg(errorCode, errstr);
			setLastError(std::string("failed to download file with error:") + errstr, ERRCODE_C2PROPAGATION_DOWNLOADFILE_FAIL);
			MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s] failed to download file with error: %s"), _strLogHint.c_str(), errstr.c_str());
			closeTargetFile();	
			return false;
		}

		if(!_pHttpDownloader->parserAavailableRange(locateResponse.availableRange, nAvailableBeginPos, nAvailableEndPos))
		{
			if(!_bFileset)
			{
				_pHttpDownloader->deleteTransferId(deleteUrl, locateResponse.transferId, locateResponse.transferHost);
			}
			int errorcode;
			std::string strErrMsg;
			_pHttpDownloader->getLastErrorMsg(errorcode, strErrMsg);
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(HTTPPropn, "copyfile()  (%d, %s) "),
				errorcode, strErrMsg.c_str()); 
			closeTargetFile();	
			return false;
		}

		//if sparsefile, adjust beginoffset and endoffset for PWE
		if(sourceNameExt.bIsSparseFile)
		{
			//sparse file FirstOffset[28876799999] FinalOffset[28876799999]
			if(sourceNameExt.firstOffset >0 && sourceNameExt.finalOffset == sourceNameExt.firstOffset)
			{
				startOffset = nAvailableBeginPos;

				if (!seekTargetFilePos(nAvailableBeginPos))
				{
					if(!_bFileset)
					{
						_pHttpDownloader->deleteTransferId(deleteUrl, locateResponse.transferId, locateResponse.transferHost);
					}

					MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s]failed to re-seek sparsefile[%lld-%lld] beginpos [%lld-%lld]"),
						_strLogHint.c_str(), sourceNameExt.finalOffset, sourceNameExt.firstOffset, nAvailableBeginPos, nAvailableEndPos);
					closeTargetFile();	
					return false;
				}
				MOLOG(Log::L_INFO, CLOGFMT(HTTPPropn, "[%s]re-seek sparsefile to beginpos[%lld] endpos[%lld], original offset[%lld-%lld]]"),
					_strLogHint.c_str(), nAvailableBeginPos, nAvailableEndPos, sourceNameExt.firstOffset, sourceNameExt.finalOffset);
			}
		}
        
		//adjust startOffset and download maxlength
		if(locateRequest.endPos > 0)
		{
			///如果locate request返回值的　AvaiabileEndpos > 所请求的ReqEndpos值, 则AvaiabileEndpos = ReqEndpos 
		    if(nAvailableEndPos > locateRequest.endPos)
				 nAvailableEndPos = locateRequest.endPos;

			maxLen = nAvailableEndPos - startOffset  +1;
		}

		char urlPort[65] = "";
		snprintf(urlPort, sizeof(urlPort) -1, "%u", _transferServerPort);

		if (locateResponse.transferportnum.length() <= 0)
			locateResponse.transferportnum = urlPort;

		char buf[256] = "";
		snprintf(buf, sizeof(buf)-2, "http://%s:%s/%s", locateResponse.transferHost.c_str(), locateResponse.transferportnum.c_str(), locateResponse.transferId.c_str());

		transferUrl = buf;
	}
	else
	{
		startOffset = sourceNameExt.firstOffset;
		transferUrl = _transferUrl +"&subfile=" + sourceNameExt.filename;
		maxLen = -1;
	}

	if(_pHttpDownloader->downloadPartialFile(&HTTPPropagation::getTransferbuf,(void *)this, transferUrl, startOffset, maxLen) < 0)
	{
		if(!_bFileset)
		{
			_pHttpDownloader->deleteTransferId(deleteUrl, locateResponse.transferId, locateResponse.transferHost);
		}

		int errorCode;
		std::string errstr;
		_pHttpDownloader->getLastErrorMsg(errorCode, errstr);
		setLastError(std::string("failed to download file with error:") + errstr, ERRCODE_C2PROPAGATION_DOWNLOADFILE_FAIL);
		MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s] failed to download file with error: %s"), _strLogHint.c_str(), errstr.c_str());
		closeTargetFile();
		return false;
	}

	if(!_bFileset)
	{
		_pHttpDownloader->deleteTransferId(deleteUrl, locateResponse.transferId, locateResponse.transferHost);
	}

	if(_bWriteFileError)
	{
		std::string strErr;
		int errCode;
		getLastError(strErr, errCode);
		MOLOG(Log::L_ERROR, CLOGFMT(HTTPPropn, "[%s] failed to write file [%s] with error: %s"), _strLogHint.c_str(), targetName.filename.c_str(), strErr.c_str());
		result = false;
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(HTTPPropn, "[%s] file [%s][%s][PID: %s][PAID: %s] copied took %d ms"), 
			_strLogHint.c_str(), targetName.filename.c_str(), sourceNameExt.filename.c_str(), _strProvideId.c_str(), _strProvideAssetId.c_str(), 
			ZQTianShan::now() - lStart);
	}

	closeTargetFile();	

	int i = 0;
	for(i = 0; i < _outputFileset.size(); i++)
	{
		if(sourceNameExt.filename == _outputFileset[i].filename)
		{
			if(_bFileset)
			{
				_outputFileset[i].firstOffset = sourceNameExt.firstOffset;
				_outputFileset[i].finalOffset = sourceNameExt.finalOffset;
			}
			else
			{
				_outputFileset[i].firstOffset = nAvailableBeginPos;
				_outputFileset[i].finalOffset = nAvailableEndPos;
			}
			
			break;
		}
	}
	if(i == _outputFileset.size())
	{
		Fileset fileset;
		fileset.filename = sourceNameExt.filename;
		if(_bFileset)
		{
			fileset.firstOffset = sourceNameExt.firstOffset;
			fileset.finalOffset = sourceNameExt.finalOffset;
		}
		else
		{
			fileset.firstOffset = nAvailableBeginPos;
			fileset.finalOffset = nAvailableEndPos;
		}

		_outputFileset.push_back(fileset);
	}

	return result;
}

void  HTTPPropagation::calculateBitrate(int64& nstart)
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
	MOLOG(Log::L_INFO, CLOGFMT(HTTPPropn, "[%s] Actual copy rate is [%d]bps"), _strLogHint.c_str(), dwTranferRate);
}

void HTTPPropagation::getLastError( std::string& strErr, int& errCode )
{
	strErr = _strErr;
	errCode = _errCode;
}
bool HTTPPropagation::getTransferbuf(void* pCtx, char*pBuf, int64 len)
{
	HTTPPropagation* pHttpPro = (HTTPPropagation*) pCtx;
	if (pHttpPro->_bStop || !pHttpPro->writeTargetFile(pBuf, len))
	{	
	    pHttpPro->_bWriteFileError = true;
		std::string strErr;
		int errCode;
		pHttpPro->getLastError(strErr, errCode);
  		pHttpPro->setLastError(std::string("write file failed with error: HTTPProSource closed or") + strErr, ERRCODE_NTFS_WRITEFILE);
		pHttpPro->_pHttpDownloader->setError(true, std::string("write file failed with error: HTTPProSource closed or") + strErr);
		return false;
	}

	if (pHttpPro->_bEnableMD5 && pHttpPro->_bmainFileFlag)
		pHttpPro->_md5ChecksumUtil.checksum(pBuf, len);

	pHttpPro->_processBytes += len;
	if (pHttpPro->_bmainFileFlag)
		pHttpPro->_OrigfileSize += len;

	return true;
//	pHttpPro->_bitrateCtrl.control(pHttpPro->_processBytes);
}

bool HTTPPropagation::getOutputFiles(std::vector<std::string>& filelists, int& outputfilecount, int& indexType)
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
void HTTPPropagation::setTimeout(int timeout)
{
	_timeout = timeout;
}
void HTTPPropagation::getOutputFileInfo(std::map<std::string, std::string>& fileMap )
{
	char tmp[40];char tmp1[40];	

	for(int i = 0; i< _outputFileset.size(); i++)
	{
		std::string tempext = _outputFileset[i].filename;
		if (tempext[0] == '.')
			tempext = tempext.substr(1);
		if(tempext == "forward/1")
			tempext = "";

		sprintf(tmp, FMT64, _outputFileset[i].firstOffset);
		sprintf(tmp1, FMT64,_outputFileset[i].finalOffset + 1);
		std::string strrange = std::string(tmp)+std::string("-")+std::string(tmp1);
		fileMap.insert(std::make_pair(tempext,strrange));
	}
}
void HTTPPropagation::getOutputFileExtCol(std::string& ext)
{
	ext.clear();
	std::string tempext;

	for(int i = 0; i< _outputFileset.size(); i++)
	{
		tempext = _srcFileset[i].filename;
		if (tempext[0] == '.')
			tempext = tempext.substr(1);
		if(tempext == "forward/1")
			tempext = "";
		ext += tempext;
		if (i != _outputFileset.size() -1)
			ext += std::string(";");
	}
}
}
}
