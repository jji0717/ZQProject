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


#include "Log.h"
#include "FilesetTarget.h"
#include "ErrorCode.h"
#include "SystemUtils.h"

#ifndef DISABLE_PACING
#include "PacedIndex.h"
#pragma comment(lib, "PacedIndex.lib")
#endif

#include "CTFLib.h"


#define FsIo			"FsIo"
#define MOLOG (*_pLog)





using namespace ZQ::common;
using namespace std;


namespace ZQTianShan {
	namespace ContentProvision {

bool FilesetTarget::_bPacingTrace = false;


FilesetTarget::FilesetTarget(FileIoFactory* pFileIoFac):_bTypeH264(false)
{
	_pFileIoFac = pFileIoFac;

	_nOutputCount = 0;
	_nInputCount = 10;

	int i;
	for(i=0;i<_nInputCount;i++)
	{
		InputPin pin;
		pin.nPrevPin = 0;
		pin.pPrevFilter = 0;		
		_inputPin.push_back(pin);
	}

	_bDriverModule = false;

	for(i=0;i<10;i++)
	{
		SubFile& subFile = _subFiles[i];

		subFile.bIndexFile = false;
		subFile.reservedBW = 0;
		subFile.bwPercentageN = 0;   // numerator for calculating bandwidth
		subFile.bwPercentageD = 0;   // denominator for calculating bandwidth
		subFile.pacingIndexCtx = 0;	//context of pacing

		subFile.llProcOffset = 0;		
	}

	_bPacing = false;
	_bCleanup = false;
	_bEnableMD5 = false;

	_DeleteTargetOnFail = 1;
	_bIndexVVC = false;
	_trickspeed[0] = 7.5;
	_mainFileExt = "";
	_enableRAID1ForIndex = false;

	_nWriteLatencyWarningMs = 100;
}

void FilesetTarget::InitPins()
{

	_nInputCount = 2 + _SpeedAndFileExt.size();

	_inputPin.clear();
	int i;
	for(i=0;i<_nInputCount;i++)
	{
		InputPin pin;
		pin.nPrevPin = 0;
		pin.pPrevFilter = 0;		
		_inputPin.push_back(pin);
	}

	for(i=0;i<_nInputCount;i++)
	{
		SubFile& subFile = _subFiles[i];

		subFile.bIndexFile = false;
		subFile.reservedBW = 0;
		subFile.bwPercentageN = 0;   // numerator for calculating bandwidth
		subFile.bwPercentageD = 0;   // denominator for calculating bandwidth
		subFile.pacingIndexCtx = 0;	//context of pacing

		subFile.llProcOffset = 0;		
	}
}

int FilesetTarget::decideOpenFileFlag(bool bIndexFile)
{
	if (bIndexFile)
	{
		return FileIo::ATTRIB_INDEXFILE;		//disable FILE_FLAG_CACHED for index file, as Rick asked, 10/28 2008
	}
	else
	{
		return FileIo::ATTRIB_NONE;
	}
}

void FilesetTarget::fileIoFailure(std::auto_ptr<FileIo>& pFileIo, const char* szLogHeader, bool bSetLastError)
{
	std::string strErr;
	int nErrorCode;
	pFileIo->getLastError(strErr, nErrorCode);

	if (bSetLastError)
		SetLastError(strErr, nErrorCode);			

	MOLOG(Log::L_ERROR, CLOGFMT(FsIo, "[%s] Failed to %s with error: %s"),
		_strLogHint.c_str(), szLogHeader, strErr.c_str());
}

bool FilesetTarget::reserveBandwith()
{
	for(int i=0;i<_nInputCount;i++)
	{
		SubFile& subFile = _subFiles[i];
		auto_ptr<FileIo>& pFileIo = subFile.pFileIo;

		//reserve bandwidth
		if (!pFileIo->reserveBandwidth(subFile.reservedBW))
		{
			fileIoFailure(pFileIo, "reserve bandwidth");
			return false;
		}
	}

	return true;
}

void FilesetTarget::releaseBandwith()
{
	for(int i=0;i<_nInputCount;i++)
	{
		SubFile& subFile = _subFiles[i];
		auto_ptr<FileIo>& pFileIo = subFile.pFileIo;

		//release bandwidth
		if (pFileIo.get())
			pFileIo->releaseBandwidth();
	}
}

#ifndef DISABLE_PACING
bool FilesetTarget::PacedIndexInit()
{
	//
	// generate pacing group
	//
	void* pacingGroup = this;

	uint32 indexFileNo = 1;
	std::string pacingType;
	if (_bTypeH264)
		pacingType="vv2";
	else if(_bIndexVVC)
		pacingType ="index";
	else
		pacingType ="vvx";

	PacedIndexSetLogCbk(1, pacingAppLogCbk);
	uint32 paceresult = 0;
	for(int i=0; i<_nInputCount; i++)
	{
		SubFile& subFile = _subFiles[i];

		// skip the vvx, make sure it was added last
		if(i == indexFileNo)
			continue;

		paceresult = PacedIndexAddEx((void *)pacingGroup, pacingType.c_str(), subFile.strPacename.c_str(), 
			pacingAppWrite, pacingAppSeek, pacingAppSetEOF, pacingAppReportOffsets, 
			(void*)&subFile,  0,  &subFile.pacingIndexCtx);
		if(paceresult)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(FsIo, "[%s] PacedIndexAdd() on pacing group 0x%08x failed with error [%s]"), 
				subFile.strPacename.c_str(), (uint32)(pacingGroup), DecodePacedIndexError(paceresult));
			SetLastError(std::string("PacedIndexAdd() failed with error ") + DecodePacedIndexError(paceresult), ERRCODE_PACING_ERROR);
			return false;	
		}
		MOLOG(Log::L_INFO, CLOGFMT(FsIo, "[%s] PacedIndexAdd() on pacing group 0x%08x successful"), 
			subFile.strPacename.c_str(), (uint32)(pacingGroup));
	}

	// add vvx last
	paceresult = PacedIndexAddEx((void*)pacingGroup, pacingType.c_str(), _subFiles[indexFileNo].strPacename.c_str(), 
		pacingAppWrite, pacingAppSeek, pacingAppSetEOF, pacingAppReportOffsets, 
		(void*)&_subFiles[indexFileNo], _enableRAID1ForIndex?PI_ADD_FLAG_RAID1:0, &_subFiles[indexFileNo].pacingIndexCtx);
	if(paceresult)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FsIo, "[%s] PacedIndexAdd() on pacing group 0x%08x failed with error [%s]"), 
			_subFiles[indexFileNo].strPacename.c_str(), (uint32)(pacingGroup), DecodePacedIndexError(paceresult));
		SetLastError(std::string("PacedIndexAdd() failed with error ") + DecodePacedIndexError(paceresult), ERRCODE_PACING_ERROR);
		return false;	
	}
	MOLOG(Log::L_INFO, CLOGFMT(FsIo, "[%s] PacedIndexAddEx() on pacing group 0x%08x successful, RAID1 %s for index"), 
		_subFiles[indexFileNo].strPacename.c_str(), (uint32)(pacingGroup), _enableRAID1ForIndex?"enabled":"disabled");

	return true;
}

void FilesetTarget::PacedIndexClose()
{
	bool bEndFlag = false;
	for(int i=0;!bEndFlag;i++)
	{
		// index
		if (i == 1)
			continue;

		if (i >= _nInputCount) //PacedIndexRemove vvx file last
		{
			i = 1;
			bEndFlag = true;
		}

		SubFile& subFile = _subFiles[i];
		if(!subFile.pacingIndexCtx)
			continue;

		uint32 paceresult = 0;				

/*
		//
		// Grab MD5 from the file
		//
		char md5[33] = "";
		if (i == 1)	//only index file have this
		{
			paceresult = PacedIndexGetPacedFileMD5(subFile.pacingIndexCtx, md5);
			if (paceresult)
			{
				MOLOG(Log::L_WARNING, CLOGFMT(FsIo, "[%s] PacedIndexGetPacedFileMD5() (%s) failed with error %s"), _strLogHint.c_str() 
					, subFile.strFilename.c_str(), DecodePacedIndexError(paceresult));
			}
			else
			{
				MOLOG(Log::L_INFO, CLOGFMT(FsIo, "[%s] PacedIndexGetPacedFileMD5() (%s) return [%s]"), _strLogHint.c_str(), 
					subFile.strFilename.c_str(), md5);
			}
		}
*/
		MOLOG(Log::L_INFO, CLOGFMT(FsIo, "[%s] to PacedIndexRemove() (%s) file context"),
			_strLogHint.c_str(), subFile.strFilename.c_str());
		paceresult = PacedIndexRemove(subFile.pacingIndexCtx);
		if (paceresult)
		{
			MOLOG(Log::L_WARNING, CLOGFMT(FsIo, "[%s] PacedIndexRemove() (%s) failed with error %s"), _strLogHint.c_str() 
				, subFile.strFilename.c_str(), DecodePacedIndexError(paceresult));
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(FsIo, "[%s] PacedIndexRemove (%s) file context successful"),
				_strLogHint.c_str(), subFile.strFilename.c_str());
		}
		subFile.pacingIndexCtx = NULL;				
	}
}
#endif

bool FilesetTarget::openSubFiles()
{
	for(int i=0;i<_nInputCount;i++)
	{
		SubFile& subFile = _subFiles[i];
		auto_ptr<FileIo>& pFileIo = subFile.pFileIo;

		int nFileFlag = decideOpenFileFlag(subFile.bIndexFile);

		pFileIo.reset(_pFileIoFac->create());
		if (!pFileIo->openfile(subFile.strFilename.c_str(), 
			FileIo::ACCESS_WRITE,
			(FileIo::ShareMode)(FileIo::SHARE_READ | FileIo::SHARE_WRITE),
			FileIo::WAY_CREATE_ALWAYS,
			(FileIo::FileAttrib)nFileFlag))
		{
			std::string strErr;
			int nErrorCode;
			pFileIo->getLastError(strErr, nErrorCode);

			SetLastError(strErr, nErrorCode);			
			MOLOG(Log::L_ERROR, CLOGFMT(FsIo, "[%s] Failed to create output file: %s with error: %s"),
				_strLogHint.c_str(), subFile.strFilename.c_str(), strErr.c_str());

			return false;
		}
		MOLOG(Log::L_INFO, CLOGFMT(FsIo, "open file [%s] successful"),
			subFile.strFilename.c_str());

		pFileIo->setOption();		///< this must be called, the fileio level would set the private options on the file
		pFileIo->setFileExtension(subFile.strFileExt.c_str());

		subFile.pThis = this;
	}

	return true;
}

void FilesetTarget::initSubFiles()
{
	std::map<std::string, int>::iterator iter;

	_subFiles[0].reservedBW = _dwBandwidth;
	_subFiles[0].strFilename = _strFilename + _mainFileExt;
	_subFiles[0].strPacename = _strFilename + std::string(".MPG");

	int i = 1;
	_subFiles[i].bIndexFile = true;
	if (_bIndexVVC)
		_subFiles[i].strFileExt = ".index";
	else if (_bTypeH264)
		_subFiles[i].strFileExt = ".VV2";
	else
		_subFiles[i].strFileExt = ".VVX";
	_subFiles[i].strFilename = _strFilename + _subFiles[i].strFileExt;
	_subFiles[i].strPacename =  _strFilename + std::string(".MPG") + _subFiles[i].strFileExt;
	_subFiles[i].reservedBW = _dwBandwidth/100;

	for (iter = _SpeedAndFileExt.begin(); iter != _SpeedAndFileExt.end(); iter++)
	{
		i++;
		_subFiles[i].strFileExt = (*iter).first;
		_subFiles[i].strFilename = _strFilename + (*iter).first;
		_subFiles[i].reservedBW = _dwBandwidth/(uint32)_trickspeed[(*iter).second];
		_subFiles[i].strPacename =  _strFilename + std::string(".MPG") + _subFiles[i].strFileExt;
	}

	MOLOG(Log::L_INFO, CLOGFMT(FsIo, "[%s] subFile setting is successful"), _strLogHint.c_str());
}

bool FilesetTarget::Init()
{
	_bCleanup = false;
	
	initSubFiles();

	if (!openSubFiles())
		return false;

#ifndef DISABLE_PACING
	if(_bPacing && !PacedIndexInit())
		return false;
#endif
	
	return true;
}
bool FilesetTarget::Start()
{
	if (!reserveBandwith())
		return false;

	return true;
}

void FilesetTarget::Stop()
{
	
}

void FilesetTarget::closeSubFiles()
{
	for(int i=0;i<_nInputCount;i++)
	{
		SubFile& subFile = _subFiles[i];

		//check saved samples
		if (subFile.samples.size())
		{
			MOLOG(Log::L_WARNING, CLOGFMT(FsIo, "[%s][%d] sample queue size is %d while close"), 
				_strLogHint.c_str(), i, subFile.samples.size());

			std::vector<MediaSample*>::iterator it=subFile.samples.begin();
			for(;it!=subFile.samples.end();it++)
			{
				GetGraph()->freeMediaSample(*it);
			}
			subFile.samples.clear();
		}

		//close file
		if (subFile.pFileIo.get())
		{
			subFile.pFileIo->closefile();
		}
	}
}

void FilesetTarget::processOutPut()
{
	if (!(GetGraph()->getProcessBytes()) || (GetGraph()->IsErrorOccurred()))
	{
		delOutput();
	}
}

void FilesetTarget::Close()
{
	if (_bCleanup)
		return;

	// release the reserved VStrm bandwidth, release it first to avoid possible session overlap
	releaseBandwith();
#ifndef DISABLE_PACING
	if(_bPacing)
	{
		PacedIndexClose();
	}
#endif

	closeSubFiles();
	
	// delete files if error happend
	processOutPut();

	_bCleanup = true;
	MOLOG(Log::L_INFO, CLOGFMT(FsIo, "[%s] closed"), _strLogHint.c_str());
}

void FilesetTarget::endOfStream()
{
	GetGraph()->Finish();
}

const char* FilesetTarget::GetName()
{
	return TARGET_TYPE_FILESET;
}

bool FilesetTarget::writeData(SubFile& subFile, char* pBuf, int dwBufLen)
{
	if (_bPacing)
	{
#ifndef DISABLE_PACING
		uint32 paceresult;

		try
		{
			if(PacedIndexWrite(subFile.pacingIndexCtx, dwBufLen, (const char*)pBuf, (unsigned long*)&paceresult) < 0)
			{
				std::string errstr = DecodePacedIndexError(paceresult);
				//			the error information has been set on vswrite failure
				//			SetLastError(std::string("PacedIndexWrite failed to write file ") + subFile.strFilename + " with error:" + errstr);
				MOLOG(Log::L_WARNING, CLOGFMT(FsIo, "[%s] PacedIndexWrite() file %s with error: (%s)"), 
					_strLogHint.c_str(), subFile.strFilename.c_str(), errstr.c_str());

				return false;
			}
		}
		catch(...)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(FsIo, "[%s] PacedIndexWrite() file [%s] len[%d] caught unknown exception"), 
				_strLogHint.c_str(), subFile.strFilename.c_str(), dwBufLen);

			SetLastError(std::string("PacedIndexWrite() caught unknown exception"), ERRCODE_PACING_UNKNOWN_EXCEPTION);		
			return false;
		}		
#endif
		return true;
	}
	else
	{
		return writeFile(subFile.pFileIo, pBuf, dwBufLen);
	}
}

bool FilesetTarget::writeSubFileNoIndex(SubFile& subFile, MediaSample* pSample)
{
	int64 llOffset;
	uint32 offlow,offhigh;
	offlow = pSample->getOffset(&offhigh);
	llOffset = offhigh;
	llOffset <<= 32;
	llOffset += offlow;
	
	char* pointer = (char*)pSample->getPointer();
	unsigned int dwActualLen = pSample->getDataLength();

	if (subFile.llProcOffset != llOffset)
	{
		if (!subFile.pFileIo->seekfile(llOffset, FileIo::POS_BEGIN))
		{
			std::string strErr;
			int nErrorCode;
			subFile.pFileIo->getLastError(strErr, nErrorCode);

			SetLastError(std::string("failed to seek file ") + subFile.strFilename + " with error:" + strErr, nErrorCode);
			MOLOG(Log::L_ERROR, CLOGFMT(FsIo, "[%s] Seek() to offset [%lld] failed with error: %s)"), _strLogHint.c_str(),
				llOffset, strErr.c_str());

			return false;
		}

		subFile.llProcOffset = llOffset;
	}

	if (!writeData(subFile, pointer, dwActualLen))
		return false;

	subFile.llProcOffset += dwActualLen;	

	GetGraph()->freeMediaSample(pSample);
	return true;
}

bool FilesetTarget::writeSubFileIndex(SubFile& subFile, MediaSample* pSample)
{
	unsigned int uOffsetLow, uOffsetHigh;
	uOffsetLow = pSample->getOffset(&uOffsetHigh);
	char* pointer = (char*)pSample->getPointer();
	unsigned int dwActualLen = pSample->getDataLength();

	// here we assume the index no seek	
	// the index could not be over 4G
	if (!_bPacing && subFile.llProcOffset != uOffsetLow)
	{
		//do seek
		MOLOG(Log::L_DEBUG, CLOGFMT(FsIo, "[%s] file seeking to offset[%d]"), 
			subFile.strFilename.c_str(), uOffsetLow);

		if (!subFile.pFileIo->seekfile(uOffsetLow, FileIo::POS_BEGIN))
		{
			std::string strErr;
			int nErrorCode;
			subFile.pFileIo->getLastError(strErr, nErrorCode);

			SetLastError(std::string("failed to seek file ") + subFile.strFilename + " with error:" + strErr, nErrorCode);
			MOLOG(Log::L_ERROR, CLOGFMT(FsIo, "[%s] Seek() to offset [%d] failed with error: %s)"), _strLogHint.c_str(),
				uOffsetLow, strErr.c_str());

			return false;
		}
	}

	if (!writeData(subFile, pointer, dwActualLen))
		return false;

	subFile.llProcOffset = uOffsetLow + dwActualLen;

	GetGraph()->freeMediaSample(pSample);
	return true;
}
// add this function for CPH_RTI retry catapture stream
// TicketId: 7384, bugid:13290
bool FilesetTarget::resetMainfile()
{
	SubFile& subFile = _subFiles[0];
	auto_ptr<FileIo>& pFileIo = subFile.pFileIo;

	//close file
	if (!subFile.pFileIo.get())
	{
		return false;
	}
	

	// 	if(!subFile.pFileIo->seek(0, ZQTianShan::ContentProvision::FileIo::POS_BEGIN))
	// 		return false;

    subFile.pFileIo->closefile();
	int nFileFlag = decideOpenFileFlag(subFile.bIndexFile);

	pFileIo.reset(_pFileIoFac->create());
	if (!pFileIo->openfile(subFile.strFilename.c_str(), 
		FileIo::ACCESS_WRITE,
		(FileIo::ShareMode)(FileIo::SHARE_READ | FileIo::SHARE_WRITE),
		FileIo::WAY_CREATE_ALWAYS,
		(FileIo::FileAttrib)nFileFlag))
	{
		std::string strErr;
		int nErrorCode;
		pFileIo->getLastError(strErr, nErrorCode);

		SetLastError(strErr, nErrorCode);			
		MOLOG(Log::L_ERROR, CLOGFMT(FsIo, "[%s] Failed to create output file: %s with error: %s"),
			_strLogHint.c_str(), subFile.strFilename.c_str(), strErr.c_str());

		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FsIo, "open file [%s] successful"),
		subFile.strFilename.c_str());

	pFileIo->setOption();		///< this must be called, the fileio level would set the private options on the file
	pFileIo->setFileExtension(subFile.strFileExt.c_str());

	subFile.pThis = this;

	return true;
}
bool FilesetTarget::Receive(MediaSample* pSample, int nInputIndex)
{
	if (nInputIndex < 0 || nInputIndex >_nInputCount-1)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FsIo, "[%s] Receive() invalid input index: %d"), _strLogHint.c_str(), nInputIndex);
		SetLastError("Receive(): invalid input index");
		return false; 
	}

	// make sure no error happen
	if (GetGraph()->IsErrorOccurred())
	{
		return false;
	}

	//
	SubFile& subFile = _subFiles[nInputIndex];
	if (!subFile.bIndexFile)
	{
		if (!writeSubFileNoIndex(subFile, pSample))
			return false;
	}
	else
	{
		if (!writeSubFileIndex(subFile, pSample))
			return false;
	}

	return true;
}

bool FilesetTarget::deleteFile(const char* szFile)
{
	if (!_pFileIoFac->deleteFile(szFile))
		return false;

	return true;
}

void FilesetTarget::delOutput()
{
	if (_DeleteTargetOnFail == 1)
	{
		for(int i=0;i<_nInputCount;i++)
		{
			SubFile& subFile = _subFiles[i];

			if(subFile.strFilename.empty())
				continue;

			bool bret = deleteFile(subFile.strFilename.c_str());
			MOLOG(Log::L_INFO, CLOGFMT(FsIo, "delete file [%s] %s"),
				subFile.strFilename.c_str(), bret?"success":"failure");
		}
	}
	else if (_DeleteTargetOnFail == 2)
	{
		for(int i=0;i<_nInputCount;i++)
		{
			SubFile& subFile = _subFiles[i];

			if(subFile.strFilename.empty())
				continue;

			std::string newName = subFile.strFilename + std::string("_bak");

			bool bret = _pFileIoFac->moveFile(subFile.strFilename.c_str(),newName.c_str());
			MOLOG(Log::L_INFO, CLOGFMT(FsIo, "rename file [%s] to [%s] %s"),
				subFile.strFilename.c_str(), newName.c_str(), bret?"success":"failure");			
		}
	}
	else
		return;
}

void FilesetTarget::setFilename(const char* szFile)
{
	_strFilename = szFile;
}

FilesetTarget::~FilesetTarget()
{
	Close();
}

void FilesetTarget::setBandwidth(unsigned int uBandwidthBps)
{
	_dwBandwidth = uBandwidthBps;
	MOLOG(Log::L_INFO, CLOGFMT(FsIo, "[%s] set Bandwidth=%d"), _strLogHint.c_str(), uBandwidthBps);
}



void FilesetTarget::pacingAppReportOffsets(const void * const pCbParam, const int64 offset1, const int64 offset2)
{
/*
	UNREFERENCED_PARAMETER(pCbParam);
	UNREFERENCED_PARAMETER(offset1);
	UNREFERENCED_PARAMETER(offset2);
*/
}

#ifndef DISABLE_PACING
#define XX(a,b) {a, b}
const char* FilesetTarget::DecodePacedIndexError(const unsigned long err)
{
	const char *errString = "unknown error";
	
	static struct
	{
		unsigned long code;
		char *str;
	} 
	errors[] = 
	{
		PACED_INDEX_ERROR_TABLE
	};
	
	for (int i = 0; i < (sizeof(errors) / sizeof(errors[0])); i++)
	{
		if (err == errors[i].code)
		{
			errString = errors[i].str;
		}
	}
	
	return errString;
}

void FilesetTarget::pacingAppLogCbk(const char * const pMsg)
{
	char				buf[1024];
	int					len;
	unsigned long		written = 0;
	
	if (_bPacingTrace)
	{
		//
		// If a msg has arrived without a CRLF, give it one now
		//
		len = strlen(pMsg);
		
		if (len > 1024 - 2)
		{
			len = 1024 - 2;
		}
		
		memcpy(buf, pMsg, len);
		buf[len] = 0;
		
		//output	
		glog(Log::L_DEBUG, buf);
	}
}

int FilesetTarget::pacingAppWrite(const void * const pCbParam, const int len, const char buf[])
{
	SubFile* subfile = (SubFile*) pCbParam;
	FilesetTarget* pThis = subfile->pThis;

	return pThis->pacingWrite(*subfile, len, buf);
}

int FilesetTarget::pacingSeek(SubFile& subFile, const int64 offset)
{
	auto_ptr<FileIo>& pFileIo = subFile.pFileIo;

	if (GetGraph()->IsErrorOccurred())
	{
		MOLOG(Log::L_INFO, CLOGFMT(FsIo, "[%s] session failure, pacingSeek() to offset %lld skipped"), subFile.strFilename.c_str(), offset);
		return 0;
	}

	if (_traceIndexWrite)
	{
		MOLOG(Log::L_DEBUG, CLOGFMT(FsIo, "[%s] seek to offset [%lld] bytes"), subFile.strFilename.c_str(), offset);
	}

	if (!pFileIo->seekfile(offset, FileIo::POS_BEGIN))
	{
		std::string strErr;
		int nErrorCode;
		pFileIo->getLastError(strErr, nErrorCode);
		
		SetLastError(std::string("failed to seek file ") + subFile.strFilename + " with error:" + strErr, nErrorCode);
		MOLOG(Log::L_ERROR, CLOGFMT(FsIo, "[%s] Seek() to offset [%lld] failed with error: %s)"), _strLogHint.c_str(),
			offset, strErr.c_str());

		return -1;
	}

	return 0;
}

int FilesetTarget::pacingWrite(SubFile& subFile, const int len, const char buf[])
{
	if (_traceIndexWrite && subFile.bIndexFile)
	{
		MOLOG(Log::L_DEBUG, CLOGFMT(FsIo, "[%s] write %d bytes"), subFile.strFilename.c_str(), len);
	}

	if (!writeFile(subFile.pFileIo, (char*)buf, len))
	{
		return -1;
	}

	return len;
}

int FilesetTarget::pacingSetEOF(SubFile& subFile, const int64 offset)
{
	auto_ptr<FileIo>& pFileIo = subFile.pFileIo;

	MOLOG(Log::L_DEBUG, CLOGFMT(FsIo, "[%s] pacingAppSetEOF() offset %lld%"), subFile.strFilename.c_str(), offset);

	if (GetGraph()->IsErrorOccurred())
	{
		MOLOG(Log::L_INFO, CLOGFMT(FsIo, "[%s] session failure, pacingAppSetEOF() to offset %lld skipped"), subFile.strFilename.c_str(), offset);
		return 0;
	}

	if (!pFileIo->setEndOfFile(offset))
	{
		std::string strErr;
		int nErrorCode;
		pFileIo->getLastError(strErr, nErrorCode);

		SetLastError(std::string("failed to setEOF for file ") + subFile.strFilename + " with error:" + strErr, nErrorCode);
		MOLOG(Log::L_ERROR, CLOGFMT(FsIo, "[%s] setEOF() to offset [%lld] failed with error: %s)"), _strLogHint.c_str(),
			offset, strErr.c_str());

		return -1;
	}

	return 0;
}

int FilesetTarget::pacingAppSeek(const void * const pCbParam, const int64 offset)
{
	SubFile* subfile = (SubFile*) pCbParam;
	FilesetTarget* pThis = subfile->pThis;

	return pThis->pacingSeek(*subfile, offset);
}

int FilesetTarget::pacingAppSetEOF(const void * const pCbParam, const int64 offset)
{
	SubFile* subfile = (SubFile*) pCbParam;
	FilesetTarget* pThis = subfile->pThis;

	return pThis->pacingSetEOF(*subfile, offset);
}
#endif

void FilesetTarget::enableMD5(bool bEnable)
{
	_bEnableMD5 = bEnable;
}

void FilesetTarget::getMD5(std::string& strMD5)
{
	strMD5 = _md5ChecksumUtil.lastChecksum();
}

uint64 FilesetTarget::getFileSize(const char* szFile)
{
	return _pFileIoFac->getFileSize(szFile);
}

uint64 FilesetTarget::getSupportFileSize()
{	
	uint64 supportFilesize = 0;

	for (int i = 2; i < _nInputCount; i++)
	{
		uint64 lSize = getFileSize(_subFiles[i].strFilename.c_str());
		
		MOLOG(Log::L_DEBUG, CLOGFMT(FsIo, "File [%s] size-[%lld]"), _subFiles[i].strFilename.c_str(), lSize);
		supportFilesize += lSize;
	}
	
	MOLOG(Log::L_INFO, CLOGFMT(FsIo, "[%s] supportFilesize-[%lld]"),_strLogHint.c_str(),supportFilesize);
	return supportFilesize;
}

bool FilesetTarget::writeFile(std::auto_ptr<FileIo>& pFileIo, char* pBuf, int dwBufLen)
{
	uint64 ut1 = SYS::getTickCount();
	unsigned int nWrite;
	if (!pFileIo->writefile(pBuf, dwBufLen, nWrite))
		return false;

	if (pFileIo.get() == _subFiles[0].pFileIo.get())
	{
		// only main file need the update the process bytes
		IncProcvBytes(dwBufLen);

		if (_bEnableMD5)
		{
			//main file
			_md5ChecksumUtil.checksum(pBuf, dwBufLen);
		}

	}

	uint uDef = SYS::getTickCount() - ut1;
	if (uDef >= _nWriteLatencyWarningMs)
	{
		//print warning
		MOLOG(Log::L_WARNING, CLOGFMT(FsIo, "[%s] write %d bytes spent %d ms"), _strLogHint.c_str(), dwBufLen, uDef);
	}

	return true;
}

/// set cache path
void FilesetTarget::setCacheDirectory(const std::string& path) 
{ 
	_cachePath = path; 
	if(_cachePath == "")
		return;

	int pos = _cachePath.length() - 1;
	if(_cachePath[pos] != '\\' && _cachePath[pos] != '/' )
	{
		_cachePath = _cachePath + FNSEPS;
	}

	MOLOG(Log::L_INFO, CLOGFMT(FsIo, "[%s] set CacheDirectory=%s"), _strLogHint.c_str(), _cachePath.c_str());
}

void FilesetTarget::setTrickSpeed( std::list<float>& trickspeed )
{
	int index = 0;
	for(std::list<float>::iterator iter = trickspeed.begin();iter != trickspeed.end();iter++,index++)
	{
		_trickspeed[index] = (*iter);
	}
}

bool FilesetTarget::getOutputFiles(std::vector<std::string>& filelists, int& outputfilecount, int& indexType)
{
    if (_bTypeH264)
		indexType = CTF_INDEX_TYPE_VV2;
	else
		indexType = CTF_INDEX_TYPE_VVX;

	for(int i = 1; i<_nInputCount; i++)
	{
		filelists.push_back(_subFiles[i].strFilename);
	}
	outputfilecount = _nInputCount -1;
	return true;
}

void FilesetTarget::setWriteLatencyWarning( int nWarningMs )
{
	_nWriteLatencyWarningMs = nWarningMs;
}
}}

