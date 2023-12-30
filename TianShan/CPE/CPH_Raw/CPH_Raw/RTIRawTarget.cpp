#include "Log.h"
#include "RTIRawTarget.h"
#include "ErrorCode.h"
#include "SystemUtils.h"

#define FsIo			"FsIo"
#define MOLOG (*_pLog)

using namespace ZQ::common;
using namespace std;

namespace ZQTianShan {
	namespace ContentProvision {
		RTIRawTarget::RTIRawTarget(FileIoFactory* pFileIoFac)
		{
			_pFileIoFac = pFileIoFac;
			_subFile.reservedBW = 0;
			_subFile.pacingIndexCtx = 0;	//context of pacing
			_subFile.llProcOffset = 0;	
			_nWriteLatencyWarningMs = 1000;
		}

		RTIRawTarget::~RTIRawTarget(void)
		{
		}

		void RTIRawTarget::fileIoFailure(std::auto_ptr<FileIo>& pFileIo, const char* szLogHeader, bool bSetLastError)
		{
			std::string strErr;
			int nErrorCode;
			pFileIo->getLastError(strErr, nErrorCode);
			if (bSetLastError)
				SetLastError(strErr, nErrorCode);			
			MOLOG(Log::L_ERROR, CLOGFMT(FsIo, "[%s] Failed to %s with error: %s"),
				_strLogHint.c_str(), szLogHeader, strErr.c_str());
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
				MOLOG(Log::L_ERROR, CLOGFMT(FsIo, "[%s] Failed to create output file: %s with error: %s"),
					_strLogHint.c_str(), _subFile.strFilename.c_str(), strErr.c_str());
				return false;
			}
			MOLOG(Log::L_INFO, CLOGFMT(FsIo, "open file [%s] successful"),
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
			_subFile.strFilename = _strFilename;
			MOLOG(Log::L_INFO, CLOGFMT(FsIo, "[%s] subFile setting is successful"), _strLogHint.c_str());
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
				MOLOG(Log::L_WARNING, CLOGFMT(FsIo, "[%s]sample queue size is %d while close"), 
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
			IncProcvBytes(nDataLen);
			return true;
		}
		bool RTIRawTarget::writeSubFileNoIndex(SubFile& subFile, MediaSample* pSample)
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
			//GetGraph()->setTotalBytes(subFile.llProcOffset);
			GetGraph()->freeMediaSample(pSample);
			return true;
		}
		bool RTIRawTarget::writeData(SubFile& subFile, char* pBuf, int dwBufLen)
		{
			return writeFile(subFile.pFileIo, pBuf, dwBufLen);
		}

		bool RTIRawTarget::writeFile(std::auto_ptr<FileIo>& pFileIo, char* pBuf, int dwBufLen)
		{
			uint64 ut1 = SYS::getTickCount();
			unsigned int nWrite;
			if (!pFileIo->writefile(pBuf, dwBufLen, nWrite))
				return false;
			uint uDef = SYS::getTickCount() - ut1;
			if (uDef >= _nWriteLatencyWarningMs)
			{
				//print warning
				MOLOG(Log::L_WARNING, CLOGFMT(FsIo, "[%s] write %d bytes spent %d ms"), _strLogHint.c_str(), dwBufLen, uDef);
			}
			return true;
		}

		const char* RTIRawTarget::GetName()
		{
			return TARGET_TYPE_RTIRAW;
		}
		void RTIRawTarget::setFilename(const char* szFile)
		{
			_strFilename = szFile;
		}
		
}}//namespace