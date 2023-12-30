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
#include "AquaFileSetTarget.h"
#include "ErrorCode.h"
#include "CTFLib.h"
#include "SystemUtils.h"

#define FsIo			"AquaFilesetTarget"
#define MOLOG (*_pLog)

#define tempIndexSuffix    ".tmp"

using namespace ZQ::common;
using namespace std;

namespace ZQTianShan {
	namespace ContentProvision {

		bool AquaFilesetTarget::_bPacingTrace = false;


		AquaFilesetTarget::AquaFilesetTarget(FileIoFactory* pFileIoFac):_bTypeH264(false)
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
			_enableCacheForIndex = true;
			_DeleteTargetOnFail = 1;
			_bIndexVVC = false;
			_trickspeed[0] = 7.5;
			_pPacedIndexFac = NULL;
			_pPacedIndexObj = NULL;
			_mainFileExt = "";
			_nWriteLatencyWarningMs = 100;
			_bIngoreMainfile = false;
			_bAquaIndexTmpfile = false;
			_strIndexFilename = "";
		}

		void AquaFilesetTarget::InitPins()
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
			setPacedIndexInfo();
		}

		int AquaFilesetTarget::decideOpenFileFlag(bool bIndexFile)
		{
			if (bIndexFile)
			{
				return (_enableCacheForIndex)?FileIo::ATTRIB_INDEXFILE:0;		//disable FILE_FLAG_CACHED for index file, as Rick asked, 10/28 2008
			}
			else
			{
				return FileIo::ATTRIB_INDEXFILE;
			}
		}

		void AquaFilesetTarget::fileIoFailure(std::auto_ptr<FileIo>& pFileIo, const char* szLogHeader, bool bSetLastError)
		{
			std::string strErr;
			int nErrorCode;
			pFileIo->getLastError(strErr, nErrorCode);

			if (bSetLastError)
				SetLastError(strErr, nErrorCode);			

			MOLOG(Log::L_ERROR, CLOGFMT(AquaFilesetTarget, "[%s] Failed to %s with error: %s"),
				_strLogHint.c_str(), szLogHeader, strErr.c_str());
		}

		bool AquaFilesetTarget::reserveBandwith()
		{
			return true;
		}

		void AquaFilesetTarget::releaseBandwith()
		{
		}

		bool AquaFilesetTarget::openSubFiles()
		{
			for(int i=0;i<_nInputCount;i++)
			{
				if(i == 0 && _bIngoreMainfile)	//Aqua source skip the mainfile
				{
					MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "[%s] ingore main file %s"),_strLogHint.c_str(), _subFiles[i].strFilename.c_str());
					continue;
				}

				SubFile& subFile = _subFiles[i];
				auto_ptr<FileIo>& pFileIo = subFile.pFileIo;

				int nFileFlag = decideOpenFileFlag(subFile.bIndexFile);

				pFileIo.reset(_pFileIoFac->create());

				if(i == 0)
					pFileIo->setMainFileFalg();
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
					MOLOG(Log::L_ERROR, CLOGFMT(AquaFilesetTarget, "[%s] Failed to create output file: %s with error: %s"),
						_strLogHint.c_str(), subFile.strFilename.c_str(), strErr.c_str());

					return false;
				}
				MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "open file [%s] successful"),
					subFile.strFilename.c_str());

				pFileIo->setOption();		///< this must be called, the fileio level would set the private options on the file
				pFileIo->setFileExtension(subFile.strFileExt.c_str());
	
				subFile.pThis = this;
			}

			return true;
		}

		void AquaFilesetTarget::initSubFiles()
		{
			std::map<std::string, int>::iterator iter;

			_subFiles[0].reservedBW = _dwBandwidth;
			if (_bIndexVVC)
				if(_mainFileExt.size() < 1)
					_subFiles[0].strFilename = _strFilename + std::string(".0X0000");
				else
					_subFiles[0].strFilename = _strFilename + _mainFileExt;
			else
				_subFiles[0].strFilename = _strFilename;

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

			_strIndexFilename = _subFiles[i].strFilename ;

			if(_bAquaIndexTmpfile)
			{
				_subFiles[i].strFilename += tempIndexSuffix;
				MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "[%s] original IndexFileName[%s], Aqua Temp IndexFilename[%s]"),_strLogHint.c_str(), _strIndexFilename.c_str(), _subFiles[i].strFilename.c_str());
			}

			_subFiles[i].strPacename =  _strFilename + std::string(".MPG") + _subFiles[i].strFileExt;
			_subFiles[i].reservedBW = _dwBandwidth/20;

			for (iter = _SpeedAndFileExt.begin(); iter != _SpeedAndFileExt.end(); iter++)
			{
				i++;
				_subFiles[i].strFileExt = (*iter).first;
				_subFiles[i].strFilename = _strFilename + (*iter).first;
				_subFiles[i].reservedBW = _dwBandwidth/(uint32)_trickspeed[(*iter).second];
				_subFiles[i].strPacename =  _strFilename + std::string(".MPG") + _subFiles[i].strFileExt;
			}

			MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "[%s] subFile setting is successful"), _strLogHint.c_str());
		}

		bool AquaFilesetTarget::Init()
		{
			_bCleanup = false;

			initSubFiles();

			if (!openSubFiles())
				return false;

			if (_bPacing && !_bIndexVVC)
				_bPacing = false;

			return true;
		}
		bool AquaFilesetTarget::Start()
		{
			if (!reserveBandwith())
				return false;

			return true;
		}

		void AquaFilesetTarget::Stop()
		{

		}

		void AquaFilesetTarget::closeSubFiles()
		{
			if (_pPacedIndexObj)
			{
				_pPacedIndexObj->close();
				_pPacedIndexObj->release();
				_pPacedIndexObj = NULL;
			}

			for(int i=0;i<_nInputCount;i++)
			{
				SubFile& subFile = _subFiles[i];

				//check saved samples
				if (subFile.samples.size())
				{
					MOLOG(Log::L_WARNING, CLOGFMT(AquaFilesetTarget, "[%s][%d] sample queue size is %d while close"), 
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

		void AquaFilesetTarget::processOutPut()
		{
			MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "[%s]processOutPut() ProcessBytes[%lld] IsErrorOccurred[%d]"), _strLogHint.c_str(), GetGraph()->getProcessBytes(), GetGraph()->IsErrorOccurred());

			if (!(GetGraph()->getProcessBytes()) || (GetGraph()->IsErrorOccurred()))
			{
				delOutput();
			}
		}

		void AquaFilesetTarget::Close()
		{
			if (_bCleanup)
				return;

			// release the reserved VStrm bandwidth, release it first to avoid possible session overlap
			releaseBandwith();
			closeSubFiles();

			// delete files if error happend
			processOutPut();

			_bCleanup = true;
			MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "[%s] closed"), _strLogHint.c_str());
		}

		void AquaFilesetTarget::endOfStream()
		{
			GetGraph()->Finish();
		}

		const char* AquaFilesetTarget::GetName()
		{
			return TARGET_TYPE_AQUAFILESET;
		}

		bool AquaFilesetTarget::writeData(SubFile& subFile, char* pBuf, int dwBufLen)
		{
			if (!_bPacing)
			{
				return writeFile(subFile.pFileIo, pBuf, dwBufLen);
			}

			// pacing
			if (&subFile == &_subFiles[1])
			{
				return _pPacedIndexObj->writeIndex(pBuf,dwBufLen,subFile.llProcOffset);
			}
			else
			{
				if (!writeFile(subFile.pFileIo, pBuf, dwBufLen))
					return false;

				if (!_pPacedIndexObj->subfileWritten(subFile.strFileExt.c_str(),dwBufLen,subFile.llProcOffset))
					return false;

				return true;
			}
		}

		bool AquaFilesetTarget::writeSubFile(SubFile& subFile, MediaSample* pSample)
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
				if (!(subFile.bIndexFile && _bPacing))
				{
					if (!subFile.pFileIo->seekfile(llOffset, FileIo::POS_BEGIN))
					{
						std::string strErr;
						int nErrorCode;
						subFile.pFileIo->getLastError(strErr, nErrorCode);

						SetLastError(std::string("failed to seek file ") + subFile.strFilename + " with error:" + strErr, nErrorCode);
						MOLOG(Log::L_ERROR, CLOGFMT(AquaFilesetTarget, "[%s] Seek() to offset [%lld] failed with error: %s)"), _strLogHint.c_str(),
							llOffset, strErr.c_str());

						return false;
					}
				}

				subFile.llProcOffset = llOffset;
			}

			if (_bPacing)
			{
				if (subFile.bIndexFile)
				{
					_pPacedIndexObj->writeIndex(pointer,dwActualLen, llOffset);
				}
				else
				{
					if (!writeFile(subFile.pFileIo, pointer, dwActualLen))
						return false;

					if (!_pPacedIndexObj->subfileWritten(subFile.strFileExt.c_str(),dwActualLen, llOffset))
						return false;
				}				
			}
			else
			{
				if (!writeFile(subFile.pFileIo, pointer, dwActualLen))
					return false;
			}

			subFile.llProcOffset += dwActualLen;	

			GetGraph()->freeMediaSample(pSample);

			return true;
		}

		bool AquaFilesetTarget::writeSubFileNoIndex(SubFile& subFile, MediaSample* pSample)
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
					MOLOG(Log::L_ERROR, CLOGFMT(AquaFilesetTarget, "[%s] Seek() to offset [%lld] failed with error: %s)"), _strLogHint.c_str(),
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

		bool AquaFilesetTarget::writeSubFileIndex(SubFile& subFile, MediaSample* pSample)
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
				MOLOG(Log::L_DEBUG, CLOGFMT(AquaFilesetTarget, "[%s] file seeking to offset[%d]"), 
					subFile.strFilename.c_str(), uOffsetLow);

				if (!subFile.pFileIo->seekfile(uOffsetLow, FileIo::POS_BEGIN))
				{
					std::string strErr;
					int nErrorCode;
					subFile.pFileIo->getLastError(strErr, nErrorCode);

					SetLastError(std::string("failed to seek file ") + subFile.strFilename + " with error:" + strErr, nErrorCode);
					MOLOG(Log::L_ERROR, CLOGFMT(AquaFilesetTarget, "[%s] Seek() to offset [%d] failed with error: %s)"), _strLogHint.c_str(),
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
		bool AquaFilesetTarget::resetMainfile()
		{
			SubFile& subFile = _subFiles[0];
			auto_ptr<FileIo>& pFileIo = subFile.pFileIo;

			//close file
			if (!subFile.pFileIo.get())
			{
				return false;
			}


			// 	if(!subFile.pFileIo->seekfile(0, ZQTianShan::ContentProvision::FileIo::POS_BEGIN))
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
				MOLOG(Log::L_ERROR, CLOGFMT(AquaFilesetTarget, "[%s] Failed to create output file: %s with error: %s"),
					_strLogHint.c_str(), subFile.strFilename.c_str(), strErr.c_str());

				return false;
			}
			MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "open file [%s] successful"),
				subFile.strFilename.c_str());

			pFileIo->setOption();		///< this must be called, the fileio level would set the private options on the file
			pFileIo->setFileExtension(subFile.strFileExt.c_str());

			subFile.pThis = this;

			return true;
		}
		bool AquaFilesetTarget::Receive(MediaSample* pSample, int nInputIndex)
		{
			if (nInputIndex < 0 || nInputIndex >_nInputCount-1)
			{
				MOLOG(Log::L_ERROR, CLOGFMT(AquaFilesetTarget, "[%s] Receive() invalid input index: %d"), _strLogHint.c_str(), nInputIndex);
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
			if(_bIngoreMainfile && 0 == nInputIndex)//skip main file
			{
				MOLOG(Log::L_DEBUG, CLOGFMT(AquaFilesetTarget, "Receive() input index: %d and IngoreMainfile is [%d] will skip main file"),nInputIndex, _bIngoreMainfile);
				unsigned int dwActualLen = pSample->getDataLength();
				subFile.llProcOffset += dwActualLen;	
				IncProcvBytes(dwActualLen);
				GetGraph()->freeMediaSample(pSample);
				return true;
			}
	
			if (!writeSubFile(subFile, pSample))
				return false;

/*			if (!subFile.bIndexFile)
			{
				if (!writeSubFileNoIndex(subFile, pSample))
					return false;
			}
			else
			{
				if (!writeSubFileIndex(subFile, pSample))
					return false;
			}
*/
			return true;
		}

		bool AquaFilesetTarget::renameBackIndexTmpName()
		{
			bool bret = true;
			if(_bAquaIndexTmpfile)
			{
				bool bret = _pFileIoFac->moveFile(_subFiles[1].strFilename.c_str(),_strIndexFilename.c_str());
				MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "rename file [%s] to [%s] %s"), _subFiles[1].strFilename.c_str(), _strIndexFilename.c_str(), bret?"success":"failure");
			}
			return bret;
		}
		bool AquaFilesetTarget::deleteFile(const char* szFile)
		{
			if (!_pFileIoFac->deleteFile(szFile))
				return false;

			return true;
		}

		void AquaFilesetTarget::delOutput()
		{
			if (_DeleteTargetOnFail == 1)
			{
				for(int i=0;i<_nInputCount;i++)
				{
					if(_bIngoreMainfile && i == 0)
						continue;
					SubFile& subFile = _subFiles[i];

					if(subFile.strFilename.empty())
						continue;

					bool bret = deleteFile(subFile.strFilename.c_str());
					MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "delete file [%s] %s"),
						subFile.strFilename.c_str(), bret?"success":"failure");
				}
			}
			else if (_DeleteTargetOnFail == 2)
			{
				for(int i=0;i<_nInputCount;i++)
				{
					if(_bIngoreMainfile && i == 0)
						continue;

					SubFile& subFile = _subFiles[i];

					if(subFile.strFilename.empty())
						continue;
						
					std::string newName = subFile.strFilename + std::string("_bak");

					bool bret = _pFileIoFac->moveFile(subFile.strFilename.c_str(),newName.c_str());
					MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "rename file [%s] to [%s] %s"),
						subFile.strFilename.c_str(), newName.c_str(), bret?"success":"failure");
				}
			}
			else
				return;
		}

		void AquaFilesetTarget::setFilename(const char* szFile)
		{
			_strFilename = szFile;
		}

		AquaFilesetTarget::~AquaFilesetTarget()
		{
			Close();
		}

		void AquaFilesetTarget::setBandwidth(unsigned int uBandwidthBps)
		{
			_dwBandwidth = uBandwidthBps;
			MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "[%s] set Bandwidth=%d"), _strLogHint.c_str(), uBandwidthBps);
		}

		bool AquaFilesetTarget::write(const char* buf, int len)
		{
			if (!writeFile(this->_subFiles[1].pFileIo, (char*)buf, len))
			{
				return false;
			}
			return true;
		}
		bool AquaFilesetTarget::seek(int offset)
		{
			auto_ptr<FileIo>& pFileIo = this->_subFiles[1].pFileIo;

			if (!pFileIo->seekfile(offset, FileIo::POS_BEGIN))
			{
				std::string strErr;
				int nErrorCode;
				pFileIo->getLastError(strErr, nErrorCode);

				SetLastError(std::string("failed to seek file ") + this->_subFiles[1].strFilename + " with error:" + strErr, nErrorCode);
				MOLOG(Log::L_ERROR, CLOGFMT(AquaFilesetTarget, "[%s] Seek() to offset [%lld] failed with error: %s)"), _strLogHint.c_str(),
					offset, strErr.c_str());

				return false;
			}

			return true;
		}
		void AquaFilesetTarget::enableMD5(bool bEnable)
		{
			_bEnableMD5 = bEnable;
		}

		void AquaFilesetTarget::getMD5(std::string& strMD5)
		{
			strMD5 = _md5ChecksumUtil.lastChecksum();
		}

		uint64 AquaFilesetTarget::getFileSize(const char* szFile)
		{
			return _pFileIoFac->getFileSize(szFile);
		}

		uint64 AquaFilesetTarget::getSupportFileSize()
		{	
			uint64 supportFilesize = 0;

			for (int i = 2; i < _nInputCount; i++)
			{
				uint64 lSize = getFileSize(_subFiles[i].strFilename.c_str());

				MOLOG(Log::L_DEBUG, CLOGFMT(AquaFilesetTarget, "File [%s] size-[%lld]"), _subFiles[i].strFilename.c_str(), lSize);
				supportFilesize += lSize;
			}

			MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "[%s] supportFilesize-[%lld]"),_strLogHint.c_str(),supportFilesize);
			return supportFilesize;
		}

		bool AquaFilesetTarget::writeFile(std::auto_ptr<FileIo>& pFileIo, char* pBuf, int dwBufLen)
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
					_md5ChecksumUtil.checksum(pBuf, nWrite);
				}
			}

			uint uDef = SYS::getTickCount() - ut1;
			if (uDef >= _nWriteLatencyWarningMs)
			{
				//print warning
				MOLOG(Log::L_WARNING, CLOGFMT(AquaFilesetTarget, "[%s] write %d bytes spent %d ms"), _strLogHint.c_str(), dwBufLen, uDef);
			}
				
			return true;
		}

		void AquaFilesetTarget::enableCacheForIndex( bool bEnable /*= false*/ )
		{
			_enableCacheForIndex = bEnable;
		}

		void AquaFilesetTarget::setTrickSpeed( std::list<float>& trickspeed)
		{
			int index = 0;
			for(std::list<float>::iterator iter = trickspeed.begin();iter != trickspeed.end();iter++,index++)
			{
				_trickspeed[index] = (*iter);
			}
		}

		bool AquaFilesetTarget::setPacedIndexInfo()
		{
			if (!_pPacedIndexFac)
			{
				MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "[%s] pPacedIndexFac is NULL"), _strLogHint.c_str());
				return false;
			}

			std::string strIndexType;
			if (_bIndexVVC)
				strIndexType = "vvc";
			else if (_bTypeH264)
				strIndexType = "VV2";
			else
				strIndexType = "VVX";

			_pPacedIndexObj = _pPacedIndexFac->create(strIndexType.c_str());
			if (!_pPacedIndexObj)
			{
				MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "[%s] failed to create PacedIndex Object with type[%s]"), _strLogHint.c_str(),strIndexType.c_str());
				return false;
			}
            
			_pPacedIndexObj->setLogHint(_strLogHint.c_str());
			_pPacedIndexObj->setIndexWriter(this);
			return true;
		}

		bool AquaFilesetTarget::getOutputFiles(std::vector<std::string>& filelists, int& outputfilecount, int& indexType)
		{
			if(_bIndexVVC)
				indexType = CTF_INDEX_TYPE_VVC;
			else
				if (_bTypeH264)
					indexType = CTF_INDEX_TYPE_VV2;
				else
					indexType = CTF_INDEX_TYPE_VVX;

			for(int i = 1; i<_nInputCount; i++)
			{
				filelists.push_back(_subFiles[i].strFilename);
			}
			outputfilecount = _nInputCount - 1;
			return true;
		}
		
		void AquaFilesetTarget::setWriteLatencyWarning( int nWarningMs )
{
	_nWriteLatencyWarningMs = nWarningMs;
}

		RTIRawTarget::RTIRawTarget(FileIoFactory* pFileIoFac):AquaFilesetTarget(pFileIoFac)
		{
			_subFile.reservedBW = 0;
			_subFile.pacingIndexCtx = 0;	//context of pacing
			_subFile.llProcOffset = 0;	
			_nWriteLatencyWarningMs = 1000;
		}

		RTIRawTarget::~RTIRawTarget(void)
		{
		}

		bool RTIRawTarget::openSubFiles()
		{
			auto_ptr<FileIo>& pFileIo = _subFile.pFileIo;
			pFileIo.reset(_pFileIoFac->create());
			if (!pFileIo->openfile(_subFile.strFilename.c_str(), 
				FileIo::ACCESS_WRITE,
				(FileIo::ShareMode)(FileIo::SHARE_READ | FileIo::SHARE_WRITE),
				FileIo::WAY_CREATE_ALWAYS,
				FileIo::ATTRIB_NONE))
			{
				std::string strErr;
				int nErrorCode;
				pFileIo->getLastError(strErr, nErrorCode);
				SetLastError(strErr, nErrorCode);			
				MOLOG(Log::L_ERROR, CLOGFMT(AquaFilesetTarget, "[%s] Failed to create output file: %s with error: %s"),
					_strLogHint.c_str(), _subFile.strFilename.c_str(), strErr.c_str());
				return false;
			}
			MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "open file [%s] successful"),
				_subFile.strFilename.c_str());
			pFileIo->setOption();		///< this must be called, the fileio level would set the private options on the file
			pFileIo->setFileExtension(_subFile.strFileExt.c_str());
			_subFile.pThis = this;
			return true;
		}
		void RTIRawTarget::initSubFiles()
		{
			std::map<std::string, int>::iterator iter;
			_subFile.reservedBW = _dwBandwidth;
			_subFile.strFilename = _mainFileName;
			MOLOG(Log::L_INFO, CLOGFMT(RTIRawTarget, "[%s] subFile setting is successful"), _strLogHint.c_str());
		}
		bool RTIRawTarget::Init()
		{
			initSubFiles();
			if (!openSubFiles())
				return false;
			return true;
		}
		void RTIRawTarget::closeSubFiles()
		{
			//check saved samples
			if (_subFile.samples.size())
			{
				MOLOG(Log::L_WARNING, CLOGFMT(RTIRawTarget, "[%s]sample queue size is %d while close"), 
					_strLogHint.c_str(),  _subFile.samples.size());
				std::vector<MediaSample*>::iterator it=_subFile.samples.begin();
				for(;it!=_subFile.samples.end();it++)
				{
					GetGraph()->freeMediaSample(*it);
				}
				_subFile.samples.clear();
			}
			//close file
			if (_subFile.pFileIo.get())
			{
				_subFile.pFileIo->closefile();
			}
		}
		bool RTIRawTarget::Receive(MediaSample* pSample, int nInputIndex)
		{
			// make sure no error happen
			if (GetGraph()->IsErrorOccurred())
			{
				return false;
			}
			if (!writeSubFileNoIndex(_subFile, pSample))
				return false;
			int nDataLen = pSample->getDataLength();
			return true;
		}
		const char* RTIRawTarget::GetName()
		{
			return TARGET_TYPE_RTIRAW;
		}
		
		uint64 RTIRawTarget::getSupportFileSize()
		{	
			uint64 supportFilesize = 0;
			MOLOG(Log::L_INFO, CLOGFMT(AquaFilesetTarget, "[%s] supportFilesize-[%lld]"),_strLogHint.c_str(),supportFilesize);
			return supportFilesize;
		}
		void  RTIRawTarget::setMainFileName(const std::string& mainFileName)
		{
			_mainFileName = mainFileName;
		}
		void RTIRawTarget::delOutput()
		{
			if (_DeleteTargetOnFail == 1)
			{
				bool bret = deleteFile(_subFile.strFilename.c_str());
				MOLOG(Log::L_INFO, CLOGFMT(RTIRawTarget, "delete file [%s] %s"),
					_subFile.strFilename.c_str(), bret?"success":"failure");
			}
			else if (_DeleteTargetOnFail == 2)
			{
				std::string newName = _subFile.strFilename + std::string("_bak");

				bool bret = _pFileIoFac->moveFile(_subFile.strFilename.c_str(),newName.c_str());
				MOLOG(Log::L_INFO, CLOGFMT(RTIRawTarget, "rename file [%s] to [%s] %s"),
					_subFile.strFilename.c_str(), newName.c_str(), bret?"success":"failure");
			}
		}
		bool RTIRawTarget::writeFile(std::auto_ptr<FileIo>& pFileIo, char* pBuf, int dwBufLen)
		{
			uint64 ut1 = SYS::getTickCount();
			unsigned int nWrite;
			if (!pFileIo->writefile(pBuf, dwBufLen, nWrite))
				return false;

			if (pFileIo.get() == _subFile.pFileIo.get())
			{
				// only main file need the update the process bytes
				IncProcvBytes(dwBufLen);

				if (_bEnableMD5)
				{
					//main file
					_md5ChecksumUtil.checksum(pBuf, nWrite);
				}
			}

			uint uDef = SYS::getTickCount() - ut1;
			if (uDef >= _nWriteLatencyWarningMs)
			{
				//print warning
				MOLOG(Log::L_WARNING, CLOGFMT(RTIRawTarget, "[%s] write %d bytes spent %d ms"), _strLogHint.c_str(), dwBufLen, uDef);
			}

			return true;
		}
		void RTIRawTarget::Close()
		{
			if (_bCleanup)
				return;

			closeSubFiles();

			// delete files if error happend
			processOutPut();

			_bCleanup = true;
			MOLOG(Log::L_INFO, CLOGFMT(RTIRawTarget, "[%s] closed"), _strLogHint.c_str());
		}
	}}
